#ifndef _TASKS_H_
#define _TASKS_H_

#include "pcb.h"
#define FREE 0
#define IN_USE 1

//the max pid (inclusive)
#define MAX_PID 16

//get address of a pid's kernel stack
#define KERNEL_STACK(next_pid) 0x800000 - (0x2000 * next_pid) - 4

void init_tasks();
int16_t tasks_pid_new();
void tasks_pid_free(int16_t);
#endif
