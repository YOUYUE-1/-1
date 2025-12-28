#include "rtc_app.h"

// 全局变量，保存当前时间
RTC_TimeTypeDef g_CurrentTime = {0};
RTC_DateTypeDef g_CurrentDate = {0};

/**
 * @brief RTC 任务函数
 * @note  定期读取RTC时间，更新全局变量
 *        不应在此函数中设置RTC时间，时间设置应通过串口命令完成
 */
void rtc_task(void)
{
    // 1. 读取 RTC 时间
    HAL_RTC_GetTime(&hrtc, &g_CurrentTime, RTC_FORMAT_BIN);

    // 2. 读取 RTC 日期 (必须紧跟在GetTime之后调用)
    HAL_RTC_GetDate(&hrtc, &g_CurrentDate, RTC_FORMAT_BIN);

    // 3. 在OLED上显示时间 (格式: HH:MM:SS)
    // 注意：根据赛题要求，只有在采样状态下才显示时间和电压
    // 非采样状态下应显示 "system idle"
    // 该显示逻辑应在OLED应用层控制，此处仅作为示例
    // Oled_printf(1,1,"Time:%02d:%02d:%02d",g_CurrentTime.Hours, g_CurrentTime.Minutes, g_CurrentTime.Seconds);
}
