#ifndef _TERMIOS_H_
#define _TERMIOS_H_

#define VIDEO_MEM 0xB8000 //physical address of video memory
#define NR_TERM 4         //number of terminals
#define IN_BUF_SIZE 128   //size of input buffer
#define TERM_WIDTH 80     //nr chars per line
#define TERM_HEIGHT 25    //nr lines per screen
#define ATTRIB 0x7 //default attribute byte (white on black)
#define TERM_TAB_WIDTH 8   //nr chars per tab
#define SCREEN_MEM TERM_WIDTH * TERM_HEIGHT * 2 //size in bytes of video memory


#define CRTC_ADDR 0x3D4     //io address of VGA CRTC address register
#define CRTC_DATA 0x3D5     //io address of VGA CRTC data register
#define CRTC_CURSOR_HIGH 0x0E //VGA register index of cursor high bits
#define CRTC_CURSOR_LOW  0x0F //VGA register index of cursor low bits



#include "../lib/types.h"
#include "keyboard.h"
#include "../lib/lib.h"

//Initialize terminal data structures
void terminal_init(void);

//Print buffer to terminal
int terminal_write(int32_t fd, const void* buf, int32_t len); 

//Read input buffer
int terminal_read(int32_t fd, void* buf, int32_t len); 

//Open and clear the screen
int terminal_open(uint8_t* number);

//close the terminal
int terminal_close(int32_t fd);

//handle keypress
int input_process_key(char key, uint8_t scancode, uint8_t control, uint8_t alt);

//switch terminal to term_num
void terminal_switch(uint8_t term_num);

//checks if terminal is the active terminal
uint8_t is_active_term(uint8_t term_index);

//gets pointer to terminal backbuffer
uint32_t term_data_ptr(uint8_t term_index);
#endif
