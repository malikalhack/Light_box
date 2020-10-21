/********************************
* Title	 : main.c				*
* Release: alpha				*
* Creator: Malik				*
* Created: 25.01.2016 17:00:24	*
* Changed: 21.10.2020			*
* MCU	 : ATtiny13A		 	*
* Clock frequency: 1.2 MHz (int)*
********************************/

#define F_CPU 1200000UL
/******************************** Included files **********************************/
#include "standard.h"
#include <util/delay.h>
/****************************** Private prototypes ********************************/
ISR (ADC_vect);
void init (void);
void change (void);
BYTE calc (void);
/****************************** Private  variables ********************************/
volatile BYTE control = 0;
BYTE res[4] = {};
BYTE result = 0;
/********************************** Entry point ***********************************/
int main(void) {
    init();
/********************************* Endlessly loop *********************************/
    while (1) {
		if (control&BIT(0)) {
			control &= ~BIT(0);
			_delay_ms(1000);
			ADCSRA |= BIT(ADSC);	/* start ADC */
		}
		if (control&BIT(1)) {
			control &= ~BIT(1);
			result  =  calc();
			control |= BIT(2);
		}
		if (control&BIT(2)) {
			control &= ~BIT(2);
			change();
			ADCSRA  |= BIT(ADSC);	/* start ADC */
		}
    }
/**********************************************************************************/
}

/******************************** Private functions *******************************/
void init (void) {
	DDRB	=  0x0f;
	PORTB	=  0;
	OCR0A	=  1;
	TCCR0A	=  0x83;				/* Fast-PWM, non-invert mode */
	TCCR0B	=  0x01;				/* No prescaling (clk/1) */
	ADMUX	=  0x22;				/* In-ADC2, reference - Vcc, ADLAR=1 */
	ADCSRA	=  0x8e;				/* ADC - enable, interrupt enable, div8 */
	ADCSRA	|= BIT(ADSC);			/* start ADC */
	sei();
}
/**********************************************************************************/
BYTE calc (void) {
	register BYTE i, min;
	min = res[0];
	for (i = 1; i < 4; i++) {
		if (min > res[i]) min = res[i];
	}
	return min;
}
/**********************************************************************************/
void change (void) {
	register BYTE temp_ocr = OCR0A;
	if (result==temp_ocr) return;
	while (result != temp_ocr) {
		if (result > temp_ocr) temp_ocr++;
		else temp_ocr--;
		OCR0A = temp_ocr;
		_delay_ms(25);
	}
}
/**********************************************************************************/
ISR (ADC_vect) {
	static unsigned char step;
	res[step++]=ADCH;
	if (step == 4) {
		step = 0;
		control |= BIT(1);
		return;
	}
	control |= BIT(0);
}
/**********************************************************************************/
