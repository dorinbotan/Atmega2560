/*
 * board_spec.h
 *
 * Created: 21-10-2015 12:20:18
 *  Author: IHA
 */ 


#ifndef BOARD_SPEC_H_
#define BOARD_SPEC_H_

// CPU
#define F_CPU		16000000L

// SPI
#define SPI_PORT	PORTB
#define SPI_CS_PORT	PORTB
#define SPI_CS		PB0
#define SPI_SCK		PB1
#define SPI_MOSI	PB2
#define SPI_MISO	PB3

// Bluetooth
// USART 0 used for Bluetooth communication
#define BT_RTS_PORT				PORTE
#define BT_RTS_PIN				PE2
#define BT_CTS_PORT				PORTE
#define BT_CTS_PIN				PE5
#define BT_RESET_PORT			PORTA
#define BT_RESET_PIN			PA0
#define BT_AUTO_DISCOVERY_PORT	PORTA
#define BT_AUTO_DISCOVERY_PIN	PA1
#define BT_MASTER_PORT			PORTA
#define BT_MASTER_PIN			PA2

// GOAL LINE + INT0 used

// TACHO - Timer 1 used
#define TACHO_TCCRA_reg			TCCR1A
#define TACHO_TCCRB_reg			TCCR1B
#define TACHO_TCCRC_reg			TCCR1C
#define TACHO_COMA0_bit			COM1A0
#define TACHO_COMA1_bit			COM1A1
#define TACHO_COMB0_bit			COM1B0
#define TACHO_COMB1_bit			COM1B1
#define TACHO_COMC0_bit			COM1C0
#define TACHO_COMC1_bit			COM1C1
#define TACHO_WGM0_bit			WGM10
#define TACHO_WGM1_bit			WGM11
#define TACHO_WGM2_bit			WGM12
#define TACHO_WGM3_bit			WGM13
#define TACHO_CS0_bit			CS10
#define	TACHO_CS1_bit			CS11
#define TACHO_CS2_bit			CS12
#define TACHO_OCRA_reg			OCR1A
#define TACHO_OCRB_reg			OCR1B
#define TACHO_OCRC_reg			OCR1C
#define TACHO_ICR_reg			ICR1
#define TACHO_TIMSK_reg			TIMSK1
#define TACHO_TIFR_reg			TIFR1
#define TACHO_TCNT_reg			TCNT1

// HORN
#define HORN_PORT_reg					PORTC
#define HORN_PIN_bit					PC3

// HEAD LIGHT
#define HEAD_LIGHT_PORT_reg				PORTC
#define HEAD_LIGHT_PIN_bit				PC2

// BRAKE LIGHT
#define BRAKE_LIGHT_PORT_reg			PORTC
#define BRAKE_LIGHT_PIN_bit				PC1

// AUX
#define AUX_PORT_reg					PORTC
#define AUX_PIN_bit						PC0

// MOTOR CONTROL - Timer 3 used
#define MOTOR_CONTROL_TCCRA_reg			TCCR3A
#define MOTOR_CONTROL_TCCRB_reg			TCCR3B
#define MOTOR_CONTROL_TCCRC_reg			TCCR3C
#define MOTOR_CONTROL_COMA0_bit			COM3A0
#define MOTOR_CONTROL_COMA1_bit			COM3A1
#define MOTOR_CONTROL_COMB0_bit			COM3B0
#define MOTOR_CONTROL_COMB1_bit			COM3B1
#define MOTOR_CONTROL_COMC0_bit			COM3C0
#define MOTOR_CONTROL_COMC1_bit			COM3C1
#define MOTOR_CONTROL_WGM0_bit			WGM30
#define MOTOR_CONTROL_WGM1_bit			WGM31
#define MOTOR_CONTROL_WGM2_bit			WGM32
#define MOTOR_CONTROL_WGM3_bit			WGM33
#define MOTOR_CONTROL_CS0_bit			CS30
#define	MOTOR_CONTROL_CS1_bit			CS31
#define MOTOR_CONTROL_CS2_bit			CS32
#define MOTOR_CONTROL_OCRA_reg			OCR3A
#define MOTOR_CONTROL_OCRB_reg			OCR3B
#define MOTOR_CONTROL_OCRC_reg			OCR3C
#define MOTOR_CONTROL_ICR_reg			ICR3
#define MOTOR_CONTROL_TIMSK_reg			TIMSK3
#define MOTOR_CONTROL_TIFR_reg			TIFR3

#define MOTOR_CONTROL_OCA_PORT_reg		PORTE
#define MOTOR_CONTROL_OCA_PIN_bit		PE3
#define MOTOR_CONTROL_OCB_PORT_reg		PORTE
#define MOTOR_CONTROL_OCB_PIN_bit		PE4

// Timer 2 used for generating 100ms ticks for dialog handler in driver 
#define DIALOG_HANDLER_TCCRA_reg			TCCR2A
#define DIALOG_HANDLER_TCCRB_reg			TCCR2B
#define DIALOG_HANDLER_COMA0_bit			COM2A0
#define DIALOG_HANDLER_COMA1_bit			COM2A1
#define DIALOG_HANDLER_COMB0_bit			COM2B0
#define DIALOG_HANDLER_COMB1_bit			COM2B1
#define DIALOG_HANDLER_WGM0_bit				WGM20
#define DIALOG_HANDLER_WGM1_bit				WGM21
#define DIALOG_HANDLER_WGM2_bit				WGM22
#define DIALOG_HANDLER_CS0_bit				CS20
#define	DIALOG_HANDLER_CS1_bit				CS21
#define DIALOG_HANDLER_CS2_bit				CS22
#define DIALOG_HANDLER_OCRA_reg				OCR2A
#define DIALOG_HANDLER_OCRB_reg				OCR2B
#define DIALOG_HANDLER_TIMSK_reg			TIMSK2
#define DIALOG_HANDLER_TIFR_reg				TIFR2
#define DIALOG_HANDLER_TOIE_bit				TOIE2
#define DIALOG_HANDLER_OCIEA_bit			OCIE2A

#endif /* BOARD_SPEC_H_ */