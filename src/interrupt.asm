global isr_division_by_zero
global isr_double_fault
global isr_page_fault
global isr_systimer
global isr_keyboard
global isr_dummy_ec
global isr_dummy

section .text
single_interrupt_wrapper:
	pushad
	push esp

	extern isr_global_interrupt_handler
	call isr_global_interrupt_handler
	
	add esp, 4
	popad

	add esp, 8
	iret

isr_division_by_zero:
	push dword 0
	push dword 0x0
	jmp single_interrupt_wrapper

isr_double_fault:
	push dword 8
	jmp single_interrupt_wrapper

isr_page_fault:
	push dword 14
	jmp single_interrupt_wrapper

isr_systimer:
	push dword 0
	push dword 32
	jmp single_interrupt_wrapper

isr_keyboard:
	push dword 0
	push dword 33
	jmp single_interrupt_wrapper

isr_dummy:
	push dword 0
	jmp single_interrupt_wrapper

isr_dummy_ec:
	push dword 0
	push dword 0
	jmp single_interrupt_wrapper
