#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>

#include <stdlib.h>
#include<math.h>
#define BAUD 9600
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)

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
float readInput()
{
    char c,in[30];
    for(int i=0;i<30;i++)
        in[i]=0;
    int t=0;
    do
    {
        c=getchar();
        in[t++]=c;
    } while(!('0'<= c && c <='9'));
    while(1)
    {
        c=getchar();
        if(c=='\n' || (int)c == 13)
            break;
        in[t++]=c;
    }
    float a;
    a = atof(in);
    return a;
}
FILE uart_file;

int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    printf("Hello world!\n");
    printf("1 int8_t\n 2 int16_t\n 3 int32_t\n 4 float\n");

    while(1)
    {
        int16_t x = 1;
        scanf("%"SCNd16, &x);
        printf("Odczytano: %"PRId16"\n", x);
        if(x == 1)
        {
            int8_t a = 1, b = 1;
            scanf("%"SCNd8, &a);
            scanf("%"SCNd8, &b);
            printf("Odczytano: %"PRId8"\n", a+b);
            printf("Odczytano: %"PRId8"\n", a*b);
            printf("Odczytano: %"PRId8"\n", a/b);
        }
        if(x == 2)
        {
            int16_t a = 1, b = 1;
            scanf("%"SCNd16, &a);
            scanf("%"SCNd16, &b);
            printf("Odczytano: %"PRId16"\n", a+b);
            printf("Odczytano: %"PRId16"\n", a*b);
            printf("Odczytano: %"PRId16"\n", a/b);

        }
        if(x == 3)
        {
            int32_t a = 1, b = 1;
            scanf("%"SCNd32, &a);
            scanf("%"SCNd32, &b);
            printf("Odczytano: %"PRId32"\n", a+b);
            printf("Odczytano: %"PRId32"\n", a*b);
            printf("Odczytano: %"PRId32"\n", a/b);
        }

        if(x == 4)
        {
            float a,b;
            a = readInput();
            printf("a%f\r\n",a);
            b = readInput();
            printf("b%f\r\n",b);
            printf("Odczytano: %f\n",a+b);
            printf("Odczytano: %f\n",a*b);
            printf("Odczytano: %f\n",a/b);

        }
    }
}
