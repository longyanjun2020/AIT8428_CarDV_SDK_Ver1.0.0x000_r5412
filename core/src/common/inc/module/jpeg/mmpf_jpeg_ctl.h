//==============================================================================
//
//  File        : mmpf_jpeg_ctl.h
//  Description : INCLUDE File for the DSC Driver Function
//  Author      : Eroy Yang
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_JPEG_CTL_H_
#define _MMPF_JPEG_CTL_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "includes_fw.h"
#include "mmp_dsc_inc.h"
#include "mmp_graphics_inc.h"
#include "mmp_scal_inc.h"
#include "mmp_ibc_inc.h"
#include "mmp_vidrec_inc.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define MMPF_JPEG_MAX_QUEUE_SIZE    (40)

#define JPG_ENC_DONE_SEM_TIMEOUT    (80)
#define JPG_DEC_DONE_SEM_TIMEOUT    (40)
#define JPG_REQ_DONE_SEM_TIMEOUT   	(100)

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

/* JPEG Encode Type ID */
typedef enum _MMP_JPEG_ENC_TYPE {
    MMP_JPEG_ENC_TYPE_FRONTCAM_MJPEG = 0,
    MMP_JPEG_ENC_TYPE_REARCAM_MJPEG,
    MMP_JPEG_ENC_TYPE_FRONTCAM_STILL_JPEG,
    MMP_JPEG_ENC_TYPE_REARCAM_STILL_JPEG,
    MMP_JPEG_ENC_TYPE_FRONTCAM_VR_THUMB,
    MMP_JPEG_ENC_TYPE_REARCAM_VR_THUMB,
    MMP_JPEG_ENC_TYPE_DUALCAM_VR_THUMB,
    MMP_JPEG_ENC_TYPE_NUM
} MMP_JPEG_ENC_TYPE;

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct _MMPF_JPG_CTL_ENCATTR 
{
    MMP_GRAPHICS_BUF_ATTR   sGraBufAttr;            ///< Graphics Buffer Attribute

    MMP_SCAL_SOURCE         sEncScalSrc;            ///< Encode Path Scaler Source Selection
    MMP_ULONG               ulEncScalInW;           ///< Encode Path Scaler Input Width
    MMP_ULONG               ulEncScalInH;           ///< Encode Path Scaler Input Height
    MMP_ULONG               ulEncScalOutW;          ///< Encode Path Scaler Output Width
    MMP_ULONG               ulEncScalOutH;          ///< Encode Path Scaler Output Height
    
    MMP_USHORT              usEncId;                ///< Encode ID
    MMP_USHORT              usEncWidth;             ///< Encode JPEG Width
    MMP_USHORT              usEncHeight;            ///< Encode JPEG Height
    MMP_ULONG               ulLineBufAddr;          ///< Encode Line Buffer Address
    MMP_ULONG               ulCompBufAddr;          ///< Encode Compress Buffer Address
    MMP_ULONG               ulCompBufSize;          ///< Encode Compress Buffer Size
    MMP_USHORT              usQFactor;              ///< Encode Q (Scale) Factor
    JpegEncCallBackFunc     *pfEncCBFunc;           ///< Encode Callback Function
    void*                   pEncCBFuncArg;          ///< Encode Callback Function Argument
    
    MMP_UBYTE               ubPipeId;               ///< Pipe ID
    MMP_UBYTE               ubEncodeMode;           ///< MMPF_JPEG_ENCDONE_MODE    
    MMP_UBYTE               ubNeedBackUpGraphic;	///< If preview pipe is always on and via graphics,need to backup graphics source setting before JPEG encode
    MMP_UBYTE               ubBackUpGraPipeId;		///< LCD/WIFI preview pipe
} MMPF_JPG_CTL_ENCATTR;

typedef struct _MMPF_JPG_CTL_DECATTR 
{
    MMP_ULONG               ulDecScalInW;           ///< Decode Path Scaler Input Width
    MMP_ULONG               ulDecScalInH;           ///< Decode Path Scaler Input Height
    MMP_ULONG               ulDecScalOutW;          ///< Decode Path Scaler Output Width
    MMP_ULONG               ulDecScalOutH;          ///< Decode Path Scaler Output Height
    MMP_IBC_PIPE_ATTR       sIbcAttr;               ///< Decode Path IBC Attribute

    MMP_ULONG               ulLineBufAddr;          ///< Decode Line Buffer Address
    MMP_ULONG               ulDCompBufAddr;         ///< Decode DeCompress Buffer Address
    MMP_ULONG               ulDCompBufSize;         ///< Decode DeCompress Buffer Size
    JpegDecCallBackFunc     *pfDecCBFunc;           ///< Decode Callback Function
    void*                   pDecCBFuncArg;          ///< Decode Callback Function Argument
    
    MMP_UBYTE               ubPipeId;               ///< Pipe ID
} MMPF_JPG_CTL_DECATTR;

typedef struct _MMPF_JPG_CTL_ATTR 
{
    MMP_BOOL                bDecode;                ///< Decode or Encode Operation
    MMPF_JPG_CTL_ENCATTR    sEncAttr;               ///< Encode Attribute
    MMPF_JPG_CTL_DECATTR    sDecAttr;               ///< Decode Attribute
} MMPF_JPG_CTL_ATTR;

typedef struct _MMPF_JPG_CTL_QUEUE 
{
    MMPF_JPG_CTL_ATTR       attr[MMPF_JPEG_MAX_QUEUE_SIZE];     ///< Queue for ready to encode/decode
    MMP_ULONG               weight[MMPF_JPEG_MAX_QUEUE_SIZE];   ///< The times to encode the same frame
    MMP_ULONG               head;                               ///< Queue head index
    MMP_ULONG               size;                               ///< Queue size
    MMP_ULONG               weighted_size;
} MMPF_JPG_CTL_QUEUE;

// For AIT Cam VR Capture
typedef struct _MMPF_JPG_CTL_VR_CAPTURE_PARAM {
    volatile MMP_BOOL       bBuf2StillJPGDoneFlag;
    MMP_USHORT              usIBCPipe;
    MMP_USHORT              usJpegWidth;
    MMP_USHORT              usJpegHeight;
    MMP_ULONG               ulVRCaptureBaseBufAddr;
    MMP_ULONG               ulVRCaptureUBufAddr;
    MMP_ULONG               ulVRCaptureVBufAddr;
    MMP_ULONG               ulJpegLineStart;
    MMP_ULONG               ulJpegCompStart;
    MMP_ULONG               ulJpegCompEnd;
    MMP_ULONG               ulBuf2JPGJpegSize;
} MMPF_JPG_CTL_VR_CAPTURE_PARAM;

//===============================================================================
//
//                               EXTERN VARIABLES
//
//===============================================================================

extern MMPF_JPG_CTL_QUEUE m_sJpegCtlQueue;

//===============================================================================
//
//                               FUNCTION PROTOTYPES
//
//===============================================================================

void        MMPF_JPEGCTL_ResetQueue(MMPF_JPG_CTL_QUEUE *queue);
MMP_ULONG   MMPF_JPEGCTL_GetQueueDepth(MMPF_JPG_CTL_QUEUE *queue);
MMP_ERR     MMPF_JPEGCTL_PushQueue(MMPF_JPG_CTL_QUEUE *queue, MMPF_JPG_CTL_ATTR attr);
MMP_ERR     MMPF_JPEGCTL_PopQueue(MMPF_JPG_CTL_QUEUE *queue, MMP_ULONG offset, MMPF_JPG_CTL_ATTR *pData);

MMP_ERR     MMPF_JPEG_SetCtrlByQueueEnable(MMP_BOOL bEnable);
MMP_BOOL    MMPF_JPEG_GetCtrlByQueueEnable(void);
MMP_ERR     MMPF_JPEG_ExeEncode(MMPF_JPG_CTL_ATTR* pAttr);
MMP_ERR     MMPF_JPEG_ExeDecode(MMPF_JPG_CTL_ATTR* pAttr);
MMP_ERR     MMPF_JPEGCTL_ResetCtlSem(void);
MMP_ERR     MMPF_JPEGCTL_PausePreview(void);
MMP_ERR     MMPF_JPEGCTL_ResumePreview(void);

#if (SUPPORT_VR_THUMBNAIL)
MMP_ERR     MMPF_JPEGCTL_CapJpegThumb2Ring(MMP_ULONG ulRingType, MMP_ULONG ulSrcW, MMP_ULONG ulSrcH, MMP_ULONG ulJPGW, MMP_ULONG ulJPGH, MMP_ULONG ulWrap, MMP_ULONG ulIdx, VIDENC_FRAME_INFO *pFrameInfo);
MMP_ERR     MMPF_JPEGCTL_SwapThumbFromRing(MMP_ULONG ulRingType, MMP_ULONG ulThumbType, VIDENC_THUMB_ATTR* pTargetThumb, VIDENC_THUMB_TARGET_CTL* pTarIdxCtl);
#endif

MMP_ERR     MMPF_JPEGCTL_UseH264Cur2StillJpeg(MMP_USHORT usIBCPipe, MMP_ULONG ulSrcW, MMP_ULONG ulSrcH, MMP_ULONG ulJPGW, MMP_ULONG ulJPGH);
#if (HANDLE_JPEG_EVENT_BY_QUEUE)
void        MMPF_JPEGCTL_Buf2StillJpegDoneSetFlag(MMP_BOOL bFlag);
void        MMPF_JPEGCTL_Buf2StillJpegDoneGetFlag(MMP_BOOL *pbFlag);
void        MMPF_JPEGCTL_Buf2StillJpegSetPipe(MMP_USHORT usPipe);
void        MMPF_JPEGCTL_Buf2StillJpegGetPipe(MMP_USHORT *pusIBCPipe);
void        MMPF_JPEGCTL_Buf2StillJpegSetWidthHeight(MMP_USHORT usWidth, MMP_USHORT usHeight);
void        MMPF_JPEGCTL_Buf2StillJpegGetWidthHeight(MMP_USHORT *pusWidth, MMP_USHORT *pusHeight);
void        MMPF_JPEGCTL_Buf2StillJpegSetRawAddress(MMP_ULONG ulBaseBufAddr, MMP_ULONG ulUBufAddr, MMP_ULONG ulVBufAddr);
void        MMPF_JPEGCTL_Buf2StillJpegGetRawAddress(MMP_ULONG *pulBaseBufAddr, MMP_ULONG *pulUBufAddr, MMP_ULONG *pulVBufAddr);
void        MMPF_JPEGCTL_Buf2StillJpegSetCompressAddress(MMP_ULONG ulJpegLineStart, MMP_ULONG ulJpegCompStart, MMP_ULONG ulJpegCompEnd);
void        MMPF_JPEGCTL_Buf2StillJpegGetCompressAddress(MMP_ULONG *pulJpegLineStart, MMP_ULONG *pulJpegCompStart, MMP_ULONG *pulJpegCompEnd);
void        MMPF_JPEGCTL_Buf2StillJpegGetSize(MMP_ULONG *pulJpegSize);
int         MMPF_JPEGCTL_Buf2StillJpegDoneCBFunc(void* pArg);
MMP_ERR     MMPF_JPEGCTL_Buf2StillJpeg(MMP_USHORT usIBCPipe, MMP_ULONG ulSrcW, MMP_ULONG ulSrcH, MMP_ULONG ulJPGW, MMP_ULONG ulJPGH);
#endif

#endif // _MMPF_JPEG_CTL_H_