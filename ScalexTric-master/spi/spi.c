/** @file spi.c
@brief Library to control the SPI communication.
@author Ib Havn
@version 1.0.0

@defgroup spi SPI Driver
@{
The implementation works with interrupt, meaning that there are no busy-waiting involved.

@note Only implemented for Master mode.

@defgroup spi_config SPI Configuration
@brief How to configure the SPI driver.

@defgroup spi_function SPI Functions
@brief Commonly used SPI functions.
Here you you will find the functions you will need.

@defgroup spi_instance spi_new_instance() parameter values.
@brief Predefined values to be used as parameters when calling spi_new_instance().
@{
@defgroup spi_mode Mode parameter values.
@brief Used for mode parameter.

@defgroup spi_clock Clock parameter values.
@brief Used for clock parameter.

@defgroup spi_clock_level Clock level parameter values.
@brief Used for clock idle level.

@defgroup spi_clock_phase Clock phase values.
@brief Used for clock phase.

@defgroup spi_data_order Data order values.
@brief Used for setting the data order.
@}

@defgroup spi_return_codes SPI Return codes
@brief Return values from SPI functions.
@}
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <stdlib.h>

#include "spi.h"

#define CS_INACTIVE 0
#define CS_ACTIVE 1

// Instance data
struct spi_struct{
	volatile uint8_t *_cs_port;
	uint8_t _cs_pin;
	uint8_t _cs_active_level;
	uint8_t _SPCR;
	uint8_t _SPSR;
	
	#if SPI_USE_BUFFER == 1
	buffer_struct_t *_tx_buf;
	buffer_struct_t *_rx_buf;
	#endif

	void(*_call_back )(spi_p,uint8_t);
};

// Module variables

static uint8_t _spi_active = 0; /**< true if the SPI bus is transmitting */
static spi_p _this = 0; /**< the current active instance of the SPI setup. 0 means no instance is active */
static uint8_t _initialised = 0; /**< the spi driver is initialised. 0 means not intialised */

// Mask for Pre-scaler SPR0 and SPR1
// Indexed by SPI_CLOCK_DIVIDER_xx defines
static const uint8_t _prescaler_mask [] = {0b00,0b01,0b10,0b11,0b00,0b01,0b10};

// Send a byte to the SPI-bus
static inline void _spi_send_byte(uint8_t byte) {
	SPDR = byte;
}

// Set the CS according to the instance
static inline void _set_cs(uint8_t state) {
	if (_this->_cs_active_level == 1) {
		if (state == CS_ACTIVE) {
			*(_this->_cs_port) |= _BV(_this->_cs_pin);
			} else {
			*(_this->_cs_port) &= ~_BV(_this->_cs_pin);
		}
		} else {
		if (state == CS_ACTIVE) {
			*(_this->_cs_port) &= ~_BV(_this->_cs_pin);

			} else {
			*(_this->_cs_port) |= _BV(_this->_cs_pin);
		}
	}
}

// Select the instance
static void _select_instance(spi_p inst) {
	// check if any current instance
	if (_this != 0) {
		_set_cs(CS_INACTIVE);
	}
	
	_this = inst;
	
	// are instance valid?
	if (inst != 0) {
		_spi_active = 0;
		// Disable SPI and SPI interrupt before changing setup to new instance
		SPCR &= ~(_BV(SPIE) | _BV(SPE));
		//
		// setup SPI to instance
		SPCR = inst->_SPCR;
		SPSR = inst->_SPSR;
		
		// enable SPI
		SPCR |= _BV(SPE);
	}
}


// Initialise the driver
static void _spi_init() {
	// SS, SCK, MOSI as output
	DDRB |= _BV(DDB0) | _BV(DDB1) |_BV(DDB2);
}


/* ======================================================================================================================= */
/**
@ingroup spi_function
@brief Setup and initialize new SPI instance.

Setup and create SPI instance according to parameters.

@note This must be called exactly once per new SPI instance.
@note Slave mode is not implemented!!!

@return handle to the SPI instance created. Should be used as parameter to the send() functions.

@param mode SPI_MODE_MASTER: Master mode\n
SPI_MODE_SLAVE: Slave Mode.
@param clock_divider defines the SPI clock divider. Use SPI_CLOCK_DIVIDER_xx as parameter.
@param spi_mode defines the SPI mode to use [0..3]. These are the standard modes specified by Motorola\n
|spi_mode | CPOL | CPHA |Clock Behavior|
|:---:    |:----:|:----:|:----------|
| 0       | 0    | 0    |Low when idle, samples on leading edge|
| 1       | 0    | 1    |Low when idle, samples on trailing edge|
| 2       | 1    | 0    |High when idle, samples on leading edge|
| 3       | 1    | 1    |High when idle, samples on trailing edge|

@param data_order SPI_DATA_ORDER_LSB: LSB transmitted first.\n
SPI_DATA_ORDER_MSB: MSB transmitted first.
@param *cs_port port register for the CS pin - Ex. PORTB.
@param *cs_pin that is connected to the slaves CS - Ex. PB6.
@param  cs_active_level 0: CS active when low.\n
1: CS active when high.
@param *rx_buf 0: No receive buffer used, means no data vil be received from slave.\n
pointer to the receive buffer structure to store received data in.
@param *tx_buf 0: No trasmit buffer used, means that only one byte can be transmitted at the time.\n
pointer to the transmit buffer structure to store transmit data in.
0: no tx buffer wanted.
@param *call_back pointer to a call back function that will be called when the driver receives a byte from the SPI bus. The function should have the following signature:\n
@code
void handler_name(spi_p spi_instance, uint8_t rx_byte)
@endcode
0: no call back function will be called.

@note If SPI_USE_BUFFER in spi_iha_config.h is 0 then the pointers rx_buf and tx_buf are ignored.
*/
spi_p spi_new_instance(uint8_t mode, int8_t clock_divider, uint8_t spi_mode, uint8_t data_order, volatile uint8_t *cs_port, uint8_t cs_pin, uint8_t cs_active_level,
buffer_struct_t *rx_buf, buffer_struct_t *tx_buf, void(*call_back )(spi_p spi, uint8_t last_rx_byte))
{
	if (!_initialised) {
		_spi_init();
		_initialised = 1;
	}
	
	spi_p _spi = malloc(sizeof *_spi);
	
	_spi->_cs_port = cs_port;
	_spi->_cs_pin = cs_pin;
	
	// Set CS pin to output
	*(cs_port-1) |= _BV(cs_pin);
	
	_spi->_cs_active_level = cs_active_level;
	_spi->_SPCR = mode | _prescaler_mask[clock_divider] |(spi_mode<<2) | data_order;
	
	if (clock_divider > 3) {
		_spi->_SPSR = _BV(SPI2X);
	}

	#if SPI_USE_BUFFER == 1
	_spi->_tx_buf = tx_buf;
	_spi->_rx_buf = rx_buf;
	#endif

	_spi->_call_back = call_back;

	// Critical section
	{
		// disable interrupt
		uint8_t c_sreg = SREG;
		cli();

		spi_p current_instance = _this;
		_this = _spi;
		_set_cs(CS_INACTIVE);
		_this = current_instance;

		// restore interrupt state
		SREG = c_sreg;
	}

	return _spi;
}

/* ======================================================================================================================= */
/**
@ingroup spi_function
@brief Send a single byte to the SPI bus.

@return \n
SPI_OK: OK byte send to SPI bus.\n
SPI_NO_ROOM_IN_TX_BUFFER: Buffer full no data send.\n
SPI_ILLEGAL_INSTANCE: instance is null.

@param spi to send to.
@param byte to be send.
*/
uint8_t spi_send_byte(spi_p spi, uint8_t byte) {
	if (spi == 0) {
		return SPI_ILLEGAL_INSTANCE;
	}

	// Select correct instance
	if (_this != spi ) {
		_select_instance(spi);
	}

	uint8_t result = SPI_OK;

	// Critical section
	{
		// disable interrupt
		uint8_t c_sreg = SREG;
		cli();

		// If SPI in idle send the first byte
		if (!_spi_active) {
			_spi_active = 1;
			_set_cs(CS_ACTIVE);
			// Enable SPI interrupt
			SPCR |= _BV(SPIE);
			// Send first byte
			SPDR = byte;
		}
		#if SPI_USE_BUFFER == 0
		else {
			result = SPI_BUSY;
		}
		#else
		else {
			// Check if buffer is free
			if ( ((spi->_tx_buf != 0) && (BUFFER_SIZE - spi->_tx_buf->no_in_buffer)) || (spi->_tx_buf == 0) ) {
				// Put in the tx buffer
				buffer_put_item(spi->_tx_buf, byte);
				}else {
				result = SPI_NO_ROOM_IN_TX_BUFFER;
			}
		}
		#endif

		// restore interrupt state
		SREG = c_sreg;
	}

	return result;
}

#if SPI_USE_BUFFER == 1
/* ======================================================================================================================= */
/**
@ingroup spi_function
@brief Send an array of bytes to to the SPI bus.

@note Can only be used if SPI_USE_BUFFER are enabled in spi_iha_defs.h

@see spi_iha_defs.h for SPI_USE_BUFFER setup.

@return SPI_OK: OK byte send to SPI bus or put in tx_buffer.\n
SPI_NO_ROOM_IN_TX_BUFFER: Buffer full no data send\n
SPI_ILLEGAL_INSTANCE: instance is null.
@param spi to send to.
@param *buf pointer to buffer to be send.
@param len no of bytes to send.
*/
uint8_t spi_send_string(spi_p spi, uint8_t *buf, uint8_t len) {
	if (spi == 0) {
		return SPI_ILLEGAL_INSTANCE;
	}

	// Select correct instance
	if (_this != spi ) {
		_select_instance(spi);
	}

	uint8_t result = SPI_OK;
	uint8_t tmp = 0;

	// Critical section
	{
		// disable interrupt
		uint8_t c_sreg = SREG;
		cli();

		// Check if buffer is free
		if ( ((spi->_tx_buf != 0) && (len > (BUFFER_SIZE - spi->_tx_buf->no_in_buffer))) || ((spi->_tx_buf == 0) && (len > 1)) ) {
			result = SPI_NO_ROOM_IN_TX_BUFFER;
			} else {
			// If SPI in idle send the first byte
			if (!_spi_active) {
				_spi_active = 1;
				_set_cs(CS_ACTIVE);
				// Enable SPI interrupt
				SPCR |= _BV(SPIE);
				// Send first byte
				SPDR =buf[0];

				tmp = 1;
			}
			// Put in the tx buffer
			for (uint8_t i = tmp; i < len; i++) {
				buffer_put_item(spi->_tx_buf, buf[i]);
			}
		}

		// restore interrupt state
		SREG = c_sreg;
	}

	return result;
}
#endif

/* ======================================================================================================================= */
/**
@todo Documentation
*/
ISR(SPI_STC_vect) {
	uint8_t item;
	#if SPI_USE_BUFFER == 1
	// store received byte if receive buffer available

	if (_this->_rx_buf != 0){
		buffer_put_item(_this->_rx_buf, SPDR);
	}
	// more bytes to send?
	if ( buffer_get_item(_this->_tx_buf, &item) == BUFFER_OK) {
		_spi_send_byte(item);
		} else {
		// No
		// Disable SPI interrupt
		SPCR &= ~_BV(SPIE);
		_spi_active = 0;
		_set_cs(CS_INACTIVE);
	}

	#else
	// No buffer used
	item = SPDR;
	// Disable SPI interrupt
	SPCR &= ~_BV(SPIE);
	_spi_active = 0;
	_set_cs(CS_INACTIVE);
	#endif

	// If handler defined - call it with instance and received byte.
	if (_this->_call_back)
	{
		_this->_call_back(_this, item);
	}
}