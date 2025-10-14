#include <lpc214x.h>
#include <stdio.h>
#include<stdlib.h>
#include <string.h>

// LCD pins
#define LCD_RS  (1<<10)
#define LCD_EN  (1<<11)
#define LCD_D4  (1<<12)
#define LCD_D5  (1<<13)
#define LCD_D6  (1<<14)
#define LCD_D7  (1<<15)

unsigned int rowPins[4] = {16, 17, 18, 19};  // P0.16–P0.19
unsigned int colPins[3] = {20, 21, 22};  // P0.20–P0.23

void delay_ms(unsigned int);
void lcd_init(void);
void lcd_cmd(unsigned char);
void lcd_data(unsigned char);
void lcd_string(char *);
void lcd_clear(void);
void lcd_send_nibble(unsigned char);
void lcd_pulse(void);
void keypad_init(void);
char keypad_getkey(void);
void pwm_init(void);
void pwm_set_duty(unsigned int);


int main(void)
{
    char key, buffer[16];
    char duty_str[4] = {0};
    unsigned int duty = 0;
    int i = 0;

    lcd_init();
    keypad_init();
    pwm_init();

    lcd_string("PWM Motor Control");
    delay_ms(1500);
    lcd_clear();

    lcd_string("Enter Duty %:");
    lcd_cmd(0xC0);

    while (1)
    {
        key = keypad_getkey();

        if (key >= '0' && key <= '9')
        {
            lcd_data(key);
            duty_str[i++] = key;
        }
        else if (key == '#')  // '#' to confirm input
        {
            duty_str[i] = '\0';
            duty = atoi(duty_str);
            if (duty > 100) duty = 100;

            pwm_set_duty(duty);

            lcd_clear();
            sprintf(buffer, "Duty: %u%%", duty);
            lcd_string(buffer);

            i = 0;  // reset for next input
            memset(duty_str, 0, sizeof(duty_str));
            lcd_cmd(0xC0);
        }
    }
}

void pwm_init(void)
{
    PINSEL0 |= 0x00008000;   // P0.7 -> PWM2
    PWMPCR = (1<<10);        // Enable PWM2 output
    PWMMR0 = 1000;           // Period (1000 = 100%)
    PWMMCR = (1<<1);         // Reset on MR0
    PWMTCR = (1<<1);         // Reset counter
    PWMTCR = (1<<0) | (1<<3); // Enable counter & PWM mode
}

void pwm_set_duty(unsigned int duty)
{
    unsigned int value;
    value = (duty * PWMMR0) / 100;
    PWMMR2 = value;
    PWMLER = (1<<2);
}

void lcd_send_nibble(unsigned char nibble)
{
    IOCLR0 = LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7;
    if (nibble & 0x01) IOSET0 = LCD_D4;
    if (nibble & 0x02) IOSET0 = LCD_D5;
    if (nibble & 0x04) IOSET0 = LCD_D6;
    if (nibble & 0x08) IOSET0 = LCD_D7;
    lcd_pulse();
}

void lcd_pulse(void)
{
    IOSET0 = LCD_EN;
    delay_ms(1);
    IOCLR0 = LCD_EN;
    delay_ms(1);
}

void lcd_cmd(unsigned char cmd)
{
    IOCLR0 = LCD_RS;
    lcd_send_nibble(cmd >> 4);
    lcd_send_nibble(cmd & 0x0F);
    delay_ms(2);
}

void lcd_data(unsigned char data)
{
    IOSET0 = LCD_RS;
    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);
    delay_ms(2);
}

void lcd_init(void)
{
    IODIR0 |= LCD_RS | LCD_EN | LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7;
    delay_ms(20);
    lcd_cmd(0x02);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
    delay_ms(2);
}

void lcd_string(char *str)
{
    while (*str)
        lcd_data(*str++);
}

void lcd_clear(void)
{
    lcd_cmd(0x01);
    delay_ms(2);
}
void keypad_init(void)
{
    int i;
    for (i = 0; i < 4; i++)
        IODIR0 |= (1 << rowPins[i]);    // Rows output

    for (i = 0; i < 4; i++)
        IODIR0 &= ~(1 << colPins[i]);   // Cols input
}

char keypad_getkey(void)
{
    const char keys[4][3] = {
        {'1','2','3'},
        {'4','5','6'},
        {'7','8','9'},
        {'*','0','#'}
    };

    int r, c;

    while (1)
    {
        for (r = 0; r < 4; r++)
        {
            IOSET0 = (0x0F << 16);     // Set all rows high
            IOCLR0 = (1 << rowPins[r]); // Pull one row low

            for (c = 0; c < 3; c++)
            {
                if ((IOPIN0 & (1 << colPins[c])) == 0)
                {
                    delay_ms(20);  // debounce
                    while ((IOPIN0 & (1 << colPins[c])) == 0);
                    return keys[r][c];
                }
            }
        }
    }
}
void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 10000; j++);
}
