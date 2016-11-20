/*
 * Calculator.asm
 *
 *  Created: 10-09-2015 11:01:58
 *   Author: Dorin
 */

.def INPUT = R19	   ; Value currently introduced by the user
.def NUM0 = R16		   ; First operand
.def NUM1 = R17		   ; Second operand
.def FLAG = R31		   ; Flag 

LDI R20, 0b00000001	   ; Registers used to flip bits
LDI R21, 0b00000010
LDI R22, 0b00000100
LDI R23, 0b00001000
LDI R24, 0b00010000
LDI R25, 0b00100000
LDI R26, 0b01000000
LDI R27, 0b00000000	   ; Used to turn off all the lights

RESET:				   ; Reset the registers before starting the program
LDI FLAG, 0b00000001
LDI INPUT, 0b00000000

MAIN:
OUT DDRA, INPUT		   ; Show the value user intoduced
CALL DELAY			   ; Give him time to release the button

SBIS PINB, 0x00		   ; If button pressed
EOR INPUT, R20		   ; Flip one bit

SBIS PINB, 0x01
EOR INPUT, R21

SBIS PINB, 0x02
EOR INPUT, R22

SBIS PINB, 0x03
EOR INPUT, R23

SBIS PINB, 0x04
EOR INPUT, R24

SBIS PINB, 0x05
EOR INPUT, R25

SBIS PINB, 0x06
EOR INPUT, R26

SBIS PINB, 0x07		   ; If leftmost switch pressed, accept the given value
RJMP REG

RJMP MAIN			   ; Repeat the cycle

REG:
CPSE FLAG, R21		   ; If flag is 2 (second press on leftmost switch), move to second step
RJMP FIRST			   ; Otherwise (first press), go to first step
RJMP SECOND

FIRST:				   ; Set first operand
MOV NUM0, INPUT
LDI INPUT, 0b00000000
INC FLAG
RJMP MAIN			   ; Go get the second operand

SECOND:				   ; Set second operand
MOV NUM1, INPUT
OUT DDRA, R27		   ; Turn off all the lights

OPERATION:			   ; Set operation
SBIS PINB, 0x00		   ; 0 switch pressed - add
RJMP SUM

SBIS PINB, 0x01		   ; 1 switch pressed - substract
RJMP DIF

SBIS PINB, 0x02        ; 3 switch pressed - multiply
RJMP MLT

SBIS PINB, 0x03		   ; 4 switch pressed - divide
RJMP DIV

RJMP OPERATION

SUM:				   ; Add numbers
ADC NUM1, NUM0
MOV INPUT, NUM1		   ; Move the result to INPUT register
RJMP WAIT

DIF:				   ; Substract
SUB NUM0, NUM1
MOV INPUT, NUM0
RJMP WAIT

MLT:				   ; Multiply
MUL NUM0, NUM1
MOV INPUT, R0
RJMP WAIT

DIV:				   ; Divide
INC INPUT
SUB NUM0, NUM1
CP NUM0, NUM1
BRGE DIV
RJMP WAIT

WAIT:				   ; Show result ...
OUT DDRA, INPUT
CALL DELAY

OUT DDRA, R27		   ; ... blinking ...
CALL DELAY

SBIS PINB, 0x07        ; ... until leftmost switch is pressed
RJMP RESET			   ; Start the program again

RJMP WAIT

DELAY:
LDI R28, 10
LOOP3: LDI R29, 255
LOOP2: LDI R30, 255
LOOP1: DEC R30
BRNE LOOP1
DEC R29
BRNE LOOP2
DEC R28 
BRNE LOOP3
RET