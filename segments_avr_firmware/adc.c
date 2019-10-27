/*
 * analog.c
 *
 * Created: 16/09/2019 11:21:27 AM
 *  Author: fhu
 */ 

#include <avr/io.h>
#include <util/atomic.h>
#include "adc.h"

static adc_callback_t adc_callback = 0;


void adc_init(void)
{
	ADCSRA |= _BV(ADEN);				// enable ADC (turn on ADC power)
	adc_setPrescaler(ADC_PRESCALE);		// set default prescaler
	adc_setReference(ADC_REFERENCE);	// set default reference
	ADMUX  &= ~_BV(ADLAR);				// set to right-adjusted result
	ADCSRA |= _BV(ADIE);				// enable interrupt
	adc_setChannel(0);
}

// configure A2D converter clock division (prescaling)
void adc_setPrescaler(uint8_t prescale)
{
	ADCSRA = (ADCSRA & ~ADC_PRESCALE_MASK) | prescale;
}

// configure A2D converter voltage reference
void adc_setReference(uint8_t ref)
{
	ADMUX = (ADMUX & ~ADC_REFERENCE_MASK) | (ref<<6);
}

// sets the a2d input channel
void adc_setChannel(uint8_t ch)
{
	ADMUX = (ADMUX & ~ADC_MUX_MASK) | (ch & ADC_MUX_MASK);	// set channel
	DIDR0 |= _BV(ch);
}

// Perform a 10-bit conversion (blocking)
// starts conversion, waits until conversion is done, and returns result
uint16_t adc_capture(uint8_t ch)
{
	adc_setChannel(ch);
	ADCSRA &= ~_BV(ADIF);						// clear hardware "conversion complete" flag
	ADCSRA |=  _BV(ADSC);						// start conversion
	loop_until_bit_is_clear( ADCSRA, ADSC);		// wait until conversion complete

	// CAUTION: MUST READ ADCL BEFORE ADCH!!!
	return ADC;	// read ADC (full 10 bits);
}

void adc_start_capture()
{
	ADCSRA |=  _BV(ADSC);
}

void register_adc_callback(adc_callback_t callback_fn)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		adc_callback = callback_fn;		
	}
}



ISR(ADC_vect)
{
	if(adc_callback)
	{
		adc_callback(ADC);
	}
}