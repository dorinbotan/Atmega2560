/*
 * Blink.asm
 *
 *  Created: 08-09-2015 08:49:28
 *   Author: Dorin
 */ 

LDI R16, 0
OUT DDRA, R16

BACK:
CALL DELAY

COM R16
OUT DDRA, R16

RJMP BACK


DELAY:
LDI R17, 10
LOOP3: LDI R18, 255
LOOP2: LDI R19, 255
LOOP1: DEC R19 
BRNE LOOP1
DEC R18 
BRNE LOOP2
DEC R17 
BRNE LOOP3
RET