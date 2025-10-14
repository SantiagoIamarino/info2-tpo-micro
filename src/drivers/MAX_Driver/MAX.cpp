/*
 * MAX.cpp
 *
 *  Created on: Oct 7, 2025
 *      Author: santiagoiamarino
 */

#include "MAX.h"

#define FREC_TICK_READ 50 // en ms
#define FREC_MUESTREO_EN_MS 10
#define MAX_LECTURAS_POR_TICK 12
#define PULSO_PICO_NO_VAL 60000 // valor fuera de rango, es primera busqueda de pico

I2C I2C_MAX(10, 11, 100);
MAX* MAX::s_self = nullptr;
TIMER* tick_MAX;

#define PULSO_SIN_CONTACTO_UMBRAL 1500
#define MUESTRAS_ESTABILIZACION_UMBRAL_MS 6000

static constexpr uint16_t PULSO_SIN_CONTACTO = 0;
static constexpr uint16_t PULSO_ESTABILIZANDO = 1;
static constexpr uint16_t PULSO_BUSCANDO_PICO = 2;
static constexpr uint16_t PULSO_PICO_DETECTADO = 3;
static constexpr uint16_t DEBOUNCE_N      = 10;   // muestras consecutivas confirmando la pendiente
static constexpr uint16_t REFRACTORY_MS   = 300; // no aceptar otro pico antes de 300 ms (max aprox 200 ppm)
static constexpr uint16_t RR_MIN_MS       = 300; // 200 bpm
static constexpr uint32_t RR_MAX_MS       = 1500; // 40 bpm
static constexpr uint16_t MIN_AMP         = 200;  // amplitud minima picos (en unidades del filtrado)
static constexpr uint16_t MIN_THR         = 80;   // piso del umbral dinámico

#define MUESTRAS_ESTABILIZACION_UMBRAL (MUESTRAS_ESTABILIZACION_UMBRAL_MS / FREC_MUESTREO_EN_MS)

MAX::MAX() {
	this->reset_vars();
	this->initiated = false;
}

void MAX::reset_vars(void) {
	this->filtro_initiated = false;
	this->pulso_ppm_actual = 0;
	this->pulso_state = PULSO_SIN_CONTACTO;
	this->pulso_state_reset_counter = 0;
	this->pulso_state_switch_counter = 0;
	this->pulso_state_switch_back_counter = 0;
	this->buscando_picos = false;
	this->pulso_t_desde_ult_pico = PULSO_PICO_NO_VAL;
}

bool MAX::probe()
{
    // ¿está en 0x57?
    if (!I2C_MAX.scanOne(MAX_ADDR)) {
        log_debug((uint8_t*)"MAX not found at 0x57\r\n",0);
        return false;
    }

    // Leer PART ID (0xFF) -> 0x15
    uint8_t partid = 0;
    if (!I2C_MAX.readReg(MAX_ADDR, REG_PART_ID, partid)) {
        log_debug((uint8_t*)"MAX read PARTID fail\r\n",0);
        return false;
    }
    char msg[32];
    int n = sprintf(msg, "MAX PARTID=0x%02X\r\n", partid);
    log_debug((uint8_t*)msg, n);
    return (partid == 0x15);
}

bool MAX::init(void) {

	bool max_probed = false;

	for (volatile int i=0; i<100; ++i) {

		if (this->probe()) {
			log_debug((uint8_t*)"maxProbe WORKED...\r\n", 0);
			max_probed = true;
			break;
		}
		log_debug((uint8_t*)"maxProbe failed, trying again...\r\n", 0);
		delay();
	}

	if(!max_probed) {
		 log_debug((uint8_t*)"maxProbe failed, MAX ATTEMPS REACHED...\r\n", 0);
		 return false;
	}

    uint8_t v;

    // 1) Reset
    I2C_MAX.writeReg(MAX_ADDR, REG_MODE_CONFIG, 0x40);           // MODE_CONFIG: RESET
    for (int i=0;i<10000;i++) __NOP();
    // esperar a que bit de reset se limpie…
    for (int k=0;k<50;k++) {
        if (I2C_MAX.readReg(MAX_ADDR, REG_MODE_CONFIG, v) && ((v & 0x40) == 0)) break;
        for (volatile int i=0;i<3000;i++) __NOP();
    }

    // 2) FIFO: sample avg=4 (2<<5), rollover=1 (1<<4), almost_full=0x0F
    I2C_MAX.writeReg(MAX_ADDR, REG_FIFO_CONFIG, (2<<5) | (1<<4) | 0x0F);

    // 3) SPO2: range=0 (8192nA), sample rate=100Hz (3<<2), pulse width=411us/18-bit (3)
    I2C_MAX.writeReg(MAX_ADDR, REG_SPO2_CONFIG, (2<<5) | (3<<2) | 3);

    // 4) Corriente LEDs
    //    LED1=RED, LED2=IR en MAX30102
    I2C_MAX.writeReg(MAX_ADDR, REG_LED1_RED, 0x20);           // LED1_PA (RED) ~6mA
    I2C_MAX.writeReg(MAX_ADDR, REG_LED2_IR, 0x20);           // LED2_PA (IR)  ~6mA

    // 5) Limpiar punteros FIFO
    I2C_MAX.writeReg(MAX_ADDR, REG_FIFO_WR_PTR, 0x00);           // FIFO_WR_PTR
    I2C_MAX.writeReg(MAX_ADDR, REG_OVF_COUNTER, 0x00);           // OVF_COUNTER
    I2C_MAX.writeReg(MAX_ADDR, REG_FIFO_RD_PTR, 0x00);           // FIFO_RD_PTR

    // 6) SPO2 mode (RED+IR)
    I2C_MAX.writeReg(MAX_ADDR, REG_MODE_CONFIG, 0x03);           // MODE_CONFIG: SpO2


    this->initiated = true;
    s_self = this;

    // MAX tick timer (lee cada FREC_TICK_READ)
    tick_MAX = new TIMER(10, FREC_TICK_READ, MAX::tick_read);

    return true;
}

int32_t MAX::pulso_filtro(int32_t ir_val){

	if(!this->filtro_initiated){
		this->filtro_dc = ir_val;
		this->filtro_valor = 0;
		this->filtro_initiated = true;
		return 0;
	}

	if ((int32_t)ir_val - this->filtro_dc > 20000) { // reiniciar si hay una variacion muy brusca
	    filtro_dc = ir_val; filtro_valor = 0;
	}

	// quitar DC: media móvil lenta
	this->filtro_dc += (ir_val - this->filtro_dc) >> 7;      // k=7 ≈ ventana ~1.3–2.6 s
	int32_t ac = ir_val - this->filtro_dc;

	// suavizado: EMA leve (a=2..4). Mayor = más suave
	this->filtro_valor += (ac - this->filtro_valor) >> 2;   // a=2

	return this->filtro_valor;
}


void MAX::tick_read(void) {
	if (!s_self) return;

	uint32_t red, ir;
	if (!s_self->read(&red, &ir)) {
		log_debug((uint8_t*)"MAX read error\r\n", 0);
	}
}

void MAX::buscar_picos(uint32_t* ir){
	if(!s_self->buscando_picos){
		s_self->pulso_valor_previo = 0;
		s_self->pulso_sube_cnt = 0;
		s_self->pulso_baja_cnt = 0;
		s_self->buscando_picos = true;
		s_self->pulso_subio = false;
	}

	const int32_t valor_filtrado = pulso_filtro((uint32_t)*ir);

	s_self->pulso_t_desde_ult_pico += FREC_MUESTREO_EN_MS; // aumento tiempo desde el ultimo pico (en ms)

	// Derivada simple (signo de la pendiente)
	const int32_t df = valor_filtrado - s_self->pulso_valor_previo;
	s_self->pulso_valor_previo = valor_filtrado;

	if (df > 0) {            // sube
		if (s_self->pulso_sube_cnt < 255) s_self->pulso_sube_cnt++;
		if (s_self->pulso_sube_cnt >= DEBOUNCE_N) { // estuvo subiendo por DEBOUNCE_N muestras
			s_self->pulso_subio = true;
		}
		if (s_self->pulso_baja_cnt > 0){ // estaba bajando hasta la ultima muestra, guardo minimo (valle)
		    s_self->pulso_valle = s_self->pulso_valor_previo;
		}
		s_self->pulso_baja_cnt = 0;
	} else if (df < 0) {     // baja
		if (s_self->pulso_baja_cnt < 255) s_self->pulso_baja_cnt++;
		s_self->pulso_sube_cnt = 0;
	}

	// Refractario: no permitir otro pico demasiado cerca
	if (s_self->pulso_t_desde_ult_pico < REFRACTORY_MS) return;

	if (s_self->pulso_baja_cnt >= DEBOUNCE_N && s_self->pulso_subio) { // estuvo a la baja por DEBOUNCE_N muestras y antes habia subido (paso un pico)

		if(s_self->pulso_t_desde_ult_pico >= PULSO_PICO_NO_VAL) { // es el primer pico encontrado, reinicio y busco proximo
			s_self->pulso_sube_cnt = 0;
			s_self->pulso_baja_cnt = 0;
			s_self->pulso_t_desde_ult_pico = 0;
			s_self->pulso_subio = false;
			return;
		}

		int32_t amp = s_self->pulso_valor_previo - s_self->pulso_valle;
		amp = (amp < 0 ? -amp : amp); // valor absoluto de la amplitud

		if (
				s_self->pulso_t_desde_ult_pico >= RR_MIN_MS && s_self->pulso_t_desde_ult_pico <= RR_MAX_MS // esta en valores razonables?
				&& amp >= MIN_AMP // pico demasiado chico, ignoro
			)
		{
			s_self->pulso_state = PULSO_PICO_DETECTADO;
		} else if(s_self->pulso_t_desde_ult_pico >= RR_MAX_MS){ // se paso, descarto
			s_self->pulso_sube_cnt = 0;
			s_self->pulso_baja_cnt = 0;
			s_self->pulso_t_desde_ult_pico = 0;
			s_self->pulso_subio = false;
		}
	}
}

void MAX::pulso_maq_estados(uint32_t* ir) {
	switch(s_self->pulso_state){
		case PULSO_SIN_CONTACTO: // sensor MAX sin contacto con la piel
			if(*ir > PULSO_SIN_CONTACTO_UMBRAL) {
				s_self->pulso_state_switch_counter++;
				if(s_self->pulso_state_switch_counter == 5) { // avanza a partir de las 5 muestras con datos
					s_self->pulso_state_switch_counter = 0;
					s_self->pulso_state = PULSO_ESTABILIZANDO;
				}
			} else {
				s_self->pulso_state_switch_counter = 0;
			}
			break;
		case PULSO_ESTABILIZANDO: // espero MUESTRAS_ESTABILIZACION desde que se detecta el dedo
			s_self->pulso_state_switch_counter++;
			if(s_self->pulso_state_switch_counter == MUESTRAS_ESTABILIZACION_UMBRAL) {
				s_self->pulso_state_switch_counter = 0;
				s_self->pulso_state = PULSO_BUSCANDO_PICO;
			}
			break;
		case PULSO_BUSCANDO_PICO: // espera hasta que se detecte el pico (cambio de tendencia, maximo y luego hacia ABAJO)
			s_self->buscar_picos(ir);
			break;
		case PULSO_PICO_DETECTADO: { // se guarda el tiempo que tomo entre PICOS, se calculan las PPM y se vuelve a el estado BUSCANDO_PICO

			const uint32_t ppm_inst = (uint16_t)(60000u / s_self->pulso_t_desde_ult_pico); // 60.000 ms / (tiempo entre picos)
			int32_t dif_ultimo_ppm = (int32_t)ppm_inst - (int32_t)s_self->pulso_ppm_actual;
			dif_ultimo_ppm = (dif_ultimo_ppm < 0) ? (-dif_ultimo_ppm) : dif_ultimo_ppm; // valor absoluto

			if(s_self->pulso_ppm_actual == 0) { // 1era muestra de pulso
				s_self->pulso_ppm_actual = ppm_inst;
			} else if(dif_ultimo_ppm > (s_self->pulso_ppm_actual / 4)) { // si la diferencia entre mediciones es mas del 25% -> DESCARTAR
				s_self->pulso_ppm_actual = s_self->pulso_ppm_actual; // mantengo valor anterior
			} else { // 2da o mas, tomo promedio
				s_self->pulso_ppm_actual = (uint16_t)((s_self->pulso_ppm_actual * 3 + ppm_inst) / 4);
			}
			s_self->pulso_sube_cnt = 0;
			s_self->pulso_baja_cnt = 0;
			s_self->pulso_t_desde_ult_pico = 0;
			s_self->pulso_subio = false;

			char line[64];
			int line_n = snprintf(line, sizeof(line), "PPM=%ld\r\n",
							 (long)s_self->pulso_ppm_actual);
			if (line_n>0) log_debug((uint8_t*)line, line_n);

			s_self->pulso_state = PULSO_BUSCANDO_PICO;

			break;
		}
		default:
			break;
	}
}

bool MAX::read(uint32_t* red, uint32_t* ir) {
	// 1) ver cuantas muestras pendientes hay
	uint8_t wr=0, rd=0;
	if (!I2C_MAX.readReg(MAX_ADDR, REG_FIFO_WR_PTR, wr)) return false;
	if (!I2C_MAX.readReg(MAX_ADDR, REG_FIFO_RD_PTR, rd)) return false;
	uint8_t avail = (wr - rd) & 0x1F;
	if (avail == 0) return false;

	// 2) limitar burst para no bloquear mucho
	uint8_t n = (avail > MAX_LECTURAS_POR_TICK) ? MAX_LECTURAS_POR_TICK : avail;

	// 3) leer n muestras
	static uint8_t buf[6 * MAX_LECTURAS_POR_TICK];
	if (!I2C_MAX.readBytes(MAX_ADDR, REG_FIFO_DATA, buf, 6*n)) return false;
    for (uint8_t i = 0; i < n; ++i) {
        const uint8_t* p = &buf[i*6];
        *red = (((uint32_t)p[0]<<16)|((uint32_t)p[1]<<8)|p[2]) & 0x3FFFF;
        *ir  = (((uint32_t)p[3]<<16)|((uint32_t)p[4]<<8)|p[5]) & 0x3FFFF;

        if (*ir < PULSO_SIN_CONTACTO_UMBRAL) { // reflexion muy baja, sin contacto con piel probablemente
			if(s_self->pulso_state != PULSO_SIN_CONTACTO) {
				s_self->pulso_state_reset_counter++;
				if(s_self->pulso_state_reset_counter == 5) { // avanza a partir de las 5 muestras con datos
					s_self->pulso_state = PULSO_SIN_CONTACTO;
				}
				continue;
			}

			s_self->reset_vars();
			continue;
		} else {
			s_self->pulso_state_reset_counter = 0;
		}

        this->pulso_maq_estados(ir);
    }

    return true;
}

MAX::~MAX() {
	// TODO Auto-generated destructor stub
}

