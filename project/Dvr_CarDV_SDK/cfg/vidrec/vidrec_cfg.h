//------------------------------------------------------------------------------
//
//  File        : vidrec_cfg.h
//  Description : Header file of Video Record configuration
//  Author      : Eroy
//  Revision    : 0.1
//
//------------------------------------------------------------------------------

#ifndef _VIDREC_CFG_H_
#define _VIDREC_CFG_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "lib_retina.h"
#include "mmp_display_inc.h"

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

// Video Current Buffer Mode (Map to VIDENC_CURBUF_MODE)
typedef enum _VIDREC_CURBUF_MODE {
    VIDREC_CURBUF_FRAME,
    VIDREC_CURBUF_RT,
    VIDREC_CURBUF_MAX
} VIDREC_CURBUF_MODE;

// H264 Video Profile (Map to VIDENC_PROFILE)
typedef enum _VIDREC_H264_PROFILE {
    VIDREC_H264_PROFILE_NONE = 0,
    VIDREC_H264_BASELINE_PROFILE,
    VIDREC_H264_MAIN_PROFILE,
    VIDREC_H264_HIGH_PROFILE,
    VIDREC_H264_PROFILE_MAX
} VIDREC_H264_PROFILE;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct {
    VIDREC_CURBUF_MODE          eCurBufMode;
    VIDREC_H264_PROFILE         eProfile;
    MMP_UBYTE                   ubPreviewPipe;
    MMP_UBYTE                   ubEncH264Pipe;
    MMP_DISPLAY_WIN_ID          eDispWinId;
    MMP_BOOL                    bPreviewActive;
} VIDREC_CFG;

//==============================================================================
//
//                              EXTERN VARIABLES
//
//==============================================================================

extern VIDREC_CFG   gsVidRecCfg[];

#define GET_VR_PREVIEW_PIPE(cam)    gsVidRecCfg[cam].ubPreviewPipe
#define GET_VR_ENCODE_PIPE(cam)     gsVidRecCfg[cam].ubEncH264Pipe 

#define GET_VR_PREVIEW_WINDOW(cam)  gsVidRecCfg[cam].eDispWinId
#define GET_VR_PREVIEW_ACTIVE(cam)  gsVidRecCfg[cam].bPreviewActive

#define GET_VR_ENCODE_BUFMODE(cam)  gsVidRecCfg[cam].eCurBufMode
#define GET_VR_ENCODE_PROFILE(cam)  gsVidRecCfg[cam].eProfile

//==============================================================================
//
//                              Function
//
//==============================================================================

void MMP_InitVidRecConfig(void);
VIDREC_CFG* MMP_GetVidRecConfig(MMP_UBYTE ubCamSel);

#endif // _VIDREC_CFG_H_

