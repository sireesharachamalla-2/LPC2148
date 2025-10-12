#include <lpc21xx.h>
#define RS (1 << 8)
#define EN (1 << 9)
#define D4 (1 << 10)
#define D5 (1 << 11)
#define D6 (1 << 12)
#define D7 (1 << 13)
#define R1 (1 << 16)
#define R2 (1 << 17)
#define R3 (1 << 18)
#define R4 (1 << 19)
#define C1 (1 << 20)
#define C2 (1 << 21)
#define C3 (1 << 22)
#define C4 (1 << 23)
char keypad[4][4] = {
    {'7','8','9','/'},
    {'4','5','6','*'},
    {'1','2','3','-'},
    {'C','0','=','+'}
};
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 6000; j++);
}
void lcd_enable_pulse() {
    IO0SET = EN;
    delay_ms(1);
    IO0CLR = EN;
    delay_ms(1);
}
void lcd_send_nibble(unsigned char nibble) {
    IO0CLR = D4 | D5 | D6 | D7;

    if(nibble & 0x01) IO0SET |= D4;
    if(nibble & 0x02) IO0SET |= D5;
    if(nibble & 0x04) IO0SET |= D6;
    if(nibble & 0x08) IO0SET |= D7;
    lcd_enable_pulse();
}
void lcd_send(unsigned char value, int is_data) {
    if(is_data)
        IO0SET = RS;
    else
        IO0CLR = RS;
    lcd_send_nibble((value >> 4) & 0x0F);
    lcd_send_nibble(value & 0x0F);
    delay_ms(2);
}
void lcd_cmd(unsigned char cmd) {
    lcd_send(cmd, 0);
}
void lcd_data(unsigned char data) {
    lcd_send(data, 1);
}
void lcd_init() {
    IO0DIR |= RS | EN | D4 | D5 | D6 | D7;
    delay_ms(20);
    lcd_send_nibble(0x03); delay_ms(5);
    lcd_send_nibble(0x03); delay_ms(5);
    lcd_send_nibble(0x03); delay_ms(5);
    lcd_send_nibble(0x02); // 4-bit mode
    lcd_cmd(0x28); 					// 4-bit, 2-line, 5x8
    lcd_cmd(0x0C); 					// Display ON, cursor OFF
    lcd_cmd(0x06); 					// Entry mode
    lcd_cmd(0x01); 					// Clear display
    delay_ms(5);
}
void lcd_goto(unsigned char row, unsigned char col) {
    if(row == 1)
        lcd_cmd(0x80 + col);
    else
        lcd_cmd(0xC0 + col);
}
void lcd_print(char *str) {
    while(*str)
        lcd_data(*str++);
	}
void itoa(int num, char *str, int base) {
	int j;
    int i = 0, isNegative = 0;
    if(num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    if(num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }
    while(num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
        num /= base;
    }
    if(isNegative)
        str[i++] = '-';
    str[i] = '\0';

    // Reverse string
    for( j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

char keypad_scan() {
    int row, col;
    IO1DIR = R1 | R2 | R3 | R4; 				// Rows output
    IO1CLR = R1 | R2 | R3 | R4;
    IO1DIR &= ~(C1 | C2 | C3 | C4); 		// Columns input

    for(row = 0; row < 4; row++) {
        IO1SET = R1 | R2 | R3 | R4;
        IO1CLR = (1 << (16 + row)); 		// Make one row LOW
        delay_ms(2);

        for(col = 0; col < 4; col++) {
            if(!(IO1PIN & (1 << (20 + col)))) {
                while(!(IO1PIN & (1 << (20 + col)))); 
                return keypad[row][col];
            }
        }
    }
    return 0;
}

//Calculator Operation 
int calculate(int a, int b, char op) {
    switch(op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? a / b : 0;
        default: return 0;
    }
}
int main() {
    char key, op = 0;
    int num1 = 0, num2 = 0, result = 0;
    int state = 0; // 0 = num1, 1 = operator, 2 = num2
    char buffer[16];

    lcd_init();
    lcd_goto(1, 0);
    lcd_print("Calculator Ready");

    while(1) {
        key = keypad_scan();
        if(key) {
            if(key >= '0' && key <= '9') {
                lcd_data(key);
                if(state == 0)
                    num1 = num1 * 10 + (key - '0');
                else if(state == 2)
                    num2 = num2 * 10 + (key - '0');
            }
            else if(key == '+' || key == '-' || key == '*' || key == '/') {
                if(state == 0) {
                    op = key;
                    state = 2; 				// Move to second number input
                    lcd_data(op);
                }
            }
            else if(key == '=') {
                result = calculate(num1, num2, op);
                lcd_cmd(0x01); // Clear screen
                lcd_goto(1, 0);
                lcd_print("Result: ");
                itoa(result, buffer, 10);
                lcd_print(buffer);

                // Reset
                num1 = num2 = result = 0;
                op = 0;
                state = 0;
            }
            else if(key == 'C') {
                // Clear all
                num1 = num2 = result = 0;
                op = 0;
                state = 0;
                lcd_cmd(0x01);
                lcd_goto(1, 0);
                lcd_print("Cleared");
                delay_ms(800);
                lcd_cmd(0x01);
                lcd_goto(1, 0);
							lcd_print("Calculator :");
            }
        }
    }
}
