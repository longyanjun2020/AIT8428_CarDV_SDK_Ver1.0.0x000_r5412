//==============================================================================
//
//  File        : mmps_iva.h
//  Description : INCLUDE File for the Intelligent video analysis function.
//  Author      :
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPS_IVA_H_
#define _MMPS_IVA_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_lib.h"
#include "ait_config.h"
#include "mmpd_fctl.h"
#include "mdtc_cfg.h"
#include "ldws_cfg.h"
#include "mmpf_vmd.h"
/** @addtogroup MMPS_IVA
@{
*/

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MMPS_IVA_FDTC_PATH {
    MMPS_IVA_FDTC_LOOPBACK = 0x00,	
    MMPS_IVA_FDTC_SUBPIPE,
    MMPS_IVA_FDTC_YUV420
} MMPS_IVA_FDTC_PATH;

typedef enum _MMPS_IVA_EVENT {
    MMPS_IVA_EVENT_MDTC = 0x0,
    MMPS_IVA_EVENT_TV_SRC_TYPE,
    MMPS_IVA_EVENT_MAX
} MMPS_IVA_EVENT;

typedef enum _MMPS_IVA_ADAS_FEATURE {
    MMPS_ADAS_LDWS = 0x0,
    MMPS_ADAS_FCWS,
    MMPS_ADAS_SAG,
    MMPS_ADAS_FEATURE_NUM
} MMPS_IVA_ADAS_FEATURE;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct _MMPS_FDTC_CONFIG {
    // ++ Face Detection
    MMP_BOOL                bFaceDetectEnable;                          ///< Need to support face detection function or not.
    MMP_BOOL                bSmileDetectEnable;                         ///< Need to support smile detection function or not.
    MMP_USHORT              usFaceDetectGrabScaleM;                     ///< The grab M factor for face detection scale path.
    /**	@brief	the maximum size of feature buffer for face detection. 
    			the buffer for Y data is used to detect face feature. */
    MMP_UBYTE               ubDetectFaceNum;                            ///< Maximum number of face to detect.
    MMP_UBYTE               ubFaceDetectInputBufCnt;                    ///< The number of FD frame buffers.
    MMPS_IVA_FDTC_PATH      faceDetectInputPath;                        ///< Face detection input path selection
    // -- Face Detection
} MMPS_FDTC_CONFIG;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_ERR  MMPS_Sensor_RegisterCallback(MMP_UBYTE ubSnrSel, MMPS_IVA_EVENT event, void *callback);

#if (defined(SUPPORT_SPEECH_RECOG)&&(SUPPORT_SPEECH_RECOG))
typedef void (*MMP_SpeechRecogCbFunc) (MMP_ULONG);
MMP_ERR MMPDS_InitializeSpeechRecog(MMP_ULONG workbuf, MMP_ULONG workbuf_len, MMP_ULONG freq, MMP_ULONG param1, MMP_UBYTE param2);
MMP_ERR MMPS_Sensor_StartSpeechRecog(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable);
#if defined(ALL_FW)
MMP_ERR MMPS_StartSpeechRecog(MMP_SpeechRecogCbFunc  CB);
#endif
#endif

#if (SUPPORT_MDTC)
MMP_ERR 	MMPS_Sensor_EnableVMD(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable);
MMP_BOOL 	MMPS_Sensor_IsVMDEnable(MMP_UBYTE ubSnrSel);
void     	MMPS_Sensor_SetVMDPipe(MMP_UBYTE ubSnrSel, MMP_IBC_PIPEID pipe);
MMP_ERR 	MMPS_Sensor_InitializeVMD(MMP_UBYTE ubSnrSel, MDTC_CFG *pMdtcCfg);
MMP_ERR  	MMPS_Sensor_AllocateVMDBuffer(MMP_UBYTE ubSnrSel, MMP_ULONG *ulStartAddr, MMP_BOOL bAllocate);
MMP_ERR  	MMPS_Sensor_GetVMDResolution(MMP_UBYTE ubSnrSel, MMP_ULONG *ulWidth, MMP_ULONG *ulHeight);
MMP_ERR  	MMPS_Sensor_StartVMD(MMP_UBYTE ubSnrSel, MMP_BOOL bStart);
MMP_BOOL 	MMPS_Sensor_IsVMDStarted(MMP_UBYTE ubSnrSel);
#endif

#if (SUPPORT_ADAS)
MMP_ERR 	MMPS_Sensor_EnableADAS(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable);
MMP_BOOL 	MMPS_Sensor_IsADASEnable(MMP_UBYTE ubSnrSel);
MMP_ERR		MMPS_Sensor_InitializeADAS(MMP_UBYTE ubSnrSel, LDWS_CFG *pLdwsCfg);
MMP_ERR		MMPS_Sensor_AllocateADASBuffer(MMP_UBYTE ubSnrSel, MMP_ULONG *ulStartAddr, MMP_BOOL bAllocate, MMP_ULONG ulDMABufSize);
MMP_ERR 	MMPS_Sensor_GetADASResolution(MMP_UBYTE ubSnrSel, MMP_ULONG *ulWidth, MMP_ULONG *ulHeight);
MMP_ERR 	MMPS_Sensor_SetADASFeatureEn(MMP_UBYTE ubSnrSel, MMPS_IVA_ADAS_FEATURE feature, MMP_BOOL bEnable);
MMP_BOOL 	MMPS_Sensor_GetADASFeatureEn(MMP_UBYTE ubSnrSel, MMPS_IVA_ADAS_FEATURE feature);
MMP_ERR  	MMPS_Sensor_StartADAS(MMP_UBYTE ubSnrSel, MMP_BOOL bStart);
MMP_BOOL 	MMPS_Sensor_IsADASStarted(MMP_UBYTE ubSnrSel);
#endif

/// @}
#endif // _MMPS_IVA_H_
