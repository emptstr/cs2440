#include <avr/io.h>
#include <util/delay.h>

void blink(){//TODO extract me and use in the pserial functions for debugging
  PORTB ^=0x80;
  _delay_ms(1000);
  PORTB ^=0x80;
}
