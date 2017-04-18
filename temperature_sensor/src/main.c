#include <avr/io.h>
#include <util/delay.h>
#include <Blink.h>

void setup_sensor();
void recieve_transmission();
void transmit_to_screen();
void PSerial_write(unsigned char port, uint8_t data);
void PSerial_open(unsigned char port, long speed, int config);
void delay_usec(int delay);

#define SERIAL_8N1  (0x00 | (3 << 1))     // (the default)

const int SENSOR_DELAY = 5;

int main(int argc, char const *argv[])
{
  recieve_transmission();
  PSerial_open(0, 41,SERIAL_8N1);
  while(1){
    PSerial_write(0, 'j');
  _delay_ms(1000);
  }
}

void recieve_transmission(){
  DDRE = (1 << PE4); //set the data pin for output DIGITAL PIN 2
  // indicate readiness for a transmission
   PORTE = 0; //drive the line low
  _delay_ms(SENSOR_DELAY); //delay
  PORTE |= 0x08; // drive the line high
  delay_usec(70);
  
}

typedef struct
{
	volatile uint8_t uscrA;
	volatile uint8_t uscrB;
	volatile uint8_t uscrC;
	volatile uint8_t reserved;
	volatile uint16_t ubrr;
	volatile uint8_t udr;
} SERIAL_REGS;

SERIAL_REGS* serial_ports[] =
{
	(SERIAL_REGS *)(0XC0),
	(SERIAL_REGS *)(0XC8),
	(SERIAL_REGS *)(0XD0),
	(SERIAL_REGS *)(0X130)
};


//Takes input parameters for 'port', 'speed' (baud rate) and 'framing' (frame parameters: number of data bits, parity, number
//of stop bits) in various combinations to configure the 2560 for communication using the specified serial port (0,1, 2, or 3)
void PSerial_open(unsigned char port, long speed, int config)
{
	serial_ports[port]->uscrA = 0x20;
	serial_ports[port]->uscrB = 0x18;
	serial_ports[port]->uscrC = config;
	serial_ports[port]->ubrr = speed;
}

//Waits for the write buffer to be available, then writes a byte value to the buffer. This function does not return anything.
void PSerial_write(unsigned char port, uint8_t data)
{
	while(!((serial_ports[port]->uscrA) & 0x20))
	{
	}
	serial_ports[port]->udr = data;
}
