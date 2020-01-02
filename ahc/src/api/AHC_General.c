/**
 @file AHC_General.c
 @brief AHC general control Function
 @author
 @version 1.0
*/

/*===========================================================================
 * Include files
 *===========================================================================*/

#include "Customer_config.h"

#include "mmp_lib.h"
#include "mmu.h"
#include "lib_retina.h"
#include "mmp_err.h"
#include "ait_utility.h"
#include "snr_cfg.h"
#include "vidrec_cfg.h"
#include "mmps_rtc.h"
#include "mmps_pio.h"
#include "mmps_usb.h"
#include "mmps_dsc.h"
#include "mmps_3gprecd.h"
#include "mmps_audio.h"
#include "mmps_aui.h"
#include "mmps_pwm.h"
#include "mmps_vidplay.h"
#include "mmpd_system.h"
#include "mmpd_ptz.h"
#include "mmpf_wd.h"
#include "mmpf_display.h"
#include "mmpf_pwm.h"
#include "mmpf_sensor.h"
#include "mmpf_storage_api.h"
#include "mmpf_dram.h"

#include "AIHC_DCF.h"
#include "AIHC_Browser.h"
#include "AIHC_GUI.h"
#include "AHC_OS.h"
#include "AHC_FS.h"
#include "AHC_General.h"
#include "AHC_Parameter.h"
#include "AHC_Message.h"
#include "AHC_Menu.h"
#include "AHC_DCFDT.h"
#include "AHC_Parameter.h"
#include "AHC_GUI.h"
#include "AHC_Audio.h"
#include "AHC_Video.h"
#include "AHC_USBHost.h"
#include "AHC_PMU.h"
#include "AHC_Sensor.h"
#include "AHC_Config_SDK.h"
#include "AHC_DateTime.h"
#include "AHC_Version.h"
#include "AHC_Stream.h"
#include "AHC_Media.h"
#include "AHC_Warningmsg.h"
#include "AHC_Utility.h"
#include "AHC_UF.h"
#include "AHC_USB.h"
#include "AHC_Callback.h"

#include "MenuSetting.h"
#include "keyparser.h"
#include "dsc_key.h"
#include "StateHDMIFunc.h"
#include "StateTVFunc.h"
#include "StateVideoFunc.h"
#include "LedControl.h"
#include "ZoomControl.h"
#include "PCAM_API.h"
#include "MenuSetting.h"
#include "MenuDrawCommon.h"
#include "SoundEffectName.h"
#include "MenuDrawingFunc.h"
#include "dram_cfg.h"

#if defined(WIFI_PORT) && (WIFI_PORT == 1)
#include "wlan.h"
#include "netapp.h"
#include "mmpf_streaming.h"
#endif
#if (GPS_CONNECT_ENABLE)
#include "GPS_ctl.h"
#endif
#if (GSENSOR_CONNECT_ENABLE)
#include "GSensor_ctrl.h"
#endif
#if (SUPPORT_IR_CONVERTER)
#include "ir_ctrl.h"
#endif
#if (SD_UPDATE_FW_EN)
#include "SD_Updater.h"
#endif
#if (EDOG_ENABLE)
#include "EDOG_ctl.h"
#endif

/*===========================================================================
 * Project definition check
 *===========================================================================*/

#if (AHC_DRAM_SIZE == COMMON_DRAM_SIZE_NG)
#error "Definition AHC_DRAM_SIZE NG!!!"
#endif

#if (VR_VIDEO_TYPE == COMMON_VR_VIDEO_TYPE_NG)
#error "Definition VR_VIDEO_TYPE NG!!!"
#endif

#if (defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE == 1)) && (AHC_SHAREENC_SUPPORT == 1)
#error "PCAM_UVC_MIX_MODE_ENABLE does not support One click share function."
#endif

#if 0
void _____Global_Variable_________(){ruturn;} //dummy
#endif

/** @addtogroup AHC_GENERAL
@{
*/
/*===========================================================================
 * Global varible
 *===========================================================================*/

AHC_OS_MQID                     AHC_MSG_QId;
void                            *AHC_MsgQueue[AHC_MSG_QUEUE_SIZE] = {0};
AHC_OS_MQID                     AHC_HP_MSG_QId;
void                            *AHC_HPMsgQueue[AHC_HP_MSG_QUEUE_SIZE] = {0};

static AHC_QUEUE_MESSAGE        m_MessageQueue[AHC_MSG_QUEUE_SIZE] = {0};
static MMP_ULONG                m_MessageQueueIndex_W;
static MMP_ULONG                m_MessageQueueIndex_R;

static AHC_QUEUE_MESSAGE        m_HPMessageQueue[AHC_HP_MSG_QUEUE_SIZE] = {0};
static MMP_ULONG                m_HPMessageQueueIndex_W;
static MMP_ULONG                m_HPMessageQueueIndex_R;

static AHC_OS_SEMID             m_AHLMessageSemID;
static AHC_OS_SEMID             m_AHLHPMessageSemID;
static AHC_OS_SEMID             m_AHLHPMessageCountSemID;
static AHC_BOOL                 m_bSendAHLMessage       = AHC_TRUE;

static AHC_MODE_ID              m_AhcSystemMode         = AHC_MODE_IDLE;

MMP_USHORT                      gsAhcPrmSensor          = PRM_SENSOR;
MMP_USHORT                      gsAhcScdSensor          = SCD_SENSOR;

static UINT32                   m_ulPlaybackFileType    = DCF_OBG_JPG;
static MMP_BYTE                 m_CurPlayFileName[MAX_FILE_NAME_SIZE];
static UINT32                   m_ulCurrentPBFileType;
UINT16                          m_ulCurrentPBWidth;
UINT16                          m_ulCurrentPBHeight;
AHC_BOOL                        m_ubPlaybackRearCam     = AHC_FALSE;

static UINT32                   m_ulVideoPlayStopStatus = AHC_VIDEO_PLAY_EOF;
static MMP_ULONG                m_ulVideoPlayStartTime  = 0;
static UINT32                   m_ulAudioPlayStopStatus = AHC_AUDIO_EVENT_EOF;
static MMP_ULONG                m_ulAudioPlayStartTime  = 0;

static UINT8                    m_bAHCGeneralInit       = 0;
MMP_ULONG                       glAhcBootComplete       = MMP_FALSE;

static AHC_BOOL                 gbPIRStarted            = AHC_FALSE;

AHC_OS_SEMID                    m_AhcModeSemID = 0xFF;

#if (SW_STICKER_EN == 1)
MMP_USHORT                      m_usStickerSrcW, m_usStickerSrcH;
MMP_USHORT                      m_usStickerCaptureW, m_usStickerCaptureH;
MMP_USHORT                      m_usStickerXoffset, m_usStickerYoffset;
MMP_UBYTE                       m_ubStickerOsdId_0, m_ubStickerOsdId_1;
#endif

AHC_BOOL                        gbBlockRealIDUIKeyTask = AHC_FALSE;

static UINT32                   gulCurKeyEventID = EVENT_NONE;

static AHC_WINDOW_RECT          gsCustomerRearPrevWindow 	= {0, 0, 0, 0};
static AHC_WINDOW_RECT          gsCustomerFrontPrevWindow 	= {0, 0, 0, 0};

MMP_USHORT                      gsStillZoomIndex = 0xFFFF;

/*===========================================================================
 * Extern varible
 *===========================================================================*/

extern AHC_PARAM                    glAhcParameter;
extern UINT8                        m_uiPlayAudioFmt;
extern AHC_BOOL                     m_bAHCAudioPlaying;
extern MMP_AUDIO_MP3_INFO           m_gsAHCMp3Info;
extern MMP_AUDIO_OGG_INFO           m_gsAHCOggInfo;
extern MMP_AUDIO_WMA_INFO           m_gsAHCWmaInfo;
extern MMP_AUDIO_WAV_INFO           m_gsAHCWavInfo;

extern AHC_BOOL                     gbAhcDbgBrk;

extern UINT32                       m_CurPlayTime_MediaError;

/*===========================================================================
 * Extern function
 *===========================================================================*/

extern void     AHC_CheckCallbackExisted( void );

/*===========================================================================
 * Main body
 *===========================================================================*/

#if 0
void _____Utility_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_KeyEventIDCheckConflict
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_KeyEventIDCheckConflict(UINT32 ulCurKeyEventID)
{
    if (((BUTTON_USB_B_DEVICE_DETECTED == gulCurKeyEventID)
        || (BUTTON_USB_B_DEVICE_REMOVED == gulCurKeyEventID)
        || (BUTTON_TV_DECODER_SRC_TYPE == gulCurKeyEventID)
        ) &&
        ((BUTTON_VRCB_RECDSTOP_CARDSLOW == ulCurKeyEventID)
        || (BUTTON_VRCB_FILE_FULL == ulCurKeyEventID)
        || (BUTTON_VRCB_MEDIA_FULL == ulCurKeyEventID))) 
    {
        AHC_PRINT_RET_ERROR(0, 0);
        printc("gulCurKeyEventID:0x%x, ulCurKeyEventID:0x%xr\n", gulCurKeyEventID, ulCurKeyEventID);
        return AHC_FALSE;
    }
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetCurKeyEventID
//  Description :
//------------------------------------------------------------------------------
void AHC_SetCurKeyEventID(UINT32 ulCurKeyEventID)
{
    gulCurKeyEventID = ulCurKeyEventID;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetCurKeyEventID
//  Description :
//------------------------------------------------------------------------------
UINT32 AHC_GetCurKeyEventID(void)
{
    return gulCurKeyEventID;
}

#if 0
void _____PTZ_Function_________(){} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_SetPreviewZoomConfig
//  Description :
//------------------------------------------------------------------------------
/**
 @brief This API configures the abilities of digital zoom.

 This function configures the abilities of digital zoom. System calculates each
 zoom interval between steps according to the max multiplier. Currently, the zoom can
 only based on center of scene.

 Parameters:

 @param[in] bySteps        Max Zoom Step
 @param[in] byMaxRatio     Max Zoom Ratio
 @retval AHC_TRUE Success.
*/
AHC_BOOL AHC_SetPreviewZoomConfig(UINT16 bySteps, UINT8 byMaxRatio)
{
    MMPS_DSC_SetZoomConfig(bySteps, byMaxRatio);
    MMPS_3GPRECD_SetZoomConfig(bySteps, byMaxRatio);

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetPreviewZoom
//  Description :
//------------------------------------------------------------------------------
/**
 @brief This API set Zoom-In / Zoom-Out

 This function set Zoom-In / Zoom-Out. Currently, the zoom can
 only based on center of scene.

 Parameters:

 @param[in] CaptureMode     Video mode or DSC mode
 @param[in] byDirection     Zoom-In or Zoom Out
 @param[in] byDirection     Zoom Step

 @retval AHC_TRUE Success.
*/
AHC_BOOL AHC_SetPreviewZoom(AHC_CAPTURE_MODE CaptureMode, AHC_ZOOM_DIRECTION byDirection, UINT8 bySteps)
{
    MMP_BOOL        bInPreview  = MMP_FALSE;
    MMP_BOOL        bInRecord   = MMP_FALSE;
    MMP_ERR         error       = MMP_ERR_NONE;
    MMP_IBC_PIPEID 	ubPipe;
    MMP_SHORT       sCurDir;
    MMP_USHORT      usCurZoomStep;

    if (VIDEO_CAPTURE_MODE == CaptureMode) {
        MMPS_3GPRECD_GetPreviewPipeStatus(gsAhcPrmSensor, &bInPreview);
        MMPS_3GPRECD_GetPreviewPipe(gsAhcPrmSensor, &ubPipe);
        MMPS_3GPRECD_GetRecordPipeStatus(MMPS_3GPRECD_FILESTREAM_NORMAL, &bInRecord);
        
        if (!bInPreview) {
            return AHC_FALSE;
        }
    }
    else if (STILL_CAPTURE_MODE == CaptureMode) {
        MMPS_DSC_GetPreviewStatus(&bInPreview);
        MMPS_DSC_GetPreviewPipe(&ubPipe);

        if (!bInPreview) {
            return AHC_FALSE;
        }
    }

    if (byDirection != AHC_SENSOR_ZOOM_STOP) {

        MMPD_PTZ_GetCurPtzStep(ubPipe, &sCurDir, &usCurZoomStep, NULL, NULL);
        
        if (byDirection == AHC_SENSOR_ZOOM_IN) {
            sCurDir = MMP_PTZ_ZOOM_INC_IN;
        }
        else if (byDirection == AHC_SENSOR_ZOOM_OUT) {
            sCurDir = MMP_PTZ_ZOOM_INC_OUT;
        }
        
        if (((MMP_SHORT)usCurZoomStep + sCurDir * 1) <= 0)
            usCurZoomStep = 0;
        else
            usCurZoomStep += (sCurDir * 1);
    }
    
    if (byDirection == AHC_SENSOR_ZOOM_IN) {
        
        if (CaptureMode == STILL_CAPTURE_MODE) {
            error = MMPS_DSC_SetPreviewZoom(MMP_PTZ_ZOOMIN, usCurZoomStep);
        }
        else {
            if (bInRecord)
                error = MMPS_3GPRECD_SetPreviewZoom(MMPS_3GPRECD_ZOOM_PATH_BOTH, MMP_PTZ_ZOOMIN, usCurZoomStep);
            else
                error = MMPS_3GPRECD_SetPreviewZoom(MMPS_3GPRECD_ZOOM_PATH_PREV, MMP_PTZ_ZOOMIN, usCurZoomStep);
        }
    }
    else if (byDirection == AHC_SENSOR_ZOOM_OUT) {

        if (CaptureMode == STILL_CAPTURE_MODE) {
            error = MMPS_DSC_SetPreviewZoom(MMP_PTZ_ZOOMOUT, usCurZoomStep);
        }
        else {
            if (bInRecord)
                error = MMPS_3GPRECD_SetPreviewZoom(MMPS_3GPRECD_ZOOM_PATH_BOTH, MMP_PTZ_ZOOMOUT, usCurZoomStep);
            else
                error = MMPS_3GPRECD_SetPreviewZoom(MMPS_3GPRECD_ZOOM_PATH_PREV, MMP_PTZ_ZOOMOUT, usCurZoomStep);
        }
    }
    else if (byDirection == AHC_SENSOR_ZOOM_STOP) {

    	if (CaptureMode == STILL_CAPTURE_MODE) {
            error = MMPS_DSC_SetPreviewZoom(MMP_PTZ_ZOOMSTOP, usCurZoomStep);
        }
        else {
            if (bInRecord)
                error = MMPS_3GPRECD_SetPreviewZoom(MMPS_3GPRECD_ZOOM_PATH_BOTH, MMP_PTZ_ZOOMSTOP, usCurZoomStep);
            else
                error = MMPS_3GPRECD_SetPreviewZoom(MMPS_3GPRECD_ZOOM_PATH_PREV, MMP_PTZ_ZOOMSTOP, usCurZoomStep);
        }
    }
    else {
        return AHC_FALSE;
    }

    return (error == MMP_ERR_NONE) ? AHC_TRUE : AHC_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_PlaybackZoom
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set playback zoom

 Performs zoom during still image playback.
 Parameters:
 @param[in] uwStartX The top-left corner's X of the zoom window.
 @param[in] uwStartY The top-left corner' Y of the zoom window.
 @param[in] uwWidth The width of zoom window.
 @param[in] uwHeight The height of zoom window.
 @retval AHC_TRUE Success.
*/
AHC_BOOL AHC_PlaybackZoom(UINT16 uwStartX, UINT16 uwStartY, UINT16 uwWidth, UINT16 uwHeight)
{
    MMP_ERR error;

    error = MMPS_DSC_PlaybackExecutePTZ(uwStartX, uwStartY, uwWidth, uwHeight);

    if (MMP_ERR_NONE == error)
        return AHC_TRUE;
    else
        return AHC_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_PreviewWindowOp
//  Description :
//------------------------------------------------------------------------------
/**
 @brief This API set/get Preview window of Front Cam / Rear Cam
 
 Parameters: 
 @param[in]     op      AHC_PREVIEW_WINDOW_OP_GET to set window rect,AHC_PREVIEW_WINDOW_OP_SET to set window rect
                        AHC_PREVIEW_WINDOW_FRONT/AHC_PREVIEW_WINDOW_REAR to choice window
 @param[in/out] pRect   in get window case, this is addressed to the memory of structure AHC_WINDOW_RECT which will be updated.
                        in set window case, use this addresed memory to set window rect.
 @retval return 0 is success ,others are unknow operations.
*/
int	AHC_PreviewWindowOp(int op, AHC_WINDOW_RECT * pRect)
{
    int subop, win;
    
    subop = op & AHC_PREVIEW_WINDOW_OP_MASK;
    win   = op & AHC_PREVIEW_WINDOW_MASK;
    
    if (subop == AHC_PREVIEW_WINDOW_OP_GET)
    {
        if (win == AHC_PREVIEW_WINDOW_FRONT)
            memcpy(pRect, &gsCustomerFrontPrevWindow, sizeof(AHC_WINDOW_RECT));
        else
            memcpy(pRect, &gsCustomerRearPrevWindow, sizeof(AHC_WINDOW_RECT)); // run this
    }
    else if (subop == AHC_PREVIEW_WINDOW_OP_SET)
    {
        if (win == AHC_PREVIEW_WINDOW_FRONT)
            memcpy(&gsCustomerFrontPrevWindow, pRect, sizeof(AHC_WINDOW_RECT));
        else
            memcpy(&gsCustomerRearPrevWindow, pRect, sizeof(AHC_WINDOW_RECT));
    }
    else {
        return -1;
    }
    
    return 0;
}

#if 0
void _____Sticker_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_STICKER_TransDateToString
//  Description : Transfer psRtcTime(int) to pbyDate(string)
//------------------------------------------------------------------------------
AHC_BOOL AHC_STICKER_TransDateToString(AHC_RTC_TIME* psRtcTime, INT8* pbyDate, UINT8 byDateConfig, UINT8 byFormatConfig)
{
    AHC_TIME_STRING sTimeString;

    AHC_UTILITY_Int2Str(psRtcTime->uwYear,     sTimeString.byYear);
    AHC_UTILITY_Int2Str(psRtcTime->uwMonth,    sTimeString.byMonth);
    AHC_UTILITY_Int2Str(psRtcTime->uwDay,      sTimeString.byDay);
    AHC_UTILITY_Int2Str(psRtcTime->uwHour,     sTimeString.byHour);
    AHC_UTILITY_Int2Str(psRtcTime->uwMinute,   sTimeString.byMinute);
    AHC_UTILITY_Int2Str(psRtcTime->uwSecond,   sTimeString.bySecond);

    if (AHC_ACC_TIMESTAMP_TIME_ONLY != byDateConfig) {

        if  (byFormatConfig == AHC_ACC_TIMESTAMP_FORMAT_1) {

            STRCPY(pbyDate, sTimeString.byYear);
            STRCAT(pbyDate, " ");

            if (psRtcTime->uwMonth < 10)
                STRCAT(pbyDate, "0");

            STRCAT(pbyDate, sTimeString.byMonth);
            STRCAT(pbyDate, " ");

            if (psRtcTime->uwDay < 10)
                STRCAT(pbyDate, "0");

            STRCAT(pbyDate, sTimeString.byDay);
        }
        else if (byFormatConfig == AHC_ACC_TIMESTAMP_FORMAT_2) {
            
            STRCPY(pbyDate, sTimeString.byYear);
            STRCAT(pbyDate, "/");

            if (psRtcTime->uwMonth < 10)
                STRCAT(pbyDate, "0");

            STRCAT(pbyDate, sTimeString.byMonth);
            STRCAT(pbyDate, "/");

            if (psRtcTime->uwDay < 10)
                STRCAT(pbyDate, "0");

            STRCAT(pbyDate, sTimeString.byDay);
        }
        else if (byFormatConfig == AHC_ACC_TIMESTAMP_FORMAT_3) {

            if (psRtcTime->uwDay < 10)
                STRCPY(pbyDate, "0");

            STRCAT(pbyDate, sTimeString.byDay);
            STRCAT(pbyDate, "/");

            if (psRtcTime->uwMonth < 10)
                STRCAT(pbyDate, "0");

            STRCAT(pbyDate, sTimeString.byMonth);
            STRCAT(pbyDate, "/");
            STRCAT(pbyDate, sTimeString.byYear);
        }
        else if (byFormatConfig == AHC_ACC_TIMESTAMP_FORMAT_4) {

            if (psRtcTime->uwMonth < 10)
                STRCPY(pbyDate, "0");

            STRCAT(pbyDate, sTimeString.byMonth);
            STRCAT(pbyDate, "/");

            if (psRtcTime->uwDay < 10)
                STRCAT(pbyDate, "0");

            STRCAT(pbyDate, sTimeString.byDay);
            STRCAT(pbyDate, "/");
            STRCAT(pbyDate, sTimeString.byYear);
        }

        if (AHC_ACC_TIMESTAMP_DATE_AND_TIME == (byDateConfig & AHC_ACC_TIMESTAMP_DATE_MASK)) {

            STRCAT(pbyDate, " ");

            if (psRtcTime->uwHour < 10)
                STRCAT(pbyDate, "0");

            STRCAT(pbyDate, sTimeString.byHour);
            STRCAT(pbyDate, ":");

            if (psRtcTime->uwMinute < 10)
                STRCAT(pbyDate,"0");

            STRCAT(pbyDate, sTimeString.byMinute);
            STRCAT(pbyDate, ":");

            if (psRtcTime->uwSecond < 10)
                STRCAT(pbyDate,"0");

            STRCAT(pbyDate, sTimeString.bySecond);
        }
    }
    else {
        if (psRtcTime->uwHour < 10) {
            STRCPY(pbyDate, "0");
            STRCAT(pbyDate, sTimeString.byHour);
        }
        else {
        STRCPY(pbyDate, sTimeString.byHour);
        }

        STRCAT(pbyDate, ":");

        if (psRtcTime->uwMinute < 10)
            STRCAT(pbyDate, "0");

        STRCAT(pbyDate, sTimeString.byMinute);
        STRCAT(pbyDate, ":");

        if (psRtcTime->uwSecond < 10)
            STRCAT(pbyDate, "0");

        STRCAT(pbyDate, sTimeString.bySecond);
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_STICKER_DrawTextEdge
//  Description :
//------------------------------------------------------------------------------
void AHC_STICKER_DrawTextEdge(UINT32 uiSrcAddr, UINT16 uwSrcW, UINT16 uwSrcH, UBYTE TextColor, UBYTE BGColor, UBYTE EdgeColor)
{
#if (STICKER_DRAW_EDGE)

    UINT16 x,y;
    MMP_UBYTE *Addr = (MMP_UBYTE *)uiSrcAddr;

    for (y = 1; y < uwSrcH-1; y++) {
        for (x = 1; x < uwSrcW-1; x++) {

            if (Addr[y*uwSrcW+x] == BGColor) {
                if ((Addr[(y-1)*uwSrcW+(x-1)] == TextColor) || 
                    (Addr[(y-1)*uwSrcW+x] == TextColor) || 
                    (Addr[(y-1)*uwSrcW+(x+1)] == TextColor) || \
                    (Addr[y*uwSrcW+(x-1)] == TextColor) || 
                    (Addr[y*uwSrcW+(x+1)] == TextColor) || \
                    (Addr[(y+1)*uwSrcW+(x-1)] == TextColor) || 
                    (Addr[(y+1)*uwSrcW+x] == TextColor) || 
                    (Addr[(y+1)*uwSrcW+(x+1)] == TextColor))
                   
                   Addr[y*uwSrcW+x] = EdgeColor;
            }
        }
    }
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_STICKER_TransRGB565toIndex8
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_STICKER_TransRGB565toIndex8(UINT32 uiSrcAddr,
                                         UINT32 uiSrcW,
                                         UINT32 uiSrcH,
                                         UINT32 uiDestAddr,
                                         UINT32 uiDestW,
                                         UINT32 uiDestH,
                                         UINT8  byForegroundColor,
                                         UINT8  byBackgroundColor)
{
    UINT16  *puwSrcBuf;
    UINT8   *pbyDestBuf;
    UINT32  uiY;
    UINT32  uiX;

    puwSrcBuf   = (UINT16 *)uiSrcAddr;
    pbyDestBuf  = (UINT8 *)uiDestAddr;

    for (uiY = 0; uiY < uiDestH; ++uiY) {

        for (uiX = 0; uiX < uiDestW; ++uiX) {

            *(pbyDestBuf + (uiY * uiDestW) + uiX) =
                (*(puwSrcBuf + (uiY * uiSrcW) + (uiX + (uiSrcW - uiDestW))) != 0) ? byForegroundColor : byBackgroundColor;
        }
    }

    return AHC_TRUE;
}

#if (SW_STICKER_EN == 1)
//------------------------------------------------------------------------------
//  Function    : AHC_SWSticker_MoveBuf
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SWSticker_MoveBuf(MMP_ULONG ulYBufAddr, MMP_ULONG ulUVBufAddr)
{
    UINT16  i,j;
    UINT32  uiSrcAddr;
    UINT32  uiTimeStampOp;
    UINT8   ubY, ubU, ubV;
    
    ubY = 235;   // WHITE RGB to YUV
    ubU = 128;
    ubV = 133;

    AHC_GetCaptureConfig(ACC_DATE_STAMP, &uiTimeStampOp);

    if (uiTimeStampOp & AHC_ACC_TIMESTAMP_ENABLE_MASK) {

        if (m_uiVideoStampBufIndex == 1) {
            AIHC_GUI_GetOSDBufAddr(m_ubStickerOsdId_0, &uiSrcAddr);
        }
        else {
            AIHC_GUI_GetOSDBufAddr(m_ubStickerOsdId_1, &uiSrcAddr);
        }

        for (j = 0; j < m_usStickerSrcH; j++)
        {
            for (i = 0; i < m_usStickerSrcW; i++)
            {
                if (*(MMP_UBYTE *)(uiSrcAddr + j * m_usStickerSrcW + i) != 0)
                {
                    #if (STICKER_PATTERN == 1)
                    if ((i < 110) && (j < 32)) {
                        ubY = 145;  // YELLOW RGB to YUV
                        ubU = 53;
                        ubV = 185;
                    }
                    else {
                        ubY = 235;  // WHITE RGB to YUV
                        ubU = 128;
                        ubV = 133;
                    }
                    #endif

                    *(MMP_UBYTE *)(ulYBufAddr + (j + m_usStickerYoffset) * m_usStickerCaptureW + (i + m_usStickerXoffset)) = ubY;

                    if ((i % 2 == 0) && (j % 2 ==0))
                    {
                        *(MMP_UBYTE *)(ulUVBufAddr + (j + m_usStickerYoffset)/2 * m_usStickerCaptureW + i + m_usStickerXoffset)     =  ubU;
                        *(MMP_UBYTE *)(ulUVBufAddr + (j + m_usStickerYoffset)/2 * m_usStickerCaptureW + i + m_usStickerXoffset + 1) =  ubV;
                    }
                }
            }
        }
    }
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SWSticker_SetCBFuncPtr
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SWSticker_SetCBFuncPtr(void* pFuncPtr)
{
    //MMPF_Display_SetSWStickerCB(pFuncPtr);

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SWSticker_SetPosition
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SWSticker_SetPosition(MMP_USHORT uwSrcW, MMP_USHORT uwSrcH, MMP_USHORT uwCaptureW, MMP_USHORT uwCaptureH,
                                   MMP_USHORT uwXoffset, MMP_USHORT uwYoffset, MMP_UBYTE ubOSDid0, MMP_UBYTE ubOSDid1)
{
    m_usStickerSrcW         = uwSrcW;
    m_usStickerSrcH         = uwSrcH;
    m_usStickerCaptureW     = uwCaptureW;
    m_usStickerCaptureH     = uwCaptureH;
    m_usStickerXoffset      = uwXoffset;
    m_usStickerYoffset      = uwYoffset;
    m_ubStickerOsdId_0      = ubOSDid0;
    m_ubStickerOsdId_1      = ubOSDid1;

    return AHC_TRUE;
}
#endif

#if 0
void _____MediaPlayback_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_SetPlayBackRearCam
//  Description :
//------------------------------------------------------------------------------
void AHC_SetPlayBackRearCam(AHC_BOOL bIsRear)
{
    m_ubPlaybackRearCam = bIsRear;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetPlayBackRearCam
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_GetPlayBackRearCam(void)
{
    return m_ubPlaybackRearCam;
}

//------------------------------------------------------------------------------
//  Function    : VideoPlayStopCallback
//  Description :
//------------------------------------------------------------------------------
void VideoPlayStopCallback(void *Context, MMP_ULONG flag1, MMP_ULONG flag2)
{
    #if (DAC_NOT_OUTPUT_SPEAKER_HAS_NOISE)
    if (AHC_TRUE == AHC_IsSpeakerEnable())
    {
        AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO, AHC_FALSE);
    }
    #endif

    m_ulVideoPlayStopStatus = flag1;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetVideoPlayStartTime
//  Description :
//------------------------------------------------------------------------------
void AHC_SetVideoPlayStartTime(MMP_ULONG ulStartTime)
{
    m_ulVideoPlayStartTime = ulStartTime;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetVideoPlayStopStatus
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_GetVideoPlayStopStatus(UINT32 *pwValue)
{
    *pwValue = m_ulVideoPlayStopStatus;

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetVideoPlayStopStatus
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetVideoPlayStopStatus(UINT32 Value)
{
    m_ulVideoPlayStopStatus = Value;

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AudioPlayStopCallback
//  Description :
//------------------------------------------------------------------------------
void AudioPlayStopCallback(void *Context, MMP_ULONG flag1, MMP_ULONG flag2)
{
    m_ulAudioPlayStopStatus = flag1;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetAudioPlayStopStatus
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_GetAudioPlayStopStatus(UINT32 *pwValue)
{
    *pwValue = m_ulAudioPlayStopStatus;

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAudioPlayStopStatus
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetAudioPlayStopStatus(UINT32 Value)
{
    m_ulAudioPlayStopStatus = Value;

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AIHC_GetCurrentPBFileType
//  Description : Get current playback file type
//------------------------------------------------------------------------------
AHC_BOOL AIHC_GetCurrentPBFileType(UINT32 *pFileType)
{
    *pFileType = m_ulCurrentPBFileType;

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AIHC_GetCurrentPBHeight
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AIHC_GetCurrentPBHeight(UINT16 *pHeight)
{
    *pHeight = m_ulCurrentPBHeight;

    return AHC_TRUE;
}

#if (SUPPORT_ESD_RECOVER_MOVIEPB == 1)
void VideoMediaErrorPlayStopHandler(void)
{
    #if (SUPPORT_ESD_RECOVER_MOVIEPB == 1)
    UINT32 CurrentDcfIdx;
    INT8  chAllowedChar[MAX_ALLOWED_WORD_LENGTH];
    AHC_BOOL    err;

    #if(defined(DEVICE_GPIO_2NDSD_PLUG))

    #if(TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_MASTER_SLAVE)
    if(AHC_IsSDInserted() && (m_ulVideoPlayStopStatus == AHC_VIDEO_PLAY_ERROR_STOP))
    #elif(TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_COMPLEMENTARY)
    if( (AHC_IsSDInserted() || AHC_Is2ndSDInserted()) && (m_ulVideoPlayStopStatus == AHC_VIDEO_PLAY_ERROR_STOP) )
    #endif//TWOSD_WORK_MODEL

    #else//DEVICE_GPIO_2NDSD_PLUG

    if(AHC_IsSDInserted() && (m_ulVideoPlayStopStatus == AHC_VIDEO_PLAY_ERROR_STOP))

    #endif//DEVICE_GPIO_2NDSD_PLUG
    {
         UINT8 bValue = 0;

         MovPBFunc_StopTimer();
         AHC_UF_GetCurrentIndex(&CurrentDcfIdx);
         AHC_UF_GetAllowedChar(chAllowedChar, MAX_ALLOWED_WORD_LENGTH);

         AIHC_StopPlaybackMode();

         //AHC_GPIO_GetData(AHC_PIO_REG_LGPIO28,&bValue);
         //if(bValue != AHC_FALSE)
         AHC_FS_IOCtl(AHC_UF_GetRootName(), 4, AHC_FS_CMD_RESET_MEDIUM, NULL, NULL);

         err = AHC_RemountDevices(MenuSettingConfig()->uiMediaSelect);

         AHC_OS_Sleep(500);

         AHC_UF_SetFreeChar( 0,DCF_SET_FREECHAR,(UINT8*)chAllowedChar);
         AHC_UF_SetCurrentIndex(CurrentDcfIdx);
        // SetKeyPadEvent(VRCB_MEDIA_ERROR);

         MovPlaybackParamReset();
         MediaPlaybackConfig( 1 );
         MovPBFunc_StartTimer(300);
         //MovPlayback_Play();
         MovPlayback_Play_MediaError(m_CurPlayTime_MediaError);
    }
    #if 1

    #if(defined(DEVICE_GPIO_2NDSD_PLUG))

    #if(TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_MASTER_SLAVE)
    else if( AHC_IsSDInserted() && (m_ulVideoPlayStopStatus == AHC_VIDEO_PLAY) )
    #elif(TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_COMPLEMENTARY)
    else if( (AHC_IsSDInserted() || AHC_Is2ndSDInserted()) && (m_ulVideoPlayStopStatus == AHC_VIDEO_PLAY) )
    #endif

    #else//DEVICE_GPIO_2NDSD_PLUG

    else if (AHC_IsSDInserted() && (m_ulVideoPlayStopStatus == AHC_VIDEO_PLAY))

    #endif//DEVICE_GPIO_2NDSD_PLUG
    {
         UINT8 bValue = 0;
         
         AHC_UF_GetCurrentIndex(&CurrentDcfIdx);
         AHC_UF_GetAllowedChar(chAllowedChar, MAX_ALLOWED_WORD_LENGTH);

         AIHC_StopPlaybackMode();

         if(bValue != AHC_FALSE)
         AHC_FS_IOCtl(AHC_UF_GetRootName(), 4, AHC_FS_CMD_RESET_MEDIUM, NULL, NULL);

         AHC_RemountDevices(MenuSettingConfig()->uiMediaSelect);

         AHC_OS_Sleep(300);
         AHC_UF_SetFreeChar( 0,DCF_SET_FREECHAR,(UINT8*)chAllowedChar);
         AHC_UF_SetCurrentIndex(CurrentDcfIdx);
        AHC_SendAHLMessage(AHLM_GPIO_BUTTON_NOTIFICATION, VRCB_MEDIA_ERROR, 0);
    }
    #endif
    
    #if 1
    #if(defined(DEVICE_GPIO_2NDSD_PLUG))

    #if(TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_MASTER_SLAVE)
    else if( AHC_IsSDInserted() && (uiGetCurrentState() == UI_BROWSER_STATE) )
    #elif(TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_COMPLEMENTARY)
    else if( (AHC_IsSDInserted() || AHC_Is2ndSDInserted()) && (uiGetCurrentState() == UI_BROWSER_STATE) )
    #endif

    #else//DEVICE_GPIO_2NDSD_PLUG

    else if(AHC_IsSDInserted() && (uiGetCurrentState() == UI_BROWSER_STATE))

    #endif//DEVICE_GPIO_2NDSD_PLUG
    {
         AHC_FS_IOCtl(AHC_UF_GetRootName(), 4, AHC_FS_CMD_RESET_MEDIUM, NULL, NULL);
         //AHC_RemountDevices(MenuSettingConfig()->uiMediaSelect);
         //AHC_OS_Sleep(200);
    }
    #if 1

    #if(defined(DEVICE_GPIO_2NDSD_PLUG))
    #if(TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_MASTER_SLAVE)
    else if( AHC_IsSDInserted() && (uiGetCurrentState() == UI_PLAYBACK_STATE) )
    #elif(TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_COMPLEMENTARY)
    else if( (AHC_IsSDInserted() || AHC_Is2ndSDInserted()) && (uiGetCurrentState() == UI_PLAYBACK_STATE) )
    #endif//TWOSD_WORK_MODEL

    #else//DEVICE_GPIO_2NDSD_PLUG

    else if(AHC_IsSDInserted() && (uiGetCurrentState() == UI_PLAYBACK_STATE))

    #endif//DEVICE_GPIO_2NDSD_PLUG
    {
         UINT8 bValue = 0;
         AHC_UF_GetCurrentIndex(&CurrentDcfIdx);
         AHC_UF_GetAllowedChar(chAllowedChar, MAX_ALLOWED_WORD_LENGTH);
         AHC_GPIO_GetData(AHC_PIO_REG_LGPIO28,&bValue);
         if(bValue != AHC_FALSE)
         AHC_FS_IOCtl(AHC_UF_GetRootName(), 4, AHC_FS_CMD_RESET_MEDIUM, NULL, NULL);

         AHC_RemountDevices(MenuSettingConfig()->uiMediaSelect);
         AHC_OS_Sleep(300);
         AHC_UF_SetFreeChar( 0,DCF_SET_FREECHAR,(UINT8*)chAllowedChar);
         AHC_UF_SetCurrentIndex(CurrentDcfIdx);
		 AHC_SendAHLMessage(AHLM_GPIO_BUTTON_NOTIFICATION, VRCB_MEDIA_ERROR, 0);
    }
    #endif
    #if 1

    #if(defined(DEVICE_GPIO_2NDSD_PLUG))
    #if(TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_MASTER_SLAVE)

    else if( AHC_IsSDInserted() )
    #elif(TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_COMPLEMENTARY)
    else if( AHC_IsSDInserted() || AHC_Is2ndSDInserted() )
    #endif//TWOSD_WORK_MODEL

    #else//DEVICE_GPIO_2NDSD_PLUG

    else if(AHC_IsSDInserted())

    #endif//DEVICE_GPIO_2NDSD_PLUG
    {
         UINT8 bValue = 0;

         //if(VideoFunc_RecordStatus() && MMPF_MP4VENC_GetStatus()!= 0x01)
         if(VideoFunc_RecordStatus())
         {
            // AHC_SetMode(AHC_MODE_IDLE);
             AHC_GPIO_GetData(AHC_PIO_REG_LGPIO28,&bValue);
             if(bValue != AHC_FALSE)
             AHC_FS_IOCtl(AHC_UF_GetRootName(), 4, AHC_FS_CMD_RESET_MEDIUM, NULL, NULL);

             DrawVideo_UpdatebyEvent(EVENT_VIDEO_KEY_RECORD_STOP);
             //AHC_WMSG_Draw(AHC_TRUE, WMSG_FILE_ERROR, 1);
             #if (SUPPORT_ESD_RECOVER_VR)

             //AHC_FS_IOCtl("SD:\\", 4, AHC_FS_CMD_RESET_MEDIUM, NULL, NULL);
             #if 0
             if(MMPF_MP4VENC_GetStatus() == 0x01)
             {
                MMPF_VIDMGR_CloseFile();

                MMPF_MP4VENC_SetStatus(0x04);
             }
             #endif
             VideoRecMode_RecordwithStatusChecking();
             #endif
         }
         else
         {
             AHC_GPIO_GetData(AHC_PIO_REG_LGPIO28,&bValue);
             if(bValue != AHC_FALSE)
             AHC_FS_IOCtl(AHC_UF_GetRootName(), 4, AHC_FS_CMD_RESET_MEDIUM, NULL, NULL);

             AHC_RemountDevices(MenuSettingConfig()->uiMediaSelect);
             AHC_OS_Sleep(300);
             VideoRecMode_RecordwithStatusChecking();
         }
    }
    #endif
    #endif
    #endif
}

AHC_BOOL AIHC_SetPlaybackMediaErrorMode(void)
{
    UINT32                      CurrentDcfIdx;
    UINT8                       FileType;
    char                        FilePathName[MAX_FILE_NAME_SIZE];
    UINT32                      Param;
	MMP_DSC_JPEG_INFO           jpeginfo;
    UINT16                      StartX, StartY,DispWidth,DispHeight;
    MMP_ERR                     sRet = MMP_ERR_NONE;
    MMP_ULONG                   ulFileNameLen;
	AHC_DISPLAY_OUTPUTPANEL	    outputPanel;
    MMPS_AUDIO_FILEINFO         AudFileInfo;
    MMPS_VIDEO_CONTAINER_INFO   VideoFileInfo;
    MMPS_VIDPLAY_SCALERCONFIG   cfg;

    AHC_UF_GetCurrentIndex(&CurrentDcfIdx);

    MEMSET(FilePathName, 0, sizeof(FilePathName));
    AHC_UF_GetFilePathNamebyIndex(CurrentDcfIdx, FilePathName);
    AHC_UF_GetFileTypebyIndex(CurrentDcfIdx, &FileType);

    m_ulCurrentPBFileType = FileType;

    if (FileType == DCF_OBG_JPG)
    {
        STRCPY(jpeginfo.bJpegFileName, FilePathName);

        jpeginfo.usJpegFileNameLength   = STRLEN(FilePathName);
        jpeginfo.ulJpegBufAddr          = 0;
        jpeginfo.ulJpegBufSize          = 0;
        jpeginfo.bDecodeThumbnail       = MMP_FALSE;
        #if (DSC_SUPPORT_BASELINE_MP_FILE)
        jpeginfo.bDecodeLargeThumb 		= MMP_FALSE;
        #endif
        jpeginfo.bValid                 = MMP_FALSE;
		jpeginfo.bPowerOffLogo          = MMP_FALSE;

        AHC_GetDisplayWindowAttr(&StartX, &StartY, &DispWidth, &DispHeight);

        AHC_GetDisplayOutputDev(&outputPanel);
		MMPS_Display_SetOutputPanel(MMP_DISPLAY_PRM_CTL, outputPanel);

        sRet = MMPS_DSC_PlaybackJpeg(&jpeginfo);
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
        
        m_ulPlaybackFileType = FileType;
        return AHC_TRUE;
    }
    else if ((FileType == DCF_OBG_MOV) ||
             (FileType == DCF_OBG_MP4) ||
             (FileType == DCF_OBG_AVI) ||
             (FileType == DCF_OBG_3GP) ||
             (FileType == DCF_OBG_WMV))
    {
        AIHC_InitAudioExtDACOut();

        MMPS_VIDPLAY_SetMemoryMode(MMP_FALSE);

        #if (SUPPORT_SPEAKER == 1)
        AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO, AHC_TRUE);
        #endif

        AHC_GetDisplayOutputDev( &outputPanel );

        MMPS_Display_SetOutputPanelMediaError(MMP_DISPLAY_PRM_CTL, outputPanel);

        STRCPY(m_CurPlayFileName, FilePathName);
        ulFileNameLen = STRLEN(FilePathName);

        MMPS_VIDPLAY_GetFileInfo(m_CurPlayFileName, ulFileNameLen, &VideoFileInfo);
        m_ulCurrentPBHeight = VideoFileInfo.video_info[0].height;

        AHC_Display_GetWidthHdight(&DispWidth, &DispHeight);

		MMPS_VIDPLAY_SetScalerCfg(&cfg, cfg.bUseScaler, DispWidth, DispHeight, 0, 0);
        
        if ((m_ulCurrentPBHeight == 1088) && (1080 == cfg.ulOutHeight)) {
            // 1080P@60 VR v.s. 1080P output => no scaling
            cfg.ulOutWidth  = 0;
            cfg.ulOutHeight = 0;
            cfg.bUseScaler  = MMP_FALSE;
        }
        else if (cfg.ulOutHeight < m_ulCurrentPBHeight) {
            cfg.bUseScaler = MMP_TRUE;

            if (cfg.ulOutHeight > AHC_HD_VIDPLAY_MAX_HEIGHT) {
                cfg.ulOutWidth = AHC_HD_VIDPLAY_MAX_WIDTH;
                cfg.ulOutHeight = AHC_HD_VIDPLAY_MAX_HEIGHT;
            }
        }
        else {
            cfg.ulOutWidth  = 0;
            cfg.ulOutHeight = 0;
            cfg.bUseScaler  = MMP_FALSE;
        }

        AHC_GetParam(PARAM_ID_DISPLAY_ROTATE_ENABLE,&Param);

        if (Param) {
            MMPS_VIDPLAY_SetDisplayMode(VIDEOPLAY_MODE_MAX, Param, MMP_FALSE, NULL, &cfg);
        }

        // set audio volume
        AHC_GetParam(PARAM_ID_AUDIO_VOLUME_DB      ,&Param);

        #if (AUDIO_SET_DB == 0x01)
        MMPS_AUDIO_SetPlayVolumeDb(Param);
        #else
        MMPS_AUDIO_SetPlayVolume(Param, MMP_FALSE);
        #endif

        //TBD
        MMPS_Display_SetWinActive(LOWER_IMAGE_WINDOW_ID, MMP_FALSE);

        sRet = MMPS_VIDPLAY_Open(m_CurPlayFileName, ulFileNameLen, m_ulVideoPlayStartTime, MMP_TRUE, MMP_TRUE, &cfg);
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
        
        #if 0
        if (err != MMP_ERR_NONE) {

            if(err != MMP_3GPPLAY_ERR_INCORRECT_STATE_OPERATION){
                PRINTF("Close Video\r\n");
                MMPS_VIDPLAY_Close();
            }

            return AHC_FALSE;
        }
        #endif
        
        m_ulVideoPlayStartTime = 0;

        m_ulPlaybackFileType = FileType;

        m_ulVideoPlayStopStatus = AHC_VIDEO_PLAY;

        sRet = MMPS_VIDPLAY_Play((void *)VideoPlayStopCallback, NULL);
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet);} 
        
        if (sRet != MMP_ERR_NONE) {

            MMPS_VIDPLAY_Close();

            #if (SUPPORT_SPEAKER == 1)
            AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO,AHC_FALSE);
            #endif

            m_ulVideoPlayStopStatus = AHC_VIDEO_PLAY_ERROR_STOP;

            AHC_OSDSetActive(THUMBNAIL_OSD_FRONT_ID, AHC_TRUE);

            return AHC_FALSE;
        }

        AHC_GetParam(PARAM_ID_MOVIE_AUTO_PLAY,&Param);

        if (Param) {
            MMPS_VIDPLAY_Pause();
        }
    }
    else if ((FileType == DCF_OBG_MP3) ||
             (FileType == DCF_OBG_WAV) ||
             (FileType == DCF_OBG_OGG) ||
             (FileType == DCF_OBG_WMA))
    {
        AIHC_InitAudioExtDACOut();

        MMPS_AUDIO_SetMediaPath(MMPS_AUDIO_MEDIA_PATH_CARD);

        STRCPY(AudFileInfo.bFileName, FilePathName);
        AudFileInfo.usFileNameLength = STRLEN(FilePathName);

        if ( FileType == DCF_OBG_MP3 ) {
            m_uiPlayAudioFmt = AHC_AUDIO_PLAY_FMT_MP3;
            sRet = MMPS_AUDIO_OpenFile(AudFileInfo, &m_gsAHCMp3Info, MMPS_AUDIO_CODEC_MP3);
        }
        else if ( FileType == DCF_OBG_WAV ) {
            m_uiPlayAudioFmt = AHC_AUDIO_PLAY_FMT_WAVE;
            sRet = MMPS_AUDIO_OpenFile(AudFileInfo, &m_gsAHCWavInfo, MMPS_AUDIO_CODEC_WAV);
        }
        else if ( FileType == DCF_OBG_OGG ) {
            m_uiPlayAudioFmt = AHC_AUDIO_PLAY_FMT_OGG;
            sRet = MMPS_AUDIO_OpenFile(AudFileInfo, &m_gsAHCOggInfo, MMPS_AUDIO_CODEC_OGG);
        }
        else if ( FileType == DCF_OBG_WMA ) {
            m_uiPlayAudioFmt = AHC_AUDIO_PLAY_FMT_WMA;
            sRet = MMPS_AUDIO_OpenFile(AudFileInfo, &m_gsAHCWmaInfo, MMPS_AUDIO_CODEC_WMA);
        }
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
        
        AHC_GetParam(PARAM_ID_AUDIO_VOLUME, &Param);

        #if (AUDIO_SET_DB == 0x01)
        MMPS_AUDIO_SetPlayVolumeDb(Param);
        #else
        MMPS_AUDIO_SetPlayVolume(Param, MMP_FALSE);
        #endif

        m_ulAudioPlayStartTime = 0;

        m_ulPlaybackFileType = FileType;

        m_ulAudioPlayStopStatus = AHC_AUDIO_EVENT_PLAY;

        sRet = MMPS_AUDIO_StartPlay((void *)AudioPlayStopCallback, (void *)"AUDIO");
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); m_ulAudioPlayStopStatus = AHC_AUDIO_EVENT_EOF; return AHC_FALSE;} 

        m_bAHCAudioPlaying = AHC_TRUE;

        #if (SUPPORT_SPEAKER == 1)
        AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO, AHC_TRUE);
        #endif
    }
    else {
        return AHC_FALSE;
    }

    return AHC_TRUE;
}

AHC_BOOL AHC_SetPlaybackMode_MediaError(UINT32 ResumeTime)
{
    m_ulVideoPlayStartTime = ResumeTime;
    AIHC_SetPlaybackMediaErrorMode();
    return AHC_TRUE;
}
#endif

/**
 @brief Start Img / Video playback

 Start Img / Video playback

 @retval AHC_BOOL
*/
static AHC_BOOL AIHC_SetPlaybackMode(void)
{
    UINT32                      CurrentDcfIdx;
    UINT8                       FileType;
    char                        FilePathName[MAX_FILE_NAME_SIZE];
    UINT32                      Param;
	MMP_DSC_JPEG_INFO           jpeginfo;
    UINT16                      StartX, StartY,DispWidth,DispHeight;
    MMP_ERR                     sRet = MMP_ERR_NONE;
    MMP_ULONG                   ulFileNameLen;
	AHC_DISPLAY_OUTPUTPANEL	    outputPanel;
    MMPS_AUDIO_FILEINFO         AudFileInfo;
    MMPS_VIDPLAY_SCALERCONFIG   cfg;    
#if !(EN_SPEED_UP_VID & PB_CASE)    
    MMPS_VIDEO_CONTAINER_INFO   VideoFileInfo;
#endif
    
    if ((!CAM_CHECK_SCD(SCD_CAM_NONE) || !CAM_CHECK_USB(USB_CAM_NONE)) && 
        (m_ubPlaybackRearCam == AHC_TRUE) &&
        ((VIDRECD_MULTI_TRACK == 0) || (AHC_UF_GetCurrentDBFlag() & DCFDT_DB_FLAG_HAS_REAR_CAM))) {
        AHC_UF_SetRearCamPathFlag(AHC_TRUE);
    }

    AHC_UF_GetCurrentIndex(&CurrentDcfIdx);

    MEMSET(FilePathName, 0, sizeof(FilePathName)); 

    AHC_UF_GetFilePathNamebyIndex(CurrentDcfIdx, FilePathName);

    if ((!CAM_CHECK_SCD(SCD_CAM_NONE) || !CAM_CHECK_USB(USB_CAM_NONE)) && 
        (m_ubPlaybackRearCam == AHC_TRUE) &&
        ((VIDRECD_MULTI_TRACK == 0) || (AHC_UF_GetCurrentDBFlag() & DCFDT_DB_FLAG_HAS_REAR_CAM))) {
        AHC_UF_SetRearCamPathFlag(AHC_FALSE);
    }

    AHC_UF_GetFileTypebyIndex(CurrentDcfIdx, &FileType);

    m_ulCurrentPBFileType = FileType;

    if (FileType == DCF_OBG_JPG)
    {
        STRCPY(jpeginfo.bJpegFileName, FilePathName);

        jpeginfo.usJpegFileNameLength   = STRLEN(FilePathName);
        jpeginfo.ulJpegBufAddr          = 0;
        jpeginfo.ulJpegBufSize          = 0;
        jpeginfo.bDecodeThumbnail       = MMP_FALSE;
        #if (DSC_SUPPORT_BASELINE_MP_FILE)
        jpeginfo.bDecodeLargeThumb 		= MMP_FALSE;
        #endif
        jpeginfo.bValid                 = MMP_FALSE;
		jpeginfo.bPowerOffLogo          = MMP_FALSE;

        AHC_GetDisplayWindowAttr(&StartX, &StartY, &DispWidth, &DispHeight);

        AHC_GetDisplayOutputDev(&outputPanel);
		MMPS_Display_SetOutputPanel(MMP_DISPLAY_PRM_CTL, outputPanel);

		if (outputPanel == MMP_DISPLAY_SEL_NTSC_TV) {
            MMPS_DSC_SetPlaybackMode(DSC_TV_NTSC_DECODE_MODE);
        }
		else if (outputPanel == MMP_DISPLAY_SEL_PAL_TV) {
            MMPS_DSC_SetPlaybackMode(DSC_TV_PAL_DECODE_MODE);
        }
		else if (outputPanel == MMP_DISPLAY_SEL_HDMI) {
            MMPS_DSC_SetPlaybackMode(DSC_HDMI_DECODE_MODE);
        }
        #if defined(CCIR656_OUTPUT_ENABLE)&&(CCIR656_OUTPUT_ENABLE)        
        else if (outputPanel == MMP_DISPLAY_SEL_CCIR) {	
            MMPS_DSC_SetPlaybackMode(DSC_CCIR_DECODE_MODE);
        }
        #endif
        else {
            MMPS_DSC_SetPlaybackMode(DSC_NORMAL_DECODE_MODE);
        }

        sRet = MMPS_DSC_PlaybackJpeg(&jpeginfo);
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(0,sRet); return AHC_FALSE;} 
        
        m_ulPlaybackFileType = FileType;
        return AHC_TRUE;
    }
    else if ( ( FileType == DCF_OBG_MOV ) ||
              ( FileType == DCF_OBG_MP4 ) ||
              ( FileType == DCF_OBG_AVI ) ||
              ( FileType == DCF_OBG_3GP ) ||
              ( FileType == DCF_OBG_WMV ) )
    {
        AIHC_InitAudioExtDACOut();

        MMPS_VIDPLAY_SetMemoryMode(MMP_FALSE);

#if (DAC_NOT_OUTPUT_SPEAKER_HAS_NOISE)
		AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO, AHC_FALSE);
#else
        #if (SUPPORT_SPEAKER == 1)
        AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO, AHC_TRUE);
        #endif
#endif

        AHC_GetDisplayOutputDev( &outputPanel );

		MMPS_Display_SetOutputPanel(MMP_DISPLAY_PRM_CTL, outputPanel);

        #if (SUPPORT_SYS_CALIBRATION)
        if (uiGetCurrentState() == UI_SYS_CALIBRATION_STATE) {
            STRCPY(m_CurPlayFileName, "SD:\\test\\VideoRec.MOV");
            ulFileNameLen = STRLEN("SD:\\test\\VideoRec.MOV");
        }
        else
        #endif
        {
            STRCPY(m_CurPlayFileName, FilePathName);
            ulFileNameLen = STRLEN(FilePathName);
        }

        AHC_GetParam(PARAM_ID_DISPLAY_ROTATE_ENABLE,&Param);

        if (Param) {

            m_ulCurrentPBWidth = MMPS_VIDPLAY_Get3gpConTnerVidInf()->width;
            m_ulCurrentPBHeight = MMPS_VIDPLAY_Get3gpConTnerVidInf()->height; 

            AHC_Display_GetWidthHdight(&DispWidth, &DispHeight);//#if defined(CCIR656_OUTPUT_ENABLE)&&(CCIR656_OUTPUT_ENABLE)

			MMPS_VIDPLAY_SetScalerCfg(&cfg, cfg.bUseScaler, DispWidth, DispHeight, 0, 0);
            if ((m_ulCurrentPBHeight == 1088) && (1080 == cfg.ulOutHeight)) {
                // 1080P@60 VR v.s. 1080P output => no scaling
                cfg.ulOutWidth  = 0;
                cfg.ulOutHeight = 0;
                cfg.bUseScaler  = MMP_FALSE;
            }
            else if (cfg.ulOutHeight < m_ulCurrentPBHeight) {
                cfg.bUseScaler = MMP_TRUE;
                
                if (cfg.ulOutHeight > AHC_HD_VIDPLAY_MAX_HEIGHT) {
                    cfg.ulOutWidth = AHC_HD_VIDPLAY_MAX_WIDTH;
                    cfg.ulOutHeight = AHC_HD_VIDPLAY_MAX_HEIGHT;
                }
            }
            else {
                cfg.ulOutWidth  = 0;
                cfg.ulOutHeight = 0;
                cfg.bUseScaler = MMP_FALSE;
            }
            MMPS_VIDPLAY_SetDisplayMode(VIDEOPLAY_MODE_MAX, Param, MMP_FALSE, NULL, &cfg);
        }
        else//HDMI TV out
        {
            AHC_DISPLAY_OUTPUTPANEL sOutputDevice;
            AHC_GetDisplayOutputDev(&sOutputDevice);
            if(sOutputDevice == AHC_DISPLAY_NTSC_TV)
                 MMPS_VIDPLAY_SetDisplayMode(VIDEOPLAY_MODE_NTSC_TV, Param, MMP_TRUE, NULL, &cfg);
            else if(sOutputDevice == AHC_DISPLAY_PAL_TV)
                MMPS_VIDPLAY_SetDisplayMode(VIDEOPLAY_MODE_PAL_TV, Param, MMP_TRUE, NULL, &cfg);
            else if(sOutputDevice == AHC_DISPLAY_HDMI)
                MMPS_VIDPLAY_SetDisplayMode(VIDEOPLAY_MODE_HDMI, Param, MMP_TRUE, NULL, &cfg);
        }

        // set audio volume
        AHC_GetParam(PARAM_ID_AUDIO_VOLUME_DB      ,&Param);

        #if (AUDIO_SET_DB == 0x01)
        MMPS_AUDIO_SetPlayVolumeDb(Param);
        #else
        MMPS_AUDIO_SetPlayVolume(Param, MMP_FALSE);
        #endif

        //TBD
	    MMPS_Display_SetWinActive(LOWER_IMAGE_WINDOW_ID, MMP_FALSE);
	    
#if !(EN_SPEED_UP_VID & PB_CASE)
        sRet = MMPS_VIDPLAY_GetFileInfo(m_CurPlayFileName, ulFileNameLen, &VideoFileInfo);
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(0,sRet); return AHC_FALSE;} 
        
        m_ulCurrentPBHeight = VideoFileInfo.video_info[0].height;

        AHC_Display_GetWidthHdight(&DispWidth, &DispHeight);//#if defined(CCIR656_OUTPUT_ENABLE)&&(CCIR656_OUTPUT_ENABLE)

        cfg.ulOutWidth = DispWidth;
        cfg.ulOutHeight = DispHeight;
        if ((m_ulCurrentPBHeight == 1088) && (1080 == cfg.ulOutHeight)) {
            // 1080P@60 VR v.s. 1080P output => no scaling
            cfg.ulOutWidth  = 0;
            cfg.ulOutHeight = 0;
            cfg.bUseScaler  = MMP_FALSE;
        }
        else if (cfg.ulOutHeight < m_ulCurrentPBHeight) {
            cfg.bUseScaler = MMP_TRUE;
            
            if (cfg.ulOutHeight > AHC_HD_VIDPLAY_MAX_HEIGHT) {
                cfg.ulOutWidth = AHC_HD_VIDPLAY_MAX_WIDTH;
                cfg.ulOutHeight = AHC_HD_VIDPLAY_MAX_HEIGHT;
            }
        }
        else {
            cfg.ulOutWidth  = 0;
            cfg.ulOutHeight = 0;
            cfg.bUseScaler = MMP_FALSE;
        }
#else
        AHC_Display_GetWidthHdight(&DispWidth, &DispHeight);//#if defined(CCIR656_OUTPUT_ENABLE)&&(CCIR656_OUTPUT_ENABLE)
		MMPS_VIDPLAY_SetScalerCfg(&cfg, cfg.bUseScaler, DispWidth, DispHeight, AHC_HD_VIDPLAY_MAX_WIDTH, AHC_HD_VIDPLAY_MAX_HEIGHT);
#endif

		#if VIDRECD_MULTI_TRACK
        if (m_ubPlaybackRearCam == AHC_TRUE)
        {
            MMPS_VIDPLAY_SetCurrentTrack(1);  //playback rear cam.
        }
        else
        {
            MMPS_VIDPLAY_SetCurrentTrack(0);  //playback front cam.
        }
		#endif

        sRet = MMPS_VIDPLAY_Open(m_CurPlayFileName, ulFileNameLen, m_ulVideoPlayStartTime, MMP_TRUE, MMP_TRUE, &cfg);
	    DBG_AutoTestPrint(ATEST_ACT_PB_VID_0x0003, 
                            ATEST_STS_RSOL_SIZE_0x0004, 
                            m_ulCurrentPBWidth, 
                            m_ulCurrentPBHeight, 
							gbAhcDbgBrk);         
#if (EN_SPEED_UP_VID & PB_CASE)		
        m_ulCurrentPBWidth = MMPS_VIDPLAY_Get3gpConTnerVidInf()->width;
        m_ulCurrentPBHeight = MMPS_VIDPLAY_Get3gpConTnerVidInf()->height; 
#endif		
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(0,sRet); return AHC_FALSE;} 

        #if 0
        if (err != MMP_ERR_NONE) {

            if(err != MMP_3GPPLAY_ERR_INCORRECT_STATE_OPERATION){
                PRINTF("Close Video\r\n");
                MMPS_VIDPLAY_Close();
            }

            return AHC_FALSE;
        }
        #endif

        m_ulVideoPlayStartTime = 0;

        m_ulPlaybackFileType = FileType;

        m_ulVideoPlayStopStatus = AHC_VIDEO_PLAY;

        sRet = MMPS_VIDPLAY_Play((void *)VideoPlayStopCallback, NULL);
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet);} 

        if (sRet != MMP_ERR_NONE) {

            MMPS_VIDPLAY_Close();

            AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO, AHC_FALSE);

            m_ulVideoPlayStopStatus = AHC_VIDEO_PLAY_ERROR_STOP;

            AHC_OSDSetActive(THUMBNAIL_OSD_FRONT_ID, AHC_TRUE);

            return AHC_FALSE;
        }

#if (DAC_NOT_OUTPUT_SPEAKER_HAS_NOISE)
        #if (SUPPORT_SPEAKER == 1)
        AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO, AHC_TRUE);
        #endif
#endif

        AHC_GetParam(PARAM_ID_MOVIE_AUTO_PLAY,&Param);

        if(Param) {
            MMPS_VIDPLAY_Pause();
        }
    }
    else if ( ( FileType == DCF_OBG_MP3 ) ||
              ( FileType == DCF_OBG_WAV ) ||
              ( FileType == DCF_OBG_OGG ) ||
              ( FileType == DCF_OBG_WMA ) )
    {
        AIHC_InitAudioExtDACOut();

        MMPS_AUDIO_SetMediaPath(MMPS_AUDIO_MEDIA_PATH_CARD);

        STRCPY(AudFileInfo.bFileName, FilePathName);
        AudFileInfo.usFileNameLength = STRLEN(FilePathName);

        if ( FileType == DCF_OBG_MP3 ) {
            m_uiPlayAudioFmt = AHC_AUDIO_PLAY_FMT_MP3;
            sRet = MMPS_AUDIO_OpenFile(AudFileInfo, &m_gsAHCMp3Info, MMPS_AUDIO_CODEC_MP3);
        }
        else if ( FileType == DCF_OBG_WAV ) {
            m_uiPlayAudioFmt = AHC_AUDIO_PLAY_FMT_WAVE;
            sRet = MMPS_AUDIO_OpenFile(AudFileInfo, &m_gsAHCWavInfo, MMPS_AUDIO_CODEC_WAV);
        }
        else if ( FileType == DCF_OBG_OGG ) {
            m_uiPlayAudioFmt = AHC_AUDIO_PLAY_FMT_OGG;
            sRet = MMPS_AUDIO_OpenFile(AudFileInfo, &m_gsAHCOggInfo, MMPS_AUDIO_CODEC_OGG);
        }
        else if ( FileType == DCF_OBG_WMA ) {
            m_uiPlayAudioFmt = AHC_AUDIO_PLAY_FMT_WMA;
            sRet = MMPS_AUDIO_OpenFile(AudFileInfo, &m_gsAHCWmaInfo, MMPS_AUDIO_CODEC_WMA);
        }
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

        AHC_GetParam(PARAM_ID_AUDIO_VOLUME, &Param);

        #if (AUDIO_SET_DB == 0x01)
        MMPS_AUDIO_SetPlayVolumeDb(Param);
        #else
        MMPS_AUDIO_SetPlayVolume(Param, MMP_FALSE);
        #endif

        m_ulAudioPlayStartTime = 0;

        m_ulPlaybackFileType = FileType;

        m_ulAudioPlayStopStatus = AHC_AUDIO_EVENT_PLAY;

        sRet = MMPS_AUDIO_StartPlay((void *)AudioPlayStopCallback, (void *)"AUDIO");
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); m_ulAudioPlayStopStatus = AHC_AUDIO_EVENT_EOF; return AHC_FALSE;} 

        m_bAHCAudioPlaying = AHC_TRUE;

        #if (SUPPORT_SPEAKER == 1)        
        AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO, AHC_TRUE);
        #endif
    }
    else {
        //TBD
        return AHC_FALSE;
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AIHC_StopPlaybackMode
//  Description : Stop Image / Video playback
//------------------------------------------------------------------------------
AHC_BOOL AIHC_StopPlaybackMode(void)
{
    MMP_M_STATE     ubVideoState;
    MMP_ULONG       KeyFrameNum;
    MMP_ERR         sRet = MMP_ERR_NONE;

    if ((m_ulPlaybackFileType == DCF_OBG_MOV) ||
        (m_ulPlaybackFileType == DCF_OBG_MP4) ||
        (m_ulPlaybackFileType == DCF_OBG_AVI) ||
        (m_ulPlaybackFileType == DCF_OBG_3GP) ||
        (m_ulPlaybackFileType == DCF_OBG_WMV)) {

        MMPS_VIDPLAY_GetState(&ubVideoState);

        if (ubVideoState != MMP_M_STATE_IDLE) {
            MMPS_VIDPLAY_Stop(&KeyFrameNum);
        }

        sRet = MMPS_VIDPLAY_Close();
        
        DBG_AutoTestPrint(ATEST_ACT_PB_VID_0x0003, ATEST_STS_END_0x0003, 0, sRet, gbAhcDbgBrk);

        #if (SUPPORT_SPEAKER == 1)
        AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO,AHC_FALSE);
        #endif
    }
    else if ((m_ulPlaybackFileType == DCF_OBG_MP3) ||
             (m_ulPlaybackFileType == DCF_OBG_WAV) ||
             (m_ulPlaybackFileType == DCF_OBG_OGG) ||
             (m_ulPlaybackFileType == DCF_OBG_WMA)) {

        MMPS_AUDIO_StopPlay();
        
        #if (SUPPORT_SPEAKER == 1)
        AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO, AHC_FALSE);
        #endif
        
        m_bAHCAudioPlaying = AHC_FALSE;
    }
    else if (m_ulPlaybackFileType == DCF_OBG_JPG) {
        MMPS_DSC_ExitJpegPlayback();
    }

    return AHC_TRUE;
}

#if 0
void _____UI_Mode_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AIHC_InitGeneralFunc
//  Description :
//------------------------------------------------------------------------------
static AHC_BOOL AIHC_InitGeneralFunc(void)
{
    MEMSET(m_MessageQueue, 0, sizeof(m_MessageQueue));
    
    m_MessageQueueIndex_W = 0;
    m_MessageQueueIndex_R = 0;

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_Initialize
//  Description :
//------------------------------------------------------------------------------
/**
@brief The intialize code that should be called once after system power-up

 The intialize code that should be called once after system power-up
 Parameters:
 @retval TRUE or FALSE. // TRUE: Success, FALSE: Fail
*/
AHC_BOOL AHC_Initialize(void)
{
    if (!m_bAHCGeneralInit)
    {
        AHC_PrintBuildTime();
        AHC_PrintFwVersion();
        
        AIHC_PARAM_Initialize();
        
        AIHC_InitGeneralFunc();
        AIHC_InitAHLMessage();
        AIHC_InitAHLMessage_HP();
        
        AHC_WMSG_Config();
        
        MMPS_VIDPLAY_InitConfig(MMPS_VIDPLAY_GetConfig());
        
        m_bAHCGeneralInit = AHC_TRUE;
    }

    // Note: Please keep below function call to make sure functions in AHC_Callback.c can work
    AHC_CheckCallbackExisted();

    #if (FS_FORMAT_FREE_ENABLE)
    MMPS_FORMATFREE_Enable(1);
    #endif

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_IDLE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_IDLE(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    
    LOCK_AHC_MODE();
    
    if (uiMode == AHC_MODE_CAPTURE_PREVIEW) {
        if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
            MMPS_3GPRECD_SetDualBayerSnrCaptureMode(MMP_TRUE);
            ahcRet = AHC_VIDEO_SetRecordMode(AHC_VIDRECD_INIT); // TBD
        }
        else {
            AIHC_SetCapturePreviewMode(AHC_FALSE, AHC_FALSE);
        }
        AHC_SetAhcSysMode(AHC_MODE_CAPTURE_PREVIEW);
    }
    else if (uiMode == AHC_MODE_USB_MASS_STORAGE) {
        ahcRet = AHC_SetUsbMode(AHC_USB_MODE_MASS_STORAGE);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); UNLOCK_AHC_MODE(); return ahcRet;}
        
        AHC_SetAhcSysMode(uiMode);
    }
    #if(SUPPORT_UVC_FUNC)
    else if (uiMode == AHC_MODE_USB_WEBCAM) {
        extern MMP_USHORT pcam_usb_CustomedPreviewAttr(MMP_BOOL bUserConfig, MMP_UBYTE ubPrevSnrMode);

        AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_DEFAULT_PCCAM_RESOLUTION);
        
        pcam_usb_CustomedPreviewAttr(MMP_TRUE, AHC_SNR_GetPresetSnrMode());

        ahcRet = AHC_SetUsbMode(AHC_USB_MODE_WEBCAM);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); UNLOCK_AHC_MODE(); return ahcRet;}
        
        AHC_SetAhcSysMode(uiMode);
    }
    #endif
    else if (uiMode == AHC_MODE_PLAYBACK) {
        ahcRet = AIHC_SetPlaybackMode();
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); UNLOCK_AHC_MODE(); return ahcRet;}

        AHC_SetAhcSysMode(uiMode);
    }
    else if (uiMode == AHC_MODE_THUMB_BROWSER) {
        MMPS_Display_SetWinActive(LOWER_IMAGE_WINDOW_ID, MMP_FALSE);

        AHC_Thumb_DrawPage(AHC_TRUE);

        AHC_SetAhcSysMode(uiMode);
    }
    else if (uiMode == AHC_MODE_RECORD_PREVIEW) {
        if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
            MMPS_3GPRECD_SetDualBayerSnrCaptureMode(MMP_FALSE); // TBD
        }
        ahcRet = AHC_VIDEO_SetRecordMode(AHC_VIDRECD_INIT);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); UNLOCK_AHC_MODE(); return ahcRet;}

        AHC_SetAhcSysMode(uiMode);
    }
    else if (uiMode == AHC_MODE_RAW_PREVIEW) {
        AIHC_SetCapturePreviewMode(AHC_TRUE, AHC_FALSE);
        AHC_SetAhcSysMode(uiMode);
    }
    else if (uiMode == AHC_MODE_NET_PLAYBACK) {
        AHC_SetAhcSysMode(uiMode);
    }
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_IDLE\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }

    UNLOCK_AHC_MODE();
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_CAPTURE_PREVIEW
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_CAPTURE_PREVIEW(AHC_MODE_ID uiMode)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    MMP_ERR     err;

    LOCK_AHC_MODE();
    
    if (uiMode == AHC_MODE_IDLE) {
    
        #if (defined(WIFI_PORT) && WIFI_PORT == 1)
        if (AHC_GetStreamingMode() != AHC_STREAM_OFF) {
            UNLOCK_AHC_MODE();
            AHC_SetStreamingMode(AHC_STREAM_OFF);
            LOCK_AHC_MODE();
        }
        #endif
        
        if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
            // Use video mode still capture as DSC mode
            AHC_VIDEO_SetRecordMode(AHC_VIDRECD_IDLE);
        }
        else {
            AIHC_StopCapturePreviewMode();
        }
        AHC_SetAhcSysMode(AHC_MODE_IDLE);
    }
    else if (uiMode == AHC_MODE_C2C_CAPTURE) {
        if (AIHC_SetCaptureMode(uiMode) == AHC_TRUE) {
            AHC_SetAhcSysMode(uiMode);
        }
        else {
            DBG_AutoTestPrint(ATEST_ACT_CAPTURE_0x0005, ATEST_STS_END_0x0003, 0, AHC_TRUE, gbAhcDbgBrk);
            UNLOCK_AHC_MODE();
            return AHC_FALSE;
        }
    }
    else if ((uiMode == AHC_MODE_DRAFT_CAPTURE) ||
             (uiMode == AHC_MODE_STILL_CAPTURE) ||
             (uiMode == AHC_MODE_SEQUENTIAL_CAPTURE)) {

        if (AIHC_SetCaptureMode(uiMode) == AHC_TRUE) {
        
            AHC_SetAhcSysMode(uiMode);

            #if defined(CCIR656_OUTPUT_ENABLE) && (CCIR656_OUTPUT_ENABLE)
            printc("## TODO : MMPS_DSC_EnablePreviewDisplay,FOR DEBUG\r\n");
            MMPS_DSC_EnablePreviewDisplay(gsAhcPrmSensor, MMP_FALSE, MMP_TRUE);
            #endif

            UNLOCK_AHC_MODE();
            
            err = AHC_SetMode(AHC_MODE_CAPTURE_PREVIEW);
            
            LOCK_AHC_MODE();
            
            if (err == AHC_FALSE) {
                UNLOCK_AHC_MODE();
                return AHC_FALSE;
            }
        }
        else {
            DBG_AutoTestPrint(ATEST_ACT_CAPTURE_0x0005, ATEST_STS_END_0x0003, 0, AHC_TRUE, gbAhcDbgBrk);            
        }
    }
    else if (uiMode == AHC_MODE_LONG_TIME_CONTINUOUS_FIRST_CAPTURE) {
        if (AIHC_SetCaptureMode(uiMode) == AHC_TRUE) {
            AHC_SetAhcSysMode(uiMode);
        }
        else {
            DBG_AutoTestPrint(ATEST_ACT_CAPTURE_0x0005, ATEST_STS_END_0x0003, 0, AHC_TRUE, gbAhcDbgBrk);
        }
    }
    else if (uiMode == AHC_MODE_CONTINUOUS_CAPTURE) {
        if (AIHC_SetCaptureMode(uiMode) == AHC_TRUE) {
            AHC_SetAhcSysMode(uiMode);
        }
        else {
            DBG_AutoTestPrint(ATEST_ACT_CAPTURE_0x0005, ATEST_STS_END_0x0003, 0, AHC_TRUE, gbAhcDbgBrk);            
        }
    }
    else {
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }
    
    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_DRAFT_CAPTURE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_DRAFT_CAPTURE(AHC_MODE_ID uiMode)
{
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_STILL_CAPTURE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_STILL_CAPTURE(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    LOCK_AHC_MODE();

    if (uiMode == AHC_MODE_CAPTURE_PREVIEW) {
        AIHC_SetCapturePreviewMode(AHC_FALSE, AHC_FALSE);

        AHC_SetAhcSysMode(AHC_MODE_CAPTURE_PREVIEW);
    }
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_STILL_CAPTURE\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }
    
    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_C2C_CAPTURE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_C2C_CAPTURE(AHC_MODE_ID uiMode)
{
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_SEQUENTIAL_CAPTURE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_SEQUENTIAL_CAPTURE(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    LOCK_AHC_MODE();
    
    if (uiMode == AHC_MODE_CAPTURE_PREVIEW) {
        AIHC_SetCapturePreviewMode(AHC_FALSE, AHC_TRUE);

        AHC_SetAhcSysMode(AHC_MODE_CAPTURE_PREVIEW);
    }
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_SEQUENTIAL_CAPTURE\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }

    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_LONG_TIME_CONTINUOUS_FIRST_CAPTURE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_LONG_TIME_CONTINUOUS_FIRST_CAPTURE(AHC_MODE_ID uiMode)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    MMP_ERR     err;

    LOCK_AHC_MODE();
    
    if (uiMode == AHC_MODE_LONG_TIME_CONTINUOUS_NEXT_CAPTURE) {
        if (AIHC_SetCaptureMode(uiMode) == AHC_TRUE) {
            AHC_SetAhcSysMode(AHC_MODE_LONG_TIME_CONTINUOUS_FIRST_CAPTURE);
        }
        else {
            DBG_AutoTestPrint(ATEST_ACT_CAPTURE_0x0005, ATEST_STS_END_0x0003, 0, AHC_TRUE, gbAhcDbgBrk);            
        }
    }
    else if (uiMode == AHC_MODE_LONG_TIME_CONTINUOUS_LAST_CAPTURE) {
        if (AIHC_SetCaptureMode(AHC_MODE_LONG_TIME_CONTINUOUS_NEXT_CAPTURE) == AHC_TRUE ) {
        
            AHC_SetAhcSysMode(AHC_MODE_LONG_TIME_CONTINUOUS_LAST_CAPTURE);
        
            UNLOCK_AHC_MODE();
            err = AHC_SetMode(AHC_MODE_CAPTURE_PREVIEW);
            LOCK_AHC_MODE();
        
            if (err == AHC_FALSE) {
                UNLOCK_AHC_MODE();
                return AHC_FALSE;
            }
        }
    }
    
    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_LONG_TIME_CONTINUOUS_NEXT_CAPTURE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_LONG_TIME_CONTINUOUS_NEXT_CAPTURE(AHC_MODE_ID uiMode)
{  
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_LONG_TIME_CONTINUOUS_LAST_CAPTURE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_LONG_TIME_CONTINUOUS_LAST_CAPTURE(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    LOCK_AHC_MODE();
    
    if (uiMode == AHC_MODE_CAPTURE_PREVIEW) {
        AIHC_SetCapturePreviewMode(AHC_FALSE, AHC_TRUE);
        AHC_SetAhcSysMode(AHC_MODE_CAPTURE_PREVIEW);
    }
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_STILL_CAPTURE\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }

    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_CONTINUOUS_CAPTURE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_CONTINUOUS_CAPTURE(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    LOCK_AHC_MODE();

    if (uiMode == AHC_MODE_CAPTURE_PREVIEW) {
        AIHC_SetCapturePreviewMode(AHC_FALSE, AHC_TRUE);
        AHC_SetAhcSysMode(uiMode);
    }

    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_USB_MASS_STORAGE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_USB_MASS_STORAGE(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    LOCK_AHC_MODE();
    
    if (uiMode == AHC_MODE_IDLE) {
        AHC_DisconnectDevice();
        AHC_SetAhcSysMode(AHC_MODE_IDLE);
    }
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_USB_MASS_STORAGE\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }

    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_USB_WEBCAM
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_USB_WEBCAM(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    LOCK_AHC_MODE();

    if (uiMode == AHC_MODE_IDLE) {
        AHC_DisconnectDevice();
        AHC_SetAhcSysMode(AHC_MODE_IDLE);
    }
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_USB_WEBCAM\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }
    
    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_PLAYBACK
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_PLAYBACK(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    LOCK_AHC_MODE();
    
    if (uiMode == AHC_MODE_IDLE) {
        AIHC_StopPlaybackMode();
        AHC_SetAhcSysMode(AHC_MODE_IDLE);
    }
    else if (uiMode == AHC_MODE_PLAYBACK) { // CHECK : The mode is the same.
        AIHC_StopPlaybackMode();

        if (AIHC_SetPlaybackMode() == AHC_FALSE) {
            UNLOCK_AHC_MODE();
            return AHC_FALSE;
        }
    }
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_PLAYBACK\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }

    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_THUMB_BROWSER
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_THUMB_BROWSER(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    LOCK_AHC_MODE();

    if (uiMode == AHC_MODE_PLAYBACK) {
        if (AIHC_SetPlaybackMode() == AHC_TRUE) {
            AHC_SetAhcSysMode(AHC_MODE_PLAYBACK);
        }
        else {
            // Turn off PIP window, to prevent customers redraw GUI without setting PIP properly
            MMPS_Display_SetWinActive(LOWER_IMAGE_WINDOW_ID, MMP_FALSE);
        }
    }
    else if (uiMode == AHC_MODE_IDLE) {

        AIHC_DrawReservedOSD(AHC_TRUE);
        AHC_OSDSetActive(THUMBNAIL_OSD_FRONT_ID, AHC_FALSE);
        AIHC_DrawReservedOSD(AHC_FALSE);
        
        // CHECK
        {
            MMP_IBC_PIPEID sIBCPipe = MMP_IBC_PIPE_0;

            for (sIBCPipe = MMP_IBC_PIPE_0; sIBCPipe < MMP_IBC_PIPE_MAX; sIBCPipe++) {
                MMPD_IBC_SetStoreEnable(sIBCPipe, MMP_FALSE);
            }
        }
        AHC_SetAhcSysMode(AHC_MODE_IDLE);
    }
    else if (uiMode == AHC_MODE_THUMB_BROWSER) { // The mode is the same.
        AHC_Thumb_DrawPage(AHC_TRUE);
    }
    
    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_VIDEO_RECORD
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_VIDEO_RECORD(AHC_MODE_ID uiMode)
{
    AHC_BOOL        ahcRet = AHC_TRUE;

    LOCK_AHC_MODE();
    
    if (uiMode == AHC_MODE_IDLE) {
    
        #if (defined(WIFI_PORT) && WIFI_PORT == 1)
        if (AHC_STREAM_H264 == AHC_GetStreamingMode()) {
            UNLOCK_AHC_MODE();
            return AHC_FALSE;
        }
        #endif

        #if (defined(WIFI_PORT) && WIFI_PORT == 1) 
        ahcRet = AHC_VIDEO_SetRecordMode(AHC_VIDRECD_STOP);
        #else
        ahcRet = AHC_VIDEO_SetRecordMode(AHC_VIDRECD_STOP);
        ahcRet = AHC_VIDEO_SetRecordMode(AHC_VIDRECD_IDLE);
        #endif
        
        AHC_SetAhcSysMode(uiMode);
    }
    else if (uiMode == AHC_MODE_RECORD_PREVIEW) {
    
        #if (defined(WIFI_PORT) && WIFI_PORT == 1) 
        ahcRet = AHC_IsRecorderControllable(uiMode);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); UNLOCK_AHC_MODE(); return ahcRet;}
        #endif

        ahcRet = AHC_VIDEO_SetRecordMode(AHC_VIDRECD_STOP);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); UNLOCK_AHC_MODE(); return ahcRet;}          
        
        AHC_SetAhcSysMode(uiMode);        
    }
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_VIDEO_RECORD\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }

    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_RECORD_PREVIEW
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_RECORD_PREVIEW(AHC_MODE_ID uiMode)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
	
    LOCK_AHC_MODE();
    
    if (uiMode == AHC_MODE_IDLE) {
    
        #if (defined(WIFI_PORT) && WIFI_PORT == 1)                
        if (AHC_STREAM_H264 == (AHC_GetStreamingMode() & AHC_STREAM_V_MASK)) {
            UNLOCK_AHC_MODE();
            return AHC_FALSE;
        }
        else if (AHC_GetStreamingMode() != AHC_STREAM_OFF) {
            UNLOCK_AHC_MODE();
            AHC_SetStreamingMode(AHC_STREAM_OFF);
            LOCK_AHC_MODE();
        }
        #endif

        // Turn off the preview, == AHC_VIDEO_StopRecordModeEx(AHC_FALSE, AHC_TRUE, AHC_TRUE);
        AHC_VIDEO_SetRecordMode(AHC_VIDRECD_IDLE);

        AHC_SetAhcSysMode(AHC_MODE_IDLE);
    }
    else if (uiMode == AHC_MODE_USB_WEBCAM) {
        ahcRet = AHC_SetUsbMode(AHC_USB_MODE_WEBCAM);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); UNLOCK_AHC_MODE(); return ahcRet;}
        
        AHC_SetAhcSysMode(AHC_MODE_USB_WEBCAM);
    }
    else if (uiMode == AHC_MODE_RECORD_PRERECD){       
        ahcRet = AHC_VIDEO_SetRecordMode(AHC_VIDRECD_PRERECD);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); UNLOCK_AHC_MODE(); return ahcRet;}
        
        AHC_SetAhcSysMode(uiMode);
    }
    else if(uiMode == AHC_MODE_VIDEO_RECORD){ 
        #if (defined(WIFI_PORT) && WIFI_PORT == 1) 
        ahcRet = AHC_IsRecorderControllable(uiMode);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); UNLOCK_AHC_MODE(); return ahcRet;}
        #endif

        ahcRet = AHC_VIDEO_SetRecordMode(AHC_VIDRECD_START);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); UNLOCK_AHC_MODE(); return ahcRet;}
        
        AHC_SetAhcSysMode(uiMode);
    }
    else {
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0);        
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }

    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_RECORD_PRERECD
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_RECORD_PRERECD(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;
	
    if(uiMode == AHC_MODE_VIDEO_RECORD){
        ahcRet = AHC_SetMode_RECORD_PREVIEW(uiMode);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); return ahcRet;}        
    }        
    else if ((uiMode == AHC_MODE_IDLE) || (uiMode == AHC_MODE_RECORD_PREVIEW)) {
        ahcRet = AHC_SetMode_VIDEO_RECORD(uiMode);
        if(ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); return ahcRet;}        
    }
    else {
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0);        
        return AHC_FALSE;
    }
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_RECORD_PREVIEW
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_RAW_PREVIEW(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    
    LOCK_AHC_MODE();

    if (uiMode == AHC_MODE_RAW_CAPTURE) {
        if (AIHC_SetCaptureMode(uiMode)) {
            // NOP
        }
        else {
            DBG_AutoTestPrint(ATEST_ACT_CAPTURE_0x0005, ATEST_STS_END_0x0003, 0, AHC_TRUE, gbAhcDbgBrk);            
        }

        AHC_SetAhcSysMode(AHC_MODE_RAW_CAPTURE);
        UNLOCK_AHC_MODE();
        AHC_SetMode(AHC_MODE_RAW_PREVIEW);
        LOCK_AHC_MODE();
    }
    else if (uiMode == AHC_MODE_IDLE) {
        AIHC_StopCapturePreviewMode();
        AHC_SetAhcSysMode(AHC_MODE_IDLE);
    }
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_RAW_CAPTURE\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }
    
    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_RAW_CAPTURE
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_RAW_CAPTURE(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    
    LOCK_AHC_MODE();
    
    if (uiMode == AHC_MODE_RAW_PREVIEW) {
        AIHC_SetCapturePreviewMode(AHC_TRUE, AHC_FALSE);
        AHC_SetAhcSysMode(AHC_MODE_RAW_PREVIEW);
    }
    else if (uiMode == AHC_MODE_IDLE) {
        AHC_SetAhcSysMode(AHC_MODE_IDLE);
    }
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_RAW_PREVIEW\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }
    
    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_NET_PLAYBACK
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_NET_PLAYBACK(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    
    LOCK_AHC_MODE();

    if (uiMode == AHC_MODE_IDLE) {
        #if (defined(WIFI_PORT) && WIFI_PORT == 1) 
        ncam_stop_transcoding();
        #endif
        AHC_SetAhcSysMode(AHC_MODE_IDLE);
    } 
    else {
        RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL, FG_RED("AHC_ERROR : Set Mode Error @AHC_MODE_NET_PLAYBACK\r\n"));
        UNLOCK_AHC_MODE();
        return AHC_FALSE;
    }

    UNLOCK_AHC_MODE();
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode_CALIBRATION
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetMode_CALIBRATION(AHC_MODE_ID uiMode)
{    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetMode
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set system mode

 Set system to assigned mode.
 Parameters:
 @param[in] uiMode AHC_MODE_IDLE, AHC_MODE_CAPTURE_PREVIEW...etc
 @retval TRUE or FALSE.
*/
AHC_BOOL AHC_SetMode(AHC_MODE_ID uiMode)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    printc("========== %s,old:0x%X,new:0x%X ========== \r\n", __func__, AHC_GetAhcSysMode(), uiMode);
   	
    if (m_AhcModeSemID == 0xFF) {
        m_AhcModeSemID = AHC_OS_CreateSem(1);
    }

    switch(AHC_GetAhcSysMode()){

    case AHC_MODE_IDLE:
        ahcRet = AHC_SetMode_IDLE(uiMode);
        break;
    case AHC_MODE_CAPTURE_PREVIEW:
        ahcRet = AHC_SetMode_CAPTURE_PREVIEW(uiMode);
        break;
    case AHC_MODE_DRAFT_CAPTURE:
        ahcRet = AHC_SetMode_DRAFT_CAPTURE(uiMode);
        break;
    case AHC_MODE_STILL_CAPTURE:
        ahcRet = AHC_SetMode_STILL_CAPTURE(uiMode);
        break;
    case AHC_MODE_C2C_CAPTURE:
        ahcRet = AHC_SetMode_C2C_CAPTURE(uiMode);
        break;
    case AHC_MODE_SEQUENTIAL_CAPTURE:
        ahcRet = AHC_SetMode_SEQUENTIAL_CAPTURE(uiMode);
        break;
    case AHC_MODE_LONG_TIME_CONTINUOUS_FIRST_CAPTURE:
        ahcRet = AHC_SetMode_LONG_TIME_CONTINUOUS_FIRST_CAPTURE(uiMode);
        break;
    case AHC_MODE_LONG_TIME_CONTINUOUS_NEXT_CAPTURE:
        ahcRet = AHC_SetMode_LONG_TIME_CONTINUOUS_NEXT_CAPTURE(uiMode);
        break;
    case AHC_MODE_LONG_TIME_CONTINUOUS_LAST_CAPTURE:
        ahcRet = AHC_SetMode_LONG_TIME_CONTINUOUS_LAST_CAPTURE(uiMode);
        break; 
    case AHC_MODE_CONTINUOUS_CAPTURE:
        ahcRet = AHC_SetMode_CONTINUOUS_CAPTURE(uiMode);
        break; 
    case AHC_MODE_USB_MASS_STORAGE:
        ahcRet = AHC_SetMode_USB_MASS_STORAGE(uiMode);
        break;  
    case AHC_MODE_USB_WEBCAM:
        ahcRet = AHC_SetMode_USB_WEBCAM(uiMode);
        break;
    case AHC_MODE_PLAYBACK:
        ahcRet = AHC_SetMode_PLAYBACK(uiMode);
        break;
    case AHC_MODE_THUMB_BROWSER:
        ahcRet = AHC_SetMode_THUMB_BROWSER(uiMode);
        break;
    case AHC_MODE_VIDEO_RECORD:
        ahcRet = AHC_SetMode_VIDEO_RECORD(uiMode);
        break;
    case AHC_MODE_RECORD_PREVIEW:
        ahcRet = AHC_SetMode_RECORD_PREVIEW(uiMode);
        break;
    case AHC_MODE_RECORD_PRERECD:
        ahcRet = AHC_SetMode_RECORD_PRERECD(uiMode);           
        break;               
    case AHC_MODE_RAW_PREVIEW:
        ahcRet = AHC_SetMode_RAW_PREVIEW(uiMode);
        break;
    case AHC_MODE_RAW_CAPTURE:
        ahcRet = AHC_SetMode_RAW_CAPTURE(uiMode);
        break;
    case AHC_MODE_NET_PLAYBACK:
        ahcRet = AHC_SetMode_NET_PLAYBACK(uiMode);
        break;
    case AHC_MODE_CALIBRATION:
        ahcRet = AHC_SetMode_CALIBRATION(uiMode);
        break;
    default:
        printc("This AHC Mode(%d) is not ready yet!\r\n", AHC_GetAhcSysMode());
        break;
    }

    if (ahcRet != AHC_TRUE) {
        RTNA_DBG_Str0(FG_RED("AHC_SetMode ERROR!\r\n"));
        return AHC_FALSE;
    }

    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetAhcSysMode
//  Description :
//------------------------------------------------------------------------------
AHC_MODE_ID AHC_GetAhcSysMode(void)
{
    #if (EN_AUTO_TEST_LOG == 1)
    if (gbAhcDbgBrk) {
        MMPD_System_IsAutoTestCmdLFull(m_AhcSystemMode);
    }
    #endif
        
    return m_AhcSystemMode;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAhcSysMode
//  Description :
//------------------------------------------------------------------------------
void AHC_SetAhcSysMode(AHC_MODE_ID uiMode)
{
    if (uiMode == AHC_MODE_IDLE) {
        #if (EN_AUTO_TEST_LOG == 1)
        if (gbAhcDbgBrk) {
            MMPD_System_IsAutoTestCmdLFull(m_AhcSystemMode);
        }
        #endif
        
        MMPD_Icon_InitLinkSrc();
    }
    
    m_AhcSystemMode = uiMode;
}

//------------------------------------------------------------------------------
//  Function    : AHC_IsInVideoMode
//  Description :
//------------------------------------------------------------------------------
int AHC_IsInVideoMode(void)
{
    return m_AhcSystemMode == AHC_MODE_VIDEO_RECORD ||
            m_AhcSystemMode == AHC_MODE_RECORD_PRERECD ||
           m_AhcSystemMode == AHC_MODE_RECORD_PREVIEW;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetSystemStatus
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get system status

 Get current system status from the information keeper.
 Parameters:
 @param[out] *pwValue Get status result.
 @retval TRUE or FALSE.
*/
AHC_BOOL AHC_GetSystemStatus(UINT32 *pwValue)
{
    MMP_ULONG fw_status = 0;

    *pwValue = (m_AhcSystemMode << 16) & 0xFFFF0000;

    switch(m_AhcSystemMode) {
        case AHC_MODE_IDLE:
            *pwValue |= (AHC_SYS_IDLE & 0xFFFF);
            break;
        case AHC_MODE_CAPTURE_PREVIEW:
            *pwValue |= (AHC_SYS_IDLE & 0xFFFF);
            break;
        case AHC_MODE_DRAFT_CAPTURE:
            *pwValue |= (AHC_SYS_IDLE & 0xFFFF);
            break;
        case AHC_MODE_STILL_CAPTURE:
            *pwValue |= (AHC_SYS_IDLE & 0xFFFF);
            break;
        case AHC_MODE_C2C_CAPTURE:
            *pwValue |= (AHC_SYS_IDLE & 0xFFFF);
            break;
        case AHC_MODE_SEQUENTIAL_CAPTURE:
            *pwValue |= (AHC_SYS_IDLE & 0xFFFF);
            break;
        case AHC_MODE_USB_MASS_STORAGE:
            fw_status = MMPS_USB_GetStatus();
            *pwValue |= (AHC_SYS_USB_STATUS + fw_status) & 0xFFFF;
            break;
        case AHC_MODE_USB_WEBCAM:
            *pwValue |= (AHC_SYS_IDLE & 0xFFFF);
            break;
        case AHC_MODE_PLAYBACK:
            if (m_ulCurrentPBFileType == DCF_OBG_JPG) 
            {
                *pwValue |= (AHC_SYS_IDLE & 0xFFFF);
            }
            else if ((m_ulCurrentPBFileType == DCF_OBG_MP3) ||
                     (m_ulCurrentPBFileType == DCF_OBG_WAV) ||
                     (m_ulCurrentPBFileType == DCF_OBG_OGG) ||
                     (m_ulCurrentPBFileType == DCF_OBG_WMA))
            {
                MMPS_AUDIO_GetPlayStatus((MMPS_AUDIO_PLAY_STATUS*)&fw_status);
                *pwValue |= (AHC_SYS_AUDPLAY_STATUS + (fw_status>>8)) & 0xFFFF;
            }
            else
            {
                MMPS_VIDPLAY_GetState((MMP_M_STATE*)&fw_status);
                *pwValue |= (AHC_SYS_VIDPLAY_INVALID + fw_status) & 0xFFFF;
            }
            break;
        case AHC_MODE_THUMB_BROWSER:
            *pwValue |= (AHC_SYS_IDLE & 0xFFFF);
            break;
        case AHC_MODE_VIDEO_RECORD:
            MMPS_3GPRECD_GetRecordStatus((VIDENC_FW_STATUS *)&fw_status);
            *pwValue |= (AHC_SYS_VIDRECD_STATUS + fw_status) & 0xFFFF;
            break;
        case AHC_MODE_RECORD_PREVIEW:
            MMPS_3GPRECD_GetPreviewPipeStatus(gsAhcPrmSensor, (MMP_BOOL*)&fw_status);
            if (fw_status == MMP_TRUE) {
                *pwValue |= AHC_SYS_VIDRECD_PREVIEW_NORMAL & 0xFFFF;
            }
            else {
                *pwValue |= AHC_SYS_VIDRECD_PREVIEW_ABNORMAL & 0xFFFF;
            }
            break;
        case AHC_MODE_CALIBRATION:
            *pwValue |= (AHC_SYS_IDLE & 0xFFFF);
            break;
    }

    return AHC_TRUE;
}

#if 0
void _____MessageQ_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_SendAHLMessageEnable
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SendAHLMessageEnable(AHC_BOOL Enable)
{
    m_bSendAHLMessage = Enable;
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SendAHLMessage
//  Description : This function sends an AHL message.
//  Notes       : Message Queue can store (AHC_MSG_QUEUE_SIZE - 1) messages
//------------------------------------------------------------------------------
/**
 @brief This function sends an AHL message.
 Parameters:
 @param[in] ulMsgID : Message ID
 @param[in] ulParam1 : The first parameter. sent with the operation.
 @param[in] ulParam2 : The second parameter sent with the operation.
 @retval TRUE or FALSE. // TRUE: Success, FALSE: Fail
*/
AHC_BOOL AHC_SendAHLMessage(UINT32 ulMsgID, UINT32 ulParam1, UINT32 ulParam2)
{
    UINT8 ret;

    static AHC_BOOL bShowDBG = AHC_TRUE;

    if (m_bSendAHLMessage == AHC_FALSE) {
        return AHC_TRUE;
    }

    if (AHC_OS_AcquireSem(m_AHLMessageSemID, 0) != OS_NO_ERR) {
        printc("AHC_SendAHLMessage OSSemPost: Fail!! \r\n");
        return AHC_FALSE;
    }

    if ((glAhcParameter.AhlMsgUnblock != glAhcParameter.AhlMsgBlock) && 
        (ulMsgID == glAhcParameter.AhlMsgBlock)) {
        // Message ID blocked !
        AHC_OS_ReleaseSem(m_AHLMessageSemID);
        return AHC_TRUE;
    }

    #if (AHC_MESSAGE_QUEUE_OVF)
    if (m_MessageQueueIndex_W > m_MessageQueueIndex_R) {
        printc("MQ : 0x%X\r\n", m_MessageQueueIndex_W - m_MessageQueueIndex_R);
    }
    else if (m_MessageQueueIndex_W < m_MessageQueueIndex_R) {
        printc("MQ : 0x%X\r\n", (AHC_MSG_QUEUE_SIZE - m_MessageQueueIndex_R) + m_MessageQueueIndex_W);
    }
    else {
        printc("MQ : 0x00\r\n");
    }
    #endif
    
    if (m_MessageQueueIndex_R == (m_MessageQueueIndex_W + AHC_MSG_QUEUE_VIP_SIZE + 1) % AHC_MSG_QUEUE_SIZE) {
        // Message Queue Full !
        if (bShowDBG) {
            #if (AHC_MESSAGE_QUEUE_OVF)
            UINT8 i;
            #endif

            bShowDBG = AHC_FALSE;
            printc(BG_YELLOW("XXX : Message Queue Full ...Fail!! XXX")"\r\n");

            #if (AHC_MESSAGE_QUEUE_OVF)
            printc("Dump Message Queue from Index_R to Index_W\r\n");
            for (i = m_MessageQueueIndex_R; i < AHC_MSG_QUEUE_SIZE; i++) {
                printc("%3d:: %4d     %4d     %4d\r\n", i, m_MessageQueue[i].ulMsgID, m_MessageQueue[i].ulParam1,m_MessageQueue[i].ulParam2);
            }
            printc("------------------------------\r\n");
            for (i = 0; i < m_MessageQueueIndex_W; i++) {
                printc("%3d:: %4d     %4d     %4dX\r\n", i, m_MessageQueue[i].ulMsgID, m_MessageQueue[i].ulParam1,m_MessageQueue[i].ulParam2);
            }
            #endif
        }
        AHC_OS_ReleaseSem(m_AHLMessageSemID);
        return AHC_FALSE;
    }

    m_MessageQueue[m_MessageQueueIndex_W].ulMsgID = ulMsgID;
    m_MessageQueue[m_MessageQueueIndex_W].ulParam1 = ulParam1;
    m_MessageQueue[m_MessageQueueIndex_W].ulParam2 = ulParam2;

    ret = AHC_OS_PutMessage(AHC_MSG_QId, (void *)(&m_MessageQueue[m_MessageQueueIndex_W]));

    if (ret != 0) {
        printc("AHC_OS_PutMessage: ret=%d  Fail!!\r\n", ret);
        AHC_OS_ReleaseSem(m_AHLMessageSemID);
        return AHC_FALSE;
    }

    bShowDBG = AHC_TRUE;

    m_MessageQueueIndex_W = (m_MessageQueueIndex_W + 1)%AHC_MSG_QUEUE_SIZE;

    AHC_OS_ReleaseSem(m_AHLMessageSemID);

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SendAHLMessage_HP
//  Description : This function sends an AHL message.
//  Notes       : Message Queue can store (AHC_MSG_QUEUE_SIZE - 1) messages
//------------------------------------------------------------------------------
/**
 @brief This function sends an AHL message.
 Parameters:
 @param[in] ulMsgID : Message ID
 @param[in] ulParam1 : The first parameter. sent with the operation.
 @param[in] ulParam2 : The second parameter sent with the operation.
 @retval TRUE or FALSE. // TRUE: Success, FALSE: Fail
*/
AHC_BOOL AHC_SendAHLMessage_HP(UINT32 ulMsgID, UINT32 ulParam1, UINT32 ulParam2)
{
    UINT8 ret;

    static AHC_BOOL bShowDBG = AHC_TRUE;

    if (AHC_OS_AcquireSem(m_AHLHPMessageSemID, 0) != OS_NO_ERR) {
        printc("AHC_SendAHLMessage OSSemPost: Fail!! \r\n");
        return AHC_FALSE;
    }

    if (m_HPMessageQueueIndex_R == (m_HPMessageQueueIndex_W + 1) % AHC_HP_MSG_QUEUE_SIZE) {
        // Message Queue Full !
        if (bShowDBG) {
            #if (AHC_MESSAGE_QUEUE_OVF)
            UINT8 i;
            #endif

            bShowDBG = AHC_FALSE;
            printc("XXX : HP Message Queue Full ...Fail!! XXX \r\n");

            #if (AHC_MESSAGE_QUEUE_OVF)
            printc("Dump Message Queue from Index_R to Index_W\r\n");
            for (i = m_HPMessageQueueIndex_R; i < AHC_HP_MSG_QUEUE_SIZE; i++) {
                printc("%3d:: %4d     %4d     %4d\r\n", i, m_HPMessageQueue[i].ulMsgID, m_HPMessageQueue[i].ulParam1, m_HPMessageQueue[i].ulParam2);
            }
            printc("------------------------------\r\n");
            for (i = 0; i < m_HPMessageQueueIndex_W; i++) {
                printc("%3d:: %4d     %4d     %4dX\r\n", i, m_HPMessageQueue[i].ulMsgID, m_HPMessageQueue[i].ulParam1, m_HPMessageQueue[i].ulParam2);
            }
            #endif
        }
        AHC_OS_ReleaseSem(m_AHLHPMessageSemID);
        return AHC_FALSE;
    }

    m_HPMessageQueue[m_HPMessageQueueIndex_W].ulMsgID = ulMsgID;
    m_HPMessageQueue[m_HPMessageQueueIndex_W].ulParam1 = ulParam1;
    m_HPMessageQueue[m_HPMessageQueueIndex_W].ulParam2 = ulParam2;

    ret = AHC_OS_PutMessage(AHC_HP_MSG_QId, (void *)(&m_HPMessageQueue[m_HPMessageQueueIndex_W]));

    if (ret != 0) {
        printc("HP AHC_OS_PutMessage: ret=%d  Fail!!\r\n", ret);
        AHC_OS_ReleaseSem(m_AHLHPMessageSemID);
        return AHC_FALSE;
    }

    bShowDBG = AHC_TRUE;

    m_HPMessageQueueIndex_W = (m_HPMessageQueueIndex_W + 1) % AHC_HP_MSG_QUEUE_SIZE;

    AHC_OS_ReleaseSem(m_AHLHPMessageSemID);

    if (AHC_OS_AcquireSem(m_AHLHPMessageCountSemID, 0) != OS_NO_ERR) {
        printc("AcquireSem m_AHLHPMessageCountSemID: Fail!! \r\n");
        return AHC_FALSE;
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetAHLMessage
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_GetAHLMessage(UINT32* ulMsgID, UINT32* ulParam1, UINT32* ulParam2)
{
    void *pMessageAddr = NULL;
    AHC_QUEUE_MESSAGE *ptr = NULL;

    if (AHC_OS_GetMessage(AHC_MSG_QId, (void*)(&pMessageAddr), AHC_AHL_MSGQ_TIMEOUT) != 0 )
    {
        if (AHC_AHL_MSGQ_TIMEOUT == 0)
            RTNA_DBG_Str(AHC_GENERAL_DBG_LEVEL,"AHC_ERROR : GetAHLMessage OS_ERR...\r\n");

        return AHC_FALSE;
    }
    else {

        ptr = (AHC_QUEUE_MESSAGE *)(pMessageAddr);
        *ulMsgID = ptr->ulMsgID;
        *ulParam1 = ptr->ulParam1;
        *ulParam2 = ptr->ulParam2;

        if (AHC_OS_AcquireSem(m_AHLMessageSemID, 0) != OS_NO_ERR) {
            printc("AHC_GetAHLMessage OSSemPost: Fail!! \r\n");
            return AHC_FALSE;
        }

        m_MessageQueueIndex_R = (m_MessageQueueIndex_R + 1)%AHC_MSG_QUEUE_SIZE;
        
        AHC_OS_ReleaseSem(m_AHLMessageSemID);
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetAHLMessage_HP
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_GetAHLMessage_HP(UINT32* ulMsgID, UINT32* ulParam1, UINT32* ulParam2)
{
    void *pMessageAddr = NULL;
    AHC_QUEUE_MESSAGE *ptr = NULL;

    if (AHC_OS_GetMessage(AHC_HP_MSG_QId, (void*)(&pMessageAddr), 0x1) != 0)
    {
        *ulMsgID = NULL;
        *ulParam1 = NULL;
        *ulParam2 = NULL;
        return AHC_FALSE;
    }
    else {

        ptr = (AHC_QUEUE_MESSAGE *)(pMessageAddr);
        *ulMsgID = ptr->ulMsgID;
        *ulParam1 = ptr->ulParam1;
        *ulParam2 = ptr->ulParam2;

        if (AHC_OS_AcquireSem(m_AHLHPMessageSemID, 0) != OS_NO_ERR) {
            printc("AHC_GetAHLMessage OSSemPost: Fail!! \r\n");
            return AHC_FALSE;
        }
        
        m_HPMessageQueueIndex_R = (m_HPMessageQueueIndex_R + 1) % AHC_HP_MSG_QUEUE_SIZE;
        
        AHC_OS_ReleaseSem(m_AHLHPMessageSemID);

        AHC_OS_ReleaseSem(m_AHLHPMessageCountSemID);
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetAHLHPMessageCount
//  Description :
//------------------------------------------------------------------------------
void AHC_GetAHLHPMessageCount(UINT16 *usCount)
{
    AHC_OS_QuerySem(m_AHLHPMessageCountSemID, usCount);
}

//------------------------------------------------------------------------------
//  Function    : AHC_DumpAHLMessage
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_DumpAHLMessage(void)
{
    UINT8 i;
    
    if (AHC_OS_AcquireSem(m_AHLHPMessageSemID, 0) != OS_NO_ERR) {
        printc("AHC_DumpAHLMessage OSSemPost: Fail!! \r\n");
        return AHC_FALSE;
    }

    printc("Dump HP Message Queue from Index_R to Index_W\r\n");
    for (i = m_HPMessageQueueIndex_R; i < AHC_HP_MSG_QUEUE_SIZE; i++) {
        printc("%3d:: %4d     %4d     %4d\r\n", i, m_HPMessageQueue[i].ulMsgID, m_HPMessageQueue[i].ulParam1, m_HPMessageQueue[i].ulParam2);
    }
    printc("------------------------------\r\n");
    for (i = 0; i < m_HPMessageQueueIndex_W; i++) {
        printc("%3d:: %4d     %4d     %4dX\r\n", i, m_HPMessageQueue[i].ulMsgID, m_HPMessageQueue[i].ulParam1, m_HPMessageQueue[i].ulParam2);
    }

    printc("------------------------------\r\n");

    printc("Dump Message Queue from Index_R to Index_W\r\n");
    for (i = m_MessageQueueIndex_R; i < AHC_MSG_QUEUE_SIZE; i++) {
        printc("%3d:: %4d     %4d     %4d\r\n", i, m_MessageQueue[i].ulMsgID, m_MessageQueue[i].ulParam1,m_MessageQueue[i].ulParam2);
    }
    printc("------------------------------\r\n");
    for (i = 0; i < m_MessageQueueIndex_W; i++) {
        printc("%3d:: %4d     %4d     %4dX\r\n", i, m_MessageQueue[i].ulMsgID, m_MessageQueue[i].ulParam1,m_MessageQueue[i].ulParam2);
    }

    AHC_OS_ReleaseSem(m_AHLHPMessageSemID);
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_InitAHLMessage
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AIHC_InitAHLMessage(void)
{
    m_AHLMessageSemID = AHC_OS_CreateSem(1);
    
    AHC_MSG_QId = AHC_OS_CreateMQueue(AHC_MsgQueue, AHC_MSG_QUEUE_SIZE);

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_InitAHLMessage_HP
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AIHC_InitAHLMessage_HP(void)
{
    m_AHLHPMessageCountSemID = AHC_OS_CreateSem(AHC_HP_MSG_QUEUE_SIZE);
    m_AHLHPMessageSemID      = AHC_OS_CreateSem(1);
    
    AHC_HP_MSG_QId = AHC_OS_CreateMQueue(AHC_HPMsgQueue, AHC_HP_MSG_QUEUE_SIZE);

    return AHC_TRUE;
}

#if 0
void ____GPIO_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_GPIO_ConfigPad
//  Description :
//  @brief This function configures the pull high or low of selected GPIO.
//  @param[in] ahc_piopin : PIO definition, please refer AHC_PIO_REG
//  @param[in] config :
/*
        #define PAD_NORMAL_TRIG             0x00
        #define PAD_SCHMITT_TRIG            0x01
        #define PAD_PULL_DOWN               0x02
        #define PAD_PULL_UP                 0x04
        #define PAD_FAST_SLEW               0x08
        #define PAD_SLOW_SLEW               0x00
        #define PAD_IDDQ_TEST_EN            0x10
        #define PAD_OUT_DRIVING(_a)         (((_a)&0x03)<<5)
*/
//------------------------------------------------------------------------------
AHC_BOOL AHC_GPIO_ConfigPad(AHC_PIO_REG piopin, MMP_UBYTE ubConfig)
{
    if (MMPS_PIO_PadConfig(piopin, ubConfig) == MMP_ERR_NONE) {
        return AHC_TRUE;
    }

    return AHC_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GPIO_SetOutputMode
//  Description :
//------------------------------------------------------------------------------
/**
 @brief This function configures the direction of selected GPIO.

 This function configures the direction of selected GPIO.
 Parameters:
 @param[in] ahc_piopin : PIO definition, please refer AHC_PIO_REG
 @param[in] bDirection : Configures the pin is input or output.
 @retval TRUE or FALSE.
*/

AHC_BOOL AHC_GPIO_SetOutputMode(AHC_PIO_REG ahc_piopin, UINT8 bDirection)
{
    MMP_ERR status;

    if (ahc_piopin == AHC_PIO_REG_UNKNOWN) {
        return AHC_FALSE;
    }

    if (bDirection == AHC_TRUE) {
        status = MMPS_PIO_EnableOutputMode((MMP_GPIO_PIN)ahc_piopin, MMP_TRUE);
    }
    else {
        status = MMPS_PIO_EnableOutputMode((MMP_GPIO_PIN)ahc_piopin, MMP_FALSE);
    }

    if (status != MMP_ERR_NONE) {
        return AHC_FALSE;
    }
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GPIO_SetTrigMode
//  Description :
//------------------------------------------------------------------------------
/**
 @brief This API set a callback to be the handler of I/O actions.

 This API set a callback to be the handler of I/O actions.
 Parameters:
 @param[in] ahc_piopin : PIO definition, please refer AHC_PIO_REG
 @param[in] bPolarity : define when the function should be called.
 @param[in] phHandleFunc : Handler function.
 @retval TRUE or FALSE.
*/
AHC_BOOL AHC_GPIO_SetTrigMode(AHC_PIO_REG ahc_piopin, AHC_PIO_TRIGMODE bPolarity)
{
    MMP_ERR         status;
    MMP_GPIO_TRIG   trigmode = bPolarity;

    if (ahc_piopin == AHC_PIO_REG_UNKNOWN) {
        return AHC_FALSE;
    }

    status = MMPS_PIO_EnableTrigMode((MMP_GPIO_PIN)ahc_piopin, trigmode, MMP_TRUE);

    if (status != MMP_ERR_NONE) {
        return AHC_FALSE;
    }
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GPIO_EnableInterrupt
//  Description :
//------------------------------------------------------------------------------
/**
 @brief This function enable or disable the handler in AHC_GPIO_SetTrigMode.

 This function enable or disable the handler in AHC_GPIO_SetTrigMode.
 Parameters:
 @param[in] ahc_piopin : PIO definition, please refer AHC_PIO_REG
 @param[in] bEnable : Configures the status of GPIO handler.
 @retval TRUE or FALSE.
*/
AHC_BOOL AHC_GPIO_EnableInterrupt(AHC_PIO_REG ahc_piopin, void* phHandleFunc, UINT8 bEnable)
{
    MMP_ERR status;

    if (ahc_piopin == AHC_PIO_REG_UNKNOWN) {
        return AHC_FALSE;
    }

    status = MMPS_PIO_EnableInterrupt((MMP_GPIO_PIN)ahc_piopin, bEnable, 0, (GpioCallBackFunc *)phHandleFunc);

    if (status != MMP_ERR_NONE) {
        return AHC_FALSE;
    }
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GPIO_SetData
//  Description :
//------------------------------------------------------------------------------
/**
 @brief This function sets the state of a selected I/O pin.

 This function sets the state of a selected I/O pin.
 Parameters:
 @param[in] ahc_piopin : PIO definition, please refer AHC_PIO_REG
 @param[in] bState : Configures the output state of selected pin.
 @retval TRUE or FALSE.
*/
AHC_BOOL AHC_GPIO_SetData(AHC_PIO_REG ahc_piopin, UINT8 bState)
{
    MMP_ERR status;

    if (ahc_piopin == AHC_PIO_REG_UNKNOWN) {
        return AHC_FALSE;
    }

    status = MMPS_PIO_SetData((MMP_GPIO_PIN)ahc_piopin, bState);

    if (status != MMP_ERR_NONE) {
        return AHC_FALSE;
    }
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GPIO_GetData
//  Description :
//------------------------------------------------------------------------------
/**
 @brief This function gets the state of a selected I/O pin.

 This function gets the state of a selected I/O pin.
 Parameters:
 @param[in] ahc_piopin : PIO definition, please refer AHC_PIO_REG
 @param[out] pwState : Get the input state of selected pin.
 @retval TRUE or FALSE.
*/
AHC_BOOL AHC_GPIO_GetData(AHC_PIO_REG ahc_piopin, UINT8 *pwState)
{
    MMP_ERR status;

    if (ahc_piopin == AHC_PIO_REG_UNKNOWN) {
        return AHC_FALSE;
    }

    status = MMPS_PIO_GetData((MMP_GPIO_PIN)ahc_piopin, pwState);

    if (status != MMP_ERR_NONE) {
        return AHC_FALSE;
    }
    return AHC_TRUE;
}

#if 0
void _____PWM_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_PWM_Initialize
//  Description :
//------------------------------------------------------------------------------
/** @brief Driver init

Driver init
@retval TRUE or FALSE.
*/
AHC_BOOL AHC_PWM_Initialize(void)
{
    MMP_ERR status;

    status = MMPS_PWM_Initialize();

    if (status != MMP_ERR_NONE) {
        return AHC_FALSE;
    }
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_PWM_OutputPulse
//  Description :
//------------------------------------------------------------------------------
/** @brief Simplely output some pulses (According to the parameters)

Simplely output some pulses (According to the parameters)
@param[in] pwm_pin : PWM I/O pin selection, please refer MMP_PWM_PIN
@param[in] bEnableIoPin : enable/disable the specific PWM pin
@param[in] ulFrquency : the pulse frequency.
@param[in] bHigh2Low : MMP_TRUE: High to Low pulse, MMP_FALSE: Low to High pulse
@param[in] bEnableInterrupt : enable interrupt or not
@param[in] pwm_callBack : call back function when interrupt occurs
@param[in] ulNumOfPulse : number of pulse, 0 stand for using PWM auto mode to generate infinite pulse.
@return It reports the status of the operation.
*/
AHC_BOOL AHC_PWM_OutputPulse(MMP_PWM_PIN pwm_pin, AHC_BOOL bEnableIoPin, UINT32 ulFrquency, AHC_BOOL bHigh2Low, AHC_BOOL bEnableInterrupt, void* pwm_callBack, UINT32 ulNumOfPulse)
{
    MMP_ERR status;

    status = MMPS_PWM_OutputPulse(pwm_pin, bEnableIoPin, ulFrquency, 50, bHigh2Low, bEnableInterrupt, (PwmCallBackFunc*)pwm_callBack, ulNumOfPulse);

    if (status != MMP_ERR_NONE) {
        return AHC_FALSE;
    }
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_PWM_OutputPulseEx
//  Description :
//------------------------------------------------------------------------------
/** @brief Simplely output some pulses (According to the parameters)

Simplely output some pulses (According to the parameters)
@param[in] pwm_pin : PWM I/O pin selection, please refer MMP_PWM_PIN
@param[in] bEnableIoPin : enable/disable the specific PWM pin
@param[in] ulFrquency : the pulse frequency.
@param[in] ulDuty : percentage of signal = high in 1 PWM cycle
@param[in] bHigh2Low : MMP_TRUE: High to Low pulse, MMP_FALSE: Low to High pulse
@param[in] bEnableInterrupt : enable interrupt or not
@param[in] pwm_callBack : call back function when interrupt occurs
@param[in] ulNumOfPulse : number of pulse, 0 stand for using PWM auto mode to generate infinite pulse.
@return It reports the status of the operation.
*/
AHC_BOOL AHC_PWM_OutputPulseEx(MMP_PWM_PIN pwm_pin, AHC_BOOL bEnableIoPin, UINT32 ulFrquency, UINT32 ulDuty, AHC_BOOL bHigh2Low, AHC_BOOL bEnableInterrupt, void* pwm_callBack, UINT32 ulNumOfPulse)
{
    MMP_ERR status;

    status = MMPS_PWM_OutputPulse(pwm_pin, bEnableIoPin, ulFrquency, ulDuty, bHigh2Low, bEnableInterrupt, (PwmCallBackFunc*)pwm_callBack, ulNumOfPulse);

    if (status != MMP_ERR_NONE) {
        return AHC_FALSE;
    }
    return AHC_TRUE;
}

#if 0
void ____WatchDog_Function_____(){ruturn;} //dummy
#endif

#if (AIT_HW_WATCHDOG_ENABLE)
//------------------------------------------------------------------------------
//  Function    : AHC_WD_Enable
//  Description :
//------------------------------------------------------------------------------
void AHC_WD_Enable(AHC_BOOL bEnable)
{
    MMP_ERR sRet = MMP_ERR_NONE;
    
    if (bEnable) {
    
        MMP_ULONG clk_div, CLK_DIV[] = {1024, 128, 32 , 8};
        MMP_ULONG ms;
        MMP_ULONG g0_slow, rst_c, c;

        ms = 4000;
        
        MMPF_PLL_GetGroupFreq(CLK_GRP_GBL, &g0_slow);
        g0_slow = g0_slow / 2;
        
        c = 0;

        printc("ms : %d, G0 slow:%d\r\n",ms, g0_slow);        

        do {
            clk_div = CLK_DIV[c];
            rst_c = (g0_slow * ms) / (clk_div * 16384);
            printc("rst_c : %d,DIV:%d\r\n",rst_c, clk_div);
            c++;
            
            if (c >= 3) {
                printc("%s,%d parameters error!\r\n", __func__, __LINE__);
                break ;
            }
        }
        while ((rst_c > 31) || (!rst_c));
                       
        sRet = MMPF_WD_SetTimeOut(rst_c, clk_div);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return;}    

        sRet = MMPF_WD_EnableWD(MMP_TRUE, MMP_TRUE, MMP_FALSE, NULL, MMP_TRUE); 
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return;}    

        sRet = MMPF_WD_SW_EnableWD(MMP_TRUE, AIT_HW_WATCHDOG_TIMEOUT);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return;}    
    }
    else {
        sRet = MMPF_WD_EnableWD(MMP_FALSE, MMP_FALSE, MMP_FALSE, NULL, MMP_FALSE);         
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return;}    

        sRet = MMPF_WD_SW_EnableWD(MMP_FALSE, 0);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return;}    
    }
}

//------------------------------------------------------------------------------
//  Function    : AHC_WD_Kick
//  Description :
//------------------------------------------------------------------------------
void AHC_WD_Kick(void)
{
    MMPF_WD_SW_Kick();
}
#endif

#if 0
void ____PowerOff_Function_____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_PowerOff_Preprocess
//  Description : Store FW/User settings before power off
//------------------------------------------------------------------------------
void AHC_PowerOff_Preprocess(void)
{
#if (POWER_OFF_PREPROCESS_EN)

    #if (ENABLE_DETECT_FLOW)
    Menu_WriteSetting();
    AHC_OS_SleepMs(10);
    #endif

 //   AHC_SetMode(AHC_MODE_IDLE);
    AHC_OS_SleepMs(10);
    
    AHC_VIDEO_WaitVideoWriteFileFinish();

    AHC_UnloadSystemFile();
    
    #if (!DEBUG_UART_TO_FILE) && (!DEBUG_UART_TO_SD)
    AHC_UnmountVolume(AHC_MEDIA_MMC);
    #endif
    
    AHC_OS_SleepMs(10);
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_PowerOff_VirtualPath
//  Description :
//------------------------------------------------------------------------------
void AHC_PowerOff_VirtualPath(void)
{
#if (SUPPORT_VIRTUAL_POWER_OFF)

    #if (SUPPORT_IR_CONVERTER)
    AHC_IR_WriteRegister(IR_CONVERTER_CLEAR_POWERON, 0x01);
    #endif

    if (uiGetCurrentState() == UI_VIRTUAL_POWER_SAVE_STATE) {
        printc("VPS -> Power Off\r\n");
    }
    else {
        if (AHC_IsUsbConnect()) {
            printc("Go -> VPS\r\n");
            #if (ENABLE_POWER_OFF_PICTURE)
            uiPowerOffPicture();
            #endif
            StateSwitchMode(UI_VIRTUAL_POWER_SAVE_STATE);
            return;
        }
    }

    AHC_PowerOff_Preprocess();

    if (uiGetCurrentState() == UI_VIRTUAL_POWER_SAVE_STATE) {
        // NOP
    }
    else {
        #if (ENABLE_POWER_OFF_PICTURE)
        uiPowerOffPicture();
        #endif
    }

    LedCtrl_LcdBackLight(AHC_FALSE);
    AHC_PMU_PowerOff();
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_PowerOff_NormalPath
//  Description :
//------------------------------------------------------------------------------
void AHC_PowerOff_NormalPath(void)
{
    #if (defined(DEVICE_GPIO_2NDSD_PLUG))
    #if (TWOSD_WORK_MODEL == TWOSD_WORK_MODEL_MASTER_SLAVE)
    MenuSettingConfig()->uiMediaSelect = MEDIA_SETTING_SD_CARD;
    #endif
    #endif
     // printc("Mode OFF  AHC_PowerOff_NormalPath\r\n");
	ReserveTheRadio_AM_FM(); // lyj 20190114 bao cun dian tai
    #if (SUPPORT_IR_CONVERTER)
    //AHC_IR_WriteRegister(IR_CONVERTER_CLEAR_POWERON, 0x01);//TBD
    #endif

    gbBlockRealIDUIKeyTask = AHC_TRUE;

    AHC_PowerOff_Preprocess();

    #ifndef CFG_POWEROFF_WITHOUT_CLEAR_WMSG
    AHC_WMSG_Draw(AHC_FALSE, WMSG_NONE, 0); // Clear WMSG
    #endif

    #if(ENABLE_POWER_OFF_PICTURE)
    uiPowerOffPicture();
    #endif

    AHC_PMU_PowerOff();

    gbBlockRealIDUIKeyTask = AHC_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_PowerOff_NormalPathEx
//  Description :
//------------------------------------------------------------------------------
void AHC_PowerOff_NormalPathEx(AHC_BOOL bForce, AHC_BOOL bByebye, AHC_BOOL bSound)
{
    extern void AHC_PMU_PowerOffEx(AHC_BOOL bSound);

    if (bForce == AHC_TRUE)
    {
        AHC_PowerOff_Preprocess();

        if (bByebye == AHC_TRUE) {
            uiPowerOffPicture();
        }
        AHC_PMU_PowerOffEx(bSound);
    }
    else {
        #if (POWER_OFF_CONFIRM_PAGE_EN)
        PowerOff_InProcess  = AHC_TRUE;
        PowerOff_Option     = CONFIRM_NOT_YET;

        if (PowerOff_Option == CONFIRM_NOT_YET) {
            MenuDrawSubPage_PowerOff(NULL);
        }
        #endif
    }
}

#if (POWER_OFF_CONFIRM_PAGE_EN)
//------------------------------------------------------------------------------
//  Function    : AHC_PowerOff_ResetVarible
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_PowerOff_ResetVarible(void)
{
    PowerOff_InProcess  = AHC_FALSE;
    PowerOff_Option     = CONFIRM_NOT_YET;
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_PowerOff_Confirmed
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_PowerOff_Confirmed(void)
{
    /*
     * Turn off backlight, to avoid user see preview before goodbye screen.
     */
    LedCtrl_LcdBackLight(AHC_FALSE);

    AHC_PowerOff_ResetVarible();

    AHC_OSDSetColor(0 /*OSD_COLOR_TRANSPARENT*/);
    AHC_OSDDrawFillRect(0 /*MAIN_DISPLAY_BUFFER*/, 0, 0 , 320, 240);

    AHC_PowerOff_Preprocess();
    
    uiPowerOffPicture();
    AHC_PMU_PowerOff();
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_PowerOff_Cancel
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_PowerOff_Cancel(void)
{
    AHC_PowerOff_ResetVarible();

    if (uiGetCurrentState() == UI_TVOUT_STATE)
    {
        #if (TVOUT_ENABLE)
        MenuDrawSubPage_PowerOffCancel_TV();
        #endif
        return AHC_TRUE;
    }
    else if (uiGetCurrentState() == UI_HDMI_STATE)
    {
        #if (HDMI_ENABLE)
        MenuDrawSubPage_PowerOffCancel_HDMI();
        #endif
        return AHC_TRUE;
    }
    else if (uiGetCurrentState() == UI_PLAYBACK_STATE)
    {
        extern AHC_BOOL gbEnterPowerOffPagePause;

        if (GetMovConfig()->iState == MOV_STATE_PAUSE && gbEnterPowerOffPagePause == AHC_TRUE)
        {
            MovPlayback_Resume();
            gbEnterPowerOffPagePause = AHC_FALSE;
        }
    }

    AHC_OSDSetColor(0 /*OSD_COLOR_TRANSPARENT*/);
    AHC_OSDDrawFillRect(0 /*MAIN_DISPLAY_BUFFER*/, 0, 0 , m_OSD[0]->width, m_OSD[0]->height);

    #ifdef FLM_GPIO_NUM
    AHC_OSDRefresh_PLCD();
    #endif

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_PowerOff_Option
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_PowerOff_Option(AHC_BOOL op)
{
    if (op == CONFIRM_OPT_YES)
        AHC_PowerOff_Confirmed();
    else if(op == CONFIRM_OPT_NO)
        AHC_PowerOff_Cancel();

    return AHC_TRUE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_PowerOff_ShowPicture
//  Description :
//------------------------------------------------------------------------------
AHC_ERR AHC_PowerOff_ShowPicture(INT8 *charStr)
{
    MMP_DSC_JPEG_INFO           jpeginfo;
    MMP_DISPLAY_OUTPUT_SEL      displaypanel;
    MMP_ERR                     err;
    UINT16                      ImgW;
    UINT16                      ImgH;

    MMPD_System_EnableClock(MMPD_SYS_CLK_BAYER, MMP_TRUE);

#if (FS_INPUT_ENCODE == UCS2)
    uniStrcpy(jpeginfo.bJpegFileName, charStr);
    jpeginfo.usJpegFileNameLength = uniStrlen((short *)charStr);
#elif (FS_INPUT_ENCODE == UTF8)
    STRCPY(jpeginfo.bJpegFileName, charStr);
    jpeginfo.usJpegFileNameLength = STRLEN(charStr);
#endif

    jpeginfo.ulJpegBufAddr          = NULL;
    jpeginfo.ulJpegBufSize          = 0;
    jpeginfo.bDecodeThumbnail       = MMP_FALSE;
    #if (DSC_SUPPORT_BASELINE_MP_FILE)
    jpeginfo.bDecodeLargeThumb 		= MMP_FALSE;
    #endif
    jpeginfo.bValid                 = MMP_FALSE;
    jpeginfo.bPowerOffLogo          = MMP_TRUE;

#if ((SUPPORT_TV) || (MENU_SINGLE_EN))
    if (AHC_IsTVConnectEx()) {
    
        UINT8 ctv_sys = 0;
    
        if ((AHC_Menu_SettingGetCB((char*)COMMON_KEY_TV_SYSTEM, &ctv_sys) == AHC_FALSE)) {
            ctv_sys = COMMON_TV_SYSTEM_NTSC;
        }

        switch(ctv_sys)
        {
        case COMMON_TV_SYSTEM_NTSC:
            MMPS_Display_SetOutputPanel(MMP_DISPLAY_PRM_CTL, MMP_DISPLAY_SEL_NTSC_TV);
            break;
        case COMMON_TV_SYSTEM_PAL:
        default:
            MMPS_Display_SetOutputPanel(MMP_DISPLAY_PRM_CTL, MMP_DISPLAY_SEL_PAL_TV);
            break;
        }
    }
    else
#endif
#if (SUPPORT_HDMI)
    if (AHC_IsHdmiConnect()) {
        MMPS_Display_SetOutputPanel(MMP_DISPLAY_PRM_CTL, MMP_DISPLAY_SEL_HDMI);
    }
    else
#endif
    {
        MMPS_Display_SetOutputPanel(MMP_DISPLAY_PRM_CTL, MMP_DISPLAY_SEL_MAIN_LCD);
    }

    MMPS_Display_SetWinActive(OSD_LAYER_WINDOW_ID, MMP_FALSE);

    MMPS_Display_GetOutputPanel(MMP_DISPLAY_PRM_CTL, &displaypanel);

#if (SUPPORT_HDMI) && (SUPPORT_TV)
    if (AHC_IsHdmiConnect() || AHC_IsTVConnectEx())
#else
    if (0)
#endif
    {
        #if (SUPPORT_TV)
        if (AHC_IsTVConnectEx())
        {
            UINT8 ctv_sys = 0;
            
            if ((AHC_Menu_SettingGetCB((char*)COMMON_KEY_TV_SYSTEM, &ctv_sys) == AHC_FALSE)){
                ctv_sys = COMMON_TV_SYSTEM_NTSC; //default value
            }

            switch(ctv_sys)
            {
            case COMMON_TV_SYSTEM_PAL:
                ImgW = TV_PAL_POWEROFF_IMG_W;
                ImgH = TV_PAL_POWEROFF_IMG_H;
                break;
            case COMMON_TV_SYSTEM_NTSC:
            default:
                ImgW = TV_NTSC_POWEROFF_IMG_W;
                ImgH = TV_NTSC_POWEROFF_IMG_H;
              break;
            }
        }
        else
        #endif
        #if (SUPPORT_HDMI)
        if (AHC_IsHdmiConnect())
        {
            switch(MenuSettingConfig()->uiHDMIOutput)
            {
            case HDMI_OUTPUT_1080I:
                ImgW = HDMI_1080I_POWEROFF_IMG_W;
                ImgH = HDMI_1080I_POWEROFF_IMG_H;
                break;
            case HDMI_OUTPUT_720P:
                ImgW = HDMI_720P_POWEROFF_IMG_W;
                ImgH = HDMI_720P_POWEROFF_IMG_H;
                break;
            case HDMI_OUTPUT_480P:
                ImgW = HDMI_480P_POWEROFF_IMG_W;
                ImgH = HDMI_480P_POWEROFF_IMG_H;
                break;
            default:
            case HDMI_OUTPUT_1080P:
                ImgW = HDMI_1080P_POWEROFF_IMG_W;
                ImgH = HDMI_1080P_POWEROFF_IMG_H;
                break;
            }
        }
        #endif

        //AHC_SetDisplayWindow(DISPLAY_SYSMODE_STILLPLAYBACK, AHC_TRUE, AHC_FALSE, 0, 0, ImgW, ImgH, 0); // TBD

        err = MMPS_DSC_PlaybackJpeg(&jpeginfo);
    }
    else if (displaypanel == MMP_DISPLAY_SEL_MAIN_LCD)
    {
        ImgW = RTNA_LCD_GetAttr()->usPanelW;
        ImgH = RTNA_LCD_GetAttr()->usPanelH;

        #ifdef FLM_GPIO_NUM
        AHC_OSDSetActive(0,  AHC_FALSE);
        AHC_OSDSetActive(20, AHC_FALSE);
        AHC_OSDRefresh_PLCD();
        #endif

        #if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
        AHC_SetDisplayWindow(DISPLAY_SYSMODE_STILLPLAYBACK, AHC_TRUE, AHC_FALSE, 0, 0, ImgW, ImgH, 0);
        #elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
        AHC_SetDisplayWindow(DISPLAY_SYSMODE_STILLPLAYBACK, AHC_TRUE, AHC_FALSE, 0, 0, ImgH, ImgW, 0);
        #endif
        
        err = MMPS_DSC_PlaybackJpeg(&jpeginfo);
    }

    return AHC_TRUE;
}

#if 0
void ____PIR_Function_____(){ruturn;} //dummy
#endif

#if (ENABLE_PIR_MODE)
//------------------------------------------------------------------------------
//  Function    : AHC_PIR_CheckStart
//  Description :
//------------------------------------------------------------------------------
void AHC_PIR_CheckStart(void)
{
    MMP_UBYTE tempValue;

    if (DEVICE_GPIO_PIR_INT != MMP_GPIO_MAX) {

        MMPF_PIO_GetData(DEVICE_GPIO_PIR_INT, &tempValue);

        if (tempValue) {
            gbPIRStarted = AHC_FALSE;
        }
        else {
            gbPIRStarted = AHC_TRUE;
        }
    }
    else {
        gbPIRStarted = AHC_FALSE;
    }
}

//------------------------------------------------------------------------------
//  Function    : AHC_PIR_CheckStart
//  Description :
//------------------------------------------------------------------------------
void AHC_PIR_SetEnable(AHC_BOOL bEnable)
{
    if (DEVICE_GPIO_PIR_EN != MMP_GPIO_MAX) {
        
        MMPF_PIO_EnableOutputMode(DEVICE_GPIO_PIR_EN, MMP_TRUE, MMP_FALSE);
       
        if (bEnable)
            MMPF_PIO_SetData(DEVICE_GPIO_PIR_EN, MMP_FALSE, MMP_FALSE);
        else
            MMPF_PIO_SetData(DEVICE_GPIO_PIR_EN, MMP_TRUE, MMP_FALSE);
    }
}
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_PIR_IsStarted
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_PIR_IsStarted(void)
{
    return gbPIRStarted;
}

#if 0
void _____Charger_Control_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_Charger_InitialIO
//  Description :
//------------------------------------------------------------------------------
void AHC_Charger_InitialIO(void)
{
#ifdef CHARGER_STATUS
    if (CHARGER_STATUS != AHC_PIO_REG_UNKNOWN) {
        AHC_GPIO_ConfigPad(CHARGER_STATUS, PAD_NORMAL_TRIG);
        AHC_GPIO_SetOutputMode(CHARGER_STATUS, AHC_FALSE);
    }

    AHC_Charger_SetTempCtrlEnable(AHC_FALSE);
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_Charger_SetEnable
//  Description :
//------------------------------------------------------------------------------
void AHC_Charger_SetEnable(AHC_BOOL bEnable)
{
#if defined(ENABLE_CHARGER_IC) && defined(CHARGER_ENABLE_GPIO)
    if (CHARGER_ENABLE_GPIO != AHC_PIO_REG_UNKNOWN)
    {
        AHC_GPIO_ConfigPad(CHARGER_ENABLE_GPIO, PAD_OUT_DRIVING(0));
        AHC_GPIO_SetOutputMode(CHARGER_ENABLE_GPIO, AHC_TRUE);
        AHC_GPIO_SetData(CHARGER_ENABLE_GPIO, bEnable ? (CHARGER_ENABLE_GPIO_ACT_LEVEL) : !(CHARGER_ENABLE_GPIO_ACT_LEVEL));
    }
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_Charger_SetEnable
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_Charger_GetEnable(void)
{
#if defined(ENABLE_CHARGER_IC) && defined(CHARGER_ENABLE_GPIO)
    AHC_BOOL bReturn = AHC_FALSE;

    if (CHARGER_ENABLE_GPIO != AHC_PIO_REG_UNKNOWN)
    {
        UINT8 gpioState;

        AHC_GPIO_GetData(CHARGER_ENABLE_GPIO, &gpioState);
        bReturn = ((CHARGER_ENABLE_GPIO_ACT_LEVEL == gpioState) ? AHC_TRUE : AHC_FALSE);
    }

    return bReturn;
#else
    return AHC_FALSE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_Charger_SetTempCtrlEnable
//  Description :
//------------------------------------------------------------------------------
void AHC_Charger_SetTempCtrlEnable(AHC_BOOL bEnable)
{
#if defined(ENABLE_CHARGER_IC) && defined(CHARGER_TEMP_CTL)
    if (CHARGER_TEMP_CTL != AHC_PIO_REG_UNKNOWN)
    {
        AHC_GPIO_ConfigPad(CHARGER_TEMP_CTL, PAD_OUT_DRIVING(0));
        AHC_GPIO_SetOutputMode(CHARGER_TEMP_CTL, AHC_TRUE);
        AHC_GPIO_SetData(CHARGER_TEMP_CTL, bEnable);
    }
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_Charger_SetTempCtrlEnable
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_Charger_GetTempCtrlStatus(void)
{
#if defined(ENABLE_CHARGER_IC) && defined(CHARGER_TEMP_CTL)
    AHC_BOOL bReturn = AHC_FALSE;

    if (CHARGER_TEMP_CTL != AHC_PIO_REG_UNKNOWN)
    {
        AHC_GPIO_GetData(CHARGER_TEMP_CTL, &bReturn);
    }

    return bReturn;
#else
    return AHC_FALSE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_Charger_GetStatus
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_Charger_GetStatus(void)
{
#if defined(ENABLE_CHARGER_IC) && defined(CHARGER_STATUS)
    if (CHARGER_STATUS != AHC_PIO_REG_UNKNOWN)
    {
        UINT8 state;

        AHC_GPIO_ConfigPad(CHARGER_STATUS, PAD_NORMAL_TRIG);
        AHC_GPIO_SetOutputMode(CHARGER_STATUS, AHC_FALSE);
        AHC_GPIO_GetData(CHARGER_STATUS, &state);

        if (CHARGER_STATUS_ACT_LEVEL == state) {
            return AHC_TRUE;
        }
    }
#endif

    if (AHC_IsDcCableConnect())
        return AHC_TRUE;

    if (AHC_IsUsbConnect())
        return AHC_TRUE;

    return AHC_FALSE;
}

#if 0
void ____Buzzer_Function____(){ruturn;} //dummy
#endif
void AHC_Brightness_Alert(UINT32 ulFrquency, UINT32 ulTimes, UINT32 ulMs)
{    
        MMP_BYTE i = 0;

        AHC_PWM_Initialize();

        for (i = 0; i < ulTimes; i++) {
          
                AHC_PWM_OutputPulse(MMP_PWM2_PIN_AGPIO3, MMP_TRUE, (MMP_ULONG)ulFrquency, MMP_TRUE, MMP_FALSE, NULL, 0xFF);
            
            if (i < (ulTimes - 1)) {
                AHC_OS_SleepMs(ulMs);
            }
        }
    }
//------------------------------------------------------------------------------
//  Function    : AHC_BUZZER_Alert
//  Description :
//------------------------------------------------------------------------------
void AHC_BUZZER_Alert(UINT32 ulFrquency, UINT32 ulTimes, UINT32 ulMs)
{
#if defined(SUPPORT_BUZZER) && (SUPPORT_BUZZER)

    #if (BUZZER_USING_SW_PWMN)

    #if defined(DEVICE_GPIO_BUZZER) && (DEVICE_GPIO_BUZZER != MMP_GPIO_MAX)
    {
        UINT32	ulPulseTimes = 300;
        
        AHC_GPIO_SetOutputMode(DEVICE_GPIO_BUZZER, AHC_TRUE);
        
        while (ulTimes > 0)
        {
            ulPulseTimes = 100*ulMs;
            
            while (ulPulseTimes > 0)
            {
                MMPF_PIO_SetData(DEVICE_GPIO_BUZZER, 0x01, MMP_TRUE);
                RTNA_WAIT_US(121);///110---4.46KHZ///122---3.968Khz////137---3.6KHZ
                MMPF_PIO_SetData(DEVICE_GPIO_BUZZER, 0x00, MMP_TRUE);
                RTNA_WAIT_US(121);
                
                ulPulseTimes--;
            }
            ulTimes--;
            AHC_OS_SleepMs(60);
        }

        AHC_OS_SleepMs(150);
    }
    #endif

    #else
    {    
        MMP_BYTE i = 0;

        AHC_PWM_Initialize();

        for (i = 0; i < ulTimes; i++) {
            #if defined(DEVICE_GPIO_BUZZER)
            if (DEVICE_GPIO_BUZZER != MMP_GPIO_MAX) {
                AHC_PWM_OutputPulse(DEVICE_GPIO_BUZZER, MMP_TRUE, (MMP_ULONG)ulFrquency, MMP_TRUE, MMP_FALSE, NULL, 0xFF);
            }
            #else
                AHC_PWM_OutputPulse(PWM2_PIN_AGPIO3, MMP_TRUE, (MMP_ULONG)ulFrquency, MMP_TRUE, MMP_FALSE, NULL, 0xFF);
            #endif

            if (i < (ulTimes - 1)) {
                AHC_OS_SleepMs(ulMs);
            }
        }
    }
    #endif
    
#endif
}

#if 0
void ____Misc_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_GetBootingSrc
//  Description :
//------------------------------------------------------------------------------
UINT8 AHC_GetBootingSrc(void)
{
//  PWR_ON_BY_KEY       0x01
//  PWR_ON_BY_VBUS      0x02
//  PWR_ON_BY_GSENSOR   0x04
//  PWR_ON_BY_IR        0x08
//  PWR_ON_BY_DC        0x10
    static MMP_UBYTE bStatus = 0xFF;

    if (bStatus == 0xFF)
    {
        bStatus = *(volatile MMP_UBYTE *) (TEMP_POWER_ON_SRC_INFO_ADDR);
        printc(FG_BLUE("PowerOnTriggerSrc = 0x%X\r\n"), *(volatile MMP_UBYTE *) (TEMP_POWER_ON_SRC_INFO_ADDR));
    }

    return bStatus;
}

//------------------------------------------------------------------------------
//  Function    : AHC_WaitForBootComplete
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_WaitForBootComplete(void)
{
    extern MMP_BOOL gbSysBootComplete;
    
    while ((glAhcBootComplete != MMP_TRUE) && (gbSysBootComplete != MMP_TRUE)) {
        AHC_OS_Sleep(100);
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_DumpRegister
//  Description : CHECK
//------------------------------------------------------------------------------
void AHC_DumpRegister(ULONG ulStartAddress, ULONG ulBytes)
{
    static MMP_ULONG preTime = 0;
    MMP_ULONG   curTime;
    ULONG       Register;

    RTNA_DBG_Str(0, "\r\n=====================================Register Dump Start\r\n");
    
    MMPF_OS_GetTime(&curTime);

    if (curTime >= preTime) {
        curTime -= preTime;
    }
    else {
        curTime += ((MMP_ULONG) -1) - preTime + 1;
    }

    if (curTime < 5000) {
        return;
    }

    MMPF_OS_GetTime(&preTime);

    for (Register = 0; Register < ulBytes; Register += 4) {
        if ((Register % 16) == 0) {
            RTNA_DBG_Str(0, "\r\n");
            RTNA_DBG_Long(0, ulStartAddress + Register);
            RTNA_DBG_Str(0, ": ");
        }

        RTNA_DBG_Long(0, *(AIT_REG_D *) (ulStartAddress + Register));
    }

    RTNA_DBG_Str(0, "\r\n========================================Register Dump End\r\n\r\n");
}

//------------------------------------------------------------------------------
//  Function    : AHC_CheckSysCalibMode
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_CheckSysCalibMode(void)
{
    UINT32  ulFileId;
    AHC_ERR error = AHC_ERR_NONE;

    if (AHC_IsSDInserted() == AHC_FALSE) {
        printc("Check FactoryModeFile - No SD\r\n");
        return AHC_FALSE;
    }

    error = AHC_FS_FileOpen("SD:\\Test\\FACTORY.CAL", AHC_StrLen("SD:\\Test\\FACTORY.CAL"), "rb", AHC_StrLen("rb"), &ulFileId);
    
    if (error == AHC_ERR_NONE) {
        AHC_FS_FileClose(ulFileId);
        printc("FactoryModeFile FACTORY.CAL was found in SD\r\n");
        return AHC_TRUE;
    }
    else {
        printc("No FactoryModeFile FACTORY.CAL in SD\r\n");
    }

    return AHC_FALSE;
}

#if (SUPPORT_HDMI_OUT_FOCUS)
//------------------------------------------------------------------------------
//  Function    : AHC_CheckHdmiOutFocus
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_CheckHdmiOutFocus(void)
{
    UINT32  ulFileId;
    AHC_ERR error = AHC_ERR_NONE;

    if (AHC_IsSDInserted() == AHC_FALSE) {
        printc("Check HdmiOutFousFile - No SD\r\n");
        return AHC_FALSE;
    }

    error = AHC_FS_FileOpen(CFG_HDMI_OUT_FOCUS_FILE, AHC_StrLen(CFG_HDMI_OUT_FOCUS_FILE), "rb", AHC_StrLen("rb"), &ulFileId);
    
    if (error == AHC_ERR_NONE) {
        AHC_FS_FileClose(ulFileId);
        printc("HdmiOutFousFile %s was found in SD\r\n", CFG_HDMI_OUT_FOCUS_FILE);
        return AHC_TRUE;
    }
    else {
        printc("No HdmiOutFousFile %s in SD\r\n", CFG_HDMI_OUT_FOCUS_FILE);
    }

    return AHC_FALSE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_RestoreFromDefaultSetting
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_RestoreFromDefaultSetting(void)
{
    #ifdef FACTORY_RESET
    AHC_SendAHLMessage(AHLM_GPIO_BUTTON_NOTIFICATION, FACTORY_RESET, 0);
    #endif
    
    return AHC_TRUE;
}

#if (SD_UPDATE_FW_EN)
//------------------------------------------------------------------------------
//  Function    : AHC_SDUpdateMode
//  Description :
//------------------------------------------------------------------------------
void AHC_SDUpdateMode(void)
{
    SD_UPDATER_ERR eError;

    printc("\n SDUpdate \r\n");

    AHC_SetMode(AHC_MODE_IDLE);

    eError = SDUpdateCheckFileExisted(SD_FW_UPDATER_BIN_NAME);

    if (eError != SD_UPDATER_ERR_FILE)
    {
        // Update Success
    //    uiPowerOffPicture(); // lyj 20191018
        AHC_PMU_PowerOff();
    }

    #if (EDOG_ENABLE)
    eError = SDUpdateCheckDBExisted(SD_DB_UPDATER_BIN_NAME);

    if (eError != SD_UPDATER_ERR_FILE)
    {
        if (eError == SD_UPDATER_ERR_FAIL)
            AHC_OS_Sleep(3000);

        // Update Success
      //  uiPowerOffPicture(); // lyj 20191018
        AHC_PMU_PowerOff();
    }
    #endif
}
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_UnloadSystemFile
//  Description : Write the system required file back to the SD card
//------------------------------------------------------------------------------
AHC_BOOL AHC_UnloadSystemFile(void)
{
    AHC_UF_WriteInfo();
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : MMP_3GPMERGR_OffsetError_CB
//  Description : This function will be called when offset value of video frame is wrong. (Only for AVI format.)
//  Return      : 0: Continue recording in driver, but following frames in current file will have broken image.
//                1: Stop recording in driver, but the file will be not closed correctly.
//                2: Dump debug messages, and system will hang. (Default: Disabled)
//------------------------------------------------------------------------------
MMP_ULONG MMP_3GPMERGR_OffsetError_CB(void)
{
    static MMP_ULONG ulPreviousTime = 0;
    MMP_ULONG ulCurrentTime = ((UINT64)OSTimeGet() * 1000) / OS_TICKS_PER_SEC;
    
    if( (ulCurrentTime - ulPreviousTime) < 500 ) // SW debounce to avoid too many callback in a short time
    {
        return 0; // Continue recording in driver.
    }
    ulPreviousTime = ulCurrentTime;

    printc(FG_GREEN("<MMP_3GPMERGR_OffsetError_CB>\r\n"));

    #if 1 //Send an Event to stop recording and re-start to record (Now "borrow" slow card message.)
    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_RECDSTOP_CARDSLOW, 0);
    return 0; // Continue recording in driver.
    #else //For Debug
    return 2; //Dump debug messages, and system will hang.
    #endif
}

