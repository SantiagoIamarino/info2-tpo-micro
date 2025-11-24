#include "I2C.h"
#include "Defines.h"

#ifndef SystemCoreClock
#define SystemCoreClock 12000000u
#endif

I2C::I2C(uint8_t sda_pin0, uint8_t scl_pin0, uint32_t khz)
: SDA(sda_pin0), SCL(scl_pin0)
{
    // 1) Habilitar clock de IOCON y GPIO si hiciera falta
    SYSCON->SYSAHBCLKCTRL0 |= (1u<<6)  /* GPIO */
                            | (1u<<18) /* IOCON */;
    // 2) Poner pines en modo digital (DIGIMODE=1). En LPC845 suele existir IOCON->PIO[port][pin].
    //    Si tu header no tiene esta forma, decime y te paso la variante.
    IOCON->PIO[IOCON_INDEX_PIO0[SDA]] = (1u<<7);	// DIGIMODE=1, resto en 0 (inactive)
	IOCON->PIO[IOCON_INDEX_PIO0[SCL]] = (1u<<7);

    // 3) Estado inicial “sueltos” (open-drain por soft). Necesitás pull-ups externos a 3V3.
    sda_hi();
    scl_hi();

    // Usamos un loop vacío proporcional al clock
    // Ajuste empírico: 1 iter ≈ 2-3 ciclos; dejamos un factor conservador
    halfDelayCycles = (SystemCoreClock / (khz * 8u));
    if (halfDelayCycles < 10u) halfDelayCycles = 10u;
}

void I2C::ioconDigital(uint8_t pin0) {
    (void)pin0; /* por si compilador se queja en otras variantes */
}


bool I2C::clockHighWait() {
    scl_hi();
    // Esperar a que SCL realmente suba (si algún slave hace clock stretching)
    // Máximo unas cuantas iteraciones para no colgarnos.
    for (int i=0; i<1000; ++i) {
        if (read_scl()) return true;
        __NOP();
    }
    return false;
}

void I2C::start() {
    sda_hi(); scl_hi();
    sda_lo();
    scl_lo();
}

void I2C::stop() {
    sda_lo();
    if (!clockHighWait()) return;
    sda_hi();
}

bool I2C::writeByte(uint8_t b) {
    for (int i=7; i>=0; --i) {
        (b & (1u<<i)) ? sda_hi() : sda_lo();
        if (!clockHighWait()) return false;
        scl_lo();
    }
    // ACK bit
    sda_hi();               // liberar SDA para que el slave responda
    if (!clockHighWait()) return false;
    bool ack = (read_sda() == 0u);
    scl_lo();
    return ack;
}

uint8_t I2C::readByte(bool ack) {
    uint8_t v = 0;
    sda_hi(); // liberar SDA (entrada)
    for (int i=7; i>=0; --i) {
        if (!clockHighWait()) break;
        v |= (read_sda() ? 1u : 0u) << i;
        scl_lo();
    }
    // enviar ACK/NACK
    if (ack) sda_lo(); else sda_hi();
    if (!clockHighWait()) {}
    scl_lo();
    sda_hi();
    return v;
}

bool I2C::writeBytes(uint8_t addr7, const uint8_t *buf, uint32_t n) {
    start();
    if (!writeByte((addr7<<1) | 0u)) { stop(); return false; } // W
    for (uint32_t i=0;i<n;i++) {
        if (!writeByte(buf[i])) { stop(); return false; }
    }
    stop();
    return true;
}

bool I2C::writeReg(uint8_t addr7, uint8_t reg, uint8_t val) {
    uint8_t b[2] = {reg, val};
    return writeBytes(addr7, b, 2);
}

bool I2C::readReg(uint8_t addr7, uint8_t reg, uint8_t &val) {
    // write reg, repeated start, read 1
    start();
    if (!writeByte((addr7<<1) | 0u)) { stop(); return false; } // W
    if (!writeByte(reg))               { stop(); return false; }
    start();
    if (!writeByte((addr7<<1) | 1u)) { stop(); return false; } // R
    val = readByte(false); // NACK al último byte
    stop();
    return true;
}

bool I2C::readBytes(uint8_t addr7, uint8_t reg, uint8_t *buf, uint32_t n) {
    start();
    if (!writeByte((addr7<<1) | 0u)) { stop(); return false; } // W
    if (!writeByte(reg))               { stop(); return false; }
    start();
    if (!writeByte((addr7<<1) | 1u)) { stop(); return false; } // R
    for (uint32_t i=0;i<n;i++) {
        buf[i] = readByte(i < (n-1)); // ACK todos salvo el último
    }
    stop();
    return true;
}

bool I2C::scanOne(uint8_t addr7) {
    start();
    bool ok = writeByte((addr7<<1) | 0u);
    stop();
    return ok;
}
