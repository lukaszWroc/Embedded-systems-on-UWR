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
    UCSR0A = 0;
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
int alp[26][5] =
{
    {1,3},  //a
    {3,1,1,1},  // b
    {3,1,3,1}, //c
    {3,1,1},  //d
    {1},  // e
    {1,1,3,1},  // f
    {3,3,1},  //g
    {1,1,1,1},//h
    {1,1},//i
    {1,3,3,3},//j
    {3,1,3},//k
    {1,3,1,1},//l
    {3,3},//m
    {3,1},//n
    {3,3,3},//o
    {1,3,3,1},//p
    {3,3,1,3},//q
    {1,3,1},//r
    {1,1,1},//s
    {3},//t
    {1,1,3},//u
    {1,1,1,3},//v
    {1,3,3},//w
    {3,1,1,3},//x
    {3,1,3,3},//y
    {3,3,1,1}//z

};
char tab[100];

void display(int k)
{
    int ct=0;

    while(alp[k][ct] != 0)
    {
        int tmp = alp[k][ct++];
        LED_PORT |= _BV(LED);
        if(tmp == 1)
            _delay_ms(1000);
        else
            _delay_ms(3*1000);
        LED_PORT &= ~_BV(LED);
        _delay_ms(1000);
    }
    _delay_ms(2*1000);
}
void translate(int t)
{
    for(int i=0;i<t;i++)
    {
        int k = tab[i] - 'a';
	    printf("Odczytano: %"PRId16"\r\n", k);
        if('a' <= tab[i] && tab[i] <= 'z')
            display(k);
        else
            _delay_ms(7*1000);
    }

}
FILE uart_file;
int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    printf("Hello world!\r\n");
    LED_DDR |= _BV(LED);
    PORTD = 0x00;
    DDRD = 0xff;
    int cnt=0;
    while(1)
    {
        char c;
        int16_t znak;
        c = getchar();
        znak = (int16_t)c;
	    printf("Odczytano: %"PRId16"\r\n", c);
        if(znak == 13)
        {
            translate(cnt);
            cnt=0;
        }
        else
            tab[cnt++] = c;
    }
}


