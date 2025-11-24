/*
 * cfg_maq_estados.h
 *
 *  Created on: Nov 8, 2025
 *      Author: santiagoiamarino
 */

#ifndef UTILS_CFG_MAQ_ESTADOS_CFG_MAQ_ESTADOS_H_
#define UTILS_CFG_MAQ_ESTADOS_CFG_MAQ_ESTADOS_H_

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


void cfg_maq_estados(int32_t b, SuenioCFG* suenio_config, bool es_update = false);



#endif /* UTILS_CFG_MAQ_ESTADOS_CFG_MAQ_ESTADOS_H_ */
