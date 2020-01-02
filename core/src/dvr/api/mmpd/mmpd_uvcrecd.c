/// @ait_only
/**
 @file mmpd_3gpmgr.c
 @brief Retina 3GP Merger Control Driver Function
 @author Will Tseng
 @version 1.0
*/

/** @addtogroup MMPD_3GPMGR
 *  @{
 */

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_lib.h"
#include "lib_retina.h"
#include "mmpd_uvcrecd.h"
#include "mmpd_3gpmgr.h"
#include "mmpd_mp4venc.h"
#include "mmph_hif.h"
#include "mmpf_3gpmgr.h"
#if (SUPPORT_USB_HOST_FUNC)
#include "mmpf_usbh_uvc.h"
#include "mmpf_usbh_ctl.h"
#endif

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void ____UVC_Record_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_EnableEmergentRecd
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Enable uvc emergent video recording.

 Enable uvc emergent video recording.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_EnableEmergentRecd(MMP_BOOL bEnabled)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, bEnabled);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_UVCEMERG_ENABLE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_StartRecd
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Start UVC video recording.

 Start to save the 3GP file.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_StartRecd(MMP_UBYTE type)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, type);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_UVCRECD_START);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_OpenFile
//  Description : 
//------------------------------------------------------------------------------
MMP_ERR MMPD_UVCRECD_OpenFile(void)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_UVCRECD_OPEN_FILE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_EnableRecd
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief STart UVC video recording.

 Enable to inform USBH, and driver will start recording later when I-frame received.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_EnableRecd(void)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetStartFrameofUVCRecd();
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_StopRecd
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Stop UVC video recording.

 Stop to save the 3GP file.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_StopRecd(void)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_UVCRECD_STOP);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetRecdSupport
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set UVC video recording support.

 Stop to save the 3GP file.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_SetRecdSupport(MMP_BOOL bSupport)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, bSupport);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_UVCRECD_SUPPORT);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_RecdInputFrame
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief input frame for UVC video recording.

 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_RecdInputFrame(MMP_ULONG bufaddr, MMP_ULONG size, MMP_ULONG timestamp, 
                                    MMP_USHORT frametype, MMP_USHORT vidaudtype)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, bufaddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, size);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, timestamp);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 12, frametype);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 14, vidaudtype);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_UVCRECD_INPUTFRAME);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_AllocFBMemory
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set start address to allocate buffers of UVC preview/record FB.
 @param[in] ulStartAddr the start memory address allowed to be used.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_AllocFBMemory(MMP_LONG plStartAddr, MMP_ULONG *plSize)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_AllocFBMemory(plStartAddr, plSize);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_StartPrevw
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Start UVC display preview mode.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_StartPrevw(void)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_StartUVCPrevw();
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_StopPrevw
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Stop UVC display preview mode.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_StopPrevw(void)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_StopUVCPrevw();
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetPrevwWinID
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set UVC display Window ID.
 @param[in] ubWinID the display window ID of UVC YUV stream used.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_SetPrevwWinID(MMP_UBYTE ubWinID)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetUVCPrevwWinID(ubWinID);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetPrevwRote
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC preview rotate.
 @param[in] ubRoteType rotate type of UVC preview.
 @retval MMP_ERR_NONE Success.
 @return It reports the status of the operation.
*/
MMP_ERR MMPD_UVCRECD_SetPrevwRote(MMP_GRAPHICS_ROTATE_TYPE ubRoteType)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetUVCPrevwRote(ubRoteType);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetRecdResol
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC H264 encoded resolution.
 @param[in] usResol Resolution for UVC H264 record video.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS Unsupport resolution.
*/
MMP_ERR MMPD_UVCRECD_SetRecdResol(MMP_UBYTE ubResol, MMP_USHORT usWidth, MMP_USHORT usHeight)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetUVCRecdResol(ubResol,usWidth,usHeight);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetRecdBitrate
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC encoded quality.
 @param[in] ulBitrate Bitrate of UVC record video.
 @retval MMP_ERR_NONE Success.
 @note It must be set after choosing resolution and format.
*/
MMP_ERR MMPD_UVCRECD_SetRecdBitrate(MMP_ULONG ulBitrate)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetUVCRecdBitrate(ulBitrate);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetRecdFrameRate
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set frame rate for UVC H264 recorded video.
 @param[in] usFrameRate Frame rate for UVC H264 record video.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_UVCRECD_SetRecdFrameRate(MMP_USHORT usTimeIncrement, MMP_USHORT usTimeIncrResol)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetUVCRecdFrameRate(usTimeIncrement, usTimeIncrResol);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetRecdPFrameCount
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set P frame count of one cycle.
 @param[in] ubFrameCnt Count of P frame.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_UVCRECD_SetRecdPFrameCount(MMP_USHORT usFrameCount)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetUVCRecdFrameCount(usFrameCount);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetPrevwResol
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC YUV encoded resolution.
 @param[in] usWidth Resolution width for UVC YUV preview video.
 @param[in] usHeight Resolution height for UVC YUV preview video.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS Unsupport resolution.
*/
MMP_ERR MMPD_UVCRECD_SetPrevwResol(MMP_USHORT usWidth, MMP_USHORT usHeight)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetUVCPrevwResol(usWidth,usHeight);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_IsPrevwResolSet
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get UVC preview resolution is set or not.
 @param[in] pbIsSet Get preview video resolution set or not.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS Unsupport resolution.
*/
MMP_ERR MMPD_UVCRECD_IsPrevwResolSet(MMP_BOOL *pbIsSet)
{
    MMP_ERR err = MMP_ERR_NONE;
    #if (SUPPORT_USB_HOST_FUNC)
    MMPF_USBH_UVC_STREAM_CFG *pUVCCfg = MMPF_USBH_GetUVCCFG();
    #endif

    *pbIsSet = MMP_FALSE;
    #if (SUPPORT_USB_HOST_FUNC)
    *pbIsSet = pUVCCfg->mPrevw.ubSet;
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetPrevwStrmTyp
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set UVC preview stream type.
 @param[in] ubPrevwStrmTyp Stream type for UVC preview video.
 @retval MMP_ERR_NONE Success.
 @retval MMP_USB_ERR_UNSUPPORT_MODE Unsupport resolution.
*/
MMP_ERR MMPD_UVCRECD_SetPrevwStrmTyp(MMP_UBYTE ubPrevwStrmTyp)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetUVCPrevwStrmTyp(ubPrevwStrmTyp);
    #endif

    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_IsDecMjpeg2Prevw
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get status if UVC decode Mjpeg to preview.
 @param[in] pbIsDecMjpeg2Prevw Fetch the status which is decode Mjpeg to preview or not.
 @return It reports the status of the operation.
*/
MMP_ERR MMPD_UVCRECD_IsDecMjpeg2Prevw(MMP_BOOL *pbIsDecMjpeg2Prevw)
{
    MMP_ERR err = MMP_ERR_NONE;

    *pbIsDecMjpeg2Prevw = MMP_FALSE;
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_IsUVCDecMjpeg2Prevw(pbIsDecMjpeg2Prevw);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetPrevwFrameRate
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set frame rate for UVC YUV preview video.
 @param[in] ubFps Frame rate for UVC YUV preview video.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_UVCRECD_SetPrevwFrameRate(MMP_UBYTE ubFps)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetUVCPrevwFrameRate(ubFps);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_AddDevCFG
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
MMP_ERR MMPD_UVCRECD_AddDevCFG(MMP_UBYTE *pubStr, void *pOpenDevCallback, void *pStartDevCallback, void *pNaluInfo)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_AddDevCFG(pubStr, pOpenDevCallback, pStartDevCallback, pNaluInfo);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_UpdDevCFG
//  Description :
//------------------------------------------------------------------------------
/**
@brief The function sets to update UVC device following configs.
@param[in] Event Operation event as update open CV, start CB, and nalu table.
@param[in] pubStr Device Chip info strings.
@param[in] pParm The parameters to be updated info.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_UVCRECD_UpdDevCFG(MMP_USBH_UPD_UVC_CFG_OP Event, MMP_UBYTE *pubStr, void *pParm)
{
#if (SUPPORT_USB_HOST_FUNC)
    return MMPF_USBH_UpdDevCFG(Event, pubStr, pParm);
#else
    return MMP_USB_ERR_UNSUPPORT_MODE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetUVCDevTotalCount
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_UVCRECD_SetUVCDevTotalCount(MMP_UBYTE ubCount)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetUVCDevTotalCount(ubCount);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_RegDevAddiCFG
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
MMP_ERR MMPD_UVCRECD_RegDevAddiCFG(MMP_ULONG ulRegTyp, MMP_UBYTE *pubStr, MMP_ULONG ulParm0, MMP_ULONG ulParm1, MMP_ULONG ulParm2, MMP_ULONG ulParm3)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_RegDevAddiCFG(ulRegTyp, pubStr, ulParm0,ulParm1,ulParm2,ulParm3);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetClassIfCmd
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
MMP_ERR MMPD_UVCRECD_SetClassIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_UBYTE *UVCDataBuf)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetClassIfCmd(bReq, wVal, wInd, wLen, UVCDataBuf);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_GetClassIfCmd
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
MMP_ERR MMPD_UVCRECD_GetClassIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_ULONG *UVCDataLength, MMP_UBYTE *UVCDataBuf)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_GetClassIfCmd(bReq, wVal, wInd, wLen, UVCDataLength, UVCDataBuf);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetStdIfCmd
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
MMP_ERR MMPD_UVCRECD_SetStdIfCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_UBYTE *UVCDataBuf)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_SetStdIfCmd(bReq, wVal, wInd, wLen, UVCDataBuf);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_GetStdDevCmd
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
MMP_ERR MMPD_UVCRECD_GetStdDevCmd(MMP_UBYTE bReq, MMP_USHORT wVal, MMP_USHORT wInd, MMP_USHORT wLen, MMP_ULONG *UVCDataLength, MMP_UBYTE *UVCDataBuf)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    #if (SUPPORT_USB_HOST_FUNC)
    err = MMPF_USBH_GetStdDevCmd(bReq, wVal, wInd, wLen, UVCDataLength, UVCDataBuf);
    #endif
    
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetRepackMiscBuf
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set start address and size of firmware AV repack buffer.

 This buffer is for packing video and audio bitstream sequentially.
 @param[in] repackbuf information of AV repack buffer, such as start address/size,
 sync buffer address, buffer address/size for frame size, and buffer address/size for frame time.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_UVCRECD_SetRepackMiscBuf(MMP_ULONG ulStreamType, MMPD_3GPMGR_REPACKBUF *pRepackBuf)
{
    MMP_ERR	status;

    // AV Repack Buffer
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, pRepackBuf->ulAvRepackStartAddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, pRepackBuf->ulAvRepackSize);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 8, ulStreamType);
    
    status = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AV_REPACK_BUF);
    if (status != MMP_ERR_NONE) {
    	MMPH_HIF_ReleaseSem(GRP_IDX_VID);
        return MMP_SYSTEM_ERR_CMDTIMEOUT;
    }

    // Aux Frame Table
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, pRepackBuf->ulVideoSizeTableAddr);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, pRepackBuf->ulVideoSizeTableSize);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 6, MMPD_3GPMGR_AUX_FRAME_TABLE);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 8, ulStreamType);
    status = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AUX_TABLE_ADDR);

    // Aux Time Table
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, pRepackBuf->ulVideoTimeTableAddr);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, pRepackBuf->ulVideoTimeTableSize);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 6, MMPD_3GPMGR_AUX_TIME_TABLE);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 8, ulStreamType);
    status = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AUX_TABLE_ADDR);

    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetEncodeFormat
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set video encode format to container information.

 @param[in] usStreamType  - video file type
 @param[in] usFormat - video encode format
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_SetEncodeFormat(MMP_USHORT usStreamType, MMP_USHORT usFormat)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usStreamType);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, usFormat);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_UVCRECD_FORMAT);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetEncodeResolution
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set video encode resolution to container information.

 @param[in] usStreamType  - video file type
 @param[in] ResolW - video encode width
 @param[in] ResolH - video encode height
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_SetEncodeResolution(MMP_USHORT usStreamType, MMP_USHORT ResolW, MMP_USHORT ResolH)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usStreamType);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ResolW);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, ResolH);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_UVCRECD_RESOLUTION);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetEncodeGOP
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set video encode GOP size to container information.

 @param[in] usStreamType  - video file type
 @param[in] ubPFrame Count of P frame.
 @param[in] ubBFrame Count of B frame.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_SetEncodeGOP(MMP_USHORT usStreamType, MMP_USHORT usPFrame, MMP_USHORT usBFrame)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);	
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usStreamType);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usPFrame);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 6, usBFrame);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_UVCRECD_GOP);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_UVCRECD_SetEncodeSPSPPSHdr
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set video encode sps,pps headerto container information.

 @param[in] usStreamType  - video file type
 @param[in] ulSPSAddr - sps buffer address
 @param[in] ulSPSSize - sps buffer size
 @param[in] ulPPSAddr - pps buffer address
 @param[in] ulPPSSize - pps buffer size
 @param[in] ulProfileIDC - h264 profile idc
 @param[in] ulLevelIDC - h264 level idc
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_UVCRECD_SetEncodeSPSPPSHdr(MMP_USHORT usStreamType, MMP_ULONG ulSPSAddr, MMP_USHORT ulSPSSize, MMP_ULONG ulPPSAddr, 
                                          MMP_USHORT ulPPSSize, MMP_USHORT ulProfileIDC, MMP_USHORT ulLevelIDC)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usStreamType);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulSPSAddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, ulPPSAddr);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 12, ulSPSSize);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 14, ulPPSSize);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 16, ulProfileIDC);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 18, ulLevelIDC);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_UVCRECD_SPSPPSHDR);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

/// @}

/// @end_ait_only
