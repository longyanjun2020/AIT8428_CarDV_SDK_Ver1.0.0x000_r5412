//==============================================================================
//
//  File        : StateVideoFunc.h
//  Description : INCLUDE File for the StateCameraFunc function porting.
//  Author      : 
//  Revision    : 1.0
//
//==============================================================================

#ifndef _STATEVIDEOFUNC_H_
#define _STATEVIDEOFUNC_H_

/*===========================================================================
 * Include files
 *===========================================================================*/ 

#include "AHC_General.h"
#include "MenuCommon.h"

typedef enum {
    F_LARGE_R_SMALL = 0,
    F_SMALL_R_LARGE,
    ONLY_FRONT,
    ONLY_REAR,
    F_TOP_R_BOTTOM,
    F_LEFT_R_RIGHT,
    PIP_SWAP_TYPE_NUM
} PIP_SWAP_TYPE;

#if defined(PCAM_UVC_MIX_MODE_ENABLE) && PCAM_UVC_MIX_MODE_ENABLE
#define UVC_XU_RECORDTIME_CONTROL   (0x01)
#define UVC_XU_RECORDRES_CONTROL    (0x02)
#define UVC_XU_FILE_CONTROL         (0x03)
#define UVC_XU_PICTURE_CONTROL      (0x04)
#define UVC_XU_GSENSOR_CONTROL      (0x05)
#define UVC_XU_AUDIO_CONTROL        (0x06)
#define UVC_XU_REC_STATUS_CONTROL   (0x07)
#define UVC_XU_REC_MODE_CONTROL     (0x08)
#define UVC_XU_FIRMWARE_CONTROL     (0x09)
#define UVC_XU_MMC_CONTROL          (0x0A)
#define UVC_XU_SWITCH_MSDC_MODE     (0x0B)
#endif

/*===========================================================================
 * Function prototype
 *===========================================================================*/ 

void 		StateVideoRecHDMIMode(UINT32 ulEvent);
void 		StateVideoRecTVMode(UINT32 ulEvent);
UINT32 StateVideoRecModeHandler(UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam );

UINT32 		VideoFunc_GetRecordTimeOffset(void);
void 		VideoRecMode_PreviewUpdate(void);
AHC_BOOL 	VideoRecMode_Start(void);
AHC_BOOL 	VideoFunc_RecordStatus(void);
void 		VideoFunc_GetFreeSpace(UINT64 *pFreeBytes);
AHC_BOOL    VideoFunc_ExitVMDMode(void);
AHC_BOOL 	VideoFunc_ZoomOperation(AHC_ZOOM_DIRECTION bZoomDir);
void 		VideoFunc_PresetSensorMode(MOVIESIZE_SETTING ubResolution);
void 		VideoFunc_PresetFrameRate(MOVIESIZE_SETTING ubResolution);
AHC_BOOL 	VideoFunc_RecordStatus(void);
AHC_BOOL 	VideoFunc_RecordRestart(void);
AHC_BOOL 	VideoFunc_RecordStop(void);
void VideoTimer_Event_Busy(AHC_BOOL bBusy);
AHC_BOOL VideoTimer_Event_IsBusy(void);
AHC_BOOL 	VideoTimer_Start(UINT32 ulTime);
AHC_BOOL 	VideoTimer_Stop(void);
AHC_BOOL 	VideoFunc_Preview(void);
AHC_BOOL 	VideoFunc_PreRecordStatus(void); 
AHC_BOOL 	VideoFunc_SetFileLock(void);
AHC_BOOL 	VideoFunc_Shutter(void);
AHC_BOOL 	VideoFunc_ShutterFail(void);
AHC_BOOL    VideoFunc_LockFileEnabled(void);
void        StateVideoRecMode_StartRecordingProc(UINT32 ulJobEvent);
void        StateVideoRecMode_StopRecordingProc(UINT32 ulJobEvent);
AHC_BOOL	VideoRec_TriggeredByVMD(void);
AHC_BOOL    VideoFunc_IsShareRecordStarted(void);

AHC_BOOL StateVideoRecModeInitLCD(void* pData);
#if (HDMI_PREVIEW_EN)
AHC_BOOL StateVideoRecModeInitHDMI(void* pData);
#endif
#if (TVOUT_PREVIEW_EN)	
AHC_BOOL StateVideoRecModeInitTV(void* pData);
#endif
AHC_BOOL VideoFunc_Init_NoDisplay(void* pData);

AHC_BOOL StateVideoRecModeShutDown(void* pData);

AHC_BOOL StateSelectFuncVideoRecordMode(void);

#if (ENABLE_ADAS_LDWS || ENABLE_ADAS_FCWS || ENABLE_ADAS_SAG)
AHC_BOOL AHC_VIDEO_SetRecordModeInitADASMode(void);
AHC_BOOL AHC_VIDEO_SetRecordModeUnInitADASMode(void);
#endif
AHC_BOOL AHC_VIDEO_ParkingModeStart(void);
AHC_BOOL AHC_VIDEO_ParkingModeStop(void);
AHC_BOOL AHC_VIDEO_SetRecordModeInitParkingMode(void);
AHC_BOOL AHC_VIDEO_SetRecordModeUnInitParkingMode(void);
AHC_BOOL AHC_VIDEO_SetRecordModeSetEmergencyParkingMode(void);
AHC_BOOL AHC_VIDEO_SetRecordModeSetAudioEncodeParkingMode(void);
AHC_BOOL AHC_VIDEO_SetRecordModeSetTimeLimitParkingMode(void);
AHC_BOOL AHC_VIDEO_SetRecordModeSetBitRateParkingMode(void);
void VRMotionDetectCB(MMP_UBYTE ubSnrSel);
void VRFileFullCBParkingMode(void);
AHC_BOOL AHC_VIDEO_SetRecordModeRegisterCallbackParkingMode(void);
AHC_BOOL AHC_VIDEO_CyclicDeleteFilesParkingMode(void);
UINT32 AHC_VIDEO_GetRecTimeLimitParkingMode(void);
AHC_BOOL AHC_VIDEO_SetRecordModeSetSeamlessParkingMode(void);
AHC_BOOL    AHC_VIDEO_ParkingModePostProcess(void);
#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE == 1)
AHC_BOOL VideoFunc_UVCXUCmdRegisterHandler(void); 
#endif
#endif //_STATEVIDEOFUNC_H_

