#include "scheduling.h"
#include "tasks.h"
#include "../x86_desc.h"
#include "paging.h"

//array of flags. Each flag is set when a pid is scheduled to run
static uint8_t sched[MAX_PID]; //bitmask of schedulable tasks


/*
 *  scheduling_init -- initialize all scheduling-related data structures
 *   INPUTS:  none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void 
scheduling_init(void)
{
	int i; //iterator

	//mark all tasks unscheduled
	for(i=0; i<=MAX_PID; i++)
		sched[i] = 0;
	//print kernel message
	printf("Enabled Scheduling\n");
	return;
}

/*
 *  schedule_task -- mark a task as runnable
 *   INPUTS:  pid -- pid of task to be scheduled
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void 
schedule_task(uint16_t pid)
{
	//mark the pid as runnable in the sched array
	sched[pid] = 1;
	return;
}

/*
 *  unschedule_task -- unmark a task so that it will not be scheduled
 *   INPUTS:  pid -- pid of task to be unscheduled
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
unschedule_task(uint16_t pid)
{
	//mask the pid in the sched array
	sched[pid] = 0;
	return;
}


void
/*
 *  scheduler_tick -- looks for next task to run and switches to it
 *                    Called by PIT handler with interrupts masked.
 *   INPUTS:  none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: does context switch to next task to execute
 */
scheduler_tick(void)
{
	uint32_t i; //iterator
	uint32_t next_pid;  //pid of next task to be scheduled
	pcb_t *context = pcb_process(); // the current context
	pcb_t *next_context; // the pcb of the next task to be scheduled

  //Look to see if any tasks are scheduled. If none are, return and don't do the
	//context switch
	for(i = 0; i <= MAX_PID; i++)
	{
		if( i == MAX_PID) //if we reached the end without finding any scheduled processes
			return;         //return without doing context switch

		if(sched[i] == 1) break; //if we find one that is scheduled, break
	}	
	
	//Find pid of next task to schedule. Start from current pid and look upward
	for(i = (context->pid+1) % MAX_PID; i != context->pid; i = (i+1) % MAX_PID)
	{
		if(sched[i] == 1) //if it's scheduled, break. 
		{
			next_pid = i; //i holds the next pid to run
			break;
		}
		else
		{
			next_pid = context->pid; //default to the current pid 

		}
	}


	//Obtain the PCB for next process
	next_context = get_pcb(next_pid);

	//Store ESP and EBP of the current process 
	//store the current process's kernel base pointer into its pcb
	uint32_t parent_k_ebp;
	asm volatile( "movl %%ebp, %0 "  
								 :"=r" (parent_k_ebp)
								 :
							);
	context->ebp_reg = parent_k_ebp;

	//store the current process's kernel stack ptr into its pcb
	uint32_t parent_k_esp;
	asm volatile( "movl %%esp, %0 "  
								 :"=r" (parent_k_esp)
								 :
							);
	context->esp_reg = parent_k_esp;

	//update tss fields
	tss.ss0 = KERNEL_DS;
	tss.esp0 = KERNEL_STACK(next_pid);
	
	//set CR3 to next task's page directory
	paging_update_control(next_pid);

	/*
	if (next_context->rtc == 1)
	{
		rtc_write(next_context->rtc_fd, &next_context->rtc_rate, 4);
	}
  */
  //if the next process has requested vidmap, set it up for them
	if (next_context->vidmap == 1)
	{
		//our mapping of video memory depends on whether the next process has
		//control of the active terminal
		if (is_active_term(next_context->term))
		{
			//if it's active, map video memory to video memory
			update_video_paging(next_context->pid, VIDEO_MEM);
		}
		else
		{
			//if it's not active, map video memory to the text backbuffer of the next
			//process's terminal
			uint32_t term = term_data_ptr(next_context->term);
			update_video_paging(next_context->pid, term);
		}
	}

	//put the next task's esp into the %esp
	asm volatile("movl %0, %%esp"       \
							 ::"r"(next_context->esp_reg));
	//put the next task's ebp into the %ebp
	asm volatile("movl %0, %%ebp"       \
							 ::"r"(next_context->ebp_reg));
}
