
//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_lib.h"
#include "ait_utility.h"
#include "mmps_3gprecd.h"
#include "mmps_uvcrecd.h"
#include "mmpd_mp4venc.h"
#include "mmpd_uvcrecd.h"
#include "mmpf_usbh_uvc.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

MMP_BOOL 	                            m_bUVCRecdSupport       = MMP_FALSE;
static MMPS_3GPRECD_AHC_PREVIEW_INFO 	m_sAhcVideoUVCPrevInfo  = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void ____UVC_Rec_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetCustomedPrevwAttr
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set video preview resolution.
 @param[in] bUserConfig		Use user defined preview configuration.
 @param[in] bRotate			Use DMA rotate to rotate preview buffer.
 @param[in] ubRotateDir		DMA rotate direction.
 @param[in] sFitMode		Scaler fit mode.
 @param[in] usBufWidth		Preview buffer width.
 @param[in] usBufHeight		Preview buffer height.
 @param[in] usStartX 		The X Offset of the display window.
 @param[in] usStartY 		The Y Offset of the display window.
 @param[in] usWinWidth  	The width of the display window.
 @param[in] usWinHeight 	The height of the display window.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_UVCRECD_SetCustomedPrevwAttr(MMP_BOOL 	    bUserConfig,
										  MMP_BOOL 	    bRotate,
										  MMP_UBYTE 	ubRotateDir,
										  MMP_UBYTE	    sFitMode,
										  MMP_USHORT    usBufWidth, MMP_USHORT usBufHeight, 
										  MMP_USHORT    usStartX, 	MMP_USHORT usStartY,
                                      	  MMP_USHORT    usWinWidth, MMP_USHORT usWinHeight)
{
    m_sAhcVideoUVCPrevInfo.bUserDefine  	= bUserConfig;
    m_sAhcVideoUVCPrevInfo.bPreviewRotate 	= bRotate;
    m_sAhcVideoUVCPrevInfo.sPreviewDmaDir	= ubRotateDir;
    m_sAhcVideoUVCPrevInfo.sFitMode		= sFitMode;
    m_sAhcVideoUVCPrevInfo.ulPreviewBufW	= usBufWidth;
    m_sAhcVideoUVCPrevInfo.ulPreviewBufH 	= usBufHeight;
    m_sAhcVideoUVCPrevInfo.ulDispStartX  	= usStartX;
    m_sAhcVideoUVCPrevInfo.ulDispStartY  	= usStartY;
    m_sAhcVideoUVCPrevInfo.ulDispWidth   	= usWinWidth;
    m_sAhcVideoUVCPrevInfo.ulDispHeight  	= usWinHeight;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_GetCustomedPrevwAttr
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_UVCRECD_GetCustomedPrevwAttr(MMP_BOOL 	    *pbUserConfig,
										  MMP_BOOL 	    *pbRotate,
										  MMP_UBYTE 	*pubRotateDir,
										  MMP_UBYTE	    *psFitMode,
										  MMP_USHORT    *pusBufWidth,   MMP_USHORT *pusBufHeight, 
										  MMP_USHORT    *pusStartX,     MMP_USHORT *pusStartY,
                                      	  MMP_USHORT    *pusWinWidth,   MMP_USHORT *pusWinHeight)    
{
    *pbUserConfig   = m_sAhcVideoUVCPrevInfo.bUserDefine;
    *pbRotate       = m_sAhcVideoUVCPrevInfo.bPreviewRotate; 
    *pubRotateDir   = m_sAhcVideoUVCPrevInfo.sPreviewDmaDir;
    *psFitMode      = m_sAhcVideoUVCPrevInfo.sFitMode;
    *pusBufWidth    = m_sAhcVideoUVCPrevInfo.ulPreviewBufW;
    *pusBufHeight   = m_sAhcVideoUVCPrevInfo.ulPreviewBufH;
    *pusStartX      = m_sAhcVideoUVCPrevInfo.ulDispStartX;
    *pusStartY      = m_sAhcVideoUVCPrevInfo.ulDispStartY;
    *pusWinWidth    = m_sAhcVideoUVCPrevInfo.ulDispWidth;
    *pusWinHeight   = m_sAhcVideoUVCPrevInfo.ulDispHeight;

    return MMP_ERR_NONE;
}

#if (UVC_VIDRECD_SUPPORT)
//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetUVCRecdSupport
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Enable UVC video recording.

 Enable UVC video recording.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_UVCRECD_SetUVCRecdSupport(MMP_BOOL bSupport)
{
    m_bUVCRecdSupport = bSupport;
    
    return MMPD_UVCRECD_SetRecdSupport(bSupport);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_GetUVCRecdSupport
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get UVC video recording enable states.

Get UVC video recording enable states.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_UVCRECD_GetUVCRecdSupport(MMP_BOOL *bSupport)
{
    *bSupport = m_bUVCRecdSupport;
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_StartRecd
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Start UVC video recording.

 Start to save the 3GP file.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_UVCRECD_StartRecd(MMP_UBYTE type)
{
    return MMPD_UVCRECD_StartRecd(type);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_OpenRecdFile
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Open UVC video recording file.

 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_UVCRECD_OpenRecdFile(void)
{
	#if (SUPPORT_UVC_FUNC)  
    MMP_BOOL bStatus;
 	 
    MMPF_USBH_GetUVCPrevwSts(&bStatus);
    
    if (bStatus == MMP_FALSE) {
        PRINTF("Enter preview first!\r\n");
        return MMP_USB_ERR_UNSUPPORT_MODE;
    }

    MMPF_USBH_GetUVCRecdSts(&bStatus);
    
    if (bStatus == MMP_TRUE) {
        PRINTF("[WARN]: RECDING \r\n");
        return MMP_USB_ERR_UNSUPPORT_MODE;
    }
	#endif
	
    #if (UVC_VIDRECD_SUPPORT) && (VIDRECD_MULTI_TRACK == 0)
    if (m_bUVCRecdSupport)
    {
        extern MMP_ULONG m_VidRecdID;
        if (MMPF_MP4VENC_GetStatus(m_VidRecdID) != VIDENC_FW_STATUS_PREENCODE) {
            MMPD_UVCRECD_OpenFile();
        }
    }
    #endif

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_EnableRecd
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Enable UVC video recording.

 Enable to inform USBH, and driver will start recording later when I-frame received.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_UVCRECD_EnableRecd(void)
{
    return MMPD_UVCRECD_EnableRecd();
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_StopRecd
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Stop UVC video recording.

 Stop UVC recording and save the 3GP file.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_UVCRECD_StopRecd(void)
{
    return MMPD_UVCRECD_StopRecd();
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_RecdInputFrame
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief input frame for UVC video recording.

 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_UVCRECD_RecdInputFrame(MMP_ULONG bufaddr, MMP_ULONG size, MMP_ULONG timestamp, 
                                       MMP_USHORT frametype, MMP_USHORT vidaudtype)
{
    return MMPD_UVCRECD_RecdInputFrame(bufaddr, size, timestamp, frametype, vidaudtype);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetRecdInfo
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set UVC video record information.

 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_UVCRECD_SetRecdInfo(MMPS_UVCRECD_CONTAINER_INFO *pInfo)
{
    MMP_USHORT  usStreamType = VIDENC_STREAMTYPE_UVCRECD;
    
    // Set video encode format
    if (pInfo->VideoEncodeFormat == MMPS_3GPRECD_VIDEO_FORMAT_H264) {
        MMPD_UVCRECD_SetEncodeFormat(usStreamType, VIDENC_FORMAT_H264);
    }
    
    // Set video encode resolution
    MMPD_UVCRECD_SetEncodeResolution(usStreamType, pInfo->ulFrameWidth, pInfo->ulFrameHeight);
    
    // Set video encode frame rate
    MMPD_3GPMGR_SetFrameRate(usStreamType, pInfo->ulTimeResolution, pInfo->ulTimeIncrement);
    
    // Set video encode GOP size
    MMPD_UVCRECD_SetEncodeGOP(usStreamType, pInfo->usPFrameCount, pInfo->usBFrameCount);
    
    // Set sps, pps header
    MMPD_UVCRECD_SetEncodeSPSPPSHdr(usStreamType, pInfo->ulSPSAddr, pInfo->ulSPSSize, pInfo->ulPPSAddr, 
                                   	pInfo->ulPPSSize, pInfo->ulProfileIDC, pInfo->ulLevelIDC);
    
    return MMP_ERR_NONE;
}
#endif

#if (SUPPORT_USB_HOST_FUNC)
//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_StartPreview
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Start UVC display preview mode, include stream on(UVC VS_COMMIT_CONTROL).
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS Unsupport resolution.
*/
MMP_ERR MMPS_UVCRECD_StartPreview(void)
{
    return MMPD_UVCRECD_StartPrevw();
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_StopPreview
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Stop UVC display preview mode, and stream off(UVC CLEAR_FEATURE).
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS Unsupport resolution.
*/
MMP_ERR MMPS_UVCRECD_StopPreview(void)
{
    return MMPD_UVCRECD_StopPrevw();
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetUVCDisplayWInID
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC display Window ID.
 @param[in] ubWinID the display window ID of UVC YUV stream used.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS Unsupport resolution.
*/
MMP_ERR MMPS_UVCRECD_SetPrevwWinID(MMP_UBYTE ubWinID)
{
    return MMPD_UVCRECD_SetPrevwWinID(ubWinID);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetUVCPrevwRote
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC preview rotate.
 @param[in] ubRoteType rotate type of UVC preview.
 @retval MMP_ERR_NONE Success.
 @return It reports the status of the operation.
*/
MMP_ERR MMPS_UVCRECD_SetUVCPrevwRote(MMP_GRAPHICS_ROTATE_TYPE ubRoteType)
{
    return MMPD_UVCRECD_SetPrevwRote(ubRoteType);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetUVCRecdResol
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC H264 encoded resolution.
 @param[in] usResol Resolution for UVC H264 record video.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS Unsupport resolution.
*/
MMP_ERR MMPS_UVCRECD_SetUVCRecdResol(MMP_UBYTE ubResol)
{
    return MMPD_UVCRECD_SetRecdResol(ubResol, 
                                     MMPS_3GPRECD_GetConfig()->usEncWidth[ubResol], 
                                     MMPS_3GPRECD_GetConfig()->usEncHeight[ubResol]);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetRecdBitrate
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC encoded quality.
 @param[in] ulBitrate Bitrate of UVC record video.
 @retval MMP_ERR_NONE Success.
 @note It must be set after choosing resolution and format.
 Here are recommended bitrate settings, "@30FPS", for reference

    Resolution          Visual Quality Level        Bitrate(bits/sec)
    ======================================================================
    640x480             Low                         500000
                        Mid                         750000
                        High                        1000000

    1280x720            Low                         2000000
                        Mid                         4000000
                        High                        8000000

    1920x1088           Low                         4000000
                        Mid                         8000000
                        High                        16000000
*/
MMP_ERR MMPS_UVCRECD_SetRecdBitrate(MMP_ULONG ulBitrate)
{
    return MMPD_UVCRECD_SetRecdBitrate(ulBitrate);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetRecdFrameRate
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set frame rate for UVC H264 recorded video.
 @param[in] usFrameRate Frame rate for UVC H264 record video.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_UVCRECD_SetRecdFrameRate(MMP_USHORT usTimeIncrement, MMP_USHORT usTimeIncrResol)
{
    return MMPD_UVCRECD_SetRecdFrameRate(usTimeIncrement, usTimeIncrResol);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetRecdPFrameCount
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set P frame count of one cycle.
 @param[in] usFrameCount Count of P frame.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_UVCRECD_SetRecdPFrameCount(MMP_USHORT usFrameCount)
{
    return MMPD_UVCRECD_SetRecdPFrameCount(usFrameCount);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetPrevwResol
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC YUV encoded resolution.
 @param[in] usWidth Resolution width for UVC YUV preview video.
 @param[in] usHeight Resolution height for UVC YUV preview video.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS Unsupport resolution.
*/
MMP_ERR MMPS_UVCRECD_SetPrevwResol(MMP_USHORT usWidth, MMP_USHORT usHeight)
{
    return MMPD_UVCRECD_SetPrevwResol(usWidth,usHeight);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetPrevwStrmTyp
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC preview stream type.
 @param[in] ubPrevwStrmTyp Stream type for UVC preview video.
 @retval MMP_ERR_NONE Success.
 @retval MMP_USB_ERR_UNSUPPORT_MODE Unsupport resolution.
*/
MMP_ERR MMPS_UVCRECD_SetPrevwStrmTyp(MMP_UBYTE ubPrevwStrmTyp)
{
    return MMPD_UVCRECD_SetPrevwStrmTyp(ubPrevwStrmTyp);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetPrevwFrameRate
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set frame rate for UVC YUV preview video.
 @param[in] ubFps Frame rate for UVC YUV preview video.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_UVCRECD_SetPrevwFrameRate(MMP_UBYTE ubFps)
{
    return MMPD_UVCRECD_SetPrevwFrameRate(ubFps);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_AddDevCFG
//  Description :
//------------------------------------------------------------------------------
/**
@brief The function sets UVC device following configs.
@param[in] pubStr Device Chip info strings.
@param[in] pOpenDevCallback The callback to be executed when USB device connected, one part of prob.
@param[in] pStartDevCallback The callback to be executed when UVC device commit(one part of commit).
@param[in] pNaluInfo The H264 nalu info, mejors are sps/pps related.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_UVCRECD_AddDevCFG(MMP_UBYTE *pubStr, void *pOpenDevCallback, void *pStartDevCallback, void *pNaluInfo)
{
    return MMPD_UVCRECD_AddDevCFG(pubStr, pOpenDevCallback, pStartDevCallback, pNaluInfo);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_UpdDevCFG
//  Description :
//------------------------------------------------------------------------------
/**
@brief The function sets to update UVC device following configs.
@param[in] Event Operation event as update open CV, start CB, and nalu table.
@param[in] pubStr Device Chip info strings.
@param[in] pParm The parameters to be updated info.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_UVC_CFG_OP Event, MMP_UBYTE *pubStr, void *pParm)
{
    return MMPD_UVCRECD_UpdDevCFG(Event, pubStr, pParm);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetDevTotalCount
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_UVCRECD_SetDevTotalCount(MMP_UBYTE ubCount)
{
    return MMPD_UVCRECD_SetUVCDevTotalCount(ubCount);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_RegDevAddiCFG
//  Description :
//------------------------------------------------------------------------------
/**
@brief The function sets to register UVC device info configs.
@param[in] ulRegTyp Register info type.
@param[in] pubStr Device Chip info strings.
@param[in] ulParm0 The parameter 0 to be registered.
@param[in] ulParm1 The parameter 1 to be registered.
@param[in] ulParm2 The parameter 2 to be registered.
@param[in] ulParm3 The parameter 3 to be registered.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_UVCRECD_RegDevAddiCFG(MMP_ULONG ulRegTyp, MMP_UBYTE *pubStr, MMP_ULONG ulParm0, MMP_ULONG ulParm1, MMP_ULONG ulParm2, MMP_ULONG ulParm3)
{
    return MMPD_UVCRECD_RegDevAddiCFG(ulRegTyp, pubStr, ulParm0,ulParm1,ulParm2,ulParm3);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetClassIfCmd
//  Description :
//------------------------------------------------------------------------------
/**
@brief Set UVC class IF command.
@param[in] bReq SETUP field bRequest.
@param[in] wVal SETUP field wValue.
@param[in] wInd SETUP field wIndex.
@param[in] wLen SETUP field wLength.
@param[in] UVCDataBuf received data.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_UVCRECD_SetClassIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_UBYTE *UVCDataBuf)
{
    return MMPD_UVCRECD_SetClassIfCmd(bReq, wVal, wInd, wLen, UVCDataBuf);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_GetClassIfCmd
//  Description :
//------------------------------------------------------------------------------
/**
@brief Get UVC class IF command.
@param[in] bReq SETUP field bRequest.
@param[in] wVal SETUP field wValue.
@param[in] wInd SETUP field wIndex.
@param[in] wLen SETUP field wLength.
@param[in] UVCDataLength exact received data length.
@param[in] UVCDataBuf received data.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_UVCRECD_GetClassIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_ULONG *UVCDataLength, MMP_UBYTE *UVCDataBuf)
{
    return MMPD_UVCRECD_GetClassIfCmd(bReq, wVal, wInd, wLen, UVCDataLength, UVCDataBuf);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_SetStdIfCmd
//  Description :
//------------------------------------------------------------------------------
/**
@brief Set UVC standard IF command.
@param[in] bReq SETUP field bRequest.
@param[in] wVal SETUP field wValue.
@param[in] wInd SETUP field wIndex.
@param[in] wLen SETUP field wLength.
@param[in] UVCDataBuf received data.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_UVCRECD_SetStdIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_UBYTE *UVCDataBuf)
{
    return MMPD_UVCRECD_SetStdIfCmd(bReq, wVal, wInd, wLen, UVCDataBuf);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_UVCRECD_GetStdDevCmd
//  Description :
//------------------------------------------------------------------------------
/**
@brief Get UVC standard DEV command.
@param[in] bReq SETUP field bRequest.
@param[in] wVal SETUP field wValue.
@param[in] wInd SETUP field wIndex.
@param[in] wLen SETUP field wLength.
@param[in] UVCDataLength exact received data length.
@param[in] UVCDataBuf received data.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_UVCRECD_GetStdDevCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_ULONG *UVCDataLength, MMP_UBYTE *UVCDataBuf)
{
    return MMPD_UVCRECD_GetStdDevCmd(bReq, wVal, wInd, wLen, UVCDataLength, UVCDataBuf);
}
#endif
