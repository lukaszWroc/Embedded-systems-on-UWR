#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>

#define BAUD 57600                         // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB4
#define BTN_PIN PINB
#define BTN_PORT PORTB

// inicjalizacja UART
void uart_init()
{
    // ustaw baudrate
    UBRR0 = UBRR_VALUE;
    // wyczyść rejestr UCSR0A
    UCSR0A = 0;
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

// oczekiwanie na zakończenie transmisji
void uart_wait()
{
    while(!(UCSR0A & _BV(TXC0)));
}

FILE uart_file;

// inicjalizacja licznika 2
void timer2_init()
{
    // preskaler 1024
    // top = 255
    // czestotliwosc = 16e6/(1024*256) = 60 Hz
    // preskaler 1024
    TCCR2B = _BV(CS20) | _BV(CS21) | _BV(CS22);
    // odmaskowanie przerwania przepełnienia licznika
    TIMSK2 |= _BV(TOIE2);
}

// procedura obsługi przerwania przepełnienia licznika
volatile int t=0;
volatile int8_t tab[60];
ISR(TIMER2_OVF_vect)
{
    if(t==60)
        t=0;
    if(tab[t] == 1)
        LED_PORT &= ~_BV(LED);
    else
        LED_PORT |= _BV(LED);
    if(BTN_PIN & _BV(BTN))
        tab[t] = 1;
    else
        tab[t] = 0;
    t++;
}

int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj licznik 2
    timer2_init();
    // ustaw tryb uśpienia na tryb bezczynności
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    // odmaskuj przerwania
    sei();
    BTN_PORT |= _BV(BTN);
    LED_DDR |= _BV(LED);
    // program testowy
    while(1)
    {
        sleep_mode();
    }
}
