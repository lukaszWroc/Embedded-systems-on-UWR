#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

char bt[256], br[256];
volatile uint8_t tp=0,tk=0,rp=0,rk=0;
volatile int tpc=0,tkc=0,rpc=0,rkc=0;

// inicjalizacja UART
void uart_init() {
    // ustaw baudrate
    UBRR0 = UBRR_VALUE;
    // włącz odbiornik i nadajnik
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    // ustaw format 8n1
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}
volatile char c;

int btFull()
{
  if(tp == tk && tpc != tkc)
    return 1;
  return 0;
}
int uart_transmit(char data, FILE *stream)
{

    while(btFull());
    cli();
    bt[tp++] = data;
	tpc++;
	if(tkc < tpc){
		tpc -= tkc;
		tkc = 0;
	}
	else
    {
        tkc -= tpc;
		tpc = 0;
    }
    if(tp ==256)
        tp = 0;
    sei();
    return 0;
}

int brEmpty()
{
    if(rpc == rkc)
        return 1;
    return 0;
}


int uart_receive(FILE *stream)
{
    while(brEmpty());
    cli();
	c = br[rk++];
	rkc++;
	if(rkc < rpc){
		rpc -= rkc;
		rkc = 0;
	}
	else
    {
        rkc -= rpc;
		rpc = 0;
    }
    if(rk ==256)
        rk = 0;
    sei();
	return c;
}
FILE uart_file;


ISR(USART_RX_vect)
{
    br[rp++] = UDR0;
	rpc++;
	if(rp == 256)
        rp = 0;
}

ISR(USART_UDRE_vect){

    if(tkc <tpc)
    {
        UDR0=bt[tk++];
        tkc++;
        if(tk == 256)
            tk = 0;
    }
}

ISR(USART_TX_vect){

}

int main()
{
  // zainicjalizuj UART
    uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
  // program testowy
    UCSR0B |= _BV(RXCIE0);
    UCSR0B |= _BV(TXCIE0);
    UCSR0B |= _BV(UDRIE0);
    SREG |= 0b00000001;
    sei();
    printf("hello world!\r\n");
    while(1)
    {

        int a = 1;
        scanf("%d", &a);
        printf("Wczytano: %d\r\n", a);
    }
}

