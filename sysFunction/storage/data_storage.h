/**
  ******************************************************************************
  * @file           : data_storage.h
  * @brief          : SD卡数据存储模块头文件
  * @author         : 哈雷酱 (CIMC 2025)
  ******************************************************************************
  * @attention
  * 本模块负责SD卡数据存储管理，包括：
  * - 创建数据文件夹：sample / overLimit / log / hideData
  * - 文件命名规则管理
  * - 数据写入接口
  ******************************************************************************
  */

#ifndef __DATA_STORAGE_H
#define __DATA_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "main.h"
#include "fatfs.h"

/* Exported defines ----------------------------------------------------------*/
#define FOLDER_SAMPLE      "sample"       // 采样数据文件夹
#define FOLDER_OVERLIMIT   "overLimit"    // 超限数据文件夹
#define FOLDER_LOG         "log"          // 日志文件夹
#define FOLDER_HIDEDATA    "hideData"     // 加密数据文件夹

/* Exported types ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief  初始化数据存储模块
 * @note   创建SD卡根目录下的4个数据文件夹
 * @retval HAL_OK: 成功, HAL_ERROR: 失败
 */
HAL_StatusTypeDef storage_init(void);

/**
 * @brief  检查SD卡是否挂载
 * @retval 1: 已挂载, 0: 未挂载
 */
uint8_t storage_is_mounted(void);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_STORAGE_H */
