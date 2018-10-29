#include "udp_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F4&F7������
//UDP ���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/2/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   
 
//UDP�������ݻ�����
u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP�������ݻ����� 
//UDP������������
const u8 *tcp_demo_sendbuf="Apollo STM32F4/F7 UDP demo send data\r\n";

//UDP ����ȫ��״̬��Ǳ���
//bit7:û���õ�
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û��������;1,��������.
//bit4~0:����
u8 udp_demo_flag;

//����Զ��IP��ַ
void udp_demo_set_remoteip(void)
{
	u8 *tbuf;
	u16 xoff;
	u8 key;
	LCD_Clear(WHITE);
	POINT_COLOR=RED;
	tbuf=mymalloc(SRAMIN,100);	//�����ڴ�
	if(tbuf==NULL)return;
	//ǰ����IP���ֺ�DHCP�õ���IPһ��
	lwipdev.remoteip[0]=lwipdev.ip[0];
	lwipdev.remoteip[1]=lwipdev.ip[1];
	lwipdev.remoteip[2]=lwipdev.ip[2]; 
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2]);//Զ��IP
	LCD_ShowString(30,150,210,16,16,tbuf); 
	POINT_COLOR=BLUE;
	xoff=strlen((char*)tbuf)*8+30;
	LCD_ShowxNum(xoff,150,lwipdev.remoteip[3],3,16,0); 
	while(1)
	{
		key=KEY_Scan(0);
		if(key==WKUP_PRES)break;
		else if(key)
		{
			if(key==KEY0_PRES)lwipdev.remoteip[3]++;//IP����
			if(key==KEY2_PRES)lwipdev.remoteip[3]--;//IP����
			LCD_ShowxNum(xoff,150,lwipdev.remoteip[3],3,16,0X80);//��ʾ��IP
		}
	}
	myfree(SRAMIN,tbuf); 
} 

//UDP����
void udp_demo_test(void)
{
 	err_t err;
	struct udp_pcb *udppcb;  	//����һ��TCP���������ƿ�
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ
 	
	u8 *tbuf;
 	u8 key;
	u8 res=0;		
	u16 t=0; 
 	
	udp_demo_set_remoteip();//��ѡ��IP
	LCD_Clear(WHITE);	//����
	POINT_COLOR=RED; 	//��ɫ����
	tbuf=mymalloc(SRAMIN,200);	//�����ڴ�
	if(tbuf==NULL)return ;		//�ڴ�����ʧ����,ֱ���˳�
	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//������IP
	LCD_ShowString(30,150,210,16,16,tbuf);  
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//Զ��IP
	LCD_ShowString(30,170,210,16,16,tbuf);  
	sprintf((char*)tbuf,"Remote Port:%d",UDP_DEMO_PORT);//�ͻ��˶˿ں�
	LCD_ShowString(30,190,210,16,16,tbuf);
	POINT_COLOR=BLUE;
	LCD_ShowString(30,210,210,16,16,"STATUS:Disconnected"); 
	udppcb=udp_new();
	if(udppcb)//�����ɹ�
	{ 
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
		err=udp_connect(udppcb,&rmtipaddr,UDP_DEMO_PORT);//UDP�ͻ������ӵ�ָ��IP��ַ�Ͷ˿ںŵķ�����
		if(err==ERR_OK)
		{
			err=udp_bind(udppcb,IP_ADDR_ANY,UDP_DEMO_PORT);//�󶨱���IP��ַ��˿ں�
			if(err==ERR_OK)	//�����
			{
				udp_recv(udppcb,udp_demo_recv,NULL);//ע����ջص����� 
				LCD_ShowString(30,210,210,16,16,"STATUS:Connected   ");//�����������(UDP�Ƿǿɿ�����,���������ʾ����UDP�Ѿ�׼����)
				udp_demo_flag |= 1<<5;			//����Ѿ�������
				POINT_COLOR=RED;
				LCD_ShowString(30,230,lcddev.width-30,lcddev.height-190,16,"Receive Data:");//��ʾ��Ϣ		
				POINT_COLOR=BLUE;//��ɫ����
			}else res=1;
		}else res=1;		
	}else res=1;
	while(res==0)
	{
		key=KEY_Scan(0);
		if(key==WKUP_PRES)break;
		if(key==KEY0_PRES)//KEY0������,��������
		{
			udp_demo_senddata(udppcb);
		}
		if(udp_demo_flag&1<<6)//�Ƿ��յ�����?
		{
			LCD_Fill(30,250,lcddev.width-1,lcddev.height-1,WHITE);//����һ������
			LCD_ShowString(30,250,lcddev.width-30,lcddev.height-230,16,udp_demo_recvbuf);//��ʾ���յ�������			
			udp_demo_flag&=~(1<<6);//��������Ѿ���������.
		} 
		lwip_periodic_handle();
		delay_us(100);
		t++;
		if(t==2000)
		{
			t=0;
			LED0!=LED0;
		}
	}
	udp_demo_connection_close(udppcb); 
	myfree(SRAMIN,tbuf);
} 

//UDP�ص�����
void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	u32 data_len = 0;
	struct pbuf *q;
	if(p!=NULL)	//���յ���Ϊ�յ�����ʱ
	{
		memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //���ݽ��ջ���������
		for(q=p;q!=NULL;q=q->next)  //����������pbuf����
		{
			//�ж�Ҫ������UDP_DEMO_RX_BUFSIZE�е������Ƿ����UDP_DEMO_RX_BUFSIZE��ʣ��ռ䣬�������
			//�Ļ���ֻ����UDP_DEMO_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//��������
			else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
		}
		upcb->remote_ip=*addr; 				//��¼Զ��������IP��ַ
		upcb->remote_port=port;  			//��¼Զ�������Ķ˿ں�
		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1 
		udp_demo_flag|=1<<6;	//��ǽ��յ�������
		pbuf_free(p);//�ͷ��ڴ�
	}else
	{
		udp_disconnect(upcb); 
		LCD_Clear(WHITE);			//����
		POINT_COLOR = RED;
		LCD_ShowString(30,30,200,16,16,"Apollo STM32F4&F7");
		LCD_ShowString(30,50,200,16,16,"UDP Test");
		LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");
		
		POINT_COLOR=BLUE;
		LCD_ShowString(30,90,200,16,16,"Connect break��");  
		LCD_ShowString(30,110,200,16,16,"KEY1:Connect");
		udp_demo_flag &= ~(1<<5);	//������ӶϿ�
	} 
} 
//UDP��������������
void udp_demo_senddata(struct udp_pcb *upcb)
{
	struct pbuf *ptr;
	ptr=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)tcp_demo_sendbuf),PBUF_POOL); //�����ڴ�
	if(ptr)
	{
		pbuf_take(ptr,(char*)tcp_demo_sendbuf,strlen((char*)tcp_demo_sendbuf)); //��tcp_demo_sendbuf�е����ݴ����pbuf�ṹ��
		udp_send(upcb,ptr);	//udp�������� 
		pbuf_free(ptr);//�ͷ��ڴ�
	} 
} 
//�ر�UDP����
void udp_demo_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);			//�Ͽ�UDP���� 
	udp_demo_flag &= ~(1<<5);	//������ӶϿ�
	LCD_Clear(WHITE);			//����
	POINT_COLOR = RED;
	LCD_ShowString(30,30,200,16,16,"Apollo STM32F4&F7");
	LCD_ShowString(30,50,200,16,16,"UDP Test");
	LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");
	
	POINT_COLOR=BLUE;
	LCD_ShowString(30,90,200,16,16,"Connect break��");  
	LCD_ShowString(30,110,200,16,16,"KEY1:Connect");
}

























