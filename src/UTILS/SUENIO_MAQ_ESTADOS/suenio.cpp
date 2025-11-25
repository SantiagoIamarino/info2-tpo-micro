#include "suenio.h"


#define MIN_PPM						30
#define MAX_PPM						200
#define LIMITE_PULSO_DORMIDO 		100 // pulsaciones por minuto
#define SIN_MOVIMIENTO_SEG 			10 // en segundos
#define PULSO_DEBAJO_LIMITE_SEG		10 // en segundos
#define TIEMPO_DESPERTAR_HARDCODE   0 // 0 para no hardcodear

#define ESTADO_DESPIERTO 0
#define ESTADO_DORMIDO 1
#define ESTADO_HORA_DE_DESPERTAR 2
#define INFO_FISIOLOGICA_CADA 2 // en segundos

uint32_t estado_suenio = ESTADO_DESPIERTO;
TIMER* suenio_tick_timer = nullptr;

extern MAX MAX_SENSOR;
extern MPU MPU_ACC;
extern PC_CON PC_CONNECTION;
extern UART0 Uart0;
extern GPIOF Pulsador;
extern PC_CON PC_CONNECTION;

uint8_t tiempo_sin_movimiento = 0;
uint8_t tiempo_pulso_bajo = 0;
uint8_t sumatoria_ppm;

int32_t tiempo_hasta_despertar = -1;
uint8_t log_counter = 0;

SuenioCFG* suenio_cfg = nullptr;

void reset_maq_estados(void){
	tiempo_sin_movimiento = 0;
	tiempo_pulso_bajo = 0;
	sumatoria_ppm = 0;

	tiempo_hasta_despertar = -1;
	log_counter = 0;
}

void suenio_maq_estados(SuenioCFG* suenio_config) {
	if(!suenio_tick_timer) {
		suenio_tick_timer = new TIMER(10, 1000, suenio_tick);
	}

	if(suenio_cfg == nullptr) {
		suenio_cfg = suenio_config;
	}

	switch(estado_suenio){
		case ESTADO_DESPIERTO: // reviso parametros
			if(tiempo_sin_movimiento >= SIN_MOVIMIENTO_SEG && tiempo_pulso_bajo >= PULSO_DEBAJO_LIMITE_SEG) {
				tiempo_sin_movimiento = 0;
				tiempo_pulso_bajo = 0;

				estado_suenio = ESTADO_DORMIDO;
			}
			break;
		case ESTADO_DORMIDO: // cuando el pulso cae a LIMITE_PULSO_DORMIDO y no hubo movimientos por SIN_MOVIMIENTO_SEG
			// empiezo a descontar el tiempo (en suenio_tick_timer) hasta activar los actuadores
			// Envia informacion fisiologica LPC->ESP_32->PC
			if(tiempo_hasta_despertar == -1) { // primera vez que entra, seteo el tiempo que falta hasta despertar (en segundos)

				tiempo_hasta_despertar = (int32_t)suenio_config->horas_suenio * (int32_t)(60 * 60);

				if(suenio_config->hora_limite_seg > 0 && suenio_config->hora_limite_seg < tiempo_hasta_despertar) {
					// Si la hora limite es menor a las horas de suenio deseadas seteo hora limite
					tiempo_hasta_despertar = suenio_config->hora_limite_seg;
				}

				if(TIEMPO_DESPERTAR_HARDCODE > 0){ // en el caso que necesite hardcodear el tiempo hasta despertar (pruebas/debug)
					tiempo_hasta_despertar = TIEMPO_DESPERTAR_HARDCODE;
				}
			}
			break;
		case ESTADO_HORA_DE_DESPERTAR:
			// activo actuadores progresivamente
			activarActuadores(true);

			// QUEDO PENDIENTE A el pulsador (el usuario ya despertó)
			if(Pulsador.Get() == ESTADO_PULSADOR_ON){
				// PARAR ACTUADORES
				pararActuadores();

				// volver a el estado inicial
				estado_suenio = ESTADO_DESPIERTO;
				reset_maq_estados();
			}
			break;
		default:
			break;
	}

	if(estado_suenio != ESTADO_DESPIERTO && MPU_ACC.Get_Posible_Caida() && PC_CONNECTION.Ready()) {
		// caida detectada, notificar pc y activar actuadores
		activarActuadores(false);
		MPU_ACC.Reset_Posible_Caida();
		PC_CONNECTION.NotificarPosibleCaida();
	}
}

void enviar_info_fisiologica(uint32_t tiempo_seg) {
	if(suenio_cfg == nullptr) return;

	uint8_t promedio_ppm = (uint8_t)(sumatoria_ppm / INFO_FISIOLOGICA_CADA);
	if(promedio_ppm < MIN_PPM || promedio_ppm > MAX_PPM) {
		sumatoria_ppm = 0;
		return;
	}

	MPU_ACC.pause(); MAX_SENSOR.pause(); // pausar sensores brevemente para evitar pedida de bytes en uart

	char buf[64];
	int n = snprintf(buf, sizeof(buf), "<INFO_FISIO:PF_ID=%02d;PPM=%02d",
					 suenio_cfg->profile_id, promedio_ppm);

	// checksum
	uint8_t checksum = 0;
	// Sumar todos los bytes entre '<>' (no tomo primer valor buf[0])
	for (int i = 1; i < n; i++) {
		checksum += (uint8_t)buf[i];
	}

	// agrego ";CS=NN>" a la trama
	int m = snprintf(buf + n, sizeof(buf) - n, ";CS=%02X>\r\n", checksum);
	if (m <= 0 || (n + m) >= (int)sizeof(buf)) {
		// en casi de error
		MPU_ACC.resume();
		MAX_SENSOR.resume();
		sumatoria_ppm = 0;
		return;
	}

    Uart0.Send((uint8_t*)buf, 0);

    MPU_ACC.resume(); MAX_SENSOR.resume(); // reanudo sensores
    sumatoria_ppm = 0;
}

void suenio_tick(void) {
	if(estado_suenio == ESTADO_DESPIERTO) {
		Uart0.Send((uint8_t*)"D", 0);
		if(MPU_ACC.Get_Estado_Movimiento() == ACC_ESTADO_QUIETO){
			tiempo_sin_movimiento++;
		} else {
			tiempo_sin_movimiento = 0;
		}

		if(MAX_SENSOR.Get_PPM() >= MIN_PPM && MAX_SENSOR.Get_PPM() <= LIMITE_PULSO_DORMIDO) {
			tiempo_pulso_bajo++;
		} else {
			tiempo_pulso_bajo = 0;
		}
	}

	if(estado_suenio == ESTADO_DORMIDO) {
		if (tiempo_hasta_despertar == 0) {
			estado_suenio = ESTADO_HORA_DE_DESPERTAR;
			return;
		}

		tiempo_hasta_despertar--;

		if(MAX_SENSOR.Get_PPM() > PPM_MIN_VALID && MAX_SENSOR.Get_PPM() < PPM_MAX_VALID) {
			if(sumatoria_ppm > 0) {
				sumatoria_ppm += MAX_SENSOR.Get_PPM();
			} else {
				sumatoria_ppm = MAX_SENSOR.Get_PPM();
			}

			if(++log_counter >= INFO_FISIOLOGICA_CADA) {
				enviar_info_fisiologica(tiempo_hasta_despertar);
				log_counter = 0;
			}
		}
	}
}

void hora_limite_tick(void) {
	if(suenio_cfg->hora_limite_seg > 0){
		suenio_cfg->hora_limite_seg--;
	}
}


