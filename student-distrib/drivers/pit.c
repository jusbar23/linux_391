#include "pit.h"
#include "../kernel/scheduling.h"
#include "../kernel/irq.h"

/*
 * pit_init
 *   DESCRIPTION: Initilize the pit to default rates for our needs
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables the pit and sets the interupt rate 
 */
void 
pit_init(void)
{
	uint32_t flags; //flags for lock
	cli_and_save(flags); //Critical section

	outb(PIT_INITIALIZE_CMD, PIT_COMMAND_REG);
	//Send 55ms to data lo
	outb(RATE_10MS_LO, PIT_CHANNEL_0_DATA);
	//Send 55ms to data hi
	outb(RATE_10MS_HI, PIT_CHANNEL_0_DATA);

	//Enable the irq line for the pit
	enable_irq(PIT_IRQ_LINE);
	
	restore_flags(flags); //End critical section
}

/*
 * pit_init
 *   DESCRIPTION: Handles the pit interups, sending an EOI and calling the
 *				  scheduler tick.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Calls the scheduler tick
 */
void
pit_handler(void)
{
	//Send EOI
	send_eoi(PIT_IRQ_LINE);
	//Run the scheduler tick
	scheduler_tick();
}
