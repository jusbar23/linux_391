#include "irq.h"

/* write interrept gate into idt */

/*
 * write_int_gate
 *   DESCRIPTION: The function writes on the IDT the interupt gate at specific location specified by the input.
 *				  Also set up the configuration for the dpl, reserved , size and seg_selector bits
 *	 INPUTS: n --   the entry number of the IDT
 *	         handler -- address of the handler  
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: write interrept gate into idt at the specified entry number
 */


void write_int_gate(int n, void (*handler)(void))
{
	SET_IDT_ENTRY(idt[n], handler); //store handler offset in idt entry
	idt[n].present  = 1; //mark interrupt present
	idt[n].dpl = 0; //requires kernel priv
	idt[n].reserved0 = 0; //Storage Segment 0 for interrupt gates
	idt[n].size = 1; // using 32 bit handler address
	idt[n].reserved1 = 1; //bits 42:40 type = interrupt gate
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 0;
	idt[n].reserved4 = 0; //bits 39:32 always 0
	idt[n].seg_selector = KERNEL_CS; //handler always kernel code
}
	
/* write system gate into idt */

/*
 * write_sys_gate
 *   DESCRIPTION: The function writes on the IDT the system gate at specific location specified by the input.
 *				  Also set up the configuration for the dpl, reserved , size and seg_selector bits.
 *	 INPUTS: n --   the entry number of the IDT
 *	         handler -- address of the handler  
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: write system gate into idt at the specified entry number
 */

void write_sys_gate(int n, void (*handler)(void))
{
	SET_IDT_ENTRY(idt[n], handler); //store handler offset in idt entry
	idt[n].present  = 1; //mark interrupt present
	idt[n].dpl = 3; //priv = 3 (user)
	idt[n].reserved0 = 0; //Storage Segment 0 for interrupt gates
	idt[n].size = 1; // using 32 bit handler address
	idt[n].reserved1 = 1; //bits 42:40 type = trap gate
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 1;
	idt[n].reserved4 = 0; //bits 39:32 always 0
	idt[n].seg_selector = KERNEL_CS; //handler always kernel code
}

/*
 * write_trap_gate
 *   DESCRIPTION: The function writes on the IDT the trap gate at specific location specified by the input.
 *				  Also set up the configuration for the dpl, reserved , size and seg_selector bits.
 *	 INPUTS: n --   the entry number of the IDT
 *	         handler -- address of the handler  
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: write trap gate into idt at the specified entry number
 */
void write_trap_gate(int n, void (*handler)(void))
{
	SET_IDT_ENTRY(idt[n], handler); //store handler offset in idt entry
	idt[n].present  = 1; //mark interrupt present
	idt[n].dpl = 0; //priv = 3 (user)
	idt[n].reserved0 = 0; //Storage Segment 0 for interrupt gates
	idt[n].size = 1; // using 32 bit handler address
	idt[n].reserved1 = 1; //bits 42:40 type = trap gate
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 1;
	idt[n].reserved4 = 0; //bits 39:32 always 0
	idt[n].seg_selector = KERNEL_CS; //handler always kernel code
}

/*
 * null_handler
 *   DESCRIPTION: This handler is used for unitilized or undifined interrupt 
 *	 INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Prints error text on the screeen and spin to halt the system
 */
void null_handler( void )
{
	printf("Null Handler Called\n\n");
	while(1);
	iret();
}



/*
 * init_idt
 *   DESCRIPTION: this functions initilize the idt by filling in the interrupt entry that are needed
 *	 INPUTS:none 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: fill the IDT with interrupt linkage.
 */

void init_idt()
{
	int i;
	for(i=0; i<NUM_VEC; i++)
	{
		write_trap_gate(i, null_handler); //install null_handler in all exceptions as interrupt
	}
	
	write_int_gate(0x20, pit_linkage);
	write_int_gate(0x21, keyboard_linkage);
	write_int_gate(0x28, rtc_linkage);
	write_sys_gate(0x80, syscall_linkage); // Setup INT x80
}
