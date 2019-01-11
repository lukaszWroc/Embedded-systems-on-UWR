#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

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
int tab[10] = {0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,
    0b01101101,0b01111101,0b00000111,0b01111111,0b01101111};
int main()
{
    spi_init();
    int ct=0;
    while(1)
    {
        PORTB &= ~_BV(PB1);
        spi_transfer(tab[ct]);
        PORTB |= _BV(PB1);
        ct++;
        if(ct == 10)
            ct=0;
        _delay_ms(1000);
    }
}

