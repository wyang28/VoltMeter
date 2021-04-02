/*
 * Project 4.c
 *
 * Created: 2/24/2021 3:23:58 PM
 * Author : willi
 */ 

#include <stdio.h>
#include <avr/io.h>
#include "avr.h"
#include "lcd.h"

#define DDR    DDRC
#define PORT   PORTC

// Returns 1 if the key at row/col is pressed. Returns 0 otherwise.
int GetSingleButton(int row, int col)
{
	DDRC = 0;
	PORTC = 0;
	SET_BIT(DDRC, row);
	SET_BIT(PORTC, col+4);
	avr_wait(1);
	return GET_BIT(PINC,col + 4) ? 0 : 1;
}

//check keypad if any buttons pressed
int ScanKeypad()
{
	int i, j;
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			if (GetSingleButton(i, j)) {
				return (i * 4) + j + 1;
			}
		}
	}
	return 0;
}

void init_ADC(){
	/*Initialize the ADC*/
	ADMUX |= 0x40;  //0100 0000, enables ADC
	ADCSRA = 0x07; //enable the exit bits
}

int get_sample(){
	/*configure the mux*/
	/*enable converter*/
	/*start conversion*/
	/*see ATMega32 214*/
	
	ADCSRA |= 0xC0; //1100 0000, start conversion
	while(!(ADCSRA & (1 << ADIF) )){}
	ADCSRA |= 0x80;
	int result = ADC;
	return result; //10 bit value. 0-1023
}

int main(void)
{
	/*Init everything*/
	/*
	State a: default
	State b: read max/min/avg
	*/
	avr_init();
	lcd_init();
	init_ADC();
	char state = 'b';
	float min = 5.0;
	float max = 0.0;
	float ct = 0.0;
	float vtot = 0.0;

    while (1) 
    {
		/*Waits 500ms, gets keypad input, clears lcd*/
		avr_wait(500);
		int c = ScanKeypad() -1;
		lcd_clr();
		
		//key #1 pressed: state a, key #2 pressed: state b, otherwise remain in current state
		//state = (c == 0) ? 'a' : ((c == 1) ? 'b' : state);
		if(c == 0){
			if(state != 'a'){
				min = 5.0;
				max = 0.0;
			}
			state = 'a';
		}else if(c == 1){
			state = 'b';
		}
		
		/*Fetch the current voltage*/
		int s;
		
		char buf[17];
		char buf2[17];
		
		s = get_sample();
		float current = (float)(s*5.0/1023);
		//0-5  -->  0-1023
		//2's complement, 6 msb's are 0, 10 lsbs, adlar to shift left all the way to msb
		//1. built in, set admux reg to ompute differential
		//2. a separate get_sample fcn, set addmux to sample from pa1, subtract two samples
		
		float avg;
		
		/*Perform action based on state*/
		if(state == 'a'){
			lcd_pos(0,0);
			sprintf(buf,"Ins:%.2f Avg:---",current);
			lcd_puts(buf);
			
			lcd_pos(1,0);
			sprintf(buf2,"Min:--- Max:---");
			lcd_puts(buf2);
			
			ct = 0;
			vtot = 0;
		}else if(state == 'b'){
			min = current < min ? current : min;
			max = current > max ? current : max;
			
			ct += 1.0;
			vtot += current;
			avg = vtot/ct;
			
			lcd_pos(0,0);
			sprintf(buf,"I:%.2f Av:%.2f",current, avg);
			lcd_puts(buf);
			
			lcd_pos(1,0);
			sprintf(buf2,"Mn:%.2f Mx:%.2f", min, max);
			lcd_puts(buf2);
		}
		
		//display s, min, max (s/1023)*5
    }
}

