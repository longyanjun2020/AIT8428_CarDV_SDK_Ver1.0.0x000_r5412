//==============================================================================
//
//  File        : mmpf_h264enc_ctl.h
//  Description : INCLUDE File for the H264 Driver Function
//  Author      : Eroy Yang
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_H264ENC_CTL_H_
#define _MMPF_H264ENC_CTL_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "includes_fw.h"
#include "mmp_rawproc_inc.h"
#include "mmp_graphics_inc.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define MMPF_H264E_MAX_QUEUE_SIZE       (30)

#define H264E_DONE_SEM_TIMEOUT          (90)

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MMP_H264_ENC_SRC {
    MMP_H264_ENC_SRC_FRM_IBC = 0,   // Frame Mode
    MMP_H264_ENC_SRC_RT_RAW,        // RealTime Mode
    MMP_H264_ENC_SRC_RT_GRA,        // RealTime Mode
    MMP_H264_ENC_SRC_NUM
} MMP_H264_ENC_SRC;

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct _MMPF_H264E_CTL_GRA_ATTR 
{
    MMP_GRAPHICS_BUF_ATTR   sGraBufAttr;            ///< Graphics Buffer Attribute
    MMP_GRAPHICS_RECT       sGraRect;
    MMP_UBYTE               ubGraPixDelayN;
    MMP_UBYTE               ubGraPixDelayM;
    MMP_USHORT              usGraLineDelay;
} MMPF_H264E_CTL_GRA_ATTR;

typedef struct _MMPF_H264E_CTL_RAW_ATTR 
{
    MMP_RAW_FETCH_ATTR      sRawFecthAttr;
    MMP_ULONG               ulRawBufAddr;
    MMP_UBYTE               ubRawPixDelay;
    MMP_USHORT              usRawLineDelay;
    MMP_ULONG               ulRawStartOfst;
    MMP_USHORT              usRawPixelOfst;
    MMP_USHORT              usRawRealLineOfst;
    MMP_UBYTE               ubRawBitMode;
} MMPF_H264E_CTL_RAW_ATTR;

typedef struct _MMPF_H264E_CTL_ATTR 
{
    MMP_H264_ENC_SRC        sEncSrc;
    MMP_UBYTE               ubSensorId;
    MMP_UBYTE               ubRawId;
    MMP_UBYTE               ubActiveEncPipe;
    MMP_UBYTE               ubInactiveEncPipe;
    MMP_BOOL                bCtlEncPipe;
    MMP_UBYTE               ubEncBufMode;
    MMP_USHORT              usStreamType;
    MMPF_H264E_CTL_GRA_ATTR sSrcGraAttr;
    MMPF_H264E_CTL_RAW_ATTR sSrcRawAttr;
} MMPF_H264E_CTL_ATTR;

typedef struct _MMPF_H264E_CTL_QUEUE 
{
    MMPF_H264E_CTL_ATTR     attr[MMPF_H264E_MAX_QUEUE_SIZE];    ///< Queue for ready to encode/decode
    MMP_ULONG               weight[MMPF_H264E_MAX_QUEUE_SIZE];  ///< The times to encode the same frame
    MMP_ULONG               head;                               ///< Queue head index
    MMP_ULONG               size;                               ///< Queue size
    MMP_ULONG               weighted_size;
} MMPF_H264E_CTL_QUEUE;

//===============================================================================
//
//                               EXTERN VARIABLES
//
//===============================================================================

extern MMPF_H264E_CTL_QUEUE m_sH264EncCtlQueue;
extern MMPF_H264E_CTL_ATTR  m_sH264EncAttr[MMP_H264_ENC_SRC_NUM];

//===============================================================================
//
//                               FUNCTION PROTOTYPES
//
//===============================================================================

void        MMPF_H264ECTL_ResetQueue(MMPF_H264E_CTL_QUEUE *queue);
MMP_ULONG   MMPF_H264ECTL_GetQueueDepth(MMPF_H264E_CTL_QUEUE *queue);
MMP_ERR     MMPF_H264ECTL_PushQueue(MMPF_H264E_CTL_QUEUE *queue, MMPF_H264E_CTL_ATTR attr);
MMP_ERR     MMPF_H264ECTL_PopQueue(MMPF_H264E_CTL_QUEUE *queue, MMP_ULONG offset, MMPF_H264E_CTL_ATTR *pData);

MMP_ERR     MMPF_H264E_SetCtrlByQueueEnable(MMP_BOOL bEnable);
MMP_BOOL    MMPF_H264E_GetCtrlByQueueEnable(void);

#endif // _MMPF_H264ENC_CTL_H_