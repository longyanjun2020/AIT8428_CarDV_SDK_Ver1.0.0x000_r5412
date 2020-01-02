//==============================================================================
//
//  File        : mmpf_mp4venc.h
//  Description : Header function of video codec
//  Author      : Will Tseng
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_MP4VENC_H_
#define _MMPF_MP4VENC_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "includes_fw.h"
#include "mmpf_3gpmgr.h"
#include "mmpf_h264enc.h"
#include "mmp_vidrec_inc.h"

/** @addtogroup MMPF_VIDEO
@{
*/

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================

#if (SHARE_REF_GEN_BUF) && (H264ENC_ICOMP_EN)
    #error With share ref/gen buffer, h264 image compression is not supported
#endif

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

// Dummy Frame Mode
typedef enum _MMPF_VIDENC_DUMMY_FRAME_MODE {
    MMPF_VIDENC_DUMMY_FRAME_BY_REENC = 0,
    MMPF_VIDENC_DUMMY_FRAME_BY_CONTAINER
} MMPF_VIDENC_DUMMY_FRAME_MODE;

// Compress Buffer Fullness Status
typedef enum _MMPF_VIDENC_CMPBUF_STATUS {
    MMPF_VIDENC_CMPBUF_LV0 = 0,
    MMPF_VIDENC_CMPBUF_LV1,
    MMPF_VIDENC_CMPBUF_LV2,
    MMPF_VIDENC_CMPBUF_LV3,
    MMPF_VIDENC_CMPBUF_LV4
} MMPF_VIDENC_CMPBUF_STATUS;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef void MMPF_VIDENC_Callback(void);

typedef struct _MMPF_VIDENC_MODULE {
    MMP_BOOL                bInitMod;
    MMP_USHORT              Format;
    MMPF_H264ENC_MODULE     H264EMod;
} MMPF_VIDENC_MODULE;

typedef struct _MMPF_VIDNEC_INSTANCE {
    MMP_BOOL                bInitInst;
    MMPF_VIDENC_MODULE      *Module;
    MMPF_H264ENC_ENC_INFO   h264e;
} MMPF_VIDENC_INSTANCE;


// vide record speed control
#define MMPF_VIDENC_SPEED_NORMAL            0x00
#define MMPF_VIDENC_SPEED_SLOW              0x01
#define MMPF_VIDENC_SPEED_FAST              0x02
#define MMPF_VIDENC_SPEED_1X                0x00
#define MMPF_VIDENC_SPEED_2X                0x01
#define MMPF_VIDENC_SPEED_3X                0x02
#define MMPF_VIDENC_SPEED_4X                0x03
#define MMPF_VIDENC_SPEED_5X                0x04
#define MMPF_VIDENC_SPEED_MAX               0x05

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#define get_vidinst_format(_p)      ((_p)->Module->Format)
#define get_vidinst_id(_p)          ((MMPF_VIDENC_INSTANCE*)(_p) - MMPF_VIDENC_GetInstance(0))

MMPF_VIDENC_MODULE* MMPF_VIDENC_GetModule(void);
MMPF_VIDENC_INSTANCE* MMPF_VIDENC_GetInstance (MMP_UBYTE InstId);

// Video Encode Function
MMP_ULONG   MMPF_VIDENC_GetQueueDepth (VIDENC_QUEUE *queue, MMP_BOOL weighted);
MMP_ERR     MMPF_VIDENC_ShowQueue(VIDENC_QUEUE *queue, MMP_ULONG offset, MMP_ULONG *data, MMP_BOOL weighted);
void        MMPF_VIDENC_ResetQueue(VIDENC_QUEUE *queue);
MMP_ERR     MMPF_VIDENC_PushQueue(VIDENC_QUEUE *queue, MMP_ULONG buffer, MMP_BOOL weighted);
MMP_ERR     MMPF_VIDENC_PopQueue(VIDENC_QUEUE *queue, MMP_ULONG offset, MMP_ULONG *data, MMP_BOOL weighted);
MMP_ERR     MMPF_VIDENC_GetParameter(MMPF_H264ENC_ENC_INFO  *pEnc, 
                                     VIDENC_ATTRIBUTE       attrib, 
                                     void                   *arg);
void        MMPF_VIDENC_SetFrameInfoList(MMPF_H264ENC_ENC_INFO *pEnc, 
                                         MMP_ULONG             ulYBuf[], 
                                         MMP_ULONG             ulUBuf[], 
                                         MMP_ULONG             ulVBuf[], 
                                         MMP_UBYTE             ubBufCnt);
MMP_ERR     MMPF_VIDENC_SetEncodeEnable(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR     MMPF_VIDENC_SetEncodeDisable(MMPF_H264ENC_ENC_INFO *pEnc);
VIDENC_FRAME_TYPE MMPF_VIDENC_GetVidRecdFrameType(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR     MMPF_VIDENC_SetCropping(MMPF_H264ENC_ENC_INFO *pEnc, MMP_USHORT usTop, MMP_USHORT usBottom, MMP_USHORT usLeft, MMP_USHORT usRight);
void        MMPF_VIDENC_SetH264ForceICount(MMP_ULONG ulH264ForceICount);

MMP_ERR     MMPF_VIDENC_TriggerEncode(void);
MMP_ERR     MMPF_VIDENC_TriggerEncodeFrmMode(MMP_USHORT usStreamType);
MMP_ERR     MMPF_VIDENC_SetRecdFrameReady(MMP_USHORT usEncID, MMP_ULONG *plCurBuf, MMP_ULONG *plIBCBuf);
#if (SUPPORT_H264_WIFI_STREAM)
MMP_ERR     MMPF_VIDENC_SetWifiFrameReady(MMP_USHORT usEncID, MMP_ULONG *plCurBuf, MMP_ULONG *plIBCBuf);
#endif
MMP_ERR     MMPF_VIDENC_ResetVLDModule(void);
MMP_BOOL    MMPF_VIDENC_IsModuleInit(void);
MMP_ERR     MMPF_VIDENC_InitModule(void);
MMP_ERR     MMPF_VIDENC_DeinitModule(void);
MMP_ERR     MMPF_VIDENC_InitInstance(MMP_ULONG *InstId, MMP_USHORT usStreamType, MMP_USHORT usRcMode);
MMP_ERR     MMPF_VIDENC_DeInitInstance(MMP_ULONG InstId);
MMP_ERR     MMPF_VIDENC_Start(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR     MMPF_VIDENC_Stop(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR     MMPF_VIDENC_Resume(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR     MMPF_VIDENC_Pause(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR     MMPF_VIDENC_PreEncode(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_BOOL    MMPF_VIDENC_CheckCapability(MMP_ULONG total_mb, MMP_ULONG fps);
void        MMPF_MP4VENC_SetStatus(MMP_ULONG ulEncId, MMP_USHORT status);
MMP_USHORT  MMPF_MP4VENC_GetStatus(MMP_ULONG ulEncId);
MMP_ERR     MMPF_VIDENC_StopRecordForStorage (MMP_ULONG ulEncId);
MMP_ERR     MMPF_VIDENC_StopRecordForStorageAll(void);
MMP_ERR     MMPF_VIDENC_CheckSeamless(MMPF_H264ENC_ENC_INFO *pEnc, MMP_BOOL *ubSeamLess);
void        MMPF_VIDENC_GetEncFrameRate(VIDENC_FPS_CTL *fps);

// Rate Control Function
void        MMPF_VidRateCtl_GetRcVersion(MMP_USHORT* RcMajorVersion, MMP_USHORT* RcMinorVersion);
MMP_LONG    MMPF_VidRateCtl_Get_VOP_QP(void* RCHandle, MMP_LONG vop_type, MMP_ULONG *target_size, MMP_ULONG *qp_delta, MMP_BOOL *bSkipFrame, MMPF_VIDENC_CMPBUF_STATUS BufLevel, MMP_ULONG ulMaxFrameSize);
MMP_ERR     MMPF_VidRateCtl_ForceQP(void* RCHandle, MMP_LONG vop_type, MMP_ULONG QP);
MMP_ULONG   MMPF_VidRateCtl_UpdateModel(void* RCHandle, MMP_LONG vop_type, MMP_ULONG CurSize, MMP_ULONG HeaderSize, MMP_ULONG last_QP, MMP_BOOL bForceSkip, MMP_BOOL *bSkipFrame, MMP_ULONG *padding_bytes);
MMP_ERR     MMPF_VidRateCtl_Init(void* *handle, MMP_ULONG idx, MMP_USHORT usVidRecdFormat, MMP_LONG targetsize, MMP_ULONG framerate, MMP_ULONG nP, MMP_ULONG nB, MMP_BOOL PreventBufOverflow, RC_CONFIG_PARAM *RcConfig, MMP_ULONG fps);
MMP_ERR     MMPF_VidRateCtl_DeInit(void* RCHandle, MMP_ULONG handle_idx, RC_CONFIG_PARAM *RcConfig, MMP_ULONG fps);
void        MMPF_VidRateCtl_ResetBitrate(void* RCHandle, MMP_LONG bit_rate, MMP_ULONG TargetSize, MMP_BOOL ResetParams, MMP_ULONG ulVBVSize, MMP_BOOL bResetBufUsage);
void        MMPF_VidRateCtl_SetQPBoundary(void* RCHandle, MMP_ULONG frame_type, MMP_LONG QP_LowerBound, MMP_LONG QP_UpperBound);
void        MMPF_VidRateCtl_GetQPBoundary(void* RCHandle, MMP_ULONG frame_type, MMP_LONG *QP_LowerBound, MMP_LONG *QP_UpperBound);
void        MMPF_VidRateCtl_ResetBufSize(void* RCHandle, MMP_LONG BufSize);

// SW Sticker Function
MMP_ERR     MMPF_VIDENC_SetSWStickerAddress(MMP_ULONG ulStickerSrcAddr);
MMP_ERR     MMPF_VIDENC_GetSWStickerAddress(MMP_ULONG *pulStickerSrcAddr);
MMP_ERR     MMPF_VIDENC_SetSWStickerAttribute(MMP_USHORT usStickerSrcWidth, MMP_USHORT usStickerSrcHeight, 
                                              MMP_USHORT usDstStartx, MMP_USHORT usDstStarty);
MMP_ERR     MMPF_VIDENC_GetSWStickerAttribute(MMP_USHORT *pusStickerSrcWidth, MMP_USHORT *pusStickerSrcHeight, 
                                              MMP_USHORT *pusDstStartx, MMP_USHORT *pusDstStarty);

MMP_ERR     MMPF_VIDENC_SetSticker( MMP_ULONG ulDstAddr, MMP_USHORT usEncWidth, MMP_USHORT usEncHeight,
                                    MMP_ULONG ulStickerSrcAddr, MMP_USHORT usStickerSrcWidth, MMP_USHORT usStickerSrcHeight, 
                                    MMP_USHORT usDstStartx, MMP_USHORT usDstStarty);
/// @}

#endif	// _MMPF_MP4VENC_H_
