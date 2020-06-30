#include "rtc.h"

volatile unsigned char rtc_flag;
/*
 * rtc Init
 *   DESCRIPTION: The function initilizes the RTC by setting a few bits at the CMOS register at the RTC
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initilize the rtc and enable IRQ line on the slave PIC
 */
void 
rtc_init(void)
{
    unsigned char prev;
    unsigned long flags;
		int32_t rate = DEFAULT_RATE;
    //disable interrupts

	//disable NMIs and set reg A
	cli_and_save(flags);

    enable_irq(RTC_IRQ_NUM);
    outb(RTC_REG_B | DISABLE_NMI, RTC_PORT);
    prev = inb(RTC_CMOS);
    outb(RTC_REG_B | DISABLE_NMI, RTC_PORT);
    outb(prev | BIT_SIX, RTC_CMOS);   //setting up the bit six at register B

    rtc_flag = FALSE;
    restore_flags(flags);
    rtc_write(NULL, &rate, NULL);
	//enable the irq
	printf("Enabled RTC\n");
}

/*
 * rtc_irq_handler
 *   DESCRIPTION: The function handles the interrupt that comes from the RTC
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Handle the RTC interrupt and send eoi to the PIC
 */
void 
rtc_irq_handler(void)
{
	//test_interrupts();
    // Reading from the the C register so that the interrupt could happen 
	outb(RTC_REG_C, RTC_PORT);
    	inb(RTC_CMOS);
	rtc_flag = TRUE;
    // test_interrupts();
	send_eoi(RTC_IRQ_NUM);
}
/*
 * rtc_open
 *   DESCRIPTION: Finds the directory entry with the named file, allocates
 *                an unused file descriptor, and set the real time clock
 *                to 2Hz.
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success and -1 on error
 *   SIDE EFFECTS: allocates a file descriptor and sets real time clock
 *                 to 2Hz.
 */
int
rtc_open()
{
		int32_t rate = DEFAULT_RATE;
    rtc_write(NULL, &rate, NULL);
    return 0;
}
/*
 * rtc_read
 *   DESCRIPTION: Waits until the next RTC interrupt and returns. 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success
 *   SIDE EFFECTS: none
 */
int
rtc_read(int32_t fd, void *buf, int32_t nbytes)
{   
    unsigned long flags;
		rtc_flag = FALSE;

    while(TRUE)
    {
        //Not sure if it is nessisary to implement some sort of locking here
        cli_and_save(flags);
        if(rtc_flag == TRUE){
            rtc_flag = FALSE;
            restore_flags(flags);
            return 0;
        }
        restore_flags(flags);
    }

}
/*
 * rtc_write
 *   DESCRIPTION: Writes a new frequency to the RTC. 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success and -1 on fail
 *   SIDE EFFECTS: Changes the interrupt rate of the RTC. 
 */
int
rtc_write(int32_t fd, void *buf, int32_t nbytes)
{
    unsigned long flags, bitmask;
    unsigned char divider_value;
		int32_t rtc_rate = *(int32_t*)buf;

    int i;
    bitmask = MASK_DEFAULT;
    //check if over max rate
    if(rtc_rate > RTC_MAX_RATE || rtc_rate < RTC_MIN_RATE)
        return -1;
    //power of 2 check 
    if(!((rtc_rate != 0) && ((rtc_rate & (~rtc_rate + 1)) == rtc_rate)))
        return -1;

    //look up the divider value 
    rtc_rate = rtc_rate & RATE_IN_MASK;
    for(i = 0, bitmask = MASK_DEFAULT; i < 12; i++, bitmask <<= 1)
    {
        if(rtc_rate & bitmask)
        {
            divider_value = DIV_MAX - i;
            break;
        }
    }

    cli_and_save(flags);
    outb(RTC_REG_A_M, RTC_PORT);                //set index to register A, disable NMI
    char prev = inb(RTC_CMOS);                  //get initial value of register A
    outb(RTC_REG_A_M, RTC_PORT);                //reset index to A
    outb((prev & RATE_MASK) | divider_value, RTC_CMOS);  //write only our rate to A. Note, rate is the bottom 4 bits.
    restore_flags(flags);
    return 0;
}
/*
 * rtc_close
 *   DESCRIPTION: Closes the RTC file descriptor.
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success and -1 on error
 *   SIDE EFFECTS: Closes the RTC file descriptor.
 */
int
rtc_close(int32_t fd, void *buf, uint32_t bytes)
{
    return 0;
}
