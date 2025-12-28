#ifndef RTC_APP_H
#define RTC_APP_H
#include "mydefine.h"

/**
 * @brief  RTC任务函数，定期读取RTC时间
 * @note   该函数应在调度器中周期性调用
 *         RTC时间设置应通过串口命令 "RTC Config" 完成
 */
void rtc_task(void);

// 全局变量，供其他模块读取当前RTC时间和日期
extern RTC_TimeTypeDef g_CurrentTime;
extern RTC_DateTypeDef g_CurrentDate;

#endif
