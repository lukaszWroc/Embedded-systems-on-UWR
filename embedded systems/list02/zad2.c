#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem


#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB4
#define BTN_PIN PINB
#define BTN_PORT PORTB


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
int stan =0;
int tab[10];
int tabCnt = 0;
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

int rowne(int x)
{
    for(int i=0;i<5;i++)
        if(tab[i] != alp[x][i])
            return 0;
    return 1;
}
int znajdz()
{
    int odp = -1;
    for(int i=0;i<26;i++)
    {
        if(rowne(i))
        {
            odp = i;
            break;
        }
    }
    return odp;
}
void wyczysc()
{
    for(int i=0;i<tabCnt;i++)
        tab[i]=0;
    tabCnt = 0;
}
void wypisz()
{
    int a = znajdz();
    printf("%c", (char)(a+'a'));
    wyczysc();
}
void wypisz_spacje()
{
    printf(" ");
    wyczysc();
}
void dodaj()
{
    if(stan == 0)
        tab[tabCnt++] = 1;
    if(stan == 1)
        tab[tabCnt++] = 3;

}
void wyczyscDiody()
{
    LED_PORT &= ~_BV(LED);
}
void pokazStan()
{
    wyczyscDiody();
    if(stan == 1){
         LED_PORT |= _BV(LED);
    }
}
void zmienStan(int cnt)
{
    if( 0 <= cnt && cnt <= 100)
        stan = 0;
    else
        stan = 1;
}
FILE uart_file;
int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    BTN_PORT |= _BV(BTN);
    LED_DDR |= _BV(LED);
    LED_PORT &= ~_BV(LED);

    printf("start\n");
    int cnt=0,cnt2=0,prev=0,prev2=0;
    while(1)
    {
        if(BTN_PIN & _BV(BTN))
        {
            cnt=0;
            cnt2++;
            if(prev == 1)
            {
                prev = 0;
                printf("<%d>", stan);
                dodaj();
            }
        }
        else
        {
            cnt2=0;
            prev2=0;
            prev=1;
            cnt++;
        }
        if(500 <= cnt2 && cnt2 <=700 && prev2 == 0)
        {
            cnt=0;
            wypisz();
            prev2=1;
        }
        if(cnt2 > 700)
        {
            cnt=0;
            cnt2=0;
            prev2=0;
            printf(" ");
        }
        zmienStan(cnt);
        pokazStan();
        _delay_ms(10);
    }
}

