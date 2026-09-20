#include <kernel.h>

void kshell(uint8_t *cmd_line, uint32_t size) {
        vga_putstr("PONG\n");
        vga_putstr(SHELL_PROMPT);
}
