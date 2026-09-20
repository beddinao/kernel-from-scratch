#include <kernel.h>

void kshell_print_help() {
        vga_putstr("list of commands:\n");
        vga_putstr("ping: prints 'pong' to the screen\n");
}

void kshell(uint8_t *cmd_line, struct vga_info_st* vga_info) {
        if (!kstrcmp(cmd_line, "help"))
	      kshell_print_help();
        else if (!kstrcmp(cmd_line, "ping"))
	      vga_putstr("pong\n");
        else
	      vga_putstr("INVALID SYNTAX; try 'help'\n");

        vga_putstr(SHELL_PROMPT);
}
