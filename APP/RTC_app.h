#ifndef RTC_APP_H
#define RTC_APP_H
#include "mydefine.h"

// 声明任务函数
void rtc_task(void);

// 声明全局变量，方便 OLED 等其他模块读取
extern RTC_TimeTypeDef g_CurrentTime;
extern RTC_DateTypeDef g_CurrentDate;

#endif

