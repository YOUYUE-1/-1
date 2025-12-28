/**
  ******************************************************************************
  * @file           : system_test.h
  * @brief          : 系统自检模块头文件
  * @author         : 哈雷酱 (CIMC 2025)
  ******************************************************************************
  * @attention
  * 本模块实现test命令，检测：
  * - Flash: 读取ID判断是否正常
  * - TF card: 检测是否存在并读取容量
  * - RTC: 读取当前时间
  ******************************************************************************
  */

#ifndef __SYSTEM_TEST_H
#define __SYSTEM_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief  执行系统自检
 * @note   检测Flash/TF卡/RTC状态并通过串口输出
 * @retval 0: 全部通过, -1: 有设备测试失败
 */
int system_selftest(void);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_TEST_H */
