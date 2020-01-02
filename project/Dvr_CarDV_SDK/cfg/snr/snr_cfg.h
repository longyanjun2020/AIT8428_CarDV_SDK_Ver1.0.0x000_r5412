//------------------------------------------------------------------------------
//
//  File        : snr_cfg.h
//  Description : Header file of Sensor configuration
//  Author      : Eroy
//  Revision    : 0.1
//
//------------------------------------------------------------------------------

#ifndef _SNR_CFG_H_
#define _SNR_CFG_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "lib_retina.h"

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum {
    DUALSNR_DUAL_PREVIEW = 0,   // Use dual pipe for preview
    DUALSNR_SINGLE_PREVIEW,     // Use single pipe for preview
    DUALSNR_PREVIEW_TYPE_NUM
} DUALSNR_PREVIEW_TYPE;

typedef enum {
    DUALSNR_DUAL_ENCODE = 0,    // Use dual pipe for encode
    DUALSNR_SINGLE_ENCODE,      // Use single pipe for encode
    DUALSNR_ENCODE_TYPE_NUM
} DUALSNR_ENCODE_TYPE;

typedef enum {
    DUALSNR_DUALENC_FRM_FRM = 0,
    DUALSNR_DUALENC_RT_RT,
    DUALSNR_DUALENC_RT_FRM,
    DUALSNR_DUALENC_FRM_RT,
    DUALSNR_DUALENC_BUF_MODE_NUM
} DUALSNR_DUALENC_BUF_MODE;

typedef enum {
    PARALLEL_FRM_LEFT_RIGHT = 0,
    PARALLEL_FRM_EQUIRETANGLE,
    PARALLEL_FRM_NOT_SUPPORT
} PARALLEL_FRM_STORE_TYPE;

typedef enum {
    CAM_TYPE_PRM = 0,
    CAM_TYPE_SCD,
    CAM_TYPE_USB,
    CAM_TYPE_NUM
} CAM_TYPE;

typedef enum {
    PRM_CAM_NONE = 0,
    PRM_CAM_BAYER_SENSOR,
    PRM_CAM_TV_DECODER,
    PRM_CAM_YUV_SENSOR	//isp bypass
} PRM_CAM_ID;

typedef enum {
    SCD_CAM_NONE = 0,
    SCD_CAM_BAYER_SENSOR,
    SCD_CAM_TV_DECODER,
    SCD_CAM_YUV_SENSOR
} SCD_CAM_ID;

typedef enum {
    USB_CAM_NONE = 0,
    USB_CAM_AIT,
    USB_CAM_SONIX_MJPEG,
    USB_CAM_SONIX_MJPEG2H264
} USB_CAM_ID;

typedef enum {
    AIT_REAR_CAM_STRM_NV12_H264	= 0,    // Preview: NV12, Save file: H264 
    AIT_REAR_CAM_STRM_MJPEG_H264,       // Preview: MJPEG, Save file: H264                     // USB Cam
    AIT_REAR_CAM_STRM_TYPE_NUM
} AIT_REAR_CAM_STRM_TYPE;

typedef enum {
    ISP_STATISTIC_MODE_INDIVIDUAL = 0,  /* Each sensor process statistic individually */
    ISP_STATISTIC_MODE_AGGREGATE,       /* All sensors share the result of statistic by average the ACC data */
    ISP_STATISTIC_MODE_DRIVER,          /* This is for backward compatible, sensor driver handles the statistic process */
    ISP_STATISTIC_MODE_NUM
} ISP_STATISTIC_MODE;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct {
    MMP_BOOL                    bDualVifCamEn;          // TRUE: Dual VIF sensor, FALSE: Single VIF sensor
    MMP_BOOL                    bIspTimeSharingEn;
    DUALSNR_PREVIEW_TYPE        sPrevwType;
    DUALSNR_ENCODE_TYPE         sEncodeType;
    DUALSNR_DUALENC_BUF_MODE    sDualEncBufMode;        // Remove later
    PARALLEL_FRM_STORE_TYPE	    sParallelFrmStoreType;
    MMP_BOOL                    bSupportDualSnrPcamOut;
} DUAL_SNR_CFG;

typedef struct {
    MMP_BOOL                    bUseDMADeinterlace;     // 1: Merge odd/even field to one frame
                                                        // 0: Duplicate one field to one frame
    MMP_BOOL                    bUseVifCntAsFieldCnt;
    MMP_BOOL                    bRawStorePathEnable;
} TVDEC_SNR_CFG;

//==============================================================================
//
//                              EXTERN VARIABLES
//
//==============================================================================

extern DUAL_SNR_CFG             gsDualSnrCfg;
extern TVDEC_SNR_CFG            gsTvDecSnrCfg;
extern ISP_STATISTIC_MODE       gubIspStatisticMode;
extern AIT_REAR_CAM_STRM_TYPE   gsAitCamStreamType;

extern PRM_CAM_ID               sPrmCam;
extern SCD_CAM_ID               sScdCam;
extern USB_CAM_ID               sUSBCam;

#define CAM_SELECT(p, s, u)     sPrmCam = p; sScdCam = s; sUSBCam = u;

#define CAM_CHECK(p, s, u)      (sPrmCam == p) && (sScdCam == s) && (sUSBCam == u) 
#define CAM_CHECK_PRM(p)        (sPrmCam == p)
#define CAM_CHECK_SCD(s)        (sScdCam == s)
#define CAM_CHECK_USB(u)        (sUSBCam == u)

#define CAM_GET_PRM             (sPrmCam)
#define CAM_GET_SCD             (sScdCam)
#define CAM_GET_USB             (sUSBCam)

//==============================================================================
//
//                              Function
//
//==============================================================================

void MMP_InitISPModeConfig(MMP_UBYTE ubRearCamSel);
void MMP_InitDualSnrConfig(MMP_UBYTE ubRearCamSel);
MMP_BOOL MMP_IsDualVifCamEnable(void);
MMP_BOOL MMP_IsIspTimeSharingEnable(void);
MMP_UBYTE MMP_GetDualSnrEncodeType(void);
MMP_UBYTE MMP_GetDualSnrPrevwType(void);
MMP_UBYTE MMP_GetParallelFrmStoreType(void);
MMP_BOOL MMP_IsSupportDualSnrPcamOut(void);

MMP_UBYTE MMP_GetPrmCamType(void);
MMP_BOOL MMP_SetPrmCamType(PRM_CAM_ID eCamID);
MMP_BOOL MMP_IsPrmCamExist(void);

MMP_UBYTE MMP_GetScdCamType(void);
MMP_BOOL MMP_SetScdCamType(SCD_CAM_ID eCamID);
MMP_BOOL MMP_IsScdCamExist(void);

MMP_UBYTE MMP_GetUSBCamType(void);
MMP_BOOL MMP_SetUSBCamType(USB_CAM_ID eCamID);
MMP_BOOL MMP_IsUSBCamExist(void);
MMP_UBYTE MMP_GetAitCamStreamType(void);

TVDEC_SNR_CFG* MMP_GetTvDecSnrAttr(void);

MMP_UBYTE MMP_CheckVideoDualRecordEnabled(MMP_UBYTE ubCamType);

#endif // _SNR_CFG_H_

