#include "usart.h"
#include "stdlib.h"
#include "string.h"
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (uint8_t) ch;      
	return ch;
}
#endif 

uint8_t Timeout = 10;

 STUsart usart2, usart3, usart1, usart4, usart5, usart6, usart7, usart8;

//初始化串口
void com_init(PSTUsart com)
{
	com->USART_Handler.Init.HwFlowCtl     		   = UART_HWCONTROL_NONE;
	com->USART_Handler.Init.OverSampling  		   = UART_OVERSAMPLING_16;
	com->USART_Handler.Init.StopBits         	   = UART_STOPBITS_1;
	com->USART_Handler.Init.WordLength     		   = UART_WORDLENGTH_8B;	
	com->USART_Handler.Init.Mode         		   = UART_MODE_TX_RX;
	HAL_UART_Init(&com->USART_Handler);
	com->buff = com->buff1;
	com->_DMA_Receive_Size = USART_RECEIVE_MAX;
	com->USART_State |= 0x2; //标记数据已处理
#if USART_DMA_ENABLE
	com->DMA_buff = com->buff2;	
	__HAL_UART_ENABLE_IT(&com->USART_Handler, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&com->USART_Handler, com->DMA_buff, com->_DMA_Receive_Size);
#else
	HAL_UART_Receive_IT(&com->USART_Handler, com->temp_buff, 1);
#endif
}

//串口数组数据处理函数
//使用DMA时,当发送的数据过大导致一个缓冲区存放不了时, 会强制交换缓冲区, 用户缓冲区内全是有效数据,但是不完整, 尾部的一部分在DMA缓冲区中。
//需要串口下一次接收到数据才能继续发生缓存区的交换, 从而处理这部分溢出的数据。即本次发送数据太长,溢出的部分存放在另外一个缓冲区中, 当下次接收到数据时才能处理这部分数据(连同下一次的数据一起处理)。
//如果实在太大,可以加大缓冲区的大小, 或则发送方在发送完数据后再延迟一段时间(两次receive_str_manage的调用间隔)再次发送一个字节的任意数据以再次触发receive_str_manage。
//当用户未及时处理数据时 不会发生缓冲区的交换,但是当缓冲区满了后会强制发生缓冲区交换。 
//未及时处理数据会存放咋DMA缓冲区中, 再下一次调用receive_str_manage时会被处理, 如果需要连续处理 则可以将第二个if复制一份放在第一个if前面
//(也就是将if(com->receive_count != 0)这一段复制一份, 放在if(((com->USART_State)&0xA) == 0xA)这一段的前面)
uint8_t receive_str_manage(PSTUsart com, void *store, uint8_t count)
{
	uint8_t tmp = 0;
#if USART_DMA_ENABLE
	if(((com->USART_State)&0xA) == 0xA) //DMA缓冲区中存在数据 需要进行缓冲区交换 且用户缓冲区的数据已处理才能进行缓冲区交换 交换完成后 紧接着就处理用户缓冲区的数据
	{
		if((com->_DMA_Receive_Size) != (com->DMA_RX_Handler.Instance->NDTR)) //DMA正在传输数据  不进行缓冲区交换, DMA接收完后会进行一次缓冲区交换
		{
			
		}
		else
		{
			uint8_t *temp;
			uint8_t buff_shift; //数组偏移量
                                                                                                      	              	
			HAL_UART_DMAStop(&com->USART_Handler);             //终止DMA传输                                                  	
																														
			temp = com->DMA_buff;                          //交换缓存  避免内存拷贝                                                         	
			com->DMA_buff = com->buff;                                                                           	
			com->buff = temp;
			
			buff_shift = 0;  //交换缓冲区后 从缓冲区的第0个位置开始 偏移量为0
			com->_DMA_Receive_Size = USART_RECEIVE_MAX; //用户缓冲区数据处理完后, 缓冲区中无可用数据,缓冲区的全部空间都可以用来存放下一次接收的数据 接收的字节数设置成最大
			
			com->USART_State &= ~0x2; //标记数据未处理
			com->receive_count = com->_DMA_Receive_Count; //保存需要处理的字节数 
			com->_DMA_Receive_Count = 0; //重新开始进行DMA接收计数																		
			com->USART_State &= ~0x8; //清除标记
			HAL_UART_Receive_DMA(&com->USART_Handler, (uint8_t *)(com->DMA_buff+buff_shift), com->_DMA_Receive_Size);   //重新启动DMA传
		}
	}
	if(com->receive_count != 0) //用户缓冲区有数据时 开始处理用户缓冲区的数据
	{
		com->USART_State |= 0x1; //标记用户缓冲区正在处理
		tmp = com->func(com->buff, com->receive_count, store, count); //由于使用了函数参数传递缓冲区的地址和接收的字节数 所以不需要担心在处理的过程中发生缓冲区交换
		if(com->USART_State & 0x4) //处理过程中又收到新的数据且DMA缓冲区满了
		{
			com->USART_State &= ~0x4; //清除标记  不标记数据处理已完成 不清零接收字节数 在中断中接收字节数已经被赋予了新的值 并且缓冲区已经交换 用户缓冲区的数据又变成了未处理的状态
		}
		else
		{
			com->receive_count = 0;  //接收数据的字节数清零
			com->USART_State |= 0x2; //标记用户缓冲区数据已处理
		}
		com->USART_State &= ~0x1; //标记本次用户缓冲区处理已完成
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

//发送字符函数
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

//发送一个字符
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

HAL_StatusTypeDef com_send_pstr(PSTUsart com,void *p) //该函数不使用DMA进行发送
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

/***********************中断回调函数************************************************/
/***********************中断回调函数************************************************/
/***********************中断回调函数************************************************/
/***********************中断回调函数************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) //接收完成回调函数  暂时未考虑频繁发送的情况  当发送频率较高时 双缓冲会出现问题 来不及处理数据
{
#if USART_DMA_ENABLE
	if(USART1 == huart->Instance)
	{
		//DMA接收完成时,说明DMA缓存已满,不考虑用户缓冲区的数据是否已经处理完成, 强制将DMA缓冲区改成用户缓冲区, 否则没有新的空间存放接收的数据 可能会造成用户缓冲区的数据覆盖
		uint8_t *temp;
		usart1._DMA_Receive_Count += usart1._DMA_Receive_Size; //计算已接收的字节数  当开启DMA并进入此函数时 说明DMA接收完成,而非串口空闲中断,接收的字节数为DMA的累加值

		temp            = usart1.DMA_buff;   //交换缓存  避免内存拷贝
		usart1.DMA_buff = usart1.buff;
		usart1.buff     = temp;
		
		usart1.receive_count = usart1._DMA_Receive_Count; //保存接收的字节数
		usart1._DMA_Receive_Size = USART_RECEIVE_MAX; //交换缓冲区后 全部空间可用 设置成最大的可接收字节数
		usart1._DMA_Receive_Count = 0; //累加计数清零 
		
		if(usart1.USART_State & 0x1)//用户缓冲区中的数据正在处理 
		{
			usart1.USART_State |= 0x4; //标记用户缓冲区处理过程中接收到数据
		}
		usart1.USART_State &= ~0x2; //标记数据未处理
		HAL_UART_Receive_DMA(&usart1.USART_Handler, (uint8_t *)usart1.DMA_buff, usart1._DMA_Receive_Size); //重新启动DMA传输
	}
	else if(USART2 == huart->Instance)
	{
		uint8_t *temp;
		usart2._DMA_Receive_Count += usart2._DMA_Receive_Size; //计算已接收的字节数
		
		temp            = usart2.DMA_buff;   //交换缓存  避免内存拷贝
		usart2.DMA_buff = usart2.buff;
		usart2.buff     = temp;
		
		usart2.receive_count = usart2._DMA_Receive_Count; //计算接收到的字节数, 重新设置DMA接收的长度  清零累加接收长度
		usart2._DMA_Receive_Size = USART_RECEIVE_MAX;
		usart2._DMA_Receive_Count = 0;
		
		if(usart2.USART_State & 0x1) //用户缓冲区数据正在处理
		{
			usart2.USART_State |= 0x4; //标记用户缓冲区处理过程中接收到数据
		}
		usart2.USART_State &= ~0x2; //标记数据未处理
		HAL_UART_Receive_DMA(&usart2.USART_Handler, (uint8_t *)usart2.DMA_buff, usart2._DMA_Receive_Size); //重新启动DMA传输
	}
	else if(USART3 == huart->Instance)
	{
		uint8_t *temp;
		usart3._DMA_Receive_Count += usart3._DMA_Receive_Size; //计算已接收的字节数
		
		temp            = usart3.DMA_buff;   //交换缓存  避免内存拷贝
		usart3.DMA_buff = usart3.buff;
		usart3.buff     = temp;

		usart3.receive_count = usart3._DMA_Receive_Count;
		usart3._DMA_Receive_Size = USART_RECEIVE_MAX;
		usart3._DMA_Receive_Count = 0;
		
		if(usart3.USART_State & 0x1)
		{
			usart3.USART_State |= 0x4;
		}
		usart3.USART_State &= ~0x2;
		
		HAL_UART_Receive_DMA(&usart3.USART_Handler, (uint8_t *)usart3.DMA_buff, usart3._DMA_Receive_Size); //重新启动DMA传输
	}
	else if(UART4 == huart->Instance)
	{
		uint8_t *temp;
		usart4._DMA_Receive_Count += usart4._DMA_Receive_Size; //计算已接收的字节数
		
		temp 			= usart4.DMA_buff; //交换缓存  避免内存拷贝
		usart4.DMA_buff = usart4.buff;
		usart4.buff     = temp;
		
		usart4.receive_count = usart4._DMA_Receive_Count;
		usart4._DMA_Receive_Size = USART_RECEIVE_MAX;
		usart4._DMA_Receive_Count = 0;
		
		if(usart4.USART_State & 0x1)
		{
			usart4.USART_State |= 0x4;
		}
		usart4.USART_State &= ~0x2;
		
		HAL_UART_Receive_DMA(&usart4.USART_Handler, (uint8_t *)usart4.DMA_buff, usart4._DMA_Receive_Size); //重新启动DMA传输
	}
	else if(UART5 == huart->Instance)
	{
		uint8_t *temp;
		usart5._DMA_Receive_Count += usart5._DMA_Receive_Size;
		
		temp 			= usart5.DMA_buff;
		usart5.DMA_buff = usart5.buff;
		usart5.buff		= temp;
		
		usart5.receive_count = usart5._DMA_Receive_Count;
		usart5._DMA_Receive_Size = USART_RECEIVE_MAX;
		usart5._DMA_Receive_Count = 0;
		
		if(usart5.USART_State & 0x1)
		{
			usart5.USART_State |= 0x4;
		}
		usart5.USART_State &= ~0x2;
		
		HAL_UART_Receive_DMA(&usart5.USART_Handler, (uint8_t *)usart5.DMA_buff, usart5._DMA_Receive_Size);
	}
	else if(USART6 == huart->Instance)
	{
		uint8_t *temp;
		usart6._DMA_Receive_Count += usart6._DMA_Receive_Size;
		
		temp 			= usart6.DMA_buff;
		usart6.DMA_buff = usart6.buff;
		usart6.buff		= temp;
		
		usart6.receive_count = usart6._DMA_Receive_Count;
		usart6._DMA_Receive_Size = USART_RECEIVE_MAX;
		usart6._DMA_Receive_Count = 0;
		
		
		if(usart6.USART_State & 0x1)
		{
			usart6.USART_State |= 0x4;
		}
		usart6.USART_State &= ~0x2;	
		HAL_UART_Receive_DMA(&usart6.USART_Handler, (uint8_t *)usart6.DMA_buff, usart6._DMA_Receive_Size);
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

/******************中断程序*******************************/
/******************中断程序*******************************/
/******************中断程序*******************************/
/******************中断程序*******************************/
/******************中断程序*******************************/
/****************串口中断服务函数***********************/
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	HAL_UART_IRQHandler(&usart1.USART_Handler);	
#if USART_DMA_ENABLE 
	uint8_t *temp;
	uint8_t buff_shift; //数组偏移量
	if((USART1->SR & USART_SR_IDLE) != RESET)                                                                  	
	{                                                                                                            	
		__HAL_UART_CLEAR_IDLEFLAG(&usart1.USART_Handler);       //清除空闲中断   
		usart1._DMA_Receive_Count += usart1._DMA_Receive_Size - usart1.DMA_RX_Handler.Instance->NDTR;   //计算已接收的字节数, 使用累加的方式 可能上一次的数据没有处理                  	
		HAL_UART_DMAStop(&usart1.USART_Handler);             //终止DMA传输                                                  	
		                                                                                                         	
		if(usart1.USART_State&0x2) //数据已处理
		{
			temp = usart1.DMA_buff;                          //交换缓存  避免内存拷贝                                                         	
			usart1.DMA_buff = usart1.buff;                                                                           	
			usart1.buff = temp;
			buff_shift = 0;  //交换缓冲区后 从缓冲区的第0个位置开始 偏移量为0
			usart1._DMA_Receive_Size = USART_RECEIVE_MAX; //用户缓冲区数据处理完后, 缓冲区中无可用数据,缓冲区的全部空间都可以用来存放下一次接收的数据 接收的字节数设置成最大
			usart1.USART_State &= ~0x2; //标记数据未处理
			usart1.receive_count = usart1._DMA_Receive_Count; //保存需要处理的字节数 
			usart1._DMA_Receive_Count = 0; //重新开始进行DMA接收计数
			usart1.USART_State &= ~0x8; //清除标记
		}
		else //数据未处理或正在处理时,不进行缓冲区交换,新接收的数据追加在缓冲区的尾部
		{			
			buff_shift = usart1._DMA_Receive_Count;  //数据未处理完 则接着上一次接收的尾部开始存放
			usart1._DMA_Receive_Size = USART_RECEIVE_MAX - buff_shift; //计算新的最大可接收字符长度
			usart1.USART_State |= 0x08; //标记DMA缓冲区中有数据, 需要进行缓冲区的交换
		}
		                                                                                                         	
		HAL_UART_Receive_DMA(&usart1.USART_Handler, (uint8_t *)(usart1.DMA_buff+buff_shift), usart1._DMA_Receive_Size);   //重新启动DMA传输           	
	}                                                                                                            	
#else                                                                                                            	
	HAL_UART_Receive_IT(&usart1.USART_Handler, usart1.temp_buff, 1);
#endif
} 

void USART2_IRQHandler(void)                	//串口2中断服务程序
{
	HAL_UART_IRQHandler(&usart2.USART_Handler);
#if USART_DMA_ENABLE
	uint8_t *temp;
	uint8_t buff_shift;  //数组偏移量
	if((USART2->SR & USART_SR_IDLE) != RESET)
	{
		__HAL_UART_CLEAR_IDLEFLAG(&usart2.USART_Handler);       //清除空闲中断        
		usart2._DMA_Receive_Count    += usart2._DMA_Receive_Size - usart2.DMA_RX_Handler.Instance->NDTR; //计算已接收的字节数  使用累加的方式 可能上一次的数据没有处理   
		HAL_UART_DMAStop(&usart2.USART_Handler);            //终止DMA传输  
		
		if(usart2.USART_State & 0x2)					    //数据已处理
		{
			temp            		  = usart2.DMA_buff;   //交换缓存  避免内存拷贝   
			usart2.DMA_buff 		  = usart2.buff;
			usart2.buff     		  = temp;
			buff_shift 				  = 0;    		       //交换缓冲区后 从缓冲区的第0个位置开始 偏移量为0
			usart2._DMA_Receive_Size  = USART_RECEIVE_MAX; //用户缓冲区数据处理完后, 缓冲区中无可用数据,缓冲区的全部空间都可以用来存放下一次接收的数据 接收的字节数设置成最大
			usart2.USART_State 		 &= ~0x2; 			   //标记数据未处理
			usart2.receive_count	  = usart2._DMA_Receive_Count;//保存需要处理的字节数 
			usart2._DMA_Receive_Count = 0;				   //重新开始进行DMA接收计数
			usart2.USART_State 		 &= ~0x8;			   //清除标记
		}
		else //数据未处理或正在处理时,不进行缓冲区交换,新接收的数据追加在缓冲区的尾部
		{
			buff_shift 				 = usart2._DMA_Receive_Count;   //数据未处理完 则接着上一次接收的尾部开始存放
			usart2._DMA_Receive_Size = USART_RECEIVE_MAX - buff_shift; //计算新的最大可接收字符长度
			usart2.USART_State		 |= 0x8;   					  //标记DMA缓冲区中有数据, 需要进行缓冲区的交换
		}
		HAL_UART_Receive_DMA(&usart2.USART_Handler, (uint8_t *)(usart2.DMA_buff + buff_shift), usart2._DMA_Receive_Size);  //重新启动DMA传输   
	}
#else
	HAL_UART_Receive_IT(&usart2.USART_Handler, (uint8_t *)&usart2.temp_buff[0], 1);
#endif
} 

void USART3_IRQHandler(void)                	//串口3中断服务程序
{
	HAL_UART_IRQHandler(&usart3.USART_Handler);
	//注释与串口1相同
#if USART_DMA_ENABLE	
	uint8_t *temp;
	uint8_t buff_shift; //数组偏移量
	if((USART3->SR & USART_SR_IDLE) != RESET) //空闲中断
	{
		__HAL_UART_CLEAR_IDLEFLAG(&usart3.USART_Handler);       //清除空闲中断   
		usart3._DMA_Receive_Count += usart3._DMA_Receive_Size - usart3.DMA_RX_Handler.Instance->NDTR; //计算已接收的字节数 使用累加的方式 可能上一次的数据没有处理   
		HAL_UART_DMAStop(&usart3.USART_Handler);  //终止DMA传输
		
		if(usart3.USART_State & 0x2)
		{
			temp            = usart3.DMA_buff;   //交换缓存  避免内存拷贝
			usart3.DMA_buff = usart3.buff;	
			usart3.buff     = temp;
			buff_shift = 0;
			usart3._DMA_Receive_Size = USART_RECEIVE_MAX;
			usart3.USART_State &= ~0x2;
			usart3.receive_count= usart3._DMA_Receive_Count;
			usart3._DMA_Receive_Count = 0;
			usart3.USART_State &= ~0x8;
		}
		else
		{
			buff_shift = usart3._DMA_Receive_Count;
			usart3._DMA_Receive_Size = USART_RECEIVE_MAX - buff_shift;
			usart3.USART_State |= 0x08;
		}
	
		HAL_UART_Receive_DMA(&usart3.USART_Handler, (uint8_t *)(usart3.DMA_buff+buff_shift), usart3._DMA_Receive_Size); //重新启动DMA传输
	}
#else
	HAL_UART_Receive_IT(&usart3.USART_Handler, (uint8_t *)&usart3.temp_buff[0], 1); //重新开启接收中断， 接收一个字符后调用回调函数
#endif
} 

void UART4_IRQHandler(void) 					//串口4中断服务程序
{
	HAL_UART_IRQHandler(&usart4.USART_Handler);
	//注释与串口1相同
#if USART_DMA_ENABLE	
	uint8_t *temp;
	uint8_t buff_shift;
	if((UART4->SR & USART_SR_IDLE) != RESET) //空闲中断
	{
		__HAL_UART_CLEAR_IDLEFLAG(&usart4.USART_Handler);       //清除空闲中断  
		usart4._DMA_Receive_Count += usart4._DMA_Receive_Size - usart4.DMA_RX_Handler.Instance->NDTR; //计算已接收的字节数
		HAL_UART_DMAStop(&usart4.USART_Handler);  //终止DMA传输
		
		if(usart4.USART_State & 0x2)
		{
			temp            = usart4.DMA_buff;   //交换缓存  避免内存拷贝
			usart4.DMA_buff = usart4.buff;
			usart4.buff     = temp;
			buff_shift = 0;
			usart4._DMA_Receive_Size = USART_RECEIVE_MAX;
			usart4.USART_State &= ~0x2;
			usart4.receive_count= usart4._DMA_Receive_Count;
			usart4._DMA_Receive_Count = 0;
			usart4.USART_State &= ~0x8;
		}
		else
		{
			buff_shift = usart4._DMA_Receive_Count;
			usart4._DMA_Receive_Size = USART_RECEIVE_MAX - buff_shift;
			usart4.USART_State |= 0x08;
		}
	
		HAL_UART_Receive_DMA(&usart4.USART_Handler, (uint8_t *)(usart4.DMA_buff+buff_shift), usart4._DMA_Receive_Size); //重新启动DMA传输
	}
#else
	HAL_UART_Receive_IT(&usart4.USART_Handler, (uint8_t *)&usart4.temp_buff[0], 1); //重新开启接收中断， 接收一个字符后调用回调函数
#endif
}

void UART5_IRQHandler(void) 					//串口5中断服务程序
{
	HAL_UART_IRQHandler(&usart5.USART_Handler);
	//注释与串口1相同
#if USART_DMA_ENABLE	
	uint8_t *temp;
	uint8_t buff_shift;
	if((UART5->SR & USART_SR_IDLE) != RESET) //空闲中断
	{
		__HAL_UART_CLEAR_IDLEFLAG(&usart5.USART_Handler);       //清除空闲中断  
		usart5._DMA_Receive_Count += usart5._DMA_Receive_Size - usart5.DMA_RX_Handler.Instance->NDTR; //计算已接收的字节数
		HAL_UART_DMAStop(&usart5.USART_Handler);  //终止DMA传输
		
		if(usart5.USART_State & 0x2)
		{
			temp            = usart5.DMA_buff;   //交换缓存  避免内存拷贝
			usart5.DMA_buff = usart5.buff;
			usart5.buff     = temp;
			buff_shift = 0;
			usart5._DMA_Receive_Size = USART_RECEIVE_MAX;
			usart5.USART_State &= ~0x2;
			usart5.receive_count= usart5._DMA_Receive_Count;
			usart5._DMA_Receive_Count = 0;
			usart5.USART_State &= ~0x8;
		}
		else
		{
			buff_shift = usart5._DMA_Receive_Count;
			usart5._DMA_Receive_Size = USART_RECEIVE_MAX - buff_shift;
			usart5.USART_State |= 0x08;
		}
	
		HAL_UART_Receive_DMA(&usart5.USART_Handler, (uint8_t *)(usart5.DMA_buff+buff_shift), usart5._DMA_Receive_Size); //重新启动DMA传输
	}
#else
	HAL_UART_Receive_IT(&usart5.USART_Handler, (uint8_t *)&usart5.temp_buff[0], 1); //重新开启接收中断， 接收一个字符后调用回调函数
#endif
}

void USART6_IRQHandler(void) 					//串口6中断服务程序
{
	HAL_UART_IRQHandler(&usart6.USART_Handler);
	//注释与串口1相同
#if USART_DMA_ENABLE	
	uint8_t *temp;
	uint8_t buff_shift;
	if((USART6->SR & USART_SR_IDLE) != RESET) //空闲中断
	{
		__HAL_UART_CLEAR_IDLEFLAG(&usart6.USART_Handler);       //清除空闲中断  
		usart6._DMA_Receive_Count += usart6._DMA_Receive_Size - usart6.DMA_RX_Handler.Instance->NDTR; //计算已接收的字节数
		HAL_UART_DMAStop(&usart6.USART_Handler);  //终止DMA传输
		
		if(usart6.USART_State & 0x2)
		{
			temp            = usart6.DMA_buff;   //交换缓存  避免内存拷贝
			usart6.DMA_buff = usart6.buff;
			usart6.buff     = temp;
			buff_shift = 0;
			usart6._DMA_Receive_Size = USART_RECEIVE_MAX;
			usart6.USART_State &= ~0x2;
			usart6.receive_count= usart6._DMA_Receive_Count;
			usart6._DMA_Receive_Count = 0;
			usart6.USART_State &= ~0x8;
		}
		else
		{
			buff_shift = usart6._DMA_Receive_Count;
			usart6._DMA_Receive_Size = USART_RECEIVE_MAX - buff_shift;
			usart6.USART_State |= 0x08;
		}
	
		HAL_UART_Receive_DMA(&usart6.USART_Handler, (uint8_t *)(usart6.DMA_buff+buff_shift), usart6._DMA_Receive_Size); //重新启动DMA传输
	}
#else
	HAL_UART_Receive_IT(&usart6.USART_Handler, (uint8_t *)&usart6.temp_buff[0], 1); //重新开启接收中断， 接收一个字符后调用回调函数
#endif
}

/****************DMA中断服务函数***********************/
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

/*********************初始化回调函数**********************************/
/*********************初始化回调函数**********************************/
/*********************初始化回调函数**********************************/
/*********************初始化回调函数**********************************/
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
		                                                                             
		usart1.DMA_TX_Handler.Instance         			= DMA2_Stream7;              //数据流选择
		usart1.DMA_TX_Handler.Init.Channel    			= DMA_CHANNEL_4;             //通道选择
		usart1.DMA_TX_Handler.Init.Direction   			= DMA_MEMORY_TO_PERIPH;      //存储器到外设
		usart1.DMA_TX_Handler.Init.PeriphInc   			= DMA_PINC_DISABLE;          //外设非增量模式
		usart1.DMA_TX_Handler.Init.MemInc      			= DMA_MINC_ENABLE;           //存储器增量模式
		usart1.DMA_TX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;       //外设数据长度:8位
		usart1.DMA_TX_Handler.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;       //存储器数据长度:8位
		usart1.DMA_TX_Handler.Init.Mode                 = DMA_NORMAL;                //外设普通模式           
		usart1.DMA_TX_Handler.Init.Priority             = DMA_PRIORITY_HIGH; 		 //优先级高
		usart1.DMA_TX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart1.DMA_TX_Handler.Init.FIFOThreshold        = DMA_FIFO_THRESHOLD_FULL;
		usart1.DMA_TX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;         //存储器突发单次传输
		usart1.DMA_TX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;		 //外设突发单次传输
		
		HAL_DMA_Init(&usart1.DMA_TX_Handler);                                                     
		__HAL_LINKDMA(huart, hdmatx, usart1.DMA_TX_Handler);                                      
		
		usart1.DMA_RX_Handler.Instance         			= DMA2_Stream5;              //数据流选择
		usart1.DMA_RX_Handler.Init.Channel    			= DMA_CHANNEL_4;             //通道选择
		usart1.DMA_RX_Handler.Init.Direction   			= DMA_PERIPH_TO_MEMORY;      //存储器到外设
		usart1.DMA_RX_Handler.Init.PeriphInc   			= DMA_PINC_DISABLE;          //外设非增量模式
		usart1.DMA_RX_Handler.Init.MemInc      			= DMA_MINC_ENABLE;           //存储器增量模式
		usart1.DMA_RX_Handler.Init.PeriphDataAlignment	= DMA_PDATAALIGN_BYTE;       //外设数据长度:8位
		usart1.DMA_RX_Handler.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;       //存储器数据长度:8位
		usart1.DMA_RX_Handler.Init.Mode                 = DMA_NORMAL;                //外设普通模式           
		usart1.DMA_RX_Handler.Init.Priority             = DMA_PRIORITY_HIGH; 		 //优先级高
		usart1.DMA_RX_Handler.Init.FIFOMode				= DMA_FIFOMODE_DISABLE;
		usart1.DMA_RX_Handler.Init.FIFOThreshold        = DMA_FIFO_THRESHOLD_FULL;
		usart1.DMA_RX_Handler.Init.MemBurst				= DMA_MBURST_SINGLE;         //存储器突发单次传输
		usart1.DMA_RX_Handler.Init.PeriphBurst			= DMA_PBURST_SINGLE;		 //外设突发单次传输                                         
		                                                                                             
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
		usart5.DMA_TX_Handler.Init.Direction	        = DMA_MEMORY_TO_PERIPH; //内存到外设
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
		usart5.DMA_RX_Handler.Init.Direction	        = DMA_PERIPH_TO_MEMORY;  //外设到内存
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
		usart6.DMA_TX_Handler.Init.Direction	        = DMA_MEMORY_TO_PERIPH; //内存到外设
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
		usart6.DMA_RX_Handler.Init.Direction	        = DMA_PERIPH_TO_MEMORY;  //外设到内存
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
		HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);	
#endif
	}
}


