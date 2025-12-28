/**
  ******************************************************************************
  * @file           : shell_cmds.c
  * @brief          : Shell命令扩展模块实现
  * @author         : 哈雷酱 (CIMC 2025)
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "shell_cmds.h"
#include "system/system_test.h"
#include "usart_app.h"
#include "rtc.h"
#include "config/config_manager.h"
#include "sample_app.h"
#include "fatfs.h"
#include "ff.h"
#include "string.h"
#include "stdio.h"
#include "ctype.h"
#include "stdlib.h"

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern RTC_HandleTypeDef hrtc;
extern FIL SDFile;      // FATFS文件对象
extern FATFS SDFatFS;   // FATFS文件系统对象
extern char SDPath[4];  // SD卡路径
extern flash_param_t g_system_params;  // 全局系统参数

/* Private variables ---------------------------------------------------------*/
static uint8_t rtc_waiting_input = 0;    // 标记是否正在等待RTC时间输入
static uint8_t ratio_waiting_input = 0;  // 标记是否正在等待变比输入
static uint8_t limit_waiting_input = 0;  // 标记是否正在等待阈值输入

/* Private functions prototypes ----------------------------------------------*/
static int cmd_test(char *args);
static int cmd_rtc_config(char *args);
static int cmd_rtc_now(char *args);
static int cmd_conf(char *args);
static int cmd_ratio(char *args);
static int cmd_limit(char *args);
static int cmd_config(char *args);
static int cmd_start(char *args);
static int cmd_stop(char *args);
static int parse_standard_datetime(const char *str, int *year, int *month, int *day,
                                    int *hour, int *minute, int *second);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  处理串口接收到的命令
 */
int shell_process_command(char *cmd_line)
{
    // 去除首尾空格和换行符
    char *p = cmd_line;
    while (*p == ' ' || *p == '\t') p++;

    char *end = p + strlen(p) - 1;
    while (end > p && (*end == ' ' || *end == '\r' || *end == '\n' || *end == '\t')) {
        *end = '\0';
        end--;
    }

    if (*p == '\0') return 0;  // 空命令

    // 检查是否在等待RTC时间输入
    if (rtc_waiting_input) {
        rtc_waiting_input = 0;  // 清除等待标志

        // 直接将输入作为时间字符串处理
        int year, month, day, hour, minute, second;
        if (parse_standard_datetime(p, &year, &month, &day, &hour, &minute, &second) != 0) {
            my_printf(&huart1, "Error: Invalid datetime format\r\n");
            return -1;
        }

        // 配置RTC
        RTC_TimeTypeDef sTime = {0};
        RTC_DateTypeDef sDate = {0};

        sTime.Hours = hour;
        sTime.Minutes = minute;
        sTime.Seconds = second;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;

        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
            my_printf(&huart1, "Error: Failed to set RTC\r\n");
            return -1;
        }

        sDate.Year = year - 2000;
        sDate.Month = month;
        sDate.Date = day;
        sDate.WeekDay = RTC_WEEKDAY_MONDAY;

        if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
            my_printf(&huart1, "Error: Failed to set RTC\r\n");
            return -1;
        }

        // 输出成功信息
        my_printf(&huart1, "RTC Config success\r\n");
        my_printf(&huart1, "Time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
                  year, month, day, hour, minute, second);
        return 0;
    }

    // 检查是否在等待变比输入
    if (ratio_waiting_input) {
        ratio_waiting_input = 0;  // 清除等待标志

        // 解析浮点数
        float new_ratio = atof(p);

        // 验证范围 (0-100)
        if (new_ratio < 0.0f || new_ratio > 100.0f) {
            my_printf(&huart1, "ratio invalid\r\n");
            my_printf(&huart1, "Ratio = %.1f\r\n", g_system_params.ratio);
            return -1;
        }

        // 更新到内存（不保存到Flash）
        g_system_params.ratio = new_ratio;

        // 输出成功信息
        my_printf(&huart1, "ratio modified success\r\n");
        my_printf(&huart1, "Ratio = %.1f\r\n", g_system_params.ratio);
        return 0;
    }

    // 检查是否在等待阈值输入
    if (limit_waiting_input) {
        limit_waiting_input = 0;  // 清除等待标志

        // 解析浮点数
        float new_limit = atof(p);

        // 验证范围 (0-500)
        if (new_limit < 0.0f || new_limit > 500.0f) {
            my_printf(&huart1, "limit invalid\r\n");
            my_printf(&huart1, "limit = %.2f\r\n", g_system_params.limit);
            return -1;
        }

        // 更新到内存（不保存到Flash）
        g_system_params.limit = new_limit;

        // 输出成功信息
        my_printf(&huart1, "limit modified success\r\n");
        my_printf(&huart1, "limit = %.2f\r\n", g_system_params.limit);
        return 0;
    }

    // 分离命令和参数
    char *args = strchr(p, ' ');
    if (args) {
        *args = '\0';
        args++;
        while (*args == ' ') args++;  // 跳过空格
    }

    // 命令匹配
    if (strcmp(p, "test") == 0) {
        return cmd_test(args);
    }
    else if (strcmp(p, "conf") == 0) {
        return cmd_conf(args);
    }
    else if (strcmp(p, "ratio") == 0) {
        return cmd_ratio(args);
    }
    else if (strcmp(p, "limit") == 0) {
        return cmd_limit(args);
    }
    else if (strcmp(p, "config") == 0) {
        return cmd_config(args);
    }
    else if (strcmp(p, "start") == 0) {
        return cmd_start(args);
    }
    else if (strcmp(p, "stop") == 0) {
        return cmd_stop(args);
    }
    else if (strcmp(p, "RTC") == 0) {
        if (args == NULL) {
            my_printf(&huart1, "Usage: RTC <Config|now>\r\n");
            return -1;
        }

        // 匹配RTC子命令
        if (strcmp(args, "Config") == 0) {
            return cmd_rtc_config(NULL);
        } else if (strcmp(args, "now") == 0) {
            return cmd_rtc_now(NULL);
        } else {
            my_printf(&huart1, "Unknown RTC command\r\n");
            return -1;
        }
    }

    // 未知命令
    my_printf(&huart1, "Unknown command: %s\r\n", p);
    return -1;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  test命令：系统自检
 */
static int cmd_test(char *args)
{
    return system_selftest();
}

/**
 * @brief  conf命令：读取TF卡根目录下的config.ini文件
 * @note   文件格式：
 *         [Ratio]
 *         Ch0 = 10.5
 *         [Limit]
 *         Ch0 = 100
 */
static int cmd_conf(char *args)
{
    FRESULT res;
    char line[64];
    float ratio_value = -1.0f;
    float limit_value = -1.0f;
    int in_ratio_section = 0;
    int in_limit_section = 0;

    // 打开config.ini文件
    res = f_open(&SDFile, "config.ini", FA_READ);
    if (res != FR_OK) {
        my_printf(&huart1, "config.ini file not found.\r\n");
        return -1;
    }

    // 逐行读取并解析
    while (f_gets(line, sizeof(line), &SDFile) != NULL) {
        // 去除行尾的换行符和回车符
        char *p = line;
        while (*p && *p != '\r' && *p != '\n') p++;
        *p = '\0';

        // 跳过空行和纯空格行
        p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') continue;

        // 检查是否是section标记
        if (p[0] == '[') {
            in_ratio_section = 0;
            in_limit_section = 0;

            if (strstr(p, "[Ratio]") != NULL) {
                in_ratio_section = 1;
            } else if (strstr(p, "[Limit]") != NULL) {
                in_limit_section = 1;
            }
            continue;
        }

        // 解析 "Ch0 = value" 格式
        if (strstr(p, "Ch0") != NULL) {
            char *equal_sign = strchr(p, '=');
            if (equal_sign != NULL) {
                // 跳过等号后的空格
                equal_sign++;
                while (*equal_sign == ' ' || *equal_sign == '\t') equal_sign++;

                // 转换为浮点数
                float value = atof(equal_sign);

                if (in_ratio_section) {
                    ratio_value = value;
                } else if (in_limit_section) {
                    limit_value = value;
                }
            }
        }
    }

    // 关闭文件
    f_close(&SDFile);

    // 检查是否成功读取到值
    if (ratio_value < 0 && limit_value < 0) {
        my_printf(&huart1, "config.ini file format error.\r\n");
        return -1;
    }

    // 更新参数到内存（不自动保存到Flash）
    if (ratio_value >= 0) {
        g_system_params.ratio = ratio_value;
    }
    if (limit_value >= 0) {
        g_system_params.limit = limit_value;
    }

    // 输出读取结果
    my_printf(&huart1, "Ratio = %.2f\r\n", g_system_params.ratio);
    my_printf(&huart1, "Limit = %.2f\r\n", g_system_params.limit);
    my_printf(&huart1, "config read success\r\n");

    return 0;
}

/**
 * @brief  ratio命令：设置变比
 * @note   两步交互：
 *         1. 显示当前变比
 *         2. 提示输入新值(0-100)
 *         仅修改内存，不自动保存到Flash
 */
static int cmd_ratio(char *args)
{
    // 显示当前变比（从内存读取）
    my_printf(&huart1, "Ratio = %.1f\r\n", g_system_params.ratio);

    // 提示输入新值
    my_printf(&huart1, "Input value(0-100):\r\n");

    // 设置等待输入标志
    ratio_waiting_input = 1;

    return 0;
}

/**
 * @brief  limit命令：设置阈值
 * @note   两步交互：
 *         1. 显示当前阈值
 *         2. 提示输入新值(0-500)
 *         仅修改内存，不自动保存到Flash
 */
static int cmd_limit(char *args)
{
    // 显示当前阈值（从内存读取）
    my_printf(&huart1, "limit = %.2f\r\n", g_system_params.limit);

    // 提示输入新值
    my_printf(&huart1, "Input value(0-500):\r\n");

    // 设置等待输入标志
    limit_waiting_input = 1;

    return 0;
}

/**
 * @brief  config命令：参数存储管理
 * @note   子命令：
 *         - config save: 保存内存中的参数到Flash
 *         - config read: 从Flash读取参数到内存
 */
static int cmd_config(char *args)
{
    // 检查子命令
    if (args == NULL) {
        my_printf(&huart1, "Usage: config <save|read>\r\n");
        return -1;
    }

    if (strcmp(args, "save") == 0) {
        // 显示当前参数
        my_printf(&huart1, "ratio: %.2f\r\n", g_system_params.ratio);
        my_printf(&huart1, "limit: %.2f\r\n", g_system_params.limit);

        // 保存到Flash
        if (flash_param_write(&g_system_params) != HAL_OK) {
            my_printf(&huart1, "save parameters failed\r\n");
            return -1;
        }

        my_printf(&huart1, "save parameters to flash\r\n");
        return 0;
    }
    else if (strcmp(args, "read") == 0) {
        // 从Flash读取参数
        flash_param_t temp_param;
        if (flash_param_read(&temp_param) == HAL_OK) {
            // 读取成功，更新到全局变量
            memcpy(&g_system_params, &temp_param, sizeof(flash_param_t));

            my_printf(&huart1, "read parameters from flash\r\n");
            my_printf(&huart1, "ratio: %.2f\r\n", g_system_params.ratio);
            my_printf(&huart1, "limit: %.2f\r\n", g_system_params.limit);
        } else {
            my_printf(&huart1, "read parameters failed\r\n");
            return -1;
        }
        return 0;
    }
    else {
        my_printf(&huart1, "Unknown config command: %s\r\n", args);
        return -1;
    }
}

/**
 * @brief  解析标准时间格式: YYYY-MM-DD HH:MM:SS
 * @param  str: 输入字符串
 * @param  year, month, day, hour, minute, second: 输出参数
 * @retval 0: 成功, -1: 失败
 */
static int parse_standard_datetime(const char *str, int *year, int *month, int *day,
                                    int *hour, int *minute, int *second)
{
    // 使用sscanf解析标准格式
    int ret = sscanf(str, "%d-%d-%d %d:%d:%d", year, month, day, hour, minute, second);

    if (ret != 6) {
        return -1;  // 格式不匹配
    }

    // 范围检查
    if (*year < 2000 || *year > 2099) return -1;
    if (*month < 1 || *month > 12) return -1;
    if (*day < 1 || *day > 31) return -1;
    if (*hour < 0 || *hour > 23) return -1;
    if (*minute < 0 || *minute > 59) return -1;
    if (*second < 0 || *second > 59) return -1;

    return 0;
}

/**
 * @brief  RTC Config命令：提示输入时间
 */
static int cmd_rtc_config(char *args)
{
    my_printf(&huart1, "Input Datetime\r\n");
    rtc_waiting_input = 1;  // 设置等待输入标志
    return 0;
}

/**
 * @brief  RTC now命令：显示当前时间
 */
static int cmd_rtc_now(char *args)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    // 读取RTC时间和日期
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // 输出当前时间
    my_printf(&huart1, "Current Time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
              sDate.Year + 2000, sDate.Month, sDate.Date,
              sTime.Hours, sTime.Minutes, sTime.Seconds);

    return 0;
}

/**
 * @brief  start命令：启动采样
 */
static int cmd_start(char *args)
{
    return sample_start();
}

/**
 * @brief  stop命令：停止采样
 */
static int cmd_stop(char *args)
{
    return sample_stop();
}
//rtc_waiting_input一会儿我自己查找该标志位作用的地方

