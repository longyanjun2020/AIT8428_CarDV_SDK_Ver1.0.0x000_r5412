#ifndef _MMPF_LDC_H_
#define _MMPF_LDC_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_ldc_inc.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define LDC_X_POS_ARRAY_SIZE    			(42)
#define LDC_Y_POS_ARRAY_SIZE    			(32)
#define LDC_DELTA_ARRAY_SIZE    			(128)

#define LDC_MAX_LB_CNT_FOR_FHD				(2)
#define LDC_MAX_LB_CNT_FOR_HD				(4)
#define LDC_MAX_LB_CNT_FOR_WVGA				(4)

/* For Debug */ 
#define LDC_DEBUG_TIME_PRINT_OUT			(0)
#define LDC_DEBUG_TIME_STORE_TBL			(1)

#define LDC_DEBUG_MSG_EN					(0)
#define LDC_DEBUG_TIME_TYPE					(LDC_DEBUG_TIME_STORE_TBL)
#define LDC_DEBUG_TIME_TBL_MAX_NUM			(100)
#if (LDC_DEBUG_TIME_TYPE == LDC_DEBUG_TIME_STORE_TBL)
#define LDC_DEBUG_FRAME_MAX_NUM				(LDC_DEBUG_TIME_TBL_MAX_NUM)
#else
#define LDC_DEBUG_FRAME_MAX_NUM				(10)
#endif

/* For SD/SF Update */
#define SD_UPDATE_LDC_LUT_ENABLE			(1)
#define SD_UPDATE_REMOVE_LUT_FILE			(0)
#define SD_UPDATE_PRINT_LUT_INFO			(0)
#define LDC_LUT_WRITE_BIN_TEMPBUF			(0x03000000)
#define LDC_LUT_BIN_TAG_MAX_LENGTH			(16)

#define SD_LDC_736P_LUT_UPDATE_FILENAME		("SD:\\SD_LDC_LUT_736P.bin")
#define SD_LDC_1536P_LUT_UPDATE_FILENAME	("SD:\\SD_LDC_LUT_1536P.bin")
#define SF_LDC_736P_LUT_UPDATE_FILENAME		("SF:1:\\SD_LDC_LUT_736P.bin")
#define SF_LDC_1536P_LUT_UPDATE_FILENAME	("SF:1:\\SD_LDC_LUT_1536P.bin")

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MMPF_LDC_INPUT_PATH {
  	MMPF_LDC_INPUT_FROM_ISP = 0,
  	MMPF_LDC_INPUT_FROM_GRA,
 	MMPF_LDC_INPUT_PATH_NUM
} MMPF_LDC_INPUT_PATH;

typedef enum _MMPF_LDC_PIPE_MODE {
  	MMPF_LDC_PIPE_MODE_LOOPBACK = 0,
  	MMPF_LDC_PIPE_MODE_DISP,
 	MMPF_LDC_PIPE_MODE_NUM
} MMPF_LDC_PIPE_MODE;

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

/* For SD/SF Update */
typedef struct _SDUPDATE_LDC_LUT_SLICE {
	MMP_ULONG				ulSliceIdx;
	
	MMP_UBYTE 				ubXposTag[LDC_LUT_BIN_TAG_MAX_LENGTH];
	MMP_USHORT 				usXposTbl[LDC_X_POS_ARRAY_SIZE];
	MMP_UBYTE				ubYposTag[LDC_LUT_BIN_TAG_MAX_LENGTH];
	MMP_USHORT				usYposTbl[LDC_Y_POS_ARRAY_SIZE];
		
	MMP_UBYTE 				ubDeltaTag[LDC_LUT_BIN_TAG_MAX_LENGTH];
	MMP_ULONG 				ulDeltaMemA000_127Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemA128_255Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemA256_335Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemB000_127Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemB128_255Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemB256_335Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemC000_127Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemC128_255Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemC256_335Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemD000_127Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemD128_255Tbl[LDC_DELTA_ARRAY_SIZE];
	MMP_ULONG 				ulDeltaMemD256_335Tbl[LDC_DELTA_ARRAY_SIZE];

	/* Graphics (Input) Relative */
    MMP_USHORT				usInXSt;
    MMP_USHORT				usInYSt;
    MMP_USHORT 				usInXLength;
    MMP_USHORT				usInYLength;
	
	/* LDC (Output) Relative */
    MMP_USHORT				usOutXSt;
    MMP_USHORT				usOutYSt;
    MMP_USHORT 				usOutXLength;
    MMP_USHORT				usOutYLength;
    MMP_USHORT				usScaleRatioH;
    MMP_USHORT				usScaleRatioV;
    
    /* IBC Relative */
    MMP_ULONG				ulDstPosX;
    MMP_ULONG				ulDstPosY;
} SDUPDATE_LDC_LUT_SLICE;

typedef struct _SDUPDATE_LDC_LUT_BIN {
	MMP_UBYTE    			ubTag[LDC_LUT_BIN_TAG_MAX_LENGTH];
	MMP_ULONG     			ulVer;
	MMP_ULONG				ulSnr0SliceNum;
	SDUPDATE_LDC_LUT_SLICE	sSnr0Lut[MAX_LDC_SLICE_NUM];
	MMP_ULONG				ulSnr1SliceNum;
	SDUPDATE_LDC_LUT_SLICE	sSnr1Lut[MAX_LDC_SLICE_NUM];
} SDUPDATE_LDC_LUT_BIN;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

void MMPF_LDC_ISR(void);
MMP_ERR MMPF_LDC_Initialize(void);
MMP_ERR MMPF_LDC_UnInitialize(void);
MMP_ERR MMPF_LDC_SetResMode(MMP_UBYTE ubResMode);
MMP_UBYTE MMPF_LDC_GetResMode(void);
MMP_ERR MMPF_LDC_SetFpsMode(MMP_UBYTE ubFpsMode);
MMP_UBYTE MMPF_LDC_GetFpsMode(void);
MMP_ERR MMPF_LDC_SetRunMode(MMP_UBYTE ubRunMode);
MMP_UBYTE MMPF_LDC_GetRunMode(void);
MMP_ERR MMPF_LDC_SetFrameRes(MMP_ULONG ulSrcW, MMP_ULONG ulSrcH,
							 MMP_ULONG ulOutW, MMP_ULONG ulOutH);
MMP_ERR MMPF_LDC_SetLinkPipe(MMP_UBYTE ubSrcPipe, MMP_UBYTE ubPrvPipe, MMP_UBYTE ubEncPipe, MMP_UBYTE ubSwiPipe, MMP_UBYTE ubJpegPipe);
MMP_ERR MMPF_LDC_GetLinkPipe(MMP_LDC_LINK *pLink);
MMP_ERR MMPF_LDC_SetAttribute(MMP_LDC_ATTR* pAttr);
MMP_ERR MMPF_LDC_UpdateLUT(MMP_USHORT* pPosTbl[], MMP_ULONG* pDeltaTbl[]);

/* Multi-Run */
void MMPF_LDC_MultiRunSwitchPipeMode(MMP_UBYTE ubMode);
void MMPF_LDC_MultiRunTriggerLoopBack(MMP_UBYTE ubIBCPipe);
MMP_ULONG MMPF_LDC_MultiRunGetMaxLoopBackCount(void);
void MMPF_LDC_MultiRunSetLoopBackCount(MMP_UBYTE ubCnt);
void MMPF_LDC_MultiRunNormalGraCallback(void);
void MMPF_LDC_MultiRunRetriggerGraCallback(void);

/* Multi-Slice */
void CallbackFunc_LdcSrcPipeFrameSt(void* argu);
void CallbackFunc_LdcDmaMoveFrame(MMP_ULONG argu);
void MMPF_LDC_MultiSliceExecuteBlending(MMP_UBYTE ubOpBlkIdx);
void MMPF_LDC_MultiSliceDmaMoveFrameDone(void);
void MMPF_LDC_MultiSliceDmaMoveFrame(void);
void MMPF_LDC_MultiSliceUpdateCurProcSnrId(MMP_UBYTE ubSnrId);
MMP_UBYTE MMPF_LDC_MultiSliceGetCurProcSnrId(void);
void MMPF_LDC_MultiSliceUpdateCurProcSliceId(MMP_USHORT usSiceId);
MMP_USHORT MMPF_LDC_MultiSliceGetCurProcSliceId(void);
void MMPF_LDC_MultiSliceUpdateOutBufIdx(void);
void MMPF_LDC_MultiSliceSwitchPipeMode(MMP_UBYTE ubMode);
void MMPF_LDC_MultiSliceNormalGraCallback(void);
void MMPF_LDC_MultiSliceRetriggerGraCallback(void);
MMP_ERR MMPF_LDC_MultiSliceInitAttr(MMP_UBYTE ubResIdx);
MMP_ERR MMPF_LDC_MultiSliceInitOutStoreBuf(MMP_UBYTE ubIdx, 
										   MMP_ULONG ulYAddr, MMP_ULONG ulUAddr, MMP_ULONG ulVAddr, 
										   MMP_UBYTE ubBufNum);
MMP_ERR MMPF_LDC_MultiSliceInitBlendBlkBuf(MMP_UBYTE ubBlkIdx, MMP_UBYTE ubBufIdx, 
										   MMP_ULONG ulYAddr, MMP_ULONG ulUAddr, MMP_ULONG ulVAddr);
MMP_ERR MMPF_LDC_MultiSliceUpdateSrcBufAddr(MMP_ULONG ulYAddr, MMP_ULONG ulUAddr, MMP_ULONG ulVAddr);
MMP_ERR MMPF_LDC_MultiSliceUpdatePipeAttr(MMP_UBYTE ubSnrSel, MMP_UBYTE ubSlice);
MMP_ERR MMPF_LDC_MultiSliceRestoreDispPipe(void);
MMP_ERR MMPF_LDC_MultiSliceResetPipeModule(MMP_UBYTE ubPipe);
MMP_USHORT MMPF_LDC_GetMultiSliceNum(MMP_UBYTE ubSnrSel);

/* SD Update */
MMP_BOOL SDUpdate_CreateLdcLUTBin(MMP_UBYTE ubLUTRes);
MMP_ERR SDUpdate_UpdateLocalLdcLUT(MMP_UBYTE ubLUTRes, MMP_BOOL bUpdateFromSIF, MMP_BOOL bUpdateToSIF);
MMP_BOOL SDUpdate_LDC_LUTIsExisted(MMP_UBYTE ubLUTRes);

#endif //_MMPF_LDC_H_