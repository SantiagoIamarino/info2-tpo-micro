/*
 * GPIOPWM.h
 *
 *  Created on: Nov 19, 2025
 *      Author: santiagoiamarino
 */

#include "GPIO.h"
#include "CALLBACK.h"

#ifndef DRIVERS_GPIOPWM_DRIVER_GPIOPWM_H_
#define DRIVERS_GPIOPWM_DRIVER_GPIOPWM_H_

class GPIOPWM : public Gpio, public CALLBACK {
public:
	GPIOPWM(uint32_t _port, uint32_t _pin);
	void SetPWM(uint8_t porcentaje_on);
	void PWM();
	virtual ~GPIOPWM();

	void FastCallBack( void );

	uint8_t porcentaje_on = 0;
	uint8_t contador = 100;
};

#endif /* DRIVERS_GPIOPWM_DRIVER_GPIOPWM_H_ */
