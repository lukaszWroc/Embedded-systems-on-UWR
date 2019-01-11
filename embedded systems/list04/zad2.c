#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
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
  // COM1A = 10   -- non-inverting mode
  // WGM1  = 1110 -- fast PWM top=ICR1
  // CS1   = 101  -- prescaler 1024
  // ICR1  = 15624
  // częstotliwość 16e6/(1024*(1+15624)) = 1 Hz
  // wzór: datasheet 20.12.3 str. 164
  ICR1=1024;
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10);// | _BV(CS12);
  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(PB1);
}

int wypelnienie()
{
    if(ADC > 1100)
        return 1;
    if(ADC > 800)
        return 2;
    if(ADC > 530)
        return 3;
    if(ADC > 256)
        return 4;
    if(ADC > 128)
        return 5;
    if(ADC > 65)
        return 5;
    if(ADC > 35)
        return 7;
    if(ADC > 17)
        return 8;
     if(ADC > 8)
        return 9;
    return 10;
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

    while(1)
    {
        ADCSRA |= _BV(ADSC); // wykonaj konwersję
        while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
        ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
        OCR1A = ICR1 - ADC;// wypelnienie(ADC);
        printf("%d\r\n",ADC);
    }
}

