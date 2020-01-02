/// @ait_only
//==============================================================================
//
//  File        : mmpd_fctl.h
//  Description : INCLUDE File for the Host Flow Control Driver.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmpd_fctl.h
 *  @brief The header File for the Host Flow Control Driver
 *  @author Penguin Torng
 *  @version 1.0
 */

#ifndef _MMPD_FCTL_H_
#define _MMPD_FCTL_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_lib.h"
#include "mmp_ibc_inc.h"
#include "mmp_dsc_inc.h"
#include "mmpd_ibc.h"
#include "mmpd_icon.h"
#include "mmpd_display.h"
#include "mmpd_graphics.h"

/** @addtogroup MMPD_FCtl
 *  @{
 */

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define MAX_PIPELINE_NUM    	(5)

#define MAX_FCTL_BUF_NUM		(4)
#define MAX_FCTL_ROT_BUF_NUM	(4)

#define FCTL_PIPE_TO_LINK(p, flink)	{	flink.scalerpath = (MMP_SCAL_PIPEID) p; \
										flink.icopipeID	 = (MMP_ICO_PIPEID)p; \
										flink.ibcpipeID  = (MMP_IBC_PIPEID)p; \
									}	

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum  _MMP_PIPE_USAGE
{
    PIPE_PREVIEW,
    PIPE_ENCODE_H264,
    PIPE_MAX_USAGE
} MMP_PIPE_USAGE;

typedef enum _MMPD_FCTL_GRA2JPEG_TYPE {
    MMPD_FCTL_GRA2JPEG_STILLCAP_FRONTCAM = 0,
    MMPD_FCTL_GRA2JPEG_STILLCAP_SONIXREARCAM,
    MMPD_FCTL_GRA2JPEG_MJEPG
}MMPD_FCTL_STILLCAP_TYPE;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct  _MMPD_FCTL_LINK
{
    MMP_SCAL_PIPEID         scalerpath;
   	MMP_ICO_PIPEID          icopipeID;
    MMP_IBC_PIPEID          ibcpipeID;
} MMP_PIPE_LINK;

typedef struct _MMPD_FCTL_ATTR 
{
    MMP_UBYTE               ubPipeLinkedSnr;
    MMP_PIPE_LINK           fctllink;
    MMP_SCAL_SOURCE         scalsrc;
    MMP_SCAL_FIT_RANGE      fitrange;
    MMP_SCAL_GRAB_CTRL      grabctl;
    MMP_SCAL_DELAY          sScalDelay;
    MMP_SCAL_COLRMTX_MODE   eScalColorRange;
    MMP_BOOL                bSetScalerSrc;
    MMP_DISPLAY_COLORMODE   colormode;
    MMP_BOOL                bRtModeOut;
    
    MMP_USHORT              usBufCnt;
    MMP_ULONG               ulBaseAddr[MAX_FCTL_BUF_NUM];
    MMP_ULONG               ulBaseUAddr[MAX_FCTL_BUF_NUM];
    MMP_ULONG               ulBaseVAddr[MAX_FCTL_BUF_NUM];
    
    MMP_BOOL                bUseRotateDMA;                          // Use rotate DMA to rotate or not
    MMP_ULONG               ulRotateAddr[MAX_FCTL_ROT_BUF_NUM];     // Dest Y buffer address for rotate DMA
    MMP_ULONG               ulRotateUAddr[MAX_FCTL_ROT_BUF_NUM];    // Dest U buffer address for rotate DMA
    MMP_ULONG               ulRotateVAddr[MAX_FCTL_ROT_BUF_NUM];    // Dest V buffer address for rotate DMA
    MMP_USHORT              usRotateBufCnt;                         // Dest buffer count for rotate DMA
} MMPD_FCTL_ATTR;

typedef struct _MMP_PIPE_ATTR 
{
    MMP_UBYTE               ubPipeId;
    MMP_SCAL_FIT_RANGE      sFitrange;
    MMP_SCAL_GRAB_CTRL      sGrabctl;
    MMP_SCAL_DELAY          sScalDelay;
    MMP_SCAL_COLRMTX_MODE   eScalColorRange;
    MMP_DISPLAY_COLORMODE   eColormode;
    MMP_IBC_PIPE_ATTR       sIbcAttr;
} MMP_PIPE_ATTR;

// For Component Control
typedef struct _MMP_TRIG_GRA_ATTR 
{
    MMP_PIPE_ATTR           sPipeAttr[PIPE_MAX_USAGE];
    MMP_GRAPHICS_BUF_ATTR   sGraBufAttr;
    MMP_GRAPHICS_RECT       sGraRect;
    MMP_UBYTE               ubGraPixDelayN;
    MMP_UBYTE               ubGraPixDelayM;
    MMP_USHORT              usGraLineDelay;
} MMP_TRIG_GRA_ATTR;

typedef struct _MMP_TRIG_JPGDEC_ATTR 
{
    MMP_PIPE_ATTR           sPipeAttr[PIPE_MAX_USAGE];
    MMP_ULONG               ulLineBufAddr;          ///< Decode Line Buffer Address
    MMP_ULONG               ulDCompBufAddr;         ///< Decode DeCompress Buffer Address
    MMP_ULONG               ulDCompBufSize;         ///< Decode DeCompress Buffer Size
    JpegDecCallBackFunc     *pfDecCBFunc;           ///< Decode Callback Function
    void*                   pDecCBFuncArg;          ///< Decode Callback Function Argument
} MMP_TRIG_JPGDEC_ATTR;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_ERR MMPD_Fctl_RawBuf2IbcBuf(MMP_PIPE_LINK           *fctllink, 
                                MMP_GRAPHICS_BUF_ATTR   *srcBufAttr, 
                                MMP_GRAPHICS_RECT       *rect, 
                                MMP_GRAPHICS_BUF_ATTR   *dstBufAttr, 
                                MMP_USHORT              usUpscale);

MMP_ERR MMPD_Fctl_SetPipeAttrForIbcFB(MMPD_FCTL_ATTR *pAttr);
MMP_ERR MMPD_Fctl_SetPipeAttrForH264FB(MMPD_FCTL_ATTR *pAttr);
MMP_ERR MMPD_Fctl_SetPipeAttrForH264Rt(MMPD_FCTL_ATTR *pAttr, MMP_ULONG ulEncWidth);
MMP_ERR MMPD_Fctl_SetPipeAttrForJpeg(MMPD_FCTL_ATTR         *pAttr,
                                    MMP_BOOL                bSetScalerGrab,
                                    MMP_BOOL                bSrcIsBT601);
MMP_ERR MMPD_Fctl_SetSubPipeAttr(MMPD_FCTL_ATTR *pAttr);

MMP_ERR MMPD_Fctl_GetAttributes(MMP_IBC_PIPEID pipeID, MMPD_FCTL_ATTR *pAttr);
MMP_ERR MMPD_Fctl_ClearPreviewBuf(MMP_IBC_PIPEID pipeID, MMP_ULONG ulClearColor);
MMP_ERR MMPD_Fctl_EnablePreview(MMP_UBYTE       ubSnrSel, 
                                MMP_IBC_PIPEID  pipeID,
                                MMP_BOOL        bEnable,
                                MMP_BOOL        bCheckFrameEnd);
MMP_ERR MMPD_Fctl_ResetIBCBufIdx(MMP_USHORT usIBCPipe);

/* IBC Link Type */
MMP_ERR MMPD_Fctl_GetIBCLinkAttr(MMP_IBC_PIPEID             pipeID, 
								 MMP_IBC_LINK_TYPE          *IBCLinkType,
								 MMP_DISPLAY_DEV_TYPE       *previewdev,
								 MMP_DISPLAY_WIN_ID         *winID,
								 MMP_DISPLAY_ROTATE_TYPE    *rotateDir);
MMP_ERR MMPD_Fctl_RestoreIBCLinkAttr(MMP_IBC_PIPEID             pipeID, 
									 MMP_IBC_LINK_TYPE          IBCLinkType,
									 MMP_DISPLAY_DEV_TYPE       previewdev,
									 MMP_DISPLAY_WIN_ID         winID,
									 MMP_DISPLAY_ROTATE_TYPE    rotateDir);

MMP_ERR MMPD_Fctl_ResetIBCLinkType(MMP_IBC_PIPEID pipeID);
MMP_ERR MMPD_Fctl_LinkPipeToUSB(MMP_IBC_PIPEID pipeID);
MMP_ERR MMPD_Fctl_LinkPipeToDma(    MMP_IBC_PIPEID          pipeID,
                                    MMP_DISPLAY_WIN_ID      winID,
                                    MMP_DISPLAY_DEV_TYPE    previewDev,
                                    MMP_DISPLAY_ROTATE_TYPE rotateDir);
MMP_ERR MMPD_Fctl_LinkPipeToDisplay(MMP_IBC_PIPEID          pipeID, 
                                    MMP_DISPLAY_WIN_ID      winID,
                                    MMP_DISPLAY_DEV_TYPE    previewDev);
MMP_ERR MMPD_Fctl_LinkPipeToVideo(MMP_IBC_PIPEID pipeID, MMP_USHORT ubEncId);
MMP_ERR MMPD_Fctl_LinkPipeToWifi(MMP_IBC_PIPEID pipeID, MMP_USHORT ubEncId);
MMP_ERR MMPD_Fctl_LinkPipeToLdc(MMP_IBC_PIPEID pipeID);
MMP_ERR MMPD_Fctl_UnLinkPipeToLdc(MMP_IBC_PIPEID pipeID);
MMP_ERR MMPD_Fctl_LinkPipeToMdtc(MMP_IBC_PIPEID pipeID);
MMP_ERR MMPD_Fctl_UnLinkPipeToMdtc(MMP_IBC_PIPEID pipeID);
MMP_ERR MMPD_Fctl_LinkPipeToGra2JPEG(MMP_IBC_PIPEID         pipeID,
								     MMP_DISPLAY_WIN_ID     winID,
									 MMP_DISPLAY_DEV_TYPE   previewDev,
									 MMP_BYTE               bStillCapture,
									 MMP_BOOL               bDisplayEnable);
MMP_ERR MMPD_Fctl_LinkPipeToGra2UVC(MMP_IBC_PIPEID pipeID);
MMP_ERR MMPD_Fctl_LinkPipeToMultiRunJpeg(MMP_IBC_PIPEID pipeID);
#if (HANDLE_JPEG_EVENT_BY_QUEUE)
MMP_ERR MMPD_Fctl_LinkPipeToGra2JPEGQ(  MMP_IBC_PIPEID	pipeID,
                                        MMP_USHORT 		usWidth,
                                        MMP_USHORT 		usHeight,
										MMP_BYTE		bWaitRearStickerDone);
#endif

/// @}

#endif // _MMPD_FCTL_H_

/// @end_ait_only
