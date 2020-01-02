//==============================================================================
//
//  File        : mmp_component_ctl.h
//  Description : INCLUDE File for the Component Control Driver Function
//  Author      : Eroy Yang
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_COMPONENT_CTL_H_
#define _MMPF_COMPONENT_CTL_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "includes_fw.h"
#include "aitu_ringbuf.h"
#include "mmpd_fctl.h"
#include "mmpf_rawproc.h"
#include "mmpf_graphics.h"
#include "mmpf_scaler.h"
#include "mmpf_icon.h"
#include "mmpf_ibc.h"
#include "mmpf_dsc.h"
#include "mmpf_display.h"
#include "mmpf_h264enc_ctl.h"

#if (SUPPORT_COMPONENT_FLOW_CTL)

//==============================================================================
//
//                              CONSTANT
//
//==============================================================================

#define MAX_DRAM_COMPONENT_NUM      (15)
#define MAX_RAW_COMPONENT_NUM       (MMP_RAW_MDL_NUM)
#define MAX_PIPE_COMPONENT_NUM      (MMP_IBC_PIPE_MAX)

#define MAX_COMPONENT_LIST_NUM      (10)
#define MAX_COMPONENT_NUM_IN_LIST   (15)
#define INVALID_COMPONENT_LIST_ID   (0xFF)

#define MAX_DRAM_BUF_NUM            (4)

#define MAX_COMPONENT_USAGE         (3) // Sync with MMP_COMPONENT_USAGE_ID_NUM

#define MAX_OUTPUT_COMP_NUM         (5) // Base on Max Pipe Num

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MMP_BUF_HANDLE_TYPE
{
    MMP_BUF_HANDLE_FILLBUF = 0,
    MMP_BUF_HANDLE_FILLBUF_DONE,
    MMP_BUF_HANDLE_EMPTYBUF,
    MMP_BUF_HANDLE_EMPTYBUF_DONE,
    MMP_BUF_HANDLE_TYPE_NUM
} MMP_BUF_HANDLE_TYPE;

typedef enum _MMP_COMPONENT_USAGE_ID
{
    MMP_COMPONENT_USAGE_ID0 = 0,
    MMP_COMPONENT_USAGE_ID1,
    MMP_COMPONENT_USAGE_ID2,
    MMP_COMPONENT_USAGE_ID_NUM
} MMP_COMPONENT_USAGE_ID;

typedef enum _MMP_COMPONENT_STATE
{
    MMP_COMPONENT_STATE_INVALID = 0,
    MMP_COMPONENT_STATE_LOADED,     // Loaded but not initialized
    MMP_COMPONENT_STATE_IDLE,       // Private data are initialized
    MMP_COMPONENT_STATE_BUSY,       // The component is executing some operation 
    MMP_COMPONENT_STATE_NUM
} MMP_COMPONENT_STATE;

typedef enum _MMP_DRAM_IDX
{
    MMP_DRAM_IDX_PIPE0_OUT = 0,
    MMP_DRAM_IDX_PIPE1_OUT,
    MMP_DRAM_IDX_PIPE2_OUT,
    MMP_DRAM_IDX_PIPE3_OUT,
    MMP_DRAM_IDX_PIPE4_OUT,
    MMP_DRAM_IDX_RAW0_STORE,
    MMP_DRAM_IDX_RAW1_STORE,
    MMP_DRAM_IDX_RAW0_DMA_OUT,
    MMP_DRAM_IDX_RAW1_DMA_OUT,
    MMP_DRAM_IDX_JPGDEC_IN,
    MMP_DRAM_IDX_NUM
} MMP_COMPONENT_DRAM_IDX;

typedef enum _MMP_COMPONENT_ID
{
    MMP_COMPONENT_ID_DRAM_START     = 0,
    MMP_COMPONENT_ID_DRAM_PIPE0_OUT = 0,
    MMP_COMPONENT_ID_DRAM_PIPE1_OUT,
    MMP_COMPONENT_ID_DRAM_PIPE2_OUT,
    MMP_COMPONENT_ID_DRAM_PIPE3_OUT,
    MMP_COMPONENT_ID_DRAM_PIPE4_OUT,
    MMP_COMPONENT_ID_DRAM_RAW0_STORE,
    MMP_COMPONENT_ID_DRAM_RAW1_STORE,
    MMP_COMPONENT_ID_DRAM_RAW0_DMA_OUT,
    MMP_COMPONENT_ID_DRAM_RAW1_DMA_OUT,
    MMP_COMPONENT_ID_DRAM_JPGDEC_IN,
    MMP_COMPONENT_ID_DRAM_END = MMP_COMPONENT_ID_DRAM_JPGDEC_IN,
    
    MMP_COMPONENT_ID_VIF2ISP,   // 10
    MMP_COMPONENT_ID_RAW2ISP,
    MMP_COMPONENT_ID_GRA,
    MMP_COMPONENT_ID_DMA,
    MMP_COMPONENT_ID_RAW0,
    MMP_COMPONENT_ID_RAW1,
    MMP_COMPONENT_ID_PIPE0,
    MMP_COMPONENT_ID_PIPE1,
    MMP_COMPONENT_ID_PIPE2,
    MMP_COMPONENT_ID_PIPE3,
    MMP_COMPONENT_ID_PIPE4,     // 20
    MMP_COMPONENT_ID_JPGENC,
    MMP_COMPONENT_ID_JPGDEC,
    MMP_COMPONENT_ID_H264ENC,
    MMP_COMPONENT_ID_DISP,
    MMP_COMPONENT_ID_NUM,
    
    MMP_COMPONENT_ID_NULL
} MMP_COMPONENT_ID;

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef int DramCallBackFunc(void*);
typedef int ListTrigFrmFunc(int, int);

typedef int FillBufferFunc(int, int);
typedef int FillBufferDoneFunc(int, int);
typedef int EmptyBufferFunc(int, int);
typedef int EmptyBufferDoneFunc(int, int);

typedef struct _MMP_COMPONENT_BASE
{
    MMP_ULONG               ulState;
    MMP_BOOL                bRegistered;
    MMP_UBYTE               ubComponentId;
    MMP_UBYTE               ubUsageId;
    MMP_UBYTE               ubListId;
    MMP_UBYTE               ubDepdListId;
    MMP_BOOL                bSharable; // Non-sharable means we could not control frame start timing.
    void*                   pInputComponent;
    void*                   pOutputComponent[MAX_OUTPUT_COMP_NUM];
    MMP_UBYTE               ubOutputPortNum;
    MMP_UBYTE               ubTriggerCnt;

    FillBufferFunc          *pfFillBufFunc;
    FillBufferDoneFunc      *pfFillBufDoneFunc;
    EmptyBufferFunc         *pfEmptyBufFunc;
    EmptyBufferDoneFunc     *pfEmptyBufDoneFunc;
    
    //MMP_ERR                 (*pfCompConstructor)(void* handle);
    //MMP_ERR                 (*pfCompDesstructor)(void* handle);
    //MMP_ERR                 (*pfCompSendCmd)(void* handle, MMP_ULONG ulCmd, MMP_ULONG ulParam);
    //MMP_ERR                 (*pfCompSetCallback)(void* handle, void* pCallback, MMP_ULONG ulCbParam);
    //MMP_ERR                 (*pfCompSetConfig)(void* handle, MMP_ULONG ulCfgIdx, MMP_ULONG ulConfig);
    //MMP_ERR                 (*pfCompGetConfig)(void* handle, MMP_ULONG ulCfgIdx, MMP_ULONG* ulConfig);
    //MMP_ERR                 (*pfCompGetState)(void* handle, MMP_ULONG* ulState);
} MMP_COMPONENT_BASE;

typedef struct _MMP_COMPONENT_DRAM
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_UBYTE               ubBufCnt;
    MMP_ULONG               ulBaseAddr[MAX_DRAM_BUF_NUM];
    MMP_ULONG               ulBaseUAddr[MAX_DRAM_BUF_NUM];
    MMP_ULONG               ulBaseVAddr[MAX_DRAM_BUF_NUM];
    MMP_ULONG               ulBaseEndAddr[MAX_DRAM_BUF_NUM];
    MMP_ULONG               ulBaseUEndAddr[MAX_DRAM_BUF_NUM];
    MMP_ULONG               ulBaseVEndAddr[MAX_DRAM_BUF_NUM];
    AUTL_RINGBUF            sRingBufCtl;
    MMP_BOOL                bIsFilled[MAX_DRAM_BUF_NUM];
    DramCallBackFunc        *pfRdyCBFunc;
    
    MMP_DSC_DECODE_BUF      sJpgDecBuf[MAX_DRAM_BUF_NUM];
} MMP_COMPONENT_DRAM;

typedef struct _MMP_COMPONENT_VIF2ISP
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_UBYTE               ubSnrSel;
    MMP_UBYTE               ubVifId;
} MMP_COMPONENT_VIF2ISP;

typedef struct _MMP_COMPONENT_RAW2ISP
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_UBYTE               ubSnrSel;
    MMP_UBYTE               ubVifId;
    MMP_UBYTE               ubRawId;
    MMP_RAW_FETCH_ATTR      sRawFecthAttr;
    MMP_ULONG               ulRawFetchAddr;
    MMP_UBYTE               ubRawPixDelay;
    MMP_USHORT              usRawLineDelay;
    MMP_ULONG               ulRawStartOfst;
    MMP_USHORT              usRawPixelOfst;
    MMP_USHORT              usRawRealLineOfst;
} MMP_COMPONENT_RAW2ISP;

typedef struct _MMP_COMPONENT_GRA 
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_GRAPHICS_BUF_ATTR   sGraBufAttr;
    MMP_GRAPHICS_RECT       sGraRect;
    MMP_UBYTE               ubGraPixDelayN;
    MMP_UBYTE               ubGraPixDelayM;
    MMP_USHORT              usGraLineDelay;
} MMP_COMPONENT_GRA;

typedef struct _MMP_COMPONENT_RAWS
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_UBYTE               ubRawBitMode;
} MMP_COMPONENT_RAWS;

typedef struct _MMP_COMPONENT_DMA
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_ULONG               ulSrcAddr;
    MMP_ULONG               ulDstAddr;
    MMP_ULONG               ulDataCnt;
} MMP_COMPONENT_DMA;

typedef struct _MMP_COMPONENT_PIPE 
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_UBYTE               ubLinkedSnr;
    MMP_PIPE_LINK           ePipeLink;
    MMP_SCAL_SOURCE         eScalSrc;
    MMP_SCAL_FIT_RANGE      sFitrange;
    MMP_SCAL_GRAB_CTRL      sGrabctl;
    MMP_SCAL_DELAY          sScalDelay;
    MMP_SCAL_COLRMTX_MODE   eScalColorRange;
    MMP_DISPLAY_COLORMODE   eColormode;
    MMP_BOOL                bRtModeOut;
} MMP_COMPONENT_PIPE;

typedef struct _MMP_COMPONENT_JPGENC 
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_USHORT              usEncId;                ///< Encode ID
    MMP_USHORT              usEncWidth;             ///< Encode JPEG Width
    MMP_USHORT              usEncHeight;            ///< Encode JPEG Height
    MMP_ULONG               ulLineBufAddr;          ///< Encode Line Buffer Address
    MMP_ULONG               ulCompBufAddr;          ///< Encode Compress Buffer Address
    MMP_ULONG               ulCompBufSize;          ///< Encode Compress Buffer Size
    MMP_USHORT              usQFactor;              ///< Encode Q (Scale) Factor
    JpegEncCallBackFunc     *pfEncCBFunc;           ///< Encode Callback Function
    void*                   pEncCBFuncArg;          ///< Encode Callback Function Argument
    MMP_UBYTE               ubEncodeMode;           ///< MMPF_JPEG_ENCDONE_MODE    
} MMP_COMPONENT_JPGENC;

typedef struct _MMP_COMPONENT_JPGDEC 
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_ULONG               ulSrcWidth;
    MMP_ULONG               ulSrcHeight;
    MMP_ULONG               ulLineBufAddr;          ///< Decode Line Buffer Address
    MMP_ULONG               ulDCompBufAddr;         ///< Decode DeCompress Buffer Address
    MMP_ULONG               ulDCompBufSize;         ///< Decode DeCompress Buffer Size
    JpegDecCallBackFunc     *pfDecCBFunc;           ///< Decode Callback Function
    void*                   pDecCBFuncArg;          ///< Decode Callback Function Argument
} MMP_COMPONENT_JPGDEC;

typedef struct _MMP_COMPONENT_H264ENC 
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_H264_ENC_SRC        sEncSrc;
    MMP_UBYTE               ubSnrSel;
    MMP_UBYTE               ubRawId;
    MMP_UBYTE               ubEncBufMode;
    MMP_USHORT              usStreamType;
} MMP_COMPONENT_H264ENC;

typedef struct _MMP_COMPONENT_DISPLAY
{
    MMP_COMPONENT_BASE      sCompBase;
    MMP_DISPLAY_WIN_ID      eWinID;
    MMP_DISPLAY_DEV_TYPE    eDispDev;
} MMP_COMPONENT_DISPLAY;

typedef struct _MMP_COMPONENT_LIST
{
    MMP_UBYTE               ubListID;
    MMP_UBYTE               ubPipeID;
    MMP_UBYTE               ubCompUsageID;
    MMP_BOOL                bListActive;
    MMP_BOOL                bSrcReady;
    ListTrigFrmFunc         *pfTrigFunc;
    void*                   pComponent[MAX_COMPONENT_NUM_IN_LIST];
} MMP_COMPONENT_LIST;

//===============================================================================
//
//                               EXTERN VARIABLES
//
//===============================================================================

extern MMP_COMPONENT_DRAM       m_sComponentDram[MAX_DRAM_COMPONENT_NUM][MAX_COMPONENT_USAGE];
extern MMP_COMPONENT_DISPLAY    m_sComponentDisp[MAX_COMPONENT_USAGE];

//===============================================================================
//
//                               MACRO DEFINE
//
//===============================================================================


#define TRANS_PIPE_TO_PIPECOMP_ID(p)        (MMP_COMPONENT_ID_PIPE0 + p)
#define TRANS_PIPE_TO_DRAMCOMP_ID(p)        (MMP_COMPONENT_ID_DRAM_PIPE0_OUT + p)
#define TRANS_PIPECOMP_ID_TO_PIPE(c)        (c - MMP_COMPONENT_ID_PIPE0)
#define TRANS_DRAMCOMP_ID_TO_PIPE(c)        (c - MMP_COMPONENT_ID_DRAM_PIPE0_OUT)

#define TRANS_RAWS_TO_RAWSCOMP_ID(p)        (MMP_COMPONENT_ID_RAW0 + p)
#define TRANS_RAWS_TO_DRAMCOMP_ID(p)        (MMP_COMPONENT_ID_DRAM_RAW0_STORE + p)
#define TRANS_RAWS_TO_DMA_DRAMCOMP_ID(p)    (MMP_COMPONENT_ID_DRAM_RAW0_DMA_OUT + p)
#define TRANS_RAWSCOMP_ID_TO_RAW(c)         (c - MMP_COMPONENT_ID_RAW0)
#define TRANS_DRAMCOMP_ID_TO_RAW(c)         (c - MMP_COMPONENT_ID_DRAM_RAW0_STORE)
#define TRANS_DMA_DRAMCOMP_ID_TO_RAW(c)     (c - MMP_COMPONENT_ID_DRAM_RAW0_DMA_OUT)

#define TRANS_COMP_ID_TO_DRAM_IDX(p)        (p <= MMP_COMPONENT_ID_DRAM_JPGDEC_IN) ? (p + MMP_DRAM_IDX_PIPE0_OUT) : (MMP_DRAM_IDX_NUM) 
#define TRANS_PIPE_TO_DRAM_IDX(p)           (MMP_DRAM_IDX_PIPE0_OUT + p)

/* Ring Buffer Control Function */
#define COMPCTL_InitRBufHandle(ubDramIdx, ubUsageId ,pvBuf, ulSize)     AUTL_RingBuf_Init((AUTL_RINGBUF *)&m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl, (void *)pvBuf, ulSize)
#define COMPCTL_AdvRBufRdPtr(ubDramIdx, ubUsageId, ulSize)              AUTL_RingBuf_CommitRead((AUTL_RINGBUF *)&m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl, ulSize)
#define COMPCTL_AdvRBufWrPtr(ubDramIdx, ubUsageId, ulSize)              AUTL_RingBuf_CommitWrite((AUTL_RINGBUF *)&m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl, ulSize)
#define COMPCTL_IsRBufFull(ubDramIdx, ubUsageId)                        AUTL_RingBuf_Full((AUTL_RINGBUF *)&m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl)
#define COMPCTL_GetRBufFreeSpace(ubDramIdx, ubUsageId, pulSpace)        AUTL_RingBuf_SpaceAvailable((AUTL_RINGBUF *)&m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl, (MMP_ULONG *)pulSpace)
#define COMPCTL_GetRBufDatCnt(ubDramIdx, ubUsageId, pulCnt)             AUTL_RingBuf_DataAvailable((AUTL_RINGBUF *)&m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl, (MMP_ULONG *)pulCnt)

#define COMPCTL_CheckRBufSpaceSts(ubDramIdx, ubUsageId) { \
                                                MMP_LONG    lRet;   \
                                                MMP_ULONG   ulFreeSize; \
                                                lRet = COMPCTL_GetRBufFreeSpace(ubDramIdx, ubUsageId, &ulFreeSize);   \
                                                if (lRet < RINGBUF_SUCCESS)  \
                                                { RTNA_DBG_Str(0, "RBuf:"); RTNA_DBG_Dec(0, ubDramIdx); RTNA_DBG_Str(0, "OVF/UDF\r\n"); }}

#define COMPCTL_GetCurRBufRdPtr(ubDramIdx, ubUsageId)                   (m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl.ptr.rd)
#define COMPCTL_SetCurRBufRdPtr(ubDramIdx, ubUsageId, ulRdPtr)          (m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl.ptr.rd = ulRdPtr)
#define COMPCTL_GetCurRBufRdWrap(ubDramIdx, ubUsageId)                  (m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl.ptr.rd_wrap)
#define COMPCTL_SetCurRBufRdWrap(ubDramIdx, ubUsageId, ulRdWrap)        (m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl.ptr.rd_wrap = ulRdWrap)
#define COMPCTL_GetCurRBufWrPtr(ubDramIdx, ubUsageId)                   (m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl.ptr.wr)
#define COMPCTL_SetCurRBufWrPtr(ubDramIdx, ubUsageId, ulWrPtr)          (m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl.ptr.wr = ulWrPtr)
#define COMPCTL_GetCurRBufWrWrap(ubDramIdx, ubUsageId)                  (m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl.ptr.wr_wrap)
#define COMPCTL_SetCurRBufWrWrap(ubDramIdx, ubUsageId, ulWrWrap)        (m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl.ptr.wr_wrap = ulWrWrap)
#define COMPCTL_GetCurRBufSz(ubDramIdx, ubUsageId)                      (m_sComponentDram[ubDramIdx][ubUsageId].sRingBufCtl.size)

#define COMPCTL_GetCurRBufFilled(ubDramIdx, ubUsageId, ulIdx)           (m_sComponentDram[ubDramIdx][ubUsageId].bIsFilled[ulIdx])
#define COMPCTL_SetCurRBufFilled(ubDramIdx, ubUsageId, ulIdx, bFilled)  (m_sComponentDram[ubDramIdx][ubUsageId].bIsFilled[ulIdx] = bFilled)

//===============================================================================
//
//                               FUNCTION PROTOTYPES
//
//===============================================================================

/* Timer Function */
void MMP_CompCtl_StartCheckListTimer(MMP_ULONG ulPeriod);
void MMP_CompCtl_StopCheckListTimer(void);

/* Trigger Function */
void _TrigEmptyRawStoreBuffer(int ListId, int DepandListId);
void _TrigEmptyJpegBSBuffer(int ListId, int DepandListId);

/* Component Control Function */
MMP_ERR MMP_CompCtl_InitComponent(void* pBase, MMP_UBYTE ubCompId, MMP_UBYTE ubUsageId);
MMP_ERR MMP_CompCtl_InitComponentEx(void);
MMP_ERR MMP_CompCtl_LinkComponents(MMP_UBYTE ubUsageId, MMP_UBYTE ubCurCompId, MMP_UBYTE ubInCompId, MMP_UBYTE ubOutCompId);
MMP_UBYTE MMP_CompCtl_LinkComponentsEx(MMP_UBYTE ubUsageId, MMP_UBYTE* pubCompIdArray, MMP_UBYTE ubCnt, void* pfTrigFunc);
MMP_ERR MMP_CompCtl_UnLinkComponentList(MMP_UBYTE ubListId, MMP_BOOL bInActiveOnly);
MMP_ERR MMP_CompCtl_UnLinkAllComponentList(void);

MMP_ERR MMP_CompCtl_SetComponentUsageState(MMP_UBYTE ubCompId, MMP_UBYTE ubUsageId, MMP_ULONG ulState);
MMP_ULONG MMP_CompCtl_GetComponentUsageState(MMP_UBYTE ubCompId, MMP_UBYTE ubUsageId);
MMP_ERR MMP_CompCtl_SetComponentGblState(MMP_UBYTE ubCompId, MMP_ULONG ulState);
MMP_ULONG MMP_CompCtl_GetComponentGblState(MMP_UBYTE ubCompId);
MMP_ERR MMP_CompCtl_SetBufHandleFlag(MMP_UBYTE ubCompId, MMP_UBYTE ubUsageId, MMP_UBYTE ubBufHandleType);
MMP_UBYTE MMP_CompCtl_GetCurUseCompUsageId(MMP_UBYTE ubCompId);
MMP_UBYTE MMP_CompCtl_GetLinkPipeInCurList(MMP_UBYTE ubListId);

/* Register Function */
MMP_ERR MMP_CompCtl_RegisterVif2IspComponent(MMP_UBYTE ubUsageId, MMP_UBYTE ubSnrSel, MMP_UBYTE ubVifId);
MMP_ERR MMP_CompCtl_RegisterRaw2IspComponent(MMP_UBYTE ubUsageId, MMP_UBYTE ubSnrSel, MMP_UBYTE ubVifId);
MMP_ERR MMP_CompCtl_RegisterRawStoreComponent(MMP_UBYTE ubUsageId, MMP_UBYTE ubRawId, MMP_UBYTE ubRawBitMode);
MMP_ERR MMP_CompCtl_RegisterGraComponent(   MMP_UBYTE               ubUsageId,
                                            MMP_GRAPHICS_BUF_ATTR   sGraBufAttr,
                                            MMP_GRAPHICS_RECT       sGraRect,
                                            MMP_UBYTE               ubGraPixDelayN,
                                            MMP_UBYTE               ubGraPixDelayM,
                                            MMP_USHORT              usGraLineDelay);
MMP_ERR MMP_CompCtl_RegisterDMAComponent(MMP_UBYTE ubUsageId, MMP_ULONG ulSrcAddr, MMP_ULONG ulDstAddr, MMP_ULONG ulDataCnt);
MMP_ERR MMP_CompCtl_RegisterPipeComponent(MMP_UBYTE ubUsageId, void* pAttr);
MMP_ERR MMP_CompCtl_RegisterRawStoreDramComponent(MMP_UBYTE     ubUsageId,
                                                  MMP_UBYTE     ubRawId, 
                                                  MMP_UBYTE     ubBufCnt, 
                                                  MMP_ULONG*    pulBufAddr, 
                                                  MMP_ULONG*    pulEndBufAddr);
MMP_ERR MMP_CompCtl_RegisterRawStoreDMADramComponent(MMP_UBYTE ubUsageId, MMP_UBYTE ubRawId, MMP_UBYTE ubBufCnt, MMP_ULONG* pulBufAddr);
MMP_ERR MMP_CompCtl_RegisterPipeOutDramComponent(MMP_UBYTE ubUsageId, void* pAttr);
MMP_ERR MMP_CompCtl_RegisterJpegDecDramComponent(MMP_UBYTE ubUsageId, MMP_UBYTE ubBufCnt, void* psSlotAttr);
MMP_ERR MMP_CompCtl_RegisterH264Component(MMP_UBYTE ubUsageId, void* pAttr);
MMP_ERR MMP_CompCtl_RegisterDisplayComponent(MMP_UBYTE ubUsageId, MMP_UBYTE ubWinID, MMP_UBYTE ubDispDev);
MMP_ERR MMP_CompCtl_RegisterJpegEncComponent(MMP_UBYTE ubUsageId, void* pAttr);
MMP_ERR MMP_CompCtl_RegisterJpegDecComponent(MMP_UBYTE ubUsageId, MMP_ULONG ulSrcW, MMP_ULONG ulSrcH);

/* Task Function */
MMP_BOOL MMP_CompCtl_CheckListStateEx(void);

#endif // SUPPORT_COMPONENT_FLOW_CTL

#endif // _MMPF_COMPONENT_CTL_H_