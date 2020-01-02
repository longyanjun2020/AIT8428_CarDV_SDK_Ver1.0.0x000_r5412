#ifndef _MMPS_UVCRECD_H_
#define _MMPS_UVCRECD_H_

//===============================================================================
//
//                               INCLUDE FILE
//
//===============================================================================

#include "config_fw.h"
#include "mmp_usb_inc.h"

//===============================================================================
//
//                               MACRO DEFINE
//
//===============================================================================

#if (SUPPORT_USB_HOST_FUNC)
#define MMPS_3GPRECD_UVC_CHIP_STR_LEN   (8) // Sync (MMPF_USBH_UVC_CHIP_STR_LEN)
#define MMPS_3GPRECD_UVC_CFG_MAX_NUM    (8) // Sync (MMPF_USBH_DEV_CFG_MAX_NUM)
#endif

//===============================================================================
//
//                               ENUMERATION
//
//===============================================================================

typedef enum _MMPS_UVCRECD_FILE_TYPE {
    MMPS_UVCRECD_MULFILE,
    MMPS_UVCRECD_MULTRACK
} MMPS_UVCRECD_FILE_TYPE;

//===============================================================================
//
//                               STRUCTURES
//
//===============================================================================

typedef struct _MMPS_UVCRECD_CONTAINER_INFO {
    MMP_USHORT      VideoEncodeFormat;  ///< video encode format
    MMP_ULONG       ulFrameWidth;       ///< video frame width
    MMP_ULONG       ulFrameHeight;      ///< video frame height
    MMP_ULONG       ulTimeIncrement;    ///< video time increment
    MMP_ULONG       ulTimeResolution;   ///< video time resolution   
    MMP_USHORT      usPFrameCount;      ///< P frames count in one GOP
    MMP_USHORT      usBFrameCount;   	///< # consecutive B frames
    
    MMP_ULONG       ulSPSAddr;
    MMP_ULONG       ulPPSAddr;
    MMP_USHORT      ulSPSSize;
    MMP_USHORT      ulPPSSize;
    MMP_USHORT      ulProfileIDC;
    MMP_USHORT      ulLevelIDC;
} MMPS_UVCRECD_CONTAINER_INFO;

//===============================================================================
//
//                               FUNCTION PROTOTYPES
//
//===============================================================================

MMP_ERR MMPS_UVCRECD_SetCustomedPrevwAttr(MMP_BOOL 	    bUserConfig,
										  MMP_BOOL 	    bRotate,
										  MMP_UBYTE 	ubRotateDir,
										  MMP_UBYTE	    sFitMode,
										  MMP_USHORT    usBufWidth, MMP_USHORT usBufHeight, 
										  MMP_USHORT    usStartX, 	MMP_USHORT usStartY,
                                      	  MMP_USHORT    usWinWidth, MMP_USHORT usWinHeight);
MMP_ERR MMPS_UVCRECD_GetCustomedPrevwAttr(MMP_BOOL 	    *pbUserConfig,
										  MMP_BOOL 	    *pbRotate,
										  MMP_UBYTE 	*pubRotateDir,
										  MMP_UBYTE	    *psFitMode,
										  MMP_USHORT    *pusBufWidth,   MMP_USHORT *pusBufHeight, 
										  MMP_USHORT    *pusStartX,     MMP_USHORT *pusStartY,
                                      	  MMP_USHORT    *pusWinWidth,   MMP_USHORT *pusWinHeight);

#if (SUPPORT_USB_HOST_FUNC)
MMP_ERR MMPS_UVCRECD_StartPreview(void);
MMP_ERR MMPS_UVCRECD_StopPreview(void);
MMP_ERR MMPS_UVCRECD_SetPrevwWinID(MMP_UBYTE ubWinID);
MMP_ERR MMPS_UVCRECD_SetUVCRecdResol(MMP_UBYTE ubResol);
MMP_ERR MMPS_UVCRECD_SetRecdBitrate(MMP_ULONG ulBitrate);
MMP_ERR MMPS_UVCRECD_SetRecdFrameRate(MMP_USHORT usTimeIncrement, MMP_USHORT usTimeIncrResol);
MMP_ERR MMPS_UVCRECD_SetRecdPFrameCount(MMP_USHORT usFrameCount);
MMP_ERR MMPS_UVCRECD_SetPrevwResol(MMP_USHORT usWidth, MMP_USHORT usHeight);
MMP_ERR MMPS_UVCRECD_SetUVCPrevwRote(MMP_GRAPHICS_ROTATE_TYPE ubRoteType);
MMP_ERR MMPS_UVCRECD_SetPrevwStrmTyp(MMP_UBYTE ubPrevwStrmTyp);
MMP_ERR MMPS_UVCRECD_SetPrevwFrameRate(MMP_UBYTE ubFps);
MMP_ERR MMPS_UVCRECD_AddDevCFG(MMP_UBYTE *pubStr, void *pOpenDevCallback, void *pStartDevCallback, void *pNaluInfo);
MMP_ERR MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_UVC_CFG_OP Event, MMP_UBYTE *pubStr, void *pParm);
MMP_ERR MMPS_UVCRECD_RegDevAddiCFG(MMP_ULONG ulRegTyp, MMP_UBYTE *pubStr, MMP_ULONG ulParm0, MMP_ULONG ulParm1, MMP_ULONG ulParm2, MMP_ULONG ulParm3);
MMP_ERR MMPS_UVCRECD_SetClassIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_UBYTE *UVCDataBuf);
MMP_ERR MMPS_UVCRECD_GetClassIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_ULONG *UVCDataLength, MMP_UBYTE *UVCDataBuf);
MMP_ERR MMPS_UVCRECD_SetStdIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_UBYTE *UVCDataBuf);
MMP_ERR MMPS_UVCRECD_GetStdDevCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_ULONG *UVCDataLength, MMP_UBYTE *UVCDataBuf);
MMP_ERR MMPS_UVCRECD_SetDevTotalCount(MMP_UBYTE ubCount);
#endif

#if (UVC_VIDRECD_SUPPORT)
MMP_ERR MMPS_UVCRECD_SetUVCRecdSupport(MMP_BOOL bSupport);
MMP_ERR MMPS_UVCRECD_GetUVCRecdSupport(MMP_BOOL *bSupport);
MMP_ERR MMPS_UVCRECD_StartRecd(MMP_UBYTE type);
MMP_ERR MMPS_UVCRECD_OpenRecdFile(void);
MMP_ERR MMPS_UVCRECD_EnableRecd(void);
MMP_ERR MMPS_UVCRECD_StopRecd(void);
MMP_ERR MMPS_UVCRECD_RecdInputFrame(MMP_ULONG bufaddr, MMP_ULONG size, MMP_ULONG timestamp,
                                    MMP_USHORT frametype, MMP_USHORT vidaudtype);
MMP_ERR MMPS_UVCRECD_SetRecdInfo(MMPS_UVCRECD_CONTAINER_INFO *pInfo);
#endif

#endif // _MMPS_UVCRECD_H_