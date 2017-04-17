/*
 * StateMachine.c
 *
 * Created: 2/19/2017 1:48:34 PM
 * Author : Matt
 */ 

#include <time.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>


#define Button1Pressed(PIN) !(PIN & 0x80)
#define Button1Released(PIN) (PIN & 0x80)
#define Button2Pressed(PIN) !(PIN & 0x40)
#define Button2Released(PIN) (PIN & 0x40)

void blink(int ms);
int button1Pressed();
int button2Pressed();
int buttonPressed(int* button);
void flashing();
void rotate();
void partialDelay(int* button, int delay);
void delayCheck(int* button, int delay);

int state[3][2] = {{1, 2}, {0, 2}, {1, 0}};
static int currState = 0;
static int shouldBreak = 0;

static clock_t CLOCKS_PER_MILLI = 0;
	
int main(void)
{
    /* Replace with your application code */
	DDRF = 0x3F;
	PORTF = 0xC0;
	int button = 0;
	DDRB = 0x80;
	_delay_ms(1000);
    while (1) 
    {
		int buttonPress = buttonPressed(&button);
		if(shouldBreak || buttonPress)
		{
			currState = state[currState][button];
			shouldBreak = 0;
			if(currState == 0)
			{
				//_delay_ms(500);
			}
			else if(currState == 1)
			{
				//_delay_ms(500);
				rotate(&button);
			}
			else if(currState == 2)
			{
				//_delay_ms(500);
				flashing(&button);
			}
		}
    }
}

int buttonPressed(int* button)
{
	while((!button1Pressed())&&(!button2Pressed()))
	{
		
	}
	if(button1Pressed())
	{
		while(button1Pressed())
		{
		}
		*button = 0;
	}
	else
	{
		while(button2Pressed())
		{
		}
		*button = 1;
	}
	return 1;
}

void blink(int ms)
{
	PORTB ^= 0x80;
	_delay_ms(500);
}

int button1Pressed()
{
	if(!(PINF & 0x80))
	{
		return 1;
	}
	return 0;
}

int button2Pressed()
{
	if(!(PINF & 0x40))
	{
		return 1;
	}
	return 0;
}

void rotate(int *button)
{
	int i = 0;
	while(1)
	{
		for(i = 0; i < 4; i++)
		{
			blink(1);
			PORTF ^= 1 << i;
			//_delay_ms(500);
			delayCheck(button, 470);	
			if(shouldBreak)
			{
				PORTF &= 0xC0;
				return;
			}
			PORTF ^= 1 << i;
		}
	}
}

void flashing(int *button)
{
	int i = 0;
	while(1)
	{
		for(i = 0; i < 4; i++)
		{
			PORTF ^= 1 << i;
		}
		//_delay_ms(200);
		delayCheck(button, 235);
		if(shouldBreak)
		{
			PORTF &= 0xC0;
			return;
		}
		for(i = 0; i < 4; i++)
		{
			PORTF ^= 1 << i;
		}
		_delay_ms(1000);
	}
}

void waitForPress(int* button, int delay)
{
    static long CYCLES_IN_MILLI = 16000;
	static long CYCLES_IN_OUTER = 44;
	static long CYCLES_IN_INNER = 36;
	
	long cycles_to_delay = delay * CYCLES_IN_MILLI; // 31 Cycles
	long num_cycles = 39; //includes the 8 cycles used to initialize this variable and the 31 above
	
	while(num_cycles < cycles_to_delay){//OUTER 2 cycles
		if(Button1Pressed(PINF)){// OUTER 8 cycles
			while (num_cycles < cycles_to_delay) // INNER 2 cycles
			{
				if(Button1Released(PINF)){// INNER 8 cycles
					*button = 0;
					return;
				}
				num_cycles += CYCLES_IN_INNER;//INNER 26 cycles
			}
		}else if(Button2Pressed(PINF)){// OUTER 8 cycles 
			while(num_cycles < cycles_to_delay){// INNER 2 cycles
				if(Button2Released(PINF)){// INNER 8 cycles
					*button = 1;
					return;
				}
				num_cycles += CYCLES_IN_INNER;// INNER 26 cycles
			}
		}
		num_cycles + CYCLES_IN_OUTER;// OUTER 26 cycles
	}
}

void delayCheck(int* button, int delay)
{
		waitForPress(button, delay);
}