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
#include "CALLBACK.h"
#include "TIMER.h"

#define MPU_ADDR 			0x68
#define REG_PWR_MGMT_1 		0x6B
#define REG_WHO_AM_I 		0x75
#define REG_ACCEL_XOUT_H 	0x3B

#define ACC_ESTADO_QUIETO 0
#define ACC_ESTADO_MICRO 1
#define ACC_ESTADO_MOVIMIENTO 2

class MPU {
public:
	MPU();

	bool init();
	void read();
	static void tick_read();
	void pause()  { paused = true; }
	void resume() { paused = false; }
	void procesarMuestra(int16_t ax, int16_t ay, int16_t az);
	char* i16toa(int16_t v, char* p);
	void log_acc(int16_t ax, int16_t ay, int16_t az);
	uint8_t Get_Estado_Movimiento(void){ return estado_movimiento; };

	uint8_t posible_caida_counter = 0;
	bool caida_detectada = false;

	bool initiated;
	bool paused = false;
	static MPU* s_self;
	virtual ~MPU();

private:
	int32_t ax_anterior = 0, ay_anterior = 0, az_anterior = 0;

	uint32_t sum_abs = 0;   // Σ |ax_ac|+|ay_ac|+|az_ac|
	uint16_t n = 0;

	// Score expuesto (promedio por muestra en la ultima ventana)
	uint32_t valor_acc_actual = 0;

	uint8_t  estado_movimiento = 0;
	uint16_t estuvo_quieto_ms = 0;
	uint16_t estuvo_activo_ms = 0;

};

#endif /* DRIVERS_MPU_H_ */
