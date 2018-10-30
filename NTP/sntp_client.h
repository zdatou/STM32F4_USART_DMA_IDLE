
#ifndef __sntp_client_H__
#define __sntp_client_H__

#include "sntp.h"
#include "common.h"


typedef void (*spstr)(void);

typedef struct {
	
 time_fp orig;//�ͻ��˷���ʱ��
 time_fp recv;//����������ʱ��
 time_fp xmt;//����������ʱ��
 time_fp rev ;//�ͻ��˽���ʱ��
}sntp_timestamp;

typedef struct __stnp_timer__{

 u32 	 		interval ;  //ִ�м��
 vu32  		num		   ;  //ִ�д���
 u32	 		elapse   ;  //�ѹ�ʱ��
 u8   		sw			 ; //����  0 �ǲ�ִ�� 1��ִ�� 2 ��һֱִ��
 spstr    callfunc    ;
}sntp_timer;


typedef enum   
{
	SW_OFF,
	SW_ON	,
	SW_Forever
}SW_STATE;




 
void udp_sntp_client_connect(void);
void udp_sntp_client_init(struct ip_addr p);
void udp_sntp_client_reinit(struct ip_addr p);
void sntp_timer_connect(u32 time , sntp_timer *p);
#endif 


