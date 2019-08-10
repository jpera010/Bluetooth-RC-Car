/* Jose Peralta, jpera010@ucr.edu 
/*
/*This is my Bluetooth RC Car Project Code 
*/
#define per 1
#define tasksNum 1
#define F_CPU 8000000UL // 8mhz microcontroller
#define ECHO 3 // input to listen for echo uses -INT0- External Interrupt Request 0.
#define TRIGGER 3 // output triggers sensor
#define CRASH 5 // output pin for when an object is detected

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "scheduler.h"
#include "usart_ATmega1284.h"

volatile unsigned short dist;
volatile unsigned short dist1;
volatile unsigned short dist2;
volatile unsigned short dist3;

volatile char halt = 0; 

void Enabledist() {
	SREG |= 0x80; // enable global interrupts

	EICRA |= (1 << ISC10); // set interrupt to trigger on logic change
	EICRA &= ~(1 << ISC11); // set interrupt to trigger on logic change
	
	EIMSK |= (1 << INT1); // enable external interrupt 1 (PD3)
	
	// set sensor trigger pin as output
	DDRA |= (1 << TRIGGER);
	PORTA &= ~(1 << TRIGGER);
	// set sensor echo pin as input, enable pull-up
	DDRD &= ~(1 << ECHO);
	PORTD |= (1 << ECHO);
	// set sensor output pin as output
	DDRA |= (1 << CRASH);
	PORTA &= ~(1 << CRASH);
}

// Returns the dist in centimeters
unsigned short distCM() {
	PORTA |= (1 << TRIGGER); // set trigger pin high
	_delay_us(10);
	PORTA &= ~(1 << TRIGGER); // set trigger pin low
	return sonar/58; // 2 (pulse travels there and back) * 10000cm (max range of pulse) / 343 (speed of sound) = 58
}


// Test harness for Ultrasonic Sensors
enum State_Machine {S1};
int ticky(int state)
{
	switch(state)
	{
		case S1:
			if(halt == 1)
			{
				PORTB = 0x80;
			}
			else { PORTB = 0x00;}
			state = S1;
			break;
		default:
			state = S1;
			break;
	}
	return state;
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00; // Output for trigger pin
	DDRB = 0xFF; PORTB = 0x00; // Output to motors and lights
	DDRD = 0xFF; PORTD = 0x00; // Output to motors
	PORTC = 0x00; DDRC = 0xFF; //output config
	int var;
	initUSART(0);
	
	// Enables dist measurement
	SREG |= 0x80; // Enable global interrupts
	// Sets interrupt to trigger on logic changes
	EICRA |= (1 << ISC10);
	EICRA &= ~(1 << ISC11);
	// Enables external interrupt 1 (PD3)
	EIMSK |= (1 << INT1);
	// Sets sensor trigger in PORTA as output
	DDRA |= (1 << TRIGGER);
	PORTA &= ~(1 << TRIGGER);
	// Sets sensor echo as input in PORTD, enable pull-up
	DDRD &= ~(1 << ECHO);
	PORTD |= (1 << ECHO);
	// Sets OBJECT DETECTED pin as output in PORTA
	DDRA |= (1 << CRASH);
	PORTA &= ~(1 << CRASH);
	// End of dist measurement enabling code
	
	Enabledist();
	TimerSet(per);
	TimerOn();
	
	unsigned char i = 0;
	tasks[i].state = -1;
	tasks[i].period = 50; // Should be .01ms instead of 1 for optimal performance
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &ticky;
	
	while (1)
	{ 
		// get the averavge of 3 distances to make the measurement more accurate 
		dist1 = distCM();
		dist2 = distCM();
		dist3 = distCM();
		
		dist = (dist1 + dist2 + dist3) / 3;
		if(dist <= 50) { halt = 1;}
		else { halt = 0;}	
			
		var = USART_Receive(0);
		
		if(var == 'L') { // left 
			USART_Flush(0);
			PORTC = 0x01;
		}
		else if (var == 'R') { // right
			USART_Flush(0);
			PORTC = 0x02;
		}
		else if (var == 'F') { // forward 
			USART_Flush(0);
			if(halt == 1) {
				PORTC = 0x00;	
			}
			else {
				PORTC = 0x04;
			}
		}
		else if (var == 'B') { // back 
			USART_Flush(0);
			PORTC = 0x08;
		}
		else if (var == 'H') { //back left
			USART_Flush(0);
			PORTC = 0x09;
		}
		else if (var == 'G') { //forward left
			USART_Flush(0);
			if(halt == 1) {
				PORTC = 0x00;	
			}
			else {
				PORTC = 0x05;
			}
		}
		else if (var == 'I') { //foward right
			USART_Flush(0);
			if(halt == 1) {
				PORTC = 0x00;	
			}
			else {
				PORTC = 0x06;
			}
		}
		else if (var == 'J') { //back right
			USART_Flush(0);
			PORTC = 0x0A;
		}
		else{
			USART_Flush(0);
			PORTC = 0x00;	
		}		 	
	}
}