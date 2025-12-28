#include "usart_app.h"
#include "usart.h"
#include "mydefine.h"
uint8_t uart_rx_buffer[128]={0};
uint8_t uart_rx_dma_buffer[128]={0};
uint8_t uart_dma_buffer[128]={0};
uint32_t uart_rx_ticks=0;
uint16_t uart_rx_index=0;
uint8_t uart_flag=0;
struct rt_ringbuffer uart_ringbuffer;
uint8_t ringbuffer_pool[128];


int my_printf(UART_HandleTypeDef *huart, const char *format, ...)
{
	char buffer[512]; // 临时存储格式化后的字符串
	va_list arg;      // 处理可变参数
	int len;          // 最终字符串长度

	va_start(arg, format);
	// 安全地格式化字符串到 buffer
	len = vsnprintf(buffer, sizeof(buffer), format, arg);
	va_end(arg);

	// 通过 HAL 库发送 buffer 中的内容
	HAL_UART_Transmit(huart, (uint8_t *)buffer, (uint16_t)len, 0xFF);
	return len;
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    // 1. 核对身份：是 USART1 的快递员吗？
//	if (huart->Instance == USART1)
//	{     
//		uart_rx_ticks = uwTick;
//		uart_rx_index++;
//        // 4. 准备下次收货：再次告诉硬件，我还想收一个字节
//		HAL_UART_Receive_IT(&huart1, &uart_rx_buffer[uart_rx_index], 1);
//	}
//}

//void uart_task(void)
//{
//    // 1. 检查货架：如果计数器为0，说明没货或刚处理完，休息。
//	if (uart_rx_index == 0)
//		return;

//    // 2. 检查手表：当前时间 - 最后收货时间 > 规定的超时时间？
//	if (uwTick - uart_rx_ticks > UART_TIMEOUT_MS) // 核心判断
//	{
//        // --- 3. 超时！开始理货 --- 
//        // "uart_rx_buffer" 里从第0个到第 "uart_rx_index - 1" 个
//        // 就是我们等到的一整批货（一帧数据）
//		my_printf(&huart1, "uart data 单片机说: %s\n", uart_rx_buffer);
//        // (在这里加入你自己的处理逻辑，比如解析命令控制LED)
//        // --- 理货结束 --- 

//		// 4. 清理现场：把处理完的货从货架上拿走，计数器归零
//		memset(uart_rx_buffer, 0, uart_rx_index);
//		uart_rx_index = 0;
//		huart1.pRxBuffPtr = uart_rx_buffer;
//	}
//    // 如果没超时，啥也不做，等下次再检查
//}		//超时解析

/**
 * @brief UART DMA接收完成或空闲事件回调函数
 * @param huart UART句柄
 * @param Size 指示在事件发生前，DMA已经成功接收了多少字节的数据
 * @retval None
 */
//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
//{
//    // 1. 确认是目标串口 (USART1)
//    if (huart->Instance == USART1)
//    {
//        // 2. 紧急停止当前的 DMA 传输 (如果还在进行中)
//        //    因为空闲中断意味着发送方已经停止，防止 DMA 继续等待或出错
//        HAL_UART_DMAStop(huart);

//        // 3. 将 DMA 缓冲区中有效的数据 (Size 个字节) 复制到待处理缓冲区
//        memcpy(uart_dma_buffer, uart_rx_dma_buffer, Size); 
//        // 注意：这里使用了 Size，只复制实际接收到的数据
//        
//        // 4. 举起"到货通知旗"，告诉主循环有数据待处理
//        uart_flag = 1;

//        // 5. 清空 DMA 接收缓冲区，为下次接收做准备
//        //    虽然 memcpy 只复制了 Size 个，但清空整个缓冲区更保险
//        memset(uart_rx_dma_buffer, 0, sizeof(uart_rx_dma_buffer));

//        // 6. **关键：重新启动下一次 DMA 空闲接收**
//        //    必须再次调用，否则只会接收这一次
//        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_dma_buffer, sizeof(uart_rx_dma_buffer));
//        
//				// 7. 如果之前关闭了半满中断，可能需要在这里再次关闭 (根据需要)
//        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
//    }
//}

//void uart_task(void)
//{
//    // 1. 检查"到货通知旗"
//    if(uart_flag == 0) 
//        return; // 旗子没举起来，说明没新货，直接返回
//    
//    // 2. 放下旗子，表示我们已经注意到新货了
//    //    防止重复处理同一批数据
//    uart_flag = 0;
//	
//    // 3. 处理 "待处理货架" (uart_dma_buffer) 中的数据
//    //    这里简单地打印出来，实际应用中会进行解析、执行命令等
//    my_printf(&huart1,"DMA data: %s\n", uart_dma_buffer);
//    //    (注意：如果数据不是字符串，需要用其他方式处理，比如按字节解析)
//    
//    // 4. 清空"待处理货架"，为下次接收做准备
//    memset(uart_dma_buffer, 0, sizeof(uart_dma_buffer));
//}//以uart_flag = 1;的空闲解析




//环形缓存区示例
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // 1. 确认是目标串口 (USART1)
    if (huart->Instance == USART1)
    {
        // 2. 紧急停止当前的 DMA 传输 (如果还在进行中)
        //    因为空闲中断意味着发送方已经停止，防止 DMA 继续等待或出错
        HAL_UART_DMAStop(huart);

       rt_ringbuffer_put(&uart_ringbuffer,uart_rx_dma_buffer,Size);

        // 5. 清空 DMA 接收缓冲区，为下次接收做准备
        //    虽然 memcpy 只复制了 Size 个，但清空整个缓冲区更保险
			// memset(uart_rx_dma_buffer, 0, sizeof(uart_rx_dma_buffer)); 
			// 可以忽略这一步，因为Size的存在rt_ringbuffer_put函数只会把新一轮的数据传入uart_ringbuffer结构体，老数据或被新数据覆盖或留在缓存区中不被处理，但是依然存在如果打印uart_rx_dma_buffer会把残留的老数据打印出来
			
			
        // 6. **关键：重新启动下一次 DMA 空闲接收**
        //    必须再次调用，否则只会接收这一次
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_dma_buffer, sizeof(uart_rx_dma_buffer));
        
				// 7. 如果之前关闭了半满中断，可能需要在这里再次关闭 (根据需要)
        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    }
}

void uart_task(void)
{
	uint16_t lenth;
	lenth = rt_ringbuffer_data_len(&uart_ringbuffer);
	if (lenth == 0) return;

	// 从环形缓冲区读取数据
	rt_ringbuffer_get(&uart_ringbuffer, uart_dma_buffer, lenth);
	uart_dma_buffer[lenth] = '\0';

	// 回显输入的命令（赛题要求）
	my_printf(&huart1, "%s\r\n", uart_dma_buffer);

	// 调用Shell命令处理框架（赛题要求的命令）
	shell_process_command((char *)uart_dma_buffer);
}
//以uart_flag = 1;的空闲解析



//void uart_task(void)
//{
//  uint16_t lenth;  
//	lenth=rt_ringbuffer_data_len(&uart_ringbuffer);
//	if(lenth==0) return;
//	
//	rt_ringbuffer_get(&uart_ringbuffer,uart_dma_buffer,lenth);
//	uart_dma_buffer[lenth] = '\0';
//    // 3. 处理 "待处理货架" (uart_dma_buffer) 中的数据
//    //    这里简单地打印出来，实际应用中会进行解析、执行命令等
//   my_printf(&huart1,"DMA data: %s\n", uart_dma_buffer);
//    //    (注意：如果数据不是字符串，需要用其他方式处理，比如按字节解析)
//    
//	if (strstr((char *)uart_dma_buffer, "LED0_ON") != NULL)
//	{
//		ucLed[0]^=1;
//		my_printf(&huart1,"LED0_SUCCESS\r\n");
//	}
//	else if (strstr((char *)uart_dma_buffer, "LED1_ON") != NULL)
//	{
//		ucLed[1]^=1;
//		my_printf(&huart1,"LED1_SUCCESS\r\n");
//	}
//	else
//		my_printf(&huart1,"EEROR\r\n");
//    // 4. 清空"待处理货架"，为下次接收做准备
//  // memset(uart_dma_buffer, 0, sizeof(uart_dma_buffer));
//	//该操作可省略原理就是前面在新数据末尾加入\0操作，可以让strstr读取函数时只读取到新数据
//}//以uart_flag = 1;的空闲解析

