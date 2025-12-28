/**
  ******************************************************************************
  * @file           : sample_app.h
  * @brief          : 采样控制模块头文件
  * @author         : 哈雷酱 (CIMC 2025)
  ******************************************************************************
  * @attention
  * 本模块负责：
  * - 采样启停控制 (start/stop命令、KEY1按键)
  * - LED1闪烁指示 (1s周期)
  * - 周期性数据采样与输出 (5s/10s/15s)
  * - 电压计算 (ADC * 变比)
  * - 超限检测与LED2控制
  * - OLED显示更新
  ******************************************************************************
  */

#ifndef __SAMPLE_APP_H__
#define __SAMPLE_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
#include "main.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
    SAMPLE_STATE_IDLE = 0,    // 停止状态
    SAMPLE_STATE_RUNNING = 1  // 采样状态
} sample_state_t;

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief  采样任务函数，在调度器中周期调用
 */
void sample_task(void);

/**
 * @brief  启动采样
 * @retval 0: 成功
 */
int sample_start(void);

/**
 * @brief  停止采样
 * @retval 0: 成功
 */
int sample_stop(void);

/**
 * @brief  切换采样状态 (用于按键控制)
 * @retval 0: 成功
 */
int sample_toggle(void);

/**
 * @brief  设置采样周期
 * @param  period: 周期 (5/10/15秒)
 * @retval 0: 成功, -1: 参数无效
 */
int sample_set_period(uint8_t period);

/**
 * @brief  获取当前采样状态
 * @retval 采样状态
 */
sample_state_t sample_get_state(void);

/**
 * @brief  获取当前电压值（带变比）
 * @retval 电压值
 */
float sample_get_voltage(void);

#ifdef __cplusplus
}
#endif

#endif /* __SAMPLE_APP_H__ */
