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

// inicjalizacja ADC
void adc_init()
{
    ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
    DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
    // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    //ADMUX |=0b00001110;
    DCSRA |= _BV(ADEN); // włącz ADC
    ADCSRA |= _BV(ADIE);
}

FILE uart_file;
volatile int state =0,vx=0,vy=0,va=0;
ISR(ADC_vect)
{
    if(state == 0)
    {
        va = ADC;
        ADMUX   = _BV(REFS0) | 0b00000001;
        state = 1;
        return;
    }
    if(state == 1)
    {
        vx=ADC;
        state = 2;
        return;
    }
    if(state == 2)
    {
        vy=ADC;
        state = 0;
    }
}
ISR(TIMER1_OVF_vect)
{
    if(state!=1)
        return;
    ADCSRA |= _BV(ADSC);
}

ISR(TIMER1_CAPT_vect)
{
    if(state!= 2)
        return;
    ADCSRA |= _BV(ADSC);
}


void timer1_init()
{
    // ustaw tryb licznika
    // COM1A = 10   -- non-inverting mode
    // WGM1  = 1000 -- top=ICR1
    // CS1   = 010  -- prescaler 1024
    // ICR1  = 15624
    // częstotliwość 16e6/(8*(1+ 2*1023)) = 977 Hz
    // wzór: datasheet 20.12.3 str. 164
    ICR1 = 2*1023;
    TCCR1A = _BV(COM1A1) ;
    TCCR1B = _BV(WGM13) | _BV(CS11) ;
    // ustaw pin OC1A (PB1) jako wyjście
    DDRB |= _BV(PB1);
    TIMSK1 |= _BV(ICIE1) | _BV(TOIE1);//ovf and capture interrupts enabled
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
    // uruchom licznik
    timer1_init();
    sei();
    while(1)
    {
        if(state == 0)
        {
            ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
            DIDR0   = _BV(ADC0D);
            ADCSRA |= _BV(ADSC); // wykonaj konwersję
            OCR1A =  va;// wypelnienie(ADC);
            printf("%d %d %d\r\n",va, vx, vy);
            _delay_ms(500);
        }
    }
}

