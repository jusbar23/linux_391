#ifndef _PIT_H_
#define _PIT_H_

#include "../lib/lib.h"
#include "../lib/types.h"

#define PIT_CHANNEL_0_DATA 0x40   // R/W
#define PIT_CHANNEL_1_DATA 0x41   // R/W
#define PIT_CHANNEL_2_DATA 0x42   // R/W
#define PIT_COMMAND_REG 0x43 // Write only

#define PIT_INITIALIZE_CMD 0x34

#define RATE_55MS_LO 0
#define RATE_55MS_HI 0
#define RATE_10MS_LO 0x9C
#define RATE_10MS_HI 0x2E

#define PIT_IRQ_LINE 0 

//PIT initilization 
extern void pit_init(void);

//PIT handler, calls scheduling tick
extern void pit_handler(void);



#endif
