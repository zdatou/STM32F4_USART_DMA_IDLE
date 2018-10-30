#ifndef _ntp_run_h_
#define _ntp_run_h_



#include "stm32f4xx.h"



typedef struct{

		uint8_t portState;
	  double  StandardtoOffset; 
	  double  DelayUnit;
 
}ts_NTPRunTimeOpts; 

enum {
	emPTPSlave,
	emPTPMaster,
  emNTPSlave,
	emNTPMaster,
};


void ntpd_init(void);
void ntpd_Periodic_Handle(uint32_t localTime);


#endif

