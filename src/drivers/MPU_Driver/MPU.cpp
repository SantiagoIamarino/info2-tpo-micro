/*
 * MPU.cpp
 *
 *  Created on: Oct 7, 2025
 *      Author: santiagoiamarino
 */

#include "MPU.h"

I2C I2C_ACC(13, 14, 100);

MPU::MPU() {
	this->initiated = false;
}

bool MPU::init()
{

	log_debug((uint8_t*)"Initiating MPU\r\n", 0);

	for(int i = 0; i < 200; i++){
		// 1) Ver si responde en 0x68
		if (!I2C_ACC.scanOne(MPU_ADDR)) {
			log_debug((uint8_t*)"MPU no responde en 0x68\r\n", 0);
			delay();continue;
		}

		// 2) Salir de sleep: escribir 0x00 en PWR_MGMT_1
		if (!I2C_ACC.writeReg(MPU_ADDR, REG_PWR_MGMT_1, 0x00)) {
			log_debug((uint8_t*)"Error PWR_MGMT_1\r\n", 0);
			delay();continue;
		}

		// Pequeño delay
		delay();

		// 3) WHO_AM_I
		uint8_t who=0;
		if (!I2C_ACC.readReg(MPU_ADDR, REG_WHO_AM_I, who)) {
			log_debug((uint8_t*)"Error WHO_AM_I\r\n", 0);
			continue;
		}

		if (who == 0x68) {
			I2C_ACC.writeReg(MPU_ADDR, REG_PWR_MGMT_1, 0x00);
			log_debug((uint8_t*)"Init MPU OK\r\n", 0);
			this->initiated = true;
			return true;
		}

		log_debug((uint8_t*)"Init MPU FAIL\r\n", 0);
	}

	log_debug((uint8_t*)"Init MPU FAIL\r\n", 0);
	return false;

}

void MPU::read(){
	uint8_t buf[6];
	if (I2C_ACC.readBytes(MPU_ADDR, REG_ACCEL_XOUT_H, buf, 6)) {
		int16_t ax = (int16_t)((buf[0] << 8) | buf[1]);
		int16_t ay = (int16_t)((buf[2] << 8) | buf[3]);
		int16_t az = (int16_t)((buf[4] << 8) | buf[5]);

		this->log_acc(ax, ay, az);
	} else {
		log_debug((uint8_t*)"ACC read error\r\n", 0);
	}
}

char* MPU::i16toa(int16_t v, char* p) {
    char tmp[6]; int i = 0; bool neg = (v < 0);
    if (neg) v = -v;
    do { tmp[i++] = '0' + (v % 10); v /= 10; } while (v);
    if (neg) *p++ = '-';
    while (i--) *p++ = tmp[i];
    *p = 0;
    return p;
}

void MPU::log_acc(int16_t ax, int16_t ay, int16_t az) {
    char buf[40], *p = buf;
    *p++='A'; *p++='C'; *p++='C'; *p++=' '; *p++='X'; *p++='=';
    p = i16toa(ax, p);
    *p++=' '; *p++='Y'; *p++='='; p = i16toa(ay, p);
    *p++=' '; *p++='Z'; *p++='='; p = i16toa(az, p);
    *p++='\r'; *p++='\n';
    log_debug((uint8_t*)buf, (uint32_t)(p - buf));
}

MPU::~MPU() {
	// TODO Auto-generated destructor stub
}

