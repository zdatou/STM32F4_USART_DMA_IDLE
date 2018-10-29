#ifndef __LAN8720_H
#define __LAN8720_H
#include "sys.h"
#include "stm32f4xx_hal_conf.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//LAN8720��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/13
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
extern ETH_HandleTypeDef ETH_Handler;               //��̫�����
extern ETH_DMADescTypeDef *DMARxDscrTab;			//��̫��DMA�������������ݽṹ��ָ��
extern ETH_DMADescTypeDef *DMATxDscrTab;			//��̫��DMA�������������ݽṹ��ָ�� 
extern uint8_t *Rx_Buff; 							//��̫���ײ���������buffersָ�� 
extern uint8_t *Tx_Buff; 							//��̫���ײ���������buffersָ��
extern ETH_DMADescTypeDef  *DMATxDescToSet;			//DMA����������׷��ָ��
extern ETH_DMADescTypeDef  *DMARxDescToGet; 		//DMA����������׷��ָ�� 
 
#define LAN8720_PHY_ADDRESS  	0x00				//LAN8720 PHYоƬ��ַ.
#define LAN8720_RST 		   	PDout(3) 			//LAN8720��λ����	




u8 LAN8720_Init(void);
u32 LAN8720_ReadPHY(u16 reg);
void LAN8720_WritePHY(u16 reg,u16 value);
u8 LAN8720_Get_Speed(void);
u8 ETH_MACDMA_Config(void);
u8 ETH_Mem_Malloc(void);
void ETH_Mem_Free(void);
u32  ETH_GetRxPktSize(ETH_DMADescTypeDef *DMARxDesc);
#endif
