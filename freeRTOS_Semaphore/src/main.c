#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef GCC_MEGA_AVR
	#include <avr/eeprom.h>
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "semphr.h"
#include "queue.h"

void vApplicationIdleHook( void );

SemaphoreHandle_t xSemaphore;
QueueHandle_t xQueue1;
TaskHandle_t x1Handle;
TaskHandle_t x2Handle;

ISR(PCINT0_vect) {
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);	

	if(xHigherPriorityTaskWoken == pdTRUE)
		taskYIELD();
}

void vHandler(void *pvParameters) {
	while (1) {
		xSemaphoreTake(xSemaphore, portMAX_DELAY);

		PORTA ^= 0xff;
		vTaskDelay(1000);
	}
}

int main( void )
{
	DDRA = 0xff;
	PORTA = 0xff;

	DDRB &= ~(1 << DDB0);
	PORTB |= (1 << PINB0);

	PCICR |= _BV(PCIE0);
	PCMSK0 |= _BV(PCINT0);

	sei();

	//vSemaphoreCreateBinary(xSemaphore);
	xSemaphore = xSemaphoreCreateCounting(100, 0);

	xTaskCreate(vHandler, "Handler", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
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