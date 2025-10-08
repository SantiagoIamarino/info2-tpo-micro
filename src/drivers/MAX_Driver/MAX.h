/*
 * MAX.h
 *
 *  Created on: Oct 7, 2025
 *      Author: santiagoiamarino
 */
#pragma once
#ifndef DRIVERS_MAX_H_
#define DRIVERS_MAX_H_

#include <cstdio>
#include "tipos.h"
#include "GPIO.h"
#include "UART0.h"
#include "I2C.h"
#include "GRAL.h"

static const uint8_t MAX_ADDR         = 0x57;

#define MAX_ADDR          0x57
#define REG_INTR_STATUS_1 0x00
#define REG_INTR_ENABLE_1 0x02
#define REG_FIFO_WR_PTR   0x04
#define REG_OVF_COUNTER   0x05
#define REG_FIFO_RD_PTR   0x06
#define REG_FIFO_DATA     0x07
#define REG_FIFO_CONFIG   0x08
#define REG_MODE_CONFIG   0x09
#define REG_SPO2_CONFIG   0x0A
#define REG_LED1_RED      0x0C
#define REG_LED2_IR       0x0D
#define REG_LED3_IR       0x0D
#define REG_MULTI_LED_1   0x11   // SLOT1
#define REG_MULTI_LED_2   0x12   // SLOT2
#define REG_PART_ID       0xFF

class MAX {
public:
	MAX();

	bool initiated;

	bool probe();
	bool init(void);
	bool read(uint32_t* red, uint32_t* ir, uint8_t* avail_out);
	virtual ~MAX();
};

#endif /* DRIVERS_MAX_H_ */
