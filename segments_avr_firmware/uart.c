/*
 * uart.cpp
 *
 * Created: 16/09/2019 12:50:29 AM
 *  Author: fhu
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/atomic.h>

#include "global.h"
#include "uart.h"

#include <util/setbaud.h>

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input  = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);


volatile char uart_buff_rx[UART_BUFFER_SIZE];
volatile char uart_buff_tx[UART_BUFFER_SIZE];
volatile uint8_t rx_sz = 0, tx_sz = 0;
volatile uint8_t msg_ready = 0;
volatile uint8_t tx_ready = 1; 




void uart_init(void) {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	
	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0) | _BV(TXCIE0);   /* Enable RX and TX, + interrupts */
}

void uart_putchar(char c, FILE *stream) {
	//if (c == '\n') {
	//	uart_putchar('\r', stream);
	//}
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

char uart_getchar(FILE *stream) {
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}


uint8_t uart_message_available()
{
	uint8_t ret = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{	
		if(msg_ready)
			return rx_sz;
	}
	return ret;
}

void uart_clear_message()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		//if(msg_ready) {		
			rx_sz = 0;
			msg_ready = 0;
		//}
	}
}

uint8_t uart_transmitter_ready()
{
	return tx_ready;
}

void uart_send_buffered(const char* src, uint8_t size)
{
	// copy to transmit buffer, 'right' aligned
	if(tx_ready) {
		size = (size <= UART_BUFFER_SIZE) ? size : UART_BUFFER_SIZE;
		tx_sz = size;
		char *dst = &uart_buff_tx[UART_BUFFER_SIZE-size];
		for(;size;size--, dst++, src++)
			*dst = *src;
		
		loop_until_bit_is_set(UCSR0A, UDRE0);
		tx_ready = 0;
		UDR0 = uart_buff_tx[UART_BUFFER_SIZE-tx_sz];
		tx_sz--;
	}
}


ISR(USART0_RX_vect)
{
	
	uint8_t chr = UDR0;
	if(chr == '$') // start of msg: move buffer pointer to the beginning
	{
		rx_sz = 0;
		msg_ready = 0;
	}
	if(!msg_ready && rx_sz < UART_BUFFER_SIZE) {
		
		if(chr == '\n') {  // end of message
			msg_ready = 1;
		}
		uart_buff_rx[rx_sz] = chr;
		rx_sz++;
	}
}

ISR(USART0_TX_vect)
{
	// tx buffer is aligned to the end, ie. '-----send this'
	if(!tx_ready) {
		if(tx_sz) {
			UDR0 = uart_buff_tx[UART_BUFFER_SIZE-tx_sz];
			tx_sz--;
		}else{
			tx_ready = 1;
		}
	}	
}


