#include "../kernel/paging.h"
#include "../kernel/tasks.h"

//Page Directory 
static pde_t page_dir_table[MAX_PID][PAGE_SIZE] __attribute__((aligned(PAGE_SIZE*4)));
//Page Table Entries
static pte_t page_table[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE * 4)));
static pte_t user_page_table[PAGE_SIZE] __attribute__((aligned (PAGE_SIZE * 4)));

static pte_t user_video_page[MAX_PID][PAGE_SIZE] __attribute__((aligned(PAGE_SIZE * 4)));
//uint32_t new_page_dir_addr;

/*
 * paging_init
 *   DESCRIPTION: this functions initilize the pading by setting up page directory and page table for the kernel and the video memory
 *   INPUTS:none 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Paging is enabled and being able to print page fault
 */

void 
paging_init(void){

    uint32_t i;
    uint32_t page_table_loc;

    for(i = 0; i < PAGE_SIZE; i++) 
    {
        page_table[i].present = 0;
        page_table[i].read_write = 0;
        page_table[i].user_supervisor = 0;
        page_table[i].write_through = 0;
        page_table[i].cache_disabled = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].zero = 0;
        page_table[i].global = 0;
        page_table[i].avail = 0;
        page_table[i].physical_page_addr = i;

        user_page_table[i].present = 0;
        user_page_table[i].read_write = 1;
        user_page_table[i].user_supervisor = 1;
        user_page_table[i].write_through = 0;
        user_page_table[i].cache_disabled = 0;
        user_page_table[i].accessed = 0;
        user_page_table[i].dirty = 0;
        user_page_table[i].zero = 0;
        user_page_table[i].global = 0;
        user_page_table[i].avail = 0;
        user_page_table[i].physical_page_addr = i;
    }
    page_table[VIDEO_MEM_START].present = 1;
    user_page_table[VIDEO_MEM_START].present = 1;


    page_dir_table[0][0].present = 1;
    page_dir_table[0][0].read_write = 0;
    page_dir_table[0][0].user_supervisor = 0;
    page_dir_table[0][0].write_through = 0;
    page_dir_table[0][0].cache_disabled = 0;
    page_dir_table[0][0].accessed = 0;
    page_dir_table[0][0].zero = 0;
    page_dir_table[0][0].page_size = 0;
    page_dir_table[0][0].global = 0;
    page_dir_table[0][0].avail = 0;
    page_table_loc = (uint32_t)page_table;
    page_dir_table[0][0].page_table_addr = page_table_loc >> TABLE_ADDRESS_SHIFT;


    page_dir_table[0][1].present = 1;
    page_dir_table[0][1].read_write = 1;
    page_dir_table[0][1].user_supervisor = 0;
    page_dir_table[0][1].write_through = 0;
    page_dir_table[0][1].cache_disabled = 0;
    page_dir_table[0][1].accessed = 0;
    page_dir_table[0][1].zero = 0;
    page_dir_table[0][1].page_size = 1;
    page_dir_table[0][1].global = 1;
    page_dir_table[0][1].avail = 0;
    page_dir_table[0][1].page_table_addr = KERNAL_START;


    for(i = 2; i < PAGE_SIZE; i++) 
    {
        page_dir_table[0][i].present = 0;
        page_dir_table[0][i].read_write = 1;
        page_dir_table[0][i].user_supervisor = 0;
        page_dir_table[0][i].write_through = 0;
        page_dir_table[0][i].cache_disabled = 0;
        page_dir_table[0][i].accessed = 0;
        page_dir_table[0][i].zero = 0;
        page_dir_table[0][i].page_size = 0;
        page_dir_table[0][i].global = 0;
        page_dir_table[0][i].avail = 0;
        page_dir_table[0][i].page_table_addr = i << 10;
    }

    //enables paging
    asm volatile(" movl $page_dir_table, %%eax \n   \
        andl $0xFFFFFFE7, %%eax \n  \
        movl %%eax, %%cr3       \n  \
                                    \
        movl %%cr4, %%eax       \n  \
        orl $0x00000010, %%eax  \n  \
        movl %%eax, %%cr4       \n  \
                                    \
        movl %%cr0, %%eax       \n  \
        orl $0x80000000, %%eax  \n  \
        movl %%eax, %%cr0"          \
            :                           
            : 
            : "cc", "%eax" );         

}

/*
 * int32_t paging_allocate(uint32_t pid)
 *   DESCRIPTION: this function creates a new page directory for a new program
 *   INPUTS: pid - the pid of the new program 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Paging is setup for the new user level program
 */
   
int32_t
paging_allocate(uint32_t pid)
{
    uint32_t new_page_table_addr;

    if(pid >= MAX_PID || pid < 0)
        return -1;

    uint32_t new_page_dir_table_addr;
    new_page_dir_table_addr = (uint32_t)(&page_dir_table[pid]);

    page_dir_table[pid][0].present = 1;
    page_dir_table[pid][0].read_write = 1;
    page_dir_table[pid][0].user_supervisor = 1;
    page_dir_table[pid][0].write_through = 0;
    page_dir_table[pid][0].cache_disabled = 0;
    page_dir_table[pid][0].accessed = 0;
    page_dir_table[pid][0].zero = 0;
    page_dir_table[pid][0].page_size = 0;
    page_dir_table[pid][0].global = 0;
    page_dir_table[pid][0].avail = 0;
    new_page_table_addr = (uint32_t) user_page_table;
    page_dir_table[pid][0].page_table_addr = new_page_table_addr >> TABLE_ADDRESS_SHIFT;

    page_dir_table[pid][1].present = 1;
    page_dir_table[pid][1].read_write = 1;
    page_dir_table[pid][1].user_supervisor = 0;
    page_dir_table[pid][1].write_through = 0;
    page_dir_table[pid][1].cache_disabled = 0;
    page_dir_table[pid][1].accessed = 0;
    page_dir_table[pid][1].zero = 0;
    page_dir_table[pid][1].page_size = 1;
    page_dir_table[pid][1].global = 1;
    page_dir_table[pid][1].avail = 0;
    page_dir_table[pid][1].page_table_addr = KERNAL_START;

    page_dir_table[pid][P_IMG].present = 1;
    page_dir_table[pid][P_IMG].read_write = 1;
    page_dir_table[pid][P_IMG].user_supervisor = 1;
    page_dir_table[pid][P_IMG].write_through = 0;
    page_dir_table[pid][P_IMG].cache_disabled = 0;
    page_dir_table[pid][P_IMG].accessed = 0;
    page_dir_table[pid][P_IMG].zero = 0;
    page_dir_table[pid][P_IMG].page_size = 1;
    page_dir_table[pid][P_IMG].global = 0;
    page_dir_table[pid][P_IMG].avail = 0;
    page_dir_table[pid][P_IMG].page_table_addr = (pid+1) << 10;


    return 0;
}

/*
 * int32_t paging_update_control(uint32_t pid)
 *   DESCRIPTION: this function changes the page directory for the appropriate user level program
 *   INPUTS: pid - the pid of the program to switch control to 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Paging is switched to the user level program
 */
int32_t
paging_update_control(uint32_t pid)
{

    if(pid >= MAX_PID || pid < 0)
        return -1;

    uint32_t sw_page_dir = (uint32_t)(&page_dir_table[pid]);

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
        :                           
        : "r" (sw_page_dir)
        : "cc", "%eax" );  

    return 0;
}

/*
 * int32_t paging_map_video(uint32_t pid, uint8_t ** screen_start)
 *   DESCRIPTION: this function is used mainly by vidmap to initialize page direction entry 31 offset 0xB8 as the video memory page and modifies the pointer to point there
 *   INPUTS: pid - The current pid of the program
 *           screen_start - pointer to pointer to modify with the new video memory pointer
 *   OUTPUTS: None
 *   RETURN VALUE: 0 for success, -1 for failure
 *   SIDE EFFECTS: Video memeory is paged to 124 MB + 0xB8000 KB offset
 */


int32_t 
paging_map_video(uint32_t pid, uint8_t ** screen_start)
{
    int i;
    // int32_t four_mb_offset = ((int32_t) screen_start) / FOUR_MB;
    // int32_t four_kb_offset = (((int32_t) screen_start % FOUR_MB) / FOUR_KB);

    // Sanity check
    if (pid < 0 || pid >= MAX_PID)
        return -1;

    // Initialize the new page
    for (i = 0; i < PAGE_SIZE; ++i)
    {
        user_video_page[pid][i].present = 0;
        user_video_page[pid][i].read_write = 1;
        user_video_page[pid][i].user_supervisor = 0;
        user_video_page[pid][i].write_through = 0;
        user_video_page[pid][i].cache_disabled = 0;
        user_video_page[pid][i].accessed = 0;
        user_video_page[pid][i].dirty = 0;
        user_video_page[pid][i].zero = 0;
        user_video_page[pid][i].global = 0;
        user_video_page[pid][i].avail = 0;
        user_video_page[pid][i].physical_page_addr = 0;
    }


    user_video_page[pid][VIDEO_MEM_START].physical_page_addr = VIDEO_MEM_START;
    user_video_page[pid][VIDEO_MEM_START].present = 1;
    user_video_page[pid][VIDEO_MEM_START].user_supervisor = 1;

    page_dir_table[pid][VIDEO_MEM_LOAD].present = 1;
    page_dir_table[pid][VIDEO_MEM_LOAD].read_write = 1;
    page_dir_table[pid][VIDEO_MEM_LOAD].user_supervisor = 1;
    page_dir_table[pid][VIDEO_MEM_LOAD].write_through = 0;
    page_dir_table[pid][VIDEO_MEM_LOAD].cache_disabled = 0;
    page_dir_table[pid][VIDEO_MEM_LOAD].accessed = 0;
    page_dir_table[pid][VIDEO_MEM_LOAD].zero = 0;
    page_dir_table[pid][VIDEO_MEM_LOAD].page_size = 0;
    page_dir_table[pid][VIDEO_MEM_LOAD].global = 0;
    page_dir_table[pid][VIDEO_MEM_LOAD].avail = 0;


    // Assign the video memory location in the appropriate place in the current page directory with the user_video_page
    uint32_t page_table_loc = (uint32_t)user_video_page[pid];
    page_dir_table[pid][VIDEO_MEM_LOAD].page_table_addr = page_table_loc >> TABLE_ADDRESS_SHIFT;

    // Change the pointer for the video memory in the user level program with the appropriate location for the video memory
    *screen_start = (uint8_t *) (PROGRAM_START - FOUR_MB) + (VIDEO_MEM_START * FOUR_KB);

    RELOAD_CR3((uint32_t)(&page_dir_table[pid]));

    return 0;

}

/*
 * void update_video_paging(uint16_t pid, uint32_t addr)
 *   DESCRIPTION: this function is used when the scheduler needs to switch programs that is vidmapped, swaps in the user video page
 *   INPUTS: pid - The current pid of the program
 *           addr - address of the page
 *   OUTPUTS: None
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Video memory is repaged
 */

void
update_video_paging(uint16_t pid, uint32_t addr)
{
    // Update the control
	user_video_page[pid][VIDEO_MEM_START].physical_page_addr = (addr >> TABLE_ADDRESS_SHIFT) & TABLE_ADDRESS_MASK;

	RELOAD_CR3((uint32_t)(&page_dir_table[pid]));
}
