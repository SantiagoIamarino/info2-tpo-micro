/*
 * GPIO.h
 *
 *  Created on: 27 may. 2025
 *      Author: gusta
 */

#ifndef GPIO_DRIVER_GPIO_H_
#define GPIO_DRIVER_GPIO_H_

#define OUTPUT	1
#define INPUT	0

class Gpio {
private:
	uint32_t port;
	uint32_t pin;
	uint32_t dir;
public:
	void Set( uint32_t state);
	void Dir( uint32_t dir);
	uint32_t Get(void);

	Gpio(uint32_t port, uint32_t pin, uint32_t dir);
};

#endif /* GPIO_DRIVER_GPIO_H_ */
