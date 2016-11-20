/*
* main.c
*
* Created: 21-10-2015 11:48:30
*  Author: DBV
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

static void vstartupTask( void *pvParameters ) {
	/* The parameters are not used. */
	( void ) pvParameters;

	int tmp = 32700;
	int tmp1 = 0 - tmp;

	while(1)
	{
		if (get_raw_y_accel() > tmp || get_raw_y_accel() < tmp1)
			set_motor_speed(100);
		else
			set_motor_speed(75);
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