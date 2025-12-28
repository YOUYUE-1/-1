#include "led_app.h"
#include <math.h> // ?????????? sinf ??

uint8_t ucLed[6] = {0,1,0,1,0,1};  // LED 状态数组 (6个LED)

/**
 * @brief 根据ucLed数组状态更新6个LED的显示
 * @param ucLed Led数据储存数组 (大小为6)
 */
void led_disp(uint8_t *ucLed)
{
    uint8_t temp = 0x00;                // 用于记录当前 LED 状态的临时变量 (最低6位有效)
    static uint8_t temp_old = 0xff;     // 记录之前 LED 状态的变量, 用于判断是否需要更新显示

    for (int i = 0; i < 6; i++)         // 遍历6个LED的状态
    {
        // 将LED状态整合到temp变量中，方便后续比较
        if (ucLed[i]) temp |= (1 << i); // 如果ucLed[i]为1, 则将temp的第i位置1
    }

    // 仅当当前状态与之前状态不同的时候，才更新显示
    if (temp != temp_old)
    {
        // 使用HAL库函数根据temp的值设置对应引脚状态 (假设高电平点亮)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (temp & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET); // LED 0 (PB12)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (temp & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET); // LED 1 (PB13)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, (temp & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET); // LED 2 (PB14)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (temp & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET); // LED 3 (PB15)
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0,  (temp & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET); // LED 4 (PD8)
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1,  (temp & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET); // LED 5 (PD9)

        temp_old = temp;                // 更新记录的旧状态
    }
}

/**
 * @brief LED 显示处理函数 (主循环调用)
 */
void led_task(void)
{
    led_disp(ucLed);                    // 调用led_disp函数更新LED状态
	
}

//void led_task(void)
//{
//	static uint32_t breathcount=0;
//	static uint8_t brightcount=0;
//	static uint8_t pwmmax=10;
//	static uint16_t breathcircle=2000;
//	static uint8_t pwmcount=0;
//	
//	breathcount=(breathcount+1)%breathcircle;	
//	
//	brightcount=(uint8_t)((sinf((breathcount*2.0f*3.14159f)/breathcircle)+1.0f)*pwmmax/2.0f);
//	
//	pwmcount=(pwmcount+1)%pwmmax;
//	
//	ucLed[0]=(pwmcount<brightcount)?1:0;
//	
//	led_disp(ucLed);
//}

