/*
    FreeRTOS V8.2.1 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

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
#include "queue.h"

/* Priority definitions for most of the tasks in the demo application.  Some
tasks just use the idle priority. */

#define task1_TASK_PRIORITY					( tskIDLE_PRIORITY + 5 )
#define task2_TASK_PRIORITY					( tskIDLE_PRIORITY + 4 )
#define task3_TASK_PRIORITY					( tskIDLE_PRIORITY + 3 )
#define task4_TASK_PRIORITY					( tskIDLE_PRIORITY + 2 )

/*
 * The idle hook is used to scheduler co-routines.
 */
void vApplicationIdleHook( void );

/*-----------------------------------------------------------*/

// Global objects
SemaphoreHandle_t xSemaphore;
QueueHandle_t xQueue1;
TaskHandle_t x1Handle;
TaskHandle_t x2Handle;

void vSend(void *pvParameters) {
	char inp;

	while (1) {
		inp = PINB;
		xQueueSend(xQueue1, &inp, portMAX_DELAY);

		taskYIELD();
	}
	vTaskDelete(NULL);
}

void vReceive(void *pvParamenters) {
	char out;

	while (1) {
		xQueueReceive(xQueue1, &out, portMAX_DELAY);
		PORTA = out;

		vTaskDelay(100);
	}
}

int main( void )
{
	xQueue1 = xQueueCreate(30, sizeof(char));

	if(xQueue1 != NULL)
	{
		DDRA = 0xff;
		DDRB = 0x0;
		PORTA = 0xff;

		xTaskCreate(vSend, "Sender", 1000, NULL, 1, NULL);
		xTaskCreate(vReceive, "Receiver", 1000, NULL, 1, NULL);
	
		vTaskStartScheduler();
	}

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