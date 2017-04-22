#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

void setup_sensor();
void recieve_transmission();
void transmit_to_screen();
void PSerial_write(unsigned char port, uint8_t data);
void PSerial_open(unsigned char port, long speed, int config);
void delay_usec(int delay);
void print_data(uint8_t buffer[]);
void print_humidity(uint8_t buffer[], int start, int end);
void print_temp(uint8_t buffer[], int start, int end);
void print_error();
void print(uint8_t buffer[]);
int to_decimal(uint8_t buffer[], int start, int end);
int length(uint8_t buffer[]);
int verify_checksum(uint8_t buffer[], int start, int end);
void begin_transmission();
void recieve_transmission(uint8_t buffer[]);

#define SERIAL_8N1  (0x00 | (3 << 1))     // (the default)

const int SENSOR_DELAY = 5;
const int BUFFER_SIZE = 40;

int main(int argc, char const *argv[])
{
  uint8_t buffer[BUFFER_SIZE];
  while(1){		
  begin_transmission();
  recieve_transmission(buffer);
  print_data(buffer);
  _delay_ms(2000);
  }
}

void begin_transmission(){
  DDRE = (1 << PE4); //set the data pin for output DIGITAL PIN 2
  // indicate readiness for a transmission
}
/*
*pre-conditions - the buffer is empty
*post-condtions - the buffer is filled with the transmission data 
* 16 bits humidity 
* 16 bits temperature 
* 8 bits checksum
*/
void recieve_transmission(uint8_t buffer[]){	 	   
	   PORTE = 0; //drive the line low
	   _delay_ms(SENSOR_DELAY); //delay
	   PORTE = 0x80; // drive the line high 
	   int i = 0;
	   
	   // busy wait the low signal
	   do{
		   delay_usec(1); 
	   }while(PORTE == 0x80);
	   
	    //busy wait the high signal 
	   do{
		   delay_usec(1);
	   }while(PORTE == 0x00);
	   
		// receive the data
	   do{
		   while(PORTE == 0x00){		  
		   delay_usec(1);
		   }
		   delay_usec(50);
		   if(PORTE == 0x80){
			buffer[i] = 1;
		   }else if(PORTE == 0x00){
			 buffer[i] = 0;
		   }
		   i++;
		   }while( i < BUFFER_SIZE);
}

void print_data(uint8_t buffer[]){
	
static int BEGIN = 0;
static int HUMITY_STOP = 15;
static int TEMP_START = 16;	
static int TEMP_STOP = 31;
static int CHKSUM_START = 32;	

	if(!verify_checksum(buffer, CHKSUM_START, BUFFER_SIZE - 1)){
		print_error();
		return;	
	}
	print_humidity(buffer, BEGIN, HUMITY_STOP);
	print_temp(buffer, TEMP_START, TEMP_STOP);	
}

int verify_checksum(uint8_t buffer[], int begin, int end){
	
	static const uint8_t checksum[] = {1,1,1,0,1,1,1,0};
	for(int i = begin; i < end + 1; i++){
		if(buffer[i] != checksum[i]){return 0;}
	}
	return 1;
}

void print_humidity(uint8_t buffer[], int start, int end){
	float relative_humidity = to_decimal(buffer, start, end) / 10;
	char humidity_string[50];
	sprintf(humidity_string, "RELATIVE HUMIDITY: %f%\n",relative_humidity);
	print(humidity_string);
}

void print_temp(uint8_t buffer[], int start, int end){
		float current_temp = to_decimal(buffer, start, end) / 10;
		char temp_string[50];
		sprintf(temp_string, "TEMPERATURE: %f\n", current_temp);
		print(temp_string);
}

void print_error(){
	static char err_string[] = "Error while reading data";
	print(err_string);
}

int to_decimal(uint8_t buffer[], int start, int end){
	
	
	int total = 0;
	int head = 0;
	int tail = end;
	
	for(int i = start; i < end + 1; i++){
		total += buffer[tail] << head;
		--tail, ++head;
	}
	// account for twos complement
	if(buffer[0] == 1){total = ~total + 1;}
		
	return total;
}



void print(uint8_t buffer[]){
	
	static int portopen = 0;
	
	if(portopen == 0){
	PSerial_open(0, 51, SERIAL_8N1);
	portopen = 1;
	}
	
	int len = length(buffer);
	
	for(int i = 0; i < len; i++){
		PSerial_write(0, buffer[i]);
	}
}

int length(uint8_t buffer[]){
	return sizeof(buffer)/sizeof(uint8_t);
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