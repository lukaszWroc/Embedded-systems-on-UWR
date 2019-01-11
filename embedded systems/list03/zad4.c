#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include<math.h>
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

void adc_init()
{
  ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
}

FILE uart_file;
void wykonaj_pomiar()
{
    ADCSRA |= _BV(ADSC); // wykonaj konwersję
    while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
    ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
}

void delay(int x)
{
    for(int i=0;i<x;i++)
        _delay_ms(1);
}
#define RT 4700
#define R 10000
#define B 3528
#define R0 4700
#define T0 298
double tp = 23+273.15;
double cal(double a , double b)
{
    if(a < b)
        return b-a;
    return a - b;
}
int goodE(double b)
{
    double T = b/(log((double)R/(double)R0)+log((1024.0-(double)ADC)/(double)ADC) + b/(double)T0);
    if(cal(T,tp) < 0.01)
        return 1;
    return 0;
}

int wyznacz()
{
    int i=6000;
    for(i;i>=0;i--)
    {
        if(goodE(i))
            break;
    }
    return i;
}


int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj ADC
    adc_init();
    while(1)
    {
        wykonaj_pomiar();
        double T = (double)B/(log((double)R/(double)R0)+log((1024.0-(double)ADC)/(double)ADC) + (double)B/(double)T0);
        double t = T - 273.15;
        printf("%f\r\n",t);
        delay(3000);
    }
}
