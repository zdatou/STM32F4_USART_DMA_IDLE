#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lan8720.h"
#include "timer.h"
#include "usmart.h"
#include "malloc.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"


void usart_init(void);
int usart2_call_back(void * str, u16 len, void * store, u16 count);
int usart3_call_back(void * str, u16 len, void * store, u16 count);
int usart1_call_back(void * str, u16 len, void * store, u16 count);
int usart4_call_back(void * str, u16 len, void * store, u16 count);
int usart5_call_back(void * str, u16 len, void * store, u16 count);
int usart6_call_back(void * str, u16 len, void * store, u16 count);

int main(void)
{   
    HAL_Init();                     //初始化HAL库
	Stm32_Clock_Init(336,8,2,7);   //设置时钟,168Mhz   
    delay_init(168);                //初始化延时函数
   
	usart_init(); //初始化串口
	my_mem_init(SRAMIN);		    //初始化内部内存池
   	
	TIM_Init(6, 1000, 84); //任务轮询定时器 1ms
	TIM_Init(7, 1000, 84); //测量代码运行时间定时器 1us
	TIM_Base_Start(6); //启动任务轮询定时器	
	
	while(lwip_comm_init())         //lwip初始化
	{
		delay_ms(500);
        delay_ms(500);
	}
#if LWIP_DHCP   //使用DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//等待DHCP获取成功/超时溢出
	{  
		lwip_periodic_handle();	//LWIP内核需要定时处理的函数
	}
#endif
	while(1)
	{
        lwip_periodic_handle();	//LWIP内核需要定时处理的函数
	}
}

void usart_init(void)
{
	usart1.USART_Handler.Instance = USART1;
	usart1.USART_Handler.Init.BaudRate = 115200;
	usart1.USART_Handler.Init.Parity = UART_PARITY_NONE;
	usart1.func = usart1_call_back;
	com_init(&usart1);
	
	usart2.USART_Handler.Instance = USART2;
	usart2.USART_Handler.Init.BaudRate = 115200;
	usart2.USART_Handler.Init.Parity = UART_PARITY_NONE;
	usart2.func = usart2_call_back;
	//com_init(&usart2);
	
	usart3.USART_Handler.Instance = USART3;
	usart3.USART_Handler.Init.BaudRate = 115200;
	usart3.USART_Handler.Init.Parity = UART_PARITY_NONE;
	usart3.func = usart3_call_back;
	com_init(&usart3);
	
	usart4.USART_Handler.Instance = UART4;
	usart4.USART_Handler.Init.BaudRate = 115200;
	usart4.USART_Handler.Init.Parity = UART_PARITY_NONE;
	usart4.func = usart1_call_back;
	com_init(&usart4);
	
	usart5.USART_Handler.Instance = UART5;
	usart5.USART_Handler.Init.BaudRate = 115200;
	usart5.USART_Handler.Init.Parity = UART_PARITY_NONE;
	usart5.func = usart2_call_back;
	com_init(&usart5);
	
	usart6.USART_Handler.Instance = USART6;
	usart6.USART_Handler.Init.BaudRate = 115200;
	usart6.USART_Handler.Init.Parity = UART_PARITY_NONE;
	usart6.func = usart3_call_back;
	com_init(&usart6);
}

int usart2_call_back(void * str, u16 len, void * store, u16 count)
{
	int ret = 0;
	com_send_str(&usart1, str, len);
	return ret;
}

int usart3_call_back(void * str, u16 len, void * store, u16 count)
{
	int ret = 0;
	com_send_str(&usart1, str, len);
	return ret;
}

int usart1_call_back(void * str, u16 len, void * store, u16 count)
{
	int ret = 0;
	com_send_str(&usart1, str, len);
	return ret;
}

int usart4_call_back(void * str, u16 len, void * store, u16 count)
{
	int ret = 0;
	com_send_str(&usart1, str, len);
	return ret;
}

int usart5_call_back(void * str, u16 len, void * store, u16 count)
{
	int ret = 0;
	com_send_str(&usart1, str, len);
	return ret;
}

int usart6_call_back(void * str, u16 len, void * store, u16 count)
{
	int ret = 0;
	com_send_str(&usart1, str, len);
	return ret;
}
