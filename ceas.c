#include <avr/delay.h>
#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "printf.h"
#include <SimpleDHT.h>
 
void init(void);

/*Global Variables Declarations*/
unsigned char hours = 21;
unsigned char minutes = 43;
unsigned char seconds = 0;
volatile int8_t delta;
int pinDHT11 = 7;
uint8_t temperature = 0;
uint8_t humidity = 0;
uint8_t a[50];

void LCD_print(int h, int m, int s);
void LCD_update_time(void);
int8_t enc_delta(void);
 
/*Timer Counter 1 Compare Match A Interrupt Service Routine/Interrupt Handler*/
ISR(TIMER1_COMPA_vect);
 
#define LCD_DATA_PORT	PORTB
#define LCD_DATA_DDR	DDRB
#define LCD_DATA_PIN	PINB
 
#define LCD_CNTRL_PORT	PORTC
#define LCD_CNTRL_DDR	DDRC
#define LCD_CNTRL_PIN	PINC
 
#define LCD_RS_PIN		5
#define LCD_RW_PIN		6
#define LCD_ENABLE_PIN		7
#define SET_HOUR		3
#define SET_MINUTE		4


#define STEP_DELAY_MS 5
#define MIN_STEP    2    /* > 0 */
#define MAX_STEP  255
 
int main(void)
{
 	init();
	init_lcd();
	_delay_ms(1000); 
	

	uint8_t cnt = MAX_STEP/2;
	uint8_t i;
	int16_t res;
 
	while(1)
    {
	for (i=cnt; i > 0; --i) {
		   _delay_ms(STEP_DELAY_MS);
		   res = cnt + enc_delta();
		   if (res > MAX_STEP) {
			   cnt = MAX_STEP;
		   } else if (res < MIN_STEP) {
			   cnt = MIN_STEP;
		   } else {
			   cnt = res;
		   }
		}
		PINB |= _BV(PINB7);   /* toggle LED */
	
  	if (simple_dht11_read(pinDHT11, &temperature, &humidity,a))
		tfp_printf("%s", "Read DHT11 failed.");

	if((int)temperature <=22)
	{
   		PORTD  |=  _BV(PD5);
		PORTD &= ~_BV(PD4);
		PORTD &= ~_BV(PD6);
	}else if(((int)temperature >=23)  && ((int)temperature <= 25) )
	{
	  	PORTD  |=  _BV(PD4);
		PORTD &= ~_BV(PD5);
		PORTD &= ~_BV(PD6);
	}else if((int)temperature >25)
	{
	   	PORTD  |=  _BV(PD6);
		PORTD &= ~_BV(PD4);
		PORTD &= ~_BV(PD5);
	}else{
		PORTD &= ~_BV(PD6);
		PORTD &= ~_BV(PD4);
		PORTD &= ~_BV(PD5);
	}
		
	_delay_ms(1000);
}

}
 
 
void LCD_print(int h, int m, int s)
{
	clear_screen();
   	 display_color(DARK_CYAN, BLACK);
	tfp_printf("\n\n\n  ");
	display_font(40,32);
	tfp_printf("%d:%d:%d\n",h,m,s);
	display_font(18,24);
  	tfp_printf("\n\n   Temp:%dC\n", (int)temperature);
  	tfp_printf("\n   Humid:%d%s", (int)humidity,"%");			
	
}

/*Timer Counter 1 Compare Match A Interrupt Service Routine/Interrupt Handler*/
ISR(TIMER1_COMPA_vect)
{		
	seconds++;
 
	if(seconds == 60)
	{
		seconds = 0;
		minutes++;
	}
	if(minutes == 60)
	{
		minutes = 0;
		hours++;		
	}
	if(hours > 23)
		hours = 0;

	LCD_print(hours, minutes, seconds);
}

ISR( TIMER0_COMPA_vect ) {
     static int8_t last;
     int8_t new, diff;
     uint8_t wheel;

//	tfp_printf("%s \n","rotary inter");	
     /*
        Scan rotary encoder
        ===================
        This is adapted from Peter Dannegger's code available at:
        http://www.mikrocontroller.net/articles/Drehgeber
     */

     wheel = PINE;
     new = 0;
     if( wheel  & _BV(PE4) ) new = 3;
     if( wheel  & _BV(PE5) )
	 new ^= 1;		        	/* convert gray to binary */
     diff = last - new;			/* difference last - new  */
     if( diff & 1 ){			/* bit 0 = value (1) */
	     last = new;		       	/* store new as next last  */
	     delta += (diff & 2) - 1;	/* bit 1 = direction (+/-) */
     }

}

void init(void) {
    /* 8MHz clock, no prescaling (DS, p. 48) */
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

	TCCR1B = (1<<CS12|1<<WGM12);
	OCR1A = 31250-1;
	TIMSK1 = 1<<OCIE1A;

 	/* Configure I/O Ports */

	DDRB  |=  _BV(PB7);   /* LED pin out */
	PORTB &= ~_BV(PB7);   /* LED off */  //laret bl comment: shift with 0 to ensure that the led is off
	DDRD  |=  _BV(PD4);
    	DDRD  |=  _BV(PD5);
    		DDRD  |=  _BV(PD6);

	DDRE &=~ (1 << PE4);		/* PE4 and PE5 as input */
	DDRE &=~ (1 << PE5);
 	PORTE |= (1 << PE4)|(1 << PE5);   /* PE4 and PE5 pull-up enabled   */

	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS01)
          | _BV(CS00);   /* F_CPU / 64 */


    /* SET OCR0A FOR A 1 MS PERIOD */

	OCR0A  =  F_CPU/(1000*2*64.0)-1;    //125;
    /* ENABLE TIMER INTERRUPT */
	TIMSK0 =  _BV(OCIE0A);

	sei();

}

/* read two step encoder */
int8_t enc_delta() {
    int8_t val;

    cli();
    val = delta;
    delta &= 1;
    sei();

    return val >> 1;
}

