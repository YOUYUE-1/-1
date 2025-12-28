/**
  ******************************************************************************
  * @file           : shell_cmds.h
  * @brief          : Shell命令扩展模块头文件
  * @author         : 哈雷酱 (CIMC 2025)
  ******************************************************************************
  * @attention
  * 本模块负责处理赛题要求的所有串口命令：
  * - test: 系统自检
  * - RTC Config: RTC时间设置
  * - RTC now: 显示当前时间
  * - conf: 读取config.ini
  * - ratio: 设置变比
  * - limit: 设置阈值
  * - config save/read: 参数保存/读取
  * - start/stop: 采样启停
  * - hide/unhide: 数据加密
  ******************************************************************************
  */

#ifndef __SHELL_CMDS_H
#define __SHELL_CMDS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief  处理串口接收到的命令
 * @param  cmd_line: 命令字符串
 * @retval 0: 成功, -1: 未知命令
 */
int shell_process_command(char *cmd_line);

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_CMDS_H */
