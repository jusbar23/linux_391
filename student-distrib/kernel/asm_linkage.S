#define ASM 1
#include "asm_linkage.h"

.globl syscall_linkage, _jump_rings
.globl keyboard_linkage, rtc_linkage, pit_linkage
.globl syscall_init_shell, syscall_halt, syscall_execute, syscall_read, syscall_write, syscall_open, syscall_close, syscall_getargs, syscall_vidmap, syscall_set_handler, syscall_sigreturn
.align 4

#keyboard_linkage
#DESCRIPTION: assembly linkage to call the keyboard handler. This linkage saves and restores all the registers
#OUTPUT : none
#RETURN VALUE : none
#SIDE EFFECTS: Link the jumptable and the handler without modifying the stack before call the handler

keyboard_linkage:
    pushal
    call keyboard_handler
    popal
    iret

#keyboard_linkage
#DESCRIPTION: assembly linkage to call the rtc_irq_handler handler. This linkage saves and restores all the registers
#OUTPUT : none
#RETURN VALUE : none
#SIDE EFFECTS: Link the jumptable and the handler without modifying the stack before call the handler

rtc_linkage:
     pushal
     call rtc_irq_handler
     popal
     iret

pit_linkage:
    pushal
    call pit_handler
    popal
    iret

syscall_linkage:
    #Push arg registers to stack for c syscall linkage 
    #syscall_handler(EAX, EBX, ECX, EDX) , return value into EAX
    pushl %edx
    pushl %ecx
    pushl %ebx

    cmpl $1, %eax
    jl syscall_failure
    cmpl $11, %eax
    jg syscall_failure

do_syscall:
    #Call c syscall handler function
    call *__syscalls_jumptable(, %eax, 4)

cleanup_syscall:
    #cleanup stack frame
    popl %ebx
    popl %ecx
    popl %edx

    #return
    iret

syscall_failure:
    movl $-1, %eax
    jmp cleanup_syscall

__syscalls_jumptable:
.long 0, syscall_halt, syscall_execute, syscall_read, syscall_write, syscall_open, syscall_close, syscall_getargs, syscall_vidmap, syscall_set_handler, syscall_sigreturn, syscall_init_shell
    
# Copied from ece391support.S
# This sets up the syscall handler for each one (halt->sigreturn)
# 
# When the syscall is called from the user program, it will push the elements to the stack which are the arguments and the number of the system call
# and then call INT 0x80, which goes to the syscall vector.  INT 0x80 contains assembly linkage to move the four registers onto the stack and call the syscall handler
# located in syscall.c.  In syscall.c it will call the respective syscall according to the number.
# With the DO_CALL command it will create the labels for each syscall so they can be called.

#define DO_CALL(name,number)   \
.GLOBL name                   ;\
name: PUSHL   %EBX          ;\
    MOVL    $number,%EAX  ;\
    MOVL    8(%ESP),%EBX  ;\
    MOVL    12(%ESP),%ECX ;\
    MOVL    16(%ESP),%EDX ;\
    INT $0x80         ;\
    ADDL    $8, %ESP    ;\
    POPL    %EBX          ;\
    RET


#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8
#define SYS_SET_HANDLER  9
#define SYS_SIGRETURN  10
#define SYS_INIT_SHELL  11

/* the system call library wrappers */
DO_CALL(ece391_halt,SYS_HALT)
DO_CALL(ece391_execute,SYS_EXECUTE)
DO_CALL(ece391_init_shell, SYS_INIT_SHELL)
DO_CALL(ece391_read,SYS_READ)
DO_CALL(ece391_write,SYS_WRITE)
DO_CALL(ece391_open,SYS_OPEN)
DO_CALL(ece391_close,SYS_CLOSE)
DO_CALL(ece391_getargs,SYS_GETARGS)
DO_CALL(ece391_vidmap,SYS_VIDMAP)
DO_CALL(ece391_set_handler,SYS_SET_HANDLER)
DO_CALL(ece391_sigreturn,SYS_SIGRETURN)

