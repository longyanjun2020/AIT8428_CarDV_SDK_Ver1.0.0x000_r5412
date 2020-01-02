//==============================================================================
//
//  File        : mmpf_fdtc.h
//  Description : INCLUDE File for the face detection Driver Function
//  Author      : Rogers Chen
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_FDTC_H_
#define _MMPF_FDTC_H_

#include "includes_fw.h"
#if ((DSC_R_EN)||(VIDEO_R_EN))&&(SUPPORT_MDTC)
#include "mmpf_vmd.h"
#endif
#if (VIDEO_R_EN)&&(SUPPORT_ADAS)
#include "mmpf_adas.h"
#endif

/** @addtogroup MMPF_FDTC
@{
*/
#if (DSC_R_EN)||(VIDEO_R_EN)

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================

/* flags for FDTC */
#define FLAG_FDTC_DMACOPY       (0x00000001)
#define FLAG_FDTC_OPER          (0x00000002)
#define SYS_FLAG_FDTC           (0x00000004)
/* flags for VMD */
#define SYS_FLAG_VMD            (0x00000008)
#define FLAG_PRM_SNR_VMD_OPER   (0x00000010)
#define FLAG_SCD_SNR_VMD_OPER   (0x00000020)
/* flags for ADAS */
#define SYS_FLAG_ADAS           	(0x00000080)
#define FLAG_PRM_SNR_ADAS_DMACOPY   (0x00000100)
#define FLAG_SCD_SNR_ADAS_DMACOPY   (0x00000200)
#define FLAG_PRM_SNR_ADAS_OPER      (0x00000400)
#define FLAG_SCD_SNR_ADAS_OPER      (0x00000800)
/* flags for AUDRECOG */
#define SYS_FLAG_AUD            (0x00001000)
#define FLAG_PCM_AUDRECOG_OPER   (0x00002000)

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

extern MMPF_OS_FLAGID   CpuB_Algorithm_Flag;
extern MMPF_OS_FLAGID   CpuA_Algorithm_Flag;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#endif

#endif // _MMPF_FDTC_H_
/// @}
