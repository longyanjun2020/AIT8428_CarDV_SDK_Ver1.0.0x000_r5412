/// @ait_only
/**
 @file mmpd_3gpmgr.h
 @brief Header File for the Host 3GP MERGER Driver.
 @author Will Tseng
 @version 1.0
*/

#ifndef _MMPD_UVCRECD_H_
#define _MMPD_UVCRECD_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "includes_fw.h"
#include "mmp_vidrec_inc.h"
#include "mmp_graphics_inc.h"
#include "mmp_usb_inc.h"
#include "mmpd_3gpmgr.h"

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

/** @addtogroup MMPD_3GPMGR
 *  @{
 */

// UVC Record Function
MMP_ERR MMPD_UVCRECD_EnableEmergentRecd(MMP_BOOL bEnabled);
MMP_ERR MMPD_UVCRECD_StartRecd(MMP_UBYTE type);
MMP_ERR MMPD_UVCRECD_OpenFile(void);
MMP_ERR MMPD_UVCRECD_EnableRecd(void);
MMP_ERR MMPD_UVCRECD_StopRecd(void);
MMP_ERR MMPD_UVCRECD_SetRecdSupport(MMP_BOOL bSupport);
MMP_ERR MMPD_UVCRECD_SetRepackMiscBuf(MMP_ULONG ulStreamType, MMPD_3GPMGR_REPACKBUF *repackbuf);
MMP_ERR MMPD_UVCRECD_RecdInputFrame(MMP_ULONG bufaddr, MMP_ULONG size, MMP_ULONG timestamp,
									MMP_USHORT frametype, MMP_USHORT vidaudtype);
MMP_ERR MMPD_UVCRECD_AllocFBMemory(MMP_LONG plStartAddr, MMP_ULONG *plSize);
MMP_ERR MMPD_UVCRECD_StartPrevw(void);
MMP_ERR MMPD_UVCRECD_StopPrevw(void);
MMP_ERR MMPD_UVCRECD_SetPrevwWinID(MMP_UBYTE ubWinID);
MMP_ERR MMPD_UVCRECD_SetPrevwRote(MMP_GRAPHICS_ROTATE_TYPE ubRoteType);
MMP_ERR MMPD_UVCRECD_SetRecdResol(MMP_UBYTE ubResol, MMP_USHORT usWidth, MMP_USHORT usHeight);
MMP_ERR MMPD_UVCRECD_SetRecdBitrate(MMP_ULONG ulBitrate);
MMP_ERR MMPD_UVCRECD_SetRecdFrameRate(MMP_USHORT usTimeIncrement, MMP_USHORT usTimeIncrResol);
MMP_ERR MMPD_UVCRECD_SetRecdPFrameCount(MMP_USHORT usFrameCount);
MMP_ERR MMPD_UVCRECD_SetPrevwResol(MMP_USHORT usWidth, MMP_USHORT usHeight);
MMP_ERR MMPD_UVCRECD_IsPrevwResolSet(MMP_BOOL *pbIsSet);
MMP_ERR MMPD_UVCRECD_SetPrevwStrmTyp(MMP_UBYTE ubPrevwStrmTyp);
MMP_ERR MMPD_UVCRECD_IsDecMjpeg2Prevw(MMP_BOOL *pbIsDecMjpeg2Prevw);
MMP_ERR MMPD_UVCRECD_SetPrevwFrameRate(MMP_UBYTE ubFps);
MMP_ERR MMPD_UVCRECD_AddDevCFG(MMP_UBYTE *pubStr, void *pOpenDevCallback, void *pStartDevCallback, void *pNaluInfo);
MMP_ERR MMPD_UVCRECD_UpdDevCFG(MMP_USBH_UPD_UVC_CFG_OP Event, MMP_UBYTE *pubStr, void *pParm);
MMP_ERR MMPD_UVCRECD_SetUVCDevTotalCount(MMP_UBYTE ubCount);
MMP_ERR MMPD_UVCRECD_RegDevAddiCFG(MMP_ULONG ulRegTyp, MMP_UBYTE *pubStr, MMP_ULONG ulParm0, MMP_ULONG ulParm1, MMP_ULONG ulParm2, MMP_ULONG ulParm3);
MMP_ERR MMPD_UVCRECD_SetClassIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_UBYTE *UVCDataBuf);
MMP_ERR MMPD_UVCRECD_GetClassIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_ULONG *UVCDataLength, MMP_UBYTE *UVCDataBuf);
MMP_ERR MMPD_UVCRECD_SetStdIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_UBYTE *UVCDataBuf);
MMP_ERR MMPD_UVCRECD_GetStdDevCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_ULONG *UVCDataLength, MMP_UBYTE *UVCDataBuf);
MMP_ERR MMPD_UVCRECD_SetEncodeFormat(MMP_USHORT usStreamType, MMP_USHORT usFormat);
MMP_ERR MMPD_UVCRECD_SetEncodeResolution(MMP_USHORT usStreamType, MMP_USHORT ResolW, MMP_USHORT ResolH);

MMP_ERR MMPD_UVCRECD_SetEncodeGOP(MMP_USHORT usStreamType, MMP_USHORT usPFrame, MMP_USHORT usBFrame);	
MMP_ERR MMPD_UVCRECD_SetEncodeSPSPPSHdr(MMP_USHORT usStreamType, MMP_ULONG ulSPSAddr, MMP_USHORT ulSPSSize, MMP_ULONG ulPPSAddr, 
									    MMP_USHORT ulPPSSize, MMP_USHORT ulProfileIDC, MMP_USHORT ulLevelIDC);

#endif // _MMPD_UVCRECD_H_

/// @end_ait_only
