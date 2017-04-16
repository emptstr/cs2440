
/*
 * acx_asm.S
 *
 * Created: 
 *  Author: Matt Spencer and Jordan Gaston
 */

 #define __SFR_OFFSET 0
 #include <avr/io.h>
 #include "acx.h"
 
		
		.section .text
		.extern x_thread_id
		.extern threads
		.extern suspend
		.extern disable
		.extern delay
		.extern checkCanaries
		.extern MEM_SIZE
		.extern x_schedule
		.global x_yield
		.global restore
x_yield:
;------------------------------------------------------------------------
; Save thread context
;-------------------------------------------------------------------------
cli;

push r2
push r3
push r4
push r5
push r6
push r7
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15
push r16
push r17
push r28
push r29

in r14, SPL
in r15, SPH

//Calculate offset into STACKCONTROL array
lds r19, x_thread_id
lsl r19
lsl r19

//Load address of STACKCONTROL array into pointer register

ldi r25, hi8(threads)
ldi r24, lo8(threads)
adc r24, r19

mov ZL, r24
mov ZH, r25

st z+, r14
st z, r15

sei;

call checkCanaries
call x_schedule
/**
;------------------------------------------------------------------------
; Schedule next thread -- must use caller save registers below here
;                         because we can reach this without saving regs
;                         if reached from k_new() function
;-------------------------------------------------------------------------
		.global	x_schedule
x_schedule:
lds r24, disable
lds r25, suspend
lds r26, delay
lds r18, x_thread_id
lds r19, x_thread_id
ldi r20, 0
OR r24, r25
OR r26, r24
COM r26

readyCheck:
inc r19			//Increment r19
ldi r20, 8
sub r20, r19	//If reached max thread, reset to 0
brne skip
ldi r19, 0

skip:
ldi r24, 0		
add r24, r19	//Load thread id into r24 for bit2mask
		ldi r26, 1
shift:
		tst r24
		breq finish
		lsl r27
		dec r24
		jmp shift	//Now r24 holds bit mask
finish:

ldi r20, 0
add r20, r24	
and r20, r26	//If 1, ready thread found
brne found

//Add back in when sleep is implemented
sub r18, r19
breq done
lds r18, x_thread_id

JMP readyCheck

found:
//x_thread_mask = r24
sts x_thread_mask, r24
//x_thread_id = r19
sts x_thread_id, r19

done:
*/
;---------------------------------------------------
; Restore context of next READY thread
;---------------------------------------------------
restore:

cli //disable global interrupts

//Calculate offset into STACKCONTROL array
lds r19, x_thread_id
lsl r19
lsl r19

//Load address of STACKCONTROL array into pointer register
ldi r24, lo8(threads)
ldi r25, hi8(threads)

adc r24, r19

mov ZL, r24
mov ZH, r25

ld r24, z+
ld r25, z

out SPL, r24
out SPH, r25

pop r29
pop r28
pop r17
pop r16
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop r7
pop r6
pop r5
pop r4
pop r3
pop r2

sei
	// Return to next thread
		ret

//----------------------------------------------------------------------------------------------
//  Function: bit2mask8
//
//  Description:   Returns a bit mask corresponding to thread ID parameter.
//
//  Input params:  uint8_t id  -- must by integer between 0 and 7
//  Returns:       mask - a bit mask having a single bit set corresponding to input ID
//						  (interpreted as bit number)
//-----------------------------------------------------------------------------------------------
		.global bit2mask8





/*
 * register_tools.S
 *
 * Created: 4/7/2017 6:20:54 PM
 *  Author: gasto
 */ 


 /*
  * saveCalleeRegisters
  * saves all callee registers to the provided stack location
  *
  */
 .global saveCalleeRegisters

saveCalleeRegisters:
  
  cli

// get the correct stack pointer
in r26, (0x3d) // save the current stack low 
in r27, (0x3e)// save the current stack high
out (0x3d), r24 // load the threads stack low 
out (0x3e), r25 // load the threads stack high

// save the correct pointers
push r2
push r3
push r4
push r5
push r6
push r7
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15
push r16
push r17
push r28
push r29

// restore the original stack pointer
out (0x3d), r26 // load the original stack low 
out (0x3e), r27 // load the original stack high

sei //enable global interrrupts

ret


/*
* loadCalleeRegisters 
* restores the callee saved registers from the provided stack location
*/
 .global loadCalleeRegisters

loadCalleeRegisters:
  
cli //disable global interrupts

// get the correct stack pointer
in r26, (0x3d) // save the current stack low 
in r27, (0x3e)// save the current stack high
out (0x3d), r24 // load the threads stack low 
out (0x3e), r25 // load the threads stack high

//restore the callee saved registers
pop r29
pop r28
pop r17
pop r16
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop r7
pop r6
pop r5
pop r4
pop r3
pop r2

sei //enable global interrrupts

ret