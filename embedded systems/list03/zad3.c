#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem
#define MAX 1023

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

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

// inicjalizacja ADC
void adc_init()
{
    ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
    DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
    // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    ADCSRA |= _BV(ADEN); // włącz ADC
}

FILE uart_file;
void wykonaj_pomiar()
{
    LED_PORT &=  ~_BV(LED);
    ADCSRA |= _BV(ADSC); // wykonaj konwersję
    while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
    ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
}
int tab[7] = {65,91,132,198,354,700,1022};
int val()
{
    int i=0;
    for(i;i<7;i++)
    {
        if(ADC < tab[i])
        break;
    }
    return i+1;
}
void delay(int x)
{
    for(int i=0;i<x;i++)
        _delay_us(100);
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
    LED_DDR |= _BV(LED);
    while(1)
    {
       wykonaj_pomiar();
        LED_PORT |= _BV(LED);
        int x = val();
        delay(x-1);
        LED_PORT &= ~_BV(LED);
        delay(7-x);
        printf("Odczytano: %d %d %d\r\n", x-1, 8-x, ADC);
    }
}


