#if (defined(WIFI_PORT) && WIFI_PORT == 1)
/*===========================================================================
 * Include files
 *===========================================================================*/ 
#include "Customer_config.h"

#include "AHC_Common.h"
#include "AHC_General.h"
#include "AHC_Sensor.h"
#include "AHC_Stream.h"
#include "AHC_Message.h"
#include "AHC_Menu.h"
#include "MenuSetting.h"
//#include "AHC_Stream.h"
#include "StateNetStreamingFunc.h"
#include "StateCameraFunc.h"
#include "StateVideoFunc.h"
#include "KeyParser.h"
#include "netapp.h"
/*===========================================================================
 * Macro define
 *===========================================================================*/

/*===========================================================================
 * Global variables
 *===========================================================================*/ 


/*===========================================================================
 * Extern variable
 *===========================================================================*/


/*===========================================================================
 * Extern function
 *===========================================================================*/ 


/*===========================================================================
 * Main body
 *===========================================================================*/ 

void STATE_NET_CGICMD_WIRELESS_SET_FLICKER(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiFlickerHz = ulParam;
    pf_General_EnSet(COMMON_KEY_FLICKER, ulParam);
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_AWB(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiWB = ulParam;
    pf_General_EnSet(COMMON_KEY_WB, ulParam);
    Menu_SetAWB(ulParam);
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_EV(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiEV = ulParam;
	pf_General_EnSet(COMMON_KEY_EV, ulParam);
    Menu_WriteSetting();
	AHC_SetAeEvBiasMode(ulParam);
}

void STATE_NET_CGICMD_WIRELESS_SET_MOVIE_SIZE(UINT32 ulEvent, UINT32 ulParam)
{
	UINT8 ubCurUIState = 0;
	UI_STATE_ID ubParentUIState = 0;
	
	MenuSettingConfig()->uiMOVSize = ulParam;
    pf_General_EnSet(COMMON_KEY_MOVIE_SIZE, ulParam);
    Menu_WriteSetting();
    ubCurUIState = uiGetCurrentState();
    StateModeGetParent(ubCurUIState, &ubParentUIState);

	if( ubParentUIState == UI_VIDEO_STATE )
	{
		VideoTimer_Stop();
		VideoRecMode_PreviewUpdate();    
	}
    
}

void STATE_NET_CGICMD_WIRELESS_SET_CLIP_TIME(UINT32 ulEvent, UINT32 ulParam)
{
	//TODO MenuSettingConfig()->?
    pf_General_EnSet(COMMON_KEY_VR_CLIP_TIME, ulParam);
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_VQUALITY(UINT32 ulEvent, UINT32 ulParam)
{
	//TODO MenuSettingConfig()->?
	pf_General_EnSet(COMMON_KEY_MOVIE_QUALITY, ulParam);
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_IMAGE_SIZE(UINT32 ulEvent, UINT32 ulParam)
{
	UINT8 ubCurUIState = 0;
	UI_STATE_ID ubParentUIState = 0;
    
    MenuSettingConfig()->uiIMGSize = ulParam;
    pf_General_EnSet(COMMON_KEY_MOVIE_SIZE, ulParam);
    Menu_WriteSetting();
    ubCurUIState = uiGetCurrentState();
    StateModeGetParent(ubCurUIState, &ubParentUIState);

	if( ubParentUIState == UI_CAMERA_STATE ) {
		CaptureTimer_Stop();
		CaptureMode_PreviewUpdate();
	}
    
}

void STATE_NET_CGICMD_WIRELESS_SET_POWROFF_DELAY(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiMOVPowerOffTime = ulParam;
    pf_General_EnSet(COMMON_KEY_POWER_OFF_DELAY, ulParam);
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_AUTO_PWROFF_TIME(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiAutoPowerOff = ulParam;
	pf_General_EnSet(COMMON_AUTO_POWER_OFF, ulParam);
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_MTD(UINT32 ulEvent, UINT32 ulParam)
{
	extern void AHC_SwitchMotionDetectionMode(void);
	UINT8 byParam = (UINT8)ulParam;
    
    MenuSettingConfig()->uiMotionDtcSensitivity = byParam;
	pf_General_EnSet(COMMON_KEY_MOTION_SENS, ulParam);
	Menu_SetMotionDtcSensitivity(byParam);
	// Switch MVD mode when sensitivity is changed
	AHC_SwitchMotionDetectionMode();
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_TIME_FMT(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiDateTimeFormat = ulParam;  
    pf_General_EnSet(COMMON_DATE_TIME_FMT, ulParam);
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_TV_SYS(UINT32 ulEvent, UINT32 ulParam)
{	
	MenuSettingConfig()->uiTVSystem = ulParam; 
	pf_General_EnSet(COMMON_KEY_TV_SYSTEM, ulParam);  
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_LCD_PWRS(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiLCDPowerSave = ulParam;
	pf_General_EnSet(COMMON_KEY_LCD_POWERS, ulParam);  
    Menu_WriteSetting();  
}

void STATE_NET_CGICMD_WIRELESS_SET_PHOTOBURST(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiBurstShot = ulParam;
	pf_General_EnSet(COMMON_KEY_BURST_SHOT, ulParam);   
    Menu_WriteSetting();  
}

void STATE_NET_CGICMD_WIRELESS_SET_TIMELAPSE(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiTimeLapseTime = ulParam;
	pf_General_EnSet(COMMON_KEY_TIMELAPSE, ulParam); 
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_UPSIDEDOWN(UINT32 ulEvent, UINT32 ulParam)
{ 
	MenuSettingConfig()->uiImageUpsideDown = ulParam;
	pf_General_EnSet(COMMON_KEY_UPSIDE_DOWN, ulParam); 
    Menu_WriteSetting();
    AHC_SNR_SetFlipDir(PRM_SENSOR, ulParam);
}

void STATE_NET_CGICMD_WIRELESS_SET_SPOTMETER(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiMetering = ulParam;
    pf_General_EnSet(COMMON_KEY_METER, ulParam);  
    Menu_WriteSetting();
    if( MenuSettingConfig()->uiMetering == METERING_CENTER_SPOT )
    	AHC_SetAeMeteringMode( AHC_AE_METERING_SPOT );
	else
        AHC_SetAeMeteringMode( AHC_AE_METERING_CENTER );
}

void STATE_NET_CGICMD_WIRELESS_SET_HDR(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiWDR = ulParam;
    pf_General_EnSet(COMMON_KEY_HDR_EN, ulParam);  
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_Q_SHOT(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiAutoRec = ulParam;
    pf_General_EnSet(COMMON_KEY_AUTO_RECORD, ulParam);  
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_STATUSLIGHTS(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiStatusLight = ulParam;
    pf_General_EnSet(COMMON_KEY_STATUS_LIGHT, ulParam);  
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_SOUNDINDICATOR(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiBeep = ulParam;
    pf_General_EnSet(COMMON_KEY_BEEP, ulParam);  
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_POWERSAVING(UINT32 ulEvent, UINT32 ulParam)
{
	MenuSettingConfig()->uiAutoPowerOff = ulParam;
    pf_General_EnSet(COMMON_AUTO_POWER_OFF, ulParam);  
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_UIMODE(UINT32 ulEvent, UINT32 ulParam)
{
    UINT8 ubCurUIState = 0;
    UI_STATE_ID ubParentUIState = 0;
	MMP_BOOL bFoundParam = MMP_FALSE;
    printc(FG_RED("Change UI mode to %d\n"),ulParam);
    
    ubCurUIState = uiGetCurrentState();
    StateModeGetParent(ubCurUIState, &ubParentUIState); 
    
    if(ubParentUIState != UI_STATE_UNSUPPORTED)
    	bFoundParam = StateModeFindState(ubCurUIState, ulParam);
    else
    	bFoundParam = (ulParam == ubCurUIState) ? 1 : 0;
	
    if( !bFoundParam && ulParam < UI_STATE_UNSUPPORTED){
	    //dettach self
	    StateDetachSubMode();
	    //change mode
	    StateSwitchMode(ulParam);
    }
}

void STATE_NET_CGICMD_WIRELESS_SET_LOOPINGVIDEO(UINT32 ulEvent, UINT32 ulParam)
{
	UINT8 byParam = (UINT8)ulParam;
    MenuSettingConfig()->uiMOVClipTime = byParam;
    pf_General_EnSet(COMMON_KEY_VR_CLIP_TIME, ulParam);  
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_GSENSOR(UINT32 ulEvent, UINT32 ulParam)
{
	UINT8 byParam = (UINT8)ulParam;
	MenuSettingConfig()->uiGsensorSensitivity = byParam;
    pf_General_EnSet(COMMON_KEY_GSENSOR_SENS, ulParam);  
    Menu_WriteSetting();
}

void STATE_NET_CGICMD_WIRELESS_SET_MUTE(UINT32 ulEvent, UINT32 ulParam)
{
	UINT8 byParam = (UINT8)ulParam;
	MenuSettingConfig()->uiMOVSoundRecord = byParam;
    pf_General_EnSet("RecordWithAudio", ulParam);
    Menu_WriteSetting();
}

AHC_BOOL StateNetCGICmdModeInit(void* pData)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    
#if (1)
    printc("%s,%d \n", __func__, __LINE__);
#endif
            
    return ahcRet;
}

AHC_BOOL StateNetCGICmdModeShutDown(void* pData)
{
#if (1)
    printc("%s,%d \n", __func__, __LINE__);
#endif

    return AHC_TRUE;
}

UINT32 StateNetCGICmdModeHandler(UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam)
{
	return uiStateNetProcessMsg(KeyParser_NetCGICmdEvent, ulMsgId, ulEvent, ulParam);
}

#endif //#if (defined(WIFI_PORT) && WIFI_PORT == 1)

