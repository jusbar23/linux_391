#include "tasks.h"

// Array of all the pids usable for in the system
static uint8_t pid_usage[MAX_PID];

// static uint8_t pid_usage_vector[MAX_PID/sizeof(uint8_t)];
// static uint8_t pid_usage_vector[MAX_PID/(sizeof(uint8_t)*8)];

/*
 * void init_tasks()
 *   DESCRIPTION: Initializes the pid usage vector so none are taken 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

void
init_tasks()
{
	//initialize bit_usage_vector to 0
	int i;
	for (i = 1; i < MAX_PID; i++)
	{
		pid_usage[i] = FREE; // 0 means it is no longer in use
	}
	// memset(pid_usage_vector, 0, MAX_PID/sizeof(uint8_t));
}


/*
 * int16_t tasks_pid_new()
 *   DESCRIPTION: get a new pid from the vector and set the vector for that pid to being used 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: the pid for the new process
 *   SIDE EFFECTS: the usage vector for that pid is set to 1 stating that it is in use
 */


int16_t
tasks_pid_new()
{
	int i; //iterator

	//start at PID 1. 
	for(i=1; i<MAX_PID; i++)
	{
		if (pid_usage[i] == FREE)
		{
			pid_usage[i] = IN_USE;
			return i;
		}
		//if ith bit in usage vector is unset, break
		// if( !(pid_usage_vector [i/sizeof(uint8_t)] & (0x1 << (i%sizeof(uint8_t)))))
		// {
		// 	pid_usage_vector [i/sizeof(uint8_t)] |= (0x1 << (i%sizeof(uint8_t)));
		// 	return i;
		// }
			
	}
	//no more PID's left :(
	return -1;
}

/*
 * void tasks_pid_free(int16_t pid)
 *   DESCRIPTION: free the passed in pid so it can be used by other programs
 *   INPUTS: pid - the pid to free
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: the usage vector for that pid is set to 0 stating it is no longer in use
 */

void
tasks_pid_free(int16_t pid)
{
	// pid_usage_vector[pid/sizeof(uint8_t)] &= 
		// ~(0x1 << (pid%sizeof(uint8_t)));
	pid_usage[pid] = FREE;
}

