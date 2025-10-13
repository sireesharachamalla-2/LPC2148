#include <LPC214x.h>
#include "header.h"

void i2c_init(void) {
    PINSEL0 |= 0x50;       // P0.2 = SCL0, P0.3 = SDA0
    I2C0CONCLR = 0x6C;     // Clear AA, SI, STO, STA, I2EN
    I2C0CONSET = 0x40;     // Enable I2C
    I2C0SCLH = 75;         // For 100kHz (with PCLK = 15MHz)
    I2C0SCLL = 75;
}

void i2c_start(void) {
    I2C0CONSET = 0x20;      // Set STA
    while (!(I2C0CONSET & 0x08));  // Wait for SI (start transmitted)
    I2C0CONCLR = 0x28;      // Clear STA & SI
}

void i2c_stop(void) {
    I2C0CONSET = 0x10;      // Set STO
    I2C0CONCLR = 0x08;      // Clear SI
}

void i2c_write(unsigned char data) {
    I2C0DAT = data;
    I2C0CONCLR = 0x08;      // Clear SI to continue
    while (!(I2C0CONSET & 0x08));  // Wait for SI set (data sent)
}
