/*! @file serial.c */

/* ################################################## Standard includes ################################################# */
#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
/* ################################################### Project includes ################################################# */
#include "serial.h"

/* ################################################### Global Variables ################################################# */
/* ############################################ Module Variables/Declarations ########################################### */
// Instance data
struct serial_struct{
	volatile uint8_t *ser_UDR;
	
	buffer_struct_t *_tx_buf;
	buffer_struct_t *_rx_buf;

	void(*_call_back )(serial_p,uint8_t);
};

#define serBAUD_DIV_CONSTANT			8UL

/* Constants for writing to UCSRA. */
#define serU2X_ENABLE					0x02
/* Constants for writing to UCSRB. */
#define serRX_INT_ENABLE				0x80
#define serRX_ENABLE					0x10
#define serTX_ENABLE					0x08
#define serTX_INT_ENABLE				0x20

/* Constants for writing to UCSRC. */
#define serUCSRC_SELECT					0x80
#define serEIGHT_DATA_BITS				0x06

#if defined (__AVR_ATmega2560__)
volatile uint8_t *_com_port_2_udr[] = {&UDR0, &UDR1, &UDR2, &UDR3};
static serial_p _ser_handle[] = {NULL, NULL, NULL, NULL};
#elif defined (__AVR_ATmega2561__)
volatile uint8_t  *_com_port_2_udr[] = {&UDR0, &UDR1};
static serial_p _ser_handle[] = {NULL, NULL};
#else
#error Serial Driver only implemented for ATMEGA256X
#endif

/* Offset to registers from UDR */
#define UBRR_off	2
#define UCSRC_off	4
#define UCSRB_off	5
#define UCSRA_off	6

/* Critical Sections */
#define ES_INIT_CRITICAL_SECTION 	\
uint8_t _sreg;

#define ES_ENTER_CRITICAL_SECTION 	\
_sreg = SREG;					\
cli();

#define ES_LEAVE_CRITICAL_SECTION	\
SREG = _sreg;


/*-----------------------------------------------------------*/
static void _serial_tx_int_on(volatile uint8_t *UDR_reg) {
	*(UDR_reg  - UCSRB_off) |= serTX_INT_ENABLE;
}

/*-----------------------------------------------------------*/
#define SERIAL_TX_INT_OFF(reg)	reg &= ~serTX_INT_ENABLE

/*-----------------------------------------------------------*/
serial_p serial_new_instance(e_com_port_t com_port, uint32_t baud, e_data_bit_t data_bit, e_stop_bit_t stop_bit, e_parity_t parity, buffer_struct_t *rx_buf, buffer_struct_t *tx_buf, void(*handler_call_back )(serial_p, uint8_t)) {
	serial_p _serial = malloc(sizeof *_serial);
	_ser_handle[com_port] = _serial;
	
	_serial->ser_UDR = _com_port_2_udr[com_port];

	_serial->_tx_buf = tx_buf;
	_serial->_rx_buf = rx_buf;
	
	_serial->_call_back = handler_call_back;
	
	ES_INIT_CRITICAL_SECTION
	ES_ENTER_CRITICAL_SECTION
	{
		/* Calculate the baud rate register value from the equation in the
		data sheet. */
		/* Set double speed */
		*(_serial->ser_UDR - UCSRA_off) |= serU2X_ENABLE;
		
		/* Set the baud rate. */
		*(_serial->ser_UDR - UBRR_off) = ( F_CPU / ( serBAUD_DIV_CONSTANT * baud ) - 1UL);

		/* Enable the Rx interrupt.  The Tx interrupt will get enabled
		later. Also enable the Rx and Tx. */
		*((_serial->ser_UDR) - UCSRB_off) = ( serRX_INT_ENABLE | serRX_ENABLE | serTX_ENABLE );

		/* Set the data bits to 8. */
		*((_serial->ser_UDR) - UCSRC_off) = serEIGHT_DATA_BITS;
	}
	ES_LEAVE_CRITICAL_SECTION
	
	return _serial;
}

/*-----------------------------------------------------------*/
uint8_t serial_send_byte(serial_p handle, uint8_t byte )
{
	if ( (handle->_tx_buf != 0)) {
		if (buffer_put_item(handle->_tx_buf,byte) == BUFFER_OK) {
			_serial_tx_int_on(handle->ser_UDR);
			return BUFFER_OK;
		}
	}
	return BUFFER_FULL;
}

/*-----------------------------------------------------------*/
uint8_t serial_send_bytes(serial_p handle, uint8_t *buf, uint8_t len )
{
	// Check if buffer is full
	if ( ((handle->_tx_buf != 0) && (len > (BUFFER_SIZE - handle->_tx_buf->no_in_buffer))) || ((handle->_tx_buf == 0) && (len > 1)) ) {
		return BUFFER_FULL;
	}
	
	// Put in the tx buffer
	for (uint8_t i = 0; i < len; i++) {
		buffer_put_item(handle->_tx_buf, buf[i]);
	}
	_serial_tx_int_on(handle->ser_UDR);
	return BUFFER_OK;
}

/*-----------------------------------------------------------*/
ISR(USART0_RX_vect)
{
	uint8_t item;
	if (_ser_handle[ser_USART0]) {
		item = UDR0;
		buffer_put_item(_ser_handle[ser_USART0]->_rx_buf, item);
		
		if (_ser_handle[ser_USART0]->_call_back) {
			_ser_handle[ser_USART0]->_call_back(_ser_handle[ser_USART0], item);
		}
	}
}

/*-----------------------------------------------------------*/
ISR(USART0_UDRE_vect)
{
	uint8_t item;
	if (_ser_handle[ser_USART0]) {
		if ((buffer_get_item(_ser_handle[ser_USART0]->_tx_buf, &item) == BUFFER_OK)) {
			UDR0 = item;
		}
		else
		{
			SERIAL_TX_INT_OFF(UCSR0B);
		}
	}
	
	else
	{
		SERIAL_TX_INT_OFF(UCSR0B);
	}
}

/*-----------------------------------------------------------*/
ISR(USART1_RX_vect)
{
	uint8_t item;
	if (_ser_handle[ser_USART1]) {
		item = UDR1;
		buffer_put_item(_ser_handle[ser_USART1]->_rx_buf, item);
		if (_ser_handle[ser_USART1]->_call_back) {
			_ser_handle[ser_USART1]->_call_back(_ser_handle[ser_USART1], item);
		}
	}
}

/*-----------------------------------------------------------*/
ISR(USART1_UDRE_vect)
{
	uint8_t item;
	if (_ser_handle[ser_USART1]) {
		if ((buffer_get_item(_ser_handle[ser_USART1]->_tx_buf, &item) == BUFFER_OK)) {
			UDR1 = item;
		}
		else
		{
			SERIAL_TX_INT_OFF(UCSR1B);
		}
	}
	
	else
	{
		SERIAL_TX_INT_OFF(UCSR1B);
	}
}


#if defined (__AVR_ATmega2560__)
/*-----------------------------------------------------------*/
ISR(USART2_RX_vect)
{
	uint8_t item;
	if (_ser_handle[ser_USART2]) {
		item = UDR2;
		buffer_put_item(_ser_handle[ser_USART2]->_rx_buf, item);
		if (_ser_handle[ser_USART2]->_call_back) {
			_ser_handle[ser_USART2]->_call_back(_ser_handle[ser_USART2], item);
		}
	}
}

/*-----------------------------------------------------------*/
ISR(USART2_UDRE_vect)
{
	uint8_t item;
	if (_ser_handle[ser_USART2]) {
		if ((buffer_get_item(_ser_handle[ser_USART2]->_tx_buf, &item) == BUFFER_OK)) {
			UDR2 = item;
		}
		else
		{
			SERIAL_TX_INT_OFF(UCSR2B);
		}
	}
	
	else
	{
		SERIAL_TX_INT_OFF(UCSR2B);
	}
}

/*-----------------------------------------------------------*/
ISR(USART3_RX_vect)
{
	uint8_t item;
	if (_ser_handle[ser_USART3]) {
		item = UDR3;
		buffer_put_item(_ser_handle[ser_USART3]->_rx_buf, item);
		if (_ser_handle[ser_USART3]->_call_back) {
			_ser_handle[ser_USART3]->_call_back(_ser_handle[ser_USART3], item);
		}
	}
}

/*-----------------------------------------------------------*/
ISR(USART3_UDRE_vect)
{
	uint8_t item;
	if (_ser_handle[ser_USART3]) {
		if ((buffer_get_item(_ser_handle[ser_USART3]->_tx_buf, &item) == BUFFER_OK)) {
			UDR3 = item;
		}
		else
		{
			SERIAL_TX_INT_OFF(UCSR3B);
		}
	}
	
	else
	{
		SERIAL_TX_INT_OFF(UCSR3B);
	}
}
#endif