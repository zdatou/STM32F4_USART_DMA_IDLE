
#ifndef __sntp_client_H__
#define __sntp_client_H__

#include "sntp.h"
#include "common.h"


typedef void (*spstr)(void);

typedef struct {
	
 time_fp orig;//客户端发送时间
 time_fp recv;//服务器接收时间
 time_fp xmt;//服务器发送时间
 time_fp rev ;//客户端接受时间
}sntp_timestamp;

typedef struct __stnp_timer__{

 u32 	 		interval ;  //执行间隔
 vu32  		num		   ;  //执行次数
 u32	 		elapse   ;  //已过时间
 u8   		sw			 ; //开关  0 是不执行 1是执行 2 是一直执行
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


