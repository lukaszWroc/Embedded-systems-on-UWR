#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#define BAUD 9600                          // baudrate
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

void timer1_init()
{
    // ustaw tryb licznika
    // WGM1  = 0000 -- normal
    // CS1   = 001  -- prescaler 1
    TCCR1B = _BV(CS10);
}
FILE uart_file;

int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj licznik
    timer1_init();
    // program testowy
    printf("start\r\n");
    int16_t c;
    while(1)
    {
        scanf("%"SCNd16,&c);
        if(c == 0)
        {
            uint16_t x1,x2,x3;
            volatile int8_t x=10;
            uint16_t a = TCNT1; // wartość licznika przed czekaniem
            x=x*x;
            uint16_t b = TCNT1; // wartość licznika po czekaniu
            x1 = b-a;
            a=TCNT1=0;
            x=x/x;
            b=TCNT1;
            x2=b-a;
            a=TCNT1=0;
            x=x+x;
            b=TCNT1;
            x3=b-a;
            printf("Zmierzony czas4: + %"PRIu16" * %"PRIu16" / %"PRIu16" cykli\r\n", x3,x1,x2);
        }
        if(c == 1)
        {
            uint16_t x1,x2,x3;
            volatile int16_t x=10;
            uint16_t a = TCNT1; // wartość licznika przed czekaniem
            x=x*x;
            uint16_t b = TCNT1; // wartość licznika po czekaniu
            x1 = b-a;
             a=TCNT1=0;
            x=x/x;
            b=TCNT1;
            x2=b-a;
             a=TCNT1=0;
            x=x+x;
            b=TCNT1;
            x3=b-a;
            printf("Zmierzony czas4: + %"PRIu16" * %"PRIu16" / %"PRIu16" cykli\r\n", x3,x1,x2);
        }
        if(c == 2)
        {
            uint16_t x1,x2,x3;
            volatile int32_t x=10;
            uint16_t a = TCNT1; // wartość licznika przed czekaniem
            x=x*x;
            uint16_t b = TCNT1; // wartość licznika po czekaniu
            x1 = b-a;
             a=TCNT1=0;
            x=x/x;
            b=TCNT1;
            x2=b-a;
             a=TCNT1=0;
            x=x+x;
            b=TCNT1;
            x3=b-a;
            printf("Zmierzony czas4: + %"PRIu16" * %"PRIu16" / %"PRIu16" cykli\r\n", x3,x1,x2);
        }

        if(c == 3)
        {
            uint16_t x1,x2,x3;
            volatile int64_t x=10;
            uint16_t a = TCNT1; // wartość licznika przed czekaniem
            x=x*x;
            uint16_t b = TCNT1; // wartość licznika po czekaniu
            x1 = b-a;
             a=TCNT1=0;
            x=x/x;
            b=TCNT1;
            x2=b-a;
             a=TCNT1=0;
            x=x+x;
            b=TCNT1;
            x3=b-a;
            printf("Zmierzony czas4: + %"PRIu16" * %"PRIu16" / %"PRIu16" cykli\r\n", x3,x1,x2);
        }
        if(c == 4)
        {
            uint16_t x1,x2,x3;
            volatile float x=10;
            uint16_t a = TCNT1; // wartość licznika przed czekaniem
            x=x*x;
            uint16_t b = TCNT1; // wartość licznika po czekaniu
            x1 = b-a;
            a=TCNT1=0;
            x=x/x;
            b=TCNT1;
            x2=b-a;
            a=TCNT1=0;
            x=x+x;
            b=TCNT1;
            x3=b-a;
            printf("Zmierzony czas4: + %"PRIu16" * %"PRIu16" / %"PRIu16" cykli\r\n", x3,x1,x2);
        }
        _delay_ms(1000);
    }
}

