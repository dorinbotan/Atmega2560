/*! @file buffer.c
  $Rev: 13 $
  $Author: iha $
  $LastChangedBy: iha $
  $LastChangedDate: 2013-05-22 17:48:03 +0200 (Wed, 22 May 2013) $
  @defgroup buffer Circular Buffer
  @{
  @brief A simple implementation of a circular buffer.
  
  This is a simple implementation of a circular buffer that can hold uint8_t data.
  It is implemented as a FIFO.
  The size of the buffer can be set in the header file @code buffer.h @endcode
  @note Maximum size = 255.
  @note The functions are NOT protected against interrupts!
	 
  @defgroup buffer_init Buffer Initialization
  @brief How to initialize the buffer.
 
	 
  @defgroup buffer_function Buffer Functions
  @brief Commonly used buffer functions.
 
  Here you you will find the functions you will need.
   
  @defgroup buffer_return Buffer Return codes
  @brief Codes returned from buffer functions.
 @}
 */

#include "buffer.h"

#define INCREMENT(x) ( (x+1) % BUFFER_SIZE ) /**< Increment % BUFFER_SIZE */

/********************************************//**
 @ingroup buffer_init
 @brief Buffer initialization.

 Initialize the buffer structure to be used for the particular buffer.
 
 Example:
 @code
 #include "buffer.h"
 // Declare a buffer structure to be used as receive buffer for SPI.
 buffer_struct_t spi_rx_buffer;
 // Initialize the buffer
 buffer_init(&spi_rx_buffer);
 
 // Now the buffer can be used
 // Put 7 into the buffer
 buffer_put_item(&spi_rx_buffer, 7);
  @endcode

 @note The buffer structure must be initialized before any of the buffer functions must be called.
 @param *buffer Pointer to the buffer structure to be used.
 ***********************************************/
void buffer_init(buffer_struct_t *buffer) {
	buffer->in_i = 0;
	buffer->out_i = 0;
	buffer->no_in_buffer = 0;
}

/********************************************//**
 @ingroup buffer_function
 @brief Get an item from the buffer.

 @return BUFFER_OK: item removed from buffer and returned in item.\n
    BUFFER_EMPTY: The buffer is empty, item is not updated.
 @param *buffer pointer to the buffer structure.
 @param *item pointer to the variable where the value of the item is returned.
 ***********************************************/
uint8_t buffer_get_item(buffer_struct_t *buffer, uint8_t *item) {
	if (buffer->no_in_buffer > 0) {
		*item = buffer->storage[buffer->out_i];
		buffer->out_i = INCREMENT(buffer->out_i);
		buffer->no_in_buffer--;
		return BUFFER_OK;
	}
	return BUFFER_EMPTY;
}

/********************************************//**
 @ingroup buffer_function
 @brief Put an item into the buffer.

 @return BUFFER_OK: item stored in the buffer.\n
    BUFFER_FULL: The buffer is full, item is not stored.
 @param *buffer pointer to the buffer structure.
 @param item to be stored in the buffer.
 ***********************************************/
uint8_t buffer_put_item(buffer_struct_t *buffer, uint8_t item) {
	if (buffer->no_in_buffer<BUFFER_SIZE) {
		buffer->storage[buffer->in_i] = item;
		buffer->in_i = INCREMENT(buffer->in_i);
		buffer->no_in_buffer++;
		return BUFFER_OK;
	}
	return BUFFER_FULL;
}

/********************************************//**
 @ingroup buffer_function
 @brief Test if the buffer is empty.

 @return true if buffer is empty.
 @param *buffer pointer to the buffer structure.
 ***********************************************/
uint8_t buffer_is_empty(buffer_struct_t *buffer) {
	return (buffer->no_in_buffer == 0);
}

/********************************************//**
 @ingroup buffer_function
 @brief Returns the no of items in the buffer.

 @return no of items in the buffer.
 @param *buffer pointer to the buffer structure.
 ***********************************************/
uint8_t buffer_no_of_items(buffer_struct_t *buffer) {
	return buffer->no_in_buffer;
}

/**********************************************************************//**
 @ingroup buffer_function
 @brief Clear the content of the buffer.

 @param *buffer pointer to the buffer structure.
 **********************************************************************/
void buffer_clear(buffer_struct_t *buffer) {
	buffer->in_i = 0;
	buffer->out_i = 0;
	buffer->no_in_buffer = 0;
}