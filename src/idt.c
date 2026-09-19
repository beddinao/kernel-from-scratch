#include <kernel.h>

extern void isr_division_by_zero();
extern void isr_double_fault();
extern void isr_page_fault();
extern void isr_systimer();
extern void isr_keyboard();
extern void isr_dummy_ec();
extern void isr_dummy();

uint8_t *idt_perm_offset() {
        struct pmm_info_st *pmm_info = (struct pmm_info_st*)(&_kernel_end);
        return (uint8_t*)pmm_info + sizeof(struct pmm_info_st) + pmm_info->size;
}

void idt_load_isr(struct idt_gate_desc* idt_gate, uint32_t isr_ptr) {
        idt_gate->offset_low = isr_ptr & 0xffff;
        idt_gate->offset_high = (isr_ptr >> 16) & 0xffff;
}

struct interrupt_frame {
        uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
        uint32_t interrupt_number;
        uint32_t error_code;
        uint32_t eip, cs, eflags;
} __attribute__((packed));

void isr_global_interrupt_handler(struct interrupt_frame* frame) {
        if (frame->interrupt_number == 33) {
	      uint8_t scancode;
	      asm volatile (
			  "in al, 0x60\n"
			  : "=a" (scancode)
		         );
	      vga_putchar(scancode);
        }
        if (frame->interrupt_number >= 32 && frame->interrupt_number <= 39) {
	      asm volatile (
			  "out 0x20, al\n"
			  :: "a" ((uint8_t)0x20)
		         );
        }
}

void pic_remap(void) {
        asm volatile (
		    "mov al, 0x11\n"
		    "out 0x20, al\n"
		    "out 0xA0, al\n"

		    "mov al, 0x20\n"
		    "out 0x21, al\n"
		    "mov al, 0x28\n"
		    "out 0xA1, al\n"

		    "mov al, 0x04\n"
		    "out 0x21, al\n"
		    "mov al, 0x02\n"
		    "out 0xA1, al\n"

		    "mov al, 0x01\n"
		    "out 0x21, al\n"
		    "out 0xA1, al\n"

		    "mov al, 0x00\n"
		    "out 0x21, al\n"
		    "out 0xA1, al\n"
		    ::: "al" 
	);
}

void idt_init() {
        uint8_t *idt_base_ptr = idt_perm_offset();
        uint8_t *idt_limit_ptr = idt_base_ptr + IDT_D_S*IDT_D_C;
        vga_putdword("\nidt_base: 0x", (uint32_t)idt_base_ptr);

        struct idt_gate_desc *gate_desc = (struct idt_gate_desc*)idt_base_ptr;

        for (uint16_t i = 0; i < 256; i++) {
	      idt_load_isr(gate_desc, (uint32_t)isr_dummy);
	      gate_desc->gdt_seg_selec = 0x8;
	      gate_desc->reserved = 0x0;
	      gate_desc->type_attr = 0b10001110;
	      gate_desc++;
        }

        gate_desc = (struct idt_gate_desc*)idt_base_ptr;
        idt_load_isr(gate_desc, (uint32_t)isr_division_by_zero);
        idt_load_isr(gate_desc + 8, (uint32_t)isr_double_fault);
        idt_load_isr(gate_desc + 14, (uint32_t)isr_page_fault);
        idt_load_isr(gate_desc + 32, (uint32_t)isr_systimer);
        idt_load_isr(gate_desc + 33, (uint32_t)isr_keyboard);
        idt_load_isr(gate_desc + 10, (uint32_t)isr_dummy_ec);
        idt_load_isr(gate_desc + 11, (uint32_t)isr_dummy_ec);
        idt_load_isr(gate_desc + 12, (uint32_t)isr_dummy_ec);
        idt_load_isr(gate_desc + 13, (uint32_t)isr_dummy_ec);
        idt_load_isr(gate_desc + 17, (uint32_t)isr_dummy_ec);
        idt_load_isr(gate_desc + 21, (uint32_t)isr_dummy_ec);

        struct idt_register idtr = { IDT_D_C*IDT_D_S - 1, (uint32_t)idt_base_ptr };

        pic_remap();
        asm volatile (
		    "lidt %0\n"
		    "sti\n"
		    :: "m" (idtr)
		 );
}
