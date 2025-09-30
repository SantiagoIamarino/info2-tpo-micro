/*
 * SYSTICK.h
 *
 *  Created on: 10 jun. 2025
 *      Author: gusta
 */

#ifndef SYSTICK_DRIVER_SYSTICK_H_
#define SYSTICK_DRIVER_SYSTICK_H_

#define MAX_TICKS		0xFFFFFF
#define FREQ_PRINCIPAL	(12000000UL)

extern "C" {
void SysTick_Handler(void);
}

class SYSTICK {
public:
	SYSTICK(uint32_t ticks);
};

#endif /* SYSTICK_DRIVER_SYSTICK_H_ */
