/// @ait_only
//==============================================================================
//
//  File        : mmpd_iva.h
//  Description : Intelligent video analysis function
//  Author      :
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPD_IVA_H_
#define _MMPD_IVA_H_

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#if (SUPPORT_MDTC)
MMP_ERR MMPD_Sensor_InitializeVMD(MMP_UBYTE ubSnrSel, MMP_UBYTE RunOnCPUX, MMP_ULONG ResolW, MMP_ULONG ResolH, MMP_UBYTE FrameGap);
MMP_ERR MMPD_Sensor_SetVMDBuf(MMP_UBYTE ubSnrSel, MMP_ULONG buf_addr, MMP_ULONG *buf_size);
MMP_ERR MMPD_Sensor_GetVMDResolution(MMP_UBYTE ubSnrSel, MMP_ULONG *width, MMP_ULONG *height);
MMP_ERR MMPD_Sensor_RegisterVMDCallback(MMP_UBYTE ubSnrSel, void *Callback);
MMP_ERR MMPD_Sensor_StartVMD(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable);
#endif

#if (SUPPORT_ADAS)
MMP_ERR MMPD_Sensor_InitializeADAS(MMP_UBYTE ubSnrSel, MMP_UBYTE RunOnCPUX, MMP_ULONG ResolW, MMP_ULONG ResolH, MMP_UBYTE FrameGap);
MMP_ERR MMPD_Sensor_SetADASBuf(MMP_UBYTE ubSnrSel, MMP_ULONG buf_addr, MMP_ULONG *buf_size, MMP_ULONG dma_buf_addr);
MMP_ERR MMPD_Sensor_GetADASResolution(MMP_UBYTE ubSnrSel, MMP_ULONG *width, MMP_ULONG *height);
MMP_ERR MMPD_Sensor_SetADASFeature(MMP_UBYTE ubSnrSel, MMP_BOOL ldws_on, MMP_BOOL fcws_on, MMP_BOOL sag_on);
MMP_ERR MMPD_Sensor_StartADAS(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable);
#endif

#endif //_MMPD_IVA_H_