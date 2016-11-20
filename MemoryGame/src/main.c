#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>

#ifdef GCC_MEGA_AVR
	#include <avr/eeprom.h>
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "semphr.h"

#define task1_TASK_PRIORITY					( tskIDLE_PRIORITY + 5 )
#define task2_TASK_PRIORITY					( tskIDLE_PRIORITY + 4 )
#define task3_TASK_PRIORITY					( tskIDLE_PRIORITY + 3 )
#define task4_TASK_PRIORITY					( tskIDLE_PRIORITY + 2 )

typedef int bool;
#define true 1
#define false 0

void vApplicationIdleHook( void );

SemaphoreHandle_t xSemaphore;
QueueHandle_t xQueue1;
TaskHandle_t x1Handle;
TaskHandle_t x2Handle;

uint8_t array[100];
uint8_t click[100];
uint8_t size = 0;

uint8_t getRandom()
{
	uint16_t currentTick = xTaskGetTickCount();
	uint8_t rNumber = currentTick%8;
	uint8_t led = 1<<rNumber;
	return led;
}

void addValue(uint8_t val)
{
	array[size++] = val;
}

void delay(int msec)
{
	for(int i = 0; i < msec; i++)
		for(int j = 0; j < 500; j++){}
}

bool check()
{
	for(int i = 0; i < size; i++)
		if(array[i] != click[i])
			return 0;

	return 1;
}

void vTask1(void *pvParameters) {
	(void) pvParameters;
	
	while (1) {
		if(check())
		{
			PORTA = 0xff;
			delay(1000);
		
			addValue(getRandom());
		
			for(int i = 0; i < size; i++)
			{
				PORTA = ~array[i];
				delay(500);
				PORTA = 0xff;
				delay(500);
			}

			vTaskDelay(3000);
		}
	}
	vTaskDelete(NULL);
}

void vTask2(void *pvParameters) {
	(void) pvParameters;

	while (1) {
		PORTA = PINB;

	}
	
	vTaskDelete(NULL);
}

int main( void )
{
	DDRA = 0xff;
	DDRB = 0x00;

	xTaskCreate(vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
	xTaskCreate(vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

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