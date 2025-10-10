/*
 * MAX.cpp
 *
 *  Created on: Oct 7, 2025
 *      Author: santiagoiamarino
 */

#include "MAX.h"

#define MAX_FREC_LECTURA 10 // en ms

I2C I2C_MAX(10, 11, 100);
MAX* MAX::s_self = nullptr;
TIMER* tick_MAX;

MAX::MAX() {
	this->initiated = false;
	this->filtro_initiated = false;
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

    // 3) SPO2: range=0 (2048nA), sample rate=100Hz (3<<2), pulse width=411us/18-bit (3)
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

    // MAX tick timer (lee cada MAX_FREC_LECTURA)
    tick_MAX = new TIMER(10, MAX_FREC_LECTURA, MAX::tick_read);

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
	uint8_t avail=0;
	if (s_self->read(&red, &ir, &avail)) {

		if (ir < 1500) { // reflexion muy baja, sin contacto con piel probablemente
			log_debug((uint8_t*)"SENSOR MAX SIN CONTACTO\r\n", 0);
			s_self->filtro_initiated = false;
			s_self->pulso_state = "SIN_CONTACTO";
			return;
		}

		uint32_t valor_filtrado = s_self->pulso_filtro(ir);

		switch(s_self->pulso_state){
			case "SIN_CONTACTO": // sensor MAX sin contacto con la piel
				break;
			case "ESTABILIZANDO": // espero TIEMPO_ESTABILIZACION desde que se detecta el dedo
				break;
			case "BUSCANDO_PICO": // espera hasta que se detecte el pico (cambio de tendencia, maximo y luego hacia ABAJO)
				break;
			case: "BUSCANDO_PISO": // espera hasta que se detecte un piso (cambio de tendencia, MINIMO y luego hacia ARRIBA)
				break;
			case: "PISO_DETECTADO": // se guarda el tiempo que tomo entre PICO y PISO, se calculan las PPM y se vuelve a el estado BUSCANDO_PISO
				break;
			default:
				break;
		}

		char line[64];
		int n = snprintf(line, sizeof(line), "IR=%ld IR_FILTRADO=%ld (avail=%u)\r\n",
						 (long)ir, (long)valor_actual, avail);
		if (n>0) log_debug((uint8_t*)line, n);
	}
	else {
			log_debug((uint8_t*)"MAX read error\r\n", 0);
	}
}


bool MAX::read(uint32_t* red, uint32_t* ir, uint8_t* avail_out) {
    uint8_t wr=0, rd=0;
    if (!I2C_MAX.readReg(MAX_ADDR, REG_FIFO_WR_PTR, wr)) return false;  // FIFO_WR_PTR
    if (!I2C_MAX.readReg(MAX_ADDR, REG_FIFO_RD_PTR, rd)) return false;  // FIFO_RD_PTR
    uint8_t avail = (wr - rd) & 0x1F;                    // profundidad FIFO=32
    if (avail_out) *avail_out = avail;
    if (avail == 0) return false;

    uint8_t buf[32*6];
    uint16_t nbytes = (uint16_t)avail * 6;
    if (!I2C_MAX.readBytes(MAX_ADDR, REG_FIFO_DATA, buf, nbytes)) return false;; // FIFO_DATA

    // 18 bits por canal, mask 0x3FFFF. Orden: RED luego IR (SpO2)
    uint32_t r = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    uint32_t i = ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 8) | buf[5];
    r &= 0x3FFFF; i &= 0x3FFFF;
    *red = r; *ir = i;
    return true;


	// usar la última (la más nueva)
	const uint8_t* p = &buf[(avail-1)*6];
	*red = (((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2]) & 0x3FFFF;
	*ir  = (((uint32_t)p[3] << 16) | ((uint32_t)p[4] << 8) | p[5]) & 0x3FFFF;
	return true;
}

MAX::~MAX() {
	// TODO Auto-generated destructor stub
}

