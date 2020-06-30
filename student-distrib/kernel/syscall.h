#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "pcb.h"

#define SYSCALL_HALT 1
#define SYSCALL_EXECUTE 2
#define SYSCALL_READ 3
#define SYSCALL_WRITE 4
#define SYSCALL_OPEN 5
#define SYSCALL_CLOSE 6
#define SYSCALL_GETARGS 7
#define SYSCALL_VIDMAP 8
#define SYSCALL_SET_HANDLER 9
#define SYSCALL_SIGRETURN 10
#define ENTRY_POINT_OFFSET 24
#define DEFAULT_STACK 0x800000 - 4
#define INITIAL_PID 1
#define INITIAL_PAGING 0
#define BUFFER_LENGTH 32
#define FD_MAX 8
#define FD_MIN 0
#define VIDEO_MEM_LOC 0x80000000 - 0x400000 + (0xB8 * 0x1000)
#define NEGATIVE_1 0xFFFF
#define STATUS_MASK 0x000000FF

int32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

int32_t syscall_halt (uint8_t status);
int32_t syscall_execute (const uint8_t * command);
int32_t syscall_read (int32_t fd, void *buf, int32_t nbytes);
int32_t syscall_write (int32_t fd, const void * buf, int32_t nbytes);
int32_t syscall_open (const uint8_t * filename);
int32_t syscall_close (int32_t fd);
int32_t syscall_getargs (uint8_t * buf, int32_t nbytes);
int32_t syscall_vidmap (uint8_t ** screen_start);
int32_t syscall_set_handler (int32_t signum, void * handler_address);
int32_t syscall_sigreturn (void);
int32_t syscall_init_shell (uint8_t term_num);

#endif
