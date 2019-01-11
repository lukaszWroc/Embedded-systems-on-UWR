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

// inicjalizacja ADC
void adc_init()
{
    ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
    DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
    // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    //ADMUX |=0b00001110;
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
void timer1_init()
{
    // ustaw tryb licznika
    // COM1B = 10   -- non-inverting mode
    // WGM1  = 1001 -- PWM Phase TOP = OCR1A
    // CS1   = 001  -- prescaler 1
    // częstotliwość 16e6/(1*2*221) = 38,7skHz
    TCCR1A = _BV(COM1B1) | _BV(WGM10);
    TCCR1B =  _BV(WGM13) | _BV(CS10) | _BV(ICES1);;
    OCR1A=221;
    OCR1B=OCR1A/2;
}

int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // uruchom licznik
    timer1_init();
    int x = ICR1;
    DDRB |= _BV(PB5);
    while(1)
    {
        if (x != ICR1)
        {
            PORTB |= _BV(PB5);
            x = ICR1;
        }
        else
        {
            PORTB &= ~_BV(PB5);
        }
        DDRB |= _BV(PB2);
        _delay_us(600);
        DDRB &= ~_BV(PB2);
        _delay_us(400);
    }
}
