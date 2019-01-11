#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include<math.h>
#include <stdlib.h>
#include <time.h>
#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem


int R,G,B;
int64_t H,S=100,V=100;

void HSVToRGB() {
    int64_t rm=1,gm=1,bm=1;
    int64_t r,g,b;
	if (S == 0)
	{
		r = V;
		g = V;
		b = V;
	}
	else
	{
		int64_t i;
		int64_t f, p, q, t;
        f = (H/6)%100;
		if (H == 36000)
			H = 0;
		else
			H = H / 6000;
		i = H;

		p = V * (100 - S);
		q = V * (10000 - (S * f));
		t = V * (10000 - (S * (100 - f)));
		switch (i)
		{
		case 0:
			r = V;
			g = t;
			b = p;
			rm=100;
			gm=1000000;
			bm=10000;
			break;
		case 1:
			r = q;
			g = V;
			b = p;
			rm=1000000;
			gm=100;
			bm=10000;
			break;
		case 2:
			r = p;
			g = V;
			b = t;
			rm=10000;
			gm=100;
			bm=1000000;
			break;
		case 3:
			r = p;
			g = q;
			b = V;
			rm=10000;
			gm=1000000;
			bm=100;
			break;
		case 4:
			r = t;
			g = p;
			b = V;
			rm=1000000;
			gm=10000;
			bm=100;
			break;

		default:
			r = V;
			g = p;
			b = q;
			rm=100;
			gm=10000;
			bm=1000000;
			break;
		}

	}
	r*=256;
	g*=256;
	b*=256;
    r/=rm;
    g/=gm;
    b/=bm;
    R=r;
    G=g;
    B=b;
}


// inicjalizacja UART
void uart_init()
{
    // ustaw baudrate
    UBRR0 = UBRR_VALUE;
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

// inicjalizacja ADC
void adc_init()
{
    ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
    DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
    // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    //ADMUX |=0b00001110;
    ADCSRA |= _BV(ADEN); // włącz ADC
}

FILE uart_file;
void wykonaj_pomiar()
{
    ADCSRA |= _BV(ADSC); // wykonaj konwersję
    while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
    ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
}

void delay(int x)
{
    for(int i=0;i<x;i++)
        _delay_ms(1);

}
int max(int x,int y)
{
    if(x<y)
        return y;
    return x;
}
int min(int x,int y)
{
    if(x>y)
        return y;
    return x;
}
static int tab[] =
{
    0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,
    1,1,1,1,1,1,2,2,
    2,2,2,2,4,4,4,4,
    7,7,7,7,8,8,8,8,
    9,9,9,9,10,10,
    10,10,11,11,12,12,13,14,
    18,20,22,24,26,28,30,30,
    32,32,35,37,38,39,40,41,
    42,44,46,48,
    50,52,55,58,61,64,67,70,
    73,76,79,82,83,87,90,93,
    96,99,102,105,108,111,112,116,
    120,124,128,132,136,140,144,148,
    152,156,160,165,170,185,190,195,
    200,204,208,212,216,220,224,228,
    232,236,240,245,250,255
};

void timer1_init()
{
    // ustaw tryb licznika
    // COM1A = 10   -- non-inverting mode
    // WGM1  = 1110 -- fast PWM top=ICR1
    // CS1   = 101  -- prescaler 1024
    // ICR1  = 15624
    // częstotliwość 16e6/(1024*(1+15624)) = 1 Hz
    // wzór: datasheet 20.12.3 str. 164
    ICR1 = 256;
    TCCR1A = _BV(COM1A1) | _BV(COM1A0) |_BV(COM1B1) | _BV(COM1B0)  | _BV(WGM11) ;
    TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS12);
    TCCR0A = _BV(COM0A1) | _BV(COM0A0)  | _BV(WGM00) ;
    TCCR0B = _BV(CS02);
    DDRD |= _BV(PD6);
    DDRB |= _BV(PB1) |_BV(PB2) ;
}

int main()
{
    srand(time(NULL));
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj ADC
    adc_init();
    // uruchom licznik
    timer1_init();

    int64_t rn;
    printf("start");
    int sr,sg,sb;
    while(1)
    {
        rn = rand();
        H = rn;
        HSVToRGB();
        printf(" %d %d %d\r\n",R,G,B);
        OCR1A = R;
        OCR1B = G;
        OCR0A = B;
        sr=R;sg=G;sb=B;
        for(int i=255;i>=0;i--)
        {
            R = min(sr,tab[i/2]);
            G = min(sg,tab[i/2]);
            B = min(sb,tab[i/2]);
            OCR1A = R;
            OCR1B = G;
            OCR0A = B;
            delay(10);
            printf(" %d %d %d\r\n",R,G,B);
        }
        delay(200);
        for(int i=0;i<256;i++)
        {
            R = min(sr,tab[i/2]);
            G = min(sg,tab[i/2]);
            B = min(sb,tab[i/2]);
            OCR1A = R;
            OCR1B = G;
            OCR0A = B;
            delay(10);
        }
        delay(3000);
    }
}
