/*===========================================================================
 * Include file 
 *===========================================================================*/ 
 
#include "Customer_Config.h"
#include "config_fw.h"
#include "AHC_Common.h"
#include "AHC_Parameter.h"
#include "AHC_General.h"
#include "AHC_Message.h"
#include "AHC_Utility.h"
#include "AHC_Display.h"
#include "AHC_Capture.h"
#include "AHC_Fs.h"
#include "AHC_Browser.h"
#include "AHC_Video.h"
#include "AHC_Audio.h"
#include "AHC_Menu.h"
#include "AHC_Os.h"
#include "AHC_Media.h"
#include "AHC_USB.h"
#include "AHC_General_CarDV.h"
#include "AHC_Warningmsg.h"
#include "AHC_UF.h"
#include "PMUCtrl.h"
#include "KeyParser.h"
#include "StateCameraFunc.h"
#include "StateVideoFunc.h"
#include "StateBrowserFunc.h"
#include "StateMoviePBFunc.h"
#include "StatePhotoPBFunc.h"
#include "StateAudioPBFunc.h"
#include "StateSlideShowFunc.h"
#include "StateMSDCFunc.h"
#include "StateHDMIFunc.h"
#include "StateTVFunc.h"

#include "MenuStateCameraMenu.h"
#include "MenuStateMovieMenu.h"
#include "MenuStatePlaybackMenu.h"
#include "MenuStateModeSelectMenu.h"
#include "MenuDrawCommon.h"
#include "MenuStateMenu.h"


#include "MenuCommon.h"
#include "MenuTouchButton.h"
#include "MenuSetting.h"
#include "LedControl.h"
#include "dsc_charger.h"
#include "ShowOSDFunc.h"
#include "MenuDrawingFunc.h"
#include "DrawStateBrowserFunc.h"
#include "MediaPlaybackCtrl.h"
#if (SD_UPDATE_FW_EN)
#include "ait_utility.h"
#include "SD_Updater.h"
#endif
#if (SUPPORT_EIS)
#include "mmpf_eis.h"
#endif
#include "pcam_api.h"
#include "mmpf_pll.h"
#include "ldws_cfg.h"
#include "hdr_cfg.h"

#include "mmpf_pio.h"//lyj

// for NETWORK
#if defined(WIFI_PORT) && (WIFI_PORT == 1)
#include "wlan.h"
#include "netapp.h"
#include "AHC_Wireless.h"
#include "StateNetStreamingFunc.h"
#include "StateNetPlaybackFunc.h"
#include "StateNetCGICmdFunc.h"
#endif
#include "UI_DrawGeneral.h"

#include "si47xx.h"

#if defined(TV_ONLY) && !defined(CFG_BOOT_BYPASS_CLOCK_CHECK)
    // force define CFG_BOOT_BYPASS_CLOCK_CHECK when TV_ONLY is defined
    #define CFG_BOOT_BYPASS_CLOCK_CHECK
#endif
#include "IconDefine.h"

#if (GSENSOR_CONNECT_ENABLE)
#include "GSensor_ctrl.h"
#endif

#ifndef CUS_AUTO_VR_DELAT_TIMES
#define CUS_AUTO_VR_DELAT_TIMES         (15)
#endif

#if 0
#define RegisterEventCb(_a, _b)   uiStateRegisterEventHandler(					\
						((_a >> EVENT_INDEX_SHIFT_BIT_FUNCTION_MAIN) & 0x0F), 	\
                        ((_a >> EVENT_INDEX_SHIFT_BIT_FUNCTION_SUB) & 0x0F), 	\
                        ((_a >> EVENT_INDEX_SHIFT_BIT_FUNCTION_MISC) & 0xFF),	\
						(void *)_b);
#else
void uiStateRegisterEventHandler(UINT32 ulFuncMain, UINT32 ulFuncSub, UINT32 ulFuncMisc, AHC_EVENT_HANDLER pfEventHandler);
void _RegisterEventCb(UINT32 _a, AHC_EVENT_HANDLER _b)
{
	uiStateRegisterEventHandler(
						((_a >> EVENT_INDEX_SHIFT_BIT_FUNCTION_MAIN) & 0x0F),
                        ((_a >> EVENT_INDEX_SHIFT_BIT_FUNCTION_SUB) & 0x0F),
                        ((_a >> EVENT_INDEX_SHIFT_BIT_FUNCTION_MISC) & 0xFF),
						_b);
}
#endif
/*===========================================================================
 * Global variable
 *===========================================================================*/ 
 
static AHC_BOOL         AhcMenuInit    = AHC_FALSE;
static UI_STATE_INFO    uiSysState;
static AHC_BOOL         uBlkEvent   = AHC_FALSE;
static UI_STATE_OP      m_StateOP[UI_STATE_NUM] = {NULL};
static UI_STATE_OP      m_PBStateOP = {NULL};

static AHC_EVENT_HANDLER gAhcEventHandler[MAX_EVENT_INDEX_FUNCTION_MAIN][MAX_EVENT_INDEX_FUNCTION_SUB][MAX_EVENT_INDEX_FUNCTION_MISC] = {NULL};

#if (defined(WIFI_PORT) && WIFI_PORT == 1)
static UI_GLOBAL_EVENT_HANDLER gGblNetEventHandler;
static AHC_NET_EVENT_HANDLER gAhcNetEventHandler[WIRELESS_SET_END - WIRELESS_MESSAGE_START] = {NULL};
#endif

MMP_BOOL BurninMode             = MMP_FALSE;
//MMP_BOOL CalibrationMode        = MMP_FALSE;

AHC_BOOL PowerOff_InProcess     = AHC_FALSE;
AHC_BOOL PowerOff_Option        = CONFIRM_NOT_YET;
//AHC_BOOL bRefreshBrowser        = AHC_FALSE;
AHC_BOOL bForce_PowerOff        = AHC_FALSE;
AHC_BOOL bShowPowerOffJpg       = AHC_FALSE;
UINT8    ubBootupSelect         = BOOTUP_MANUAL;

 AHC_BOOL flagJump = 100;

/*===========================================================================
 * Extern varible
 *===========================================================================*/
//extern AHC_BOOL bForceSwitchBrowser;

extern AHC_BOOL gbAhcDbgBrk;
extern AHC_BOOL 			return_flag;
/*===========================================================================
 * Extern function
 *===========================================================================*/

extern void     DefaultMenuSettingInit(void);
extern void     MenuItemDateTimeFormatToAhcTimeStampSetting(void);
extern AHC_BOOL CheckMenuSetting(MenuInfo* CurMenu);
extern AHC_BOOL uiStateProcessGlobalEvent(UINT32 ulMsgId, UINT32* pulEvent , UINT32* pulParam);
extern AHC_BOOL AHC_Charger_GetStatus(void);
extern MMP_BOOL SDUpdateIsWpExisted(void);
extern SD_UPDATER_ERR SDUpdateCfgWpFile(char ubFileName[], char ubFileExt[], MMP_BYTE ubWpFileNum);

#ifdef CFG_POWER_ON_CLEAN_SCREEN
extern void     RTNA_LCD_FillColor(MMP_ULONG color);
#endif
#if USB_MODE_SELECT_EN
AHC_BOOL USBSelectModePCam(PSMENUITEM pItem, AHC_BOOL bIsHover);
#endif

/*===========================================================================
 * Main body
 *===========================================================================*/

//void STATE_COMMON_DROP_EVENT(UINT32 ulEvent, UINT32 ulParam)
DECL_AHC_EV_HANDLER(STATE_COMMON_DROP_EVENT)
{
	//Do nothing to drop event
}

void StateLedControl( UINT32 uiMode )
{
    switch(uiMode)
    {
        case UI_VIDEO_STATE:
            LedCtrl_VideoModeLed(AHC_FALSE);
            LedCtrl_CameraModeLed(AHC_TRUE);
            LedCtrl_PlaybackModeLed(AHC_TRUE);
        break;
        case UI_CAMERA_STATE:
            LedCtrl_VideoModeLed(AHC_TRUE);
            LedCtrl_CameraModeLed(AHC_FALSE);
            LedCtrl_PlaybackModeLed(AHC_TRUE);
        break;
        case UI_BROWSER_STATE:
        case UI_PLAYBACK_STATE:
        case UI_MSDC_STATE:
            LedCtrl_VideoModeLed(AHC_TRUE);
            LedCtrl_CameraModeLed(AHC_TRUE);
            LedCtrl_PlaybackModeLed(AHC_TRUE);
        break;
        case UI_PCCAM_STATE:
            LedCtrl_VideoModeLed(AHC_TRUE);
            LedCtrl_CameraModeLed(AHC_TRUE);
            LedCtrl_PlaybackModeLed(AHC_TRUE);
        break;
        default:
            LedCtrl_VideoModeLed(AHC_TRUE);
            LedCtrl_CameraModeLed(AHC_TRUE);
            LedCtrl_PlaybackModeLed(AHC_TRUE);
        break;
    }
}

void StateLCDCheckStatus(UINT8 mode)
{
    UINT8 ubLCDstatus, ubSNRstatus;

    AHC_LCD_GetStatus(&ubLCDstatus);
    
    ubSNRstatus = AHC_GetSensorStatus();

    if(mode==UI_PCCAM_STATE)//For PCam
        AHC_SetKitDirection(ubLCDstatus, AHC_TRUE, ubSNRstatus, AHC_TRUE);
    else
        AHC_SetKitDirection(ubLCDstatus, AHC_TRUE, ubSNRstatus, AHC_FALSE);
}

AHC_BOOL StateModeResetAllOperation(void)
{
    UINT32 i;
    
    for (i = 0; i <= UI_STATE_UNSUPPORTED; i++) {
        m_StateOP[i].sParentUIState = UI_STATE_UNSUPPORTED;       
        m_StateOP[i].pfInitFunc = NULL;
        m_StateOP[i].pfShutDownFunc = NULL;
        m_StateOP[i].pfUIStateHandlerFunc = NULL;
    }

    return AHC_TRUE;
}

AHC_BOOL StateModeResetAllPBOperation(void)
{
    //Playback sub state.
    m_PBStateOP.sParentUIState = UI_STATE_UNSUPPORTED;       
    m_PBStateOP.pfInitFunc = NULL;
    m_PBStateOP.pfShutDownFunc = NULL;
    m_PBStateOP.pfUIStateHandlerFunc = NULL;
    return AHC_TRUE;
}

//AHC_BOOL StateModeSetOperation(UI_STATE_ID mState, void* pfInit, void* pfShutDown, void* pfHandler)
AHC_BOOL StateModeSetOperation(UI_STATE_ID mState, pfFunc pfInit, pfFunc pfShutDown, pfHandlerFunc pfHandler)
{
	printc("%s,%d \n", __func__, __LINE__); // long 4-27
    m_StateOP[mState].pfInitFunc = (pfFunc)pfInit;
    m_StateOP[mState].pfShutDownFunc = (pfFunc)pfShutDown;
    m_StateOP[mState].pfUIStateHandlerFunc = (pfHandlerFunc)pfHandler;
    //m_StateOP[mState].pfUIStateExecution = (UIStateExecution)pfExecution;
    
    return AHC_TRUE;
}

AHC_BOOL StatePBModeSetOperation(void* pfInit, void* pfShutDown, void* pfHandler)
{
    m_PBStateOP.pfInitFunc = (pfFunc)pfInit;
    m_PBStateOP.pfShutDownFunc = (pfFunc)pfShutDown;
    m_PBStateOP.pfUIStateHandlerFunc = (pfHandlerFunc)pfHandler;
    
    return AHC_TRUE;
}

void uiStateRegisterEventHandler(UINT32 ulFuncMain, UINT32 ulFuncSub, UINT32 ulFuncMisc, AHC_EVENT_HANDLER pfEventHandler)
{
    gAhcEventHandler[ulFuncMain][ulFuncSub][ulFuncMisc] = (AHC_EVENT_HANDLER)pfEventHandler;
}


#if (defined(WIFI_PORT) && WIFI_PORT == 1)
AHC_BOOL uiStateGetNetEventHanderIdx(UINT32 ulEvent, UINT32* idx)
{
	if ((ulEvent >= WIRELESS_SET_END) || (ulEvent < WIRELESS_MESSAGE_START)) {
		printd(BG_RED("Invalid Net ulEvent to handle %X")"\r\n", ulEvent);
		return AHC_FALSE;
	}
	*idx = ulEvent - WIRELESS_MESSAGE_START;
	return AHC_TRUE;
}

#if (defined(WIFI_PORT) && WIFI_PORT == 1)
///Register global network event handler
void uiStateRegisterGlobalNetEventHandler(UI_GLOBAL_EVENT_HANDLER pfEventHandler)
{
	gGblNetEventHandler = pfEventHandler;
}

void uiStateRegisterNetEventHandler(UINT32 ulFunc, AHC_NET_EVENT_HANDLER pfEventHandler)
{
	UINT32 idx;
	if (uiStateGetNetEventHanderIdx(ulFunc, &idx) == AHC_TRUE) {
		gAhcNetEventHandler[idx] = pfEventHandler;
	}
}
#endif

AHC_BOOL uiStateHandleNetEvent(UINT32 ulEvent, UINT32 ulParam)
{
	UINT32 idx;
#if ASSERT_UI_NET_EVENT
	if (ulEvent == 0)
		printd(BG_RED("Try event 0 at %s %d at state %d")"\r\n", __FUNCTION__, __LINE__, uiGetCurrentState());
#endif
	if (uiStateGetNetEventHanderIdx(ulEvent, &idx) == AHC_TRUE) {
		if(gAhcNetEventHandler[idx]){
			gAhcNetEventHandler[idx](ulEvent, ulParam);
		}
		else {
			printd(BG_PURPLE("message %d(0x%X) is not registered")"\r\n", ulEvent, ulEvent);
			return AHC_FALSE;
		}
		return AHC_TRUE;
	}
	return AHC_FALSE;
}

#endif

AHC_BOOL StateModeDoInit(UI_STATE_ID sState)
{
    AHC_BOOL    ahcRet = AHC_TRUE;

    if (m_StateOP[sState].pfInitFunc != NULL){
        ahcRet = m_StateOP[sState].pfInitFunc(NULL);
        if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;}         
    }
    
    return ahcRet;
}

AHC_BOOL StateModeDoShutdown(UI_STATE_ID sState)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    
    if (m_StateOP[sState].pfShutDownFunc != NULL) {
        ahcRet = m_StateOP[sState].pfShutDownFunc(NULL);
        if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 
    }
    
    return ahcRet;    
}
		
AHC_BOOL StateModeDoHandler(UI_STATE_ID sState, UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    
    uiSetWorkingState(sState);  
    if(m_StateOP[sState].pfUIStateHandlerFunc){
        m_StateOP[sState].pfUIStateHandlerFunc(ulMsgId, ulEvent, ulParam);
    }
    
    return ahcRet;    
}

AHC_BOOL StateModeClearEvent(void)
{
	//UINT sizeArry;
	//sizeArry = sizeof(gAhcEventHandler);
	//printc("StateModeClearEvent=%d\r\n",sizeArry);
	memset(gAhcEventHandler,0,sizeof(gAhcEventHandler));
	return AHC_TRUE;
}

AHC_BOOL StateModeDoHandlerEvent(UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam)
{
    switch(ulMsgId){
#if (defined(WIFI_PORT) && WIFI_PORT == 1)
        case AHLM_WIRELESS_CONNECT:
        case AHLM_NET:
		#if ASSERT_UI_NET_EVENT
        	if (ulEvent == 0) {
        		printd(BG_RED("Try event 0 at %s:line%d "
        				"id:%s param:x%X at state %d")"\r\n", __FUNCTION__, __LINE__,
        				ulMsgId==AHLM_NET?"AHLM_NET":"AHLM_CON", ulParam, uiGetCurrentState());
        	}
		#endif
		#if DBG_UI_NET_EVENT
        	if (ulEvent != 0) {
				printd(FG_GREEN("Try event %X at %s:line%d "
					"id:%s param:x%X at state %d")"\r\n", ulEvent, __FUNCTION__, __LINE__,
					ulMsgId==AHLM_NET?"AHLM_NET":"AHLM_CON", ulParam, uiGetCurrentState());
			}
		#endif
        	return uiStateHandleNetEvent(ulEvent, ulParam);
            break;
#endif

        case AHLM_UI_NOTIFICATION:
        case AHLM_GPIO_BUTTON_NOTIFICATION:
            {
                UINT32 ulFuncMain, ulFuncSub, ulFuncMisc;

                ulFuncMain = (ulEvent >> EVENT_INDEX_SHIFT_BIT_FUNCTION_MAIN) & 0x0F;
                ulFuncSub = (ulEvent >> EVENT_INDEX_SHIFT_BIT_FUNCTION_SUB) & 0x0F; 
                ulFuncMisc = (ulEvent >> EVENT_INDEX_SHIFT_BIT_FUNCTION_MISC) & 0xFF;
                
                if(gAhcEventHandler[ulFuncMain][ulFuncSub][ulFuncMisc]){
                    gAhcEventHandler[ulFuncMain][ulFuncSub][ulFuncMisc](ulEvent);
                }
            }
            break;

        case AHLM_TIMER_INDEX: //TBD
            printc("AHLM_TIMER_INDEX\r\n");
            break;
            
        default:
            printc("Unknown MsgId:0x%x\r\n", ulMsgId);
            AHC_PRINT_RET_ERROR(0, 0);
            break;            
    }
    
    return 0;
}

#if 0
UI_STATE_OP *StateModeGetObject(UI_STATE_ID mState)
{
    return &m_StateOP[mState];
}
#endif

void uiSetWorkingState(UINT8 uWorkingState)
{
    uiSysState.WorkingState = uWorkingState;
}

UINT8 uiGetWorkingState(void)
{
    return uiSysState.WorkingState;
}

void StateModeSetParent(UI_STATE_ID sState, UI_STATE_ID sParentState)
{
    m_StateOP[sState].sParentUIState = sParentState;       
}

void StateModeGetParent(UI_STATE_ID sState, UI_STATE_ID *psParentState)
{
    *psParentState = m_StateOP[sState].sParentUIState;       
}


AHC_BOOL StateModeFindState(UI_STATE_ID sState,  UI_STATE_ID sWantedState)
{  
    UI_STATE_ID sTmpState = 0;
    UI_STATE_ID ubParentUIState = 0;
    MMP_SHORT  usStateLayerMaxCount = 5;
    sTmpState = sState;
    
    do{
        StateModeGetParent(sTmpState, &ubParentUIState); 
        usStateLayerMaxCount     -=1;
        sTmpState   =  ubParentUIState;
        if(sWantedState == ubParentUIState)
             return AHC_TRUE;
    }while( (ubParentUIState != UI_STATE_UNSUPPORTED) && (usStateLayerMaxCount >=0));

    //if(usStateLayerMaxCount < 0)
        
    return AHC_FALSE;
}


#if 0
AHC_BOOL StateIsInMenuMode(void)
{
    if( UI_CAMERA_MENU_STATE   <= uiSysState.CurrentState &&
        UI_PLAYBACK_MENU_STATE >= uiSysState.CurrentState ){
        return AHC_TRUE;
    }
    
    return AHC_FALSE;
}
#endif



UINT32 StateCommonPBModeHandler(UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam)
{
    return m_PBStateOP.pfUIStateHandlerFunc(ulMsgId, ulEvent, ulParam);
}

AHC_BOOL StateCommonPBModeInit(void* pData)
{
    //AHC_BOOL ahcRet = AHC_TRUE;
    
#if (1)
    printc("%s,%d \n", __func__, __LINE__);
#endif

    return m_PBStateOP.pfInitFunc(pData);
}

AHC_BOOL StateCommonPBModeShutDown(void* pData)
{
#if (1)
    printc("%s,%d \n", __func__, __LINE__);
#endif

    return m_PBStateOP.pfShutDownFunc(pData);
}

AHC_BOOL StateSelectPBFunc(UINT8 ubUIMode)
{
    if( MOVPB_MODE == ubUIMode){
		#if (HDMI_ENABLE)
        	if (AHC_IsHdmiConnect()){
        		StatePBModeSetOperation((void*)StateMoviePlaybackModeInitHDMI, (void*)StateMoviePlaybackModeShutDown, (void *)StateMoviePlaybackModeHandler);
        	}else
		#endif
		#if (TVOUT_ENABLE)
    		if (AHC_IsTVConnectEx())
    		{

    			StatePBModeSetOperation((void*)StateMoviePlaybackModeInitTV, (void*)StateMoviePlaybackModeShutDown, (void *)StateMoviePlaybackModeHandler);
    		}else
		#endif
    		{
    			StatePBModeSetOperation((void*)StateMoviePlaybackModeInitLCD, (void*)StateMoviePlaybackModeShutDown, (void *)StateMoviePlaybackModeHandler);
    		}
    }
    else if( JPGPB_MODE == ubUIMode){
		#if (HDMI_ENABLE)
        	if (AHC_IsHdmiConnect()){
        		StatePBModeSetOperation((void*)StatePhotoPlaybackModeInitHDMI, (void*)StatePhotoPlaybackModeShutDown, (void *)StatePhotoPlaybackModeHandler);
        	}else
		#endif
		#if (TVOUT_ENABLE)
    		if (AHC_IsTVConnectEx())
    		{
    			StatePBModeSetOperation((void*)StatePhotoPlaybackModenitTV, (void*)StatePhotoPlaybackModeShutDown, (void *)StatePhotoPlaybackModeHandler);
    		}else
		#endif
    		{
    			StatePBModeSetOperation((void*)StatePhotoPlaybackModeInitLCD, (void*)StatePhotoPlaybackModeShutDown, (void *)StatePhotoPlaybackModeHandler);
    		}
    }
    else if( AUDPB_MODE == ubUIMode){
        StatePBModeSetOperation((void*)StateAudioPlaybackModeInit, (void*)StateAudioPlaybackModeShutDown, (void *)StateAudioPlaybackModeHandler);                    
    }
    else if( JPGPB_MOVPB_MODE == ubUIMode){
        UINT32 uiCurrentIdx;
        UINT8  ubFileType;
        AHC_UF_GetCurrentIndex(&uiCurrentIdx);
        AHC_UF_GetFileTypebyIndex(uiCurrentIdx, &ubFileType);
        if ( ubFileType == DCF_OBG_JPG ){
		#if (HDMI_ENABLE)
        	if (AHC_IsHdmiConnect()){
        		StatePBModeSetOperation((void*)StatePhotoPlaybackModeInitHDMI, (void*)StatePhotoPlaybackModeShutDown, (void *)StatePhotoPlaybackModeHandler);
        	}else
		#endif
		#if (TVOUT_ENABLE)
    		if (AHC_IsTVConnectEx())
    		{
    			StatePBModeSetOperation((void*)StatePhotoPlaybackModenitTV, (void*)StatePhotoPlaybackModeShutDown, (void *)StatePhotoPlaybackModeHandler);
    		}else
		#endif
    		{
    			StatePBModeSetOperation((void*)StatePhotoPlaybackModeInitLCD, (void*)StatePhotoPlaybackModeShutDown, (void *)StatePhotoPlaybackModeHandler);
    		}
        }
        else{
		#if (HDMI_ENABLE)
        	if (AHC_IsHdmiConnect()){
        		StatePBModeSetOperation((void*)StateMoviePlaybackModeInitHDMI, (void*)StateMoviePlaybackModeShutDown, (void *)StateMoviePlaybackModeHandler);
        	}else
		#endif
		#if (TVOUT_ENABLE)
    		if (AHC_IsTVConnectEx())
    		{
    			StatePBModeSetOperation((void*)StateMoviePlaybackModeInitTV, (void*)StateMoviePlaybackModeShutDown, (void *)StateMoviePlaybackModeHandler);
    		}else
		#endif
    		{
    			StatePBModeSetOperation((void*)StateMoviePlaybackModeInitLCD, (void*)StateMoviePlaybackModeShutDown, (void *)StateMoviePlaybackModeHandler);
    		}
        }
    }                
    
    return AHC_TRUE;
}

AHC_BOOL StateSelectFuncPlaybackMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    if(MOVPB_MODE == GetCurrentOpMode())
    {
        ahcRet = StateSelectPBFunc(MOVPB_MODE);
    
        RegisterEventCb(EVENT_KEY_UP,							STATE_MOVPB_MODE_EVENT_KEY_UP);
        RegisterEventCb(EVENT_KEY_DOWN,							STATE_MOVPB_MODE_EVENT_KEY_DOWN);
        RegisterEventCb(EVENT_MOVPB_TOUCH_BKWD_PRESS,			STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_BKWD_PRESS);
        RegisterEventCb(EVENT_MOVPB_TOUCH_FRWD_PRESS,			STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_FRWD_PRESS);
        RegisterEventCb(EVENT_MOVPB_TOUCH_PLAY_PRESS,			STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_PLAY_PRESS);
        RegisterEventCb(EVENT_MOVPB_TOUCH_RETURN,				STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_RETURN);
        RegisterEventCb(EVENT_KEY_MENU,							STATE_MOVPB_MODE_EVENT_KEY_MENU);
        RegisterEventCb(EVENT_MOVPB_TOUCH_PREV_PRESS, 			STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_PREV_PRESS);
        RegisterEventCb(EVENT_MOVPB_TOUCH_NEXT_PRESS, 			STATE_MOVPB_MODE_EVENT_MOVPB_TOUCH_NEXT_PRESS);
        RegisterEventCb(EVENT_VIDEO_PREVIEW,					STATE_MOVPB_MODE_EVENT_VIDEO_PREVIEW);
        RegisterEventCb(EVENT_CAMERA_PREVIEW,					STATE_MOVPB_MODE_EVENT_CAMERA_PREVIEW);
        RegisterEventCb(EVENT_FILE_DELETING,					STATE_MOVPB_MODE_EVENT_FILE_DELETING);
        RegisterEventCb(EVENT_LOCK_FILE_M,						STATE_MOVPB_MODE_EVENT_LOCK_FILE_M);

        RegisterEventCb(EVENT_DC_CABLE_IN,						STATE_MOVPB_MODE_EVENT_DC_CABLE_IN);
        RegisterEventCb(EVENT_DC_CABLE_OUT,						STATE_MOVPB_MODE_EVENT_DC_CABLE_OUT);
        RegisterEventCb(EVENT_USB_DETECT,						STATE_MOVPB_MODE_EVENT_USB_DETECT);
        RegisterEventCb(EVENT_USB_REMOVED,						STATE_MOVPB_MODE_EVENT_USB_REMOVED);
        RegisterEventCb(EVENT_SD_DETECT,						STATE_MOVPB_MODE_EVENT_SD_DETECT);
        RegisterEventCb(EVENT_SD_REMOVED,						STATE_MOVPB_MODE_EVENT_SD_REMOVED);
#if (TVOUT_ENABLE)
        RegisterEventCb(EVENT_TV_DETECT,						STATE_MOVPB_MODE_EVENT_TV_DETECT);
        RegisterEventCb(EVENT_TV_REMOVED,						STATE_MOVPB_MODE_EVENT_TV_REMOVED);
#endif
#if (HDMI_ENABLE)
        RegisterEventCb(EVENT_HDMI_DETECT,						STATE_MOVPB_MODE_EVENT_HDMI_DETECT);
        RegisterEventCb(EVENT_HDMI_REMOVED,						STATE_MOVPB_MODE_EVENT_HDMI_REMOVED);
#endif
#if (UVC_HOST_VIDEO_ENABLE)
		RegisterEventCb(EVENT_USB_B_DEVICE_DETECT,				STATE_PLAYBACK_MODE_EVENT_USB_B_DEVICE_DETECT);
		RegisterEventCb(EVENT_USB_B_DEVICE_REMOVED, 			STATE_PLAYBACK_MODE_EVENT_USB_B_DEVICE_REMOVED);
#endif
        RegisterEventCb(EVENT_MOVPB_UPDATE_MESSAGE,				STATE_MOVPB_MODE_EVENT_MOVPB_UPDATE_MESSAGE);

        RegisterEventCb(EVNET_SUB_MODE_ENTER,					STATE_MOVPB_MODE_EVENT_SUB_MODE_ENTER);
        RegisterEventCb(EVNET_SUB_MODE_EXIT,					STATE_MOVPB_MODE_EVENT_SUB_MODE_EXIT);
    #if defined(WIFI_PORT) && (WIFI_PORT == 1) 
        RegisterEventCb(EVENT_NET_ENTER_PLAYBACK,				STATE_MOVPB_MODE_EVENT_ENTER_NET_PLAYBACK);
    #endif
    }
    else if (JPGPB_MODE == GetCurrentOpMode())
    {
        ahcRet = StateSelectPBFunc(JPGPB_MODE);

        RegisterEventCb(EVENT_KEY_UP,							STATE_PHOTOPB_MODE_EVENT_KEY_UP);
        RegisterEventCb(EVENT_KEY_DOWN,							STATE_PHOTOPB_MODE_EVENT_KEY_DOWN);
        RegisterEventCb(EVENT_KEY_LEFT,							STATE_PHOTOPB_MODE_EVENT_KEY_LEFT);
        RegisterEventCb(EVENT_KEY_RIGHT,						STATE_PHOTOPB_MODE_EVENT_KEY_RIGHT);
        RegisterEventCb(EVENT_KEY_SET,							STATE_PHOTOPB_MODE_EVENT_KEY_SET);
        RegisterEventCb(EVENT_JPGPB_TOUCH_RETURN,				STATE_PHOTOPB_MODE_EVENT_JPGPB_TOUCH_RETURN);
        RegisterEventCb(EVENT_KEY_MENU,							STATE_PHOTOPB_MODE_EVENT_KEY_MENU);
        RegisterEventCb(EVENT_JPGPB_TOUCH_PREV_PRESS,			STATE_PHOTOPB_MODE_EVENT_JPGPB_TOUCH_PREV_PRESS);
        RegisterEventCb(EVENT_JPGPB_TOUCH_NEXT_PRESS,			STATE_PHOTOPB_MODE_EVENT_JPGPB_TOUCH_NEXT_PRESS);
        RegisterEventCb(EVENT_VIDEO_PREVIEW,					STATE_PHOTOPB_MODE_EVENT_VIDEO_PREVIEW);
        RegisterEventCb(EVENT_CAMERA_PREVIEW,					STATE_PHOTOPB_MODE_EVENT_CAMERA_PREVIEW);
        RegisterEventCb(EVENT_FILE_DELETING,					STATE_PHOTOPB_MODE_EVENT_FILE_DELETING);
        RegisterEventCb(EVENT_LOCK_FILE_M,						STATE_PHOTOPB_MODE_EVENT_LOCK_FILE_M);

        RegisterEventCb(EVENT_DC_CABLE_IN,						STATE_PHOTOPB_MODE_EVENT_DC_CABLE_IN);
        RegisterEventCb(EVENT_DC_CABLE_OUT,						STATE_PHOTOPB_MODE_EVENT_DC_CABLE_OUT);
        RegisterEventCb(EVENT_USB_DETECT,						STATE_PHOTOPB_MODE_EVENT_USB_DETECT);
        RegisterEventCb(EVENT_USB_REMOVED,						STATE_PHOTOPB_MODE_EVENT_USB_REMOVED);
        RegisterEventCb(EVENT_SD_DETECT,						STATE_PHOTOPB_MODE_EVENT_SD_DETECT);
        RegisterEventCb(EVENT_SD_REMOVED,						STATE_PHOTOPB_MODE_EVENT_SD_REMOVED);
#if (TVOUT_ENABLE)
        RegisterEventCb(EVENT_TV_DETECT,						STATE_PHOTOPB_MODE_EVENT_TV_DETECT);
        RegisterEventCb(EVENT_TV_REMOVED,						STATE_PHOTOPB_MODE_EVENT_TV_REMOVED);
#endif
#if (HDMI_ENABLE)
        RegisterEventCb(EVENT_HDMI_DETECT,						STATE_PHOTOPB_MODE_EVENT_HDMI_DETECT);
        RegisterEventCb(EVENT_HDMI_REMOVED,						STATE_PHOTOPB_MODE_EVENT_HDMI_REMOVED);
#endif
#if (UVC_HOST_VIDEO_ENABLE)
		RegisterEventCb(EVENT_USB_B_DEVICE_DETECT,				STATE_PLAYBACK_MODE_EVENT_USB_B_DEVICE_DETECT);
		RegisterEventCb(EVENT_USB_B_DEVICE_REMOVED, 			STATE_PLAYBACK_MODE_EVENT_USB_B_DEVICE_REMOVED);
#endif
        RegisterEventCb(EVENT_JPGPB_UPDATE_MESSAGE,				STATE_PHOTOPB_MODE_EVENT_JPGPB_UPDATE_MESSAGE);

        RegisterEventCb(EVNET_SUB_MODE_ENTER,					STATE_PHOTOPB_MODE_EVENT_SUB_MODE_ENTER);
        RegisterEventCb(EVNET_SUB_MODE_EXIT,					STATE_PHOTOPB_MODE_EVENT_SUB_MODE_EXIT);  
    #if defined(WIFI_PORT) && (WIFI_PORT == 1) 
        RegisterEventCb(EVENT_NET_ENTER_PLAYBACK,				STATE_PHOTOPB_MODE_EVENT_ENTER_NET_PLAYBACK);
    #endif      
    }
    
    StateModeSetOperation(UI_PLAYBACK_STATE, StateCommonPBModeInit, StateCommonPBModeShutDown, StateCommonPBModeHandler);                    

    ahcRet =  SwitchUIDrawSetPlaybackMode();
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return ahcRet;} 

    return ahcRet;    
}

#if (DSC_MODE_ENABLE)
AHC_BOOL StateSelectFuncCameraMenuMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    
    //StateModeSetOperation(UI_CAMERA_MENU_STATE, (void*)MenuStateCameraModeInit, (void*)MenuStateCameraModeShutDown, (void *)MenuStateCameraModeHandler);                                        
    ahcRet = StateModeSetOperation(UI_CAMERA_MENU_STATE, MenuStateCameraModeInit, MenuStateCameraModeShutDown, MenuStateHandlerCommon);
#if 0
    RegisterEventCb(EVENT_TV_DETECT,						STATE_CAMERA_MENU_EVENT_TV_DETECT);
    RegisterEventCb(EVENT_TV_REMOVED,						STATE_CAMERA_MENU_EVENT_TV_REMOVED);
    RegisterEventCb(EVENT_HDMI_DETECT,						STATE_CAMERA_MENU_EVENT_HDMI_DETECT);
    RegisterEventCb(EVENT_HDMI_REMOVED,						STATE_CAMERA_MENU_EVENT_HDMI_REMOVED);
#endif    
    return ahcRet;    
}
#endif

AHC_BOOL StateSelectFuncVideoRecordMenuMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
	printc("%s,%d \n", __func__, __LINE__); // long 4-27

    //ahcRet = StateModeSetOperation(UI_VIDEO_MENU_STATE, (void*)MenuStateVideoModeInit, (void*)MenuStateVideoModeShutDown, (void *)MenuStateVideoModeHandler);                                        
    ahcRet = StateModeSetOperation(UI_VIDEO_MENU_STATE, MenuStateVideoModeInit, MenuStateVideoModeShutDown, MenuStateHandlerCommon);
    RegisterEventCb(EVENT_DROP, 			 STATE_COMMON_DROP_EVENT);  //啥都没有做

    return ahcRet;    
}

AHC_BOOL StateSelectFuncEditToolMenuMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
       
    ahcRet = StateModeSetOperation(UI_EDIT_TOOL_MENU_STATE, MenuStateEditToolModeInit, MenuStateEditConfirmModeShutDown, MenuStateHandlerCommon);                                        
      
    return ahcRet;    
}

AHC_BOOL StateSelectFuncPlaybackMenuMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;

    //ahcRet = StateModeSetOperation(UI_PLAYBACK_MENU_STATE, (void*)MenuStatePlaybackModeInit, (void*)MenuStatePlaybackModeShutDown, (void *)MenuStatePlaybackModeHandler);                                        
    ahcRet = StateModeSetOperation(UI_PLAYBACK_MENU_STATE, MenuStatePlaybackModeInit, MenuStatePlaybackModeShutDown, MenuStateHandlerCommon);                                        
      
    return ahcRet;    
}

AHC_BOOL StateSelectFuncSDUpdateFWMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    #if (SD_UPDATE_FW_EN)
    ahcRet = StateModeSetOperation(UI_SD_UPDATE_STATE, MenuStateSDUpdateModeInit, MenuStateSDUpdateModeShutDown, MenuStateSDUpdateModeHandler);                                        
    #endif
    return ahcRet;    
}

#if (POWER_ON_MENU_SET_EN)
AHC_BOOL StateSelectFuncPowerOnSetMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;

    ahcRet = StateModeSetOperation(UI_POWERON_SET, (void*)MenuStatePowerOnSettingModeInit, (void*)MenuStatePowerOnSettingModeShutDown, (void *)MenuStatePowerOnSettingModeHandler);                                                    
        
    return ahcRet;    
}
#endif

#if defined(WIFI_PORT) && (WIFI_PORT == 1)
void uiStateRegisterCommonCgiHandlers(void)
{
    uiStateRegisterNetEventHandler(WIRELESS_SET_FLICKER,  STATE_NET_CGICMD_WIRELESS_SET_FLICKER);
    uiStateRegisterNetEventHandler(WIRELESS_SET_AWB,      STATE_NET_CGICMD_WIRELESS_SET_AWB);
    uiStateRegisterNetEventHandler(WIRELESS_SET_EV, STATE_NET_CGICMD_WIRELESS_SET_EV);
    uiStateRegisterNetEventHandler(WIRELESS_SET_MOVIE_SIZE, STATE_NET_CGICMD_WIRELESS_SET_MOVIE_SIZE);
    uiStateRegisterNetEventHandler(WIRELESS_SET_CLIP_TIME, STATE_NET_CGICMD_WIRELESS_SET_CLIP_TIME);
    uiStateRegisterNetEventHandler(WIRELESS_SET_VQUALITY, STATE_NET_CGICMD_WIRELESS_SET_VQUALITY);
    uiStateRegisterNetEventHandler(WIRELESS_SET_IMAGE_SIZE, STATE_NET_CGICMD_WIRELESS_SET_IMAGE_SIZE);
    uiStateRegisterNetEventHandler(WIRELESS_SET_POWROFF_DELAY, STATE_NET_CGICMD_WIRELESS_SET_POWROFF_DELAY);
    uiStateRegisterNetEventHandler(WIRELESS_SET_AUTO_PWROFF_TIME, STATE_NET_CGICMD_WIRELESS_SET_AUTO_PWROFF_TIME);
    uiStateRegisterNetEventHandler(WIRELESS_SET_MTD, STATE_NET_CGICMD_WIRELESS_SET_MTD);
    uiStateRegisterNetEventHandler(WIRELESS_SET_TIME_FMT, STATE_NET_CGICMD_WIRELESS_SET_TIME_FMT);
    uiStateRegisterNetEventHandler(WIRELESS_SET_TV_SYS, STATE_NET_CGICMD_WIRELESS_SET_TV_SYS);
    uiStateRegisterNetEventHandler(WIRELESS_SET_LCD_PWRS, STATE_NET_CGICMD_WIRELESS_SET_LCD_PWRS);
    uiStateRegisterNetEventHandler(WIRELESS_SET_PHOTOBURST, STATE_NET_CGICMD_WIRELESS_SET_PHOTOBURST);
    uiStateRegisterNetEventHandler(WIRELESS_SET_TIMELAPSE, STATE_NET_CGICMD_WIRELESS_SET_TIMELAPSE);
    uiStateRegisterNetEventHandler(WIRELESS_SET_UPSIDEDOWN, STATE_NET_CGICMD_WIRELESS_SET_UPSIDEDOWN);
    uiStateRegisterNetEventHandler(WIRELESS_SET_SPOTMETER, STATE_NET_CGICMD_WIRELESS_SET_SPOTMETER);
    uiStateRegisterNetEventHandler(WIRELESS_SET_HDR, STATE_NET_CGICMD_WIRELESS_SET_HDR);
    uiStateRegisterNetEventHandler(WIRELESS_SET_Q_SHOT, STATE_NET_CGICMD_WIRELESS_SET_Q_SHOT);
    uiStateRegisterNetEventHandler(WIRELESS_SET_STATUSLIGHTS, STATE_NET_CGICMD_WIRELESS_SET_STATUSLIGHTS);
    uiStateRegisterNetEventHandler(WIRELESS_SET_SOUNDINDICATOR, STATE_NET_CGICMD_WIRELESS_SET_SOUNDINDICATOR);
    uiStateRegisterNetEventHandler(WIRELESS_SET_POWERSAVING, STATE_NET_CGICMD_WIRELESS_SET_POWERSAVING);
    uiStateRegisterNetEventHandler(WIRELESS_SET_UIMODE, STATE_NET_CGICMD_WIRELESS_SET_UIMODE);
    uiStateRegisterNetEventHandler(WIRELESS_SET_LOOPINGVIDEO, STATE_NET_CGICMD_WIRELESS_SET_LOOPINGVIDEO);
	uiStateRegisterNetEventHandler(WIRELESS_SET_GSENSOR_SENSITIVITY, STATE_NET_CGICMD_WIRELESS_SET_GSENSOR);
	uiStateRegisterNetEventHandler(WIRELESS_SET_REC_MUTE, STATE_NET_CGICMD_WIRELESS_SET_MUTE);
}

///common handler of the network message
UINT32 uiStateNetProcessMsg(UI_KEYPARSER_FUNC keyParser, UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam)
{
    UINT32 ulOperation = 0;
    AHC_BOOL    ahcRet = AHC_TRUE;
    AHC_BOOL	bIsFromNet = AHC_FALSE;
    AHC_BOOL	bNeedCgiFeedback = AHC_FALSE;
    UI_STATE_ID sParentState;
#if DBG_UI_NET_EVENT
    if (ulMsgId != AHLM_UI_NOTIFICATION && ulEvent != BUTTON_UPDATE_MESSAGE)
    	printc("%s:%x,%x,%x\r\n", __func__, ulMsgId, ulEvent, ulParam);
#endif
    if(ulMsgId == AHLM_WIRELESS_CONNECT || ulMsgId == AHLM_NET) {
    	bIsFromNet = AHC_TRUE;
	#if DBG_UI_NET_EVENT
    	if (ulMsgId == AHLM_WIRELESS_CONNECT) printc(" =CONN(%X)= ", ulMsgId);
    	else printc (" =NET(%X)= ", ulMsgId);
	#endif
		if (ncgi_is_need_feedback()) {
		#if DBG_UI_NET_EVENT
			printc(" [need FB] ");
		#endif
			bNeedCgiFeedback = AHC_TRUE;
			CGI_SET_STATUS(ulEvent, CGI_ERR_NOT_IMPLEMENTED);
		}
    }

    if (keyParser == NULL) {
    	AHC_PRINT_RET_ERROR(0, -1);
    	return -1;
    }
    ulOperation = keyParser(ulMsgId, ulEvent, ulParam);
    if (ulOperation == 0) {
    	printd(BG_RED("Msg:%X ulEvent:%X ulOperation:%X")"\r\n", ulMsgId, ulEvent, ulOperation);
    }
#if DBG_UI_NET_EVENT
    if (bIsFromNet)
    	printd(BG_BLUE("Msg:%X ulEvent:%X ulOperation:%X")"\r\n", ulMsgId, ulEvent, ulOperation);
#endif

    //Not WiFi event.
    if(WIRELESS_NOT_NET_EVENT == ulOperation){
        StateModeGetParent(uiGetWorkingState(), &sParentState);
        if(sParentState != UI_STATE_UNSUPPORTED){
			if(uiGetCurrentState() == UI_NET_STREAMING_STATE){
				KeyParser_NetStreaming2ParentEvent(&ulMsgId, &ulEvent, &ulParam);
			}
			else{
	            KeyParser_NetCGICmd2ParentEvent(&ulMsgId, &ulEvent, &ulParam);

	            //Switch mode. Detach CGI cmd mode.
	            if(BUTTON_MODE_REL == ulEvent){
	                ahcRet = StateDetachSubMode();
	                if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); /*return ahcRet;*/}
	            }
			}
            //Pass event to parent UI state.
		#if DBG_UI_NET_EVENT
            if (ulMsgId != AHLM_UI_NOTIFICATION && ulEvent != BUTTON_UPDATE_MESSAGE)
            	printd(BG_BLUE("parent state:%X ulMsg:%X Event:%X param:%X")"\r\n", sParentState, ulMsgId, ulEvent, ulParam);
		#endif
            ahcRet = StateModeDoHandler(sParentState, ulMsgId, ulEvent, ulParam);
            if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); /*return ahcRet;*/}
        }
    }
    else { //is WiFi event
    	if (bIsFromNet) {
    		ahcRet = StateModeDoHandlerEvent(ulMsgId, ulOperation, ulParam);
    		if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); /*return ahcRet;*/}
    	}
    }

    if(bNeedCgiFeedback) {
    	if (ahcRet) {
    		CGI_SET_STATUS(ulEvent, CGI_ERR_NONE);
    	}
    	CGI_FEEDBACK();
    }

    return 0;
}

AHC_BOOL StateSelectFuncNetFWUpdateMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;

    //ahcRet = StateModeSetOperation(UI_NET_FWUPDATE_STATE, (void*)MenuStatePowerOnSettingModeInit, (void*)MenuStatePowerOnSettingModeShutDown, (void *)MenuStatePowerOnSettingModeHandler);                                                    
        
    return ahcRet;    
}

AHC_BOOL StateSelectFuncNetCGICmdMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;

    ahcRet = StateModeSetOperation(UI_NET_CGICMD_STATE, StateNetCGICmdModeInit, StateNetCGICmdModeShutDown, StateNetCGICmdModeHandler);                                                    

    uiStateRegisterCommonCgiHandlers();

    return ahcRet;    
}
#endif

AHC_BOOL StateSelectFunc(UI_STATE_ID mState)
{
    AHC_BOOL    ahcRet = AHC_TRUE;

    ahcRet = SwitchDrawOSDIDFunc();
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

    switch( mState ){
        case UI_VIDEO_STATE:
			StateModeClearEvent();
            ahcRet = StateSelectFuncVideoRecordMode();
            break;

#if (DSC_MODE_ENABLE)
        case UI_CAMERA_STATE:
			StateModeClearEvent();
            ahcRet = StateSelectFuncCameraMode();
            break;
#endif

        case UI_BROWSER_STATE:
			StateModeClearEvent();
            ahcRet = StateSelectFuncBrowserMode();            
            break;
                
        case UI_PLAYBACK_STATE:
			StateModeClearEvent();
            ahcRet = StateSelectFuncPlaybackMode();
            break;
            
        case UI_PCCAM_STATE:
        case UI_MSDC_STATE:
            ahcRet = StateSelectFuncUSB_MSDC_PCAM_Mode();            
            break;

#if (DSC_MODE_ENABLE)
        case UI_CAMERA_MENU_STATE:
            ahcRet = StateSelectFuncCameraMenuMode();
#endif            
            break;

        case UI_VIDEO_MENU_STATE:
            ahcRet = StateSelectFuncVideoRecordMenuMode();
            break;

        case UI_PLAYBACK_MENU_STATE:
            ahcRet = StateSelectFuncPlaybackMenuMode();
            break;
			
		case UI_EDIT_TOOL_MENU_STATE:
            ahcRet = StateSelectFuncEditToolMenuMode();
            break;

        #if (SD_UPDATE_FW_EN)
        case UI_CLOCK_SETTING_STATE:
            ahcRet = StateSelectFuncClockSettingMode();          
            break;
        #endif    
        
#if (SD_UPDATE_FW_EN)
        case UI_SD_UPDATE_STATE:
            ahcRet = StateSelectFuncSDUpdateFWMode();            
            break;
#endif
    
#if (USB_MODE_SELECT_EN)
        case UI_USBSELECT_MENU_STATE:
            ahcRet = StateSelectFuncUSBSelectMenuMode();
            break;
#endif

#if (POWER_ON_MENU_SET_EN)
        case UI_POWERON_SET:
            ahcRet = StateSelectFuncPowerOnSetMode();
            break;
#endif

#if defined(WIFI_PORT) && (WIFI_PORT == 1)
        case UI_NET_STREAMING_STATE:
            ahcRet = StateSelectFuncNetStreamingMode();
            break;
            
        case UI_NET_PLAYBACK_STATE: 
            ahcRet = StateSelectFuncNetPlaybackMode();
            break;        
            
        case UI_NET_FWUPDATE_STATE: 
            ahcRet = StateSelectFuncNetFWUpdateMode();        
            break;                             

        case UI_NET_CGICMD_STATE:
            ahcRet = StateSelectFuncNetCGICmdMode();
            break;
#endif

        default:
            //AHC_SetMode(AHC_MODE_IDLE);
            AHC_PRINT_RET_ERROR(0,0);
            return AHC_FALSE;
        break;
    }
    
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

    return ahcRet;
}

#if CR_USE_STATE_SWITCH_SUB_MODE
/** @brief Transit UI State from current mode to new mode.
 *
 * This function detach the current mode and transit to new one
 * @param[in] mState The new state to be transit.
 * @param[in] funcName The function name of the caller
 * @param[in] line The line number of the caller
 */
void _StateReplaceSubMode(UI_STATE_ID mState, const char *funcName, UINT32 line)
{
	AHC_BOOL ahcRet;
	UI_STATE_ID ubParentUIState = UI_STATE_NUM;

	printc("~~~~~~~~~~long~~%s,%d, DettachSubMode\r\n", __func__, __LINE__); // long 4-28

	StateModeGetParent(uiGetCurrentState(), &ubParentUIState);

	printc("~~~~~~~~~~long~~%s,%d, DettachSubMode:%d\r\n", __func__, __LINE__,uiGetCurrentState()); // long 4-28  1
	printc("~~~~~~~~~~long~ubParentUIState~%d\r\n",ubParentUIState); // long 4 -28   25
    if(UI_STATE_UNSUPPORTED != ubParentUIState){
        printc("%s,%d, DettachSubMode:%d\r\n", __func__, __LINE__, uiGetCurrentState());
        ahcRet = StateDetachSubMode();
        if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0, ahcRet);}
    }
    ahcRet = StateAttachSubMode(mState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); /*return ahcRet;*/}
}
#endif

AHC_BOOL StateAttachSubMode(UI_STATE_ID mState)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    UI_STATE_ID sParentUIStateID = UI_STATE_UNSUPPORTED;
    
#if DBG_UI_NET_EVENT
    {
    	INT16U AHC_DBG_GetCurPrioEx(void);
    	UINT16 prio = AHC_DBG_GetCurPrioEx();
    	if (prio != TASK_AHC_INIT_PRIO) {
    		printd(FG_YELLOW("[WARN] AttachSubMode is called in task %d")"\r\n", prio);
    	}
    }
#endif
    if (mState >= UI_STATE_NUM){
        return AHC_FALSE;
    }

    //sCurrentUIStateID = uiGetCurrentState();
    if(uiSysState.CurrentState== mState){
        printc("Error! Parent state is the same as child state.\r\n");
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0);
        return AHC_FALSE;
    }

    StateModeGetParent(uiSysState.CurrentState, &sParentUIStateID);

	printc("~~~~~~~~~~long~ubParentUIState~%d\r\n",sParentUIStateID); // long 4 -28   25
    if(UI_STATE_UNSUPPORTED != sParentUIStateID){

        printc("Error! Can not attach sub mode in another sub mode.\r\n");
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0);
        return AHC_FALSE;
    }
    
    ahcRet = StateSelectFunc(mState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

    StateModeSetParent(mState, uiSysState.CurrentState);

    uiSysState.LastState = uiSysState.CurrentState;
    uiSysState.CurrentState = mState;

    //Do not shut down parent UI state.
    //ahcRet = StateModeDoShutdown(uiSysState.LastState);
    //if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 
    
    ahcRet = StateModeDoHandler(uiSysState.LastState, AHLM_UI_NOTIFICATION, BUTTON_SUB_MODE_ENTER, (UINT32)mState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); /*return ahcRet;*/} 
    
    ahcRet = StateModeDoInit(uiSysState.CurrentState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 
    

    return ahcRet;    
}

AHC_BOOL StateDetachSubMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    UI_STATE_ID sParentUIStateID;
    

//	printc("~~~~~~~~~~long~~%s,%d, DettachSubMode:%d\r\n", __func__, __LINE__); // long 4-28

    StateModeGetParent(uiSysState.CurrentState, &sParentUIStateID);

    if(UI_STATE_UNSUPPORTED == sParentUIStateID){
        printc("Error! Parent state is empty.\r\n");
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0);
        return AHC_FALSE;
    }

    //ahcRet = StateSelectFunc(mState);
    //if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

    StateModeSetParent(uiSysState.CurrentState, UI_STATE_UNSUPPORTED);

    uiSysState.LastState = uiSysState.CurrentState;
    uiSysState.CurrentState = sParentUIStateID;

    ahcRet = StateModeDoShutdown(uiSysState.LastState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

    ahcRet = StateModeDoHandler(uiSysState.CurrentState, AHLM_UI_NOTIFICATION, BUTTON_SUB_MODE_EXIT, (UINT32)uiSysState.LastState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); /*return ahcRet;*/} 
        
    //ahcRet = StateModeDoInit(uiSysState.CurrentState);
    //if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

    return ahcRet;
}

/** @brief If there are more than one sub state attached at the same time, call this function to detach middle sub state.
 *
 * This function detach the working sub state then connect another sub state to the parent.
 * For example, now state-chain is "Video menu" -> "Wifi CGI" -> "Video record" and you want to detach "Wifi CGI" sub state. 
 * After StateDetachWorkingSubMode is called, the state-chain will be "Video menu" -> "Video record".
 *
 * @param[in] mState The target sub state that will be detached.
 */
AHC_BOOL StateDetachWorkingSubMode(UI_STATE_ID mState)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    UI_STATE_ID sTmpState = UI_STATE_UNSUPPORTED;
    UI_STATE_ID sParentUIStateID;
    MMP_SHORT  usStateLayerMaxCount = 5;

    if(mState == uiSysState.CurrentState){
        return StateDetachSubMode();
    }
    
    sTmpState = uiSysState.CurrentState;

    //Find sub state above mState
    do{
        StateModeGetParent(sTmpState, &sParentUIStateID); 
        usStateLayerMaxCount--;
    }while((sParentUIStateID != mState) && (sParentUIStateID != UI_STATE_UNSUPPORTED) && (usStateLayerMaxCount >=0));
    
    if(UI_STATE_UNSUPPORTED == sParentUIStateID){
        printc("Error! Can not find woking sub mode.\r\n");
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0);
        return AHC_FALSE;
    }

    StateModeGetParent(mState, &sParentUIStateID); 
    if(UI_STATE_UNSUPPORTED == sParentUIStateID){
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0);
        return AHC_FALSE;
    }
    
    StateModeSetParent(sTmpState, sParentUIStateID);
    StateModeSetParent(mState, UI_STATE_UNSUPPORTED);

    ahcRet = StateModeDoShutdown(mState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

    ahcRet = StateModeDoHandler(sParentUIStateID, AHLM_UI_NOTIFICATION, BUTTON_SUB_MODE_EXIT, (UINT32)mState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); /*return ahcRet;*/} 
        
    return ahcRet;
}

AHC_BOOL _StateSwitchSubMode(UI_STATE_ID mState)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    UI_STATE_ID sParentUIStateID;

    StateModeGetParent(uiSysState.CurrentState, &sParentUIStateID);
    if(UI_STATE_UNSUPPORTED == sParentUIStateID){
        printc("Error! Parent state is empty.\r\n");
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0);
        return AHC_FALSE;
    }

    //ahcRet = StateSelectFunc(mState);
    //if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

    //Dettach current sub mode to the parent.
    StateModeSetParent(uiSysState.CurrentState, UI_STATE_UNSUPPORTED);

    //Attach new sub mode to the parent.
    StateModeSetParent(mState, sParentUIStateID);

    uiSysState.LastState = uiSysState.CurrentState;
    uiSysState.CurrentState = mState;

    ahcRet = StateModeDoShutdown(uiSysState.LastState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 
        
    ahcRet = StateModeDoInit(uiSysState.CurrentState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

    return ahcRet;
}

AHC_BOOL StateSwitchMode(UI_STATE_ID mState)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    if (mState >= UI_STATE_NUM)
        return AHC_FALSE;

    if(uiSysState.CurrentState == mState){
        printc(FG_RED("Warning! StateSwitchMode state is the same.\r\n"));
        //return AHC_TRUE;
    }

    //uiSysState.LastState = uiSysState.CurrentState;

    CHARGE_ICON_ENABLE(AHC_FALSE);

#ifdef SLIDE_STRING
    StopSlideString();
#endif
    
    ahcRet = StateSelectFunc(mState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

    
    ahcRet = StateModeDoShutdown(uiSysState.CurrentState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 
    
    if(m_StateOP[uiSysState.CurrentState].sParentUIState != UI_STATE_UNSUPPORTED) {
    	printc(FG_PURPLE("Switch to UI Mode %d : there is a attached parent mode %d!\n"),
    			mState,m_StateOP[uiSysState.CurrentState].sParentUIState);
    	ahcRet = StateModeDoShutdown(m_StateOP[uiSysState.CurrentState].sParentUIState);
    	if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 
    }
    
    uiSysState.LastState = uiSysState.CurrentState;
    uiSysState.CurrentState = mState;
    
    ahcRet = StateModeDoInit(uiSysState.CurrentState);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 

#if 0
    if (m_StateOP[uiSysState.LastState].pfShutDownFunc != NULL) {
        ahcRet = m_StateOP[uiSysState.LastState].pfShutDownFunc(NULL);
        if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 
    }

    if (m_StateOP[uiSysState.CurrentState].pfInitFunc != NULL){
        ahcRet = m_StateOP[uiSysState.CurrentState].pfInitFunc(NULL);
        if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;}         
    }
#endif

    //SwitchUIDrawSet(mState);
    CHARGE_ICON_ENABLE(AHC_TRUE);
    
    return ahcRet;
}

void uiStateSystemStart(void)
{
    AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, 0, 0);
}

void uiCheckDefaultMenuExist(void)
{
    MenuInfo *useMenu;
    MenuInfo *defMenu;
    AHC_BOOL ret;

    printc("### %s -\r\n", __func__);

    useMenu = MenuSettingConfig();
    defMenu = DefaultMenuSettingConfig();

#ifdef CFG_MENU_SETTING_USING_PROGRAM_DEFAULT
    DefaultMenuSettingInit();       // Set setting as program default
    CheckMenuSetting(defMenu);
    memcpy(useMenu, defMenu, sizeof(MenuInfo));
    useMenu->uiBatteryVoltage = BATTERY_VOLTAGE_FULL;
#else
    ret = AHC_PARAM_Menu_Read((UINT8*)defMenu, AHC_DEFAULT_FACTORY_FAT);

    if (!ret) {
        // No default setting file
        printc("###############################\r\n"
               "--E-- No Default Settings\r\n"
               "###############################\r\n");

        DefaultMenuSettingInit();       // Set setting as program default
        CheckMenuSetting(defMenu);
        RestoreFromDefaultSetting();    // Set setting as current
        AHC_PARAM_Menu_Write(AHC_DEFAULT_USER_FAT); // Write to file
        Menu_WriteLcdSnrTVHdmiStauts2SF();
    }

    ret = AHC_PARAM_Menu_Read((UINT8*)useMenu, AHC_DEFAULT_USER_FAT);

    if (!ret) {
        printc("###############################\r\n"
                "--E-- No User Settings\r\n"
                "###############################\r\n");

        printc(FG_RED("Format Partition 2\r\n"));
		AHC_FormatStorageMedia(AHC_MEDIA_USER_FAT);			
        
        memcpy(useMenu, defMenu, sizeof(MenuInfo));
        // Make the battery is not 0, otherwise it will show battery warning at
        // the first time power on after FW updated
        useMenu->uiBatteryVoltage = BATTERY_VOLTAGE_FULL;

        CheckMenuSetting(useMenu);
        // Write to User setting file in SF
        AHC_PARAM_Menu_Write(AHC_DEFAULT_USER_FAT);
        Menu_WriteLcdSnrTVHdmiStauts2SF();
    }
#endif

    ApplyUserTextSetting("SD:0:\\usercfg.txt", useMenu);
    CheckMenuSetting(useMenu);
    Menu_Setting_BackupMenuAtoms();
    #if defined(TXT_MENU_WRITE_TO_SD)
    ExportMenuAtom((char*)"SD:0:\\usercfg.txt", TXT_COMMENT | TXT_VERSION | TXT_MENU_ACT);
    #endif

    // Initial ACC_DATE_STAMP
    MenuItemDateTimeFormatToAhcTimeStampSetting();
#if(SUPPORT_ADAS)
	#ifdef CUS_ADAS_PARA
	MMPS_ADAS_CustomerInitPara(LDWS_ALARM_DELAY_TIME,FCWS_ALARM_RESET_TIME,FCWS_ALARM_SPEED_THD
									FCWS_ALARM_TTC_THD_1,FCWS_ALARM_TTC_THD_2
									FCWS_ALARM_DIST_THD_1,FCWS_ALARM_DIST_THD_2);
	MMPS_ADAS_CustomerSpecifyPara(ADAS_FOCAL_LENGTH,ADAS_PIXEL_SIZE,
									ADAS_DZOOM_N,ADAS_DZOOM_M);
	#endif

    LDWS_CalibrationParam_UserUpdate();
    LDWS_CalibrationParam_Read();
    #ifdef BACKUP_LDWS_CALIBRATION_FILE
    LDWS_CalibrationParam_Backup();
    #endif
#endif
#if (SUPPORT_EIS)
	MMPF_EIS_CustomerSpecifyPara(ADAS_FOCAL_LENGTH);
#endif
}

void VideoSettingInit( void )
{
    AHC_AUDIO_FORMAT aformat;

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
#elif (VR_AUDIO_TYPE==VR_AUDIO_TYPE_PCM)
    aformat = AHC_MOVIE_AUDIO_CODEC_PCM;    
#endif
        
    AHC_VIDEO_ConfigAudio(AHC_AUDIO_STREAM_ID, aformat, AHC_AUDIO_CHANNEL_MONO_R);

    #if (GSENSOR_CONNECT_ENABLE)
    // Since the default setting of ubStoreFreq = 4, and it makes problems with below formula in StateVideoRecMode()::case EVENT_GPSGSENSOR_UPDATE:
    // if( 0 == (ulCounterForGpsGsnrUpdate % ((1000/VIDEO_TIMER_UNIT)/(AHC_Gsensor_GetAttributeAddr()->ubStoreFreq))))
    //   AHC_Gsensor_SetCurInfo();
    // So, change the setting to 5.
    AHC_Gsensor_GetAttributeAddr()->ubStoreFreq = 5;  //Store 5 times GSensor data per second
    #endif

    //Set ADAS parameter
    {
        UINT32 bLDWS_En = LDWS_EN_OFF;
        UINT32 bFCWS_En = FCWS_EN_OFF;
        UINT32 bSAG_En  = SAG_EN_OFF;
        UINT32 uiAdasFlag = 0;
        
        if ( AHC_Menu_SettingGetCB( (char *)COMMON_KEY_LDWS_EN, &bLDWS_En ) == AHC_TRUE ) {
            if (bLDWS_En == LDWS_EN_ON ) {
                uiAdasFlag |= AHC_ADAS_ENABLED_LDWS;
            }
        }

        if ( AHC_Menu_SettingGetCB( (char *)COMMON_KEY_FCWS_EN, &bFCWS_En ) == AHC_TRUE ) {
            if (bFCWS_En == FCWS_EN_ON ) {
                uiAdasFlag |= AHC_ADAS_ENABLED_FCWS;
            }
        }

        if ( AHC_Menu_SettingGetCB( (char *)COMMON_KEY_SAG_EN, &bSAG_En ) == AHC_TRUE ) {
            if (bSAG_En == SAG_EN_ON ) {
                uiAdasFlag |= AHC_ADAS_ENABLED_SAG;
            }
        }
        
        AHC_SetParam( PARAM_ID_ADAS, uiAdasFlag );
    }

}

void CheckRtcAndAutoSetRtc(void)
{
	UINT16		uwYear = 0, uwMonth = 0, uwDay,uwDayInWeek,uwHour,uwMinute,uwSecond;
	UINT8		 ubAmOrPm, b_12FormatEn;

	AHC_SetParam(PARAM_ID_USE_RTC, 1);
	AHC_RTC_UpdateTime();
	AHC_GetClock( &uwYear, &uwMonth, &uwDay, &uwDayInWeek, &uwHour, &uwMinute, &uwSecond, &ubAmOrPm, &b_12FormatEn); 

	if (!AHC_Validate_Year(uwYear))
	{
		AHC_RestoreDefaultTime(&uwYear, &uwMonth, &uwDay, &uwDayInWeek, &uwHour, &uwMinute, &uwSecond, &ubAmOrPm, &b_12FormatEn);
		AHC_SetClock(uwYear, uwMonth, uwDay, uwDayInWeek, uwHour, uwMinute, uwSecond, ubAmOrPm, b_12FormatEn);
	}
}

static void uiStateInitialize(void)
{
    AHC_BOOL    runOp  = AHC_FALSE;
#if WIFI_PORT
    AHC_BOOL    wifiOn;
    extern UINT32 ulLastWiFiChagneTime;
#endif
	MMP_ULONG i;    
    SystemSettingInit();
    VideoSettingInit();

#if WIFI_PORT
    wifiOn = (Getpf_WiFi() == 0) ? AHC_TRUE : AHC_FALSE;
#endif

    #ifdef BMICOM_FILE_DAMAGE
    AHC_ICON_LoadFileDamage((GUI_BITMAP*)&BMICOM_FILE_DAMAGE);
    #endif
    #ifdef BMICOM_FILE_G_PROTECTKEY
    AHC_ICON_LoadProtectKey(AHC_PROTECT_G,(GUI_BITMAP*)&BMICOM_FILE_G_PROTECTKEY);
    #endif
    #ifdef BMICOM_FILE_M_PROTECTKEY
    AHC_ICON_LoadProtectKey(AHC_PROTECT_MENU,(GUI_BITMAP*)&BMICOM_FILE_M_PROTECTKEY);
    #endif

    uiSysState.CurrentState = UI_STATE_UNSUPPORTED;

    StateModeResetAllOperation();
    StateModeResetAllPBOperation();
	printc("~~~~~~~~~~~~~~~~~~~~~bing~~~~~~~\r\n");
    
    StateModeSetOperation(UI_VIDEO_STATE, StateVideoRecModeInitLCD, StateVideoRecModeShutDown, StateVideoRecModeHandler);
#if (DSC_MODE_ENABLE)
    StateModeSetOperation(UI_CAMERA_STATE, StateCaptureModeInitLCD, StateCaptureModeShutDown, StateCaptureModeHnadler);                                            
#endif
    StateModeSetOperation(UI_BROWSER_STATE, StateBrowserModeInitLCD, StateBrowserModeShutDown, StateBrowserModeHandler);
    StateModeSetOperation(UI_PLAYBACK_STATE, StateMoviePlaybackModeInitLCD, StateMoviePlaybackModeShutDown, StateMoviePlaybackModeHandler);                                                                
    StateModeSetOperation(UI_MSDC_STATE, StateMSDCModeInit, StateMSDCModeShutDown, StateMSDCModeHandler);                                        
    StateModeSetOperation(UI_PCCAM_STATE, StatePCCAMModeInit, StatePCCAMModeShutDown, StateMSDCModeHandler);    
	//printc("%s,%d \n", __func__, __LINE__); //long 4 -27
    StateModeSetOperation(UI_CAMERA_MENU_STATE, MenuStateCameraModeInit, MenuStateCameraModeShutDown, MenuStateHandlerCommon);                                        
    StateModeSetOperation(UI_VIDEO_MENU_STATE, MenuStateVideoModeInit, MenuStateVideoModeShutDown, MenuStateHandlerCommon);                                        
    StateModeSetOperation(UI_PLAYBACK_MENU_STATE, MenuStatePlaybackModeInit, MenuStatePlaybackModeShutDown, MenuStateHandlerCommon);                                        
    StateModeSetOperation(UI_CLOCK_SETTING_STATE, MenuStateClockSettingsModeInitLCD, MenuStateClockSettingsModeShutDown, MenuStateClockSettingsModeHandler);                                        
#if (SD_UPDATE_FW_EN)
    StateModeSetOperation(UI_SD_UPDATE_STATE, MenuStateSDUpdateModeInit, MenuStateSDUpdateModeShutDown, MenuStateSDUpdateModeHandler);                                        
#endif   
#if (USB_MODE_SELECT_EN)
    StateModeSetOperation(UI_USBSELECT_MENU_STATE, MenuStateUSBModeSelectModeInitLCD, MenuStateUSBModeSelectModeShutDown, MenuStateUSBModeSelectModeHandler);                                                                
#endif

    {
        ShowOSD_SetLanguage(0xff/* here is dummy */);
        AHC_SetRecordByChargerIn(0);

        #ifdef CFG_BOOT_POWER_ON_BY_USB_SOS //may be defined in config_xxx.h
        if (KEY_GPIO_SOS != MMP_GPIO_MAX)
        {   /* Power on by USB with SOS key to enter PCCAM */
            MMP_UBYTE tempValue = 0;

            MMPF_PIO_EnableOutputMode(KEY_GPIO_SOS, MMP_FALSE, MMP_TRUE);
            MMPF_PIO_GetData(KEY_GPIO_SOS, &tempValue);

            if(tempValue == KEY_PRESSLEVEL_SOS)
                AHC_SetButtonStatus(KEY_LPRESS_REC);
        }
        #endif
        
        runOp = AHC_PowerOnOption();

        if (AHC_IsDCInBooting()) {
            ubBootupSelect = BOOTUP_CHARGER;
        }
        else if (AHC_IsUsbBooting()) {
            AHC_USB_WaitUntilFirstRoundDone(1200);

            if (IsAdapter_Connect()) {
                ubBootupSelect = BOOTUP_CHARGER;
            }
            else {
                ubBootupSelect = BOOTUP_MANUAL;//BOOTUP_USB; // lyj 20190929
            }
        }

#if (SD_UPDATE_FW_EN)
        SDUpdateCfgWpFile(SD_WP_BASENAME, SD_WP_EXTNAME, 2);
        #if defined(SD_UPDATE_FW_FORCE_EN) && (SD_UPDATE_FW_FORCE_EN == 1)
        if (1) 
        #else
        if (AHC_IsDcCableConnect() || AHC_IsUsbConnect()) 
        #endif
        {
            // Go to update firmware when SD_CarDV.bin exist and Charging
            if (SDUpdateIsFWExisted() == AHC_TRUE || SDUpdateIsWpExisted() == AHC_TRUE)
            {
                CheckRtcAndAutoSetRtc();
                StateSwitchMode(UI_SD_UPDATE_STATE);
            }
        }
#endif

        if( AHC_USB_IsForcedPCam() == AHC_TRUE )
        {
			StateSwitchMode(UI_PCCAM_STATE);
            return;
        }

		#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE)// Pcam Mix Mode ... UVC function work under Video/DSC mode
        ubBootupSelect = BOOTUP_USB;
		#endif

        if (ubBootupSelect==BOOTUP_USB)
        {
            printc(FG_GREEN("@@@ Boot from USB\r\n"));

            #ifdef CFG_BOOT_FAST_LPRESS_REC_WHEN_USB_BOOT //may be defined in config_xxx.h
            if (KEY_GPIO_REC != MMP_GPIO_MAX)
            {
                UINT8 tempValue = 0;

                AHC_GPIO_ConfigPad(KEY_GPIO_REC, PAD_NORMAL_TRIG);
                AHC_GPIO_SetOutputMode(KEY_GPIO_REC, MMP_FALSE);
                AHC_GPIO_GetData(KEY_GPIO_REC, &tempValue);

                if(tempValue == KEY_PRESSLEVEL_REC)
                    AHC_SetButtonStatus(KEY_LPRESS_REC);
            }
            #endif
		#if WIFI_PORT
            wifiOn = AHC_FALSE;
		#endif

		#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE)// Pcam Mix Mode ... UVC function work under Video/DSC mode
            if(1)
		#else    
            if (AHC_GetQuickKey(QUICK_PCCAM) || (MenuSettingConfig()->uiUSBFunction == MENU_SETTING_USB_PCAM))
		#endif
            {        
                #if (USB_MODE_SELECT_EN)
                USBSelectModePCam(NULL, 0);
                #endif

                #if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE == 1)
                {
                    MMP_ERR sRet = MMP_ERR_NONE;
                    
                    VideoFunc_UVCXUCmdRegisterHandler();
                    
                    sRet = MMPS_3GPRECD_PreviewPipeInUVCMixModeEnable(PRM_SENSOR, MMP_TRUE);
                    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet);} 

                    PCAM2MMP_SetUVCMixMode(MMP_TRUE);

                    CheckRtcAndAutoSetRtc();
                    StateSwitchMode(UI_VIDEO_STATE);
                    AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_USB_DETECTED, 0);
                }
                #else
                StateSwitchMode(UI_PCCAM_STATE);
                #endif
            }
            else
            {
                #if (USB_MODE_SELECT_EN)
                if (MMPS_USB_NORMAL == MMPS_USB_GetStatus()) {
					CheckRtcAndAutoSetRtc();//	If Date Time invalid, Record will fail.
                    StateSwitchMode(UI_USBSELECT_MENU_STATE);
                }
                else
                {
                    #ifdef CFG_POWER_ON_ENTER_CAMERA_STATE
                    StateSwitchMode(UI_CAMERA_STATE);
                    #else
                    AHC_SetRecordByChargerIn(CUS_AUTO_VR_DELAT_TIMES);
                    if (MOTION_DTC_SENSITIVITY_OFF != MenuSettingConfig()->uiMotionDtcSensitivity) {
                        Menu_SetMotionDtcSensitivity(MenuSettingConfig()->uiMotionDtcSensitivity);
                    }
				printc("##%s--->%d\n",__func__,__LINE__);
                    StateSwitchMode(UI_VIDEO_MENU_STATE);  // lyj 20190929
                    #endif
                }
                #else
                if(MenuSettingConfig()->uiUSBFunction == MENU_SETTING_USB_MSDC)
                {
                    #if (USB_MODE_SELECT_EN)
                	USBSelectModeMSDC(NULL,0);
					#endif
                    StateSwitchMode(UI_MSDC_STATE);
                }
                else
                {
                    #if (USB_MODE_SELECT_EN)
					USBSelectModePCam(NULL,0);
					#endif
                    StateSwitchMode(UI_PCCAM_STATE);
                }
                #endif
            }
        }
        else if(ubBootupSelect==BOOTUP_CHARGER || ubBootupSelect==BOOTUP_MANUAL)
        {
            UINT16      uwYear,uwMonth,uwDay,uwDayInWeek,uwHour,uwMinute,uwSecond;
            UINT8       ubAmOrPm, b_12FormatEn;
            AHC_BOOL    RTC_ret = AHC_FALSE;
            #if (RTC_CHECK)
		    UINT32      ulRTCSecondsOffset, ulRTCSecondsOffset_temp;
		    MMP_ULONG   ulOSTickCount;
		    #endif 

            printc(FG_GREEN("@@@ Boot from Charger or Manual\r\n"));
            
            #ifdef CFG_BOOT_DC_IN_POWER_OFF
            if ((AHC_GetBootingSrc() & PWR_ON_BY_KEY) != PWR_ON_BY_KEY)
            {
                printc("Power on by src %d\r\n",AHC_GetBootingSrc());
                AHC_PowerOff_NormalPathEx(AHC_TRUE, AHC_FALSE, AHC_FALSE);
            }
            #endif

            AHC_SetParam(PARAM_ID_USE_RTC, 1);
            for (i = 0 ; i < 10; i++) {
                #if (RTC_CHECK)
                UINT j;	
                MMP_BYTE ubTestPassCunt = 0;
                #endif
			AHC_RTC_UpdateTime();
            
            RTC_ret = AHC_GetClock( &uwYear, &uwMonth, &uwDay, &uwDayInWeek, &uwHour, &uwMinute, &uwSecond, &ubAmOrPm, &b_12FormatEn); 
                #if 0  //Test
			    {
			    	static MMP_ULONG t = 0;
			    	t++;
			        if(t < 3) {
			        	uwYear = 2060;
			        }
			    	printc("uiStateInitialize RTC Test: %2d %2d %2d - %2d:%2d:%2d  %d %d\r\n", uwYear, uwMonth, uwDay, uwHour, uwMinute, uwSecond, ubAmOrPm, b_12FormatEn );
			    }
			    #endif
	            if (AHC_Validate_Year(uwYear)) {
	            #if (RTC_CHECK)	
					ulRTCSecondsOffset_temp = AHC_DT_GetSecondsOffset(uwYear, uwMonth, uwDay, uwHour, uwMinute, uwSecond);
					for(j=0; j<3; j++) {
						AHC_RTC_UpdateTime();
						RTC_ret = AHC_GetClock( &uwYear, &uwMonth, &uwDay, &uwDayInWeek, &uwHour, &uwMinute, &uwSecond, &ubAmOrPm, &b_12FormatEn); 
	            		ulRTCSecondsOffset = AHC_DT_GetSecondsOffset(uwYear, uwMonth, uwDay, uwHour, uwMinute, uwSecond);
					    if((ulRTCSecondsOffset - ulRTCSecondsOffset_temp) < RTC_MAX_INTERVAL) {
					    	ubTestPassCunt++;
					    	ulRTCSecondsOffset_temp = ulRTCSecondsOffset;
					    }
					}
					if(ubTestPassCunt == 3) {		
						MMPF_OS_GetTime(&ulOSTickCount);
						printc("Time: RTC:%d OS:%d\r\n", ulRTCSecondsOffset, ulOSTickCount/1000);
						AHC_RTC_SetOSTimeInterval(ulRTCSecondsOffset - ulOSTickCount/1000);
						break;
					}
					else {
						MMPF_RTC_ForceReset();
    	        		printc("time invalid0: %d\r\n", uwYear);
					}
				#else
	            	break;
	            #endif
	            }
	            else {
	            	MMPF_RTC_ForceReset(); // lyj 20181018
    	        	printc("time invalid1: %d\r\n", uwYear);

	            }
			}			
            #ifdef CFG_POWER_ON_ALWAYS_REC
            AHC_SetRecordByChargerIn(CUS_AUTO_VR_DELAT_TIMES);
            #endif

            if ( (RTC_ret && AHC_Validate_Year(uwYear) && !Getfp_ResetTime()) ||
                 runOp == AHC_TRUE /* To Run PowerOn Option, not to enter clock setting */)
            {
            		 printc("Enter Clock Setting Page  nonthing ...\r\n");
                if(!AHC_Validate_Year(uwYear))
                {
                    //AHC_RestoreDefaultTime(&uwYear, &uwMonth, &uwDay, &uwDayInWeek, &uwHour, &uwMinute, &uwSecond, &ubAmOrPm, &b_12FormatEn) ;
                    //AHC_SetClock(uwYear, uwMonth, uwDay, uwDayInWeek, uwHour, uwMinute, uwSecond, ubAmOrPm, b_12FormatEn); // lyj 20181018
                }// lyj 20181018
                
                {
#if (MENU_GENERAL_BOOT_UP_MODE_EN)
                    
                    AHC_SetBootUpUIMode();
                    
#else
                    
                #ifdef CFG_BOOT_LCD_ALWAYS_REC_BY_CHARGER_IN //may be defined in config_xxx.h
                    AHC_SetRecordByChargerIn(CUS_AUTO_VR_DELAT_TIMES);
                #else
                    if(AHC_Charger_GetStatus() && !runOp)
                    {
                        AHC_SetRecordByChargerIn(CUS_AUTO_VR_DELAT_TIMES);
                    }
                    #if (SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
                    else
                    {   
                        if ((AHC_GetBootingSrc() & PWR_ON_BY_GSENSOR) == PWR_ON_BY_GSENSOR)
                        {
                            if(GSNR_PWRON_ACT == GSNR_PWRON_REC_AUTO)
                                AHC_SetRecordByChargerIn(CUS_AUTO_VR_DELAT_TIMES);
                        }
                    }
                    #endif
                #endif

                    if (AHC_IsUsbConnect() && !IsAdapter_Connect()) {
                        #if (USB_MODE_SELECT_EN)
                        if (MMPS_USB_NORMAL == MMPS_USB_GetStatus()) {
                            StateSwitchMode(UI_USBSELECT_MENU_STATE);
                        }
                        else
                        {
                            #ifdef CFG_POWER_ON_ENTER_CAMERA_STATE
                            StateSwitchMode(UI_CAMERA_STATE);
                            #else
                            if (MOTION_DTC_SENSITIVITY_OFF != MenuSettingConfig()->uiMotionDtcSensitivity) {
                                Menu_SetMotionDtcSensitivity(MenuSettingConfig()->uiMotionDtcSensitivity);
                            }

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
                    else
                    {
                        #ifdef CFG_POWER_ON_ENTER_CAMERA_STATE
                        StateSwitchMode(UI_CAMERA_STATE);
                        #else
                        if (MOTION_DTC_SENSITIVITY_OFF != MenuSettingConfig()->uiMotionDtcSensitivity) {
                            Menu_SetMotionDtcSensitivity(MenuSettingConfig()->uiMotionDtcSensitivity);
                        }

                        StateSwitchMode(UI_VIDEO_MENU_STATE);// lyj 20181018 UI_VIDEO_STATE
                        #endif
                    }
#endif
                }
            }
            else 
            {
                AHC_RestoreDefaultTime(&uwYear, &uwMonth, &uwDay, &uwDayInWeek, &uwHour, &uwMinute, &uwSecond, &ubAmOrPm, &b_12FormatEn);
               AHC_SetClock(uwYear, uwMonth, uwDay, uwDayInWeek, uwHour, uwMinute, uwSecond, ubAmOrPm, b_12FormatEn); 

                #if  0//defined(TV_ONLY) || defined(CFG_BOOT_BYPASS_CLOCK_CHECK)  //may be defined in config_xxx.h
                    #ifdef CFG_POWER_ON_ENTER_CAMERA_STATE
                    StateSwitchMode(UI_CAMERA_STATE);
                    #elif defined(CFG_POWER_ON_ENTER_BROSWER_STATE)
                    StateSwitchMode(UI_BROWSER_STATE);
                    #else
                    if (MOTION_DTC_SENSITIVITY_OFF != MenuSettingConfig()->uiMotionDtcSensitivity) {
                        Menu_SetMotionDtcSensitivity(MenuSettingConfig()->uiMotionDtcSensitivity);
                    }

                    StateSwitchMode(UI_VIDEO_STATE);
			//StateSwitchMode(UI_VIDEO_MENU_STATE);
					
                    #endif
                #else
                    printc("Enter Clock Setting Page ...\r\n");
					if(AIHC_GetOSDInitStatus())
					{
                  //  	StateSwitchMode(UI_CLOCK_SETTING_STATE);
                    	StateSwitchMode(UI_VIDEO_MENU_STATE); // run this
                //  StateSwitchMode(UI_VIDEO_STATE);
					}
					else
					{
						//AhcMenuClockInit = AHC_TRUE;
						StateSwitchMode(UI_VIDEO_MENU_STATE);
					}
                #endif
            }

            #ifdef CFG_BOOT_INIT_LAST_STATE_AS_CUR //may be defined in config_xxx.h
            if(uiGetLastState() == UI_STATE_UNSUPPORTED)
                uiSysState.LastState = uiGetCurrentState(); 
            #endif
        }
    }

    // StartUp NetWork
#if WIFI_PORT
    if (wifiOn) {
        nhw_init_network();
        MMPF_OS_GetTime(&ulLastWiFiChagneTime);
    }
#endif
}

void uiStateSetLocked(AHC_BOOL bLock)
{
    uiSysState.bModeLocked = bLock;
}

UINT8 uiGetLastState(void)
{
    return uiSysState.LastState;
}

UINT8 uiGetCurrentState(void)
{
    return uiSysState.CurrentState;
}

void uiSaveCurrentState(void)
{
	uiSysState.LastState = uiSysState.CurrentState;
}

AHC_BOOL uiStateProcessGlobalEvent2(UINT32 ulMsgId, UINT32* pulEvent, UINT32* pulParam)
{
#if (NO_CARD_POWER_OFF_EN)  
    extern AHC_BOOL gbNoCardPowerOff;
#endif

    switch(*pulEvent)
    {
        case BUTTON_SD_DETECTED:
            #if (NO_CARD_POWER_OFF_EN)
            gbNoCardPowerOff = AHC_FALSE;
            #endif

            #if (POWER_ON_BUTTON_ACTION)
            if(AHC_WMSG_GetInfo()==WMSG_SHOW_FW_VERSION)
                break;
            #endif 
			AHC_ShowErrorDialog(WMSG_NONE);	// Dismiss Error Dialog
            AHC_WMSG_Draw(AHC_TRUE, WMSG_WAIT_INITIAL_DONE, 3);

            #if UVC_HOST_VIDEO_ENABLE
            if (uiGetCurrentState() == UI_BROWSER_STATE) {
                AHC_SetDisplayMode(DISPLAY_MODE_PIP_DISABLE);
                AHC_OS_SleepMs(1000);
            }
            #endif
            break;

        case BUTTON_SD_REMOVED:
            #if (NO_CARD_POWER_OFF_EN)
            gbNoCardPowerOff = AHC_TRUE;
            #endif

            #if (POWER_ON_BUTTON_ACTION)
            if(AHC_WMSG_GetInfo()==WMSG_SHOW_FW_VERSION)
                break;
            #endif 

            AHC_WMSG_Draw(AHC_TRUE, WMSG_PLUG_OUT_SD_CARD, 2);

            #if UVC_HOST_VIDEO_ENABLE
            if (uiGetCurrentState() == UI_BROWSER_STATE) {
                AHC_SetDisplayMode(DISPLAY_MODE_PIP_DISABLE);
                AHC_OS_SleepMs(1000);
            }
            #endif
            break;
            
        case BUTTON_CLEAR_WMSG:
           // AHC_WMSG_Draw(AHC_FALSE, WMSG_NONE, 0);
           AHC_WMSG_DrawBMP(WMSG_CLEAR_SECTION,0); // lyj 20190814

            #if UVC_HOST_VIDEO_ENABLE
            if (uiGetCurrentState() == UI_BROWSER_STATE) {
                AHC_SetDisplayMode(DISPLAY_MODE_PIP_ENABLE);
            }
            #endif
        break;

        case BUTTON_UPDATE_CHARGE_ICON:
#if OSD_SHOW_BATTERY_STATUS
            if(AHC_TRUE == AHC_Charger_GetStatus())
            {
                AHC_DrawChargingIcon();
            }
#endif
        break;
            
        case BUTTON_DEVICES_READY:
#ifdef CFG_POWER_ON_CLEAN_SCREEN     // MCU Panel, need clean Panel GRAM
            #if (TVOUT_PREVIEW_EN)
            if(!AHC_IsTVConnectEx())
            #endif
            #if (HDMI_PREVIEW_EN)
            if(!AHC_IsHdmiConnect())
            #endif
            {
                RTNA_LCD_FillColor(0x00000000);
                AHC_OS_SleepMs(100);
            }
#endif

            // All of devices initialized
            // Turn ON LCD backlight
            LedCtrl_LcdBackLightLock(AHC_FALSE);
            //#if (TVOUT_PREVIEW_EN)
            //if(!AHC_IsTVConnectEx())
            //#endif
            //#if (HDMI_PREVIEW_EN)
            //if(!AHC_IsHdmiConnect())
            //#endif
            //Andy Liu TBD
			if(1
			#if (TVOUT_PREVIEW_EN)
            &&(!AHC_IsTVConnectEx())
            #endif
			#if (HDMI_PREVIEW_EN)
			&&(!AHC_IsHdmiConnect())
            #endif
			)
            {
                LedCtrl_LcdBackLight(AHC_TRUE);
            }
        break;

        case BUTTON_DC_CABLE_IN:
        case BUTTON_USB_DETECTED:
            AHC_SetShutdownByChargerOut(AHC_FALSE);
            break;
        case BUTTON_USB_B_DEVICE_DETECTED:
        case BUTTON_USB_B_DEVICE_REMOVED:
            if(uiSysState.CurrentState == UI_STATE_UNSUPPORTED ||
               uiSysState.CurrentState == UI_CLOCK_SETTING_STATE){
            AHC_SetCurKeyEventID(EVENT_NONE);
            }
            break;
        case BUTTON_UPDATE_MESSAGE:
#if defined(WIFI_PORT) && (WIFI_PORT == 1)
            WiFiStreamingLED_Timer_Start(100);
#endif
        break;

        default:
        break;  
    }
    return AHC_TRUE;
}

AHC_BOOL ubUIInitDone(void)
{
    return AhcMenuInit;
}

void uiStateMachine( UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam)
{
    AHC_BOOL    ahcRet = AHC_TRUE;

#if 0 //Why put it here???
	if(AhcMenuClockInit && AIHC_GetOSDInitStatus())
	{
		StateSwitchMode(UI_CLOCK_SETTING_STATE);
		AhcMenuClockInit = AHC_FALSE;
	}
#endif

    if(AHC_FALSE == AhcMenuInit){
		printc("~~~~~~~~~~~long long ~~~~~~~~\r\n");
        uiStateInitialize();      
        AhcMenuInit = AHC_TRUE;
		#if (SWITCH_MODE_FREEZE_WIN)
        MMPS_Display_SetFreezeWinEn(AHC_TRUE);
		#endif
    }
        
#if (FLIP_CTRL_METHOD & CTRL_BY_KEY) && (KEY_ROTATE_OSD_SNR != BUTTON_ID_UNKNOWN)
    if (ulEvent == KEY_ROTATE_OSD_SNR)
    {
        AHC_SetRotateSrc(SRC_KEY);
        printc("Rotate OSD By Key\r\n");
    }
#endif


    if (uBlkEvent || (uiStateProcessGlobalEvent( ulMsgId, &ulEvent, &ulParam ) == AHC_FALSE)){
        //return;
    }
    else if(uiStateProcessGlobalEvent2( ulMsgId, &ulEvent, &ulParam ) == AHC_FALSE){
        //return;
    }
#if (defined(WIFI_PORT) && WIFI_PORT == 1)    
    else if((gGblNetEventHandler != NULL) && (gGblNetEventHandler( ulMsgId, &ulEvent, &ulParam) == AHC_FALSE)) {
    	//this message is consider consumed here.
    	//typically calls uiStateProcessGlobalWifiEvent
    }
#endif    
    else{
        //printc("[UIevent:0x%x]",ulEvent);
        ahcRet = StateModeDoHandler(uiSysState.CurrentState, ulMsgId, ulEvent, ulParam);
    }
}

void PowerOffCB(void)
{
    printc("PowerOffCB: The second time Power Off !!!\r\n");
    //PMUCtrl_Power_Off();
}

extern void OSDDraw_EnterDrawing(UINT8* pbID0, UINT8* pbID1);
extern void OSDDraw_ExitDrawing(UINT8* pbID0, UINT8* pbID1);

void PowerOffClearBuff(void)
{
	AHC_BOOL bIsTvOut;
	UINT8 bID0,bID1;

	bIsTvOut = AHC_IsTVConnectEx();

	{
	OSDDraw_EnterDrawing(&bID0, &bID1);
	AHC_OSDClearBuffer(bID0);
	OSDDraw_ExitDrawing(&bID0, &bID1);
	}

	BEGIN_LAYER(0);
	AHC_OSDClearBuffer(0);
	END_LAYER(0);

	if(/*UI_TVOUT_STATE == uiGetCurrentState()*/bIsTvOut || UI_BROWSER_STATE == uiGetCurrentState())
	{
		AIHC_DrawReservedOSD(TRUE);
		BEGIN_LAYER(32);
		AHC_OSDClearBuffer(32);
		END_LAYER(32);
		
		BEGIN_LAYER(33);
		AHC_OSDClearBuffer(33);
		END_LAYER(33);
		AIHC_DrawReservedOSD(FALSE);
	}

}

void uiPowerOffPicture(void)
{
UINT8 bID0=0;// 20170927
    AHC_OS_TMRID timerID = 0xFF;
    UINT16  WaitSecondTimePowerOff = 5000;  //unit: ms

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }
    
    printc("The second time Power Off: %d ms\r\n", WaitSecondTimePowerOff);
    timerID = AHC_OS_StartTimer(MS_TO_TICK(WaitSecondTimePowerOff), OS_TMR_OPT_ONE_SHOT, (OS_TMR_CALLBACK)&PowerOffCB, NULL);
    
    if((timerID != 0xFF) && (timerID != 0xFE))
    {
     //   AHC_SetMode(AHC_MODE_IDLE);
        //AHC_OS_StopTimer(timerID, OS_TMR_OPT_NONE);
    }

	PowerOffClearBuff();


#if defined(WIFI_PORT) && (WIFI_PORT == 1)
	{	
	AHC_WiFi_Switch(AHC_FALSE);
	WiFiStreamingLED_Timer_Stop();
	}
#endif

    bShowPowerOffJpg = AHC_TRUE;

#ifdef CFG_NOSD_POWEROFF
    if(AHC_FALSE ==AHC_IsSDInserted())
    {
        #ifdef CFG_BOOT_POWER_OFF_VIBRATION
            AHC_Vibration_Enable(400);
        #endif

        LedCtrl_FlickerLedEx(AHC_GetVideoModeLedGPIOpin(), 5, 100);

        #ifdef CFG_BOOT_POWER_OFF_VIBRATION
        AHC_Vibration_Enable(400);
        #endif

        if(pf_GetLaserLedStatus())
            LedCtrl_FlickerLedEx(LED_GPIO_LASER, 5, 80);
    }
   else
    {
        #ifdef CFG_BOOT_POWER_OFF_VIBRATION
            AHC_Vibration_Enable(400);
        #endif
             AHC_OS_SleepMs(500);
        #ifdef CFG_BOOT_POWER_OFF_VIBRATION
            AHC_Vibration_Enable(400);
        #endif
             AHC_OS_SleepMs(600);
        if(pf_GetLaserLedStatus())
            LedCtrl_FlickerLedEx(LED_GPIO_LASER,10,200);
    }
#else
    #ifdef CFG_BOOT_POWER_OFF_VIBRATION
        AHC_Vibration_Enable(300);
    #endif
#endif

#ifdef CFG_BOOT_IGNORE_POWER_OFF_PIC //may be defined in config_xxx.h
    // NOP
#else
 //  AHC_PowerOff_ShowPicture("SF:0:\\goodbye.jpg");
	   
   //     OSDDraw_EnableSemiTransparent(bID1, AHC_FALSE);
			
	OSDDraw_GetOvlDrawBuffer(&bID0);
	OSDDraw_EnableSemiTransparent(bID0, AHC_FALSE);// liao 20180316    AHC_TRUE
	AHC_OSDDrawBitmap(bID0, &bmwelogo1, 0, 0);// 开机logo
	OSDDraw_SetOvlDrawBuffer(bID0);
	AHC_OSDSetCurrentDisplay(bID0);
	AHC_OSDRefresh();
		
#endif

#ifdef CFG_BOOT_IGNORE_POWER_OFF_PIC_ON_LCD     // No Panel, TV/HDMI-Out only
    // NOP
#else
    // If Backlight is off then turn on,
    // It can avoid LCD flash preview screen between Power off submenu and Goodbye!!!
    if (!AHC_IsHdmiConnect() && !AHC_IsTVConnectEx())
    {
        if (!LedCtrl_GetBacklightStatus()) 
            LedCtrl_LcdBackLight(AHC_TRUE);
    }

    AHC_OS_Sleep(Sleep_MS(1000));
	LedCtrl_LcdBackLight(AHC_FALSE);// liao
#endif

#if defined(CFG_BOOT_POWER_OFF_BY_POWER_PIN) && defined(DEVICE_GPIO_CUS_POWER_PIN_EN) && (DEVICE_GPIO_CUS_POWER_PIN_EN != MMP_GPIO_MAX)
    AHC_OS_Sleep(Sleep_MS(1000));
    MMPF_PIO_EnableOutputMode(DEVICE_GPIO_CUS_POWER_PIN_EN, MMP_TRUE, MMP_TRUE);
    MMPF_PIO_SetData(DEVICE_GPIO_CUS_POWER_PIN_EN, DEVICE_GPIO_CUS_POWER_OFF);
#endif
}

/*
 * Block Events when system in busy, like as
 * 1. Battery Low and PowerOff detected
 * 2. <Please add on here>
 */
void uiBlockEvent(int arg)
{
    uBlkEvent = arg;
}

UINT32 MenuStateExecutionCommon( UINT32 ulEvent, UINT32 ulPosition)
{
    UINT32       ulResult;
    PSMENUSTRUCT pCurrMenu = NULL;

    pCurrMenu = GetCurrentMenu();

    if(pCurrMenu == NULL)
        return MENU_ERR_EXIT;

    ulResult = pCurrMenu->pfEventHandler( pCurrMenu, ulEvent, ulPosition );//调用UINT32 MainMenuEventHandler(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam)；
	//printc("~~~~~~~~~~~~~~~long~~~%d~~ulEvent~%d~~ulPosition~%d~~\r\n",ulResult,ulEvent,ulPosition);
    if( ulResult == MENU_ERR_EXIT ){
#ifdef SLIDE_MENU
        if(IsSlidingMenu())
            StopSlideMenu();
#endif    
	printc("~~~~~~~~~~~~~~~long~~~%d~~ulEvent~%d~~ulPosition~%d~~\r\n",ulResult,ulEvent,ulPosition);
        ResetCurrentMenu();
        Menu_WriteSetting();
    }

    return ulResult;
}

//====================lyj=====
extern AHC_BOOL switch_search;
//unsigned char  fm_si47xx_get_mode(void);
extern unsigned char  work_mode;
//extern PowerUpFlag PowerOnFlag;
void  MenuStateExecutionCommon1(UINT32 ulPosition)
{
    UINT32       ulResult;
    PSMENUSTRUCT pCurrMenu = NULL;
	PSMENUSTRUCT pParentMenu = NULL;

    pCurrMenu = GetCurrentMenu();

    if(pCurrMenu == NULL)
        return;

	//printc("~~iCurrentPos = %d~~~~~~~\r\n",pCurrMenu->iCurrentPos);
	if(ulPosition == 8)
	{
		pCurrMenu->iCurrentPos = 8;
	}else if(ulPosition == 3)
	{
		pCurrMenu->iCurrentPos = 3;
	}
	else
	{
		pCurrMenu->iCurrentPos = ulPosition;
		if(pCurrMenu->iCurrentPos == 8)
			pCurrMenu->iCurrentPos = 0;
	}
	if(pCurrMenu->pParentMenu == NULL)
	{
		//printc("~~~the parentMenu is NULL \r\n",ulPosition);
		return_flag = 0;
		//if(flag !=  ulPosition || pCurrMenu->iMenuId == 6)
    		ulResult = pCurrMenu->pfEventHandler( pCurrMenu, MENU_OK, ulPosition );
	}
	else{

		             //PSMENUSTRUCT pParentMenu;
					

			if((flagJump !=  ulPosition)  || flagJump ==3)
			{
                    pCurrMenu->bSetOne     = 1;
                    pParentMenu        = pCurrMenu->pParentMenu;
                    pCurrMenu->pParentMenu = NULL;
			#if 0
                    if( pParentMenu == NULL ){
                        return;
                    }
			#endif
                    SetCurrentMenu(pParentMenu);
			}
                  //  pParentMenu->pfEventHandler(pParentMenu, MENU_INIT, ulPosition); // lyj

					
					//printc("~~~~~~0000000g~~~flag = %d~~~~~\r\n",flag);
					//printc("~~~~~~~~~~000g~~~ulPosition = %d~~~\r\n",ulPosition);
					if(flagJump ==  ulPosition)
					{
						if(flagJump == 3)
						{
							//printc("  RUN~the same page~1~~\r\n");
							return_flag = 0;
							
							pParentMenu->pfEventHandler(pParentMenu, MENU_INIT, ulPosition);
						}
						//printc("  RUN~the same page~EX~~\r\n");
						//pCurrMenu->pParentMenu = pParentMenu; // lyj 20181024
					}
					else
					{
						//if( flag == 100)
						//{
						//	printc(" The first RUN~~2~~\r\n");
						//	pParentMenu->pfEventHandler(pParentMenu, MENU_INIT, ulPosition);// lyj 20180930 
						//}
						//else
					
							
							return_flag = 0;
							pCurrMenu = GetCurrentMenu();
							if(ulPosition == 1)
							{
								if(work_mode == 1)
								{
								Close_channal();
								fm_si47xx_exit();
								}// lyj 20190213
								switch_search =0;
								//AHC_OS_SleepMs(100);
							//	PowerOnFlag.Fm = 100; // lyj 20190605
								MenuSettingConfig()->uiPowerUpFlag = 0;
							}
							else if(ulPosition == 2)
							{
								if(work_mode == 0)
								{
								Close_channal();
								am_si47xx_exit();
								}// lyj 20190213
								switch_search =0;
								//AHC_OS_SleepMs(300);
							//	PowerOnFlag.Fm = 100; // lyj 20190605
								MenuSettingConfig()->uiPowerUpFlag = 0;
							}
							else if(ulPosition == 0)
							{
								//Close_channal();
						 		// Bluetooth_LEdoff(); // lyj 20181024
						 	//	PowerOnFlag.Fm = 100; // lyj 20190605
						 		MenuSettingConfig()->uiPowerUpFlag = 0;
							}
							else //if(ulPosition == 6 || ulPosition == 5) 
							{
								//Close_channal();
						//		PowerOnFlag.Fm = 100; // lyj 20190605
								MenuSettingConfig()->uiPowerUpFlag = 0;
							}
							//printc("~~~~1111110000~~~~~~ulPosition = %d~~~~~~~\r\n",ulPosition);
							#if 1
							if(ulPosition == 8)
							{
								pCurrMenu->iCurrentPos = 8;
							}else if(ulPosition == 3)
							{
								pCurrMenu->iCurrentPos = 3;
							}
							else
							{

								pCurrMenu->iCurrentPos = ulPosition; //lyj
								if(pCurrMenu->iCurrentPos == 8)
										pCurrMenu->iCurrentPos = 0;
							}
							#endif
							
							Menu_Set_Page(ulPosition);
							//Bluetooth_LEdoff(); // lyj 20190423
							
							pCurrMenu->pfEventHandler( pCurrMenu, MENU_OK, ulPosition );

							
							printc("~~~~~lyj~~~jump successfull~~~~~\r\n");
					
					}


					
					//printc("~~~llllllllllog~~~flag = %d~~~~~~~~~\r\n",flag);
			
	}

	flagJump = ulPosition;
	

}

#if 0
extern MMP_USHORT Mannal_Freq;
extern MMP_USHORT radio[21];
extern MMP_USHORT radio_am[21];
extern MMP_USHORT AM_Freq;
extern short Radio_i;
extern short Radio_y;

extern short Radio_am_i;
extern short Radio_am_y;
extern int 			volume_num;
extern int 			volume_num1;
extern int 			volume_num2;

void auto_exit(void)
{
    PSMENUSTRUCT pCurrMenu = NULL;

    pCurrMenu = GetCurrentMenu();

	pCurrMenu->pfEventHandler(pCurrMenu, MENU_INIT, 0);

	if(pCurrMenu->pParentMenu != NULL)
	{
		if(return_flag == 1)
		{

			if(Menu_Get_Page() == 1)
			{
				switch(MenuSettingConfig()->uiMOVQuality)
				{
						case 0:
							Draw_FM_AM_icon(Mannal_Freq);

							break;

						case 1:

							if(radio[Radio_i] != 0)
							{
								Reserved_radio(radio);
								Draw_FM_AM_icon(radio[Radio_i]);
								Reserved_radio(radio);
							}
							else
								Draw_FM_AM_icon(9050);

							break;


						case 2:

							if(radio[Radio_y] != 0)
							{
								Reserved_radio(radio);
								Draw_FM_AM_icon(radio[Radio_y]);
								Reserved_radio(radio);
							}

							else
								Draw_FM_AM_icon(9050);

							break;

				}


			}

			if(Menu_Get_Page() == 2)
			{
				switch(MenuSettingConfig()->uiBeep)
				{
						case 0:
							Draw_AM_icon(AM_Freq);

							break;

						case 1:

							if(radio_am[Radio_am_i] != 0)
							{
								Reserved_radio(radio_am);
								Draw_AM_icon(radio_am[Radio_am_i]);
								Reserved_radio(radio_am);
							}
							else
								Draw_AM_icon(747);

							break;


						case 2:

							if(radio_am[Radio_am_y] != 0)
							{
								Reserved_radio(radio_am);
								Draw_AM_icon(radio_am[Radio_am_y]);
								Reserved_radio(radio_am);
							}
							else
								Draw_AM_icon(765);

							break;

				}


			}

			#if 1
			if(Menu_Get_Page() == 3)
			{
				if(MenuSettingConfig()->uiVMDRecTime == 0)
					MenuDrawChangeSubItem3(pCurrMenu, volume_num, 0, pCurrMenu->iSelected);
				if(MenuSettingConfig()->uiVMDRecTime == 1)
					MenuDrawChangeSubItem3(pCurrMenu, volume_num1, 0, pCurrMenu->iSelected);
				if(MenuSettingConfig()->uiVMDRecTime == 2)
					MenuDrawChangeSubItem3(pCurrMenu, volume_num2, 0, pCurrMenu->iSelected);

			}
			#endif
		}

	}


}
#else
void auto_exit(UINT16 bID0)
{
		 //UINT8   bID0 = 0, bID1 = 0;
		GUI_COLOR Color_1;
		 RECT tmpRECT1 ={420, 105, 50, 203};//{415,118,60,187}

		//   OSDDraw_EnterMenuDrawing(&bID0, &bID1);

		switch(Main_Get_Page())
		{
		case MENU_MAIN_FLAG :        Color_1 = OSD_COLOR_MAIN_PAGE;  break;
		case BLUETOOH_FLAG  :         Color_1 = OSD_COLOR_BLUETOOTH;  break;
		case RADIO_FM_FLAG :          Color_1 = OSD_COLOR_FM;  break;
		case RADIO_AM_FLAG :         Color_1 = OSD_COLOR_AM;  break;
		case VOLUME_FLAG :             Color_1 = OSD_COLOR_VOL;  break;
		case LIGHT_BAR_FLAG :       Color_1 = OSD_COLOR_BAR;  break;
		case AUX_FLAG :                 Color_1 = OSD_COLOR_AUX; break;
		case USB_FLAG : 			 Color_1 = OSD_COLOR_USB;  break;
		case RGB_FLAG :  			Color_1 = OSD_COLOR_RGB; break;
		case BRIGHTNESS_FLAG :     Color_1 = OSD_COLOR_BRIGHTNESS;  break;
	}
		#if 0 // lyj 20191018
		if(Main_Get_Page() == RGB_FLAG)
		{
			AHC_OSDDrawBitmap(bID0, &bmIcon_Num_1,tmpRECT1.uiLeft, tmpRECT1.uiTop);
		}
		else
		#endif
		{
			AHC_OSDSetColor(bID0, Color_1); 
		   	AHC_OSDDrawFillRect(bID0, tmpRECT1.uiLeft, tmpRECT1.uiTop, 470, 308);
		}
		//OSDDraw_ExitMenuDrawing(&bID0, &bID1);
		

}

#endif
#if 0
void Right_left_code(AHC_BOOL codeflag)
{
	#if 0
	PSMENUSTRUCT pCurrMenu = NULL;
	
	pCurrMenu = GetCurrentMenu();
	if(pCurrMenu == NULL)
	{
		return ;
	}
	if(codeflag == 1)
		pCurrMenu->pfEventHandler(pCurrMenu, MENU_RIGHT, 0);
	if(codeflag == 0)
		pCurrMenu->pfEventHandler(pCurrMenu, MENU_LEFT, 0);
	#endif

	if(codeflag ==0)
		SetKeyPadEvent(ENCODE_BUTTON_LEFT);
	if(codeflag == 1)
		SetKeyPadEvent(ENCODE_BUTTON_RIGHT);
}
#endif



#if(ENCODE_GPIO_ISR)
void InitEncodeGpio( MMP_GPIO_PIN piopin,GpioCallBackFunc* CallBackFunc,MMP_GPIO_TRIG flag)
{
    if(piopin == MMP_GPIO_MAX)
		return;
    
    printc("%s \r\n", __func__);
    //Set GPIO as input mode
    MMPF_PIO_EnableOutputMode(piopin, MMP_FALSE, MMP_TRUE);

    // Set the trigger mode.  设置触发模式
    MMPF_PIO_EnableTrigMode(piopin, flag, MMP_TRUE, MMP_TRUE);

    //Enable Interrupt  使能中断
    MMPF_PIO_EnableInterrupt(piopin, MMP_TRUE, 0, (GpioCallBackFunc *)CallBackFunc, MMP_TRUE);

}
#endif
UINT32 MenuStateHandlerCommon(UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam )//循环执行放在ui初始化里面
{
    UINT32 ulOperation = 0, ulMenuResult = 0;
    AHC_BOOL    ahcRet = AHC_TRUE;
    UI_STATE_ID sParentState;

  //  printc("~~~~~~~~~~~~long~~~%s:%x,%x,%x\r\n", __func__, ulMsgId, ulEvent, ulParam);  //long 4 -28
    
    MenuModePreCheck(ulEvent);
    ulOperation = MenuButtonToMenuOp(ulEvent, ulParam);

	//printc("~~~~~~$$$$$$$$$$$$$$$$$~bing~~~~=%d\r\n",ulOperation);
    
    if(MENU_NOT_MENU_EVENT != ulOperation){
        ulMenuResult = MenuStateExecutionCommon(ulOperation, ulParam);//ulMenuResult = MenuStateVideoMode(ulOperation, ulParam);    
    }
	//printc("~~~~~~$$$$$$$$$$$$$$$$$~bing~~~~=%d\r\n",ulMenuResult);
	if(MENU_ERR_FORCE_BROWSER_EXIT == ulMenuResult)
	{
		StateSwitchMode(UI_BROWSER_STATE);
	}
	else
	{
	    StateModeGetParent(uiGetWorkingState(), &sParentState);
	    //printc("~~~~~11~$$$$$$$$$$$$$$$$$~bing~~~~=%d\r\n",sParentState);	

	    if(sParentState != UI_STATE_UNSUPPORTED){ 
			//printc("~~~~~~~~11333322~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
	        if((MENU_ERR_EXIT == ulMenuResult) ||(MENU_EXIT_DO_PARENT_EVENT == ulMenuResult)){
				//printc("~~~~~~~~113333~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
	            ahcRet = StateDetachSubMode();
	            if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(0,ahcRet); return ahcRet;} 
	        }
	            
	        if((MENU_EXIT_DO_PARENT_EVENT == ulMenuResult) || (MENU_EXIT == ulOperation) || (MENU_NOT_MENU_EVENT == ulMenuResult)){                
	            //Pass event to parent UI state.
	            //printc("~~~~~~~~11333344~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
	            StateModeDoHandler(sParentState, ulMsgId, ulEvent, ulParam);
	        }
	    }    
	}
    return ulMenuResult;
}

