/*
 * GPIO.cpp
 *
 *  Created on: 27 may. 2025
 *      Author: gusta
 */

#include "Defines.h"

void Gpio::Set(uint32_t state){
	if(state)
		GPIO->SET[port] |= (1 << pin);
	else
		GPIO->CLR[port] |= (1 << pin);
}

uint32_t Gpio::Get(void){
	if(GPIO->PIN[port] & (1 << pin))
		return 1;
	else
		return 0;
}

void Gpio::Dir(uint32_t _dir){
	dir = _dir;

	if(dir)
		GPIO->DIR[port] |= (1 << pin);
	else
		GPIO->DIR[port] &= ~(1 << pin);
}

Gpio::Gpio(uint32_t _port, uint32_t _pin, uint32_t _dir){
	SYSCON->SYSAHBCLKCTRL0 |= (1 << 7);								// 7 = SWM
	SYSCON->SYSAHBCLKCTRL0 |= (1 << 6) | (1 << 20) | (1 << 18);		// 6 = GPIO0	20 = GPIO1	18 = IOCON

	port = _port;
	pin = _pin;

	Dir(_dir);
}
