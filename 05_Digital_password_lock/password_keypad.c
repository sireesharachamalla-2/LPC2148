#include <lpc214x.h>
#include <string.h>

// LCD pin connections
#define LCD_RS  (1<<10)   // P0.10
#define LCD_EN  (1<<11)   // P0.11
#define LCD_D4  (1<<12)   // P0.12
#define LCD_D5  (1<<13)   // P0.13
#define LCD_D6  (1<<14)   // P0.14
#define LCD_D7  (1<<15)   // P0.15

// Keypad connections
#define ROWS 4
#define COLS 3
unsigned int rowPins[4] = {16, 17, 18,19};  // P0.16–P0.19
unsigned int colPins[3] = {20, 21, 22};  // P0.20–P0.23

// LED/Relay output
#define LOCK_PIN (1<<25)  // P0.25

// Function prototypes
void delay_ms(unsigned int ms);
void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_init(void);
void lcd_string(char *str);
void lcd_clear(void);
char keypad_getkey(void);

void lcd_pulse_enable(void);
void lcd_send_nibble(unsigned char nibble);

void init_keypad(void);
void init_lcd_pins(void);

// Stored password
char password[] = "1234";

int main(void)
{
    char entered[5];
    int i;

    // Configure I/O
    init_lcd_pins();
    init_keypad();
    lcd_init();

    IODIR0 |= LOCK_PIN;   // Configure P0.25 as output (LED/Relay)
    IOCLR0 = LOCK_PIN;    // Lock OFF initially

    lcd_string("Enter Password:");
    lcd_cmd(0xC0);        // Move to 2nd line

    for (i = 0; i < 4; i++)
    {
        entered[i] = keypad_getkey();
        lcd_data('*');        
    }
    entered[4] = '\0';        

    delay_ms(500);

    if (strcmp(entered, password) == 0)
    {
        lcd_clear();
        lcd_string("SUCCESS");
        IOSET0 = LOCK_PIN;     
    }
    else
    {
        lcd_clear();
        lcd_string("FAILURE");
        IOCLR0 = LOCK_PIN;     
    }

    while (1);
}

void init_lcd_pins(void)
{
    IODIR0 |= LCD_RS | LCD_EN | LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7; // Set as output
}

void lcd_send_nibble(unsigned char nibble)
{
    IOCLR0 = LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7;   // Clear lines

    if (nibble & 0x01) IOSET0 = LCD_D4;
    if (nibble & 0x02) IOSET0 = LCD_D5;
    if (nibble & 0x04) IOSET0 = LCD_D6;
    if (nibble & 0x08) IOSET0 = LCD_D7;

    lcd_pulse_enable();
}

void lcd_pulse_enable(void)
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
    delay_ms(20);
    lcd_cmd(0x02);  // Initialize in 4-bit mode
    lcd_cmd(0x28);  // 2 lines, 5x8 font
    lcd_cmd(0x0C);  // Display ON, Cursor OFF
    lcd_cmd(0x06);  // Entry mode
    lcd_cmd(0x01);  // Clear display
    delay_ms(2);
}

void lcd_clear(void)
{
    lcd_cmd(0x01);
    delay_ms(2);
}

void lcd_string(char *str)
{
    while (*str)
        lcd_data(*str++);
}

void init_keypad(void)
{
    int i;
    for (i = 0; i < ROWS; i++)
        IODIR0 |= (1 << rowPins[i]);    // Set rows as output

    for (i = 0; i < COLS; i++)
        IODIR0 &= ~(1 << colPins[i]);   // Set cols as input
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
        for (r = 0; r < ROWS; r++)
        {
            // Set all rows high first
            IOSET0 = (0x0F << 16);
            IOCLR0 = (1 << rowPins[r]); // Pull one row low

            for (c = 0; c < COLS; c++)
            {
                if ((IOPIN0 & (1 << colPins[c])) == 0)  // Key pressed
                {
                    delay_ms(20); // Debounce delay
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
