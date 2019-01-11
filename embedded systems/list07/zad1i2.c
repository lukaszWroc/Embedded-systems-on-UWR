#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include <string.h>

#include "i2c.h"

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
    // ustaw baudrate
    UBRR0 = UBRR_VALUE;
    // wyczysć rejestr UCSR0A
    UCSR0A = 0;
    // włšcz odbiornik i nadajnik
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    // ustaw format 8n1
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
    // czekaj aż transmiter gotowy
    while(!(UCSR0A & _BV(UDRE0)));
    UDR0 = data;
    return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
    // czekaj aż znak dostępny
    while (!(UCSR0A & _BV(RXC0)));
    return UDR0;
}

FILE uart_file;

const uint8_t eeprom_addr = 0xa0;

#define i2cCheck(code, msg) \
    if ((TWSR & 0xf8) != (code)) { \
      printf(msg " failed, status: %.2x\r\n", TWSR & 0xf8); \
      i2cReset(); \
    }
int min(int a,int b)
{
	if(a>b)
		return b;
	return a;
}


int charToInt(char c)
{
    if('a' <= c && c <='f')
        return (c - 'a') + 10;
    if('0' <= c && c <='9')
        return c-'0';
    return 0;
}
int charToInt2(char c1,char c2)
{
    return charToInt(c1) * 16 + charToInt(c2);
}
int tab2[80];
void convFromInt(char tab[]){
    for(int i=0;i<20;i++)
    {
        tab2[i] = charToInt2(tab[i*2+1],tab[i*2+1+1]);
    }
}

void convToInt(uint16_t len,uint16_t addr,uint8_t arr[])
{

	for(int i=0;i<len;i+=16)
	{
		uint8_t check = 0;
		printf(":");
		printf("%.2x ",min(len-i,16));
		printf("%.4x ",addr);
		addr+=16;
		printf("00 ");
		check += min(len-i,16) + addr;
		for(int j=0;j<min(len-i,16);j++)
		{
			check += arr[i+j];
			printf("%.2x ",arr[i+j]);
		}
        printf("%.2x \r\n",check);
	}
}
void readHelper(uint16_t addr, uint16_t len,uint16_t shift,uint8_t arr[])
{
    _delay_ms(30);
    i2cStart();
    i2cCheck(0x08, "I2C start")
    i2cSend(eeprom_addr | ((0x101 & 0x100) >> 7));
    i2cCheck(0x18, "I2C EEPROM write request")
    i2cSend(addr & 0xff);
    i2cCheck(0x28, "I2C EEPROM set address")
    i2cStart();
    i2cCheck(0x10, "I2C second start")
    i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
    i2cCheck(0x40, "I2C EEPROM read request")

    if(addr+len > 256)
    {
        for(int i = 0; addr + i < 255; i++)
        {
            uint8_t data = i2cReadAck();
            i2cCheck(0x50, "I2C EEPROM read")
            arr[shift+i] = data;
        }
        uint8_t data = i2cReadNoAck();
        arr[shift+255-addr] = data;
    }
    else
    {
        for(int i = 0; i < len - 1; i++)
        {
            uint8_t data = i2cReadAck();
            i2cCheck(0x50, "I2C EEPROM read")
            arr[shift+i] = data;
        }
        uint8_t data = i2cReadNoAck();
        arr[shift+len-1] = data;
    }
    i2cCheck(0x58, "I2c ending read")
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
}

void read2(uint16_t addr, uint16_t len)
{
    int state =0;
    if(addr > 511)
	return;
    if(addr > 255)
   	 state=1;
    uint16_t addr1 = addr;
    i2cStart();
    i2cCheck(0x08, "I2C start")
    i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
    i2cCheck(0x18, "I2C EEPROM write request")
    i2cSend(addr & 0xff);
    i2cCheck(0x28, "I2C EEPROM set address")
    i2cStart();
    i2cCheck(0x10, "I2C second start")
    i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
    i2cCheck(0x40, "I2C EEPROM read request")
    addr &= 0xff;
    uint8_t arr[530];
    int spr =0;
    if(addr+len > 256)
    {
        if(state == 1)
        {
            for(int i = 0; addr + i < 255; i++)
            {
                uint8_t data = i2cReadAck();
                i2cCheck(0x50, "I2C EEPROM read")
                arr[i] = data;
            }
            uint8_t data = i2cReadNoAck();
            arr[255-addr] = data;
        }
        else
        {
            for(int i = 0; addr + i < 255; i++)
            {
                uint8_t data = i2cReadAck();
                i2cCheck(0x50, "I2C EEPROM read")
                arr[i] = data;
            }
            uint8_t data = i2cReadNoAck();
            arr[255-addr] = data;
            spr=1;
        }
    }
    else
    {
        for(int i = 0; i < len - 1; i++)
        {
            uint8_t data = i2cReadAck();
            i2cCheck(0x50, "I2C EEPROM read")
            arr[i] = data;
        }
        uint8_t data = i2cReadNoAck();
        arr[len-1] = data;
    }
    i2cCheck(0x58, "I2c ending read")
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
    if(spr == 1 && len-(256-addr) > 0 )
    {
        readHelper(0,len-(256-addr),256-addr,arr);
    }
    convToInt(len,addr1,arr);

}

void write2()
{
    uint16_t addr = tab2[1] *16*16 + tab2[2];
    uint16_t length = tab2[0];
    i2cStart();
    i2cCheck(0x8, "I2cStart")
    i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
    i2cCheck(0x18, "I2C EEPROM write request")
    i2cSend(addr & 0xff);
    i2cCheck(0x28, "I2C EEPROM set address")
    for(int i = 0; i < length; i++)
    {
        uint8_t data = tab2[i+4];

        i2cSend(data);
        i2cCheck(0x28, "I2C EEPROM set data")
    }
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
}

void write3()
{
   while(1)
    {
        char tab[80];
        for(int i=0;i<80;i++)
            tab[i]=0;
        scanf("%s",tab);
        convFromInt(tab);
        if(tab2[3] == 1)
            return;
        write2();
    }
}
void read1(uint16_t addr)
{
    i2cStart();
    i2cCheck(0x08, "I2C start")
    i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
    i2cCheck(0x18, "I2C EEPROM write request")
    i2cSend(addr & 0xff);
    i2cCheck(0x28, "I2C EEPROM set address")
    i2cStart();
    i2cCheck(0x10, "I2C second start")
    i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
    i2cCheck(0x40, "I2C EEPROM read request")
    uint8_t data = i2cReadNoAck();
    i2cCheck(0x58, "I2C EEPROM read")
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
    printf("%"PRIu8"\r\n",data);
}

void write1(uint16_t addr, uint8_t data){
    i2cStart();
    i2cCheck(0x8, "I2cStart")
    i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
    i2cCheck(0x18, "I2C EEPROM write request")
    i2cSend(addr & 0xff);
    i2cCheck(0x28, "I2C EEPROM set address")
    i2cSend(data);
    i2cCheck(0x28, "I2C EEPROM send data")
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
}
int type()
{
    char tab[10];
    for(int i=0;i<10;i++)
    tab[i]=0;
    int cnt=0;
    char c;
    while(1)
    {
        scanf("%c", &c);
        if(c ==  13)
            break;
        if(cnt ==10)
            break;
        tab[cnt++]=c;
    }
    if(strcmp(tab, "read1") == 0)
        return 1;
    if(strcmp(tab, "write1") == 0)
        return 2;
    if(strcmp(tab, "read2") == 0)
        return 3;
    if(strcmp(tab, "write2") == 0)
        return 4;
    printf("%s\r\n", tab);
    return 0;
}
int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejscia/wyjscia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj I2C
    i2cInit();
    uint16_t addr;
    uint8_t data;
    uint16_t len;
    while(1)
    {
        int state = type();
        if(state == 1)
        {
            printf("czytanie1\r\n");
            scanf("%"SCNu16, &addr);
            read1(addr);
        }
        if(state == 2)
        {
            printf("pisanie1\r\n");
            scanf("%"SCNu16, &addr);
            scanf("%hhu", &data);
            write1(addr, data);
            printf("%"PRIu16" %"PRIu8"\r\n", addr, data);
        }
       if(state == 3)
        {
            printf("czytanie2\r\n");
            scanf("%"SCNu16, &addr);
            scanf("%"SCNu16, &len);
            read2(addr,len);
        }
        if(state == 4)
        {
            printf("pisanie2\r\n");
            write3();
        }
    }
}

