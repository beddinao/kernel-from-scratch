global _asm_interrupt_fn
global _asm_fatal_interrupt_fn

section .text
_asm_interrupt_fn:
	
	push dword 0

	;mov eax, [ebp + 8] 	
	;mov ebx, [ebp + 12] 

	mov word [0xb8000], 0b0100001001001000
	mov word [0xb8002], 0b0100001001001001

	add esp, 4

	iret

_asm_fatal_interrupt_fn:
	

	;mov eax, [ebp + 8] 	
	;mov ebx, [ebp + 12] 

	mov word [0xb8000], 0b0100001001001000
	mov word [0xb8002], 0b0100001001001001

	add esp, 4

	iret
