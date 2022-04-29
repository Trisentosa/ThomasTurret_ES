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

#include "IR_Receiver.h"

//Functions declarations

void usart_init();
unsigned char usart_receives();
void usart_transmit(char arr[]);
void removeGarbage();
void ir_actions(int commandCode);
void servo_init();

int main(void)
{
	int currentCommand;
	
	//servo initialization
	DDRB |= 1 << PINB1; // Set pin 9 on arduino to output
	DDRD |= 0 << PIND4;
	PORTD |= 1 << PIND4;
	
	//usart, receiver initialization
	usart_init();
	sei();
	init_receiver();

	
	usart_transmit("START\n");
	while (1) {
		struct IR_Packet received_packet;
		cli();
		uint8_t check_result = check_new_packet(&received_packet);	
		sei();

		if (check_result)
		{
			char buff[10];
			if (received_packet.repeat > 0)
			{
				//utoa(received_packet.repeat, buff, 10);
				//usart_transmit(" Repeat: "); // Command repeat counter
				//usart_transmit(buff);
				ir_actions(currentCommand);
			} else
			{
				usart_transmit("\n\r");
				utoa(received_packet.command, buff, 16);
				currentCommand = received_packet.command;
				ir_actions(currentCommand);
			}
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

void ir_actions(int commandCode){
	switch (commandCode)
		{
			case 0x45:
			usart_transmit("POWER ON");
			break;
			case 0x46:
			usart_transmit("SWITCH MODE");
			break;
			case 0x47:
			usart_transmit("SHOOT");
			break;
			case 0x40:
			servo_init();
			OCR1A = 1999 - 800;
			usart_transmit("LEFT");
			_delay_ms(1000);
			break;
			case 0x43:
			servo_init();
			usart_transmit("RIGHT");
			OCR1A = 3999 + 800;
			_delay_ms(1000);
			break;
		}
}

void servo_init(){
	/* 1. Set Fast PWM mode 14: set WGM11, WGM12, WGM13 to 1 */
	/* 3. Set pre-scalar of 8 */
	/* 4. Set Fast PWM non-inverting mode */
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);

	/* 2. Set ICR1 register: PWM period */
	ICR1 = 39999;
}