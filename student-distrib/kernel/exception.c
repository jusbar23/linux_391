#include "exception.h"

//General exception handler 
void divide(void);
void debug(void);
void nmi(void);
void breakpoint(void);
void overflow(void);
void bound(void);
void invalid_opcode(void);
void device_not_available(void);
void double_fault(void);
void invalid_tss(void);
void segment(void);
void stack(void);
void gpf(void);
void page_fault(void);
void core_dump(char * message);



/*
 * install_exception
 *   DESCRIPTION: The function writes on the IDT table and fills in the the exceptions entries by calling write_int_gate function
 *	 INPUTS: none	  
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: the exceptions in the IDT are filled and "linked" to the correct handler 
 */

void install_exceptions(void)
{
	write_int_gate(0, divide);
	write_int_gate(1, debug);
	write_int_gate(2, nmi);
	write_int_gate(3, breakpoint);
	write_int_gate(4, overflow);
	write_int_gate(5, bound);
	write_int_gate(6, invalid_opcode);
	write_int_gate(7, device_not_available);
	write_int_gate(8, double_fault);
	write_int_gate(10, invalid_tss);
	write_int_gate(11, segment);
	write_int_gate(12, stack);
	write_int_gate(13, gpf);
	write_int_gate(14, page_fault);
}

/*
 * devide
 *   DESCRIPTION: Exception handler for deviding by zero
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Devide exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void divide(void)
{
	char message[] = "Divide exception";
	error_screan(message);
  while(1);
	iret();
}

/*
 * debug
 *   DESCRIPTION: Exception handler for debug
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Debug exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void debug(void)
{
	char message[] = "Debug exception";
	error_screan(message);
	while(1);
	iret();
}

/*
 * nmi
 *   DESCRIPTION: Exception handler for nmi
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Kernel received NMI" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void nmi(void)
{
	char message[] = "Kernel received NMI.";
	error_screan(message);
	while(1);
	iret();
}

/*
 * breakpoint
 *   DESCRIPTION: Exception handler for brekakpoint
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Kernel received breakpoint exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void breakpoint(void)
{
	char message[] = "Kernel received breakpoint exception.";
	error_screan(message);
	while(1);
	iret();
}

/*
 * overflow
 *   DESCRIPTION: Exception handler for overflow
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Overflow exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void overflow(void)
{
	char message[] = "Overflow exception";
	error_screan(message);
	while(1);
	iret();
}

/*
 * bound
 *   DESCRIPTION: Exception handler for bound range exceeded exception
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Bound range exceeded exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void bound(void)
{

	char message[] = "Bound range exceeded exception";
	error_screan(message);
	while(1);
	iret();
}

/*
 * invalid_opcode
 *   DESCRIPTION: Exception handler for invalid_opcode
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Invalid opcode exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void invalid_opcode(void)
{
	char message[] = "Invalid opcode exception";
	error_screan(message);
	while(1);
	iret();
}
/*
 * device_not_available
 *   DESCRIPTION: Exception handler for device_not_available
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "device_not_available exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void device_not_available(void)
{
	char message[] = "Device not available exception";
	error_screan(message);
	while(1);
	iret();
}
/*
 * double_fault
 *   DESCRIPTION: Exception handler for double_fault
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Double fault exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void double_fault(void)
{
	char message[] = "Double fault exception";
	error_screan(message);
	while(1);
	iret();
}
/*
 * invalid_tss
 *   DESCRIPTION: Exception handler for invalid tss
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Invalid TSS exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void invalid_tss(void)
{
	char message[] = "Invalid TSS exception";
	error_screan(message);
	while(1);
	iret();
}
/*
 * segment
 *   DESCRIPTION: Exception handler for segment
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Segment not present exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void segment(void)
{
	char message[] = "Segment not present exception";
	error_screan(message);
	while(1);
	iret();
}
/*
 * stack
 *   DESCRIPTION: Exception handler for stack segment
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Stack segment exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void stack(void)
{

	char message[] = "Stack segment exception";
	error_screan(message);
	while(1);
	iret();
}
/*
 * gpf
 *   DESCRIPTION: Exception handler for General protection fault
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "General protection fault exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void gpf(void)
{
	char message[] = "General protection fault exception";
	error_screan(message);
	while(1);
	iret();
}
/*
 * page_fault
 *   DESCRIPTION: Exception handler for page fault
 *	 INPUTS: none	  
 *   OUTPUTS: Prints "Page fault exception" on the shell
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the exception and spin to halt the system
 */
void page_fault(void)
{
	char message[] = "Page Fault Exception";
	error_screan(message);
	while(1);
	iret();
}

/*
 * core_dump
 *   DESCRIPTION: Prints useful information related to the error to help with debugging
 *	 INPUTS: char* -- Pointer to the error message string	  
 *   OUTPUTS: Prints "Page fault exception" on the terminal
 *   RETURN VALUE: none
 */
void core_dump(char * message)
{
	clear();
	pcb_t * curr = pcb_process();
	int32_t temp;
	int i;

	printf("Core dump - %s\n", message);
	printf("PCB - 0x%#x\n", curr);
	printf("PID - %d\n", (uint16_t) curr->pid);
	printf("Term - %d\n", curr->term);

	asm ("movl %%ebp, %0 "
			: "=r" (temp)
			:);

	asm ("movl %%esp, %0 "
			: "=r" (temp)
			:);


	printf("EBP - 0x%#x\n", temp);
	printf("ESP - 0x%#x\n", temp);
	for (i = 0; i < 64; i++)
	{
		asm ("movl (%%esp, %1, 4), %0"
				: "=r" (temp)
				: "r" (i));
		printf("0x%#x  ", temp);
		if(i%4==0)printf("\n");
	}


}

/*
 * error_screan
 *   DESCRIPTION: Prints the error message and the Scatman or core dump
 *	 INPUTS: char* -- Pointer to the error message string	  
 *   OUTPUTS: Prints "Page fault exception" on the terminal
 *   RETURN VALUE: none
 */
void error_screan(char *message)
{
	//Set the bacground blue
	outb(0x00,0x03C8);
	outb(0x00,0x03C9);
	outb(0x00,0x03C9);
	outb(0x3f,0x03C9);
	//Clear the screen
	clear();

	if(message == NULL)
		return;

	//The folowing code creates a modified message string for the Scatman error message
	int R_spaces, L_spaces, i;
	int length = strlen(message);
	char rightstr[MAX_STR_LENGTH];
	char leftstr[MAX_STR_LENGTH];

	//Cuts off excess from message to fit the image
	if(length > MAX_STR_LENGTH)
	{
		message[MAX_STR_LENGTH -1] = '\0';
		length = strlen(message);
	}

	//Center the message on the screen 
	if(length % 2) 
	{
		R_spaces = ((MAX_STR_LENGTH - length +1)/2) -1;
		L_spaces = ((MAX_STR_LENGTH - length +1)/2);
	}else
	{
		R_spaces = ((MAX_STR_LENGTH - length)/2);
		L_spaces = ((MAX_STR_LENGTH - length)/2);
	}

	//Greates a right string of right spaces 
	for(i = 0; i < R_spaces; i++)
	{
		rightstr[i] = ' ';
	}
	rightstr[i] = '\0';

	//Greates a right string of left spaces
	for(i = 0; i < L_spaces; i++)
	{
		leftstr[i] = ' ';
	}
	leftstr[i] = '\0';

	//prints R-spaces, message, L-spaces 
	//Print the error message of Scatman, Uncomment for fun
	//printf("                                                      ##################           #### #   ##    ####         ####                   ##################Efj        #    # #       ## #         ## #                   ######################:      #### ### ##    #### ####    #### #### ####         ####################            # # # ## ## ## # #  # ## ## # #  # #  #   tWW###########Lf,:    ##W          #### # # ##    #### ####    #### #### ####  :#######Wt             ##                                                 #         E#G #            #  ,f                                                #             j  ####   ###    ;                                                                  ii    LfL   :                                                                                                                                                                          ####         ####              ####                                             ## #         ## #              ## #                                             #### ####    ## # #### ####    #### #### ####             ###,,###              ## # # ## ## ## # #  # #  # ## ## # #  # #  #           ############            #### ## #    #### #### ####    #### #### ####           ,i#      #i,                                   #                 #                 ii  ii    #                                 #                 #                   ''     ###j:                                                                          # #####WL:                                                          f          EK i########                                                   ,GE####         :j   ########%s%s%s:#########,      ::     ########                                             G############              i#######                                            ##############               #######                                            ##############               ######", leftstr, message, rightstr);
	
	//Prints useful information related to the error to help with debugging
	core_dump(message);
}
