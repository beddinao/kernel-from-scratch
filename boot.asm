bits 32

MAGIC_NM	equ 0x1badb002
FLAGS	equ 0x0
CSUM	equ -(MAGIC_NM + FLAGS)
_8KB	equ 0x2000

global start
extern kmain

section .multiboot
align 4
	dd MAGIC_NM
	dd FLAGS
	dd CSUM

section .text
start:
	cli
	mov esp, stack_p
	call kmain
	hlt

section .bss
resb _8KB
stack_p:
