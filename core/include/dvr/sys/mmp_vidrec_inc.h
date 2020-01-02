//------------------------------------------------------------------------------
//
//  File        : mmp_vidrec_inc.h
//  Description : Header file of Video Record configuration
//  Author      : 
//  Revision    : 1.0
//
//------------------------------------------------------------------------------

#ifndef _MMP_VIDREC_INC_H_
#define _MMP_VIDREC_INC_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "config_fw.h"

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================

#define H264ENC_I2MANY_EN                   (0)
#define H264ENC_HBR_EN                      (0)
#define H264ENC_TNR_EN                      (0)
#define H264ENC_RDO_EN                      (1)
#define H264ENC_ICOMP_EN                    (0) // Support in MCR_V2 MP only
    #define H264ENC_ICOMP_TEST      (0)
    #define ICOMP_TEST_PATTERN_W    (1920)
    #define ICOMP_TEST_PATTERN_H    (1088)
#define H264ENC_LBR_EN                      (1)
    #define H264ENC_LBR_FLOAT_RATIO (1)
    #define H264ENC_LBR_UNDERCHK    (1)

//==============================================================================
//
//                              MACRO DEFINE (Rate Control)
//
//==============================================================================

/*
 * Rate control related
 */
#define MAX_NUM_TMP_LAYERS                  (2)
#define MAX_NUM_TMP_LAYERS_LBR              (2) // For H264ENC_LBR_EN
#define TEMPORAL_ID_MASK                    (0x03)

#define RC_MIN_VBV_FRM_NUM                  ((RC_MAX_WEIGHT_I + 2000) / 1000)
#define RC_PSEUDO_GOP_SIZE                  (1000)

/*
 * Low bitrate ratio for I layers for static or motion scene
 */
#if (H264ENC_LBR_EN)&&(H264ENC_LBR_FLOAT_RATIO)
// For static scene
#define LBR_STATIC_I_RATIO_30FPS            (80)
#define LBR_STATIC_I_RATIO_60FPS            (70)
#define LBR_STATIC_I_RATIO_90FPS            (65)
#define LBR_STATIC_I_RATIO_120FPS           (60)

// For motion scene
#define LBR_MOTION_I_RATIO_30FPS            (20)
#define LBR_MOTION_I_RATIO_60FPS            (15)
#define LBR_MOTION_I_RATIO_90FPS            (10)
#define LBR_MOTION_I_RATIO_120FPS           (5)
#endif

#if (H264ENC_LBR_EN)
#define RC_MAX_WEIGHT_I                     (2000)
#define RC_INIT_WEIGHT_I                    (1000)
#else
#define RC_MAX_WEIGHT_I                     (3500)
#define RC_INIT_WEIGHT_I                    (2500)
#endif

//==============================================================================
//
//                              MACRO DEFINE (Encoder)
//
//==============================================================================

#define VIDENC_MAX_B_FRAME_NUMS             (0)

#define VIDENC_MAX_QUEUE_SIZE               (4)
#define	VIDENC_MAX_INPUT_FB_CNT             (4)

// Dual Record Flow Selection
#define DUAL_REC_DISABLE                    (0x00)
#define DUAL_REC_STORE_FILE                 (0x01)
#define DUAL_REC_ENCODE_H264                (0x02)

// Video Format
#define VIDENC_FORMAT_OTHERS                (0x00)
#define VIDENC_FORMAT_H264                  (0x01)
#define VIDENC_FORMAT_MJPEG                 (0x02)

// Video Max Stream Num
#if (DUALENC_SUPPORT)
#define MAX_VIDEO_STREAM_NUM                (2)
#else
#define MAX_VIDEO_STREAM_NUM                (1)
#endif

#if (SUPPORT_H264_WIFI_STREAM)
#define MAX_WIFI_STREAM_NUM                 (2) // Front Cam + Rear Cam
#else
#define MAX_WIFI_STREAM_NUM                 (0)
#endif

#define VIDENC_MAX_STREAM_NUM               (MAX_VIDEO_STREAM_NUM + MAX_WIFI_STREAM_NUM)

// Video Parameter Set Max Num
#define VIDENC_MAX_PARAM_SET_NUM            (16)

// Video Instance ID
#define INVALID_ENC_ID                      (0xFFFFFFFF)

//==============================================================================
//
//                              MACRO DEFINE (Merger/Misc)
//
//==============================================================================

// Merger Relative
#define VIDENC_SPEED_CTL                    (1) ///< video speed control
#define VIDENC_SEAMLESS                     (1) ///< support seamless video encoding

#define PRE_ALLOC_VR_THUMBNAIL_SIZE         (0x10000) ///< alloc 64K

#define INVALID_THUMB_PROG_CNT              (0xFFFFFFFF)

// The buffer reserve for VR Thumbnail in file
#define VR_THUMB_NORMRECD_BUF_NUM           (2)
#define VR_THUMB_UVCRECD_BUF_NUM            (2)
#define VR_THUMB_DUALRECD_BUF_NUM           (2)
#define VR_THUMB_EMERG_BUF_NUM              (1)

#define VR_THUMB_MAX_BUF_NUM                (15)

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef void VidEncEndCallBackFunc(void *);

// Thumbnail Type
typedef enum _VIDENC_THUMB_TYPE {
    VIDENC_THUMB_FRONT_RECD = 0,
    VIDENC_THUMB_FRONT_EMERG,
    VIDENC_THUMB_UVC_RECD,
    VIDENC_THUMB_UVC_EMERG,
    VIDENC_THUMB_DUAL_RECD,
    VIDENC_THUMB_DUAL_EMERG,
    /* Below are ring buffer type */
    VIDENC_THUMB_RING_BUF_FRONT,
    VIDENC_THUMB_RING_BUF_UVC,
    VIDENC_THUMB_RING_BUF_DUAL,
    VIDENC_THUMB_MAX_TYPE
} VIDENC_THUMB_TYPE;

// Video Record Speed Mode
typedef enum _VIDENC_SPEED_MODE {
    VIDENC_SPEED_NORMAL = 0,
    VIDENC_SPEED_SLOW,
    VIDENC_SPEED_FAST
} VIDENC_SPEED_MODE;

// Video Record Speed Ratio
typedef enum _VIDENC_SPEED_RATIO {
    VIDENC_SPEED_1X  = 0,
    VIDENC_SPEED_2X,
    VIDENC_SPEED_3X,
    VIDENC_SPEED_4X,
    VIDENC_SPEED_5X,
    VIDENC_SPEED_MAX
} VIDENC_SPEED_RATIO;

// Video FW Status
typedef enum _VIDENC_FW_STATUS {
    VIDENC_FW_STATUS_NONE       = 0x0000,
    VIDENC_FW_STATUS_START      = 0x0001,
    VIDENC_FW_STATUS_PAUSE      = 0x0002,
    VIDENC_FW_STATUS_RESUME     = 0x0003,
    VIDENC_FW_STATUS_STOP       = 0x0004,
    VIDENC_FW_STATUS_PREENCODE  = 0x0005
} VIDENC_FW_STATUS;

// Video Frame Type
typedef enum _VIDENC_FRAME_TYPE {
    VIDENC_I_FRAME = 0,
    VIDENC_P_FRAME,
    VIDENC_B_FRAME,
    VIDENC_FRAME_TYPE_NUM
} VIDENC_FRAME_TYPE;

// Video Source Mode
typedef enum _VIDENC_SRCMODE {
    VIDENC_SRCMODE_MEM = 0x00,  ///< Video encode by memory mode
    VIDENC_SRCMODE_CARD,        ///< Video encode by card mode
    VIDENC_SRCMODE_STREAM       ///< Video encode by call-back
} VIDENC_SRCMODE;

// Video Encode Attribute
typedef enum _VIDENC_ATTRIBUTE {
    VIDENC_ATTRIBUTE_PROFILE = 0,
    VIDENC_ATTRIBUTE_LEVEL,
    VIDENC_ATTRIBUTE_ENTROPY_MODE,
    VIDENC_ATTRIBUTE_FRM_QP,
    VIDENC_ATTRIBUTE_FRM_QP_BOUND,
    VIDENC_ATTRIBUTE_BR,
    VIDENC_ATTRIBUTE_LB_SIZE,
    VIDENC_ATTRIBUTE_CROPPING,
    VIDENC_ATTRIBUTE_GOP_CTL,
    VIDENC_ATTRIBUTE_FORCE_I,
    VIDENC_ATTRIBUTE_CURBUF_MODE,
    VIDENC_ATTRIBUTE_RESOLUTION,
    VIDENC_ATTRIBUTE_ME_ITR_MAX_STEPS,
    
    // TNR control
    VIDENC_ATTRIBUTE_TNR_EN,
    VIDENC_ATTRIBUTE_TNR_LOW_MV_THR,
    VIDENC_ATTRIBUTE_TNR_ZERO_MV_THR,
    VIDENC_ATTRIBUTE_TNR_ZERO_LUMA_PXL_DIFF_THR,
    VIDENC_ATTRIBUTE_TNR_ZERO_CHROMA_PXL_DIFF_THR,
    VIDENC_ATTRIBUTE_TNR_ZERO_MV_4x4_CNT_THR,
    VIDENC_ATTRIBUTE_TNR_LOW_MV_FILTER,
    VIDENC_ATTRIBUTE_TNR_ZERO_MV_FILTER,
    VIDENC_ATTRIBUTE_TNR_HIGH_MV_FILTER,

    // RDO control
    VIDENC_ATTRIBUTE_RDO_EN,
    VIDENC_ATTRIBUTE_RDO_QSTEP3_P1,
    VIDENC_ATTRIBUTE_RDO_QSTEP3_P2,

    // RC Control
    VIDENC_ATTRIBUTE_RC_MODE,
    VIDENC_ATTRIBUTE_RC_SKIPPABLE,
    VIDENC_ATTRIBUTE_RC_SKIPTYPE,
    VIDENC_ATTRIBUTE_MEMD_PARAM,

    // Frame rate control
    VIDENC_ATTRIBUTE_MAX_FPS,
    VIDENC_ATTRIBUTE_ENC_FPS,
    VIDENC_ATTRIBUTE_SNR_FPS
} VIDENC_ATTRIBUTE;

// Video Current Buffer Mode
typedef enum _VIDENC_CURBUF_MODE {
    VIDENC_CURBUF_FRAME,
    VIDENC_CURBUF_RT,
    VIDENC_CURBUF_MAX
} VIDENC_CURBUF_MODE;

// H264 Padding Type
typedef enum _VIDENC_PADDING_TYPE {
    H264ENC_PADDING_NONE = 0,
    H264ENC_PADDING_ZERO,
    H264ENC_PADDING_REPEAT
} VIDENC_PADDING_TYPE;

// H264 Entropy Coding
typedef enum _VIDENC_ENTROPY {
    H264ENC_ENTROPY_CAVLC = 0,
    H264ENC_ENTROPY_CABAC,
    H264ENC_ENTROPY_NONE
} VIDENC_ENTROPY;

// H264 Video Profile
typedef enum _VIDENC_PROFILE {
    H264ENC_PROFILE_NONE = 0,
    H264ENC_BASELINE_PROFILE,
    H264ENC_MAIN_PROFILE,
    H264ENC_HIGH_PROFILE,
    H264ENC_PROFILE_MAX
} VIDENC_PROFILE;

// Video Rate Control Mode
typedef enum _VIDENC_RC_MODE {
    VIDENC_RC_MODE_CBR = 0,
    VIDENC_RC_MODE_VBR,
    VIDENC_RC_MODE_CQP,
    VIDENC_RC_MODE_LOWBR,
    VIDENC_RC_MODE_MAX
} VIDENC_RC_MODE;

// RC Skip Type
typedef enum _VIDENC_RC_SKIPTYPE {
    VIDENC_RC_SKIP_DIRECT = 0,
    VIDENC_RC_SKIP_SMOOTH
} VIDENC_RC_SKIPTYPE;

// TNR Enable Feature
typedef enum _VIDENC_TNR_FEAT {
    TNR_ZERO_MV_EN  = 1 << 0,
    TNR_LOW_MV_EN   = 1 << 1,
    TNR_HIGH_MV_EN  = 1 << 2
} VIDENC_TNR_FEAT;

// Encode Stream Type
typedef enum _VIDENC_STREAMTYPE {
    VIDENC_STREAMTYPE_VIDRECD = 0,
    VIDENC_STREAMTYPE_EMERGENCY,
    VIDENC_STREAMTYPE_UVCRECD,
    VIDENC_STREAMTYPE_UVCEMERG,
    VIDENC_STREAMTYPE_DUALENC,
    VIDENC_STREAMTYPE_DUALEMERG,
    VIDENC_STREAMTYPE_REFIXTAIL,
    VIDENC_STREAMTYPE_WIFIFRONT,
    VIDENC_STREAMTYPE_WIFIREAR,
    VIDENC_STREAMTYPE_MAX
} VIDENC_STREAMTYPE;

// Video Event
typedef enum _VIDMGR_EVENT {
    VIDMGR_EVENT_NONE = 0,
    VIDMGR_EVENT_MEDIA_FULL,
    VIDMGR_EVENT_FILE_FULL,
    VIDMGR_EVENT_LONG_TIME_FILE_FULL,
    VIDMGR_EVENT_MEDIA_SLOW,
    VIDMGR_EVENT_SEAMLESS,
    VIDMGR_EVENT_MEDIA_ERROR,
    VIDMGR_EVENT_ENCODE_START,
    VIDMGR_EVENT_ENCODE_STOP,
    #if (DUALENC_SUPPORT)
    VIDMGR_EVENT_DUALENCODE_START,
    VIDMGR_EVENT_DUALENCODE_STOP,
    #endif
    #if (SUPPORT_H264_WIFI_STREAM)
    VIDMGR_EVENT_WIFIENCODE_START,
    VIDMGR_EVENT_WIFIENCODE_STOP,
    #endif
    VIDMGR_EVENT_POSTPROCESS,
    VIDMGR_EVENT_BITSTREAM_DISCARD,
    VIDMGR_EVENT_MEDIA_WRITE,
    VIDMGR_EVENT_STREAMCB, 
    VIDMGR_EVENT_EMERGFILE_FULL,
    VIDMGR_EVENT_RECDSTOP_CARDSLOW, 
    VIDMGR_EVENT_APSTOPVIDRECD,
    VIDMGR_EVENT_PREGETTIME_CARDSLOW,
    VIDMGR_EVENT_UVCFILE_FULL,
    VIDMGR_EVENT_COMPBUF_FREE_SPACE,
    VIDMGR_EVENT_APNEED_STOP_RECD,
    VIDMGR_EVENT_DUALENC_FILE_FULL,
    VIDMGR_EVENT_NUM
} VIDMGR_EVENT;

// Video Container Type
typedef enum _VIDMGR_CONTAINER_TYPE {
    VIDMGR_CONTAINER_3GP = 0,
    VIDMGR_CONTAINER_AVI,
    VIDMGR_CONTAINER_NONE,
    VIDMGR_CONTAINER_UNKNOWN
} VIDMGR_CONTAINER_TYPE;

// Check compress buffer stage
typedef enum _VIDMGR_COMPBUF_FREESIZE_STAGE {
    VIDMGR_COMPBUF_LESS_1MB = 0,
    VIDMGR_COMPBUF_LESS_2MB,
    VIDMGR_COMPBUF_LESS_3MB,
    VIDMGR_COMPBUF_LESS_4MB,
    VIDMGR_COMPBUF_LESS_5MB,
    VIDMGR_COMPBUF_SAFE
} VIDMGR_COMPBUF_FREESIZE_STAGE;

// AV sync method 
typedef enum _VIDMGR_AVSYNC_METHOD {
    VIDMGR_AVSYNC_REF_AUD = 0, 	// Use audio clock as reference clock
    VIDMGR_AVSYNC_REF_VID     	// Use video clock as reference clock
} VIDMGR_AVSYNC_METHOD;

// Video Encode Using Mode
typedef enum _VIDRECD_USEMODE {
    VIDRECD_USEMODE_RECD = 0,
    VIDRECD_USEMODE_CB2AP,
    VIDRECD_USEMODE_MAX
} VIDRECD_USEMODE;

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct _VIDENC_INPUT_BUF {
	MMP_ULONG ulBufCnt;
    MMP_ULONG ulY[VIDENC_MAX_INPUT_FB_CNT];	///< Video encode input Y buffer start address
    MMP_ULONG ulU[VIDENC_MAX_INPUT_FB_CNT]; ///< Video encode input U buffer start address
    MMP_ULONG ulV[VIDENC_MAX_INPUT_FB_CNT]; ///< Video encode input V buffer start address
} VIDENC_INPUT_BUF;

typedef struct _VIDENC_RESOLUTION {
    MMP_USHORT  usWidth;
    MMP_USHORT  usHeight;
} VIDENC_RESOLUTION;

typedef struct _VIDENC_RC_MODE_CTL {
    VIDENC_RC_MODE  RcMode;
    MMP_BOOL        bLayerGlobalRc;
} VIDENC_RC_MODE_CTL;

typedef struct _VIDENC_GOP_CTL {
    MMP_USHORT  usGopSize;
    MMP_USHORT  usMaxContBFrameNum;
    MMP_BOOL    bReset;
} VIDENC_GOP_CTL;

typedef struct _VIDENC_CROPPING {
    MMP_USHORT  usTop;
    MMP_USHORT  usBottom;
    MMP_USHORT  usLeft;
    MMP_USHORT  usRight;
} VIDENC_CROPPING;

typedef struct _VIDENC_BITRATE_CTL {            ///< bitrate param control
    MMP_UBYTE   ubLayerBitMap;                  ///< 0'b111 means all temporal layers
    MMP_ULONG   ulBitrate[MAX_NUM_TMP_LAYERS];  ///< bitrate, bits
} VIDENC_BITRATE_CTL;

typedef struct _VIDENC_LEAKYBUCKET_CTL {        ///< leacky bucket param control
    MMP_UBYTE   ubLayerBitMap;                  ///< 0'b111 means all temporal layers
    MMP_ULONG   ulLeakyBucket[MAX_NUM_TMP_LAYERS];///< in ms
} VIDENC_LEAKYBUCKET_CTL;

typedef struct _VIDENC_QP_CTL {                 ///< QP control, for initail QP and CQP
    MMP_UBYTE   ubTID;                          ///< 0'b111 means all temporal layers
    MMP_UBYTE   ubTypeBitMap;                   ///< 0: I, 1: P, 2: B
    MMP_UBYTE   ubQP[3];
    MMP_LONG    CbrQpIdxOffset[3];              ///< Chroma QP index offset
    MMP_LONG    CrQpIdxOffset[3];               ///< 2nd chroma QP index offset
} VIDENC_QP_CTL;

typedef struct _VIDENC_QP_BOUND_CTL {           ///< QP Boundary
    MMP_UBYTE   ubLayerID;                      ///< 0'b111 means all temporal layers
    MMP_UBYTE   ubTypeBitMap;                   ///< 0: I, 1: P, 2: B
    MMP_UBYTE   ubQPBound[3][2];
} VIDENC_QP_BOUND_CTL;

typedef struct _VIDENC_QUEUE {
    MMP_ULONG   buffers[VIDENC_MAX_QUEUE_SIZE]; ///< queue for buffer ready to encode, in display order
    MMP_ULONG   weight[VIDENC_MAX_QUEUE_SIZE];  ///< the times to encode the same frame
    MMP_ULONG   head;
    MMP_ULONG   size;
    MMP_ULONG   weighted_size;
} VIDENC_QUEUE;

typedef struct _VIDENC_FRAME_INFO {
    MMP_ULONG   ulYAddr;
    MMP_ULONG   ulUAddr;
    MMP_ULONG   ulVAddr;
    MMP_ULONG   ulTimestamp;
} VIDENC_FRAME_INFO;

typedef struct _VIDENC_FRAMEBUF_BD {
    VIDENC_FRAME_INFO   LowBound;
    VIDENC_FRAME_INFO   HighBound;
} VIDENC_FRAMEBUF_BD;

typedef struct _VIDENC_DUMMY_DROP_INFO {
    MMP_ULONG	ulDummyFrmCnt;                  ///< specified how many video frames to be duplicated
    MMP_ULONG   ulDropFrmCnt;                   ///< specified how many video frames to be dropped
    MMP_USHORT  usAccumSkipFrames;              ///< number of skip frames within 1 sec
    MMP_ULONG   ulBeginSkipTimeInMS;            ///< the absolute timer counter of the beginning skip frame within 1 sec.
} VIDENC_DUMMY_DROP_INFO;

typedef struct _VIDENC_FPS_CTL {
    MMP_ULONG   ulResol;
    MMP_ULONG   ulIncr;
    MMP_ULONG   ulIncrx1000;
} VIDENC_FPS_CTL;

typedef union {
    VIDENC_BITRATE_CTL      Bitrate;
    VIDENC_RC_MODE_CTL      RcMode;
    VIDENC_LEAKYBUCKET_CTL  CpbSize;
    VIDENC_QP_CTL           Qp;
    VIDENC_GOP_CTL          Gop;
    VIDENC_FPS_CTL          Fps;
    MMP_ULONG               ConusI;             ///< Contiguous I-frame count
} VIDENC_CTL;

typedef struct _VIDENC_PARAM_CTL {
    VIDENC_ATTRIBUTE        Attrib;
    void (*CallBack)(MMP_ERR);
    VIDENC_CTL              Ctl;
} VIDENC_PARAM_CTL;

typedef struct _VIDENC_THUMB_ATTR {
    MMP_ULONG               uladdr;             // Thumbnail store address
    MMP_ULONG               ulsize;             // Thumbnail size
    MMP_ULONG               ulprog_cnt;
    MMP_ULONG               ulidx;              // Frame Queue Index
} VIDENC_THUMB_ATTR;

typedef struct _VIDENC_THUMB_TARGET_CTL {
    MMP_ULONG               ulidx;
    MMP_ULONG               ulMarked;
} VIDENC_THUMB_TARGET_CTL;

#endif // _MMP_VIDREC_INC_H_

