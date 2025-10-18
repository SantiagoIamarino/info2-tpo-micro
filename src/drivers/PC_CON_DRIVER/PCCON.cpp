/*
 * PCCON.cpp
 *
 *  Created on: Oct 17, 2025
 *      Author: santiagoiamarino
 */

#include "PCCON.h"

UART0	Uart0(9600);

PC_CON::PC_CON() {
	// TODO Auto-generated constructor stub

}

void init_cb(PC_CON* self) {
	self->initiated = true;
}

void PC_CON::init() {
	Uart0.Send((uint8_t*)"<PING>", 0);
	this->cmd_a_buscar = (uint8_t*)"PONG";
	this->cb = init_cb;

	while(!this->initiated){};

	Uart0.Send((uint8_t*)"IN", 0);
	this->cmd_a_buscar = nullptr;
	this->cb = nullptr;
}

void PC_CON::Procesar_Mensaje(uint8_t byte){
	if(!start_found && byte == (uint8_t)'<') { // busco comienzo de comando
		start_found = true;
		return;
	}

	if(cmd_found && byte == '>'){ // busco fin de comando
		cmd_index = 0;
		start_found = false;
		cmd_found = false;
		cmd_a_buscar = nullptr;
		attempts = 0;
		this->cb(this);
	} else {
		cmd_found = false;
	}

	if(!start_found) {
		return;
	}

	if(byte == cmd_a_buscar[cmd_index]){ // comparo letra a letra
		cmd_index++;
	} else {
		cmd_index = 0;
		cmd_found = false;
	}

	if(Uart0.CADENAS_Strlen(cmd_a_buscar) == cmd_index) { // comando encontrado, paso a buscar fin
		cmd_found = true;
		cmd_index = 0;
		start_found = false;
	}
}

PC_CON::~PC_CON() {
	// TODO Auto-generated destructor stub
}

