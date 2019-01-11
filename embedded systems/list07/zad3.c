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

const uint8_t eeprom_addr = 0b11010000; //0xa0;

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


uint8_t read1(uint16_t addr)
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
    return data;
}

void write1(uint16_t addr, uint8_t data)
{
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

int getBit(uint8_t date, int bit)
{
    return ((date >> bit) & 1);
}
uint16_t startYear = 1980;
void date()
{
    uint8_t d = read1(4);
    uint8_t m = read1(5);
    uint16_t y = read1(6);
    uint8_t mies = (m&0b1111) + getBit(m,4)*10;
    uint8_t dni = (d&0b1111) + ((d&0b111111) >> 4) * 10;
    uint16_t rok = (y & 0b1111) + ((y>> 4 ) & 0b1111)* 10; //* getBit(m,7)*100;
    rok+=startYear;
    printf("%.2d-%.2d-%.4d\r\n",dni,mies,rok);
}

void setdate()
{
    uint8_t d;
    uint8_t m;
    uint16_t y;
    scanf("%hhu-%hhu-%d", &d, &m, &y);
    printf("date to set %.2d-%.2d-%.4d",d,m,y);
    if(y > startYear)
        y-=startYear;
    uint8_t yy =y;
    uint8_t dni = (((d/10)&3) << 4) + ((d%10)&0b1111);
    uint8_t mies = (((m/10)&1) << 4) + ((m%10)&0b1111);
    uint8_t rok = (((yy/10)&0b1111) << 4) + ((yy%10)&0b1111);
    write1(4,dni);
    write1(5,mies);
    write1(6,rok);
}


void time1()
{
    uint8_t s = read1(0);
    uint8_t m = read1(1);
    uint8_t h = read1(2);
    uint8_t sek = (s&0b1111) + ((s&0b1111111)>>4)*10;
    uint8_t min =( m&0b1111) + ((m&0b1111111)>>4)*10;
    uint8_t godz =( h&0b1111) + getBit(h,4)*10 + getBit(h,5)*20;
    printf("time :  %.2d:%.2d:%.2d\r\n",godz,min,sek);
}
void settime1()
{
    uint8_t s;
    uint8_t m;
    uint8_t g;
    scanf("%hhu:%hhu:%hhu", &g,&m,&s);
    printf("time to set :  %.2d:%.2d:%.2d\r\n",g,m,s);
    uint8_t sek = (s%10) + ((s/10)<<4);
    uint8_t min =(m%10) + ((m/10)<<4);
    uint8_t godz =(g%10);
    if(g<20 && g >9)
        godz += (1<<4);
    if(g>20)
        godz += (1<<5);
    write1(0,sek);
    write1(1,min);
    write1(2,godz);
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
    if(strcmp(tab, "date") == 0)
        return 3;
    if(strcmp(tab, "time") == 0)
        return 4;
    if(strcmp(tab, "set date") == 0)
     return 5;
    if(strcmp(tab, "set time") == 0)
        return 6;
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

    while(1)
    {
        int state = type();
        if(state == 3)
        {
            printf("date\r\n");
            date();
        }
        if(state == 4)
        {
            printf("time\r\n");
            time1();
        }
        if(state == 5)
        {
            printf("set date\r\n");
            setdate();
        }
        if(state == 6)
        {
            printf("set time\r\n");
            settime1();
        }
    }
}

