/*
 * GPIOF.cpp
 *
 *  Created on: 24 jun. 2025
 *      Author: gusta
 */
#include "GPIOF.h"

GPIOF::GPIOF(uint32_t port, uint32_t pin) : Gpio(port, pin, INPUT){
	contador = 0;
	estAnterior = 0;
	estEstable = 0;
}

void GPIOF::Antirebote(void){
	uint32_t estActual = Gpio::Get();

	if(estActual != estEstable){
		contador++;
		if(contador == CANTIDAD_ESTADOS_ESTABLES){
			estEstable = estActual;
		}
	} else {
		contador = 0;
	}

}

void GPIOF::Callback( void ){
	Antirebote();
}

uint32_t GPIOF::Get( void ){
	return estEstable;
}
