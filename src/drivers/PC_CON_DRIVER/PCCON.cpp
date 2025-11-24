/*
 * PCCON.cpp
 *
 *  Created on: Oct 17, 2025
 *      Author: santiagoiamarino
 */

#include "PCCON.h"
#include "cfg_maq_estados.h"

#define INIT_MAX_ATTEMPTS 10
#define MAX_NO_RESP_COUNTER 99999999

#define TIEMPO_CAIDA_TICK_MS 10
#define WAIT_TIME_CAIDA_MS 500
#define MAX_REINTENTOS_CAIDA 10

extern bool cfg_msg_done;
extern bool cfg_msg_error;

static const uint8_t* POSIBLE_CAIDA_CMD = (uint8_t*)"<POSIBLE_CAIDA>";
static const uint8_t* ACK_STR = (uint8_t*)"ACK_CAIDA";

enum StateAckCaida : uint8_t {
	S_ACK_WAIT_LT = 0,   // esperando '<'
	S_ACK_CHECK,         // comparando "ACK_CAIDA"
	S_ACK_DONE,
	S_ACK_ERROR
};
static StateAckCaida ack_state = S_ACK_WAIT_LT;
static uint8_t ack_index = 0;

UART0	Uart0(9600);
TIMER* timer_posible_caida;

PC_CON* PC_CON::s_self = nullptr;

void posible_caida_tick(void);

PC_CON::PC_CON() {
	// TODO Auto-generated constructor stub
	s_self = this;
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
			/*if((uint8_t)byte != (uint8_t)'<'){
				Uart0.PushTx((uint8_t)byte);
			} else {
				Uart0.PushTx((uint8_t)'$');
			}*/
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

bool PC_CON::Obtener_Respuesta(uint8_t** buf, uint8_t resp_length){
	uint8_t i = 0;
	uint8_t attempts = 0;
	bool se_obtuvo_resp = false;

	while(attempts < MAX_NO_RESP_COUNTER) {
		attempts++;

		int32_t byte = Uart0.PopRx();
		if(byte < 0) continue;

		(*buf)[i] = (uint8_t)byte;
		i++;

		if(i == resp_length) {
			se_obtuvo_resp = true;
			break;
		}
	}

	return se_obtuvo_resp;
}

bool PC_CON::Obtener_Configuracion(SuenioCFG* cfg){

	if(!this->initiated) {
		return false;
	}

	this->Enviar_Comando((uint8_t*)"<REQ_CONFIG>");

	if(!this->Leer_Resp_Con_Reintentos((uint8_t*)"<ACK_REQ_CONFIG>")){
		return false;
	}

	uint32_t timeout = MAX_NO_RESP_COUNTER;
	while (timeout--) { // bloqueante, usar solo en la inicializacion
		int32_t b = Uart0.PopRx();
		cfg_maq_estados(b, cfg);

		if (cfg_msg_done) {
			return true;
		}

		if (cfg_msg_error) {
			return false;
		}
	}

	// timeout sin recibir config
	return false;

}

bool PC_CON::NotificarPosibleCaida(void){
	if(!this->initiated) {
		return false;
	}

	// Reset
	this->t_esperando_resp_caida = 0;
	this->resp_caida_intentos    = 0;
	ack_state = S_ACK_WAIT_LT;
	ack_index = 0;
	this->ready = false;
	this->esperando_resp_caida = true;

	// Timer reintentos
	timer_posible_caida = new TIMER(10, TIEMPO_CAIDA_TICK_MS, PC_CON::posible_caida_tick);

	return true;
}


void PC_CON::posible_caida_tick(void){
	if(s_self->t_esperando_resp_caida == 0) { // primera vez
		s_self->Enviar_Comando((uint8_t*)POSIBLE_CAIDA_CMD);
		s_self->t_esperando_resp_caida += TIEMPO_CAIDA_TICK_MS;
		return;
	}

	s_self->t_esperando_resp_caida += TIEMPO_CAIDA_TICK_MS;

	// maq estados detectar resp ACK_STR = <ACK_CAIDA>
	int32_t b = Uart0.PopRx();
	if(b >= 0){
		switch(ack_state) {
			case S_ACK_WAIT_LT:
				if (b == '<') {
					ack_state = S_ACK_CHECK;
					ack_index = 0;
				}
				break;

			case S_ACK_CHECK:
				if (b == ACK_STR[ack_index]) {
					ack_index++;
					if (ACK_STR[ack_index] == '\0') {
						ack_state = S_ACK_DONE;
					}
				} else {
					// si no coincide vuelvo al inicio
					ack_state = (b == '<') ? S_ACK_CHECK : S_ACK_WAIT_LT;
					ack_index = 0;
				}
				break;

			case S_ACK_DONE:
				if (b == '>') {
					timer_posible_caida->Stop();
					timer_posible_caida = nullptr;
					s_self->t_esperando_resp_caida = 0;
					s_self->resp_caida_intentos = 0;
					ack_state = S_ACK_WAIT_LT;
					ack_index = 0;

					Uart0.Send((uint8_t*)"<ACK_CAIDA_RECIBIDO>", 0);

					s_self->ready = true;
					return;
				} else {
					ack_state = S_ACK_WAIT_LT;
					ack_index = 0;
				}
				break;

			case S_ACK_ERROR:
			default:
				ack_state = S_ACK_WAIT_LT;
				break;
		}
	}


	if(s_self->t_esperando_resp_caida >= WAIT_TIME_CAIDA_MS){ // supero tiempo de espera
		s_self->t_esperando_resp_caida = 0;
		ack_state = S_ACK_WAIT_LT;
		ack_index = 0;

		s_self->resp_caida_intentos++;
		if(s_self->resp_caida_intentos >= MAX_REINTENTOS_CAIDA){ // supero reintentos MAX
			s_self->resp_caida_intentos = 0;
			s_self->ready = true;
			log_debug((uint8_t*)"<ERR:POSIBLE_CAIDA>", 0);
			timer_posible_caida->Stop();
			timer_posible_caida = nullptr;
			return;
		}
	}

}

PC_CON::~PC_CON() {
	// TODO Auto-generated destructor stub
}

