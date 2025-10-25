#include<LPC214x.h>
#include<stdio.h>
#include<string.h>

#define E_READ  0XA1   //read (24c02 device addres)
#define E_ADDR 0XA0   //write(24c02 device address)

unsigned int log_addr=0;   //EEPROM address
char buf[20];

//delay 
void delay_ms(unsigned int ms)
{
	unsigned int i,j;
	for(i=0;i<ms;i++)
	for(j=0;j<6000;j++);
}
//         UART
void UartInit(void)
{
	PINSEL0|=0x05;   //p0.0=txd,p0.1=rxd
	U0LCR=0X83;       //8-bit data, 1stop, no parity, enable DLAB
	U0DLL=97;          //pclk=15Mhz,9600 baudrate
	U0DLM=0;         
	U0LCR=0X03;        //disable baudrate
}

void UartSend(char ch)
{
	while(!(U0LSR&0x20));  //wait THR to transmit data
  U0THR=ch;
}

char UartRec(void)
{
	while(!(U0LSR &0x01));
	return U0RBR;
}

void UartStr(char *str)
{
	while(*str)
		UartSend(*str++);
}


//         I2C 

void I2cInit(void)
{
	PINSEL0|=0x50 ;   //p0.2=SCL0, p0.3=SDA0
	I2C0CONCLR=0x6c;  //clear flags
	I2C0SCLH=75;
	I2C0SCLL=75;      //100khz ,15Mhz PCLK
	I2C0CONSET=0X40;  //enable I2c
}

void I2cStart(void)
{
	I2C0CONSET=0X20;  //set start bit
	while(!(I2C0CONSET & 0x08));  //wait for SI
	I2C0CONCLR=0X28;   //clear start,SI
}


void I2cStop(void)
{
	I2C0CONSET=0X10;   //set stop
	I2C0CONCLR=0x08;    //clear SI
}

void I2cWrite(unsigned char data)
{
	I2C0DAT=data;
	I2C0CONCLR=0x08;
	while(!(I2C0CONSET & 0X08));
}
unsigned char I2cRead(unsigned char ack)
{
	if(ack)
		I2C0CONSET=0x04;
	else
		I2C0CONCLR=0X04;
	
	I2C0CONCLR=0x08;
	while(!(I2C0CONSET & 0x08));
	
	return I2C0DAT;
}

void EEPROM_WriteByte(unsigned int addr,unsigned char data)
{
	I2cStart();
	I2cWrite(E_ADDR);
	I2cWrite((unsigned char)(addr & 0XFF));
	I2cWrite(data);
	I2cStop();
	delay_ms(10);
}

unsigned char EEPROM_ReadByte(unsigned int addr)
{
	unsigned char data;
	I2cStart();
	I2cWrite(E_ADDR);
	I2cWrite((unsigned char)(addr & 0XFF));
	I2cStart();
	I2cWrite(E_READ);
	data=I2cRead(0);
	I2cStop();
	return data;
}

int main()
{
	unsigned int value=0;
	char cmd[10];
	unsigned int i,n;
	 
	I2cInit();
	UartInit();
	UartStr("EEPROM Data Logger Start\r\n");
	
	while(1)
	{
		UartStr("\r\n Enter cmd(LOG/READALL):");
		i=0;
	while(1)
	{
		char ch=UartRec();
		UartSend(ch);
		if(ch=='\r')
			break;
		cmd[i++]=ch;
	}
	cmd[i]='\0';
	
	if(strcmp(cmd,"LOG")==0)
	{
		value++;
		EEPROM_WriteByte(log_addr++,value);
		sprintf(buf,"\r\nLogged value: %u at ADDR: %u\r\n",value,log_addr-1);
		UartStr(buf);
	}
	else if(strcmp(cmd,"READALL")==0)
	{
		UartStr("\r\nStored EEPROM data: \r\n");
		for(n=0;n<log_addr;n++)
		{
			unsigned char val=EEPROM_ReadByte(n);
			sprintf(buf,"Addr %u:%u\r\n",n,val);
			UartStr(buf);
			delay_ms(100);
		}
	}
	else
	{
		UartStr("\r\ninvalid cmd");
	}
}
}
	