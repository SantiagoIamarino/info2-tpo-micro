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

int8_t hex_digit_to_val(uint8_t c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

uint8_t hex_a_dec(const uint8_t* buf2)
{
    int8_t hi = hex_digit_to_val(buf2[0]);
    int8_t lo = hex_digit_to_val(buf2[1]);
    if (hi < 0 || lo < 0) {
        return 0;
    }
    return (uint8_t)((hi << 4) | lo);
}
