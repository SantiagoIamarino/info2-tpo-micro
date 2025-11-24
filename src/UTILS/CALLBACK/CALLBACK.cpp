/*
 * CALLBACK.cpp
 *
 *  Created on: 23 ago. 2023
 *      Author: gusta
 */

#include "Defines.h"

std::vector<CALLBACK*> vCallBack;
std::vector<CALLBACK*> vFastCallBack;

void CALLBACK::Callback( void )
{
	// Hay que crearla vacia, es la que luego se carga en la herencia
}

void CALLBACK::FastCallBack( void )
{
	// Hay que crearla vacia, es la que luego se carga en la herencia
}

CALLBACK::CALLBACK(bool fast)
{
	if(!fast) {
		vCallBack.push_back( this );
	} else {
		vFastCallBack.push_back( this );
	}

}

uint8_t counter = 0;

void CALLBACK::SysTickCall( void )
{

	// FastCallback cada 0.1ms
	for (CALLBACK* q : vFastCallBack )
		q->FastCallBack();

	// Callback cada 1ms
	counter++;
	if(counter >= 10) {
		counter = 0;

		for (CALLBACK* q : vCallBack )
			q->Callback();
	}

}
