/ Source of assembler to be inserted into mch.s
/ This assembler code halts the machine.
/ Inserted after the start code
/ and before the various bootstraps
/ The code that stops the system is in stopunix()
/ should be added at the end of ken/sys4.c
/

halt	= 0		// Halt instruction not in assembler

.text

.globl	_stopit
_stopit:
	mov	$0340,*$6
	reset
	halt
