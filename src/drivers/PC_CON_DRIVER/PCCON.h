/*
 * PCCON.h
 *
 *  Created on: Oct 17, 2025
 *      Author: santiagoiamarino
 */

#ifndef DRIVERS_PC_CON_DRIVER_PCCON_H_
#define DRIVERS_PC_CON_DRIVER_PCCON_H_

#include <cstdio>
#include "tipos.h"
#include "LPC845.h"
#include "Hardware.h"
#include "GPIO.h"
#include "SYSTICK.h"
#include "CALLBACK.h"
#include "TIMER.h"
#include "UART0.h"

class PC_CON {
public:
	using CallbackFn = void (*)(PC_CON* self);

	PC_CON();
	void init();
	virtual ~PC_CON();

	bool initiated = false;

	uint32_t cmd_index = 0;
	bool start_found = false;
	bool cmd_found = false;
	uint8_t* cmd_a_buscar = nullptr;
	uint32_t attempts = 0;

	CallbackFn cb = nullptr;

	void Procesar_Mensaje(uint8_t byte);
};

#endif /* DRIVERS_PC_CON_DRIVER_PCCON_H_ */
