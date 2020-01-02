//==============================================================================
//
//  File        : mmpf_display.h
//  Description : INCLUDE File for the Firmware Display Control driver function, including LCD/TV/Win
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_DISPLAY_H_
#define _MMPF_DISPLAY_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "lcd_common.h"
#include "mmp_ibc_inc.h"
#include "mmp_display_inc.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define DISP_START_PREVIEW_SEM_TIMEOUT      (3000)
#define DISP_STOP_PREVIEW_SEM_TIMEOUT       (3000)

#define DISP_DISABLE_PREVIEW_DELAY_COUNT    (0xFE)

//==============================================================================
//
//                              EXTERN VARIABLE
//
//==============================================================================

extern MMP_IBC_BUF_ATTR gsIBCBufAttr[MMP_IBC_PIPE_MAX];

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#if (AIT_REAR_CAM_VR_CAPTURE_EN)
MMP_ERR MMPF_Display_StartBuf2StillJpeg(void);
#endif

// For Dual Bayer Sensor
void IBCCallbackFunc_SrcPipeFrameSt(void* argu);
void IBCCallbackFunc_EncPipeFrameSt(void* argu);
void IBCCallbackFunc_WifiPipeFrameSt(void* argu);

// Buffer Operation Function
MMP_UBYTE MMPF_IBC_GetReadyBufIdx(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId);
MMP_UBYTE MMPF_IBC_GetUsableBufIdx(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId);
void MMPF_IBC_SetReadyBufIdx(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId, MMP_UBYTE ubIdx);
void MMPF_IBC_SetUsableBufIdx(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId, MMP_UBYTE ubIdx);
void MMPF_IBC_UpdateReadyBufIdx(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId, MMP_UBYTE ubOffset);
void MMPF_IBC_UpdateUsableBufIdx(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId, MMP_UBYTE ubOffset);
void MMPF_IBC_ResetBufIdx(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId);

MMP_ULONG MMPF_IBC_GetBufYAddr(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId, MMP_UBYTE ubIdx);
MMP_ULONG MMPF_IBC_GetBufUAddr(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId, MMP_UBYTE ubIdx);
MMP_ULONG MMPF_IBC_GetBufVAddr(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId, MMP_UBYTE ubIdx);

// Basic Function
MMP_UBYTE MMPF_Display_GetActivePipeNum(MMP_UBYTE ubSnrSel);
MMP_ERR MMPF_Display_GetActivePipeId(MMP_UBYTE ubSnrSel, MMP_UBYTE *pPipe, MMP_UBYTE ubSearchNum);
MMP_ERR MMPF_Display_SetCCIRActivePipeSnr(MMP_UBYTE ubActivePipeSnr);
MMP_ERR MMPF_Display_StartSoftwareRefresh(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId);
MMP_ERR MMPF_Display_ResetSnrInitFlag(MMP_UBYTE ubSnrSel);
MMP_ERR MMPF_Display_StartPreview(MMP_UBYTE ubSnrSel, MMP_USHORT usIBCPipe, MMP_BOOL bCheckFrameEnd);
MMP_ERR MMPF_Display_StopPreview(MMP_UBYTE ubSnrSel, MMP_USHORT usIBCPipe, MMP_BOOL bCheckFrameEnd);
MMP_ERR MMPF_Display_StopFlushWin(MMP_UBYTE ubSnrSel, MMP_USHORT usIBCPipe, MMP_BOOL benable);
MMP_ERR MMPF_Display_GetPreviewSts(MMP_UBYTE *pubStatus);
MMP_ERR MMPF_Display_FrameDoneTrigger(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId);
MMP_ERR MMPF_Display_DMARotateFrame(MMP_UBYTE ubPipeId, MMP_UBYTE ubUsageId);
MMP_ERR MMPF_Display_WaitRotateDMA(MMP_UBYTE ubPipeId);
MMP_ERR MMPF_Display_ResetRotateDMABufIdx(MMP_UBYTE ubCamType);
MMP_ERR MMPF_Display_SetRefresh(MMP_DISPLAY_CONTROLLER controller);
MMP_ERR MMPF_Display_GetExposedFrameBuffer(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId, MMP_ULONG *pulYAddr, MMP_ULONG *pulUAddr, MMP_ULONG *pulVAddr);
MMP_ERR MMPF_Display_GetCurIbcFrameBuffer(MMP_USHORT usIBCPipe, MMP_UBYTE ubUsageId, MMP_ULONG *pulYAddr, MMP_ULONG *pulUAddr, MMP_ULONG *pulVAddr);

// FLM Function
MMP_ERR MMPF_Display_StartFLMRefresh(void);
MMP_ERR MMPF_Display_StopFLMRefresh(void);
MMP_ERR MMPF_Display_SetFLMRefresh(MMP_DISPLAY_CONTROLLER ctrl, MMP_BOOL bWait);

// Output Function
MMP_ERR MMPF_Display_SetNoneOutput(MMP_DISPLAY_CONTROLLER controller);
MMP_ERR MMPF_Display_SetPLCDOutput( MMP_DISPLAY_CONTROLLER  controller,
                                    MMP_LCD_CS_SEL          cssel, 
                                    MMP_LCD_ATTR            *pLcdAttr);
MMP_ERR MMPF_Display_SetRGBLCDOutput(MMP_DISPLAY_CONTROLLER controller,MMP_LCD_ATTR *pLcdAttr,
                                     MMP_DISPLAY_RGB_IF rgbIf);
MMP_ERR MMPF_Display_DisableRGBOutput(MMP_DISPLAY_CONTROLLER controller,MMP_DISPLAY_RGB_IF ubRgbIf);
MMP_ERR MMPF_Display_GetOutputDev(MMP_DISPLAY_CONTROLLER controller, MMP_DISPLAY_DEV_TYPE *pOutput);
MMP_ERR MMPF_Display_SetOutputDev(MMP_DISPLAY_CONTROLLER controller, MMP_DISPLAY_DEV_TYPE Output);
MMP_ERR MMPF_Display_GetWidthHeight(MMP_DISPLAY_CONTROLLER controller, MMP_USHORT *usWidth, MMP_USHORT *usHeight);
MMP_ERR MMPF_Display_WaitDisplayRefresh(MMP_DISPLAY_CONTROLLER controller);
MMP_ERR MMPF_Display_SetDisplayRefresh(MMP_DISPLAY_CONTROLLER controller);

// LCD Function
MMP_ERR MMPF_LCD_SendCommand(MMP_ULONG ulCommand);
MMP_ERR MMPF_LCD_SendIndex(MMP_ULONG ulIndex);
MMP_ERR MMPF_LCD_SendIndexCommand(MMP_ULONG ulIndex, MMP_ULONG ulCommand);
MMP_ERR MMPF_LCD_GetIndexCommand(MMP_ULONG ulIndex, MMP_ULONG *ulCommand, MMP_ULONG ulReadCount);

// RGB Function
MMP_ERR MMPF_RGBLCD_WaitFrame(MMP_UBYTE ubFrameCount, MMP_DISPLAY_RGB_IF rgbIf);
MMP_ERR MMPF_RGBLCD_SetPartialPosition(MMP_DISPLAY_RECT *pRect, MMP_DISPLAY_RGB_IF rgbIf);
MMP_ERR MMPF_RGBLCD_GetDotClk(MMP_ULONG *ulRGBDotClk);

// Window Function
void    MMPF_Display_SetFreezeWinEn(MMP_BOOL En);
MMP_BOOL MMPF_Display_GetFreezeWinEn(void);
void    MMPF_Display_FreezeWinUpdate(MMP_BOOL PIPEn, MMP_BOOL OVLYEn, MMP_BOOL OSDEn);
void    MMPF_Display_SetTempMemHeapEnd(MMP_ULONG addr);
MMP_ERR MMPF_Display_GetWinAttributes(MMP_DISPLAY_WIN_ID winID, MMP_DISPLAY_WIN_ATTR *pWinAttr);
MMP_ERR MMPF_Display_SetWinAttributes(MMP_DISPLAY_WIN_ID winID, MMP_DISPLAY_WIN_ATTR *winAttr);
MMP_ERR MMPF_Display_ClearWindowBuf(MMP_DISPLAY_WIN_ATTR    *pWinAttr,
                                    MMP_GRAPHICS_RECT       *rect, 
                                    MMP_ULONG               ulClearColor);
MMP_ERR MMPF_Display_GetWinBindController(MMP_DISPLAY_WIN_ID        winID,
                                          MMP_DISPLAY_CONTROLLER    *pController);
MMP_ERR MMPF_Display_BindWinToController(MMP_DISPLAY_WIN_ID     winID,
                                         MMP_DISPLAY_CONTROLLER controller);
MMP_ERR MMPF_Display_BindBufToWin(MMP_GRAPHICS_BUF_ATTR *pBufAttr, MMP_DISPLAY_WIN_ID winID);
MMP_ERR MMPF_Display_UpdateWinAddr( MMP_DISPLAY_WIN_ID  winID,
                                    MMP_ULONG           ulBaseAddr,
                                    MMP_ULONG           ulBaseUAddr,
                                    MMP_ULONG           ulBaseVAddr);
MMP_ERR MMPF_Display_SetWinToDisplay(MMP_DISPLAY_WIN_ID winID, MMP_DISPLAY_DISP_ATTR *dispAttr);
MMP_ERR MMPF_Display_SetWinDuplicate(MMP_DISPLAY_WIN_ID     winID, 
                                     MMP_DISPLAY_DUPLICATE  h_ratio, 
                                     MMP_DISPLAY_DUPLICATE  v_ratio);
MMP_ERR MMPF_Display_SetWinScaling( MMP_DISPLAY_WIN_ID  winID, 
                                    MMP_SCAL_FIT_RANGE  *fitrange,
                                    MMP_SCAL_GRAB_CTRL  *grabctl);
MMP_ERR MMPF_Display_SetWinScaleAndOffset(  MMP_DISPLAY_WIN_ID  winID,
                                            MMP_SCAL_FIT_RANGE  *fitrange,
                                            MMP_SCAL_GRAB_CTRL  *grabctl,
                                            MMP_USHORT          usWinOffsetX,
                                            MMP_USHORT          usWinOffsetY);
MMP_ERR MMPF_Display_SetWinPriority(MMP_DISPLAY_WIN_ID prio1,
                                    MMP_DISPLAY_WIN_ID prio2,
                                    MMP_DISPLAY_WIN_ID prio3,
                                    MMP_DISPLAY_WIN_ID prio4);
MMP_ERR MMPF_Display_LockWindow(MMP_DISPLAY_WIN_ID winID);
MMP_ERR MMPF_Display_UnlockWindow(MMP_DISPLAY_WIN_ID winID);
MMP_ERR MMPF_Display_SetWinActive(MMP_DISPLAY_WIN_ID winID, MMP_BOOL bActive);
MMP_ERR MMPF_Display_GetWinActive(MMP_DISPLAY_WIN_ID winID, MMP_BOOL *bActive);
MMP_ERR MMPF_Display_SetWinAddr(MMP_DISPLAY_WIN_ID winID, MMP_ULONG ulBaseAddr,
                                MMP_ULONG ulBaseUAddr, MMP_ULONG ulBaseVAddr);
MMP_ULONG MMPF_Display_SetWinTransparent(MMP_DISPLAY_WIN_ID winID,
                                         MMP_BOOL           bTranspActive, 
                                         MMP_ULONG          ulTranspColor);
MMP_ERR MMPF_Display_SetWinSemiTransparent( MMP_DISPLAY_WIN_ID          winID, 
                                            MMP_BOOL                    bSemiTpActive, 
                                            MMP_DISPLAY_SEMITP_FUNC     semifunc,
                                            MMP_USHORT                  usSemiWeight);
MMP_ERR MMPF_Display_GetWinSemiTransparent(MMP_DISPLAY_WIN_ID winID, MMP_USHORT* usSemiWeight);
MMP_ERR MMPF_Display_SetAlphaBlending(MMP_DISPLAY_WIN_ID        winID,
                                      MMP_BYTE                  bAlphaEn,
                                      MMP_DISPLAY_ALPHA_FMT     alphaformat);
MMP_ERR	MMPF_Display_SetAlphaWeight(MMP_UBYTE* ubAlphaWeight);
MMP_ERR MMPF_Display_LoadWinPalette(MMP_DISPLAY_WIN_ID winID, MMP_UBYTE *ubPalette, MMP_USHORT usEntry);
MMP_ERR MMPF_Display_AdjustWinColorMatrix(  MMP_DISPLAY_WIN_ID      winID, 
                                            MMP_DISPLAY_WIN_COLRMTX *DspyColrMtx, 
                                            MMP_BOOL                bApplyColMtx); 
MMP_ERR MMPF_Display_SetYuvAttribute(MMP_BYTE ubU2U,
                                     MMP_BYTE ubV2U,
                                     MMP_BYTE ubU2V,
                                     MMP_BYTE ubV2V,
                                     MMP_BYTE YGain,
                                     MMP_BYTE Yoffset);

// TV Function
MMP_ERR MMPF_TV_Initialize(void);
MMP_ERR MMPF_TV_Uninitialize(void);
MMP_ERR MMPF_TV_ColorBar(MMP_TV_TYPE TVtype, MMP_BOOL turnOn, MMP_UBYTE colorBarType);
MMP_ERR MMPF_TV_SetInterlaceMode(MMP_BOOL bEnable);
MMP_ERR MMPF_TV_PowerDownDAC(void);

// HDMI Function
MMP_ERR MMPF_HDMI_PowerDownPHY(void);
MMP_BOOL MMPF_HDMI_IsConnect(void);
MMP_ERR MMPF_HDMI_ParseEDID(MMP_UBYTE *pEDID, MMP_ULONG *pSpportFormat, MMP_ULONG *pEDIDType);

#endif // _MMPF_DISPLAY_H_
