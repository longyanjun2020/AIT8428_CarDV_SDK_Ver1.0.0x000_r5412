//==============================================================================
//
//  File        : StateMoviePBFunc.h
//  Description : INCLUDE File for the StateCameraFunc function porting.
//  Author      : 
//  Revision    : 1.0
//
//==============================================================================

#ifndef _STATEMOVIEPBFUNC_H_
#define _STATEMOVIEPBFUNC_H_

/*===========================================================================
 * Include files
 *===========================================================================*/ 

#include "MenuCommon.h"

/*===========================================================================
 * Macro define
 *===========================================================================*/ 

#define MOVPB_MAX_SPEED			 (8)
#define MOVPB_MIN_SPEED			 (2)

/*===========================================================================
 * Function prototype
 *===========================================================================*/

DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_KEY_UP);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_KEY_DOWN);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_BKWD_PRESS);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_FRWD_PRESS);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_PLAY_PRESS);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_RETURN);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_KEY_MENU);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_PREV_PRESS);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_NEXT_PRESS);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_VIDEO_PREVIEW);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_CAMERA_PREVIEW);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_FILE_DELETING);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_LOCK_FILE_M);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_DC_CABLE_IN);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_DC_CABLE_OUT);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_USB_DETECT);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_USB_REMOVED);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_SD_DETECT);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_SD_REMOVED);
#if (HDMI_ENABLE)
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_HDMI_DETECT);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_HDMI_REMOVED);
#endif
#if (TVOUT_ENABLE)
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_TV_DETECT);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_TV_REMOVED);
#endif
#if (UVC_HOST_VIDEO_ENABLE)
DECL_AHC_EV_HANDLER(STATE_PLAYBACK_MODE_EVENT_USB_B_DEVICE_DETECT);
DECL_AHC_EV_HANDLER(STATE_PLAYBACK_MODE_EVENT_USB_B_DEVICE_REMOVED);
#endif
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_MOVPB_UPDATE_MESSAGE);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_SUB_MODE_ENTER);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_SUB_MODE_EXIT);
DECL_AHC_EV_HANDLER(STATE_MOVPB_MODE_EVENT_ENTER_NET_PLAYBACK);


AHC_BOOL 	MoviePBMode_Start(void);
UINT32 StateMoviePlaybackModeHandler(UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam);
void 		StateMoviePlaybackMode( UINT32 ulEvent );
void 		MovPBFunc_SetOSDShowStatus(AHC_BOOL state);
AHC_BOOL 	MovPBFunc_GetOSDShowStatus(void);
AHC_BOOL 	MovPBTimer_Start(UINT32 ulTime);
AHC_BOOL 	MovPBTimer_Stop(void);
AHC_BOOL    MovPBAdjVolumeTimer_Stop(void);
void        MovPBFunc_SetOSDShowStatus(AHC_BOOL state);
void 		MovPBTimer_ResetCounter(void);
void 		MovPB_SDMMC_Change(void);
//void 		PlaybackMode_SDMMC_In(void);
//void 		PlaybackMode_SDMMC_Out(void);

AHC_BOOL StateMoviePlaybackModeInitLCD(void* pData);
AHC_BOOL StateMoviePlaybackModeInitHDMI(void* pData);
AHC_BOOL StateMoviePlaybackModeInitTV(void* pData);
AHC_BOOL StateMoviePlaybackModeShutDown(void* pData);
void 		StateAudioPlaybackMode(UINT32 ulEvent);
AHC_BOOL 	AudioPBMode_Start(void);
void 		AudioPBMode_Update(void);
void 		MoviePBMode_Update(void);
void 		PhotoPBMode_Update(void);
void 		PhotoPB_SDMMC_Change(void);
void		MovPBFunc_ResetPlaySpeed(void);

#endif //_STATEMOVIEPBFUNC_H_
