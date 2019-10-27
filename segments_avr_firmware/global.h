/*
 * options.h
 *
 * Created: 15/09/2019 10:40:03 PM
 *  Author: fhu
 */ 


#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <avr/io.h>

#define F_CPU 16000000L

#define LED_PIN 5
#define LED_DDR DDRB

#define MOTOR_C0_PIN  5
#define MOTOR_C0_DDR  DDRD
#define MOTOR_C0_PORT PORTD

#define MOTOR_C1_PIN  6
#define MOTOR_C1_DDR  DDRD
#define MOTOR_C1_PORT PORTD

#define MOTOR_C2_PIN  7
#define MOTOR_C2_DDR  DDRD
#define MOTOR_C2_PORT PORTD

#define MOTOR_C3_PIN  0
#define MOTOR_C3_DDR  DDRB
#define MOTOR_C3_PORT PORTB


#define MAG_TRIGGER_THRESHOLD 435

#endif /* OPTIONS_H_ */