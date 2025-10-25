#include<LPC214x.h>
#include<stdio.h>
volatile unsigned int cnt=0;

#define LCD_DATA (D4|D5|D6|D7)
#define RS (1<<16)       // P0.16
#define EN (1<<17)       // P0.17
#define D4 (1<<20)       // P0.20
#define D5 (1<<21)       // P0.21
#define D6 (1<<22)       // P0.22
#define D7 (1<<23)       // P0.23

//delay
void delay_ms(unsigned int ms) {
    unsigned int i,j;
    for(i=0;i<ms;i++) {
        for(j=0;j<10000;j++);
    }
}

//send to lcd
void lcd_send_nibble(unsigned char nibble) {
    IOCLR0 = LCD_DATA;       // clear D4-D7
    if(nibble & 0x10) IOSET0 = D4;
    if(nibble & 0x20) IOSET0 = D5;
    if(nibble & 0x40) IOSET0 = D6;
    if(nibble & 0x80) IOSET0 = D7;

    IOSET0 = EN;
    delay_ms(1);
    IOCLR0 = EN;
}

void lcd_cmd(unsigned char cmd) {
    IOCLR0 = RS;
    lcd_send_nibble(cmd & 0xF0);
    lcd_send_nibble((cmd<<4) & 0xF0);
    delay_ms(2);
}

void lcd_data(unsigned char data) {
    IOSET0 = RS;
    lcd_send_nibble(data & 0xF0);
    lcd_send_nibble((data<<4) & 0xF0);
    delay_ms(2);
}

// Initialize LCD in 4-bit mode
void lcd_init(void)
{
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

void lcd_string(char *str)
{
    while(*str) {
        lcd_data(*str++);
    }
}

/*void GPIO_Reset(void) 
{
    IODIR0 &= ~(1 << 10);        //P0.10 as input-Reset
}*/

void EINT0_ISR(void)__irq
{
	cnt++;
	EXTINT=1<<0;
	VICVectAddr=0;
}

void EINT0_Init(void)
{
    PINSEL0 |= (3 << 29);        // P0.14 as EINT0
    EXTMODE |= (1 << 0);         
    EXTPOLAR &= ~(1 << 0);        
    VICIntSelect &= ~(1 << 14);  // EINT0 as IRQ
    VICVectAddr1 = (unsigned int)EINT0_ISR;
    VICVectCntl1 = 0x20 | 14;   // Enable slot for EINT0
    VICIntEnable = (1 << 14);    // Enable EINT0 interrupt
}

int main(void)
{
  char buffer[16];
	lcd_init();
	lcd_cmd(0x80);   //1st line
	lcd_string("Event Count:");
	
	lcd_cmd(0xC0);  //2nd line
  
	sprintf(buffer,"%d ",cnt);
	
	lcd_string(buffer);
	
	//lcd_str("\r\n");
	
    EINT0_Init();
	__enable_irq();
    //GPIO_Reset();
		while(1)
    {
        if((IOPIN0 &(1 <<10))==0) 
				{
            cnt = 0;
           
            while((IOPIN0&(1<<10))==0); // wait for release
						delay_ms(100);
				}
						lcd_cmd(0xC0);
            sprintf(buffer,"%d",cnt);
						lcd_string(buffer);
						delay_ms(500);
       }
			 return 0;
}
