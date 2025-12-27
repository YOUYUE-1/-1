#include "Key_app.h"

typedef enum {
    Key1 = 0,  // 菜单键
    Key2,     // 确认键 (自动是1)
		Key3,
		Key4,
		Key5,
		Key6
} MyKeyIDs;

// 定义按键的参数配置 (必须放在 my_buttons 数组之前)
static const ebtn_btn_param_t common_cfg = EBTN_PARAMS_INIT(
    20,   // 按下去抖时间 (ms)
    20,   // 松开去抖时间 (ms)
    50,   // 最小短按时间 (ms)，小于这个时间不算按下
    1000, // 最大短按时间 (ms)，超过这个时间进入长按逻辑
    300,  // 连击间隔时间 (ms)，双击之间的最大间隔
    500,  // 长按周期 (ms)，长按触发后每隔多久发一次事件
    1     // 最大连击次数 (例如只支持双击就填2，支持三连击填3)
);

// 你的按键数组定义 (紧接着写在后面)
static ebtn_btn_t my_buttons[] = {
    EBTN_BUTTON_INIT(Key1, &common_cfg), // 这里现在能找到 common_cfg 了
    EBTN_BUTTON_INIT(Key2, &common_cfg), 
		EBTN_BUTTON_INIT(Key3, &common_cfg), // 这里现在能找到 common_cfg 了
    EBTN_BUTTON_INIT(Key4, &common_cfg), 
		EBTN_BUTTON_INIT(Key5, &common_cfg), // 这里现在能找到 common_cfg 了
    EBTN_BUTTON_INIT(Key6, &common_cfg), 
};


// 示例：假设我们使用 key_id 来代表 GPIO 的 Pin 号
// 返回值：1 代表按键被按下（有效），0 代表松开
uint8_t my_btn_get_state(ebtn_btn_t* btn)
{
    uint8_t is_pressed = 0;

    // 根据你在定义按键时设置的 key_id 来区分不同的物理引脚
    switch (btn->key_id)
    {
        case 0: // 对应按键 0
            // 假设低电平有效 (按下接地)
            // HAL_GPIO_ReadPin 是 STM32 的库函数，请替换为你平台的函数
            if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_13) == GPIO_PIN_RESET) {
                is_pressed = 1;
            }
            break;

        case 1: // 对应按键 1
            if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_12) == GPIO_PIN_RESET) {
                is_pressed = 1;
            }
            break;
            
       case 2: // 对应按键 2
            if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11) == GPIO_PIN_RESET) {
                is_pressed = 1;
            }
            break;
						
				case 3: // 对应按键 3
            if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10) == GPIO_PIN_RESET) {
                is_pressed = 1;
            }
            break;
						
				case 4: // 对应按键 4
            if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_9) == GPIO_PIN_RESET) {
                is_pressed = 1;
            }
            break;
						
				case 5: // 对应按键 5
            if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_8) == GPIO_PIN_RESET) {
                is_pressed = 1;
            }
            break;
						
    }

    return is_pressed;
}


void my_btn_callback(ebtn_btn_t* btn, ebtn_evt_t evt)
{
    // 必须加这一句！只在“单击确认”事件发生时才执行
    // 如果你想按下就亮，这里改成 EBTN_EVT_ONPRESS
    if (evt != EBTN_EVT_ONCLICK) {
        return; 
    }

    switch (btn->key_id)
    {
        case Key1: ucLed[0] ^= 1; break;
        case Key2: ucLed[1] ^= 1; break;
        case Key3: ucLed[2] ^= 1; break;
        case Key4: ucLed[3] ^= 1; break;
        case Key5: ucLed[4] ^= 1; break;
        case Key6: ucLed[5] ^= 1; break;
    }
}

void app_btn_init(void)
{
	// --- ebtn 库初始化 ---
    // 参数含义: 
    // 1. 数组地址, 2. 数组长度, 3. 组合键(无), 4. 组合键数(0), 
    // 5. 硬件读取函数, 6. 事件回调函数
    ebtn_init(my_buttons, 
              EBTN_ARRAY_SIZE(my_buttons), 
              NULL, 0, 
              my_btn_get_state, 
              my_btn_callback);
	
}

void ebtn_task(void)
{
	ebtn_process(uwTick);
}
