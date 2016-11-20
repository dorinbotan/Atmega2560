/*! @file serial.h
@brief Serial driver for the USART0.

@author iha

@defgroup  serial_driver Driver for ATMEGA52561 USART0.
@{
@brief A driver for USART0.

@note Only implemented for USART0!!!!
@note Only implemented for 8,N,1 Data format!!!!

@}
*/
#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#include "../buffer/buffer.h"

// Abstract Data Type (ADT)
typedef struct serial_struct *serial_p;

typedef enum
{
	ser_USART0 = 0,
	ser_USART1,
	ser_USART2,
	ser_USART3
} e_com_port_t;

typedef enum
{
	ser_NO_PARITY,
	ser_ODD_PARITY,
	ser_EVEN_PARITY,
	ser_MARK_PARITY,
	ser_SPACE_PARITY
} e_parity_t;

typedef enum
{
	ser_STOP_1,
	ser_STOP_2
} e_stop_bit_t;

typedef enum
{
	ser_BITS_5,
	ser_BITS_6,
	ser_BITS_7,
	ser_BITS_8
} e_data_bit_t;

/* ======================================================================================================================= */
/**
@ingroup serial_driver

@todo Documentation

*/
serial_p serial_new_instance(e_com_port_t com_port, uint32_t baud, e_data_bit_t data_bit, e_stop_bit_t stop_bit, e_parity_t parity, buffer_struct_t *rx_buf, buffer_struct_t *tx_buf, void(*handler_call_back )(serial_p, uint8_t));
/* ======================================================================================================================= */
/**
@ingroup serial_driver

@todo Documentation

*/
uint8_t serial_send_bytes(serial_p handle, uint8_t *buf, uint8_t len);
/* ======================================================================================================================= */
/**
@ingroup serial_driver

@todo Documentation

*/
uint8_t serial_send_byte(serial_p handle, uint8_t byte);

#endif

