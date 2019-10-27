/*
 * timer.h
 *
 * Created: 16/09/2019 10:22:38 PM
 *  Author: fhu
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include "global.h"

typedef void (*step_callback_t)(int);

void timers_init();
int16_t get_motor_position();
int16_t get_motor_target();
void set_motor_position(int16_t new_pos);
void set_motor_target(int16_t trg_pos);
void shift_motor(int16_t offset);
void register_step_callback( step_callback_t p_callback, int16_t every_n_steps );


#define TIMER_PRESCALER_DIV1 _BV(CS40)
#define TIMER_PRESCALER_MASK 0x07

// motor
#define STEP_FREQ 550 // Hz
#define STEPS_PER_REVOLUTION (64*32)
#define STEPS_PER_DIGIT ((int)(STEPS_PER_REVOLUTION * 200L / 360))    // 200 degrees per digit
#define STEPS_PER_ROUNDTRIP ((int)(STEPS_PER_REVOLUTION * 2000L / 360)) // 10 digits




//debug
extern int16_t target_pos;
extern volatile uint8_t motor_on;

#endif /* TIMER_H_ */