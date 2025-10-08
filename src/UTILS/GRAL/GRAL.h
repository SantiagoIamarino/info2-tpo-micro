/*
 * GRAL.h
 *
 *  Created on: Oct 7, 2025
 *      Author: santiagoiamarino
 */

#ifndef UTILS_GRAL_GRAL_H_
#define UTILS_GRAL_GRAL_H_

#include "tipos.h"
#include "UART0.h"

#ifndef __NOP
  #ifdef __GNUC__
    #define __NOP() __asm volatile ("nop")
  #else
    #define __NOP() __nop()
  #endif
#endif


extern UART0 Uart0;

void delay();

void log_debug(uint8_t* text, int length);

#endif /* UTILS_GRAL_GRAL_H_ */
