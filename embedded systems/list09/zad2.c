#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <math.h>
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
int tab_size = 250;
int tab[250];
void zamien()
{
    for(int i=0;i<tab_size;i++)
    {
        tab[i] -= 512;
    }
}
int oblicz_srednia()
{
    zamien();
    long double sum = 0;
    for(int i=0;i<tab_size;i++)
        sum += tab[i] * tab[i];
    sum /= (long double)tab_size;
    return sqrt((double)sum);
}
double conv(double x)
{
    return 20.0*log10(x/350.0);
}

void timer2_init()
{
    // preskaler 1024
    // top = 255
    // czestotliwosc = 16e6/(1024*2*256) = 30 Hz
    TCCR2B = _BV(CS21); // | _BV(CS20) | _BV(CS22);
    // odmaskowanie przerwania przepełnienia licznika
    TIMSK2 |= _BV(TOIE2);
    ADCSRA |= _BV(ADIE);
}
void adc_init()
{
    ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
    DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
    // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    //ADMUX |=0b00001110;
    ADCSRA |= _BV(ADEN); // włącz ADC
}

// procedura obsługi przerwania przepełnienia licznika
ISR(TIMER2_OVF_vect)
{
    ADCSRA |= _BV(ADSC); // wykonaj konwersję
}

volatile int ct =0;
volatile double wyn = 0;
ISR(ADC_vect)
{
    tab[ct++] = ADC;
    if(ct == tab_size)
    {
        ct = 0;
        wyn = conv(oblicz_srednia());
    }
}
int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // uruchom licznik
    adc_init();
    timer2_init();
    sei();
    while(1)
    {
        printf("%f\r\n",wyn);
        _delay_ms(500);
    }
}
