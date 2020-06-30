#include "termios.h"
#include "../kernel/pcb.h"
#include "../kernel/paging.h"

/* Struct to hold all terminal-related data */
typedef struct term_data
{
  //output and cursor fields
  uint16_t * screen_buf;    //backbuffer for screen data
  int cur_x;                //cursor column
  int cur_y;                //cursor row
  //input fields
  char in_buf[IN_BUF_SIZE]; //the input buffer for this term
  int in_dat_index;   //index of next available char in input buffer
  int in_dat_end;     //index of char following input data
  int in_dat_nr_ret;  //number of carriage returns currently in in_buf

  //control fields
  char is_active;   //set if this terminal is being drawn to display
  char usr_count;   //number of processes using this terminal
} term_data_t; 


//Text-mode screen buffers--one for each virtual terminal (Aligned to start at
//page boundaries
volatile uint16_t screen_buffers[NR_TERM][TERM_WIDTH * TERM_HEIGHT] __attribute__((aligned(NR_TERM * PAGE_SIZE * 4)));

//terminal data and associated pointer to the active terminal
volatile term_data_t term_data_array[NR_TERM]; //array of terminal data
volatile term_data_t *active_term;             //ptr to active terminal struct

//file-scope helper functions
int terminal_putc(term_data_t* term_data, char the_char, int x, int y); //putc to specific terminal
char terminal_getc(term_data_t* term_data, int x, int y); //getc to specific terminal
void terminal_clear(term_data_t * term_data); //fill terminal with ' ' (attribute filled too)
//keyboard callback for terminal display
int terminal_handle_key( term_data_t *term_data , char ascii, uint8_t scancode, uint8_t control, uint8_t alt);
void terminal_refresh(term_data_t * term_data); //refresh vga cursor
void terminal_scroll_up(term_data_t* term_data); //scroll a given framebuffer up by 1
void terminal_set_cursor(int row, int col); //set vga cursor to position

/* terminal_init -- initialize all virtual terminal data structures to known values
 * INPUT: none
 * OUTPUT: none
 * RETURN: none
 */ 
void terminal_init(void)
{
  int i; //iterator

  //fill all term_data_t structs with 0x00
  memset((void*)term_data_array, (int32_t)0, (uint32_t)NR_TERM*sizeof(term_data_array));
  active_term = &term_data_array[0]; //set active terminal to 0 (first one)

  //attach each virtual terminal with its text backbuffer
  for (i = 0; i < NR_TERM; i++)
  {
    term_data_array[i].screen_buf = (uint16_t *) screen_buffers[i];
  }

  return;
}   

/* terminal_data_ptr -- returns a pointer to the term_data struct of a terminal
 * INPUT: index -- the terminal number
 * OUTPUT: none
 * RETURN: pointer to terminal data struct of the terminal 
 */
uint32_t
term_data_ptr (uint8_t term_index)
{
  return (uint32_t) term_data_array[term_index].screen_buf;
}

/* is_active_term -- checks to see if a terminal is active 
 * INPUT: term_index -- the terminal index to check
 * OUTPUT: none
 * RETURN: 1 if it is the active terminal. 0 otherwise
 */
uint8_t
is_active_term(uint8_t term_index)
{
  return &term_data_array[term_index] == active_term;
}
 

/* terminal_write -- Print buffer of ascii chars to terminal
 * INPUT: buf -- char buffer of data to print
 *        len -- length of buffer to print
 *         fd -- file descriptor. not used, but needed to match jump table
 *             signature
 * OUTPUT: none
 * RETURN: number of bytes written
 */ 
int terminal_write(int32_t fd, const void* buf, int32_t len)
{
  pcb_t *context = pcb_process(); //running task's PCB
  int i; //index into string buf
    
  //print each char to the display
  for(i=0; i<len; i++)
  {
    //if putc returns an error, stop and
    //return number of chars writtern
    if((terminal_handle_key((term_data_t *)&term_data_array[context->term], ((char*)(buf))[i], (uint8_t)0xFF, 0, 0)))
      return i;
  }
    
  return i;
}   

/* terminal_read -- read from input buffer. Wait for a newline before returing.
 * INPUT:  len -- maximum size of buffer
 *         fd  -- file descriptor. not used, but needed to match file operation
 *                function pointer signature
 * OUTPUT: buf -- char buffer to store data in
 * RETURN: number of bytes read
 */ 
int terminal_read(int32_t fd, void* buf, int32_t len)
{
  int i, j; //iterator over input buffer. also count of chars copied
  char cr_read = 0; //number of cr read from in_buf
  term_data_t *context_term; //the terminal data for the running task
  pcb_t *context; //PCB of running process

  //set context_term
  context = pcb_process();
  context_term = (term_data_t *) &term_data_array[context->term]; 

  //spin until a cr has been loaded into input buffer
  // if (active_term->terminal_desc == 1) return -1;
  while(!(context_term->in_dat_nr_ret));

  //now in_buf has at least 1 cr. Copy chars from in_buf to buf until:
  //1. we fill buf
  //2. we reach end of in_buf
  //3. we hit a cr
  for(i=0; (i<context_term->in_dat_end) && (i<len); i++)
  {
    ((char*)buf)[i] = context_term->in_buf[i];
    if(((char*)buf)[i] == '\n') //if we just moved a newline..
    {
      i++;             //increment i to keep equal to nr moved
      cr_read++;       //maintain cr count
      break;           //stop coyping
    }
  }

  //now chars have been copied to buf. now update input buffer to reflect 
  //these changes
 
  for(j=0; i+j < context_term->in_dat_end; j++)
        active_term->in_buf[j] = context_term->in_buf[i+j];

  context_term->in_dat_end = j;  //update in_dat_end
  context_term->in_dat_index -= i; //
  context_term->in_dat_nr_ret -= cr_read; //update return count
    
  //return nr chars copied into buf
  return i;
}

/* input_process_key -- pass any keypresses into input buffer
 * INPUT: key -- ascii representation of key pressed 
 *               (NULL if not printable)
 *        scancode -- scancode of key pressed 
 *        control -- number of control keys pressed
 *        alt -- number of alt keys pressed
 * OUTPUT: none 
 * RETURN: 0 on success, -1 on error  
 */ 
int input_process_key(char key, uint8_t scancode, uint8_t control, uint8_t alt)   
{
  int i; //iterator for shifting buffer
  int is_modified = 0; //is the input buffer modified?
  
  //if terminal is not active, do nothing
  if(active_term->is_active == 0) return 0;

  switch(key)
  {
    case KEY_BACKSPACE:
      //if we're at the beginning of the input buffer
      if(active_term->in_dat_index == 0); //do nothing
      else //otherwise
      {
        active_term->in_dat_index--; //shift input index left
        
        //shift line left   
        for(i=active_term->in_dat_index; i<(active_term->in_dat_end-1); i++)
          active_term->in_buf[i] = active_term->in_buf[i+1];
          active_term->in_dat_end--; //decrement end
          is_modified = 1;   //buffer was modified
      }
      break;
    case KEY_DELETE:
      //if we're not at the end of the buffer
      if(active_term->in_dat_index < active_term->in_dat_end) 
      {
        //shift line left   
        for(i=active_term->in_dat_index-1; i<active_term->in_dat_end; i++) 
          active_term->in_buf[i] = active_term->in_buf[i+1];

        //move in_buffer end
        active_term->in_dat_end--;
        is_modified = 1;
      }
      break;
    default:
      //check to see if key is valid and space in buffer
      if(key != 0 && active_term->in_dat_end < IN_BUF_SIZE)
      {
        //increase buffer end to make space for new char
        active_term->in_dat_end++;
            
        //move index to next position
        active_term->in_dat_index++;

        //shift chars right of index on space to the right
        for(i=active_term->in_dat_index; i<active_term->in_dat_end; i++ )
          active_term->in_buf[i] = active_term->in_buf[i-1];
    
        //insert `key` at index
        active_term->in_buf[active_term->in_dat_index-1] = key;
          is_modified = 1; //buffer was modified

        //if `key` was a newline, increment newline count
        if(key == KEY_ENTER) active_term->in_dat_nr_ret++;
        }   
        break;
    }

  if(is_modified) //if buffer was modified, print the char to the terminal
  {
    terminal_handle_key((term_data_t *)active_term, key, scancode,  control,  alt);
  }
  else //buffer was not modified
  {
    //pass the scancode, but dont print a character
    terminal_handle_key((term_data_t *)active_term, NULL, scancode, control, alt); 
  }

  return 0;
}

/* terminal_open -- reset term_data struct for the given terminal. 
 *                  set terminal as open (process keypresses)
 * INPUT: number -- number of virtual terminal to open 
 * OUTPUT: none 
 * RETURN: 0 on success, -1 on failure 
 */ 
int terminal_open(uint8_t *number)
{
  int i; //iterator
  int vtnum = (int) number; //terminal number
  term_data_t *term_data; //terminal data struct that is being opened

  //set term_dat for this vtnum
  term_data = (term_data_t *) &term_data_array[vtnum];

  //if this terminal is already open, nothing to do
  if(term_data->usr_count != 0) return 0;

  //Clear screen and reset cursor
  terminal_clear(term_data);

  //Flush input buffer
  for(i=0; i<IN_BUF_SIZE; i++)
    term_data->in_buf[i] = 0x0;
  //
  //Reset in_buf data
  term_data->in_dat_index = 0; //index of last char stored in input buffer
  term_data->in_dat_end = 0;   //number of chars in input buffer
  term_data->in_dat_nr_ret = 0; //number of carriage returns currently in in_buf
  
  //not really sure where to do this?? 
  term_data-> is_active = 1;   //set if this terminal is being drawn to display

  return 0;

}

//close the terminal
/* terminal_close -- set terminal as closed (don't process any input)
 * INPUT: number of terminal to close 
 * OUTPUT: 
 * RETURN: 
 */ 
int terminal_close(int32_t number)
{
  term_data_t *term_data; //terminal data struct that is being opened

  //decrement usr_count for this terminal. Close if we're the last ones out
  if(--(term_data->usr_count) == 0)
  {
      term_data->is_active = 0;
  }

  return 0;
}

/* terminal_handle_key -- pass any chars to be written to the terminal
 *                        also pass keypresses for control sequences 
 * INPUT: key -- ascii representation of char or key pressed 
 *               NULL if non/printable keypress
 *        scancode -- scancode of key pressed. NULL if not a keypress.
 *        control -- number of control keys pressed
 *        alt -- number of alt keys pressed
 * OUTPUT: none 
 * RETURN: 0 on success, -1 on failure. 
 */ 
int terminal_handle_key(term_data_t *term_data, char key, uint8_t scancode, uint8_t control, uint8_t alt)
{
  int i; //iterator
  
    switch(key)
    {
      /*case KEY_LEFT:
        //move cursor left, and up if necessary.
        if(term_data->cur_x == 0)
        {
          term_data->cur_x = TERM_WIDTH; //move to edge
          //move up a line, but don't let it go off the screen
          term_data->cur_y -= (term_data->cur_y == 0)? 0: 1;  
        }
        else term_data->cur_x -= 1; 
        break;
      case KEY_RIGHT:
        //move cursor right, and down if necessary.
        if(term_data->cur_x == TERM_WIDTH)
        {
          term_data->cur_x = 0; //move to left edge
          //move down a line, but not off screen
          term_data->cur_y +=(term_data->cur_y == TERM_HEIGHT)? 0:1;
        }
        else term_data->cur_x += 1;
        break;
        */
      case KEY_BACKSPACE:
        //if we're at the beginning of the line
        if (term_data->cur_x == 0 && term_data->cur_y != 0 )
        {
          term_data->cur_y -= 1;
          term_data->cur_x = TERM_WIDTH - 1;
          terminal_putc(term_data, ' ', term_data->cur_x, term_data->cur_y);
        }
        else //otherwise
        {
          //shift cursor left
          term_data->cur_x -= 1;
          
          //shift line left   
          for(i=term_data->cur_x; i<TERM_WIDTH-1; i++)
            terminal_putc(term_data,  terminal_getc(term_data, (i+1), term_data->cur_y), i, term_data->cur_y);
          //insert space at end of line
          terminal_putc(term_data,  ' ', i, term_data->cur_y);
        }
        break;
      case KEY_DELETE:
        //shift line left   
        for(i=term_data->cur_x; i<TERM_HEIGHT-1; i++)
          terminal_putc(term_data, terminal_getc(term_data, i+1, term_data->cur_y), i , term_data->cur_y);
        //insert space at end of line
        terminal_putc(term_data, ' ', i+1, term_data->cur_y);
        break;
      case KEY_ENTER:
        //if we're at the bottom of the screen, scroll up
        if(term_data->cur_y == TERM_HEIGHT-1) terminal_scroll_up(term_data);
        //move cursor to next line at the beginning
        term_data->cur_y++;
        term_data->cur_x = 0;
        break;
      case KEY_TAB:
        //advance cursor to the next multiple of TERM_TAB_WIDTH
        term_data->cur_x = ((term_data->cur_x / TERM_TAB_WIDTH)+1)*TERM_TAB_WIDTH;

        //if we've run off the line, go to the next one
        if(term_data->cur_x >= TERM_WIDTH-1)
        {
          //if we're at the bottom of the screen, scroll up
          if(term_data->cur_y == TERM_HEIGHT-1)terminal_scroll_up(term_data);
          term_data->cur_y++;
          term_data->cur_x = 0;
        }

        break;
      default:
        if(control)
        {
          //control+L should clear the screen
          if(key == 'l') terminal_clear(term_data);
          // else if(key == 'c') 
          // {
          //  send_eoi(KEYBOARD_IRQ_NUM);
          //  memset(term_data->in_buf, 0, IN_BUF_SIZE);
          //  syscall_halt(-1);
          // }
        } else if (alt)
        {

        }
        else if(key != 0) //control is not pressed
        {   
          //put char at cursor
          terminal_putc(term_data, key, term_data->cur_x, term_data->cur_y);
          //if we're at the end of the line, go to the next one
          if(term_data->cur_x == TERM_WIDTH-1)
          {
            //if we're at the bottom of the screen, scroll up
            if(term_data->cur_y == TERM_HEIGHT-1)terminal_scroll_up(term_data);
            term_data->cur_y++;
            term_data->cur_x = 0;
          }
          else term_data->cur_x++;
        }
        else if(key == NULL && scancode == 0xFF )
        {
          terminal_putc(term_data, ' ', term_data->cur_x, term_data->cur_y);
          //if we're at the end of the line, go to the next one
          if(term_data->cur_x == TERM_WIDTH-1)
          {
            //if we're at the bottom of the screen, scroll up
            if(term_data->cur_y == TERM_HEIGHT-1)terminal_scroll_up(term_data);
            term_data->cur_y++;
            term_data->cur_x = 0;
          }
          else term_data->cur_x++;
        }
        break;  
    }
  
  //redraw screen
  terminal_refresh(term_data);    
  return 0;
}

/* terminal_clear -- clear the display & display buffer
 * INPUT: term_data -- pointer to term_data struct of the terminal to be cleared 
 * OUTPUT: none 
 * RETURN: none 
 */ 
void terminal_clear(term_data_t * term_data)
{
  int i;
  for(i=0; i<TERM_HEIGHT*TERM_WIDTH; i++)
  {
    //fill screen buffer with spaces and default attribute byte
    terminal_putc(term_data, ' ', i%TERM_WIDTH, i/TERM_WIDTH);
  }

  for(i=0; i<IN_BUF_SIZE; i++)
    term_data->in_buf[i] = 0x0;

  //Reset in_buf data
  term_data->in_dat_index = 0; //index of last char stored in input buffer
  term_data->in_dat_end = 0;   //number of chars in input buffer
  term_data->in_dat_nr_ret = 0; //number of carriage returns currently in in_buf
  

  //set cursor to upper left position
  term_data->cur_x = 0; 
  term_data->cur_y = 0;

  //redraw the screen to video memory
  //eventually, we will only do this if the terminal is active...
  terminal_refresh(term_data);
}   

/* terminal_refresh -- refresh the vga cursor position
 * INPUT: term_data -- the termianl data whose cur_x and cur_y members we will
 *                     used to get new cursor coordinates 
 * OUTPUT: none 
 * RETURN: none 
 */ 
void terminal_refresh(term_data_t * term_data)
{
  //if the terminal we're refreshing is the active one...
  if(term_data == active_term)
  {
    //update vga cursor position, since we are the writing to the active term
    terminal_set_cursor(term_data->cur_y, term_data->cur_x);
  }
}

/* terminal_putc -- print a character to a given terminal at at given position
 * INPUT: term_data -- pointer to term_data struct
 *        the_char -- the character to be printed
 *        x -- the column
 *        y -- the row
 * OUTPUT: none
 * RETURN: 0 on success, -1 on error 
 */
int terminal_putc(term_data_t *term_data, char the_char, int x, int y)
{
  //if the terminal of the active task is the active terminal
  if(term_data == active_term)
  {
    //write directly into video memory
    *((uint16_t*)VIDEO_MEM + y*TERM_WIDTH + x) = (ATTRIB<<8 | the_char);
  }
  else
  {
    term_data->screen_buf[y*TERM_WIDTH + x] = (ATTRIB<<8 | the_char);
  }
  return 0;
}

/* terminal_getc -- gets a character from a virtual terminal at a given location
 * INPUT: term_data -- pointer to term_data struct
 *        x -- column
 *        y -- row
 * OUTPUT: none
 * RETURN: the character at the location
 */
char terminal_getc(term_data_t *term_data, int x, int y)
{
  //if the terminal of the active task is the active terminal
  if(term_data == active_term)
  {
    //write directly into video memory
    
    return *((uint16_t*)VIDEO_MEM + y*TERM_WIDTH + x); 
  }
  else
  {
    return term_data->screen_buf[y*TERM_WIDTH + x]; 
                      
  }
}


/* terminal_switch -- switch the active terminal 
 * INPUT: term_num -- the number of the new active terminal
 * OUTPUT: none
 * RETURN: none
 */
void terminal_switch(uint8_t term_num)
{

  uint32_t flags; //flags for locking
  //beign critical section
  cli_and_save(flags);

  term_data_t * new_term = (term_data_t *) &term_data_array[term_num];
  //if we try to switch to the active terminal, nothing to do
  if(new_term == active_term) 
	{	
		//release lock
		restore_flags(flags);
		return;
	}
  
  //copy video memory into active_term's screen buffer
  memcpy((void*)active_term->screen_buf, (void*)VIDEO_MEM, 
    TERM_HEIGHT*TERM_WIDTH*sizeof(uint16_t));
    
  //copy new_term's buffer to video memory    
  memcpy((void*)VIDEO_MEM, (void*)term_data_array[term_num].screen_buf, 
    TERM_HEIGHT*TERM_WIDTH*sizeof(uint16_t));

  //mark active_term as inactive and new_term as active
  active_term->is_active = 0;
  new_term->is_active = 1;

  //switch active_term to new_term
  active_term = new_term;

  //refresh the active term to update cursor position.
  terminal_refresh((term_data_t *) active_term);
  //syscall_no_execute("shell");
  
  //end critical section
  restore_flags(flags);

}

/* terminal_scroll_up -- scroll display buffer 1 line up
 *                       and refresh to video mem
 * INPUT: term_data -- pointer to term_data struct 
 * OUTPUT: none
 * RETURN: none
 */ 
void terminal_scroll_up(term_data_t* term_data)
{
  int x, y; //coordinate interators

  for(y=0; y<(TERM_HEIGHT-1); y++) //all but the last row
    for(x=0; x<TERM_WIDTH; x++)
      terminal_putc( term_data, terminal_getc(term_data, x, y+1), x, y);
  for(x=0; x<TERM_WIDTH; x++)
    terminal_putc(term_data, ' ', x, TERM_HEIGHT-1);
    
  //move cursor
  term_data->cur_y--;
}

//NOTE* This code was modified from source at 
// http://wiki.osdev.org/Text_Mode_Cursor
/* terminal_set_cursor -- put the VGA cursor at a give position
 * INPUT: row -- row index of the cursor
 *        col -- column index of the cursor
 * OUTPUT: none 
 * RETURN: none 
 */ 
void terminal_set_cursor(int row, int col)
{
  uint16_t position = (row*TERM_WIDTH + col);
    
  // cursor LOW port to vga INDEX register
  outb(CRTC_CURSOR_LOW, CRTC_ADDR);
  outb((unsigned char)(position&0xFF), CRTC_DATA);

  //cursor HIGH port to vga INDEX register
  outb(CRTC_CURSOR_HIGH, CRTC_ADDR);
  outb((unsigned char)((position>>8)&0xFF), CRTC_DATA);
}
