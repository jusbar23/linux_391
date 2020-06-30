#ifndef _RTC_H_
#define _RTC_H_

#include "../lib/lib.h"
#include "i8259.h"

#define TRUE 1
#define FALSE 0

#define RTC_PORT 0x70
#define RTC_CMOS 0x71

#define RTC_IRQ_NUM 8
#define RTC_REG_A 0x0A
#define RTC_REG_A_M 0x8A
#define RTC_REG_B 0x0B
#define RTC_REG_C 0x0C
#define DISABLE_NMI 0x80
#define ENABLE_NMI 0x7F
#define BIT_SIX 0x40
#define RATE_MASK 0xF0
#define RTC_MAX_RATE 1024
#define RTC_MIN_RATE 2
#define DEFAULT_RATE 2
#define MASK_DEFAULT 0x01
#define DIV_MAX 16
#define RATE_IN_MASK 0xFFF

void rtc_init();
void rtc_irq_handler();


int rtc_open();
int rtc_read(int32_t fd, void *buf, int32_t nbytes);
int rtc_write(int32_t fd, void *buf, int32_t nbytes);
int rtc_close(int32_t fd, void *buf, uint32_t bytes);

#endif
