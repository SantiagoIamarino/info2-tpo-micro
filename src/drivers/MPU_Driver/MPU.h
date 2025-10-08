/*
 * MPU.h
 *
 *  Created on: Oct 7, 2025
 *      Author: santiagoiamarino
 */

#ifndef DRIVERS_MPU_H_
#define DRIVERS_MPU_H_

#include <cstdio>
#include "tipos.h"
#include "GPIO.h"
#include "UART0.h"
#include "I2C.h"
#include "GRAL.h"

#define MPU_ADDR 			0x68
#define REG_PWR_MGMT_1 		0x6B
#define REG_WHO_AM_I 		0x75
#define REG_ACCEL_XOUT_H 	0x3B

class MPU {
public:
	MPU();

	bool init();
	void read();
	char* i16toa(int16_t v, char* p);
	void log_acc(int16_t ax, int16_t ay, int16_t az);

	bool initiated;
	virtual ~MPU();
};

#endif /* DRIVERS_MPU_H_ */
