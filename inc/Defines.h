

#include <cr_section_macros.h>
#include <vector>
#include <cstdio>
#include "tipos.h"
#include "LPC845.h"
#include "Hardware.h"
#include "GPIO.h"
#include "SYSTICK.h"
#include "CALLBACK.h"
#include "TIMER.h"
#include "UART0.h"
#include "I2C.h"

#ifndef __NOP
  #ifdef __GNUC__
    #define __NOP() __asm volatile ("nop")
  #else
    #define __NOP() __nop()
  #endif
#endif

