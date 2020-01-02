/// @ait_only
/**
 @file mmpd_mp4venc.h
 @brief INCLUDE File of Host VIDEO ENCODE Driver.
 @author Will Tseng
 @version 1.0
*/

#ifndef _MMPD_MP4VENC_H_
#define _MMPD_MP4VENC_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_lib.h"
#include "ait_config.h"
#include "mmp_vidrec_inc.h"

/** @addtogroup MMPD_3GP
 *  @{
 */

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define	VIDEO_INIT_QP_STEP_NUM      (3)

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MMPD_VIDENC_MCI_MODE {
    #define MCI_MODE_MASK       	(0x000F)
    #define MCI_PIPE_MASK       	(0x00F0)
    #define MCI_RAWS_MASK       	(0x0F00)
    #define MCI_2NDPIPE_MASK    	(0xF000)

    #define MCI_SET_MODE(m)     	(m)
    #define MCI_SET_PIPE(p)     	((p) << 4)
    #define MCI_SET_RAWS(s)     	((s) << 8)
    #define MCI_SET_2NDPIPE(p)		((p) << 12)

    #define MCI_GET_MODE(_mode) 	(_mode & MCI_MODE_MASK)
    #define MCI_GET_PIPE(_mode) 	((_mode & MCI_PIPE_MASK) >> 4)
    #define MCI_GET_RAWS(_mode) 	((_mode & MCI_RAWS_MASK) >> 8)
    #define MCI_GET_2NDPIPE(_mode) 	((_mode & MCI_2NDPIPE_MASK) >> 12)

    MMPD_VIDENC_MCI_DEFAULT      			= MCI_SET_MODE(1),
    MMPD_VIDENC_MCI_DMAR_H264    			= MCI_SET_MODE(2),
    MMPD_VIDENC_MCI_DMAR_H264_P0 			= MCI_SET_MODE(2)|MCI_SET_PIPE(0),
    MMPD_VIDENC_MCI_DMAR_H264_P1 			= MCI_SET_MODE(2)|MCI_SET_PIPE(1),
    MMPD_VIDENC_MCI_DMAR_H264_P2 			= MCI_SET_MODE(2)|MCI_SET_PIPE(2),
    MMPD_VIDENC_MCI_DMAR_H264_P3 			= MCI_SET_MODE(2)|MCI_SET_PIPE(3),
    MMPD_VIDENC_MCI_RAW      				= MCI_SET_MODE(3),
    MMPD_VIDENC_MCI_RAW0_P0   				= MCI_SET_MODE(3)|MCI_SET_PIPE(0),
    MMPD_VIDENC_MCI_RAW0_P1   				= MCI_SET_MODE(3)|MCI_SET_PIPE(1),
    MMPD_VIDENC_MCI_RAW0_P2   				= MCI_SET_MODE(3)|MCI_SET_PIPE(2),
    MMPD_VIDENC_MCI_RAW0_P3   				= MCI_SET_MODE(3)|MCI_SET_PIPE(3),
    MMPD_VIDENC_MCI_RAW1_P0 				= MCI_SET_MODE(3)|MCI_SET_PIPE(0)|MCI_SET_RAWS(1),
    MMPD_VIDENC_MCI_RAW1_P1 				= MCI_SET_MODE(3)|MCI_SET_PIPE(1)|MCI_SET_RAWS(1),
    MMPD_VIDENC_MCI_GRA_LDC_H264 			= MCI_SET_MODE(4),
    MMPD_VIDENC_MCI_GRA_LDC_P0				= MCI_SET_MODE(4)|MCI_SET_PIPE(0),
    MMPD_VIDENC_MCI_GRA_LDC_P1				= MCI_SET_MODE(4)|MCI_SET_PIPE(1),
    MMPD_VIDENC_MCI_GRA_LDC_P2				= MCI_SET_MODE(4)|MCI_SET_PIPE(2),
    MMPD_VIDENC_MCI_GRA_LDC_P3				= MCI_SET_MODE(4)|MCI_SET_PIPE(3),
    MMPD_VIDENC_MCI_GRA_LDC_P0P2_H264_P1	= MCI_SET_MODE(4)|MCI_SET_PIPE(0)|MCI_SET_2NDPIPE(2),
    MMPD_VIDENC_MCI_GRA_LDC_P3P1_H264_P0	= MCI_SET_MODE(4)|MCI_SET_PIPE(3)|MCI_SET_2NDPIPE(1),
    MMPD_VIDENC_MCI_INVALID      			= MCI_SET_MODE(MCI_MODE_MASK)
} MMPD_VIDENC_MCI_MODE;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct _MMPD_H264ENC_HEADER_BUF {
    MMP_ULONG ulSPSStart;
    MMP_ULONG ulSPSSize;
    MMP_ULONG ulTmpSPSStart;
    MMP_ULONG ulTmpSPSSize;
    MMP_ULONG ulPPSStart;
    MMP_ULONG ulPPSSize;
} MMPD_H264ENC_HEADER_BUF;

typedef struct _MMPD_H264ENC_MISC_BUF {
    MMP_ULONG ulMVBuf;		    ///< Video encode MV start buffer
    MMP_ULONG ulSliceLenBuf;    ///< H264 slice length buffer
} MMPD_H264ENC_MISC_BUF;

typedef struct _MMPD_H264ENC_BITSTREAM_BUF {
    MMP_ULONG ulStart;          ///< Video encode compressed buffer start address
    MMP_ULONG ulEnd;		    ///< Video encode compressed buffer end address
} MMPD_H264ENC_BITSTREAM_BUF;

typedef struct _MMPD_H264ENC_REFGEN_BD {
    MMP_ULONG ulRefYStart;      ///< Video encode Y reference buffer start address
    MMP_ULONG ulRefYEnd;        ///< Video encode Y reference buffer end address
    MMP_ULONG ulRefUVStart;     ///< Video encode U reference buffer start address
    MMP_ULONG ulRefUVEnd;       ///< Video encode U reference buffer end address
    MMP_ULONG ulGenYStart;
    MMP_ULONG ulGenYEnd;
    MMP_ULONG ulGenUVStart;
    MMP_ULONG ulGenUVEnd;
} MMPD_H264ENC_REFGEN_BD;

typedef struct _MMPD_MP4VENC_VIDEOBUF {
    MMPD_H264ENC_MISC_BUF       miscbuf;
    MMPD_H264ENC_BITSTREAM_BUF  bsbuf;
    MMPD_H264ENC_REFGEN_BD      refgenbd;
} MMPD_MP4VENC_VIDEOBUF;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

// Generic API
#if (DUALENC_SUPPORT)
MMP_ERR MMPD_VIDENC_SetSWStickerAttr(MMP_USHORT usStickerSrcW, MMP_USHORT usStickerSrcH,
                                     MMP_USHORT usDstStartX, MMP_USHORT usDstStartY); 
MMP_ERR MMPD_VIDENC_SetSWStickerAddress(MMP_ULONG ulStickerSrcAddr);                                           
#endif
MMP_BOOL MMPD_VIDENC_IsModuleInit(void);
MMP_ERR MMPD_VIDENC_DeinitModule(void);
MMP_ERR MMPD_VIDENC_IsTimerEnabled(MMP_BOOL* pbEnable);
MMP_ERR MMPD_VIDENC_EnableTimer(MMP_BOOL bEnable);
MMP_ERR MMPD_VIDENC_InitModule(void);
MMP_ERR MMPD_VIDENC_InitInstance(MMP_ULONG *InstId, MMP_USHORT usStreamType, MMP_USHORT usRcMode);
MMP_ERR MMPD_VIDENC_DeInitInstance(MMP_ULONG InstId);
MMP_ERR MMPD_VIDENC_SetSrcPipe(MMP_ULONG ulEncId, MMP_UBYTE ubPipe);
MMP_ERR MMPD_VIDENC_SetResolution(MMP_ULONG ulEncId, MMP_USHORT usWidth, MMP_USHORT usHeight);
MMP_ERR MMPD_VIDENC_SetQuality(MMP_ULONG ulEncId, MMP_ULONG ulTargetSize, MMP_ULONG ulBitrate);
MMP_ERR MMPD_VIDENC_SetProfile(MMP_ULONG ulEncId, VIDENC_PROFILE profile);
MMP_ERR MMPD_VIDENC_SetLevel(MMP_ULONG ulEncId, MMP_ULONG ulLevel);
MMP_ERR MMPD_VIDENC_SetEntropy(MMP_ULONG ulEncId, VIDENC_ENTROPY entropy);
MMP_ERR MMPD_VIDENC_SetForceI(MMP_ULONG ulEncId, MMP_ULONG ulCnt);
MMP_ERR MMPD_VIDENC_SetRcMode(MMP_ULONG ulEncId, VIDENC_RC_MODE mode);
MMP_ERR MMPD_VIDENC_SetRcSkipEn(MMP_ULONG ulEncId, MMP_BOOL bSkip);
MMP_ERR MMPD_VIDENC_SetRcSkipType(MMP_ULONG ulEncId, VIDENC_RC_SKIPTYPE type);
MMP_ERR MMPD_VIDENC_SetRcVBVSize(MMP_ULONG ulEncId, MMP_ULONG lbs);
MMP_ERR MMPD_VIDENC_SetTNREnable(MMP_ULONG ulEncId, MMP_ULONG tnr);
MMP_ERR MMPD_VIDENC_SetInitQP(MMP_ULONG ulEncId, MMP_UBYTE ubIQP, MMP_UBYTE ubPQP);
MMP_ERR MMPD_VIDENC_SetQPBoundary(MMP_ULONG ulEncId, MMP_ULONG ulFrmType, MMP_ULONG ulLowerBound, MMP_ULONG ulUpperBound);
MMP_ERR MMPD_VIDENC_GetSkipThreshold(MMP_ULONG ulEncId, MMP_ULONG *pThr);
MMP_ERR MMPD_VIDENC_SetEncodeMode(void);
MMP_ERR MMPD_VIDENC_SetGOP(MMP_ULONG ulEncId, MMP_USHORT usPFrame, MMP_USHORT usBFrame);
MMP_ERR MMPD_VIDENC_SetBitrate(MMP_ULONG ulEncId, MMP_ULONG ulBitrate);
MMP_ERR MMPD_VIDENC_SetEncFrameRate(MMP_ULONG ulEncId, MMP_ULONG ulTimeIncrement, MMP_ULONG ulTimeResol);
MMP_ERR MMPD_VIDENC_UpdateEncFrameRate(MMP_ULONG ulEncId, MMP_ULONG ulTimeIncrement, MMP_ULONG ulTimeResol);
MMP_ERR MMPD_VIDENC_SetSnrFrameRate(MMP_ULONG ulEncId, MMP_USHORT usStreamType, MMP_ULONG ulTimeIncrement, MMP_ULONG ulTimeResol);
MMP_ERR MMPD_VIDENC_SetCropping (MMP_ULONG ulEncId, MMP_USHORT usTop, MMP_USHORT usBottom, MMP_USHORT usLeft, MMP_USHORT usRight);
MMP_ERR MMPD_VIDENC_SetCurBufMode(MMP_ULONG ulEncId, VIDENC_CURBUF_MODE VideoCurBufMode);
MMP_ERR MMPD_VIDENC_GetNumOpen(MMP_ULONG *ulNumOpening);
MMP_ERR MMPD_VIDENC_GetStatus(MMP_ULONG ulEncId, VIDENC_FW_STATUS *status);
MMP_ERR MMPD_VIDENC_GetMergerStatus(MMP_ERR *status, MMP_ULONG *tx_status);
MMP_ERR MMPD_VIDENC_CheckCapability(MMP_ULONG w, MMP_ULONG h, MMP_ULONG fps);
MMP_ERR MMPD_VIDENC_EnableClock(MMP_BOOL bEnable);
MMP_ERR MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_MODE mode);
void MMPD_VIDENC_TunePipeMaxMCIPriority(MMP_UBYTE ubPipe);
void MMPD_VIDENC_TunePipe2ndMCIPriority(MMP_UBYTE ubPipe);

#if (SUPPORT_VR_THUMBNAIL)
MMP_ERR MMPD_VIDENC_EnableVrThumbnail(MMP_UBYTE ubEnable, MMP_UBYTE ubIsCreateJpg);
MMP_UBYTE MMPD_VIDENC_GetVrThumbnailSts(void);
MMP_ERR MMPD_VIDENC_SetVrThumbRingBufNum(MMP_UBYTE ubRingBufNum);
#endif

MMP_ERR MMPD_VIDENC_StartStreaming(MMP_ULONG ulEncId, MMP_USHORT usStreamType);
MMP_ERR MMPD_VIDENC_StopStreaming(MMP_ULONG ulEncId, MMP_USHORT usStreamType);

// H264 API
MMP_ERR MMPD_H264ENC_SetBitstreamBuf(MMP_ULONG ulEncId, MMPD_H264ENC_BITSTREAM_BUF *bsbuf);
MMP_ERR MMPD_H264ENC_SetSourcePPBuf(VIDENC_INPUT_BUF *inputbuf);
MMP_ERR MMPD_H264ENC_SetRefGenBound(MMP_ULONG ubEncId, MMPD_H264ENC_REFGEN_BD *refgenbd);
MMP_ERR MMPD_H264ENC_SetMiscBuf(MMP_ULONG ubEncId, MMPD_H264ENC_MISC_BUF *miscbuf);
MMP_ERR MMPD_H264ENC_CalculateRefBuf(MMP_USHORT usWidth, MMP_USHORT usHeight,
                                     MMPD_H264ENC_REFGEN_BD *refgenbd, MMP_ULONG *ulCurAddr);
MMP_ERR MMPD_H264ENC_SetPadding(MMP_ULONG ulEncId, MMP_USHORT usType, MMP_USHORT usCnt);
MMP_ERR MMPD_H264ENC_SetEncByteCnt(MMP_USHORT usByteCnt);
MMP_ERR MMPD_H264ENC_SetHeaderBuf(MMP_ULONG ulEncId, MMPD_H264ENC_HEADER_BUF *hdrbuf);
MMP_ERR MMPD_H264ENC_GetHeaderInfo(MMPD_H264ENC_HEADER_BUF *hdrbuf);
MMP_ERR MMPD_H264ENC_SetUVCHdrBuf(MMPD_H264ENC_HEADER_BUF *hdrbuf);

/// @}

#endif // _MMPD_MP4VENC_H_
