/*
 * TIMER.cpp
 *
 *  Created on: 10 jun. 2025
 *      Author: gusta
 */

#include "Defines.h"

TIMER::TIMER(uint32_t timeStart, uint32_t timeReload, void (*func)(void)){
	Start(timeStart, timeReload, func);
}

void TIMER::Start(uint32_t timeStart, uint32_t timeReload, void (*func)(void)){
	this->time = timeStart;
	this->timeReload = timeReload;
	this->func = func;
}

void TIMER::Discount(void){
	if(this->stopped) return;

	if(this->time > 0){
		this->time--;
		if(0 == this->time){
			this->time = this->timeReload;

			this->func();
		}
	}
}

void TIMER::Callback(void){
	Discount();
}
