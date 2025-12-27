#include "rtc_app.h"
// 全局变量，存放当前时间
RTC_TimeTypeDef g_CurrentTime = {0};
RTC_DateTypeDef g_CurrentDate = {0};

/**
 * @brief RTC 任务函数
 * @note  放入调度器每 1000ms 调用一次
 */
void rtc_Init(void)
{
	static unsigned int start_flag=0;
	if(!start_flag++)
	{
		RTC_TimeTypeDef sTime = {0};
//  RTC_DateTypeDef sDate = {0};

  // 1. 填充你想要的时间 (比如 13:14:00)
  sTime.Hours   = 13;
  sTime.Minutes = 14;
  sTime.Seconds = 00;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;

  // 2. 关键动作：写入硬件！(拨动挂钟的指针)
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
      // 写入失败的处理，实际项目里可以加报错
      Error_Handler();
  }

//  // 3. 别忘了日期也要设置，不然年份可能是错的
//  sDate.Year    = 25; // 2025年
//  sDate.Month   = 12;
//  sDate.Date    = 25;
//  sDate.WeekDay = RTC_WEEKDAY_THURSDAY;
//  
//  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
//  {
//      Error_Handler();
//  }
	}
	
}
void rtc_task(void)
{
		rtc_Init();
    // 1. 必须先读 Time
    HAL_RTC_GetTime(&hrtc, &g_CurrentTime, RTC_FORMAT_BIN);
    
    // 2. 必须后读 Date (解锁影子寄存器)
    HAL_RTC_GetDate(&hrtc, &g_CurrentDate, RTC_FORMAT_BIN);

    // 这里可以加一行调试代码，看看串口有没有输出，证明任务在跑
	Oled_printf(1,1,"Time:%02d-%02d-%02d",g_CurrentTime.Hours, g_CurrentTime.Minutes, g_CurrentTime.Seconds);
}



