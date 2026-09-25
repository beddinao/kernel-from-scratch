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
        vga_putstr("clear            : clears entire screen\n");
	vga_putstr("sdump [depth]    : dumps the current stack: sttop-depth - sttop\n");
	vga_putstr("mdump [0xa] [0xb]: dumps memory from addr1 to addr2\n");
	vga_putstr("bg, fg [NM]      : change colors: [black, blue, green, .., 16]\n");
	vga_putstr("panic [STR]      : triggers a kernel panic (no going back)\n");
        vga_putstr("ping             : prints 'pong' to the screen\n");
	vga_putstr("hdr              : prints the kernel welcome hdr\n");
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

uint8_t kshell_sdump_wrp(uint8_t *cmd_line, uint8_t size) {
	uint8_t arg[size], arg_size;
	uint32_t depth;
	just_arg(arg, cmd_line);
	arg_size = kstrlen(arg);

	if (!arg_size || arg_size > 4 || (depth = ktonum(arg)) > KSTACK_S || !depth) {
		vga_putdword("depth must be between 1 and stack size 0x", KSTACK_S);
		vga_putchar('\n');
		return 1;
	}

	
	dump((uint32_t)&_kernel_stack-depth, (uint32_t)&_kernel_stack);
	return 1;
}

uint8_t kshell_mdump_wrp(uint8_t *cmd_line, uint8_t size) {
	vga_putstr("NOT IMPLEMENTED YET!\n");
	return 1;
}

uint8_t kshell_panic_wrp(uint8_t *cmd_line, uint8_t size) {
	uint8_t arg[size], arg_size;
	just_arg(arg, cmd_line);
	arg_size = kstrlen(arg);
	if (!arg_size || arg_size > (VGA_W - 3))
		return 0;
	panic(arg);
	return 1;
}

uint8_t kshell_hdr_wrp(uint8_t *cmd_line, uint8_t size) {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_kernel_end;
	print_hdr(pmm_info->mm_ptr);
	return 1;
}

void kshell(uint8_t *cmd_line, struct vga_info_st* vga_info) {
	uint8_t cmd_line_size, status = 0;
	if (!(cmd_line_size = kstrlen(cmd_line))) {
		vga_putstr(SHELL_PROMPT);
		return;
	}
	uint8_t *avail_cmds_names[9] = { "help", "clear", "sdump", "mdump", "bg", "fg", "panic", "ping", "hdr" };
	uint8_t (*cmds_handlers[9])(uint8_t*, uint8_t) = {
		kshell_print_help, kshell_clear_wrp, kshell_sdump_wrp, kshell_mdump_wrp,
		kshell_bg_wrp, kshell_fg_wrp, kshell_panic_wrp, pong, kshell_hdr_wrp
	};

	uint8_t cmd[cmd_line_size];
	just_cmd(cmd, cmd_line);

	for (uint8_t cur_cmd = 0; cur_cmd < 9; cur_cmd++) {
		if (!kstrcmp(cmd, avail_cmds_names[cur_cmd])) {
			status = cmds_handlers[cur_cmd](cmd_line, cmd_line_size);
			break;
		}
	}

	if (!status)
		vga_putstr("INVALID SYNTAX; try 'help'\n");
        vga_putstr(SHELL_PROMPT);
}
