
/*
 * delay_usec.S
 *
 * Created: 4/22/2017 10:26:49 AM
 *  Author: gasto
 */ 
 
.section .text
.global delay_usec

delay_usec: sbiw r24, 1
            breq isOne
            jmp overOne
isOne:      nop
            ret
overOne:    call addCycles
            jmp delay_usec
addCycles:  ret
