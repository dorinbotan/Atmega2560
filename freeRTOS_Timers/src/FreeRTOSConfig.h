#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <avr/io.h>

#define portUSE_TIMER3											// portUSE_TIMER3 to use 16 bit Timer3

#define configUSE_PREEMPTION		1
#define configUSE_IDLE_HOOK			1
#define configUSE_TICK_HOOK			0
#define configUSE_TIMERS			1
#define configTIMER_TASK_PRIORITY	( 4 )
#define configTIMER_QUEUE_LENGTH	1
#define configTIMER_TASK_STACK_DEPTH configMINIMAL_STACK_SIZE
#define configCPU_CLOCK_HZ			( ( unsigned long ) F_CPU )
#define configTICK_RATE_HZ			( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES		( 4 )
#define configMINIMAL_STACK_SIZE	( ( unsigned short ) 185 )
#define configTOTAL_HEAP_SIZE		( (size_t ) ( 2500 ) )
#define configMAX_TASK_NAME_LEN		( 8 )
#define configUSE_TRACE_FACILITY	0
#define configUSE_16_BIT_TICKS		1
#define configIDLE_SHOULD_YIELD		1
#define configQUEUE_REGISTRY_SIZE	0
#define configCHECK_FOR_STACK_OVERFLOW	1
#define configUSE_COUNTING_SEMAPHORES	1


/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		1
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet		0
#define INCLUDE_uxTaskPriorityGet		0
#define INCLUDE_vTaskDelete				1
#define INCLUDE_vTaskCleanUpResources	0
#define INCLUDE_vTaskSuspend			0
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay				1


#endif /* FREERTOS_CONFIG_H */
