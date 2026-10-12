# *`kernel-from-scratch (KFS-1-TO-4)`*

<img width="1802" height="990" alt="kfs-entry" src="https://github.com/user-attachments/assets/1c313e3e-a722-4fc1-8ec1-b8487d8c7460" />

## ENVIRONMENT
This project was developed, compiled, and emulated using standard Linux build tools rather than a dedicated cross-compiler. The specific tools utilized are:
```
 GCC / GNU ld (Standard compiler and linker, utilizing -m32 and elf_i386 flags for the 32-bit target)
 NASM (x86 Assembler using elf32 format)
 QEMU (Specifically qemu-system-i386 with KVM acceleration enabled)
 GRUB (grub-mkrescue utilizing fetched i386-pc debian modules for Multiboot ISO generation)
 GDB (GNU Debugger for remote target attachment and high-half break states)
```
Linker scripts, assembly bootstrap files, and build configurations are stored at the root of the repository.

## ARCHITECTURE
this repository is a single, unified codebase. It integrates the requirements of the 42 kernel-from-scratch curriculum, currently implementing KFS-1 through KFS-4, and is structured to scale continuously for future phases (KFS-5 through KFS-X).

```
 KFS-1 : Multiboot compliance, VGA basic driver, the linker script, and basic assembly boot code.
 KFS-2 : Global Descriptor Table (GDT) configuration and kernel stack setup.
 KFS-3 : Memory paging (VMM), Physical Memory Management (PMM), and the virtual heap SLAB allocator.
 KFS-4 : Interrupt Descriptor Table (IDT), hardware interrupts, and keyboard handling.
       : Foundations laid for KFS-5 to KFS-X (processes, filesystems, and beyond).
```
Note: The project utilizes a high-half architecture.
Additionally, a small interactive shell with some basic commands is integrated into the environment for testing and debugging purposes.
The display driver supports multiple virtual screens, allowing for switching between active terminal sessions by using CTRL+LARROW and CTRL+RARROW.
