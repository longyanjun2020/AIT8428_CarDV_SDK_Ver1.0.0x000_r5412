//==============================================================================
//
//  File        : RTC_AIT.h
//  Description : INCLUDE File for RTC Control Interface
//  Author      : Jerry Li
//  Revision    : 1.0
//
//==============================================================================
 
#ifndef _RTC_AIT_H_
#define _RTC_AIT_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "Customer_config.h"
#include "mmpf_rtc_ctl.h"
#include "includes_fw.h"
#include "mmp_rtc_inc.h"

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

int RTC_AIT_Module_Init(void) ;
MMP_ERR RTC_AIT_Open(struct _3RD_PARTY_RTC *pthis);
MMP_ERR RTC_AIT_Release(struct _3RD_PARTY_RTC *pthis);
MMP_ERR RTC_AIT_Ioctl(struct _3RD_PARTY_RTC *pthis, MMP_ULONG cmd, MMP_U_LONG arg);
MMP_ERR	RTC_AIT_Set_Time(struct _3RD_PARTY_RTC *pthis, AUTL_DATETIME *pRtcData);
MMP_ERR RTC_AIT_Read_Time(struct _3RD_PARTY_RTC *pthis, AUTL_DATETIME *pRtcData);
MMP_ERR RTC_AIT_Read_ShadowTime(struct _3RD_PARTY_RTC *pthis, AUTL_DATETIME *pRtcData);
MMP_ERR RTC_AIT_Read_Alarm(struct _3RD_PARTY_RTC *pthis, RTC_WKALRM *pAlarm);
MMP_ERR RTC_AIT_Set_Alarm(struct _3RD_PARTY_RTC *pthis, RTC_WKALRM *pAlarm);
MMP_ERR RTC_AIT_Set_Mmss(struct _3RD_PARTY_RTC *pthis, MMP_U_LONG secs);
MMP_ERR RTC_AIT_Read_Callback(struct _3RD_PARTY_RTC *pthis, MMP_LONG data);
MMP_ERR RTC_AIT_Alarm_Irq_Enable(struct _3RD_PARTY_RTC *pthis, MMP_ULONG enabled);

MMP_ERR RTC_AIT_ForceReset(struct _3RD_PARTY_RTC *pthis);
MMP_BOOL RTC_AIT_IsValid(struct _3RD_PARTY_RTC *pthis);
MMP_BOOL RTC_AIT_Read_SecondsToDate(struct _3RD_PARTY_RTC *pthis, MMP_ULONG ulSeconds, AUTL_DATETIME *pRtcData);
MMP_ULONG RTC_AIT_ReadTime_InSeconds(struct _3RD_PARTY_RTC *pthis);

#endif //_RTC_AIT_H_

