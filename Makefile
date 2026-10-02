KERNEL_BIN = kernel.bin
ISO_NAME   = kfs.iso

CC         = gcc
CFLAGS	   = -m32 -masm=intel -Iinclude -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs -ggdb3 #-O0 -fno-strict-aliasing -fno-omit-frame-pointer
AS         = nasm
ASFLAGS    = -f elf32 -g -F dwarf
LDFLAGS    = -m elf_i386 -T linker.ld

HEADERS    = $(wildcard include/*.h)
C_SRCS     = $(wildcard src/*.c)
ASM_SRCS   = $(wildcard src/*.asm)

C_OBJS     = $(patsubst src/%.c, build/%.o, $(C_SRCS))
ASM_OBJS   = $(patsubst src/%.asm, build/%.o, $(ASM_SRCS))
OBJS       = $(C_OBJS) $(ASM_OBJS)

GRUB_I386  = i386-pc

all: $(ISO_NAME) 

i386_modules:
	mkdir -p $(GRUB_I386) 
	wget -P $(GRUB_I386) http://ftp.us.debian.org/debian/pool/main/g/grub2/grub-pc-bin_2.14-3_i386.deb
	dpkg -x i386-pc/grub-pc-bin_2.14-3_i386.deb $(GRUB_I386)


$(ISO_NAME): $(KERNEL_BIN)
	@if [ -d $(GRUB_I386) ]; then\
		echo "grub $(GRUB_I386) exists..skiping";\
	else \
		$(MAKE) i386_modules;\
	fi
	mkdir -p iso/boot/grub
	cp $(KERNEL_BIN) iso/boot/
	cp grub.cfg iso/boot/grub/
	grub-mkrescue -o $(ISO_NAME) iso -d $(GRUB_I386)/usr/lib/grub/i386-pc --install-modules="iso9660 multiboot normal" --locales="" --fonts="" --themes="" 

$(KERNEL_BIN): $(OBJS)
	ld $(LDFLAGS) -o $(KERNEL_BIN) $(OBJS)

build/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -Iinclude $(CFLAGS) -c $< -o $@ 

build/%.o: src/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@ 

run:
	qemu-system-i386 -m 128M -cdrom $(ISO_NAME) -display gtk,zoom-to-fit=on -enable-kvm -cpu host

clean:
	rm -rf build iso

debug:
	qemu-system-i386 -m 128 -cdrom $(ISO_NAME) -s -S &
	gdb $(KERNEL_BIN) -ex 'target remote localhost:1234' -ex 'b *high_half-0xc0000000' -ex 'b *start-0xc0000000' -ex 'b *kmain-0xc0000000' -ex 'b *start+34-0xc0000000' -ex 'b *start+35-0xc0000000'

fclean: clean
	rm -rf $(KERNEL_BIN) $(ISO_NAME) #i386-pc 

re: fclean all 

#re: fclean all

.PHONY: all clean fclean re
