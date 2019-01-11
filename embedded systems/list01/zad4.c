#include <avr/io.h>
#include <util/delay.h>

#define LED PD0
#define LED_DDR DDRD
#define LED_PORT PORTD

int32_t tab[10] =
{
    0b10111110,
    0b00001100,
    0b11010110,
    0b01011110,
    0b01101100,
    0b01111010,
    0b11111010,
    0b00001110,
    0b11111110,
    0b01111110
};

void loop()
{
    for(int j=0;j<10;j++)
    {
        PORTD = 0xff;
        LED_PORT |= tab[j];
        _delay_ms(500);
        LED_PORT &= ~tab[j];
        _delay_ms(500);

    }


}


int main()
{
    UCSR0B  &= ~_BV(RXEN0) & ~_BV(TXEN0);
    PORTD = 0xff;
    DDRD = 0xff;

    while(1)
        loop();

}


