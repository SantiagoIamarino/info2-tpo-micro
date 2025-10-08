/*
 * GRAL.cpp
 *
 *  Created on: Oct 7, 2025
 *      Author: santiagoiamarino
 */

#include "GRAL.h"

void delay(){
	for (volatile int i=0; i<30000; ++i) __NOP();
}

void log_debug(uint8_t* text, int length = 0) {
	Uart0.Send((uint8_t*)text, length);
}
