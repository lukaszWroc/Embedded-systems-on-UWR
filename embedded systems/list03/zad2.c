#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem


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

int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj ADC
    adc_init();
    // mierz napięcia
    LED_DDR |= _BV(LED);
    int prev = 0;
    while(1)
    {
        if(prev == 0)
            LED_PORT |= _BV(LED);
        _delay_ms(10);
        ADCSRA |= _BV(ADSC); // wykonaj konwersję
        prev +=1;
        prev %=2;
        while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
        if(prev == 1)
            LED_PORT &= ~_BV(LED);
        ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
        float a = ADC;
        float v = (1.1 * 1023.0)/a; // weź zmierzoną wartość (0..1023)
        printf("Odczytano: %.10f %d\r\n", v, prev);
        _delay_ms(1000);
    }
}

