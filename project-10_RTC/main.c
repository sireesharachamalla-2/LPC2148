#include <lpc214x.h>
#include <stdio.h>
#include <stdbool.h> 
#define LCD_RS (1<<10)
#define LCD_RW (1<<11)
#define LCD_EN (1<<12)
#define LCD_DATA_MASK (0xFF<<16)

volatile unsigned int hh=0, mm=0, ss=0;  // Time variables
char rx_buf[16];
unsigned int rx_index=0;
bool time_set = false; 

//Delay 
void delay_ms(unsigned int ms){
    unsigned int i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<6000;j++);
}

//LCD 
void lcd_cmd(unsigned char cmd){
    IOCLR0 = LCD_DATA_MASK;
    IOSET0 = (cmd<<16);
    IOCLR0 = LCD_RS;
    IOCLR0 = LCD_RW;
    IOSET0 = LCD_EN;
    delay_ms(2);
    IOCLR0 = LCD_EN;
}

void lcd_data(unsigned char data){
    IOCLR0 = LCD_DATA_MASK;
    IOSET0 = (data<<16);
    IOSET0 = LCD_RS;
    IOCLR0 = LCD_RW;
    IOSET0 = LCD_EN;
    delay_ms(2);
    IOCLR0 = LCD_EN;
}

void lcd_init(void){
    IODIR0 |= LCD_RS|LCD_RW|LCD_EN|LCD_DATA_MASK;
    delay_ms(20);
    lcd_cmd(0x38); // 8-bit, 2-line
    lcd_cmd(0x0C); // Display ON
    lcd_cmd(0x01); // Clear
    delay_ms(2);
}

void lcd_string(char *str){
    while(*str) lcd_data(*str++);
}

//---------------- UART ----------------
void uart0_init(void){
    PINSEL0 |= 0x05;  // P0.0 = TXD0, P0.1 = RXD0
    U0LCR = 0x83;     // 8N1, enable DLAB
    U0DLL = 97;       // 9600 baud @ 15MHz PCLK
    U0DLM = 0;
    U0LCR = 0x03;     // Disable DLAB
}

void uart0_tx(char ch){
    while(!(U0LSR & 0x20));
    U0THR = ch;
}

void uart0_string(char *s){
    while(*s) uart0_tx(*s++);
}

char uart0_rx(void){
    while(!(U0LSR & 0x01));
    return U0RBR;
}

//Timer0 ISR 
__irq void timer0_isr(void){
    T0IR = 1; // clear interrupt flag

    if(time_set){   // Only run clock after time is set
        ss++;
        if(ss>=60){ ss=0; mm++; }
        if(mm>=60){ mm=0; hh++; }
        if(hh>=24){ hh=0; }
    }

    VICVectAddr = 0; // end of interrupt
}

void timer0_init(void){
    T0CTCR = 0x0;       // Timer mode
    T0PR   = 15000000-1; // Prescaler for 1 sec (PCLK=15MHz)
    T0MR0  = 1;          // Match after 1 increment
    T0MCR  = 3;          // Interrupt + Reset on MR0

    VICIntSelect &= ~(1<<4);  // Timer0 = IRQ
    VICVectAddr0 = (unsigned)timer0_isr;
    VICVectCntl0 = (1<<5) | 4; // Enable slot, Timer0
    VICIntEnable = (1<<4);     // Enable Timer0 interrupt

    T0TCR = 1; // Enable timer
}

//Main 
int main(void){
    char buf[20];

    lcd_init();
    uart0_init();
    timer0_init();

    lcd_cmd(0x80);
    lcd_string("Set Time UART");
    lcd_cmd(0xC0);
    lcd_string("Format HH:MM:SS");
    uart0_string("\r\nEnter Time in HH:MM:SS format:\r\n");

    //Read time from UART 
    while(!time_set){
        char ch = uart0_rx();
        uart0_tx(ch);  // echo back

        if(ch=='\r'){ // Enter pressed
            rx_buf[rx_index] = '\0';
            sscanf(rx_buf,"%u:%u:%u",&hh,&mm,&ss);
            time_set = 1;
        } else {
            rx_buf[rx_index++] = ch;
        }
    }

    lcd_cmd(0x01); // clear

    // Main Loop 
    while(1){
        sprintf(buf,"Time %02d:%02d:%02d",hh,mm,ss);
        lcd_cmd(0x80);
        lcd_string(buf);
        delay_ms(200);
    }
}
