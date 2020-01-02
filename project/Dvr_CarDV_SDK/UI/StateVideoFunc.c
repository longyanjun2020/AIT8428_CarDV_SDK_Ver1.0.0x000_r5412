/*===========================================================================
 * Include files
 *===========================================================================*/

#include "Customer_config.h"
#include "AHC_Common.h"
#include "AHC_General.h"
#include "AHC_Message.h"
#include "AHC_Display.h"
#include "AHC_Audio.h"
#include "AHC_Capture.h"
#include "AHC_Dcf.h"
#include "AHC_UF.h"
#include "AHC_USBHost.h"
#include "AHC_Media.h"
#include "AHC_Video.h"
#include "AHC_Menu.h"
#include "AHC_Os.h"
#include "AHC_Fs.h"
#include "AHC_Gsensor.h"
#include "AHC_Isp.h"
#include "AHC_Parameter.h"
#include "AHC_Warningmsg.h"
#include "AHC_Usb.h"
#include "AHC_Stream.h"
#include "AHC_General_CarDV.h"
#include "AHC_General.h"
#include "AIHC_Dcf.h"
#include "AIT_Utility.h"
#include "IconPosition.h"
#include "ZoomControl.h"
#include "StateVideoFunc.h"
#include "StateHDMIFunc.h"
#include "StateTVFunc.h"
#include "MenuCommon.h"
#include "MenuDrawCommon.h"
#include "MenuTouchButton.h"
#include "MenuSetting.h"
#include "UI_DrawGeneral.h"
#include "DrawStateVideoFunc.h"
#include "DrawStateCameraFunc.h"
#include "KeyParser.h"
#include "LedControl.h"
#include "dsc_charger.h"
#include "lib_retina.h"
#if (GSENSOR_CONNECT_ENABLE)
#include "GSensor_ctrl.h"
#endif
#if (GPS_CONNECT_ENABLE)
#include "GPS_ctl.h"
#endif
#if (TOUCH_UART_FUNC_EN)
#include "Touch_Uart_ctrl.h"
#endif
#include "disp_drv.h"
#include "MenuDrawingFunc.h"
#include "mmps_iva.h"
#include "mmps_fs.h"
#include "mmps_3gprecd.h"
#include "mmps_sensor.h"
#include "AHC_USBHost.h"
#include "snr_cfg.h"
#include "ldc_cfg.h"
#include "vidrec_cfg.h"
#if (SUPPORT_UVC_FUNC)
#include "pCam_api.h"
#endif
#if defined(WIFI_PORT) && (WIFI_PORT == 1)
#include "amnss_mmobjs.h"
#include "wlan.h"
#include "netapp.h"
#endif

#if (ENABLE_ADAS_LDWS)
#include "ldws_cfg.h"
#include "mmps_aui.h"
#include "mmps_adas.h"
#endif
#include "hdr_cfg.h"
#include "AHC_Sensor.h"
#include "Driving_Safety.h"

#if (USB_EN)&&(SUPPORT_USB_HOST_FUNC)
#include "mmpf_usbh_uvc.h"
#endif
#include "mmpf_jpeg_ctl.h"

#include "AHC_ADAS.h"

#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE == 1)
#include "mmpf_usbuvc.h"
#endif

#include "ParkingModeCtrl.h"
#include "dram_cfg.h"
/*===========================================================================
 * Macro define
 *===========================================================================*/

/* Lock File functions */
//  Export functions
AHC_BOOL VideoFunc_LockFileEnabled(void);
//  Internal Functions & Variables
static AHC_BOOL     _bLockVRFile = AHC_FALSE;
static int          _nLockType;
UINT32			ulCurLockTime ;
static void EnableLockFile(AHC_BOOL bEnable, int type);
static void LockFileTypeChange(int arg);
/* Lock File functions */

// The definition is at mmpf_3gpmgr.c also.

/*===========================================================================
 * Macro define
 *===========================================================================*/
#define PARKING_RECORD_FORCE_20SEC              (0)

#define STORAGE_MIN_SIZE                    (2*1024*1024)
#define PRE_RECORD_STORAGE_MIN_SIZE         (4*1024*1024)
#define VIDEO_TIMER_UNIT                    (100)//unit :ms
#define TICKS_PER_SECOND                    (1000)
#define TICKS_PER_MINUTE                    (60 * 1000)

#define MOTION_DETECTION_STABLE_TIME_PARKING_NONE 	10
#define MOTION_DETECTION_STABLE_TIME_PARKING	 	2
       
/*===========================================================================
 * Global variable
 *===========================================================================*/

MOVIESIZE_SETTING           VideoRecSize            = MOVIE_SIZE_NUM;
AHC_BOOL		            bVideoRecording         = AHC_FALSE;
UINT8                       bVideoPreRecordStatus   = AHC_FALSE;
AHC_BOOL                    bAudioRecording         = AHC_FALSE;
UINT8                       VideoTimerID            = 0xFF;
UINT32                      VideoCounter            = 0;
UINT32                      RecordTimeOffset        = 0;
AHC_BOOL                    bShowHdmiWMSG           = AHC_TRUE;
AHC_BOOL                    bShowTvWMSG             = AHC_TRUE;

AHC_BOOL                    bMuteRecord             = AHC_FALSE;
AHC_BOOL                    bDisableVideoPreRecord  = AHC_FALSE;
UINT32                      m_uiSlowMediaCBCnt      = 0;
AHC_BOOL                    bNightMode              = AHC_FALSE;
AHC_BOOL                    bGPS_PageExist          = AHC_FALSE;

#if (LIMIT_MAX_LOCK_FILE_NUM)
UINT32                      m_ulLockFileNum         = 0;
UINT32                      m_ulLockEventNum        = 0;
#endif

#if (LIMIT_MAX_LOCK_FILE_TIME)
UINT32                      ulVRTotalTime           = 0;
#endif

#if (VIDEO_DIGIT_ZOOM_EN==1)
static AHC_BOOL             bZoomStop               = AHC_TRUE;
#endif

#if (GSENSOR_CONNECT_ENABLE || GPS_CONNECT_ENABLE)
static UINT32               ulCounterForGpsGsnrUpdate;
#endif

#if (MOTION_DETECTION_EN)
AHC_BOOL                    m_ubInRemindTime        = AHC_FALSE;
#ifdef CFG_REC_CUS_VMD_REMIND_TIME
UINT32                      m_ulVMDRemindTime       = CFG_REC_CUS_VMD_REMIND_TIME;
#else
UINT32                      m_ulVMDRemindTime       = 10;
#endif
UINT32                      m_ulVMDCloseBacklightTime  = 41;
AHC_BOOL                    m_ubVMDStart            = AHC_FALSE;
AHC_BOOL                    m_ulVMDCancel           = AHC_FALSE;
UINT32               		m_ulVMDStableCnt        = 0;  // This counter is used to wait AE stable after enter Pre-recording mode
static UINT32               m_ulVMDStopCnt          = 0;
#endif
#ifdef CFG_ENABLE_VIDEO_LASER_LED
UINT32                      m_ulLaserTime           = 10*10; //10s
AHC_BOOL                    m_ubLaserStart          = AHC_FALSE;
static UINT32               m_ubLaserStop           = AHC_FALSE;
#endif

#if (VR_PREENCODE_EN)
AHC_BOOL                    m_ubPreRecording        = AHC_FALSE;
AHC_BOOL                    m_ubPreEncodeEn         = AHC_FALSE;
#endif

#if (SUPPORT_TOUCH_PANEL)
AHC_BOOL                    m_ubShowVRTouchPage     = AHC_FALSE;
#endif

#if (SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
UINT32                      m_ulGSNRRecStopCnt      = POWER_ON_GSNR_MOVIE_TIME * 10;
#endif

AHC_BOOL                    m_ubParkingModeRecTrigger = AHC_FALSE;

UINT32               m_ulEventPreRecordTime = 0;
UINT32               m_ulEventHappenTime = 0;

AHC_BOOL                    m_bCurrentTimeLessThanPreRecord = AHC_FALSE;
#if (ENABLE_ADAS_LDWS)
UINT32                      m_ulLDWSWarn = 0;
#endif
#if (ENABLE_ADAS_FCWS)
UINT32                      m_ulFCWSWarn = 0;
#endif
#if (ENABLE_ADAS_SAG)
UINT32                      m_ulSAGWarn = 0;
#endif

static UINT8                gbWinExchangedCnt = F_LARGE_R_SMALL;

#if (UPDATE_UI_USE_MULTI_TASK)
AHC_BOOL                    m_ubUpdateUiByOtherTask = AHC_FALSE;
#endif
AHC_BOOL                    gbVideoInSubMode = AHC_FALSE;
AHC_BOOL                    gbRearPreviewNeedResetBuf = AHC_FALSE;

AHC_BOOL                    gbVideoTimerEventBusy = AHC_FALSE;

#if (AHC_SHAREENC_SUPPORT)
UINT32                      m_ulDualPreRecordTime = 0;
UINT32                      m_ulDualHappenTime = 0;
#endif

/*===========================================================================
 * Extern variable
 *===========================================================================*/

extern UINT8                bZoomDirect;
extern AHC_BOOL             bForce_PowerOff;
extern UINT8                m_ubDSCMode;
extern UINT8                m_ubSDMMCStatus;
#if (AUTO_HIDE_OSD_EN)
extern AHC_BOOL             m_ubHideOSD;
extern UINT32               m_ulOSDCounter;
#endif

#if (MOTION_DETECTION_EN)
extern AHC_BOOL             m_ubMotionDtcEn;
#endif

#if (GPS_CONNECT_ENABLE)
extern GPS_ATTRIBUTE        gGPS_Attribute;
#endif

#if (SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
extern AHC_BOOL             ubGSnrPowerOn;
extern AHC_BOOL             ubGsnrPwrOnActStart;
extern AHC_BOOL             ubGsnrPwrOnFirstREC;
extern AHC_BOOL             m_ubGsnrIsObjMove;
#endif

#if (GSENSOR_CONNECT_ENABLE)
extern AHC_BOOL             m_ubGsnrIsObjMove;
#endif

extern AHC_BOOL             gbAhcDbgBrk;

#if (USB_MODE_SELECT_EN)
extern UINT8	 ubUSBSelectedMode;
#endif

#if (AHC_SHAREENC_SUPPORT)
extern AHC_BOOL m_bShareRecPostDone;
extern AHC_BOOL m_bFirstShareFile;
#endif

extern MMP_BOOL	g_bDrawUnfix;//use to draw GPS unfix message onetime.
#if defined(PCAM_UVC_MIX_MODE_ENABLE) && PCAM_UVC_MIX_MODE_ENABLE
extern AHC_BOOL m_ubFormatSDing;
#endif

extern MMP_USHORT   gsAhcPrmSensor;
extern MMP_USHORT   gsAhcScdSensor;
#if(UVC_HOST_VIDEO_ENABLE)
extern MMP_USHORT   gsAhcUsbhSensor;
#endif
/*===========================================================================
 * Extern Function
 *===========================================================================*/
#if (SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
extern UINT32               AHC_GSNR_PWROn_MovieTimeReset(void);
#endif

extern void Oem_Switch_To_TVOUT_Mode(void);
extern void InitOSD(void);
/*===========================================================================
 * Main body
 *===========================================================================*/
extern void SetUITimeEvent(void);

#if (ENABLE_ADAS_LDWS)
void ResetLDWSCounter(void)
{
	m_ulLDWSWarn = 0;
}
#endif

#if (ENABLE_ADAS_FCWS)
void ResetFCWSCounter(void)
{
	m_ulFCWSWarn = 0;
}
#endif

#if (ENABLE_ADAS_SAG)
void ResetSAGCounter(void)
{
	m_ulSAGWarn = 0;
}
#endif

void VideoTimer_Event_Busy(AHC_BOOL bBusy)
{
    gbVideoTimerEventBusy = bBusy;
}

AHC_BOOL VideoTimer_Event_IsBusy(void)
{
    return gbVideoTimerEventBusy;
}

void VideoTimer_ISR(void *tmr, void *arg)
{
    /*
     * if want to display current time/recording time and update it by every second.
     * not to define CFG_REC_CUS_EVENT_PERIOD.
     *                                          Canlet
     */
    {
        static unsigned int     _kt      = 0;
        AHC_RTC_TIME            sRtcTime;

        AHC_RTC_GetTime(&sRtcTime);

        if (_kt != sRtcTime.uwSecond) {
            _kt = sRtcTime.uwSecond;
            // time updated to compensate timer error by Shadow RTC
            VideoCounter = 0;
            SetUITimeEvent();
        }
    }

#if (ENABLE_ADAS_LDWS)
	{
	    #if 0
		if (LDWS_IsStart() == AHC_TRUE)
		{
			m_ulLDWSWarn++;
			if (m_ulLDWSWarn == (4*5)) // period = 250msec.
			{
				m_ulLDWSWarn = 0;
				//LDWS_Unlock();
				AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_LDWS_STOP, 0);
			}
		}
		#endif

		if(m_ulLDWSWarn == 1) {
			m_ulLDWSWarn = 0;
			AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_LDWS_STOP, 0);
		} else if(m_ulLDWSWarn > 0) {
			m_ulLDWSWarn--;
		}
	}
#endif

#if (ENABLE_ADAS_FCWS)
	if(m_ulFCWSWarn == 1) {
		m_ulFCWSWarn = 0;
		AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_FCWS_STOP, 0);
	} else if(m_ulFCWSWarn > 0) {
		m_ulFCWSWarn--;
	}
#endif
#if (ENABLE_ADAS_SAG)
    if (m_ulSAGWarn == 1){
        m_ulSAGWarn = 0;
        AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_SAG_STOP, 0);
    }
    else if (m_ulSAGWarn > 0){
        m_ulSAGWarn--;
	}
#endif

    #ifdef CFG_ENABLE_VIDEO_LASER_LED
    if((!m_ubLaserStart)&&(VideoFunc_RecordStatus()))
    {
        RTNA_DBG_Str(0, "m_ubLaserStart \r\n");
        AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_FUNC2_LPRESS, 0);
        m_ubLaserStart = AHC_TRUE;
    }
    else if((VideoFunc_RecordStatus())&&(m_ulLaserTime>0)&&(m_ubLaserStart))
    {
        m_ulLaserTime--;
        if(m_ulLaserTime==0)
        {
            m_ubLaserStop = AHC_TRUE;
            RTNA_DBG_Str(0, "m_ubLaserStop \r\n");
            AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_FUNC2_LPRESS, 0);
        }
    }
    #endif

    #if defined(CFG_WIFI_TIMEOUT_OFF) && defined(WIFI_PORT) && (WIFI_PORT == 1)
    {
        static  MMP_USHORT cnt;

        if ((nhw_get_status() != 0) && (NETAPP_NET_STATUS_NONE != nhw_get_status()))
        {
            if (AHC_CheckWiFiOnOffInterval(ulWiFiSwitchToggleInterval) == 0)
            {
                if (AHC_Get_WiFi_Streaming_Status())
                {
                    cnt = 0;
                }
                else
                {
                    cnt++;
                    if (cnt>(10*3*60))  //100*10ms
                    {  // TurnOffWiFiModule
                        //AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_REC_REL, 0);  // TBD
                        cnt = 0;
                    }
                }
            }
        }
        else
        {
            cnt = 0;
        }
    }
    #endif

    if( VideoCounter == 0)
    {
        #ifdef CFG_REC_CUS_EVENT_PERIOD //may be defined in config_xxx.h, must not == 0
        VideoCounter = CFG_REC_CUS_EVENT_PERIOD;
        #else
        VideoCounter = 5;
        #endif
		#if (UPDATE_UI_USE_MULTI_TASK)
		//if(m_ubUpdateUiByOtherTask)
			//SetUIUpdateEvent();
		//else
			//AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_UPDATE_MESSAGE, 0);
		//#else
        AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_UPDATE_MESSAGE, 0);
		#endif
		#if (TOUCH_UART_FUNC_EN)
		#if (DTV_UART_SEND_LDWS)
		if(DTV_UART_Get_LDWS())	
			AHC_OS_SetFlags(UartCtrlFlag, Touch_Uart_FLAG_SETPACK, AHC_OS_FLAG_SET);
		#endif //#if (DTV_UART_SEND_LDWS)		
		
		#if (DTV_UART_SEND_FCWS)
		if(DTV_UART_Get_FCWS())	
			AHC_OS_SetFlags(UartCtrlFlag, Touch_Uart_FLAG_SETPACK, AHC_OS_FLAG_SET);		
		#endif  //#if (DTV_UART_SEND_FCWS)
		#endif  //#if (TOUCH_UART_FUNC_EN)
        #ifdef VMD_EN_BY_CHARGER_OUT
        if(m_ubMotionDtcEn)
        {
            AutoPowerOffCounterReset();
            LCDPowerSaveCounterReset();
            VideoPowerOffCounterReset();
        }
        #endif
    }

    VideoCounter--;

#if((SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN) && \
    (GSNR_PWRON_REC_BY && (GSNR_PWRON_REC_BY_SHAKED || GSNR_PWRON_REC_BY_VMD)))
    if(ubGsnrPwrOnActStart)
    {
        LCDPowerSaveCounterReset();
        VideoPowerOffCounterReset();
        if(m_ulGSNRRecStopCnt && VideoFunc_RecordStatus())
        {
            m_ulGSNRRecStopCnt--;

            if(m_ulGSNRRecStopCnt==0)
            {
                if(POWER_ON_GSNR_IDLE_TIME == 0)
                {
                    bForce_PowerOff = AHC_TRUE;
                    AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, KEY_POWER_OFF, 0);
                    m_ulGSNRRecStopCnt = 0;
                }
                else
                {
                    AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, KEY_VIDEO_RECORD, EVENT_VRCB_MOTION_DTC);
                    AutoPowerOffCounterReset();
                }
            }
        }
    }
    else
#endif
    {
        #if (MOTION_DETECTION_EN) && (VMD_ACTION & VMD_RECORD_VIDEO)
        if (m_ulVMDStopCnt && m_ubVMDStart)
        {
            m_ulVMDStopCnt--;

            if (m_ulVMDStopCnt == 0) {
                // Stop Video Record in VMD mode.
                // Please make sure EVENT - BUTTON_REC_PRESS is to stop video recording
                AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, KEY_VIDEO_RECORD, EVENT_VRCB_MOTION_DTC);
            }
        }
        #endif

        #if (MOTION_DETECTION_EN)
        if( m_ulVMDStableCnt > 0 )
        {
            m_ulVMDStableCnt--;
            if( m_ulVMDStableCnt == 0 )
            {
                m_ubVMDStart = AHC_TRUE; // Really start to detect motions
            }
        }
        #endif
    }

    #if ( ((GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO))     || \
          ((GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)) )
    if((VideoFunc_RecordStatus() || uiGetParkingModeEnable() == AHC_TRUE || AHC_VIDEO_IsEmergRecStarted() == AHC_TRUE) && AHC_IsSDInserted())
    {
        AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_GPSGSENSOR_UPDATE, 0);
    }
    else
    {
        if(AHC_IsSDInserted())
            ulCounterForGpsGsnrUpdate = 0;
    }
    #endif
}

AHC_BOOL VideoTimer_Start(UINT32 ulTime)
{
    if (0xFE <= VideoTimerID) {
        VideoCounter = 0;
        VideoTimerID = AHC_OS_StartTimer( ulTime, AHC_OS_TMR_OPT_PERIODIC, VideoTimer_ISR, (void*)NULL );
		printc(FG_GREEN("VideoTimer_Start: VideoTimerID =0x%X\r\n"), VideoTimerID);
        if(0xFE <= VideoTimerID) {
            printc(FG_RED("Start Video Timer fail - 0x%X!!!")"\r\n", VideoTimerID);
            return AHC_FALSE;
        }
    }

    #ifdef CFG_CUS_VIDEO_TIMER_START
    CFG_CUS_VIDEO_TIMER_START();
    #endif

    return AHC_TRUE;
}

AHC_BOOL VideoTimer_Stop(void)
{
    UINT8 ret = 0;
    UINT32 ulTimeout = 0x1000000;
    AHC_BOOL bVideoTimerBusy = AHC_FALSE;
    
    #ifdef CFG_CUS_VIDEO_TIMER_STOP
    CFG_CUS_VIDEO_TIMER_STOP();
    #endif

    if (0xFE > VideoTimerID) {
        printc(FG_GREEN("VideoTimer_Stop: VideoTimerID =0x%X\r\n"), VideoTimerID);
        
        ret = AHC_OS_StopTimer( VideoTimerID, AHC_OS_TMR_OPT_PERIODIC );
        VideoTimerID = 0xFF;

        do{

            bVideoTimerBusy = VideoTimer_Event_IsBusy();
        }while((AHC_TRUE == bVideoTimerBusy) && (--ulTimeout > 0));

        printc(FG_RED("\r\n\r\nStop video timer wait time:%d\r\n"), 0x1000000-bVideoTimerBusy);
        
        if(0 == ulTimeout){
            AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0);
        }
        
        if(0xFF == ret) {
            printc(FG_RED("Stop Video Timer fail !!!")"\r\n");
            return AHC_FALSE;
        }
    }

    return AHC_TRUE;
}

AHC_BOOL VideoFunc_SetAttribute(void)
{
    UINT16      zoomratio      = 4;

    AHC_BOOL ahcRet = AHC_TRUE;

    AHC_UF_SetFreeChar(0, DCF_SET_FREECHAR, (UINT8 *) VIDEO_DEFAULT_FLIE_FREECHAR);

    VideoFunc_PresetSensorMode(MenuSettingConfig()->uiMOVSize);
    VideoFunc_SetResolution(MenuSettingConfig()->uiMOVSize);
    VideoFunc_PresetFrameRate(MenuSettingConfig()->uiMOVSize);

    AHC_SetPreviewZoomConfig(ZoomCtrl_GetVideoDigitalZoomMax(), (UINT8)zoomratio);

    return ahcRet;
}

AHC_BOOL VideoFunc_SetPreviewWindow(AHC_BOOL bPreSet)
{
    UINT8                       bRotate = 0;
    UINT16                      dw, dh, rearW, rearH;
    MMP_ULONG                   ScaleInW, ScaleInH;
    AHC_WINDOW_RECT             sRearRect;
    AHC_DISPLAY_OUTPUTPANEL	    outputPanel;
    AHC_BOOL                    ahcRet = AHC_TRUE;

#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0) 
    bRotate = 0;
#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
    bRotate = 1;
#endif
   
#if (TVOUT_PREVIEW_EN)
    if (AHC_IsTVConnectEx())
    {
        bRotate = 0;

        if (MenuSettingConfig()->uiTVSystem == TV_SYSTEM_PAL)
        {
            AHC_SetDisplayOutputDev(DISP_OUT_TV_PAL, AHC_DISPLAY_DUPLICATE_1X);
            AHC_GetPalTvDisplayWidthHeight(&dw, &dh);
        }
        else
        {
            AHC_SetDisplayOutputDev(DISP_OUT_TV_NTSC, AHC_DISPLAY_DUPLICATE_1X);
            AHC_GetNtscTvDisplayWidthHeight(&dw, &dh);
        }
    }
    else
#endif
#if (HDMI_PREVIEW_EN)
    if (AHC_IsHdmiConnect())
    {
        bRotate = 0;

        AHC_GetHdmiDisplayWidthHeight(&dw, &dh);
    }
    else
#endif
    {
        dw = RTNA_LCD_GetAttr()->usPanelW;
        dh = RTNA_LCD_GetAttr()->usPanelH;
    }

    if (bPreSet) {
        
        gbRearPreviewNeedResetBuf = AHC_FALSE;

        #if (ENABLE_SET_YUV_ATTRIBUTE)
        MMPS_Display_SetYuvAttribute(YUV_U2U, YUV_V2U, YUV_U2V, YUV_V2V, YUV_YGAIN, YUV_YOFFSET);
        #endif
        
       
        if (MMP_IsPrmCamExist()) {
            if (GET_VR_PREVIEW_WINDOW(gsAhcPrmSensor) == UPPER_IMAGE_WINDOW_ID)
                AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_PRM, AHC_TRUE, bRotate, 0, 0, dw >> 1, dh >> 1, 0);
            else
                AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_PRM, AHC_TRUE, bRotate, 0, 0, dw, dh, 0);
        }
        
        if (MMP_IsUSBCamExist()) {
        
            #if (TVOUT_PREVIEW_EN || HDMI_PREVIEW_EN)
            if (AHC_IsTVConnectEx() || AHC_IsHdmiConnect()){
                AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_UVC, AHC_TRUE, bRotate, 0, 0/*dh >> 1*/, dw >> 1, dh >> 1, 0);
            }
            else
            #endif
            {
                #if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
                #if(UVC_HOST_VIDEO_ENABLE)
                if (GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor) == UPPER_IMAGE_WINDOW_ID)
                    if (!CAM_CHECK_PRM(PRM_CAM_NONE))
                    AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_UVC, AHC_TRUE, bRotate, 0, 0, dw >> 1, dh >> 1, 0);
                    else {
                        if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
                            AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_UVC, AHC_TRUE, bRotate, 0, 0, dw, dh, 0);
                    }
                else
                    AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_UVC, AHC_TRUE, bRotate, 0, 0, dw, dh, 0);
                #endif
                #elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90)
                AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_UVC, AHC_TRUE, bRotate, 0, 0, dw >> 1, dh >> 1, 0);
                #elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
                AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_UVC, AHC_TRUE, bRotate, dw >> 1, 0, dw >> 1, dh >> 1, 0);
                #endif
            }
        }
        
        if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) || 
            (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
            
            // For Sticker. Need initial encode resolution here.
            AHC_VIDEO_SetDualEncSetting();
             
            AHC_GetDisplayOutputDev(&outputPanel);
        
            if (outputPanel == MMP_DISPLAY_SEL_CCIR) {
                AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_SCD, AHC_TRUE, bRotate, 0, 0, dw, dh, 0);
            }
            else {
                rearW = dw >> 1;
                rearH = dh >> 1;
                
                MMPS_Sensor_GetCurPrevScalInputRes(gsAhcScdSensor, &ScaleInW, &ScaleInH);
                
                if ((!AHC_IsHdmiConnect()) && (!AHC_IsTVConnectEx()))
                {
                #if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
                    if (((ScaleInW > rearW) && (ScaleInH < rearH)) || 
                        ((ScaleInW < rearW) && (ScaleInH > rearH)))
                    {
                        gbRearPreviewNeedResetBuf = AHC_TRUE;
                        RTNA_DBG_Str(0, FG_YELLOW("Scaler HW limitation: one side scale up, one side down \r\n"));
                        RTNA_DBG_Str(0, FG_YELLOW("Re-arrange rear cam buffer size to scaler-in W x H \r\n"));
                        AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_SCD, AHC_TRUE, bRotate, 0, 0, ScaleInW, ScaleInH, 0);
                    }
                #else
                    if (((ScaleInW > rearH) && (ScaleInH < rearW)) || 
                        ((ScaleInW < rearH) && (ScaleInH > rearW)))
                    {
                        gbRearPreviewNeedResetBuf = AHC_TRUE;
                        RTNA_DBG_Str(0, FG_YELLOW("Scaler HW limitation: one side scale up, one side down \r\n"));
                        RTNA_DBG_Str(0, FG_YELLOW("Re-arrange rear cam buffer size to scaler-in W x H \r\n"));
                        AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_SCD, AHC_TRUE, bRotate, 0, 0, ScaleInH, ScaleInW, 0);
                    }
                #endif
                    else
                    {
                        if (GET_VR_PREVIEW_WINDOW(gsAhcScdSensor) == UPPER_IMAGE_WINDOW_ID)
                            AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_SCD, AHC_TRUE, bRotate, 0, 0, rearW, rearH, 0);
                        else
                            AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_SCD, AHC_TRUE, bRotate, 0, 0, dw, dh, 0);
                    }
                }
                else
                {
                    if (((ScaleInW > rearW) && (ScaleInH < rearH)) || 
                        ((ScaleInW < rearW) && (ScaleInH > rearH)))
                    {
                        gbRearPreviewNeedResetBuf = AHC_TRUE;
                        RTNA_DBG_Str(0, FG_YELLOW("Scaler HW limitation: one side scale up, one side down \r\n"));
                        RTNA_DBG_Str(0, FG_YELLOW("Re-arrange rear cam buffer size to scaler-in W x H \r\n"));
                        AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_SCD, AHC_TRUE, bRotate, 0, 0, ScaleInW, ScaleInH, 0);
                    }
                    else
                    {
                        AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_SCD, AHC_TRUE, bRotate, 0, rearH, rearW, rearH, 0);
                    }
                }
            }
        }
    }
    else {
        if (gbRearPreviewNeedResetBuf == AHC_TRUE) {
            rearW = dw >> 1;
            rearH = dh >> 1;

            RTNA_DBG_Str(0, FG_YELLOW("Use display scaling to achieve desired W & H\r\n"));
            
            AHC_PreviewWindowOp(AHC_PREVIEW_WINDOW_OP_GET | AHC_PREVIEW_WINDOW_REAR, &sRearRect);
            
            MMPS_Display_SetWinScaleAndOffset(GET_VR_PREVIEW_WINDOW(gsAhcScdSensor), 
                                              MMP_SCAL_FITMODE_OPTIMAL,
                                              sRearRect.usWidth, sRearRect.usHeight, rearW, rearH,
                                              sRearRect.usLeft, sRearRect.usTop);
        }
    }

    #if (AHC_SHAREENC_SUPPORT)
    AHC_VIDEO_SetShareRecdRes(VIDRECD_RESOL_640x480);
    #endif
    
    return ahcRet;
}

AHC_BOOL VideoFunc_IsShareRecordStarted(void)
{
    #if (AHC_SHAREENC_SUPPORT)
    return AHC_VIDEO_IsShareRecStarted();
    #endif
    return AHC_FALSE;
}

#if (AHC_SHAREENC_SUPPORT)
AHC_BOOL VideoFunc_StartShareRecord(void)
{
    AHC_BOOL ret = AHC_TRUE;

    UINT32 ulCurrentRecordedTime = 0;

    AHC_VIDEO_GetCurRecordingTime(&ulCurrentRecordedTime);
    AHC_VIDEO_GetShareRecTimeOffset(&m_ulDualPreRecordTime);
    m_ulDualHappenTime = OSTimeGet();
    
    printc(FG_PURPLE("VideoFunc_StartShareRecord-- %d ms\r\n"), ulCurrentRecordedTime);
    DBG_AutoTestPrint(ATEST_ACT_1CLICKSHARE_0x000A, ATEST_STS_START_0x0001, 0, 0, gbAhcDbgBrk);
  	#if (GPS_CONNECT_ENABLE && (GPS_FUNC & FUNC_VIDEOSTREM_INFO))
  	AHC_GPS_SetSHAREIndex();  	
  	#endif
  	#if (GSENSOR_CONNECT_ENABLE && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO))
    AHC_Gsensor_SetSHAREIndex();
  	#endif
    ret = AHC_VIDEO_StartShareRecord();
	return ret;
}

AHC_BOOL VideoFunc_StopShareRecord(void)
{

    AHC_VIDEO_StopShareRecord();

    return AHC_TRUE;
}
#endif

AHC_BOOL VideoFunc_PreRecordStatus(void)
{
#if (VR_PREENCODE_EN)
    return m_ubPreRecording;
#else
    return AHC_FALSE;
#endif
}

AHC_BOOL VideoFunc_RecordStatus(void)
{
    return bVideoRecording;
}

AHC_BOOL VideoFunc_GPSPageStatus(void)
{
    return bGPS_PageExist;
}

UINT32 VideoFunc_GetRecordTimeOffset(void)
{
    return RecordTimeOffset;
}

void VideoFunc_GetFreeSpace(UINT64 *pFreeBytes)
{
    AHC_Media_GetFreeSpace(pFreeBytes);
}

void VideoFunc_SetResolution(MOVIESIZE_SETTING ubResolution)
{
    VideoRecSize = ubResolution;

    //printc("[VideoFunc_SetResolution] VideoRecSize = %d\r\n",VideoRecSize);
    
    switch(VideoRecSize)
    {
        #if (MENU_MOVIE_SIZE_4K_24P_EN)
		case MOVIE_SIZE_4K_24P:
			AHC_SetImageSize(VIDEO_CAPTURE_MODE, 3200, 1808/*1800*/);
		break;
		#endif
        #if (MENU_MOVIE_SIZE_1440_30P_EN)
        case MOVIE_SIZE_1440_30P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 2560, 1440);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_SHD_30P_EN)
        case MOVIE_SIZE_SHD_30P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 2304, 1296);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_SHD_25P_EN)
        case MOVIE_SIZE_SHD_25P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 2304, 1296);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_SHD_24P_EN)
        case MOVIE_SIZE_SHD_24P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 2304, 1296);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_1080_60P_EN)
        case MOVIE_SIZE_1080_60P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1920, 1088);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_1080_24P_EN)
        case MOVIE_SIZE_1080_24P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1920, 1088);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_1080P_EN)
        case MOVIE_SIZE_1080P:
            if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
                if (MMP_GetParallelFrmStoreType() != PARALLEL_FRM_NOT_SUPPORT) {
                    if (MMP_GetDualSnrEncodeType() == DUALSNR_DUAL_ENCODE) {
                        AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1088, 1088);
                    }
                    else {
                        if (MMP_IsVidLdcSupport())
                            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1472, 736);
                        else
                            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 2176, 1088);
                    }
                }
                else {
                    if (MMP_IsVidLdcSupport())
                        AHC_SetImageSize(VIDEO_CAPTURE_MODE, 736, 736);
                    else
                        AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1088, 1088);
                }
            }
            else {
                AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1920, 1088);
            }
        break;
        #endif
	    #if (MENU_MOVIE_SIZE_1080P_30_HDR_EN)
        case MOVIE_SIZE_1080_30P_HDR:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1920, 1088);
        break;
        #endif
	    #if (MENU_MOVIE_SIZE_900P_30P_EN)
        case MOVIE_SIZE_900P_30P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1600, 912);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_960P_30P_EN)
        case MOVIE_SIZE_960P_30P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1280, 960);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720P_EN)
        case MOVIE_SIZE_720P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1280, 720);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720_60P_EN)
        case MOVIE_SIZE_720_60P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1280, 720);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720_24P_EN)
        case MOVIE_SIZE_720_24P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1280, 720);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720_120P_EN)
        case MOVIE_SIZE_720_120P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 1280, 720);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_VGA30P_EN)
        case MOVIE_SIZE_VGA_30P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 640, 480);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_VGA120P_EN)
        case MOVIE_SIZE_VGA_120P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 640, 480);
        break;
        #endif
        case MOVIE_SIZE_360_30P:
            AHC_SetImageSize(VIDEO_CAPTURE_MODE, 640, 368);
        break;
        default:
        break;
    }
}

void VideoFunc_PresetSensorMode(MOVIESIZE_SETTING ubResolution)
{
    AHC_BOOL ahc_ret = AHC_FALSE;

    switch(ubResolution)
    {
        #if (MENU_MOVIE_SIZE_4K_24P_EN)
		case MOVIE_SIZE_4K_24P:
			ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_4K2K_24P_RESOLUTION);
		break;
		#endif
        #if (MENU_MOVIE_SIZE_1440_30P_EN)
        case MOVIE_SIZE_1440_30P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_1440_30P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_SHD_30P_EN)
        case MOVIE_SIZE_SHD_30P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_SUPER_HD_30P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_SHD_25P_EN)
        case MOVIE_SIZE_SHD_25P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_SUPER_HD_25P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_SHD_24P_EN)
        case MOVIE_SIZE_SHD_24P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_SUPER_HD_24P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_1080_60P_EN)
        case MOVIE_SIZE_1080_60P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_FULL_HD_60P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_1080P_EN)
        case MOVIE_SIZE_1080P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_FULL_HD_30P_RESOLUTION);
        break;
        #endif
	    #if (MENU_MOVIE_SIZE_1080P_30_HDR_EN)
        case MOVIE_SIZE_1080_30P_HDR:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_FULL_HD_30P_RESOLUTION_HDR);
        break;
        #endif	
        #if (MENU_MOVIE_SIZE_1080_24P_EN)
        case MOVIE_SIZE_1080_24P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_FULL_HD_24P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_900P_30P_EN)
        case MOVIE_SIZE_900P_30P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_1600x900_30P_RESOLUTION);
        break;
        #endif    
        #if (MENU_MOVIE_SIZE_960P_30P_EN)
        case MOVIE_SIZE_960P_30P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_4TO3_1D2M_30P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720P_EN)
        case MOVIE_SIZE_720P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_HD_30P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720_60P_EN)
        case MOVIE_SIZE_720_60P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_HD_60P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720_24P_EN)
        case MOVIE_SIZE_720_24P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_HD_24P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720_120P_EN)
        case MOVIE_SIZE_720_120P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_HD_120P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_VGA30P_EN)
        case MOVIE_SIZE_VGA_30P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_VGA_30P_RESOLUTION);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_VGA120P_EN)
        case MOVIE_SIZE_VGA_120P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_VGA_120P_RESOLUTION);
        break;
        #endif
        case MOVIE_SIZE_360_30P:
            ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_VGA_30P_RESOLUTION);
        break;
        default:
            printc( FG_RED("VideoFunc_PresetSensorMode: Unsupported resolution - %d\r\n"), ubResolution);
        break;
    }
}

void VideoFunc_PresetFrameRate(MOVIESIZE_SETTING ubResolution)
{
    AHC_BOOL ahc_ret = AHC_FALSE;

    switch(ubResolution)
    {
        #if (MENU_MOVIE_SIZE_4K_24P_EN)
		case MOVIE_SIZE_4K_24P:
			ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 24);
		break;
		#endif
        #if (MENU_MOVIE_SIZE_1440_30P_EN)
        case MOVIE_SIZE_1440_30P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 30);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_SHD_30P_EN)
        case MOVIE_SIZE_SHD_30P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 30);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_SHD_25P_EN)
        case MOVIE_SIZE_SHD_25P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 25);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_SHD_24P_EN)
        case MOVIE_SIZE_SHD_24P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 24);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_1080_60P_EN)
        case MOVIE_SIZE_1080_60P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 60);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_1080_24P_EN)
        case MOVIE_SIZE_1080_24P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 24);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_1080P_EN)
        case MOVIE_SIZE_1080P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 30);
        break;
        #endif
	    #if (MENU_MOVIE_SIZE_1080P_30_HDR_EN)
        case MOVIE_SIZE_1080_30P_HDR:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 30);
        break;
        #endif
	    #if (MENU_MOVIE_SIZE_900P_30P_EN)
        case MOVIE_SIZE_900P_30P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 30);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_960P_30P_EN)
        case MOVIE_SIZE_960P_30P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 30);
        break;
        #endif	
        #if (MENU_MOVIE_SIZE_720P_EN)
        case MOVIE_SIZE_720P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 30);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720_60P_EN)
        case MOVIE_SIZE_720_60P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 60);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720_24P_EN)
        case MOVIE_SIZE_720_24P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 24);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_720_120P_EN)
        case MOVIE_SIZE_720_120P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 120);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_VGA30P_EN)
        case MOVIE_SIZE_VGA_30P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 30);
        break;
        #endif
        #if (MENU_MOVIE_SIZE_VGA120P_EN)
        case MOVIE_SIZE_VGA_120P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 120);
        break;
        #endif
        case MOVIE_SIZE_360_30P:
            ahc_ret = AHC_VIDEO_SetMovieConfig(0, AHC_FRAME_RATE, 30);
        break;
        default:
            printc( FG_RED("VideoFunc_PresetFrameRate: Unsupported resolution - %d\n"), ubResolution);
        break;
    }
}

AHC_BOOL VideoFunc_PreRecord(void)
{
    AHC_BOOL ahcRet = AHC_FALSE;

    if(AHC_MODE_RECORD_PREVIEW != AHC_GetAhcSysMode()){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk, ahcRet); return ahcRet;}

    if(MenuSettingConfig()->uiMOVSoundRecord != MOVIE_SOUND_RECORD_ON){
        if (MMPS_3GPRECD_GetAVSyncMethod() == VIDMGR_AVSYNC_REF_AUD)
            AHC_VIDEO_SetRecordModeAudioOn(AHC_TRUE);
        else
            AHC_VIDEO_SetRecordModeAudioOn(AHC_FALSE);       
    }
    else{
        AHC_VIDEO_SetRecordModeAudioOn(AHC_TRUE);       
    }
    
    ahcRet = AHC_SetMode(AHC_MODE_RECORD_PRERECD);
    if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk, ahcRet); return ahcRet;}
    
    return ahcRet;
}

VIDEO_RECORD_STATUS VideoFunc_Record(void)
{
    AHC_BOOL    ahc_ret         = AHC_TRUE;
    UINT64      ulFreeSpace     = 0;
    UINT8       Hour, Min, Sec;
    UINT32      ulAudBitRate    = 128000;//128K
    UINT32      ulVidBitRate;
    UINT32      ulTimelimit     = AHC_VIDEO_GetRecTimeLimit();
    UINT64      ulSpaceNeeded   = 0;
    UINT32      MaxDcfObj       = 0;
    UINT32      ulResvSize      = 0;
    UINT32      ulPreRecordedTime = 0;
    AHC_AUDIO_FORMAT aformat;
    AHC_BOOL    bEnableSlowMotion = AHC_FALSE;
    AHC_BOOL    bTimeLapseEnable  = AHC_FALSE;

    if(AHC_FALSE == AHC_SDMMC_IsSD1MountDCF())
    {
        printc(FG_RED("No Card: Mount Fail !!!!!!\r\n"));
        return VIDEO_REC_NO_SD_CARD;
    }

    VideoFunc_GetFreeSpace(&ulFreeSpace);
    AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_BITRATE, &ulVidBitRate);

	DBG_AutoTestPrint(ATEST_ACT_REC_0x0001, ATEST_STS_START_0x0001, 0, 0, gbAhcDbgBrk);
	DBG_AutoTestPrint(ATEST_ACT_REC_0x0001, ATEST_STS_CYCLE_TIME_0x0005, (ulTimelimit & 0xFFFF0000)>>16, (ulTimelimit & 0xFFFF), gbAhcDbgBrk);
	DBG_AutoTestPrint(ATEST_ACT_REC_0x0001, ATEST_STS_BIT_RATE_0x0007, (ulVidBitRate & 0xFFFF0000)>>16, (ulVidBitRate & 0xFFFF), gbAhcDbgBrk);		

    if(ulTimelimit==NON_CYCLING_TIME_LIMIT)
    {
        // 0 to get time for current setting of bitrate
        AHC_VIDEO_AvailableRecTime(&Hour, &Min, &Sec);

        if(Hour == 0 && Min == 0 && Sec <= 2)
        {
            printc(FG_RED("Space is not enough for Non-Cycling recording, Card Full!\r\n"));
            return VIDEO_REC_CARD_FULL;
        }
    }
    else
    {
        /* For Cyclic-Record Space Guaranty */
#if (defined(VIDEO_REC_TIMELAPSE_EN) && VIDEO_REC_TIMELAPSE_EN)
        UINT32 slVRTimelapse = 0;		// Off

        if ((AHC_Menu_SettingGetCB(COMMON_KEY_VR_TIMELAPSE, &slVRTimelapse) == AHC_TRUE) && (slVRTimelapse != PRJ_VR_TIMELAPSE_OFF)){
            UINT32      Framerate, Frate;
            printc(" TimeLapse record.....\r\n");

            AIHC_VIDEO_GetMovieCfgEx(0, AHC_FRAME_RATE, &Frate);
            Framerate = AHC_VIDEO_GetVideoRealFpsX1000(Frate) / AHC_VIDRECD_TIME_INCREMENT_BASE;
            AHC_VIDEO_GetTimeLapseBitrate(Framerate, slVRTimelapse, &ulVidBitRate, &ulAudBitRate);
        }
        else
#endif
        {
            if (MenuSettingConfig()->uiMOVSoundRecord == MOVIE_SOUND_RECORD_OFF)
                ulAudBitRate = 0;
        }

#if ( FS_FORMAT_FREE_ENABLE == 0)
        {
            #if (DELETION_BY_FILE_NUM)
            {
                UINT32 uiFileCount;
                DCF_DB_TYPE sCurDB;
                DCF_DB_TYPE sDB;
                sCurDB = AHC_UF_GetDB();
                #if (DCF_DB_COUNT >= 2)
                if(uiGetParkingModeEnable() == AHC_TRUE) 
                {
                    sDB = DCF_DB_TYPE_2ND_DB;
                }
                else
                #endif
                {                                     
                    sDB = DCF_DB_TYPE_1ST_DB;
                }    
                AHC_UF_SelectDB(sDB);
                AHC_UF_GetTotalFileCount(&uiFileCount);
        	    if (uiFileCount >= AHC_UF_GetFileTH(sDB))
        	    {
        		    if(AHC_Deletion_RemoveEx(sDB, AHC_VIDEO_GetRecTimeLimit()) == AHC_FALSE)
        		    {
        			    printc(FG_RED("AHC_Deletion_Romove Error\r\n"));
        			    return AHC_FALSE;
        		    }
        	    }
                AHC_UF_SelectDB(sCurDB); 
            }
            #endif //#if (DELETION_BY_FILE_NUM)
            AHC_VIDEO_GetRecStorageSpaceNeed(ulVidBitRate, ulAudBitRate, ulTimelimit, &ulSpaceNeeded);

            if( ulFreeSpace < ulSpaceNeeded )
            {
                MaxDcfObj = AHC_GetMappingFileNum(FILE_MOVIE);

                if(MaxDcfObj==0)
                {
                    printc("--E-- Space is not enough for cycle recording !!!\r\nPlease Clean SD card!\r\n");
                    return VIDEO_REC_SEAMLESS_ERROR;
                }
                else
                {
                    printc("-I- Space is not enough for recording, Delete video file first!\r\n");
                    if(AHC_VIDEO_DeleteDcfMinKeyFile(AHC_TRUE, (const char *) AHC_GetVideoExt()) != AHC_TRUE)
                        return VIDEO_REC_SEAMLESS_ERROR;
                }
            }
        }
#endif
    }

    //Video
    AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_CODEC_TYPE        , AHC_MOVIE_VIDEO_CODEC_H264);
    AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_COMPRESSION_RATIO , MenuSettingConfig()->uiMOVQuality);
    AHC_VIDEO_SetMovieConfig(0, AHC_MAX_PFRAME_NUM          , 14);

    //Audio
#if (VR_AUDIO_TYPE==VR_AUDIO_TYPE_AAC)
    aformat = AHC_MOVIE_AUDIO_CODEC_AAC;
#elif (VR_AUDIO_TYPE==VR_AUDIO_TYPE_MP3)
    aformat = AHC_MOVIE_AUDIO_CODEC_MP3;
#elif (VR_AUDIO_TYPE==VR_AUDIO_TYPE_ADPCM)
    aformat = AHC_MOVIE_AUDIO_CODEC_ADPCM;
#endif

    AHC_VIDEO_ConfigAudio(AHC_AUDIO_STREAM_ID, aformat, AHC_AUDIO_CHANNEL_MONO_R);

    #if (VR_PREENCODE_EN)
    ulResvSize = (m_ubPreEncodeEn)?(STORAGE_MIN_SIZE+PRE_RECORD_STORAGE_MIN_SIZE):(STORAGE_MIN_SIZE);
    #else
    ulResvSize = STORAGE_MIN_SIZE;
    #endif
    AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_RESERVED_SIZE     , ulResvSize);

    if( uiGetParkingModeEnable() == AHC_TRUE )
    {
        AHC_VIDEO_GetCurRecordingTime(&ulPreRecordedTime);
    }

    #if (GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO)
    if( uiGetParkingModeEnable() == AHC_TRUE )
    {
        AHC_GPS_TriggerRestorePreRecordInfo( AHC_TRUE, ulPreRecordedTime, AHC_FALSE );
    }
    else
    {
        GPSCtrl_ResetBufferControlVariable();
    }
    #endif

    #if (GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)
    if( uiGetParkingModeEnable() == AHC_TRUE )
    {
        AHC_Gsensor_TriggerRestorePreRecordInfo( AHC_TRUE, ulPreRecordedTime, AHC_FALSE );
    }
    else
    {
        AHC_Gsensor_ResetBufCtrlVariable();
    }
    #endif

    #if (GSENSOR_CONNECT_ENABLE)
    {
        extern AHC_BOOL AHC_Gsensor_GetCurIntStatus(AHC_BOOL* bObjMove);

        AHC_BOOL    dummy;
        // Clean GSensor INT status to avoid getting the status of before recording
        AHC_Gsensor_GetCurIntStatus(&dummy);
    }
    #endif

    m_uiSlowMediaCBCnt  = 0;

    {
        int iSlowMotionRatioSetting = pf_SLOWMOTION_EnGet();
        UINT32 uiSlowMotionRatio = 1;
        UINT32 Param, FrameRate, usVopTimeIncrement, usVopTimeIncrResol;

        AIHC_VIDEO_GetMovieCfgEx(0, AHC_FRAME_RATE, &Param);
        FrameRate = AHC_VIDEO_GetVideoRealFpsX1000(Param);

        usVopTimeIncrement = AHC_VIDRECD_TIME_INCREMENT_BASE;
        usVopTimeIncrResol = FrameRate;

        switch(iSlowMotionRatioSetting){
            case SLOWMOTION_X2:
                uiSlowMotionRatio = 2;
                bEnableSlowMotion = AHC_TRUE;
                break;
            case SLOWMOTION_X4:
                uiSlowMotionRatio = 4;
                bEnableSlowMotion = AHC_TRUE;
                break;
            case SLOWMOTION_X8:
                uiSlowMotionRatio = 8;
                bEnableSlowMotion = AHC_TRUE;
                break;
            case SLOWMOTION_X1:
            default:
                uiSlowMotionRatio = 1;
                break;
        }

        printc(FG_GREEN("SlowMotionRatio:X%x\r\n"), uiSlowMotionRatio);
        AHC_VIDEO_SetSlowMotionFPS(bEnableSlowMotion, AHC_VIDRECD_TIME_INCREMENT_BASE, usVopTimeIncrResol * AHC_VIDRECD_TIME_INCREMENT_BASE / usVopTimeIncrement / uiSlowMotionRatio);
    }

#if (defined(VIDEO_REC_TIMELAPSE_EN) && VIDEO_REC_TIMELAPSE_EN == 1)
    {
        int iTimeLapseSetting;
        UINT32 uiTimeLapseRatio = 1;
        UINT32 Param, FrameRate, usVopTimeIncrement, usVopTimeIncrResol;

        pf_General_EnGet(COMMON_KEY_VR_TIMELAPSE, &iTimeLapseSetting);
        AIHC_VIDEO_GetMovieCfgEx(0, AHC_FRAME_RATE, &Param);
        FrameRate = AHC_VIDEO_GetVideoRealFpsX1000(Param);

        usVopTimeIncrement = AHC_VIDRECD_TIME_INCREMENT_BASE;
        usVopTimeIncrResol = FrameRate;

        switch(iTimeLapseSetting){
            case PRJ_VR_TIMELAPSE_1SEC:
                bTimeLapseEnable = AHC_TRUE;
                uiTimeLapseRatio = 1;
                break;
            case PRJ_VR_TIMELAPSE_5SEC:
                bTimeLapseEnable = AHC_TRUE;
                uiTimeLapseRatio = 5;
                break;
            case PRJ_VR_TIMELAPSE_10SEC:
                bTimeLapseEnable = AHC_TRUE;
                uiTimeLapseRatio = 10;
                break;
            case PRJ_VR_TIMELAPSE_30SEC:
                bTimeLapseEnable = AHC_TRUE;
                uiTimeLapseRatio = 30;
                break;
            case PRJ_VR_TIMELAPSE_60SEC:
                bTimeLapseEnable = AHC_TRUE;
                uiTimeLapseRatio = 60;
                break;

            case PRJ_VR_TIMELAPSE_OFF:
            default:
                uiTimeLapseRatio = 1;
                break;
        }
        if(AHC_TRUE== bTimeLapseEnable){
            printc(FG_GREEN("TimeLapse:0x%x sec.\r\n"), uiTimeLapseRatio);
        }
        else{
            printc(FG_GREEN("TimeLapse: Off.\r\n"));
        }
        AHC_VIDEO_SetTimeLapseFPS(bTimeLapseEnable, AHC_VIDRECD_TIME_INCREMENT_BASE, usVopTimeIncrement / uiTimeLapseRatio);
    }
#endif

    AHC_UF_SetFreeChar(0, DCF_SET_FREECHAR, (UINT8 *) VIDEO_DEFAULT_FLIE_FREECHAR);

    if((MenuSettingConfig()->uiMOVSoundRecord != MOVIE_SOUND_RECORD_ON) || (AHC_TRUE == bEnableSlowMotion) || (AHC_TRUE == bTimeLapseEnable)){
        if (MMPS_3GPRECD_GetAVSyncMethod() == VIDMGR_AVSYNC_REF_AUD)
        	AHC_VIDEO_SetRecordModeAudioOn(AHC_TRUE);       
        else
        	AHC_VIDEO_SetRecordModeAudioOn(AHC_FALSE);       
    }
    else{
        AHC_VIDEO_SetRecordModeAudioOn(AHC_TRUE);       
    }

    ahc_ret = AHC_SetMode(AHC_MODE_VIDEO_RECORD);
     if (ahc_ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk, ahc_ret); return VIDEO_REC_STOP;}
     
#if (MOTION_DETECTION_EN)
    // m_ulVMDStopCnt is not zero that is trigged by MVD, update VMD stop count again.
    if (m_ulVMDStopCnt && (AHC_TRUE == ahc_ret)) {
        #if (PARKING_RECORD_FORCE_20SEC == 1)
        UINT32 ulTime = 0;
        AHC_VIDEO_GetCurRecordingTime(&ulTime);
        ulTime += VideoFunc_GetRecordTimeOffset();
        printc(FG_BLUE("Parking: Recorded time = %d ms\r\n"), ulTime);
        m_ulVMDStopCnt = 200 - (ulTime / 100); // Set remaining recording time
        #else
        m_ulVMDStopCnt = AHC_GetVMDRecTimeLimit() * 1000 / VIDEO_TIMER_UNIT;
        #endif
    }
    else
    {
        // When VR is error, stop down count for VMD VR
        m_ulVMDStopCnt = 0;
    }
#endif

    if( AHC_TRUE == ahc_ret )
    {
        uiStateSetLocked(AHC_TRUE);
        bVideoRecording = AHC_TRUE;
    }
    else
    {
        return VIDEO_REC_STOP;
    }
	#if (defined(CUS_ADAS_OUTPUT_LOG) && ADAS_OUTPUT_LOG == 1)
	ADAS_open_txt((char *) AHC_VIDEO_GetCurRecFileName(0));
    ADAS_CTL_SetADASOutputLog(MMP_TRUE);
	#endif
    return VIDEO_REC_START;
}

AHC_BOOL VideoFunc_RecordStop(void)
{
    AHC_BOOL ahc_ret    = AHC_TRUE;

#if (FLICKER_PWR_LED_AT_VR_STATE)
    LedCtrl_PowerLed(AHC_TRUE);
#endif

#if (VR_PREENCODE_EN)
    if(m_ubPreEncodeEn)
        m_ubPreRecording = AHC_FALSE;
#endif

    if (AHC_VIDEO_IsEmergRecStarted() == AHC_TRUE)
    {        
        if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_MOVE_FILE)
        {
            AHC_VIDEO_SetEmergRecStarted(AHC_FALSE);
        }
        else if ((MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE) ||
                 (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE) )
        {
            AHC_VIDEO_StopEmergRecord();
            SystemSettingConfig()->byStartNormalRecordAfterEmergencyRecord = 0;
            AHC_VIDEO_EmergRecPostProcess();
        }
    }
    else if (uiGetParkingModeEnable())
    {
		if(VideoFunc_RecordStatus())
			AHC_VIDEO_ParkingModePostProcess();
    }

    ahc_ret = VideoFunc_SetAttribute();
    
#if (MOTION_DETECTION_EN)
    m_ulVMDRemindTime = 1;
    #if !defined(CFG_MVD_MODE_LINK_WITH_MENU_SENSITIVITY)
    m_ubVMDStart      = AHC_FALSE;
    #endif
    m_ulVMDStopCnt    = 0;
#endif

    if(VideoFunc_LockFileEnabled()) {
        // Remove Lock Next file
        LockFileTypeChange(1 /* Remove Next */);
    }
    printc("set mode\r\n");
    ahc_ret = AHC_SetMode(AHC_MODE_RECORD_PREVIEW);

#if (VR_PREENCODE_EN)
    if(m_ubPreEncodeEn)
        m_ubPreRecording = AHC_TRUE;
#endif

    if( AHC_TRUE == ahc_ret )
    {
        uiStateSetLocked(AHC_FALSE);
        bVideoRecording = AHC_FALSE;

        #if (AHC_SHAREENC_SUPPORT)
        m_bShareRecPostDone = AHC_TRUE;
        AHC_VIDEO_SetShareRecStarted(AHC_FALSE);
        m_bFirstShareFile = AHC_FALSE;
        #endif
    }
    else
    {
        printc("Back to Preview Error\n");
    }
	#if (defined(CUS_ADAS_OUTPUT_LOG) && ADAS_OUTPUT_LOG == 1)
	ADAS_close_txt();
	ADAS_CTL_SetADASOutputLog(MMP_FALSE);
	#endif

    DBG_AutoTestPrint(ATEST_ACT_REC_0x0001, ATEST_STS_END_0x0003, 0, !ahc_ret, gbAhcDbgBrk);    

    return ahc_ret;
}

AHC_BOOL VideoFunc_RecordPause(void)
{
    AHC_BOOL ahc_ret = AHC_TRUE;
    UINT32   CurSysMode;
    
	DBG_AutoTestPrint(ATEST_ACT_REC_0x0001, ATEST_STS_PAUSE_0x0002, 0, 0, gbAhcDbgBrk);

    AHC_GetSystemStatus(&CurSysMode);

    CurSysMode >>= 16;

    if( ( CurSysMode != AHC_MODE_VIDEO_RECORD ))
    {
        return AHC_FALSE;
    }

    ahc_ret = AHC_VIDEO_CaptureClipCmd(AHC_CAPTURE_CLIP_PAUSE, 0);

    return ahc_ret;
}

AHC_BOOL VideoFunc_RecordResume(void)
{
    AHC_BOOL ahc_ret = AHC_TRUE;
    UINT32   CurSysMode;

    AHC_FS_SetCreationTime();
    AHC_GetSystemStatus(&CurSysMode);

    CurSysMode >>= 16;

    if( ( CurSysMode != AHC_MODE_VIDEO_RECORD ) )
    {
        return AHC_FALSE;
    }

    ahc_ret = AHC_VIDEO_CaptureClipCmd(AHC_CAPTURE_CLIP_RESUME, 0);

    return ahc_ret;
}

AHC_BOOL VideoFunc_Shutter(void)
{
    AHC_BOOL    bon;

    bon = AHC_VIDEO_CaptureClipCmd(AHC_CAPTURE_SNAPSHOT, 0);

    return bon;
}

AHC_BOOL VideoFunc_ShutterFail(void)
{
    return AHC_TRUE;
}

AHC_BOOL VideoFunc_Preview(void)
{
    AHC_BOOL    ahc_ret = AHC_TRUE;
    UINT32      ulNightMode = 0;
    
#if (ENABLE_ADAS_LDWS || ENABLE_ADAS_FCWS || ENABLE_ADAS_SAG)
    UINT32 bLDWS_En = COMMON_LDWS_EN_OFF; 
    UINT32 bFCWS_En = COMMON_FCWS_EN_OFF;
    UINT32 bSAG_En  = COMMON_SAG_EN_OFF;
    UINT32 uiAdasFlag = 0;
#endif
    AHC_BOOL ahcRet = AHC_TRUE;

#if (VR_PREENCODE_EN)
    if (m_ubPreEncodeEn)
        m_ubPreRecording = AHC_FALSE;
#endif

#if MENU_MOVIE_SIZE_1080P_30_HDR_EN
	{
	    INT32       iMovSize = 0;
	    AHC_Menu_SettingGetCB(COMMON_KEY_MOVIE_SIZE, &iMovSize);
	    if (MMP_IsVidHDREnable() && (iMovSize == MOVIE_SIZE_1440_30P)) {
	        //Fix Issue: HDR mode switch to 1440@30fps mode need stop preview.
	        AHC_SetMode(AHC_MODE_IDLE);
	    }
    }
#endif


    ahc_ret = VideoFunc_SetPreviewWindow(AHC_TRUE);
    ahc_ret = VideoFunc_SetAttribute();

#ifdef  _OEM_
    Oem_SetVideo_Prop(MenuSettingConfig()->uiMOVSize);
#endif

    AHC_PreSetFlashLED();

#if (MOTION_DETECTION_EN)
    m_ubVMDStart      = AHC_FALSE;
    m_ulVMDStopCnt    = 0;
#endif

    //Select Y-type.
#if (ENABLE_ADAS_LDWS || ENABLE_ADAS_FCWS || ENABLE_ADAS_SAG)
    AHC_GetParam(PARAM_ID_ADAS, &uiAdasFlag);
    
#if (ENABLE_ADAS_LDWS)
    #ifdef CFG_ADAS_MENU_SETTING_OLD_STYLE
    if (MenuSettingConfig()->uiLDWS == COMMON_LDWS_EN_ON)
    #else
    if (uiAdasFlag & AHC_ADAS_ENABLED_LDWS)
    #endif
    {
        bLDWS_En = COMMON_LDWS_EN_ON;
    }
#endif

#if (ENABLE_ADAS_FCWS)
    #ifdef CFG_ADAS_MENU_SETTING_OLD_STYLE
    if (MenuSettingConfig()->uiFCWS == COMMON_FCWS_EN_ON)
    #else
    if (uiAdasFlag & AHC_ADAS_ENABLED_FCWS)
    #endif
    {
        bFCWS_En = COMMON_FCWS_EN_ON;
    }
#endif

#if (ENABLE_ADAS_SAG)
    if (uiAdasFlag & AHC_ADAS_ENABLED_SAG)
    {
        bSAG_En = COMMON_SAG_EN_ON;
    }
#endif

    if (CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
	{
		UINT32 TimeStampOp;
		
		AHC_GetCaptureConfig(ACC_DATE_STAMP, &TimeStampOp);
		if (TimeStampOp & AHC_ACC_TIMESTAMP_ENABLE_MASK)
		{
			AHC_RTC_TIME sRtcTime;
			AHC_RTC_GetTime(&sRtcTime);
			AHC_VIDEO_ConfigRecTimeStamp(TimeStampOp, &sRtcTime, MMP_TRUE);
		}
	}

    //Init With ADAS mode.
    if (bLDWS_En == COMMON_LDWS_EN_ON || 
        bFCWS_En == COMMON_FCWS_EN_ON || 
        bSAG_En  == COMMON_SAG_EN_ON){        
        ahcRet = AHC_VIDEO_SetRecordModeRegisterInit((void *)AHC_VIDEO_SetRecordModeInitADASMode);
        ahcRet = AHC_VIDEO_SetRecordModeRegisterUnInit((void *)AHC_VIDEO_SetRecordModeUnInitADASMode);        
    }
    else
#endif
    {
        //Init With default mode.
        ahcRet = AHC_VIDEO_SetRecordModeRegisterInit((void *)AHC_VIDEO_SetRecordModeInit);
        ahcRet = AHC_VIDEO_SetRecordModeRegisterUnInit((void *)AHC_VIDEO_SetRecordModeUnInit);                
    }
   
    AHC_GetParam(PARAM_ID_NIGHT_MODE_AE, &ulNightMode);

	AHC_VIDEO_SetNightMode(ulNightMode, 4); 
	
	// for only sonix
	if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264)) {
		MMPS_3GPRECD_SetAVSyncMethod(VIDMGR_AVSYNC_REF_AUD); 
	}
    
    ahc_ret = AHC_SetMode(AHC_MODE_RECORD_PREVIEW);

    ahc_ret = VideoFunc_SetPreviewWindow(AHC_FALSE);

#if (VR_PREENCODE_EN)
    if(m_ubPreEncodeEn)
    {
        m_ubPreRecording = AHC_TRUE;
        AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_RESERVED_SIZE, STORAGE_MIN_SIZE+PRE_RECORD_STORAGE_MIN_SIZE);
    }
#endif

 #if (AHC_VR_THUMBNAIL_JPG_ENABLE)
     #if (SUPPORT_VR_THUMBNAIL) // TBD:Need Move to record function.
		if (m_DramSettings.DRAMID == DRAM_DDR)	
			MMPS_3GPRECD_SetVRThumbJpgSize(80,48);
		else if (m_DramSettings.DRAMID == DRAM_DDR3)
			MMPS_3GPRECD_SetVRThumbJpgSize(240,136);

        #if (AHC_VR_THUMBNAIL_JPG_ENABLE)
        if (CAM_CHECK(PRM_CAM_BAYER_SENSOR, SCD_CAM_NONE, USB_CAM_NONE)||
            CAM_CHECK(PRM_CAM_BAYER_SENSOR, SCD_CAM_NONE, USB_CAM_SONIX_MJPEG2H264)) {
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE)
            MMPS_3GPRECD_EnableVRThumbnail(MMP_TRUE, MMP_TRUE);
            #else
            MMPS_3GPRECD_EnableVRThumbnail(MMP_TRUE, MMP_FALSE);
            #endif
            MMPS_3GPRECD_SetVRThumbRingBufNum((EMER_RECORD_DUAL_WRITE_PRETIME * 2) + 1); // RingBufferNum = PreEncTime * 2 + 1
        }
        else {
            MMPS_3GPRECD_EnableVRThumbnail(MMP_FALSE, MMP_FALSE);
            MMPS_3GPRECD_SetVRThumbRingBufNum(0);
        }
        #else
        MMPS_3GPRECD_EnableVRThumbnail(MMP_FALSE, MMP_FALSE);
        MMPS_3GPRECD_SetVRThumbRingBufNum(0);
        #endif
    #else
        MMPS_3GPRECD_EnableVRThumbnail(MMP_FALSE, MMP_FALSE);
        MMPS_3GPRECD_SetVRThumbRingBufNum(0);  
    #endif
#endif
        
#if (SUPPORT_EIS)
    {
        static int testgyro_flag = 0;

        extern void MMPF_EIS_mode_init(void);
        extern void MMPF_EIS_start_gyro_sample(void);
        extern MMP_SLONG MMPF_EIS_enable(MMP_ULONG en);
        
        MMPF_EIS_mode_init();
        printc("\r\nMMPF_EIS_mode_init\r\n");

        if (testgyro_flag == 0) {
            testgyro_flag = 1;

            MMPF_EIS_start_gyro_sample();
            printc("\r\nMMPF_EIS_start_gyro_sample\r\n");
        }
        MMPF_EIS_enable(1);
    }
#endif

    return ahc_ret;
}

AHC_BOOL VideoFunc_DualBayerSnrCapturePreview(void)
{
    AHC_BOOL ahc_ret        = AHC_TRUE;
    AHC_BOOL ubSnrFlipEn    = AHC_FALSE;
    UINT16   zoomratio      = 4;
    UINT8    bRotate        = VERTICAL_LCD;
    UINT16   dw, dh;
    MMP_ULONG codec_type, streaming;

    AHC_SetPreviewZoomConfig(ZoomCtrl_GetVideoDigitalZoomMax(), (UINT8)zoomratio);

    dw = RTNA_LCD_GetAttr()->usPanelW;
    dh = RTNA_LCD_GetAttr()->usPanelH;

    AHC_SetDisplayWindow(DISPLAY_SYSMODE_VIDEOCAPTURE_PRM, AHC_TRUE, bRotate, 0, 0, dw, dh, 0);

    AHC_UF_SetFreeChar(0, DCF_SET_FREECHAR, (UINT8 *) VIDEO_DEFAULT_FLIE_FREECHAR);

    AIHC_VIDEO_GetMovieCfgEx(1, AHC_VIDEO_CODEC_TYPE, &codec_type);
    AIHC_VIDEO_GetMovieCfgEx(1, AHC_VIDEO_STREAMING, &streaming);
    
    if (!streaming || codec_type != AHC_MOVIE_VIDEO_CODEC_H264) {
        VideoFunc_SetResolution(MenuSettingConfig()->uiMOVSize);
    }

    AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_BEST_CAMERA_PREVIEW_RESOLUTION);
    VideoFunc_PresetFrameRate(MenuSettingConfig()->uiMOVSize);

    ahc_ret = AHC_SetMode(AHC_MODE_CAPTURE_PREVIEW);

    ubSnrFlipEn = AHC_CheckSNRFlipEn(CTRL_BY_ALL);
    AHC_SetKitDirection(AHC_LCD_NOFLIP, AHC_FALSE, AHC_GetSensorStatus(), ubSnrFlipEn);

    return ahc_ret;
}

AHC_BOOL VideoFunc_EnterVMDMode(void)
{
#if (MOTION_DETECTION_EN)

    if (AHC_SDMMC_BasicCheck() != AHC_FALSE ||
        MenuSettingConfig()->uiMotionDtcSensitivity == MOTION_DTC_SENSITIVITY_OFF)
        return AHC_FALSE;

    if(VideoFunc_RecordStatus())
        return AHC_TRUE;

    m_ulVMDCancel    = AHC_FALSE;
    m_ubInRemindTime = AHC_TRUE;

    #ifdef CFG_REC_CUS_VMD_REMIND_TIME //may be defined in config_xxx.h
    m_ulVMDRemindTime = CFG_REC_CUS_VMD_REMIND_TIME;
    #else
    m_ulVMDRemindTime = 10;
    #endif

    m_ubMotionDtcEn = AHC_TRUE;
    #if (VR_PREENCODE_EN)
    m_ubPreEncodeEn = AHC_FALSE;
    #endif
    AHC_SetMode(AHC_MODE_IDLE);
    VideoFunc_Preview();

#endif
    return AHC_TRUE;
}

AHC_BOOL VideoFunc_ExitVMDMode(void)
{
#if (MOTION_DETECTION_EN)

    #if (VR_PREENCODE_EN)
    m_ubPreEncodeEn = AHC_FALSE;
    #endif
    m_ubMotionDtcEn = AHC_FALSE;

    m_ubInRemindTime = AHC_FALSE;

    if(VideoFunc_RecordStatus())
    {
        VideoFunc_RecordStop();
        DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);
    }
    else
    {
        bDisableVideoPreRecord = AHC_TRUE;
    }

    if (m_ulVMDRemindTime)
    {
        m_ulVMDCancel = AHC_TRUE;
    }
    else
    {
        AHC_SetMode(AHC_MODE_IDLE);
        VideoFunc_Preview();
    }
#endif
    return AHC_TRUE;
}

AHC_BOOL VideoFunc_ZoomOperation(AHC_ZOOM_DIRECTION bZoomDir)
{
    AHC_BOOL ahc_ret = AHC_TRUE;

#if (VIDEO_DIGIT_ZOOM_EN)

    if( AHC_SENSOR_ZOOM_IN == bZoomDir )
    {
        if( ZoomCtrl_GetVideoDigitalZoomMax() > ZoomCtrl_GetVideoDigitalZoomLevel() )
        {
            ahc_ret = ZoomCtrl_DigitalZoom(VIDEO_CAPTURE_MODE, AHC_SENSOR_ZOOM_IN);
        }
    }
    else if(AHC_SENSOR_ZOOM_OUT == bZoomDir )
    {
        if( 0 < ZoomCtrl_GetVideoDigitalZoomLevel() )
        {
            ahc_ret = ZoomCtrl_DigitalZoom(VIDEO_CAPTURE_MODE, AHC_SENSOR_ZOOM_OUT);
        }
    }
    else
    {
        ahc_ret = ZoomCtrl_DigitalZoom(VIDEO_CAPTURE_MODE, AHC_SENSOR_ZOOM_STOP);
    }
#endif
    return ahc_ret;
}

AHC_BOOL VideoFunc_RecordRestart(void)
{
    UINT64      ulFreeSpace     = 0;
    UINT32      ulAudBitRate    = 128000;//128K
    UINT32      ulVidBitRate    = 0;
    AHC_BOOL    ubDeleteFile    = AHC_FALSE;
    AHC_BOOL    ahc_ret         = AHC_TRUE;
    UINT32      ulTimelimit;
    UINT64      ulSpaceNeeded   = 0;
#if (DCF_DB_COUNT >= 2)
    UINT32      ulSpaceNeededInClusters = 0;
    DCF_DB_TYPE sDB;
#endif

#if (defined(VIDEO_REC_TIMELAPSE_EN) && VIDEO_REC_TIMELAPSE_EN)
        UINT32 slVRTimelapse = 0;		// Off
#endif
    VIDENC_FW_STATUS sMergerStatus = VIDENC_FW_STATUS_NONE;

    _AHC_PRINT_FUNC_ENTRY_();

    MMPS_3GPRECD_GetRecordStatus(&sMergerStatus);
    if (sMergerStatus != VIDENC_FW_STATUS_PREENCODE){
        printc(FG_RED("sMergerStatus:%d\r\n"),sMergerStatus);        
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0);
        return 0;
    }
    
    ahc_ret = VideoFunc_SetAttribute();

    if(AHC_SDMMC_BasicCheck()==AHC_FALSE)
    {
        printc("AHC_SDMMC_BasicCheck Fail\r\n");
        return AHC_FALSE;
    }

    ulTimelimit = AHC_VIDEO_GetRecTimeLimit();
    if(ulTimelimit==NON_CYCLING_TIME_LIMIT)
    {
        UINT32 ulCurrRecTime;

        AHC_VIDEO_GetCurRecordingTime(&ulCurrRecTime);
        RecordTimeOffset   += ulCurrRecTime;
        ulSpaceNeeded       = 0x24000000;
        ubDeleteFile        = AHC_FALSE;
    }
    else
    {
#if (LIMIT_MAX_LOCK_FILE_TIME)
        //Calculate Correct Time Offset
        if (VideoFunc_LockFileEnabled()) {
            RecordTimeOffset += ulVRTotalTime;
            ulTimelimit = AHC_GetVideoMaxLockFileTime();
            ulVRTotalTime = ulTimelimit * 1000;
        } else {
            if (ulVRTotalTime != 0) {
                RecordTimeOffset += ulVRTotalTime;
                ulVRTotalTime = 0;
            } else {
                RecordTimeOffset += (ulTimelimit*1000/*ms*/);
            }
        }
#else
        RecordTimeOffset += (ulTimelimit*1000/*ms*/);
#endif

        AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_BITRATE, &ulVidBitRate);

#if (defined(VIDEO_REC_TIMELAPSE_EN) && VIDEO_REC_TIMELAPSE_EN)
        if ((AHC_Menu_SettingGetCB(COMMON_KEY_VR_TIMELAPSE, &slVRTimelapse) == AHC_TRUE) && (slVRTimelapse != PRJ_VR_TIMELAPSE_OFF)){
            UINT32      Framerate, Frate;
            printc(" TimeLapse record.....\r\n");

            AIHC_VIDEO_GetMovieCfgEx(0, AHC_FRAME_RATE, &Frate);
            Framerate = AHC_VIDEO_GetVideoRealFpsX1000(Frate) / AHC_VIDRECD_TIME_INCREMENT_BASE;
            AHC_VIDEO_GetTimeLapseBitrate(Framerate, slVRTimelapse, &ulVidBitRate, &ulAudBitRate);
        }
        else
#endif
        {
            if (MenuSettingConfig()->uiMOVSoundRecord == MOVIE_SOUND_RECORD_OFF)
                ulAudBitRate = 0;
        }

        AHC_VIDEO_GetRecStorageSpaceNeed(ulVidBitRate, ulAudBitRate, ulTimelimit, &ulSpaceNeeded);

        VideoFunc_GetFreeSpace(&ulFreeSpace);

        #if (DCF_DB_COUNT >= 2)
        sDB = AHC_UF_GetDB();
        if (sDB != DCF_DB_TYPE_1ST_DB) {
            printc(FG_YELLOW("1: %s, %d, current db:%d\r\n"),__func__,__LINE__, sDB);
            AHC_UF_SelectDB(DCF_DB_TYPE_1ST_DB);
        }
        #endif
        
#if (DCF_DB_COUNT >= 2)
        if(uiGetParkingModeEnable() == AHC_FALSE) //DCF_DB_TYPE_1ST_DB
        {
            sDB = DCF_DB_TYPE_1ST_DB;
        }
        else
        {                                      //DCF_DB_TYPE_2ND_DB
            sDB = DCF_DB_TYPE_2ND_DB;
        }
        ulSpaceNeededInClusters = ulSpaceNeeded /AHC_UF_GetClusterSize(sDB);
        if((AHC_UF_GetFileCluster(sDB)+ ulSpaceNeededInClusters > AHC_UF_GetClusterTH(sDB))
           || (ulFreeSpace < ulSpaceNeeded))
        {
            printc("--I-- %s:%d Storage space is not enough for recording, delete video file\r\n", __func__, __LINE__);
            ubDeleteFile = AHC_TRUE;
        }
#if (DELETION_BY_FILE_NUM)
        else 
		{
            UINT32 uiFileCount;
            DCF_DB_TYPE sCurDB;
            //sCurDB = AHC_UF_GetDB();
            //AHC_UF_SelectDB(sCurDB);

            #if 1
            sDB = AHC_UF_GetDB();
            if (sDB != DCF_DB_TYPE_1ST_DB) {
                printc(FG_YELLOW("2: %s, %d, current db:%d\r\n"),__func__,__LINE__, sDB);
                AHC_UF_SelectDB(DCF_DB_TYPE_1ST_DB);
            }
            #endif
            

            AHC_UF_GetTotalFileCount(&uiFileCount);
            if(uiFileCount >= AHC_UF_GetFileTH(sCurDB)){
                printc("FileCount > FileNum Threshold\r\n");
            	ubDeleteFile = AHC_TRUE;
            }
        }
#endif //#if (DELETION_BY_FILE_NUM)
#else
        if( ulFreeSpace < ulSpaceNeeded )
        {
            printc("--I-- %s:%d Storage space is not enough for recording, delete video file\r\n", __func__, __LINE__);
            ubDeleteFile = AHC_TRUE;
        }
#if (DELETION_BY_FILE_NUM)
        else
		{
            UINT32 uiFileCount;
            AHC_UF_GetTotalFileCount(&uiFileCount);
            if(uiFileCount >= AHC_UF_GetFileTH(DCF_DB_TYPE_1ST_DB)){
                printc("FileCount > FileNum Threshold\r\n");
                ubDeleteFile = AHC_TRUE;
            }
        }
#endif //#if (DELETION_BY_FILE_NUM)
#endif
    }

#if (RESET_RECORDED_TIME)
    RecordTimeOffset    = 0;
#endif
    printc("Restart RecordTimeOffset [%d] ms\r\n", RecordTimeOffset);

    #if 1
    sDB = AHC_UF_GetDB();
    if (sDB != DCF_DB_TYPE_1ST_DB) {
        printc(FG_YELLOW("3: %s, %d, current db:%d\r\n"),__func__,__LINE__, sDB);
        AHC_UF_SelectDB(DCF_DB_TYPE_1ST_DB);
    }
    #endif
    
    AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_RESERVED_SIZE, STORAGE_MIN_SIZE );//TBD

    m_uiSlowMediaCBCnt = 0;

	#if (UPDATE_UI_USE_MULTI_TASK)
	m_ubUpdateUiByOtherTask = AHC_TRUE;
	#endif
    AHC_VIDEO_SetRecordModeDeleteFile(ubDeleteFile);    
    ahc_ret = AHC_VIDEO_RestartRecMode();
	#if (UPDATE_UI_USE_MULTI_TASK)
	m_ubUpdateUiByOtherTask = AHC_FALSE;
	#endif	
    // Re-Set TimeLimit in case a time specfied at file locked.
    AHC_VIDEO_SetRecTimeLimit(ulTimelimit);

    if( AHC_TRUE == ahc_ret )
    {
        uiStateSetLocked(AHC_TRUE);
        bVideoRecording = AHC_TRUE;
    }
	#if (defined(CUS_ADAS_OUTPUT_LOG) && ADAS_OUTPUT_LOG == 1)
	 ADAS_close_txt();
	 ADAS_open_txt((char *) AHC_VIDEO_GetCurRecFileName(0));
	 #endif
    return ahc_ret;
}

AHC_BOOL VideoFunc_ChangeCurFileTimeLimit(void)
{
    #if (LIMIT_MAX_LOCK_FILE_TIME)
    {
        UINT32 ulCurVRTime;
        UINT32 ulMaxVRTime = AHC_GetVideoMaxLockFileTime();

        AHC_VIDEO_GetCurRecordingTime(&ulCurVRTime);//ms
        ulCurLockTime = ulCurVRTime;//ms
        printc("Current Recording Time %d ms\r\n",ulCurVRTime);

        if(ulCurVRTime > ulMaxVRTime * 1000)
        {
            ulVRTotalTime = ulCurVRTime;
        } else {
            ulVRTotalTime = ulMaxVRTime * 1000;
        }
            
        #if 1
        AHC_VIDEO_ChangeCurFileTimeLimit((ulMaxVRTime+ulCurVRTime/1000)*1000);
        #else
	    AHC_VIDEO_SetRecTimeLimit(ulMaxVRTime+ulCurVRTime/1000);//continue record locktime
		#endif
        //AHC_VIDEO_SetRecTimeLimit(1);//trigger media file full evnet,restart lock file...
    }
    #endif
    return AHC_TRUE;
}

AHC_BOOL VideoFunc_SetFileLock(void)
{
    if(AHC_WMSG_States())
    {
        AHC_WMSG_Draw(AHC_FALSE, WMSG_NONE, 0);
    }

    if(!VideoFunc_LockFileEnabled())
    {
        #if (LIMIT_MAX_LOCK_FILE_NUM) && (MAX_LOCK_FILE_ACT==LOCK_FILE_STOP)
        if (m_ulLockFileNum  >= MAX_LOCK_FILE_NUM )
            return AHC_FALSE;
        #endif

        AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
        EnableLockFile(AHC_TRUE, VR_LOCK_FILE_TYPE);

        #if (LIMIT_MAX_LOCK_FILE_NUM)
        m_ulLockEventNum++;
        #endif

        #if (LIMIT_MAX_LOCK_FILE_TIME)
        {
            UINT32 ulCurVRTime;
            UINT32 ulMaxVRTime = AHC_GetVideoMaxLockFileTime();

            AHC_VIDEO_GetCurRecordingTime(&ulCurVRTime);//ms
            printc("Current Recording Time %d ms\r\n",ulCurVRTime);

            if(ulCurVRTime > ulMaxVRTime * 1000)
            {
                ulVRTotalTime = ulCurVRTime;
            } else {
                ulVRTotalTime = ulMaxVRTime * 1000;
            }

			#if (LIMIT_MAX_LOCK_FILE_TIME)
			VideoFunc_ChangeCurFileTimeLimit();
			#endif
        }
        #endif
    }
    else
    {
        #if (LIMIT_MAX_LOCK_FILE_NUM)
        m_ulLockEventNum--;
        #endif
        EnableLockFile(AHC_FALSE, 0);
        AHC_WMSG_Draw(AHC_TRUE, WMSG_UNLOCK_CUR_FILE, 2);
        AHC_Protect_SetType(AHC_PROTECT_NONE);
    }

    return AHC_TRUE;
}

AHC_BOOL VideoRecMode_Start(void)
{
    AHC_BOOL    ahc_ret = AHC_TRUE;
    UINT8       ubLCDstatus;
    MMP_UBYTE   ubNightMode;

    bShowHdmiWMSG   = AHC_TRUE;
    bShowTvWMSG     = AHC_TRUE;

    //printc(FG_GREEN("***** pf_SLOWMOTION_EnGet() = %d \n"),pf_SLOWMOTION_EnGet());
    //20150611 Terry
    if(pf_SLOWMOTION_EnGet() == SLOWMOTION_X2) {
        //printc(FG_RED("SLOWMOTION_X2\n"));
    }
    else if(pf_SLOWMOTION_EnGet() == SLOWMOTION_X4) {
        //printc(FG_RED("SLOWMOTION_X4\n"));
    }
    else if(pf_SLOWMOTION_EnGet() == SLOWMOTION_X8){
        //printc(FG_RED("SLOWMOTION_X8\n"));
    }
    else {
        SetCurrentOpMode(VIDEOREC_MODE);
        //printc(FG_RED("SLOWMOTION_X1\n"));
    }
 
    #if (MENU_MOVIE_PRE_RECORD_EN) 
    if(MenuSettingConfig()->uiMOVPreRecord == MOVIE_PRE_RECORD_ON)
    	AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_PRERECORD_STATUS, 1);
    else
    	AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_PRERECORD_STATUS, 0); 
    #endif  

	#if (MENU_MOVIE_NIGHT_MODE_EN)	
    AHC_Menu_SettingGetCB((char *)COMMON_KEY_NIGHT_MODE_EN, &ubNightMode);
    if(ubNightMode == COMMON_NIGHT_MODE_EN_ON)
    	AHC_SetParam(PARAM_ID_NIGHT_MODE_AE, 1);
    else
    	AHC_SetParam(PARAM_ID_NIGHT_MODE_AE, 0); 
    #endif

    AHC_OSDSetActive(0, 0);

    if(AHC_WMSG_States())
       AHC_WMSG_Draw(AHC_FALSE, WMSG_NONE, 0);

	#if (!SWITCH_MODE_FREEZE_WIN)
    AHC_OSDSetActive(OVL_DISPLAY_BUFFER, 0);
	#endif

    ahc_ret = VideoFunc_Preview();
    if (AHC_TRUE != ahc_ret){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk, ahc_ret); return ahc_ret;}

    //Enter video pre-encode flow.
    if (MenuSettingConfig()->uiMOVPreRecord == MOVIE_PRE_RECORD_ON){
          ahc_ret = VideoFunc_PreRecord();  
    }
    
#if (SUPPORT_TOUCH_PANEL) && !defined(_OEM_TOUCH_)
    KeyParser_ResetTouchVariable();
    KeyParser_TouchItemRegister(&VideoMainPage_TouchButton[0], ITEMNUM(VideoMainPage_TouchButton));
#endif
    VideoTimer_Start(VIDEO_TIMER_UNIT);

    bMuteRecord = (MenuSettingConfig()->uiMOVSoundRecord == MOVIE_SOUND_RECORD_ON)?(AHC_FALSE):(AHC_TRUE);

#if (DAY_NIGHT_MODE_SWITCH_EN)
    bNightMode = (MenuSettingConfig()->uiScene == SCENE_TWILIGHT)?(AHC_TRUE):(AHC_FALSE);
#endif

    AHC_LCD_GetStatus(&ubLCDstatus);

    if(ubLCDstatus == AHC_LCD_NORMAL)
        DrawStateVideoRecUpdate(EVENT_LCD_COVER_NORMAL);
    else if(ubLCDstatus == AHC_LCD_REVERSE)
        DrawStateVideoRecUpdate(EVENT_LCD_COVER_ROTATE);

//TODO:Need to review    AHC_OSDSetActive(0, 1);
#if 0 //Andy Liu TBD
    if(AHC_IsHdmiConnect() && bShowHdmiWMSG) {
        bShowHdmiWMSG = AHC_FALSE;
        AHC_WMSG_Draw(AHC_TRUE, WMSG_HDMI_TV, 3);
    }
    else {
        if(AHC_IsTVConnectEx() && bShowTvWMSG) {
            bShowTvWMSG = AHC_FALSE;
            AHC_WMSG_Draw(AHC_TRUE, WMSG_HDMI_TV, 3);
        }
    }
#endif

    return ahc_ret;
}

void VideoRecMode_PreviewUpdate(void)
{
    UINT8       ubLCDstatus;
    AHC_BOOL    ubUpdatePreview   = AHC_FALSE;

    if(VideoRecSize != MenuSettingConfig()->uiMOVSize)
        ubUpdatePreview = AHC_TRUE;

    #if (ENABLE_ADAS_LDWS)
    {
        UINT32 bLDWS_En;

        if (LDWS_SettingChanged() == MMP_TRUE){
            ubUpdatePreview = AHC_TRUE;
        }

        if ( AHC_Menu_SettingGetCB( (char *)COMMON_KEY_LDWS_EN, &bLDWS_En ) == AHC_TRUE ) {
            if ((LDWS_EN_OFF == bLDWS_En) && (MMPS_Sensor_GetADASFeatureEn(PRM_SENSOR,MMPS_ADAS_LDWS) == MMP_TRUE) ) {
                ubUpdatePreview = AHC_TRUE;
            }
            else if ((LDWS_EN_ON == bLDWS_En) && (MMPS_Sensor_GetADASFeatureEn(PRM_SENSOR,MMPS_ADAS_LDWS) == MMP_FALSE) ) {
                ubUpdatePreview = AHC_TRUE;
            }
        }
		if(LDWS_IsStart())
		{
			LDWS_Unlock();
			ResetLDWSCounter();
		}
    }
    #endif

    #if (ENABLE_ADAS_FCWS)
    {
        UINT32 bFCWS_En;

        if ( AHC_Menu_SettingGetCB( (char *)COMMON_KEY_FCWS_EN, &bFCWS_En ) == AHC_TRUE ) {
            if ((FCWS_EN_OFF == bFCWS_En) && (MMPS_Sensor_GetADASFeatureEn(PRM_SENSOR,MMPS_ADAS_FCWS) == MMP_TRUE) ) {
                ubUpdatePreview = AHC_TRUE;
            }
            else if ((pf_FCWS_EnGet() == FCWS_EN_ON) && (MMPS_Sensor_GetADASFeatureEn(PRM_SENSOR,MMPS_ADAS_FCWS) == MMP_FALSE) ) {
                ubUpdatePreview = AHC_TRUE;
            }
        }
    }
    #endif

    {
        UINT32 hdr_En;

        if ( AHC_Menu_SettingGetCB( (char *)COMMON_KEY_HDR_EN, &hdr_En ) == AHC_TRUE ) {
            if ((MMP_IsVidHDREnable() == MMP_TRUE) && (hdr_En == COMMON_HDR_EN_OFF) ) {
                ubUpdatePreview = AHC_TRUE;
            }
            else if ((MMP_IsVidHDREnable() == MMP_FALSE) && (hdr_En == COMMON_HDR_EN_ON) ) {
                ubUpdatePreview = AHC_TRUE;
            }
        }
    }

#if (MOTION_DETECTION_EN)
    if (m_ubMotionDtcEn && (MMPS_Sensor_IsVMDStarted(PRM_SENSOR) == MMP_FALSE)) {
        printc(FG_GREEN("Motion setting changed - Need turn-on\r\n"));
        ubUpdatePreview = AHC_TRUE;
    }
    else if (!m_ubMotionDtcEn && (MMPS_Sensor_IsVMDStarted(PRM_SENSOR) == MMP_TRUE)) {
        printc(FG_GREEN("Motion setting changed - Need turn-off\r\n"));
        ubUpdatePreview = AHC_TRUE;
    }
#endif

	#if (VR_PREENCODE_EN)
    {
        UINT32  ulPreRecordStatus = 0;

        AHC_VIDEO_GetPreRecordStatus(&ulPreRecordStatus);

        if(bVideoPreRecordStatus != ulPreRecordStatus)
        {
            bVideoPreRecordStatus = ulPreRecordStatus;
            ubUpdatePreview       = AHC_TRUE;
        }
    }
	#endif
	
	#if (MENU_MOVIE_NIGHT_MODE_EN)
	{
	    MMP_UBYTE   ubNightMode;
		UINT32 		ulNightMode;
		AHC_GetParam(PARAM_ID_NIGHT_MODE_AE, &ulNightMode);
		AHC_Menu_SettingGetCB((char *)COMMON_KEY_NIGHT_MODE_EN, &ubNightMode);
		if((ubNightMode == COMMON_NIGHT_MODE_EN_OFF) && (ulNightMode == 1)) { 
		   AHC_SetParam(PARAM_ID_NIGHT_MODE_AE, 0);
		   ubUpdatePreview       = AHC_TRUE;  
		} else if((ubNightMode == COMMON_NIGHT_MODE_EN_ON)  && (ulNightMode == 0)) { 
		   AHC_SetParam(PARAM_ID_NIGHT_MODE_AE, 1);
		   ubUpdatePreview       = AHC_TRUE; 
		}    
	}	
	#endif

	g_bDrawUnfix = MMP_FALSE;
	
    AHC_LCD_GetStatus(&ubLCDstatus);

    if(ubLCDstatus == AHC_LCD_NORMAL)
        DrawStateVideoRecUpdate(EVENT_LCD_COVER_NORMAL);
    else if(ubLCDstatus == AHC_LCD_REVERSE)
        DrawStateVideoRecUpdate(EVENT_LCD_COVER_ROTATE);
    
    if(ubUpdatePreview == AHC_TRUE)
    {
		#if (VR_PREENCODE_EN)
        if(!m_ubPreEncodeEn)
            bDisableVideoPreRecord = AHC_TRUE;
		#endif
        AHC_SetMode(AHC_MODE_IDLE);
		#if (SWITCH_MODE_FREEZE_WIN)
		MMPS_Display_FreezeWinUpdate(AHC_TRUE, AHC_TRUE, AHC_TRUE);// Video preview to Video preview
		#endif
        VideoFunc_Preview();
    }

    bMuteRecord = (MenuSettingConfig()->uiMOVSoundRecord == MOVIE_SOUND_RECORD_ON)?(AHC_FALSE):(AHC_TRUE);

	#if (DAY_NIGHT_MODE_SWITCH_EN)
    bNightMode = (MenuSettingConfig()->uiScene == SCENE_TWILIGHT)?(AHC_TRUE):(AHC_FALSE);
	#endif

	#if (SUPPORT_TOUCH_PANEL) && !defined(_OEM_TOUCH_)
    KeyParser_ResetTouchVariable();
    KeyParser_TouchItemRegister(&VideoMainPage_TouchButton[0], ITEMNUM(VideoMainPage_TouchButton));
	#endif

	#if VIRTUAL_KEY_BOARD_FOR_WIFI
    {
        extern unsigned char ucWifiAPParamModified;
        extern unsigned char ucWifiSTAParamModified;
        extern unsigned char ucwifiStassid[6][64];
        extern unsigned char ucwifiStapswd[6][64];
        //amn_currConfig_get_enum( "Net.Dev.%d.BootProto", ucWifiAPParamModified )->v.strVal;

        printc("ucWifiAPParamModified %d, ucWifiSTAParamModified %d\n",ucWifiAPParamModified, ucWifiSTAParamModified);
        if(ucWifiAPParamModified)
        {
            ucWifiAPParamModified = 0;
            nhw_reset_network();
        }

        if(ucWifiSTAParamModified)
        {
            //0 none,  1 AES,  2 WEP
            switch (LwIP_join_WLAN_AP(ucwifiStassid[ucWifiSTAParamModified], 1, ucwifiStapswd[ucWifiSTAParamModified], 28 ))
            {
                case 1:
                    printc("... failed\n");
                    return;
                case 0:
                    printc("joined sucessful\n");
                    break;
            }

            if (LwIP_start_netif( NULL, NULL ) < 0)
            {
                nhw_set_status(NETAPP_NET_STATUS_NOIP);
            }
            else
            {
                nhw_set_status(NETAPP_NET_STATUS_READY);
            }

            ucWifiSTAParamModified = 0;
        }
    }
	#endif
	Menu_SetEV(MenuSettingConfig()->uiEV);
    VideoTimer_Start(VIDEO_TIMER_UNIT);
}

AHC_BOOL VideoFunc_TriggerEmerRecord(void)
{
#if (EMER_RECORD_DUAL_WRITE_ENABLE == 1)
    if(AHC_VIDEO_IsEmergRecStarted() == AHC_TRUE)
    {

        UINT32 uiTime;
        UINT32 uiInterval;

        AHC_VIDEO_GetEmergRecTime(&uiTime);
        //printc("Emer Cur Time : %d \n", uiTime);

        uiInterval = AHC_VIDEO_GetEmergRecInterval();
        uiTime = (uiTime+1000 - 1)/1000;
        //printc("Emer Cur Time2 : %d (%d)\n", uiTime, uiInterval);

        if(uiInterval < EMER_RECORD_DUAL_WRTIE_MAX_TIME)
        {
            if(uiTime >= (uiInterval - EMER_RECORD_DUAL_WRTIE_DELTA))
            {
                uiTime+=EMER_RECORD_DUAL_WRITE_POSTTIME;

                if(uiTime > EMER_RECORD_DUAL_WRTIE_MAX_TIME){
                    uiTime = EMER_RECORD_DUAL_WRTIE_MAX_TIME;
                }else if(uiTime < EMER_RECORD_DUAL_WRITE_INTERVAL){
                    uiTime = EMER_RECORD_DUAL_WRITE_INTERVAL;
                }

                AHC_VIDEO_SetEmergRecInterval(uiTime);
            }
        }

        return AHC_FALSE;
    }
    
    if ( (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE) || (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE) )
    {
    if( (AHC_VIDEO_IsEmergRecStarted() == AHC_FALSE) && (AHC_VIDEO_IsEmergPostDone() == AHC_TRUE) && (AHC_VIDEO_GetKernalEmergStopStep() == AHC_KERNAL_EMERGENCY_AHC_DONE) )
    {
        #if (GPS_RAW_FILE_EMER_EN == 1)
        if(GPSCtrl_GetGPSRawStart_Emer() == AHC_FALSE)
        #else
        if(1)
        #endif
        {
            UINT32 ulCurrentRecordedTime = 0;
            AHC_VIDEO_GetCurRecordingTime(&ulCurrentRecordedTime);

            AHC_VIDEO_GetEmergRecTimeOffset(&m_ulEventPreRecordTime);
            m_ulEventHappenTime = OSTimeGet();
            printc(FG_BLUE("VideoFunc_TriggerEmerRecord--ulEmergencyPreRecordTime-- %d (%d) \r\n"), m_ulEventPreRecordTime, ulCurrentRecordedTime);
            m_bCurrentTimeLessThanPreRecord = (m_ulEventPreRecordTime > ulCurrentRecordedTime)? AHC_TRUE : AHC_FALSE;

#if (GPS_CONNECT_ENABLE)
            AHC_GPS_FlushBackupBuffer( AHC_TRUE );
#endif
#if (GSENSOR_CONNECT_ENABLE)
            AHC_Gsensor_FlushBackupBuffer( AHC_TRUE );
#endif
            if (AHC_VIDEO_StartEmergRecord() == AHC_TRUE)
            {
                AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_RECORD);
                return AHC_TRUE;
            }
            else
            {
                return AHC_FALSE;
            }
        }
        else
        {
            return AHC_FALSE;
        }
    }
    }    
    else if((AHC_VIDEO_IsEmergRecStarted() == AHC_FALSE) && (AHC_VIDEO_IsEmergPostDone() == AHC_TRUE))
    {
        #if (GPS_RAW_FILE_EMER_EN == 1)
        if(GPSCtrl_GetGPSRawStart_Emer() == AHC_FALSE)
        #else
        if(1)
        #endif
        {
            AHC_VIDEO_StartEmergRecord();
            return AHC_TRUE;
        }
        else
        {
            return AHC_FALSE;
        }
    }
        
    #if (DELETION_BY_FILE_NUM)
    {
        UINT32 uiFileCount;
        DCF_DB_TYPE sCurDB;
       
        sCurDB = AHC_UF_GetDB();
        AHC_UF_GetTotalFileCount(&uiFileCount);
        if(uiFileCount >= AHC_UF_GetFileTH(sCurDB))
        {
            if(AHC_Deletion_RemoveEx(sCurDB, AHC_VIDEO_GetRecTimeLimit()) == AHC_FALSE)
            {
                printc(FG_RED("AHC_Deletion_Romove Error\r\n"));
        	    return AHC_FALSE;
        	}
        }
    }
    #endif
#endif

    return AHC_FALSE;
}

//******************************************************************************
//
//                              AHC State Video Mode
//
//******************************************************************************
void StateVideoRecMode_StartRecordingProc(UINT32 ulJobEvent)
{
    VIDEO_RECORD_STATUS retVal;

    if(AHC_SDMMC_BasicCheck()==AHC_FALSE)
    {
        #ifdef CFG_VIDEO_RECORD_FLICK_LED_BY_CARD_ERROR
        LedCtrl_FlickerLedByCustomer(LED_GPIO_SDMMC_ERROR, LED_GPIO_SDMMC_FLICK_PERIOD, LED_GPIO_SDMMC_FLICK_TIMES);
        #endif

        RTNA_DBG_Str(0, FG_RED("--E-- Memory Card Error\r\n"));
        CGI_SET_STATUS(ulJobEvent, CGI_ERR_CARD_ERROR/*-3*/);
        return;//break;
    }

    #if defined(CFG_ENABLE_VIDEO_REC_VIBRATION) && defined(CFG_VIDEO_REC_VIBRATION_TIME)
    if (AHC_SDMMC_IsSD1MountDCF())
    {
        UINT64 ulFreeSpace;

        VideoFunc_GetFreeSpace(&ulFreeSpace);

        if (ulFreeSpace)
        {
            AHC_Vibration_Enable(CFG_VIDEO_REC_VIBRATION_TIME);

            #if defined(CFG_VIDEO_REC_VIBRATION_TIMES) && (CFG_VIDEO_REC_VIBRATION_TIMES >= 2)
            {
                UINT16 times = CFG_VIDEO_REC_VIBRATION_TIMES - 1;

                while (times--) {
                    AHC_OS_SleepMs(CFG_VIDEO_REC_VIBRATION_TIME + 100);
                    AHC_Vibration_Enable(CFG_VIDEO_REC_VIBRATION_TIME);
                }
            }
            #endif

            AHC_OS_SleepMs(CFG_VIDEO_REC_VIBRATION_TIME);
        }
    }
    #endif  // defined(CFG_ENABLE_VIDEO_REC_VIBRATION) && defined(CFG_VIDEO_REC_VIBRATION_TIME)

    #if (DCF_DB_COUNT >= 2)
    if(uiGetParkingModeEnable() == AHC_TRUE)
    {
        AHC_UF_SelectDB(DCF_DB_TYPE_2ND_DB);
    }
    else
    {
        AHC_UF_SelectDB(DCF_DB_TYPE_1ST_DB);
    }
    #endif

    RecordTimeOffset = 0;
    retVal = VideoFunc_Record();
    RTNA_DBG_Str(0, "StateVideoRecMode_StartRecordingProc: VideoFunc_Record return");
    RTNA_DBG_Short(0, retVal);
    RTNA_DBG_Str(0, "\r\n");

    // TODO:???
    switch (retVal) {
        case VIDEO_REC_START:
#ifdef CFG_ENABLE_VIDEO_REC_LED
                RecLED_Timer_Start(100);
#endif
                break;

            case VIDEO_REC_NO_SD_CARD:
            case VIDEO_REC_CARD_FULL:
#ifdef CFG_VIDEO_RECORD_FLICK_LED_BY_CARD_ERROR
                LedCtrl_FlickerLedByCustomer(LED_GPIO_SDMMC_ERROR, LED_GPIO_SDMMC_FLICK_PERIOD, LED_GPIO_SDMMC_FLICK_TIMES);
#endif
                break;

            case VIDEO_REC_PRERECD:
                printc("Enter pre record mode and wait to start record...\r\n");
                break;
    }

    if(uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE)
    {
	    if(VIDEO_REC_START != retVal && AHC_TRUE == m_ubParkingModeRecTrigger)
	        m_ubParkingModeRecTrigger = AHC_FALSE;
    }

    #if 0//(BUTTON_BIND_LED)
    if(VIDEO_REC_START != retVal)
        LedCtrl_ButtonLed(AHC_GetVideoModeLedGPIOpin(), AHC_FALSE);
    #endif

    if (VIDEO_REC_NO_SD_CARD == retVal) {
        CGI_SET_STATUS(ulJobEvent, CGI_ERR_NO_CARD /* NO SD */);
        AHC_WMSG_Draw(AHC_TRUE, WMSG_INSERT_SD_AGAIN, 3);
        printc("%s,%d error!",__func__,__LINE__);
        return;//break;
    }
    else if (VIDEO_REC_CARD_FULL == retVal) {
        CGI_SET_STATUS(ulJobEvent, CGI_ERR_CARD_FULL/*-4 *//* SD FULL */);
        AHC_WMSG_Draw(AHC_TRUE, WMSG_STORAGE_FULL, 3);
        printc("%s,%d error!",__func__,__LINE__);
        return;//break;
    }
    else if(VIDEO_REC_SEAMLESS_ERROR == retVal){
        CGI_SET_STATUS(ulJobEvent, CGI_ERR_SEAMLESS/*-5*/ /* INTERNAL ERROR */);
        AHC_WMSG_Draw(AHC_TRUE, WMSG_SEAMLESS_ERROR, 3);
        printc("%s,%d error!",__func__,__LINE__);
        return;//break;
    }
    else if(VIDEO_REC_SD_CARD_ERROR == retVal){
        CGI_SET_STATUS(ulJobEvent, CGI_ERR_CARD_ERROR/*-3*/ /* SD ERROR */);
        AHC_WMSG_Draw(AHC_TRUE, WMSG_FORMAT_SD_CARD, 3);
        printc("%s,%d error!",__func__,__LINE__);
        return;//break;
    }
    else if(VIDEO_REC_START == retVal){
        CGI_SET_STATUS(ulJobEvent, CGI_ERR_NONE/*0 *//* SUCCESSFULLY */);
    }
    else {
        CGI_SET_STATUS(ulJobEvent, CGI_ERR_CARD_ERROR/*-3*/ /* SD ERROR */);
    }

    if (VIDEO_REC_START == retVal)
        DrawStateVideoRecUpdate(ulJobEvent);
    else
        DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);
}

void StateVideoRecMode_StopRecordingProc(UINT32 ulJobEvent)
{
    UINT8	 Motion_Detection_Stable_Time = MOTION_DETECTION_STABLE_TIME_PARKING_NONE;
    
    Motion_Detection_Stable_Time = (uiGetParkingCfg()->bParkingModeFuncEn)? MOTION_DETECTION_STABLE_TIME_PARKING : MOTION_DETECTION_STABLE_TIME_PARKING_NONE;
    
    VideoFunc_RecordStop();

    #if (SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
    m_ulGSNRRecStopCnt = 0;
    #endif

    //not muted recoding every time when recording starts
    #ifdef CFG_REC_FORCE_UNMUTE //may be defined in config_xxx.h
    if (bMuteRecord)
        AHC_ToggleMute();
    #endif

    //TODO Add callbacks here
    #ifdef CFG_ENABLE_VIDEO_REC_LED
    RecLED_Timer_Stop();
    #endif

    if (uiGetParkingModeEnable() == AHC_TRUE) {
        #if (MOTION_DETECTION_EN)
        m_ulVMDStableCnt = Motion_Detection_Stable_Time * 1000 / VIDEO_TIMER_UNIT;
        m_ubVMDStart     = AHC_FALSE;
        #endif
        m_ubParkingModeRecTrigger = AHC_FALSE;
    }

    DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);
    
    #ifdef NET_SYNC_PLAYBACK_MODE
    if (ulJobEvent != EVENT_NET_ENTER_PLAYBACK)
    #endif
    	CGI_SET_STATUS(ulJobEvent, CGI_RET_STOP/*1*/ /* STOP */);
}

#if (ENABLE_ADAS_LDWS)

extern MMP_UBYTE ADAS_CTL_GetLDWSAttr(ldws_params_t *ldws_attr, MMP_LONG *alert);

void VideoFunc_LDWSWarn(void)
{
    ldws_params_t cur_ldws_attribute;
    MMP_LONG  ulAlert;

    ADAS_CTL_GetLDWSAttr(&cur_ldws_attribute, &ulAlert);

	#if (OSD_SHOW_LDWS_ALARM)
	if (ulAlert == LDWS_STATE_DEPARTURE_LEFT)
	{   // Left Shift
		AHC_WMSG_Draw(AHC_TRUE, WMSG_LDWS_LeftShift, 1);
		#ifdef OSD_SHOW_LDWS_ALARM_WITH_MULTI_AUDIO
		AHC_PlaySoundEffect(AHC_SOUNDEFFECT_ATTENTION);
		#endif
		AHC_PlaySoundEffect(AHC_SOUNDEFFECT_LDWS_WARNING);
	}
	else if(ulAlert == LDWS_STATE_DEPARTURE_RIGHT)
	{   // Right Shift
		AHC_WMSG_Draw(AHC_TRUE, WMSG_LDWS_RightShift, 1);
		#ifdef OSD_SHOW_LDWS_ALARM_WITH_MULTI_AUDIO
		AHC_PlaySoundEffect(AHC_SOUNDEFFECT_ATTENTION);
		#endif
		AHC_PlaySoundEffect(AHC_SOUNDEFFECT_LDWS_WARNING);
	}
	#endif
}
#endif

#if (ENABLE_ADAS_FCWS)
extern void DrawVideo_FCWSWarn(int ldws_state, AHC_BOOL bDraw);

void VideoFunc_FCWSWarn(void)
{
    AHC_PlaySoundEffect(AHC_SOUNDEFFECT_FCWS_WARNING);
	DrawVideo_FCWSWarn(0, AHC_TRUE);
}
#endif

#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE)
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_RECORDTIME_CONTROL)
{
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_RECORDRES_CONTROL)
{
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_FILE_CONTROL)
{
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_PICTURE_CONTROL)
{
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_GSENSOR_CONTROL)
{
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_AUDIO_CONTROL)
{
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_STATUS_CONTROL)
{
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_REC_MODE_CONTROL)
{
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_FIRMWARE_CONTROL)
{
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_MMC_CONTROL)
{
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_SWITCH_MSDC_MODE)
{
    AHC_DisconnectDevice();

    printc( "Set AHC idle mode...\r\n");
    
    printc( "Set USB MSDC mode...\r\n");

    StateSwitchMode(UI_MSDC_STATE);
}
#endif

#if (VIDEO_DIGIT_ZOOM_EN)
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_KEY_TELE_PRESS_LCD)
{
    AHC_BOOL    ret = AHC_TRUE;

    if((!bZoomStop)&&(bZoomDirect == AHC_SENSOR_ZOOM_OUT)){
        VideoFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
        bZoomStop = AHC_TRUE;
    }

    if(bZoomStop){
        ret = VideoFunc_ZoomOperation(AHC_SENSOR_ZOOM_IN);
        bZoomStop = AHC_FALSE;
        bZoomDirect = AHC_SENSOR_ZOOM_IN;
        DrawStateVideoRecUpdate(EVENT_KEY_TELE_PRESS);
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_KEY_WIDE_PRESS_LCD)
{
    AHC_BOOL    ret = AHC_TRUE;

    if((!bZoomStop)&&(bZoomDirect == AHC_SENSOR_ZOOM_IN)){
        ret = VideoFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
        if(ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, ret); }                 
        
        bZoomStop = AHC_TRUE;
    }

    if(bZoomStop){
        ret = VideoFunc_ZoomOperation(AHC_SENSOR_ZOOM_OUT);
        if(ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, ret); }                 
        
        bZoomStop = AHC_FALSE;
        bZoomDirect = AHC_SENSOR_ZOOM_OUT;
        DrawStateVideoRecUpdate(EVENT_KEY_WIDE_PRESS);
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_KEY_WIDE_STOP_LCD)
{
    AHC_BOOL    ret = AHC_TRUE;

    ret = VideoFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
    if(ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, ret); }                 
    
    bZoomStop = AHC_TRUE;
    DrawStateVideoRecUpdate(EVENT_KEY_WIDE_STOP);
}
#endif

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_EV_INCREASE_LCD)
{
    AHC_ChangeEV(AHC_FALSE, 1);    
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_EV_DECREASE_LCD)
{
    AHC_ChangeEV(AHC_FALSE, 0);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_KEY_LEFT_LCD)
{
    AHC_ChangeEV(AHC_TRUE, 0);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_KEY_UP_LCD)
{
#if (defined(SUPPORT_SPEECH_RECOG)&&(SUPPORT_SPEECH_RECOG))
    MMPS_Sensor_StartSpeechRecog(0,MMP_TRUE);
#endif
}


DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_KEY_RIGHT_LCD)
{
    AHC_ToggleFlashLED(LED_MODE_AUTO_ON_OFF);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_KEY_SET_LCD)
{
    UINT32      CurSysMode;

    AHC_GetSystemStatus(&CurSysMode);

    if( (CurSysMode == ((AHC_MODE_VIDEO_RECORD<<16)|AHC_SYS_VIDRECD_PAUSE)) ){
        VideoFunc_RecordResume();
        DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD);
    }
    else if((CurSysMode == ((AHC_MODE_VIDEO_RECORD<<16)|AHC_SYS_VIDRECD_RESUME)) ||
        (CurSysMode == ((AHC_MODE_VIDEO_RECORD<<16)|AHC_SYS_VIDRECD_START)) ) {
        VideoFunc_RecordPause();
        DrawStateVideoRecUpdate(EVENT_KEY_SET);
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_KEY_MENU_LCD)
{
#if CR_USE_STATE_SWITCH_SUB_MODE == 0
    AHC_BOOL    ret = AHC_TRUE;
	UI_STATE_ID ubParentUIState = 0;
#endif
	
#if defined(WIFI_PORT) && (WIFI_PORT == 1)
    if(AHC_STREAM_OFF != AHC_GetStreamingMode())
        return;//break;
#endif

	g_bDrawUnfix = MMP_FALSE;

    if (!VideoFunc_RecordStatus())
    {
        // Clean downcount screen
#if MOTION_DETECTION_EN
        if (m_ulVMDRemindTime)
        UpdateMotionRemindTime(-1);
#endif

#if 0
        DrawStateVideoRecUpdate(EVENT_KEY_MENU);
        VideoTimer_Stop();
#endif
		
#if CR_USE_STATE_SWITCH_SUB_MODE
        StateReplaceSubMode(UI_VIDEO_MENU_STATE); //  liao 20180316

		printc("~~~~~~~~~~long~~%s,%d, DettachSubMode:%d\r\n", __func__, __LINE__); // long 4-28
	//  StateSwitchMode(UI_VIDEO_MENU_STATE);
#else
		StateModeGetParent(uiGetCurrentState(), &ubParentUIState);
        if(UI_STATE_UNSUPPORTED != ubParentUIState){
            printc("%s,%d, DettachSubMode:%d\r\n", __func__, __LINE__, uiGetCurrentState());
            ret = StateDetachSubMode();
            if(ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, ret);}                                  
        }
        StateAttachSubMode(UI_VIDEO_MENU_STATE);
        if(ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ret); } 
		
#endif
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_KEY_MODE_LCD)
{
    AHC_BOOL ret = AHC_TRUE;
	
    #if defined(WIFI_PORT) && (WIFI_PORT == 1)
    if(AHC_STREAM_OFF != AHC_GetStreamingMode())
        return;//break;
    #endif

    AHC_PauseKeyUI();

    if(!VideoFunc_RecordStatus())
    {
        DrawStateVideoRecUpdate(EVENT_KEY_MODE);
        VideoTimer_Stop();

        #if (VR_PREENCODE_EN)
        bDisableVideoPreRecord = AHC_TRUE;
        #endif

        #if (MOTION_DETECTION_EN)
        // Reset MVD downcount to zero when switch to DSC mode
        if (m_ubInRemindTime) {
            m_ulVMDRemindTime = 0;
            m_ubInRemindTime = 0;
        }
        #endif

        
        if( uiGetParkingModeEnable() == AHC_TRUE && (uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE))
        {
            uiSetParkingModeEnable( AHC_FALSE );
            AHC_VIDEO_SetRecordMode(AHC_VIDRECD_STOP);
            AHC_VIDEO_SetRecordMode(AHC_VIDRECD_IDLE);
        }

        #if (DSC_MODE_ENABLE)
        ret = StateSwitchMode(UI_CAMERA_STATE);
        #else
        ret = StateSwitchMode(UI_BROWSER_STATE);
        #endif
        if(ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ret); }                         
    }

	#if (MENU_GENERAL_LCD_ROTATE_EN)
	if(MenuSettingConfig()->uiLCDRotate == LCD_ROTATE_ON){
		AHC_SNR_SetFlipDir(PRM_SENSOR, SENSOR_180_DEGREE);
	}
	#endif
	
    AHC_ResumeKeyUI();
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_ENTER_NET_PLAYBACK_LCD)
{
	AHC_PauseKeyUI();
			
	if (VideoFunc_RecordStatus())
	{
        #ifdef CFG_ENABLE_MIN_VR_TIME
        // For min VR time
        UINT32 ulTime = CFG_ENABLE_MIN_VR_TIME;

        AHC_VIDEO_GetCurRecordingTime(&ulTime);

        if (ulTime < CFG_ENABLE_MIN_VR_TIME)
            AHC_OS_SleepMs(CFG_ENABLE_MIN_VR_TIME - ulTime);
        #endif

        StateVideoRecMode_StopRecordingProc(EVENT_NET_ENTER_PLAYBACK);
    }
    

    DrawStateVideoRecUpdate(ulEvent);
    VideoTimer_Stop();

    #if (VR_PREENCODE_EN)
    bDisableVideoPreRecord = AHC_TRUE;
    #endif

    #if (MOTION_DETECTION_EN)
    // Reset MVD downcount to zero when switch to DSC mode
    if (m_ubInRemindTime) {
    	m_ulVMDRemindTime = 0;
        m_ubInRemindTime = 0;
    }
    #endif

    if( uiGetParkingModeEnable() == AHC_TRUE && (uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE))
    {
        uiSetParkingModeEnable( AHC_FALSE );
        AHC_VIDEO_SetRecordMode(AHC_VIDRECD_STOP);
        AHC_VIDEO_SetRecordMode(AHC_VIDRECD_IDLE);
    }

    StateSwitchMode(UI_NET_PLAYBACK_STATE);

    AHC_ResumeKeyUI();
    
    CGI_SET_STATUS(ulEvent, CGI_ERR_NONE /* SUCCESSFULLY */);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_KEY_RECORD_LCD)
{

	MMP_BOOL bParkGsensorCheck = (uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE 
                                  && uiGetParkingModeEnable()
                                  && (uiGetParkingCfg()->ubTriggerEncodeMethod & PARKING_MODE_TRIGGER_ENCODE_GSENSOR));
                                  
    printc("@@@ EVENT_VIDEO_KEY_RECORD -\r\n");

#if (FS_FORMAT_FREE_ENABLE)
    if (SystemSettingConfig()->byNeedToFormatMediaAsFormatFree > 0)
    {
        printc(FG_RED("MediaError!!! Need to format media as FORMAT FREE type!!!\r\n"));
        return;
    }
#endif

    if (bAudioRecording)
        return;

    AHC_PauseKeyUI();

    if (VideoFunc_RecordStatus()) {
        #ifdef CFG_ENABLE_MIN_VR_TIME
        // For min VR time
        UINT32 ulTime = CFG_ENABLE_MIN_VR_TIME;

        AHC_VIDEO_GetCurRecordingTime(&ulTime);

        if (ulTime < CFG_ENABLE_MIN_VR_TIME)
            AHC_OS_SleepMs(CFG_ENABLE_MIN_VR_TIME - ulTime);
        #endif

        StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);

        if(uiGetParkingModeEnable() == AHC_TRUE){
			DrawStateVideoRecUpdate(EVENT_VIDREC_UPDATE_MESSAGE);
            //Enter pre-encode.
            VideoFunc_PreRecord();
			DrawVideoParkingMode( uiGetParkingModeEnable() );
        }		
    }
    else {
        StateVideoRecMode_StartRecordingProc(EVENT_VIDEO_KEY_RECORD);
    }
    
#if (SUPPORT_GSENSOR)    
    if(bParkGsensorCheck)
		m_ubGsnrIsObjMove = AHC_FALSE;	
#endif

    AHC_ResumeKeyUI();
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_KEY_PLAYBACK_MODE_LCD)
{
    AHC_BOOL    ret = AHC_TRUE;

    if(!VideoFunc_RecordStatus())
    {
        DrawStateVideoRecUpdate(EVENT_KEY_PLAYBACK_MODE);
        VideoTimer_Stop();

#if (VR_PREENCODE_EN)
        bDisableVideoPreRecord = AHC_TRUE;
#endif
        ret = StateSwitchMode(UI_BROWSER_STATE);
        if(ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ret); }
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_POWER_OFF_LCD)
{
    AHC_BOOL    ret = AHC_TRUE;

    AHC_PauseKeyUI();
    printc("Video Rec Mode OFF\r\n");
    if( VideoFunc_RecordStatus() )
    {
          printc("Video Rec Mode OFF   2\r\n");
        ret = VideoFunc_RecordStop();
        if(ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ret); }
        
        DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);
        AHC_OS_Sleep(100);
    }

#ifdef CFG_BOOT_CLEAR_ICON_WHEN_OFF //may be defined in config_xxx.h
    //If LCD gets poor quality, and icons is sticky
    //Need to clear icons
    DrawStateVideoRecClearIcon();
#endif

    AHC_PowerOff_NormalPath();
    AHC_ResumeKeyUI();
/*
	ret = StateSwitchMode(UI_VIDEO_STATE);
	if(ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ret); } 
	
	#if defined(WIFI_PORT) && (WIFI_PORT == 1)
    if (AHC_TRUE == AHC_WiFi_Switch(AHC_TRUE)) {
        Setpf_WiFi(WIFI_MODE_ON);
        // Need save menusetting to Flash - TBD
    }
    #endif
	
	LedCtrl_LcdBackLight(AHC_TRUE);
	*/
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_CHANGE_LED_MODE_LCD)
{
#if (LED_FLASH_CTRL!=LED_BY_NONE)
    AHC_ToggleFlashLED(LED_MODE_ON_OFF);
#endif
}

#ifdef LED_GPIO_LASER
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_LASER_LED_ONOFF_LCD)
{
    if (LED_GPIO_LASER != MMP_GPIO_MAX)
    {
        static MMP_UBYTE flag = AHC_FALSE;

        if(!flag)
        {
            LedCtrl_LaserLed(LED_GPIO_LASER_ACT_LEVEL);
            flag = AHC_TRUE;
        }
        else
        {
            LedCtrl_LaserLed(!LED_GPIO_LASER_ACT_LEVEL);
            flag = AHC_FALSE;
        }
    }
}
#endif

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_FORMAT_MEDIA_LCD)
{
#if (QUICK_FORMAT_SD_EN || POWER_ON_BUTTON_ACTION)
    printc(">>> EVENT_FORMAT_MEDIA or EVENT_FORMAT_RESET_ALL\n");

    if( VideoFunc_RecordStatus() )
    {
        StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);

        AHC_OS_SleepMs(500);
        QuickMediaOperation(MEDIA_CMD_FORMAT);

        AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, KEY_VIDEO_RECORD, 0);
    }
    else
    {
        QuickMediaOperation(MEDIA_CMD_FORMAT);
    }
#endif
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_KEY_CAPTURE_LCD)
{
#ifdef KEY_CAPATURE_SWITCH_VIDEO //may be defined in config_xxx.h
    AHC_BOOL    ret = AHC_TRUE;
#endif
    if(AHC_SDMMC_BasicCheck()==AHC_FALSE) {
        #if (USE_SHUTTER_SOUND)
        AHC_PlaySoundEffect(AHC_SOUNDEFFECT_BUTTON);
        #endif
        // Call trigger the op is failed!!
        VideoFunc_ShutterFail();

        return;//break;
    }

    AHC_PauseKeyUI();

//Switch to DSC Mode then switch back
#ifdef KEY_CAPATURE_SWITCH_VIDEO //may be defined in config_xxx.h
    if(!VideoFunc_RecordStatus())
    {
        DrawStateVideoRecUpdate(EVENT_KEY_MODE);
        VideoTimer_Stop();

        #if (VR_PREENCODE_EN)
        bDisableVideoPreRecord = AHC_TRUE;
        #endif

        ret = StateSwitchMode(UI_CAMERA_STATE);

        if(ret)
            SetKeyPadEvent(KEY_DSC_CAPTURE);
        else
            VideoFunc_Shutter();
    }
    else
#endif
    {
        DCF_DB_TYPE sType = AHC_UF_GetDB();
        AHC_BOOL bSuccess;
        #if (DCF_DB_COUNT >= 4)
        AHC_UF_SelectDB(DCF_DB_TYPE_4TH_DB);
        #endif

        #if defined(WIFI_PORT) && (WIFI_PORT == 1)
        #if (HANDLE_JPEG_EVENT_BY_QUEUE)
        if (!MMPF_JPEG_GetCtrlByQueueEnable()) {
            AHC_SetStreamingMode(AHC_STREAM_PAUSE);
        }
        #endif
        #endif

        #if (USE_SHUTTER_SOUND)
        AHC_PlaySoundEffect(AHC_SOUNDEFFECT_SHUTTER);
        #endif

        #ifdef CFG_CAPTURE_WITH_VIBRATION
        AHC_Vibration_Enable(CFG_CAPTURE_VIBRATION_TIME);
        #endif

        #ifdef CFG_CAPTURE_WITH_KEY_LED_CAPTURE
        LedCtrl_ButtonLed(KEY_LED_CAPTURE, AHC_TRUE);
        #endif

        DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_CAPTURE);
        bSuccess = VideoFunc_Shutter();

        #if defined(WIFI_PORT) && (WIFI_PORT == 1)
        #if (HANDLE_JPEG_EVENT_BY_QUEUE)
        if (!MMPF_JPEG_GetCtrlByQueueEnable()) {
            AHC_SetStreamingMode(AHC_STREAM_RESUME);
        }
        #endif
        ncgi_op_feedback((void*)ulEvent, (int)(bSuccess? 0:-1)) ;//netapp_CGIOpFeedback((void*)ulEvent, (int)(bSuccess? 0:-1)) ;
        #endif

        if (VideoFunc_RecordStatus()) {
            DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD);
        } else {
            DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);
        }

        AHC_UF_SelectDB(sType);
    }

    AHC_ResumeKeyUI();
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_RECORD_MUTE_LCD)
{
   // AHC_ToggleMute();
   AHC_ReciveAduio2(); // long 4-23
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_LCD_POWER_SAVE_LCD)
{
    AHC_SwitchLCDBackLight();
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_CUS_SW1_ON_LCD)
{
    if(!VideoFunc_RecordStatus())
    {
        printc("MenuSettingConfig()->uiMOVSize %d\n", MenuSettingConfig()->uiMOVSize);
        if(MenuSettingConfig()->uiMOVSize == MOVIE_SIZE_1080P)
        {
            MenuSettingConfig()->uiMOVSize= MOVIE_SIZE_720_60P;
            AHC_SetMode(AHC_MODE_IDLE);
            VideoFunc_Preview();
        }
    }
}

void STATE_VIDEO_REC_MODE_EVENT_CUS_SW1_OFF_LCD(UINT32 ulEvent)
{
    if(!VideoFunc_RecordStatus())
    {
        printc("MenuSettingConfig()->uiMOVSize %d\n", MenuSettingConfig()->uiMOVSize);
        if(MenuSettingConfig()->uiMOVSize == MOVIE_SIZE_720_60P)
        {
            MenuSettingConfig()->uiMOVSize= MOVIE_SIZE_1080P;	//MOVIE_SIZE_720P;
            AHC_SetMode(AHC_MODE_IDLE);
            VideoFunc_Preview();
        }
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_CHANGE_NIGHT_MODE_LCD)
{
    AHC_ToggleTwilightMode();
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_ALL_BROWSER_LCD)
{
    if(!VideoFunc_RecordStatus())
    {
        DrawStateVideoRecUpdate(EVENT_KEY_PLAYBACK_MODE);
        VideoTimer_Stop();

        #if (VR_PREENCODE_EN)
        bDisableVideoPreRecord = AHC_TRUE;
        #endif

        if(ulEvent==EVENT_CAMERA_BROWSER)
            SetCurrentOpMode(JPGPB_MODE);
        else if(ulEvent==EVENT_VIDEO_BROWSER)
            SetCurrentOpMode(MOVPB_MODE);
        else if(ulEvent==EVENT_ALL_BROWSER)
            SetCurrentOpMode(JPGPB_MOVPB_MODE);

        StateSwitchMode(UI_BROWSER_STATE);
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_CAMERA_PREVIEW_LCD)
{
    AHC_BOOL    ret = AHC_TRUE;

    if(!VideoFunc_RecordStatus())
    {
        DrawStateVideoRecUpdate(EVENT_KEY_MODE);
        VideoTimer_Stop();

        #if (VR_PREENCODE_EN)
        bDisableVideoPreRecord = AHC_TRUE;
        #endif

        ret = StateSwitchMode(UI_CAMERA_STATE);
        if(ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ret); }                         
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_AUDIO_RECORD_LCD)
{
    if(VideoFunc_RecordStatus())
        return;//break;

    if(bAudioRecording)
    {
        if(AHC_RecordAudioCmd(AHC_AUDIO_CODEC_MP3, AHC_AUDIO_RECORD_STOP))
        {
            bAudioRecording = AHC_FALSE;
            DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);
        }
    }
    else
    {
        if(AHC_SDMMC_BasicCheck()==AHC_FALSE)
            return;//break;

        if(AHC_RecordAudioCmd(AHC_AUDIO_CODEC_MP3, AHC_AUDIO_RECORD_START))
        {
            bAudioRecording = AHC_TRUE;
            DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD);
        }
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_SWITCH_VMD_MODE_LCD)
{
#if (MOTION_DETECTION_EN)


    if (!m_ubMotionDtcEn &&
#if VR_PREENCODE_EN
        !m_ubPreEncodeEn &&
#endif
        1)
    {
        VideoFunc_EnterVMDMode();

#ifdef CFG_LCD_RESET_POWER_SAVER_IN_VMD //may be defined in config_xxx.h
        LCDPowerSaveCounterReset();
#endif
    }
    else
    {
        VideoFunc_ExitVMDMode();

#ifdef CFG_LCD_RESET_POWER_SAVER_IN_VMD //may be defined in config_xxx.h
        LCDPowerSaveCounterReset();
#endif
    }
#endif

}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_SWITCH_PANEL_TVOUT_LCD)
{
    AHC_SwitchLCDandTVOUT();
}

#if (SUPPORT_TOUCH_PANEL) && !defined(_OEM_TOUCH_)
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_SWITCH_TOUCH_PAGE_LCD)
{
    m_ubShowVRTouchPage ^= 1;

    if(m_ubShowVRTouchPage)
    {
        KeyParser_ResetTouchVariable();
        KeyParser_TouchItemRegister(&VideoCtrlPage_TouchButton[0], ITEMNUM(VideoCtrlPage_TouchButton));

        DrawVideoRecStatePageSwitch(TOUCH_CTRL_PAGE);
    }
    else
    {
        KeyParser_ResetTouchVariable();
        KeyParser_TouchItemRegister(&VideoMainPage_TouchButton[0], ITEMNUM(VideoMainPage_TouchButton));

        DrawVideoRecStatePageSwitch(TOUCH_MAIN_PAGE);
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_TOUCH_MENU_LCD)
{
    //TBD
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_TOUCH_MODE_LCD)
{
    //TBD
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_TOUCH_RECORD_PRESS_LCD)
{
    //TBD
}
#endif

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_EVENT_DC_CABLE_IN_LCD)
{
#if defined(VMD_EN_BY_CHARGER_OUT)
    VideoFunc_ExitVMDMode();
    LedCtrl_LcdBackLight(AHC_TRUE);
#endif

#if (CHARGER_IN_ACT_VIDEO_MODE==ACT_RECORD_VIDEO)
    if(VideoFunc_RecordStatus()==AHC_FALSE)
        AHC_SetRecordByChargerIn(3);
#elif (CHARGER_IN_ACT_VIDEO_MODE == ACT_FORCE_POWER_OFF)
    SetKeyPadEvent(KEY_POWER_OFF);
#endif
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_DC_CABLE_OUT_LCD)
{
    AHC_Charger_SetEnable(AHC_FALSE);

    #if defined(VMD_EN_BY_CHARGER_OUT)
    if(MenuSettingConfig()->uiMotionDtcSensitivity != MOTION_DTC_SENSITIVITY_OFF)
    {
        LedCtrl_LcdBackLight(AHC_FALSE);
        if( VideoFunc_RecordStatus() )
        {
            VideoFunc_RecordStop();
            AHC_OS_Sleep(100);
        }

        VideoFunc_EnterVMDMode();
    }
    else
    #endif
    {
        if( VideoFunc_RecordStatus() )
        {
            #if (CHARGER_OUT_ACT_VIDEO_REC==ACT_FORCE_POWER_OFF || CHARGER_OUT_ACT_VIDEO_REC==ACT_DELAY_POWER_OFF)
            AHC_SetShutdownByChargerOut(AHC_TRUE);
            #endif
        }
        else
        {
            #if (CHARGER_OUT_ACT_OTHER_MODE==ACT_FORCE_POWER_OFF || CHARGER_OUT_ACT_OTHER_MODE==ACT_DELAY_POWER_OFF)
            AHC_SetShutdownByChargerOut(AHC_TRUE);
            #endif
        }
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_USB_DETECT_LCD)
{
    if (!AHC_IsUsbConnect()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return;}

    if (IsAdapter_Connect())
    {
#ifdef CFG_REC_IGNORE_USB //may be defined in config_xxx.h
        if (VideoFunc_RecordStatus()) {
            // Video Recording to ignore USB Charger
            return;//break;
        }
        else {
            //Power Off
        }
#else
        #if defined(VMD_EN_BY_CHARGER_OUT)
        VideoFunc_ExitVMDMode();
        LedCtrl_LcdBackLight(AHC_TRUE);
        #endif

        #if (CHARGER_IN_ACT_VIDEO_MODE == ACT_RECORD_VIDEO)
        if (VideoFunc_RecordStatus()==AHC_FALSE)
            AHC_SetRecordByChargerIn(3);
        #elif (CHARGER_IN_ACT_VIDEO_MODE == ACT_FORCE_POWER_OFF)
        SetKeyPadEvent(KEY_POWER_OFF);
        #endif
#endif
    }
    else
    {
        #if (SUPPORT_UVC_FUNC)
        if (MMP_TRUE == PCAM2MMP_GetUVCMixMode()){
        AHC_SetUsbMode(AHC_USB_MODE_WEBCAM);
        }
        else
        #endif
        {
            #ifdef CFG_REC_IGNORE_USB //may be defined in config_xxx.h
            if (VideoFunc_RecordStatus()) {
                // Video Recording to ignore USB
                return;//break;
            }
            else {
                VideoTimer_Stop();
            }
            #else
            if( VideoFunc_RecordStatus() )
            {
                StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);
                AHC_VIDEO_WaitVideoWriteFileFinish();
            }

            VideoTimer_Stop();
            #endif

            #if (VR_PREENCODE_EN)
            bDisableVideoPreRecord = AHC_TRUE;
            #endif

            #if (USB_MODE_SELECT_EN)
            if (MMPS_USB_NORMAL == MMPS_USB_GetStatus()) {
                StateSwitchMode(UI_USBSELECT_MENU_STATE);
            }
            else
            {
                #ifdef CFG_POWER_ON_ENTER_CAMERA_STATE
                StateSwitchMode(UI_CAMERA_STATE);
                #else
                StateSwitchMode(UI_VIDEO_STATE);
                #endif
            }
             #else
            if(MenuSettingConfig()->uiUSBFunction == MENU_SETTING_USB_MSDC)
            {
                StateSwitchMode(UI_MSDC_STATE);
            }
            else
            {
                StateSwitchMode(UI_PCCAM_STATE);
            }
            #endif
        }
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_USB_REMOVED_LCD)
{
    /* In Video state, the event should always be USB adapter out,
     * the real USB plug-out event should be received in MSDC state.
     *
     * The function AHC_USB_GetLastStatus() may return wrong status,
     * when USB plug-in/out then adapter plug-in.
     *
     */
    if (AHC_IsDcCableConnect() == AHC_TRUE)
        return;//break;

    #if (USB_MODE_SELECT_EN)
    if (ubUSBSelectedMode == USB_PTP_MODE)
        return;//break;
    #endif
    
    AHC_Charger_SetEnable(AHC_FALSE);
    #if defined(VMD_EN_BY_CHARGER_OUT)
    if(MenuSettingConfig()->uiMotionDtcSensitivity != MOTION_DTC_SENSITIVITY_OFF)
    {
        LedCtrl_LcdBackLight(AHC_FALSE);

        if( VideoFunc_RecordStatus() )
        {
            VideoFunc_RecordStop();
            AHC_OS_Sleep(100);
        }

        VideoFunc_EnterVMDMode();
    }
    else
    #endif
    {
        if( VideoFunc_RecordStatus() )
        {
            #if (CHARGER_OUT_ACT_VIDEO_REC==ACT_FORCE_POWER_OFF || CHARGER_OUT_ACT_VIDEO_REC==ACT_DELAY_POWER_OFF)
            AHC_SetShutdownByChargerOut(AHC_TRUE);
            #endif
        }
        else
        {
            #if (CHARGER_OUT_ACT_OTHER_MODE==ACT_FORCE_POWER_OFF || CHARGER_OUT_ACT_OTHER_MODE==ACT_DELAY_POWER_OFF)
            AHC_SetShutdownByChargerOut(AHC_TRUE);
            #endif
        }
    }

    if ((MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE) ||
        (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE) )
    {
        AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_AHC_DONE);
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVNET_SUB_MODE_ENTER_LCD)
{
    //NOP
    gbVideoInSubMode = AHC_TRUE;
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVNET_SUB_MODE_EXIT_LCD)
{
    gbVideoInSubMode = AHC_FALSE;
	g_bDrawUnfix = MMP_FALSE;    
    DrawStateVideoRecUpdate(EVENT_LCD_COVER_NORMAL); //TBD. Clear OSD buffer and reset transparent color.
}


DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDEO_KEY_SWAP_PIP_LCD)
{
    MMP_DISPLAY_WIN_ATTR    winattr;
    MMP_DISPLAY_DISP_ATTR   dispAtt = {0};
    MMP_USHORT              usDisplayWidth;
    MMP_USHORT              usDisplayHeight;

    if (MMP_IsUSBCamExist()) {
        
        UINT8                   action = 1;
        USB_DETECT_PHASE        USBCurrentStates = 6;

        printc("EVENT_VIDEO_KEY_SWAP_PIP \n");

     //   AHC_USBGetStates(&USBCurrentStates, AHC_USB_GET_PHASE_CURRENT);
        printc("USBCurrentStates=%d\r\n",USBCurrentStates);
        if (USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) {
            action = 0;
        }

        if (action == 0)
        {
            gbWinExchangedCnt++;

            MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, MMP_FALSE);
            MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID,  MMP_FALSE);
            
			if (AHC_IsHdmiConnect()) {
				usDisplayWidth  = MMPS_Display_GetConfig()->hdmi.usDisplayWidth;
			    usDisplayHeight = MMPS_Display_GetConfig()->hdmi.usDisplayHeight;
			}
			#if (TVOUT_PREVIEW_EN)
			else if (AHC_IsTVConnect()) {
				if(TV_SYSTEM_PAL == MenuSettingConfig()->uiTVSystem)
				{
				    usDisplayWidth = MMPS_Display_GetConfig()->paltv.usDisplayWidth;
				    usDisplayHeight = MMPS_Display_GetConfig()->paltv.usDisplayHeight;				
				}
				else
				{
				    usDisplayWidth = MMPS_Display_GetConfig()->ntsctv.usDisplayWidth;
					usDisplayHeight = MMPS_Display_GetConfig()->ntsctv.usDisplayHeight;				
				}
			}
			#endif
			else {
			    usDisplayHeight  = RTNA_LCD_GetAttr()->usPanelW;
			    usDisplayWidth = RTNA_LCD_GetAttr()->usPanelH;
			}
			 printc("usDisplayWidth=%d-------usDisplayHeight=%d\r\n",usDisplayWidth,usDisplayHeight);
			 printc("gbWinExchangedCnt % PIP_SWAP_TYPE_NUM=%d\r\n",gbWinExchangedCnt % PIP_SWAP_TYPE_NUM);
            switch (gbWinExchangedCnt % PIP_SWAP_TYPE_NUM)
            {
                case F_LARGE_R_SMALL:
            		printc("F_LARGE_R_SMALL \r\n");
                    MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, REAR_CAM_WINDOW_ID, FRONT_CAM_WINDOW_ID);

                    MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr);
 
                    dispAtt.usStartX = 0;
                    dispAtt.usStartY = 0;
                    dispAtt.usDisplayWidth = winattr.usWidth;
                    dispAtt.usDisplayHeight = winattr.usHeight;
                    dispAtt.usDisplayOffsetX = (usDisplayWidth - winattr.usWidth)>>1;
                    dispAtt.usDisplayOffsetY = (usDisplayHeight - winattr.usHeight)>>1;
                    
                    MMPS_Display_SetWindowAttrToDisp(FRONT_CAM_WINDOW_ID, winattr, dispAtt);                    		                        	                    
                    
                    AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID, (usDisplayWidth >> 1), (usDisplayHeight >>1), 0, 0, MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);

                    MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, AHC_TRUE);
                    MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, AHC_TRUE);
                    break;
                case F_SMALL_R_LARGE:
                		printc("F_SMALL_R_LARGE\r\n");
                	
                    MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, FRONT_CAM_WINDOW_ID, REAR_CAM_WINDOW_ID);
									
					#if (HDMI_ENABLE)	
					if (AHC_IsHdmiConnect())
					{
						MMPS_Display_SetWinScaleAndOffset(FRONT_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OPTIMAL,
														winattr.usWidth, winattr.usHeight, winattr.usWidth >> 1, winattr.usHeight >>1, 0, 0);  

						MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr);

						dispAtt.usStartX = 0;
						dispAtt.usStartY = 0;
						dispAtt.usDisplayWidth = winattr.usWidth >> 1;
						dispAtt.usDisplayHeight = winattr.usHeight >> 1;
						dispAtt.usDisplayOffsetX = (usDisplayWidth - winattr.usWidth)>>1;
						dispAtt.usDisplayOffsetY = (usDisplayHeight - winattr.usHeight)>>1;

						MMPS_Display_SetWindowAttrToDisp(FRONT_CAM_WINDOW_ID, winattr, dispAtt);											
					}	
					else
					#endif	
					{
						MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr);
						MMPS_Display_SetWinScaleAndOffset(FRONT_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OPTIMAL,
								  winattr.usWidth, winattr.usHeight, winattr.usWidth >> 1, winattr.usHeight >>1, 0, 0);
					}

                    AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID, usDisplayWidth, usDisplayHeight, 0, 0,MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);

                    MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, AHC_TRUE);
                    MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, AHC_TRUE);
                    break;
                   
                case ONLY_FRONT:
                	printc("ONLY_FRONT\r\n");
                	MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, FRONT_CAM_WINDOW_ID, REAR_CAM_WINDOW_ID);

                	MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr);
                	
                	dispAtt.usStartX = 0;
                    dispAtt.usStartY = 0;
                    dispAtt.usDisplayWidth = winattr.usWidth;
                    dispAtt.usDisplayHeight = winattr.usHeight;
                    dispAtt.usDisplayOffsetX = (usDisplayWidth - winattr.usWidth)>>1;
                    dispAtt.usDisplayOffsetY = (usDisplayHeight - winattr.usHeight)>>1;
	                   
                	MMPS_Display_SetWindowAttrToDisp(FRONT_CAM_WINDOW_ID, winattr, dispAtt);

                	//AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID, (usDisplayWidth >> 1), (usDisplayHeight >>1), 0, 0, MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);
                	AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID, (usDisplayWidth>>1), (usDisplayHeight>>1), (usDisplayWidth>>2), (usDisplayHeight>>2), MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);


                	MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, AHC_TRUE);
                	MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, MMP_FALSE);
                	break;
                	
                case ONLY_REAR:
                	printc("ONLY_REAR\r\n");
					
                	MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, REAR_CAM_WINDOW_ID, FRONT_CAM_WINDOW_ID);
                   	                          
                	MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr);               	
                  
                	AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID, usDisplayWidth, usDisplayHeight, 0, 0, MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);

                	MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, MMP_FALSE);
                	MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, AHC_TRUE);
			//MMPS_Display_SetWinScaleAndOffset(FRONT_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OPTIMAL,
										//		  160, 160, 1,2, 0, 10);		
		//	AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID, 320, 180, 0,30,MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);
                	break;
                 
                case F_TOP_R_BOTTOM:
                	printc("F_TOP_R_BOTTOM\r\n");
    				MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, FRONT_CAM_WINDOW_ID, REAR_CAM_WINDOW_ID);

                	MMPS_Display_SetWinScaleAndOffset(FRONT_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OUT,
                			                          usDisplayWidth, usDisplayHeight, usDisplayWidth, usDisplayHeight >> 1, 0, 0);

                    MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr); 
                    
                	dispAtt.usDisplayWidth  = winattr.usWidth;
                	dispAtt.usDisplayHeight = winattr.usHeight >> 1;
                	dispAtt.usDisplayOffsetX = (usDisplayWidth - winattr.usWidth)>>1;
                    dispAtt.usDisplayOffsetY = (usDisplayHeight - winattr.usHeight)>>1;
                	dispAtt.usStartX        = 0;
					
					#if (TVOUT_PREVIEW_EN)
					if (AHC_IsTVConnect())
					{
                		dispAtt.usStartY        = 0;
                	}
					else
					#endif
					{
	                	dispAtt.usStartY        = usDisplayHeight >> 2; //??
					}
                
                	MMPS_Display_SetWindowAttrToDisp(FRONT_CAM_WINDOW_ID, winattr, dispAtt);

    				#ifdef FULL_IMAGE_SHOW_IN_HALF_PANEL
                    AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID, 
                            						usDisplayWidth, usDisplayHeight >> 1, 
                            						0, usDisplayHeight >> 1,
                            						MMP_SCAL_FITMODE_OUT, AHC_FALSE);
    				#else
    				AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID, 
                            						usDisplayWidth , usDisplayHeight, 
                            						0, 0,
                            						MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);
    				#endif
    						
                	MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, AHC_TRUE);
                	MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, AHC_TRUE);
                	break;
              
                case F_LEFT_R_RIGHT:
                	printc("F_LEFT_R_RIGHT\r\n");
                    if(854 == usDisplayHeight) // patch for 480x854 panel UDF. TBD
                    {
        				MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, REAR_CAM_WINDOW_ID, FRONT_CAM_WINDOW_ID);

    					MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr);

                    	MMPS_Display_SetWinScaleAndOffset(  FRONT_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OUT,
                                                    		usDisplayWidth, usDisplayHeight,
                                                    		usDisplayWidth, usDisplayHeight,
                                                    		0, 0);

                    	dispAtt.usDisplayWidth  = usDisplayWidth;
                    	dispAtt.usDisplayHeight = usDisplayHeight;
    	 	            dispAtt.usStartX        = 0;//usDisplayWidth >> 2;//??
                    	dispAtt.usStartY        = 0;

                    	MMPS_Display_SetWindowAttrToDisp(FRONT_CAM_WINDOW_ID, winattr, dispAtt);

        				#ifdef FULL_IMAGE_SHOW_IN_HALF_PANEL
                        AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID,
                                						usDisplayWidth >> 1, usDisplayHeight,
                                						usDisplayWidth >> 1,0,
                                						MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);
        				#else
        				AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID,
                                						usDisplayWidth >> 1 , usDisplayHeight,
                                						0,0,
                                						MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);
        				#endif

                    	MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, AHC_TRUE);
                    	MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, AHC_TRUE);
                    	break;
                    }
    				MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, FRONT_CAM_WINDOW_ID, REAR_CAM_WINDOW_ID);

					MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr);
						
                	MMPS_Display_SetWinScaleAndOffset(  FRONT_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OUT,
                                                		usDisplayWidth, usDisplayHeight, 
                                                		usDisplayWidth >> 1, usDisplayHeight, 
                                                		0, 0);

                	dispAtt.usDisplayWidth  = usDisplayWidth >> 1 ;
                	dispAtt.usDisplayHeight = usDisplayHeight;
					#if (TVOUT_PREVIEW_EN)
					if (AHC_IsTVConnect())
					{
                		dispAtt.usStartX        = 0;
                	}
					else
					#endif
					{
	 	               	dispAtt.usStartX        = 0;//usDisplayWidth >> 2;//??
					}
                	dispAtt.usStartY        = 0;
                	
                	MMPS_Display_SetWindowAttrToDisp(FRONT_CAM_WINDOW_ID, winattr, dispAtt);

    				#ifdef FULL_IMAGE_SHOW_IN_HALF_PANEL
                    AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID, 
                            						usDisplayWidth >> 1, usDisplayHeight, 
                            						usDisplayWidth >> 1,0,
                            						MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);
    				#else
    				AHC_HostUVCVideoSetWinAttribute(REAR_CAM_WINDOW_ID, 
                            						usDisplayWidth , usDisplayHeight, 
                            						0,0,
                            						MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);
    				#endif
    						
                	MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, AHC_TRUE);
                	MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, AHC_TRUE);
                	break;
            }
        }
    }
    else if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) || 
             (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {

        AHC_WINDOW_RECT sFrontRect, sRearRect;
        UINT8           action = 1;

        if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER))
        {
            MMP_SNR_TVDEC_SRC_TYPE TVCurrentStates = 0;

            MMPS_Sensor_GetTVDecSrcType(&TVCurrentStates);
            
            if (TVCurrentStates != MMP_SNR_TVDEC_SRC_NO_READY) {
                action = 0;
            }
        }
        else {
            action = 0;
        }

        printc("EVENT_VIDEO_KEY_SWAP_PIP \n");

        if (action == 0)
        {
            gbWinExchangedCnt++;

            AHC_PreviewWindowOp(AHC_PREVIEW_WINDOW_OP_GET | AHC_PREVIEW_WINDOW_REAR, &sRearRect);
            AHC_PreviewWindowOp(AHC_PREVIEW_WINDOW_OP_GET | AHC_PREVIEW_WINDOW_FRONT, &sFrontRect);

            MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, MMP_FALSE);
            MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, MMP_FALSE);

			if (AHC_IsHdmiConnect()) {
				sRearRect.usWidth  = sFrontRect.usWidth/2;
				sRearRect.usHeight = sFrontRect.usHeight/2;
			}
#if (TVOUT_PREVIEW_EN)
			else if (AHC_IsTVConnect()) {
				sRearRect.usWidth  = sFrontRect.usWidth/2;
				sRearRect.usHeight = sFrontRect.usHeight/2;		
			}
#endif
            else
            {
			    usDisplayWidth  = RTNA_LCD_GetAttr()->usPanelW;
    		    usDisplayHeight = RTNA_LCD_GetAttr()->usPanelH;
            }
            
            switch (gbWinExchangedCnt % 4)
            {
            case F_LARGE_R_SMALL:
                printc("F_LARGE_R_SMALL \r\n");
                MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, REAR_CAM_WINDOW_ID, FRONT_CAM_WINDOW_ID);

                dispAtt.usDisplayWidth  = sFrontRect.usWidth;
                dispAtt.usDisplayHeight = sFrontRect.usHeight;

                if( usDisplayWidth > dispAtt.usDisplayWidth )
                {
                    dispAtt.usStartX = (usDisplayWidth - dispAtt.usDisplayWidth) / 2;
                }
                if( usDisplayHeight > dispAtt.usDisplayHeight )
                {
                    dispAtt.usStartY = (usDisplayHeight - dispAtt.usDisplayHeight) / 2;
                }
                
                MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr);
                MMPS_Display_SetWindowAttrToDisp(FRONT_CAM_WINDOW_ID, winattr, dispAtt);

                if (gbRearPreviewNeedResetBuf == AHC_FALSE)
            	{
        			dispAtt.usDisplayWidth = sRearRect.usWidth;
        			dispAtt.usDisplayHeight = sRearRect.usHeight;
        			
        			MMPS_Display_GetWinAttributes(REAR_CAM_WINDOW_ID, &winattr);
        			MMPS_Display_SetWindowAttrToDisp(REAR_CAM_WINDOW_ID, winattr, dispAtt);
            	}
            	else
            	{
            		MMPS_Display_SetWinScaleAndOffset(REAR_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OPTIMAL,
            				                          sRearRect.usWidth, sRearRect.usHeight, sFrontRect.usWidth / 2, sFrontRect.usHeight / 2, sFrontRect.usLeft, sFrontRect.usTop);
            	}
            	
            	MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, AHC_TRUE);
            	MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, AHC_TRUE);
            	break;
            case F_SMALL_R_LARGE:
            	printc("F_SMALL_R_LARGE \r\n");
            	MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, FRONT_CAM_WINDOW_ID, REAR_CAM_WINDOW_ID);

            	if (gbRearPreviewNeedResetBuf == AHC_FALSE)
            	{
            		MMPS_Display_SetWinScaleAndOffset(FRONT_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OPTIMAL,
            									      sFrontRect.usWidth, sFrontRect.usHeight, sRearRect.usWidth, sRearRect.usHeight, sRearRect.usLeft, sRearRect.usTop);
            	}
            	else
            	{
            		MMPS_Display_SetWinScaleAndOffset(FRONT_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OPTIMAL,
            		    						      sFrontRect.usWidth, sFrontRect.usHeight, sFrontRect.usWidth / 2, sFrontRect.usHeight / 2, sRearRect.usLeft, sRearRect.usTop);
            	}

                if( usDisplayWidth > sFrontRect.usWidth )
                {
                    sFrontRect.usLeft = (usDisplayWidth - sFrontRect.usWidth) / 2;
                }
                if( usDisplayHeight > sFrontRect.usHeight )
                {
                    sFrontRect.usTop = (usDisplayHeight - sFrontRect.usHeight) / 2;
                }
                
            	MMPS_Display_SetWinScaleAndOffset(REAR_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OPTIMAL,
            									  sRearRect.usWidth, sRearRect.usHeight, sFrontRect.usWidth, sFrontRect.usHeight, sFrontRect.usLeft, sFrontRect.usTop);
            	
            	MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, AHC_TRUE);
            	MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, AHC_TRUE);
            	break;
            case ONLY_FRONT:
            	printc("ONLY_FRONT \r\n");
        		MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, FRONT_CAM_WINDOW_ID, REAR_CAM_WINDOW_ID);

        		dispAtt.usDisplayWidth = sFrontRect.usWidth;
        		dispAtt.usDisplayHeight = sFrontRect.usHeight;
        		
        		MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr);

        		MMPS_Display_SetWindowAttrToDisp(FRONT_CAM_WINDOW_ID, winattr, dispAtt);

        		MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, AHC_TRUE);
        		MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, MMP_FALSE);
            	break;
            case ONLY_REAR:
            	printc("ONLY_REAR \r\n");
        		MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, REAR_CAM_WINDOW_ID, FRONT_CAM_WINDOW_ID);

                if( usDisplayWidth > sFrontRect.usWidth )
                {
                    sFrontRect.usLeft = (usDisplayWidth - sFrontRect.usWidth) / 2;
                }
                if( usDisplayHeight > sFrontRect.usHeight )
                {
                    sFrontRect.usTop = (usDisplayHeight - sFrontRect.usHeight) / 2;
                }

        		MMPS_Display_SetWinScaleAndOffset(REAR_CAM_WINDOW_ID, MMP_SCAL_FITMODE_OPTIMAL,
        		    							  sRearRect.usWidth, sRearRect.usHeight, sFrontRect.usWidth, sFrontRect.usHeight, sFrontRect.usLeft, sFrontRect.usTop);

        		MMPS_Display_SetWinActive(FRONT_CAM_WINDOW_ID, MMP_FALSE);
        		MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, AHC_TRUE);
            	break;
            }
        }
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_SD_DETECT_LCD)
{
    #if (FS_FORMAT_FREE_ENABLE)
    SystemSettingConfig()->byNeedToFormatMediaAsFormatFree = 0; //Reset it in case user plug-in correct card
    if( AHC_CheckMedia_FormatFree( AHC_MEDIA_MMC ) == AHC_FALSE )
    {
        SystemSettingConfig()->byNeedToFormatMediaAsFormatFree = 1;
    }
    else
    #endif
    {
    	AHC_RemountDevices(MenuSettingConfig()->uiMediaSelect);
    }
    
    AHC_UF_SetFreeChar(0, DCF_SET_FREECHAR, (UINT8 *) VIDEO_DEFAULT_FLIE_FREECHAR);

}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_SD_REMOVED_LCD)
{
    if(VideoFunc_RecordStatus())
    {
        StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);
        AHC_WMSG_Draw(AHC_TRUE, WMSG_NO_CARD, 3);
    }
    if(AHC_TRUE == AHC_SDMMC_IsSD1MountDCF())
    {
        AHC_DisMountStorageMedia(AHC_MEDIA_MMC);
        Enable_SD_Power(0 /* Power Off */);
    }

    if ((MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE) ||
        (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE))
    {
        AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_AHC_DONE);
    }
}

#if (TVOUT_ENABLE)
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_TV_DETECT_LCD)
{
    StateSwitchMode(UI_VIDEO_STATE);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_TV_REMOVED_LCD)
{
    bShowTvWMSG = AHC_TRUE;
    StateSwitchMode(UI_VIDEO_STATE);
}
#endif

#if (HDMI_ENABLE)
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_HDMI_DETECT_LCD)
{
    StateSwitchMode(UI_VIDEO_STATE);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_HDMI_REMOVED_LCD)
{
    bShowHdmiWMSG = AHC_TRUE;

    StateSwitchMode(UI_VIDEO_STATE);

    //RTNA_LCD_Backlight(MMP_TRUE);
}
#endif

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_LCD_COVER_OPEN_LCD)
{
    if (!LedCtrl_GetBacklightStatus())
        LedCtrl_LcdBackLight(AHC_TRUE);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_LCD_COVER_CLOSE_LCD)
{
    LedCtrl_LcdBackLight(AHC_FALSE);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_LCD_COVER_NORMAL_LCD)
{
    AHC_DrawRotateEvent(ulEvent);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VRCB_MEDIA_FULL_LCD)
{
    #if defined(CFG_VR_DISABLE_CYCLING_RECORDING)
    // No cycling record mode
    if (1)
    #else
    if (MenuSettingConfig()->uiMOVClipTime == MOVIE_CLIP_TIME_OFF)
    #endif
    {
        StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);
        AHC_WMSG_Draw(AHC_TRUE, WMSG_STORAGE_FULL, 3);
        #ifdef CFG_CUS_HANDLER_VR_MEDIA_FULL
        CFG_CUS_HANDLER_VR_MEDIA_FULL();
        #endif
    }
    else
    {
        #if 1//For Seamless Exception
        StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);
        RecordTimeOffset = 0;
        StateVideoRecMode_StartRecordingProc(EVENT_VIDEO_KEY_RECORD);
        #else
        if(VideoFunc_RecordRestart()==AHC_FALSE)
        {
            printc("VideoFunc_RecordRestart Fail\r\n");

            if(VideoFunc_RecordStatus())
            {
                StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);
            }
            break;
        }
        #endif
        DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD);
    }
    
    if(AHC_GetCurKeyEventID() == BUTTON_VRCB_MEDIA_FULL){
        AHC_SetCurKeyEventID(EVENT_NONE);
    }
    else{
    	AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0);
        printc("KeyEventID: BUTTON_VRCB_MEDIA_FULL is interrupted.\r\n");
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VRCB_FILE_FULL_LCD)
{
    AHC_BOOL ahcRet = AHC_FALSE;

    if (AHC_VIDEO_IsVRSeamless() == AHC_FALSE) {
        #if (VR_CONTAINER_TYPE==COMMON_VR_VIDEO_TYPE_3GP) ||\
            (VR_CONTAINER_TYPE==COMMON_VR_VIDEO_TYPE_AVI)
        StateVideoRecMode_StopRecordingProc(EVENT_VRCB_FILE_FULL);  
        DrawStateVideoRecUpdate(EVENT_VIDREC_UPDATE_MESSAGE);
        
        if(uiGetParkingModeEnable() == AHC_TRUE){
			DrawStateVideoRecUpdate(EVENT_VIDREC_UPDATE_MESSAGE);
            //Enter pre-encode.
            ahcRet = VideoFunc_PreRecord();
            DrawVideoParkingMode( uiGetParkingModeEnable() );
        }
        #endif
    }
    else {
        if(VideoFunc_RecordRestart()==AHC_FALSE){
            printc("--E-- VideoFunc_RecordRestart Fail\r\n");
            StateVideoRecMode_StopRecordingProc(EVENT_VRCB_FILE_FULL);           
            AHC_ShowErrorDialog(WMSG_STORAGE_FULL);

#ifdef CFG_REC_WD_RESET_AFTER_RESTART_FAIL //may be defined in config_xxx.h
            while (1);  // Let OEM WD reset.
#endif
        }
        else{                    
            DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD);
        }
    }
    
    if(AHC_GetCurKeyEventID() == BUTTON_VRCB_FILE_FULL){
        AHC_SetCurKeyEventID(EVENT_NONE);
    }
    else{
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0);
        printc("KeyEventID: BUTTON_VRCB_FILE_FULL is interrupted.\r\n");
    }            
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VRCB_LONG_TIME_FILE_FULL_LCD)
{
    printc("long time file full \r\n");
    StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);
    bVideoRecording = 0;

    DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);

    printc("long time file full start record\r\n");
    StateVideoRecMode_StartRecordingProc(EVENT_VIDEO_KEY_RECORD);

    bVideoRecording = 1;
}


DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VRCB_MEDIA_SLOW_LCD)
{
    if(VideoFunc_RecordStatus())
    {
        UINT8 bSDClass = 0;

        m_uiSlowMediaCBCnt++;

        if(m_uiSlowMediaCBCnt == SLOW_MEDIA_CB_THD)
        {
            bSDClass = AHC_SDMMC_GetClass(AHC_SD_0);

            if(bSDClass==0xFF || bSDClass < SLOW_MEDIA_CLASS)
            {
                StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);
                DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);
                AHC_ShowErrorDialog(WMSG_CARD_SLOW);
            }
            m_uiSlowMediaCBCnt = 0;
        }
    }
}


DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VRCB_RECDSTOP_CARDSLOW_LCD)
{
    AHC_BOOL ahcRet = AHC_FALSE;
    
    printc(" card slow\r\n");
    StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);
    bVideoRecording = 0;
    
    DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);

    #if 1
    if (AHC_VIDEO_GetKernalEmergStopStep() != AHC_KERNAL_EMERGENCY_AHC_DONE)
    {
        AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_AHC_DONE);
        AHC_VIDEO_SetEmergPostDone(AHC_TRUE);
        AHC_VIDEO_SetEmergRecStarted(AHC_FALSE);
    }
    #endif
    
    #if 0
    AHC_ShowErrorDialog(WMSG_CARD_SLOW);
	
    if(AHC_TRUE == AHC_SDMMC_GetMountState())
    {
        AHC_DisMountStorageMedia(AHC_MEDIA_MMC);
        Enable_SD_Power(0 /* Power Off */);
        
        if ((MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE) ||
            (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE) )
        {
            AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_AHC_DONE);
        }

		AHC_RemountDevices(MenuSettingConfig()->uiMediaSelect);
        AHC_UF_SetFreeChar(0, DCF_SET_FREECHAR, (UINT8 *) VIDEO_DEFAULT_FLIE_FREECHAR);
    }
    #else
    if(uiGetParkingModeEnable() == AHC_TRUE)
    {
        DrawStateVideoRecUpdate(EVENT_VIDREC_UPDATE_MESSAGE);
        //Enter pre-record.
        ahcRet = VideoFunc_PreRecord();
        DrawVideoParkingMode( uiGetParkingModeEnable() );
    }
    else 
    {
        DrawStateVideoRecUpdate(EVENT_VIDREC_UPDATE_MESSAGE);
        printc("card slow start record\r\n");
        StateVideoRecMode_StartRecordingProc(EVENT_VIDEO_KEY_RECORD);
        //VideoFunc_Record();
        bVideoRecording = 1;
    }
    #endif

    if(AHC_GetCurKeyEventID() == BUTTON_VRCB_RECDSTOP_CARDSLOW){
        AHC_SetCurKeyEventID(EVENT_NONE);
    }
    else{
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0);
        printc("KeyEventID: BUTTON_VRCB_RECDSTOP_CARDSLOW is interrupted.\r\n");
    }   
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VRCB_SEAM_LESS_LCD)
{
    //Prevent multiple event of Filefull and Seamless
}


DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VRCB_MOTION_DTC_LCD)
{
#if (MOTION_DETECTION_EN) && (VMD_ACTION & VMD_RECORD_VIDEO)


    if(m_ubVMDStart)
    {
        if (AHC_SDMMC_BasicCheck() == AHC_FALSE)
            return;//break;

        #if (PARKING_RECORD_FORCE_20SEC == 0)
        // Update VMD stop recording timer
        m_ulVMDStopCnt = AHC_GetVMDRecTimeLimit() * 10; // Video Time period is 100ms.
        #endif

        #if((SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN) && \
            (GSNR_PWRON_REC_BY && GSNR_PWRON_REC_BY_VMD))
        if(ubGsnrPwrOnActStart)
        {
            m_ulGSNRRecStopCnt = AHC_GSNR_PWROn_MovieTimeReset();
        }
        #endif
        
        if( m_ubParkingModeRecTrigger == AHC_FALSE && (uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE))      
        {
            m_ulVMDStopCnt = AHC_GetVMDRecTimeLimit() * 1000 / VIDEO_TIMER_UNIT;

            // VMD Trigger Video record and countdown (m_ulVMDStopCnt) to stop recording.
            printc(FG_RED("!!! SomeBody/SomeThing is Moved!!! m_ulVMDStopCnt=%d %d\r\n"), m_ulVMDStopCnt, EVENT_VRCB_MOTION_DTC);

            if (VideoFunc_RecordStatus() == AHC_FALSE)
                AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, KEY_VIDEO_RECORD, EVENT_VRCB_MOTION_DTC);

            if(uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE)
            m_ubParkingModeRecTrigger = AHC_TRUE;
            
        }
    }
#endif

}


DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VRCB_MEDIA_ERROR_LCD)
{
    RTNA_DBG_Str(0, FG_RED("EVENT_VRCB_MEDIA_ERROR\r\n"));
    AHC_WMSG_Draw(AHC_TRUE, WMSG_INSERT_SD_AGAIN, 2);
    if(VideoFunc_RecordStatus())
    {
        StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);
        AHC_ShowErrorDialog(WMSG_INSERT_SD_AGAIN);
    }
}


DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VR_START_LCD)
{
    #if (GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO)
    GPSCtrl_StartRecord();
    #endif
    #if (GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)
    AHC_Gsensor_StartRecord();
    #endif
}


DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VR_STOP_LCD)
{
#if (GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO)
    GPSCtrl_EndRecord();
#endif
#if (GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)
    AHC_Gsensor_EndRecord();
#endif
}


DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VR_WRITEINFO_LCD)
{
    //NOP
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_LOCK_VR_FILE_LCD)
{
    if(VideoFunc_RecordStatus()) {
        AHC_Protect_SetType(AHC_PROTECT_MENU);
        VideoFunc_SetFileLock();
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_LOCK_FILE_G_LCD)
{
    printc(FG_BLUE("@@@ EVENT_LOCK_FILE_G\r\n"));

#if (SUPPORT_GSENSOR) && (GSENSOR_FUNC & FUNC_LOCK_FILE)
    #if((SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN) && \
        (GSNR_PWRON_REC_BY & GSNR_PWRON_REC_BY_SHAKED) && \
        (GSNR_PWRON_MOVIE_SHAKED_ACT == GSNR_PWRON_MOVIE_LOCKED))
    if (ubGsnrPwrOnActStart && ubGsnrPwrOnFirstREC)
    {
        if ( VideoFunc_RecordStatus() && !VideoFunc_LockFileEnabled() && (m_ubGsnrIsObjMove == AHC_TRUE) )
        {
            RTNA_DBG_Str(0, FG_BLUE("Lock file by G-Sensor Power-On !!!\r\n"));

            AHC_Protect_SetType(AHC_PROTECT_G);

        #if (LIMIT_MAX_LOCK_FILE_NUM)
            #if (MAX_LOCK_FILE_ACT == LOCK_FILE_STOP)
            if (m_ulLockFileNum < MAX_LOCK_FILE_NUM)
            #endif
            {
                AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
                EnableLockFile(AHC_TRUE, LOCK_FILE_CUR);
                m_ulLockEventNum++;
            }
        #else
            {
                AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
                EnableLockFile(AHC_TRUE, LOCK_FILE_CUR);
            }
        #endif

            m_ubGsnrIsObjMove = AHC_FALSE;  //Reset
            ubGsnrPwrOnFirstREC = AHC_FALSE;
        }
    }
    else
    #endif//POWER_ON_BY_GSENSOR_EN
    if ( (MenuSettingConfig()->uiGsensorSensitivity != GSENSOR_SENSITIVITY_OFF) &&
         (VideoFunc_RecordStatus() && !VideoFunc_LockFileEnabled()) )
    {
        if (m_ubGsnrIsObjMove == AHC_TRUE)
        {
            printc("CarDV is Shaking !! Lock Previous/Current Files\r\n");

            AHC_Protect_SetType(AHC_PROTECT_G);

        #if (LIMIT_MAX_LOCK_FILE_NUM)
            #if (MAX_LOCK_FILE_ACT == LOCK_FILE_STOP)
            if (m_ulLockFileNum < MAX_LOCK_FILE_NUM)
            #endif
            {
                AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
                EnableLockFile(AHC_TRUE, VR_LOCK_FILE_TYPE);
                m_ulLockEventNum++;
            }
        #else
            {
                AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
                EnableLockFile(AHC_TRUE, VR_LOCK_FILE_TYPE);
            }
        #endif

            m_ubGsnrIsObjMove = AHC_FALSE;//Reset
        }
    }
    else
    {
        m_ubGsnrIsObjMove = AHC_FALSE;//Reset
    }
#endif
}

#if (AHC_SHAREENC_SUPPORT == 0) || (AHC_EMERGENTRECD_SUPPORT)
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VR_EMERGENT_LCD)
{
    if( uiGetParkingModeEnable() == AHC_TRUE ) {
    	return;
    }
    if( MenuSettingConfig()->uiGsensorSensitivity != GSENSOR_SENSITIVITY_OFF && VideoFunc_RecordStatus() )
    {        
        #if (GSENSOR_CONNECT_ENABLE)
        m_ubGsnrIsObjMove = AHC_FALSE;//Reset
        #endif

        printc("CarDV is Shaking !!!!!\r\n");
        
        switch (MMPS_3GPRECD_GetEmergActionType()) {
            case MMP_3GPRECD_EMERG_DUAL_FILE:
                printc("Action: Emerg Dual File.\r\n");
                break;
            case MMP_3GPRECD_EMERG_SWITCH_FILE:
                printc("Action: Emerg Switch File.\r\n");
                break;
            case MMP_3GPRECD_EMERG_MOVE_FILE:
                printc("Action: Emerg Move File.\r\n");
                break;
            case MMP_3GPRECD_EMERG_NO_ACT:
                printc("Action: Emerg No Act.\r\n");
                break;
        }
        
        if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_NO_ACT)
        {
            printc("No pre-defined action to do. \r\n");        
            return;
        }

        //AHC_PlaySoundEffect(AHC_SOUNDEFFECT_VR_EMER);
        #if (0)//(SUPPORT_SHARE_REC)
        if (!VideoFunc_IsShareRecordStarted())
        {
            AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
            (void)VideoFunc_StartShareRecord();//Always returns true
            CGI_SET_STATUS(WIRELESS_REC_SHORT, CGI_ERR_NONE)
        }
        else {
           	CGI_SET_STATUS(WIRELESS_REC_SHORT, CGI_ERR_INVALID_STATE)
        }
        #else
        if( (AHC_VIDEO_IsEmergRecStarted() == AHC_FALSE) &&
            (AHC_VIDEO_IsEmergPostDone() == AHC_TRUE))
        {
            if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_MOVE_FILE)
            {
                if (AHC_Deletion_RemoveEx(DCF_DB_TYPE_3RD_DB, AHC_VIDEO_GetRecTimeLimit()) == AHC_TRUE)
                {
                    printc("Emergency Record: Go !!!! \n");

                    if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_MOVE_FILE)
                    {                        
                        AHC_VIDEO_SetNormal2Emergency(AHC_TRUE);   
                        {
                            UINT32 ulTickCount = OSTimeGet();
                            //DrawVideo_UpdatebyEvent(EVENT_VR_EMERGENT);
                            AHC_VIDEO_SetEmergRecStarted(AHC_TRUE);
                            AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
                            printc("AHC_WMSG_Draw() Time = %d ms \n\r", ((OSTimeGet() - ulTickCount)*1000)/ OS_TICKS_PER_SEC);
                            //CGI_SET_STATUS(WIRELESS_REC_EMERGENCY, CGI_ERR_NONE)
                        }                        
                    }
                    else
                    {
                        if (VideoFunc_TriggerEmerRecord())
                        {
                            UINT32 ulTickCount = OSTimeGet();
                            //DrawVideo_UpdatebyEvent(EVENT_VR_EMERGENT);
                            AHC_VIDEO_SetEmergRecStarted(AHC_TRUE);
                            AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
                            printc("AHC_WMSG_Draw() Time = %d ms \n\r", ((OSTimeGet() - ulTickCount)*1000)/ OS_TICKS_PER_SEC);
                            //CGI_SET_STATUS(WIRELESS_REC_EMERGENCY, CGI_ERR_NONE)
                        }
                    }                    
                }
            }
            else 
            {
                if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE)
                {
                    VIDENC_FW_STATUS  status_vid;
                    MMPS_3GPRECD_GetRecordStatus(&status_vid);
                    if (status_vid != VIDENC_FW_STATUS_START)
                    {
                        printc(FG_RED("Failed to start Emergency record because Normal recording is not triggered!!!(1)\r\n"));
                        // Wait a while, then trigger Emergency record again.
                    	AHC_OS_Sleep(300);
        				AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_LOCK_FILE_G, 0);
                        return;
                    }
                }

                if (AHC_Deletion_RemoveEx(DCF_DB_TYPE_3RD_DB, EMER_RECORD_DUAL_WRTIE_MAX_TIME) == AHC_TRUE)
                {
                    printc("Emergency Record: Go !!!! \n");

                    if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_MOVE_FILE)
                    {                        
                        AHC_VIDEO_SetNormal2Emergency(AHC_TRUE);   
                        {
                            UINT32 ulTickCount = OSTimeGet();
                            //DrawVideo_UpdatebyEvent(EVENT_VR_EMERGENT);
                            AHC_VIDEO_SetEmergRecStarted(AHC_TRUE);
                            AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
                            printc("AHC_WMSG_Draw() Time = %d ms \n\r", ((OSTimeGet() - ulTickCount)*1000)/ OS_TICKS_PER_SEC);
                            //CGI_SET_STATUS(WIRELESS_REC_EMERGENCY, CGI_ERR_NONE)
                        }                        
                    }
                    else
                    {
                        if (VideoFunc_TriggerEmerRecord())
                        {
                            UINT32 ulTickCount = OSTimeGet();
                            //DrawVideo_UpdatebyEvent(EVENT_VR_EMERGENT);
                            AHC_VIDEO_SetEmergRecStarted(AHC_TRUE);
                            AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
                            printc("AHC_WMSG_Draw() Time = %d ms \n\r", ((OSTimeGet() - ulTickCount)*1000)/ OS_TICKS_PER_SEC);
                            //CGI_SET_STATUS(WIRELESS_REC_EMERGENCY, CGI_ERR_NONE)
                        }
                    }                                        
                }
            }            
        }
        else
        {
            printc("Emergency Record: Keep going !!!! \n");

            if(VideoFunc_TriggerEmerRecord())
            {
                //DrawVideo_UpdatebyEvent(EVENT_VR_EMERGENT);
            	//CGI_SET_STATUS(WIRELESS_REC_EMERGENCY, CGI_ERR_INVALID_STATE)
            }
        }
		#endif
    }
    else {
      	CGI_SET_STATUS(WIRELESS_REC_EMERGENCY, CGI_ERR_INVALID_STATE)
	}
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VRCB_EMER_DONE_LCD)
{
    if ((MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE) || 
        (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE) )
    {
        if (AHC_VIDEO_GetKernalEmergStopStep() == AHC_KERNAL_EMERGENCY_STOP)
            AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_AHC_DONE);
        else
            return;//break;
    }

    #if 0
    SystemSettingConfig()->byStartNormalRecordAfterEmergencyRecord = (UINT8)VideoFunc_RecordStatus();
    #else
    {
        if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE)
            SystemSettingConfig()->byStartNormalRecordAfterEmergencyRecord = AHC_FALSE;
        else
            SystemSettingConfig()->byStartNormalRecordAfterEmergencyRecord = AHC_TRUE;
    }    
    #endif
    AHC_VIDEO_EmergRecPostProcess();
    //DrawVideo_UpdatebyEvent(EVENT_VRCB_EMER_DONE);

    AHC_VIDEO_SetEmergRecStarted(AHC_FALSE);
}

#else
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VR_EMERGENT_LCD)
{
    MMP_ULONG   newtime;
    UINT32      ulTime = 0;
    AHC_PauseKeyUI();

    if( uiGetParkingModeEnable() == AHC_TRUE ) {
    	return;
    }
    if( (AHC_VIDEO_IsEmergRecStarted() == AHC_FALSE) && (AHC_VIDEO_IsEmergPostDone() == AHC_TRUE))
    {
        if (VideoFunc_RecordStatus())
        {
            printc(FG_PURPLE("EVENT_VR_EMERGENT: Go !!!! \r\n")); 
            AHC_VIDEO_SetEmergRecStarted(AHC_TRUE);
            AHC_VIDEO_SetEmergPostDone(AHC_FALSE);
            
            AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
            EnableLockFile(AHC_TRUE, VR_LOCK_FILE_TYPE);
            
            AHC_VIDEO_GetCurRecordingTime(&ulTime);
            printc(FG_PURPLE("Emer time -- %d ms\r\n"), ulTime);
            newtime = ulTime+(EMER_RECORD_WRITE_INTERVAL*1000);
            printc("new due time = %d ms\r\n", newtime);
            MMPS_3GPRECD_ChangeCurFileTimeLimit(newtime);
        }        
    }
    else if (AHC_VIDEO_IsEmergRecStarted() == AHC_TRUE)
    {
        printc("EVENT_VR_EMERGENT: Keep going !!!! \r\n");
    }

    AHC_ResumeKeyUI();
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VRCB_EMER_DONE_LCD)
{
    printc(FG_PURPLE("EVENT_VRCB_EMER_DONE -\r\n")); 
    printc("Emer file=%s\r\n", AHC_VIDEO_GetCurRecFileName(0));

    AHC_VIDEO_SetEmergPostDone(AHC_TRUE);
    AHC_VIDEO_SetEmergRecStarted(AHC_FALSE);        
}
#endif

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VIDREC_UPDATE_MESSAGE_LCD)
{
    MMPS_FS_SetCreationTime();
#if (VIDEO_DIGIT_ZOOM_EN ==1)
    if(!AHC_VIDEO_GetCurZoomStatus()){
        if(!bZoomStop){
            VideoFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
            bZoomStop = AHC_TRUE;	               
        }
    }
#endif

#if (MOTION_DETECTION_EN)
    if(m_ubInRemindTime && m_ulVMDRemindTime==0)
    {
        DrawStateVideoRecUpdate(ulEvent);
        m_ubInRemindTime = AHC_FALSE;
		DrawVideoParkingMode(uiGetParkingModeEnable());
        return;//break;
    }
#endif

#if (FLICKER_PWR_LED_AT_VR_STATE)
    if(VideoFunc_RecordStatus())
    {
        static AHC_BOOL ubLEDstatus = 0;
        LedCtrl_PowerLed(ubLEDstatus++ & 0x2);
    }
#endif

    {
        static UINT32 m_VidWarningCounter = 0;

        if(AHC_IsDialogPresent())
            m_VidWarningCounter++;
        else
            m_VidWarningCounter = 0;

        if(m_VidWarningCounter!=0 && m_VidWarningCounter%10==0)
            AHC_ShowSoundWarning();
    }

    DrawStateVideoRecUpdate(ulEvent);

}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_SHOW_FW_VERSION_LCD)
{
    AHC_WMSG_Draw(AHC_TRUE, WMSG_SHOW_FW_VERSION, 5);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_SHOW_GPS_INFO_LCD)
{
    #if (SUPPORT_GPS)
    if(bGPS_PageExist)
    {
        bGPS_PageExist = AHC_FALSE;
        DrawStateVideoRecUpdate(EVENT_LCD_COVER_NORMAL);
    }
    else
    {
        UINT8 bID0 = 0, bID1 = 0;

        CHARGE_ICON_ENABLE(AHC_FALSE);
        bGPS_PageExist = AHC_TRUE;
        OSDDraw_EnterDrawing(&bID0, &bID1);
        QuickDrawGPSInfoPage(bID0, AHC_FALSE);
        QuickDrawGPSInfoPage(bID0, AHC_TRUE);
        OSDDraw_ExitDrawing(&bID0, &bID1);
        CHARGE_ICON_ENABLE(AHC_TRUE);
    }
    #endif
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_GPSGSENSOR_UPDATE_LCD)
{
#if (GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO)
    if( 0 == (ulCounterForGpsGsnrUpdate % ((1000/VIDEO_TIMER_UNIT)/(gGPS_Attribute.ubStoreFreq))))
        AHC_GPS_SetCurInfo();
#endif
#if (GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)
    if( 0 == (ulCounterForGpsGsnrUpdate % ((1000/VIDEO_TIMER_UNIT)/(AHC_Gsensor_GetAttributeAddr()->ubStoreFreq))))
        AHC_Gsensor_SetCurInfo();
#endif
#if (GPS_CONNECT_ENABLE || GSENSOR_CONNECT_ENABLE)
    ulCounterForGpsGsnrUpdate++;
#endif
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_VR_OSD_SHOW_LCD)
{
#if (AUTO_HIDE_OSD_EN)
    m_ulOSDCounter  = 0;
    m_ubHideOSD     = AHC_FALSE;
    DrawStateVideoRecInit();
#endif
}

#if defined(WIFI_PORT) && (WIFI_PORT == 1)
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_OPEN_H264_STREAM_LCD)
{
    #if (VR_PREENCODE_EN)
    m_ubPreEncodeEn = AHC_TRUE;
    #endif
    AHC_SetMode(AHC_MODE_IDLE);
    VideoFunc_Preview();
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_CLOSE_H264_STREAM_LCD)
{
    #if (VR_PREENCODE_EN)
    m_ubPreEncodeEn = AHC_FALSE;
    #endif
    AHC_SetMode(AHC_MODE_IDLE);
    VideoFunc_Preview();
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_SWITCH_WIFI_STREAMING_MODE_LCD)
{
MMPF_PIO_EnableOutputMode(MMP_GPIO32, MMP_TRUE, MMP_TRUE);
MMPF_PIO_SetData(MMP_GPIO32, 1, MMP_TRUE); 
AHC_OS_SleepMs(800);
MMPF_PIO_SetData(MMP_GPIO32, 0, MMP_TRUE); 
/*
    if (NETAPP_NET_STATUS_NONE == nhw_get_status()) {
        return;//break;
    }

    if(aitstreamer_is_ready()==AHC_FALSE) {
        printc("--E-- A-I-T streamer is not ready !!!\n");
        return;//break;
    }

    if (nhw_get_status()) {
        AHC_WiFi_Toggle_StreamingMode();
    }*/
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_WIFI_SWITCH_TOGGLE_LCD)
{
    printc("EVENT_WIFI_SWITCH_TOGGLE -\r\n");

    if ((NETAPP_NET_STATUS_DOWN == nhw_get_status()) || (NETAPP_NET_STATUS_NONE == nhw_get_status()) || (WLAN_SYS_GetMode() == -1))
        AHC_WiFi_Switch(AHC_TRUE);
    else
        AHC_WiFi_Switch(AHC_FALSE);
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_WIFI_SWITCH_DISABLE_LCD)
{
    printc("EVENT_WIFI_SWITCH_DISABLE -\r\n");

    if (AHC_TRUE == AHC_WiFi_Switch(AHC_FALSE)) {
        Setpf_WiFi(WIFI_MODE_OFF);
        // Need save menusetting to Flash - TBD
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_WIFI_SWITCH_ENABLE_LCD)
{
    printc("EVENT_WIFI_SWITCH_ENABLE -\r\n");

    if (AHC_TRUE == AHC_WiFi_Switch(AHC_TRUE)) {
        Setpf_WiFi(WIFI_MODE_ON);
        // Need save menusetting to Flash - TBD
    }
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_CGI_DUMMY)
{
    printc("EVENT_WIFI_CGI_DUMMY -\r\n");

    printd(BG_RED("Get event ID %d. TBD")"\r\n", ulEvent);
    CGI_SET_STATUS(ulEvent, CGI_ERR_GENERAL)
}

extern AHC_BOOL     m_bStartShareRec;
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_REC_SHORT)
{
    printc("REC_SHORT is triggering --\r\n");

#if (AHC_SHAREENC_SUPPORT)
    if(bVideoRecording && (m_bStartShareRec == AHC_FALSE))
    {
    	AHC_WMSG_Draw(AHC_TRUE, WMSG_LOCK_CURRENT_FILE, 2);
    	if (!VideoFunc_StartShareRecord())
    	{
    	    CGI_SET_STATUS(WIRELESS_REC_SHORT, CGI_ERR_GENERAL)
    	}
    	else
    	CGI_SET_STATUS(WIRELESS_REC_SHORT, CGI_ERR_NONE)
    } else {
    	printc("The main video must be recording and the short video is not recording.\r\n");
		CGI_SET_STATUS(ulEvent, CGI_ERR_INVALID_STATE)
    }
#else
    printd(BG_RED("Get event ID %d. TBD")"\r\n", ulEvent);
    CGI_SET_STATUS(ulEvent, CGI_ERR_GENERAL)
#endif
}

#include "amn_sysobjs.h"
extern struct osal_ev_download_parm ev_download_param;
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_DOWNLOAD)
{
	/** @note If the downloading file is too small, when the event is processed here,
	 *  it is closed. In that file close will be seen twice.
	 *
	 *  @note Due to current code structure, only ulEvent is passed here.
	 *  now temporary use a quick-dirty global variable here.
	 *  A parameter for event data is expected.
	 *
	 *  @todo Compare the file name with DCF, if that is a certain file, then display
	 *  downloading icon or hide that icon while closed.
	 */
    printc("STATE_VIDEO_REC_MODE_EVENT_DOWNLOAD -\r\n");

    printd("event file:%s %s\r\n", ev_download_param.filename, ev_download_param.start ? "open": "close");
    if (ev_download_param.start == 0) {
    	ncgi_clr_notify_fn(NCGI_ANY_FN, ev_download_param.filename);
    }
}
#endif

AHC_BOOL VIDEO_REC_MODE_EVENT_PARKING_REGCB(AHC_BOOL bEnable)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    if(bEnable){
        //Init With default mode.
        ahcRet = AHC_VIDEO_SetRecordModeRegisterInit((void *)AHC_VIDEO_SetRecordModeInitParkingMode);    

        ahcRet = AHC_VIDEO_SetRecordModeRegisterUnInit((void *)AHC_VIDEO_SetRecordModeUnInitParkingMode);    
        
        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD, 
                    AHC_VIDRECD_MODE_API_SET_BITRATE, 
                    (void *)AHC_VIDEO_SetRecordModeSetBitRateParkingMode);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD, 
                    AHC_VIDRECD_MODE_API_SET_AUDIO_ENCODE, 
                    (void *)AHC_VIDEO_SetRecordModeSetAudioEncodeParkingMode);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD, 
                    AHC_VIDRECD_MODE_API_SET_TIME_LIMIT, 
                    (void *)AHC_VIDEO_SetRecordModeSetTimeLimitParkingMode);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD, 
                    AHC_VIDRECD_MODE_API_SET_SEAMLESS, 
                    (void *)AHC_VIDEO_SetRecordModeSetSeamlessParkingMode);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD, 
                    AHC_VIDRECD_MODE_API_SET_EMERGENCY, 
                    (void *)AHC_VIDEO_SetRecordModeSetEmergencyParkingMode);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PRERECORD_TO_RECORD, 
                    AHC_VIDRECD_MODE_API_REGISTER_CALLBACK, 
                    (void *)AHC_VIDEO_SetRecordModeRegisterCallbackParkingMode);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_SEAMLESS_START_NEXT_RECORD, 
                    AHC_VIDRECD_MODE_API_CYCLIC_DELETE_FILES, 
                    (void *)AHC_VIDEO_CyclicDeleteFilesParkingMode);        
    }
    else{
        ahcRet = AHC_VIDEO_SetRecordModeRegisterInit((void *)AHC_VIDEO_SetRecordModeInit);    
        
        ahcRet = AHC_VIDEO_SetRecordModeRegisterUnInit((void *)AHC_VIDEO_SetRecordModeUnInit);    
        
        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD, 
                    AHC_VIDRECD_MODE_API_SET_BITRATE, 
                    (void *)AHC_VIDEO_SetRecordModeSetBitRate);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD, 
                    AHC_VIDRECD_MODE_API_SET_AUDIO_ENCODE, 
                    (void *)AHC_VIDEO_SetRecordModeSetAudioEncode);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD, 
                    AHC_VIDRECD_MODE_API_SET_TIME_LIMIT, 
                    (void *)AHC_VIDEO_SetRecordModeSetTimeLimit);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD, 
                    AHC_VIDRECD_MODE_API_SET_SEAMLESS, 
                    (void *)AHC_VIDEO_SetRecordModeSetSeamless);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD, 
                    AHC_VIDRECD_MODE_API_SET_EMERGENCY, 
                    (void *)AHC_VIDEO_SetRecordModeSetEmergency);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_PRERECORD_TO_RECORD, 
                    AHC_VIDRECD_MODE_API_REGISTER_CALLBACK, 
                    (void *)AHC_VIDEO_SetRecordModeRegisterCallback);

        ahcRet =  AHC_VIDEO_SetRecordModeRegisterAction(
                    AHC_VIDRECD_FLOW_TYPE_SEAMLESS_START_NEXT_RECORD, 
                    AHC_VIDRECD_MODE_API_CYCLIC_DELETE_FILES, 
                    (void *)AHC_VIDEO_CyclicDeleteFiles);
    }

    return ahcRet;
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_PARKING_KEY_LCD)
{
    AHC_BOOL ahcRet = AHC_TRUE;

	if( MenuSettingConfig()->uiMotionDtcSensitivity == MOTION_DTC_SENSITIVITY_OFF ){
		return;
	}	
 
    if(AHC_IsSDInserted() && AHC_GetMountStorageMediaStatus() && AHC_SDMMC_IsSD1MountDCF()){
        //NOP
    }
    else{
        AHC_PRINT_RET_ERROR(0, 0);
		AHC_WMSG_Draw(AHC_TRUE, WMSG_NO_CARD, 2);
        printc("SD card is not ready yet!\r\n");        
        return;
    }

    //Enable Parking mode.
    if( uiGetParkingModeEnable() == AHC_FALSE ){
        ahcRet = AHC_VIDEO_ParkingModeStart();   

        //Enter video preview with motion detection.
        ahcRet = AHC_SetMode(AHC_MODE_RECORD_PREVIEW);
        if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, 0); return;}
        
        //Enter pre-encode flow.
        ahcRet = VideoFunc_PreRecord();
        if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, 0); return;}               
    }
    else{ //Disable Parking mode.
        ahcRet = AHC_VIDEO_ParkingModeStop();  
        ahcRet = VideoFunc_Preview();
    }

    DrawVideoParkingMode( uiGetParkingModeEnable() );
    //#endif

}

#if (UVC_HOST_VIDEO_ENABLE)
DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_USB_B_DEVICE_DETECT_LCD)
{
	UINT8 ParkingmodeStatus;

	ParkingmodeStatus = uiGetParkingModeEnable();
	
    /////////////////////////////////////////////////////// Description ////////////////////////////////////////////////////
    // EVENT_USB_B_DEVICE_DETECT: Got when Rear Cam streaming was detected
    // EVENT_USB_B_DEVICE_REMOVED: Got when Rear Cam streaming was disconnected
    // Note: When some UVC error happened (ex: AHC_HostUVCVideoSetEp0TimeoutCB() / AHC_HostUVCVideoSetFrmRxTimeout2CB() was called),
    //       EVENT_USB_B_DEVICE_REMOVED will be sent. Below sample code will stop current recording and start next recording with only Front Cam.
    //       Once AHC_USBDetectHandler() detected Rear Cam streaming was recovered, EVENT_USB_B_DEVICE_DETECT will be sent.
    //       Below sample code will stop current recording and start next recording with Front & Rear Cam.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        MMP_BOOL bStatus = MMP_FALSE;
        AHC_BOOL brecording=AHC_FALSE;

        //AHC_PRINT_RET_ERROR(0, 0);
        //Receive USB B device event during booting time.
        if (ulEvent == EVENT_USB_B_DEVICE_DETECT) {
            //Check if UVC preview is enabled
            MMPF_USBH_GetUVCPrevwSts(&bStatus);

			#if defined(WIFI_PORT) && (WIFI_PORT == 1)
            ncam_set_rear_cam_ready(AHC_TRUE);
			#endif
			
            if (bStatus){
                AHC_PRINT_RET_ERROR(0, 0);
                AHC_USB_PauseDetection(0);
                if((AHC_GetCurKeyEventID() == BUTTON_USB_B_DEVICE_DETECTED) ||
                    (AHC_GetCurKeyEventID() == BUTTON_USB_B_DEVICE_REMOVED)){
                    AHC_SetCurKeyEventID(EVENT_NONE);
                }
                else{
                    AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0);
                    printc("KeyEventID: BUTTON_USB_B_DEVICE_DETECTED,REMOVED is interrupted.\r\n");
                }                           
                return;//break;
            }
        }

        brecording = VideoFunc_RecordStatus();
        if(brecording)
        {
            StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD); //stop video record.           
            
            if (AHC_VIDEO_GetKernalEmergStopStep() != AHC_KERNAL_EMERGENCY_AHC_DONE)
            {
                AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_AHC_DONE);
                AHC_VIDEO_SetEmergPostDone(AHC_TRUE);
                AHC_VIDEO_SetEmergRecStarted(AHC_FALSE);
            }

            AHC_USB_PauseDetection(0);            
        }    
            
		if(ParkingmodeStatus)
		{
			AHC_OS_SleepMs(500);
			STATE_VIDEO_REC_MODE_EVENT_PARKING_KEY_LCD(ulEvent);
		}

        if(ulEvent == EVENT_USB_B_DEVICE_REMOVED){
            AIHC_UVCPreviewStop();
            //MMPS_Display_SetWinActive(REAR_CAM_WINDOW_ID, MMP_FALSE);
            /* BE CAREFUL, forced stop frame Rx and clear FIFO */
            MMPS_USB_StopFrmRxClrFIFO();
            /* BE CAREFUL, SPECIAL CASE! */
            /* forced to clear connect status because of disconnected, not from USB_OTG_ISR (PHY) */
            MMPS_USB_SetDevConnSts(MMP_FALSE);
            
            #if defined(WIFI_PORT) && (WIFI_PORT == 1)
            if (AHC_STREAM_REAR_USBH == ncam_get_cam_src((unsigned int)AHC_STREAM_H264)) {
        		AHC_SetStreamingMode(AHC_STREAM_OFF);
            }
            ncam_set_rear_cam_ready(AHC_FALSE);
            #endif
            
            if ((gbWinExchangedCnt != F_LARGE_R_SMALL) || (gbWinExchangedCnt != ONLY_FRONT)) {
                MMP_DISPLAY_WIN_ATTR winattr;
                MMP_DISPLAY_DISP_ATTR dispAtt = {0};

                MMPS_Display_SetWinPriority(WMSG_LAYER_WINDOW_ID, OSD_LAYER_WINDOW_ID, REAR_CAM_WINDOW_ID, FRONT_CAM_WINDOW_ID);

                AHC_Display_GetWidthHdight(&dispAtt.usDisplayWidth, &dispAtt.usDisplayHeight);
                MMPS_Display_GetWinAttributes(FRONT_CAM_WINDOW_ID, &winattr);

                MMPS_Display_SetWindowAttrToDisp(FRONT_CAM_WINDOW_ID, winattr , dispAtt);
            }
        }

#if 0
        AHC_SetMode(AHC_MODE_IDLE);
#endif

        if(ulEvent == EVENT_USB_B_DEVICE_REMOVED){
            /* reset USB */
            MMPS_USB_DisconnectDevice();
        }

        if (ulEvent == EVENT_USB_B_DEVICE_DETECT) {
            MMP_ERR sRet = MMP_ERR_NONE;
            MMP_USHORT                  usVideoPreviewMode;
            AHC_DISPLAY_OUTPUTPANEL OutputDevice;
            UINT32 ulFlickerMode = COMMON_FLICKER_50HZ;
                    
            AHC_Menu_SettingGetCB( (char *)COMMON_KEY_FLICKER, &ulFlickerMode);

            AHC_GetDisplayOutputDev(&OutputDevice);
            sRet = MMPS_Display_SetOutputPanel(MMP_DISPLAY_PRM_CTL, OutputDevice);
            if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

            switch(OutputDevice){
                case MMP_DISPLAY_SEL_MAIN_LCD:
                case MMP_DISPLAY_SEL_SUB_LCD:
                    usVideoPreviewMode = VIDRECD_NORMAL_PREVIEW_MODE;
                    break;
                case MMP_DISPLAY_SEL_NTSC_TV:
                    usVideoPreviewMode = VIDRECD_NTSC_PREVIEW_MODE;
                    break;
                case MMP_DISPLAY_SEL_PAL_TV:
                    usVideoPreviewMode = VIDRECD_PAL_PREVIEW_MODE;
                    break;
                case MMP_DISPLAY_SEL_HDMI:
                    usVideoPreviewMode = VIDRECD_HDMI_PREVIEW_MODE;
                    break;
                case MMP_DISPLAY_SEL_CCIR:	
                    usVideoPreviewMode = VIDRECD_CCIR_PREVIEW_MODE;
                    break;                    
                case MMP_DISPLAY_SEL_NONE:
                default:
                    usVideoPreviewMode = VIDRECD_NORMAL_PREVIEW_MODE;
                    printc("%s,%d  not support yet!"__func__, __LINE__);
                    break;
            }
                    
            if(!AIHC_UVCPreviewStart(usVideoPreviewMode, OutputDevice, ulFlickerMode))
        	{
        		printc("%s,%d,CNNT RearCam type(%X) fail!\r\n"__func__, __LINE__,CAM_GET_SCD);
        	}
        } 

		#if 0
        VideoFunc_Preview();
		#endif    
		
		if(ParkingmodeStatus)
		{
			AHC_OS_SleepMs(1000);
			STATE_VIDEO_REC_MODE_EVENT_PARKING_KEY_LCD(ulEvent);
		}

        if(brecording){
            DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);
			AHC_OS_SleepMs(500);//1000			
            StateVideoRecMode_StartRecordingProc(EVENT_VIDEO_KEY_RECORD); //restart video record.
        }
    }

    AHC_USB_PauseDetection(0);
    if((AHC_GetCurKeyEventID() == BUTTON_USB_B_DEVICE_DETECTED) ||
        (AHC_GetCurKeyEventID() == BUTTON_USB_B_DEVICE_REMOVED)){
        AHC_SetCurKeyEventID(EVENT_NONE);
    }
    else{
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0);
        printc("KeyEventID: BUTTON_USB_B_DEVICE_DETECTED,REMOVED is interrupted.\r\n");
    }  

}
#endif

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_TV_DECODER_SRC_TYPE_LCD)
{
    AHC_BOOL brecording=AHC_FALSE;

    MMP_SNR_TVDEC_SRC_TYPE sSnrTVSrc;

    MMPS_Sensor_GetTVDecSrcType(&sSnrTVSrc);
   
    if ((sSnrTVSrc == MMP_SNR_TVDEC_SRC_PAL) || (sSnrTVSrc == MMP_SNR_TVDEC_SRC_NTSC)){
        #if defined(WIFI_PORT) && (WIFI_PORT == 1)
        ncam_set_rear_cam_ready(AHC_TRUE);
        #endif
    }
    else {
        #if defined(WIFI_PORT) && (WIFI_PORT == 1)
        ncam_set_rear_cam_ready(AHC_FALSE);
        #endif
    }

    brecording = VideoFunc_RecordStatus();
    if (brecording) {
        StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD); //stop video record.      
        if (sSnrTVSrc == MMP_SNR_TVDEC_SRC_NO_READY) {            
            MMPS_3GPRECD_EnableDualEmergentRecd(AHC_FALSE);
        }

        if (AHC_VIDEO_GetKernalEmergStopStep() != AHC_KERNAL_EMERGENCY_AHC_DONE)
        {
            AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_AHC_DONE);
            AHC_VIDEO_SetEmergPostDone(AHC_TRUE);
            AHC_VIDEO_SetEmergRecStarted(AHC_FALSE);
        }
        
        DrawStateVideoRecUpdate(EVENT_VIDEO_KEY_RECORD_STOP);
		AHC_OS_SleepMs(500);
        StateVideoRecMode_StartRecordingProc(EVENT_VIDEO_KEY_RECORD); //restart video record.        
    }

#if 0                
    if(AHC_TRUE == AHC_GetVRFileFullFlag()){
        AHC_SetVRFileFullFlag(AHC_FALSE);
        AHC_PRINT_RET_ERROR(0, 0);
        printc("Disable EVENT_VRCB_FILE_FULL due to EVENT_TV_DECODER_SRC_TYPE.\r\n");
    }                  
#endif     
       
    if (AHC_GetCurKeyEventID() == BUTTON_TV_DECODER_SRC_TYPE){
        AHC_SetCurKeyEventID(EVENT_NONE);
    }
    else {
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0);
        printc("KeyEventID: BUTTON_TV_DECODER_SRC_TYPE is interrupted.\r\n");
    }  
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_LDWS_START_LCD)
{
#if (ENABLE_ADAS_LDWS)

    LDWS_Lock();

    MMPS_AUI_StopWAVPlay();
    m_ulLDWSWarn = 20;
    VideoFunc_LDWSWarn();
#endif
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_LDWS_STOP_LCD)
{
#if (ENABLE_ADAS_LDWS)
    LDWS_Unlock();
    ResetLDWSCounter();

    MMPS_AUI_StopWAVPlay();
    DrawStateVideoRecUpdate(EVENT_LDWS_STOP);
#endif
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_FCWS_START_LCD)
{
#if (ENABLE_ADAS_FCWS)

    MMPS_AUI_StopWAVPlay();

    m_ulFCWSWarn = 20;
    VideoFunc_FCWSWarn();
#endif
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_FCWS_STOP_LCD)
{
#if (ENABLE_ADAS_FCWS)
    ResetFCWSCounter();

    //			if ((CheckAlertState() != ALERT_NON) && (CheckAlertState() != ALERT_FCWS))
    //				return;

    MMPS_AUI_StopWAVPlay();
    DrawStateVideoRecUpdate(EVENT_FCWS_STOP);
#endif
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_SAG_START_LCD)
{
#if (ENABLE_ADAS_SAG)
	if(m_ulSAGWarn > 0){
		printc("===== >>> Previous SAG warning is not over yet! <<< ===== \r\n");
		return;//break;
	}
	MMPS_AUI_StopWAVPlay();
	m_ulSAGWarn = 20;
	printc("===== >>> SAG warning begin! <<< ===== \r\n");
	AHC_PlaySoundEffect(AHC_SOUNDEFFECT_BUTTON); //TBD. This is only an example.
#endif
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_SAG_STOP_LCD)
{
#if (ENABLE_ADAS_SAG)
	ResetSAGCounter();
	MMPS_AUI_StopWAVPlay();
	printc("===== >>> SAG warning end! <<< ===== \r\n");				
#endif
}

DECL_AHC_EV_HANDLER(STATE_VIDEO_REC_MODE_EVENT_MENU_EXIT_LCD)
{
    VideoRecMode_PreviewUpdate();
}

UINT32 StateVideoRecModeHandler(UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam)//(UINT32 ulEvent)
{
    UINT32 ulVideoEvent = 0;
    ulVideoEvent  = KeyParser_VideoRecEvent(ulMsgId, ulEvent, ulParam);
#if DBG_UI_NET_EVENT
    if (ulVideoEvent != EVENT_VIDREC_UPDATE_MESSAGE) {
    	if (ulVideoEvent >= WIRELESS_MESSAGE_START) {
    		printc("ulMsgId:%X, param:%X, ", ulMsgId, ulParam);
    	}
    	printc("ulVideoEvent:x%X\r\n", ulVideoEvent);
    }
#endif
    StateModeDoHandlerEvent(ulMsgId, ulVideoEvent, ulParam);
    
    return 0;
}


void StateVideoRecTVMode(UINT32 ulJobEvent)
{
}


/*
 * lock functions
 */
/*
 * Get lock file status;
 */
AHC_BOOL VideoFunc_LockFileEnabled(void)
{
    return _bLockVRFile;
}

void EnableLockFile(AHC_BOOL bEnable, int type)
{
    _bLockVRFile = bEnable;
    _nLockType   = type;
}

/*
 * Change Lock file type, when stop recording by manual
 */
void LockFileTypeChange(int arg)
{
    if (arg == 1 /* Remove type of lock NEXT file */) {
        // When Stop recording by manual, remove lock NEXT
        if (_nLockType == LOCK_FILE_CUR_NEXT) {
            _nLockType = LOCK_FILE_CUR;
        } else if (_nLockType == LOCK_FILE_PREV_CUR_NEXT) {
            _nLockType = LOCK_FILE_PREV_CUR;
        }
    }
}

/*
 * a Hook function called by MMPF after VID file closed.
 */
void CUS_VideoProtectProcess(void)
{
    if (VideoFunc_LockFileEnabled()) {
        switch (_nLockType) {
        case LOCK_FILE_PREV_CUR:
        case LOCK_FILE_PREV_CUR_NEXT:
            printc("LOCK PREVIOUS and CURRENT...\r\n");
           	DBG_AutoTestPrint(ATEST_ACT_EMERGENCY_0x0009, ATEST_STS_END_0x0003, 0, 0, gbAhcDbgBrk);
            AHC_Protect_SetVRFile(AHC_PROTECTION_BOTH_FILE, AHC_Protect_GetType());

            if (_nLockType == LOCK_FILE_PREV_CUR_NEXT) {
                _nLockType = LOCK_FILE_CUR;
            } else {
                printc("LOCK FINISH...\r\n");
                EnableLockFile(AHC_FALSE, 0);
                AHC_Protect_SetType(AHC_PROTECT_NONE);
            }
            break;

        case LOCK_FILE_CUR:
        case LOCK_FILE_CUR_NEXT:
            printc("LOCK CURRENT...\r\n");
            AHC_Protect_SetVRFile(AHC_PROTECTION_CUR_FILE, AHC_Protect_GetType());

            if (_nLockType == LOCK_FILE_CUR_NEXT) {
                _nLockType = LOCK_FILE_CUR;
            } else {
                printc("LOCK FINISH...\r\n");
                EnableLockFile(AHC_FALSE, 0);
                AHC_Protect_SetType(AHC_PROTECT_NONE);
            }
        }
    }
}

AHC_BOOL VideoRec_TriggeredByVMD(void) {
#if (MOTION_DETECTION_EN && (VMD_ACTION & VMD_RECORD_VIDEO))
    return (0 == m_ulVMDStopCnt) ? AHC_FALSE : AHC_TRUE;
#else
    return AHC_FALSE;
#endif
}

#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE == 1)
int VideoFunc_UVCXUCmd_GetAtomValue(int CtlType, int *pCurr, int *pMin, int *pMax)
{
    AHC_BOOL bRet = AHC_TRUE;
    
#if 0 //Andy Liu TBD
    switch(CtlType) {
        case UVC_XU_RECORDTIME_CONTROL:
            bRet = GetAtomValue(COMMON_KEY_VR_CLIP_TIME, pCurr, pMin, pMax);
            break;
        case UVC_XU_RECORDRES_CONTROL:
            bRet = GetAtomValue(COMMON_KEY_MOVIE_SIZE, pCurr, pMin, pMax);
            break;
        case UVC_XU_GSENSOR_CONTROL:
            bRet = GetAtomValue(COMMON_KEY_GSENSOR_SENS, pCurr, pMin, pMax);
            break;
        case UVC_XU_AUDIO_CONTROL:
            bRet = GetAtomValue("RecordWithAudio", pCurr, pMin, pMax);
            break;
        case UVC_XU_REC_STATUS_CONTROL:
            *pCurr = VideoFunc_RecordStatus();
            *pMin  = 0;
            *pMax  = 1;
            break;
        case UVC_XU_REC_MODE_CONTROL:
            bRet = GetAtomValue("AutoRec", pCurr, pMin, pMax);
            break;
        default:
            AHC_PRINT_RET_ERROR(0, 0);
            break;
    }
#endif
    return bRet;
}

int VideoFunc_UVCXUCmd_SetAtomValue(int CtlType, int nValue)
{
    AHC_BOOL bRet = AHC_TRUE;
    
    switch(CtlType) {
        case UVC_XU_RECORDTIME_CONTROL:
            MenuSettingConfig()->uiMOVClipTime = nValue;
            break;
        case UVC_XU_RECORDRES_CONTROL:
            MenuSettingConfig()->uiMOVSize = nValue;
            break;
        case UVC_XU_GSENSOR_CONTROL:
            MenuSettingConfig()->uiGsensorSensitivity = nValue;
            break;
        case UVC_XU_AUDIO_CONTROL:
            MenuSettingConfig()->uiMOVSoundRecord = nValue;
            break;
        case UVC_XU_REC_STATUS_CONTROL:
            break;
        case UVC_XU_REC_MODE_CONTROL:
            MenuSettingConfig()->uiAutoRec = nValue;
            break;
        default:
            AHC_PRINT_RET_ERROR(0, 0);
            break;
    }

    Menu_WriteSetting();

    return bRet;
}

void VideoFunc_UVCXUCmd_CommonCtl(int CtlType, MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    AHC_BOOL bRecording = AHC_FALSE;
    int nCurr = 0,nMin = 0,nMax = 0;

    if (m_ubFormatSDing) 
        return;

    if (VideoFunc_UVCXUCmd_GetAtomValue(CtlType, &nCurr, &nMin, &nMax) == AHC_FALSE)
        return;

    bRecording = VideoFunc_RecordStatus();    

	switch(pCmd[1]) {
        case SET_CUR_CMD:
			if (CtlType == UVC_XU_REC_STATUS_CONTROL)
			{
				if (bRecording == AHC_TRUE && pCmd[2] == 0)
				{
					AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_REC_REL, 0);
				}
				else if(bRecording == AHC_FALSE && pCmd[2] == 1)
				{
					AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_REC_REL, 0);
				}
			}
			else if (CtlType == UVC_XU_PICTURE_CONTROL)
			{
				AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_REC_LPRESS, 0);
			}
			else if (CtlType == UVC_XU_MMC_CONTROL)
			{
				if (bRecording)
					break;
				
				m_ubFormatSDing = AHC_TRUE;
				QuickMediaOperation(MEDIA_CMD_FORMAT);
				m_ubFormatSDing = AHC_FALSE;
			}
			else
			{
				if (bRecording)
					break;
				
				if (pCmd[2] >= nMin && pCmd[2] <= nMax)
				{
				    VideoFunc_UVCXUCmd_SetAtomValue(CtlType, pCmd[2]);
				}
			}
            break;
        case GET_CUR_CMD:
            pResult[0] = nCurr;
            break;
        case GET_MIN_CMD:
            pResult[0] = nMin;
            break;
        case GET_MAX_CMD:
            pResult[0] = nMax;
            break;
        case GET_RES_CMD:
            pResult[0] = 1;
            break;
        case GET_LEN_CMD:
            break;
        case GET_INFO_CMD:
            break;
        case GET_DEF_CMD:
            pResult[0] = nMin;
            if (CtlType == UVC_XU_AUDIO_CONTROL)
            {
                pResult[0] = nMax;
            }
            break;
        default:
            AHC_PRINT_RET_ERROR(0, 0);
            break;
    }
}

// Depends on customer.
void VideoFunc_UVCXUCmd_RecordTimeCtl(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    VideoFunc_UVCXUCmd_CommonCtl(UVC_XU_RECORDTIME_CONTROL, pCmd, pResult, ulCmdLen, ulResultLen);  
}

void VideoFunc_UVCXUCmd_RecordResCtl(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    VideoFunc_UVCXUCmd_CommonCtl(UVC_XU_RECORDRES_CONTROL, pCmd, pResult, ulCmdLen, ulResultLen);      
}

void VideoFunc_UVCXUCmd_FileCtl(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    // TBD
}

void VideoFunc_UVCXUCmd_PictureCtl(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    VideoFunc_UVCXUCmd_CommonCtl(UVC_XU_PICTURE_CONTROL, pCmd, pResult, ulCmdLen, ulResultLen);
}

void VideoFunc_UVCXUCmd_GSensorCtl(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    VideoFunc_UVCXUCmd_CommonCtl(UVC_XU_GSENSOR_CONTROL, pCmd, pResult, ulCmdLen, ulResultLen); 
}

void VideoFunc_UVCXUCmd_AudioCtl(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    VideoFunc_UVCXUCmd_CommonCtl(UVC_XU_AUDIO_CONTROL, pCmd, pResult, ulCmdLen, ulResultLen);   
}

void VideoFunc_UVCXUCmd_RecordStatusCtl(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    VideoFunc_UVCXUCmd_CommonCtl(UVC_XU_REC_STATUS_CONTROL, pCmd, pResult, ulCmdLen, ulResultLen);      
}

void VideoFunc_UVCXUCmd_RecordModeCtl(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    VideoFunc_UVCXUCmd_CommonCtl(UVC_XU_REC_MODE_CONTROL, pCmd, pResult, ulCmdLen, ulResultLen);  
}

void VideoFunc_UVCXUCmd_FirmwareCtl(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    // TBD
}

void VideoFunc_UVCXUCmd_MMCCtl(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    VideoFunc_UVCXUCmd_CommonCtl(UVC_XU_MMC_CONTROL, pCmd, pResult, ulCmdLen, ulResultLen);
}

void VideoFunc_UVCXUCmd_SwitchMSDCMode(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    if (VideoFunc_RecordStatus() == AHC_FALSE)
    {
        AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_VIDEO_UVC_XU_SWITCH_MSDC_MODE, 0);
    }
}

void VideoFunc_UVCXUCmd_MMP16CmdHandler(MMP_UBYTE *pCmd, MMP_UBYTE *pResult, MMP_ULONG ulCmdLen, MMP_ULONG ulResultLen)
{
    printc("%s,%d cmd:0x%x\r\n", __func__, __LINE__, pCmd[0]);

    switch(pCmd[0]) {
        case UVC_XU_RECORDTIME_CONTROL:
            VideoFunc_UVCXUCmd_RecordTimeCtl(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        case UVC_XU_RECORDRES_CONTROL:
            VideoFunc_UVCXUCmd_RecordResCtl(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        case UVC_XU_FILE_CONTROL:
            VideoFunc_UVCXUCmd_FileCtl(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        case UVC_XU_PICTURE_CONTROL:
            VideoFunc_UVCXUCmd_PictureCtl(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        case UVC_XU_GSENSOR_CONTROL:
            VideoFunc_UVCXUCmd_GSensorCtl(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        case UVC_XU_AUDIO_CONTROL:
            VideoFunc_UVCXUCmd_AudioCtl(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        case UVC_XU_REC_STATUS_CONTROL:
            VideoFunc_UVCXUCmd_RecordStatusCtl(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        case UVC_XU_REC_MODE_CONTROL:
            VideoFunc_UVCXUCmd_RecordModeCtl(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        case UVC_XU_FIRMWARE_CONTROL:
            VideoFunc_UVCXUCmd_FirmwareCtl(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        case UVC_XU_MMC_CONTROL:
            VideoFunc_UVCXUCmd_MMCCtl(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        case UVC_XU_SWITCH_MSDC_MODE:
            VideoFunc_UVCXUCmd_SwitchMSDCMode(pCmd, pResult, ulCmdLen, ulResultLen);
            break;
        default:
            AHC_PRINT_RET_ERROR(0, pCmd[0]);
            break;
    }
}

AHC_BOOL VideoFunc_UVCXUCmdRegisterHandler(void) 
{
    usb_vc_eu1_mmp_cmd16_register_handler((void *)VideoFunc_UVCXUCmd_MMP16CmdHandler);
    return AHC_TRUE;
}
#endif

// for ADAS_LDWS, TBD
AHC_BOOL StateVideoRecModeInitLCD(void* pData)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    ahcRet = AHC_SwitchDisplayOutputDevLCD();

#if (ENABLE_ADAS_LDWS)
    // Put here temporary. Matt.
    //LDWS_RegistCallback(LDWS_Callback);
    MMPS_ADAS_RegisterCallBack(LDWS_Callback);
#endif

    MMPC_HDMI_PC_Enable(MMP_TRUE);
    StateLedControl(UI_VIDEO_STATE);
    StateLCDCheckStatus(UI_VIDEO_STATE);

#if (SWITCH_MODE_FREEZE_WIN)
    if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) || 
        (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
        MMPS_Display_FreezeWinUpdate(AHC_TRUE, AHC_FALSE, AHC_TRUE);// Browser to Video preview
    }
    else if (CAM_CHECK_USB(USB_CAM_AIT) || 
             CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264)) {
        MMPS_Display_FreezeWinUpdate(AHC_FALSE, AHC_FALSE, AHC_FALSE);// Browser to Video preview
    }
#endif

    RTNA_LCD_Backlight(MMP_TRUE);
    SetCurrentOpMode(VIDEOREC_MODE);
    //uiSysState.CurrentState = UI_VIDEO_STATE;   //When Hdmi remove ,Video ICON no display, need check
    ahcRet = VideoRecMode_Start();

    return ahcRet;
}

#if (TVOUT_ENABLE)
AHC_BOOL StateVideoRecModeInitTV(void* pData)
{
    AHC_BOOL ahcRet = AHC_TRUE;

#if (1)
	printc("%s,%d \n", __func__, __LINE__);
#endif
	
	RTNA_LCD_Backlight(MMP_FALSE);
#ifdef CFG_ENBLE_PANEL_SLEEP
	RTNA_LCD_Enter_Sleep();
#endif

    ahcRet = AHC_SwitchDisplayOutputDevTVout();

#if (ENABLE_ADAS_LDWS)
	// Put here temporary. Matt.
	//LDWS_RegistCallback(LDWS_Callback);
	MMPS_ADAS_RegisterCallBack(LDWS_Callback);
#endif
	
	MMPC_HDMI_PC_Enable(MMP_TRUE);
	//StateLedControl(UI_VIDEO_STATE);
	//StateLCDCheckStatus(UI_VIDEO_STATE);
	
	//AHC_SetMode(AHC_MODE_IDLE);
		
	SetCurrentOpMode(VIDEOREC_MODE);
	//uiSysState.CurrentState = UI_VIDEO_STATE;   //When Hdmi remove ,Video ICON no display, need check
	ahcRet = VideoRecMode_Start();
	
	return ahcRet;
}
#endif

#if (HDMI_PREVIEW_EN)
AHC_BOOL StateVideoRecModeInitHDMI(void* pData)
{
    AHC_BOOL ahcRet = AHC_TRUE;

#if (1)
    printc("%s,%d \n", __func__, __LINE__);
#endif
    
    RTNA_LCD_Backlight(MMP_FALSE);
#ifdef CFG_ENBLE_PANEL_SLEEP
    RTNA_LCD_Enter_Sleep();
#endif

    //AHC_SetMode(AHC_MODE_IDLE);

    ahcRet = AHC_SwitchDisplayOutputDevHDMI();
        
#if (ENABLE_ADAS_LDWS)
    // Put here temporary. Matt.
    //LDWS_RegistCallback(LDWS_Callback);
    ADAS_CTL_RegistCallback(LDWS_Callback);
#endif

    MMPC_HDMI_PC_Enable(MMP_TRUE);
    StateLedControl(UI_VIDEO_STATE);
    //StateLCDCheckStatus(UI_VIDEO_STATE);
    
    SetCurrentOpMode(VIDEOREC_MODE);
    //uiSysState.CurrentState = UI_VIDEO_STATE;   //When Hdmi remove ,Video ICON no display, need check
    ahcRet = VideoRecMode_Start();    
    //return StateVideoRecModeInitLCD();
    
    return ahcRet;
}
#endif

AHC_BOOL VideoFunc_Init_NoDisplay(void* pData)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    return ahcRet;
}

AHC_BOOL StateVideoRecModeShutDown(void* pData)
{
    AHC_BOOL ahcRet = AHC_TRUE;
#if 1//(defined(STATE_VIDEO_DEBUG) && STATE_VIDEO_DEBUG == 1)
    printc("%s,%d \n", __func__, __LINE__);
#endif

    if( uiGetParkingModeEnable() == AHC_TRUE ){
        AHC_VIDEO_ParkingModeStop();    
    }

    if(VideoFunc_RecordStatus())
        StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);

#if (VR_PREENCODE_EN)
    bDisableVideoPreRecord = AHC_TRUE;
#endif

	g_bDrawUnfix	= MMP_FALSE;

    VideoTimer_Stop();

    if (AHC_WMSG_States()) {
        AHC_WMSG_Draw(AHC_FALSE, WMSG_NONE, 0);
    }

#if (MOTION_DETECTION_EN)
#if (defined(PROJECT_ID) && (PROJECT_ID == PROJECT_ID_SP86G || PROJECT_ID == PROJECT_ID_SP20G))
    if (m_ubMotionDtcEn)	
    {
        VideoFunc_ExitVMDMode();
    }
#endif
#endif

    //AHC_OSDUninit(); //TBD

    AHC_SetMode(AHC_MODE_IDLE);
#if (UVC_HOST_VIDEO_ENABLE)    
    ahcRet = AHC_UVCSetIdleStates();
#endif

    return AHC_TRUE;
}

AHC_BOOL StateSelectFuncVideoRecordMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;

    uiSaveCurrentState();

#if 0//#ifndef CFG_MENU_CAMERA_ALWAYS_ENTER_CAPTURE_MODE //may be defined in config_xxx.h
    if(AHC_TRUE == StateIsInMenuMode()){
        StateModeSetOperation(UI_VIDEO_STATE, (void*)StateVideoRecModeInitFromMenu, (void*)StateVideoRecModeShutDown, (void *)StateVideoRecModeHandler);
    }
    else
#endif
    {
        // Common function.
        #if (VIDEO_DIGIT_ZOOM_EN)
        RegisterEventCb(EVENT_KEY_TELE_PRESS,			STATE_VIDEO_REC_MODE_EVENT_KEY_TELE_PRESS_LCD);
        RegisterEventCb(EVENT_KEY_WIDE_PRESS,			STATE_VIDEO_REC_MODE_EVENT_KEY_WIDE_PRESS_LCD);
        RegisterEventCb(EVENT_KEY_WIDE_STOP,			STATE_VIDEO_REC_MODE_EVENT_KEY_WIDE_STOP_LCD);
        #endif
	//RegisterEventCb(EVENT_SWITCH_WIFI_STREAMING_MODE,     STATE_VIDEO_REC_MODE_EVENT_SWITCH_WIFI_STREAMING_MODE_LCD);
        RegisterEventCb(EVENT_EV_INCREASE,				STATE_VIDEO_REC_MODE_EVENT_EV_INCREASE_LCD);
        RegisterEventCb(EVENT_EV_DECREASE,				STATE_VIDEO_REC_MODE_EVENT_EV_DECREASE_LCD);
        RegisterEventCb(EVENT_MENU_EXIT,			    STATE_VIDEO_REC_MODE_EVENT_MENU_EXIT_LCD);
        RegisterEventCb(EVENT_KEY_UP,					STATE_VIDEO_REC_MODE_EVENT_KEY_UP_LCD);//for debug speech recog~
		RegisterEventCb(EVENT_KEY_LEFT,					STATE_VIDEO_REC_MODE_EVENT_KEY_LEFT_LCD);
        RegisterEventCb(EVENT_KEY_RIGHT,				STATE_VIDEO_REC_MODE_EVENT_KEY_RIGHT_LCD);
        RegisterEventCb(EVENT_KEY_SET,					STATE_VIDEO_REC_MODE_EVENT_KEY_SET_LCD);
        RegisterEventCb(EVENT_KEY_MENU,					STATE_VIDEO_REC_MODE_EVENT_KEY_MENU_LCD);
        RegisterEventCb(EVENT_KEY_MODE,					STATE_VIDEO_REC_MODE_EVENT_KEY_MODE_LCD);
        RegisterEventCb(EVENT_VIDEO_KEY_RECORD,			STATE_VIDEO_REC_MODE_EVENT_VIDEO_KEY_RECORD_LCD);
        RegisterEventCb(EVENT_VIDEO_KEY_SWAP_PIP,		STATE_VIDEO_REC_MODE_EVENT_VIDEO_KEY_SWAP_PIP_LCD);
        RegisterEventCb(EVENT_RECORD_MUTE,				STATE_VIDEO_REC_MODE_EVENT_RECORD_MUTE_LCD);
        RegisterEventCb(EVENT_KEY_PLAYBACK_MODE,		STATE_VIDEO_REC_MODE_EVENT_KEY_PLAYBACK_MODE_LCD);
        RegisterEventCb(EVENT_POWER_OFF,				STATE_VIDEO_REC_MODE_EVENT_POWER_OFF_LCD);
        RegisterEventCb(EVENT_CHANGE_LED_MODE, 			STATE_VIDEO_REC_MODE_EVENT_CHANGE_LED_MODE_LCD);
        RegisterEventCb(EVENT_PARKING_KEY, 				STATE_VIDEO_REC_MODE_EVENT_PARKING_KEY_LCD);
#if defined(WIFI_PORT) && (WIFI_PORT == 1)
        RegisterEventCb(EVENT_NET_ENTER_PLAYBACK, 		STATE_VIDEO_REC_MODE_EVENT_ENTER_NET_PLAYBACK_LCD);
        #if 1
        RegisterEventCb(EVENT_VR_EMERGENT, 		        STATE_VIDEO_REC_MODE_EVENT_VR_EMERGENT_LCD);
        RegisterEventCb(EVENT_LOCK_FILE_G, 		        STATE_VIDEO_REC_MODE_EVENT_VR_EMERGENT_LCD);
        RegisterEventCb(EVENT_VRCB_EMER_DONE, 		    STATE_VIDEO_REC_MODE_EVENT_VRCB_EMER_DONE_LCD);
        #else
        RegisterEventCb(EVENT_VR_EMERGENT, 		        STATE_VIDEO_REC_MODE_EVENT_CGI_DUMMY);
        RegisterEventCb(EVENT_LOCK_FILE_G, 		        STATE_VIDEO_REC_MODE_EVENT_CGI_DUMMY);
        #endif
		#if (AHC_SHAREENC_SUPPORT)
        RegisterEventCb(EVENT_VR_SHORT, 			    STATE_VIDEO_REC_MODE_EVENT_REC_SHORT);
        #endif
        RegisterEventCb(EVENT_NET_DOWNLOAD, 	     	STATE_VIDEO_REC_MODE_EVENT_DOWNLOAD);
#else
        #if (EMERGENTRECD_SUPPORT)
        RegisterEventCb(EVENT_VR_EMERGENT, 		        STATE_VIDEO_REC_MODE_EVENT_VR_EMERGENT_LCD);
        RegisterEventCb(EVENT_LOCK_FILE_G, 		        STATE_VIDEO_REC_MODE_EVENT_VR_EMERGENT_LCD);
        RegisterEventCb(EVENT_VRCB_EMER_DONE, 		    STATE_VIDEO_REC_MODE_EVENT_VRCB_EMER_DONE_LCD);    
        #endif
#endif
#ifdef LED_GPIO_LASER
        RegisterEventCb(EVENT_LASER_LED_ONOFF,			STATE_VIDEO_REC_MODE_EVENT_LASER_LED_ONOFF_LCD);
#endif
        RegisterEventCb(EVENT_FORMAT_MEDIA,				STATE_VIDEO_REC_MODE_EVENT_FORMAT_MEDIA_LCD);
        RegisterEventCb(EVENT_VIDEO_KEY_CAPTURE,		STATE_VIDEO_REC_MODE_EVENT_VIDEO_KEY_CAPTURE_LCD);
        RegisterEventCb(EVENT_CAMERA_PREVIEW,			STATE_VIDEO_REC_MODE_EVENT_CAMERA_PREVIEW_LCD);

        RegisterEventCb(EVENT_USB_DETECT,				STATE_VIDEO_REC_MODE_EVENT_USB_DETECT_LCD);
        RegisterEventCb(EVENT_USB_REMOVED, 				STATE_VIDEO_REC_MODE_EVENT_USB_REMOVED_LCD);
        RegisterEventCb(EVENT_SD_DETECT, 				STATE_VIDEO_REC_MODE_EVENT_SD_DETECT_LCD);
        RegisterEventCb(EVENT_SD_REMOVED,               STATE_VIDEO_REC_MODE_EVENT_SD_REMOVED_LCD);
#if (TVOUT_ENABLE)
        RegisterEventCb(EVENT_TV_DETECT, 				STATE_VIDEO_REC_MODE_EVENT_TV_DETECT_LCD);
        RegisterEventCb(EVENT_TV_REMOVED,				STATE_VIDEO_REC_MODE_EVENT_TV_REMOVED_LCD);
#endif
#if (HDMI_ENABLE)
        RegisterEventCb(EVENT_HDMI_DETECT,				STATE_VIDEO_REC_MODE_EVENT_HDMI_DETECT_LCD);
        RegisterEventCb(EVENT_HDMI_REMOVED,				STATE_VIDEO_REC_MODE_EVENT_HDMI_REMOVED_LCD);
#endif
        RegisterEventCb(EVENT_VRCB_MEDIA_FULL,			STATE_VIDEO_REC_MODE_EVENT_VRCB_MEDIA_FULL_LCD);
        RegisterEventCb(EVENT_VRCB_FILE_FULL, 			STATE_VIDEO_REC_MODE_EVENT_VRCB_FILE_FULL_LCD);
        RegisterEventCb(EVENT_VRCB_LONG_TIME_FILE_FULL, 			STATE_VIDEO_REC_MODE_EVENT_VRCB_LONG_TIME_FILE_FULL_LCD);
        RegisterEventCb(EVENT_VRCB_MEDIA_SLOW,			STATE_VIDEO_REC_MODE_EVENT_VRCB_MEDIA_SLOW_LCD);
        RegisterEventCb(EVENT_VRCB_RECDSTOP_CARDSLOW,          STATE_VIDEO_REC_MODE_EVENT_VRCB_RECDSTOP_CARDSLOW_LCD);
        RegisterEventCb(EVENT_VRCB_SEAM_LESS,			STATE_VIDEO_REC_MODE_EVENT_VRCB_SEAM_LESS_LCD);
        RegisterEventCb(EVENT_VRCB_MEDIA_ERROR,			STATE_VIDEO_REC_MODE_EVENT_VRCB_MEDIA_ERROR_LCD);
        RegisterEventCb(EVENT_VRCB_MOTION_DTC,			STATE_VIDEO_REC_MODE_EVENT_VRCB_MOTION_DTC_LCD);
        RegisterEventCb(EVENT_VR_START, 				STATE_VIDEO_REC_MODE_EVENT_VR_START_LCD);
        RegisterEventCb(EVENT_VR_STOP,					STATE_VIDEO_REC_MODE_EVENT_VR_STOP_LCD);
        RegisterEventCb(EVENT_VR_WRITEINFO,				STATE_VIDEO_REC_MODE_EVENT_VR_WRITEINFO_LCD);
        RegisterEventCb(EVENT_LOCK_VR_FILE,				STATE_VIDEO_REC_MODE_EVENT_LOCK_VR_FILE_LCD);
        RegisterEventCb(EVENT_VIDREC_UPDATE_MESSAGE, 	STATE_VIDEO_REC_MODE_EVENT_VIDREC_UPDATE_MESSAGE_LCD);
        RegisterEventCb(EVENT_GPSGSENSOR_UPDATE,		STATE_VIDEO_REC_MODE_EVENT_GPSGSENSOR_UPDATE_LCD);
#if (UVC_HOST_VIDEO_ENABLE)
        RegisterEventCb(EVENT_USB_B_DEVICE_DETECT,		STATE_VIDEO_REC_MODE_EVENT_USB_B_DEVICE_DETECT_LCD);
        RegisterEventCb(EVENT_USB_B_DEVICE_REMOVED,		STATE_VIDEO_REC_MODE_EVENT_USB_B_DEVICE_DETECT_LCD);
#endif

        RegisterEventCb(EVENT_LDWS_START,				STATE_VIDEO_REC_MODE_EVENT_LDWS_START_LCD);
        RegisterEventCb(EVENT_LDWS_STOP,				STATE_VIDEO_REC_MODE_EVENT_LDWS_STOP_LCD);
        RegisterEventCb(EVENT_FCWS_START,				STATE_VIDEO_REC_MODE_EVENT_FCWS_START_LCD);
        RegisterEventCb(EVENT_FCWS_STOP,				STATE_VIDEO_REC_MODE_EVENT_FCWS_STOP_LCD);
        RegisterEventCb(EVENT_SAG_START,				STATE_VIDEO_REC_MODE_EVENT_SAG_START_LCD);
        RegisterEventCb(EVENT_SAG_STOP,					STATE_VIDEO_REC_MODE_EVENT_SAG_STOP_LCD);

#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE)
        RegisterEventCb(EVENT_VIDEO_UVC_XU_RECORDTIME_CONTROL,  STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_RECORDTIME_CONTROL);
        RegisterEventCb(EVENT_VIDEO_UVC_XU_RECORDRES_CONTROL,   STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_RECORDRES_CONTROL);
        RegisterEventCb(EVENT_VIDEO_UVC_XU_FILE_CONTROL,        STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_FILE_CONTROL);
        RegisterEventCb(EVENT_VIDEO_UVC_XU_PICTURE_CONTROL,     STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_PICTURE_CONTROL);
        RegisterEventCb(EVENT_VIDEO_UVC_XU_GSENSOR_CONTROL,     STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_GSENSOR_CONTROL);
        RegisterEventCb(EVENT_VIDEO_UVC_XU_AUDIO_CONTROL,       STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_AUDIO_CONTROL);
        RegisterEventCb(EVENT_VIDEO_UVC_XU_REC_MODE_CONTROL,    STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_REC_MODE_CONTROL);
        RegisterEventCb(EVENT_VIDEO_UVC_XU_STATUS_CONTROL,      STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_STATUS_CONTROL);
        RegisterEventCb(EVENT_VIDEO_UVC_XU_FIRMWARE_CONTROL,    STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_FIRMWARE_CONTROL);
        RegisterEventCb(EVENT_VIDEO_UVC_XU_MMC_CONTROL,         STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_MMC_CONTROL);
        RegisterEventCb(EVENT_VIDEO_UVC_XU_SWITCH_MSDC_MODE,    STATE_VIDEO_REC_MODE_EVENT_VIDEO_UVC_XU_SWITCH_MSDC_MODE);
#endif

        RegisterEventCb(EVNET_SUB_MODE_ENTER,			STATE_VIDEO_REC_MODE_EVNET_SUB_MODE_ENTER_LCD);
        RegisterEventCb(EVNET_SUB_MODE_EXIT,			STATE_VIDEO_REC_MODE_EVNET_SUB_MODE_EXIT_LCD);

        if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) {
            RegisterEventCb(EVENT_TV_DECODER_SRC_TYPE,	        STATE_VIDEO_REC_MODE_EVENT_TV_DECODER_SRC_TYPE_LCD);
        }

        #if (HDMI_ENABLE)
		MMPF_OS_Sleep_MS(5);
        if (AHC_IsHdmiConnect()){//LCD
            //Register HDMI specific functions here!
            StateModeSetOperation(UI_VIDEO_STATE, StateVideoRecModeInitHDMI, StateVideoRecModeShutDown, StateVideoRecModeHandler);
        }
        else
        #endif
        #if (TVOUT_ENABLE)
        if (AHC_IsTVConnectEx()){
            StateModeSetOperation(UI_VIDEO_STATE, StateVideoRecModeInitTV, StateVideoRecModeShutDown, StateVideoRecModeHandler);
        }else
        #endif
        {
            StateModeSetOperation(UI_VIDEO_STATE, StateVideoRecModeInitLCD, StateVideoRecModeShutDown, StateVideoRecModeHandler);
        }
    }

    ahcRet =  SwitchUIDrawSetVideoRecordMode();
    if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return ahcRet;}

    return ahcRet;
}


//For parking mode.
#if (ENABLE_ADAS_LDWS || ENABLE_ADAS_FCWS || ENABLE_ADAS_SAG)
//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeInit
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeInitADASMode(void)
{
    extern MMP_UBYTE gbADASSrcYUV;
    UINT8 byType = MMPS_3GPRECD_Y_FRAME_TYPE_NONE;
    UINT32 bLDWS_En = COMMON_LDWS_EN_OFF; 
    UINT32 bFCWS_En = COMMON_FCWS_EN_OFF;
    UINT32 bSAG_En  = COMMON_SAG_EN_OFF;
    UINT32 uiAdasFlag = 0;

    AHC_BOOL ahcRet = AHC_TRUE;

#if (CHANGE_ADAS_SRC_FROM_Y_ONLY_TO_YUV)
    gbADASSrcYUV = 1;
#endif

    AHC_GetParam(PARAM_ID_ADAS, &uiAdasFlag);
        
#if (ENABLE_ADAS_LDWS)
    #ifdef CFG_ADAS_MENU_SETTING_OLD_STYLE
    if (MenuSettingConfig()->uiLDWS == COMMON_LDWS_EN_ON)
    #else
    if (uiAdasFlag & AHC_ADAS_ENABLED_LDWS)
    #endif
    {
        bLDWS_En = COMMON_LDWS_EN_ON;
        MMPS_Sensor_SetADASFeatureEn(PRM_SENSOR, MMPS_ADAS_LDWS, MMP_TRUE);
    }
    else
    {
        MMPS_Sensor_SetADASFeatureEn(PRM_SENSOR, MMPS_ADAS_LDWS, MMP_FALSE);
    }
#endif

#if (ENABLE_ADAS_FCWS)
    #ifdef CFG_ADAS_MENU_SETTING_OLD_STYLE
    if (MenuSettingConfig()->uiFCWS == COMMON_FCWS_EN_ON)
    #else
    if (uiAdasFlag & AHC_ADAS_ENABLED_FCWS)
    #endif
    {
        bFCWS_En = COMMON_FCWS_EN_ON;
        MMPS_Sensor_SetADASFeatureEn(PRM_SENSOR, MMPS_ADAS_FCWS, MMP_TRUE);
    }
    else
    {
        MMPS_Sensor_SetADASFeatureEn(PRM_SENSOR, MMPS_ADAS_FCWS, MMP_FALSE);
    }
#endif

#if (ENABLE_ADAS_SAG)
    if (uiAdasFlag & AHC_ADAS_ENABLED_SAG)
    {
        bSAG_En = COMMON_SAG_EN_ON;
        MMPS_Sensor_SetADASFeatureEn(PRM_SENSOR, MMPS_ADAS_SAG, MMP_TRUE);
    }
    else
    {
        MMPS_Sensor_SetADASFeatureEn(PRM_SENSOR, MMPS_ADAS_SAG, MMP_FALSE);
    }
#endif
    
    if (bLDWS_En == COMMON_LDWS_EN_ON || 
        bFCWS_En == COMMON_FCWS_EN_ON || 
        bSAG_En  == COMMON_SAG_EN_ON)
    {
        byType = MMPS_3GPRECD_Y_FRAME_TYPE_ADAS;
    }

    // It must be set before enter Preview mode
    MMPS_3GPRECD_SetYFrameType(byType);

    ahcRet = AHC_VIDEO_SetRecordModeInitCommon();
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, 0); return ahcRet;}

#if (ENABLE_ADAS_LDWS || ENABLE_ADAS_FCWS || ENABLE_ADAS_SAG)
        
    #if (MOTION_DETECTION_EN)
    if (MMPS_Sensor_IsVMDStarted(PRM_SENSOR)== MMP_FALSE)
    #else
    if (1)
    #endif
    {
        UINT32 uiAdasFlag = 0;
        
        AHC_GetParam(PARAM_ID_ADAS, &uiAdasFlag);

        #ifdef CFG_ADAS_MENU_SETTING_OLD_STYLE
        if ((MenuSettingConfig()->uiLDWS == COMMON_LDWS_EN_ON) ||
            (MenuSettingConfig()->uiFCWS == COMMON_FCWS_EN_ON) ||
            (uiAdasFlag & AHC_ADAS_ENABLED_SAG))
        #else
        if ((uiAdasFlag & AHC_ADAS_ENABLED_LDWS) || 
            (uiAdasFlag & AHC_ADAS_ENABLED_FCWS) || 
            (uiAdasFlag & AHC_ADAS_ENABLED_SAG ) )
        #endif
		{
            AHC_StartADAS(AHC_TRUE);
		}
		else {
            AHC_StartADAS(AHC_FALSE);
		}
	}
#endif
        
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeUnInitADASMode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeUnInitADASMode(void)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    MMP_ERR sRet = MMP_ERR_NONE;
      
    sRet = MMPS_Sensor_SetADASFeatureEn(PRM_SENSOR, MMPS_ADAS_LDWS, MMP_FALSE);
    sRet = MMPS_Sensor_SetADASFeatureEn(PRM_SENSOR, MMPS_ADAS_FCWS, MMP_FALSE);
    sRet = MMPS_Sensor_SetADASFeatureEn(PRM_SENSOR, MMPS_ADAS_SAG, MMP_FALSE);

    ahcRet = AHC_StartADAS(AHC_FALSE);
        
    return ahcRet;
}
#endif

AHC_BOOL AHC_VIDEO_ParkingModeStart(void)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    UINT8	 Motion_Detection_Stable_Time = MOTION_DETECTION_STABLE_TIME_PARKING_NONE;
    
    Motion_Detection_Stable_Time = (uiGetParkingCfg()->bParkingModeFuncEn)? MOTION_DETECTION_STABLE_TIME_PARKING : MOTION_DETECTION_STABLE_TIME_PARKING_NONE;

    if((AHC_MODE_RECORD_PRERECD == AHC_GetAhcSysMode()) ||(AHC_MODE_VIDEO_RECORD == AHC_GetAhcSysMode())){
        StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD); //VideoFunc_RecordStop(AHC_TRUE);
    }   
    
    AHC_SetMode( AHC_MODE_IDLE );

    uiSetParkingModeEnable( AHC_TRUE );

    #if (MOTION_DETECTION_EN)
    //if( MenuSettingConfig()->uiMotionDtcSensitivity == MOTION_DTC_SENSITIVITY_OFF )
    //{
    //    MenuSettingConfig()->uiMotionDtcSensitivity = MOTION_DTC_SENSITIVITY_LOW;
    //}
    Menu_SetMotionDtcSensitivity( MenuSettingConfig()->uiMotionDtcSensitivity );

    m_ulVMDStableCnt = Motion_Detection_Stable_Time * 1000 / VIDEO_TIMER_UNIT;
    m_ubVMDStart     = AHC_FALSE;
    #endif
           
    ahcRet = VIDEO_REC_MODE_EVENT_PARKING_REGCB(AHC_TRUE);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, 0); return ahcRet;}
        
    return ahcRet;
}

AHC_BOOL AHC_VIDEO_ParkingModeStop(void)
{
    //VIDEO_RECORD_STATUS sVidRecStatus;

    AHC_BOOL ahcRet = AHC_TRUE;

    if((AHC_MODE_RECORD_PRERECD == AHC_GetAhcSysMode()) ||(AHC_MODE_VIDEO_RECORD == AHC_GetAhcSysMode())){
        StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD); //VideoFunc_RecordStop(AHC_TRUE);
    }
    
    AHC_SetMode( AHC_MODE_IDLE );

    
    uiSetParkingModeEnable( AHC_FALSE );

    #if (MOTION_DETECTION_EN)
    Menu_SetMotionDtcSensitivity(MOTION_DTC_SENSITIVITY_OFF/* MenuSettingConfig()->uiMotionDtcSensitivity */);				
    m_ubVMDStart     = AHC_FALSE;
    m_ulVMDStopCnt   = 0;
    m_ulVMDStableCnt = 0;
    #endif
    
    ahcRet = VIDEO_REC_MODE_EVENT_PARKING_REGCB(AHC_FALSE);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, 0); return ahcRet;}


    return ahcRet;
}


//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeUnInitADASMode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeInitParkingMode(void)
{
    UINT8       byType = MMPS_3GPRECD_Y_FRAME_TYPE_NONE;
    #if (MOTION_DETECTION_EN)
    UINT8       ubMvTh, ubCntTh;
    MMP_ERR     sRet = MMP_ERR_NONE;
    #endif
    AHC_BOOL    ahcRet = AHC_TRUE;

    // Enter parking mode.
    byType = MMPS_3GPRECD_Y_FRAME_TYPE_VMD;        

    // It must be set before enter Preview mode
    MMPS_3GPRECD_SetYFrameType(byType);

    //Exec common init.
    ahcRet = AHC_VIDEO_SetRecordModeInitCommon();
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, 0); return ahcRet;}

    if (MenuSettingConfig()->uiMotionDtcSensitivity == MOTION_DTC_SENSITIVITY_OFF)
    {
        MenuSettingConfig()->uiMotionDtcSensitivity = MOTION_DTC_SENSITIVITY_LOW;
        Menu_SetMotionDtcSensitivity(MenuSettingConfig()->uiMotionDtcSensitivity);
    }
    
    #if (MOTION_DETECTION_EN)
    AHC_GetMotionDtcSensitivity(&ubMvTh, &ubCntTh);
    AHC_SetMotionDtcSensitivity(ubMvTh, ubCntTh);
    
	#if (FRONT_MOTION_DETECTION_EN)
    sRet = MMPS_Sensor_RegisterCallback(PRM_SENSOR, MMPS_IVA_EVENT_MDTC, (void *)VRMotionDetectCB);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk, sRet); return AHC_FALSE;}
	#endif
	#if (REAR_MOTION_DETECTION_EN)
    sRet = MMPS_Sensor_RegisterCallback(SCD_SENSOR, MMPS_IVA_EVENT_MDTC, (void *)VRMotionDetectCB);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk, sRet); return AHC_FALSE;}
	#endif
	
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
    
    AHC_StartMotionDetection(AHC_TRUE);
    AHC_SetMotionDetectionStatus(AHC_TRUE);
    #endif
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeUnInitParkingMode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeUnInitParkingMode(void)
{    
    AHC_BOOL ahcRet = AHC_TRUE;
    
    #if (MOTION_DETECTION_EN)
    AHC_StartMotionDetection(AHC_FALSE);
    AHC_SetMotionDetectionStatus(AHC_FALSE);
    #endif

    AHC_VIDEO_SetMovieConfig(0, AHC_AUD_PRERECORD_DAC, AHC_TRUE);
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetEmergencyParkingMode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetEmergencyParkingMode(void)
{
    AHC_BOOL ahcRet = AHC_TRUE;   
    MMP_ERR sRet = MMP_ERR_NONE;

    #if ((UVC_HOST_VIDEO_ENABLE) && (UVC_EMERGRECD_SUPPORT) && (AHC_UVC_EMERGRECD_SUPPORT))
    sRet = MMPS_3GPRECD_EnableUVCEmergentRecd(MMP_FALSE);
    #endif
    #if (DUAL_EMERGRECD_SUPPORT) && (AHC_DUAL_EMERGRECD_SUPPORT)
    sRet = MMPS_3GPRECD_EnableDualEmergentRecd( MMP_FALSE );
    #endif
    #if (AHC_EMERGENTRECD_SUPPORT)
    sRet = MMPS_3GPRECD_EnableEmergentRecd(MMP_FALSE);
    #endif
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetAudioEncode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetAudioEncodeParkingMode(void)
{
    UINT8       ubSoundEffectStatus = 0;
    AHC_BOOL    ahcRet = AHC_TRUE;
    	
    AHC_GetSoundEffectStatus(&ubSoundEffectStatus);

    if (ubSoundEffectStatus == AHC_SOUND_EFFECT_STATUS_START)
        MMPS_AUI_StopWAVPlay();

    AHC_VIDEO_SetMovieConfig(0, AHC_AUD_PRERECORD_DAC, AHC_FALSE);
    
    ahcRet = AHC_VIDEO_SetRecordModeSetAudioEncode();
    if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk, ahcRet); return ahcRet;} 
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetTimeLimit
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetTimeLimitParkingMode(void)
{
    UINT32                      TimeLimit = 0;

    AHC_BOOL ahcRet = AHC_TRUE;

    TimeLimit = AHC_VIDEO_GetRecTimeLimitParkingMode();
    AHC_VIDEO_SetRecTimeLimit(TimeLimit);
    printc("%s,%d,TimeLimit:%d\r\n",__func__,__LINE__,TimeLimit);
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetBitRate
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetBitRateParkingMode(void)
{
    MMP_ULONG                   ulBitRate = 0;

    AHC_BOOL ahcRet = AHC_TRUE;
    MMP_ERR sRet = MMP_ERR_NONE;
    
    sRet = MMPS_3GPRECD_SetBitrate(MMPS_3GPRECD_FILESTREAM_NORMAL, AHC_VIDEO_MAXBITRATE_PRERECORD);    
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    sRet = MMPS_3GPRECD_GetParameter(MMPS_3GPRECD_PARAMETER_BITRATE, &ulBitRate);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    printc("VR:...ulBitRateParkingMode=%d \r\n", ulBitRate);
    
    return ahcRet;
}

#if (MOTION_DETECTION_EN)
//------------------------------------------------------------------------------
//  Function    : VRMotionDetectCB
//  Description :
//------------------------------------------------------------------------------
void VRMotionDetectCB(MMP_UBYTE ubSnrSel)
{
    static AHC_BOOL bMDRunning = AHC_FALSE;
    static MMP_ULONG ulPreviousTime[MAX_ALGO_PIPE_NUM] = {0};
    MMP_ULONG ulCurrentTime;

    // Add this protection since it may be called by multi-tasks
    if (bMDRunning == AHC_TRUE) { 
        return;
    }

    bMDRunning = AHC_TRUE;

    ulCurrentTime = ((UINT64)OSTimeGet() * 1000) / OS_TICKS_PER_SEC;
    if ((ulCurrentTime - ulPreviousTime[ubSnrSel]) > 500) // SW debounce
    {
        printc("\r\nVR MD ubSnrSel %d  CB\n", ubSnrSel);
        ulPreviousTime[ubSnrSel] = ulCurrentTime;
   		if (uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE && (uiGetParkingCfg()->ubTriggerEncodeMethod & PARKING_MODE_TRIGGER_ENCODE_MOTION))
        	AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_MOTION_DTC, 0);
    }

    bMDRunning = AHC_FALSE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : VRFileFullCBParkingMode
//  Description :
//------------------------------------------------------------------------------
void VRFileFullCBParkingMode(void)
{
    AHC_BOOL            ahcRet = AHC_TRUE;   

    printc(FG_GREEN("<VRFileFullCBParkingMode>\r\n"));

    ahcRet = AHC_KeyEventIDCheckConflict(BUTTON_VRCB_FILE_FULL);
    if (ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(0,ahcRet); return;}      
    
    AHC_SetCurKeyEventID(BUTTON_VRCB_FILE_FULL);
    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_FILE_FULL, 0);
}

AHC_BOOL AHC_VIDEO_SetRecordModeRegisterCallbackParkingMode(void)
{
    AHC_BOOL            ahcRet = AHC_TRUE;           
    MMP_ERR sRet = MMP_ERR_NONE;
    
    ahcRet = AHC_VIDEO_SetRecordModeRegisterCallback();

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_FILE_FULL, (void*)VRFileFullCBParkingMode);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}         
    
    return ahcRet;    
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_CyclicDeleteFiles
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_CyclicDeleteFilesParkingMode(void)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    AHC_BOOL b_delete = 1;

    b_delete = AHC_VIDEO_GetRecordModeDeleteFile();
    
    /* Protect File Flow */
    //NOP
    
    #if (FS_FORMAT_FREE_ENABLE)
    b_delete = AHC_FALSE;
    #endif
    
    /* Delete File Flow */
    if (b_delete){
        if (AHC_Deletion_RemoveEx(DCF_DB_TYPE_2ND_DB, AHC_VIDEO_GetRecTimeLimitParkingMode()) == AHC_FALSE){
            if (AHC_Deletion_RemoveEx(DCF_DB_TYPE_2ND_DB, AHC_VIDEO_GetRecTimeLimitParkingMode()) == AHC_FALSE)
                return AHC_FALSE;            
        }
    }
    
    return ahcRet;    
}    

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecTimeLimit
//  Description :
//------------------------------------------------------------------------------
UINT32 AHC_VIDEO_GetRecTimeLimitParkingMode(void)
{    
    return uiGetParkingCfg()->ulTotalRecordLengthInMs / 1000;
}

AHC_BOOL AHC_VIDEO_SetRecordModeSetSeamlessParkingMode(void)
{    
    AHC_BOOL ahcRet = AHC_TRUE;   
    MMP_ERR sRet = MMP_ERR_NONE;

    sRet = MMPS_3GPRECD_StartSeamless(MMP_FALSE);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    AHC_VIDEO_SetVRSeamless(AHC_FALSE);
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_ParkingModePostProcess
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_ParkingModePostProcess(void)
{
    UINT16 usCurVideoDirKey = 0;
    UINT8* pbCurVideoFileName = NULL;
#if (VIDRECD_MULTI_TRACK == 0)
    UINT8 *pbCurVideoRearFileName = NULL;
#endif

    usCurVideoDirKey = AHC_VIDEO_GetCurRecDirKey();
    pbCurVideoFileName = AHC_VIDEO_GetCurRecFileName(AHC_FALSE);
#if (VIDRECD_MULTI_TRACK == 0)
    if(!CAM_CHECK_SCD(SCD_CAM_NONE)){
        pbCurVideoRearFileName = AHC_VIDEO_GetCurRecRearFileName(AHC_FALSE);
    }
    else if(!CAM_CHECK_USB(USB_CAM_NONE)){
        pbCurVideoRearFileName = AHC_VIDEO_GetCurRecUSBHFileName(AHC_FALSE);
    }	
#endif

    #if(GPS_RAW_FILE_ENABLE == 1)
    GPSCtrl_SwitchRawFilePingPongBuffer();
    #endif

    AHC_UF_SelectDB(DCF_DB_TYPE_2ND_DB);
    AHC_UF_PostAddFile(usCurVideoDirKey, (INT8*)pbCurVideoFileName);

    // Mantis : 1219044 /1223597
#if (VIDRECD_MULTI_TRACK == 0) // CHECK
    if (!CAM_CHECK_SCD(SCD_CAM_NONE) || !CAM_CHECK_USB(USB_CAM_NONE))
    { 
		/* Mantis : 1286138
		VIDMGR_CONTAINER_TYPE   ContainerType;
		UINT32                  Param;

		AIHC_VIDEO_GetMovieCfgEx(0, AHC_CLIP_CONTAINER_TYPE, &Param);
		ContainerType = Param;
			
		STRCAT((INT8*)pbCurVideoRearFileName, EXT_DOT);	        		            
		if (ContainerType == VIDMGR_CONTAINER_3GP) {                             
			STRCAT((INT8*)pbCurVideoRearFileName, MOVIE_3GP_EXT);
		}
		else {                                              
			STRCAT((INT8*)pbCurVideoRearFileName, MOVIE_AVI_EXT);               
		} Mantis : 1286138 */
		       
		AHC_UF_PostAddFile(usCurVideoDirKey,(INT8*)pbCurVideoRearFileName);
    }        
#endif        

    return AHC_TRUE;
}
