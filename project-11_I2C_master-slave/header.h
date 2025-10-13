// header.h
#ifndef HEADER_H
#define HEADER_H

void uart_init(unsigned int baudrate);
void uart_tx(unsigned char data);
void uart_tx_string(char *str);

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(unsigned char data);
unsigned char i2c_read(unsigned char ack);

void delay_ms(unsigned int ms);

#endif
