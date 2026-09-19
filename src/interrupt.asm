global isr_division_by_zero
global isr_double_fault
global isr_page_fault
global isr_systimer
global isr_keyboard
global isr_dummy_error_code
global isr_dummy

section .text
isr_wrapper_fn:
	iret

isr_division_by_zero:
	push 0
	push 0x0
	jmp isr_wrapper_fn

isr_double_fault:
	push 0
	push 8
	jmp isr_wrapper_fn

isr_page_fault:
	push 0
	push 14
	jmp isr_wrapper_fn

isr_systimer:
	push 0
	push 32
	jmp isr_wrapper_fn

isr_keyboard:
	push 0
	push 33
	jmp isr_wrapper_fn

isr_dummy:
	push 0
	jmp isr_wrapper_fn

isr_dummy_error_code:
	push 0
	push 0
	jmp isr_wrapper_fn
