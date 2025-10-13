#include<LPC214X.h>
// Send Start Condition
void i2c_start(void) {
    I2C0CONSET = 0x20;             // Set STA bit
    while (!(I2C0CONSET & 0x08));  // Wait for SI to be set
    I2C0CONCLR = 0x28;             // Clear STA and SI bits
}

// Send Stop Condition
void i2c_stop(void) {
    I2C0CONSET = 0x10;   // Set STO
    I2C0CONCLR = 0x08;   // Clear SI
    while (I2C0CONSET & 0x10); // Wait for STOP to complete
}

// Write a byte to I2C bus
void i2c_write(unsigned char data) {
    I2C0DAT = data;
    I2C0CONCLR = 0x08;            // Clear SI
    while (!(I2C0CONSET & 0x08)); // Wait for SI
}

// Read a byte with ACK
unsigned char i2c_read_ack(void) {
    I2C0CONSET = 0x04;            // Set AA (assert ACK)
    I2C0CONCLR = 0x08;            // Clear SI
    while (!(I2C0CONSET & 0x08)); // Wait for SI
    return I2C0DAT;
}

// Read a byte with NACK
unsigned char i2c_read_nack(void) {
    I2C0CONCLR = 0x04;            // Clear AA (assert NACK)
    I2C0CONCLR = 0x08;            // Clear SI
    while (!(I2C0CONSET & 0x08)); // Wait for SI
    return I2C0DAT;
}

// Multi-byte Read
void i2c_multiread(char *arr, int bytes) {
	int i;
    for (i = 0; i < bytes - 1; i++) {
        I2C0CONSET = 0x04;            // Set AA (ACK)
        I2C0CONCLR = 0x08;            // Clear SI
        while (!(I2C0CONSET & 0x08)); // Wait for SI
        arr[i] = I2C0DAT;
    }

    I2C0CONCLR = 0x04;            // Clear AA (NACK)
    I2C0CONCLR = 0x08;            // Clear SI
    while (!(I2C0CONSET & 0x08)); // Wait for SI
    arr[bytes - 1] = I2C0DAT;
}
