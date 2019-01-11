#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
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
    ADMUX |=0b00001110;
    ADCSRA |= _BV(ADEN); // włącz ADC
}


FILE uart_file;
int tab1[100],tab2[100];
volatile int t=0;
ISR(ADC_vect)
{
	if (t < 100)
	{
        tab2[t] = ADC;
        t++;
	}
}
double war(int tab[])
{
    double sum = 0;
    for(int i=20;i<100;i++)
        sum +=tab[i];
    sum/=(double)80;
    double w=0;
    for(int i=20;i<100;i++)
    {
        double t = sum - tab[i];

        w+= t*t;
    }
    return (double)w/(double)80;
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

    printf("wynik:\r\n");

    for(int i=0;i<100;i++)
    {
        ADCSRA |= _BV(ADSC); // wykonaj konwersję
        while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
        ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
        tab1[i] = ADC;
    }
    set_sleep_mode(SLEEP_MODE_ADC);
    sei();
    ADCSRA |= _BV(ADIE);
    for(int i=0;i<100;i++)
    {
       sleep_mode();
    }
    ADCSRA &=  ~_BV(ADIE);

    printf("%f %f\r\n",war(tab1) , war(tab2));

    while(1);
}
