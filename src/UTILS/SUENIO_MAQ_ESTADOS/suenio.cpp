#include "suenio.h"
#include <cstdio>
#include "tipos.h"

#define ESTADO_DESPIERTO 0
#define ESTADO_DORMIDO 1
#define ESTADO_HORA_DE_DESPERTAR 2

uint32_t estado_suenio = ESTADO_DESPIERTO;

void suenio_maq_estados(void) {
	switch(estado_suenio){
		case ESTADO_DESPIERTO: // reviso parametros constantemente
			break;
		case ESTADO_DORMIDO: // cuando el pulso cae a LIMITE_PULSO_DORMIDO & no hubo movimientos por SIN_MOVIMIENTO_MIN
			// seteo timer y empiezo a descontar el tiempo hasta activar los actuadores
			// empiezo a enviar informacion fisiologica por el ESP_32
			break;
		case ESTADO_HORA_DE_DESPERTAR:
			// activo actuadores progresivamente ACTUADORES_MAQUINA_ESTADO
			// QUEDO PENDIENTE A NOTIFICACION DESDE LA PC (EL USUARIO YA DESPERTO)
			break;
		default:
			break;
	}
}



