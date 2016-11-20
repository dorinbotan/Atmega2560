/*
 * Array.asm
 *
 *  Created: 22/09/15 08:58:08
 *   Author: Dorin
 */ 

.org 0x100
.equ MEMSTARTHIGH = 0x05
.equ MEMSTARTLOW = 0x00

; Initialize array pointer to the start of the array:

LDI R29, MEMSTARTHIGH
LDI R28, MEMSTARTLOW
LDI R17, 'h'
ST Y+, R17
LDI R17, 'e'
ST Y+, R17
LDI R17, 'l'
ST Y+, R17
LDI R17, 'l'
ST Y+, R17
LDI R17, 'o'
ST Y+, R17
LDI R17, 0
ST Y+, R17

LDI R27, 
LDI R26, 
LD R17, -Y;
LD R17, -Y;
LD R17, -Y;
LD R17, -Y;
LD R17, -Y;
LD R17, -Y;
INFI: RJMP INFI