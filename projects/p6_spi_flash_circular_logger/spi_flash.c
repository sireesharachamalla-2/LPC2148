/* spi_circular_logger.c
   LPC2148 + SPI EEPROM/Flash circular logger
   Keil µVision (C89)
*/

#include <LPC214x.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ----------------- CONFIG: adapt to your device ----------------- */
/* Make sure these match the SPI device used in Proteus */
#define FLASH_TOTAL_SIZE   65536U    /* total bytes in device (e.g. 64KB) */
#define FLASH_PAGE_SIZE    64U       /* page size for page program */
#define FLASH_BASE_ADDR    0x000000  /* starting offset used for logs */

/* Header layout at FLASH_BASE_ADDR:
   [0..3]  magic (0xDEADBEEF)
   [4..7]  write_index (uint32) -> number of records written so far
   Header size = 8 bytes
*/
#define HEADER_MAGIC       0xDEADBEEFUL
#define HEADER_ADDR        (FLASH_BASE_ADDR)
#define HEADER_SIZE        8U

/* Choose record size (must be <= FLASH_PAGE_SIZE but we handle multi-page writes) */
#define RECORD_SIZE        16U

/* Derived */
#define MAX_RECORDS  ((FLASH_TOTAL_SIZE - HEADER_SIZE) / RECORD_SIZE)

/* SPI Flash commands (common) */
#define CMD_WREN  0x06  /* Write Enable */
#define CMD_READ  0x03  /* Read Data */
#define CMD_WRITE 0x02  /* Page Program / Byte Program */
#define CMD_RDSR  0x05  /* Read Status Register */

#define STATUS_WIP_BIT 0x01

/* MCU pin mapping for CS */
#define SPI_CS_PIN  (1<<7)   /* P0.7 as CS */
#define SPI_CS_LOW()  IOCLR0 = SPI_CS_PIN
#define SPI_CS_HIGH() IOSET0 = SPI_CS_PIN

/* Prototypes */
void delay_ms(unsigned int ms);
void SPI_Init(void);
unsigned char SPI_xfer(unsigned char data);
void flash_write_enable(void);
unsigned char flash_read_status(void);
void flash_wait_write_complete(void);
void flash_read_bytes(unsigned int addr, unsigned char *buf, unsigned int len);
void flash_write_bytes(unsigned int addr, unsigned char *buf, unsigned int len);
int flash_init_header(void);
unsigned int flash_read_write_index(void);
void flash_write_write_index(unsigned int idx);

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
char uart_getc_block(void);

void handle_uart_command(void);
void log_sample(void);
void read_last_n(unsigned int n);
void clear_logs(void);

/* Globals for simple demo */
char uart_buf[64];

/* ---------------- Utility delays ---------------- */
void delay_ms(unsigned int ms)
{
    unsigned int i,j;
    for (i=0;i<ms;i++)
        for (j=0;j<6000;j++);
}

/* ---------------- SPI functions ---------------- */
void SPI_Init(void)
{
    /* P0.4 = SCK0, P0.5 = MISO0, P0.6 = MOSI0 */
    PINSEL0 &= ~((3<<8)|(3<<10)|(3<<12));
    PINSEL0 |=  ((1<<8)|(1<<10)|(1<<12));

    IODIR0 |= SPI_CS_PIN;     /* CS output */
    SPI_CS_HIGH();

    /* S0SPCR: MSTR = 1 (bit 5), CPHA/CPOL default 0 (mode0),
       bit6 CPOL in some docs; here set MSTR only. */
    S0SPCR = (1<<5);          /* Master */
    S0SPCCR = 8;              /* Divider -> PCLK/8 */
}

/* Simple blocking SPI transfer (8-bit) */
unsigned char SPI_xfer(unsigned char data)
{
    S0SPDR = data;
    while (!(S0SPSR & (1<<7))); /* SPIF */
    return (unsigned char)S0SPDR;
}

/* ---------------- Flash primitives ---------------- */
void flash_write_enable(void)
{
    SPI_CS_LOW();
    SPI_xfer(CMD_WREN);
    SPI_CS_HIGH();
}

unsigned char flash_read_status(void)
{
    unsigned char s;
    SPI_CS_LOW();
    SPI_xfer(CMD_RDSR);
    s = SPI_xfer(0xFF);
    SPI_CS_HIGH();
    return s;
}

void flash_wait_write_complete(void)
{
    unsigned char s;
    do {
        s = flash_read_status();
    } while (s & STATUS_WIP_BIT);
}

/* Read sequence (24-bit address used; adjust if device uses 16-bit) */
void flash_read_bytes(unsigned int addr, unsigned char *buf, unsigned int len)
{
    unsigned int i;
    SPI_CS_LOW();
    SPI_xfer(CMD_READ);
    /* send 24-bit address */
    SPI_xfer((addr >> 16) & 0xFF);
    SPI_xfer((addr >> 8) & 0xFF);
    SPI_xfer((addr) & 0xFF);
    for (i=0;i<len;i++) buf[i] = SPI_xfer(0xFF);
    SPI_CS_HIGH();
}

/* Write bytes using page program across page boundaries.
   This implementation assumes the device supports CMD_WRITE with 24-bit address */
void flash_write_bytes(unsigned int addr, unsigned char *buf, unsigned int len)
{
    unsigned int offset = 0;
    while (offset < len) {
        unsigned int page_off = addr % FLASH_PAGE_SIZE;
        unsigned int page_rem = FLASH_PAGE_SIZE - page_off;
        unsigned int write_len = (len - offset) < page_rem ? (len - offset) : page_rem;

        flash_write_enable();
        SPI_CS_LOW();
        SPI_xfer(CMD_WRITE);
        /* 24-bit address */
        SPI_xfer((addr >> 16) & 0xFF);
        SPI_xfer((addr >> 8) & 0xFF);
        SPI_xfer((addr) & 0xFF);
        /* write write_len bytes */
        {
            unsigned int k;
            for (k=0;k<write_len;k++) {
                SPI_xfer(buf[offset + k]);
            }
        }
        SPI_CS_HIGH();
        flash_wait_write_complete();

        addr += write_len;
        offset += write_len;
    }
}

/* ---------------- Flash header helpers ---------------- */
int flash_init_header(void)
{
    unsigned char hbuf[8];

    unsigned int magic = ((unsigned int)hbuf[0] << 24) | ((unsigned int)hbuf[1] << 16)
                       | ((unsigned int)hbuf[2] << 8) | ((unsigned int)hbuf[3]);
	 flash_read_bytes(HEADER_ADDR, hbuf, HEADER_SIZE);
    if (magic != HEADER_MAGIC) {
        /* write header with magic and index 0 */
        unsigned char out[HEADER_SIZE];
        out[0] = (unsigned char)((HEADER_MAGIC >> 24) & 0xFF);
        out[1] = (unsigned char)((HEADER_MAGIC >> 16) & 0xFF);
        out[2] = (unsigned char)((HEADER_MAGIC >> 8) & 0xFF);
        out[3] = (unsigned char)(HEADER_MAGIC & 0xFF);
        out[4] = out[5] = out[6] = out[7] = 0;
        flash_write_bytes(HEADER_ADDR, out, HEADER_SIZE);
        return 0;
    }
    return 1;
}

unsigned int flash_read_write_index(void)
{
    unsigned char b[4];
    flash_read_bytes(HEADER_ADDR + 4, b, 4);
    return ((unsigned int)b[0] << 24) | ((unsigned int)b[1] << 16) | ((unsigned int)b[2] << 8) | ((unsigned int)b[3]);
}

void flash_write_write_index(unsigned int idx)
{
    unsigned char b[4];
    b[0] = (unsigned char)((idx >> 24) & 0xFF);
    b[1] = (unsigned char)((idx >> 16) & 0xFF);
    b[2] = (unsigned char)((idx >> 8) & 0xFF);
    b[3] = (unsigned char)(idx & 0xFF);
    /* write directly (will obey page boundaries) */
    flash_write_bytes(HEADER_ADDR + 4, b, 4);
}

/* ---------------- UART simple I/O ---------------- */
void uart_init(void)
{
    PINSEL0 |= 0x00000005;  /* P0.0=TXD0, P0.1=RXD0 */
    U0LCR = 0x83;
    U0DLL = 97;              /* 9600 @ PCLK 15MHz (adjust if different) */
    U0LCR = 0x03;
}

void uart_putc(char c)
{
    while (!(U0LSR & 0x20));
    U0THR = c;
}

void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

char uart_getc_block(void)
{
    while (!(U0LSR & 0x01));
    return (char)U0RBR;
}

/* ---------------- High-level app logic ---------------- */

/* returns pointer flash address for record index i (0-based) */
unsigned int record_addr(unsigned int idx)
{
    unsigned int start = HEADER_ADDR + HEADER_SIZE;
    return start + (idx % MAX_RECORDS) * RECORD_SIZE;
}

/* Write record (RECORD_SIZE bytes) at next slot, update write_index atomically */
void write_record(unsigned char *rec)
{
    unsigned int idx = flash_read_write_index();
    unsigned int addr = record_addr(idx);
    /* write record */
    flash_write_bytes(addr, rec, RECORD_SIZE);
    /* increment index and write header index */
    idx++;
    flash_write_write_index(idx);
}

/* For demo, produce a sample record bytes from counter */
void make_sample_record(unsigned int sample_value, unsigned char *rec_out)
{
    unsigned int t = sample_value;
    /* simple record format: [4 bytes seq][4 bytes sample][8 bytes reserved] = 16 bytes */
    rec_out[0] = (unsigned char)((t >> 24) & 0xFF);
    rec_out[1] = (unsigned char)((t >> 16) & 0xFF);
    rec_out[2] = (unsigned char)((t >> 8) & 0xFF);
    rec_out[3] = (unsigned char)(t & 0xFF);

    rec_out[4] = rec_out[5] = rec_out[6] = rec_out[7] = 0;
    rec_out[8] = rec_out[9] = rec_out[10] = rec_out[11] = 0;
    rec_out[12] = rec_out[13] = rec_out[14] = rec_out[15] = 0;
}

/* Read last n records (n <= MAX_RECORDS), print via UART; newest first */
void read_last_n(unsigned int n)
{
	 unsigned int first_idx;
    unsigned int idx = flash_read_write_index();
    if (n > MAX_RECORDS) n = MAX_RECORDS;
    if (n == 0) return;

   
    if (idx == 0)
		{
        uart_puts("No records stored\r\n");
        return;
    }

    /* Compute starting index for last n records */
    if (idx >= n) first_idx = idx - n;
    else first_idx = (idx + MAX_RECORDS) - n; /* wrap */
    {
        unsigned int i;
				 /* parse record fields for printing */
            unsigned int seq = ((unsigned int)rec[0] << 24) | ((unsigned int)rec[1] << 16)
                             | ((unsigned int)rec[2] << 8) | ((unsigned int)rec[3]);
        for (i = 0; i < n; i++) {
            unsigned int rec_idx = (first_idx + i) % MAX_RECORDS;
            unsigned char rec[RECORD_SIZE];
            unsigned int addr = record_addr(rec_idx);
            flash_read_bytes(addr, rec, RECORD_SIZE);

           
            sprintf(uart_buf, "Rec idx %u: seq=%u\r\n", rec_idx, seq);
            uart_puts(uart_buf);
        }
    }
}

/* Clear logs: re-write header magic & index to 0 (effectively clears) */
void clear_logs(void)
{
    unsigned char out[HEADER_SIZE];
    out[0] = (unsigned char)((HEADER_MAGIC >> 24) & 0xFF);
    out[1] = (unsigned char)((HEADER_MAGIC >> 16) & 0xFF);
    out[2] = (unsigned char)((HEADER_MAGIC >> 8) & 0xFF);
    out[3] = (unsigned char)(HEADER_MAGIC & 0xFF);
    out[4] = out[5] = out[6] = out[7] = 0;
    flash_write_bytes(HEADER_ADDR, out, HEADER_SIZE);
    uart_puts("Logs cleared\r\n");
}

/* Handle received commands from UART.
   Supported:
     LOG        -> write a sample record (auto increment sample counter)
     READN n    -> read last n records (example: READN 5)
     CLEAR      -> clear logs
*/
void handle_uart_command(void)
{
    char cmd[16];
    unsigned int i = 0;
    char ch;

    /* read until CR */
    while (1) {
        ch = uart_getc_block();
        if (ch == '\r' || ch == '\n') break;
        if (i < sizeof(cmd)-1) cmd[i++] = ch;
        uart_putc(ch); /* echo back */
    }
    cmd[i] = '\0';
    /* uppercase simple */
    for (i=0; i<strlen(cmd); i++) if (cmd[i]>='a' && cmd[i]<='z') cmd[i] -= 32;

    if (strncmp(cmd, "LOG", 3) == 0) {
        /* log one record */
        static unsigned int sample_counter = 1;
        unsigned char rec[RECORD_SIZE];
        make_sample_record(sample_counter++, rec);
        write_record(rec);
        uart_puts("\r\nLogged\r\n");
    } else if (strncmp(cmd, "READN ", 6) == 0) {
        unsigned int n = atoi(&cmd[6]);
        sprintf(uart_buf, "\r\nReading last %u records\r\n", n);
        uart_puts(uart_buf);
        read_last_n(n);
    } else if (strcmp(cmd, "CLEAR") == 0) {
        clear_logs();
    } else {
        uart_puts("\r\nUnknown cmd\r\n");
    }
}

/* ---------------- main ---------------- */
int main(void)
{
    unsigned int ok;

    SPI_Init();
    uart_init();
    uart_puts("\r\nSPI Circular Logger Starting\r\n");

    ok = flash_init_header();
    if (!ok) uart_puts("Header created\r\n");
    else uart_puts("Header present\r\n");

    uart_puts("Commands: LOG, READN n, CLEAR\r\n> ");

    while (1) {
        handle_uart_command();
        uart_puts("> ");
    }

    return 0;
}
