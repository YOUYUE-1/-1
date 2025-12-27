#include "stdio.h"
#include "string.h"
#include "stdarg.h"


#include "main.h"
#include "usart.h"
#include "dma.h"
#include "adc.h"
#include "tim.h"
#include "dac.h"
#include "rtc.h"
#include "oled.h"
#include "i2c.h"
#include "ringbuffer.h"
#include "ebtn.h"
#include "gd25qxx.h"

#include "scheduler.h"
#include "led_app.h"
#include "Key_app.h"
#include "usart_app.h"
#include "adc_app.h"
#include "Oled_app.h"
#include "flash_app.h"
#include "RTC_app.h"


extern uint8_t ucLed[6];
extern uint8_t uart_rx_buffer[128];
extern uint8_t uart_dma_buffer[128];
extern uint8_t uart_rx_dma_buffer[128];
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern struct rt_ringbuffer uart_ringbuffer;
extern uint8_t ringbuffer_pool[128];


