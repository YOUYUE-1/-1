#ifndef __OLED_APP_H__
#define __OLED_APP_H__

#include "mydefine.h"

void oled_task(void);
int Oled_printf(uint8_t x,uint8_t y, const char *format, ...);
#endif

