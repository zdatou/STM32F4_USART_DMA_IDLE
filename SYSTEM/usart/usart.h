#ifndef _USART_H
#define _USART_H
#include "stm32f4xx_hal.h"
#include "stdio.h"	
#include "stdint.h"

#define USART_DMA_ENABLE 1
#define USART_RECEIVE_MAX  555

typedef int (*USART_Func)(void *, uint16_t, void *, uint16_t);

typedef struct _st_usart
{
	UART_HandleTypeDef    	USART_Handler;
	uint16_t 				receive_count;
	uint8_t 				*buff;
	USART_Func 				func;
	uint8_t					buff1[USART_RECEIVE_MAX];
#if USART_DMA_ENABLE
	uint8_t					*DMA_buff;  //ʹ��DMAʱ ��Ҫʹ����������  һ���������洮�ڽ��յ�����  һ���û�ʹ��
	uint8_t					buff2[USART_RECEIVE_MAX];//˫����  �����ڴ濽��
	uint16_t				_DMA_Receive_Count;
	uint8_t					USART_State; //����״̬  bit0-bit1 -- 0:����δ���� 1:�������ڴ��� 2:�����Ѵ���  bit7-bit2 ��δʹ��
	DMA_HandleTypeDef       DMA_TX_Handler;
	DMA_HandleTypeDef       DMA_RX_Handler;
	
#else
	uint8_t 				temp_buff[8];    //��ʹ��DMAʱ ʹ��һ����С����ʱ�����Ž��յ�һ���ַ������ַ�
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
