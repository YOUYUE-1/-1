/**
  ******************************************************************************
  * @file           : system_test.c
  * @brief          : 系统自检模块实现
  * @author         : 哈雷酱 (CIMC 2025)
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "system_test.h"
#include "gd25qxx.h"
#include "fatfs.h"
#include "rtc.h"
#include "usart_app.h"
#include "config/config_manager.h"
#include "stdio.h"
#include "string.h"

/* External variables --------------------------------------------------------*/
extern FATFS SDFatFS;
extern char SDPath[4];
extern RTC_HandleTypeDef hrtc;
extern UART_HandleTypeDef huart1;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  执行系统自检（按照赛题格式）
 */
int system_selftest(void)
{
    uint8_t flash_ok = 0;
    uint8_t tf_ok = 0;
    uint32_t flash_id = 0;
    uint32_t tf_capacity_kb = 0;
    char buffer[128];

    // 打印开始标题（赛题格式）
    my_printf(&huart1, "======system selftest======\r\n");

    // 1. Flash检测（读取ID判断）
    flash_id = spi_flash_read_id();
    if (flash_id != 0x000000 && flash_id != 0xFFFFFF) {
        flash_ok = 1;
        my_printf(&huart1, "flash............ok\r\n");
    } else {
        my_printf(&huart1, "flash............error\r\n");
    }

    // 2. TF卡检测（是否存在判断）
    FRESULT res = f_mount(&SDFatFS, SDPath, 1);
    if (res == FR_OK) {
        tf_ok = 1;
        my_printf(&huart1, "TF card...........ok\r\n");
    } else {
        my_printf(&huart1, "TF card...........error\r\n");
    }

    // 3. 输出Flash ID（赛题格式：注意中文冒号）
    snprintf(buffer, sizeof(buffer), "flash ID\xA3\xBA 0x%06lX\r\n", flash_id);
    my_printf(&huart1, buffer);

    // 4. 输出TF卡容量或错误信息
    if (tf_ok) {
        FATFS *fs;
        DWORD fre_clust;
        if (f_getfree(SDPath, &fre_clust, &fs) == FR_OK) {
            tf_capacity_kb = (fs->n_fatent - 2) * fs->csize / 2;
            snprintf(buffer, sizeof(buffer), "TF card memory\xA3\xBA %lu KB\r\n", tf_capacity_kb);
            my_printf(&huart1, buffer);
        }
    } else {
        my_printf(&huart1, "can not find TF card\r\n");
    }

    // 5. 输出RTC时间（赛题格式）
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    if (HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) == HAL_OK) {
        HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);  // 必须调用GetDate同步
        snprintf(buffer, sizeof(buffer), "RTC\xA3\xBA 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
                 date.Year, date.Month, date.Date,
                 time.Hours, time.Minutes, time.Seconds);
        my_printf(&huart1, buffer);
    }

    // 打印结束标题（赛题格式）
    my_printf(&huart1, "======system selftest======\r\n");

    return (flash_ok && tf_ok) ? 0 : -1;
}
