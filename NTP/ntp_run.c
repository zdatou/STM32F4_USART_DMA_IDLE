#include "Allheader.h"

extern ts_FLASH_DATA_SAVE  TS_FDS;
ts_NTPRunTimeOpts   TS_NRTP;

extern PtpClock ptpClock;
#define NTP_Client_Interval  1000
u32     g_NTP_CLIENT_TIME  = 0;


void ntpd_init(void)
{
  struct ip_addr  ipaddr_t;
	
	sntp_server_init();	
	ipaddr_t.addr  = htonl(TS_FDS.ts_remote_ipaddr.ui_ip);
  udp_sntp_client_init(ipaddr_t);
}


void ntpd_Periodic_Handle(uint32_t localTime)
{
		if( localTime - g_NTP_CLIENT_TIME > NTP_Client_Interval )
		{
		    g_NTP_CLIENT_TIME = localTime;
			  udp_sntp_client_connect();
		}
}
