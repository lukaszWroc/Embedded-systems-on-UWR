#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <inttypes.h>
#include <stdlib.h>

#define BAUD 9600                        // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

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

FILE uart_file;
volatile char c = 'a';

ISR(USART_RX_vect)
{
	c = UDR0;
	UCSR0A &= ~_BV(RXC0);
	UCSR0B |= _BV(UDRIE0);
	SREG |= 0b00000001;
	UCSR0B &= ~_BV(RXCIE0);
}


ISR(USART_UDRE_vect)
{
	UDR0 = c;
	UCSR0A |= _BV(RXC0);
	UCSR0B &= ~_BV(UDRIE0);
	UCSR0B |= _BV(RXCIE0);
	SREG &= ~0b00000001;
}


int main(void)
{
    uart_init();
 	// skonfiguruj strumienie wejścia/wyjścia
  	fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  	stdin = stdout = stderr = &uart_file;
  	// zainicjalizuj SPI
	set_sleep_mode(SLEEP_MODE_IDLE);
	sei();
	UCSR0B |= (1 << UDRIE0);
	while (1)
	{
		sleep_mode();
	}
}
