/*
 * MPU.cpp
 *
 *  Created on: Oct 7, 2025
 *      Author: santiagoiamarino
 */

#include "MPU.h"

I2C I2C_ACC(13, 14, 100);
TIMER* tick_MPU;
MPU* MPU::s_self = nullptr;

#define FREC_TICK_READ 50

// cada cuando analizo muestras
static const uint16_t WIN_MS = 100;
static const uint16_t WIN_N = WIN_MS / FREC_TICK_READ;


static const uint32_t SCORE_QUIETO = 1000;
static const uint32_t SCORE_ACTIVO = 8000;
static const uint32_t SCORE_POSIBLE_CAIDA = 30000;
static const uint32_t MIN_POSIBLE_CAIDA_MUESTRAS = 3;

MPU::MPU() {
	this->initiated = false;
	this->ax_anterior = 0;
	this->ay_anterior = 0;
	this->az_anterior = 0;
}

bool MPU::init()
{

	log_debug((uint8_t*)"Initiating MPU\r\n", 0);

	for(int i = 0; i < 200; i++){
		// 1) Ver si responde en MPU_ADDR
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

		if (who == MPU_ADDR) {
			I2C_ACC.writeReg(MPU_ADDR, REG_PWR_MGMT_1, 0x00);
			log_debug((uint8_t*)"Init MPU OK\r\n", 0);
			this->initiated = true;
			tick_MPU = new TIMER(10, FREC_TICK_READ, MPU::tick_read);
			s_self = this;
			return true;
		}

		log_debug((uint8_t*)"Init MPU FAIL\r\n", 0);
	}

	log_debug((uint8_t*)"Init MPU FAIL\r\n", 0);
	return false;

}

void MPU::tick_read(void) {
	if (!s_self || s_self->paused) return;

	s_self->read();
}



void MPU::procesarMuestra(int16_t ax, int16_t ay, int16_t az) {
	//this->log_acc(ax, ay, az);

    int32_t ax_ac = (int32_t)ax - ax_anterior;
    int32_t ay_ac = (int32_t)ay - ay_anterior;
    int32_t az_ac = (int32_t)az - az_anterior;

	this->ax_anterior = ax;
	this->ay_anterior = ay;
	this->az_anterior = az;

    // valor absoluto de las picos y sumo todos los ejes
    uint32_t sabs = (uint32_t)((ax_ac < 0 ? -ax_ac : ax_ac)
                             + (ay_ac < 0 ? -ay_ac : ay_ac)
                             + (az_ac < 0 ? -az_ac : az_ac));

    sum_abs += sabs;

    if (++n >= WIN_N) {
        // promedio por muestra en la ventana
        valor_acc_actual = sum_abs / n;

        switch (estado_movimiento) {
            case ACC_ESTADO_QUIETO:
                if (valor_acc_actual > SCORE_ACTIVO) {
                    estuvo_activo_ms += 1000;
                    if (estuvo_activo_ms >= 1000) { estado_movimiento = 2; estuvo_activo_ms = 0; } // exigir ≥1 s
                } else if (valor_acc_actual > SCORE_QUIETO) {
                    estado_movimiento = ACC_ESTADO_MICRO;
                    estuvo_activo_ms = 0;
                } else {
                    estuvo_activo_ms = 0;
                }
                break;
            case ACC_ESTADO_MICRO:
                if (valor_acc_actual > SCORE_ACTIVO) {
                    estado_movimiento = ACC_ESTADO_MOVIMIENTO;
                } else if (valor_acc_actual <= SCORE_QUIETO) {
                    // volver a QUIETO si sostenido SCORE_QUIETO
                    estuvo_quieto_ms += 1000;
                    if (estuvo_quieto_ms >= 2000) { estado_movimiento = 0; estuvo_quieto_ms = 0; }
                } else {
                    estuvo_quieto_ms = 0;
                }
                break;
            case ACC_ESTADO_MOVIMIENTO:
                if (valor_acc_actual <= SCORE_QUIETO) {
                    estuvo_quieto_ms += 1000;
                    if (estuvo_quieto_ms >= 2000) { estado_movimiento = 1; estuvo_quieto_ms = 0; }
                } else {
                    estuvo_quieto_ms = 0;
                }
                break;
        }

        // detecto posible caida
        if(this->valor_acc_actual >= SCORE_POSIBLE_CAIDA) {
        	if(this->posible_caida_counter++ == MIN_POSIBLE_CAIDA_MUESTRAS) {
        		this->caida_detectada = true;
        	}
        } else {
        	this->posible_caida_counter = 0;
        }

        // reset
        sum_abs = 0;
        n = 0;
    }
}


void MPU::read(){
	uint8_t buf[6];
	if (I2C_ACC.readBytes(MPU_ADDR, REG_ACCEL_XOUT_H, buf, 6)) {
		int16_t ax = (int16_t)((buf[0] << 8) | buf[1]);
		int16_t ay = (int16_t)((buf[2] << 8) | buf[3]);
		int16_t az = (int16_t)((buf[4] << 8) | buf[5]);

		this->procesarMuestra(ax, ay, az);
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

