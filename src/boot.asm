bits 32

MAGIC_NM equ 0x1badb002
FLAGS equ 0b11 
CSUM equ -(MAGIC_NM + FLAGS)
_8KB equ 0x2000

global start
extern kmain
extern _kernel_stack

extern _boot_page_directory_start
extern _boot_page_directory_768
extern _boot_page_table_start

section .multiboot
align 4
	dd MAGIC_NM
	dd FLAGS
	dd CSUM

section .text

zero_pages:
	mov ecx, 0
	.zero_loop:
		mov dword [_boot_page_directory_start+ecx*4], 0b0
		mov dword [_boot_page_table_start+ecx*4], 0b0
		inc ecx
		cmp ecx, 1024
		jne .zero_loop
	ret

setup_dir_slots_0_768:
	mov eax, _boot_page_table_start
	or eax, 0b11
	mov dword [_boot_page_directory_start], eax
	mov dword [_boot_page_directory_768], eax
	ret

setup_page_table:
	mov eax, 0
	mov ecx, 0
	.fill_page_entries:
		mov ebx, eax
		or ebx, 0b11
		mov dword [_boot_page_table_start+ecx*4], ebx
		add eax, 4096
		inc ecx
		cmp ecx, 1024
		jne .fill_page_entries
	ret

enable_paging:
	mov eax, _boot_page_directory_start
	mov cr3, eax
	mov eax, cr0
	or eax, 0x80000000 ;[31:1]
	mov cr0, eax
	ret

start:
	cli
	mov esp, (_kernel_stack-0xc0000000)
	push ebx
	push eax
	call zero_pages
	call setup_dir_slots_0_768
	call setup_page_table
	call enable_paging
	add esp, 0xc0000000
	; WHAT A STUPID ATTEMPT
	;mov dword [_boot_page_directory_start], 0
	;mov eax, cr3
	;mov cr3, eax
	call kmain
	hlt
