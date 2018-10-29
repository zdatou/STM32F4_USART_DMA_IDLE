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
    HAL_Init();                     //��ʼ��HAL��
	Stm32_Clock_Init(336,8,2,7);   //����ʱ��,168Mhz   
    delay_init(168);                //��ʼ����ʱ����
   
	usart_init(); //��ʼ������
	my_mem_init(SRAMIN);		    //��ʼ���ڲ��ڴ��
   	
	TIM_Init(6, 1000, 84); //������ѯ��ʱ�� 1ms
	TIM_Init(7, 1000, 84); //������������ʱ�䶨ʱ�� 1us
	TIM_Base_Start(6); //����������ѯ��ʱ��	
	
	while(lwip_comm_init())         //lwip��ʼ��
	{
		delay_ms(500);
        delay_ms(500);
	}
#if LWIP_DHCP   //ʹ��DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//�ȴ�DHCP��ȡ�ɹ�/��ʱ���
	{  
		lwip_periodic_handle();	//LWIP�ں���Ҫ��ʱ����ĺ���
	}
#endif
	while(1)
	{
        lwip_periodic_handle();	//LWIP�ں���Ҫ��ʱ����ĺ���
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
