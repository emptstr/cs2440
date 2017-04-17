/*
 * StateMachine.c
 *
 * Created: 2/19/2017 1:48:34 PM
 * Author : Matt
 */

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

void blink(int ms);
int button1Pressed();
int button2Pressed();
int buttonPressed(int* button);
void flashing(int* button);
void rotate(int* button);
void partialDelay(int* button, int delay);
void delayCheck(int* button, int delay);

int state[3][2] = {{1, 2}, {0, 2}, {1, 0}};
static int currState = 0;
static int shouldBreak = 0;

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

void partialDelay(int* button, int delay)
{
	int pressed = 0;
	for(int i = 0; i < delay; i++)
	{
		if((!(PINF & 0x80) || !(PINF & 0x40)) && !pressed)
		{
			pressed = 1;
		}
		if(pressed)
		{
			if(PINF & 0x80)
			{
				*button = 0;
				shouldBreak = 1;
				return;
			}
			if(PINF & 0x40)
			{
				*button = 1;
				shouldBreak = 1;
				return;
			}
		}
	}
}

void delayCheck(int* button, int delay)
{
	int i = 0;
	for(i = 0; i < delay; i++)
	{
		partialDelay(button, delay);
	}
}
