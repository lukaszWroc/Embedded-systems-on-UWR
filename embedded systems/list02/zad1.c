#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>


#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB4
#define BTN_PIN PINB
#define BTN_PORT PORTB


int8_t tab[100];
int main()
{
    BTN_PORT |= _BV(BTN);
    LED_DDR |= _BV(LED);
    int8_t t=0;
    while(1)
    {
        if(t==100)
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
        _delay_ms(10);
    }
}

