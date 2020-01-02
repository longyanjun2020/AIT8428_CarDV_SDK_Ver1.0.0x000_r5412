/*===========================================================================
 * Include file
 *===========================================================================*/

#include "customer_config.h"
#include "AHC_Common.h"
#include "AHC_General.h"
#include "AHC_Message.h"
#include "AHC_Display.h"
#include "AHC_Media.h"
#include "AHC_Capture.h"
#include "AHC_Dcf.h"
#include "AHC_Menu.h"
#include "AHC_Os.h"
#include "AHC_Fs.h"
#include "AHC_UF.h"
#include "AHC_Isp.h"
#include "AHC_Parameter.h"
#include "AHC_Warningmsg.h"
#include "AHC_Usb.h"
#include "AHC_General_CarDV.h"
#include "AIHC_Dcf.h"
#include "StateCameraFunc.h"
#include "StateVideoFunc.h"
#include "StateTVFunc.h"
#include "StateHDMIFunc.h"
#include "ZoomControl.h"
#include "MenuDrawCommon.h"
#include "MenuTouchButton.h"
#include "MenuSetting.h"
#include "UI_DrawGeneral.h"
#include "DrawStateCameraFunc.h"
#include "DrawStateVideoFunc.h"
#include "IconPosition.h"
#include "KeyParser.h"
#include "dsc_charger.h"
#include "LedControl.h"
#include "mmps_3gprecd.h"
#include "Ait_Utility.h"
#include "disp_drv.h"
#include "snr_cfg.h"
#include "lib_retina.h"
#include "mmp_reg_mci.h"

#if defined(WIFI_PORT) && (WIFI_PORT == 1)
#include "netapp.h"
#include "AHC_Stream.h"
#endif

/*===========================================================================
 * Macro define
 *===========================================================================*/

#define DSC_TIMER_UNIT                  (100)//unit :ms

/*===========================================================================
 * Global variable
 *===========================================================================*/


IMAGESIZE_SETTING	CaptureResolution	= IMAGE_SIZE_NUM;
UINT8				bZoomDirect			= AHC_SENSOR_ZOOM_STOP;
UINT8				CaptureTimerID		= 0xFF;
UINT32				CaptureCounter		= 0;
UINT32				RemainCaptureNum	= 0;
AHC_BOOL			bBurstShotComplete	= AHC_TRUE;
AHC_BOOL			bContiShotComplete	= AHC_TRUE;
UINT8				m_ubDSCMode			= DSC_ONE_SHOT;
AHC_BOOL			m_ubTimeLapseStart	= AHC_FALSE;
UINT32				m_ulTimeLapseReady	= AHC_FALSE;
UINT32				m_ulTimeLapseFileId;

#if (DSC_SELFSHOT_EN)
UINT16				SelfTime 			= 0;
AHC_BOOL			InSelfTime 			= AHC_FALSE;
AHC_BOOL			ubBackupLCDStatus;
#endif

#if (MOTION_DETECTION_EN) && (VMD_ACTION & VMD_BURST_CAPTURE)
static UINT8 		ubDSCMotionMvTh 	= 0;
static UINT8 		ubDSCMotionCntTh	= 0;
#endif

#if (SUPPORT_TOUCH_PANEL)
AHC_BOOL			m_ubShowDSCTouchPage= AHC_FALSE;
#endif
#if (DSC_DIGIT_ZOOM_ENABLE)
static AHC_BOOL		bZoomStop			= AHC_TRUE;
static AHC_BOOL		bZooming			= AHC_FALSE;
#endif
static AHC_BOOL		bShotByVMD			= AHC_FALSE;

AHC_BOOL gbCameraInSubMode = AHC_FALSE;

/*===========================================================================
 * Extern function
 *===========================================================================*/
extern void InitOSD(void);
/*===========================================================================
 * Extern variable
 *===========================================================================*/

extern AHC_BOOL 	bShowHdmiWMSG;
extern AHC_BOOL		bShowTvWMSG;
extern MMP_USHORT	gsStillZoomIndex;
extern AHC_BOOL		bForce_PowerOff;
extern AHC_BOOL		bNightMode;

#if (MOTION_DETECTION_EN)
extern AHC_BOOL 	m_ubMotionDtcEn;
extern UINT32		m_ulVMDRemindTime;
extern AHC_BOOL		m_ubInRemindTime;
extern AHC_BOOL		m_ulVMDCancel;
extern AHC_BOOL		m_ubVMDStart;
#endif

extern AHC_BOOL     gbAhcDbgBrk;

#if (HDMI_ENABLE)
//extern AHC_BOOL		bHaveInitOsdHdmi;
#endif

/*===========================================================================
 * Main body
 *===========================================================================*/

#ifdef CFG_LED_DSC_FLICKER_NUM //may be defined in config_xxx.h
static void CaptureTmHookCb(void)
{
	static int	t = 0;
	int MaxFlickerNum = CFG_LED_DSC_FLICKER_NUM;


	MMPF_PIO_SetData(LED_GPIO_POWER, ((t >> 1) & 1), 0);

	if (t >= MaxFlickerNum) {
		AHC_TmKeypadHook(NULL);
		t = 0;
	} else {
		t++;
	}
}
#endif

void CaptureTimer_ISR(void *tmr, void *arg)
{
#if (DSC_MODE_ENABLE)

    CaptureCounter++;

	#if (DSC_SELFSHOT_EN)
    if(InSelfTime)
    {
    	if( 0 == CaptureCounter%5 ) {

            if( 0 == CaptureCounter%10 )
            	SelfTime-- ;

            AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_UPDATE_MESSAGE, 0);
        }
    }
	else
	#endif	// DSC_SELFSHOT_EN
	{
		#if (DSC_MULTI_MODE & DSC_TIME_LAPSE)

		if(m_ubTimeLapseStart && m_ubDSCMode == DSC_TIME_LAPSE)
		{
			UINT32 ulTimeIntreval = CaptureFunc_GetTimeLapseInterval();

			if(0 == (CaptureCounter%ulTimeIntreval))
			{
				if (AHC_TRUE == m_ulTimeLapseReady) {
					m_ulTimeLapseReady = AHC_FALSE;

					RTNA_DBG_Str(0, "DSC_TIME_LAPSE time-up !!!\r\n");
					AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, KEY_DSC_CAPTURE, 0);
				}
				else {
					CaptureCounter--;
				}
			}
		}

		if (0 == CaptureCounter%5)
			AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_UPDATE_MESSAGE, 0);

		#else

	    if( 0 == CaptureCounter%5)
			AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_UPDATE_MESSAGE, 0);

    	#endif	// DSC_MULTI_MODE
    }

#endif
}

AHC_BOOL CaptureTimer_Start(UINT32 ulTime)
{
#if (DSC_MODE_ENABLE)

	if (0xFE <= CaptureTimerID) {
		CaptureCounter = 0;
		CaptureTimerID = AHC_OS_StartTimer( ulTime, AHC_OS_TMR_OPT_PERIODIC, CaptureTimer_ISR, (void*)NULL );
	}

	if (0xFE <= CaptureTimerID) {
		printc("--E-- Create Capture Timer fail !!!\r\n");
		return AHC_FALSE;
	}
	else
		return AHC_TRUE;
#else
	return AHC_TRUE;
#endif
}

AHC_BOOL CaptureTimer_Stop(void)
{
#if (DSC_MODE_ENABLE)
    UINT8 ret = 0;

	if (0xFE > CaptureTimerID) {
		ret = AHC_OS_StopTimer( CaptureTimerID, AHC_OS_TMR_OPT_PERIODIC );
	}

	CaptureCounter = 0;
	CaptureTimerID = 0xFF;

	if (0xFF == ret)
		return AHC_FALSE;
#endif

	return AHC_TRUE;
}

UINT32 CaptureFunc_GetTimeLapseInterval(void)
{
#if (MENU_STILL_TIMELAPSE_TIME_EN)

	UINT32 ulInterval = 0xFFFFFFFF;
	UINT8  ubVal	  = MenuSettingConfig()->uiTimeLapseTime;

	extern UINT16 m_ulTimeLapseTime[];

	#define MS_PER_SECOND		(1000)
	#define DSC_TIMER_PRIEOD 	(100)

	switch(ubVal)
    {
	#if (MENU_STILL_TIMELAPSE_TIME1_EN)
    	case TIMELAPSE_INTERVAL_1:
			ulInterval = ((double)(0.5)*(MS_PER_SECOND))/(DSC_TIMER_PRIEOD);
		break;
	#endif
	#if (MENU_STILL_TIMELAPSE_TIME2_EN)
    	case TIMELAPSE_INTERVAL_2:
			ulInterval = (m_ulTimeLapseTime[ubVal]*(MS_PER_SECOND))/(DSC_TIMER_PRIEOD);
		break;
	#endif
	#if (MENU_STILL_TIMELAPSE_TIME3_EN)
    	case TIMELAPSE_INTERVAL_3:
			ulInterval = (m_ulTimeLapseTime[ubVal]*(MS_PER_SECOND))/(DSC_TIMER_PRIEOD);
		break;
	#endif
	#if (MENU_STILL_TIMELAPSE_TIME4_EN)
    	case TIMELAPSE_INTERVAL_4:
			ulInterval = (m_ulTimeLapseTime[ubVal]*(MS_PER_SECOND))/(DSC_TIMER_PRIEOD);
		break;
	#endif
	#if (MENU_STILL_TIMELAPSE_TIME5_EN)
    	case TIMELAPSE_INTERVAL_5:
			ulInterval = (m_ulTimeLapseTime[ubVal]*(MS_PER_SECOND))/(DSC_TIMER_PRIEOD);
		break;
	#endif
	#if (MENU_STILL_TIMELAPSE_TIME6_EN)
    	case TIMELAPSE_INTERVAL_6:
			ulInterval = (m_ulTimeLapseTime[ubVal]*(MS_PER_SECOND))/(DSC_TIMER_PRIEOD);
		break;
	#endif
	#if (MENU_STILL_TIMELAPSE_TIME7_EN)
    	case TIMELAPSE_INTERVAL_7:
			ulInterval = (m_ulTimeLapseTime[ubVal]*(MS_PER_SECOND))/(DSC_TIMER_PRIEOD);
		break;
	#endif
	    default:
			ulInterval = 0xFFFFFFFF;
	    break;
    }

    return ulInterval;
#else
	return 0xFFFFFFFF;
#endif
}

AHC_BOOL CaptureFunc_SetResolution(IMAGESIZE_SETTING ubResolution, AHC_BOOL bSetOnly)
{
    AHC_BOOL ahc_ret = AHC_TRUE;

#if (DSC_MODE_ENABLE)

    CaptureResolution = ubResolution;

    switch(CaptureResolution)
    {
	#if (MENU_STILL_SIZE_30M_EN)
		case IMAGE_SIZE_30M:
			ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 6400, 4800);
      	break;
    #endif
    #if (MENU_STILL_SIZE_14M_EN)
		case IMAGE_SIZE_14M:
			ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 4352, 3264);
      	break;
    #endif
    #if (MENU_STILL_SIZE_12M_EN)
	 	case IMAGE_SIZE_12M:
			ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 4000, 3000);
		break;
    #endif
    #if (MENU_STILL_SIZE_8M_EN)
      	case IMAGE_SIZE_8M:
           	ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 3264, 2448);
      	break;
    #endif
    #if (MENU_STILL_SIZE_5M_EN)
      	case IMAGE_SIZE_5M:
           	ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 2560, 1920);
      	break;
    #endif
    #if (MENU_STILL_SIZE_3M_EN)
      	case IMAGE_SIZE_3M:
        #if (BIND_SENSOR_OV4689)
           	ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 2016, 1512);
        #else
           	ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 2048, 1536);
        #endif
      	break;
    #endif
    #if (MENU_STILL_SIZE_2M_WIDE_EN)
      	case IMAGE_SIZE_2M:
           	ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 1920, 1080);
           	//ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 1600, 1200);
      	break;
    #endif
    #if (MENU_STILL_SIZE_1d2M_EN)
      	case IMAGE_SIZE_1_2M:
           	ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 1280, 960);
      	break;
    #endif
    #if (MENU_STILL_SIZE_VGA_EN)
      	case IMAGE_SIZE_VGA:
           	ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 640, 480);
      	break;
    #endif
      	default:
           	ahc_ret = AHC_SetImageSize(STILL_CAPTURE_MODE+bSetOnly, 2560, 1920);
      	break;
    }
#endif
    return ahc_ret;
}

AHC_BOOL CaptureFunc_PresetSensorMode(IMAGESIZE_SETTING ubResolution)
{
    AHC_BOOL ahc_ret = AHC_TRUE;

#if (DSC_MODE_ENABLE)

	ahc_ret = AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_BEST_CAMERA_PREVIEW_RESOLUTION);

	switch(ubResolution)
	{
	#if (MENU_STILL_SIZE_30M_EN)
		case IMAGE_SIZE_30M:
			AHC_SNR_PresetCaptureSnrMode(AHC_SENSOR_MODE_4TO3_3M_30P_RESOLUTION);
		break;
	#endif
	#if (MENU_STILL_SIZE_14M_EN)
		case IMAGE_SIZE_14M:
			AHC_SNR_PresetCaptureSnrMode(AHC_SENSOR_MODE_4TO3_14M_30P_RESOLUTION);
		break;
	#endif
	#if (MENU_STILL_SIZE_12M_EN)
		case IMAGE_SIZE_12M:
			AHC_SNR_PresetCaptureSnrMode(AHC_SENSOR_MODE_4TO3_12M_30P_RESOLUTION);
		break;
	#endif
	#if (MENU_STILL_SIZE_8M_EN)
		case IMAGE_SIZE_8M:
			AHC_SNR_PresetCaptureSnrMode(AHC_SENSOR_MODE_4TO3_8M_30P_RESOLUTION);
		break;
	#endif
	#if (MENU_STILL_SIZE_5M_EN)
		case IMAGE_SIZE_5M:
			AHC_SNR_PresetCaptureSnrMode(AHC_SENSOR_MODE_4TO3_5M_30P_RESOLUTION);
		break;
    #endif
    #if (MENU_STILL_SIZE_3M_EN)
		case IMAGE_SIZE_3M:
			AHC_SNR_PresetCaptureSnrMode(AHC_SENSOR_MODE_4TO3_3M_30P_RESOLUTION);
		break;
	#endif
	#if (MENU_STILL_SIZE_2M_WIDE_EN)
		case IMAGE_SIZE_2M:
			AHC_SNR_PresetCaptureSnrMode(AHC_SENSOR_MODE_FULL_HD_30P_RESOLUTION);
		break;
	#endif
	#if (MENU_STILL_SIZE_1d2M_EN)
		case IMAGE_SIZE_1_2M:
			AHC_SNR_PresetCaptureSnrMode(AHC_SENSOR_MODE_4TO3_1D5M_30P_RESOLUTION);
		break;
    #endif
    #if (MENU_STILL_SIZE_VGA_EN)
      	case IMAGE_SIZE_VGA:
          	AHC_SNR_PresetCaptureSnrMode(AHC_SENSOR_MODE_4TO3_VGA_30P_RESOLUTION);
      	break;
    #endif
      	default:
          	AHC_SNR_PresetCaptureSnrMode(AHC_SENSOR_MODE_4TO3_1D5M_30P_RESOLUTION);
      	break;
    }

#endif
    return ahc_ret;
}

AHC_BOOL CaptureFunc_CheckMemSizeAvailable(UINT64 *pFreeBytes, UINT32 *pRemainCaptureNum)
{
    AHC_BOOL 	ahc_ret = AHC_TRUE;

#if (DSC_MODE_ENABLE)
    UINT64		uAvailSize = 0;

    if (AHC_ERR_NONE != AHC_FS_GetStorageFreeSpace(AHC_GetMediaPath(), &uAvailSize))
    {
        ahc_ret = AHC_FALSE;
        *pRemainCaptureNum = 0;
    }
    else
    {
        if ( uAvailSize > AHC_GetStillImageTargetSize()) {
            ahc_ret = AHC_TRUE;
            *pRemainCaptureNum = uAvailSize / 1024 / (AHC_GetStillImageTargetSize() / 1024 * 144 / 100);
        }
        else {
            ahc_ret = AHC_FALSE;
            *pRemainCaptureNum = 0;
        }
    }
    *pFreeBytes = uAvailSize;
#endif

    return ahc_ret;
}

UINT32 CaptureFunc_GetRemainCaptureNum(void)
{
#if (DSC_MODE_ENABLE)

    UINT64   ulAvailbleSize = 0;

	CaptureFunc_CheckMemSizeAvailable(&ulAvailbleSize, &RemainCaptureNum);

    return RemainCaptureNum;
#else
	return 0;
#endif
}

UINT16 CaptureFunc_CheckSelfTimer(void)
{
#if (DSC_MODE_ENABLE) && (DSC_SELFSHOT_EN)

	UINT8 ubLCDstatus;

    if(InSelfTime)
        return SelfTime;

	AHC_LCD_GetStatus(&ubLCDstatus);
	ubBackupLCDStatus = ubLCDstatus;

    if(MenuSettingConfig()->uiSelfTimer == SELF_TIMER_OFF)
    {
        SelfTime = 0;
        InSelfTime = AHC_FALSE;
        return 0;
    }

    switch(MenuSettingConfig()->uiSelfTimer)
    {
	    case SELF_TIMER_2S:
	        SelfTime = 2;
	    break;
	    case SELF_TIMER_10S:
	        SelfTime = 10;
	    break;
    }

    InSelfTime = AHC_TRUE;
    CaptureCounter = 0;
    DrawStateCaptureUpdate(EVENT_DSC_KEY_UPDATE_MESSAGE);

    return SelfTime;

#else
	return 0;
#endif
}

UINT16 CaptureFunc_GetSelfTime(void)
{
#if (DSC_MODE_ENABLE) && (DSC_SELFSHOT_EN)
    return SelfTime;
#else
	return 0;
#endif
}

AHC_BOOL CaptureFunc_Shutter(void)
{
    AHC_BOOL ahc_ret = AHC_TRUE;

#if (DSC_MODE_ENABLE)

    if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
        return AHC_VIDEO_CaptureClipCmd(AHC_CAPTURE_SNAPSHOT, 0);
    }
    else {
    
    AHC_ConfigEXIF_SystemInfo();
	AHC_ConfigEXIF_RTC();
	AHC_ConfigEXIF_MENU();

 	#if (DSC_BURSTSHOT_EN) 					|| \
 		(TIMELAPSE_MODE!=TIMELAPSE_NONE)	|| \
 		((MOTION_DETECTION_EN) && (VMD_ACTION & VMD_BURST_CAPTURE))

 	AHC_SetImageTime(100/*ReviewTime*/, 100/*WindowTime*/);
 	#endif

	{
		UINT8 MultiShotNum   = 1;
		UINT8 MultiShotDelay = 150;

		bBurstShotComplete = AHC_FALSE;

		#if ((MOTION_DETECTION_EN) && (VMD_ACTION & VMD_BURST_CAPTURE))
		if (AHC_TRUE == bShotByVMD)
		{
			MultiShotNum = AHC_GetVMDShotNumber();
		}
		else
		#endif
		#if (DSC_BURSTSHOT_EN)
		if (DSC_BURST_SHOT == m_ubDSCMode)
		{
			switch(MenuSettingConfig()->uiBurstShot)
			{
			case BURST_SHOT_LO:
				MultiShotNum = 2;
			break;
			case BURST_SHOT_MID:
				MultiShotNum = 3;
			break;
			case BURST_SHOT_HI:
				MultiShotNum = 5;
			break;
			}
		}
		#endif

		#if (MENU_STILL_SIZE_14M_EN)
		if (MenuSettingConfig()->uiIMGSize==IMAGE_SIZE_14M)
			MultiShotNum = 1;//TBD, 14M/12M Multishot will exceed 32MB dram range and cause system hang.
		#endif

		#if (MENU_STILL_SIZE_12M_EN)
		if (MenuSettingConfig()->uiIMGSize==IMAGE_SIZE_12M)
			MultiShotNum = 1;//TBD, 14M/12M Multishot will exceed 32MB dram range and cause system hang.
		#endif

		printc("MultiShotNum = %d\r\n", MultiShotNum);
        //check filenum TH and delete files when num is bigger than TH

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
            
	#if (defined(WIFI_PORT) && WIFI_PORT == 1)
		/*
		Whatever MJPEG or H264 streaming is running,
		pause the streaming here to prevent 
		the sensor mode from changing .
		*/
		AHC_SetStreamingMode(AHC_STREAM_PAUSE);
	#endif
            
		if (MultiShotNum==1)
		{
			ahc_ret = AHC_SetMode(AHC_MODE_STILL_CAPTURE);
		}
		else
		{
 			AHC_SetSeqCaptureParam(MultiShotNum, MultiShotDelay);
			ahc_ret = AHC_SetMode(AHC_MODE_SEQUENTIAL_CAPTURE);
		}
		
	#if (defined(WIFI_PORT) && WIFI_PORT == 1)
		AHC_SetStreamingMode(AHC_STREAM_RESUME);
		CGI_SET_STATUS(EVENT_DSC_KEY_CAPTURE, CGI_ERR_NONE/*0*/ /* SUCCESSFULLY */);
	#endif
	
		bBurstShotComplete = AHC_TRUE;
		bShotByVMD = AHC_FALSE;
		m_ulTimeLapseReady = AHC_TRUE;

		#if (MOTION_DETECTION_EN)
		m_ulVMDRemindTime = 1;
		m_ubVMDStart = AHC_FALSE;
		#endif
	}
    }
#endif
    
    return ahc_ret;
}

AHC_BOOL CaptureFunc_ContiShot(void)
{
    AHC_BOOL ahc_ret = AHC_TRUE;

#if (DSC_MODE_ENABLE)

    AHC_ConfigEXIF_SystemInfo();
	AHC_ConfigEXIF_RTC();
	AHC_ConfigEXIF_MENU();
	
	bContiShotComplete = AHC_FALSE;

	ahc_ret = AHC_SetMode(AHC_MODE_CONTINUOUS_CAPTURE);

#endif
    return ahc_ret;
}

AHC_BOOL CaptureFunc_Preview(void)
{
#if (DSC_MODE_ENABLE)

    AHC_BOOL ahc_ret		= AHC_TRUE;

    UINT64   ulAvailbleSize	= 0;
    UINT16   zoomratio		= 4;
    UINT8    bRotate		= 0;//VERTICAL_LCD;
    UINT16 dw, dh;

    if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
        extern AHC_BOOL VideoFunc_DualBayerSnrCapturePreview(void);
        return VideoFunc_DualBayerSnrCapturePreview();
    }

#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0) 
    bRotate		= 0;
#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
    bRotate		= 1;             
#endif

    AHC_SetPreviewZoomConfig(ZoomCtrl_GetStillDigitalZoomMax(), (UINT8)zoomratio);

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

    AHC_SetDisplayWindow(DISPLAY_SYSMODE_STILLCAPTURE, AHC_TRUE, bRotate, 0, 0, dw, dh, 0);

    AHC_UF_SetFreeChar(0, DCF_SET_FREECHAR, (UINT8 *) DSC_DEFAULT_FLIE_FREECHAR);

    CaptureFunc_SetResolution(MenuSettingConfig()->uiIMGSize, AHC_FALSE);

    CaptureFunc_PresetSensorMode(MenuSettingConfig()->uiIMGSize);

	#ifdef	_OEM_
	Oem_SetPhoto_Prop(MenuSettingConfig()->uiIMGSize);
	#endif

	AHC_PreSetFlashLED();

	#if (MOTION_DETECTION_EN)
	m_ubVMDStart 	  = AHC_FALSE;
	#endif

    ahc_ret = AHC_SetMode(AHC_MODE_CAPTURE_PREVIEW);



	switch(MenuSettingConfig()->uiIMGQuality)
	{
	    case QUALITY_SUPER_FINE:
	        AHC_SetCompressionRatio(350);
	    break;
	    case QUALITY_FINE:
	        AHC_SetCompressionRatio(280);
	    break;
    }

    CaptureFunc_CheckMemSizeAvailable(&ulAvailbleSize, &RemainCaptureNum);

    return ahc_ret;
#else
	return AHC_TRUE;
#endif
}

AHC_BOOL CaptureFunc_EnterVMDMode(void)
{
#if (DSC_MODE_ENABLE) && (MOTION_DETECTION_EN)

	m_ubInRemindTime = AHC_TRUE;

    #ifdef CFG_REC_CUS_VMD_REMIND_TIME
	m_ulVMDRemindTime = CFG_REC_CUS_VMD_REMIND_TIME;
	#else
	m_ulVMDRemindTime = 10;
	#endif

	m_ubMotionDtcEn = AHC_TRUE;
	m_ulVMDCancel   = AHC_FALSE;

	AHC_SetMode(AHC_MODE_IDLE);
	CaptureFunc_Preview();

#endif
	return AHC_TRUE;
}

AHC_BOOL CaptureFunc_ExitVMDMode(void)
{
#if (DSC_MODE_ENABLE) && (MOTION_DETECTION_EN)

	m_ubMotionDtcEn = AHC_FALSE;

	m_ubInRemindTime = AHC_FALSE;

    if (m_ulVMDRemindTime)
    {
    	m_ulVMDCancel = AHC_TRUE;
	} else {
	    AHC_SetMode(AHC_MODE_IDLE);
	    CaptureFunc_Preview();
    }
#endif
	return AHC_TRUE;
}

AHC_BOOL CaptureFunc_TimeLapseVideo(AHC_BOOL bEn)
{
#if ((DSC_MODE_ENABLE) && SUPPORT_TIMELAPSE)

	static AHC_BOOL 		bFileOpened = AHC_FALSE;
	UINT8 					ubFrameRate = 10;
	UINT32 					uwImgH, uwImgW;
	MMP_ERR                 err;
	MMP_BYTE                bAviFileName[MAX_FILE_NAME_SIZE];
	MMP_BYTE                DirName[16];
	UINT16                  DirKey;
	UINT8                   FileName[16];
	UINT8                   bCreateNewDir;
	
    if(AHC_SDMMC_BasicCheck() == AHC_FALSE)
    	return AHC_FALSE;

    if(!bEn)
    {
		if(bFileOpened)
		{
			printc("Close TimeLapse File!!\r\n");
			MMPS_FS_FileClose(m_ulTimeLapseFileId);
			bFileOpened = AHC_FALSE;
			return AHC_TRUE;
		}
    }
    else
    {
        MEMSET(bAviFileName, 0, sizeof(bAviFileName));
        MEMSET(DirName, 0, sizeof(DirName));
        MEMSET(FileName, 0, sizeof(FileName));

		AHC_UF_SetFreeChar(0,DCF_SET_FREECHAR, (UINT8 *) "TLAP");

		if ( AHC_UF_GetName(&DirKey, (INT8*)DirName, (INT8*)FileName, &bCreateNewDir) == AHC_TRUE) 
		{
			printc("DirKey=%d  FileName=%s\r\n", DirKey, FileName);

			STRCPY(bAviFileName,(MMP_BYTE*)AHC_UF_GetRootDirName());
			STRCAT(bAviFileName,"\\");
			STRCAT(bAviFileName,DirName);

			if ( bCreateNewDir ) {
				MMPS_FS_DirCreate((INT8*)bAviFileName, STRLEN(bAviFileName));
				AHC_UF_AddDir(DirName);
			}

			STRCAT(bAviFileName,"\\");
			STRCAT(bAviFileName,(INT8*)FileName);
			STRCAT(bAviFileName,".");
			STRCAT(bAviFileName,"AVI");

			AHC_GetImageSize(STILL_CAPTURE_MODE, &uwImgW, &uwImgH);

			err = MMPS_3GPRECD_InitAVIFile(bAviFileName, strlen(bAviFileName), uwImgW, uwImgH,
										   0, ubFrameRate*1000, (uwImgW*uwImgH*2)*8*ubFrameRate,
										   MMP_TRUE,
										   &m_ulTimeLapseFileId);

			printc("Init TimeLapse File uwImgW[%d] uwImgH[%d] FileID[0x%x]\r\n",uwImgW, uwImgH, m_ulTimeLapseFileId);

			if ( MMP_ERR_NONE == err )
			{
				bFileOpened = AHC_TRUE;

				STRCAT((INT8*)FileName,".");
				STRCAT((INT8*)FileName,"AVI");

				AHC_UF_PreAddFile(DirKey,(INT8*)FileName);
				AHC_UF_PostAddFile(DirKey,(INT8*)FileName);
			}
		}
	}
#endif
	return AHC_TRUE;
}

AHC_BOOL CaptureFunc_ZoomOperation(AHC_ZOOM_DIRECTION bZoomDir)
{
    AHC_BOOL ahc_ret = AHC_TRUE;

#if (DSC_DIGIT_ZOOM_ENABLE && (DSC_MODE_ENABLE))

    if( AHC_SENSOR_ZOOM_IN == bZoomDir )
    {
		if( ZoomCtrl_GetStillDigitalZoomMax() > ZoomCtrl_GetStillDigitalZoomLevel() )
        {
        	ahc_ret = ZoomCtrl_DigitalZoom(STILL_CAPTURE_MODE, AHC_SENSOR_ZOOM_IN);
        }
    }
    else if ( AHC_SENSOR_ZOOM_OUT == bZoomDir )
    {
		ahc_ret = ZoomCtrl_DigitalZoom(STILL_CAPTURE_MODE, AHC_SENSOR_ZOOM_OUT);
    }
	else
    {
		ahc_ret = ZoomCtrl_DigitalZoom(STILL_CAPTURE_MODE, AHC_SENSOR_ZOOM_STOP);
    }
#endif
    return ahc_ret;
}

AHC_BOOL CaptureMode_Start(void)
{
    AHC_BOOL 	ahc_ret = AHC_TRUE;

#if (DSC_MODE_ENABLE)

    if(AHC_WMSG_States())
       AHC_WMSG_Draw(AHC_FALSE, WMSG_NONE, 0);

    bShowHdmiWMSG 	= AHC_TRUE;
    bShowTvWMSG 	= AHC_TRUE;

    ahc_ret = CaptureFunc_Preview();

    AHC_SetFastAeAwbMode( AHC_TRUE );

    if( AHC_TRUE == ahc_ret )
    {
		#if (SUPPORT_TOUCH_PANEL)
        KeyParser_ResetTouchVariable();
        KeyParser_TouchItemRegister(&CaptureMainPage_TouchButton[0], ITEMNUM(CaptureMainPage_TouchButton));
		#endif

        CaptureTimer_Start(DSC_TIMER_UNIT);

		#if (DAY_NIGHT_MODE_SWITCH_EN)
		bNightMode = (MenuSettingConfig()->uiScene == SCENE_TWILIGHT)?(AHC_TRUE):(AHC_FALSE);
		#endif

        DrawStateCaptureUpdate(EVENT_DSC_PREVIEW_INIT);
    }

    /*
     *	Init Icon Window Color Palette, for Time Stamp.
     */
	AHC_OSDLoadWinPalette(DATESTAMP_PRIMARY_OSD_ID);

#endif
    return ahc_ret;
}

AHC_BOOL CaptureMode_PreviewUpdate(void)
{
#if (DSC_MODE_ENABLE)

    AHC_BOOL 	ahc_ret 		= AHC_TRUE;
	AHC_BOOL	ubUpdatePreview = AHC_FALSE;
    UINT64   	ulAvailbleSize 	= 0;
	UINT8 	 	ubLCDstatus;

	if(CaptureResolution != MenuSettingConfig()->uiIMGSize)
		ubUpdatePreview = AHC_TRUE;

    if(ubUpdatePreview == AHC_TRUE)
    {
        AHC_SetMode(AHC_MODE_IDLE);
        ahc_ret = CaptureFunc_Preview();
    }

    CaptureTimer_Start(DSC_TIMER_UNIT);

	#if (DAY_NIGHT_MODE_SWITCH_EN)
	bNightMode = (MenuSettingConfig()->uiScene == SCENE_TWILIGHT)?(AHC_TRUE):(AHC_FALSE);
	#endif

    switch(MenuSettingConfig()->uiIMGQuality)
    {
	    case QUALITY_SUPER_FINE:
	        AHC_SetCompressionRatio(350);
	    break;
	    case QUALITY_FINE:
	        AHC_SetCompressionRatio(280);
	    break;
    }

    CaptureFunc_CheckMemSizeAvailable(&ulAvailbleSize, &RemainCaptureNum);

	#if (SUPPORT_TOUCH_PANEL)
    KeyParser_ResetTouchVariable();
    KeyParser_TouchItemRegister(&CaptureMainPage_TouchButton[0], ITEMNUM(CaptureMainPage_TouchButton));
	#endif

    AHC_LCD_GetStatus(&ubLCDstatus);

    if(ubLCDstatus == AHC_LCD_NORMAL)
    	DrawStateCaptureUpdate(EVENT_LCD_COVER_NORMAL);
    else if(ubLCDstatus == AHC_LCD_REVERSE)
    	DrawStateCaptureUpdate(EVENT_LCD_COVER_ROTATE);

    return ahc_ret;

#else
	return AHC_TRUE;
#endif
}

UINT32 StateCaptureModeHnadler(UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam)
{
    UINT32 ulCaptureEvent = 0;
    ulCaptureEvent = KeyParser_CaptureEvent(ulMsgId, ulEvent, ulParam);
    StateModeDoHandlerEvent(ulMsgId, ulCaptureEvent, ulParam);
    
    return 0;
}

//******************************************************************************
//
//                              AHC State Capture Mode
//
//******************************************************************************


void StateCaptureHDMIMode( UINT32 ulJobEvent )
{
}

void StateCaptureTVMode( UINT32 ulJobEvent )
{
}

AHC_BOOL StateCaptureModeInitLCD(void* pData)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    MMPC_HDMI_PC_Enable(MMP_TRUE);
    
    StateLedControl(UI_CAMERA_STATE);
    StateLCDCheckStatus(UI_CAMERA_STATE);
    
#if (SWITCH_MODE_FREEZE_WIN)
	AHC_OS_SleepMs(100);// Prevent changing mode too fast 
#endif
    AHC_SetMode(AHC_MODE_IDLE);
#if (SWITCH_MODE_FREEZE_WIN)
    MMPS_Display_FreezeWinUpdate(AHC_TRUE, AHC_TRUE, AHC_TRUE);// Video preview to DSC preview
#endif	

    ahcRet = AHC_SwitchDisplayOutputDevLCD();
	RTNA_LCD_Backlight(MMP_TRUE);
    SetCurrentOpMode(CAPTURE_MODE);
    ahcRet = CaptureMode_Start();

    return ahcRet;
}

#if (TVOUT_ENABLE)
AHC_BOOL StateCaptureModeInitTV(void* pData)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    RTNA_LCD_Backlight(MMP_FALSE);
#ifdef CFG_ENBLE_PANEL_SLEEP
    RTNA_LCD_Enter_Sleep();
#endif

#if 0
    AHC_SetMode(AHC_MODE_IDLE);
	{
		if(AHC_IsTVConnectEx()){ //TBD
			AHC_OSDUninit(); //TBD
			InitOSD();			
			//TVFunc_InitPreviewOSD();  
			
//			m_uwTVPrevOsdId = TVFunc_InitPreviewOSD();
//		    CaptureTimer_Start(100); 
//		    if(CaptureFunc_Preview()) {
//		        DrawStateTVPreviewCameraInit(m_uwTVPrevOsdId);
//		        BEGIN_LAYER(m_uwTVPrevOsdId);
//		        AHC_OSDSetActive(m_uwTVPrevOsdId, 1);
//		        END_LAYER(m_uwTVPrevOsdId);
//		        m_TVStatus = AHC_TV_DSC_PREVIEW_STATUS;
//		    }			
		}
	}
#endif

    ahcRet = AHC_SwitchDisplayOutputDevTVout();

    MMPC_HDMI_PC_Enable(MMP_TRUE);
    //StateLedControl(UI_CAMERA_STATE);
    //StateLCDCheckStatus(UI_CAMERA_STATE);

    //AHC_SetMode(AHC_MODE_IDLE);
    SetCurrentOpMode(CAPTURE_MODE);
    ahcRet = CaptureMode_Start();

    return ahcRet;
}
#endif

#if (HDMI_ENABLE)
AHC_BOOL StateCaptureModeInitHDMI(void* pData)
{
    AHC_BOOL ahcRet = AHC_TRUE;
	AITPS_MCI   pMCI = AITC_BASE_MCI;

	pMCI->MCI_INIT_DEC_VAL[MCI_SRC_LCD1_IBC3] = MCI_INIT_WT_MAX;
	pMCI->MCI_INIT_DEC_VAL[MCI_SRC_LCD2_CCIR1_IBC4] = MCI_INIT_WT_MAX ;
	pMCI->MCI_INIT_DEC_VAL[MCI_SRC_LCD3_CCIR2] = MCI_INIT_WT_MAX ;

//	pMCI->MCI_INIT_DEC_VAL[MCI_SRC_IBC1] = MCI_INIT_WT_MAX ;
	pMCI->MCI_INIT_DEC_VAL[MCI_SRC_RAW_F] = MCI_INIT_WT_MAX ;
#if (1)
    printc("%s,%d \n", __func__, __LINE__);
#endif
    RTNA_LCD_Backlight(MMP_FALSE);
#ifdef CFG_ENBLE_PANEL_SLEEP
    RTNA_LCD_Enter_Sleep();
#endif
    ahcRet = AHC_SwitchDisplayOutputDevHDMI();

    MMPC_HDMI_PC_Enable(MMP_TRUE);
    StateLedControl(UI_CAMERA_STATE);
    StateLCDCheckStatus(UI_CAMERA_STATE);

    SetCurrentOpMode(CAPTURE_MODE);
    ahcRet = CaptureMode_Start();

    return ahcRet;
}
#endif

AHC_BOOL StateCaptureModeShutDown(void* pData)
{
#if (1)
    printc("%s,%d \n", __func__, __LINE__);
#endif
    CaptureTimer_Stop();

    AHC_SetMode(AHC_MODE_IDLE);

    return AHC_TRUE;
}

//******************************************************************************
//
//                              AHC State Camera Mode
//
//******************************************************************************
#if (DSC_MODE_ENABLE)

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_TELEPRESS)
{
#if (DSC_DIGIT_ZOOM_ENABLE)
	AHC_BOOL 	ret 			= AHC_TRUE;
	
	if((bZoomDirect == AHC_SENSOR_ZOOM_OUT) && !(bZoomStop)){
        CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
        bZoomStop = AHC_TRUE;
    }

    #ifdef PREVIEW_ZOOM_BY_TOGGLE
    if ((bZoomStop == AHC_FALSE) && (bZoomDirect == AHC_SENSOR_ZOOM_IN)) {
        CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
        bZoomStop = AHC_TRUE;
        bZoomDirect = AHC_SENSOR_ZOOM_IN;
        DrawStateCaptureUpdate(ulJobEvent);
        break;
    }
    #endif

    if(bZoomStop){
        ret = CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_IN);
        bZoomStop = AHC_FALSE;
        #ifdef	PREVIEW_ZOOM_BY_STEP
        /* TODO: Zoom by Step don't work, sleep is a workaround solution */
        AHC_OS_Sleep(230);	// 230 is try and error for 4 steps, customer ask
        CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
        bZoomStop = AHC_TRUE;
        bZooming = AHC_FALSE;
    #endif
    }

    bZoomDirect = AHC_SENSOR_ZOOM_IN;
    DrawStateCaptureUpdate(ulEvent);
#endif    
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_WIDEPRESS)
{
#if (DSC_DIGIT_ZOOM_ENABLE)

	AHC_BOOL 	ret 			= AHC_TRUE;
	
	if((bZoomDirect == AHC_SENSOR_ZOOM_IN) && !(bZoomStop)){
        CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
        bZoomStop = AHC_TRUE;
    }

    #ifdef PREVIEW_ZOOM_BY_TOGGLE
    if ((bZoomStop == AHC_FALSE) && (bZoomDirect == AHC_SENSOR_ZOOM_OUT)) {
        CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
        bZoomStop = AHC_TRUE;
        bZoomDirect = AHC_SENSOR_ZOOM_OUT;
        DrawStateCaptureUpdate(ulJobEvent);
        break;
    }
    #endif

    if(bZoomStop){
        ret = CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_OUT);
        bZoomStop = AHC_FALSE;
        #ifdef	PREVIEW_ZOOM_BY_STEP
        /* TODO: Zoom by Step don't work, sleep is a workaround solution */
        AHC_OS_Sleep(230);	// 230 is try and error for 4 steps, customer ask
        CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
        bZoomStop = AHC_TRUE;
        #endif
    }

    bZoomDirect = AHC_SENSOR_ZOOM_OUT;
    DrawStateCaptureUpdate(ulEvent);
#endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_TELESTOP)
{
#if (DSC_DIGIT_ZOOM_ENABLE)
	AHC_BOOL 	ret 			= AHC_TRUE;
	ret = CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
    bZoomStop = AHC_TRUE;
    bZooming = AHC_FALSE;
    DrawStateCaptureUpdate(ulEvent);
#endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_WIDESTOP)
{
#if (DSC_DIGIT_ZOOM_ENABLE)
	AHC_BOOL	ret 			= AHC_TRUE;
	ret = CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
	bZoomStop = AHC_TRUE;
	bZooming = AHC_FALSE;
	DrawStateCaptureUpdate(ulEvent);
#endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_UP)
{
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_DOWN)
{
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_LEFT)
{
	AHC_ChangeEV(AHC_TRUE, NULL);
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_RIGHT)
{
	AHC_ToggleFlashLED(LED_MODE_AUTO_ON_OFF);
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_MENU)
{
#if CR_USE_STATE_SWITCH_SUB_MODE
	StateReplaceSubMode(UI_CAMERA_MENU_STATE);
#else
    AHC_BOOL    ahcRet = AHC_TRUE;
	UI_STATE_ID ubParentUIState = 0;
	
	StateModeGetParent(uiGetCurrentState(), &ubParentUIState);
    if(UI_STATE_UNSUPPORTED != ubParentUIState){
        printc("%s,%d, DettachSubMode:%d\r\n", __func__, __LINE__, uiGetCurrentState());
        ahcRet = StateDetachSubMode();
        if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, ahcRet);}                                  
    }
    ahcRet = StateAttachSubMode(UI_CAMERA_MENU_STATE);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); /*return ahcRet;*/} 
#endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_CHANGE_SHOT_MODE)
{
#if (DSC_MULTI_MODE != DSC_MULTI_DIS)
	if (DSC_ONE_SHOT == m_ubDSCMode) {
		#if (DSC_MULTI_MODE & DSC_BURST_SHOT)
		m_ubDSCMode	= DSC_BURST_SHOT;
		RTNA_DBG_Str(0, " ===> DSC_BURST_SHOT\r\n");
		#elif (DSC_MULTI_MODE & DSC_TIME_LAPSE)
		m_ubDSCMode	= DSC_TIME_LAPSE;
		RTNA_DBG_Str(0, " ===> DSC_TIME_LAPSE\r\n");
		#endif
	}
	else if (DSC_BURST_SHOT == m_ubDSCMode) {
		#if (DSC_MULTI_MODE & DSC_TIME_LAPSE)
		m_ubDSCMode	= DSC_TIME_LAPSE;
		RTNA_DBG_Str(0, " ===> DSC_TIME_LAPSE\r\n");
		#elif (DSC_MULTI_MODE & DSC_ONE_SHOT)
		m_ubDSCMode	= DSC_ONE_SHOT;
		RTNA_DBG_Str(0, " ===> DSC_ONE_SHOT\r\n");
		#endif
	}
	else if (DSC_TIME_LAPSE == m_ubDSCMode) {
		if (AHC_FALSE == m_ubTimeLapseStart) {
			#if (DSC_MULTI_MODE & DSC_ONE_SHOT)
			m_ubDSCMode	= DSC_ONE_SHOT;
			RTNA_DBG_Str(0, " ===> DSC_ONE_SHOT\r\n");
			#elif (DSC_MULTI_MODE & DSC_BURST_SHOT)
			m_ubDSCMode	= DSC_BURST_SHOT;
			RTNA_DBG_Str(0, " ===> DSC_BURST_SHOT\r\n");
			#endif
		}
	}
#endif	
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_MODE)
{
	AHC_BOOL	ret 			= AHC_TRUE;
	
        AHC_SetFastAeAwbMode( AHC_FALSE );		
		DrawStateCaptureUpdate(ulEvent);
		CaptureTimer_Stop();
		gsStillZoomIndex = ZoomCtrl_GetStillDigitalZoomLevel();
	#if (CYCLIC_MODE_CHANGE)
	#if (ALL_TYPE_FILE_BROWSER_PLAYBACK )
        SetCurrentOpMode(JPGPB_MOVPB_MODE);
	#else
		SetCurrentOpMode(MOVPB_MODE);
	#endif
			
	#if (FOCUS_ON_LATEST_FILE)
    	AHC_SetPlayFileOrder(PLAY_LAST_FILE);
    #endif
		ret = StateSwitchMode(UI_BROWSER_STATE);
	#else
		ret = StateSwitchMode(UI_VIDEO_STATE);
	#endif		
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_CAPTURE)
{
	UINT64   	ulAvailbleSize 	= 0;
    AHC_BOOL	playButtonSound = AHC_TRUE;

#if DSC_DIGIT_ZOOM_ENABLE
if(!bZoomStop)
{
	printc("Stop Zoom First\r\n");
	CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
	bZoomStop = AHC_TRUE;
	//goto L_CaptureEnd;
}
#endif

if(AHC_WMSG_States())
	AHC_WMSG_Draw(AHC_FALSE, WMSG_NONE, 0);

if (AHC_SDMMC_BasicCheck() == AHC_FALSE)
{
	printc("--E-- %s: AHC_SDMMC_BasicCheck fail\r\n", __func__);
	goto L_CaptureEnd;
}

if (!CaptureFunc_CheckMemSizeAvailable(&ulAvailbleSize, &RemainCaptureNum))
{
	AHC_WMSG_Draw(AHC_TRUE, WMSG_STORAGE_FULL, 3);
	printc("--E-- %s: Storage full !!!\r\n", __func__);

	goto L_CaptureEnd;
}

if (CaptureFunc_CheckSelfTimer())
{
	printc("--W-- In DSC Self timer ...\r\n");

	goto L_CaptureEnd;
}

if (AHC_FALSE == AHC_SDMMC_IsSD1MountDCF())
{
	AHC_WMSG_Draw(AHC_TRUE, WMSG_INSERT_SD_AGAIN, 2);
	printc("--E-- %s: Storage is not mounted !!!\r\n", __func__);

	goto L_CaptureEnd;
}

playButtonSound = AHC_FALSE;

#ifdef CFG_CAPTURE_WITH_VIBRATION
AHC_Vibration_Enable(CFG_CAPTURE_VIBRATION_TIME);
#endif

#ifdef CFG_CAPTURE_WITH_KEY_LED_CAPTURE
if (KEY_LED_CAPTURE == LED_GPIO_POWER) {
	// Same LED : Turn-Off LED
	LedCtrl_ButtonLed(LED_GPIO_POWER, !LED_GPIO_POWER_ACT_LEVEL);
}
else {
	// Turn-On LED
	LedCtrl_ButtonLed(KEY_LED_CAPTURE, AHC_TRUE);
}
#endif

#if (USE_SHUTTER_SOUND)
AHC_PlaySoundEffect(AHC_SOUNDEFFECT_SHUTTER);
#endif

#ifdef CFG_LED_DSC_FLICKER_NUM //may be defined in config_xxx.h
AHC_TmKeypadHook((void*)CaptureTmHookCb);
#endif

#if (DCF_DB_COUNT >= 4)
AHC_UF_SelectDB(DCF_DB_TYPE_4TH_DB);
#endif

CaptureFunc_Shutter();

CaptureFunc_CheckMemSizeAvailable(&ulAvailbleSize, &RemainCaptureNum);
DrawStateCaptureUpdate(EVENT_DSC_KEY_CAPTURE_STATUS_CLEAR);

L_CaptureEnd:
if (playButtonSound) {
	AHC_PlaySoundEffect(AHC_SOUNDEFFECT_BUTTON);
}

#ifdef CFG_CAPTURE_WITH_KEY_LED_CAPTURE
if (KEY_LED_CAPTURE == LED_GPIO_POWER) {
	// Same LED : Turn-On LED
	LedCtrl_ButtonLed(LED_GPIO_POWER, LED_GPIO_POWER_ACT_LEVEL);
}
else {
	// Turn-Off LED
	LedCtrl_ButtonLed(KEY_LED_CAPTURE, AHC_FALSE);
}
#endif

#ifdef KEY_CAPATURE_SWITCH_VIDEO //may be defined in config_xxx.h
SetKeyPadEvent(KEY_CAPATURE_SWITCH_VIDEO);//Switch to Video Preview
#endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_KEY_PLAYBACKMODE)
{
	AHC_BOOL	ret 			= AHC_TRUE;

	{
		DrawStateCaptureUpdate(ulEvent);
		CaptureTimer_Stop();
		gsStillZoomIndex = ZoomCtrl_GetStillDigitalZoomLevel();
		ret = StateSwitchMode(UI_BROWSER_STATE);
	}
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_POWEROFF)
{
	AHC_PowerOff_NormalPath();
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_CHANGELED)
{
	#if (LED_FLASH_CTRL!=LED_BY_NONE)
    AHC_ToggleFlashLED(LED_MODE_ON_OFF);
	#endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_POWERSAVE)
{
	AHC_SwitchLCDBackLight();
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_NIGHTMODE)
{
    #if (DAY_NIGHT_MODE_SWITCH_EN)
    AHC_ToggleTwilightMode();
    #endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_CAMERABROWSER)
{
	DrawStateCaptureUpdate(EVENT_KEY_PLAYBACK_MODE);
	CaptureTimer_Stop();
	gsStillZoomIndex = ZoomCtrl_GetStillDigitalZoomLevel();

	if(ulEvent==EVENT_CAMERA_BROWSER)
		SetCurrentOpMode(JPGPB_MODE);
	else if(ulEvent==EVENT_VIDEO_BROWSER)
		SetCurrentOpMode(MOVPB_MODE);
	else if(ulEvent==EVENT_ALL_BROWSER)
		SetCurrentOpMode(JPGPB_MOVPB_MODE);

	#if (FOCUS_ON_LATEST_FILE)
	AHC_SetPlayFileOrder(PLAY_LAST_FILE);
	#endif

	StateSwitchMode(UI_BROWSER_STATE);
	
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_VIDEOBROWSER)
{
	DrawStateCaptureUpdate(EVENT_KEY_PLAYBACK_MODE);
	CaptureTimer_Stop();
	gsStillZoomIndex = ZoomCtrl_GetStillDigitalZoomLevel();

	if(ulEvent==EVENT_CAMERA_BROWSER)
		SetCurrentOpMode(JPGPB_MODE);
	else if(ulEvent==EVENT_VIDEO_BROWSER)
		SetCurrentOpMode(MOVPB_MODE);
	else if(ulEvent==EVENT_ALL_BROWSER)
		SetCurrentOpMode(JPGPB_MOVPB_MODE);

	#if (FOCUS_ON_LATEST_FILE)
	AHC_SetPlayFileOrder(PLAY_LAST_FILE);
	#endif

	StateSwitchMode(UI_BROWSER_STATE);
	
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_ALLBROWSER)
{
	DrawStateCaptureUpdate(EVENT_KEY_PLAYBACK_MODE);
	CaptureTimer_Stop();
	gsStillZoomIndex = ZoomCtrl_GetStillDigitalZoomLevel();

	if(ulEvent==EVENT_CAMERA_BROWSER)
		SetCurrentOpMode(JPGPB_MODE);
	else if(ulEvent==EVENT_VIDEO_BROWSER)
		SetCurrentOpMode(MOVPB_MODE);
	else if(ulEvent==EVENT_ALL_BROWSER)
		SetCurrentOpMode(JPGPB_MOVPB_MODE);

	#if (FOCUS_ON_LATEST_FILE)
	AHC_SetPlayFileOrder(PLAY_LAST_FILE);
	#endif

	StateSwitchMode(UI_BROWSER_STATE);
}

static void STATE_CAMERA_MODE_EVENT_PREVIEWMAIN(void)
{
	CaptureTimer_Stop();
	gsStillZoomIndex = ZoomCtrl_GetStillDigitalZoomLevel();
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_PREVIEW)
{	
	AHC_BOOL 	ret 			= AHC_TRUE;

	{
		DrawStateCaptureUpdate(EVENT_KEY_MODE);
		STATE_CAMERA_MODE_EVENT_PREVIEWMAIN();
		ret = StateSwitchMode(UI_VIDEO_STATE);
	}	
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_VMD)
{	
	//AHC_BOOL 	ret 			= AHC_TRUE;
#if (MOTION_DETECTION_EN)
	if(!m_ubMotionDtcEn)
		CaptureFunc_EnterVMDMode();
	else
		CaptureFunc_ExitVMDMode();
#endif	
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_TIMELAPSE)
{
	if(AHC_SDMMC_BasicCheck() == AHC_FALSE)
		return ;
	
	m_ulTimeLapseReady = AHC_TRUE;
	m_ubTimeLapseStart ^= 1;
	printc("EVENT_SWITCH_TIME_LAPSE_EN = %d\r\n", m_ubTimeLapseStart);
	
	if (m_ubTimeLapseStart) {
	CaptureCounter = 1;
	}
	
#if (TIMELAPSE_MODE & TIMELAPSE_VIDEO)
	CaptureFunc_TimeLapseVideo(m_ubTimeLapseStart);
#endif

}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_PANELTVOUT)
{
	AHC_SwitchLCDandTVOUT();
}

#if (SUPPORT_TOUCH_PANEL)
DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_TOUCHPAGE)
{
	m_ubShowDSCTouchPage ^= 1;	
	if(m_ubShowDSCTouchPage)
	{
		KeyParser_ResetTouchVariable();
		KeyParser_TouchItemRegister(&CaptureCtrlPage_TouchButton[0], ITEMNUM(CaptureCtrlPage_TouchButton));	
		DrawCaptureStatePageSwitch(TOUCH_CTRL_PAGE);
	}
	else
	{
		KeyParser_ResetTouchVariable();
		KeyParser_TouchItemRegister(&CaptureMainPage_TouchButton[0], ITEMNUM(CaptureMainPage_TouchButton));	
		DrawCaptureStatePageSwitch(TOUCH_MAIN_PAGE);
	}
}
#endif

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_CABLEIN)
{
#if (CHARGER_IN_ACT_OTHER_MODE==ACT_RECORD_VIDEO)
	DrawStateCaptureUpdate(EVENT_KEY_MODE);
	CaptureTimer_Stop();
	gsStillZoomIndex = ZoomCtrl_GetStillDigitalZoomLevel();
	ret = StateSwitchMode(UI_VIDEO_STATE);
	AHC_SetRecordByChargerIn(3);
#elif (CHARGER_IN_ACT_OTHER_MODE == ACT_FORCE_POWER_OFF)
	SetKeyPadEvent(KEY_POWER_OFF);
#endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_CABLEOUT)
{
	AHC_Charger_SetEnable(AHC_FALSE);
#if (CHARGER_OUT_ACT_OTHER_MODE==ACT_FORCE_POWER_OFF || CHARGER_OUT_ACT_OTHER_MODE==ACT_DELAY_POWER_OFF)
	AHC_SetShutdownByChargerOut(AHC_TRUE);
#endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_USB_DETECT)
{
	if(IsAdapter_Connect())
	{
	#if (CHARGER_IN_ACT_OTHER_MODE==ACT_RECORD_VIDEO)
		DrawStateCaptureUpdate(EVENT_KEY_MODE);
		CaptureTimer_Stop();
		gsStillZoomIndex = ZoomCtrl_GetStillDigitalZoomLevel();
		ret = StateSwitchMode(UI_VIDEO_STATE);
		AHC_SetRecordByChargerIn(3);
	#elif (CHARGER_IN_ACT_OTHER_MODE == ACT_FORCE_POWER_OFF)
		SetKeyPadEvent(KEY_POWER_OFF);
	#endif
	}
	else
	{
		CaptureTimer_Stop();
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

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_USB_REMOVED)
{
	if(AHC_USB_GetLastStatus() == AHC_USB_NORMAL) // MSDC mode
	{
		AHC_SetMode(AHC_MODE_IDLE);
		bForce_PowerOff = AHC_TRUE;
		AHC_PowerOff_NormalPath();
	}
	else
	{
		if (AHC_IsDcCableConnect() == AHC_TRUE)
			return;
	
		AHC_Charger_SetEnable(AHC_FALSE);
	#if (CHARGER_OUT_ACT_OTHER_MODE==ACT_FORCE_POWER_OFF || CHARGER_OUT_ACT_OTHER_MODE==ACT_DELAY_POWER_OFF)
		AHC_SetShutdownByChargerOut(AHC_TRUE);
	#endif
	}
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_SD_DETECT)
{
	AHC_RemountDevices(MenuSettingConfig()->uiMediaSelect);
	AHC_UF_SetFreeChar( 0,DCF_SET_FREECHAR, (UINT8 *) DSC_DEFAULT_FLIE_FREECHAR );
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_SD_REMOVED)
{
	if(AHC_TRUE == AHC_SDMMC_IsSD1MountDCF())
	{
		AHC_DisMountStorageMedia(AHC_MEDIA_MMC);
		Enable_SD_Power(0 /* Power Off */);
	}
}

#if (TVOUT_ENABLE)
DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_TV_DETECT)
{
#if (TVOUT_PREVIEW_EN==0)
	if(bShowTvWMSG) {
		bShowTvWMSG = AHC_FALSE;
		AHC_WMSG_Draw(AHC_TRUE, WMSG_HDMI_TV, 3);
	}
#else
	//if(AHC_IsTVConnectEx()) {
		//CaptureTimer_Stop();
		//SetTVState(AHC_TV_DSC_PREVIEW_STATUS);
		StateSwitchMode(UI_CAMERA_STATE);
	//}
#endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_TV_REMOVED)
{
    bShowTvWMSG = AHC_TRUE;

#if 1
    StateSwitchMode(UI_CAMERA_STATE);
#else
    //if( m_TVStatus )
	 //{
    AHC_OSDUninit();
    AHC_SetMode(AHC_MODE_IDLE);

    AHC_SetDisplayOutputDev(DISP_OUT_LCD, AHC_DISPLAY_DUPLICATE_1X);
    InitOSD();

    StateSwitchMode(UI_CAMERA_STATE);

    RTNA_LCD_Backlight(MMP_TRUE);
#endif    
	 //}
}
#endif

#if (HDMI_ENABLE)
DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_HDMI_DETECT)
{
#if (HDMI_PREVIEW_EN==0)
#ifdef CFG_HDMI_AS_JPGPB //may be defined in config_xxx.h
		CaptureTimer_Stop();
		SetCurrentOpMode(JPGPB_MODE);
		m_ubDSCMode = DSC_ONE_SHOT;
		//if(m_ubTimeLapseStart)
			//StateCaptureLCDMode(EVENT_SWITCH_TIME_LAPSE_EN);
		AHC_DisplayOff();
		//StateSwitchMode(UI_HDMI_STATE);
#else
		if(bShowHdmiWMSG) {
			bShowHdmiWMSG = AHC_FALSE;
			AHC_WMSG_Draw(AHC_TRUE, WMSG_HDMI_TV, 3);
		}
#endif
#else
//	CaptureTimer_Stop();

//	SetHDMIState(AHC_HDMI_DSC_PREVIEW_STATUS);
	StateSwitchMode(UI_CAMERA_STATE);
#endif
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_HDMI_REMOVED)
{
	bShowHdmiWMSG = AHC_TRUE;
    StateSwitchMode(UI_CAMERA_STATE);
}
#endif

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_LCD_COVER_OPEN)
{
	LedCtrl_LcdBackLight(AHC_TRUE);
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_LCD_COVER_CLOSE)
{
	LedCtrl_LcdBackLight(AHC_FALSE);
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_LCD_COVER_NORMAL)
{
	AHC_DrawRotateEvent(ulEvent);
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_LCD_COVER_ROTATE)
{
	AHC_DrawRotateEvent(ulEvent);
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_VRCB_MOTION_DTC)
{
#if (MOTION_DETECTION_EN) && (VMD_ACTION & VMD_BURST_CAPTURE)
if(m_ubVMDStart)
{
	if(bBurstShotComplete)
	{
		printc("DSC SomeBody/SomeThing is Moved!!!\r\n");
		bShotByVMD = AHC_TRUE;
		AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, KEY_DSC_CAPTURE, 0);
	}
}
#endif	
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_DSC_KEY_UPDATE_MESSAGE)
{
    if(AHC_TRUE == gbCameraInSubMode){ //Refresh OSD by sub mode.
        return;
    }

#if (DSC_DIGIT_ZOOM_ENABLE)        
#ifdef PREVIEW_ZOOM_STEP_BY_STEP	//cardv
    if(!AHC_GetCaptureCurZoomStatus()){
    	if(!bZoomStop){
    		CaptureFunc_ZoomOperation(AHC_SENSOR_ZOOM_STOP);
    		bZoomStop = AHC_TRUE;
    		bZooming = AHC_FALSE;
    	}
    }
#endif
#endif
    if (!bContiShotComplete) {
    	MMP_ULONG ulStreamBusy = 1;
    	MMPS_DSC_CheckContiShotStreamBusy(&ulStreamBusy);

    	if (ulStreamBusy == 0) {
    		printc(">>>RESTORE DSC PREIVEW\r\n");
    		AHC_SetMode(AHC_MODE_CAPTURE_PREVIEW);
    		bContiShotComplete = AHC_TRUE;
    	}
    }

#if (MOTION_DETECTION_EN)
    if(m_ubInRemindTime && m_ulVMDRemindTime==0)
    {
    	DrawStateCaptureUpdate(ulEvent);
    	m_ubInRemindTime = AHC_FALSE;
    	return ;
    }
#endif

#if (DSC_SELFSHOT_EN)
    if(InSelfTime)
    {
    UINT8 LCDstatus;

    AHC_LCD_GetStatus(&LCDstatus);

    if(ubBackupLCDStatus != LCDstatus)
    {
    	ubBackupLCDStatus = LCDstatus;

    	if(LCDstatus == AHC_LCD_NORMAL)
    	{
    		AHC_SetKitDirection(AHC_LCD_NORMAL, AHC_TRUE, AHC_SNR_NOFLIP, AHC_FALSE);
    		DrawStateCaptureUpdate(EVENT_LCD_COVER_NORMAL);
    	}
    	else
    	{
    		AHC_SetKitDirection(AHC_LCD_REVERSE, AHC_TRUE, AHC_SNR_NOFLIP, AHC_FALSE);
    		DrawStateCaptureUpdate(EVENT_LCD_COVER_ROTATE);
    	}

    	DrawStateCaptureUpdate(EVENT_DSC_KEY_FOCUS);
    }

    	if(SelfTime == 0)
    	{
    		//MenuSettingConfig()->uiSelfTimer = SELF_TIMER_OFF;
    		InSelfTime = AHC_FALSE;
    		DrawStateCaptureUpdate(ulEvent);

    		DrawStateCaptureUpdate(EVENT_DSC_KEY_FOCUS_STATUS_CLEAR);

    		CaptureFunc_Shutter();

    		CaptureFunc_CheckMemSizeAvailable(&ulAvailbleSize, &RemainCaptureNum);
    		DrawStateCaptureUpdate(EVENT_DSC_KEY_CAPTURE_STATUS_CLEAR);
    	}
    	else
    	{
    		DrawStateCaptureUpdate(ulEvent);
    	}
    }
    else
#endif
    	{
    		DrawStateCaptureUpdate(ulEvent);
    	}	
}

#if (UVC_HOST_VIDEO_ENABLE)
DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_USB_B_DEVICE_DETECT)
{
	AHC_USB_PauseDetection(0);
	if((AHC_GetCurKeyEventID() == BUTTON_USB_B_DEVICE_DETECTED) ||
		(AHC_GetCurKeyEventID() == BUTTON_USB_B_DEVICE_REMOVED)){
		AHC_SetCurKeyEventID(EVENT_NONE);
	}
	else{
		AHC_PRINT_RET_ERROR(0, 0);
		printc("KeyEventID: BUTTON_USB_B_DEVICE_DETECTED,REMOVED is interrupted.\r\n");
	} 
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_USB_B_DEVICE_REMOVED)
{
	AHC_USB_PauseDetection(0);
	if((AHC_GetCurKeyEventID() == BUTTON_USB_B_DEVICE_DETECTED) ||
		(AHC_GetCurKeyEventID() == BUTTON_USB_B_DEVICE_REMOVED)){
		AHC_SetCurKeyEventID(EVENT_NONE);
	}
	else{
		AHC_PRINT_RET_ERROR(0, 0);
		printc("KeyEventID: BUTTON_USB_B_DEVICE_DETECTED,REMOVED is interrupted.\r\n");
	} 
}
#endif

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_SUB_MODE_ENTER)
{

    gbCameraInSubMode = AHC_TRUE;
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_SUB_MODE_EXIT)
{
    gbCameraInSubMode = AHC_FALSE;
    //Redraw OSD
    DrawStateCaptureUpdate(EVENT_LCD_COVER_NORMAL);//CaptureMode_PreviewUpdate();
}

DECL_AHC_EV_HANDLER(STATE_CAMERA_MODE_EVENT_ENTER_NET_PLAYBACK)
{
	AHC_SetFastAeAwbMode( AHC_FALSE );

	DrawStateCaptureUpdate(ulEvent);
	CaptureTimer_Stop();
	gsStillZoomIndex = ZoomCtrl_GetStillDigitalZoomLevel();

	StateSwitchMode(UI_NET_PLAYBACK_STATE);
    CGI_SET_STATUS(ulEvent, CGI_ERR_NONE /* SUCCESSFULLY */);
}

AHC_BOOL StateSelectFuncCameraMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
#if 0
#ifdef CFG_MENU_CAMERA_ALWAYS_ENTER_CAPTURE_MODE //may be defined in config_xxx.h
    // NOP
#else
    if(AHC_TRUE == StateIsInMenuMode()){
        StateModeSetOperation(UI_CAMERA_STATE, (void*)StateCaptureModeInitFromMenu, (void*)StateCaptureModeShutDown, (void *)StateCaptureModeHnadler);
    }
    else
#endif
#endif
    {
#if DSC_DIGIT_ZOOM_ENABLE
        RegisterEventCb(EVENT_KEY_TELE_PRESS, 				STATE_CAMERA_MODE_EVENT_KEY_TELEPRESS);
        RegisterEventCb(EVENT_KEY_WIDE_PRESS, 				STATE_CAMERA_MODE_EVENT_KEY_WIDEPRESS);
        RegisterEventCb(EVENT_KEY_TELE_STOP, 				STATE_CAMERA_MODE_EVENT_KEY_TELESTOP);
        RegisterEventCb(EVENT_KEY_WIDE_STOP, 				STATE_CAMERA_MODE_EVENT_KEY_WIDESTOP);
#endif
        RegisterEventCb(EVENT_KEY_UP, 						STATE_CAMERA_MODE_EVENT_KEY_UP);
        RegisterEventCb(EVENT_KEY_DOWN, 					STATE_CAMERA_MODE_EVENT_KEY_DOWN);
        RegisterEventCb(EVENT_KEY_LEFT, 					STATE_CAMERA_MODE_EVENT_KEY_LEFT);
        RegisterEventCb(EVENT_KEY_RIGHT, 					STATE_CAMERA_MODE_EVENT_KEY_RIGHT);
        RegisterEventCb(EVENT_KEY_MENU, 					STATE_CAMERA_MODE_EVENT_KEY_MENU);
        RegisterEventCb(EVENT_DSC_KEY_CHANGE_SHOT_MODE, 	STATE_CAMERA_MODE_EVENT_KEY_CHANGE_SHOT_MODE);
        RegisterEventCb(EVENT_KEY_MODE, 					STATE_CAMERA_MODE_EVENT_KEY_MODE);
        RegisterEventCb(EVENT_DSC_KEY_CAPTURE, 				STATE_CAMERA_MODE_EVENT_KEY_CAPTURE);
        RegisterEventCb(EVENT_KEY_PLAYBACK_MODE, 			STATE_CAMERA_MODE_EVENT_KEY_PLAYBACKMODE);
        RegisterEventCb(EVENT_POWER_OFF, 					STATE_CAMERA_MODE_EVENT_POWEROFF);
        RegisterEventCb(EVENT_CHANGE_LED_MODE, 				STATE_CAMERA_MODE_EVENT_CHANGELED);
        RegisterEventCb(EVENT_LCD_POWER_SAVE, 				STATE_CAMERA_MODE_EVENT_POWERSAVE);
        RegisterEventCb(EVENT_CHANGE_NIGHT_MODE, 			STATE_CAMERA_MODE_EVENT_NIGHTMODE);
        RegisterEventCb(EVENT_CAMERA_BROWSER, 				STATE_CAMERA_MODE_EVENT_CAMERABROWSER);
        RegisterEventCb(EVENT_VIDEO_BROWSER, 				STATE_CAMERA_MODE_EVENT_VIDEOBROWSER);
        RegisterEventCb(EVENT_ALL_BROWSER, 					STATE_CAMERA_MODE_EVENT_ALLBROWSER);
        RegisterEventCb(EVENT_VIDEO_PREVIEW, 				STATE_CAMERA_MODE_EVENT_PREVIEW);
        RegisterEventCb(EVENT_SWITCH_VMD_MODE, 				STATE_CAMERA_MODE_EVENT_VMD);
        RegisterEventCb(EVENT_SWITCH_TIME_LAPSE_EN, 		STATE_CAMERA_MODE_EVENT_TIMELAPSE);
        RegisterEventCb(EVENT_SWITCH_PANEL_TVOUT, 			STATE_CAMERA_MODE_EVENT_PANELTVOUT);
#if (SUPPORT_TOUCH_PANEL)
        RegisterEventCb(EVENT_SWITCH_TOUCH_PAGE, 			STATE_CAMERA_MODE_EVENT_TOUCHPAGE);
#endif

        RegisterEventCb(EVENT_DC_CABLE_IN,					STATE_CAMERA_MODE_EVENT_CABLEIN);
        RegisterEventCb(EVENT_DC_CABLE_OUT,					STATE_CAMERA_MODE_EVENT_CABLEOUT);
        RegisterEventCb(EVENT_USB_DETECT,  					STATE_CAMERA_MODE_EVENT_USB_DETECT);
        RegisterEventCb(EVENT_USB_REMOVED,					STATE_CAMERA_MODE_EVENT_USB_REMOVED);
        RegisterEventCb(EVENT_SD_DETECT, 					STATE_CAMERA_MODE_EVENT_SD_DETECT);
        RegisterEventCb(EVENT_SD_REMOVED,                   STATE_CAMERA_MODE_EVENT_SD_REMOVED);
#if (TVOUT_ENABLE)
        RegisterEventCb(EVENT_TV_DETECT,					STATE_CAMERA_MODE_EVENT_TV_DETECT);
        RegisterEventCb(EVENT_TV_REMOVED,					STATE_CAMERA_MODE_EVENT_TV_REMOVED);
#endif
#if (HDMI_ENABLE)
        RegisterEventCb(EVENT_HDMI_DETECT, 					STATE_CAMERA_MODE_EVENT_HDMI_DETECT);
        RegisterEventCb(EVENT_HDMI_REMOVED,  				STATE_CAMERA_MODE_EVENT_HDMI_REMOVED);
#endif
        RegisterEventCb(EVENT_LCD_COVER_OPEN,				STATE_CAMERA_MODE_EVENT_LCD_COVER_OPEN);
        RegisterEventCb(EVENT_LCD_COVER_CLOSE,				STATE_CAMERA_MODE_EVENT_LCD_COVER_CLOSE);
        RegisterEventCb(EVENT_LCD_COVER_NORMAL,				STATE_CAMERA_MODE_EVENT_LCD_COVER_NORMAL);
        RegisterEventCb(EVENT_LCD_COVER_ROTATE,				STATE_CAMERA_MODE_EVENT_LCD_COVER_ROTATE);
        RegisterEventCb(EVENT_VRCB_MOTION_DTC,				STATE_CAMERA_MODE_EVENT_VRCB_MOTION_DTC);
        RegisterEventCb(EVENT_DSC_KEY_UPDATE_MESSAGE,		STATE_CAMERA_MODE_EVENT_DSC_KEY_UPDATE_MESSAGE);
#if (UVC_HOST_VIDEO_ENABLE)
        RegisterEventCb(EVENT_USB_B_DEVICE_DETECT,			STATE_CAMERA_MODE_EVENT_USB_B_DEVICE_DETECT);
        RegisterEventCb(EVENT_USB_B_DEVICE_REMOVED,			STATE_CAMERA_MODE_EVENT_USB_B_DEVICE_REMOVED);
#endif
        RegisterEventCb(EVNET_SUB_MODE_ENTER,				STATE_CAMERA_MODE_EVENT_SUB_MODE_ENTER);
        RegisterEventCb(EVNET_SUB_MODE_EXIT,				STATE_CAMERA_MODE_EVENT_SUB_MODE_EXIT);

#if defined(WIFI_PORT) && (WIFI_PORT == 1)
        RegisterEventCb(EVENT_NET_ENTER_PLAYBACK,			STATE_CAMERA_MODE_EVENT_ENTER_NET_PLAYBACK);
#endif

#if (HDMI_ENABLE)
        if (AHC_IsHdmiConnect()){
            //Register HDMI specific functions here!
            StateModeSetOperation(UI_CAMERA_STATE, StateCaptureModeInitHDMI, StateCaptureModeShutDown, StateCaptureModeHnadler);
        }else
#endif
#if (TVOUT_ENABLE)
        if (AHC_IsTVConnectEx()){
            StateModeSetOperation(UI_CAMERA_STATE, StateCaptureModeInitTV, StateCaptureModeShutDown, StateCaptureModeHnadler);

        }else
#endif
        {
            //StateHDMI_EventMenu_Handler(ulJobEvent);
            StateModeSetOperation(UI_CAMERA_STATE, StateCaptureModeInitLCD, StateCaptureModeShutDown, StateCaptureModeHnadler);
        }

    }

    ahcRet =  SwitchUIDrawSetCameraMode();
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return ahcRet;}

    return ahcRet;
}

#endif //#if (DSC_MODE_ENABLE)




