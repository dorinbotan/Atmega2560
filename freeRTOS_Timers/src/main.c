/*
* main.c
*
* Created: 21-10-2015 11:48:30
*  Author: IHA
*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "include/board.h"
/* Scheduler include files. */
#include "FreeRTOS/Source/include/FreeRTOS.h"

#include "task.h"
#include "croutine.h"

#define startup_TASK_PRIORITY				( tskIDLE_PRIORITY )
#define just_a_task_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )


static const uint8_t _BT_RX_QUEUE_LENGTH = 30; 
static SemaphoreHandle_t  goal_line_semaphore = NULL;
static QueueHandle_t _xBT_received_chars_queue = NULL;



uint8_t bt_initialised = 0;

void bt_status_call_back(uint8_t status) {
	if (status == DIALOG_OK_STOP) {
		bt_initialised = 1;
		} else if (status == DIALOG_ERROR_STOP) {
		// What to do??
	}
}

void bt_com_call_back(uint8_t byte) {
	char buf[20];
	
	if (bt_initialised) {
		switch (byte) {
			case 'a': {
				set_head_light(0);
				break;
			}
			
			case 'A': {
				set_head_light(1);
				break;
			}
			
			case 'b': {
				set_brake_light(0);
				break;
			}
			
			case 'B': {
				set_brake_light(1);
				break;
			}
			
			case 'c': {
				set_horn(0);
				break;
			}
			
			case 'C': {
				set_horn(1);
				break;
			}
			
			case 'd': {
				set_motor_speed(0);
				break;
			}
			
			case 'D': {
				set_motor_speed(75);
				break;
			}
			
			case 'e': {
				set_brake(0);
				break;
			}
			
			case 'E': {
				set_brake(100);
				break;
			}
			
			case 'F': {
				uint16_t raw_x = get_raw_x_accel();
				sprintf(buf, "raw-x:%4d", raw_x);
				bt_send_bytes((uint8_t *)buf, strlen(buf));
				break;
			}
			default:;
		}
	}
}

static void vjustATask( void *pvParameters ) {
	/* The parameters are not used. */
	( void ) pvParameters;

	/* Cycle for ever, one cycle each time the goal line is passed. */
	for( ;; )
	{
		// Wait for goal line is passed
		xSemaphoreTake(goal_line_semaphore, portMAX_DELAY);
		set_horn(1);
		vTaskDelay( 200/ portTICK_PERIOD_MS);
		set_horn(0);
	}
}

static void vstartupTask( void *pvParameters ) {
	/* The parameters are not used. */
	( void ) pvParameters;
	
	goal_line_semaphore = xSemaphoreCreateBinary();
	_xBT_received_chars_queue = xQueueCreate( _BT_RX_QUEUE_LENGTH, ( unsigned portBASE_TYPE ) sizeof( uint8_t ) );
	
	if( goal_line_semaphore == NULL ) {
		/* There was insufficient OpenRTOS heap available for the semaphore to
		be created successfully. */
		// What to do here ?????????????????????????????????
		} else {
		set_goal_line_semaphore(goal_line_semaphore);
	}
	
	// Initialize Bluetooth Module
	vTaskDelay( 1000/ portTICK_PERIOD_MS);
	set_bt_reset(0);  // Disable reset line of Blue tooth module
	vTaskDelay( 1000/ portTICK_PERIOD_MS);
	init_bt_module(bt_status_call_back, _xBT_received_chars_queue);
	
	xTaskCreate( vjustATask, "JustATask", configMINIMAL_STACK_SIZE, NULL, just_a_task_TASK_PRIORITY, NULL );
	uint8_t _byte;
	
	for( ;; ) {
		xQueueReceive( _xBT_received_chars_queue, &_byte, portMAX_DELAY );
		bt_com_call_back(_byte);
	}
}

int main(void)
{
	init_main_board();
	xTaskCreate( vstartupTask, "StartupTask", configMINIMAL_STACK_SIZE, NULL, startup_TASK_PRIORITY, NULL );
	vTaskStartScheduler();
}


// Called is TASK Stack overflows
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
	set_horn(1);
}
