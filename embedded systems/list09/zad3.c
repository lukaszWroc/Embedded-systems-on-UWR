#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include<math.h>
#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem


// inicjalizacja UART
void uart_init()
{
    // ustaw baudrate
    UBRR0 = UBRR_VALUE;
    // włącz odbiornik i nadajnik
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

void timer1_init()
{
    // ustaw tryb licznika
    // COM1B = 10   -- non-inverting mode
    // WGM1  = 1001 -- PWM Phase TOP = OCR1A
    // CS1   = 001  -- prescaler 1
    // częstotliwość 16e6/(1*2*221) = 38,7kHz
    // wzór: datasheet 20.12.3 str. 164
    TCCR1A = _BV(COM1B1) | _BV(WGM10);
    TCCR1B =  _BV(WGM13) | _BV(CS10)  | _BV(ICES1);;
    OCR1A=221;
    OCR1B=OCR1A/2;
    TCCR0A = _BV(COM0A1) | _BV(COM0A0)  | _BV(WGM00) ;
    TCCR0B = _BV(CS02);
    TIMSK1 |= _BV(ICIE1);
    DDRD |= _BV(PD6);
    DDRB |= _BV(PB1) |_BV(PB2) ;
}
void timer2_init()
{
    // preskaler 1024
    // top = 255
    // czestotliwosc = 16e6/(1024*2*256) = 30 Hz
        TCCR2B = _BV(CS20) | _BV(CS21) | _BV(CS22);
    // odmaskowanie przerwania przepełnienia licznika
        TIMSK2 |= _BV(TOIE2);
}

volatile int32_t ovf=0,t=0;
volatile int fre=0;
// procedura obsługi przerwania przepełnienia licznika
ISR(TIMER2_OVF_vect)
{
    if(t==60)
    {
        t=0;
        fre = ovf;
        ovf=0;
    }
    t++;
}

ISR(TIMER1_CAPT_vect)
{
    ovf++;
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
    timer2_init();
    sei();
    while(1)
    {
       printf("%d\r\n", fre);
       _delay_ms(1000);
    }
}
