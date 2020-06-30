#ifndef _PCB_H_
#define _PCB_H_

#include "../lib/types.h"
#include "../drivers/fs.h"
#include "../drivers/termios.h"
#include "../drivers/rtc.h"
#include "../drivers/pit.h"

//first byte after kernel memory
#define KERNEL_MEM_END 0x00800000
//size of per-process kernel stack
#define KERNEL_STACK_SIZE 0x2000
#define PCB_MASK 0xFFFFE000
#define FD_MAX 8 


typedef int (*func_ptr)();

typedef struct {
    func_ptr * file_operation_jmp_tbl;
    uint32_t inode_ptr; 
    uint32_t file_position;
    uint32_t flags; // open/close
} __attribute__((packed)) file_descriptor_element_t;

typedef struct {
	file_descriptor_element_t elements[8]; // Files opened
    struct pcb_t *parent_pcb; // For the execute sys call
    struct pcb_t *child;
    uint32_t esp_reg; // kernel stack pointer
	uint32_t ebp_reg; // kernel base pointer
    uint16_t pid;
    uint8_t args[100];
    uint8_t vidmap;
	int term; //index of terminal the process is attached to
    uint8_t rtc_fd;
    uint8_t rtc;
    int rtc_rate;
} __attribute__((packed)) pcb_t;


extern int pcb_init(pcb_t *pcb);
extern int pcb_close(pcb_t * pcb, uint8_t fd);
extern int pcb_open(pcb_t * pcb, void * file_op_tbl);
extern pcb_t * pcb_process();
extern pcb_t * get_pcb(int16_t pid);


#endif
