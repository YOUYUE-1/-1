#ifndef __USART_APP_H__
#define __USART_APP_H__
#define UART_TIMEOUT_MS 100
#include "mydefine.h"

int my_printf(UART_HandleTypeDef *huart, const char *format, ...);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void uart_task(void);
#endif
