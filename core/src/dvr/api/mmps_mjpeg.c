//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "lib_retina.h"
#include "mmp_lib.h"
#include "ait_utility.h"
#include "snr_cfg.h"
#include "usb_cfg.h"
#include "mmps_system.h"
#include "mmps_3gprecd.h"
#include "mmps_dsc.h"
#include "mmps_sensor.h"
#include "mmpd_fctl.h"
#include "mmpd_scaler.h"
#include "mmpf_sensor.h"
#include "mmpf_ldc.h"
#include "mmpf_ringbuf.h"
#include "mmpf_jpeg_ctl.h"
#if (SUPPORT_UVC_FUNC)    
#include "mmpf_usb_h264.h"
#endif

#if 1//(SUPPORT_MJPEG_WIFI_STREAM)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

static MMP_ULONG            m_ulMjpegCaptDramAddr = 0; 
static MMP_ULONG            m_ulMjpegCaptSramAddr = 0;

static MMP_MJPEG_OBJ        m_sMjpegStreamObj[MAX_MJPEG_STREAM_NUM];
static MMP_MJPEG_LINKATTR   m_sMjpegStreamLinkAttr[MAX_MJPEG_STREAM_NUM];

//==============================================================================
//
//                              EXTERN VARIABLES
//
//==============================================================================

extern MMP_ULONG        m_ulPrimaryJpegCompStart;
extern MMP_ULONG        m_ulPrimaryJpegCompEnd;
extern MMP_ULONG        m_ulPrimaryJpegLineStart;

#if (HANDLE_JPEG_EVENT_BY_QUEUE)
extern MMPD_FCTL_ATTR   m_VRPreviewFctlAttr[];
extern MMPD_FCTL_ATTR   m_DSCPreviewFctlAttr;
#endif
extern MMPD_FCTL_ATTR   m_DecMjpegToPrevwFctlAttr;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void _____MJPEG_Streaming_Function_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_MJPEG_IsValidObj
//  Description :
//------------------------------------------------------------------------------
static MMP_BOOL MMPS_MJPEG_IsValidObj(MMP_MJPEG_OBJ_PTR pHandle)
{
    if ((pHandle == NULL) || (pHandle->ul4cc != CODEC_MJPG)) {
        return MMP_FALSE;
    }
    return MMP_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_MJPEG_SetCaptureAddr
//  Description : The function saves head memory address to serve Still Capture.
//                Becasue that live streaming of MJPEG is always allocate memory as
//                Video record mode, but if UI is at Capture Mode (Capture preview)
//                and do capture, that memory allocated by Capture mode from FW ending,
//                it will overlay with streaming even streaming is at PAUSE state.
//                The function saves the head address for Capture in streaming.
//                To get the address to allocate buffer to capture in streaming.
//------------------------------------------------------------------------------
MMP_ERR MMPS_MJPEG_SetCaptureAddr(MMP_ULONG ulDramAddr, MMP_ULONG ulSramAddr)
{
    m_ulMjpegCaptDramAddr = ulDramAddr;
    m_ulMjpegCaptSramAddr = ulSramAddr;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_MJPEG_GetCaptureAddr
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_MJPEG_GetCaptureAddr(MMP_ULONG *pulDram, MMP_ULONG *pulSram)
{
    *pulDram = m_ulMjpegCaptDramAddr;
    *pulSram = m_ulMjpegCaptSramAddr;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_MJPEG_OpenStream
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_MJPEG_OpenStream(MMP_USHORT usEncID, MMP_USHORT usMode, MMP_MJPEG_OBJ_PTR *ppHandle)
{
    MMP_IBC_PIPEID  ubPipe = MMP_IBC_PIPE_2; // Becareful about this.
    MMP_ERR         sRet = MMP_ERR_NONE;
    
    if ((usEncID != MMP_MJPEG_STREAM_FRONTCAM_VIDEO) &&
        (usEncID != MMP_MJPEG_STREAM_REARCAM_VIDEO)) {
        MMP_PRINT_RET_ERROR(0, usEncID, "", 0);
        return MMP_MJPGD_ERR_PARAMETER;
    }

    if (usMode >= MMP_MJPEG_UI_MODE_ID_NUM) {
        MMP_PRINT_RET_ERROR(0, usEncID, "", 0);
        return MMP_MJPGD_ERR_PARAMETER;
    }
    
    *ppHandle = NULL;
    
    /* Get the MJPEG encode pipe */
    if (usMode == MMP_MJPEG_UI_MODE_WIFI_VR) {
        if (usEncID == MMP_MJPEG_STREAM_FRONTCAM_VIDEO){
            MMPS_3GPRECD_GetPreviewPipe(0/*PRM_SENSOR*/, &ubPipe);
        }
        else if (usEncID == MMP_MJPEG_STREAM_REARCAM_VIDEO) {
            if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264) ||\
                CAM_CHECK_USB(USB_CAM_SONIX_MJPEG)) {
                ubPipe = MMPS_3GPRECD_GetDecMjpegToPreviewPipeId();
            }
            else if (CAM_CHECK_USB(USB_CAM_AIT)) {
                ubPipe = MMPS_3GPRECD_GetDecMjpegToPreviewPipeId();
            }
        }
        else {
            MMP_PRINT_RET_ERROR(0, usEncID, "", 0);
            return MMP_MJPGD_ERR_PARAMETER;
        }
    }
    else if (usMode == MMP_MJPEG_UI_MODE_WIFI_DSC) {
        if (usEncID == MMP_MJPEG_STREAM_FRONTCAM_VIDEO) {
            MMPS_DSC_GetPreviewPipe(&ubPipe);
        }
        else {
            MMP_PRINT_RET_ERROR(0, usEncID, "", 0);
            return MMP_MJPGD_ERR_INVALID_STATE;
        }
    }
    else if (usMode == MMP_MJPEG_UI_MODE_UVC_VR) {
        if (usEncID == MMP_MJPEG_STREAM_FRONTCAM_VIDEO){
            MMPS_3GPRECD_GetPreviewPipe(0/*PRM_SENSOR*/, &ubPipe);
        }
        else{
            MMP_PRINT_RET_ERROR(0, usEncID, "", 0);
            return MMP_MJPGD_ERR_PARAMETER;
        }
    }
    else {
        return MMP_ERR_NONE;
    }
    
    sRet = MMPD_MJPEG_NewOneJPEG(usEncID);
    
    *ppHandle = &m_sMjpegStreamObj[usEncID];
    
    m_sMjpegStreamObj[usEncID].usEncID      = usEncID;
    m_sMjpegStreamObj[usEncID].sMJPGModeID  = usMode;
    m_sMjpegStreamObj[usEncID].ul4cc        = CODEC_MJPG;
    m_sMjpegStreamObj[usEncID].PipeID       = ubPipe;
    m_sMjpegStreamObj[usEncID].pLinkAttr    = &m_sMjpegStreamLinkAttr[usEncID];

    sRet = MMPD_MJPEG_SetLinkPipe(usEncID, ubPipe);
    sRet = MMPD_MJPEG_SetModeID(usEncID, usMode);
    if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "", gubMmpDbgBk); return sRet;}

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_MJPEG_StartFrontCamStream
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_MJPEG_StartFrontCamStream( MMP_UBYTE           ubSnrSel,
                                        MMP_UBYTE           ubMode,
                                        MMP_MJPEG_OBJ_PTR   pHandle, 
                                        MMP_MJPEG_ENC_INFO  *pMjpegInfo,
                                        MMP_MJPEG_RATE_CTL  *pRateCtrl)
{
    MMP_ULONG               ulDramAddr, ulDramAddrEnd, ulSramAddr;
    MMP_ULONG               ulInWidth, ulInHeight;
    MMP_SCAL_FIT_RANGE      fitrange;
    MMP_SCAL_GRAB_CTRL      grabctl;
    MMPD_FCTL_ATTR          fctlAttr;
    MMP_PIPE_LINK           fctllink;
    MMP_ERR                 err = MMP_ERR_NONE;
    #if (HANDLE_JPEG_EVENT_BY_QUEUE)
    MMP_ULONG               ulCompBuf, ulCompSize; 
    MMP_ULONG               ulLineBuf, ulLineSize;
    #endif

    if (pHandle == NULL) {
        MMP_PRINT_RET_ERROR(0, 0, "", 0);
        return MMP_MJPGD_ERR_PARAMETER;
    }

    if ((MMP_MJPEG_STREAM_FRONTCAM_VIDEO != pHandle->usEncID) || 
        (pHandle->sMJPGModeID != ubMode)) {
        MMP_PRINT_RET_ERROR(0, ubMode, "", 0);
        return MMP_MJPGD_ERR_PARAMETER;
    }
    
    MMPD_System_GetSramEndAddr(&ulSramAddr);
    
    /* Get Buffer Address */
    if ((ubMode == MMP_MJPEG_UI_MODE_WIFI_VR) || (ubMode == MMP_MJPEG_UI_MODE_UVC_VR)) {
        // Get the reserved buffer address
        #if (HANDLE_JPEG_EVENT_BY_QUEUE)
        if (MMPF_JPEG_GetCtrlByQueueEnable()) {
            MMPS_3GPRECD_GetFrontCamBufForDualStreaming(&ulCompBuf, &ulCompSize, &ulLineBuf, &ulLineSize);
        }
        else {
            MMPS_3GPRECD_GetStillCaptureAddr(&ulSramAddr, &ulDramAddr);
        }
        #endif
    }
    else {
        // Get the preview end address
        MMPS_DSC_SetCaptureBuf(NULL, NULL, MMP_FALSE, MMP_FALSE, MMP_DSC_CAPTURE_NO_ROTATE, MMP_TRUE);
        MMPS_DSC_GetCaptureBuf(&ulSramAddr, &ulDramAddr, &ulDramAddrEnd);
    }

    /* Save the memory address that used when Still Capture called in DSC mode */
    MMPS_MJPEG_SetCaptureAddr(ulDramAddr, ulSramAddr);

    /* Set JPEG Parameters */
    MMPS_DSC_SetShotMode(MMPS_DSC_SINGLE_SHOTMODE);
    MMPS_DSC_SetJpegEncParam(DSC_RESOL_IDX_UNDEF, pMjpegInfo->usEncWidth, pMjpegInfo->usEncHeight, MMP_DSC_JPEG_RC_ID_MJPEG_1ST_STREAM);
    MMPS_DSC_ConfigThumbnail(0, 0, MMP_DSC_THUMB_SRC_NONE);

    #if (HANDLE_JPEG_EVENT_BY_QUEUE)
    if (MMPF_JPEG_GetCtrlByQueueEnable()) {
        MMPS_DSC_SetCaptureJpegQualityEx(MMP_DSC_JPEG_RC_ID_MJPEG_1ST_STREAM,
                                         pMjpegInfo->bTargetCtl, pMjpegInfo->bLimitCtl, pMjpegInfo->bTargetSize,
                                         pMjpegInfo->bLimitSize, pMjpegInfo->bMaxTrialCnt, pMjpegInfo->Quality);
    }
    else {
        MMPS_DSC_SetCaptureJpegQuality( MMP_DSC_JPEG_RC_ID_MJPEG_1ST_STREAM,
                                        pMjpegInfo->bTargetCtl, pMjpegInfo->bLimitCtl, pMjpegInfo->bTargetSize,
                                        pMjpegInfo->bLimitSize, pMjpegInfo->bMaxTrialCnt, pMjpegInfo->Quality);	
    }
    #endif

    if (ubMode == MMP_MJPEG_UI_MODE_UVC_VR) {
        //NOP.
    }
    else {
        MMPS_DSC_SetCaptureBuf(&ulSramAddr, &ulDramAddr, MMP_TRUE, MMP_FALSE ,MMP_DSC_CAPTURE_NO_ROTATE, MMP_TRUE);
    }

    /* Set Pipeline Parameters */
    MMPS_Sensor_GetCurPrevScalInputRes(ubSnrSel, &ulInWidth, &ulInHeight);

    fitrange.fitmode        = MMP_SCAL_FITMODE_OUT;
    fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;
    
    if (ubMode == MMP_MJPEG_UI_MODE_WIFI_VR) {
        if (MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE 	||
            MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI 	||
            MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
            
            MMPS_3GPRECD_GetLdcMaxOutRes(&fitrange.ulInWidth, &fitrange.ulInHeight);
        }
        else {
            fitrange.ulInWidth  = ulInWidth;
            fitrange.ulInHeight = ulInHeight;
        }
    }
    else {
        fitrange.ulInWidth  = ulInWidth;
        fitrange.ulInHeight = ulInHeight;
    }
    
    fitrange.ulOutWidth	    = pMjpegInfo->usEncWidth;
    fitrange.ulOutHeight    = pMjpegInfo->usEncHeight;

    fitrange.ulInGrabX      = 1;
    fitrange.ulInGrabY      = 1;
    fitrange.ulInGrabW      = fitrange.ulInWidth;
    fitrange.ulInGrabH      = fitrange.ulInHeight;
    fitrange.ubChoseLit     = 0;

    MMPD_Scaler_GetGCDBestFitScale(&fitrange, &grabctl);

    if (ubMode == MMP_MJPEG_UI_MODE_WIFI_VR) {
        if (MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE) {
            pHandle->pLinkAttr->scalerSrc = MMP_SCAL_SOURCE_LDC;
        }
        else if (MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI 	||
                 MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
            pHandle->pLinkAttr->scalerSrc = MMP_SCAL_SOURCE_GRA;
        }
        else {
            pHandle->pLinkAttr->scalerSrc = MMP_SCAL_SOURCE_ISP;
        }
    }
    else {
        pHandle->pLinkAttr->scalerSrc = MMP_SCAL_SOURCE_ISP;
    }

    FCTL_PIPE_TO_LINK(pHandle->PipeID, fctllink);

    /* Back up the Scaler and IBC setting and close the IBC of preview path */
    MMPD_Scaler_BackupAttributes(pHandle->PipeID);

    MMPD_IBC_ClearFrameEnd(pHandle->PipeID);
    MMPD_IBC_CheckFrameEnd(pHandle->PipeID);
    MMPD_IBC_SetInterruptEnable(pHandle->PipeID, MMP_IBC_EVENT_FRM_END, MMP_FALSE);
    MMPD_IBC_SetStoreEnable(pHandle->PipeID, MMP_FALSE);
    
    // IBC can't sync with ISR, wait 40 msec for IBC store done.
    MMPF_OS_Sleep(40);

    MMPD_Scaler_ResetModule(pHandle->PipeID); 
    MMPD_Icon_ResetModule(pHandle->PipeID);
    MMPD_IBC_ResetModule(pHandle->PipeID);
    
    MMPD_Fctl_ResetIBCBufIdx(pHandle->PipeID);

    MMPD_Fctl_GetIBCLinkAttr(pHandle->PipeID, &(pHandle->pLinkAttr->IBCLinkType), &(pHandle->pLinkAttr->previewDev), &(pHandle->pLinkAttr->winID), &(pHandle->pLinkAttr->rotateDir));
    MMPD_Fctl_ResetIBCLinkType(pHandle->PipeID);

    pHandle->pLinkAttr->IBCPipeAttr.function = MMP_IBC_FX_TOFB;
    MMPD_IBC_GetAttributes(pHandle->PipeID, &(pHandle->pLinkAttr->IBCPipeAttr));

    /* Change the path to LDC/JPEG in order to keep the setting of pipe normal */
    #if (HANDLE_JPEG_EVENT_BY_QUEUE)
    if (MMPF_JPEG_GetCtrlByQueueEnable()) {
        MMPD_Scaler_SetPath(pHandle->PipeID, MMP_SCAL_SOURCE_LDC, MMP_TRUE);

        if ((ubMode == MMP_MJPEG_UI_MODE_WIFI_VR) || (ubMode == MMP_MJPEG_UI_MODE_UVC_VR)) {
            fctlAttr = m_VRPreviewFctlAttr[0];
        }
        else {
            fctlAttr = m_DSCPreviewFctlAttr;
        }
    }
    else {
        MMPD_Scaler_SetPath(pHandle->PipeID, MMP_SCAL_SOURCE_JPG, MMP_TRUE);
    }
    #endif
    
    fctlAttr.bRtModeOut         = MMP_FALSE;
    fctlAttr.fctllink           = fctllink;
    fctlAttr.fitrange           = fitrange;
    fctlAttr.grabctl            = grabctl;
    fctlAttr.scalsrc            = pHandle->pLinkAttr->scalerSrc;
    fctlAttr.sScalDelay         = m_sFullSpeedScalDelay;
    fctlAttr.bSetScalerSrc      = MMP_FALSE;
    fctlAttr.ubPipeLinkedSnr    = ubSnrSel;

    #if (HANDLE_JPEG_EVENT_BY_QUEUE)
    if (MMPF_JPEG_GetCtrlByQueueEnable()) {
        // CHECK : Fix WIFI have problem.
        if ((ubMode == MMP_MJPEG_UI_MODE_WIFI_VR) || (ubMode == MMP_MJPEG_UI_MODE_WIFI_DSC)) {    
            fctlAttr.colormode = MMP_DISPLAY_COLOR_RGB565;
        }
        else{
            fctlAttr.colormode = MMP_DISPLAY_COLOR_YUV420;
        }
        
        if (fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB565 || 
            fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB888) {
            fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_RGB;
        }
        else {
            #if (CCIR656_FORCE_SEL_BT601)
            fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
            #else
            fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_YUV_FULLRANGE;
            #endif
        }
		
		if(ubMode == MMP_MJPEG_UI_MODE_WIFI_DSC){
			MMP_ULONG ulTmpBufSize,i;
			// re-allocate YVU buffers
			fctlAttr.usBufCnt = 2;
			for (i = 0; i < fctlAttr.usBufCnt; i++)
		    {
		        switch (fctlAttr.colormode) {
		        case MMP_DISPLAY_COLOR_RGB565:
		        case MMP_DISPLAY_COLOR_YUV422:
		            ulTmpBufSize = ALIGN32(fitrange.ulInWidth * 2 * fitrange.ulInHeight);
		            
		            fctlAttr.ulBaseAddr[i] = ulDramAddr;
		            fctlAttr.ulBaseUAddr[i] = 0;
		            fctlAttr.ulBaseVAddr[i] = 0;
		            ulDramAddr += ulTmpBufSize;
		            break;
		        case MMP_DISPLAY_COLOR_RGB888:
		            ulTmpBufSize = ALIGN32(fitrange.ulInWidth * 3 * fitrange.ulInHeight);
		            
		            fctlAttr.ulBaseAddr[i] = ulDramAddr;
		            fctlAttr.ulBaseUAddr[i] = 0;
		            fctlAttr.ulBaseVAddr[i] = 0;
		            ulDramAddr += ulTmpBufSize;
		            break;
		        case MMP_DISPLAY_COLOR_YUV420:
		            ulTmpBufSize = fitrange.ulInWidth * fitrange.ulInHeight;
		            
		            fctlAttr.ulBaseAddr[i] = ulDramAddr;
		            fctlAttr.ulBaseUAddr[i] = fctlAttr.ulBaseAddr[i] + ALIGN32(ulTmpBufSize);
		            fctlAttr.ulBaseVAddr[i] = fctlAttr.ulBaseUAddr[i] + ALIGN32(ulTmpBufSize >> 2);
		            ulDramAddr += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>2)*2);
		            break;
		        case MMP_DISPLAY_COLOR_YUV420_INTERLEAVE:
		            ulTmpBufSize = fitrange.ulInWidth * fitrange.ulInHeight;
		        
		            fctlAttr.ulBaseAddr[i] = ulDramAddr;
		            fctlAttr.ulBaseUAddr[i] = fctlAttr.ulBaseAddr[i] + ALIGN32(ulTmpBufSize);
		            fctlAttr.ulBaseVAddr[i] = fctlAttr.ulBaseUAddr[i];
		            ulDramAddr += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>1));
		            break;
		        }
		    }
		}
    
        MMPD_Fctl_SetPipeAttrForIbcFB(&fctlAttr);

        MMPD_Fctl_LinkPipeToGra2JPEG(   pHandle->PipeID,
                                        pHandle->pLinkAttr->winID,
                                        pHandle->pLinkAttr->previewDev,
                                 		MMPD_FCTL_GRA2JPEG_MJEPG,
                                        MMP_FALSE);

        MMPD_MJPEG_SetDualStreamingEnable(MMP_TRUE, MMP_FALSE);
    }
    else {
        MMPD_Fctl_SetPipeAttrForJpeg(&fctlAttr, MMP_TRUE, MMP_FALSE);

        MMPD_DSC_SetCapturePath(pHandle->PipeID, pHandle->PipeID, pHandle->PipeID);
    }
    #endif
    
    if (pRateCtrl != NULL) {
        MMPD_MJPEG_SetFPS(MMP_MJPEG_STREAM_FRONTCAM_VIDEO, pRateCtrl->FPSx10, 10);
    } 
    else {
        MMPD_MJPEG_SetFPS(MMP_MJPEG_STREAM_FRONTCAM_VIDEO, 300, 10);
    }

    /* Change the path to the frame source */
    MMPD_IBC_ClearFrameEnd(pHandle->PipeID);
    MMPD_Scaler_SetPath(pHandle->PipeID, pHandle->pLinkAttr->scalerSrc, MMP_TRUE);
    MMPD_IBC_CheckFrameEnd(pHandle->PipeID);
    
    /* Assign streaming buffer and Enable Streaming */
    if ((ubMode == MMP_MJPEG_UI_MODE_WIFI_VR) || 
        (ubMode == MMP_MJPEG_UI_MODE_WIFI_DSC)) {
        #if (HANDLE_JPEG_EVENT_BY_QUEUE)
        if (MMPF_JPEG_GetCtrlByQueueEnable() && ubMode == MMP_MJPEG_UI_MODE_WIFI_VR) {
            err = MMPD_Streaming_SetCompBuf(VIDEO1_RING_STREAM, ubMode, ulCompBuf, ulCompSize, MMP_STREAM_JPEG);
            err = MMPD_Streaming_SetLineBuf(VIDEO1_RING_STREAM, ulLineBuf, ulLineSize);
        }
        else {
            err = MMPD_Streaming_SetCompBuf(VIDEO1_RING_STREAM, ubMode, m_ulPrimaryJpegCompStart,
                                            m_ulPrimaryJpegCompEnd - m_ulPrimaryJpegCompStart, MMP_STREAM_JPEG);
        }
        #endif

        MMPD_Streaming_Enable(VIDEO1_RING_STREAM, MMP_TRUE, MMP_FALSE);
    }
    #if (SUPPORT_UVC_FUNC)
    else if ((ubMode == MMP_MJPEG_UI_MODE_UVC_VR) || 
             (ubMode == MMP_MJPEG_UI_MODE_UVC_DSC)) {
        #if (HANDLE_JPEG_EVENT_BY_QUEUE)    
        if (MMPF_JPEG_GetCtrlByQueueEnable()) {
        
            extern MMP_ULONG   gulPcamJPEGCompBufAddr;
            extern MMP_ULONG   gulPcamJPEGCompBufSize;
            
            err = MMPD_MJPEG_SetCompBuf(VIDEO1_RING_STREAM, MMP_MJPEG_UI_MODE_UVC_VR, gulPcamJPEGCompBufAddr + FRAME_PAYLOAD_HEADER_SZ, 
                                        gulPcamJPEGCompBufSize - 1, MMP_STREAM_JPEG);
            err = MMPD_Streaming_SetLineBuf(VIDEO1_RING_STREAM, m_ulPrimaryJpegLineStart, 0xFFFF);
        }
        #endif
    }
    #endif

    MMPD_DSC_SetMJPEGPipe(pHandle->PipeID);
    MMPD_DSC_StartMJPEGStream(PRM_SENSOR);

    /* Enable IBC store for trigger loop-back JPEG encode */
    #if (HANDLE_JPEG_EVENT_BY_QUEUE)
    if (MMPF_JPEG_GetCtrlByQueueEnable()) {
        MMPD_IBC_SetStoreEnable(pHandle->PipeID, MMP_TRUE);
    }
    #endif

    if (err != MMP_ERR_NONE) {
        return MMP_WIFI_ERR_PARAMETER;
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_MJPEG_StartRearCamStream
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_MJPEG_StartRearCamStream(  MMP_UBYTE           ubSnrSel,
                                        MMP_UBYTE           ubMode,
                                        MMP_MJPEG_OBJ_PTR   pHandle, 
                                        MMP_MJPEG_ENC_INFO  *pMjpegInfo,
                                        MMP_MJPEG_RATE_CTL  *pRateCtrl)
{
#if (HANDLE_JPEG_EVENT_BY_QUEUE)
    MMP_ULONG               ulCompAddr, ulCompSize, ulLineBuf, ulLineSize;
    MMP_SCAL_FIT_RANGE      fitrange;
    MMP_SCAL_GRAB_CTRL      grabctl;
    MMPD_FCTL_ATTR          fctlAttr;
    MMP_PIPE_LINK           fctllink;
    MMP_ERR                 err= MMP_ERR_NONE;

    if (pHandle == NULL) {
        MMP_PRINT_RET_ERROR(0, 0, "", 0);
        return MMP_WIFI_ERR_PARAMETER;
    }

    if (!MMPF_JPEG_GetCtrlByQueueEnable()) {
        return MMP_ERR_NONE;
    }

    if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264) ||\
        CAM_CHECK_USB(USB_CAM_SONIX_MJPEG)) {

        /* Get the Reserved Buffer Information */
        MMPD_DSC_GetRearCamBufForDualStreaming(&ulCompAddr, &ulCompSize, &ulLineBuf, &ulLineSize);

        /* Set JPEG Parameters */
        MMPS_DSC_SetJpegEncParam(DSC_RESOL_IDX_UNDEF, pMjpegInfo->usEncWidth, pMjpegInfo->usEncHeight, MMP_DSC_JPEG_RC_ID_MJPEG_2ND_STREAM);
        MMPS_DSC_ConfigThumbnail(0, 0, MMP_DSC_THUMB_SRC_NONE);

        MMPS_DSC_SetCaptureJpegQualityEx(MMP_DSC_JPEG_RC_ID_MJPEG_2ND_STREAM,
                                         pMjpegInfo->bTargetCtl, pMjpegInfo->bLimitCtl, pMjpegInfo->bTargetSize,
                                         pMjpegInfo->bLimitSize, pMjpegInfo->bMaxTrialCnt, pMjpegInfo->Quality);

        /* Set Pipeline Parameters */
        MMPS_3GPRECD_GetDecMjpegToPreviewSrcAttr((MMP_USHORT*)&fitrange.ulInWidth, (MMP_USHORT*)&fitrange.ulInHeight);

        fitrange.fitmode        = MMP_SCAL_FITMODE_OUT;
        fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;
        fitrange.ulOutWidth	    = pMjpegInfo->usEncWidth;
        fitrange.ulOutHeight    = pMjpegInfo->usEncHeight;

        fitrange.ulInGrabX      = 1;
        fitrange.ulInGrabY      = 1;
        fitrange.ulInGrabW      = fitrange.ulInWidth;
        fitrange.ulInGrabH      = fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;

        MMPD_Scaler_GetGCDBestFitScale(&fitrange, &grabctl);

        pHandle->pLinkAttr->scalerSrc = MMP_SCAL_SOURCE_JPG;

        FCTL_PIPE_TO_LINK(pHandle->PipeID, fctllink);

        /* Back up the Scaler and IBC setting and close the IBC of preview path */
        MMPD_Scaler_BackupAttributes(pHandle->PipeID);

        MMPD_IBC_ClearFrameEnd(pHandle->PipeID);
        MMPD_IBC_CheckFrameEnd(pHandle->PipeID);
        MMPD_IBC_SetInterruptEnable(pHandle->PipeID, MMP_IBC_EVENT_FRM_END, MMP_FALSE);
        MMPD_IBC_SetStoreEnable(pHandle->PipeID, MMP_FALSE);
        
        // IBC can't sync with ISR, wait 40 msec for IBC store done.
        MMPF_OS_Sleep(40);
        
        MMPD_Fctl_ResetIBCBufIdx(pHandle->PipeID);
        
        MMPD_Fctl_GetIBCLinkAttr(pHandle->PipeID, &(pHandle->pLinkAttr->IBCLinkType), &(pHandle->pLinkAttr->previewDev), &(pHandle->pLinkAttr->winID), &(pHandle->pLinkAttr->rotateDir));
        MMPD_Fctl_ResetIBCLinkType(pHandle->PipeID);

        pHandle->pLinkAttr->IBCPipeAttr.function = MMP_IBC_FX_TOFB;
        MMPD_IBC_GetAttributes(pHandle->PipeID, &(pHandle->pLinkAttr->IBCPipeAttr));

        /* Change the path to LDC in order to keep the setting of pipe normal. */
        MMPD_Scaler_SetPath(pHandle->PipeID, MMP_SCAL_SOURCE_LDC, MMP_TRUE);
        
        fctlAttr = m_DecMjpegToPrevwFctlAttr;

        fctlAttr.bRtModeOut         = MMP_FALSE;
        fctlAttr.fctllink           = fctllink;
        fctlAttr.fitrange           = fitrange;
        fctlAttr.grabctl            = grabctl;
        fctlAttr.scalsrc            = pHandle->pLinkAttr->scalerSrc;
        fctlAttr.sScalDelay         = m_sFullSpeedScalDelay;
        fctlAttr.bSetScalerSrc      = MMP_FALSE;
        fctlAttr.ubPipeLinkedSnr    = ubSnrSel;
       
        MMPD_Fctl_SetPipeAttrForIbcFB(&fctlAttr);

        MMPD_Fctl_LinkPipeToGra2JPEG(pHandle->PipeID,
                                     pHandle->pLinkAttr->winID,
                                     pHandle->pLinkAttr->previewDev,
                                 	 MMPD_FCTL_GRA2JPEG_MJEPG,
                                     MMP_FALSE); 
        
        MMPD_MJPEG_SetDualStreamingEnable(MMP_TRUE, MMP_TRUE);
        
        if (pRateCtrl != NULL) {
            MMPD_MJPEG_SetFPS(MMP_MJPEG_STREAM_REARCAM_VIDEO, pRateCtrl->FPSx10, 10);
        }
        else {
            MMPD_MJPEG_SetFPS(MMP_MJPEG_STREAM_REARCAM_VIDEO, 300, 10);
        }

        /* Enable IBC store for trigger loop-back JPEG encode */
        MMPD_IBC_SetStoreEnable(pHandle->PipeID, MMP_TRUE);

        /* Assign streaming buffer */
        MMPD_Streaming_SetCompBuf(VIDEO2_RING_STREAM, MMP_MJPEG_UI_MODE_WIFI_VR, ulCompAddr, ulCompSize, MMP_STREAM_JPEG2);
        MMPD_Streaming_SetLineBuf(VIDEO2_RING_STREAM, ulLineBuf, ulLineSize);

        /* Enable Streaming */
        MMPD_Streaming_Enable(VIDEO2_RING_STREAM, MMP_TRUE, MMP_FALSE);
    }
    else if (CAM_CHECK_USB(USB_CAM_AIT)) {
    
        /* AIT Cam output MJPEG at streaming mode,
         * But it needs transcoding to YUV at preview mode 
         */ 

        /* Back up the Scaler and IBC setting and close the IBC of preview path */
        MMPD_Scaler_BackupAttributes(pHandle->PipeID);

        MMPD_IBC_ClearFrameEnd(pHandle->PipeID);
        MMPD_IBC_CheckFrameEnd(pHandle->PipeID);
        MMPD_IBC_SetInterruptEnable(pHandle->PipeID, MMP_IBC_EVENT_FRM_END, MMP_FALSE);
        MMPD_IBC_SetStoreEnable(pHandle->PipeID, MMP_FALSE);

        MMPD_Fctl_GetIBCLinkAttr(pHandle->PipeID, &(pHandle->pLinkAttr->IBCLinkType), &(pHandle->pLinkAttr->previewDev), &(pHandle->pLinkAttr->winID), &(pHandle->pLinkAttr->rotateDir));
        MMPD_Fctl_ResetIBCLinkType(pHandle->PipeID);

        pHandle->pLinkAttr->IBCPipeAttr.function = MMP_IBC_FX_TOFB;
        MMPD_IBC_GetAttributes(pHandle->PipeID, &(pHandle->pLinkAttr->IBCPipeAttr));

        /* Get the Reserved Buffer Information */
        MMPD_DSC_GetRearCamBufForDualStreaming(&ulCompAddr, &ulCompSize, &ulLineBuf, &ulLineSize);

        /* Assign streaming buffer */
        MMPD_Streaming_SetCompBuf(VIDEO2_RING_STREAM, MMP_MJPEG_UI_MODE_WIFI_VR, ulCompAddr, ulCompSize, MMP_STREAM_JPEG2);
        MMPD_Streaming_SetLineBuf(VIDEO2_RING_STREAM, ulLineBuf, ulLineSize);
        
        /* Enable Streaming */
        MMPD_Streaming_Enable(VIDEO2_RING_STREAM, MMP_TRUE, MMP_FALSE);
    }

    if (err != MMP_ERR_NONE) {
        return MMP_WIFI_ERR_PARAMETER;
    }
    
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_MJPEG_StopStream
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_MJPEG_StopStream(MMP_MJPEG_OBJ_PTR pHandle)
{
    MMP_BOOL    bStreamEnable = MMP_FALSE;
    MMP_ERR     sRet = MMP_ERR_NONE;

    if (MMP_FALSE == MMPS_MJPEG_IsValidObj(pHandle)) {
        return MMP_MJPGD_ERR_PARAMETER;
    }

    if ((MMP_MJPEG_UI_MODE_WIFI_VR == pHandle->sMJPGModeID) || 
        (MMP_MJPEG_UI_MODE_WIFI_DSC == pHandle->sMJPGModeID)) {
        
        if (pHandle->usEncID == MMP_MJPEG_STREAM_FRONTCAM_VIDEO) {
            bStreamEnable = MMPF_StreamRing_IsEnabled(VIDEO1_RING_STREAM);
        }
        else if (pHandle->usEncID == MMP_MJPEG_STREAM_REARCAM_VIDEO) {
            bStreamEnable = MMPF_StreamRing_IsEnabled(VIDEO2_RING_STREAM);  
        }

        if (!bStreamEnable) { MMP_PRINT_RET_ERROR(0, 0, "MJPEG has stopped already!", 0); return MMP_MJPGD_ERR_INVALID_STATE;}
        
        sRet = MMPD_Streaming_Enable(pHandle->usEncID, MMP_FALSE, MMP_FALSE);
            
        sRet = MMPD_MJPEG_CheckEncode(pHandle->usEncID);

        if (pHandle->usEncID == MMP_MJPEG_STREAM_FRONTCAM_VIDEO) {
            MMPD_JPEG_Ctl_PausePreview();
            sRet = MMPD_DSC_StopMJPEGStream(pHandle->usEncID);
        }
        
        MMPS_MJPEG_Return2Display(pHandle);
        MMPD_JPEG_Ctl_ResumePreview();
        
        if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "", gubMmpDbgBk); return sRet;}        
    }
    #if (SUPPORT_UVC_FUNC)
    else if ((MMP_MJPEG_UI_MODE_UVC_VR == pHandle->sMJPGModeID) || 
             (MMP_MJPEG_UI_MODE_UVC_DSC == pHandle->sMJPGModeID)) {
             
        sRet = MMPD_MJPEG_CheckEncode(pHandle->usEncID);
        sRet = MMPD_DSC_StopMJPEGStream(pHandle->usEncID);
        if (sRet != MMP_ERR_NONE){MMP_PRINT_RET_ERROR(0, sRet, "", gubMmpDbgBk); return sRet;}        
    }
    #endif    
    else {
        return MMP_MJPGD_ERR_PARAMETER;
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_MJPEG_CloseStream
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_MJPEG_CloseStream(MMP_MJPEG_OBJ_PTR* ppHandle)
{
    MMP_ERR err = MMP_ERR_NONE;
    
    //err = MMPS_MJPEG_StopStream(*ppHandle);
    
    if (err == MMP_ERR_NONE) {
        (*ppHandle)->usEncID        = MAX_MJPEG_STREAM_NUM;
        (*ppHandle)->sMJPGModeID    = MMP_MJPEG_UI_MODE_ID_NUM;
        (*ppHandle)->PipeID         = MMP_IBC_PIPE_MAX;
        (*ppHandle)->pLinkAttr      = NULL;
        (*ppHandle)                 = NULL;
    }
    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_MJPEG_Return2Display
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_MJPEG_Return2Display(MMP_MJPEG_OBJ_PTR pHandle)
{
    MMP_BOOL bDSCOn = MMP_FALSE;

    if (!MMPS_MJPEG_IsValidObj(pHandle)) {
        return MMP_WIFI_ERR_PARAMETER;
    }
    
    /* Disable scaler because it might still previewing or recording */
    MMPD_Scaler_SetEnable(pHandle->PipeID, MMP_FALSE);

    /* Change the path to JPEG in order to keep the setting of scaler normal */
    #if (HANDLE_JPEG_EVENT_BY_QUEUE)
    if (MMPF_JPEG_GetCtrlByQueueEnable()) {
        MMPD_Scaler_SetPath(pHandle->PipeID, MMP_SCAL_SOURCE_LDC, MMP_TRUE);
    }
    else {
        MMPD_Scaler_SetPath(pHandle->PipeID, MMP_SCAL_SOURCE_JPG, MMP_TRUE);
    }
    #endif

    MMPS_DSC_GetPreviewStatus(&bDSCOn);

    /* Restore the setting of Scaler/IBC */
    if (bDSCOn) {

        #if (HANDLE_JPEG_EVENT_BY_QUEUE)
        if (MMPF_JPEG_GetCtrlByQueueEnable()) {
            // Fix WIFI preview -> LCD preview have white line issue.   
            MMPD_IBC_SetStoreEnable(pHandle->PipeID, MMP_FALSE);
            
            // IBC can't sync with ISR, wait 40 msec for IBC store done.
            MMPF_OS_Sleep(40);
            
            MMPD_Fctl_ResetIBCBufIdx(pHandle->PipeID);
            MMPD_Fctl_SetPipeAttrForIbcFB(&m_DSCPreviewFctlAttr);
        }
        else {
            MMPD_Scaler_RestoreAttributes(pHandle->PipeID);
            MMPD_IBC_SetAttributes(pHandle->PipeID, &pHandle->pLinkAttr->IBCPipeAttr);	
        }
        #endif
    }
    else if (pHandle->usEncID == MMP_MJPEG_STREAM_FRONTCAM_VIDEO) {

        #if (HANDLE_JPEG_EVENT_BY_QUEUE)
        if (MMPF_JPEG_GetCtrlByQueueEnable()) {
            // Fix WIFI preview -> LCD preview have white line issue.   
            MMPD_IBC_SetStoreEnable(pHandle->PipeID, MMP_FALSE);
            
            // IBC can't sync with ISR, wait 40 msec for IBC store done.
            MMPF_OS_Sleep(40);

            MMPD_Scaler_ResetModule(pHandle->PipeID);
            MMPD_Icon_ResetModule(pHandle->PipeID);
            MMPD_IBC_ResetModule(pHandle->PipeID);
            
            MMPD_Fctl_ResetIBCBufIdx(pHandle->PipeID);
            MMPD_Fctl_SetPipeAttrForIbcFB(&m_VRPreviewFctlAttr[0]);
        }
        else {
            MMPD_Scaler_RestoreAttributes(pHandle->PipeID);
            MMPD_IBC_SetAttributes(pHandle->PipeID, &pHandle->pLinkAttr->IBCPipeAttr);	
        }
        #endif
    }
    else if (pHandle->usEncID == MMP_MJPEG_STREAM_REARCAM_VIDEO) {
        
        if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264) ||\
            CAM_CHECK_USB(USB_CAM_SONIX_MJPEG)) {
            // For restore gsPreviewBufWidth and gsPreviewBufHeight. (TBD)
            MMPD_Fctl_SetPipeAttrForIbcFB(&m_DecMjpegToPrevwFctlAttr);
        }
        else if (CAM_CHECK_USB(USB_CAM_AIT)) {
            #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
            if (MMP_IsSupportDecMjpegToPreview()) {
                // For restore gsPreviewBufWidth and gsPreviewBufHeight. (TBD)	
                MMPD_Fctl_SetPipeAttrForIbcFB(&m_DecMjpegToPrevwFctlAttr);
            }
            #endif
        }
    }
    
    /* Restore the link setting of IBC */
    MMPD_Fctl_RestoreIBCLinkAttr(pHandle->PipeID, 
                                 pHandle->pLinkAttr->IBCLinkType, 
                                 pHandle->pLinkAttr->previewDev, 
                                 pHandle->pLinkAttr->winID,
                                 pHandle->pLinkAttr->rotateDir);

    MMPD_IBC_ClearFrameEnd(pHandle->PipeID);
    MMPD_Scaler_SetPath(pHandle->PipeID, pHandle->pLinkAttr->scalerSrc, MMP_TRUE);
    MMPD_Scaler_SetEnable(pHandle->PipeID, MMP_TRUE);
    MMPD_IBC_CheckFrameEnd(pHandle->PipeID);

    MMPD_IBC_SetInterruptEnable(pHandle->PipeID, MMP_IBC_EVENT_FRM_END, MMP_TRUE);
    MMPD_IBC_SetStoreEnable(pHandle->PipeID, MMP_TRUE);

    return MMP_ERR_NONE;
}

#endif //SUPPORT_MJPEG_WIFI_STREAM
