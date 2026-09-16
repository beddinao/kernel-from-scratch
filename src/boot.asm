bits 32

MAGIC_NM	equ 0x1badb002
FLAGS	equ 0b11
CSUM	equ -(MAGIC_NM + FLAGS)
_8KB	equ 0x2000

global start
extern kmain
extern _kernel_stack

section .multiboot
align 4
	dd MAGIC_NM
	dd FLAGS
	dd CSUM

section .text
start:
	cli
	mov esp, _kernel_stack
	push ebx
	push eax
	call kmain
	hlt
