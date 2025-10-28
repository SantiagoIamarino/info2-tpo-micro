/*
 * suenio.h
 *
 *  Created on: Oct 17, 2025
 *      Author: santiagoiamarino
 */

#ifndef UTILS_SUENIO_MAQ_ESTADOS_SUENIO_H_
#define UTILS_SUENIO_MAQ_ESTADOS_SUENIO_H_

#include <cstdio>
#include "tipos.h"
#include "GPIO.h"
#include "UART0.h"
#include "I2C.h"
#include "GRAL.h"
#include "CALLBACK.h"
#include "TIMER.h"
#include "UART0.h"
#include "GRAL.h"
#include "I2C.h"
#include "MAX.h"
#include "MPU.h"
#include "suenio.h"
#include "PCCON.h"

void suenio_maq_estados(SuenioCFG* suenio_config);
void suenio_tick(void);

#endif /* UTILS_SUENIO_MAQ_ESTADOS_SUENIO_H_ */
