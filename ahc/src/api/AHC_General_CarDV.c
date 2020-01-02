/**
 @file AHC_Genera_CarDV.c
 @brief AHC general control Function for CarDV project
 @author
 @version 1.0
*/

/*===========================================================================
 * Include file
 *===========================================================================*/

#include "customer_config.h"
#include "AHC_config_SDK.h"
#include "AHC_Display.h"

#include "mmp_lib.h"
#include "mmp_err.h"
#include "ait_utility.h"
#include "mmps_i2cm.h"
#include "mmps_uart.h"
#include "mmps_rtc.h"
#include "mmps_pio.h"
#include "mmps_timer.h"
#include "mmps_sensor.h"
#include "mmps_usb.h"
#include "mmps_dsc.h"
#include "mmps_3gprecd.h"
#include "mmps_audio.h"
#include "mmps_aui.h"
#include "mmps_pwm.h"
#include "mmpd_system.h"
#include "mmps_vidplay.h"
#include "mmpf_sf.h"
#include "mmpf_vif.h"
#include "mmpf_timer.h"
#include "mmpf_fs_api.h"
#include "mmpf_pio.h"
#include "mmpf_hw_counter.h"
#include "lib_retina.h"

#include "AHC_OS.h"
#include "AHC_FS.h"
#include "AHC_General.h"
#include "AHC_Parameter.h"
#include "AHC_Message.h"
#include "AHC_Menu.h"
#include "AHC_Parameter.h"
#include "AHC_GUI.h"
#include "AHC_Video.h"
#include "AHC_Isp.h"
#include "AHC_Sensor.h"
#include "AHC_Version.h"
#include "AHC_Media.h"
#include "AHC_Utility.h"
#include "AHC_Warningmsg.h"
#include "AHC_General_CarDV.h"
#include "AIHC_DCF.h"
#include "AHC_UF.h"
#include "AIHC_Browser.h"
#include "AHC_Capture.h"
#include "AIHC_GUI.h"
#include "AHC_USB.h"
#include "AHC_GSensor.h"

#include "Keyparser.h"
#include "StateHDMIFunc.h"
#include "StateTVFunc.h"
#include "StateVideoFunc.h"
#include "StateCameraFunc.h"
#include "ZoomControl.h"
#include "LedControl.h"
#include "GSensor_ctrl.h"
#include "MenuSetting.h"
#include "MenuDrawCommon.h"
#include "MediaPlaybackCtrl.h"
#include "ShowOSDFunc.h"
#include "OsdStrings.h"
#ifdef _OEM_MENU_
#include "Oem_Menu.h"
#endif

#if defined(WIFI_PORT) && (WIFI_PORT==1)
#include "netapp.h"
#endif


//long 4-23
#include "fm_i2c.h"

/*===========================================================================
 * Macro define
 *===========================================================================*/

//#define DBG_DUMP_SF_LAST_SECTOR_DATA
//#define BACKUP_SF_LAST_SECTOR_DATA     // TBD

/*===========================================================================
 * Type define
 *===========================================================================*/

typedef	void (DRAW_EVENT)(UINT32 Event);
typedef	void (DRAW_MENUEVENT)(PSMENUSTRUCT pMenu);

/*===========================================================================
 * Global variable
 *===========================================================================*/

static UINT32		m_ulVRPowerOffTime 			= 0;	//Unit:seconds
static UINT8		m_ubBootwithCharger 		= 0;
static AHC_BOOL		m_ubShutdownByChargerOut 	= AHC_FALSE;
static UINT8 		m_ubDialogState 			= MSG_DISMISS;
static UINT8 		m_ubDialogEvent				= WMSG_NONE;
static UINT8		m_ubMenuFlashLed;
AHC_BOOL 			m_ubInTVMode 				= AHC_FALSE;
AHC_BOOL 			bUpdateFlashLedIcon			= AHC_FALSE;
TMHOOK 				gTmKeypadHook 				= NULL;
AHC_BOOL			bInLCDPowerSave				= AHC_FALSE;

#if (POWER_ON_BUTTON_ACTION)
static UINT32		m_ulQuickKey   				= 0;
static UINT16		m_usOption 					= 1;
static AHC_BOOL 	PowerOnAction 				= AHC_FALSE;
#endif

#if (SWITCH_TVOUT_BY_KEY)
AHC_BOOL			m_ubGoToTVmode				= AHC_TRUE;
#endif
#if (defined(SWITCH_TVOUT_BY_MENU) && SWITCH_TVOUT_BY_MENU)
AHC_BOOL			m_ubGoToTVmode				= AHC_FALSE;
#endif
#if(SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
UINT32	GSNRPowerOnThd=0;
#endif

UINT8				m_ubSensorStatus			= AHC_SNR_NORMAL;
static UINT8		m_ubRotateSrc				= SRC_MAX;
DRAW_EVENT			*DrawEventHandler			= NULL;
DRAW_MENUEVENT		*DrawMenuEventHandler		= NULL;

static OSD_LANGSET gLangSet[] =
{
#if (MENU_GENERAL_LANGUAGE_ENGLISH_EN)
	{(const unsigned char **)gEnglishOSDStringPool,		&LANGUAGE_FONT_ENGLISH},
#endif
#if (MENU_GENERAL_LANGUAGE_SPANISH_EN)
	{(const unsigned char **)gSpanishOSDStringPool,		&LANGUAGE_FONT_SPANISH},
#endif
#if (MENU_GENERAL_LANGUAGE_PORTUGUESE_EN)
	{(const unsigned char **)gPortugueseOSDStringPool,	&LANGUAGE_FONT_PORTUGUESE},
#endif
#if (MENU_GENERAL_LANGUAGE_RUSSIAN_EN)
	{(const unsigned char **)gRussianOSDStringPool,		&LANGUAGE_FONT_RUSSIAN},
#endif
#if (MENU_GENERAL_LANGUAGE_SCHINESE_EN)
	{(const unsigned char **)gSChineseOSDStringPool,	&LANGUAGE_FONT_SCHINESE},
	#endif	
#if (MENU_GENERAL_LANGUAGE_TCHINESE_EN)
	{(const unsigned char **)gTChineseOSDStringPool,	&LANGUAGE_FONT_TCHINESE},
#endif
#if (MENU_GENERAL_LANGUAGE_GERMAN_EN)
	{(const unsigned char **)gGermanOSDStringPool,		&LANGUAGE_FONT_GERMAN},
#endif
#if (MENU_GENERAL_LANGUAGE_ITALIAN_EN)
	{(const unsigned char **)gItalianOSDStringPool,		&LANGUAGE_FONT_ITALIAN},
#endif
#if (MENU_GENERAL_LANGUAGE_LATVIAN_EN)
	{(const unsigned char **)gLatvianOSDStringPool,		&LANGUAGE_FONT_LATVIAN},
#endif
#if (MENU_GENERAL_LANGUAGE_POLISH_EN)
	{(const unsigned char **)gPolishOSDStringPool,		&LANGUAGE_FONT_POLISH},
#endif
#if (MENU_GENERAL_LANGUAGE_ROMANIAN_EN)
	{(const unsigned char **)gRomanianOSDStringPool,	&LANGUAGE_FONT_ROMANIAN},
#endif
#if (MENU_GENERAL_LANGUAGE_SLOVAK_EN)
	{(const unsigned char **)gSlovakOSDStringPool,		&LANGUAGE_FONT_SLOVAK},
#endif
#if (MENU_GENERAL_LANGUAGE_UKRANINIAN_EN)
	{(const unsigned char **)gUkraninianOSDStringPool,	&LANGUAGE_FONT_UKRANINIAN},
#endif
#if (MENU_GENERAL_LANGUAGE_FRENCH_EN)
	{(const unsigned char **)gFrenchOSDStringPool,		&LANGUAGE_FONT_FRENCH},
#endif
#if (MENU_GENERAL_LANGUAGE_JAPANESE_EN)
	{(const unsigned char **)gJapaneseOSDStringPool,	&LANGUAGE_FONT_JAPANESE},
#endif
#if (MENU_GENERAL_LANGUAGE_KOREAN_EN)
	{(const unsigned char **)gKoreanOSDStringPool,		&LANGUAGE_FONT_KOREAN},
#endif
#if (MENU_GENERAL_LANGUAGE_CZECH_EN)
	{(const unsigned char **)gCzechOSDStringPool,		&LANGUAGE_FONT_CZECH},
#endif
#if (MENU_GENERAL_LANGUAGE_TURKISH_EN)
	{(const unsigned char **)gTurkishOSDStringPool,		&LANGUAGE_FONT_TURKISH},
#endif
#if (MENU_GENERAL_LANGUAGE_DUTCH_EN)
	{(const unsigned char **)gDutchOSDStringPool,		&LANGUAGE_FONT_DUTCH},
#endif
#if (MENU_GENERAL_LANGUAGE_DANISH_EN)
	{(const unsigned char **)gDanishOSDStringPool,		&LANGUAGE_FONT_DANISH},
#endif
#if (MENU_GENERAL_LANGUAGE_GREEK_EN)
	{(const unsigned char **)gGreekOSDStringPool,		&LANGUAGE_FONT_GREEK},
#endif
#if (MENU_GENERAL_LANGUAGE_ARABIC_EN)
	{(const unsigned char **)gArabicOSDStringPool,		&LANGUAGE_FONT_ARABIC},
#endif
#if (MENU_GENERAL_LANGUAGE_THAI_EN)
	{(const unsigned char **)gThaiOSDStringPool,		&LANGUAGE_FONT_THAI},
#endif
};

/*===========================================================================
 * Extern variable
 *===========================================================================*/

extern UINT32   	ObjSelect;
extern AHC_PARAM 	glAhcParameter;
extern AHC_BOOL 	PowerOff_InProcess;
extern AHC_BOOL 	Protecting_InBrowser;
extern AHC_BOOL 	Deleting_InBrowser;
extern AHC_BOOL		Delete_File_Confirm;
extern AHC_BOOL		Protect_File_Confirm;
extern AHC_BOOL		UnProtect_File_Confirm;

#if(SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
extern AHC_BOOL	    ubGsnrPwrOnActStart;
extern UINT32	    m_ulGSNRRecStopCnt;
extern AHC_BOOL     ubGsnrPwrOnFirstREC;
#endif

#if (MOTION_DETECTION_EN)
extern UINT32		m_ulVMDRemindTime;
extern UINT32		m_ulVMDCloseBacklightTime;
extern AHC_BOOL		m_ubInRemindTime;
extern AHC_BOOL		m_ulVMDCancel;
#endif

#if (VR_PREENCODE_EN)
extern AHC_BOOL 	m_ubPreRecording;
extern AHC_BOOL		m_ubPreEncodeEn;
#endif

extern AHC_BOOL		bDisableVideoPreRecord;
#if (SUPPORT_GSENSOR)
extern int			dumpXYZ;
#endif

extern MMP_USHORT   gsAhcPrmSensor;

/*===========================================================================
 * Extern function
 *===========================================================================*/

extern int  sprintf(char * /*s*/, const char * /*format*/, ...);
extern int  Getpf_GSNR_THR(void);

/* UI Drawing Function */
extern void DrawStateVideoRecUpdate(UINT32 ubEvent);
extern void DrawStateCaptureUpdate(UINT32 ubEvent);
extern void DrawStateLCDBrowserUpdate(UINT32 ubEvent);
extern void DrawStateMovPlaybackUpdate(UINT32 ubEvent);
extern void DrawStateAudPlaybackUpdate(UINT32 ubEvent);
extern void DrawStatePhotoPb_handler_Lcd(UINT32 ubEvent);
extern void DrawStateSlideShowUpdate(UINT32 ubEvent);

/* Menu Drawing Function */
extern void MenuDrawMainPage(PSMENUSTRUCT pMenu);
extern void MenuDrawMainPage_ModeSelect(PSMENUSTRUCT pMenu);
extern void MenuDrawTopMenuPage(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_FlashLevel(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_ClockSetting(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_FwVersionInfo(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_GPSInfo(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_EV(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_StorageInfo(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_Volume(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_EditOneFile(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_PowerOff(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_ConfirmPage(PSMENUSTRUCT pMenu);
extern void MenuDrawSubPage_DriverIdSetting(PSMENUSTRUCT pMenu);

extern AHC_BOOL CaptureFunc_EnterVMDMode(void);
extern AHC_BOOL CaptureFunc_ExitVMDMode(void);
extern AHC_BOOL VideoFunc_EnterVMDMode(void);
extern AHC_BOOL VideoFunc_ExitVMDMode(void);
extern AHC_BOOL VideoFunc_LockFileEnabled(void);

extern void     MMPF_MMU_FlushDCache(void);
extern AHC_BOOL AHC_VIDEO_SetRecVolumeByMenuSetting(AHC_BOOL bEnable);

/*===========================================================================
 * Main body
 *===========================================================================*/

#if 0
void ____Language_Function____(){ruturn;} //dummy
#endif

#define	NUM_LANGUAGE	(sizeof(gLangSet) / sizeof(OSD_LANGSET))

extern const unsigned char **pShowOSDStringPool;

UINT8 ShowOSD_GetLanguageEx()
{
	return MenuSettingConfig()->uiLanguage;
}

void ShowOSD_SetLanguageEx(UINT8 id, UINT8 nlang)
{
	if (nlang >= NUM_LANGUAGE)
		nlang = SHOWOSD_LANG_ENG;

	pShowOSDStringPool = gLangSet[nlang].strpool;
	if (id != 0xff) {
		AHC_OSDSetFont(id, gLangSet[nlang].fontmap);
	}
}

#if 0
void ____Timer_Hook_Function_____(){ruturn;} //dummy
#endif

void AHC_TmKeypadHook(void *cb)
{
	gTmKeypadHook = (TMHOOK)cb;
}

#if 0
void ____RTC_TimeStamp_Function_____(){ruturn;} //dummy
#endif

#define	AHC_SYSTEM_TIMER	MMPF_TIMER_4

UINT8 sm[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void AIHC_AdvanceTime(AHC_RTC_TIME *pt)
{
	pt->uwSecond++;
	if (pt->uwSecond < 60)
		return;

	pt->uwSecond = 0;
	pt->uwMinute++;
	if (pt->uwMinute < 60)
		return;

	pt->uwMinute = 0;
	pt->uwHour++;
	if (pt->uwHour < 24)
		return;

	pt->uwHour = 0;
	pt->uwDay++;
	if (pt->uwDay <= sm[pt->uwMonth-1])
		return;

	if (pt->uwMonth == 2) {
		if (pt->uwDay == 29) {
			if (pt->uwYear % 4 == 0) {
				if (pt->uwYear % 100 == 0) {
					if (pt->uwYear % 400 == 0) {
						// leap year
						return;
					}
				} else {
					// leap year
					return;
				}
			}
		}
	}

	pt->uwDay = 1;
	pt->uwMonth++;
	if (pt->uwMonth < 13)
		return;

	pt->uwMonth = 1;
	pt->uwYear++;
}

AHC_BOOL AHC_Validate_Year(UINT16 year)
{
	#if (RTC_CHECK)
	return ((year >= RTC_DEFAULT_YEAR) && (year <= RTC_MAX_YEAR));
	#else
	return (year >= RTC_DEFAULT_YEAR);
	#endif
}

UINT8 AHC_Validate_ClockSetting(INT32 *pDatetime, UINT8 ubCheckType, AHC_BOOL bAutoRestore)
{
	INT32 Year,Month,Day,Hour,Min,Sec;

	Year  = *(pDatetime+IDX_YEAR);
	Month = *(pDatetime+IDX_MONTH);
	Day   = *(pDatetime+IDX_DAY);
	Hour  = *(pDatetime+IDX_HOUR);
	Min   = *(pDatetime+IDX_MIN);
	Sec   = *(pDatetime+IDX_SEC);

	//Check Year
	if(ubCheckType & CHECK_YEAR)
	{
		if(Year > RTC_MAX_YEAR)
		{
			if(bAutoRestore)
				*(pDatetime+IDX_YEAR) = RTC_MIN_YEAR;
			else
				return IDX_YEAR;
		}
		else if(Year < RTC_MIN_YEAR)
		{
			if(bAutoRestore)
				*(pDatetime+IDX_YEAR) = RTC_MAX_YEAR;
	 		else
	 			return IDX_YEAR;
	 	}

	    if( ((Year %4 == 0) && (Year %100 != 0)) || ( Year %400 == 0))//Leap Year
		{
			if( Month == 2 && Day == 28 )
			{
				if(bAutoRestore)
					*(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_LEAP_YEAR;
	            else
	            	return IDX_DAY;
	        }
		}
		else
		{
			if( Month == 2 && Day == 29 )
			{
				if(bAutoRestore)
					*(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_NONLEAP_YEAR;
	            else
	            	return IDX_DAY;
	        }
		}
	}

	//Check Month
	if(ubCheckType & CHECK_MONTH)
	{
		if(Month > RTC_MAX_MONTH)
		{
			if(bAutoRestore)
				*(pDatetime+IDX_MONTH) = RTC_MIN_MONTH;
			else
				return IDX_MONTH;
		}
		else if(Month < RTC_MIN_MONTH)
		{
			if(bAutoRestore)
				*(pDatetime+IDX_MONTH) = RTC_MAX_MONTH;
			else
				return IDX_MONTH;
		}

		if(Month == 2)
		{
			if( ((Year %4 == 0) && (Year %100 != 0)) || ( Year %400 == 0))//Leap Year
			{
				if(Day == 30 || Day == 31)
				{
					if(bAutoRestore)
						*(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_LEAP_YEAR;
					else
						return IDX_DAY;
				}
			}
			else
			{
				if(Day == 29 || Day == 30 || Day == 31)
				{
					if(bAutoRestore)
						*(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_NONLEAP_YEAR;
					else
						return IDX_DAY;
				}
			}
		}
		else if(Month == 4 || Month == 6 || Month == 9 || Month == 11)
		{
			if(Day == 31)
			{
				if(bAutoRestore)
					*(pDatetime+IDX_DAY) = RTC_MAX_DAY_30;
				else
					return IDX_DAY;
			}
		}
	}

	//Check Day
	if(ubCheckType & CHECK_DAY)
	{
		if(Month==1  || Month==3 || Month==5 || Month==7 || Month==8 || Month==10 || Month==12)
		{
			if(Day >RTC_MAX_DAY_31)
			{
				if(bAutoRestore)
					*(pDatetime+IDX_DAY) = RTC_MIN_DAY;
				else
					return IDX_DAY;

			}
			else if(Day < RTC_MIN_DAY)
			{
				if(bAutoRestore)
					*(pDatetime+IDX_DAY) = RTC_MAX_DAY_31;
				else
					return IDX_DAY;
			}
		}

		if(Month==4 || Month==6 || Month==9 || Month==11)
		{
			if(Day >RTC_MAX_DAY_30)
			{
				if(bAutoRestore)
					*(pDatetime+IDX_DAY) = RTC_MIN_DAY;
				else
					return IDX_DAY;
	        }
			else if(Day < RTC_MIN_DAY)
			{
				if(bAutoRestore)
					*(pDatetime+IDX_DAY) = RTC_MAX_DAY_30;
				else
					return IDX_DAY;
			}
		}

		if(Month==2)
		{
			if( ((Year %4 == 0) && (Year %100 != 0)) || ( Year %400 == 0))//Leap Year
			{
				if(Day >RTC_MAX_DAY_FEB_LEAP_YEAR)
				{
					if(bAutoRestore)
						*(pDatetime+IDX_DAY) = RTC_MIN_DAY;
					else
						return IDX_DAY;
				}
				else if(Day < RTC_MIN_DAY)
				{
					if(bAutoRestore)
						*(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_LEAP_YEAR;
					else
						return IDX_DAY;
				}
			}
			else
			{
				if(Day >RTC_MAX_DAY_FEB_NONLEAP_YEAR)
				{
					if(bAutoRestore)
						*(pDatetime+IDX_DAY) = RTC_MIN_DAY;
					else
						return IDX_DAY;
				}
				else if(Day < RTC_MIN_DAY)
				{
					if(bAutoRestore)
						*(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_NONLEAP_YEAR;
					else
						return IDX_DAY;
				}
			}
		}
	}

	//Check Hour
	if(ubCheckType & CHECK_HOUR)
	{
		if(Hour > RTC_MAX_HOUR)
		{
			if(bAutoRestore)
				*(pDatetime+IDX_HOUR) = RTC_MIN_HOUR;
			else
				return IDX_HOUR;

		}
		else if(Hour < RTC_MIN_HOUR)
		{
			if(bAutoRestore)
				*(pDatetime+IDX_HOUR) = RTC_MAX_HOUR;
			else
				return IDX_HOUR;
		}
	}

	//Check Minute
	if(ubCheckType & CHECK_MIN)
	{
		if(Min > RTC_MAX_MIN)
		{
			if(bAutoRestore)
				*(pDatetime+IDX_MIN) = RTC_MIN_MIN;
			else
				return IDX_MIN;
		}
		else if(Min < RTC_MIN_MIN)
		{
			if(bAutoRestore)
				*(pDatetime+IDX_MIN) = RTC_MAX_MIN;
			else
				return IDX_MIN;
		}
	}

	//Check Second
	if(ubCheckType & CHECK_SEC)
	{
		if(Sec > RTC_MAX_SEC)
		{
			if(bAutoRestore)
				*(pDatetime+IDX_SEC) = RTC_MIN_SEC;
			else
				return IDX_SEC;
		}
		else if(Sec < RTC_MIN_SEC)
		{
			if(bAutoRestore)
				*(pDatetime+IDX_SEC) = RTC_MAX_SEC;
			else
				return IDX_SEC;
		}
	}

	return CHECK_PASS;
}

AHC_BOOL AHC_RestoreDefaultTime(UINT16 *py, UINT16 *pm, UINT16 *pd, UINT16 *pd_in_week,UINT16 *ph, UINT16 *pmm, UINT16 *ps, UINT8 *pam_pm, UINT8 *p12format_en)
{
    AHC_SetParam(PARAM_ID_USE_RTC,1);

    *py  	= RTC_DEFAULT_YEAR;
    *pm 	= RTC_DEFAULT_MONTH;
    *pd   	= RTC_DEFAULT_DAY;
    *ph  	= RTC_DEFAULT_HOUR;
    *pmm	= RTC_DEFAULT_MIN;
    *ps		= RTC_DEFAULT_SEC;

    *pd_in_week = 0;
    *pam_pm = 0;
    *p12format_en = 0;

    return AHC_TRUE;
}

#if 0
void ____Video_Record_Function_____(){ruturn;} //dummy
#endif

UINT32 AHC_GetVideoPowerOffTime(void)
{
	switch(MenuSettingConfig()->uiMOVPowerOffTime)
    {
    #if (MENU_MOVIE_POWER_OFF_DELAY_0SEC_EN)
	    case MOVIE_POWEROFF_TIME_0MIN:
	        m_ulVRPowerOffTime = 0;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_5SEC_EN)
	    case MOVIE_POWEROFF_TIME_5SEC:
	        m_ulVRPowerOffTime = 5;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_10SEC_EN)
	    case MOVIE_POWEROFF_TIME_10SEC:
	        m_ulVRPowerOffTime = 10;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_15SEC_EN)
	    case MOVIE_POWEROFF_TIME_15SEC:
	        m_ulVRPowerOffTime = 15;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_30SEC_EN)
	    case MOVIE_POWEROFF_TIME_30SEC:
	        m_ulVRPowerOffTime = 30;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_1MIN_EN)
	    case MOVIE_POWEROFF_TIME_1MIN:
	        m_ulVRPowerOffTime = 60;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_2MIN_EN)
	    case MOVIE_POWEROFF_TIME_2MIN:
	        m_ulVRPowerOffTime = 120;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_3MIN_EN)
	    case MOVIE_POWEROFF_TIME_3MIN:
	        m_ulVRPowerOffTime = 180;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_5MIN_EN)
	    case MOVIE_POWEROFF_TIME_5MIN:
	        m_ulVRPowerOffTime = 300;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_10MIN_EN)
	    case MOVIE_POWEROFF_TIME_10MIN:
	        m_ulVRPowerOffTime = 600;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_15MIN_EN)
	    case MOVIE_POWEROFF_TIME_15MIN:
	        m_ulVRPowerOffTime = 900;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_30MIN_EN)
	    case MOVIE_POWEROFF_TIME_30MIN:
	        m_ulVRPowerOffTime = 1800;
	        break;
	#endif
	#if (MENU_MOVIE_POWER_OFF_DELAY_60MIN_EN)
	    case MOVIE_POWEROFF_TIME_60MIN:
	        m_ulVRPowerOffTime = 3600;
	        break;
	#endif
	    default:
	        m_ulVRPowerOffTime = 0;
	        break;
    }

	return m_ulVRPowerOffTime;
}

UINT32 AHC_GetVideoMaxLockFileTime(void)
{
	return MAX_LOCK_FILE_TIME;
}

INT8* AHC_GetLockedFileName(UINT8 bSelect)
{
    INT8 		FileName[DCF_MAX_FILE_NAME_SIZE] = {0};
	UINT32		MaxDcfObj;

	MaxDcfObj = AHC_GetMappingFileNum(FILE_MOVIE);

	if(MaxDcfObj == 0)
		return (INT8*)DEFAULT_LOCKED_FILENAME;

	switch(bSelect)
	{
		case 0://Previous
			if(MaxDcfObj<=1)
				return (INT8*)DEFAULT_LOCKED_FILENAME;
			else
                AHC_UF_GetFileNamebyIndex(MaxDcfObj-2,FileName) ;
		break;
		case 1://Current
            AHC_UF_GetFileNamebyIndex(MaxDcfObj-2,FileName) ;
		break;
		default:
			return (INT8*)DEFAULT_LOCKED_FILENAME;
		break;
	}
	return FileName;
}

void AHC_SetShutdownByChargerOut(AHC_BOOL bEnable)
{
	VideoPowerOffCounterReset();
	m_ubShutdownByChargerOut = bEnable;
}

AHC_BOOL AHC_GetShutdownByChargerOut(void)
{
    return m_ubShutdownByChargerOut;
}

void AHC_SetRecordByChargerIn(UINT8 ubDelay)
{
	#if (SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
	if ((AHC_GetBootingSrc() & PWR_ON_BY_GSENSOR) == PWR_ON_BY_GSENSOR)
	{
		if (GSNR_PWRON_ACT == GSNR_PWRON_REC_AUTO) {
    		m_ubBootwithCharger = ubDelay;
    		return;
        }
	}
	#endif

	if (AHC_GetAutoRec()) {
		m_ubBootwithCharger = ubDelay;
    }
	else {
		printc(FG_RED("!!! MenuSettingConfig()->uiAutoRec is OFF\r\n"));
		m_ubBootwithCharger = 0;
	}
}

UINT8 AHC_GetRecordByChargerIn(void)
{
	return m_ubBootwithCharger;
}

AHC_BOOL AHC_GetAutoRec(void)
{
	return (MenuSettingConfig()->uiAutoRec == AUTO_REC_ON)? (AHC_TRUE) : (AHC_FALSE);
}
AHC_BOOL AHC_RollBackMinKeyProtectFile(void)
{
    extern UINT32       m_ulLockFileNum;
 	AHC_BOOL			ahc_ret 		= AHC_FALSE;
   	UINT32				MaxObjIdx;
	UINT32				ObjIdx;
    AHC_BOOL            bReadOnly;
    AHC_BOOL            bCharLock;
    
	MaxObjIdx = AHC_GetMappingFileNum(FILE_MOVIE);

	if(MaxObjIdx == 0) {
		printc("No More DCF File for Delete!\r\n");
		return AHC_FALSE;
	}

	for(ObjIdx = 0; ObjIdx < MaxObjIdx; ObjIdx++) {
        if (m_ulLockFileNum <= MAX_LOCK_FILE_NUM) {
            return AHC_TRUE;
        }

		AHC_UF_SetCurrentIndex(ObjIdx);

		if(AHC_FALSE == AHC_UF_IsReadOnlybyIndex(ObjIdx,&bReadOnly)) {
			printc(FG_RED("AHC_UF_IsReadOnlybyIndex Error\r\n"));
			return AHC_FALSE;
		}

		if (AHC_FALSE == AHC_UF_IsCharLockbyIndex(ObjIdx, &bCharLock)) {
			printc(FG_RED("AHC_UF_IsCharLockbyIndex Error\r\n"));
			return AHC_FALSE;
		}

		if (bReadOnly) {
			// Read Only File
			ahc_ret = AHC_UF_FileOperation_ByIdx(ObjIdx, DCF_FILE_NON_READ_ONLY_ALL_CAM, NULL, NULL);
			if(ahc_ret == AHC_FALSE) {
	    		printc(FG_RED("AHC_UF_FileOperation_ByIdx DCF_FILE_NON_READ_ONLY_ALL_CAM Error\r\n"));
    			return AHC_FALSE;
	    	}
		}

		if (bCharLock) {
			// Char Lock
            ahc_ret = AHC_UF_FileOperation_ByIdx(ObjIdx, DCF_FILE_CHAR_UNLOCK_ALL_CAM, NULL, NULL);
			if(ahc_ret==AHC_FALSE) {
	    		printc(FG_RED("AHC_UF_FileOperation_ByIdx DCF_FILE_CHAR_UNLOCK_ALL_CAM Error\r\n"));
	    	}
		}

        if (bReadOnly && bCharLock) {
            m_ulLockFileNum++;
        }
    }
    
	return ahc_ret;
}

#if 0
void ____TV_Function_____(){ruturn;} //dummy
#endif

void AHC_SwitchLCDandTVOUT(void)
{
#if (SWITCH_TVOUT_BY_KEY)

	#ifdef TV_ONLY

	if(AHC_IsTVConnectEx())
	{
		if(m_ubInTVMode)
		{
			m_ubInTVMode = AHC_FALSE;
			MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_TV, MMP_FALSE);
		}
		else
		{
			m_ubInTVMode = AHC_TRUE;
			MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_TV, MMP_TRUE);
		}
	}

	#else//TV_ONLY

	if(AHC_IsTVConnectEx())
	{
		m_ubGoToTVmode = AHC_FALSE;
		StateTVMode(EVENT_TV_KEY_LCD_OUTPUT);
	}
	else
	{
		m_ubGoToTVmode = AHC_TRUE;
		AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_TV_DETECTED, 0);
	}
	#endif
#endif
}

#if 0
void ____LCD_Function_____(){ruturn;} //dummy
#endif

AHC_BOOL AHC_SwitchLCDBackLight(void)
{
	if(!LedCtrl_GetBacklightStatus())
	{
		LedCtrl_LcdBackLight(AHC_TRUE);
	}
	else
	{
		bInLCDPowerSave = AHC_TRUE;
		LedCtrl_LcdBackLight(AHC_FALSE);
	}

	return AHC_TRUE;
}

AHC_BOOL AHC_GetFlipSelection(UINT8 ubCtrl,UINT8* pSelection,UINT8* pbLinkMenu)
{
	if(ubCtrl == CTRL_BY_MENU)
	{
		*pSelection = MENU_FLIP_SELECT;
		*pbLinkMenu = AHC_TRUE;
	}
	else if(ubCtrl == CTRL_BY_KEY)
	{
		*pSelection = KEY_FLIP_SELECT;
		*pbLinkMenu = KEY_FLIP_LINK_MENU;
	}
	else if(ubCtrl == CTRL_BY_HALL_SNR)
	{
		*pSelection = HALL_SNR_FLIP_SELECT;
		*pbLinkMenu = HALL_SNR_FLIP_LINK_MENU;
	}
	else if(ubCtrl == CTRL_BY_G_SNR)
	{
		*pSelection = G_SNR_FLIP_SELECT;
		*pbLinkMenu = G_SNR_FLIP_LINK_MENU;
	}

	return AHC_TRUE;
}

AHC_BOOL AHC_CheckOSDFlipEn(UINT8 ubCheck)
{
	UINT8    ubOSDFlipEn = 0;
	UINT8	 ubSelect1   = 0, ubSelect2   = 0, ubSelect3   = 0, ubSelect4   = 0;
	UINT8 	 ubLinkMenu1 = 0, ubLinkMenu2 = 0, ubLinkMenu3 = 0, ubLinkMenu4 = 0;

	if(ubCheck & CTRL_BY_MENU)
		AHC_GetFlipSelection(CTRL_BY_MENU, &ubSelect1, &ubLinkMenu1);

	if(ubCheck & CTRL_BY_KEY)
		AHC_GetFlipSelection(CTRL_BY_KEY, &ubSelect2, &ubLinkMenu2);

	if(ubCheck & CTRL_BY_HALL_SNR)
		AHC_GetFlipSelection(CTRL_BY_HALL_SNR, &ubSelect3, &ubLinkMenu3);

	if(ubCheck & CTRL_BY_G_SNR)
		AHC_GetFlipSelection(CTRL_BY_G_SNR, &ubSelect4, &ubLinkMenu4);

	ubOSDFlipEn = ((ubSelect1 & FLIP_OSD) | (ubSelect2 & FLIP_OSD) | (ubSelect3 & FLIP_OSD) | (ubSelect4 & FLIP_OSD)) ;

	return (ubOSDFlipEn)?(AHC_TRUE):(AHC_FALSE);
}

AHC_BOOL AHC_CheckSNRFlipEn(UINT8 ubCheck)
{
	UINT8    ubSNRFlipEn = 0;
	UINT8	 ubSelect1   = 0, ubSelect2   = 0, ubSelect3   = 0, ubSelect4   = 0;
	UINT8 	 ubLinkMenu1 = 0, ubLinkMenu2 = 0, ubLinkMenu3 = 0, ubLinkMenu4 = 0;

	if(ubCheck & CTRL_BY_MENU)
		AHC_GetFlipSelection(CTRL_BY_MENU, &ubSelect1, &ubLinkMenu1);

	if(ubCheck & CTRL_BY_KEY)
		AHC_GetFlipSelection(CTRL_BY_KEY, &ubSelect2, &ubLinkMenu2);

	if(ubCheck & CTRL_BY_HALL_SNR)
		AHC_GetFlipSelection(CTRL_BY_HALL_SNR, &ubSelect3, &ubLinkMenu3);

	if(ubCheck & CTRL_BY_G_SNR)
		AHC_GetFlipSelection(CTRL_BY_G_SNR, &ubSelect4, &ubLinkMenu4);

	ubSNRFlipEn = ((ubSelect1 & FLIP_SNR) | (ubSelect2 & FLIP_SNR) | (ubSelect3 & FLIP_SNR) | (ubSelect4 & FLIP_SNR)) ;

	return (ubSNRFlipEn)?(AHC_TRUE):(AHC_FALSE);
}

AHC_BOOL AHC_CheckLinkWithMenu(UINT8 ubCheck)
{
	UINT8    ubLinkMenu  = 0;
	UINT8	 ubSelect1   = 0, ubSelect2   = 0, ubSelect3   = 0, ubSelect4   = 0;
	UINT8 	 ubLinkMenu1 = 0, ubLinkMenu2 = 0, ubLinkMenu3 = 0, ubLinkMenu4 = 0;

	if(ubCheck & CTRL_BY_MENU)
		AHC_GetFlipSelection(CTRL_BY_MENU, &ubSelect1, &ubLinkMenu1);

	if(ubCheck & CTRL_BY_KEY)
		AHC_GetFlipSelection(CTRL_BY_KEY, &ubSelect2, &ubLinkMenu2);

	if(ubCheck & CTRL_BY_HALL_SNR)
		AHC_GetFlipSelection(CTRL_BY_HALL_SNR, &ubSelect3, &ubLinkMenu3);

	if(ubCheck & CTRL_BY_G_SNR)
		AHC_GetFlipSelection(CTRL_BY_G_SNR, &ubSelect4, &ubLinkMenu4);

	//ubLinkMenu = (ubLinkMenu1| ubLinkMenu2 | ubLinkMenu3 | ubLinkMenu4) ;
	ubLinkMenu = (ubLinkMenu2 | ubLinkMenu3 | ubLinkMenu4) ;

	return (ubLinkMenu)?(AHC_TRUE):(AHC_FALSE);
}

void AHC_SetSensorStatus(UINT8 ubSNRState)
{
	m_ubSensorStatus = ubSNRState;
}

UINT8 AHC_GetSensorStatus(void)
{
	return m_ubSensorStatus;
}

void AHC_SetRotateSrc(UINT8 src)
{
	m_ubRotateSrc = src;
}

UINT8 AHC_GetRotateSrc(void)
{
	return m_ubRotateSrc;
}

AHC_BOOL AHC_LinkLCDStatusToPara(UINT32 wValue)
{
	AHC_BOOL ubOSDFlipEn,ubLinkMenu;

	switch( AHC_GetRotateSrc() )
	{
		case SRC_KEY:
			ubOSDFlipEn = AHC_CheckOSDFlipEn(CTRL_BY_KEY);
			ubLinkMenu  = AHC_CheckLinkWithMenu(CTRL_BY_KEY);
		break;
		case SRC_HALL_SNR:
			ubOSDFlipEn = AHC_CheckOSDFlipEn(CTRL_BY_HALL_SNR);
			ubLinkMenu  = AHC_CheckLinkWithMenu(CTRL_BY_HALL_SNR);
		break;
		case SRC_G_SNR:
			ubOSDFlipEn = AHC_CheckOSDFlipEn(CTRL_BY_G_SNR);
			ubLinkMenu  = AHC_CheckLinkWithMenu(CTRL_BY_G_SNR);
		break;
		default:
			ubOSDFlipEn = AHC_FALSE;
			ubLinkMenu  = AHC_TRUE;
		break;
	}

	if(ubOSDFlipEn && ubLinkMenu)
	{
		MenuSettingConfig()->uiLCDRotate = (wValue==AHC_LCD_REVERSE)?(LCD_ROTATE_ON):(LCD_ROTATE_OFF);
	}
	return AHC_TRUE;
}

AHC_BOOL AHC_DrawRotateEvent(UINT32 Event)
{
	UINT8 		LCDdir, SNRdir, UImode;
    AHC_BOOL 	ubOSDFlipEn, ubSNRFlipEn;

	if(Event==EVENT_LCD_COVER_NORMAL)
	{
		LCDdir = AHC_LCD_NORMAL;
		SNRdir = AHC_SNR_NORMAL;
	}
	else if(Event==EVENT_LCD_COVER_ROTATE)
	{
		LCDdir = AHC_LCD_REVERSE;
		SNRdir = AHC_SNR_REVERSE;
    }
    else
		return AHC_TRUE;

    //Check Filp Condition
	switch( AHC_GetRotateSrc() )
	{
		case SRC_KEY:
			ubOSDFlipEn = AHC_CheckOSDFlipEn(CTRL_BY_KEY);
			ubSNRFlipEn = AHC_CheckSNRFlipEn(CTRL_BY_KEY);
		break;
		case SRC_HALL_SNR:
			ubOSDFlipEn = AHC_CheckOSDFlipEn(CTRL_BY_HALL_SNR);
			ubSNRFlipEn = AHC_CheckSNRFlipEn(CTRL_BY_HALL_SNR);
		break;
		case SRC_G_SNR:
			ubOSDFlipEn = AHC_CheckOSDFlipEn(CTRL_BY_G_SNR);
			ubSNRFlipEn = AHC_CheckSNRFlipEn(CTRL_BY_G_SNR);
		break;
		default:
			ubOSDFlipEn = AHC_FALSE;
			ubSNRFlipEn = AHC_FALSE;
		break;
	}

	//Set Drawing Function
	UImode = uiGetCurrentState();

    switch( UImode )
    {
        case UI_CAMERA_STATE    :
			DrawEventHandler = (DRAW_EVENT*) DrawStateCaptureUpdate;
        break;

        case UI_VIDEO_STATE     :
			DrawEventHandler = (DRAW_EVENT*) DrawStateVideoRecUpdate;
        break;

        case UI_BROWSER_STATE   :
			DrawEventHandler = (DRAW_EVENT*) DrawStateLCDBrowserUpdate;
        break;

        case UI_PLAYBACK_STATE  :
			if( JPGPB_MODE == GetCurrentOpMode())
			{
				DrawEventHandler = (DRAW_EVENT*) DrawStatePhotoPb_handler_Lcd;
			}
			else if( MOVPB_MODE == GetCurrentOpMode() )
			{
				DrawEventHandler = (DRAW_EVENT*) DrawStateMovPlaybackUpdate;
			}
			else if( AUDPB_MODE == GetCurrentOpMode() )
			{
				DrawEventHandler = (DRAW_EVENT*) DrawStateAudPlaybackUpdate;
			}
			else if( JPGPB_MOVPB_MODE == GetCurrentOpMode())
			{
			    UINT32 uiCurrentIdx;
                UINT8  ubFileType;
				AHC_UF_GetCurrentIndex(&uiCurrentIdx);
                AHC_UF_GetFileTypebyIndex(uiCurrentIdx, &ubFileType);
                if ( ubFileType == DCF_OBG_JPG )
				    DrawEventHandler = (DRAW_EVENT*) DrawStatePhotoPb_handler_Lcd;
				 else
				    DrawEventHandler = (DRAW_EVENT*) DrawStateMovPlaybackUpdate;   
			}
			
        break;

        default                       :
			DrawEventHandler = NULL;
        break;
    }

    //Set LCD/Sensor Status
    if(ubOSDFlipEn)
		AHC_SetParam(PARAM_ID_LCD_STATUS, LCDdir);

	if(ubSNRFlipEn)
		AHC_SetSensorStatus(SNRdir);

	//Rotate LCD/Sensors
	LedCtrl_LcdBackLight(AHC_FALSE);
	AHC_SetKitDirection(LCDdir, ubOSDFlipEn ,SNRdir, ubSNRFlipEn);

	if(DrawEventHandler && ubOSDFlipEn)
	{
		DrawEventHandler(Event);
		DrawEventHandler = NULL;
	}

	LedCtrl_LcdBackLight(AHC_TRUE);
	AHC_SetRotateSrc(SRC_MAX);//Reset

	return AHC_TRUE;
}

AHC_BOOL AHC_DrawMenuRotateEvent(UINT32 Event, PSMENUSTRUCT pMenu)
{
	UINT8 		LCDdir, SNRdir;
    AHC_BOOL 	ubOSDFlipEn,ubSNRFlipEn;
	UINT32		MenuId = pMenu->iMenuId;

	if(Event==MENU_LCD_NORMAL)
	{
		LCDdir = AHC_LCD_NORMAL;
		SNRdir = AHC_SNR_NORMAL;
	}
	else if(Event==MENU_LCD_ROTATE)
	{
		LCDdir = AHC_LCD_REVERSE;
		SNRdir = AHC_SNR_REVERSE;
    }
    else
    	return AHC_TRUE;

    //Check Filp Condition
	switch( AHC_GetRotateSrc() )
	{
		case SRC_KEY:
			ubOSDFlipEn = AHC_CheckOSDFlipEn(CTRL_BY_KEY);
			ubSNRFlipEn = AHC_CheckSNRFlipEn(CTRL_BY_KEY);
		break;
		case SRC_HALL_SNR:
			ubOSDFlipEn = AHC_CheckOSDFlipEn(CTRL_BY_HALL_SNR);
			ubSNRFlipEn = AHC_CheckSNRFlipEn(CTRL_BY_HALL_SNR);
		break;
		case SRC_G_SNR:
			ubOSDFlipEn = AHC_CheckOSDFlipEn(CTRL_BY_G_SNR);
			ubSNRFlipEn = AHC_CheckSNRFlipEn(CTRL_BY_G_SNR);
		break;
		default:
			ubOSDFlipEn = AHC_FALSE;
			ubSNRFlipEn = AHC_FALSE;
		break;
	}

	//Set Special Drawing Function
	if(Deleting_InBrowser || Protecting_InBrowser)
	{
		if( (Deleting_InBrowser && Delete_File_Confirm) 		||
			(Protecting_InBrowser==1 && Protect_File_Confirm) 	||
			(Protecting_InBrowser==2 && UnProtect_File_Confirm)	)
			DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_EditOneFile;

		goto L_Set;
	}
	else if(PowerOff_InProcess==AHC_TRUE)
	{
		#if (POWER_OFF_CONFIRM_PAGE_EN)
		DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_PowerOff;
		#endif
		goto L_Set;
	}

	//Set Drawing Function
	if( MenuId >= MENUID_QUICK_MENU_VIDEO && MenuId <= MENUID_MAIN_MENU_EXIT )//Main Menu
	{
		switch( MenuId )
		{
			#if (USB_MODE_SELECT_EN)
			case MENUID_MODE_SELECT:
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawMainPage_ModeSelect;
			break;
			#endif

			#if (TOP_MENU_PAGE_EN)
			case MENUID_TOP_MENU:
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawTopMenuPage;
			break;
			#endif

			default:
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawMainPage;
			break;
		}
	}
	else
	{
	    switch( MenuId )
	    {
	        case MENUID_SUB_MENU_FLASH_LEVEL    :
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_FlashLevel;
	        break;

	        case MENUID_SUB_MENU_DELETE_ALL_VIDEO     :
			case MENUID_SUB_MENU_DELETE_ALL_IMAGE	  :
			case MENUID_SUB_MENU_PROTECT_ALL_VIDEO    :
			case MENUID_SUB_MENU_PROTECT_ALL_IMAGE    :
			case MENUID_SUB_MENU_UNPROTECT_ALL_VIDEO  :
			case MENUID_SUB_MENU_UNPROTECT_ALL_IMAGE  :
			case MENUID_SUB_MENU_FORMAT_SD_CARD    	  :
	        case MENUID_SUB_MENU_RESET_SETUP    	  :
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_ConfirmPage;
	        break;

	        case MENUID_SUB_MENU_CLOCK_SETTINGS    :
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_ClockSetting;
	        break;

			#if (MENU_GENERAL_DRIVER_ID_SETTING_EN)
			case MENUID_SUB_MENU_DRIVER_ID_SETTINGS:
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_DriverIdSetting;
			break;
			#endif

	        case MENUID_SUB_MENU_FW_VERSION_INFO    :
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_FwVersionInfo;
	        break;

			#if (MENU_GENERAL_GPSINFO_EN)
	        case MENUID_SUB_MENU_GPS_INFO    :
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_GPSInfo;
	        break;
	        #endif

	        case MENUID_SUB_MENU_EV    :
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_EV;
	        break;

	        case MENUID_SUB_MENU_SD_CARD_INFO    :
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_StorageInfo;
	        break;

	        case MENUID_SUB_MENU_VOLUME    :
				DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage_Volume;
	        break;

	        default                       :
	        	DrawMenuEventHandler = (DRAW_MENUEVENT*) MenuDrawSubPage;
	        break;
	    }
    }

L_Set:

    //Set LCD/Sensor Status
	if(ubOSDFlipEn)
		AHC_SetParam(PARAM_ID_LCD_STATUS, LCDdir);

	if(ubSNRFlipEn)
		AHC_SetSensorStatus(SNRdir);

	//Rotate LCD/Sensors
	LedCtrl_LcdBackLight(AHC_FALSE);
	AHC_SetKitDirection(LCDdir, ubOSDFlipEn, SNRdir, ubSNRFlipEn);

	if(DrawMenuEventHandler && ubOSDFlipEn)
	{
		DrawMenuEventHandler(pMenu);
		DrawMenuEventHandler = NULL;
	}

	LedCtrl_LcdBackLight(AHC_TRUE);
	AHC_SetRotateSrc(SRC_MAX);//Reset

	return AHC_TRUE;
}

AHC_BOOL AHC_SetKitDirection(UINT8 LCDdir, AHC_BOOL bSetLCD, UINT8 SNRdir, AHC_BOOL bSetSnr)
{
//There is a problem when HDMI connecting and 0x8000_2BBF = 0xA5 (AHC_LCD_Direction).
#if (TVOUT_PREVIEW_EN)
    if (AHC_IsTVConnectEx()) {
        bSetLCD = AHC_FALSE;
    }
#endif
#if (HDMI_PREVIEW_EN)
    if (AHC_IsHdmiConnect()) {
        bSetLCD = AHC_FALSE;
    }
#endif
	if (bSetLCD)
	{
		if (LCDdir == AHC_LCD_NORMAL)
			AHC_LCD_Direction(LCD_0_DEGREE);
		else if (LCDdir == AHC_LCD_REVERSE)
			AHC_LCD_Direction(LCD_180_DEGREE);
	}

	if (bSetSnr)
	{
        if (AHC_SNR_IsSnrInitialized(gsAhcPrmSensor))
        {
    		if (SNRdir == AHC_SNR_NORMAL)
    			AHC_SNR_SetFlipDir(gsAhcPrmSensor, SENSOR_0_DEGREE);
    		else if (SNRdir == AHC_SNR_REVERSE)
    			AHC_SNR_SetFlipDir(gsAhcPrmSensor, SENSOR_180_DEGREE);
        }
	}

	return AHC_TRUE;
}

AHC_BOOL AHC_PreSetLCDDirection(void)
{
	#define INVALID_DATA	0xFF

    MMP_UBYTE checkSum = 0;
    MMP_USHORT i;
	MMP_UBYTE ubLCDStatus, ubSNRStatus, ubLCDSetting;
	MMP_ULONG SF_DestAddr;
    MMP_UBYTE tempBuf[MENU_STATUS_REGION_SIZE];
    MMPF_SIF_INFO *info;

	info = MMPF_SF_GetSFInfo();

	SF_DestAddr = info->ulSFTotalSize - MENU_STATUS_REGION_SIZE;

    MMPF_MMU_FlushDCacheMVA((MMP_ULONG)tempBuf, sizeof(tempBuf));
	MMPF_SF_ReadData(SF_DestAddr, (MMP_ULONG) &tempBuf, MENU_STATUS_REGION_SIZE);

    for (i = 0; i < SETTING_TOTAL_ITEMS; i++) {
        checkSum += tempBuf[MENU_STATUS_REGION_SIZE - 1 - i];    
    	//RTNA_DBG_PrintByte(0, tempBuf[sizeof(tempBuf) - 1 - i]); 
    }

	printc("Check sum = 0x%02X\r\n", checkSum);

    if (SETTING_CHECK_VALUE != checkSum) {
        RTNA_DBG_Str(0, FG_RED("AHC_PreSetLCDDirection: Check sum is invaild !!!\r\n"));
    	ubSNRStatus  = AHC_SNR_NORMAL;
    	ubLCDStatus  = (MenuSettingConfig()->uiLCDRotate == LCD_ROTATE_OFF) ? AHC_LCD_NORMAL : AHC_LCD_REVERSE;
    	ubLCDSetting = MenuSettingConfig()->uiLCDRotate;
        Menu_WriteLcdSnrTVHdmiStauts2SF();
    }
    else {
    	ubSNRStatus  = tempBuf[SNR_STATUS_OFFSET];
    	ubLCDStatus  = tempBuf[LCD_STATUS_OFFSET];
    	ubLCDSetting = tempBuf[LCD_MENU_SETTING_OFFSET];
    }

	printc("Read From SF SNR %d LCD %d Menu %d\r\n",ubSNRStatus, ubLCDStatus, ubLCDSetting);

	//Set OSD/Sensor Status
	ubLCDStatus = (MenuSettingConfig()->uiLCDRotate == LCD_ROTATE_ON) ? (AHC_LCD_REVERSE) : (AHC_LCD_NORMAL);

	AHC_SetParam(PARAM_ID_LCD_STATUS, ubLCDStatus);
	AHC_SetSensorStatus(ubSNRStatus);
	// Purpose: run SetSensorDefaultPosition to right initial setting of _snpos
	// 720P@30 and defined SENSOR_UPSIDE_DOWN => PIP window has garbage on the buttom
	AHC_SetKitDirection(ubLCDStatus, AHC_FALSE, ubSNRStatus, AHC_FALSE);

    return AHC_TRUE;
}

#if 0
void ____Gsensor_Function_____(){ruturn;} //dummy
#endif

/* Set Power On Gsensor Interrupt Status */
AHC_BOOL AHC_Gsensor_SetPowerOnIntThreshold(void)
{
#if (SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
    UINT32 sens;
    MMP_UBYTE ubStableTimes = 0;
    
    if ((AHC_Menu_SettingGetCB((char*)COMMON_KEY_GSENSOR_PWR_ON_SENS, &sens) == AHC_FALSE))
    {
    	sens = GSENSOR_POWERON_SENSITIVITY_OFF; // default 
    }

	switch(sens)
	{
		case GSENSOR_POWERON_SENSITIVITY_L0:
			GSNRPowerOnThd = POWER_ON_GSNR_SENS_LEVEL0;
		break;

		case GSENSOR_POWERON_SENSITIVITY_L1:
			GSNRPowerOnThd = POWER_ON_GSNR_SENS_LEVEL1;
		break;

		case GSENSOR_POWERON_SENSITIVITY_L2:
			GSNRPowerOnThd = POWER_ON_GSNR_SENS_LEVEL2;
		break;

		//case GSENSOR_POWERON_SENSITIVITY_OFF:
		default:
			GSNRPowerOnThd = 0;
		break;
	}

    AHC_Gsensor_IOControl(GSNR_CMD_SET_PWR_ON_INT_THD , &GSNRPowerOnThd);

    while (1) {
        MMP_UBYTE ubIntStatus;
        
        AHC_Gsensor_IOControl(GSNR_CMD_GET_INT_STATUS , &ubIntStatus);

        if (0 == ubIntStatus) {
            ubStableTimes++;
        }
        else {
            ubStableTimes = 0;
            RTNA_DBG_Str(0, FG_RED("G-Sensor is sharking ...\r\n"));
        }

        AHC_OS_SleepMs(10);

        if (ubStableTimes >= 20) {
            RTNA_DBG_Str(0, "G-Sensor is silent ...\r\n");
            break;
        }
    }
#endif

	return AHC_TRUE;
}

#if(SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
UINT32 AHC_GSNR_PWROn_MovieTimeReset(void)
{
	return POWER_ON_GSNR_MOVIE_TIME*10;
}
#endif



AHC_BOOL AHC_ChangeEV(AHC_BOOL bCycle, UINT8 bDir)
{
	extern void DrawStateCaptureUpdate(UINT32 ubEvent);
	UINT8 cev = 0;
    if((AHC_Menu_SettingGetCB((char*)COMMON_KEY_EV, &cev) == AHC_FALSE))
    {
    	cev = COMMON_EVVALUE_DEFAULT; // default;
    }
	if(bCycle)
	{
		if(cev < (COMMON_EVVALUE_NUM - 1))
		{
			cev++;
		}
		else if(cev == (COMMON_EVVALUE_NUM - 1))
		{
			cev = 0;
		}
	}
	else
	{
		if(bDir && cev < (COMMON_EVVALUE_NUM - 1))
		{
			cev++;
		}
		else if(!bDir && cev > 0)
		{
			cev--;
		}
	}
	AHC_SetAeEvBiasMode(Menu_EV_To_AE_EV_BIAS(cev));
	DrawStateCaptureUpdate(EVENT_DSC_SET_EV);

	return AHC_TRUE;
}

AHC_BOOL AHC_ChangeVolume(AHC_BOOL bCycle,UINT8 bDir)
{
	#define MAX_VOLUME (10)

	if(bCycle)
	{
		if(MenuSettingConfig()->uiVolume < MAX_VOLUME)
		{
			MenuSettingConfig()->uiVolume++;
		}
		else if(MenuSettingConfig()->uiVolume == MAX_VOLUME)
		{
			MenuSettingConfig()->uiVolume = 0;
		}
	}
	else
	{
		if(bDir && MenuSettingConfig()->uiVolume < MAX_VOLUME)
		{
			MenuSettingConfig()->uiVolume++;
		}
		else if(!bDir && MenuSettingConfig()->uiVolume > 0)
		{
			MenuSettingConfig()->uiVolume--;
		}
	}

	Menu_SetVolume(MenuSettingConfig()->uiVolume);

	return AHC_TRUE;
}

AHC_BOOL AHC_ToggleMute(void)
{
	extern AHC_BOOL	bMuteRecord;

	if(MenuSettingConfig()->uiMOVSoundRecord==MOVIE_SOUND_RECORD_ON)
	{
		bMuteRecord = AHC_TRUE;
		Menu_SetMICSensitivity(MenuSettingConfig()->uiMicSensitivity);//backup AGAIN&DGAIN
		Menu_SetSoundRecord(MOVIE_SOUND_RECORD_OFF);
	}
	else
	{
		bMuteRecord = AHC_FALSE;
		Menu_SetSoundRecord(MOVIE_SOUND_RECORD_ON);
	}

	if(VideoFunc_RecordStatus())
	{
		if(bMuteRecord)
			AHC_VIDEO_SetRecVolumeByMenuSetting(AHC_FALSE);
		else
			AHC_VIDEO_SetRecVolumeByMenuSetting(AHC_TRUE);
	}
    else
    {
		if(bMuteRecord)
			AHC_PlaySoundEffect(AHC_SOUNDEFFECT_MICOFF);
		else
			AHC_PlaySoundEffect(AHC_SOUNDEFFECT_MICON);
    }

	return AHC_TRUE;
}

AHC_BOOL AHC_ReciveAduio2(void)//long 4-20
{

	printc("~~~~~~~~555555666~~~~~~\t\n");

	BD3490_init();
	printc("~~~~~~~~555flong~~~~~~\t\n");


	return AHC_TRUE;
}

AHC_BOOL AHC_ToggleTwilightMode(void)
{
#if (DAY_NIGHT_MODE_SWITCH_EN)

	extern AHC_BOOL bNightMode;
	UINT8 csc = 0;

	if((AHC_Menu_SettingGetCB((char*)COMMON_KEY_SCENE, &csc) == AHC_FALSE)){
        csc = COMMON_SCENE_AUTO;//default value
    }

    switch(csc)
    {
    case COMMON_SCENE_TWILIGHT:
        Menu_SetSceneMode(SCENE_TWILIGHT);
		bNightMode = AHC_TRUE;
		printc("TWILIGHT MODE!!\r\n");
        break;
    case COMMON_SCENE_AUTO:
        Menu_SetSceneMode(SCENE_AUTO);
		bNightMode = AHC_FALSE;
		printc("AUTO MODE!\r\n");
        break;
    }
#endif
	return AHC_TRUE;
}

#if 0
void ____LED_Function_____(){ruturn;} //dummy
#endif

void AHC_SetMenuFlashLEDStatus(UINT8 ubLed)
{
	bUpdateFlashLedIcon = AHC_TRUE;
	m_ubMenuFlashLed = ubLed;
}

UINT8 AHC_GetMenuFlashLEDStatus(void)
{
	return m_ubMenuFlashLed;
}

AHC_BOOL AHC_ToggleFlashLED(UINT8 ubMode)
{
	UINT8 ubCtrl = LED_FLASH_CTRL;
	UINT8 LEDState;

	if(ubMode==LED_MODE_AUTO_ON_OFF)
	{
		MenuSettingConfig()->uiFlashMode = (MenuSettingConfig()->uiFlashMode + 1) % FLASH_NUM;
		Menu_SetFlashLEDMode( MenuSettingConfig()->uiFlashMode );
	}
	else if(ubMode==LED_MODE_ON_OFF)
	{
		switch(ubCtrl)
		{
			case LED_BY_KEY:
				if(MenuSettingConfig()->uiFlashMode==FLASH_ON)
					MenuSettingConfig()->uiFlashMode = FLASH_OFF;
				else if(MenuSettingConfig()->uiFlashMode==FLASH_OFF)
					MenuSettingConfig()->uiFlashMode = FLASH_ON;

				Menu_SetFlashLEDMode( MenuSettingConfig()->uiFlashMode );
			break;

			case LED_BY_KEY_WITH_MENU_CONFIRMED:

				if(MenuSettingConfig()->uiLEDFlash==LED_FALSH_ON)
				{
					if(MenuSettingConfig()->uiFlashMode==FLASH_ON)
						MenuSettingConfig()->uiFlashMode = FLASH_OFF;
					else if(MenuSettingConfig()->uiFlashMode==FLASH_OFF)
						MenuSettingConfig()->uiFlashMode = FLASH_ON;
				}

				Menu_SetFlashLEDMode( MenuSettingConfig()->uiFlashMode );
			break;

			case LED_BY_MENU:
				//TBD
			break;

			case LED_BY_MENU_AND_KEY_LINK:

				if(MenuSettingConfig()->uiLEDFlash==LED_FALSH_ON)
					MenuSettingConfig()->uiLEDFlash = LED_FALSH_OFF;
				else if(MenuSettingConfig()->uiLEDFlash==LED_FALSH_OFF)
					MenuSettingConfig()->uiLEDFlash = LED_FALSH_ON;

				AHC_SetMenuFlashLEDStatus (MenuSettingConfig()->uiLEDFlash);
				Menu_SetFlashLEDMode( MenuSettingConfig()->uiLEDFlash );
			break;

			case LED_BY_MENU_AND_KEY_NOT_LINK:

				LEDState = AHC_GetMenuFlashLEDStatus();
				LEDState = (LEDState == LED_FALSH_ON)?(LED_FALSH_OFF):(LED_FALSH_ON);

				AHC_SetMenuFlashLEDStatus(LEDState);
				LedCtrl_FlashLed(!LEDState);
			break;

			default:
				printc("Not Support LED Control Mode\r\n");
			break;
		}
	}
	bUpdateFlashLedIcon = AHC_TRUE;

	return AHC_TRUE;
}

void AHC_PreSetFlashLED(void)
{
#if (LED_FLASH_CTRL==LED_BY_MENU 				|| \
	 LED_FLASH_CTRL==LED_BY_MENU_AND_KEY_LINK	|| \
	 LED_FLASH_CTRL==LED_BY_MENU_AND_KEY_NOT_LINK)

	Menu_SetFlashLEDMode(AHC_GetMenuFlashLEDStatus());

#elif ( LED_FLASH_CTRL==LED_BY_KEY || \
		LED_FLASH_CTRL==LED_BY_KEY_WITH_MENU_CONFIRMED)

	Menu_SetFlashLEDMode(MenuSettingConfig()->uiFlashMode);
#endif
}

#if 0
void ____Misc_Function_____(){ruturn;} //dummy
#endif

AHC_BOOL AHC_PowerOn_Welcome(void)
{
//	MMPS_AUDIO_PLAY_STATUS 	status;
//    MMP_ERR 				ret_val;
	AHC_BOOL				ahc_ret;

#ifdef CFG_CUS_POWERON_SOUNDEFFECT
	CFG_CUS_POWERON_SOUNDEFFECT();
#else
	ahc_ret = AHC_PlaySoundEffect(AHC_SOUNDEFFECT_POWERON);
#endif

	/* Delay more time for task init priority */
	if(AHC_TRUE != ahc_ret)
		return AHC_TRUE;
//CZ patch...20160204		
#if 0 // For power on speed. Don't wait power on sound here. If need we can call stop play later.
	do{
        ret_val = MMPS_AUI_GetPlayStatus(&status);
        AHC_OS_Sleep(100);
    } while((MMP_ERR_NONE == ret_val) && (MMPS_AUDIO_PLAY_STOP != status));
#endif
    return AHC_TRUE;
}

AHC_BOOL AHC_ShowErrorDialog(WMSG_INFO iInfo)
{
	if (iInfo != WMSG_NONE)
	{
		m_ubDialogState = MSG_PRESENT;
		AHC_WMSG_Draw(AHC_TRUE, iInfo, 0);
		m_ubDialogEvent = iInfo;
	}
	else
	{
		m_ubDialogState = MSG_DISMISS;
		AHC_WMSG_Draw(AHC_FALSE, WMSG_NONE, 0);
		m_ubDialogEvent = WMSG_NONE;
	}
	return AHC_TRUE;
}

AHC_BOOL AHC_CheckErrorDialog(UINT32 ulEvent)
{
	if (ERROR_DIALOG_PRESENT(m_ubDialogState))
	{
        if (AHC_WMSG_GetInfo() == WMSG_NONE) {
            AHC_WMSG_Draw(AHC_TRUE, m_ubDialogEvent, 0);
        }

        if(m_ubDialogEvent == WMSG_INSERT_SD_AGAIN && ulEvent == EVENT_SD_DETECT)
        {
            AHC_ShowErrorDialog(WMSG_NONE);	// Dismiss Error Dialog
            AHC_WMSG_Draw(AHC_TRUE, WMSG_WAIT_INITIAL_DONE, 3);
            return AHC_TRUE;
        }
        
		// Critical Error Presetns and waiting any keys to release
        if ((ulEvent == EVENT_POWER_OFF)        ||
            ((ulEvent >= EVENT_KEY_START) && (ulEvent < EVENT_KEY_END)))
		{
			AHC_ShowErrorDialog(WMSG_NONE);	// Dismiss Error Dialog
		}
		return AHC_FALSE;
	}
	return AHC_TRUE;
}

AHC_BOOL AHC_IsDialogPresent(void)
{
	return ERROR_DIALOG_PRESENT(m_ubDialogState);
}

AHC_BOOL AHC_ShowSoundWarning(void)
{
	if (ERROR_DIALOG_PRESENT(m_ubDialogState))
	{
		if (MenuSettingConfig()->uiBeep == BEEP_ON && MenuSettingConfig()->uiVolume != 0)
	    {
	        if(!VideoFunc_RecordStatus() && !VideoFunc_PreRecordStatus())
				AHC_PlaySoundEffect(AHC_SOUNDEFFECT_BATWARNING);
		}
	}
	return AHC_TRUE;
}

AHC_BOOL AHC_CheckCurSysMode(UINT16 CheckMode)
{
    UINT32   CurSysMode;

    AHC_GetSystemStatus(&CurSysMode);

	CurSysMode >>= 16;

	return (CurSysMode==CheckMode)?(AHC_TRUE):(AHC_FALSE);
}

AHC_BOOL AHC_CheckCurFileLockStatus(void)
{
	UINT32 CurObjIdx;

	AHC_UF_GetCurrentIndex(&CurObjIdx);

	return (AHC_IsFileProtect(CurObjIdx))?(AHC_TRUE):(AHC_FALSE);
}

AHC_BOOL AHC_IsCurFileCharLock(void)
{
	UINT32 CurObjIdx;

	AHC_UF_GetCurrentIndex(&CurObjIdx);

	return (AHC_Thumb_IsFileLocked(CurObjIdx))?(AHC_TRUE):(AHC_FALSE);
}

AHC_BOOL AHC_SetPlayFileOrder(UINT8 ubOrder)
{
	UINT32 MaxDcfObj;

	if(GetCurrentOpMode()==MOVPB_MODE || GetCurrentOpMode()==VIDEOREC_MODE)
		MaxDcfObj = AHC_GetMappingFileNum(FILE_MOVIE);
	else if(GetCurrentOpMode()==JPGPB_MODE || GetCurrentOpMode()==CAPTURE_MODE)
		MaxDcfObj = AHC_GetMappingFileNum(FILE_PHOTO);
	else if(GetCurrentOpMode()==JPGPB_MOVPB_MODE)
        MaxDcfObj = AHC_GetMappingFileNum(FILE_PHOTO_MOVIE);
	
	if(MaxDcfObj==0)
		return AHC_FALSE;

	switch(ubOrder)
	{
		case PLAY_FIRST_FILE:
			ObjSelect = 0;
		break;
		case PLAY_CUR_FILE:
			//TBD
		break;
		case PLAY_LAST_FILE:
			ObjSelect = MaxDcfObj-1;
		break;
		default:
		break;
	}
	return AHC_TRUE;
}

UINT32 AHC_GetMappingFileNum(UINT8 ubFileType)
{
    UINT32	MaxDcfObj = 0;

    switch(ubFileType)
    {
    	case FILE_PHOTO:
        	AHC_UF_SetFreeChar(0, DCF_SET_ALLOWED, (UINT8 *) SEARCH_PHOTO);
		break;

    	case FILE_MOVIE:
        	AHC_UF_SetFreeChar(0, DCF_SET_ALLOWED, (UINT8 *) SEARCH_MOVIE);
		break;

    	case FILE_PHOTO_MOVIE:
        	AHC_UF_SetFreeChar(0, DCF_SET_ALLOWED, (UINT8 *) SEARCH_PHOTO_MOVIE);
		break;

		default:
			return 0;
    }

	AHC_UF_GetTotalFileCount(&MaxDcfObj);
   	return MaxDcfObj;
}

AHC_BOOL AHC_SetBootUpUIMode(void)
{
#ifdef CFG_CUS_BOOT_UP_UI_MODE
    CFG_CUS_BOOT_UP_UI_MODE();
#else
    #if (MENU_GENERAL_BOOT_UP_MODE_EN)

	extern UINT8 m_ubDSCMode;

	switch(MenuSettingConfig()->uiBootUpMode)
    {
	#if (MENU_GENERAL_BOOT_UP_VR_MODE_EN)
    	case BOOT_UP_MODE_DVR:
			StateSwitchMode(UI_VIDEO_STATE);
	    break;
	#endif
	#if (MENU_GENERAL_BOOT_UP_DSC_MODE_EN)
	    case BOOT_UP_MODE_DSC:
	    	m_ubDSCMode = DSC_ONE_SHOT;
			StateSwitchMode(UI_CAMERA_STATE);
		break;
	#endif
	#if (MENU_GENERAL_BOOT_UP_BURST_MODE_EN)
	    case BOOT_UP_MODE_BURST:
	    	m_ubDSCMode = DSC_BURST_SHOT;
			StateSwitchMode(UI_CAMERA_STATE);
		break;
	#endif
	#if (MENU_GENERAL_BOOT_UP_TIMELAPSE_MODE_EN)
	    case BOOT_UP_MODE_TIMELAPSE:
	    	m_ubDSCMode = DSC_TIME_LAPSE;
			StateSwitchMode(UI_CAMERA_STATE);
		break;
	#endif
	#if (MENU_GENERAL_BOOT_UP_WIFI_SWITCH_MODE_EN)
    	case BOOT_UP_MODE_WIFI_SWITCH:
			StateSwitchMode(UI_VIDEO_STATE);
	    break;
    #endif
	#if (MENU_GENERAL_BOOT_UP_6S_REC_MODE_EN)
    	case BOOT_UP_MODE_6S_REC:
			StateSwitchMode(UI_VIDEO_STATE);
	    break;
	#endif
	    default:
			StateSwitchMode(UI_VIDEO_STATE);
		break;
    }
    #endif
#endif

	return AHC_TRUE;
}

static	int	m_ulKeyWorking = 0;

int AHC_PauseKeyUI(void)
{
	m_ulKeyWorking++;
	return m_ulKeyWorking;
}

int AHC_ResumeKeyUI(void)
{
	if (m_ulKeyWorking == 0) {
		printc("!!!!! Pause/Resume isn't match!!!!!\r\n");
		return -1;
	}

	m_ulKeyWorking--;
	return m_ulKeyWorking;
}

int AHC_KeyUIWorking(void)
{
	return (m_ulKeyWorking == 0);
}

#if 0
void ____FileIO_Function_____(){ruturn;} //dummy
#endif

#define	IS_LINE_ENDING(c)	((c) == 0x0a || (c) == 0x0d)

int AHC_ReadlLine(UINT32 fd, char* buf, int size)
{
	AHC_BOOL	err;
	UINT32		out;
	int			i;

	i = 0;

	do {
		out = 0;

		err = AHC_FS_FileRead(fd, (unsigned char*)buf + i, 1, &out);

		if (err == 0 && out == 1)
		{
			if (IS_LINE_ENDING(*(buf + i)))
			{
				/* got line ending */
				if (i == 0)
					continue;

				*(buf + i) = 0;
				break;
			}

			i++;

			if (i == size - 1)
			{
				/* line is too long, read and drop it */
				do {
					out = 0;
					err = AHC_FS_FileRead(fd, (unsigned char*)buf + i, 1, &out);

					if (err == 0 && out == 1) {
						if (IS_LINE_ENDING(*(buf + i)))
							break;
					} else {
						break;
					}
				} while (1);

				*(buf + i) = 0;
				break;
			}
		}
		else
		{
			*(buf + i) = 0;
			break;
		}

	} while (1);

	return i;
}

/*
 * fbuf: source buffer
 * fp  : source current index
 * fs  : soufce buffer size
 * outl: output buffer
 * size: output buffer size
 */
int AHC_ReadLineMem(char *fbuf, int* fp, int fs, char* outl, int size)
{
	int		i;

	i = 0;

	do {
		*(outl + i) = *(fbuf + *fp);

		(*fp)++;

		if (*fp < fs)
		{
			if (IS_LINE_ENDING(*(outl + i)))
			{
				/* got line ending */
				if (i == 0)
					continue;

				*(outl + i) = 0;
				break;
			}

			i++;

			if (i == size - 1)
			{
				/* line is too long, read and drop it */
				do {
					*(outl + i) = *(fbuf + *fp);

					(*fp)++;
					if (*fp < fs) {
						if (IS_LINE_ENDING(*(outl + i)))
							break;
					} else {
						break;
					}
				} while (1);

				*(outl + i) = 0;
				break;
			}
		}
		else
		{
			*(outl + i) = 0;
			break;
		}

	} while (1);

	return i;
}

#if 0
void ____SDMMC_Function_____(){ruturn;} //dummy
#endif

UINT32 AHC_SDMMC_GetLockFileNum(void)
{
    AHC_BOOL        ahc_ret = AHC_TRUE;
    UINT32          MaxDcfObj, i;
    UINT32          CurrentObjIdx;
    UINT32			ulFileNum = 0;
    AHC_BOOL        bReadOnly;
    #if (PROTECT_FILE_TYPE == PROTECT_FILE_RENAME)
    AHC_BOOL        bDeleteFile;
    #endif
    INT8            byFileName[DCF_MAX_FILE_NAME_SIZE];

    if(!AHC_IsSDInserted()) {
        RTNA_DBG_Str(0, "SDMMC is not inseart\r\n");
    	return 0;
    }

    MaxDcfObj = AHC_GetMappingFileNum(FILE_MOVIE);

    if(MaxDcfObj == 0) {
        RTNA_DBG_Str(0, "MaxDcfObj is 0\r\n");
    	return 0;
    }

	AHC_UF_SetCurrentIndex(0);
    AHC_UF_InitLockFileTable();

	for(i=0;i<MaxDcfObj;i++)
	{
		AHC_UF_GetCurrentIndex(&CurrentObjIdx);
        ahc_ret = AHC_UF_IsReadOnlybyIndex(CurrentObjIdx, &bReadOnly);

		if(ahc_ret == AHC_FALSE) {
		    printc(FG_RED("AHC_UF_IsReadOnlybyIndex error - %d\r\n"), ulFileNum);
			return ulFileNum;
		}

		if (bReadOnly)
		{
			ulFileNum++;
		}
        else {
            AHC_UF_GetFileNamebyIndex(CurrentObjIdx, byFileName);
        
			#if (PROTECT_FILE_TYPE == PROTECT_FILE_RENAME)
			if(memcmp(byFileName, FILE_RENAME_STRING, strlen(FILE_RENAME_STRING))==0)
            {
				ulFileNum++;
                AHC_UF_UpdateLockFileTable(CurrentObjIdx,&bDeleteFile);
                if(bDeleteFile == AHC_TRUE)
                {
                    CurrentObjIdx--;           //if delete lockfile Index would shift up 1 offset
                    MaxDcfObj--;
                    bDeleteFile = AHC_FALSE;
                }
            }
			#endif
        }
        
		CurrentObjIdx++;
		AHC_UF_SetCurrentIndex(CurrentObjIdx);
    }

	printc(FG_GREEN("%s [%d]\r\n"), __func__, ulFileNum);

	return ulFileNum;
}

#if 0
void ____PowerOn_Option_Function_____(){ruturn;} //dummy
#endif

void AHC_CancelQuickKey(int qk)
{
#if (POWER_ON_BUTTON_ACTION)
	m_ulQuickKey &= ~qk;
#endif
}

INT32 AHC_GetQuickKey(int qk)
{
#if (POWER_ON_BUTTON_ACTION)
	return m_ulQuickKey & qk;
#else
	return 0;
#endif
}

int AHC_SetQuickKey(int qk)
{
#if (POWER_ON_BUTTON_ACTION)
	if (m_usOption) {
		m_ulQuickKey |= qk;
	}
	return m_ulQuickKey;
#else
	return 0;
#endif
}

AHC_BOOL AHC_SetButtonStatus(UINT32 keyId)
{
#if (POWER_ON_BUTTON_ACTION)
	switch(keyId)
	{
    #ifdef CFG_KEY_POWER_ON_KEYSET_2 //may be defined in config_xxx.h
		case KEY_PRESS_FUNC2:
		case KEY_LPRESS_FUNC2:
	#else
		case KEY_LPRESS_REC:	//REC
		case KEY_PRESS_REC:
	#endif

            #ifdef CFG_BOOT_CUS_QUICK_KEY //may be defined in config_xxx.h, could be QUICK_REC
			AHC_SetQuickKey(CFG_BOOT_CUS_QUICK_KEY);
			#else
			AHC_SetQuickKey(QUICK_PCCAM);
			#endif
		break;


		case KEY_LPRESS_MENU:	//Menu
		case KEY_PRESS_MENU:
			AHC_SetQuickKey(QUICK_MENU);
		break;

		case KEY_LPRESS_MODE:	//Mode
		case KEY_PRESS_MODE:
			AHC_SetQuickKey(QUICK_MODE);
		break;


		case KEY_LPRESS_PLAY:	//Play
		case KEY_PRESS_PLAY:
			AHC_SetQuickKey(QUICK_PLAY);
		break;

		case KEY_LPRESS_UP:		//Up
		case KEY_PRESS_UP:
			AHC_SetQuickKey(QUICK_UP);
		break;

		case KEY_LPRESS_DOWN:	//Down
		case KEY_PRESS_DOWN:
			AHC_SetQuickKey(QUICK_DOWN);
		break;
	}
#endif
	return AHC_TRUE;
}

AHC_BOOL AHC_PowerOnOption(void)
{
#if (POWER_ON_BUTTON_ACTION)

	int		quick;

	quick = AHC_GetQuickKey(QUICK_ALL);


	if ((quick & QUICK_REC) ||
	    (quick & QUICK_MENU))  //Show FW version or PCAM
	{
		if (quick & QUICK_MENU)
			printc("@@@ POWER_ON QUCIK MENU!!\r\n");

		if (quick & QUICK_REC)
			printc("@@@ POWER_ON QUCIK REC!!\r\n");

        #ifdef CFG_BOOT_QUICK_POWER_ON_MENU //may be defined in config_xxx.h
			AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_POWER_ON_MENU, 0);
	    #endif
	}

	if (quick & QUICK_PLAY) //TBD
	{
		printc("POWER_ON QUCIK PLAY!!!\r\n");
    #ifdef CFG_BOOT_QUICK_POWER_ON_PLAY //may be defined in config_xxx.h
		AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_POWER_ON_PLAY, 0);
    #endif
	}

	if (quick & QUICK_MODE) //Format and Restore menu setting.
	{
		printc(">>> POWER_ON QUCIK MODE!!!\r\n");
    #ifdef CFG_BOOT_QUICK_POWER_ON_FORMAT //may be defined in config_xxx.h
		QuickMediaOperation(MEDIA_CMD_FORMAT);
		RestoreFromDefaultSetting();
	#endif
	}

	if (quick & QUICK_UP) 	//Update FW
	{
        #if defined CFG_BOOT_QUICK_UPDATE_FW || defined CFG_BOOT_QUICK_UPDATE_FW_PER_USB //may be defined in config_xxx.h
		extern AHC_BOOL ubUpdateFWbySD;
		#endif

		printc(">>> POWER_ON QUCIK UP!!!\r\n");
        #ifdef CFG_BOOT_QUICK_UPDATE_FW //may be defined in config_xxx.h
			ubUpdateFWbySD = AHC_TRUE;
		#elif defined (CFG_BOOT_QUICK_UPDATE_FW_PER_USB)
			{
				if (AHC_FALSE == AHC_IsUsbConnect())
					ubUpdateFWbySD = AHC_TRUE;
				else
					AHC_SetQuickKey(QUICK_PCCAM);
			}
		#endif
	}

	if (quick & QUICK_DOWN) 	//Format and Restore menu setting.
	{
		printc(">>> POWER_ON QUCIK DOWN!!!\r\n");
        #ifdef CFG_BOOT_QUICK_FORMAT_AND_OFF //may be defined in config_xxx.h
			LedCtrl_LcdBackLightLock(AHC_FALSE);
			#if (TVOUT_PREVIEW_EN)
			if(!AHC_IsTVConnectEx())
			#endif
				LedCtrl_LcdBackLight(AHC_TRUE);
			QuickMediaOperation(MEDIA_CMD_FORMAT);
			RestoreFromDefaultSetting();
			AHC_PowerOff_NormalPath();
		#endif
	}

	if (quick)
		PowerOnAction = AHC_TRUE;
	else
		PowerOnAction = AHC_FALSE;

	m_usOption = 0;	// make AHC_SetQuickKey dummy
					// avoid USB handler to refer qucik key to do PCCAM
	return PowerOnAction;

#else

	return AHC_FALSE;

#endif
}

#if 0
void ____USB_Function____(){ruturn;} //dummy
#endif

/*
 app == 1 for PCCAM
 app == 0 for MSDC
 */
void AHC_CheckUSB_KeyFile(char *kf, int app)
{
#if (USB_MODE_SELECT_EN==0)
	MMP_ULONG	fd;
	MMP_ERR		err;

	err = MMPC_FS_FileOpen(kf, "r", (void **)&fd);
	printc("Open %s %d\r\n", kf, err);

	if (err == MMP_ERR_NONE) {
		MenuSettingConfig()->uiUSBFunction = (app == 1)? MENU_SETTING_USB_PCAM : MENU_SETTING_USB_MSDC;
		MMPC_FS_FileClose((void *)fd);
	} else {
		MenuSettingConfig()->uiUSBFunction = MENU_SETTING_USB_MSDC;
	}
	printc("Open %s %d USB is %s\r\n", kf, err, (MenuSettingConfig()->uiUSBFunction == MENU_SETTING_USB_PCAM)? "PCCAM" : "MSDC");

#endif
}

AHC_BOOL AHC_SetUSBLable(char *pszLable)
{
	if (pszLable) {
		FS_SetVolumeLabel("SD:0:\\", pszLable);
	}
	return AHC_TRUE;
}

#if 0
void ____Vibration_Function____(){ruturn;} //dummy
#endif

#if defined(DEVICE_GPIO_VIBRATION) && (DEVICE_GPIO_VIBRATION != MMP_GPIO_MAX)
static UINT8 vibrationTimerID = 0xFF;

static void Vibration_Timer_ISR(void *ptmr, void *parg)
{
    AHC_OS_StopTimer(vibrationTimerID, AHC_OS_TMR_OPT_NONE );	// must be
	AHC_GPIO_SetData(DEVICE_GPIO_VIBRATION, (0 == DEVICE_GPIO_VIBRATION_ON) ? 1 : 0);
	vibrationTimerID = 0xFF;
}
#endif

AHC_BOOL AHC_Vibration_Enable(UINT32 ulMs)
{
#if defined(DEVICE_GPIO_VIBRATION) && (DEVICE_GPIO_VIBRATION != MMP_GPIO_MAX)
	#if defined MENU_GENERAL_MOTO_EN &&(MENU_GENERAL_MOTO_EN==1)
	if(!pf_GetMotoStatus()) return AHC_TRUE;
	#endif

	if ((UINT32) -1 > ulMs) {
	    if (vibrationTimerID < 0xFE) {
	        AHC_OS_StopTimer( vibrationTimerID, AHC_OS_TMR_OPT_NONE);
	        vibrationTimerID = 0xFF;
	    }

        AHC_GPIO_ConfigPad(DEVICE_GPIO_VIBRATION, PAD_OUT_DRIVING(0));
	    vibrationTimerID = AHC_OS_StartTimer( ulMs, AHC_OS_TMR_OPT_ONE_SHOT, Vibration_Timer_ISR, (void *) NULL );

	    if (0xFE <= vibrationTimerID) {
	        // Start timer error
			AHC_GPIO_SetOutputMode(DEVICE_GPIO_VIBRATION, AHC_TRUE);
			AHC_GPIO_SetData(DEVICE_GPIO_VIBRATION, DEVICE_GPIO_VIBRATION_ON);
	    	AHC_OS_SleepMs(ulMs);
			AHC_GPIO_SetData(DEVICE_GPIO_VIBRATION, (0 == DEVICE_GPIO_VIBRATION_ON) ? 1 : 0);

			return AHC_FALSE;
		}
		else {
			AHC_GPIO_SetOutputMode(DEVICE_GPIO_VIBRATION, AHC_TRUE);
			AHC_GPIO_SetData(DEVICE_GPIO_VIBRATION, DEVICE_GPIO_VIBRATION_ON);

			return AHC_TRUE;
		}
	}

	AHC_GPIO_SetOutputMode(DEVICE_GPIO_VIBRATION, AHC_TRUE);
	AHC_GPIO_SetData(DEVICE_GPIO_VIBRATION, DEVICE_GPIO_VIBRATION_ON);

	return AHC_TRUE;
#else
	return AHC_FALSE;
#endif
}

AHC_BOOL AHC_Vibration_Disable(void)
{
#if defined(DEVICE_GPIO_VIBRATION) && (DEVICE_GPIO_VIBRATION != MMP_GPIO_MAX)
    AHC_BOOL ret = 0;

	if (vibrationTimerID < 0xFE)
	{
	    ret = AHC_OS_StopTimer(vibrationTimerID, AHC_OS_TMR_OPT_NONE );
		vibrationTimerID = 0xFF;

	    if (0xFF == ret)
	 		ret = AHC_FALSE;
		else
			ret = AHC_TRUE;
	}

	AHC_GPIO_SetData(DEVICE_GPIO_VIBRATION, (0 == DEVICE_GPIO_VIBRATION_ON) ? 1 : 0);

	return ret;
#else
	return AHC_FALSE;
#endif
}

// TY Miao - TBD
const char* AHC_GetVideoExt(void)
{
	UINT32	ct;

	AIHC_VIDEO_GetMovieCfgEx(0, AHC_CLIP_CONTAINER_TYPE, &ct);
	return (ct == VIDMGR_CONTAINER_3GP)? VR_3GP_EXT_FILENAME : "AVI";
}
