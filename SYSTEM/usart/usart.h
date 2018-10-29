#ifndef _USART_H
#define _USART_H
#include "stm32f4xx_hal.h"
#include "stdio.h"	
#include "stdint.h"

#define USART_DMA_ENABLE 1
#define USART_RECEIVE_MAX  50

typedef int (*USART_Func)(void *, uint16_t, void *, uint16_t);

typedef struct _st_usart
{
	UART_HandleTypeDef    	USART_Handler;
	uint16_t 				receive_count;
	uint8_t 				*buff;
	USART_Func 				func;
	uint8_t					buff1[USART_RECEIVE_MAX];
#if USART_DMA_ENABLE
	uint8_t					*DMA_buff;  //使用DMA时 需要使用两个缓存  一个用来保存串口接收的数据  一个用户使用
	uint8_t					buff2[USART_RECEIVE_MAX];//双缓存  避免内存拷贝
	uint16_t				_DMA_Receive_Count; //DMA接收的字节数,当未及时处理数据时,该变量的值代表未处理的字节数。该变量不对外使用！！！！
	uint16_t				_DMA_Receive_Size; //每次启动DMA时设置要接收的字节数。该变量不对外使用！！！
	uint8_t					USART_State; //串口状态  bit0-bit1 -- 0:用户缓冲区数据未处理 1:用户缓冲区数据正在处理 2:用户缓冲区数据已处理  bit2-- 1:处理过程中接收到数据且DMA缓冲区满了 
	DMA_HandleTypeDef       DMA_TX_Handler;
	DMA_HandleTypeDef       DMA_RX_Handler;
	
#else
	uint8_t 				temp_buff[8];    //不使用DMA时 使用一个较小的临时数组存放接收的一个字符或多个字符
#endif
}STUsart, *PSTUsart;

extern STUsart usart1;
extern STUsart usart2;
extern STUsart usart3;
extern STUsart usart4;
extern STUsart usart5;
extern STUsart usart6;


void com_init(PSTUsart com);
HAL_StatusTypeDef com_send_str(PSTUsart com,void* str,uint16_t len);
uint8_t receive_str_manage(PSTUsart com, void *store, uint8_t count);
HAL_StatusTypeDef com_send_byte(PSTUsart com,uint8_t byte);
HAL_StatusTypeDef com_send_pstr(PSTUsart com,void *p);

#endif
