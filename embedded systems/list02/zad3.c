#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define LED PD0
#define LED_DDR DDRD
#define LED_PORT PORTD

#define BTN1 PB1
#define BTN4 PB4
#define BTN2 PB2
#define BTN_PIN PINB
#define BTN_PORT PORTB


int helper(int x)
{
    return x ^ (x >> 1);
}

int main()
{
   	UCSR0B  &= ~_BV(RXEN0) & ~_BV(TXEN0);
    DDRD = 0xff;
    BTN_PORT |= _BV(BTN1) ;
    BTN_PORT |= _BV(BTN2) ;
    BTN_PORT |= _BV(BTN4);
    LED_PORT=0x00;
    int cnt=0;
    while(1)
    {
        if(!(BTN_PIN  & _BV(BTN2)))
        {
            LED_PORT=0x00;
            cnt=0;
            _delay_ms(300);
        }
        if(!(BTN_PIN & _BV(BTN1)))
        {
            int x;
            cnt++;
            cnt%=16;
            x = helper(cnt);
            LED_PORT=0x00;
            LED_PORT|=x;
            _delay_ms(30);
        }
        if(!(BTN_PIN & _BV(BTN4)))
        {
            int x;
            cnt--;
            cnt+=16;
            cnt%=16;
            x = helper(cnt);
            LED_PORT=0x00;
            LED_PORT|=x;
            _delay_ms(30);
        }
    }
}

