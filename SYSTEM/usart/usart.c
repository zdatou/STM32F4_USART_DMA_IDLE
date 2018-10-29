#include "usart.h"
#include "stdlib.h"
#include "string.h"
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (uint8_t) ch;      
	return ch;
}
#endif 

uint8_t Timeout = 10;

STUsart usart2, usart3, usart1, usart4, usart5, usart6, usart7, usart8;

//��ʼ������
void com_init(PSTUsart com)
{
	com->USART_Handler.Init.HwFlowCtl     		   = UART_HWCONTROL_NONE;
	com->USART_Handler.Init.OverSampling  		   = UART_OVERSAMPLING_16;
	com->USART_Handler.Init.StopBits         	   = UART_STOPBITS_1;
	com->USART_Handler.Init.WordLength     		   = UART_WORDLENGTH_8B;	
	com->USART_Handler.Init.Mode         		   = UART_MODE_TX_RX;
	HAL_UART_Init(&com->USART_Handler);
	com->buff = com->buff1;
#if USART_DMA_ENABLE
	com->DMA_buff = com->buff2;	
	__HAL_UART_ENABLE_IT(&com->USART_Handler, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&com->USART_Handler, com->DMA_buff, USART_RECEIVE_MAX);
#else
	HAL_UART_Receive_IT(&com->USART_Handler, com->temp_buff, 1);
#endif
}

//�����������ݴ�����
uint8_t receive_str_manage(PSTUsart com, void *store, uint8_t count)
{
	uint8_t tmp = 0;
#if USART_DMA_ENABLE
	if(com->receive_count != 0)
	{
		com->USART_State |= 0x1; //������ڴ���
		tmp = com->func(com->buff, com->receive_count, store, count);
		com->receive_count = 0;
		com->USART_State &= ~0x1; 
		com->USART_State |= 0x2; //����Ѵ���
	}
#else	
	static uint32_t usart1_tmp = 0;
	static uint32_t usart2_tmp = 0;
	static uint32_t usart3_tmp = 0;
	static uint32_t usart4_tmp = 0;
	static uint32_t usart5_tmp = 0;
	static uint32_t usart6_tmp = 0;
	
	static uint8_t usart1_state = 0;
	static uint8_t usart2_state = 0;
	static uint8_t usart3_state = 0;
	static uint8_t usart4_state = 0;
	static uint8_t usart5_state = 0;
	static uint8_t usart6_state = 0;
	
	if(com->USART_Handler.Instance == USART1)
	{
		if(com->receive_count != usart1_tmp)
		{	
			usart1_tmp = com->receive_count;
			usart1_state++;
			return tmp;
		}
		if(usart1_state > 0)
		{
			usart1_tmp = 0;
			usart1_state = 0;
			
			tmp = com->func(com->buff, com->receive_count, store, count);	
			memset(com->buff, 0, USART_RECEIVE_MAX);
			com->receive_count = 0;			
		}
	}
	else if(com->USART_Handler.Instance == USART2)
	{
		if(com->receive_count != usart2_tmp)
		{	
			usart2_tmp = com->receive_count;
			usart2_state++;
			return tmp;
		}
		if(usart2_state > 0)
		{	
			usart2_tmp = 0;
			usart2_state = 0;
		
			tmp = com->func(com->buff, com->receive_count, store, count);	
			memset(com->buff, 0, USART_RECEIVE_MAX);
			com->receive_count = 0;			
		}
	}
	else if(com->USART_Handler.Instance == USART3)
	{
		if(com->receive_count != usart3_tmp)
		{	
			usart3_tmp = com->receive_count;
			usart3_state++;
			return tmp;
		}
		if(usart3_state > 0)
		{		
			usart3_tmp = 0;
			usart3_state = 0;
			
			tmp = com->func(com->buff, com->receive_count, store, count);		
			memset(com->buff, 0, USART_RECEIVE_MAX);
			com->receive_count = 0;			
		}
	}
	else if(com->USART_Handler.Instance == UART4)
	{
		if(com->receive_count != usart4_tmp)
		{	
			usart4_tmp = com->receive_count;
			usart4_state++;
			return tmp;
		}
		if(usart4_state > 0)
		{		
			usart4_tmp = 0;
			usart4_state = 0;
			
			tmp = com->func(com->buff, com->receive_count, store, count);		
			memset(com->buff, 0, USART_RECEIVE_MAX);
			com->receive_count = 0;			
		}
	}
	else if(com->USART_Handler.Instance == UART5)
	{
		if(com->receive_count != usart5_tmp)
		{	
			usart5_tmp = com->receive_count;
			usart5_state++;
			return tmp;
		}
		if(usart5_state > 0)
		{		
			usart5_tmp = 0;
			usart5_state = 0;
			
			tmp = com->func(com->buff, com->receive_count, store, count);		
			memset(com->buff, 0, USART_RECEIVE_MAX);
			com->receive_count = 0;			
		}
	}
	else if(com->USART_Handler.Instance == USART6)
	{
		if(com->receive_count != usart6_tmp)
		{	
			usart6_tmp = com->receive_count;
			usart6_state++;
			return tmp;
		}
		if(usart6_state > 0)
		{		
			usart6_tmp = 0;
			usart6_state = 0;
			
			tmp = com->func(com->buff, com->receive_count, store, count);		
			memset(com->buff, 0, USART_RECEIVE_MAX);
			com->receive_count = 0;			
		}
	}
#endif
	return tmp;		
}

//�����ַ�����
HAL_StatusTypeDef com_send_str(PSTUsart com,void* str,uint16_t len)
{	
#if USART_DMA_ENABLE
	uint8_t Timeout = 10;
	uint32_t Tickstart = HAL_GetTick();
	while(HAL_UART_GetState(&com->USART_Handler) == HAL_UART_STATE_BUSY_TX)
	{
      if(((HAL_GetTick()-Tickstart) > Timeout))
      {
        return HAL_TIMEOUT;
      }	
	}	
	return HAL_UART_Transmit_DMA(&com->USART_Handler, (uint8_t *)(str), len);
#else
	return HAL_UART_Transmit(&com->USART_Handler, (uint8_t *)str, len, 1000);
#endif	
}

//����һ���ַ�
HAL_StatusTypeDef com_send_byte(PSTUsart com,uint8_t byte)
{
#if USART_DMA_ENABLE	
	uint8_t Timeout = 10;
	uint32_t Tickstart = HAL_GetTick();
	while(HAL_UART_GetState(&com->USART_Handler) == HAL_UART_STATE_BUSY_TX)
	{
      if(((HAL_GetTick()-Tickstart) > Timeout))
      {
        return HAL_TIMEOUT;
      }	
	}
#endif
	while((com->USART_Handler.Instance->SR & UART_FLAG_TXE) == RESET)
	{
		;
	}
	com->USART_Handler.Instance->DR = byte;
	return HAL_OK;
}

HAL_StatusTypeDef com_send_pstr(PSTUsart com,void *p) //�ú�����ʹ��DMA���з���
{	
#if USART_DMA_ENABLE
	uint32_t Tickstart = HAL_GetTick();
	while(HAL_UART_GetState(&com->USART_Handler) == HAL_UART_STATE_BUSY_TX)
	{
      if(((HAL_GetTick()-Tickstart) > Timeout))
      {
        return HAL_TIMEOUT;
      }	
	}
#endif
	char * str = (char *)p;
	while('\0' != *str)
	{
		while((com->USART_Handler.Instance->SR & UART_FLAG_TXE) == RESET)
		{
			;
		}
		com->USART_Handler.Instance->DR = *str++;
	}
	
	while((com->USART_Handler.Instance->SR & UART_FLAG_TXE) == RESET)
	{
		;
	}
	return HAL_OK;
}

/***********************�жϻص�����************************************************/
/***********************�жϻص�����************************************************/
/***********************�жϻص�����************************************************/
/***********************�жϻص�����************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) //������ɻص�����  ��ʱδ����Ƶ�����͵����  ������Ƶ�ʽϸ�ʱ ˫������������ ��������������
{
#if USART_DMA_ENABLE
	if(USART1 == huart->Instance)
	{
		uint8_t *temp;
		usart1.receive_count = USART_RECEIVE_MAX; //�����ѽ��յ��ֽ���  ������DMA������˺���ʱ ˵��DMA������ɣ� ���յ��ֽ���Ϊ֮ǰ���õ��ֽ���

		temp            = usart1.DMA_buff;   //��������  �����ڴ濽��
		usart1.DMA_buff = usart1.buff;
		usart1.buff     = temp;

		HAL_UART_Receive_DMA(&usart1.USART_Handler, (uint8_t *)usart1.DMA_buff, USART_RECEIVE_MAX); //��������DMA����
	}
	else if(USART2 == huart->Instance)
	{
		uint8_t *temp;
		usart2.receive_count = USART_RECEIVE_MAX; //�����ѽ��յ��ֽ���
		
		temp            = usart2.DMA_buff;   //��������  �����ڴ濽��
		usart2.DMA_buff = usart2.buff;
		usart2.buff     = temp;

		HAL_UART_Receive_DMA(&usart2.USART_Handler, (uint8_t *)usart2.DMA_buff, USART_RECEIVE_MAX); //��������DMA����
	}
	else if(USART3 == huart->Instance)
	{
		uint8_t *temp;
		usart3.receive_count = USART_RECEIVE_MAX; //�����ѽ��յ��ֽ���
		
		temp            = usart3.DMA_buff;   //��������  �����ڴ濽��
		usart3.DMA_buff = usart3.buff;
		usart3.buff     = temp;

		HAL_UART_Receive_DMA(&usart3.USART_Handler, (uint8_t *)usart3.DMA_buff, USART_RECEIVE_MAX); //��������DMA����
	}
	else if(UART4 == huart->Instance)
	{
		uint8_t *temp;
		usart4.receive_count = USART_RECEIVE_MAX; //�����ѽ��յ��ֽ���
		
		temp 			= usart4.DMA_buff; //��������  �����ڴ濽��
		usart4.DMA_buff = usart4.buff;
		usart4.buff     = temp;
		
		HAL_UART_Receive_DMA(&usart4.USART_Handler, (uint8_t *)usart4.DMA_buff, USART_RECEIVE_MAX); //��������DMA����
	}
	else if(UART5 == huart->Instance)
	{
		uint8_t *temp;
		usart5.receive_count = USART_RECEIVE_MAX;
		
		temp 			= usart5.DMA_buff;
		usart5.DMA_buff = usart5.buff;
		usart5.buff		= temp;
		
		HAL_UART_Receive_DMA(&usart5.USART_Handler, (uint8_t *)usart5.DMA_buff, USART_RECEIVE_MAX);
	}
	else if(USART6 == huart->Instance)
	{
		uint8_t *temp;
		usart6.receive_count = USART_RECEIVE_MAX;
		
		temp 			= usart6.DMA_buff;
		usart6.DMA_buff = usart6.buff;
		usart6.buff		= temp;
		
		HAL_UART_Receive_DMA(&usart6.USART_Handler, (uint8_t *)usart6.DMA_buff, USART_RECEIVE_MAX);
	}
#else
	if(USART1 == huart->Instance)
	{
		(usart1.buff)[usart1.receive_count++] = usart1.temp_buff[0];
		usart1.receive_count %= USART_RECEIVE_MAX;
	}
	else if(USART2 == huart->Instance)
	{
		(usart2.buff)[usart2.receive_count++] = usart2.temp_buff[0];
		usart2.receive_count %= USART_RECEIVE_MAX;
	}
	else if(USART3 == huart->Instance)
	{
		(usart3.buff)[usart3.receive_count++] = usart3.temp_buff[0];
		usart3.receive_count %= USART_RECEIVE_MAX;
	}
	else if(UART4 == huart->Instance)
	{
		(usart4.buff)[usart4.receive_count++] = usart4.temp_buff[0];
		usart4.receive_count %= USART_RECEIVE_MAX;
	}
	else if(UART5 == huart->Instance)
	{
		(usart5.buff)[usart5.receive_count++] = usart5.temp_buff[0];
		usart5.receive_count %= USART_RECEIVE_MAX;
	}
	else if(USART6 == huart->Instance)
	{
		(usart6.buff)[usart6.receive_count++] = usart6.temp_buff[0];
		usart6.receive_count %= USART_RECEIVE_MAX;
	}
#endif
}

/******************�жϳ���*******************************/
/******************�жϳ���*******************************/
/******************�жϳ���*******************************/
/******************�жϳ���*******************************/
/******************�жϳ���*******************************/
/****************�����жϷ�����***********************/
void USART1_IRQHandler(void)                	//����1�жϷ������
{
	HAL_UART_IRQHandler(&usart1.USART_Handler);	
#if USART_DMA_ENABLE 
	uint8_t *temp;
	if((USART1->SR & USART_SR_IDLE) != RESET)                                                                  	
	{                                                                                                            	
		__HAL_UART_CLEAR_IDLEFLAG(&usart1.USART_Handler);       //��������ж�                                                              		
		usart1.receive_count = USART_RECEIVE_MAX - usart1.DMA_RX_Handler.Instance->NDTR;   //�����ѽ��յ��ֽ���                     	
		HAL_UART_DMAStop(&usart1.USART_Handler);             //��ֹDMA����                                                  	
		                                                                                                         	
		temp = usart1.DMA_buff;                          //��������  �����ڴ濽��                                                         	
		usart1.DMA_buff = usart1.buff;                                                                           	
		usart1.buff = temp;                                                                                      	
		                                                                                                         	
		HAL_UART_Receive_DMA(&usart1.USART_Handler, (uint8_t *)usart1.DMA_buff, USART_RECEIVE_MAX);   //��������DMA����           	
	}                                                                                                            	
#else                                                                                                            	
	HAL_UART_Receive_IT(&usart1.USART_Handler, usart1.temp_buff, 1);
#endif
} 

void USART2_IRQHandler(void)                	//����2�жϷ������
{
	HAL_UART_IRQHandler(&usart2.USART_Handler);
#if USART_DMA_ENABLE
	uint8_t *temp;
	if((USART2->SR & USART_SR_IDLE) != RESET)
	{
		__HAL_UART_CLEAR_IDLEFLAG(&usart2.USART_Handler);       //��������ж�        
		usart2.receive_count    = USART_RECEIVE_MAX - usart2.DMA_RX_Handler.Instance->NDTR; //�����ѽ��յ��ֽ���  
		HAL_UART_DMAStop(&usart2.USART_Handler);      //��ֹDMA����  
		
		temp            = usart2.DMA_buff;		//��������  �����ڴ濽��   
		usart2.DMA_buff = usart2.buff;
		usart2.buff     = temp;
		
		HAL_UART_Receive_DMA(&usart2.USART_Handler, (uint8_t *)usart2.DMA_buff, USART_RECEIVE_MAX);  //��������DMA����   
	}
#else
	HAL_UART_Receive_IT(&usart2.USART_Handler, (uint8_t *)&usart2.temp_buff[0], 1);
#endif
} 

void USART3_IRQHandler(void)                	//����3�жϷ������
{
	HAL_UART_IRQHandler(&usart3.USART_Handler);
#if USART_DMA_ENABLE	
	uint8_t *temp;
	if((USART3->SR & USART_SR_IDLE) != RESET) //�����ж�
	{
		__HAL_UART_CLEAR_IDLEFLAG(&usart3.USART_Handler);       //��������ж�   
		usart3.receive_count = USART_RECEIVE_MAX - usart3.DMA_RX_Handler.Instance->NDTR; //�����ѽ��յ��ֽ���
		HAL_UART_DMAStop(&usart3.USART_Handler);  //��ֹDMA����
		
		temp            = usart3.DMA_buff;   //��������  �����ڴ濽��
		usart3.DMA_buff = usart3.buff;	
		usart3.buff     = temp;
	
		HAL_UART_Receive_DMA(&usart3.USART_Handler, (uint8_t *)usart3.DMA_buff, USART_RECEIVE_MAX); //��������DMA����
	}
#else
	HAL_UART_Receive_IT(&usart3.USART_Handler, (uint8_t *)&usart3.temp_buff[0], 1); //���¿��������жϣ� ����һ���ַ�����ûص�����
#endif
} 

void UART4_IRQHandler(void) 					//����4�жϷ������
{
	HAL_UART_IRQHandler(&usart4.USART_Handler);
#if USART_DMA_ENABLE	
	uint8_t *temp;
	if((UART4->SR & USART_SR_IDLE) != RESET) //�����ж�
	{
		__HAL_UART_CLEAR_IDLEFLAG(&usart4.USART_Handler);       //��������ж�  
		usart4.receive_count = USART_RECEIVE_MAX - usart4.DMA_RX_Handler.Instance->NDTR; //�����ѽ��յ��ֽ���
		HAL_UART_DMAStop(&usart4.USART_Handler);  //��ֹDMA����
		
		temp            = usart4.DMA_buff;   //��������  �����ڴ濽��
		usart4.DMA_buff = usart4.buff;
		usart4.buff     = temp;
	
		HAL_UART_Receive_DMA(&usart4.USART_Handler, (uint8_t *)usart4.DMA_buff, USART_RECEIVE_MAX); //��������DMA����
	}
#else
	HAL_UART_Receive_IT(&usart4.USART_Handler, (uint8_t *)&usart4.temp_buff[0], 1); //���¿��������жϣ� ����һ���ַ�����ûص�����
#endif
}

void UART5_IRQHandler(void) 					//����5�жϷ������
{
	HAL_UART_IRQHandler(&usart5.USART_Handler);
#if USART_DMA_ENABLE	
	uint8_t *temp;
	if((UART5->SR & USART_SR_IDLE) != RESET) //�����ж�
	{
		__HAL_UART_CLEAR_IDLEFLAG(&usart5.USART_Handler);       //��������ж�  
		usart5.receive_count = USART_RECEIVE_MAX - usart5.DMA_RX_Handler.Instance->NDTR; //�����ѽ��յ��ֽ���
		HAL_UART_DMAStop(&usart5.USART_Handler);  //��ֹDMA����
		
		temp            = usart5.DMA_buff;   //��������  �����ڴ濽��
		usart5.DMA_buff = usart5.buff;
		usart5.buff     = temp;
	
		HAL_UART_Receive_DMA(&usart5.USART_Handler, (uint8_t *)usart5.DMA_buff, USART_RECEIVE_MAX); //��������DMA����
	}
#else
	HAL_UART_Receive_IT(&usart5.USART_Handler, (uint8_t *)&usart5.temp_buff[0], 1); //���¿��������жϣ� ����һ���ַ�����ûص�����
#endif
}

void USART6_IRQHandler(void) 					//����6�жϷ������
{
	HAL_UART_IRQHandler(&usart6.USART_Handler);
#if USART_DMA_ENABLE	
	uint8_t *temp;
	if((USART6->SR & USART_SR_IDLE) != RESET) //�����ж�
	{
		__HAL_UART_CLEAR_IDLEFLAG(&usart6.USART_Handler);       //��������ж�  
		usart6.receive_count = USART_RECEIVE_MAX - usart6.DMA_RX_Handler.Instance->NDTR; //�����ѽ��յ��ֽ���
		HAL_UART_DMAStop(&usart6.USART_Handler);  //��ֹDMA����
		
		temp            = usart6.DMA_buff;   //��������  �����ڴ濽��
		usart6.DMA_buff = usart6.buff;
		usart6.buff     = temp;
	
		HAL_UART_Receive_DMA(&usart6.USART_Handler, (uint8_t *)usart6.DMA_buff, USART_RECEIVE_MAX); //��������DMA����
	}
#else
	HAL_UART_Receive_IT(&usart6.USART_Handler, (uint8_t *)&usart6.temp_buff[0], 1); //���¿��������жϣ� ����һ���ַ�����ûص�����
#endif
}

/****************DMA�жϷ�����***********************/
#if USART_DMA_ENABLE
void DMA1_Stream3_IRQHandler(void) //USART3_Tx
{
	HAL_DMA_IRQHandler(usart3.USART_Handler.hdmatx);
}
void DMA1_Stream1_IRQHandler(void) //USART3_RX
{
	HAL_DMA_IRQHandler(usart3.USART_Handler.hdmarx);
}

void DMA2_Stream7_IRQHandler(void) //USART1_TX
{
	HAL_DMA_IRQHandler(usart1.USART_Handler.hdmatx);	
}
void DMA2_Stream5_IRQHandler(void) //USART1_RX
{
	HAL_DMA_IRQHandler(usart1.USART_Handler.hdmarx);	
}

void DMA1_Stream5_IRQHandler(void) //USART2_RX
{
	HAL_DMA_IRQHandler(usart2.USART_Handler.hdmarx);
}
void DMA1_Stream6_IRQHandler(void) //USART2_TX
{
	HAL_DMA_IRQHandler(usart2.USART_Handler.hdmatx);
}

void DMA1_Stream2_IRQHandler(void) //UART4_RX
{
	HAL_DMA_IRQHandler(usart4.USART_Handler.hdmarx);
}
void DMA1_Stream4_IRQHandler(void) //UART4_TX
{
	HAL_DMA_IRQHandler(usart4.USART_Handler.hdmatx);
}

void DMA1_Stream0_IRQHandler(void)//UART5_Rx
{
	HAL_DMA_IRQHandler(usart5.USART_Handler.hdmarx);
}

void DMA1_Stream7_IRQHandler(void) //UART5_Tx
{
	HAL_DMA_IRQHandler(usart5.USART_Handler.hdmatx);
}

void DMA2_Stream1_IRQHandler(void) //USART6_Rx
{	
	HAL_DMA_IRQHandler(usart6.USART_Handler.hdmarx);
}

void DMA2_Stream6_IRQHandler(void) //USART6_Tx
{
	HAL_DMA_IRQHandler(usart6.USART_Handler.hdmatx);
}
#endif

/*********************��ʼ���ص�����**********************************/
/*********************��ʼ���ص�����**********************************/
/*********************��ʼ���ص�����**********************************/
/*********************��ʼ���ص�����**********************************/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(huart->Instance==USART1)
	{
		__HAL_RCC_USART1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		HAL_NVIC_SetPriority(USART1_IRQn, 2,0);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
#if USART_DMA_ENABLE
		__HAL_RCC_DMA2_CLK_ENABLE(); 
		                                                                             
		usart1.DMA_TX_Handler.Instance         			= DMA2_Stream7;              //������ѡ��
		usart1.DMA_TX_Handler.Init.Channel    			= DMA_CHANNEL_4;             //ͨ��ѡ��
		usart1.DMA_TX_Handler.Init.Direction   			= DMA_MEMORY_TO_PERIPH;      //�洢��������
		usart1.DMA_TX_Handler.Init.PeriphInc   			= DMA_PINC_DISABLE;          //���������ģʽ
		usart1.DMA_TX_Handler.Init.MemInc      			= DMA_MINC_ENABLE;           //�洢������ģʽ
		usart1.DMA_TX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;       //�������ݳ���:8λ
		usart1.DMA_TX_Handler.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;       //�洢�����ݳ���:8λ
		usart1.DMA_TX_Handler.Init.Mode                 = DMA_NORMAL;                //������ͨģʽ           
		usart1.DMA_TX_Handler.Init.Priority             = DMA_PRIORITY_HIGH; 		 //���ȼ���
		usart1.DMA_TX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart1.DMA_TX_Handler.Init.FIFOThreshold        = DMA_FIFO_THRESHOLD_FULL;
		usart1.DMA_TX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;         //�洢��ͻ�����δ���
		usart1.DMA_TX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;		 //����ͻ�����δ���
		
		HAL_DMA_Init(&usart1.DMA_TX_Handler);                                                     
		__HAL_LINKDMA(huart, hdmatx, usart1.DMA_TX_Handler);                                      
		
		usart1.DMA_RX_Handler.Instance         			= DMA2_Stream5;              //������ѡ��
		usart1.DMA_RX_Handler.Init.Channel    			= DMA_CHANNEL_4;             //ͨ��ѡ��
		usart1.DMA_RX_Handler.Init.Direction   			= DMA_PERIPH_TO_MEMORY;      //�洢��������
		usart1.DMA_RX_Handler.Init.PeriphInc   			= DMA_PINC_DISABLE;          //���������ģʽ
		usart1.DMA_RX_Handler.Init.MemInc      			= DMA_MINC_ENABLE;           //�洢������ģʽ
		usart1.DMA_RX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;       //�������ݳ���:8λ
		usart1.DMA_RX_Handler.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;       //�洢�����ݳ���:8λ
		usart1.DMA_RX_Handler.Init.Mode                 = DMA_NORMAL;                //������ͨģʽ           
		usart1.DMA_RX_Handler.Init.Priority             = DMA_PRIORITY_HIGH; 		 //���ȼ���
		usart1.DMA_RX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart1.DMA_RX_Handler.Init.FIFOThreshold        = DMA_FIFO_THRESHOLD_FULL;
		usart1.DMA_RX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;         //�洢��ͻ�����δ���
		usart1.DMA_RX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;		 //����ͻ�����δ���                                         
		                                                                                             
		HAL_DMA_Init(&usart1.DMA_RX_Handler);                                                   
		__HAL_LINKDMA(huart, hdmarx, usart1.DMA_RX_Handler);                                                                                       
		                                                                                            
		HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 2, 0);    //Tx                                         
		HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);   
		
		HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 2, 0);    //Rx                                         
		HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);  
#endif
	}
	else if(huart->Instance==USART2) //
	{
		__HAL_RCC_USART2_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		
		GPIO_InitStruct.Pin = GPIO_PIN_2;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;	
		GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		GPIO_InitStruct.Pin = GPIO_PIN_3;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		HAL_NVIC_SetPriority(USART2_IRQn, 2, 1);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
#if USART_DMA_ENABLE
		__HAL_RCC_DMA1_CLK_ENABLE();
		
		usart2.DMA_TX_Handler.Instance         			= DMA1_Stream6;
		usart2.DMA_TX_Handler.Init.Channel    			= DMA_CHANNEL_4;
		usart2.DMA_TX_Handler.Init.Direction   			= DMA_MEMORY_TO_PERIPH;
		usart2.DMA_TX_Handler.Init.PeriphInc   			= DMA_PINC_DISABLE;
		usart2.DMA_TX_Handler.Init.MemInc      			= DMA_MINC_ENABLE;
		usart2.DMA_TX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;
		usart2.DMA_TX_Handler.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
		usart2.DMA_TX_Handler.Init.Mode                 = DMA_NORMAL;
		usart2.DMA_TX_Handler.Init.Priority             = DMA_PRIORITY_MEDIUM;
		usart2.DMA_TX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart2.DMA_TX_Handler.Init.FIFOThreshold        = DMA_FIFO_THRESHOLD_FULL;
		usart2.DMA_TX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;         
		usart2.DMA_TX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;
		
		HAL_DMA_Init(&usart2.DMA_TX_Handler);
		__HAL_LINKDMA(huart, hdmatx, usart2.DMA_TX_Handler);
		
		usart2.DMA_RX_Handler.Instance   				= DMA1_Stream5;
		usart2.DMA_RX_Handler.Init.Channel   			= DMA_CHANNEL_4;
		usart2.DMA_RX_Handler.Init.Direction  			= DMA_PERIPH_TO_MEMORY;
		usart2.DMA_RX_Handler.Init.PeriphInc            = DMA_PINC_DISABLE;
		usart2.DMA_RX_Handler.Init.MemInc				= DMA_MINC_ENABLE;
		usart2.DMA_RX_Handler.Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
		usart2.DMA_RX_Handler.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
		usart2.DMA_RX_Handler.Init.Mode 				= DMA_NORMAL;
		usart2.DMA_RX_Handler.Init.Priority 			= DMA_PRIORITY_MEDIUM;
		usart2.DMA_RX_Handler.Init.FIFOMode       		= DMA_FIFOMODE_DISABLE;
		usart2.DMA_RX_Handler.Init.FIFOThreshold        = DMA_FIFO_THRESHOLD_FULL;
		usart2.DMA_RX_Handler.Init.MemBurst  			= DMA_MBURST_SINGLE;
		usart2.DMA_RX_Handler.Init.PeriphBurst 			= DMA_PBURST_SINGLE;
		
		
		HAL_DMA_Init(&usart2.DMA_RX_Handler);
		__HAL_LINKDMA(huart, hdmarx, usart2.DMA_RX_Handler);
		
		HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 2, 1); //Tx
		HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
		
		HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 2, 1); //Rx
		HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);		
#endif
	}
	else if(huart->Instance== USART3)
	{
		__HAL_RCC_USART3_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_11;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		
		HAL_NVIC_SetPriority(USART3_IRQn, 2, 2);
		HAL_NVIC_EnableIRQ(USART3_IRQn);
		
#if USART_DMA_ENABLE
		__HAL_RCC_DMA1_CLK_ENABLE();
		
		usart3.DMA_TX_Handler.Instance         			= DMA1_Stream3;
		usart3.DMA_TX_Handler.Init.Channel 				= DMA_CHANNEL_4;
		usart3.DMA_TX_Handler.Init.Direction   			= DMA_MEMORY_TO_PERIPH;
		usart3.DMA_TX_Handler.Init.PeriphInc   			= DMA_PINC_DISABLE;
		usart3.DMA_TX_Handler.Init.MemInc      			= DMA_MINC_ENABLE;
		usart3.DMA_TX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;
		usart3.DMA_TX_Handler.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
		usart3.DMA_TX_Handler.Init.Mode                 = DMA_NORMAL;
		usart3.DMA_TX_Handler.Init.Priority             = DMA_PRIORITY_LOW;
		usart3.DMA_TX_Handler.Init.FIFOMode  			= DMA_FIFOMODE_DISABLE;
		usart3.DMA_TX_Handler.Init.FIFOThreshold        = DMA_FIFO_THRESHOLD_FULL;
		usart3.DMA_TX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;
		usart3.DMA_TX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;
		
		HAL_DMA_Init(&usart3.DMA_TX_Handler);
		__HAL_LINKDMA(huart, hdmatx, usart3.DMA_TX_Handler);
		
		usart3.DMA_RX_Handler.Instance   				= DMA1_Stream1;
		usart3.DMA_RX_Handler.Init.Channel				= DMA_CHANNEL_4;
		usart3.DMA_RX_Handler.Init.Direction  			= DMA_PERIPH_TO_MEMORY;
		usart3.DMA_RX_Handler.Init.PeriphInc            = DMA_PINC_DISABLE;
		usart3.DMA_RX_Handler.Init.MemInc				= DMA_MINC_ENABLE;
		usart3.DMA_RX_Handler.Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
		usart3.DMA_RX_Handler.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
		usart3.DMA_RX_Handler.Init.Mode 				= DMA_NORMAL;
		usart3.DMA_RX_Handler.Init.Priority 			= DMA_PRIORITY_MEDIUM;
		usart3.DMA_RX_Handler.Init.FIFOMode             = DMA_FIFOMODE_DISABLE;
		usart3.DMA_RX_Handler.Init.FIFOThreshold		= DMA_FIFO_THRESHOLD_FULL;
		usart3.DMA_RX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;
		usart3.DMA_RX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;
		
		HAL_DMA_Init(&usart3.DMA_RX_Handler);
		__HAL_LINKDMA(huart, hdmarx, usart3.DMA_RX_Handler);	
		
		HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 2, 2);   //Tx
		HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
		
		HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 2, 2);	//Rx
		HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
#endif		
	}
	else if(huart->Instance == UART4) //PC10 PC11
	{
		__HAL_RCC_UART4_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_11;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		
		HAL_NVIC_SetPriority(UART4_IRQn, 2, 3);
		HAL_NVIC_EnableIRQ(UART4_IRQn);
#if USART_DMA_ENABLE
		__HAL_RCC_DMA1_CLK_ENABLE();
		
		usart4.DMA_TX_Handler.Instance         			= DMA1_Stream4;
		usart4.DMA_TX_Handler.Init.Channel				= DMA_CHANNEL_4;
		usart4.DMA_TX_Handler.Init.Direction   			= DMA_MEMORY_TO_PERIPH;
		usart4.DMA_TX_Handler.Init.PeriphInc   			= DMA_PINC_DISABLE;
		usart4.DMA_TX_Handler.Init.MemInc      			= DMA_MINC_ENABLE;
		usart4.DMA_TX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;
		usart4.DMA_TX_Handler.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
		usart4.DMA_TX_Handler.Init.Mode                 = DMA_NORMAL;
		usart4.DMA_TX_Handler.Init.Priority             = DMA_PRIORITY_LOW;
		usart4.DMA_TX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart4.DMA_TX_Handler.Init.FIFOThreshold		= DMA_FIFO_THRESHOLD_FULL;
		usart4.DMA_TX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;
		usart4.DMA_TX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;
		
		HAL_DMA_Init(&usart4.DMA_TX_Handler);
		__HAL_LINKDMA(huart, hdmatx, usart4.DMA_TX_Handler);
		
		usart4.DMA_RX_Handler.Instance   				= DMA1_Stream2;
		usart4.DMA_RX_Handler.Init.Channel				= DMA_CHANNEL_4;
		usart4.DMA_RX_Handler.Init.Direction  			= DMA_PERIPH_TO_MEMORY;
		usart4.DMA_RX_Handler.Init.PeriphInc            = DMA_PINC_DISABLE;
		usart4.DMA_RX_Handler.Init.MemInc				= DMA_MINC_ENABLE;
		usart4.DMA_RX_Handler.Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
		usart4.DMA_RX_Handler.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
		usart4.DMA_RX_Handler.Init.Mode 				= DMA_NORMAL;
		usart4.DMA_RX_Handler.Init.Priority 			= DMA_PRIORITY_MEDIUM;
		usart4.DMA_RX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart4.DMA_RX_Handler.Init.FIFOThreshold		= DMA_FIFO_THRESHOLD_FULL;
		usart4.DMA_RX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;
		usart4.DMA_RX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;
		
		HAL_DMA_Init(&usart4.DMA_RX_Handler);
		__HAL_LINKDMA(huart, hdmarx, usart4.DMA_RX_Handler);	
		
		HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 2, 3); //Rx
		HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);	 
		
		HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 2, 3); //Tx
		HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);	 
#endif
	}
	else if(huart->Instance == UART5) // PD2 PC12
	{
		__HAL_RCC_UART5_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		
		GPIO_InitStruct.Pin = GPIO_PIN_2;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_12;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		
		HAL_NVIC_SetPriority(UART5_IRQn, 2, 3);
		HAL_NVIC_EnableIRQ(UART5_IRQn);
#if USART_DMA_ENABLE
		__HAL_RCC_DMA1_CLK_ENABLE();
		
		usart5.DMA_TX_Handler.Instance 					= DMA1_Stream7;
		usart5.DMA_TX_Handler.Init.Channel	 			= DMA_CHANNEL_4;
		usart5.DMA_TX_Handler.Init.Direction	        = DMA_MEMORY_TO_PERIPH; //�ڴ浽����
		usart5.DMA_TX_Handler.Init.PeriphInc    	    = DMA_PINC_DISABLE;
		usart5.DMA_TX_Handler.Init.MemInc				= DMA_MINC_ENABLE;
		usart5.DMA_TX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;
		usart5.DMA_TX_Handler.Init.MemDataAlignment		= DMA_MDATAALIGN_BYTE;
		usart5.DMA_TX_Handler.Init.Mode 				= DMA_NORMAL;
		usart5.DMA_TX_Handler.Init.Priority				= DMA_PRIORITY_LOW;
		usart5.DMA_TX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart5.DMA_TX_Handler.Init.FIFOThreshold		= DMA_FIFO_THRESHOLD_FULL;
		usart5.DMA_TX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;
		usart5.DMA_TX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;	
		
		HAL_DMA_Init(&usart5.DMA_TX_Handler);
		__HAL_LINKDMA(huart, hdmatx, usart5.DMA_TX_Handler);
		
		usart5.DMA_RX_Handler.Instance 					= DMA1_Stream0;
		usart5.DMA_RX_Handler.Init.Channel	 			= DMA_CHANNEL_4;
		usart5.DMA_RX_Handler.Init.Direction	        = DMA_PERIPH_TO_MEMORY;  //���赽�ڴ�
		usart5.DMA_RX_Handler.Init.PeriphInc    	    = DMA_PINC_DISABLE;
		usart5.DMA_RX_Handler.Init.MemInc				= DMA_MINC_ENABLE;
		usart5.DMA_RX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;
		usart5.DMA_RX_Handler.Init.MemDataAlignment		= DMA_MDATAALIGN_BYTE;
		usart5.DMA_RX_Handler.Init.Mode 				= DMA_NORMAL;
		usart5.DMA_RX_Handler.Init.Priority				= DMA_PRIORITY_LOW;
		usart5.DMA_RX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart5.DMA_RX_Handler.Init.FIFOThreshold		= DMA_FIFO_THRESHOLD_FULL;
		usart5.DMA_RX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;
		usart5.DMA_RX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;	
		
		HAL_DMA_Init(&usart5.DMA_RX_Handler);
		__HAL_LINKDMA(huart, hdmarx, usart5.DMA_RX_Handler);
		
		HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 2, 3); //Tx
		HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
		
		HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 2, 3); //Rx
		HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
#endif
	}
	else if(huart->Instance == USART6) //PC7 PC6
	{
		__HAL_RCC_USART6_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		
		GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		
		HAL_NVIC_SetPriority(USART6_IRQn, 2, 3);
		HAL_NVIC_EnableIRQ(USART6_IRQn);
#if USART_DMA_ENABLE
		__HAL_RCC_DMA2_CLK_ENABLE();
		
		usart6.DMA_TX_Handler.Instance 					= DMA2_Stream6;
		usart6.DMA_TX_Handler.Init.Channel	 			= DMA_CHANNEL_5;
		usart6.DMA_TX_Handler.Init.Direction	        = DMA_MEMORY_TO_PERIPH; //�ڴ浽����
		usart6.DMA_TX_Handler.Init.PeriphInc    	    = DMA_PINC_DISABLE;
		usart6.DMA_TX_Handler.Init.MemInc				= DMA_MINC_ENABLE;
		usart6.DMA_TX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;
		usart6.DMA_TX_Handler.Init.MemDataAlignment		= DMA_MDATAALIGN_BYTE;
		usart6.DMA_TX_Handler.Init.Mode 				= DMA_NORMAL;
		usart6.DMA_TX_Handler.Init.Priority				= DMA_PRIORITY_LOW;
		usart6.DMA_TX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart6.DMA_TX_Handler.Init.FIFOThreshold		= DMA_FIFO_THRESHOLD_FULL;
		usart6.DMA_TX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;
		usart6.DMA_TX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;	
		
		HAL_DMA_Init(&usart6.DMA_TX_Handler);
		__HAL_LINKDMA(huart, hdmatx, usart6.DMA_TX_Handler);
		
		usart6.DMA_RX_Handler.Instance 					= DMA2_Stream1;
		usart6.DMA_RX_Handler.Init.Channel	 			= DMA_CHANNEL_5;
		usart6.DMA_RX_Handler.Init.Direction	        = DMA_PERIPH_TO_MEMORY;  //���赽�ڴ�
		usart6.DMA_RX_Handler.Init.PeriphInc    	    = DMA_PINC_DISABLE;
		usart6.DMA_RX_Handler.Init.MemInc				= DMA_MINC_ENABLE;
		usart6.DMA_RX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;
		usart6.DMA_RX_Handler.Init.MemDataAlignment		= DMA_MDATAALIGN_BYTE;
		usart6.DMA_RX_Handler.Init.Mode 				= DMA_NORMAL;
		usart6.DMA_RX_Handler.Init.Priority				= DMA_PRIORITY_LOW;
		usart6.DMA_RX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart6.DMA_RX_Handler.Init.FIFOThreshold		= DMA_FIFO_THRESHOLD_FULL;
		usart6.DMA_RX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;
		usart6.DMA_RX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;	
		
		HAL_DMA_Init(&usart6.DMA_RX_Handler);
		__HAL_LINKDMA(huart, hdmarx, usart6.DMA_RX_Handler);
		
		HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 2, 3); //Tx
		HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);	
		
		HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 2, 3); //Rx
		HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);	
#endif
	}
}


