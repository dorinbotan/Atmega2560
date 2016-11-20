; Register R20 holds the number we’re finde the factorial of. If R20 = 3, we’re looking for 4!.
LDI R20, 1
; Setup call:
PUSH R20
; CALL the subroutine:
CALL FACT
; Retrieve the result:
POP R21
DONE: RJMP DONE

FACT:
PUSH R20
PUSH R21
PUSH R26
PUSH R27
IN R26, SPL
IN R27, SPH
ADIW R26, 9 ; 4 + 3 + 1 + 1 = 9
LD R20, -X
; if FACT == 0: return 1
CPI R20, 1
BREQ ONE
JMP HIGHER

ONE:
ST	X, R20
POP R27
POP R26
POP R21
POP R20
RET

HIGHER:
MOV  R21, R20
SUBI R21, 1
PUSH R21
CALL FACT
POP	 R21
MUL	 R21, R20
ST	 X, R0
POP  R27
POP  R26
POP  R21
POP  R20
RET