//------------------------------------------------------------------------------
//
//  File        : snr_cfg.c
//  Description : Source file of Sensor configuration
//  Author      : Eroy
//  Revision    : 0.1
//
//------------------------------------------------------------------------------

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "customer_config.h"
#include "snr_cfg.h"

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

/*
 * Configure of Dual Sensor
 */
DUAL_SNR_CFG gsDualSnrCfg = {
    MMP_FALSE,                  // bDualSnrEn
    MMP_FALSE,                  // bIspTimeSharingEn
    DUALSNR_DUAL_PREVIEW,       // sPrevwType
    DUALSNR_DUAL_ENCODE,        // sEncodeType
    DUALSNR_DUALENC_FRM_FRM,    // sDualEncBufMode
    PARALLEL_FRM_NOT_SUPPORT,   // sParallelFrmStoreType
    MMP_FALSE,                  // bSupportDualFrmPcamOut
};

/*
 * Configure of TV Decoder Sensor
 */
TVDEC_SNR_CFG gsTvDecSnrCfg = {
    MMP_FALSE,                  // bUseDMADeinterlace
    MMP_FALSE,                  // bUseVifCntAsFieldCnt
    MMP_FALSE                   // bRawStorePathEnable
};

/*
 * Configure of ISP Mode
 */
ISP_STATISTIC_MODE      gubIspStatisticMode = ISP_STATISTIC_MODE_DRIVER;

/*
 * Configure of AIT Cam Stream Type
 */
AIT_REAR_CAM_STRM_TYPE  gsAitCamStreamType = AIT_REAR_CAM_STRM_MJPEG_H264;

/*
 * Configure of Primary Cam
 */
PRM_CAM_ID sPrmCam = PRM_CAM_NONE; 
//PRM_CAM_ID sPrmCam = PRM_CAM_BAYER_SENSOR;
//PRM_CAM_ID sPrmCam = PRM_CAM_YUV_SENSOR;
//PRM_CAM_ID sPrmCam = PRM_CAM_TV_DECODER;

/*
 * Configure of Secondary Cam
 */
//SCD_CAM_ID sScdCam = SCD_CAM_NONE;
SCD_CAM_ID sScdCam = SCD_CAM_TV_DECODER;
//SCD_CAM_ID sScdCam = SCD_CAM_BAYER_SENSOR;
//SCD_CAM_ID sScdCam = SCD_CAM_YUV_SENSOR;

/*
 * Configure of USB Cam
 */
//USB_CAM_ID sUSBCam = USB_CAM_NONE;
//USB_CAM_ID sUSBCam = USB_CAM_AIT;
//USB_CAM_ID sUSBCam = USB_CAM_SONIX_MJPEG;
USB_CAM_ID sUSBCam = USB_CAM_NONE;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void _____DualSnr_Functions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMP_InitISPModeConfig
//  Description :
//------------------------------------------------------------------------------
void MMP_InitISPModeConfig(MMP_UBYTE ubCamSel)
{
    if (ubCamSel == SCD_CAM_BAYER_SENSOR)
        gubIspStatisticMode = ISP_STATISTIC_MODE_AGGREGATE;
    else
        gubIspStatisticMode = ISP_STATISTIC_MODE_DRIVER;
}

//------------------------------------------------------------------------------
//  Function    : MMP_InitDualSnrConfig
//  Description :
//------------------------------------------------------------------------------
void MMP_InitDualSnrConfig(MMP_UBYTE ubScdCamSel)
{
    switch (ubScdCamSel) {
    case SCD_CAM_TV_DECODER:
        gsDualSnrCfg.bDualVifCamEn          = MMP_TRUE;
        gsDualSnrCfg.bIspTimeSharingEn      = MMP_FALSE;
        gsDualSnrCfg.sPrevwType             = DUALSNR_DUAL_PREVIEW;
        gsDualSnrCfg.sEncodeType            = DUALSNR_DUAL_ENCODE;
        gsDualSnrCfg.sDualEncBufMode        = DUALSNR_DUALENC_FRM_FRM;
        gsDualSnrCfg.sParallelFrmStoreType  = PARALLEL_FRM_NOT_SUPPORT;
        gsDualSnrCfg.bSupportDualSnrPcamOut = MMP_FALSE;
    break;
    case SCD_CAM_YUV_SENSOR:
        gsDualSnrCfg.bDualVifCamEn          = MMP_TRUE;
        gsDualSnrCfg.bIspTimeSharingEn      = MMP_FALSE;
        gsDualSnrCfg.sPrevwType             = DUALSNR_DUAL_PREVIEW;
        gsDualSnrCfg.sEncodeType            = DUALSNR_DUAL_ENCODE;
        gsDualSnrCfg.sDualEncBufMode        = DUALSNR_DUALENC_FRM_RT;//DUALSNR_DUALENC_RT_FRM;//DUALSNR_DUALENC_FRM_FRM;
        gsDualSnrCfg.sParallelFrmStoreType  = PARALLEL_FRM_NOT_SUPPORT;
        gsDualSnrCfg.bSupportDualSnrPcamOut = MMP_FALSE;
    break;
    case SCD_CAM_BAYER_SENSOR:
        gsDualSnrCfg.bDualVifCamEn          = MMP_TRUE;
        gsDualSnrCfg.bIspTimeSharingEn      = MMP_TRUE;
        gsDualSnrCfg.sPrevwType             = DUALSNR_SINGLE_PREVIEW;
        gsDualSnrCfg.sEncodeType            = DUALSNR_SINGLE_ENCODE;//DUALSNR_DUAL_ENCODE;
        gsDualSnrCfg.sDualEncBufMode        = DUALSNR_DUALENC_FRM_FRM;//DUALSNR_DUALENC_RT_RT
        gsDualSnrCfg.sParallelFrmStoreType  = PARALLEL_FRM_LEFT_RIGHT;
        gsDualSnrCfg.bSupportDualSnrPcamOut = MMP_TRUE;
    break;
    case SCD_CAM_NONE:
    default:
        gsDualSnrCfg.bDualVifCamEn          = MMP_FALSE;
        gsDualSnrCfg.bIspTimeSharingEn      = MMP_FALSE;
        gsDualSnrCfg.sPrevwType             = DUALSNR_DUAL_PREVIEW;
        gsDualSnrCfg.sEncodeType            = DUALSNR_DUAL_ENCODE;
        gsDualSnrCfg.sDualEncBufMode        = DUALSNR_DUALENC_FRM_FRM;
        gsDualSnrCfg.sParallelFrmStoreType  = PARALLEL_FRM_NOT_SUPPORT;
        gsDualSnrCfg.bSupportDualSnrPcamOut = MMP_FALSE;
    break;
    }
    
    if (ubScdCamSel == SCD_CAM_TV_DECODER) {
		#if defined(TVDEC_USE_AHD_SENSOR_R)&&(TVDEC_USE_AHD_SENSOR_R)//AHD use as Tvdec rear cam
        gsTvDecSnrCfg.bUseDMADeinterlace    = MMP_FALSE;
        gsTvDecSnrCfg.bUseVifCntAsFieldCnt  = MMP_FALSE;
        #else
		gsTvDecSnrCfg.bUseDMADeinterlace    = MMP_TRUE;
        gsTvDecSnrCfg.bUseVifCntAsFieldCnt  = MMP_TRUE;
		#endif
        gsTvDecSnrCfg.bRawStorePathEnable   = MMP_TRUE;
    }
	else if(ubScdCamSel == SCD_CAM_YUV_SENSOR){//AHD use as YUV rear cam.
		gsTvDecSnrCfg.bUseDMADeinterlace    = MMP_FALSE;
        gsTvDecSnrCfg.bUseVifCntAsFieldCnt  = MMP_FALSE;
        gsTvDecSnrCfg.bRawStorePathEnable   = MMP_TRUE;
	}
    else {
        gsTvDecSnrCfg.bUseDMADeinterlace    = MMP_FALSE;
        gsTvDecSnrCfg.bUseVifCntAsFieldCnt  = MMP_FALSE;
        gsTvDecSnrCfg.bRawStorePathEnable   = MMP_FALSE;
    }
}

//------------------------------------------------------------------------------
//  Function    : MMP_IsDualVifCamEnable
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMP_IsDualVifCamEnable(void)
{
    return gsDualSnrCfg.bDualVifCamEn;
}

//------------------------------------------------------------------------------
//  Function    : MMP_IsIspTimeSharingEnable
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMP_IsIspTimeSharingEnable(void)
{
    return gsDualSnrCfg.bIspTimeSharingEn;
}

//------------------------------------------------------------------------------
//  Function    : MMP_GetDualSnrPrevwType
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE MMP_GetDualSnrPrevwType(void)
{
    return gsDualSnrCfg.sPrevwType;
}

//------------------------------------------------------------------------------
//  Function    : MMP_GetDualSnrEncodeType
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE MMP_GetDualSnrEncodeType(void)
{
    return gsDualSnrCfg.sEncodeType;
}

//------------------------------------------------------------------------------
//  Function    : MMP_GetParallelFrmStoreType
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE MMP_GetParallelFrmStoreType(void)
{
    return gsDualSnrCfg.sParallelFrmStoreType;
}

//------------------------------------------------------------------------------
//  Function    : MMP_IsSupportDualSnrPcamOut
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMP_IsSupportDualSnrPcamOut(void)
{
    return gsDualSnrCfg.bSupportDualSnrPcamOut;
}

#if 0
void _____PrimaryCam_Functions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMP_GetPrmCamType
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE MMP_GetPrmCamType(void)
{
    return sPrmCam;
}

//------------------------------------------------------------------------------
//  Function    : MMP_SetPrmCamType
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMP_SetPrmCamType(PRM_CAM_ID eCamID)
{
    if (eCamID >= PRM_CAM_BAYER_SENSOR && 
        eCamID <= PRM_CAM_YUV_SENSOR)
    {
        sPrmCam = eCamID;
        return MMP_TRUE;
    }

    return MMP_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : MMP_IsPrmCamExist
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMP_IsPrmCamExist(void)
{
    return ((sPrmCam == PRM_CAM_TV_DECODER) ||\
            (sPrmCam == PRM_CAM_BAYER_SENSOR) ||\
            (sPrmCam == PRM_CAM_YUV_SENSOR));
}

#if 0
void _____SecondaryCam_Functions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMP_GetScdCamType
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE MMP_GetScdCamType(void)
{
    return sScdCam;
}

//------------------------------------------------------------------------------
//  Function    : MMP_SetScdCamType
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMP_SetScdCamType(SCD_CAM_ID eCamID)
{
    if (eCamID >= SCD_CAM_NONE && 
        eCamID <= SCD_CAM_YUV_SENSOR)
    {
        sScdCam = eCamID;
        return MMP_TRUE;
    }

    return MMP_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : MMP_IsScdCamExist
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMP_IsScdCamExist(void)
{
    return ((sScdCam == SCD_CAM_TV_DECODER) ||\
            (sScdCam == SCD_CAM_BAYER_SENSOR) ||\
            (sScdCam == SCD_CAM_YUV_SENSOR));
}

#if 0
void _____USBCam_Functions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMP_GetUSBCamType
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE MMP_GetUSBCamType(void)
{
    return sUSBCam;
}

//------------------------------------------------------------------------------
//  Function    : MMP_SetScdCamType
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMP_SetUSBCamType(USB_CAM_ID eCamID)
{
    if (eCamID >= USB_CAM_NONE && 
        eCamID <= USB_CAM_SONIX_MJPEG2H264)
    {
        sUSBCam = eCamID;
        return MMP_TRUE;
    }

    return MMP_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : MMP_IsUSBCamExist
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMP_IsUSBCamExist(void)
{
    return ((sUSBCam == USB_CAM_AIT) ||\
            (sUSBCam == USB_CAM_SONIX_MJPEG) ||\
            (sUSBCam == USB_CAM_SONIX_MJPEG2H264));
}

//------------------------------------------------------------------------------
//  Function    : MMP_GetAitCamStreamType
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE MMP_GetAitCamStreamType(void)
{
    return gsAitCamStreamType;
}

#if 0
void _____TVDec_Functions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMP_GetTvDecSnrAttr
//  Description :
//------------------------------------------------------------------------------
TVDEC_SNR_CFG* MMP_GetTvDecSnrAttr(void)
{
    return &gsTvDecSnrCfg;
}

#if 0
void _____MiscFunctions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMP_CheckVideoDualRecordEnabled
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE MMP_CheckVideoDualRecordEnabled(MMP_UBYTE ubCamType)
{
    if (CAM_CHECK_SCD(SCD_CAM_NONE) && 
        CAM_CHECK_USB(USB_CAM_NONE)) {
        #if (SUPPORT_SHARE_REC)
        return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
        #else
        return DUAL_REC_DISABLE;
        #endif
    }

    if (ubCamType == CAM_TYPE_SCD) {
    
        switch(CAM_GET_SCD)
        {
        case SCD_CAM_TV_DECODER:
            return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
        break;
        case SCD_CAM_BAYER_SENSOR:
            if (DUALSNR_DUAL_ENCODE == MMP_GetDualSnrEncodeType())
                return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
            else
                return DUAL_REC_DISABLE;
        break;
        case SCD_CAM_YUV_SENSOR:
            return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
        break;
        default:
        break;
        }
    }
    else if (ubCamType == CAM_TYPE_USB) {
    
        switch(CAM_GET_USB)
        {
        case USB_CAM_AIT:
            return DUAL_REC_STORE_FILE;
        break;
        case USB_CAM_SONIX_MJPEG:
            return DUAL_REC_STORE_FILE;
        break;
        case USB_CAM_SONIX_MJPEG2H264:
            return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
        break;
        default:
        break;
        }
    }
    
    return DUAL_REC_DISABLE;
}

