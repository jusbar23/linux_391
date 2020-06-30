/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */



/*
 * i8259_init
 *   DESCRIPTION: Initializing the Master and the slave PIC. The function sends ICW1 command to both PIC, disable all the IRQ and lastly enables IRQ 2 at the master
 *   INPUT : none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializing both master and slave PIC and making sure that the IRQ2 at the master is enables, since it's connected to the slave
 */

void
i8259_init(void)
{
	printf("\nInitializing the PIC...");
    int i;
	//Send the initialise command word to the master PIC
	outb(ICW1, MASTER_8259_PORT);	 
    outb(ICW1, SLAVE_8259_PORT);    

	//Send the initialise command word to the slave PIC

    outb(ICW2_MASTER, MASTER_MASK_8259_PORT);   //IR0-7 mapped to 0x20-0x27 (master)
	outb(ICW2_SLAVE, SLAVE_MASK_8259_PORT);	//IR0-7 mapped to 0x28-0x2F (slave)

    outb(ICW3_MASTER, MASTER_MASK_8259_PORT);   //Set up the cascasding 
	outb(ICW3_SLAVE, SLAVE_MASK_8259_PORT);	//Set up the cascasding

	//Master and Slave expects normal EOI
    outb(ICW4, MASTER_MASK_8259_PORT); 
	outb(ICW4, SLAVE_MASK_8259_PORT); 
	
	//restore master and slve IRQ masks  
	printf("the PIC initialized\n");

    for (i = 0; i < NR_IRQS; i++)
    {
        disable_irq(i);
    } // optimize later

    enable_irq(SLAVE_IRQ_NUM); //  re-enable the slave
	// restore_flags(flags);
}



/*
 * enable_irq
 *   DESCRIPTION: the function enables the irq that is specified by the input
 *   INPUT : irq_num -- the irq that want to be enabled
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enable (unmask) the specified IRQ
 */
void
enable_irq(uint32_t irq_num)
{
    uint16_t port;
    uint8_t value;
	// if the irq is more the 8, send the enable masking to the slave, otherwise to to master
    if (irq_num < 8)
    {
        port = MASTER_MASK_8259_PORT;
		value = inb(port) & ~(1 << irq_num);
    } else
    {
        port = SLAVE_MASK_8259_PORT;
        value = inb(port) & ~(1 << (irq_num-8));
    }

    outb(value, port);
	//printf("Enabled irq:0x%x value:0x%x port:0x%x\n",irq_num, value, port);

}



/*
 * disable_irq
 *   DESCRIPTION: the function enables the irq that is specified by the input
 *   INPUT : irq_num -- the irq that want to be disabled
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Disable (mask) the specified IRQ
 */
void
disable_irq(uint32_t irq_num)
{
    uint16_t port;
    uint8_t value;
	// if the irq is more the 8, send the disable masking to the slave, otherwise to to master
    if (irq_num < 8)
    {
        port = MASTER_MASK_8259_PORT;
		value = inb(port) | (1 << irq_num);
    } else
    {
        port = SLAVE_MASK_8259_PORT;
        value = inb(port) | (1 << (irq_num -8));
    }

    outb(value, port);
	// printf("Disabled irq:0x%x value:0x%x port:0x%x\n",irq_num, value, port);

}



/*
 * send_eoi
 *   DESCRIPTION: This function sends the eoi back to the PIC either the master or the slave.
 *   INPUT : irq_num -- the irq that want to be disabled
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Send end-of-interrupt signal for the specified IRQ
 */

void
send_eoi(uint32_t irq_num)
{
	unsigned char master_eoi, slave_eoi;
	// if the irq is more the 8, send the eoi both to the slave and master, otherwise just to the master
    if (irq_num >= 8)
	{
		slave_eoi = EOI | (irq_num - 8);
		master_eoi = EOI | 2;
        outb(slave_eoi, SLAVE_8259_PORT);
		outb(master_eoi, MASTER_8259_PORT);
	} else
    {
    	master_eoi = EOI | irq_num;
    	outb(master_eoi, MASTER_8259_PORT);
	}
	//printf("EOI sent\n");
}
