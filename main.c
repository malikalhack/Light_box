/************************************
* Title	 	: main.c				*
* Release	: 0.1.B					*
* Creator	: Malik					*
* Created	: 25.01.2016 17:00:24	*
* Changed	: 21.10.2020			*
* MCU	 	: ATtiny13A		 		*
* Frequency	: 1.2 MHz (int)			*
************************************/

/******************************** Included files **********************************/
#include "standard.h"
/********************************** Definition ************************************/
#define DISCRETELY
#define MEAS_NUM	10
#define DEF_COUNTER 18
/****************************** Private prototypes ********************************/
ISR (ADC_vect);
ISR (TIM0_OVF_vect);
void init (void);
BYTE calc (void);
void adjust (BYTE);
/****************************** Private  variables ********************************/
static BYTE control = 0;
static BYTE step = 0;
static BYTE counter = DEF_COUNTER;
static BYTE mesurment[MEAS_NUM] = {};
/******************************* Additional Definition ****************************/
#define CHK_FLAG	control&BIT(0)
#define SET_FLAG	control |= BIT(0)
#define CLR_FLAG	control &= ~BIT(0)
/********************************** Entry point ***********************************/
int main(void) {
    init();
/********************************* Endlessly loop *********************************/
    while (1) {
		if(CHK_FLAG) {
			CLR_FLAG;
			adjust(calc());
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
	TCCR0B	=  0x03;				/* No pre-scaling (clock/64) */
	TIMSK0  =  0x02;
	ADMUX	=  0x22;				/* ADC2(PB4), reference - Vcc, left adjust */
	ADCSRA	=  0x8b;				/* ADC and interrupt are enable, div8 */
	ADCSRA |=  BIT(ADSC);			/* start ADC */
	sei();
}
/**********************************************************************************/
BYTE calc (void) {
	cli();
	register BYTE i, min;
	min = mesurment[0];
	for (i = 1; i < MEAS_NUM; i++) {
		if (min > mesurment[i]) min = mesurment[i];
	}
	sei();
	return min;
}
/**********************************************************************************/
void adjust (register BYTE result) {
	register BYTE t_OCR0A = OCR0A;
	if (result == t_OCR0A) return;
#ifdef DISCRETELY
	OCR0A = result;
#else
	while (result != t_OCR0A) {
		if (result > t_OCR0A) t_OCR0A++;
		else t_OCR0A--;
		OCR0A = t_OCR0A;
	}
#endif
}
/**********************************************************************************/
ISR (ADC_vect) {
	mesurment[step] = ADCH;			/* without the last two bits */
	if (++step == MEAS_NUM) {
		step = 0;
		SET_FLAG;
	}
}
/**********************************************************************************/
ISR (TIM0_OVF_vect) {
	if (!(--counter)) {
		counter = DEF_COUNTER;
		ADCSRA |=  BIT(ADSC);
	}
}
/**********************************************************************************/
