#ifndef _IRQ_H
#define _IRQ_H

#include "../x86_desc.h"
#include "../lib/lib.h"
#include "../drivers/rtc.h"
#include "../drivers/pit.h"
#include "asm_linkage.h"

/* write interrept gate into idt */
extern void write_int_gate(int n, void (*handler)(void));

/* write system gate into idt */
void write_sys_gate(int n, void (*handler)(void));
/* write kernel trap gate into idt */
void write_trap_gate(int n, void (*handler)(void));

/* fill IDT with null handlers */ 
void init_idt();

#define iret()                           \
do {                                    \
	asm volatile("iret"                  \
			:                       \
			:                       \
			: "memory", "cc"        \
			);                      \
} while(0)                    

#endif 
