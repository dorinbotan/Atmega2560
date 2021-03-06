LDI R16, 0xFF
OUT DDRA, R16
OUT DDRB, R16
LDI R16, 0
OUT DDRF, R16

// Set Aref and input channel
LDI R16, 0b01000000
STS ADMUX, R16

// Enable ADC, choose speed and start conversion
LDI R17, 0b11000111
READ_ADC:
STS ADCSRA, R17																					q

// Wait until the convertion is done
KEEP_POLING:
LDS R16, ADCSRA
SBRS R16, 4
RJMP KEEP_POLING

LDS R16, ADCL
COM R16
OUT PORTA, R16
LDS R16, ADCH
COM R16
OUT PORTB, R16

RJMP READ_ADC