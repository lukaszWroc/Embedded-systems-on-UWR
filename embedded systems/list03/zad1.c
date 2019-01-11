#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#define BUZZ PB5
#define BUZZ_DDR DDRB
#define BUZZ_PORT PORTB


#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

void uart_init()
{
    UBRR0 = UBRR_VALUE;
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

int uart_transmit(char data, FILE *stream)
{
    while(!(UCSR0A & _BV(UDRE0)));
    UDR0 = data;
    return 0;
}

int uart_receive(FILE *stream)
{
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}
int c = 261;
int d = 294;
int e = 329;
int f = 349;
int g = 391;
int gS = 415;
int a = 440;
int aS = 455;
int b = 466;
int cH = 523;
int cSH = 554;
int dH = 587;
int dSH = 622;
int eH = 659;
int fH = 698;
int fSH = 740;
int gH = 784;
int gSH = 830;
int aH = 880;
static const char tab[] PROGMEM  =
"<a,500>"
"<a,500>"
"<a,500>"
"<f,350>"
"<cH,150>"
"<a,500>"
"<f,350>"
"<cH,150>"
"<a,650>"
"<Q,500>"
"<eH,500>"
"<eH,500>"
"<eH,500>"
"<fH,350>"
"<cH,150>"
"<gS,500>"
"<f,350>"
"<cH,150>"
"<a,650>"
"<Q,500>"
"<aH,500>"
"<a,300>"
"<a,150>"
"<aH,500>"
"<gSH,325>"
"<gH,175>"
"<fSH,125>"
"<fH,125>"
"<fSH,250>"
"<Q,325>"
"<aS,250>"
"<dSH,500>"
"<dH,325>"
"<cSH,175>"
"<cH,125>"
"<b,125>"
"<cH,250>"
"<Q,350>"
"<f,250>"
"<gS,500>"
"<f,350>"
"<a,125>"
"<cH,500>"
"<a,375>"
"<cH,125>"
"<eH,650>"
"<Q,500>"
"<aH,500>"
"<a,300>"
"<a,150>"
"<aH,500>"
"<gSH,325>"
"<gH,175>"
"<fSH,125>"
"<fH,125>"
"<fSH,250>"
"<Q,325>"
"<aS,250>"
"<dSH,500>"
"<dH,325>"
"<cSH,175>"
"<cH,125>"
"<b,125>"
"<cH,250>"
"<Q,350>"
"<f,250>"
"<gS,500>"
"<f,375>"
"<cH,125>"
"<a,500>"
"<f,375>"
"<cH,125>"
"<a,650>"
"<Q,650>";

#define TONE(step, delay) \
    for (uint16_t i = 0; i < (uint32_t)1000 * (delay) / (step) / 2; i++) { \
      BUZZ_PORT |= _BV(BUZZ); \
      delayUs(step); \
      BUZZ_PORT &= ~_BV(BUZZ); \
      delayUs(step); \
    }

void delayUs(int x)
{
    for(int i=0;i<x;i++)
        _delay_us(1);
}
void delay(int x)
{
    for(int i=0;i<x;i++)
        _delay_ms(1);

}

int cmp(char tmp[])
{
    if(strcmp(tmp,"c") == 0)
        return 261;
    if(strcmp(tmp,"d") == 0)
        return 294;
    if(strcmp(tmp,"e") == 0)
        return 329;
    if(strcmp(tmp,"f") == 0)
        return 349;
    if(strcmp(tmp,"g") == 0)
        return 391;
    if(strcmp(tmp,"gS") == 0)
        return 415;
    if(strcmp(tmp,"a") == 0)
        return 440;
    if(strcmp(tmp,"aS") == 0)
        return 455;
    if(strcmp(tmp,"b") == 0)
        return 466;
    if(strcmp(tmp,"cH") == 0)
        return 523;
    if(strcmp(tmp,"cSH") == 0)
        return 554;
    if(strcmp(tmp,"dH") == 0)
        return 587;
    if(strcmp(tmp,"dSH") == 0)
        return 622;
    if(strcmp(tmp,"eH") == 0)
        return 659;
    if(strcmp(tmp,"fH") == 0)
        return 698;
    if(strcmp(tmp,"fSH") == 0)
        return  740;
    if(strcmp(tmp,"gH") == 0)
        return  784;
    if(strcmp(tmp,"gSH") == 0)
        return 830;
    if(strcmp(tmp,"aH") == 0)
        return 880;
    return 0;
}


int toInt(char tmp[])
{
    _delay_ms(500);
    int sum = 0;
    for(int i=0;i<4;i++)
    {
    if(tmp[i] == 0)
    break;
        sum*=10;
        sum +=(int)(tmp[i] - '0');
    }
    return sum;
}
void parse()
{
    for (uint8_t i = 0; i < sizeof(tab); i++)
    {
        char c = pgm_read_byte(&tab[i]);

        if(c == '<')
        {
            uint8_t cnt=1;
            char tmp[10];
            for(int j=0;j<10;j++)
                tmp[j] = 0;
            while(1)
            {
                if(pgm_read_byte(&tab[i+cnt]) == ',')
                    break;
                tmp[cnt-1] = pgm_read_byte(&tab[i+cnt]);
                cnt++;
            }
            int hz = cmp(tmp);
            i+=cnt;
            cnt=1;
            for(int j=0;j<10;j++)
                tmp[j] = 0;
            while(1)
            {
                if(pgm_read_byte(&tab[i+cnt]) == '>')
                    break;
                tmp[cnt-1] = pgm_read_byte(&tab[i+cnt]);
                cnt++;
            }
            int d = toInt(tmp);
            i+=cnt;
            if(hz == 0)
                delay(d-minus);
            else
                TONE(hz,d-minus);
        }
    }
}

FILE uart_file;

int main() {
    BUZZ_DDR |= _BV(BUZZ);
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj ADC
    printf("start");
    while (1)
        parse();

}
