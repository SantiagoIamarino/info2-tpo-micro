/*
 * PCCON.cpp
 *
 *  Created on: Oct 17, 2025
 *      Author: santiagoiamarino
 */

#include "PCCON.h"

#define INIT_MAX_ATTEMPTS 10
#define MAX_NO_RESP_COUNTER 99999999

UART0	Uart0(9600);

PC_CON::PC_CON() {
	// TODO Auto-generated constructor stub

}

bool PC_CON::Leer_Resp(uint8_t* resp_a_buscar, bool debug = false) {
	uint16_t resp_index = 0;
	bool resp_found = false;
	uint32_t no_resp_counter = 0;

	int32_t byte;
	while(no_resp_counter < MAX_NO_RESP_COUNTER){
		byte = Uart0.PopRx();
		if(byte < 0) {
			no_resp_counter++;
			continue;
		}

		if(debug) {
			Uart0.PushTx((uint8_t)'?');
			if((uint8_t)byte != (uint8_t)'<'){
				Uart0.PushTx((uint8_t)byte);
			} else {
				Uart0.PushTx((uint8_t)'$');
			}
		}

		if(byte == resp_a_buscar[resp_index]){ // comparo byte a byte
			resp_index++;
		} else if(resp_index > 0) { // habia encontrado al menos un byte y luego fallo
			resp_found = false;
			continue;
		}

		if(Uart0.CADENAS_Strlen(resp_a_buscar) == resp_index) { // respuesta encontrada
			resp_found = true;
			break;
		}
	};

	return resp_found;
}

bool PC_CON::Leer_Resp_Con_Reintentos(uint8_t* resp_a_buscar, uint32_t max_intentos = 3, bool debug = false) {
	uint8_t attempts = 1;

	while(!this->Leer_Resp(resp_a_buscar, debug) && attempts < max_intentos) {
		attempts++;
		delay();
	}

	if(attempts >= max_intentos) {
		Uart0.Send((uint8_t*)"<ERR:RESP_NOT_FOUND>", 0);
		return false;
	}

	return true;
}

void PC_CON::init() {
	this->Enviar_Comando((uint8_t*)"<PING>");

	if(Leer_Resp_Con_Reintentos((uint8_t*)"<PONG>", INIT_MAX_ATTEMPTS)){
		this->initiated = true;
	}
}

void PC_CON::Enviar_Comando(uint8_t* cmd) {
	Uart0.Send((uint8_t*)cmd, 0);
}

bool PC_CON::Obtener_Configuracion(SuenioCFG* cfg){

	if(!this->initiated) {
		return false;
	}

	this->Enviar_Comando((uint8_t*)"<REQ_CONFIG>");

	if(!Leer_Resp_Con_Reintentos((uint8_t*)"<ACK_REQ_CONFIG>")){
		return false;
	}

	if(!Leer_Resp_Con_Reintentos((uint8_t*)"<CFG:HORAS_SUENIO=", 3, true)){
		return false;
	}

	Uart0.PushTx((uint8_t)'!');

	return true;

}

PC_CON::~PC_CON() {
	// TODO Auto-generated destructor stub
}

