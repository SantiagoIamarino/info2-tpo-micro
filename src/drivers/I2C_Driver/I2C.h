#pragma once
#include "tipos.h"
#include "LPC845.h"
#include "Hardware.h"
#include "GPIO.h"


/**> Index for the IOCON Register Port0 */
const uint8_t IOCON_INDEX_PIO0[] = { 17,11,6,5,4,3,16,15,4,13,8,7,2,1,18,10,9,0,30,29,28,27,26,25,24,23,22,21,20,0,0,35};
/**> Index for the IOCON Register Port1 */
const uint8_t IOCON_INDEX_PIO1[] = { 36,37,3,41,42,43,46,49,31,32,55,54,33,34,39,40,44,45,47,48,52,53,0,0,0,0,0,0,0,50,51};

class I2C {
public:
    // pines del PORT0 (ej: SDA=10, SCL=11 → P0_10 / P0_11)
	I2C(uint8_t sda_pin0, uint8_t scl_pin0, uint32_t khz = 100);

    bool writeReg(uint8_t addr7, uint8_t reg, uint8_t val);      // write 1 byte
    bool readReg(uint8_t addr7, uint8_t reg, uint8_t &val);      // read 1 byte
    bool readBytes(uint8_t addr7, uint8_t reg, uint8_t *buf, uint32_t n);
    bool writeBytes(uint8_t addr7, const uint8_t *buf, uint32_t n);

    // utilidades
    bool scanOne(uint8_t addr7);
private:
    uint8_t SDA, SCL;
    uint32_t halfDelayCycles;

    // GPIO helpers (PORT0)
    inline void sda_hi() { GPIO->DIR[0] &= ~(1u<<SDA); }            // input → suelto (pull-ups lo llevan a 1)
    inline void sda_lo() { GPIO->DIR[0] |=  (1u<<SDA); GPIO->CLR[0] = (1u<<SDA); }
    inline void scl_hi() { GPIO->DIR[0] &= ~(1u<<SCL); }
    inline void scl_lo() { GPIO->DIR[0] |=  (1u<<SCL); GPIO->CLR[0] = (1u<<SCL); }

    inline uint32_t read_sda() { return (GPIO->PIN[0] >> SDA) & 1u; }
    inline uint32_t read_scl() { return (GPIO->PIN[0] >> SCL) & 1u; }

    void delayHalf();
    bool clockHighWait();                 // sube SCL y espera que realmente esté alto (por si hay stretch)
    void start();
    void stop();
    bool writeByte(uint8_t b);            // devuelve ACK (true si ack=0)
    uint8_t readByte(bool ack);           // ack=false → NACK
    void ioconDigital(uint8_t pin0);
};
