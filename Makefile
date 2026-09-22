KERNEL_BIN = kernel.bin
ISO_NAME   = kfs.iso

CC         = gcc
CFLAGS     = -m32 -masm=intel -Iinclude -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs
AS         = nasm
ASFLAGS    = -f elf32
LDFLAGS    = -m elf_i386 -T linker.ld

HEADERS    = $(wildcard include/*.h)
C_SRCS     = $(wildcard src/*.c)
ASM_SRCS   = $(wildcard src/*.asm)

C_OBJS     = $(patsubst src/%.c, build/%.o, $(C_SRCS))
ASM_OBJS   = $(patsubst src/%.asm, build/%.o, $(ASM_SRCS))
OBJS       = $(C_OBJS) $(ASM_OBJS)

GRUB_I386  =

all: $(ISO_NAME) 

i386_modules:
	mkdir -p i386-pc
	wget -P i386-pc http://ftp.us.debian.org/debian/pool/main/g/grub2/grub-pc-bin_2.14-3_i386.deb
	dpkg -x i386-pc/grub-pc-bin_2.14-3_i386.deb i386-pc/
	GRUB_I386="./i386-pc/usr/lib/grub/i386-pc/"


$(ISO_NAME): $(KERNEL_BIN)
	mkdir -p iso/boot/grub
	cp $(KERNEL_BIN) iso/boot/
	cp grub.cfg iso/boot/grub/
	grub-mkrescue -o $(ISO_NAME) $(GRUB_I386) --install-modules="iso9660 multiboot normal" --locales="" --fonts="" --themes="" 

$(KERNEL_BIN): $(OBJS)
	ld $(LDFLAGS) -o $(KERNEL_BIN) $(OBJS)

build/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -Iinclude $(CFLAGS) -c $< -o $@ 

build/%.o: src/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@ 

run:
	qemu-system-i386 -m 128M -cdrom $(ISO_NAME) -display gtk,zoom-to-fit=on

clean:
	rm -rf build iso

fclean: clean
	rm -rf $(KERNEL_BIN) $(ISO_NAME) i386-pc 

re: fclean all run

#re: fclean all

.PHONY: all clean fclean re
