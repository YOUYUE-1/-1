/**
  ******************************************************************************
  * @file           : data_storage.c
  * @brief          : SD卡数据存储模块实现
  * @author         : 哈雷酱 (CIMC 2025)
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "data_storage.h"
#include "string.h"
#include "stdio.h"

/* Private variables ---------------------------------------------------------*/
static uint8_t g_storage_initialized = 0;  // 初始化标志

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  初始化数据存储模块
 */
HAL_StatusTypeDef storage_init(void)
{
    FRESULT res;
    const char *folders[] = {FOLDER_SAMPLE, FOLDER_OVERLIMIT, FOLDER_LOG, FOLDER_HIDEDATA};

    // 1. 确保SD卡已挂载
    if (!storage_is_mounted()) {
        // 尝试挂载
        extern FATFS SDFatFS;
        extern char SDPath[4];
        res = f_mount(&SDFatFS, SDPath, 1);
        if (res != FR_OK) {
            return HAL_ERROR;
        }
    }

    // 2. 创建4个数据文件夹
    for (int i = 0; i < 4; i++) {
        res = f_mkdir(folders[i]);
        // 忽略已存在错误（FR_EXIST）
        if (res != FR_OK && res != FR_EXIST) {
            // printf("Failed to create folder: %s (error: %d)\r\n", folders[i], res);
            // 继续创建其他文件夹
        }
    }

    g_storage_initialized = 1;
    return HAL_OK;
}

/**
 * @brief  检查SD卡是否挂载
 */
uint8_t storage_is_mounted(void)
{
    extern FATFS SDFatFS;
    return (SDFatFS.fs_type != 0);  // fs_type非0表示已挂载
}
