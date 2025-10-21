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

bool STR_Comparar(const uint8_t* s1, const uint8_t* s2) {
    while (*s1 && *s2) {         // mientras no llegue al final
        if (*s1 != *s2)          // si algun caracter difiere
            return false;            // son distintos
        s1++;
        s2++;
    }
    return (*s1 == *s2) ? true : false; // iguales solo si ambos terminaron al mismo tiempo
}
