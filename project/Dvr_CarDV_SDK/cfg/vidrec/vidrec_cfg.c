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
#include "vidrec_cfg.h"
#include "snr_cfg.h"
#include "mmp_vidrec_inc.h"
#include "mmp_snr_inc.h"
#include "mmp_ibc_inc.h"
#include "mmps_3gprecd.h"

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

/*
 * Configure of Video Record
 */
VIDREC_CFG gsVidRecCfg[MAX_CAM_NUM] = {
    {   // PRM_CAM
        VIDREC_CURBUF_FRAME,                    // eCurBufMode
        VIDREC_H264_BASELINE_PROFILE,           // eProfile
        MMP_IBC_PIPE_3,                         // ubPreviewPipe
        MMP_IBC_PIPE_0,                         // ubEncH264Pipe
        MMP_DISPLAY_WIN_PIP,/*LOWER_WINDOW*/    // eDispWinId
        MMP_TRUE,                               // bPreviewActive
    },
    {   // SCD_CAM
        VIDREC_CURBUF_FRAME,                    // eCurBufMode
        VIDREC_H264_BASELINE_PROFILE,           // eProfile
        MMP_IBC_PIPE_2,                         // ubPreviewPipe
        MMP_IBC_PIPE_1,                         // ubEncH264Pipe
        MMP_DISPLAY_WIN_OVERLAY,/*UPPER_WINDOW*/// eDispWinId
        MMP_TRUE,                               // bPreviewActive
    },
    {   // USBH_CAM
        VIDREC_CURBUF_FRAME,                    // eCurBufMode
        VIDREC_H264_BASELINE_PROFILE,           // eProfile
        MMP_IBC_PIPE_2,                         // ubPreviewPipe
        MMP_IBC_PIPE_1,                         // ubEncH264Pipe
        MMP_DISPLAY_WIN_OVERLAY,/*UPPER_WINDOW*/// eDispWinId
        MMP_TRUE,                               // bPreviewActive
    },
};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void _____VideoRec_Functions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMP_InitVidRecConfig
//  Description :
//------------------------------------------------------------------------------
void MMP_InitVidRecConfig(void)
{
	#if defined(ALL_FW)
    int i = 0;
    #endif
    
    if (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
        gsVidRecCfg[PRM_SENSOR].eCurBufMode     = VIDREC_CURBUF_FRAME;
        gsVidRecCfg[SCD_SENSOR].eCurBufMode     = VIDREC_CURBUF_FRAME; //VIDREC_CURBUF_RT-->(VIDREC_CURBUF_FRAME for sScdCam = SCD_CAM_YUV_SENSOR;)
    }
    else {
        gsVidRecCfg[PRM_SENSOR].eCurBufMode     = VIDREC_CURBUF_FRAME;  
        gsVidRecCfg[SCD_SENSOR].eCurBufMode     = VIDREC_CURBUF_FRAME;
    }
    
    gsVidRecCfg[USBH_SENSOR].eCurBufMode    = VIDREC_CURBUF_FRAME;
    
    gsVidRecCfg[PRM_SENSOR].eProfile        = VIDREC_H264_BASELINE_PROFILE;
    gsVidRecCfg[SCD_SENSOR].eProfile        = VIDREC_H264_BASELINE_PROFILE;
    gsVidRecCfg[USBH_SENSOR].eProfile       = VIDREC_H264_BASELINE_PROFILE;
    
    #if defined(ALL_FW) // TBD

    if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
    {//Change encode pipe when no Prm Cam, Sonix rear cam only
    	gsVidRecCfg[PRM_SENSOR].ubEncH264Pipe = MMP_IBC_PIPE_1;
    	gsVidRecCfg[USBH_SENSOR].ubEncH264Pipe = MMP_IBC_PIPE_0;
    }

    for (i = 0; i < MAX_CAM_NUM; i++) {
        MMPS_3GPRECD_SetPreviewPipe(i, gsVidRecCfg[i].ubPreviewPipe);
        MMPS_3GPRECD_SetEncodePipe(i, gsVidRecCfg[i].ubEncH264Pipe);
    }
    #endif
        
    #if 0
    if (gsVidRecCfg[ubCamSel].eCurBufMode == VIDREC_CURBUF_RT &&
        gsVidRecCfg[ubCamSel].eProfile    == VIDREC_H264_HIGH_PROFILE) {
        // RTNA_DBG_Str(0, "ERROR!, Not support H264 high profile with realtime encode!\r\n");
    }
    #endif        
}

//------------------------------------------------------------------------------
//  Function    : MMP_GetVidRecConfig
//  Description :
//------------------------------------------------------------------------------
VIDREC_CFG* MMP_GetVidRecConfig(MMP_UBYTE ubCamSel)
{
    return &gsVidRecCfg[ubCamSel];
}

