/*
 * MAX.cpp
 *
 *  Created on: Oct 7, 2025
 *      Author: santiagoiamarino
 */

#include "MAX.h"

I2C I2C_MAX(10, 11, 100);

MAX::MAX() {
	initiated = false;
	this->init();
}

bool MAX::probe()
{
    // ¿está en 0x57?
    if (!I2C_MAX.scanOne(MAX_ADDR)) {
        log_debug((uint8_t*)"MAX not found at 0x57\r\n",0);
        return false;
    }

    // Leer PART ID (0xFF) -> 0x15
    uint8_t partid = 0;
    if (!I2C_MAX.readReg(MAX_ADDR, REG_PART_ID, partid)) {
        log_debug((uint8_t*)"MAX read PARTID fail\r\n",0);
        return false;
    }
    char msg[32];
    int n = sprintf(msg, "MAX PARTID=0x%02X\r\n", partid);
    log_debug((uint8_t*)msg, n);
    return (partid == 0x15);
}

bool MAX::init(void) {

	bool max_probed = false;

	for (volatile int i=0; i<100; ++i) {

		if (this->probe()) {
			log_debug((uint8_t*)"maxProbe WORKED...\r\n", 0);
			max_probed = true;
			break;
		}
		log_debug((uint8_t*)"maxProbe failed, trying again...\r\n", 0);
		delay();
	}

	if(!max_probed) {
		 log_debug((uint8_t*)"maxProbe failed, MAX ATTEMPS REACHED...\r\n", 0);
		 return false;
	}

    uint8_t v;

    // 1) Reset
    I2C_MAX.writeReg(0x57, 0x09, 0x40);           // MODE_CONFIG: RESET
    for (int i=0;i<10000;i++) __NOP();
    // esperar a que bit de reset se limpie…
    for (int k=0;k<50;k++) {
        if (I2C_MAX.readReg(0x57, 0x09, v) && ((v & 0x40) == 0)) break;
        for (volatile int i=0;i<3000;i++) __NOP();
    }

    // 2) FIFO: sample avg=4 (2<<5), rollover=1 (1<<4), almost_full=0x0F
    I2C_MAX.writeReg(0x57, 0x08, (2<<5) | (1<<4) | 0x0F);

    // 3) SPO2: range=0 (2048nA), sample rate=100Hz (3<<2), pulse width=411us/18-bit (3)
    I2C_MAX.writeReg(0x57, 0x0A, (1<<5) | (3<<2) | 3);

    // 4) Corriente LEDs
    //    LED1=RED, LED2=IR en MAX30102
    I2C_MAX.writeReg(0x57, 0x0C, 0x20);           // LED1_PA (RED) ~6mA
    I2C_MAX.writeReg(0x57, 0x0D, 0x20);           // LED2_PA (IR)  ~6mA

    // 5) Limpiar punteros FIFO
    I2C_MAX.writeReg(0x57, 0x04, 0x00);           // FIFO_WR_PTR
    I2C_MAX.writeReg(0x57, 0x05, 0x00);           // OVF_COUNTER
    I2C_MAX.writeReg(0x57, 0x06, 0x00);           // FIFO_RD_PTR

    // 6) SPO2 mode (RED+IR)
    I2C_MAX.writeReg(0x57, 0x09, 0x03);           // MODE_CONFIG: SpO2


    initiated = true;

    return true;
}


bool MAX::read(uint32_t* red, uint32_t* ir, uint8_t* avail_out) {
    uint8_t wr=0, rd=0;
    if (!I2C_MAX.readReg(0x57, 0x04, wr)) return false;  // FIFO_WR_PTR
    if (!I2C_MAX.readReg(0x57, 0x06, rd)) return false;  // FIFO_RD_PTR
    uint8_t avail = (wr - rd) & 0x1F;                    // profundidad FIFO=32
    if (avail_out) *avail_out = avail;
    if (avail == 0) return false;

    uint8_t buf[6];
    if (!I2C_MAX.readBytes(0x57, 0x07, buf, 6)) return false; // FIFO_DATA

    // 18 bits por canal, mask 0x3FFFF. Orden: RED luego IR (SpO2)
    uint32_t r = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    uint32_t i = ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 8) | buf[5];
    r &= 0x3FFFF; i &= 0x3FFFF;
    *red = r; *ir = i;
    return true;
}

MAX::~MAX() {
	// TODO Auto-generated destructor stub
}

