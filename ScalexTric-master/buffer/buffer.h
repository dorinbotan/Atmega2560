/*
 * buffer.h
 *
 * Created: 08-04-2013 12:26:44
  $Rev: 13 $
  $Author: iha $
  $LastChangedDate: 2013-05-22 17:48:03 +0200 (Wed, 22 May 2013) $
 */ 

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>

// Max size 255
#define BUFFER_SIZE 16

/**
   @ingroup buffer_return
   @{
 */
#define BUFFER_OK 0
#define BUFFER_EMPTY 1
#define BUFFER_FULL 2
/**
   @}
 */ 

typedef struct buffer_struct {
	uint8_t storage[BUFFER_SIZE];
	uint8_t in_i;
	uint8_t out_i;
	uint8_t no_in_buffer;
} buffer_struct_t;

void buffer_init(buffer_struct_t *buffer);
uint8_t buffer_get_item(buffer_struct_t *buffer, uint8_t *item);
uint8_t buffer_put_item(buffer_struct_t *buffer, uint8_t item);
uint8_t buffer_is_empty(buffer_struct_t *buffer);
uint8_t buffer_no_of_items(buffer_struct_t *buffer);
void buffer_clear(buffer_struct_t *buffer);

#endif /* BUFFER_H_ */