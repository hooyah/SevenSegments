/*
 * main.c
 *
 * Created: 15/09/2019 10:30:43 PM
 * Author : fhu
 *
 * V1.0 - shipping
 */ 

#include "global.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "uart.h"
#include "adc.h"
#include "timers.h"

/* ---- fuses atmega328P, not necessary unless you made your own board ---- */
/*#include <avr/io.h>
__fuse_t __fuse __attribute__((section (".fuse"))) =
{
	.low = 0xFF,
	.high = 0xD9,
	.extended = 0xFF,
};*/


const int16_t digit_offset = -STEPS_PER_DIGIT/2;  // device specific magnet position offset
int16_t position_deviation = 0;					  // once a magnetic event is detected the deviation from the actual and the assumed position shows up here

int8_t debug = 0;
uint8_t maglock = 1;

volatile uint16_t analog = 0;
volatile uint8_t  sampling_enabled = 1;
volatile uint16_t sampling_min_val = 0xffff;
volatile int16_t  sampling_min_pos = 0;
volatile uint16_t capture_position = 0;



// this is triggered every nth step, make it as lean as possible
void step_callback(int position)
{
	// trigger an adc sample, but don't wait for it to finish, irq will handle the result
	adc_start_capture();
	capture_position = position;
}


void adc_callback(uint16_t val)
{
static uint8_t inside_sampling_window = 0;
	
	analog = val;
	if(sampling_enabled) 
	{		
		if(val < MAG_TRIGGER_THRESHOLD)
		{
			inside_sampling_window = 1;
			// look for the smallest value (max magnetic field)
			if(val < sampling_min_val) 
			{
				sampling_min_val = val;
				sampling_min_pos = capture_position;
			}
		}
		else
		{
			if(inside_sampling_window) // done sampling 
			{
				sampling_enabled = 0;
				inside_sampling_window = 0;
			}
		}
	}
}



void move_to_digit(uint8_t digit)
{
	int16_t new_pos = STEPS_PER_DIGIT * digit + digit_offset;
	if(new_pos < 0)
		new_pos += STEPS_PER_ROUNDTRIP;
	set_motor_target(new_pos);
	if(debug)
		printf("new pos: cur:%d trg:%d\n", get_motor_position(), get_motor_target());
}



void handle_position_feedback()
{

	// sampling done, let's see what it looks like
	if(!sampling_enabled && sampling_min_val < 0xffff) {
	
		if(maglock)
		{	
			// define the minimum position as new origin (0)	
			if(sampling_min_pos > STEPS_PER_ROUNDTRIP/2)
				position_deviation = STEPS_PER_ROUNDTRIP-sampling_min_pos;
			else
				position_deviation = -sampling_min_pos;

			if(debug)
				printf("shifting: cur:%d trg:%d dev:%d\n", get_motor_position(), get_motor_target(), position_deviation);	
				
			shift_motor(position_deviation);
		}		
		// switch sampling back on
		sampling_min_val = 0xffff;
		sampling_min_pos=0;
		sampling_enabled = 1;
		maglock = 1;
	}
}



void handle_messages()
{
static char queued[UART_BUFFER_SIZE];
static uint8_t queue_sz = 0;
	
	uint8_t msg_size;

	// check if we need to forward a queued message
	if(queue_sz && !motor_on) {
		
		if(debug) {
			for(uint8_t c=0; c < queue_sz; c++)
				uart_putchar(queued[c], 0);
		}else{
			uart_send_buffered(queued, queue_sz);
		}
		queue_sz = 0;
	}

	if( (msg_size = uart_message_available()) != 0 ) {
		
		
		if(uart_buff_rx[0] == '$' && msg_size > 2) {// valid message
			
			if(uart_buff_rx[1] == 'D') { // a number to display
				
				// use the last digit
				uint8_t digit = (uart_buff_rx[msg_size-2]-'0');
				move_to_digit(digit%10);
				
				// forward remaining digits to next segment, but later
				if(msg_size > 4) {  
					uart_buff_rx[msg_size-2] = '\n';
					//uart_send_buffered(uart_buff_rx, msg_size-1);
					memcpy(queued, uart_buff_rx, msg_size-1);
					queue_sz = msg_size-1;
				}
			}
			else if(uart_buff_rx[1] == '%') { // message forwarding
				uart_buff_rx[1] = '$';
				uart_send_buffered(&uart_buff_rx[1], msg_size-1);
			}
			
			if( (uart_buff_rx[1] == 'd' && uart_buff_rx[2] == 'b' && uart_buff_rx[3] == 'g') ||
					((PINB & _BV(3)) == 0) ) 
			{
				if(!debug) // switch into debug mode 
				{
					printf("debug mode\n");
					printf("digit offset=%d\n", digit_offset);
					printf("current pos: %d\n", get_motor_position());
					printf("steps per digit: %d\n", STEPS_PER_DIGIT);
					debug = 1;
				}
			}
		}
		uart_clear_message();
	}
}







void setup()
{
	LED_DDR |= _BV(LED_PIN);			// LED pin to output
	MOTOR_C0_DDR |= _BV(MOTOR_C0_PIN);  // MOTOR pins tp output
	MOTOR_C1_DDR |= _BV(MOTOR_C1_PIN);
	MOTOR_C2_DDR |= _BV(MOTOR_C2_PIN);
	MOTOR_C3_DDR |= _BV(MOTOR_C3_PIN);

	PORTB |= _BV(3);	// enable pullup resistor on PB3 (MOSI) (debug pin)

	register_step_callback(step_callback, STEPS_PER_DIGIT/10);
	register_adc_callback(adc_callback);
	
	uart_init();
	adc_init();
	timers_init();
	
	stdout = &uart_output;
	stdin  = &uart_input;
	
	// enable interrupts
	sei();
	
	// todo: power down all regions of the chip that I don't need
}




int main(void)
{
uint16_t count = 1;
	
	setup();
	set_motor_position(-STEPS_PER_ROUNDTRIP); // initial motor outside of range to force mag lock
	

	uint16_t mag = adc_capture(0); 
	if(mag <= MAG_TRIGGER_THRESHOLD) { // if we wake up over the magnet, we cannot trust the minimum value calculation, so maglock will throw away the current measurement run
		maglock = 0;
		set_motor_position(-2*STEPS_PER_ROUNDTRIP);
		
		if(debug)
			printf("mag=%d\n", mag);
	}


	//move_to_digit(5);
    // main loop
    while (1) 
    {
		if(count % 100 == 0)
			PORTB |= _BV(LED_PIN);
		else
			PORTB &= ~_BV(LED_PIN);
		//PORTB ^= _BV(LED_PIN);
			
		_delay_ms(100);

		handle_position_feedback();
		handle_messages();
	
		count++;

		if(debug && count%2==0 && motor_on)
			printf("pos=%d trg=%d dev=%d, adc=%d\n", get_motor_position(), get_motor_target(), position_deviation, analog);

    }
}



