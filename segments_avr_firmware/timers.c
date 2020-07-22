/*
 * timer.c
 *
 * Created: 16/09/2019 10:22:16 PM
 *  Author: fhu
 */ 

#include "timers.h"
#include <avr/interrupt.h>
#include <util/atomic.h>

// 12 rpm -> 5 rps -> 64*32*5 steps per second -> 10240Hz
#define MOTOR_MAX_RPM 12;
volatile int16_t current_pos = 0;
int16_t target_pos = 0;
volatile uint8_t motor_on = 0;

int16_t steps_per_callback = STEPS_PER_DIGIT / 10;
step_callback_t step_cb = 0;



void set_prescaler(uint8_t val)
{
	TCCR1B = (TCCR1B & ~TIMER_PRESCALER_MASK) | val;  
}

void timers_init()
{
	// timer1: step timer
	//prescaler
	//set_prescaler(TIMER_PRESCALER_DIV1);  // leave at clk_io/1
	// timer mode 4 (CTC on OCA)
	TCCR1B |= _BV(WGM12);
	
	OCR1A = F_CPU / STEP_FREQ; // frequency
	TCNT1 = 0; // reset counter
	//enable overflow interrupt
	TIMSK1 |= _BV(OCF1A);
}

void set_motor_on()
{
	motor_on = 1;	
	set_prescaler(TIMER_PRESCALER_DIV1);  // leave at clk_io/1
	TCNT1 = 0; // reset counter
}

void set_motor_off()
{
	motor_on = 0;
	set_prescaler(0);  // leave at clk_io/1	
}

/// changes the motors position (just the value) 
void set_motor_position(int16_t new_pos)
{	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		current_pos = new_pos;
	}	
}

void set_motor_target(int16_t trg_pos)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		target_pos = trg_pos;
		if(!motor_on) 
			set_motor_on();
	}
}

// savely offsets position and adjusts target if necessary
void shift_motor(int16_t offset)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		int16_t new_pos = current_pos + offset;
		if(new_pos > target_pos && current_pos <= target_pos)
			target_pos = new_pos;
		current_pos = new_pos;
	}
}


int16_t get_motor_position()
{
	int16_t ret;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		ret = current_pos;
	}	
	return ret;
}


int16_t get_motor_target()
{
	return target_pos;
}


void register_step_callback( step_callback_t p_callback, int16_t every_n_steps )
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		step_cb = p_callback;
		steps_per_callback = every_n_steps;
	}
}


void step()
{
static uint8_t  stp = 0;
static uint16_t internal_count = 0;
	
	#ifdef MOTOR_REVERSE
	uint8_t pattern = 0x33 << (3-stp);
	#else
	uint8_t pattern = 0x33 << stp;
	#endif

	if(pattern&16)
		MOTOR_C0_PORT |= _BV(MOTOR_C0_PIN);
	else
		MOTOR_C0_PORT &= ~_BV(MOTOR_C0_PIN);

	if(pattern&32)
		MOTOR_C1_PORT |= _BV(MOTOR_C1_PIN);
	else
		MOTOR_C1_PORT &= ~_BV(MOTOR_C1_PIN);

	if(pattern&64)
		MOTOR_C2_PORT |= _BV(MOTOR_C2_PIN);
	else
		MOTOR_C2_PORT &= ~_BV(MOTOR_C2_PIN);

	if(pattern&128)
		MOTOR_C3_PORT |= _BV(MOTOR_C3_PIN);
	else
		MOTOR_C3_PORT &= ~_BV(MOTOR_C3_PIN);

	stp = (stp + 1) % 4;
	if(internal_count++ >= steps_per_callback)
	{
		internal_count=0;
		if(step_cb)
			step_cb(current_pos);
	}	
		
	current_pos++;
	if(current_pos >= STEPS_PER_ROUNDTRIP)
		current_pos = 0;
		
}




// Timer1 interrupt handler

ISR(TIMER1_COMPA_vect)
{
	if(current_pos == target_pos) { //switch motor & timer off
		set_motor_off();
		MOTOR_C0_PORT &= ~_BV(MOTOR_C0_PIN);
		MOTOR_C1_PORT &= ~_BV(MOTOR_C1_PIN);
		MOTOR_C2_PORT &= ~_BV(MOTOR_C2_PIN);
		MOTOR_C3_PORT &= ~_BV(MOTOR_C3_PIN);
		return;
	}

	step();	
}