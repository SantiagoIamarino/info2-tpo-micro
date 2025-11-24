/*
 * GPIOPWM.cpp
 *
 *  Created on: Nov 19, 2025
 *      Author: santiagoiamarino
 */

#include "GPIOPWM.h"

GPIOPWM::GPIOPWM(uint32_t _port, uint32_t _pin): Gpio(_port, _pin, OUTPUT), CALLBACK(true) {
	// TODO Auto-generated constructor stub

}

void GPIOPWM::SetPWM(uint8_t _porcentaje_on) {
	if(_porcentaje_on > 100 ||  _porcentaje_on < 0 ) return;

	porcentaje_on = _porcentaje_on;
}

void GPIOPWM::FastCallBack( void ){
	PWM();
}

void GPIOPWM::PWM(void){
	if(contador < porcentaje_on){
		Set(1);
	} else {
		Set(0);
	}

	contador++;

	if(contador >= 100) {
		contador = 0;
	}
}

GPIOPWM::~GPIOPWM() {
	// TODO Auto-generated destructor stub
}

