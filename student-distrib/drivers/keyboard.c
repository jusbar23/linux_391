#include "keyboard.h"
#include "termios.h"
#include "../kernel/syscall.h"
#include "../kernel/tasks.h"

//scancodes shift/capslock keys
#define KEY_CAPS_LOCK 0x3A
#define KEY_LSHIFT_D 0x2A
#define KEY_RSHIFT_D 0x36
#define KEY_LSHIFT_U 0xAA
#define KEY_RSHIFT_U 0xB6

/*This snippet is borrowed from http://www.osdever.net/bkerndev/Docs/keyboard.html
*This array contains the translation from the code sent by the keyboard to char
*first map is for no shift, second is with shift, third is with caps, fourth is
*caps with shift.
 */
static unsigned char font_data[4][128] =
{{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
    '9', '0', '-', '=', '\b',	/* Backspace */
    '\t',			/* Tab */
    'q', 'w', 'e', 'r',	/* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
      0,			/* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
   '\'', '`',   0,		/* Left shift */
   '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
    'm', ',', '.', '/',   0,				/* Right shift */
    '*',
      0,	/* Alt */
    ' ',	/* Space bar */
      0,	/* Caps lock */
      0,	/* 59 - F1 key ... > */
      0,   0,   0,   0,   0,   0,   0,   0,
      0,	/* < ... F10 */
      0,	/* 69 - Num lock*/
      0,	/* Scroll Lock */
      0,	/* Home key */
      0,	/* Up Arrow */
      0,	/* Page Up */
    '-',
      0,	/* Left Arrow */
      0,
      0,	/* Right Arrow */
    '+',
      '\n',	/* 79 - End key*/
      0,	/* Down Arrow */
      0,	/* Page Down */
      0,	/* Insert Key */
      127,	/* Delete Key */
      0,   0,   0,
      0,	/* F11 Key */
      0,	/* F12 Key */
      0,	/* All other keys are undefined */
},		
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
    '(', ')', '_', '+', '\b',	/* Backspace */
    '\t',			/* Tab */
    'Q', 'W', 'E', 'R',	/* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
      0,			/* 29   - Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
   '"', '~',   0,		/* Left shift */
   '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
    'M', '<', '>', '?',   0,				/* Right shift */
    '*',
      0,	/* Alt */
    ' ',	/* Space bar */
      0,	/* Caps lock */
      0,	/* 59 - F1 key ... > */
      0,   0,   0,   0,   0,   0,   0,   0,
      0,	/* < ... F10 */
      0,	/* 69 - Num lock*/
      0,	/* Scroll Lock */
      0,	/* Home key */
      0,	/* Up Arrow */
      0,	/* Page Up */
    '-',
      0,	/* Left Arrow */
      0,
      0,	/* Right Arrow */
    '+',
      0,	/* 79 - End key*/
      0,	/* Down Arrow */
      0,	/* Page Down */
      0,	/* Insert Key */
      127,	/* Delete Key */
      0,   0,   0,
      0,	/* F11 Key */
      0,	/* F12 Key */
      0,	/* All other keys are undefined */
},		
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
    '9', '0', '-', '=', '\b',	/* Backspace */
    '\t',			/* Tab */
    'Q', 'W', 'E', 'R',	/* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',	/* Enter key */
      0,			/* 29   - Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',	/* 39 */
   '\'', '`',   0,		/* Left shift */
   '\\', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
    'M', ',', '.', '/',   0,				/* Right shift */
    '*',
      0,	/* Alt */
    ' ',	/* Space bar */
      0,	/* Caps lock */
      0,	/* 59 - F1 key ... > */
      0,   0,   0,   0,   0,   0,   0,   0,
      0,	/* < ... F10 */
      0,	/* 69 - Num lock*/
      0,	/* Scroll Lock */
      0,	/* Home key */
      0,	/* Up Arrow */
      0,	/* Page Up */
    '-',
      0,	/* Left Arrow */
      0,
      0,	/* Right Arrow */
    '+',
      0,	/* 79 - End key*/
      0,	/* Down Arrow */
      0,	/* Page Down */
      0,	/* Insert Key */
      127,	/* Delete Key */
      0,   0,   0,
      0,	/* F11 Key */
      0,	/* F12 Key */
      0,	/* All other keys are undefined */
},		
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
    '(', ')', '_', '+', '\b',	/* Backspace */
    '\t',			/* Tab */
    'q', 'w', 'e', 'r',	/* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',	/* Enter key */
      0,			/* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',	/* 39 */
   '\'', '~',   0,		/* Left shift */
   '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
    'm', '<', '>', '?',   0,				/* Right shift */
    '*',
      0,	/* Alt */
    ' ',	/* Space bar */
      0,	/* Caps lock */
      0,	/* 59 - F1 key ... > */
      0,   0,   0,   0,   0,   0,   0,   0,
      0,	/* < ... F10 */
      0,	/* 69 - Num lock*/
      0,	/* Scroll Lock */
      0,	/* Home key */
      0,	/* Up Arrow */
      0,	/* Page Up */
    '-',
      0,	/* Left Arrow */
      0,
      0,	/* Right Arrow */
    '+',
      0,	/* 79 - End key*/
      0,	/* Down Arrow */
      0,	/* Page Down */
      0,	/* Insert Key */
      127,	/* Delete Key */
      0,   0,   0,
      0,	/* F11 Key */
      0,	/* F12 Key */
      0,	/* All other keys are undefined */
}};		

static int caps_lock = 0; //1 if caps lock is on
static int shift = 0;     //1 if a shift is pressed
static uint8_t control; //number of control keys pressed
static uint8_t alt;     //number of alt keys pressed

//if !=0, signifies a shell has been started on that terminal
static uint8_t shell_active[NR_TERM]; 
		
/*
 * keyboard_handler
 *   DESCRIPTION: Reads from the the keyboard port and call putc function to show character to the
 *				  screen. Currently, neglect all the the 'up' code that keyboard is released.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints the character pressed on the keyboard to the screen and send eoi to the PIC
 */
void keyboard_handler(void)
{


    // Old working code that puts it directly on the screen
    unsigned char key_pressed = inb(KEYBOARD_PORT);
    unsigned char font;

		//check for caps lock
		if(key_pressed == KEY_CAPS_LOCK) caps_lock ^= 0x01; //toggle caps_lock
		else if ( (key_pressed==KEY_LSHIFT_D) || (key_pressed==KEY_RSHIFT_D))
			shift += 1; //increment shift count;
	 	else if ( (key_pressed==KEY_LSHIFT_U) || (key_pressed==KEY_RSHIFT_U))
			shift -= 1; //decrement shift count;
		else 
    {
			//begin critical section
			cli(); //Redundant.. but still a good reminder. Interrupts should be cleared anyway since this is called from an int gate
			
			//font table is only applicable for codes < 0x81. Otherwise we set to NULL
			//to signal that it is non-printable.
      if(key_pressed<0x81 ) font=font_data[(caps_lock<<1)|(shift>0)][key_pressed];
			else font = NULL;

			//Handle keys alt+Fn and arrows
      switch(key_pressed)
      {
        case KEY_CTRL_D:
          control++;               //increment ctrl key count
          break;
        case KEY_CTRL_U:
          control--;               //decrement ctrl key count
          break;
        case KEY_ALT_D:
          alt++;                   //increment alt key count
          break;
        case KEY_ALT_U:
          alt--;                   //decrement alt key count..
          break;
        case KEY_F1:
          if (alt) terminal_switch(0); //switch to the first terminal
          break;
        case KEY_F2:
          if (alt)
          {
            terminal_switch(1);       //switch to the second terminal
            if(shell_active[1] != 1)  //start the shell if we haven't already
            {
              shell_active[1] = 1;
							//must now send the EOI and reset IF since we will be doing a
							//context switch before we reach the end of this handler
              send_eoi(KEYBOARD_IRQ_NUM);
							sti();                 
							//start up the new shell and attach it to the second terminal
              syscall_init_shell (1);
            }
          }
          break;
        case KEY_F3:
          if (alt) //alt^f3
          {
            terminal_switch(2);    //switch to the third terminal
            if(shell_active[2] != 1) //start the shell if we haven't already 
            {
              shell_active[2] = 1;
							//there will be a context switch before we reach the end of this
							//handler, so we must send the eoi and set IF here
              send_eoi(KEYBOARD_IRQ_NUM);
							sti(); 
							//start up the new shell and attach it to the third terminal
              syscall_init_shell (2);
            }
          }
		  break;
        case KEY_F4:
          if (alt)
          {
            terminal_switch(3);
            if(shell_active[3] != 1)
            {
              shell_active[3] = 1;
              send_eoi(KEYBOARD_IRQ_NUM);
							sti(); //re-eable interrupts 
              syscall_init_shell (3);
            }
          }
		  break;
      } 

			input_process_key(font, key_pressed, control, alt);
      //putc(font);

			//end critical section
			sti();
    }

    send_eoi(KEYBOARD_IRQ_NUM);
    return;
}

/*
 * keyboard_init
 *   DESCRIPTION: this function enables the keyboard irq 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: keyboard IRQ gets enabled
 */
void keyboard_init(void)
{
  int i;
  enable_irq(KEYBOARD_IRQ_NUM);     //enabling the irq
  shell_active[0] = 1;
  for(i = 1; i < NR_TERM; i++)
  {
    shell_active[i] = 0;
  }
}
