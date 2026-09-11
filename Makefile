all:
	nasm -f elf32 boot.asm -o boot.o
	gcc -m32 -c kernel.c -o kernel.o -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs
	ld -m elf_i386 -T linker.ld -o iso/boot/kernel.bin boot.o kernel.o
	grub-mkrescue -o kkk.iso iso/
