/**
  ******************************************************************************
  * @file           : config_manager.h
  * @brief          : Flash参数管理模块头文件
  * @author         : 哈雷酱 (CIMC 2025)
  ******************************************************************************
  * @attention
  * 本模块负责管理Flash中的系统参数，包括：
  * - 变比(ratio): 0-100
  * - 阈值(limit): 0-500V
  * - 采样周期(sample_period): 5/10/15秒
  * - 设备ID(device_id): "Device_ID:2025-CIMC-XXX"
  * - 上电次数(boot_count): 用于log文件命名
  ******************************************************************************
  */

#ifndef __CONFIG_MANAGER_H
#define __CONFIG_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
#include "main.h"

/* Exported defines ----------------------------------------------------------*/
#define FLASH_PARAM_BASE_ADDR    0x000000    // 参数存储起始地址
#define FLASH_PARAM_SECTOR_SIZE  4096        // 4KB扇区大小
#define FLASH_PARAM_MAGIC        0xA5A5CAFE  // 魔数，验证数据有效性

/* Exported types ------------------------------------------------------------*/
/**
 * @brief Flash参数结构体（总大小88字节）
 */
typedef struct {
    uint32_t magic;           // 0x000000: 魔数(0xA5A5CAFE)
    uint32_t boot_count;      // 0x000004: 上电次数
    float    ratio;           // 0x000008: 变比(0-100)
    float    limit;           // 0x00000C: 阈值(0-500V)
    uint8_t  sample_period;   // 0x000010: 采样周期(5/10/15秒)
    uint8_t  reserved[3];     // 0x000011: 保留字节（对齐）
    char     device_id[64];   // 0x000014: Device_ID字符串
    uint32_t checksum;        // 0x000054: CRC32校验和
} flash_param_t;

/* Exported variables --------------------------------------------------------*/
extern flash_param_t g_system_params;  // 全局系统参数

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief  初始化Flash参数管理模块
 * @note   首次上电会创建默认参数，后续上电会读取保存的参数
 * @retval HAL_OK: 成功, HAL_ERROR: 失败
 */
HAL_StatusTypeDef flash_param_init(void);

/**
 * @brief  从Flash读取参数
 * @param  param: 参数结构体指针
 * @retval HAL_OK: 成功, HAL_ERROR: 失败
 */
HAL_StatusTypeDef flash_param_read(flash_param_t *param);

/**
 * @brief  写入参数到Flash
 * @param  param: 参数结构体指针
 * @retval HAL_OK: 成功, HAL_ERROR: 失败
 */
HAL_StatusTypeDef flash_param_write(const flash_param_t *param);

/**
 * @brief  获取并递增上电次数
 * @note   用于log文件命名（log0.txt, log1.txt...）
 * @retval 当前上电次数
 */
uint32_t flash_get_boot_count(void);

/**
 * @brief  设置变比
 * @param  ratio: 变比值(0-100)
 * @retval HAL_OK: 成功, HAL_ERROR: 参数无效
 */
HAL_StatusTypeDef flash_set_ratio(float ratio);

/**
 * @brief  设置阈值
 * @param  limit: 阈值(0-500V)
 * @retval HAL_OK: 成功, HAL_ERROR: 参数无效
 */
HAL_StatusTypeDef flash_set_limit(float limit);

/**
 * @brief  设置采样周期
 * @param  period: 周期(5/10/15秒)
 * @retval HAL_OK: 成功, HAL_ERROR: 参数无效
 */
HAL_StatusTypeDef flash_set_period(uint8_t period);

/**
 * @brief  设置设备ID
 * @param  device_id: 设备ID字符串
 * @retval HAL_OK: 成功, HAL_ERROR: 失败
 */
HAL_StatusTypeDef flash_set_device_id(const char *device_id);

/**
 * @brief  获取变比
 * @retval 当前变比
 */
float flash_get_ratio(void);

/**
 * @brief  获取阈值
 * @retval 当前阈值
 */
float flash_get_limit(void);

/**
 * @brief  获取采样周期
 * @retval 当前采样周期
 */
uint8_t flash_get_period(void);

/**
 * @brief  获取设备ID
 * @retval 设备ID字符串指针
 */
const char* flash_get_device_id(void);

#ifdef __cplusplus
}
#endif

#endif /* __CONFIG_MANAGER_H */
