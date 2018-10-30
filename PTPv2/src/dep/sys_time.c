/* sys.c */

#include "../ptpd.h"



void getTime(TimeInternal *time)
{
    struct ptptime_t timestamp;
    ETH_PTPTime_GetTime(&timestamp);
    time->seconds = timestamp.tv_sec;
    time->nanoseconds = timestamp.tv_nsec;
}



void setTime(const TimeInternal *time)
{
    struct ptptime_t ts;
	
    ts.tv_sec = time->seconds;
    ts.tv_nsec = time->nanoseconds;

    ETH_PTPTime_SetTime(&ts);

    DBG("resetting system clock to %ds %dns\n", time->seconds, time->nanoseconds);
}

void updateTime(const TimeInternal *time)
{

    struct ptptime_t timeoffset;

    DBGV("updateTime: %ds %dns\n", time->seconds, time->nanoseconds);

    timeoffset.tv_sec = -time->seconds;
    timeoffset.tv_nsec = -time->nanoseconds;

	/* Coarse update method */
    ETH_PTPTime_UpdateOffset(&timeoffset);
    DBGV("updateTime: updated\n");
}

UInteger32 getRand(UInteger32 randMax)
{
    return rand() % randMax;
}

Boolean adjFreq(Integer32 adj)
{
    DBGV("adjFreq %d\n", adj);

    if (adj > ADJ_FREQ_MAX)
        adj = ADJ_FREQ_MAX;
    else if (adj < -ADJ_FREQ_MAX)
        adj = -ADJ_FREQ_MAX;

    /* Fine update method */
	ETH_PTPTime_AdjFreq(adj);

    return TRUE;
}
