#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem


#define myMOSI  PD4
#define myMISO  PD2
#define mySS PD3
#define mySSPORT PORTD
#define mySCK PD5


FILE uart_file;


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
void uart_init()
{
    // ustaw baudrate
    UBRR0 = UBRR_VALUE;
    // wyczysć rejestr UCSR0A
    UCSR0A = 0;
    // włšcz odbiornik i nadajnik
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    // ustaw format 8n1
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
    // skonfiguruj strumienie wejscia/wyjscia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
}

void init()
{
    SPCR |= _BV(SPE) | _BV(SPIE);
    DDRD |= _BV(myMOSI) | _BV(mySCK) | _BV(mySS);
    SREG |= 1;
    DDRB |= _BV(PB4);
}

ISR(SPI_STC_vect)
{
    cli();
    uint8_t tp = SPDR;
    printf("0: %d\r\n", tp);
    SPDR=tp+3;
    sei();
}

uint8_t rev(uint8_t t)
{
	uint8_t tmp = 0;
	for(int i=0;i<8;i++){
		tmp |= (t&1);
        if(i<7)
            tmp <<=1;
        t >>=1;
	}
    return tmp;
}


int main() {
    // zainicjalizuj UART
    uart_init();
    init();
    SPDR=1;
    sei();
    uint8_t cnt=0;
    while(1)
    {
        uint8_t tr=0,ans=0;
        mySSPORT &= ~_BV(mySS);
        tr = rev(cnt);
        for(int i = 0; i < 8; i++)
        {
            if(tr&1)
                PORTD |= _BV(myMOSI);
            else
                PORTD &= ~_BV(myMOSI);


            PORTD |= _BV(mySCK);
            PORTD &= ~_BV(mySCK);
            if(PIND & _BV(myMISO))
                ans |= 1;
            if(i<7)
                ans <<= 1;
            tr>>=1;
        }
        cnt+=2;
        printf("1: %d\r\n", ans);
        mySSPORT |= _BV(mySS);
        _delay_ms(2000);
    }
}
