/*----------------------------------------------------------------
 * serial7seg.c
 *
 * Created: 29/12/2012 14:03:33
 *  Author: Fab
 -----------------------------------------------------------------*/ 

#include <stdlib.h>
#include <math.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#define F_CPU 9600000UL    /* Clock Frequency = 9.6Mhz */
#include <util/delay.h>

void s7s_ckpulse();
void s7s_send6digit (long  int);
void s7s_sendInt(unsigned char );
void s7s_cleardigit ();


// 7 segments map table
const unsigned char s7s_map[] = {
	0xEE, 0x82, 0xDC, 0xD6, 0xB2, 0x76, 0x7E, 0xC2, //   0_  0  1  2  3  4  5  6  7 
	0xFE, 0xF6, 0xFA, 0x3E, 0x6C, 0x9E, 0x7C, 0x78, //   8_  8  9  A  b  C  d  E  F
	0xEF, 0x83, 0xDD, 0xD7, 0xB3, 0x77, 0x7F, 0xC3, //  16_  0. 1. 2. 3. 4. 5. 6. 7.
	0xFF, 0xF7, 0xFB, 0x3F, 0x6D, 0x9F, 0x7D, 0x79, //  24_  8. 9. A. b. C. d. E. F. 
	0x00, 0x10, 0x40, 0x04, 0x28, 0x28, 0xAA        //  32_ null - - _  '| ' ' |' '||'
};

int main(void)
{
	long int x, y, res;
	unsigned char i = 0;
	DDRB |=  (1<<PB4)|(1<<PB2);

	//-------------------------------------------------------
	// Configuration of ADC
	//-------------------------------------------------------
	PRR &= ~(1<<PRADC); // Power reduction disabled

	// • Set the Mux bitfield (MUX3:0) in ADC’s MUX register (ADMUX) equal to 0001 to
	// select ADC Channel 1
	ADMUX |= (1<<MUX1)|(1<<MUX0); // PB3 select, ADC3
	
	// • Set the Voltage Reference bitfield (REFS0) in ADMUX equal to 0 to select Internal
	// 1.1V reference
	// n/a here

	// • Set the ADC Enable bit (ADEN) in ADC Control and Status Register A (ADCSRA)
	// to enable the ADC module
	ADCSRA |= (1<<ADEN);

	// • Set the ADC Prescaler bitfield (ADPS2:0) in ADCSRA equal to 100 to prescale
	// system clock by 16
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	// • Set the Auto Trigger Enable bit (ADATE) in ADCSRA equal to 1 to enable auto
	// triggered mode
	ADCSRA |= (1<<ADATE);

	// • By default, the Auto Trigger Source bitfield (ADTS2:0) in ADC Control and Status
	// Register B (ADCSRB) is set to 000, which represents Free-running mode
	ADCSRB &= ~(1<<ADTS2) & ~(1<<ADTS2) & ~(1<<ADTS2);

	// • Set the Start Conversion bit (ADSC) in ADCSRA to start the first conversion
	ADCSRA |=  (1 << ADSC);

	// • Optionally wait for the Interrupt Flag bit in the ADCSRA register to be set,
	// indicating that a new conversion is finished

	// • Read the Result register pair for (ADCL/ADCH) to get the 10-bit conversion result
	// as a 2-byte value

	// Enable interrupt for ADC
	/*ADCSRA |= (1<<ADIE);*/
	
	/*sei();*/
	//-------------------------------------------------------
	// Main loop
	//-------------------------------------------------------
	while (1) {
		_delay_ms(500);
		res = 0;
	
		for(i=0; i< 8; i++) {
			x = (long int)ADCL;
			y = (long int)ADCH;
			res += x + (y<<8);
			_delay_ms(1);
		}
		s7s_send6digit( (res*625)>>10 );
	}
	return 0;
}

//-------------------------------------------------------
// Display 6 digit
//-------------------------------------------------------
void s7s_send6digit ( long int number_value)
{
	char number_str[6];
	ltoa(number_value, number_str, 10);
	
	s7s_cleardigit();
	for(unsigned char i = 0; i < 6; i++ ) {
		if ( number_str[i] != 0 ) {
			s7s_sendInt( number_str[i] - 48 );
		} else {
			break;
		}
	}
}

//-------------------------------------------------------
// Clear digit
//-------------------------------------------------------
void s7s_cleardigit () {
	unsigned char i = 6*8;
	PORTB &= ~(1<<PB4);
	for (i=0; i<6*8; i++) {
		PORTB |=  (1<<PB2); // s7s_ck = 1;	// Clock pulse
		PORTB &= ~(1<<PB2); // s7s_ck = 0;
	}
}

//-------------------------------------------------------
// Display a single int from 0 to 15 (hexa)
//-------------------------------------------------------
void s7s_sendInt( unsigned char m )
{
	signed char j;

	for(j=7; j>=0; j--) {
		if (((s7s_map[m]>>j) & 0x01) == 0x01) {
			PORTB |= (1<<PB4);
		} else {
			PORTB &= ~(1<<PB4);
		}
		
		PORTB |=  (1<<PB2); // s7s_ck = 1;	// Clock pulse
		PORTB &= ~(1<<PB2); // s7s_ck = 0;
	}
}
