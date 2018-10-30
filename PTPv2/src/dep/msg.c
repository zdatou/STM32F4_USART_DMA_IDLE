/* msg.c */


#include "../ptpd.h"

#define XOR_VALUE 0X0A0A0A0A

/*Unpack Header from IN buffer to msgTmpHeader field */
void msgUnpackHeader(const Octet *buf, MsgHeader *header)
{
    Integer32 msb;
    UInteger32 lsb;

    header->transportSpecific = (*(Nibble*)(buf + 0)) >> 4;
    header->messageType = (*(Enumeration4*)(buf + 0)) & 0x0F;
    header->versionPTP = (*(UInteger4*)(buf + 1)) & 0x0F; //force reserved bit to zero if not
    header->messageLength = flip16(*(UInteger16*)(buf + 2));
    header->domainNumber = (*(UInteger8*)(buf + 4));
    memcpy(header->flagField, (buf + 6), FLAG_FIELD_LENGTH);
    memcpy(&msb, (buf + 8), 4);
    memcpy(&lsb, (buf + 12), 4);
    header->correctionfield = flip32(msb);
    header->correctionfield <<= 32;
    header->correctionfield += flip32(lsb);
    memcpy(header->sourcePortIdentity.clockIdentity, (buf + 20), CLOCK_IDENTITY_LENGTH);
    header->sourcePortIdentity.portNumber = flip16(*(UInteger16*)(buf + 28));
    header->sequenceId = flip16(*(UInteger16*)(buf + 30));
    header->controlField = (*(UInteger8*)(buf + 32));
    header->logMessageInterval = (*(Integer8*)(buf + 33));
}

/*Pack header message into OUT buffer of ptpClock*/
void msgPackHeader(const PtpClock *ptpClock, Octet *buf)
{
    Nibble transport = 0x80; //(spec annex D)
    *(UInteger8*)(buf + 0) = transport;
    *(UInteger4*)(buf + 1) = ptpClock->portDS.versionNumber;
    *(UInteger8*)(buf + 4) = ptpClock->defaultDS.domainNumber;

    if (ptpClock->defaultDS.twoStepFlag)
    {
        *(UInteger8*)(buf + 6) = FLAG0_TWO_STEP;
    }

    memset((buf + 8), 0, 8);

    memcpy((buf + 20), ptpClock->portDS.portIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH);
    *(UInteger16*)(buf + 28) = flip16(ptpClock->portDS.portIdentity.portNumber);
    *(UInteger8*)(buf + 33) = 0x7F; //Default value (spec Table 24)
}

/*Pack Announce message into OUT buffer of ptpClock*/
void msgPackAnnounce(const PtpClock *ptpClock, Octet *buf)
{
    /*changes in header*/
    *(char*)(buf + 0) = *(char*)(buf + 0) & 0xF0; //RAZ messageType
    *(char*)(buf + 0) = *(char*)(buf + 0) | ANNOUNCE; //Table 19
    *(UInteger16*)(buf + 2) = flip16(ANNOUNCE_LENGTH);
    *(UInteger16*)(buf + 30) = flip16(ptpClock->sentAnnounceSequenceId);
    *(UInteger8*)(buf + 32) = CTRL_OTHER; /* Table 23 - controlField */
    *(Integer8*)(buf + 33) = ptpClock->portDS.logAnnounceInterval;

    /*Announce message*/
    memset((buf + 34), 0, 10); /* originTimestamp */
    *(Integer16*)(buf + 44) = flip16(ptpClock->timePropertiesDS.currentUtcOffset);
    *(UInteger8*)(buf + 47) = ptpClock->parentDS.grandmasterPriority1;
    *(UInteger8*)(buf + 48) = ptpClock->defaultDS.clockQuality.clockClass;
    *(Enumeration8*)(buf + 49) = ptpClock->defaultDS.clockQuality.clockAccuracy;
    *(UInteger16*)(buf + 50) = flip16(ptpClock->defaultDS.clockQuality.offsetScaledLogVariance);
    *(UInteger8*)(buf + 52) = ptpClock->parentDS.grandmasterPriority2;
    memcpy((buf + 53), ptpClock->parentDS.grandmasterIdentity, CLOCK_IDENTITY_LENGTH);
    *(UInteger16*)(buf + 61) = flip16(ptpClock->currentDS.stepsRemoved);
    *(Enumeration8*)(buf + 63) = ptpClock->timePropertiesDS.timeSource;
}

/*Unpack Announce message from IN buffer of ptpClock to msgtmp.Announce*/
void msgUnpackAnnounce(const Octet *buf, MsgAnnounce *announce)
{
    announce->originTimestamp.secondsField.msb = flip16(*(UInteger16*)(buf + 34))  ;
    announce->originTimestamp.secondsField.lsb = flip32(*(UInteger32*)(buf + 36))  ;
    announce->originTimestamp.nanosecondsField = flip32(*(UInteger32*)(buf + 40))  ;
    announce->currentUtcOffset = flip16(*(UInteger16*)(buf + 44));
    announce->grandmasterPriority1 = *(UInteger8*)(buf + 47);
    announce->grandmasterClockQuality.clockClass = *(UInteger8*)(buf + 48);
    announce->grandmasterClockQuality.clockAccuracy = *(Enumeration8*)(buf + 49);
    announce->grandmasterClockQuality.offsetScaledLogVariance = flip16(*(UInteger16*)(buf + 50));
    announce->grandmasterPriority2 = *(UInteger8*)(buf + 52);
    memcpy(announce->grandmasterIdentity, (buf + 53), CLOCK_IDENTITY_LENGTH);
    announce->stepsRemoved = flip16(*(UInteger16*)(buf + 61));
    announce->timeSource = *(Enumeration8*)(buf + 63);
}

/*Pack SYNC message into OUT buffer of ptpClock*/
void msgPackSync(const PtpClock *ptpClock, Octet *buf, const Timestamp *originTimestamp)
{
    /*changes in header*/
    *(char*)(buf + 0) = *(char*)(buf + 0) & 0xF0; //RAZ messageType
    *(char*)(buf + 0) = *(char*)(buf + 0) | SYNC; //Table 19
    *(UInteger16*)(buf + 2) = flip16(SYNC_LENGTH);
    *(UInteger16*)(buf + 30) = flip16(ptpClock->sentSyncSequenceId);
    *(UInteger8*)(buf + 32) = CTRL_SYNC; //Table 23
    *(Integer8*)(buf + 33) = ptpClock->portDS.logSyncInterval;
    memset((buf + 8), 0, 8); /* correction field */

    /*Sync message*/
    *(UInteger16*)(buf + 34) = flip16(originTimestamp->secondsField.msb)             ;
    *(UInteger32*)(buf + 36) = flip32(originTimestamp->secondsField.lsb)  ;
    *(UInteger32*)(buf + 40) = flip32(originTimestamp->nanosecondsField)  ;
}

/*Unpack Sync message from IN buffer */
void msgUnpackSync(const Octet *buf, MsgSync *sync)
{
    sync->originTimestamp.secondsField.msb = flip16(*(UInteger16*)(buf + 34))             ;
    sync->originTimestamp.secondsField.lsb = flip32(*(UInteger32*)(buf + 36)) ;
    sync->originTimestamp.nanosecondsField = flip32(*(UInteger32*)(buf + 40)) ;
}

/*pack delayReq message into OUT buffer of ptpClock*/
void msgPackDelayReq(const PtpClock *ptpClock, Octet *buf, const Timestamp *originTimestamp)
{
    /*changes in header*/
    *(char*)(buf + 0) = *(char*)(buf + 0) & 0xF0; //RAZ messageType
    *(char*)(buf + 0) = *(char*)(buf + 0) | DELAY_REQ; //Table 19
    *(UInteger16*)(buf + 2) = flip16(DELAY_REQ_LENGTH);
    *(UInteger16*)(buf + 30) = flip16(ptpClock->sentDelayReqSequenceId);
    *(UInteger8*)(buf + 32) = CTRL_DELAY_REQ; //Table 23
    *(Integer8*)(buf + 33) = 0x7F; //Table 24
    memset((buf + 8), 0, 8);

    /*delay_req message*/
    *(UInteger16*)(buf + 34) = flip16(originTimestamp->secondsField.msb)             ;
    *(UInteger32*)(buf + 36) = flip32(originTimestamp->secondsField.lsb) ;
    *(UInteger32*)(buf + 40) = flip32(originTimestamp->nanosecondsField);
}

/*Unpack delayReq message from IN buffer of ptpClock to msgtmp.req*/
void msgUnpackDelayReq(const Octet *buf, MsgDelayReq *delayreq)
{
    delayreq->originTimestamp.secondsField.msb = flip16(*(UInteger16*)(buf + 34))             ;
    delayreq->originTimestamp.secondsField.lsb = flip32(*(UInteger32*)(buf + 36)) ;
    delayreq->originTimestamp.nanosecondsField = flip32(*(UInteger32*)(buf + 40))  ;
}

/*pack Follow_up message into OUT buffer of ptpClock*/
void msgPackFollowUp(const PtpClock *ptpClock, Octet*buf, const Timestamp *preciseOriginTimestamp)
{
    /*changes in header*/
    *(char*)(buf + 0) = *(char*)(buf + 0) & 0xF0; //RAZ messageType
    *(char*)(buf + 0) = *(char*)(buf + 0) | FOLLOW_UP; //Table 19
    *(UInteger16*)(buf + 2) = flip16(FOLLOW_UP_LENGTH);
    *(UInteger16*)(buf + 30) = flip16(ptpClock->sentSyncSequenceId - 1);//sentSyncSequenceId has already been incremented in issueSync
    *(UInteger8*)(buf + 32) = CTRL_FOLLOW_UP; //Table 23
    *(Integer8*)(buf + 33) = ptpClock->portDS.logSyncInterval;

    /*Follow_up message*/
    *(UInteger16*)(buf + 34) = flip16(preciseOriginTimestamp->secondsField.msb)  						;
    *(UInteger32*)(buf + 36) = flip32(preciseOriginTimestamp->secondsField.lsb);
    *(UInteger32*)(buf + 40) = flip32(preciseOriginTimestamp->nanosecondsField) ;
}

/*Unpack Follow_up message from IN buffer of ptpClock to msgtmp.follow*/
void msgUnpackFollowUp(const Octet *buf, MsgFollowUp *follow)
{
    follow->preciseOriginTimestamp.secondsField.msb = flip16(*(UInteger16*)(buf + 34))             ;
    follow->preciseOriginTimestamp.secondsField.lsb = flip32(*(UInteger32*)(buf + 36)) ;
    follow->preciseOriginTimestamp.nanosecondsField = flip32(*(UInteger32*)(buf + 40))  ;
}

/*pack delayResp message into OUT buffer of ptpClock*/
void msgPackDelayResp(const PtpClock *ptpClock, Octet *buf, const MsgHeader *header, const Timestamp *receiveTimestamp)
{
    /*changes in header*/
    *(char*)(buf + 0) = *(char*)(buf + 0) & 0xF0; //RAZ messageType
    *(char*)(buf + 0) = *(char*)(buf + 0) | DELAY_RESP; //Table 19
    *(UInteger16*)(buf + 2) = flip16(DELAY_RESP_LENGTH);
    /* *(UInteger8*)(buf+4) = header->domainNumber; */ /* TODO: Why? */
    memset((buf + 8), 0, 8);

    /*Copy correctionField of delayReqMessage*/
    *(Integer32*)(buf + 8) = flip32(header->correctionfield >> 32);
    *(Integer32*)(buf + 12) = flip32((Integer32)header->correctionfield);

    *(UInteger16*)(buf + 30) = flip16(header->sequenceId);

    *(UInteger8*)(buf + 32) = CTRL_DELAY_RESP; //Table 23
    *(Integer8*)(buf + 33) = ptpClock->portDS.logMinDelayReqInterval; //Table 24

    /*delay_resp message*/
    *(UInteger16*)(buf + 34) = flip16(receiveTimestamp->secondsField.msb)             ;
    *(UInteger32*)(buf + 36) = flip32(receiveTimestamp->secondsField.lsb) ;
    *(UInteger32*)(buf + 40) = flip32(receiveTimestamp->nanosecondsField) ;
    memcpy((buf + 44), header->sourcePortIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH);
    *(UInteger16*)(buf + 52) = flip16(header->sourcePortIdentity.portNumber);
}

/*Unpack delayResp message from IN buffer of ptpClock to msgtmp.presp*/
void msgUnpackDelayResp(const Octet *buf, MsgDelayResp *resp)
{
    resp->receiveTimestamp.secondsField.msb = flip16(*(UInteger16*)(buf + 34)) ;
    resp->receiveTimestamp.secondsField.lsb = flip32(*(UInteger32*)(buf + 36)) ;
    resp->receiveTimestamp.nanosecondsField = flip32(*(UInteger32*)(buf + 40)) ;
    memcpy(resp->requestingPortIdentity.clockIdentity, (buf + 44), CLOCK_IDENTITY_LENGTH);
    resp->requestingPortIdentity.portNumber = flip16(*(UInteger16*)(buf + 52));
}

/*pack PdelayReq message into OUT buffer of ptpClock*/
void msgPackPDelayReq(const PtpClock *ptpClock, Octet *buf, const Timestamp *originTimestamp)
{
    /*changes in header*/
    *(char*)(buf + 0) = *(char*)(buf + 0) & 0xF0; //RAZ messageType
    *(char*)(buf + 0) = *(char*)(buf + 0) | PDELAY_REQ; //Table 19
    *(UInteger16*)(buf + 2) = flip16(PDELAY_REQ_LENGTH);
    *(UInteger16*)(buf + 30) = flip16(ptpClock->sentPDelayReqSequenceId);
    *(UInteger8*)(buf + 32) = CTRL_OTHER; //Table 23
    *(Integer8*)(buf + 33) = 0x7F; //Table 24
    memset((buf + 8), 0, 8);

    /*Pdelay_req message*/
    *(UInteger16*)(buf + 34) = flip16(originTimestamp->secondsField.msb);
    *(UInteger32*)(buf + 36) = flip32(originTimestamp->secondsField.lsb) ;
    *(UInteger32*)(buf + 40) = flip32(originTimestamp->nanosecondsField) ;

    memset((buf + 44), 0, 10); // RAZ reserved octets
}

/*Unpack PdelayReq message from IN buffer of ptpClock to msgtmp.req*/
void msgUnpackPDelayReq(const Octet *buf, MsgPDelayReq *pdelayreq)
{
    pdelayreq->originTimestamp.secondsField.msb = flip16(*(UInteger16*)(buf + 34));
    pdelayreq->originTimestamp.secondsField.lsb = flip32(*(UInteger32*)(buf + 36)) ;
    pdelayreq->originTimestamp.nanosecondsField = flip32(*(UInteger32*)(buf + 40)) ;
}

/*pack PdelayResp message into OUT buffer of ptpClock*/
void msgPackPDelayResp(Octet *buf, const MsgHeader *header, const Timestamp *requestReceiptTimestamp)
{
    /*changes in header*/
    *(char*)(buf + 0) = *(char*)(buf + 0) & 0xF0; //RAZ messageType
    *(char*)(buf + 0) = *(char*)(buf + 0) | PDELAY_RESP; //Table 19
    *(UInteger16*)(buf + 2) = flip16(PDELAY_RESP_LENGTH);
    /* *(UInteger8*)(buf+4) = header->domainNumber; */ /* TODO: Why? */
    memset((buf + 8), 0, 8);


    *(UInteger16*)(buf + 30) = flip16(header->sequenceId);

    *(UInteger8*)(buf + 32) = CTRL_OTHER; //Table 23
    *(Integer8*)(buf + 33) = 0x7F; //Table 24

    /*Pdelay_resp message*/
    *(UInteger16*)(buf + 34) = flip16(requestReceiptTimestamp->secondsField.msb);
    *(UInteger32*)(buf + 36) = flip32(requestReceiptTimestamp->secondsField.lsb) ;
    *(UInteger32*)(buf + 40) = flip32(requestReceiptTimestamp->nanosecondsField) ;
    memcpy((buf + 44), header->sourcePortIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH);
    *(UInteger16*)(buf + 52) = flip16(header->sourcePortIdentity.portNumber);

}

/*Unpack PdelayResp message from IN buffer of ptpClock to msgtmp.presp*/
void msgUnpackPDelayResp(const Octet *buf, MsgPDelayResp *presp)
{
    presp->requestReceiptTimestamp.secondsField.msb = flip16(*(UInteger16*)(buf + 34));
    presp->requestReceiptTimestamp.secondsField.lsb = flip32(*(UInteger32*)(buf + 36)) ;
    presp->requestReceiptTimestamp.nanosecondsField = flip32(*(UInteger32*)(buf + 40))  ;
    memcpy(presp->requestingPortIdentity.clockIdentity, (buf + 44), CLOCK_IDENTITY_LENGTH);
    presp->requestingPortIdentity.portNumber = flip16(*(UInteger16*)(buf + 52));
}

/*pack PdelayRespfollowup message into OUT buffer of ptpClock*/
void msgPackPDelayRespFollowUp(Octet *buf, const MsgHeader *header, const Timestamp *responseOriginTimestamp)
{
    /*changes in header*/
    *(char*)(buf + 0) = *(char*)(buf + 0) & 0xF0; //RAZ messageType
    *(char*)(buf + 0) = *(char*)(buf + 0) | PDELAY_RESP_FOLLOW_UP; //Table 19
    *(UInteger16*)(buf + 2) = flip16(PDELAY_RESP_FOLLOW_UP_LENGTH);
    *(UInteger16*)(buf + 30) = flip16(header->sequenceId);
    *(UInteger8*)(buf + 32) = CTRL_OTHER; //Table 23
    *(Integer8*)(buf + 33) = 0x7F; //Table 24

    /*Copy correctionField of PdelayReqMessage*/
    *(Integer32*)(buf + 8) = flip32(header->correctionfield >> 32);
    *(Integer32*)(buf + 12) = flip32((Integer32)header->correctionfield);

    /*Pdelay_resp_follow_up message*/
    *(UInteger16*)(buf + 34) = flip16(responseOriginTimestamp->secondsField.msb);
    *(UInteger32*)(buf + 36) = flip32(responseOriginTimestamp->secondsField.lsb) ;
    *(UInteger32*)(buf + 40) = flip32(responseOriginTimestamp->nanosecondsField) ;
    memcpy((buf + 44), header->sourcePortIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH);
    *(UInteger16*)(buf + 52) = flip16(header->sourcePortIdentity.portNumber);
}

/*Unpack PdelayResp message from IN buffer of ptpClock to msgtmp.presp*/
void msgUnpackPDelayRespFollowUp(const Octet *buf, MsgPDelayRespFollowUp *prespfollow)
{
    prespfollow->responseOriginTimestamp.secondsField.msb = flip16(*(UInteger16*)(buf + 34));
    prespfollow->responseOriginTimestamp.secondsField.lsb = flip32(*(UInteger32*)(buf + 36)) ;
    prespfollow->responseOriginTimestamp.nanosecondsField = flip32(*(UInteger32*)(buf + 40));
    memcpy(prespfollow->requestingPortIdentity.clockIdentity, (buf + 44), CLOCK_IDENTITY_LENGTH);
    prespfollow->requestingPortIdentity.portNumber = flip16(*(UInteger16*)(buf + 52));
}

#if LWIP_PTP_MONITOR
/*pack Management message into OUT buffer of ptpClock*/
void msgPackManagement(const Octet *buf, const Timestamp *offset, UInteger32 local_ip)
{
    /*changes in header*/
    *(char*)(buf + 0) = *(char*)(buf + 0) & 0xF0; //RAZ messageType
    *(char*)(buf + 0) = *(char*)(buf + 0) | MANAGEMENT; //Table 19
    *(UInteger16*)(buf + 2) = flip16(MANAGEMENT_LENGTH);
//    *(UInteger16*)(buf + 30) = flip16(header->sequenceId);
    *(UInteger8*)(buf + 32) = CTRL_MANAGEMENT; //Table 23
    *(Integer8*)(buf + 33) = 0x7F; //Table 24

    /*Management message*/ //����ͬ�������Լ�IP��ַ
    *(UInteger16*)(buf + 34) = flip16(offset->secondsField.msb);
    *(UInteger32*)(buf + 36) = flip32(offset->secondsField.lsb)  ;
    *(UInteger32*)(buf + 40) = flip32(offset->nanosecondsField)  ;
	  *(UInteger32*)(buf + 44) = flip32(local_ip);
}

void msgUnpackManagement(const Octet *buf, MsgManagement *prespfollow)
{
	  prespfollow->toMasterOffset.secondsField.msb = flip16(*(UInteger16*)(buf + 34));
    prespfollow->toMasterOffset.secondsField.lsb = flip32(*(UInteger32*)(buf + 36));
    prespfollow->toMasterOffset.nanosecondsField = flip32(*(UInteger32*)(buf + 40));
	  prespfollow->ipaddr 												 = flip32(*(UInteger32*)(buf + 44));
}
#endif
