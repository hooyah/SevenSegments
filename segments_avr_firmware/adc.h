/*
 * analog.h
 *
 * Created: 16/09/2019 11:22:07 AM
 *  Author: fhu
 */ 


#ifndef ANALOG_H_
#define ANALOG_H_

// ------------ PRESCALER ------------
#define ADC_PRESCALE_DIV2		0x00	///< 0x01,0x00 -> CPU clk/2
#define ADC_PRESCALE_DIV4		0x02	///< 0x02 -> CPU clk/4
#define ADC_PRESCALE_DIV8		0x03	///< 0x03 -> CPU clk/8
#define ADC_PRESCALE_DIV16		0x04	///< 0x04 -> CPU clk/16
#define ADC_PRESCALE_DIV32		0x05	///< 0x05 -> CPU clk/32
#define ADC_PRESCALE_DIV64		0x06	///< 0x06 -> CPU clk/64
#define ADC_PRESCALE_DIV128		0x07	///< 0x07 -> CPU clk/128
// default value
#define ADC_PRESCALE			ADC_PRESCALE_DIV64
#define ADC_PRESCALE_MASK		0x07

// --------------- REFERENCE -------------
#define ADC_REFERENCE_AREF		0x00	///< 0x00 -> AREF pin, internal VREF turned off
#define ADC_REFERENCE_AVCC		0x01	///< 0x01 -> AVCC pin, internal VREF turned off
#define ADC_REFERENCE_RSVD		0x02	///< 0x02 -> Reserved
#define ADC_REFERENCE_256V		0x03	///< 0x03 -> Internal 1.1V VREF
// default value
#define ADC_REFERENCE			ADC_REFERENCE_AVCC
// do not change the mask value
#define ADC_REFERENCE_MASK		0xC0

#define ADC_MUX_MASK			0x0F



typedef void (*adc_callback_t)(uint16_t);

void adc_init(void);
void adc_setPrescaler(uint8_t prescale);
void adc_setReference(uint8_t ref);
void adc_setChannel(uint8_t ch);
uint16_t adc_capture(uint8_t ch);
void adc_start_capture();
void register_adc_callback(adc_callback_t);

#endif /* ANALOG_H_ */