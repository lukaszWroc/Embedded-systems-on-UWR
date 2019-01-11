#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "sample.h"

// inicjalizacja SPI
void spi_init()
{
    // ustaw piny MOSI, SCK i ~SS jako wyjścia
    DDRB |= _BV(DDB3) | _BV(DDB5) | _BV(DDB2);
    // włącz SPI w trybie master z zegarem 250 kHz
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1);
    DDRB |= _BV(PB1) ;
}

// transfer jednego bajtu
uint8_t spi_transfer(uint8_t data)
{
    // rozpocznij transmisję
    SPDR = data;
    // czekaj na ukończenie transmisji
    while (!(SPSR & _BV(SPIF)));
    // wyczyść flagę przerwania
    SPSR |= _BV(SPIF);
    // zwróć otrzymane dane
    return SPDR;
}

int n =  sizeof(sample)/sizeof(sample[0]);
int main() {
    spi_init();
	int i =0;
	int8_t settings = 0b01110000;
    while(1)
    {
        PORTB &= ~_BV(PB2);
        uint8_t x = pgm_read_byte(&sample[i++]);
        spi_transfer(settings + (x >> 4));
        spi_transfer((x << 4));
        PORTB |= _BV(PB2);
        if(i  == n)
            i = 0;
    }
}
