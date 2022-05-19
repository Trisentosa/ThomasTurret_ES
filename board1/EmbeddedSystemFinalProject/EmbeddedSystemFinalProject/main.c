// CPU Frequency -> 16 MHz
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//Set Baudrate
#define USART_BAUDRATE 9600
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

//Imports
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <avr/power.h>

#include "IR_Receiver.h"

//Functions declarations

void usart_init();
unsigned char usart_receives();
void usart_transmit(char arr[]);
void removeGarbage();
void ir_actions(int commandCode);
void servo_init();
void servo_auto();

//state initialization
int state = 0; //0 = manual, 1 = auto
struct IR_Packet received_packet;


int main(void)
{
	int currentCommand;
	
	//servo initialization
	DDRB |= 1 << PINB1; // Set pin 9 on arduino to output
	DDRD |= 0 << PIND4;
	PORTD |= 1 << PIND4;
	DDRB |= 0B000001; // PORTB5, LED
	
	//usart, receiver initialization
	usart_init();
	sei();
	init_receiver();
	servo_init();
	
	usart_transmit("START\n");
	while (1) {
		cli();
		uint8_t check_result = check_new_packet(&received_packet);	
		sei();
		
		if (state == 1)
		{
			servo_auto();
		}

		if (check_result)
		{
			//char buff[10];
			usart_transmit("\n\r");
			//utoa(received_packet.command, buff, 16);
			//usart_transmit(buff);
			currentCommand = received_packet.command;
			ir_actions(currentCommand);
		}
	}
	return 0;
}

void usart_init() {
	UBRR0H = (unsigned char)(UBRR_VALUE>>8);
	UBRR0L = (unsigned char)UBRR_VALUE;
	// Set frame format to 8 data bits, no parity, 2 start bit, 1 stop bit (11 bits)
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
	//enable transmission and reception inturrupt
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
}

unsigned char usart_receives() {
	// Wait for byte to be received
	while(!(UCSR0A&(1<<RXC0))){};
	// Return received data
	return UDR0; // get char from buffer
}

void usart_transmit(char arr[]) {
	int i;
	for (i = 0; i <= strlen(arr)-1; i++) {
		while (!( UCSR0A & (1<<UDRE0))) {}; // wait until buffer empty
		UDR0 = arr[i];
	}
}

void removeGarbage(){
	unsigned char x = usart_receives();
	while(x != 0x0A){
		x = usart_receives();
	}
}

void servo_init(){
	DDRD |= _BV(5); // pin 5 out
	
	TCCR0A = _BV(WGM00) | _BV(COM0B1) | _BV(COM0B0) ; // mode5 , set on rising clear on falling
	
	TCCR0B = _BV(WGM02) | _BV(CS02) | _BV(CS00); //1024 prescaler
	
	OCR0A = 156; // 156*2=312=20ms period
	OCR0B = 136;
}

void servo_auto(){
	while(1){
		for (OCR0B = 136; OCR0B < 152; OCR0B++){//1ms to 2ms duty
			_delay_ms(100);
			uint8_t check_result = check_new_packet(&received_packet);
			//check for incoming signal, if signal is "MODE" button then stop
			if (check_result && received_packet.command == 0x46 && received_packet.repeat == 0)
			{
				usart_transmit("new");
				state = 0;
				return;
			}
		}
			
		for (OCR0B = 152; OCR0B > 136; OCR0B--){//2ms to 1ms duty
			_delay_ms(100);
			uint8_t check_result = check_new_packet(&received_packet);
			//check for incoming signal, if signal is "MODE" button then stop
			if (check_result && received_packet.command == 0x46 && received_packet.repeat == 0)
			{
				usart_transmit("new");
				state = 0;
				return;
			}
		}
	}
		
}

void ir_actions(int commandCode){
	switch (commandCode)
		{
			case 0x45:
			usart_transmit("POWER ON");
			break;
			case 0x46:
			usart_transmit("SWITCH MODE");
			if (received_packet.repeat == 0)
			{
				if (state == 0)
				{
					state = 1;
				}else{
					state = 0;
				}
				
			}
			break;
			case 0x47:
			usart_transmit("SHOOT");
			// turn LED on
			PORTB |= 0B000001; // PORTB5
			_delay_ms(100);

			// turn LED off
			PORTB &= ~ 0B000001; // PORTB5
			_delay_ms(100);
			break;
			case 0x40:
			usart_transmit("LEFT");
			if (OCR0B > 136)
			{
				OCR0B = OCR0B -1;
			}
			break;
			case 0x43:
			usart_transmit("RIGHT");
			if (OCR0B < 152)
			{
				OCR0B = OCR0B +1;
			}
			break;
			case 0x09:
			usart_transmit("reset right");
			OCR0B = 152;
			break;
			case 0x15:
			usart_transmit("reset left");
			OCR0B = 136;
			break;
		}
}

