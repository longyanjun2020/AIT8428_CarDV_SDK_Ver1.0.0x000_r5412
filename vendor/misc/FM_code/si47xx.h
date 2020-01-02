/*
*********************************************************************************************************
*											        ePDK
*						            the Easy Portable/Player Develop Kits
*									           hello world sample
*
*						        (c) Copyright 2006-2007, Steven.ZGJ China
*											All	Rights Reserved
*
* File    : rda5820.h
* By      : xueli
* Version : V1.00
*********************************************************************************************************
*/
#ifndef  _FM_SI47XX_H_
#define  _FM_SI47XX_H_
#include "commanddefs.h"



extern   void  fm_si47xx_init(void);
extern   void  am_si47xx_init(void);
extern   __s32  fm_si47xx_exit(void);
extern   __s32  am_si47xx_exit(void);
extern   __s32  fm_si47xx_play(__s32 freq);
extern   __s32  fm_si47xx_auto_search(__s32 freq);
extern   __s32  fm_si47xx_manual_search(__s32 freq);

extern   __s32  fm_si47xx_stereo_choose(__s32 audio_method);
extern   __s32  fm_si47xx_mute(__s32 voice_onoff);
extern   __s32  fm_si47xx_signal_level(__s32 signal_level);

extern   __s32  fm_si47xx_send_on(void);
extern   __s32  fm_si47xx_send(__s32 freq);
extern   __s32  fm_si47xx_send_off(void);
extern   __s32  fm_si47xx_pa_gain(__u8 pagain);
extern   __s32  fm_si47xx_get_status(void);


extern __s32  fm_si47xx_get_rssi_value(void);
extern __s32  fm_si47xx_get_snr_valule(void);

extern void wait_10ms(u16 ms);





//extern __u32 fm_snr;
//extern __u32 fm_rssi;
extern __u32 am_snr;
extern __u32 am_rssi;
extern __u32  g_fm_db;
extern __u32  g_loc_default;
extern __u32  g_loc_setval;



#endif /*_FM_TEA5767_H_*/

