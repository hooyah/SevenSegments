/*
 * uart.h
 *
 * Created: 16/09/2019 12:53:32 AM
 *  Author: fhu
 */ 


#ifndef UART_H_
#define UART_H_

#include <stdio.h>

#define BAUD 9600
#define UART_BUFFER_SIZE 32

void uart_init(void);

// direct, blocking
void uart_putchar(char c, FILE *stream);
char uart_getchar(FILE *stream);

// interrupt driven, buffered
uint8_t uart_message_available();
void    uart_clear_message();
uint8_t uart_transmitter_ready();
void    uart_send_buffered(const char* src, uint8_t size);



extern FILE uart_output; 
extern FILE uart_input; 
volatile char uart_buff_rx[UART_BUFFER_SIZE];


#endif /* UART_H_ */