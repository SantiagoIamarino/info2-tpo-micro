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
#include "CALLBACK.h"
#include "TIMER.h"

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
#define REG_MULTI_LED_1   0x11   // SLOT1
#define REG_MULTI_LED_2   0x12   // SLOT2
#define REG_PART_ID       0xFF

class MAX {
public:
	MAX();
	bool init(void);
	bool read(uint32_t* red, uint32_t* ir);
	static void tick_read();
	virtual ~MAX();
private:
	bool initiated;
	bool filtro_initiated;
	int32_t filtro_dc;
	int32_t filtro_valor;
	uint8_t pulso_state;
	uint16_t pulso_ppm_actual;
	uint32_t pulso_state_reset_counter;
	uint32_t pulso_state_switch_counter;
	uint32_t pulso_state_switch_back_counter;
	bool buscando_picos;
	uint32_t pulso_t_desde_ult_pico;
	uint16_t pulso_valor_previo;
	uint16_t pulso_sube_cnt;
	uint16_t pulso_baja_cnt;
	uint16_t pulso_valle;
	bool pulso_subio;

	void reset_vars(void);
	bool probe(void);
	int32_t pulso_filtro(int32_t ir_val);
	void pulso_maq_estados(uint32_t* ir);
	void buscar_picos(uint32_t* ir);
	static MAX* s_self;
};

#endif /* DRIVERS_MAX_H_ */
