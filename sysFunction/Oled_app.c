#include "Oled_app.h"


int Oled_printf(uint8_t x,uint8_t y, const char *format, ...)
{
	char buffer[512]; // 临时存储格式化后的字符串
	va_list arg;      // 处理可变参数
	int len;          // 最终字符串长度

	va_start(arg, format);
	// 安全地格式化字符串到 buffer
	len = vsnprintf(buffer, sizeof(buffer), format, arg);
	va_end(arg);

	// 通过 HAL 库发送 buffer 中的内容
	OLED_ShowStr(x,y,buffer,8);
	return len;
}




void oled_task(void)
{

	
}
