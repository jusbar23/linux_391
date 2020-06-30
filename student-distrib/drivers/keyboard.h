#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "i8259.h"
#include "../lib/lib.h"
#include "termios.h"

#define KEYBOARD_IRQ_NUM 1
#define KEYBOARD_PORT 0x60


#define KEY_BACKSPACE '\b'
#define KEY_DELETE 127
#define KEY_ENTER '\n'
#define KEY_TAB '\t'

//Non-ascii scancodes
#define KEY_CAPS_LOCK 0x3A
#define KEY_LSHIFT_D 0x2A
#define KEY_RSHIFT_D 0x36
#define KEY_LSHIFT_U 0xAA
#define KEY_RSHIFT_U 0xB6
#define KEY_CTRL_D 0x1D
#define KEY_CTRL_U 0x9D 
#define KEY_ALT_D 0x38
#define KEY_ALT_U 0xB8
#define KEY_F1 0x3B
#define KEY_F2 0x3C
#define KEY_F3 0x3D
#define KEY_F4 0x3E

//multibyte keycodes begin with this
#define KEY_MULTI_BYTE 0xE0

//the keyboard handler function
extern void keyboard_handler(void);
//keyboard initialization function
extern void keyboard_init(void);
#endif
