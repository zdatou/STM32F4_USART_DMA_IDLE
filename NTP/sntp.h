/*************************************************************************
 * 
 * Copyright (c) 2002-2005 by Zhang Huiyong All Rights Reserved
 *
 * FILENAME:  Sntp.h
 *
 * PURPOSE :  �������ݽṹ������.
 *  
 * 
 **************************************************************************/

#ifndef SNTP_H_
#define SNTP_H_

/* SNTP ���� */
#define SNTP_VERSION     ((unsigned char)3) /* ��ǰ�汾�� */
#define SNTP_OLD_VERSION ((unsigned char)1) /* �ɵİ汾 */
#define SNTP_MCAST_ADDR  "224.0.1.1"   /* �ಥ��ַ      */
#define SNTP_DEF_PORT     5123         /* SNTP Ĭ�϶˿� */
#define SNTP_PORT  123

/* Mode ֵ */
#define MODE_RESERV      0 /* reserved */
#define MODE_ACTIVE      1 /* symmetric active */
#define MODE_PASSIVE     2 /* symmetric passive */
#define MODE_CLIENT      3 /* client */
#define MODE_SERVER      4 /* server */
#define MODE_BROADCAST   5 /* broadcast */
#define MODE_CONTROL     6

/* Leap ֵ */
#define LEAP_NO_WARNING  0 /* no warning */
#define LEAP_ADD_SECOND  1 /* last minute has 61 seconds */
#define LEAP_DEL_SECOND  2 /* last minute has 59 seconds */
#define LEAP_NOT_SYNC    3 /* alarm condition (clock not synchronized) */

#define SNTP_STRATUM     2 /* secondary reference  */

/* SystemTimeToFileTime() �õ���ʱ��, 1900.1.1 �� 1601.1.1 100ns �ļ�� */
//#define SNTP_TIME_1900          0x014F373BFDE04000
//#define SNTP_HECTO_NANOSECONDS  10000000
#define SNTP_TS_FRAC            4294967296.  /* 2^32 as a double */

/* �õ� SNTP ��Ϣ�е� Mode �� Version */
#define MSG_MODE(vi_vn_md)    ((unsigned char)((vi_vn_md) & 0x7))
#define MSG_VERSION(vi_vn_md) ((unsigned char)(((vi_vn_md) >> 3) & 0x7))

/* �� Leap, Version, Mode �ŵ�һ�� */
#define MSG_LI_VN_MODE(li, vn, md) \
((unsigned char)((((li) << 6) & 0xc0) | (((vn) << 3) & 0x38) | ((md) & 0x7)))

#define HTONL_FP(h, n) do { (n)->fp_ui = htonl((h)->fp_ui); \
                            (n)->fp_uf = htonl((h)->fp_uf); } while (0)

#define NTOHL_FP(n, h) do { (h)->fp_ui = ntohl((n)->fp_ui); \
                            (h)->fp_uf = ntohl((n)->fp_uf); } while (0)
//�෴��
#define FP_NEG(v_i, v_f) /* v = -v */ \
    do { \
        if ((v_f) == 0) \
            (v_i) = -((int)(v_i)); \
        else { \
            (v_f) = -((int)(v_f)); \
            (v_i) = ~(v_i); \
        } \
    } while(0)
		
//FP ��
#define FP_ADD(r_i, r_f, a_i, a_f) /* r += a */ \
    do { \
        unsigned int lo_half; \
        unsigned int hi_half; \
        \
        lo_half = ((r_f) & 0xffff) + ((a_f) & 0xffff); \
        hi_half = (((r_f) >> 16) & 0xffff) + (((a_f) >> 16) & 0xffff); \
        if (lo_half & 0x10000) \
            hi_half++; \
        (r_f) = ((hi_half & 0xffff) << 16) | (lo_half & 0xffff); \
        \
        (r_i) += (a_i); \
        if (hi_half & 0x10000) \
            (r_i)++; \
    } while (0)
//��
#define FP_SUB(r_i, r_f, a_i, a_f) /* r -= a */ \
    do { \
        unsigned int lo_half; \
        unsigned int hi_half; \
        \
        if ((a_f) == 0) { \
            (r_i) -= (a_i); \
        } else { \
            lo_half = ((r_f) & 0xffff) + ((-((int)(a_f))) & 0xffff); \
            hi_half = (((r_f) >> 16) & 0xffff) \
                     + (((-((int)(a_f))) >> 16) & 0xffff); \
            if (lo_half & 0x10000) \
               hi_half++; \
            (r_f) = ((hi_half & 0xffff) << 16) | (lo_half & 0xffff); \
            \
            (r_i) += ~(a_i); \
            if (hi_half & 0x10000) \
                (r_i)++; \
        } \
    } while (0)
//����λ
#define FP_RSHIFT(v_i, v_f) /* v >>= 1, v is signed */ \
    do { \
        (v_f) = (unsigned int)(v_f) >> 1; \
        if ((v_i) & 01) \
            (v_f) |= 0x80000000; \
        if ((v_i) & 0x80000000) \
            (v_i) = ((v_i) >> 1) | 0x80000000; \
        else \
            (v_i) = (v_i) >> 1; \
    } while (0)
//�����ȵ��ྫ��
#define FP_TOD(r_i, r_uf, d) /* fixed-point to double */ \
    do { \
        time_fp l_tmp; \
        \
        l_tmp.fp_si = (r_i); \
        l_tmp.fp_sf = (r_uf); \
        if (l_tmp.fp_si < 0) { \
            FP_NEG(l_tmp.fp_si, l_tmp.fp_uf); \
            (d) = -((double)l_tmp.fp_si+((double)l_tmp.fp_uf)/SNTP_TS_FRAC);\
        } else { \
            (d) = (double)l_tmp.fp_si+((double)l_tmp.fp_uf)/SNTP_TS_FRAC;\
        } \
    } while (0)

/* ʱ�����, ��, ���Ƽ�ת��Ϊ˫���� */
#define TS_ADD(r, a)   FP_ADD((r)->fp_ui, (r)->fp_uf, (a)->fp_ui, (a)->fp_uf)
#define TS_SUB(r, a)   FP_SUB((r)->fp_ui, (r)->fp_uf, (a)->fp_ui, (a)->fp_uf)
#define TS_RSHIFT(v)   FP_RSHIFT((v)->fp_si, (v)->fp_uf)
#define TS_FPTOD(v, d) FP_TOD((v)->fp_ui, (v)->fp_uf, (d))

/* SNTPʱ����ṹ */
typedef struct 
{
    union /* ���� */
    {
        unsigned int ui;
        int si;
    } intg;

    union /* С�� */
    {
        unsigned int uf;
        int sf;
    } frag;
} time_fp;

#define fp_ui    intg.ui
#define fp_si    intg.si
#define fp_uf    frag.uf
#define fp_sf    frag.sf

/* SNTP ��Ϣ��ʽ */
struct sntp_s
{
    unsigned char li_vn_md; /* Leap, Version, Mode */
    unsigned char stratum;  /* Stratum */
    unsigned char poll;     /* Poll Interval */
    char percision;         /* Percision */
    unsigned int root_delay;/* Root Delay */
    unsigned int root_disp; /* Root Dispersion */
    unsigned int ref_id;    /* Reference Identifier */
    time_fp ref_time;       /* Reference Timestamp */
    time_fp orig;           /* Originate Timestamp */
    time_fp recv;           /* Receive Timestamp */
    time_fp xmt;            /* Transmit Timestamp */
//    char key_id[32];        /* Key Identifier */
//    char digest[128];       /* Message Digest */
};

/* SNTP ��ؽṹ��*/
struct sntp_c{
  unsigned int ipaddr;  //4�ֽ�
  unsigned int Time;   //4�ֽ�
	time_fp root_disp;    //4�ֽ�
};


extern int sntp_get_sys_time(time_fp *current);
#endif /* SNTP_H_ */
