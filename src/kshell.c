#include <kernel.h>

void just_cmd(uint8_t *cmd, uint8_t *cmd_line) {
	for (;; cmd_line++, cmd++) {
		if (!*cmd_line || *cmd_line == ' ') {
			*cmd = 0;
			break;
		}
		*cmd = *cmd_line;
	}
}

void just_arg(uint8_t *arg, uint8_t *cmd_line) {
	uint8_t past_cmd = 0;
	for (;; cmd_line++) {
		if (*cmd_line == ' ') {
			past_cmd = 1;
			continue;
		}
		if (past_cmd)
			*arg++ = *cmd_line;
		if (!*cmd_line)
			break;
	}
	*arg = *cmd_line;
}

uint8_t kshell_print_help(uint8_t *cmd_line, uint8_t size) {
        vga_putstr("list of commands:\n");
        vga_putstr("ping: prints 'pong' to the screen\n");
        vga_putstr("clear: clears entire screen\n");
	vga_putstr("bg, fg [NM]: change colors:\n");
	vga_putstr("             [black, blue, green, cyan, red, .., 16]\n");
	return 1;
}

uint8_t kshell_clear_wrp(uint8_t *cmd_line, uint8_t size) {
	vga_reset_screen();
	return 1;
}

uint8_t pong(uint8_t *cmd_line, uint8_t size) {
	vga_putstr("pong\n");
	return 1;
}

uint8_t kshell_bg_wrp(uint8_t *cmd_line, uint8_t size) {
	uint8_t arg[size], arg_size, bgc;
	just_arg(arg, cmd_line);
	arg_size = kstrlen(arg);

	if (!arg_size || arg_size > 2 || (bgc = ktonum(arg)&0xff) > 16 || !bgc)
		return 0;

	vga_set_bg_color(bgc-1);
	vga_clear_screen(0);
	return 1;
}

uint8_t kshell_fg_wrp(uint8_t *cmd_line, uint8_t size) {
	uint8_t arg[size], arg_size, fgc;
	just_arg(arg, cmd_line);
	arg_size = kstrlen(arg);

	if (!arg_size || arg_size > 2 || (fgc = ktonum(arg)&0xff) > 16 || !fgc)
		return 0;

	vga_set_fg_color(fgc-1);
	vga_clear_screen(0);
	return 1;
}

void kshell(uint8_t *cmd_line, struct vga_info_st* vga_info) {
	uint8_t cmd_line_size, status = 0;
	if (!(cmd_line_size = kstrlen(cmd_line))) {
		vga_putstr(SHELL_PROMPT);
		return;
	}
	uint8_t *avail_cmds_names[5] = { "help", "clear", "ping", "bg", "fg" };
	uint8_t (*cmds_handlers[5])(uint8_t*, uint8_t) = {
		kshell_print_help, kshell_clear_wrp, pong, kshell_bg_wrp, kshell_fg_wrp
	};

	uint8_t cmd[cmd_line_size];
	just_cmd(cmd, cmd_line);

	for (uint8_t cur_cmd = 0; cur_cmd < 5; cur_cmd++) {
		if (!kstrcmp(cmd, avail_cmds_names[cur_cmd])) {
			status = cmds_handlers[cur_cmd](cmd_line, cmd_line_size);
			break;
		}
	}

	if (!status)
		vga_putstr("INVALID SYNTAX; try 'help'\n");
        vga_putstr(SHELL_PROMPT);
}
