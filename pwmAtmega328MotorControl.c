/*
 * pwmAtmega328MotorControl.c
 *
 * Created: 5-1-2013 19:44:19
 *  Author: George Okeke
 */ 


#include <avr/io.h>

// initialize the usart
void USART_Init(void)
{
	UBRR0H = 0x00;		// 9600 bps (@ 8MHz)
	UBRR0L = 0x33;
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);		// receive and transmit Enable
	UCSR0C |= 0x06;		//8 databit, geen pariteit, 1 stopbit
	UCSR0A = (UCSR0A & 0xFD);
}

// This program sends karakter
void USART_Transmit (unsigned char data)	// pollend zenden van een karakter
{
	while (!(UCSR0A & 0x20)) {		// wacht totdat zendbuffer(UDRE) leeg is
		
	}		

	UDR0 = data;			// zend data
	UCSR0A = (UCSR0A | (1<<TXC0));
}

// This program receives karakter
unsigned char USART_Receive (void)			// pollend ontvangen van karakter
{
	unsigned char data;
	
	while (!(UCSR0A & 0x80)) {	//wacht totdat karakter binnen is (RXC bit set)
		
	}		
	
	data = UDR0;
	
	return data;			// geef karakter terug aan programma
}

// this program flushes the receive and transmit registers
void USART_Flush( void )
{
	unsigned char trowaway;
	
	while ( UCSR0A & (1<<RXC0) ) {
		trowaway = UDR0;
	}	
}


int main(void)
{
	// Initialize the USART
	USART_Init();
	
	// Enable the PRTIM1 bit in Power Reduction Register to disable the Timer/Counter1
	//PRR = (PRR | (1 << PRTIM1) | (1 << PRTIM0) | (1 << PRUSART0));
	
	// (WGM13:0) is set to mode 9(PWM, Phase and Frequency Correct, 16-bit)
	// (COM1B1:0) is set to mode 3(set OC1B on compare match)
	// (CS12:0) is set to mode 1 (clk/1)from prescaler

	TCCR1A = 0xF0;	//bit7: compare output mode for compare unit A(COM1A1)
					//bit6: compare output mode for compare unit A(COM1A0)
					//bit5: compare output mode for compare unit B(COM1B1)
					//bit4: compare output mode for compare unit B(COM1B0)
					//bit3: 
					//bit2: 
					//bit1:0 Waveform Generation Mode (WGM11:0)
	TCCR0A = 0xA3;	//bit7: compare output mode for compare unit A(COM1A1)
					//bit6: compare output mode for compare unit A(COM1A0)
					//bit5: compare output mode for compare unit B(COM1B1)
					//bit4: compare output mode for compare unit B(COM1B0)
					//bit3:
					//bit2:
					//bit1:0 Waveform Generation Mode (WGM11:0)
	
	
	TCCR1B = 0x11;	//bit7: Input Capture Noise Canceler(ICNC1
					//bit6: Input Capture Edge Select(ICES1)
					//bit5: - Reserved bit
					//bit4:3 Waveform Generation Mode(WGM13:2)
					//bit2:0 Clock Select(CS12:0)
	TCCR0B = 0x01;	//bit4:3 Waveform Generation Mode(WGM13:2)
					//bit2:0 Clock Select(CS12:0)

	DDRB = 0xFF;	// PORTB the direction is defined as OUTPUT
	DDRD = 0xFF;	// PortD the direction is defined as OUTPUT
	
	// defines the top values used in generating pwm for the servo motor
	ICR1H = 0xFF;
	ICR1L = 0xFF;
	
	// after setting the counter registers value then disable the PRTIM1 bit in Power Reduction Register to enable the Timer/Counter1
	//  and disable the PRTIM0 bit in the Power Reduction register to enable the Timer/counter0
	// and disable the Power Reduction USART bit (PRUART0) in order to make use of the usart.
	PRR = (PRR & 0xD5);
	
	unsigned int count = 0;
	const unsigned char BUFLEN = 6;
	unsigned char PACKETBEGIN = 1;
	unsigned char recvBuf[] = {0, 0, 0, 0, 0, 0};	// byte 0: check byte must be 1.
														// byte 1: OCR1AH value is the High byte for the left servo motor
														// byte 2: OCR1AL value is the Low byte for the left servo motor
														// byte 3: OCR1BH value is the High byte for the right servo motor
														// byte 4: OCR1BL vlaue is the Low byte for the left servo motor
														// byte 5: OCR0B  value is the speed control for the DC motor	
	
	
	// only used as the default values
	OCR0B = 0x7F;
	OCR1AH = 0xF0;
	OCR1AL= 0x00;
	OCR1BH = 0xFA;
	OCR1BL= 0x00;	
	PORTB = 0xFF;
	//--------------------
	USART_Flush();
		
    while(1)
    {
		// receive the control data's (BLOCK if there is no data to receive)
		for (count = 0; count < BUFLEN; count++) {
			recvBuf[count] = USART_Receive();
		}
		// send back the received control data's for correctness checking
		for (count = 0; count < BUFLEN; count++) {
			USART_Transmit(recvBuf[count]);
		}

		if (recvBuf[0] == PACKETBEGIN) {
			// update the registers with the new values
			OCR1AH = recvBuf[1];
			OCR1AL= recvBuf[2];
			OCR1BH = recvBuf[3];
			OCR1BL= recvBuf[4];
			OCR0B = recvBuf[5];
		}
	}		
}