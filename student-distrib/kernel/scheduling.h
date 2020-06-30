#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "../drivers/i8259.h"
#include "../drivers/pit.h"
#include "syscall.h"
#include "../drivers/termios.h"

//initialize scheduling-related data structures
void scheduling_init(void);

//find next task to execute and switch to it
void scheduler_tick(void);

//mark a pid as runnable
void schedule_task(uint16_t pid);

//unmark a pid as runnable
void unschedule_task(uint16_t pid);
#endif
