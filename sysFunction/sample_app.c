/**
  ******************************************************************************
  * @file           : sample_app.c
  * @brief          : 采样控制模块实现
  * @author         : 哈雷酱 (CIMC 2025)
  ******************************************************************************
  * @note
  * LED索引对应关系: ucLed[0-5] 对应 LED1-LED6
  * - ucLed[0] = LED1 (采样指示，1s闪烁)
  * - ucLed[1] = LED2 (超限报警)
  * - ucLed[2-5] = LED3-6 (保留)
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sample_app.h"
#include "adc.h"
#include "config/config_manager.h"
#include "RTC_app.h"
#include "Oled_app.h"
#include "../Components/Oled/oled.h"
#include "usart_app.h"
#include "led_app.h"
#include "string.h"
#include "stdio.h"

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern ADC_HandleTypeDef hadc1;
extern uint32_t adc_dma_buffer[32];  // ADC DMA缓冲区
extern uint8_t ucLed[6];              // LED状态数组
extern RTC_TimeTypeDef g_CurrentTime;
extern RTC_DateTypeDef g_CurrentDate;

/* Private variables ---------------------------------------------------------*/
static sample_state_t g_sample_state = SAMPLE_STATE_IDLE;  // 采样状态
static uint32_t g_last_sample_tick = 0;  // 上次采样时间戳
static uint32_t g_led_blink_tick = 0;    // LED闪烁时间戳
static uint8_t g_led_blink_state = 0;    // LED闪烁状态
static uint32_t g_oled_update_tick = 0;  // OLED更新时间戳

/* Private function prototypes -----------------------------------------------*/
static void sample_output_data(void);
static float sample_read_voltage(void);
static void sample_update_oled(void);
static void sample_update_led(void);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  启动采样
 */
int sample_start(void)
{
    if (g_sample_state == SAMPLE_STATE_RUNNING) {
        return 0;  // 已经在运行
    }

    g_sample_state = SAMPLE_STATE_RUNNING;
    g_last_sample_tick = HAL_GetTick();
    g_led_blink_tick = HAL_GetTick();
    g_oled_update_tick = HAL_GetTick();
    g_led_blink_state = 0;

    // 输出启动信息
    my_printf(&huart1, "Periodic Sampling\r\n");
    my_printf(&huart1, "sample cycle: %ds\r\n", g_system_params.sample_period);

    // 立即执行一次采样和显示更新
    sample_output_data();
    sample_update_oled();

    return 0;
}

/**
 * @brief  停止采样
 */
int sample_stop(void)
{
    if (g_sample_state == SAMPLE_STATE_IDLE) {
        return 0;  // 已经停止
    }

    g_sample_state = SAMPLE_STATE_IDLE;

    // 熄灭LED1 (ucLed[0] = LED1)
    ucLed[0] = 0;

    // OLED显示system idle
    OLED_Clear();
    Oled_printf(0, 0, "system idle");

    // 输出停止信息
    my_printf(&huart1, "Periodic Sampling STOP\r\n");

    return 0;
}

/**
 * @brief  切换采样状态
 */
int sample_toggle(void)
{
    if (g_sample_state == SAMPLE_STATE_IDLE) {
        return sample_start();
    } else {
        return sample_stop();
    }
}

/**
 * @brief  设置采样周期
 */
int sample_set_period(uint8_t period)
{
    // 验证周期 (5/10/15秒)
    if (period != 5 && period != 10 && period != 15) {
        return -1;
    }

    g_system_params.sample_period = period;

    // 如果正在采样，输出周期调整信息
    if (g_sample_state == SAMPLE_STATE_RUNNING) {
        my_printf(&huart1, "sample cycle adjust: %ds\r\n", period);
    }

    return 0;
}

/**
 * @brief  获取采样状态
 */
sample_state_t sample_get_state(void)
{
    return g_sample_state;
}

/**
 * @brief  获取当前电压值（带变比）
 */
float sample_get_voltage(void)
{
    float raw_voltage = sample_read_voltage();
    return raw_voltage * g_system_params.ratio;
}

/**
 * @brief  采样任务函数
 */
void sample_task(void)
{
    uint32_t current_tick = HAL_GetTick();

    // 如果是停止状态，不执行采样逻辑
    if (g_sample_state == SAMPLE_STATE_IDLE) {
        return;
    }

    // LED1闪烁控制 (1s周期)
    sample_update_led();

    // OLED更新 (1s周期)
    if (current_tick - g_oled_update_tick >= 1000) {
        g_oled_update_tick = current_tick;
        sample_update_oled();
    }

    // 周期性采样输出 (5/10/15s周期)
    uint32_t sample_period_ms = g_system_params.sample_period * 1000;
    if (current_tick - g_last_sample_tick >= sample_period_ms) {
        g_last_sample_tick = current_tick;
        sample_output_data();
    }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  读取ADC原始电压
 */
static float sample_read_voltage(void)
{
    uint32_t adc_sum = 0;

    // 计算平均值
    for (uint16_t i = 0; i < 32; i++) {
        adc_sum += adc_dma_buffer[i];
    }

    uint32_t adc_val = adc_sum / 32;

    // 转换为电压 (3.3V参考, 12位ADC)
    float voltage = ((float)adc_val * 3.3f) / 4096.0f;

    return voltage;
}

/**
 * @brief  输出采样数据
 */
static void sample_output_data(void)
{
    float voltage = sample_get_voltage();
    bool is_over_limit = false;

    // 检查是否超限
    if (voltage > g_system_params.limit) {
        is_over_limit = true;
        ucLed[1] = 1;  // LED2点亮 (ucLed[1] = LED2)
    } else {
        ucLed[1] = 0;  // LED2熄灭
    }

    // 输出格式: 2025-01-01 00:30:05 ch0=10.5V
    if (is_over_limit) {
        my_printf(&huart1, "%04d-%02d-%02d %02d:%02d:%02d ch0=%.2fV OverLimit (%.2f) !\r\n",
                  g_CurrentDate.Year + 2000, g_CurrentDate.Month, g_CurrentDate.Date,
                  g_CurrentTime.Hours, g_CurrentTime.Minutes, g_CurrentTime.Seconds,
                  voltage, g_system_params.limit);
    } else {
        my_printf(&huart1, "%04d-%02d-%02d %02d:%02d:%02d ch0=%.2fV\r\n",
                  g_CurrentDate.Year + 2000, g_CurrentDate.Month, g_CurrentDate.Date,
                  g_CurrentTime.Hours, g_CurrentTime.Minutes, g_CurrentTime.Seconds,
                  voltage);
    }
}

/**
 * @brief  更新OLED显示
 */
static void sample_update_oled(void)
{
    float voltage = sample_get_voltage();

    // 第一行：时间 (hh:mm:ss)
    Oled_printf(0, 0, "%02d:%02d:%02d",
                g_CurrentTime.Hours, g_CurrentTime.Minutes, g_CurrentTime.Seconds);

    // 第二行：电压 (xx.xx V)
    Oled_printf(0, 1, "%.2f V    ", voltage);
}

/**
 * @brief  更新LED1闪烁
 */
static void sample_update_led(void)
{
    uint32_t current_tick = HAL_GetTick();

    // 1s周期 = 500ms亮 + 500ms灭
    if (current_tick - g_led_blink_tick >= 500) {
        g_led_blink_tick = current_tick;
        g_led_blink_state = !g_led_blink_state;
        ucLed[0] = g_led_blink_state;  // LED1 (ucLed[0] = LED1)
    }
}
