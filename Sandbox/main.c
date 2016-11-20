
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>

#ifdef GCC_MEGA_AVR
	/* EEPROM routines used only with the WinAVR compiler. */
	#include <avr/eeprom.h>
#endif

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "semphr.h"


/* Priority definitions for most of the tasks in the demo application.  Some
tasks just use the idle priority. */

#define task1_TASK_PRIORITY					( tskIDLE_PRIORITY + 5 )
#define task2_TASK_PRIORITY					( tskIDLE_PRIORITY + 4 )
#define task3_TASK_PRIORITY					( tskIDLE_PRIORITY + 3 )
#define task4_TASK_PRIORITY					( tskIDLE_PRIORITY + 2 )


/*
 * Checks the unique counts of other tasks to ensure they are still operational.
 * Flashes an LED if everything is okay.
 */

/*
 * Called on boot to increment a count stored in the EEPROM.  This is used to
 * ensure the CPU does not reset unexpectedly.
 */
//static void prvIncrementResetCount( void );

/*
 * The idle hook is used to scheduler co-routines.
 */
void vApplicationIdleHook( void );


// Global objects
SemaphoreHandle_t xSemaphore;
QueueHandle_t xQueue1;
TaskHandle_t x1Handle;
TaskHandle_t x2Handle;

void vTask1(void *pvParameters) {
	// Remove compiler warnings.
	(void) pvParameters;
	
	while (1) {
		PORTA = 0xfb;
		vTaskDelay(2000);		
	}
	vTaskDelete(NULL);
}

void vTask2(void *pvParameters) {
	// Remove compiler warnings.
	(void) pvParameters;
	vTaskDelay(1000);
	while (1) {
		PORTA = 0xff;
		vTaskDelay(2000);
	}
	vTaskDelete(NULL);
}

int main( void )
{
	DDRA = 0xff;

	xTaskCreate(vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL,
	task1_TASK_PRIORITY, NULL);
	xTaskCreate(vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL,
	task2_TASK_PRIORITY, NULL);

	vTaskStartScheduler();

	return 0;
}

void vApplicationIdleHook( void )
{
	vCoRoutineSchedule();
}

ISR(BADISR_vect)
{
	PORTA &= ~_BV(PA1);
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
	PORTA |= _BV(PA7);
}