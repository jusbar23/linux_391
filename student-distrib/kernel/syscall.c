#include "syscall.h"
#include "../drivers/termios.h"
#include "tasks.h"
#include "pcb.h"
#include "asm_linkage.h"
#include "../x86_desc.h"
#include "paging.h"
#include "scheduling.h"

/* 
    File operation jump table for the open command
*/

extern int (*rtc_driver[4]);
extern int (*file_driver[4]);
extern int (*dir_driver[4]);
extern int (*terminal_driver[4]);

/*
 * int32_t syscall_halt (uint8_t status)
 *   DESCRIPTION: Halts the user level program
 *   INPUTS: status - return value of the user level program
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: halts user level program and gives control back to the parent program
 */

int32_t 
syscall_halt (uint8_t status)
{
    int i;
    uint8_t tempstatus = status;

		//begin critical section
		cli(); //have to use cli here. can't push flags to stack because we will switch stacks

    pcb_t * curr = pcb_process();
    pcb_t * c_parent_pcb = (pcb_t *) curr->parent_pcb;

    if (curr->parent_pcb == NULL)
    {
        ///Should not halt shell
        tasks_pid_free(curr->pid);
        asm volatile("movl %0, %%esp"       \
                      :: "r" (KERNEL_STACK(curr->pid)));

				//release lock
				sti();

        syscall_init_shell(curr->term);
        // syscall_restart_shell (c_parent_pcb)
        return -1;
    }

    // Close all opened files except stdin and stdout
    for (i = 2; i < FD_MAX; i++) if (curr->elements[i].flags) syscall_close(i);

    //remove child PCB
    c_parent_pcb -> child = NULL;
    
    //free this task's pid
    tasks_pid_free(curr->pid);


  //schedule this task and unschedule parent
  
    schedule_task(c_parent_pcb->pid);
  unschedule_task(curr->pid);
  

    if (paging_update_control(c_parent_pcb -> pid) != 0)
        return -1;

    //set kernal stack position in tasks
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_STACK(c_parent_pcb->pid);

    //put the "parent_esp" into the %esp
    asm volatile("movl %0, %%esp"       \
                 ::"r"(c_parent_pcb->esp_reg));
    //put the "parent_esp" into the %ebp
    asm volatile("movl %0, %%ebp"       \
                 ::"r"(c_parent_pcb->ebp_reg));
                 
    // store status into eax for return value
    asm volatile ("movl %0, %%eax"  \
               ::"r" ((int32_t) status & STATUS_MASK)
                   : "eax", "cc");

    status = tempstatus;
    asm volatile ("movl %0, %%eax"  \
                   ::"r" ((int32_t) status & STATUS_MASK)
                   : "eax", "cc");

		//end critical section
    sti();

    return 0;
}

/*
 * int32_t syscall_execute (const uint8_t * command)
 *   DESCRIPTION: Executes the user level program passed in 
 *   INPUTS: command - the name of the command
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: executes user level code, doesn't return back from this until the user level code is done executing
 */

int32_t 
syscall_execute (const uint8_t * command)
{
    uint8_t fname[BUFFER_LENGTH];
    uint8_t fargs[IN_BUF_SIZE];
  uint16_t pid;
    uint32_t i, fname_length = 0, file_flag = 0,  entry_point;
    int32_t flags;
    dentry_t dentry;
    pcb_t * curr = pcb_process(), *newPCB;

    //Check for an invalid command.
    if( command == NULL )
        return -1;

    //Parse file name and arguments
    for (i = 0, file_flag = 0; command[i] != '\0'; i++)
    {
        if(command[i] == ' ' && file_flag == 0)
        {
            file_flag = 1;
            fname_length = i;
            fname[i] = '\0';
        }
        else if(file_flag == 1)
            fargs[i-fname_length-1] = command[i];
        else if(file_flag == 1 && command[i] == ' ')
            fname[i] = command[i];
        else if(i >= BUFFER_LENGTH && file_flag == 0)
                return -1;
        else 
            fname[i] = command[i];
    }
    fargs[i-fname_length - 1] = '\0';

    if(file_flag == 0)
        fname[i] = '\0';

    // Check if its an executable

    if (read_dentry_by_name((char *) fname, &dentry))
        return -1;

    if (executable_check(&dentry))
        return -1;

  pid = tasks_pid_new();

    // out of pids
    if (pid == NEGATIVE_1)
        return -1;
    
  
		//begin critical section
		cli_and_save(flags); // Do we need this? How do we reenable interrupts afterwards
  
    
    // 3) setup paging and load CR3, flush TLB
    paging_allocate(pid);
    paging_update_control(pid);
		
    // 4) Call loader
    entry_point = loader((char *) fname);
    if (entry_point == -1)
    {
        tasks_pid_free(pid);
        paging_update_control(curr->pid);
        restore_flags(flags);
        return -1;
    }
	
    //schedule this task and unschedule parent
    schedule_task(pid);
    unschedule_task(curr->pid);

    // 5) Setup PCB
    newPCB = get_pcb(pid);
    pcb_init(newPCB);


    newPCB->pid = pid;
    newPCB->esp_reg = ((uint32_t)(newPCB)) + KERNEL_STACK_SIZE - 4;
	newPCB->term = curr->term;
	newPCB->parent_pcb = (struct pcb_t *) curr;

    // Store the args passed to this function into the PCB.
    strcpy((int8_t*)newPCB->args, (const int8_t*)fargs);
    
    tss.ss0 = KERNEL_DS;
    tss.esp0 = newPCB->esp_reg;

  //store the current process's kernel base pointer into its pcb
  uint32_t parent_k_ebp;
  asm volatile( "movl %%ebp, %0 "  
                 :"=r" (parent_k_ebp)
                 :
              );
  curr->ebp_reg = parent_k_ebp;

  //store the current process's kernel stack ptr into its pcb
  uint32_t parent_k_esp;
  asm volatile( "movl %%esp, %0 "  
                 :"=r" (parent_k_esp)
                 :
              );
  curr->esp_reg = parent_k_esp;

    asm volatile (
        "    xorl %%eax,%%eax    \n\
             movw $0x2B, %%ax         \n\
             movw %%ax, %%ds                       \n\
             movw %%ax, %%es                       \n\
             movw %%ax, %%fs                       \n\
             movw %%ax, %%gs                       \n\
             movl $0x83FFFFC, %%eax                    \n\
             pushl $0x2B             \n\
             pushl $0x83FFFFC                       \n\
             pushl %1                  \n\
             pushl $0x23    \n\
             pushl %0   \n\
             iret   "
            : //no outputs
            : "r" (entry_point), "r" (flags)
            : "eax", "memory"
        );

    return 0;
}

/*
 * int32_t syscall_init_shell (uint8_t term_num)
 *   DESCRIPTION: Executes a new shell and assigns it a terminal number for which to use the appropriate buffer 
 *   INPUTS: term_num - the number of buffer to use
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: executes a new shell that is assigned to an appropriate terminal buffer
 */

int32_t 
syscall_init_shell (uint8_t term_num)
{
    uint8_t fname[BUFFER_LENGTH];
    uint8_t fargs[IN_BUF_SIZE];
    uint16_t pid;
    uint32_t i, fname_length = 0, file_flag = 0,  entry_point;
    int32_t flags;
    const uint8_t command[] = "shell";
    dentry_t dentry;
    pcb_t * curr = pcb_process(), *newPCB;

    //Check for an invalid command.
    if( command == NULL )
        return -1;

    //Parse file name and arguments (tokenize)
    for (i = 0, file_flag = 0; command[i] != '\0'; i++)
    {
        if(command[i] == ' ' && file_flag == 0)
        {
            file_flag = 1;
            fname_length = i;
            fname[i] = '\0';
        }
        else if(file_flag == 1)
            fargs[i-fname_length-1] = command[i];
        else if(file_flag == 1 && command[i] == ' ')
            fname[i] = command[i];
        else if(i >= BUFFER_LENGTH && file_flag == 0)
                return -1;
        else 
            fname[i] = command[i];
    }
    fargs[i-fname_length - 1] = '\0';

    if(file_flag == 0)
        fname[i] = '\0';

    // Check if its an executable

    if (read_dentry_by_name((char *) fname, &dentry))
        return -1;

    // Check for magic numbers
    if (executable_check(&dentry))
        return -1;

    pid = tasks_pid_new();

    // Check if its negative 1
    if (pid == NEGATIVE_1)
        return -1;
    
    
    //begin critical section
    cli_and_save(flags); // Do we need this? How do we reenable interrupts afterwards
    
    // 3) setup paging and load CR3, flush TLB
    paging_allocate(pid);
    paging_update_control(pid);

    // 4) Call loader
    entry_point = loader((char *) fname);
    if (entry_point == -1)
    {
        tasks_pid_free(pid);
        paging_update_control(curr->pid);
        restore_flags(flags);
        return -1;
    }
    
    // open it's terminal
    terminal_open((uint8_t*)( term_num + 0l));

    //schedule this task
    schedule_task(pid);

    // 5) Setup PCB
    newPCB = get_pcb(pid);
    pcb_init(newPCB);


    newPCB->parent_pcb = NULL;
    newPCB->pid = pid;
    newPCB->esp_reg = ((uint32_t)(newPCB)) + KERNEL_STACK_SIZE - 4;
    newPCB->term = term_num;
    // Store the args passed to this function into the PCB.
    strcpy((int8_t*)newPCB->args, (const int8_t*)fargs);
    
    tss.ss0 = KERNEL_DS;
    tss.esp0 = newPCB->esp_reg;

    //store the current process's kernel base pointer into its pcb
    uint32_t parent_k_ebp;
    asm volatile( "movl %%ebp, %0 "  
                       :"=r" (parent_k_ebp)
                                 :
                            );
    curr->ebp_reg = parent_k_ebp;

    //store the current process's kernel stack ptr into its pcb
    uint32_t parent_k_esp;
    asm volatile( "movl %%esp, %0 "  
                       :"=r" (parent_k_esp)
                                 :
                            );
    curr->esp_reg = parent_k_esp;

    asm volatile (
        "    xorl %%eax,%%eax    \n\
             movw $0x2B, %%ax         \n\
             movw %%ax, %%ds                       \n\
             movw %%ax, %%es                       \n\
             movw %%ax, %%fs                       \n\
             movw %%ax, %%gs                       \n\
             movl $0x83FFFFC, %%eax                    \n\
             pushl $0x2B             \n\
             pushl $0x83FFFFC                       \n\
             pushl %1                  \n\
             pushl $0x23    \n\
             pushl %0   \n\
             iret   "
            : //no outputs
            : "r" (entry_point), "r" (flags)
            : "eax", "memory"
        );

    return 0;
}

/*
 * int32_t syscall_read (int32_t fd, void *buf, int32_t nbytes)
 *   DESCRIPTION: Performs a read on the passed in fd, calls the read operation in the fd's jump table 
 *   INPUTS: fd - file to perform read operation on
 *           buf - buffer to read into
 *           nbytes - number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: If it's a regular file it will increase the offset in the pcb
 */

int32_t 
syscall_read (int32_t fd, void *buf, int32_t nbytes)
{
    pcb_t * curr = pcb_process();

    // Sanity check
    if (nbytes == 0) return 0;
    if (fd >= FD_MAX || fd < FD_MIN || fd == 1) return -1;

    // Check if the current fd is enabled or not
    if (curr->elements[fd].flags == 0) return -1;

    if (buf == NULL) return -1;
    return (*(curr->elements[fd].file_operation_jmp_tbl[2]))(fd, buf, nbytes);
}

/*
 * int32_t syscall_write (int32_t fd, const void * buf, int32_t nbytes)
 *   DESCRIPTION: Performs a write on the passed in fd, calls the write operation in the fd's jump table 
 *   INPUTS: fd - file to perform write operation on
 *           buf - buffer to write into
 *           nbytes - number of bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: If it's a regular file it will increase the offset in the pcb
 */

int32_t 
syscall_write (int32_t fd, const void * buf, int32_t nbytes)
{
    pcb_t * curr = pcb_process();

    if (fd >= FD_MAX || fd < FD_MIN || nbytes == 0 || fd == 0) return -1;

    if (buf == NULL) return -1;


    // FD isn't open in the first place
    if (curr->elements[fd].flags == 0) return -1;

    if (fd == curr->rtc_fd)
        curr->rtc_rate = *((int *) buf);

    return (*(curr->elements[fd].file_operation_jmp_tbl[1]))(fd, buf, nbytes);
}

/*
 * int32_t syscall_open (const uint8_t * filename)
 *   DESCRIPTION: Performs a open on the passed in fd, calls the open operation in the fd's jump table 
 *   INPUTS: filename - Opens the filename
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: opens the fd in the pcb
 */

int32_t 
syscall_open (const uint8_t * filename)
{
    int fd;

    pcb_t * curr = pcb_process();
    // Sanity check
    if (filename == NULL) return -1;

    // Open the file
    fd = file_open(filename);
    if (fd == -1) return -1;

    if (strncmp("rtc", (char*)filename, 3) != -1)
        curr->rtc_fd = fd;

    (*(curr->elements[fd].file_operation_jmp_tbl[0]))();
    return fd;
}

/*
 * int32_t syscall_close (int32_t fd)
 *   DESCRIPTION: Performs a open on the passed in fd, calls the open operation in the fd's jump table 
 *   INPUTS: fd - fd to close
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: opens the fd in the pcb
 */

int32_t 
syscall_close (int32_t fd)
{
    pcb_t * curr = pcb_process();

    // Sanity check
    if (fd >= FD_MAX || fd < 2) return -1;


    // File isn't open in the first place
    if (curr->elements[fd].flags == 0) return -1;

    // Set the flag to 0
    pcb_close(curr, fd);

    // Run the call for the appropriate file
    return (*(curr->elements[fd].file_operation_jmp_tbl[3]))(fd, NULL, 0);
}

/*
 * int32_t syscall_getargs (uint8_t * buf, int32_t nbytes)
 *   DESCRIPTION: Copies the arguments into the PCB of the current user level program
 *   INPUTS: buf - buffer to copy to
 *           nbytes - number of bytes to coy
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: copys data into the pcb
 */

int32_t 
syscall_getargs (uint8_t * buf, int32_t nbytes)
{

    //Check for invalid parameters
    if( buf == NULL || nbytes == 0 )
        return -1;

    pcb_t * curr = pcb_process();
    
    //Should not be requesting more than fits in the buffer
    if( strlen((const int8_t*)curr->args) > nbytes )
        return -1;
    //Copy the args
    strcpy((int8_t*)buf, (const int8_t*)curr->args);
    
    return 0;
}

/*
 * int32_t syscall_vidmap (uint8_t ** screen_start)
 *   DESCRIPTION: Maps the video memory for the current process and change the pointer passed in to point to the newly mapped video memory
 *   INPUTS: screen_start - ptr to ptr of video memory
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: changes the page setup and sets the vidmap flag in the pcb to true
 */

int32_t 
syscall_vidmap (uint8_t ** screen_start)
{
    pcb_t * curr = pcb_process();

    if (screen_start == 0)
        return -1;

    if ((uint32_t) screen_start < KERNEL_MEM_END)
        return -1;

    // Map the screen_start for the current pid
    if (paging_map_video(curr->pid, screen_start) == -1)
        return -1;
    
  curr->vidmap = 1;

    return VIDEO_MEM_LOC;
}

int32_t 
syscall_set_handler (int32_t signum, void * handler_address)
{
    return 0;
}

int32_t 
syscall_sigreturn (void)
{
    return 0;
}
