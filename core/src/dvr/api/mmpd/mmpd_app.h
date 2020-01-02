/**
 *  @file mmpd_app.h
 *  @brief Header file for the Customer Application Driver
 *  @author Alterman
 *  @version 1.0
 */
#ifndef _MMPD_APP_H_
#define _MMPD_APP_H_

#include "mmp_lib.h"

/** @addtogroup MMPD_APP
@{
*/

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================

//==============================================================================
//
//                              CONSTANTS
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

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
#if (ARCSOFT_SDTC_EN)
MMP_ERR MMPD_APP_AllocationSmileDetectBuf(MMP_ULONG ulWorkBufAddr, MMP_ULONG ulWorkBufSize, 
                                          MMP_ULONG ulInputBufAddr, MMP_ULONG ulInputBufSize);
MMP_ERR MMPD_APP_SetSmileDetectEnable(MMP_BOOL bEnable);
#endif
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}

#endif // _MMPS_APP_H_
