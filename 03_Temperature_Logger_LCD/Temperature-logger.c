#include <LPC214x.h>
#include <stdio.h>

#define RS (1<<16)       // P0.16
#define EN (1<<17)       // P0.17
#define D4 (1<<20)       // P0.20
#define D5 (1<<21)       // P0.21
#define D6 (1<<22)       // P0.22
#define D7 (1<<23)       // P0.23
#define LCD_DATA (D4|D5|D6|D7)

// Delay function
void delay_ms(unsigned int ms) {
    unsigned int i,j;
    for(i=0;i<ms;i++) {
        for(j=0;j<10000;j++);
    }
}

// Send nibble to LCD
void lcd_send_nibble(unsigned char nibble) {
    IOCLR0 = LCD_DATA; // clear D4-D7
    if(nibble & 0x10) IOSET0 = D4;
    if(nibble & 0x20) IOSET0 = D5;
    if(nibble & 0x40) IOSET0 = D6;
    if(nibble & 0x80) IOSET0 = D7;

    IOSET0 = EN;
    delay_ms(1);
    IOCLR0 = EN;
}

// Send command to LCD
void lcd_cmd(unsigned char cmd) {
    IOCLR0 = RS;
    lcd_send_nibble(cmd & 0xF0);
    lcd_send_nibble((cmd<<4) & 0xF0);
    delay_ms(2);
}

// Send data to LCD
void lcd_data(unsigned char data) {
    IOSET0 = RS;
    lcd_send_nibble(data & 0xF0);
    lcd_send_nibble((data<<4) & 0xF0);
    delay_ms(2);
}

// Initialize LCD in 4-bit mode
void lcd_init(void) {
    IODIR0 |= RS|EN|LCD_DATA; // Set pins as output
    delay_ms(20);

    // LCD initialization 
    lcd_send_nibble(0x30);
    delay_ms(5);
    lcd_send_nibble(0x30);
    delay_ms(1);
    lcd_send_nibble(0x30);
    delay_ms(1);
    lcd_send_nibble(0x20); // Set 4-bit mode
    delay_ms(1);

    lcd_cmd(0x28); // 4-bit, 2 lines
    lcd_cmd(0x0C); // Display ON, cursor OFF
    lcd_cmd(0x06); // Entry mode
    lcd_cmd(0x01); // Clear display
    delay_ms(2);
}


void lcd_string(char *str) {
    while(*str) {
        lcd_data(*str++);
    }
}

// ADC (P0.28 = AD0.1)
void adc_init(void) {
    PINSEL1 |= (1<<24);      // P0.28 as AD0.1
    AD0CR = (1<<1)            // Select channel 1
          | (4<<8)            // Clock divisor
          | (1<<21);          // Enable ADC
}

// Read  value
unsigned int adc_read(void) {
    unsigned int result;
    AD0CR |= (1<<24);         // Start conversion now
    while((AD0GDR & (1U<<31)) == 0); // Wait until done
    result = (AD0GDR >> 6) & 0x3FF;  // 10-bit result
    return result;
}

int main(void) {
    char buffer[16];
    unsigned int adc_val;
    float voltage, temp;

    lcd_init();
    adc_init();

    lcd_cmd(0x80);
    lcd_string("LM35 Temperature");
    delay_ms(2000);
    lcd_cmd(0x01); // Clear display

    while(1) {
        adc_val = adc_read();
        voltage = (adc_val * 3.3) / 1023.0; // Convert ADC to voltage
        temp = voltage * 100.0;             
        lcd_cmd(0x80); 
      printf("Temp: %.2f C   ", temp);  
			lcd_string(buffer);

        delay_ms(500);
    }
}

