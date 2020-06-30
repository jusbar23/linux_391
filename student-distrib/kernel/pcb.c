#include "pcb.h"

// Function pointer interfaces

func_ptr rtc_driver[4] = { rtc_open, rtc_write, rtc_read, rtc_close };
func_ptr file_driver[4] = { file_open, file_write, file_read, file_close };
func_ptr dir_driver[4] = { dir_open, dir_write, dir_read, dir_close };
func_ptr terminal_driver[4] = { terminal_open, terminal_write, terminal_read, terminal_close };


/*
 * int pcb_init(pcb_t *pcb)
 *   DESCRIPTION: initializes a pcb and sets every value inside to its default and sets stdin/stdout
 *   INPUTS: pointer to a pcb 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: passed in pcb is initialized to the default value
 */

int 
pcb_init(pcb_t *pcb)
{

    int i;

    // Setup the PCB
		pcb->term = 0;

    for (i = 0; i < FD_MAX; i++)
    {
        pcb->elements[i].file_operation_jmp_tbl = NULL;
        pcb->elements[i].inode_ptr = NULL;
        pcb->elements[i].file_position = 0;
        pcb->elements[i].flags = 0;
    }

    // stdin
    pcb->elements[0].file_operation_jmp_tbl = terminal_driver;
    pcb->elements[0].flags = 1;

    // stdout
    pcb->elements[1].file_operation_jmp_tbl = terminal_driver;
    pcb->elements[1].flags = 1;
	

    return 0;
}

/*
 * int pcb_open(pcb_t * pcb, void * file_op_tbl)
 *   DESCRIPTION: opens a pcb with a file operation table pointer
 *   INPUTS: pcb - pointer to the pcb being modified
 *           file_op_tbl - The file operation table pointer (jumptable) 
 *   OUTPUTS: none
 *   RETURN VALUE: the fd on success, -1 on failure
 *   SIDE EFFECTS: the fd's jump table is initialized to the correct jump table in the passed in pcb
 */

int
pcb_open(pcb_t * pcb, void * file_op_tbl)
{
    int i;

    // Open up a new line in the PCB by finding the first open line
    for (i = 2; i < FD_MAX; i++)
    {
        if (!pcb->elements[i].flags)
        {
            pcb->elements[i].file_operation_jmp_tbl = file_op_tbl;
            pcb->elements[i].file_position = 0;
            pcb->elements[i].inode_ptr = NULL;
            pcb->elements[i].flags = 1;
            return i; // Return the line that was opened
        }
    }

    return -1;

}

/*
 * int pcb_close(pcb_t * pcb, uint8_t fd)
 *   DESCRIPTION: closes a fd in the passed in pcb
 *   INPUTS: pcb - pointer to the pcb being modified
 *           fd  - the fd to close
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: the fd is closed
 */

int 
pcb_close(pcb_t * pcb, uint8_t fd)
{
    // Sanity check
    if (fd >= FD_MAX)
        return -1;

    // Set the flag to closed
    pcb->elements[fd].flags = 0;
    return 0;
}

/*
 * pcb_t *pcb_process()
 *   DESCRIPTION: returns the current pcb for the process
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to the current pcb
 *   SIDE EFFECTS: none
 */

pcb_t *
pcb_process()
{
    pcb_t * new_pcb;
    uint32_t esp_curr;

    // Move the current esp to into the variable
    asm ("movl %%esp, %0"
        : "=r" (esp_curr)
        );

    new_pcb = (pcb_t *) (esp_curr & PCB_MASK);
    return new_pcb;
}

/*
 * pcb_t *get_pcb(int16_t pid)
 *   DESCRIPTION: returns the pointer to the pcb for a certain pid
 *   INPUTS: pid - pid to find a pcb
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to the requested pcb
 *   SIDE EFFECTS: none
 */

pcb_t *
get_pcb(int16_t pid)
{
		//start at end of kernel memory and move down by KERNEL_STACK_SIZE*PID
    uint32_t pcb_addr = KERNEL_MEM_END -1;
		pcb_addr -= KERNEL_STACK_SIZE*pid;

		//mask off bits to put PCB at lowest end of memory region
		pcb_addr &= (~KERNEL_STACK_SIZE + 1);
    return (pcb_t *) pcb_addr;
}
