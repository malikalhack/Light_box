/********************************
* Название: Light.c				*
* Версия: alpha					*
* Автор	: Malik					*
* Дата 	: 25.01.2016 17:00:24	*
* Для AVR: ATTINY13A			*
* Тактовая частота: 1.2МГц (int)*
********************************/ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1200000UL
#define BIT _BV
#include <util/delay.h>
void init (void);
unsigned char calc (void);
void change (void);
volatile unsigned char control;
unsigned char res[4]={};
unsigned char result=0;

ISR (ADC_vect) {
	static unsigned char step;
	res[step++]=ADCH;
	if (step==4) {
		step=0;
		control|=BIT(1);
		return;
	}
	control|=BIT(0);
}

int main(void) {
    init();
    while (1) {
		if (control&BIT(0)) {
			control&=~BIT(0);
			_delay_ms(1000);
			ADCSRA|=BIT(ADSC); //start ADC
		}
		if (control&BIT(1)) {
			control&=~BIT(1);
			result=calc();
			control|=BIT(2);
		}
		if (control&BIT(2)) {
			control&=~BIT(2);
			change();
			ADCSRA|=BIT(ADSC); //start ADC
		}
    }
}

void init (void) {
	DDRB=0x0f;
	PORTB=0;
	OCR0A=1;
	TCCR0A=0x83; //Fast-PWM, non-invert mode
	TCCR0B=0x01; //No prescaling (clk/1)
	ADMUX=0x22; //In-ADC2, reference - Vcc, ADLAR=1
	ADCSRA=0x8e; //ADC - enable, interrupt enable, div8
	ADCSRA|=BIT(ADSC); //start ADC
	sei();
}

unsigned char calc (void) {
	register unsigned char min=res[0];
	if (min>res[1]) min=res[1];
	if (min>res[2]) min=res[2];
	if (min>res[3]) min=res[3];
	return min;
}
void change (void) {
	register unsigned char temp_ocr=OCR0A;
	if (result==temp_ocr) return;
	while (result!=temp_ocr) {
		if (result>temp_ocr) temp_ocr++;
		else temp_ocr--;
		OCR0A=temp_ocr;
		_delay_ms(25);
	}
}