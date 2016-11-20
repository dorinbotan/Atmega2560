/*! @file board.c */

/* ################################################## Standard includes ################################################# */
#include <avr/io.h>
#include <avr/interrupt.h>
/* ################################################### Project includes ################################################# */
#include "../FreeRTOS/Source/include/FreeRTOS.h"
#include "../FreeRTOS/Source/include/task.h"
#include "../FreeRTOS/Source/include/semphr.h"

#include "board_spec.h"
#include "../include/board.h"
#include "../spi/spi.h"
#include "../serial/serial.h"
#include "../dialog_handler/dialog_handler.h"
/* ################################################### Global Variables ################################################# */
/* ############################################ Module Variables/Declarations ########################################### */
#define MOTOR_CONTROL_PWM_FREQ	5000L
#define MOTOR_CONTROL_PRESCALER	1L
#define MOTOR_CONTROL_TOP		(F_CPU/(MOTOR_CONTROL_PWM_FREQ * MOTOR_CONTROL_PRESCALER)-1L)

#define DIALOG_HANDLER_FREQ			100L  // Scaled down to 10 Hz in ISR
#define DIALOG_HANDLER_PRESCALER	1024L
#define DIALOG_HANDLER_TOP		(F_CPU/(DIALOG_HANDLER_FREQ * DIALOG_HANDLER_PRESCALER)-1L)

// MPU-9250 Gyro/Acc definitions
// Read bit or to register address to read from it
#define	MPU9520_READ				0x80

// MPU-9250 Registers
#define MPU9520_GYRO_CONFIG_REG		0x1B
#define MPU9520_ACCEL_CONFIG_REG	0x1C
#define MPU9520_ACCEL_XOUT_H_REG	0x3B
#define MPU9520_GYRO_XOUT_H_REG		0x43

#define GYRO_FULL_SCALE_250_DPS		0x00
#define GYRO_250_DPS_DIVIDER		131.0
#define GYRO_FULL_SCALE_500_DPS		0x08
#define GYRO_500_DPS_DIVIDER		65.5
#define GYRO_FULL_SCALE_1000_DPS	0x10
#define GYRO_1000_DPS_DIVIDER		32.8
#define GYRO_FULL_SCALE_2000_DPS	0x18
#define GYRO_2000_DPS_DIVIDER		16.4

#define ACC_FULL_SCALE_2_G			0x00
#define ACC_2G_DIVIDER				16384
#define ACC_FULL_SCALE_4_G			0x08
#define ACC_4G_DIVIDER				8192
#define ACC_FULL_SCALE_8_G			0x10
#define ACC_8G_DIVIDER				4096
#define ACC_FULL_SCALE_16_G			0x18
#define ACC_16G_DIVIDER				2048

typedef enum {_mpu9520_init_acc,_mpu9520_init_gyro, _mpu9520_poll_acc, _mpu9520_read_acc, _mpu9520_poll_gyro, _mpu9520_read_gyro} _mpu9520_states_t;

// Handle to SPI
static spi_p _spi_mpu9520 = 0;
static serial_p _bt_serial_instance = 0;

// mpu9520
static buffer_struct_t _mpu9520_rx_buffer;
static buffer_struct_t _mpu9250_tx_buffer;

static int16_t _x_acc = 0;
static int16_t _y_acc = 0;
static int16_t _z_acc = 0;

static int16_t _x_gyro = 0;
static int16_t _y_gyro = 0;
static int16_t _z_gyro = 0;

// dialog sequences to setup BT module
typedef enum { eENTER_CMD0=0, eENTER_CMD1, eAUTHENTICATION, eNAME, eREBOOT1, eREBOOT2 } en_init_dialog_states;
// BT dialog
dialog_seq_t _dialog_bt_init_seq[] = {
	{ 0, LEN(0), (uint8_t *)"DUMMY", LEN(5), TO(1), eENTER_CMD1, eENTER_CMD1, DIALOG_NO_BUFFER },  // eENTER_CMD0: Enter command mode
	{ (uint8_t *)"$$$", LEN(3), (uint8_t *)"CMD\x0D\x0A",LEN(5), TO(10), eAUTHENTICATION, DIALOG_ERROR_STOP, DIALOG_NO_BUFFER },  // eENTER_CMD1
	{ (uint8_t *)"SA,1\x0D", LEN(5), (uint8_t *)"AOK\x0D\x0A",LEN(5), TO(10), eNAME, DIALOG_ERROR_STOP, DIALOG_NO_BUFFER },  // eAUTHENTICATION: Set to mode 1
	{ (uint8_t *)"S-,VIA-Car\x0D", LEN(11), (uint8_t *)"AOK\x0D\x0A",LEN(5), TO(10), eREBOOT1, DIALOG_ERROR_STOP, DIALOG_NO_BUFFER },  // eNAME: Set device name
	{ (uint8_t *)"R,1\x0D", LEN(4), (uint8_t *)"Reboot!\x0D\x0A",LEN(9), TO(10), eREBOOT2, DIALOG_ERROR_STOP, DIALOG_NO_BUFFER },  // eREBOOT1: Send R,1
	{ 0, LEN(0), (uint8_t *)"DUMMY",LEN(5), TO(10), DIALOG_OK_STOP, DIALOG_OK_STOP, DIALOG_NO_BUFFER },  // eREBOOT2: Just a pause to wait for Reboot
};

static uint8_t _bt_dialog_active = 0;
// Pointer to Application BT call back functions
static void (*_app_bt_status_call_back)(uint8_t result) = NULL;
static QueueHandle_t _xRxedCharsQ = NULL;

// Semaphore to be given when the goal line is passed.
static SemaphoreHandle_t  _goal_line_semaphore = NULL;

/* ################################################# Function prototypes ################################################ */
static void _init_mpu9520();
static void _mpu9250_write_2_reg(uint8_t reg, uint8_t value);
static void _mpu9250_call_back(spi_p spi_instance, uint8_t spi_last_received_byte);
static void _bt_call_back(serial_p _bt_serial_instance, uint8_t serial_last_received_byte);
static void	_init_dialog_handler_timer();

// ----------------------------------------------------------------------------------------------------------------------
void init_main_board() {
	// HORN
	*(&HORN_PORT_reg - 1) |= _BV(HORN_PIN_bit); // set pin to output

	// HEAD LIGHT
	*(&HEAD_LIGHT_PORT_reg - 1) |= _BV(HEAD_LIGHT_PIN_bit); // set pin to output

	// BRAKE LIGHT
	*(&BRAKE_LIGHT_PORT_reg - 1) |= _BV(BRAKE_LIGHT_PIN_bit); // set pin to output
	
	// AUX
	*(&AUX_PORT_reg - 1) |= _BV(AUX_PIN_bit); // set pin to output
	
	// MOTOR SPEED Fast-PWM ICR = TOP setup
	// Mode 14
	MOTOR_CONTROL_TCCRA_reg |= _BV(MOTOR_CONTROL_WGM1_bit);
	MOTOR_CONTROL_TCCRB_reg |= _BV(MOTOR_CONTROL_WGM2_bit) | _BV(MOTOR_CONTROL_WGM3_bit);
	// Set OCA on compare match and clear on BOTTOM
	MOTOR_CONTROL_OCRA_reg = MOTOR_CONTROL_TOP;
	MOTOR_CONTROL_TCCRA_reg |= _BV(MOTOR_CONTROL_COMA1_bit) | _BV(MOTOR_CONTROL_COMA0_bit);
	*(&MOTOR_CONTROL_OCA_PORT_reg - 1) |= _BV(MOTOR_CONTROL_OCA_PIN_bit); // set pin to output
	// Set OCB on compare match and clear on BOTTOM
	MOTOR_CONTROL_OCRB_reg = MOTOR_CONTROL_TOP;
	MOTOR_CONTROL_TCCRA_reg |= _BV(MOTOR_CONTROL_COMB1_bit) | _BV(MOTOR_CONTROL_COMB0_bit);
	*(&MOTOR_CONTROL_OCB_PORT_reg - 1) |= _BV(MOTOR_CONTROL_OCB_PIN_bit); // set pin to output
	// PWM Freq set to 25 KHz
	// TOP = F_CPU/(Fpwm*N)-1
	MOTOR_CONTROL_ICR_reg = MOTOR_CONTROL_TOP;
	// Set prescaler and start timer
	#if (MOTOR_CONTROL_PRESCALER == 1)
	MOTOR_CONTROL_TCCRB_reg |= _BV(MOTOR_CONTROL_CS0_bit);    // Prescaler 1 and Start Timer
	#elif ((MOTOR_CONTROL_PRESCALER == 8))
	MOTOR_CONTROL_TCCRB_reg |= _BV(MOTOR_CONTROL_CS1_bit);    // Prescaler 8 and Start Timer
	#elif ((MOTOR_CONTROL_PRESCALER == 64))
	MOTOR_CONTROL_TCCRB_reg |= _BV(MOTOR_CONTROL_CS0_bit) | _BV(MOTOR_CONTROL_CS1_bit);    // Prescaler 64 and Start Timer
	#elif ((MOTOR_CONTROL_PRESCALER == 256))
	MOTOR_CONTROL_TCCRB_reg |= _BV(MOTOR_CONTROL_CS2_bit);    // Prescaler 256 and Start Timer
	#elif ((MOTOR_CONTROL_PRESCALER == 1024))
	MOTOR_CONTROL_TCCRB_reg |= _BV(MOTOR_CONTROL_CS0_bit) | _BV(MOTOR_CONTROL_CS2_bit); ;    // Prescaler 1024 and Start Timer
	#endif
	set_motor_speed(0);
	
	// TACHO Counter
	// External Clock source - Falling Edge
	TACHO_TCCRB_reg |= _BV(TACHO_CS2_bit) | _BV(TACHO_CS1_bit);
	
	// Bluetooth
	*(&BT_RTS_PORT - 1) &= ~_BV(BT_RTS_PIN); // set pin to input
	*(&BT_CTS_PORT - 1) |= _BV(BT_CTS_PIN); // set pin to output
	BT_CTS_PORT &= ~_BV(BT_CTS_PIN); // Set CTS pin Low
	
	*(&BT_RESET_PORT - 1) |= _BV(BT_RESET_PIN); // set pin to output
	BT_RESET_PORT &= ~_BV(BT_RESET_PIN); // Set RESET Low/active
	
	*(&BT_AUTO_DISCOVERY_PORT - 1) |= _BV(BT_AUTO_DISCOVERY_PIN); // set pin to output
	BT_AUTO_DISCOVERY_PORT &= ~_BV(BT_AUTO_DISCOVERY_PIN); // Set AUTO_DISCOVERY Low/disable
	
	*(&BT_MASTER_PORT - 1) |= _BV(BT_MASTER_PIN); // set pin to output
	BT_MASTER_PORT &= ~_BV(BT_MASTER_PIN); // Set BT_MASTER Low/client mode
	
	// Enable goal-line interrupt - INT0 falling edge
	EICRA |= _BV(ISC01);
	EIMSK |= _BV(INT0);
	
	static buffer_struct_t _bt_rx_buffer;
	static buffer_struct_t _bt_tx_buffer;
	buffer_init(&_bt_rx_buffer);
	buffer_init(&_bt_tx_buffer);
	_bt_serial_instance = serial_new_instance(ser_USART0, 57000UL, ser_BITS_8, ser_STOP_1, ser_NO_PARITY, &_bt_rx_buffer, &_bt_tx_buffer, _bt_call_back);
	
	_init_mpu9520();
	_init_dialog_handler_timer();
}

// ----------------------------------------------------------------------------------------------------------------------
void set_horn(uint8_t state){
	if (state) {
		HORN_PORT_reg |= _BV(HORN_PIN_bit);
	}
	else {
		HORN_PORT_reg &= ~_BV(HORN_PIN_bit);
	}
}

// ----------------------------------------------------------------------------------------------------------------------
void set_head_light(uint8_t state){
	if (state) {
		HEAD_LIGHT_PORT_reg |= _BV(HEAD_LIGHT_PIN_bit);
	}
	else {
		HEAD_LIGHT_PORT_reg &= ~_BV(HEAD_LIGHT_PIN_bit);
	}
}

// ----------------------------------------------------------------------------------------------------------------------
void set_brake_light(uint8_t state){
	if (state) {
		BRAKE_LIGHT_PORT_reg |= _BV(BRAKE_LIGHT_PIN_bit);
	}
	else {
		BRAKE_LIGHT_PORT_reg &= ~_BV(BRAKE_LIGHT_PIN_bit);
	}
}

// ----------------------------------------------------------------------------------------------------------------------
void set_motor_speed(uint8_t speed_percent){
	if (speed_percent > 100) {
		speed_percent = 100;
	}
	
	if  (speed_percent > 0) {
		MOTOR_CONTROL_OCRA_reg = speed_percent*MOTOR_CONTROL_TOP/100;
		MOTOR_CONTROL_OCRB_reg = speed_percent*MOTOR_CONTROL_TOP/100;
		} else { // Free Run
		MOTOR_CONTROL_OCRA_reg = 0;
		MOTOR_CONTROL_OCRB_reg = 0;
	}
}

// ----------------------------------------------------------------------------------------------------------------------
void set_brake(uint8_t brake_percent) {
	if (brake_percent > 100) {
		brake_percent = 100;
	}
	
	if  (brake_percent > 0) {
		MOTOR_CONTROL_OCRA_reg = brake_percent*MOTOR_CONTROL_TOP/100;
		MOTOR_CONTROL_OCRB_reg = 0;
		} else { // Free Run
		MOTOR_CONTROL_OCRA_reg = 0;
		MOTOR_CONTROL_OCRB_reg = 0;
	}
}

// ----------------------------------------------------------------------------------------------------------------------
float get_x_accel() {
	uint8_t _sreg = SREG;
	cli();
	float _tmp = ((float)_x_acc)/ACC_2G_DIVIDER;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
float get_y_accel() {
	uint8_t _sreg = SREG;
	cli();
	float _tmp = ((float)_y_acc)/ACC_2G_DIVIDER;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
float get_z_accel() {
	uint8_t _sreg = SREG;
	cli();
	float _tmp = ((float)_z_acc)/ACC_2G_DIVIDER;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
int16_t get_raw_x_accel() {
	uint8_t _sreg = SREG;
	cli();
	int16_t _tmp = _x_acc;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
int16_t get_raw_y_accel() {
	uint8_t _sreg = SREG;
	cli();
	int16_t _tmp = _y_acc;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
int16_t get_raw_z_accel() {
	uint8_t _sreg = SREG;
	cli();
	int16_t _tmp = _z_acc;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
float get_x_rotation() {
	uint8_t _sreg = SREG;
	cli();
	float _tmp = ((float)_x_gyro)/GYRO_500_DPS_DIVIDER;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
float get_y_rotation() {
	uint8_t _sreg = SREG;
	cli();
	float _tmp = ((float)_y_gyro)/GYRO_500_DPS_DIVIDER;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
float get_z_rotation() {
	uint8_t _sreg = SREG;
	cli();
	float _tmp = ((float)_z_gyro)/GYRO_500_DPS_DIVIDER;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
int16_t get_raw_x_rotation() {
	uint8_t _sreg = SREG;
	cli();
	int16_t _tmp = _x_gyro;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
int16_t get_raw_y_rotation() {
	uint8_t _sreg = SREG;
	cli();
	int16_t _tmp = _y_gyro;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
int16_t get_raw_z_rotation() {
	uint8_t _sreg = SREG;
	cli();
	int16_t _tmp = _z_gyro;
	SREG = _sreg;
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
void _init_mpu9520() {
	buffer_init(&_mpu9520_rx_buffer);
	buffer_init(&_mpu9250_tx_buffer);
	_spi_mpu9520 = spi_new_instance(SPI_MODE_MASTER, SPI_CLOCK_DIVIDER_128, 3, SPI_DATA_ORDER_MSB, &PORTB, PB0, 0,	&_mpu9520_rx_buffer, &_mpu9250_tx_buffer, &_mpu9250_call_back);
	_mpu9250_call_back(0,0);
}

// ----------------------------------------------------------------------------------------------------------------------
static void _mpu9250_write_2_reg(uint8_t reg, uint8_t value) {
	uint8_t buf[2];
	buf[0] = reg;
	buf[1] = value;

	spi_send_string(_spi_mpu9520, buf , 2);
}

// ----------------------------------------------------------------------------------------------------------------------
static void _mpu9250_read_reg(uint8_t reg, uint8_t no_of_bytes)
{
	uint8_t send[no_of_bytes+1];
	send[0] = MPU9520_READ | reg;
	for (uint8_t i = 1; i <= no_of_bytes; i++)
	{
		send[i] = 0;
	}
	
	buffer_clear(&_mpu9520_rx_buffer);
	spi_send_string(_spi_mpu9520, send , no_of_bytes+1);
}

// ----------------------------------------------------------------------------------------------------------------------
static void _poll_acc() {
	_mpu9250_read_reg(MPU9520_ACCEL_XOUT_H_REG ,6);
}

// ----------------------------------------------------------------------------------------------------------------------
static void _poll_gyro() {
	_mpu9250_read_reg(MPU9520_GYRO_XOUT_H_REG ,6);
}
// ----------------------------------------------------------------------------------------------------------------------
static void _mpu9250_call_back(spi_p spi_instance, uint8_t spi_last_received_byte)
{
	uint8_t lsb, msb;
	static _mpu9520_states_t state = _mpu9520_init_acc;

	switch (state)
	{
		case _mpu9520_init_acc:
		{
			// Setup Acc
			state = _mpu9520_init_gyro;
			_mpu9250_write_2_reg(MPU9520_ACCEL_CONFIG_REG, ACC_FULL_SCALE_2_G);
			break;
		}
		
		case _mpu9520_init_gyro:
		{
			if (buffer_no_of_items(&_mpu9520_rx_buffer) == 2) {
				// Setup Gyro
				state = _mpu9520_poll_acc;
				buffer_clear(&_mpu9520_rx_buffer);
				_mpu9250_write_2_reg(MPU9520_GYRO_CONFIG_REG, GYRO_FULL_SCALE_500_DPS);
			}
			break;
		}

		case _mpu9520_poll_acc:
		{
			if (buffer_no_of_items(&_mpu9520_rx_buffer) == 2) {
				state = _mpu9520_read_acc;
				buffer_clear(&_mpu9520_rx_buffer);
				_poll_acc();
			}
			break;
		}

		case _mpu9520_read_acc:
		{
			if (buffer_no_of_items(&_mpu9520_rx_buffer) == 7) {
				uint8_t _sreg = SREG;
				cli();
				buffer_get_item(&_mpu9520_rx_buffer, &lsb); // Throw away the command response
				buffer_get_item(&_mpu9520_rx_buffer, &msb);
				buffer_get_item(&_mpu9520_rx_buffer, &lsb);
				_x_acc = (msb << 8) | lsb;
				buffer_get_item(&_mpu9520_rx_buffer, &msb);
				buffer_get_item(&_mpu9520_rx_buffer, &lsb);
				_y_acc = (msb << 8) | lsb;
				buffer_get_item(&_mpu9520_rx_buffer, &msb);
				buffer_get_item(&_mpu9520_rx_buffer, &lsb);
				SREG = _sreg;
				_z_acc = (msb << 8) | lsb;
				
				state = _mpu9520_read_gyro;
				_poll_gyro();
			}
			break;
		}

		case _mpu9520_read_gyro:
		{
			if (buffer_no_of_items(&_mpu9520_rx_buffer) == 7) {
				uint8_t _sreg = SREG;
				cli();
				buffer_get_item(&_mpu9520_rx_buffer, &lsb); // Throw away the command response
				buffer_get_item(&_mpu9520_rx_buffer, &msb);
				buffer_get_item(&_mpu9520_rx_buffer, &lsb);
				_x_gyro = (msb << 8) | lsb;
				buffer_get_item(&_mpu9520_rx_buffer, &msb);
				buffer_get_item(&_mpu9520_rx_buffer, &lsb);
				_y_gyro = (msb << 8) | lsb;
				buffer_get_item(&_mpu9520_rx_buffer, &msb);
				buffer_get_item(&_mpu9520_rx_buffer, &lsb);
				SREG = _sreg;
				_z_gyro = (msb << 8) | lsb;

				state = _mpu9520_read_acc;
				_poll_acc();
			}
			break;
		}

		default:
		break;
	}
}

// ----------------------------------------------------------------------------------------------------------------------
uint16_t get_tacho_count() {
	static uint16_t _last_reading = 0;
	
	uint16_t _tmp = TACHO_TCNT_reg;
	uint16_t _tmp_last = _tmp;
	
	if (_tmp < _last_reading) {
		_tmp = (UINT16_MAX - _last_reading + _tmp);
		} else {
		_tmp = _tmp-_last_reading;
	}
	_last_reading = _tmp_last;
	
	return _tmp;
}

// ----------------------------------------------------------------------------------------------------------------------
void set_bt_reset(uint8_t state) {
	if (state) {
		BT_RESET_PORT &= ~_BV(BT_RESET_PIN); // Set RESET low/active
		} else {
		BT_RESET_PORT |= _BV(BT_RESET_PIN); // Set RESET high/in-active
	}
}

// ----------------------------------------------------------------------------------------------------------------------
static void _send_bytes_to_bt(uint8_t *bytes, uint8_t len) {
	serial_send_bytes(_bt_serial_instance, bytes, len);
}

// ----------------------------------------------------------------------------------------------------------------------
uint8_t bt_send_bytes(uint8_t *bytes, uint8_t len) {
	return serial_send_bytes(_bt_serial_instance, bytes, len);
}

// ----------------------------------------------------------------------------------------------------------------------
void _bt_status_call_back(uint8_t result) {
	_bt_dialog_active = 0;
	if (_app_bt_status_call_back) {
		_app_bt_status_call_back(result);
	}
}

// ----------------------------------------------------------------------------------------------------------------------
void init_bt_module(void (*bt_status_call_back)(uint8_t result), QueueHandle_t RX_Que) {
	_xRxedCharsQ = RX_Que;
	_app_bt_status_call_back = bt_status_call_back;
	_bt_dialog_active = 1;
	dialog_start(_dialog_bt_init_seq, _send_bytes_to_bt, _bt_status_call_back);
}

// ----------------------------------------------------------------------------------------------------------------------
static void _bt_call_back(serial_p _bt_serial_instance, uint8_t serial_last_received_byte) {
	if (_bt_dialog_active) {
		dialog_byte_received(serial_last_received_byte);
		} else {
		if (_xRxedCharsQ) {
			signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

			xQueueSendFromISR( _xRxedCharsQ, &serial_last_received_byte, &xHigherPriorityTaskWoken );

			if( xHigherPriorityTaskWoken != pdFALSE )
			{
				taskYIELD();
			}
		}
	}
}

// ----------------------------------------------------------------------------------------------------------------------
void set_goal_line_semaphore(SemaphoreHandle_t goal_line_semaphore) {
	if (goal_line_semaphore) {
		_goal_line_semaphore = goal_line_semaphore;
	}
}

ISR(INT0_vect) {
	static signed portBASE_TYPE _higher_priority_task_woken;
	if (_goal_line_semaphore) {
		_higher_priority_task_woken = pdFALSE;

		xSemaphoreGiveFromISR(_goal_line_semaphore, &_higher_priority_task_woken);
		
		if (_higher_priority_task_woken != pdFALSE) {
			portYIELD();
		}
	}
}

// ----------------------------------------------------------------------------------------------------------------------
static void _init_dialog_handler_timer() {
	DIALOG_HANDLER_OCRA_reg = DIALOG_HANDLER_TOP;
	DIALOG_HANDLER_TCCRB_reg |= _BV(DIALOG_HANDLER_WGM1_bit); // CTC Mode
	DIALOG_HANDLER_TIMSK_reg |= _BV(DIALOG_HANDLER_OCIEA_bit); // Enable Compare A interrupt
	
	#if (DIALOG_HANDLER_PRESCALER == 1)
	DIALOG_HANDLER_TCCRB_reg |= _BV(DIALOG_HANDLER_CS0_bit);    // Prescaler 1 and Start Timer
	#elif ((DIALOG_HANDLER_PRESCALER == 8))
	DIALOG_HANDLER_TCCRB_reg |= _BV(DIALOG_HANDLER_CS1_bit);    // Prescaler 8 and Start Timer
	#elif ((DIALOG_HANDLER_PRESCALER == 64))
	DIALOG_HANDLER_TCCRB_reg |= _BV(DIALOG_HANDLER_CS0_bit) | _BV(DIALOG_HANDLER_CS1_bit);    // Prescaler 64 and Start Timer
	#elif ((DIALOG_HANDLER_PRESCALER == 256))
	DIALOG_HANDLER_TCCRB_reg |= _BV(DIALOG_HANDLER_CS2_bit);    // Prescaler 256 and Start Timer
	#elif ((DIALOG_HANDLER_PRESCALER == 1024))
	DIALOG_HANDLER_TCCRB_reg |= _BV(DIALOG_HANDLER_CS0_bit) | _BV(DIALOG_HANDLER_CS2_bit); ;    // Prescaler 1024 and Start Timer
	#endif
}

ISR(TIMER2_COMPA_vect) {
	static uint8_t _count = 10;
	if (_bt_dialog_active) {
		if (--_count == 0) {
			_count = 10;
			dialog_tick();
		}
	}
}
