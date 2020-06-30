#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include "irq.h"
#include "../lib/lib.h"
#define MAX_STR_LENGTH 48
#define MAX_STR_LENGTH_BOOT 78

//Installs the exception handler 
extern void install_exceptions(void);

//Print the universal error screen to the screen 
extern void error_screan(char *message);

#endif

