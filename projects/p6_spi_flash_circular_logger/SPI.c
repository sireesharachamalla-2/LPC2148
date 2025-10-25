#include <LPC214x.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SSEL_PIN (1<<6)    // P0.6 for CS
#define FLASH_SIZE  2048   // 2KB EEPROM in Proteus (e.g. 25LC020)
#define RECORD_SIZE 16
#define MAX_RECORDS (FLASH_SIZE / RECORD_SIZE)

// SPI Commands (25LCxxx EEPROM)
#define CMD_WREN  0x06
#define CMD_WRITE 0x02
#define CMD_READ  0x03
#define CMD_RDSR  0x05
#define STATUS_WIP 0x01

unsigned int write_index = 0;

//  Delay
void delay_ms(unsigned int ms) {
    unsigned int i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<6000;j++);
}

// UART 
void uart_init(void) {
    PINSEL0 |= 0x00000005; // P0.0 TXD0, P0.1 RXD0
    U0LCR = 0x83;
    U0DLL = 97; // 9600 baud ,15MHz PCLK
    U0DLM = 0;
    U0LCR = 0x03;
}

void uart_tx(char c) {
    while(!(U0LSR & 0x20));
    U0THR = c;
}

void uart_str(const char *s) {
    while(*s) 
			uart_tx(*s++);
}

char uart_rx(void) {
    while(!(U0LSR & 0x01));
    return U0RBR;
}


void spi_init(void) {
    PINSEL0 |= (1<<14)|(1<<18);  // P0.7=SCK0, P0.9=MOSI0
    IODIR0 |= SSEL_PIN;
    IOSET0 = SSEL_PIN;           // CS high
    S0SPCR = (1<<5);             // Master mode
    S0SPCCR = 8;                 // Clock divider
}

unsigned char spi_tx(unsigned char d) {
    S0SPDR = d;
    while(!(S0SPSR & (1<<7)));
    return S0SPDR;
}

void spi_select(void) 
{
IOCLR0 = SSEL_PIN;
}

void spi_deselect(void) 
{
IOSET0 = SSEL_PIN;
}


void flash_write_enable(void) {
    spi_select();
    spi_tx(CMD_WREN);
    spi_deselect();
}

unsigned char flash_read_status(void) {
    unsigned char st;
    spi_select();
    spi_tx(CMD_RDSR);
    st = spi_tx(0xFF);
    spi_deselect();
    return st;
}

void flash_wait_done(void) {
    while(flash_read_status() & STATUS_WIP);
}

void flash_write(unsigned int addr, unsigned char *data, unsigned int len) {
    unsigned int i;
    flash_write_enable();
    spi_select();
    spi_tx(CMD_WRITE);
    spi_tx((addr >> 8) & 0xFF);
    spi_tx(addr & 0xFF);
    for(i=0;i<len;i++) 
			spi_tx(data[i]);
	
    spi_deselect();
    flash_wait_done();
}

void flash_read(unsigned int addr, unsigned char *data, unsigned int len) {
    unsigned int i;
    spi_select();
    spi_tx(CMD_READ);
    spi_tx((addr >> 8) & 0xFF);
    spi_tx(addr & 0xFF);
   
	for(i=0;i<len;i++) 
			data[i] = spi_tx(0xFF);
    spi_deselect();
}


unsigned int get_record_addr(unsigned int idx) {
    return (idx % MAX_RECORDS) * RECORD_SIZE;
}

void write_record(unsigned int value) {
    unsigned char rec[RECORD_SIZE];
    int i;
    for(i=0;i<RECORD_SIZE;i++) 
			rec[i] = 0;
    rec[0] = (value >> 8) & 0xFF;
    rec[1] = (value & 0xFF);
    flash_write(get_record_addr(write_index), rec, RECORD_SIZE);
    write_index = (write_index + 1) % MAX_RECORDS;
    uart_str("Record Logged\r\n");
}

void read_last_n(unsigned int n) {
    unsigned int i, idx;
    unsigned char rec[RECORD_SIZE];
    char msg[32];
    if(n > MAX_RECORDS) 
			n = MAX_RECORDS;
    uart_str("\r\nLast Records:\r\n");
    for(i=0;i<n;i++) {
        if(write_index == 0) 
					idx = MAX_RECORDS - n + i;
        else 
					idx = (write_index + MAX_RECORDS - n + i) % MAX_RECORDS;
        flash_read(get_record_addr(idx), rec, RECORD_SIZE);
        sprintf(msg, "R%03u: %u\r\n", idx, (rec[0]<<8)|rec[1]);
        uart_str(msg);
    }
}

void clear_logs(void) {
    unsigned char blank[RECORD_SIZE];
    unsigned int i;
    for(i=0;i<RECORD_SIZE;i++) blank[i]=0;
    for(i=0;i<MAX_RECORDS;i++)
        flash_write(get_record_addr(i), blank, RECORD_SIZE);
    write_index = 0;
    uart_str("Logs Cleared\r\n");
}

void handle_uart_command(void) {
    char buf[32];
    char c;
    int i=0;
    uart_str("\r\n> ");
    while(1) {
        c = uart_rx();
        if(c=='\r' || c=='\n')
					break;
        if(i<31) 
					buf[i++]=c;
        uart_tx(c);
    }
    buf[i]='\0';
    for(i=0;i<strlen(buf);i++)
        if(buf[i]>='a' && buf[i]<='z') 
					buf[i]-=32;

    if(strcmp(buf,"LOG")==0) 
		{
        static unsigned int val=1;
        write_record(val++);
    } 
    else if(strncmp(buf,"READN ",6)==0) 
			{
        unsigned int n = atoi(&buf[6]);
        read_last_n(n);
    } 
    else if(strcmp(buf,"CLEAR")==0) 
			{
        clear_logs();
    } 
    else {
        uart_str("Unknown cmd\r\n");
    }
}


int main(void) {
    spi_init();
    uart_init();
    uart_str("SPI EEPROM Circular Logger\r\n");
    uart_str("Commands: LOG, READN n, CLEAR\r\n");

    while(1) {
        handle_uart_command();
    }
}
