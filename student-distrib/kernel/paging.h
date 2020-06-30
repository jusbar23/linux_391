#ifndef _PAGING_H_
#define _PAGING_H_

#include "../lib/lib.h"

#define	TABLE_ADDRESS_SHIFT 12   
#define TABLE_ADDRESS_MASK 0x3FF
#define VIDEO_MEM_START 0xB8
#define PAGE_SIZE 1024
#define DEFAULT_PD_ENTRY 0x00000002 /* User/Supervisor => Supervisor
                                        Read/Write => Write
                                        Present => False */
#define FOUR_KB 0x1000
#define FOUR_MB 0x400000

#define SIZE_OF_VIDEO_MEM FOUR_KB
#define KERNAL_START 0x400
#define KERNAL_END 0x800
#define PROGRAM_START 0x8000000 // The virtual memory location where programs are located
#define PROGRAM_OFFSET 0x48000 // offset to start loading the program into virtual memory


#define PTE_ADDR_MASK 0xFFFFF000
#define PTE_PRIVILEDGES 0x3 /* User/Supervisor => Supervisor 
                                Read/Write => Write 
                                Present => True */
#define WRITABLE 0x2
#define PRESENT 0x1
#define SUPERVISOR 0x4
#define LARGE_PAGE 0x080
#define P_IMG 0x20

#define VIDEO_MEM_LOAD 31

                        

//Page directory entry
typedef union pde_t {
	uint32_t val;
	struct {
		uint32_t present : 1;
		uint32_t read_write : 1;
		uint32_t user_supervisor : 1;
		uint32_t write_through : 1;
		uint32_t cache_disabled : 1;
		uint32_t accessed : 1;
		uint32_t zero : 1; 
		uint32_t page_size : 1;
		uint32_t global : 1;
		uint32_t avail : 3;
		uint32_t page_table_addr : 20;
	} __attribute__((packed));
} pde_t;

//Page table entry
typedef union pte_t {
	uint32_t val;
	struct {
		uint32_t present : 1;
		uint32_t read_write : 1;
		uint32_t user_supervisor : 1;
		uint32_t write_through : 1;
		uint32_t cache_disabled : 1;
		uint32_t accessed : 1;
		uint32_t dirty : 1;
		uint32_t zero : 1;
		uint32_t global : 1;
		uint32_t avail : 3;
		uint32_t physical_page_addr : 20;
	} __attribute__((packed));
} pte_t;

#define FLUSH_TLB(addr) \
do { \
	asm volatile("invlpg (%0)" \
		: \
		:"r" (addr) \
		: "memory"); \
} while (0);

#define RELOAD_CR3(addr) \
asm volatile(" movl %0, %%eax \n   \
    andl $0xFFFFFFE7, %%eax \n  \
    movl %%eax, %%cr3       \n  \
                                \
    movl %%cr4, %%eax       \n  \
    orl $0x00000090, %%eax  \n  \
    movl %%eax, %%cr4       \n  \
                                \
    movl %%cr0, %%eax       \n  \
    orl $0x80000000, %%eax  \n  \
    movl %%eax, %%cr0"          \
        :          			\
        : "r" (addr)		\
        : "cc", "%eax" );  

extern void paging_init();
extern int32_t paging_allocate(uint32_t pid);
extern int32_t paging_update_control(uint32_t pid);
extern int32_t paging_map_video(uint32_t pid, uint8_t ** screen_start);
extern void update_video_paging(uint16_t pid, uint32_t addr);

#endif
