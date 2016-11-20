/*
 * Fibonacci.asm
 *
 *  Created: 22/09/15 10:51:38
 *   Author: Dorin
 */ 
	
.equ MEMSTARTHIGH = 0x05
.equ MEMSTARTLOW = 0x00

; Initialize array pointer to the start of the array where we will store the FIB-numbers
LDI R27, MEMSTARTHIGH
LDI R26, MEMSTARTLOW

; Register R20 holds the Fibonacci number we’re looking for. If R20 = 3, ; we’re looking for the 3rd Fibonacci number
LDI R16, 3

NEXT:
; Setup call, pushing the argument:
PUSH R16
; CALL the subroutine:
CALL FIB
; Retrieve the result (return value):
POP R21
; Store results and continue
ST X+, R21
INC R16

CPI R16, 12
BREQ WAIT
JMP NEXT

WAIT: JMP WAIT

FIB: 
PUSH R21
PUSH R22
PUSH R28
PUSH R29

IN R28, SPL
IN R29, SPH
ADIW Y, 8
LD R20, Y

;If argument is 0, return 0
CPI R20, 0
BREQ GO0 
	
;If argument is 1, return 1
CPI R20, 1
BREQ GO1 

;If argument is higher than 1, return fib(argument-1)+fib(argument-2)
MOV R21, R20
DEC R21
MOV R22, R21
DEC R22

PUSH R21
CALL FIB
POP R21

PUSH R22
CALL FIB
POP R22

ADD R21, R22
ST Y, R21

POP R29
POP R28
POP R22
POP R21
RET

GO0:
LDI R17, 0
ST Y, R17
POP R29
POP R28
POP R22
POP R21
RET

GO1:
LDI R17, 1
ST Y, R17
POP R29
POP R28
POP R22
POP R21
RET