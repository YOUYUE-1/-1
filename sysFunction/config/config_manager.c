/**
  ******************************************************************************
  * @file           : config_manager.c
  * @brief          : Flash参数管理模块实现
  * @author         : 哈雷酱 (CIMC 2025)
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "config_manager.h"
#include "gd25qxx.h"
#include "string.h"
#include "stdio.h"

/* Private defines -----------------------------------------------------------*/
#define CRC32_POLYNOMIAL 0xEDB88320  // CRC32多项式

/* Private variables ---------------------------------------------------------*/
flash_param_t g_system_params = {0};  // 全局系统参数
static uint8_t g_params_loaded = 0;   // 参数加载标志

/* Private function prototypes -----------------------------------------------*/
static uint32_t calc_crc32(const uint8_t *data, uint32_t length);
static bool verify_checksum(const flash_param_t *param);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  初始化Flash参数管理模块
 */
HAL_StatusTypeDef flash_param_init(void)
{
    flash_param_t temp_param;

    // 0. 检查Flash是否就绪（读取ID验证）
    uint32_t flash_id = spi_flash_read_id();
    if (flash_id == 0x000000 || flash_id == 0xFFFFFF) {
        // Flash未就绪或不存在，使用默认参数
        goto use_default_params;
    }

    // 1. 从Flash读取参数
    spi_flash_buffer_read((uint8_t*)&temp_param, FLASH_PARAM_BASE_ADDR,
                          sizeof(flash_param_t));

    // 2. 验证魔数和校验和
    if (temp_param.magic == FLASH_PARAM_MAGIC && verify_checksum(&temp_param)) {
        // 参数有效，复制到全局变量
        memcpy(&g_system_params, &temp_param, sizeof(flash_param_t));
        g_params_loaded = 1;
        return HAL_OK;
    }

use_default_params:
    // 3. 首次上电或参数无效，创建默认参数
    g_system_params.magic = FLASH_PARAM_MAGIC;
    g_system_params.boot_count = 0;
    g_system_params.ratio = 1.0f;        // 默认变比1:1
    g_system_params.limit = 100.0f;      // 默认阈值100V
    g_system_params.sample_period = 5;   // 默认5秒周期
    strcpy(g_system_params.device_id, "Device_ID:2025-CIMC-001");

    // 4. 计算校验和
    g_system_params.checksum = calc_crc32((uint8_t*)&g_system_params,
                                          sizeof(flash_param_t) - sizeof(uint32_t));

    // 5. 仅当Flash有效时才写入
    if (flash_id != 0x000000 && flash_id != 0xFFFFFF) {
        spi_flash_sector_erase(FLASH_PARAM_BASE_ADDR);
        spi_flash_buffer_write((uint8_t*)&g_system_params, FLASH_PARAM_BASE_ADDR,
                               sizeof(flash_param_t));
    }

    g_params_loaded = 1;
    return HAL_OK;
}

/**
 * @brief  从Flash读取参数
 */
HAL_StatusTypeDef flash_param_read(flash_param_t *param)
{
    if (param == NULL) return HAL_ERROR;

    spi_flash_buffer_read((uint8_t*)param, FLASH_PARAM_BASE_ADDR,
                          sizeof(flash_param_t));

    // 验证数据有效性
    if (param->magic != FLASH_PARAM_MAGIC || !verify_checksum(param)) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief  写入参数到Flash
 */
HAL_StatusTypeDef flash_param_write(const flash_param_t *param)
{
    if (param == NULL) return HAL_ERROR;

    flash_param_t temp_param;
    memcpy(&temp_param, param, sizeof(flash_param_t));

    // 计算校验和
    temp_param.checksum = calc_crc32((uint8_t*)&temp_param,
                                     sizeof(flash_param_t) - sizeof(uint32_t));

    // 擦除扇区并写入
    spi_flash_sector_erase(FLASH_PARAM_BASE_ADDR);
    spi_flash_buffer_write((uint8_t*)&temp_param, FLASH_PARAM_BASE_ADDR,
                           sizeof(flash_param_t));

    // 更新全局变量
    memcpy(&g_system_params, &temp_param, sizeof(flash_param_t));

    return HAL_OK;
}

/**
 * @brief  获取并递增上电次数
 */
uint32_t flash_get_boot_count(void)
{
    g_system_params.boot_count++;
    flash_param_write(&g_system_params);  // 立即保存
    return g_system_params.boot_count;
}

/**
 * @brief  设置变比
 */
HAL_StatusTypeDef flash_set_ratio(float ratio)
{
    // 验证范围
    if (ratio < 0.0f || ratio > 100.0f) {
        return HAL_ERROR;
    }

    g_system_params.ratio = ratio;
    return flash_param_write(&g_system_params);
}

/**
 * @brief  设置阈值
 */
HAL_StatusTypeDef flash_set_limit(float limit)
{
    // 验证范围
    if (limit < 0.0f || limit > 500.0f) {
        return HAL_ERROR;
    }

    g_system_params.limit = limit;
    return flash_param_write(&g_system_params);
}

/**
 * @brief  设置采样周期
 */
HAL_StatusTypeDef flash_set_period(uint8_t period)
{
    // 验证有效值
    if (period != 5 && period != 10 && period != 15) {
        return HAL_ERROR;
    }

    g_system_params.sample_period = period;
    return flash_param_write(&g_system_params);
}

/**
 * @brief  设置设备ID
 */
HAL_StatusTypeDef flash_set_device_id(const char *device_id)
{
    if (device_id == NULL) return HAL_ERROR;

    strncpy(g_system_params.device_id, device_id, sizeof(g_system_params.device_id) - 1);
    g_system_params.device_id[sizeof(g_system_params.device_id) - 1] = '\0';

    return flash_param_write(&g_system_params);
}

/**
 * @brief  获取变比
 */
float flash_get_ratio(void)
{
    return g_system_params.ratio;
}

/**
 * @brief  获取阈值
 */
float flash_get_limit(void)
{
    return g_system_params.limit;
}

/**
 * @brief  获取采样周期
 */
uint8_t flash_get_period(void)
{
    return g_system_params.sample_period;
}

/**
 * @brief  获取设备ID
 */
const char* flash_get_device_id(void)
{
    return g_system_params.device_id;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  计算CRC32校验和
 */
static uint32_t calc_crc32(const uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            } else {
                crc = crc >> 1;
            }
        }
    }

    return ~crc;
}

/**
 * @brief  验证校验和
 */
static bool verify_checksum(const flash_param_t *param)
{
    uint32_t calculated_crc = calc_crc32((uint8_t*)param,
                                         sizeof(flash_param_t) - sizeof(uint32_t));
    return (calculated_crc == param->checksum);
}
