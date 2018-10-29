#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define SYS_LIGHTWEIGHT_PROT    0

//NO_SYS==1:��ʹ�ò���ϵͳ
#define NO_SYS                  1  //��ʹ��UCOS����ϵͳ

//ʹ��4�ֽڶ���ģʽ
#define MEM_ALIGNMENT           4  

//MEM_SIZE:heap�ڴ�Ĵ�С,�����Ӧ�����д������ݷ��͵Ļ����ֵ������ô�һ�� 
#define MEM_SIZE                16000 //�ڴ�Ѵ�С

//MEMP_NUM_PBUF:memp�ṹ��pbuf����,���Ӧ�ô�ROM���߾�̬�洢�����ʹ�������ʱ,���ֵӦ�����ô�һ��
#define MEMP_NUM_PBUF           10

//MEMP_NUM_UDP_PCB:UDPЭ����ƿ�(PCB)����.ÿ�����UDP"����"��Ҫһ��PCB.
#define MEMP_NUM_UDP_PCB        6

//MEMP_NUM_TCP_PCB:ͬʱ���������TCP����
#define MEMP_NUM_TCP_PCB        10

//MEMP_NUM_TCP_PCB_LISTEN:�ܹ�������TCP��������
#define MEMP_NUM_TCP_PCB_LISTEN 6

//MEMP_NUM_TCP_SEG:���ͬʱ�ڶ����е�TCP������
#define MEMP_NUM_TCP_SEG        15

//MEMP_NUM_SYS_TIMEOUT:�ܹ�ͬʱ�����timeout����
#define MEMP_NUM_SYS_TIMEOUT    8


/* ---------- Pbufѡ��---------- */
//PBUF_POOL_SIZE:pbuf�ڴ�ظ���. 
#define PBUF_POOL_SIZE          20

//PBUF_POOL_BUFSIZE:ÿ��pbuf�ڴ�ش�С. 
#define PBUF_POOL_BUFSIZE       512


/* ---------- TCPѡ��---------- */
#define LWIP_TCP                1  //Ϊ1��ʹ��TCP
#define TCP_TTL                 255//����ʱ��

/*��TCP�����ݶγ�������ʱ�Ŀ���λ,���豸���ڴ��С��ʱ�����ӦΪ0*/
#define TCP_QUEUE_OOSEQ         0

//���TCP�ֶ�
#define TCP_MSS                 (1500 - 40)	  //TCP_MSS = (MTU - IP��ͷ��С - TCP��ͷ��С

//TCP���ͻ�������С(bytes).
#define TCP_SND_BUF             (4*TCP_MSS)

//TCP_SND_QUEUELEN: TCP���ͻ�������С(pbuf).���ֵ��СΪ(2 * TCP_SND_BUF/TCP_MSS) 
#define TCP_SND_QUEUELEN        (2* TCP_SND_BUF/TCP_MSS)

//TCP���ʹ���
#define TCP_WND                 (2*TCP_MSS)


/* ---------- ICMPѡ��---------- */
#define LWIP_ICMP                 1 //ʹ��ICMPЭ��

/* ---------- DHCPѡ��---------- */
//��ʹ��DHCPʱ��λӦ��Ϊ1,LwIP 0.5.1�汾��û��DHCP����.
#define LWIP_DHCP               1

/* ---------- UDPѡ�� ---------- */ 
#define LWIP_UDP                1 //ʹ��UDP����
#define UDP_TTL                 255 //UDP���ݰ�����ʱ��


/* ---------- Statistics options ---------- */
#define LWIP_STATS 0
#define LWIP_PROVIDE_ERRNO 1


//STM32F746����ͨ��Ӳ��ʶ��ͼ���IP,UDP��ICMP��֡У���
#define CHECKSUM_BY_HARDWARE //����CHECKSUM_BY_HARDWARE,ʹ��Ӳ��֡У��

#ifdef CHECKSUM_BY_HARDWARE
  //CHECKSUM_GEN_IP==0: Ӳ������IP���ݰ���֡У���
  #define CHECKSUM_GEN_IP                 0
  //CHECKSUM_GEN_UDP==0: Ӳ������UDP���ݰ���֡У���
  #define CHECKSUM_GEN_UDP                0
  //CHECKSUM_GEN_TCP==0: Ӳ������TCP���ݰ���֡У���
  #define CHECKSUM_GEN_TCP                0 
  //CHECKSUM_CHECK_IP==0: Ӳ����������IP���ݰ�֡У���
  #define CHECKSUM_CHECK_IP               0
  //CHECKSUM_CHECK_UDP==0: Ӳ����������UDP���ݰ�֡У���
  #define CHECKSUM_CHECK_UDP              0
  //CHECKSUM_CHECK_TCP==0: Ӳ����������TCP���ݰ�֡У���
  #define CHECKSUM_CHECK_TCP              0
  /* CHECKSUM_CHECK_ICMP==0:Ӳ����������ICMP���ݰ�֡У���*/  
  #define CHECKSUM_GEN_ICMP               0
#else
  //CHECKSUM_GEN_IP==1: �������IP���ݰ�֡У���
  #define CHECKSUM_GEN_IP                 1
  // CHECKSUM_GEN_UDP==1: �������UDOP���ݰ�֡У���
  #define CHECKSUM_GEN_UDP                1
  //CHECKSUM_GEN_TCP==1: �������TCP���ݰ�֡У���
  #define CHECKSUM_GEN_TCP                1
  // CHECKSUM_CHECK_IP==1: �����������IP���ݰ�֡У���
  #define CHECKSUM_CHECK_IP               1
  // CHECKSUM_CHECK_UDP==1: �����������UDP���ݰ�֡У���
  #define CHECKSUM_CHECK_UDP              1
  //CHECKSUM_CHECK_TCP==1: �����������TCP���ݰ�֡У���
  #define CHECKSUM_CHECK_TCP              1
   /* CHECKSUM_CHECK_ICMP==0:�����������ICMP���ݰ�֡У���*/  
  #define CHECKSUM_GEN_ICMP               1
#endif


/*
   ----------------------------------------------
   ---------- SequentialAPIѡ��----------
   ----------------------------------------------
*/

//LWIP_NETCONN==1:ʹ��NETCON����(Ҫ��ʹ��api_lib.c)
#define LWIP_NETCONN                    0

/*
   ------------------------------------
   ---------- Socket APIѡ��----------
   ------------------------------------
*/
//LWIP_SOCKET==1:ʹ��Socket API(Ҫ��ʹ��sockets.c)
#define LWIP_SOCKET                     0

#define LWIP_COMPAT_MUTEX               1

#define LWIP_SO_RCVTIMEO                1 //ͨ������LWIP_SO_RCVTIMEOʹ��netconn�ṹ����recv_timeout,ʹ��recv_timeout���Ա��������߳�


/*
   ----------------------------------------
   ---------- Lwip����ѡ��----------
   ----------------------------------------
*/
//#define LWIP_DEBUG                     1 //����DEBUGѡ��

#define ICMP_DEBUG                      LWIP_DBG_OFF //����/�ر�ICMPdebug

#endif /* __LWIPOPTS_H__ */

