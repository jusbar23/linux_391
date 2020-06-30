#ifndef ASM_LINKAGE_H
#define ASM_LINKAGE_H

#ifndef ASM

#include "syscall.h"

//the linkage
extern void keyboard_linkage();
extern void rtc_linkage();
extern void syscall_linkage();
extern void pit_linkage();
extern void _jump_rings(uint32_t entry);

//ECE 391 system call library
extern int32_t ece391_halt (uint8_t status);
extern int32_t ece391_execute (const uint8_t* command);
extern int32_t ece391_init_shell (const uint8_t* command);
extern int32_t ece391_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t ece391_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t ece391_open (const uint8_t* filename);
extern int32_t ece391_close (int32_t fd);
extern int32_t ece391_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t ece391_vidmap (uint8_t** screen_start);
extern int32_t ece391_set_handler (int32_t signum, void* handler);
extern int32_t ece391_sigreturn (void);

#endif
#endif
