//==============================================================================
//
//  File        : mmpf_ldws.h
//  Description : INCLUDE File for the Lane Departure Warning System Function
//  Author      : Alterman
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_LDWS_H_
#define _MMPF_LDWS_H_

#include "includes_fw.h"
#if (CPU_ID == CPU_A)
#include "mmpf_fdtc.h"
#endif

/** @addtogroup MMPF_LDWS
@{
*/
#if (VIDEO_R_EN)&&(SUPPORT_ADAS)

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

#if (CPU_ID == CPU_A)
typedef enum _MMPF_ADAS_STATE {
    MMPF_ADAS_IDLE_STATE = 0,
    MMPF_ADAS_ACTIVE_STATE,
    MMPF_ADAS_WAIT_FRAME_STATE,
    MMPF_ADAS_FRAME_READY_STATE,
    MMPF_ADAS_OPERATING_STATE,
    MMPF_ADAS_DMA_TRIGGER_STATE,
    MMPF_ADAS_DMA_MOVING_STATE
} MMPF_ADAS_STATE;
#endif

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

#if (CPU_ID == CPU_A)
typedef struct _MMPF_ADAS_ATTRIBUTE {
    MMP_ULONG   ulWorkBuf;          ///< the addr of working buffer
    MMP_ULONG   ulWorkBufSize;      ///< the size of working buffer
	MMP_USHORT  usInputW;           ///< the width of input frame to do motion detection
	MMP_USHORT  usInputH;           ///< the height of input frame to do motion detection
	MMP_ULONG   ulInputLumaAddr;    ///< the addr of luma source
	MMP_ULONG   ulLumaSize;         ///< the size of luma image
	MMP_ULONG   ulImgBufAddr;       ///< the addr of dma destination source
    MMP_UBYTE   ubFrameGap;         ///< the gap (frame count) of the two frames for calculation
    MMP_ULONG   ulGapTimeMs;        ///< the gap time in unit of ms
} MMPF_ADAS_ATTRIBUTE;

/**	@brief	ADASEnabled indicates if ADAS is enabled or not */
/**	@brief	Indicates if each features in ADAS is enabled or not */
typedef struct _MMPF_ADAS_INSTANCE {
	MMP_UBYTE			SnrSel;
	MMP_UBYTE			RunOnCPUX; // 0:CPUA  1:CPUB
	MMP_BOOL            ADASEnabled;
	MMP_BOOL            LDWSEnabled;
	MMP_BOOL            FCWSEnabled;
	MMP_BOOL            SnGEnabled;
	MMPF_ADAS_ATTRIBUTE	Attr;
}MMPF_ADAS_INSTANCE;

#endif

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

typedef void MMPF_ADAS_Callback(void);

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#if (CPU_ID == CPU_A)
MMP_ERR     MMPF_ADAS_ProcessCmd(void);
MMP_BOOL    MMPF_ADAS_IsEnable(MMP_UBYTE ubSnrSel);
MMP_UBYTE   MMPF_LDWS_GetFrameGapConfig(MMP_UBYTE ubSnrSel);
void        MMPF_LDWS_UpdateInputLumaAddr(MMP_UBYTE ubSnrSel, MMP_ULONG ulSrcLumaAddr);
MMP_UBYTE   MMPF_LDWS_GetFrameGapConfig(MMP_UBYTE ubSnrSel);
MMP_ERR     MMPF_ADAS_DmaCopy(MMP_UBYTE ubSnrSel);
MMP_ERR     MMPF_ADAS_Operate(MMP_UBYTE ubSnrSel);
MMPF_ADAS_STATE MMPF_ADAS_GetState(MMP_UBYTE ubSnrSel);
#endif

#endif // (VIDEO_R_EN)&&(SUPPORT_ADAS)
#endif // _MMPF_LDWS_H_
/// @}
