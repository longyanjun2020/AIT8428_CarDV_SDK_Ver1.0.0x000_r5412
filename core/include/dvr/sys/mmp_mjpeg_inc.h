/// @ait_only
//==============================================================================
//
//  File        : mmp_mjpeg_inc.h
//  Description : INCLUDE File for the Common MJPEG Driver.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMP_MJPEG_INC_H_
#define _MMP_MJPEG_INC_H_

/** @addtogroup MMPF_DSC
 *  @{
 */

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_scal_inc.h"
#include "mmp_ibc_inc.h"
#include "mmp_dsc_inc.h"
#include "mmp_display_inc.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define MAX_MJPEG_STREAM_NUM		(2)

#define MAX_FRONT_CAM_ENC_COMPBUF_SZ    	(256 * 1024) // 256KB
#define MAX_FRONT_CAM_ENC_LINEBUF_SZ 	    (240 * 1024) // 240KB
#define MAX_REAR_CAM_ENC_COMPBUF_SZ     	(256 * 1024) // 256KB
#define MAX_REAR_CAM_ENC_LINEBUF_SZ 	    (240 * 1024) // 240KB

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

// Map to MMPF_STREAM_RING_ID
typedef enum _MMP_MJPEG_STREAM_ID {
    MMP_MJPEG_STREAM_FRONTCAM_VIDEO = 0,
    MMP_MJPEG_STREAM_REARCAM_VIDEO,
    MMP_MJPEG_STREAM_FRONTCAM_AUDIO,
    MMP_MJPEG_STREAM_ID_NUM
} MMP_MJPEG_STREAM_ID;

typedef enum _MMP_MJPEG_UI_MODE_ID {
    MMP_MJPEG_UI_MODE_WIFI_VR = 0,
    MMP_MJPEG_UI_MODE_WIFI_DSC,
    MMP_MJPEG_UI_MODE_UVC_VR,
    MMP_MJPEG_UI_MODE_UVC_DSC,    
    MMP_MJPEG_UI_MODE_ID_NUM
} MMP_MJPEG_UI_MODE_ID;

typedef enum _MMP_MJPEG_VIDEO_STREAM_EN {
    MMP_MJPEG_VIDEO_STREAM_DIS     = 0x00,
    MMP_MJPEG_VIDEO_STREAM_FCAM_EN = 0x01,
    MMP_MJPEG_VIDEO_STREAM_RCAM_EN = 0x02
} MMP_MJPEG_VIDEO_STREAM_EN;

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct _MMP_MJPEG_LINKATTR 
{
    MMP_IBC_PIPE_ATTR		IBCPipeAttr;
    MMP_IBC_LINK_TYPE       IBCLinkType;
    MMP_DISPLAY_DEV_TYPE	previewDev;
    MMP_DISPLAY_WIN_ID 		winID;
    MMP_DISPLAY_ROTATE_TYPE rotateDir;
    MMP_SCAL_SOURCE			scalerSrc;
} MMP_MJPEG_LINKATTR;

typedef struct _MMP_MJPEG_RATE_CTL 
{
    MMP_ULONG   			ulBitrate;
    MMP_USHORT  			FPSx10;		// 300 = 30FPS, 75 = 7.5 FPS
} MMP_MJPEG_RATE_CTL;

typedef struct _MMP_MJPEG_OBJ 
{
    MMP_USHORT		 		usEncID; 	// JPEG structure index
    MMP_MJPEG_UI_MODE_ID    sMJPGModeID;   
    MMP_ULONG 				ul4cc;
    MMP_IBC_PIPEID 			PipeID; 	// IBC pipeline being used
    MMP_MJPEG_LINKATTR	    *pLinkAttr;
} MMP_MJPEG_OBJ, *MMP_MJPEG_OBJ_PTR;

typedef struct _MMP_MJPEG_ENC_INFO 
{
    MMP_USHORT              usEncWidth;
    MMP_USHORT              usEncHeight;
    MMP_BOOL                bTargetCtl;
    MMP_BOOL                bLimitCtl;
    MMP_ULONG               bTargetSize;
    MMP_ULONG               bLimitSize;
    MMP_USHORT              bMaxTrialCnt;
    MMP_DSC_JPEG_QUALITY    Quality;
} MMP_MJPEG_ENC_INFO;

/// @}

#endif // _MMP_MJPEG_INC_H_

/// @end_ait_only
