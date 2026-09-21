#include <kernel.h>

void kshell_print_help(uint8_t *cmd_line) {
        vga_putstr("list of commands:\n");
        vga_putstr("ping: prints 'pong' to the screen\n");
        vga_putstr("clear: clears entire screen\n");
	vga_putstr("bg [NM]: change the background color [0-15]\n");
	vga_putstr("fg [NM]: change the fourground color [0-15]\n");
}

void kshell_clear_wrp(uint8_t *cmd_line) {
	vga_reset_screen();
}

void pong(uint8_t *cmd_line) {
	vga_putstr("pong\n");
}

void kshell_bg_wrp(uint8_t *cmd_line) {
	vga_set_bg_color(5);
	vga_clear_screen(0);
}

void kshell_fg_wrp(uint8_t *cmd_line) {
	vga_set_fg_color(3);
	vga_clear_screen(0);
}

void just_cmd(uint8_t *cmd, uint8_t *cmd_line) {
	for (;; cmd_line++, cmd++) {
		if (!*cmd_line || *cmd_line == ' ') {
			*cmd = 0;
			break;
		}
		*cmd = *cmd_line;
	}
}

void kshell(uint8_t *cmd_line, struct vga_info_st* vga_info) {
	uint8_t cmd_line_size = kstrlen(cmd_line);
	if (!cmd_line_size) {
		vga_putstr(SHELL_PROMPT);
		return;
	}

	uint8_t *avail_cmds_names[5] = {
		"help", "clear", "ping", "bg", "fg"
	};

	void (*cmds_handlers[5])(uint8_t*) = {
		kshell_print_help, kshell_clear_wrp, pong,
		kshell_bg_wrp, kshell_fg_wrp
	};

	uint8_t cmd[cmd_line_size];
	just_cmd(cmd, cmd_line);

	for (uint8_t cur_cmd = 0; cur_cmd < 5; cur_cmd++) {
		if (!kstrcmp(cmd, avail_cmds_names[cur_cmd])) {
			cmds_handlers[cur_cmd](cmd_line);
			vga_putstr(SHELL_PROMPT);
			return;
		}
	}

	vga_putstr("INVALID SYNTAX; try 'help'\n");
        vga_putstr(SHELL_PROMPT);
}
