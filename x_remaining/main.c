/*
 * Lab9.c
 *
 * Created: 3/28/2017 1:52:21 PM
 * Author : Matt Spencer and Jordan Gaston
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <stdbool.h>
#include "acx.h"


//---------------------------------------------------
// Exec State Variables 
//---------------------------------------------------
uint16_t disable = 0xFE;     //leave Thread 0 enabled
uint16_t suspend = 0x00;
uint16_t delay = 0x00;

uint8_t x_thread_id = 0; //current thread
uint16_t x_thread_mask = 0x01;  //bit 0 set corresponds to thread 0

int global_timer = 0;

int stackSizes[8] = {STACK_SIZE0,STACK_SIZE1,STACK_SIZE2,STACK_SIZE3,STACK_SIZE4,STACK_SIZE5,STACK_SIZE6,STACK_SIZE7};
int offsets[8] = {STACK_OFFSET0,STACK_OFFSET1,STACK_OFFSET2,STACK_OFFSET3,STACK_OFFSET4,STACK_OFFSET5,STACK_OFFSET6,STACK_OFFSET7};
	
//---------------------------------------------------
// Stack Control
//---------------------------------------------------
STACKCONTROL threads[NUM_THREADS] = {{0}};

//---------------------------------------------------
// Stack Memory
//---------------------------------------------------
byte stackMem[MEM_SIZE] = {0};

//---------------------------------------------------
// Thread Delay Counters
//---------------------------------------------------
volatile uint16_t x_thread_delay[MAX_THREADS] = {0};
	
//---------------------------------------------------
// Local Functions
//---------------------------------------------------
void fillCanaries()
{
	for(int i = 0; i < NUM_THREADS; i++)
	{
		stackMem[offsets[i]] = 0xAA;
	}
}

void initThreads()
{
	//threads[0].pStackBase = &stackMem[STACK_OFFSET0 + STACK_SIZE0 - 1];
	//threads[0].sp = &stackMem[STACK_OFFSET0 + STACK_SIZE0 - STACK_CONTEXT - 1];
	
	int temp = 0;
	for(int i = 0; i < NUM_THREADS; i++)
	{
		temp = offsets[i] + stackSizes[i] - 1;
		threads[i].pStackBase = &stackMem[temp];
		threads[i].sp = &stackMem[temp];
	}
}

void checkCanaries()
{
	for(int i = 0; i < NUM_THREADS; i++)
	{
		if(stackMem[offsets[i]] != 0xAA)
		{
			bufferOverflow();
		}
	}
}

void bufferOverflow()
{
	while(1)
	{
		//Blink in a specific way, for now we know infinite loop means buffer overflow
	}
}

//---------------------------------------------------
// ACX Functions
//---------------------------------------------------
void x_init(void)
{
	cli();

	fillCanaries();
	initThreads();
	
	//cli();
	TCCR0A = 0;     // set entire TCCR0A register to 0
	TCCR0B = 0;     // same for TCCR0B
	// set compare match register to desired timer count:
	OCR0A = 250;

	TCCR0B |= (1 << WGM02);

	// Set CS10 and CS12 bits for 1024 prescaler:
	TCCR0B |= (1 << CS00);
	TCCR0B |= (1 << CS01);
	
	//Set compare on match interrupt
	TIMSK0 |= (1 << OCIE0A);
	
	
	int spasd = 0x21ff - SP;
	changeStack(SP,(byte*)threads[0].pStackBase,spasd);
	SP = threads[0].sp - spasd;

	sei();

	// return to caller.
}

int main(void)
{
	volatile int j = 0;
	x_init();
	x_new(1, testThread1, true);  // create thread, ID=1
	x_new(0, testThread0, true);  // replace current thread
	while(1){
		x_yield();
	}
}
//------------------------
// A test thread
//------------------------
void testThread0(void)
{
	DDRF = 0xC0;
	while(1){
		PORTF ^= 0x80;
		x_delay(1500);
	}
}

void testThread1(void)
{
	DDRF = 0xC0;
	while(1){	
		PORTF ^= 0x40;
		x_delay(500);
	}
}

/**The x_delay function causes a thread to place itself in a "blocked" condition for a specified number of "system ticks". If there are other READY 
threads, then one of them will be selected by the scheduler to be placed into execution.*/
void x_delay(unsigned int x)
{
	//copy the delay value into the calling thread's delay counter
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		x_thread_delay[x_thread_id] = x;
	
	
		//set the x_delay_status bit corresponding to the calling thread's ID, then
		delay = delay | (1 << x_thread_id);
	}
	//call x_yield to initiate thread rescheduling
	x_yield();
	
	
	/**
	Make sure to enclose any accesses to x_thread_delay elements in the following:
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		access to variable
	}*/
}

unsigned long x_gtime()
{
	return 0;
}

/**When called, x_new initializes the stack of the specified thread by copying the function pointer onto its stack (as if it were a return address).
The thread's stack pointer is then be updated to a value that allows space for all the registers that are normally saved when x_yield is called
(x_yield is the primary rescheduling function of the kernel). For example, if the value of the new thread's sp is initialized to 0x1FF, then the
newthread address is copied to 1FF (low byte), 1FE (mid byte), and 1FD(high byte) . Then the thread's saved stack pointer (e.g., stack[Tx_ID].pstack
where x is 0 through 7) is decremented by:*/
void x_new(byte threadID, PTHREAD newthread, byte isEnabled)
{
	PTU currThread;
	currThread.thread = newthread;
	
	uint8_t *stackPtr = threads[threadID].pStackBase;
	
	*stackPtr = currThread.addr[0];
	stackPtr--;
	*stackPtr = currThread.addr[1];
	stackPtr--;
	*stackPtr = currThread.addr[2];
	stackPtr--;
	
	stackPtr -= 18;
	threads[threadID].sp = stackPtr;
	
	uint8_t newMask = ~(1 << threadID); 
	
	if(isEnabled)
	{	
		disable = disable & newMask;
	}
	
	if(threadID != x_thread_id)
	{
		return;
		//after carrying out its function of setting up the specified thread's stack, x_new simply returns to the calling thread. The newly initialized 
		//thread will be scheduled to run when the next rescheduling call (x_new or x_delay) is made, and it the next READY thread.
	}
	else
	{
		x_schedule();
		/*the x_new function does not return to the caller, but instead must jump to the rescheduling part of the x_yield function (skipping the register save 
		part).  This is the x_schedule entry point. This allows the scheduler to find the next READY thread, and if the new thread is the next ready thread, it
		 will be restored to execution in the usual way. To implement this you will simply call x_schedule as if it were a regular C function (when it is in 
		 fact just an alternate entry point into the x_yield function).*/
	}
}
//suspend the specified thread by setting its "suspend" status bit.
void x_suspend(uint8_t x)
{
	uint8_t temp = SREG;   // save SREG --holds global interrupt enable bit
	cli();  // disable interrupts
	//do the atomic access
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// do atomic access
		suspend = suspend | (1 << x);
	}
	SREG = temp;   // restore interrupt state
}

//resume the specified thread by clearing its "suspend" status bit.
void x_resume(uint8_t x)
{
	uint8_t temp = SREG;   // save SREG --holds global interrupt enable bit
	cli();  // disable interrupts
	//do the atomic access
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// do atomic access
		suspend = suspend & ~(1 << x);
	}
	SREG = temp;   // restore interrupt state
}

//disable the specified thread by setting its "disable" status bit.
void x_disable(uint8_t x)
{
	uint8_t temp = SREG;   // save SREG --holds global interrupt enable bit
	cli();  // disable interrupts
	//do the atomic access
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// do atomic access
		 disable = disable | (1 << x);
	}
	SREG = temp;   // restore interrupt state
}

//enable the specified thread by clearing its "disable" status bit.
void x_enable(uint8_t x)
{
	uint8_t temp = SREG;   // save SREG --holds global interrupt enable bit
	cli();  // disable interrupts
	//do the atomic access
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// do atomic access
		disable = disable & ~(1 << x);
	}
	SREG = temp;   // restore interrupt state
}

void changeStack(uint8_t * stackPointer, uint8_t * newStackBase, int num_bytes)
{
	byte *current = stackPointer;
	byte buff[num_bytes];
	
	for(int i = 0; i < num_bytes; i++)
	{
		current = current + 1;
		buff[i] = (*current);
	}
	
	current = newStackBase;
	
	for(int i = (num_bytes -1); i >= 0; i--)
	{	
		(*current) = buff[i];
		current = current - 1;
	}
}


/**When a Timer0 interrupt occurs, the Timer0 interrupt handler should check each thread delay counter. If a counter is non-zero, 
it should be decremented. If the resulting count becomes zero, the x_delay_status bit corresponding to that thread should be cleared 
to zero, thus removing the delay blocking condition from that thread.

This will ideally be less than 160 clock cycles*/
ISR(TIMER0_COMPA_vect)
{
		
	for(int i = 0; i < NUM_THREADS; i++)
	{
		if(x_thread_delay[i] != 0)
		{	
			x_thread_delay[i] = x_thread_delay[i] - 1;
			if(x_thread_delay[i] == 0)
			{
				uint8_t newMask = ~(1 << i);
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				delay = delay & newMask;
				}
			}
		}
	}
}

void x_schedule()
{
	byte isDone = 0;
	byte current_thread = x_thread_id + 1;
	do 
	{
		x_thread_mask = (1 << current_thread);	
		if((x_thread_mask & ~(disable | suspend | delay)))
		{
			isDone = true;
			x_thread_id = current_thread;
			restore();
		}
		if(current_thread < (NUM_THREADS - 1))
		{
			current_thread++;
		}
		else
		{
			current_thread = 0;
		}
		
	} while (true);
}

