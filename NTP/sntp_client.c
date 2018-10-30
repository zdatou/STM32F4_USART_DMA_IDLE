#include "Allheader.h"
#include "sntp.h"

#define UDP_Des_PORT    123   /* define the UDP remote connection port */
#define UDP_BIND_PORT      9923

void sntp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
void sntp_client_time_handle(time_fp rev_time,struct sntp_s *sntp_msg , time_fp *delay , time_fp *offset);
void udp_sntp_client_control_send(time_fp delay , time_fp offset);
void NTPupdateclock(double NanoSec);
#define UDP_SNTP_BUFSIZE		2048	//定义udp最大接收数据长度 
u8 udp_sntp_recvbuf[UDP_SNTP_BUFSIZE];	//UDP接收数据缓冲区

time_fp recentlyoffset = {0};

struct udp_pcb *clientupcb; 
extern PtpClock ptpClock;
extern ts_NTPRunTimeOpts   TS_NRTP;
//sntp_timer stmr ;//sntp 定时器结构体
extern ts_FLASH_DATA_SAVE  TS_FDS;
void udp_sntp_client_init(struct ip_addr p)
{
	err_t err = 0;
	
 
	clientupcb = udp_new();
	if( clientupcb !=  NULL )
	{
    /* configure destination IP address and port */
     err= udp_connect(clientupcb, &p, UDP_Des_PORT);
      
      if(err == ERR_OK)
      {
        /* Set a receive callback for the upcb */
        udp_recv(clientupcb, sntp_receive_callback, NULL);
			}
				else
			{
					/* free the UDP connection, so we can accept new clients */
					udp_remove(clientupcb);
			}
  }
	else
	{
		udp_remove(clientupcb);
	}
}


void udp_sntp_client_reinit(struct ip_addr p)
{
	err_t err = 0;
	
	if(clientupcb !=  NULL  )
	{
  	udp_remove(clientupcb);
	}
	clientupcb = udp_new();
	if( clientupcb !=  NULL )
	{
    /* configure destination IP address and port */
     err= udp_connect(clientupcb, &p, UDP_Des_PORT);
      
      if(err == ERR_OK)
      {
        /* Set a receive callback for the upcb */
        udp_recv(clientupcb, sntp_receive_callback, NULL);
			}
				else
			{
					/* free the UDP connection, so we can accept new clients */
					udp_remove(clientupcb);
			}
  }
	else
	{
		udp_remove(clientupcb);
	}
}


void udp_sntp_client_connect(void)
{
	struct pbuf *ptr;
	struct sntp_s sntp_msg;
	unsigned char mode = MODE_CLIENT;
  
	ptr = pbuf_alloc(PBUF_TRANSPORT,sizeof(sntp_msg), PBUF_POOL);  
	if (ptr != NULL)
	{
		memset(&sntp_msg, 0, sizeof(sntp_msg));
		sntp_msg.li_vn_md = MSG_LI_VN_MODE(LEAP_NO_WARNING, SNTP_VERSION, mode);
		sntp_msg.stratum = SNTP_STRATUM;
		get_sys_timer(&sntp_msg.xmt);//获取系统时间
		HTONL_FP(&sntp_msg.xmt, &sntp_msg.xmt); //转换网络格式	
		/* copy data to pbuf */
		pbuf_take(ptr, &sntp_msg , sizeof(sntp_msg));
		/* send udp data */
		udp_send(clientupcb, ptr); 
		/* free pbuf */
		pbuf_free(ptr);
	}
} 


double TimeDiff = 0;
void sntp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	  struct sntp_s sntp_msg;

	  unsigned char mode = MODE_SERVER;
	  struct pbuf *q;
		u32 data_len = 0;
	  time_fp rev_time;
	 static  time_fp delay_t;
	 static  time_fp offset_t;
     
      if (p != NULL)
     {
						for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
					{
						//判断要拷贝到UDP_DEMO_RX_BUFSIZE中的数据是否大于UDP_DEMO_RX_BUFSIZE的剩余空间，如果大于
						//的话就只拷贝UDP_DEMO_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
						if(q->len > (UDP_SNTP_BUFSIZE-data_len)) memcpy(udp_sntp_recvbuf+data_len,q->payload,(UDP_SNTP_BUFSIZE-data_len));//拷贝数据
						else memcpy(udp_sntp_recvbuf+data_len,q->payload,q->len);
						data_len += q->len;  	
						if(data_len > UDP_SNTP_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
					}
				memcpy((char*)&sntp_msg,udp_sntp_recvbuf,sizeof(sntp_msg));
				if ((data_len < sizeof(sntp_msg)) || ((MSG_MODE(sntp_msg.li_vn_md) != mode) && (mode != MODE_BROADCAST)))
				{
					   pbuf_free(p);
						return ;
				}
				else
				{
					memcpy((char*)&sntp_msg,udp_sntp_recvbuf,sizeof(sntp_msg));
					get_sys_timer(&rev_time);
					/* Tell the client that we have accepted it */
					sntp_client_time_handle(rev_time,&sntp_msg,&delay_t,&offset_t);
					udp_sntp_client_control_send(delay_t,offset_t);//控制报文
			    TS_FPTOD(&offset_t,TimeDiff);
					if( TS_FDS.PTP_MODE == emNTPSlave )
					{
						TimeDiff = TimeDiff*1000000000;
						NTPupdateclock(TimeDiff);	
					}
				}					
     }
		  /* Free the p buffer */
    pbuf_free(p);
}

void sntp_client_time_handle(time_fp rev_time,struct sntp_s *sntp_msg , time_fp *delay , time_fp *offset)
{
  
    time_fp t2_1, t3_4; //cal offset
	  time_fp t4_1, t3_2;
    time_fp offset_t, orig_t, recv_t, xmt_t,delay_t;
	
    NTOHL_FP(&sntp_msg->orig, &orig_t);
    NTOHL_FP(&sntp_msg->recv, &recv_t);
    NTOHL_FP(&sntp_msg->xmt, &xmt_t);
	
   if (orig_t.fp_ui == 0 && orig_t.fp_uf == 0)
    {
        offset_t = xmt_t;
        TS_SUB(&offset_t, &rev_time);
    }
    else
    {
        /* 计算时钟偏差 ((T2 - T1) + (T3 - T4)) / 2 */
        t3_4 = xmt_t;  //t3
        TS_SUB(&t3_4, &rev_time); /* T3 - T4 */
        t2_1 = recv_t;  //t2
        TS_SUB(&t2_1, &orig_t);
         
        offset_t = t3_4;
        TS_RSHIFT(&offset_t); // offset_t / 2
        TS_RSHIFT(&t2_1);    // t2_1 / 2
        TS_ADD(&offset_t, &t2_1); // offset + t2_1
			  *offset = offset_t;
			  
			  /* cal delay */
			  t4_1 = rev_time;         // t4
			  TS_SUB(&t4_1, &orig_t);  /* T4 - T1 */ 
	      t3_2 = xmt_t;            //t3 
        TS_SUB(&t3_2, &recv_t);	 /* T3 - T4 */ 	   
			  delay_t = t4_1;
			  TS_SUB(&delay_t, &t3_2);
		  	*delay = delay_t;
    }
}

void NTPupdateclock(double NanoSec)
{
	static  TimeInternal nTime_t;
  Integer64 TmpNano;
 
	TmpNano = -1 * NanoSec ;
 
  nTime_t.seconds = TmpNano / 1000000000;
	nTime_t.nanoseconds = TmpNano % 1000000000;
 
  ptpClock.currentDS.offsetFromMaster = nTime_t;
  updateClock(&ptpClock);
}
	

void udp_sntp_client_control_send(time_fp delay , time_fp offset)
{
	struct pbuf *ptr;
	struct sntp_s sntp_msg;
	unsigned char mode = MODE_CONTROL;
  
	ptr = pbuf_alloc(PBUF_TRANSPORT,sizeof(sntp_msg), PBUF_POOL);  
	if (ptr != NULL)
	{
		memset(&sntp_msg, 0, sizeof(sntp_msg));
		sntp_msg.li_vn_md = MSG_LI_VN_MODE(LEAP_NO_WARNING, SNTP_VERSION, mode);
		sntp_msg.stratum = SNTP_STRATUM;
  
		HTONL_FP(  &delay ,&sntp_msg.xmt ); //转换网络格式	delay
		HTONL_FP(  &offset,&sntp_msg.recv); //转换网络格式	offset
		/* copy data to pbuf */
		pbuf_take(ptr, &sntp_msg , sizeof(sntp_msg));
		/* send udp data */
		udp_send(clientupcb, ptr); 
		/* free pbuf */
		pbuf_free(ptr);
	}
} 




