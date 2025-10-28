#include "suenio.h"


#define MIN_PPM						30
#define MAX_PPM						200
#define LIMITE_PULSO_DORMIDO 		100 // pulsaciones por minuto
#define SIN_MOVIMIENTO_SEG 			10 // en segundos
#define PULSO_DEBAJO_LIMITE_SEG		10 // en segundos

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

uint8_t tiempo_sin_movimiento = 0;
uint8_t tiempo_pulso_bajo = 0;
uint8_t sumatoria_ppm;

int32_t tiempo_hasta_despertar = -1;
uint8_t log_counter = 0;

SuenioCFG* suenio_cfg = nullptr;

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
				tiempo_hasta_despertar = (int32_t)suenio_config->horas_suenio * (int32_t)(60 * 60) ;
			}
			break;
		case ESTADO_HORA_DE_DESPERTAR:
			// activo actuators progresivamente ACTUADORES_MAQUINA_ESTADO
			// QUEDO PENDIENTE A NOTIFICACION DESDE LA PC (EL USUARIO YA DESPERTO) o movimiento (podria agregarse un pulsador)
			break;
		default:
			break;
	}
}

static void uart_send_2d(uint8_t n) {
    Uart0.PushTx((uint8_t)('0' + (n / 10)));
    Uart0.PushTx((uint8_t)('0' + (n % 10)));
}

void enviar_info_fisiologica(uint32_t tiempo_seg) {
	if(suenio_cfg == nullptr) return;

	uint8_t promedio_ppm = (uint8_t)(sumatoria_ppm / INFO_FISIOLOGICA_CADA);
	if(promedio_ppm < MIN_PPM || promedio_ppm > MAX_PPM) {
		sumatoria_ppm = 0;
		return;
	}

	MPU_ACC.pause(); MAX_SENSOR.pause(); // pausar sensores brevemente para evitar pedida de bytes en uart

    Uart0.Send((uint8_t*)"<INFO_FISIO:PF_ID=", 0);
    uart_send_2d(suenio_cfg->profile_id);
    Uart0.Send((uint8_t*)";PPM=", 0);
    uart_send_2d(promedio_ppm);
    Uart0.Send((uint8_t*)">\r\n", 0);

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

		if(sumatoria_ppm > 0) {
			sumatoria_ppm += MAX_SENSOR.Get_PPM();
		} else {
			sumatoria_ppm = MAX_SENSOR.Get_PPM();
		}

		tiempo_hasta_despertar--;

		if(++log_counter >= INFO_FISIOLOGICA_CADA) {
			enviar_info_fisiologica(tiempo_hasta_despertar);
			log_counter = 0;
		}

	}
}



