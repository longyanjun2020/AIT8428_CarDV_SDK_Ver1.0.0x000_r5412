/*===========================================================================
 * Include files
 *===========================================================================*/

#include "customer_config.h"
#include "AHC_Config_SDK.h"
#include "AHC_Parameter.h"
#include "AHC_Warningmsg.h"
#include "AHC_Message.h"
#include "AHC_Menu.h"
#include "AHC_Os.h"
#include "AHC_General.h"
#include "AHC_Display.h"
#include "AHC_Utility.h"
#include "AHC_General.h"
#include "ColorDefine.h"
#include "mmp_lib.h"
#include "MenuCommon.h"
#include "MenuDrawCommon.h"
#include "OsdStrings.h"
#include "IconDefine.h"
#include "mmps_display.h"
#include "mmps_3gprecd.h"
#include "MenuSetting.h"
#include "ait_utility.h"
#include "ShowOSDFunc.h"
#include "StateHDMIFunc.h"
#include "lib_retina.h"
#include "lcd_common.h"
#if (ENABLE_ADAS_LDWS||ENABLE_ADAS_FCWS)
#include "ldws.h"
#include "ldws_cfg.h"
#include "GPS_Ctl.h"

// Maybe defined in config_sdk.h
#ifndef LDWS_L_X_OFFSET
#define LDWS_L_X_OFFSET         (0)
#endif

#ifndef LDWS_L_Y_OFFSET
#define LDWS_L_Y_OFFSET         (0)
#endif

#ifndef LDWS_R_X_OFFSET
#define LDWS_R_X_OFFSET         (0)
#endif

#ifndef LDWS_R_Y_OFFSET
#define LDWS_R_Y_OFFSET         (0)
#endif

#ifndef LDWS_Y_INC_LENGTH
#define LDWS_Y_INC_LENGTH       (35)
#endif

#endif

#if (ENABLE_ADAS_FCWS)
#include "fcws.h"
#endif

/*===========================================================================
 * Macro define
 *===========================================================================*/

#define USE_WMSG_SEMI (1)

#if defined(WIFI_PORT) && (WIFI_PORT == 1)
    #include "netapp.h"
#endif

#define DRAW_WARNING_MSG_ENABLE    (1)

/*===========================================================================
 * Global variable
 *===========================================================================*/

AHC_WMSG_CFG    m_WMSG;
UINT8           m_WMSGShowTime  = 10;
UINT32          m_LensError     = 0;

static WMSG_INFO     m_WMSGInfo = WMSG_NONE;
#if (USE_WMSG_SEMI)
AHC_OS_SEMID    m_WMSGSemID = 0xFF;
#endif

#if (ENABLE_ADAS_LDWS)
static AHC_BOOL m_bDrawLDWS = AHC_FALSE;
#endif

#if (DRIVING_SAFETY_REMIND_HEADLIGHT)
static AHC_BOOL m_bDrawRemindHeadlight = AHC_FALSE;
#endif

UINT32 m_uiWMSGStringID[END_OF_WMSG]={0};

#ifndef BM_ICON_WMSG
#define BM_ICON_WMSG        bmIcon_WMSG
#endif

/*===========================================================================
 * Extern variable
 *===========================================================================*/

//extern MMP_BOOL CalibrationMode;
extern AHC_OSD_INSTANCE *m_OSD[];

extern AHC_BOOL Deleting_InBrowser;
extern AHC_BOOL Protecting_InBrowser;
extern AHC_BOOL PowerOff_InProcess;

extern MMP_USHORT   gsAhcPrmSensor;
extern AHC_BOOL     gbAhcDbgBrk;
/*===========================================================================
 * Extern function
 *===========================================================================*/

//extern AHC_BOOL AHC_SendAHLMessage(UINT32 ulMsgID, UINT32 ulParam1, UINT32 ulParam2);
extern int      sprintf(char * /*s*/, const char * /*format*/, ...);
extern int      AHC_GetMinorVersion(void);

#if (ENABLE_ADAS_LDWS)
extern MMP_UBYTE ADAS_CTL_GetLDWSAttr(ldws_params_t *ldws_attr, MMP_LONG *alert);
#endif
#if (ENABLE_ADAS_FCWS)
extern MMP_UBYTE ADAS_CTL_GetFCWSAttr(fcws_info_t *pfcws_info);
#endif

static int      find_newline(char *line1, char *line2);
static unsigned char find_newline2(char *line1, unsigned char len);

/*===========================================================================
 * Main body
 *===========================================================================*/

#ifdef CFG_CUS_DRAW_ICON_WSG
AHC_BOOL AHC_WMSG_Config(void)
{
    m_WMSG.DisplayID        = 0;                    // Fixed at 0
    m_WMSG.StartX           = AHC_WMSG_START_X;     // default: 0, it might be defined in AHC_Config_SDK.h
    m_WMSG.StartY           = AHC_WMSG_START_Y;     // default: 60, it might be defined in AHC_Config_SDK.h
    m_WMSG.DisplayWidth     = AHC_WMSG_WIDTH;       // default: 320, it might be defined in AHC_Config_SDK.h
    m_WMSG.DisplayHeight    = AHC_WMSG_HEIGHT;      // default: 120, it might be defined in AHC_Config_SDK.h
    m_WMSG.byBkColor        = AHC_WMSG_BK_COLOR;    // default: 0xFF00D0D0, it might be defined in AHC_Config_SDK.h
    m_WMSG.byBoundColor     = AHC_WMSG_BOUND_COLOR; // default: 0xAA009090, it might be defined in AHC_Config_SDK.h

#ifdef STRETCH_X
    m_WMSG.StartY           = STRETCH_Y(m_WMSG.StartY);
    m_WMSG.DisplayWidth     = STRETCH_X(m_WMSG.DisplayWidth);
    m_WMSG.DisplayHeight    = STRETCH_Y(m_WMSG.DisplayHeight);
#endif

#if (USE_WMSG_SEMI)
    if (m_WMSGSemID >= 0xFE)
        m_WMSGSemID         = AHC_OS_CreateSem(1);
#endif

    return AHC_TRUE;
}
#else
AHC_BOOL AHC_WMSG_Config(void)
{
    m_WMSG.DisplayID        = 0;    // Fixed at 0
    m_WMSG.StartX           = 0;
    m_WMSG.StartY           = 60;
    m_WMSG.DisplayWidth     = 320;
    m_WMSG.DisplayHeight    = 120;
    #ifdef CUS_WMSG_BK_COLOR
    m_WMSG.byBkColor        = CUS_WMSG_BK_COLOR;
    #else
    m_WMSG.byBkColor        = 0xFF00D0D0;
    #endif
    m_WMSG.byBoundColor     = 0xAA009090;

    #ifdef STRETCH_X
    m_WMSG.StartY           = STRETCH_Y(m_WMSG.StartY);
    m_WMSG.DisplayWidth     = STRETCH_X(m_WMSG.DisplayWidth);
    m_WMSG.DisplayHeight    = STRETCH_Y(m_WMSG.DisplayHeight);
    #endif

#if (USE_WMSG_SEMI)
    if (m_WMSGSemID >= 0xFE)
        m_WMSGSemID         = AHC_OS_CreateSem(1);
#endif

    return AHC_TRUE;
}
#endif

void AHC_WMSG_Init(void)
{
#if defined(CCIR656_OUTPUT_ENABLE) && (CCIR656_OUTPUT_ENABLE)
    return;
#endif

#if (DRAW_WARNING_MSG_ENABLE == 0)
    return;
#endif

    AHC_WMSG_Config();
}

WMSG_INFO AHC_WMSG_GetInfo(void)
{
    return m_WMSGInfo;
}

AHC_BOOL AHC_WMSG_States(void)
{
#if defined(CCIR656_OUTPUT_ENABLE) && (CCIR656_OUTPUT_ENABLE)
    return AHC_TRUE;
#endif

#if (DRAW_WARNING_MSG_ENABLE == 0)
    return AHC_TRUE;
#endif

    if(m_WMSGInfo == WMSG_NONE) {
        return AHC_FALSE;
    }
    else {
        return AHC_TRUE;
    }
}

AHC_BOOL AHC_WMSG_IsTimeOut(UINT32 msTimer)
{
#if defined(CCIR656_OUTPUT_ENABLE) && (CCIR656_OUTPUT_ENABLE)
    return AHC_TRUE;
#endif

#if (DRAW_WARNING_MSG_ENABLE == 0)
    return AHC_TRUE;
#endif

    if(m_WMSGShowTime == 0)
        return AHC_TRUE;

    if(msTimer > (UINT32)m_WMSGShowTime*1000) {

#if (POWER_ON_BUTTON_ACTION)
        if(m_WMSGInfo==WMSG_SHOW_FW_VERSION)
        {
            AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, KEY_POWER_OFF, 0);
        }
#endif

        AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_CLEAR_WMSG, 0);
        return AHC_TRUE;
    }
    else {
        return AHC_FALSE;
    }
}

AHC_BOOL AHC_WMSG_SetLensError(UINT32 Number)
{
    m_LensError = Number;

    return AHC_TRUE;
}

AHC_BOOL AHC_WMSG_GetLensError(UINT32 *Number)
{
    *Number = m_LensError;

    return AHC_TRUE;
}

void AHC_WMSG_SetStringID(WMSG_INFO sWMSGInfoIndex, UINT32 uiStringID)
{
    if(sWMSGInfoIndex >= END_OF_WMSG){
        printc(FG_RED("%s,%d,%d error!\r\n"), __func__, __LINE__, sWMSGInfoIndex);
        return;
    }

    m_uiWMSGStringID[sWMSGInfoIndex] = uiStringID;
}

#if 1




void RectDrawingTest(char * szvwarning)
{
		 RECT tmpRECTwarning = {100, 120, 280, 80};
		AHC_OSDSetColor(m_WMSG.DisplayID, 0xAA009090); 
		AHC_OSDSetFont(m_WMSG.DisplayID, &GUI_Font20B_1);
	  	 AHC_OSDDrawFillRect(m_WMSG.DisplayID, 100, 120, 380, 200);
		OSD_ShowString( m_WMSG.DisplayID,szvwarning, NULL, tmpRECTwarning, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);

}


void AHC_WMSG_DrawBMP(WMSG_INFO WMSGInfo, UINT8 WMSGShowTime)//lyj 20190806
{
			//static AHC_BOOL stringIDTest =0;
			char stringWarning6[12][15] = {
/*	[0] = sprintf("Test FM IC",
	[1]="FM IIC ACK",
	[2]="FM IIC NACK",

	[3] = "Test e2prom",
	[4] = "e2prom ACK",
	[5] = "e2prom NACK",

	[6] ="chanel IC Test",
		
	[7] = "chanel IC ACK",

	[8] = "chanel IC NACK",

	[9] = "Test big lamp",
	[10] = "Test RGB",
	[11] = "Test USB",*/0
};
			//stringWarning6[0] = "Test FM IC";
			sprintf(stringWarning6[0], "Test FM IC");
			sprintf(stringWarning6[1], "FM IIC ACK");
			sprintf(stringWarning6[2], "FM IIC NACK");
			sprintf(stringWarning6[3], "Test e2prom");
			sprintf(stringWarning6[4], "e2prom ACK");
			sprintf(stringWarning6[5], "e2prom NACK");
			sprintf(stringWarning6[6], "chanel IC Test");
			sprintf(stringWarning6[7], "chanel IC ACK");
			sprintf(stringWarning6[8], "chanel IC NACK");
			sprintf(stringWarning6[9], "Test big lamp");
			sprintf(stringWarning6[10], "Test RGB");
			sprintf(stringWarning6[11], "Test USB");

			OSDDraw_EnableSemiTransparent(m_WMSG.DisplayID, AHC_FALSE);
			AHC_OSDClearBuffer(m_WMSG.DisplayID);
			 AHC_OSDSetCurrentDisplay(m_WMSG.DisplayID);
			OSDDraw_EnableSemiTransparent(m_WMSG.DisplayID, AHC_FALSE);
        		AHC_OSDSetActive(m_WMSG.DisplayID, 1);
			OSDDraw_EnableSemiTransparent(m_WMSG.DisplayID, AHC_FALSE);
			m_WMSGShowTime = WMSGShowTime;
       		 m_WMSGInfo     = WMSGInfo;
			//BEGIN_LAYER(m_WMSG.DisplayID);
			OSDDraw_EnableSemiTransparent(m_WMSG.DisplayID, AHC_FALSE);
			//printc("m_WMSG.Display 11L= %d\r\n", m_WMSG.DisplayID);//ÅÜÕâ¸ö
			switch (WMSGInfo)

			{
				case WMSG_FILE_ERROR:
					
					RectDrawingTest(stringWarning6[0]);	
					break;
				case WMSG_INSERT_SD_AGAIN:
						RectDrawingTest(stringWarning6[1]);	
					break;
				case	WMSG_NO_CARD:

					RectDrawingTest(stringWarning6[2]);	
					break;
				case WMSG_PLUG_OUT_SD_CARD:
					//AHC_OSDDrawBitmap(m_WMSG.DisplayID, &bmwarning_sd, 65, 120);
					//printc("3333333333333333333333\r\n");
					
						RectDrawingTest(stringWarning6[3]);	
				
					break;

				case WMSG_WAIT_INITIAL_DONE:
					//AHC_OSDDrawBitmap(m_WMSG.DisplayID, &bmreadsdcard, 50,120);
					{
					 	char stringWarning1[] = "Entrance Test Mode";
					
						RectDrawingTest(stringWarning1);
					}
					
					break;

				case WMSG_FORMAT_SD_CARD_OK:
					//AHC_OSDDrawBitmap(m_WMSG.DisplayID, &bmformat02_bg, 90, 90); // lyj
					//AHC_PlaySoundEffect(AHC_SOUNDEFFECT_FOCUS);
					RectDrawingTest(stringWarning6[4]);	
								break;
				case WMSG_LOCK_CURRENT_FILE:
					RectDrawingTest(stringWarning6[5]);	
					break;

				case WMSG_STORAGE_FULL:
					RectDrawingTest(stringWarning6[6]);	
					break;

					case WMSG_LOW_BATTERY:
						RectDrawingTest(stringWarning6[7]);	
					break;
					
					case WMSG_CARD_LOCK:
						RectDrawingTest(stringWarning6[8]);	
					break;
					
					case WMSG_ADAPTER_ERROR:
						RectDrawingTest(stringWarning6[9]);	
					break;
					
					case WMSG_BATTERY_FULL:
						RectDrawingTest(stringWarning6[10]);	
					break;
					
					case WMSG_FHD_VR:
						RectDrawingTest(stringWarning6[11]);	
					break;

					case WMSG_LDWS_RightShift:
								{
					 	char stringWarning3[] = "Test finish Poweroff!";
					
						RectDrawingTest(stringWarning3);
						}
						break;
					
			
			        case WMSG_CLEAR:
					// AHC_OSDSetColor(m_WMSG.DisplayID, OSD_COLOR_TRANSPARENT);
					 //AHC_OSDDrawFillRect(m_WMSG.DisplayID,0,0 , 480,320);
					 	//m_WMSGInfo = WMSG_NONE;
						{
					 	char stringWarning2[] = "Background Test";
					
						RectDrawingTest(stringWarning2);
						}
					break;
				 case WMSG_CLEAR_SECTION:
					 AHC_OSDSetColor(m_WMSG.DisplayID, OSD_COLOR_TRANSPARENT);
					 AHC_OSDDrawFillRect(m_WMSG.DisplayID,100,120 , 380,200);
					 	m_WMSGInfo = WMSG_NONE; 
					 printc("^^^^^^^^^^^^^111\r\n");
					break;	
				case 9999:
					

					 break;

				default:

					break;
						
			}
				
			//AHC_OSDDrawBitmap(m_WMSG.DisplayID, &bmwarning_sd, 50, 80);
			
		
			//END_LAYER(m_WMSG.DisplayID) ;
}
#endif
AHC_BOOL AHC_WMSG_Draw(AHC_BOOL Enable, WMSG_INFO WMSGInfo, UINT8 WMSGShowTime)
{
#if 0//!defined(NO_PANEL) || defined(TV_ONLY)

    AHC_DISPLAY_OUTPUTPANEL  OutputDevice;
    RECT        StrRECT;
    UINT32      iStrID;
    UINT8       bLine = 1;
    // Using static to save the stack size
    static char TempStr[200];
    static char Temp1Str[100];
    static char Temp2Str[100];
    UINT16      Width, Height;
    UINT32      iVal[2];
    UINT8       bLine1MaxLeng = 18;


    /* The WARN MESSAGE should be OPENED and CLOSED in Browser mode */
    if(PowerOff_InProcess)
    {
        return AHC_TRUE;
    }



    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return AHC_TRUE; }




    #if (USE_WMSG_SEMI)
    // give a timeout to allow failure to avoid dead-lock
    if(AHC_OS_AcquireSem(m_WMSGSemID, 200) != OS_NO_ERR)
    {
        printc("--E-- m_WMSGSemID OSSemPost: Fail!! \r\n");
        return AHC_TRUE;
    }
    #endif

    BEGIN_LAYER(m_WMSG.DisplayID);

    // Drop this time when WMSGInfo is same as show or WMSG_NONE to save task loading
    if (Enable && ((WMSG_NONE == WMSGInfo) || (WMSGInfo == m_WMSGInfo))) {
        m_WMSGShowTime = WMSGShowTime;      // update show time only
        goto Label_AHC_WMSG_END;
    }

    // Update m_WMSG again.
    // Wrong configure when Panel size is not 320x240 under TV/HDMI out.
    AHC_WMSG_Config();

    AHC_GetDisplayOutputDev(&OutputDevice);

    if (OutputDevice == MMP_DISPLAY_SEL_MAIN_LCD) {
        if (OSD_DISPLAY_SCALE_LCD) {
            iVal[0] = OSD_DISPLAY_SCALE_LCD - 1;
            iVal[1] = OSD_DISPLAY_SCALE_LCD - 1;
            AHC_OSDSetDisplayAttr(m_WMSG.DisplayID, AHC_OSD_ATTR_DISPLAY_SCALE, iVal);
        }
        else {
            iVal[0] = MMP_DISPLAY_DUPLICATE_1X; // No Scale for Message Box in LCD
            iVal[1] = MMP_DISPLAY_DUPLICATE_1X;
            AHC_OSDSetDisplayAttr(m_WMSG.DisplayID, AHC_OSD_ATTR_DISPLAY_SCALE, iVal);
        }
        m_WMSG.StartX = (AHC_GET_ATTR_OSD_W(m_WMSG.DisplayID) - m_WMSG.DisplayWidth) / 2;
        m_WMSG.StartY = (AHC_GET_ATTR_OSD_H(m_WMSG.DisplayID) - m_WMSG.DisplayHeight) / 2;
//        iVal[0] = (LCD_ATTR_DISP_W - AHC_GET_ATTR_OSD_W(m_WMSG.DisplayID)*OSD_DISPLAY_SCALE_LCD) / 2;
//        iVal[1] = (LCD_ATTR_DISP_H - AHC_GET_ATTR_OSD_H(m_WMSG.DisplayID)*OSD_DISPLAY_SCALE_LCD) / 2;
//
//        AHC_OSDSetDisplayAttr(m_WMSG.DisplayID, AHC_OSD_ATTR_DISPLAY_OFFSET, iVal);
    }
    

    if (Enable) {
        MEMSET(TempStr,  0, sizeof(TempStr));
        MEMSET(Temp1Str, 0, sizeof(Temp1Str));
        MEMSET(Temp2Str, 0, sizeof(Temp2Str));

        StrRECT.uiLeft      = m_WMSG.StartX;
        StrRECT.uiTop       = m_WMSG.StartY;
        StrRECT.uiWidth     = m_WMSG.DisplayWidth;
        StrRECT.uiHeight    = m_WMSG.DisplayHeight;

        ////////////////////////////////////////////////////////////////
        switch(WMSGInfo) {
        case WMSG_CARD_ERROR:
            iStrID = IDS_DS_MSG_CARD_ERROR;

            switch(MenuSettingConfig()->uiLanguage) {
            #if (MENU_GENERAL_LANGUAGE_SPANISH_EN)
            case LANGUAGE_SPANISH:
                bLine = 2;
                bLine1MaxLeng = 36;
            break;
            #endif
            #if (MENU_GENERAL_LANGUAGE_PORTUGUESE_EN)
            case LANGUAGE_PORTUGUESE:
                bLine = 2;
                bLine1MaxLeng = 37;
            break;
            #endif
            #if (MENU_GENERAL_LANGUAGE_RUSSIAN_EN)
            case LANGUAGE_RUSSIAN:
                //bLine = 2;
                //bLine1MaxLeng = 25;
            break;
            #endif
            }
        break;

        case WMSG_STORAGE_FULL:
            iStrID = IDS_DS_MSG_STORAGE_FULL;
        break;

        case WMSG_PARKING_OPERATING_DESCRIPTION:
            #ifdef CFG_CUS_DRAW_MULTI_LINE // TBD
            iStrID = IDS_DS_MSG_PARKING_OPERATING_DESCRIPTION;
            #else
            iStrID = m_uiWMSGStringID[WMSG_PARKING_OPERATING_DESCRIPTION];
            #endif

            switch (MenuSettingConfig()->uiLanguage)
            {
                #if (MENU_GENERAL_LANGUAGE_ENGLISH_EN)
                case LANGUAGE_ENGLISH:
                    bLine = 2;
                    bLine1MaxLeng = 20;
                break;
                #endif
                #if (MENU_GENERAL_LANGUAGE_TCHINESE_EN)
                case LANGUAGE_TCHINESE:

                break;
                #endif
                #if (MENU_GENERAL_LANGUAGE_JAPANESE_EN)
                case LANGUAGE_JAPANESE:

                break;
                #endif
                #if (MENU_GENERAL_LANGUAGE_KOREAN_EN)
                case LANGUAGE_KOREAN:
                    bLine = 2;
                    bLine1MaxLeng = 10;
                break;
                #endif
            }
        break;

        case WMSG_MOTION_OPERATING_DESCRIPTION:
            #ifdef CFG_CUS_DRAW_MULTI_LINE // TBD
            iStrID = IDS_DS_MSG_MOTION_OPERATING_DESCRIPTION;
            #else
            iStrID = m_uiWMSGStringID[WMSG_MOTION_OPERATING_DESCRIPTION];
            #endif

            switch (MenuSettingConfig()->uiLanguage)
            {
                #if (MENU_GENERAL_LANGUAGE_ENGLISH_EN)
                case LANGUAGE_ENGLISH:
                    bLine = 2;
                    bLine1MaxLeng = 23;
                break;
                #endif
                #if (MENU_GENERAL_LANGUAGE_TCHINESE_EN)
                case LANGUAGE_TCHINESE:

                break;
                #endif
                #if (MENU_GENERAL_LANGUAGE_JAPANESE_EN)
                case LANGUAGE_JAPANESE:

                break;
                #endif
                #if (MENU_GENERAL_LANGUAGE_KOREAN_EN)
                case LANGUAGE_KOREAN:
                    bLine = 2;
                    bLine1MaxLeng = 10;
                break;
                #endif
            }
        break;

        case WMSG_MSG_EMERGENCY_FILE_FULL:
            //call AHC_WMSG_SetStringID(WMSG_MSG_EMERGENCY_FILE_FULL, StringID) before this.
            iStrID = m_uiWMSGStringID[WMSG_MSG_EMERGENCY_FILE_FULL];
            //iStrID = IDS_DS_MSG_EMERGENCY_FILE_FULL;
        break;

        case WMSG_NO_CARD:
            iStrID = IDS_DS_MSG_NO_CARD;
        break;

        case WMSG_LOW_BATTERY:
            iStrID = IDS_DS_MSG_LOW_BATTERY;
        break;

        case WMSG_FILE_ERROR:
            iStrID = IDS_DS_MSG_FILE_ERROR;
        break;

        case WMSG_CARD_LOCK:
            iStrID = IDS_DS_MSG_CARD_LOCK;
        break;

        case WMSG_CARD_SLOW:
            iStrID = IDS_DS_MSG_CARD_SLOW;
            bLine = 2;
        break;

        case WMSG_CANNOT_DELETE:
            iStrID = IDS_DS_MSG_CANNOT_DELETE;
            #ifdef CFG_CUS_DRAW_MULTI_LINE // TBD
            switch (MenuSettingConfig()->uiLanguage)
            {
                #if (MENU_GENERAL_LANGUAGE_ENGLISH_EN)
                case LANGUAGE_ENGLISH:
                break;
                #endif
                #if (MENU_GENERAL_LANGUAGE_TCHINESE_EN)
                case LANGUAGE_TCHINESE:

                break;
                #endif
                #if (MENU_GENERAL_LANGUAGE_JAPANESE_EN)
                case LANGUAGE_JAPANESE:
                    bLine = 2;
                    bLine1MaxLeng = 22;
                break;
                #endif
                #if (MENU_GENERAL_LANGUAGE_KOREAN_EN)
                case LANGUAGE_KOREAN:
                    bLine = 2;
                    bLine1MaxLeng = 22;
                break;
                #endif
            }
            #endif
        break;

        case WMSG_PLUG_OUT_SD_CARD:
            iStrID = IDS_DS_MSG_PLUG_OUT_SD;
        break;

        case WMSG_WAIT_INITIAL_DONE:
            iStrID = IDS_DS_MSG_WAIT_INITIAL_DONE;
        break;

        case WMSG_INSERT_SD_AGAIN:
            iStrID = IDS_DS_MSG_INSERT_SD_AGAIN;
        break;

        case WMSG_FORMAT_SD_CARD_OK:
            iStrID = IDS_DS_MSG_FORMAT_SD_CARD_OK;
        break;

        case WMSG_FORMAT_SD_CARD_FAIL:
            iStrID = IDS_DS_MSG_FORMAT_SD_CARD_FAIL;
        break;

        case WMSG_ADAPTER_ERROR:
            iStrID = IDS_DS_MSG_ADAPTER_ERROR;

            switch(MenuSettingConfig()->uiLanguage) {
            #if (MENU_GENERAL_LANGUAGE_SPANISH_EN)
            case LANGUAGE_SPANISH:
            break;
            #endif
            #if (MENU_GENERAL_LANGUAGE_PORTUGUESE_EN)
            case LANGUAGE_PORTUGUESE:
                bLine = 2;
                bLine1MaxLeng = 29;
            break;
            #endif
            #if (MENU_GENERAL_LANGUAGE_RUSSIAN_EN)
            case LANGUAGE_RUSSIAN:
                //bLine = 2;
                //bLine1MaxLeng = 25;
            break;
            #endif
            }
        break;

        case WMSG_INVALID_OPERATION:
            iStrID = IDS_DS_MSG_INVALID_OPERATION;
        break;

        case WMSG_BATTERY_FULL:
            iStrID = IDS_DS_MSG_BATTERY_FULL;
        break;

        case WMSG_LENS_ERROR:
            iStrID = IDS_DS_MSG_LENS_ERROR;
            bLine = 2;
        break;

        case WMSG_HDMI_TV:
            iStrID = IDS_DS_MSG_HDMI_TV;
        break;

        case WMSG_FHD_VR:
            iStrID = IDS_DS_MSG_FHD_VR;

            switch(MenuSettingConfig()->uiLanguage) {
            #if (MENU_GENERAL_LANGUAGE_ENGLISH_EN)
            case LANGUAGE_ENGLISH:
                bLine = 2;
                bLine1MaxLeng = 37;
            break;
            #endif
            #if (MENU_GENERAL_LANGUAGE_SPANISH_EN)
            case LANGUAGE_SPANISH:
                bLine = 2;
                bLine1MaxLeng = 38;
            break;
            #endif
            #if (MENU_GENERAL_LANGUAGE_PORTUGUESE_EN)
            case LANGUAGE_PORTUGUESE:
                bLine = 2;
                bLine1MaxLeng = 38;
            break;
            #endif
            #if (MENU_GENERAL_LANGUAGE_RUSSIAN_EN)
            case LANGUAGE_RUSSIAN:
                bLine = 2;
                bLine1MaxLeng = 63;
            break;
            #endif
            }
        break;

        case WMSG_FORMAT_SD_PROCESSING:
            iStrID = IDS_DS_MSG_FORMAT_SD_PROCESSING;
        break;

        case WMSG_SEAMLESS_ERROR:
            iStrID = IDS_DS_MSG_CYCLE_RECORD_CLEAN_SPACE;
        break;

        case WMSG_FORMAT_SD_CARD:
            iStrID = IDS_DS_MSG_FORMAT_SD_CARD;
        break;

        case WMSG_LOCK_CURRENT_FILE:
            iStrID = IDS_DS_MSG_LOCK_CURRENT_FILE;
        break;

        case WMSG_UNLOCK_CUR_FILE:
            iStrID = IDS_DS_MSG_UNLOCK_FILE;
        break;

        case WMSG_DELETE_FILE_OK:
            iStrID = IDS_DS_MSG_DELETE_FILE_OK;
        break;

        case WMSG_PROTECT_FILE_OK:
            iStrID = IDS_DS_MSG_PROTECT_FILE_OK;
        break;

        case WMSG_GOTO_POWER_OFF:
            iStrID = IDS_DS_MSG_GOTO_POWER_OFF;
        break;

        case WMSG_NO_FILE_IN_BROWSER:
            iStrID = IDS_DS_MSG_NO_FILE;
        break;

        case WMSG_CAPTURE_CUR_FRAME:
            iStrID = IDS_DS_MSG_CAPTURE_SCREEN;
        break;

        #ifdef WMSG_SHOW_WAIT
        case WMSG_OPENFILE_WAIT:
            iStrID = IDS_DS_MSG_OPENFILE_WAIT;
        break;
        #endif

        case WMSG_SHOW_FW_VERSION:
            iStrID = IDS_DS_FW_VERSION_INFO;
            bLine = 2;
        break;




        case WMSG_FATIGUEALERT:
                iStrID = IDS_DS_NO;
                m_WMSGShowTime = WMSGShowTime;
                m_WMSGInfo     = WMSGInfo;
        break;
        default:
            printc(FG_RED("Check: WMSGInfo = %d\r\n"), WMSGInfo);
            goto Label_AHC_WMSG_END;
        break;
        }

#ifdef FLM_GPIO_NUM
        AHC_OS_SleepMs(100);
#endif

    #ifdef CUS_WSG_DRAW
        CUS_WSG_DRAW
    #else
        AHC_OSDSetFont(m_WMSG.DisplayID, &GUI_Font24_1);

    #ifdef AHC_WMSG_WITHOUT_LINE_SETTING
    bLine = 1;
    #endif

        if(bLine == 1) {
            MEMCPY(Temp1Str, (const char*)OSD_GetStringViaID(iStrID), sizeof(Temp1Str));
            if (find_newline(Temp1Str, Temp2Str) == 1)
                OSD_ShowStringPool(m_WMSG.DisplayID, iStrID, StrRECT, AHC_WMSG_FORE_COLOR/*0xAA0000FF*/, m_WMSG.byBkColor, GUI_TA_CENTER|GUI_TA_VCENTER);
            else
                OSD_ShowStringPool2(m_WMSG.DisplayID, Temp1Str, Temp2Str, StrRECT, AHC_WMSG_FORE_COLOR/*0xAA0000FF*/, m_WMSG.byBkColor, GUI_TA_CENTER|GUI_TA_VCENTER);
        }
        else {
            UINT8 bLine2Leng = 0;

            switch(WMSGInfo) {
            case WMSG_CARD_SLOW:
                MEMCPY(Temp1Str, (const char*)OSD_GetStringViaID(IDS_DS_MSG_CARD_SLOW), sizeof(Temp1Str));
                MEMCPY(Temp2Str, (const char*)OSD_GetStringViaID(IDS_DS_MSG_USE_RECOMMENDED), sizeof(Temp2Str));
            break;
            case WMSG_LENS_ERROR:
                MEMCPY(Temp1Str, (const char*)OSD_GetStringViaID(IDS_DS_MSG_LENS_ERROR), sizeof(Temp1Str));
                sprintf(Temp2Str, "%d", m_LensError);
                m_LensError = 0;
            break;
            case WMSG_SHOW_FW_VERSION:
                MEMCPY(Temp1Str, (const char*)OSD_GetStringViaID(IDS_DS_FW_VERSION_INFO), sizeof(Temp1Str));
                sprintf(Temp2Str,"%d",AHC_GetMinorVersion());
            break;
            default:
                MEMCPY(TempStr, (const char*)OSD_GetStringViaID(iStrID), sizeof(TempStr));
                MEMCPY(Temp1Str, (const char*)TempStr, bLine1MaxLeng);
                Temp1Str[sizeof(Temp1Str) - 1] = 0;

                if (strlen(TempStr) > bLine1MaxLeng) {
                    bLine2Leng = find_newline2(TempStr, bLine1MaxLeng);
                    Temp1Str[bLine2Leng] = 0;
                    MEMCPY(Temp2Str, TempStr+bLine2Leng, sizeof(Temp2Str));
                }
                else
                {
                    Temp2Str[0] = 0;
                }
            break;
            }

            #ifdef CFG_CUS_DRAW_MULTI_LINE // TBD
            if (bLine == 2)
            {
                OSD_ShowStringPoolByCase(m_WMSG.DisplayID, Temp1Str, Temp2Str, StrRECT, AHC_WMSG_FORE_COLOR/*0xAA0000FF*/, m_WMSG.byBkColor, GUI_TA_CENTER|GUI_TA_VCENTER, -10);
            }
            else
            #endif
            {
                OSD_ShowStringPool2(m_WMSG.DisplayID, Temp1Str, Temp2Str, StrRECT, AHC_WMSG_FORE_COLOR/*0xAA0000FF*/, m_WMSG.byBkColor, GUI_TA_CENTER|GUI_TA_VCENTER);
            }
        }

        AHC_OSDSetColor(m_WMSG.DisplayID, m_WMSG.byBoundColor);
        AHC_OSDSetPenSize(m_WMSG.DisplayID, 3);
        AHC_OSDDrawLine(m_WMSG.DisplayID, m_WMSG.StartX+1, m_WMSG.StartX+m_WMSG.DisplayWidth-1, m_WMSG.StartY+1, m_WMSG.StartY+1);
        AHC_OSDDrawLine(m_WMSG.DisplayID, m_WMSG.StartX+1, m_WMSG.StartX+m_WMSG.DisplayWidth-1, m_WMSG.StartY+m_WMSG.DisplayHeight-1, m_WMSG.StartY+m_WMSG.DisplayHeight-1);
        AHC_OSDDrawLine(m_WMSG.DisplayID, m_WMSG.StartX+1, m_WMSG.StartX+1, m_WMSG.StartY+1, m_WMSG.StartY+m_WMSG.DisplayHeight-1);
        AHC_OSDDrawLine(m_WMSG.DisplayID, m_WMSG.StartX+m_WMSG.DisplayWidth-1, m_WMSG.StartX+m_WMSG.DisplayWidth-1, m_WMSG.StartY+1, m_WMSG.StartY+m_WMSG.DisplayHeight-1);

        #ifndef AHC_WMSG_WITHOUT_WMSG_Icon
        AHC_OSDDrawBitmap(m_WMSG.DisplayID, &BM_ICON_WMSG, m_WMSG.StartX+5, m_WMSG.StartY+5);
        #endif

        OSDDraw_SetSemiTransparent(m_WMSG.DisplayID, 1, AHC_OSD_SEMITP_AVG, 240);
    #endif

        AHC_OSDSetCurrentDisplay(m_WMSG.DisplayID);

        AHC_OSDSetActive(m_WMSG.DisplayID, 1);

        m_WMSGShowTime = WMSGShowTime;
        m_WMSGInfo     = WMSGInfo;
        #if defined(WIFI_PORT) && (WIFI_PORT == 1)
        ncgi_set_warning_msg(m_WMSGInfo);
        #endif
    }
    else {
        AHC_OSDSetColor(m_WMSG.DisplayID, OSD_COLOR_TRANSPARENT);

        /* Disable WMSG window to save hardware badndwidth */
        if (WMSG_LAYER_WINDOW_ID != OSD_LAYER_WINDOW_ID)
            AHC_OSDSetActive(m_WMSG.DisplayID, 0);

        #ifdef FLM_GPIO_NUM
        AHC_OS_SleepMs(100);
        #endif

        #if (ENABLE_ADAS_LDWS)
        if(m_bDrawLDWS)
        {
            m_bDrawLDWS = AHC_FALSE;
            AHC_OSDDrawFillRect(m_WMSG.DisplayID, 0, 0, (UINT16) m_OSD[m_WMSG.DisplayID]->width, (UINT16) m_OSD[m_WMSG.DisplayID]->height);
        }
        else
        #endif
        {
            AHC_OSDDrawFillRect(m_WMSG.DisplayID, m_WMSG.StartX, m_WMSG.StartY-1, m_WMSG.StartX+m_WMSG.DisplayWidth, m_WMSG.StartY+m_WMSG.DisplayHeight+1);
        }

        #if (DRIVING_SAFETY_REMIND_HEADLIGHT)
        if (m_bDrawRemindHeadlight)
        {
            m_bDrawRemindHeadlight = AHC_FALSE;
            AHC_OSDDrawFillRect(m_WMSG.DisplayID, 0, 0, (UINT16)m_OSD[m_WMSG.DisplayID]->width, (UINT16)m_OSD[m_WMSG.DisplayID]->height);
        }
        else
        {
            AHC_OSDDrawFillRect(m_WMSG.DisplayID, m_WMSG.StartX, m_WMSG.StartY-1, m_WMSG.StartX+m_WMSG.DisplayWidth, m_WMSG.StartY+m_WMSG.DisplayHeight+1);
        }
        #endif

        m_WMSGInfo = WMSG_NONE;
    }

Label_AHC_WMSG_END:

    END_LAYER(m_WMSG.DisplayID);
    AHC_OSDRefresh();

    #if (USE_WMSG_SEMI)
    AHC_OS_ReleaseSem(m_WMSGSemID);
    #endif

#endif//NO_PANEL

    return AHC_TRUE;
}

static int find_newline(char *line1, char *line2)
{
    static char *newline = "\\\\";
    char    *p;

    p = strstr(line1, newline);

    if (p == NULL) {
        return 1;
    }
    *p = 0;
    STRCPY(line2, p + 2);
    return 2;
}

static unsigned char find_newline2(char *line1, unsigned char len)
{
    char *p = line1;
    unsigned char pos = 0;

    while (*line1) {
        if ((*line1 > 0xA0) && (*line1 <= 0xAF)) {
            p += 2;

            if (((unsigned int) p - (unsigned int) line1) >= len)
                break;

            pos += 2;
        }
        else {
            // ASCII
            p++;

            if (((unsigned int) p - (unsigned int) line1) >= len)
                break;

            pos += 1;
        }
    }

    return pos;
}

#if (ENABLE_ADAS_LDWS)
void LDWS_DrawLaneLine(UINT16 ubID, AHC_BOOL bClear, ldws_params_t* preLDWS_attribute, ldws_params_t* curLDWS_attribute, GUI_COLOR left_lane_Color, GUI_COLOR right_lane_Color )
{
    #ifdef CUS_DRAW_LDWS_LANE_LINE  // maybe defined in Config_SDK.h

    CUS_DRAW_LDWS_LANE_LINE(bClear, preLDWS_attribute, curLDWS_attribute, left_lane_Color, right_lane_Color);

    #else

    UINT16 usDispalyWidth, usDispalyHeight;
    UINT32 ulPrevieweWidth, ulPrevieweHeight;
    UINT32 uiLeftStartX = 0;
    UINT32 uiLeftStartY = 0;
    UINT32 uiLeftEndX   = 0;
    UINT32 uiLeftEndY   = 0;
    UINT32 uiRightStartX = 0;
    UINT32 uiRightStartY = 0;
    UINT32 uiRightEndX   = 0;
    UINT32 uiRightEndY   = 0;
    UINT32 ulOffsetX, ulOffsetY;

#if (DRAW_WARNING_MSG_ENABLE == 0)
    return;
#endif

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }

    BEGIN_LAYER(ubID);

    MMPS_3GPRECD_GetPreviewRes(gsAhcPrmSensor, &ulPrevieweWidth, &ulPrevieweHeight);
    OSDDraw_GetDisplaySize( ubID, &usDispalyWidth, &usDispalyHeight );

    ulOffsetX = abs(usDispalyWidth*OSD_DISPLAY_SCALE_LCD - ulPrevieweWidth) / 2;
    ulOffsetY = abs(usDispalyHeight*OSD_DISPLAY_SCALE_LCD - ulPrevieweHeight) / 2;

    //printc("--%s-- DisplaySize: %d x %d\r\n", __func__, usDispalyWidth, usDispalyHeight);
    //printc("                      PreviewSize: %d x %d\r\n", ulPrevieweWidth, ulPrevieweHeight);

    if ((bClear == AHC_TRUE) && preLDWS_attribute)
    {
        AHC_OSDSetPenSize(ubID, 3);
        AHC_OSDSetColor(ubID, OSD_COLOR_TRANSPARENT);

        if ( preLDWS_attribute->left_lane[0].x < gstLdwsCfg[PRM_SENSOR].width    &&
             preLDWS_attribute->left_lane[0].y < gstLdwsCfg[PRM_SENSOR].height   &&
             preLDWS_attribute->left_lane[1].x < gstLdwsCfg[PRM_SENSOR].width    &&
             preLDWS_attribute->left_lane[1].y < gstLdwsCfg[PRM_SENSOR].height )
        {
            uiLeftStartX = ( preLDWS_attribute->left_lane[0].x * usDispalyWidth) / gstLdwsCfg[PRM_SENSOR].width + ulOffsetX;
            uiLeftStartY = ( preLDWS_attribute->left_lane[0].y * usDispalyHeight) / gstLdwsCfg[PRM_SENSOR].height + ulOffsetY;
            uiLeftEndX   = ( preLDWS_attribute->left_lane[1].x * usDispalyWidth) / gstLdwsCfg[PRM_SENSOR].width + ulOffsetX;
            uiLeftEndY   = ( preLDWS_attribute->left_lane[1].y * usDispalyHeight) / gstLdwsCfg[PRM_SENSOR].height + ulOffsetY;

            if( uiLeftStartX < usDispalyWidth  ||
                uiLeftEndX   < usDispalyWidth  ||
                uiLeftStartY < usDispalyHeight ||
                uiLeftEndY   < usDispalyHeight )
            {
                AHC_OSDDrawLine( ubID, uiLeftStartX, uiLeftEndX, uiLeftStartY, uiLeftEndY);
            }
        }

        if( preLDWS_attribute->right_lane[0].x < gstLdwsCfg[PRM_SENSOR].width    &&
            preLDWS_attribute->right_lane[0].y < gstLdwsCfg[PRM_SENSOR].height   &&
            preLDWS_attribute->right_lane[1].x < gstLdwsCfg[PRM_SENSOR].width    &&
            preLDWS_attribute->right_lane[1].y < gstLdwsCfg[PRM_SENSOR].height )
        {
            uiRightStartX = ( preLDWS_attribute->right_lane[0].x * usDispalyWidth  ) / gstLdwsCfg[PRM_SENSOR].width + ulOffsetX;
            uiRightStartY = ( preLDWS_attribute->right_lane[0].y * usDispalyHeight) / gstLdwsCfg[PRM_SENSOR].height + ulOffsetY;
            uiRightEndX   = ( preLDWS_attribute->right_lane[1].x * usDispalyWidth  ) / gstLdwsCfg[PRM_SENSOR].width + ulOffsetX;
            uiRightEndY   = ( preLDWS_attribute->right_lane[1].y * usDispalyHeight) / gstLdwsCfg[PRM_SENSOR].height + ulOffsetY;

            if( uiRightStartX < usDispalyWidth  ||
                uiRightEndX   < usDispalyWidth  ||
                uiRightStartY < usDispalyHeight ||
                uiRightEndY   < usDispalyHeight )
            {
                AHC_OSDDrawLine( ubID, uiRightStartX, uiRightEndX, uiRightStartY, uiRightEndY);
            }
        }
    }

    if (curLDWS_attribute)
    {
        if ( curLDWS_attribute->left_lane[0].x < gstLdwsCfg[PRM_SENSOR].width    &&
             curLDWS_attribute->left_lane[0].y < gstLdwsCfg[PRM_SENSOR].height   &&
             curLDWS_attribute->left_lane[1].x < gstLdwsCfg[PRM_SENSOR].width    &&
             curLDWS_attribute->left_lane[1].y < gstLdwsCfg[PRM_SENSOR].height )
        {
            uiLeftStartX = ( curLDWS_attribute->left_lane[0].x * usDispalyWidth) / gstLdwsCfg[PRM_SENSOR].width + ulOffsetX;
            uiLeftStartY = ( curLDWS_attribute->left_lane[0].y * usDispalyHeight) / gstLdwsCfg[PRM_SENSOR].height + ulOffsetY;
            uiLeftEndX   = ( curLDWS_attribute->left_lane[1].x * usDispalyWidth) / gstLdwsCfg[PRM_SENSOR].width + ulOffsetX;
            uiLeftEndY   = ( curLDWS_attribute->left_lane[1].y * usDispalyHeight) / gstLdwsCfg[PRM_SENSOR].height + ulOffsetY;

            if( uiLeftStartX < usDispalyWidth  &&
                uiLeftStartY < usDispalyHeight &&
                uiLeftEndX   < usDispalyWidth  &&
                uiLeftEndY   < usDispalyHeight )
            {
    			//printc("L:%d,%d - %d,%d\r\n", uiLeftStartX, uiLeftStartY, uiLeftEndX, uiLeftEndY);

                AHC_OSDSetPenSize(ubID, 3);
                AHC_OSDSetColor(ubID, left_lane_Color);

                AHC_OSDDrawLine( ubID, uiLeftStartX, uiLeftEndX, uiLeftStartY, uiLeftEndY);
            }
        }

        if( curLDWS_attribute->right_lane[0].x < gstLdwsCfg[PRM_SENSOR].width    &&
            curLDWS_attribute->right_lane[0].y < gstLdwsCfg[PRM_SENSOR].height   &&
            curLDWS_attribute->right_lane[1].x < gstLdwsCfg[PRM_SENSOR].width    &&
            curLDWS_attribute->right_lane[1].y < gstLdwsCfg[PRM_SENSOR].height )
        {
            uiRightStartX = ( curLDWS_attribute->right_lane[0].x * usDispalyWidth) / gstLdwsCfg[PRM_SENSOR].width + ulOffsetX;
            uiRightStartY = ( curLDWS_attribute->right_lane[0].y * usDispalyHeight) / gstLdwsCfg[PRM_SENSOR].height + ulOffsetY;
            uiRightEndX   = ( curLDWS_attribute->right_lane[1].x * usDispalyWidth) / gstLdwsCfg[PRM_SENSOR].width + ulOffsetX;
            uiRightEndY   = ( curLDWS_attribute->right_lane[1].y * usDispalyHeight) / gstLdwsCfg[PRM_SENSOR].height + ulOffsetY;

            if( uiRightStartX < usDispalyWidth  &&
                uiRightEndX   < usDispalyWidth  &&
                uiRightStartY < usDispalyHeight &&
                uiRightEndY   < usDispalyHeight  )
            {
    			//printc("R:%d,%d - %d,%d\r\n", uiRightStartX, uiRightStartY, uiRightEndX, uiRightEndY);

                AHC_OSDSetPenSize(ubID, 3);
                AHC_OSDSetColor(ubID, right_lane_Color);

                AHC_OSDDrawLine( ubID, uiRightStartX, uiRightEndX, uiRightStartY, uiRightEndY);
            }
        }
    }

    END_LAYER(ubID);



    #endif
}

void AHC_WMSG_LDWS_Line(UINT16 ubID, AHC_BOOL needClean)
{
#ifdef CUS_WMSG_LDWS_LINE   // maybe defined in Config_SDK.h

#if defined(CCIR656_OUTPUT_ENABLE) && (CCIR656_OUTPUT_ENABLE)
    return;
#endif

#if (DRAW_WARNING_MSG_ENABLE == 0)
    return;
#endif

    CUS_WMSG_LDWS_LINE(needClean);

#else

    static AHC_BOOL printed_low_speed_msg = AHC_FALSE;
    MMP_LONG  ulAlert;
    ldws_params_t cur_ldws_attribute = {0};

#if defined(CCIR656_OUTPUT_ENABLE) && (CCIR656_OUTPUT_ENABLE)
    return;
#endif

#if (DRAW_WARNING_MSG_ENABLE == 0)
    return;
#endif

    if (((m_WMSGInfo != WMSG_NONE) && (m_WMSGInfo != WMSG_LDWS_RightShift) && (m_WMSGInfo != WMSG_LDWS_LeftShift)) || IN_MENU()) {
        return;
    }

    // For LDWS
    {
        UINT32 uiAdasFlag = 0;
        AHC_GetParam( PARAM_ID_ADAS, &uiAdasFlag );

        if ((uiAdasFlag & AHC_ADAS_ENABLED_LDWS) == 0) {
            return;
        }
    }

#if (GPS_CONNECT_ENABLE && LDWS_WARN_MIN_SPEED)
#define LDWS_TEST_FORCE_SPEED   (70)
#ifdef LDWS_TEST_FORCE_SPEED
    if(LDWS_TEST_FORCE_SPEED < LDWS_WARN_MIN_SPEED)
#else
    if(GPSCtrl_GetSpeed(0) < LDWS_WARN_MIN_SPEED)
#endif
    {
        if (printed_low_speed_msg == AHC_FALSE) {
            printc("--W-- Speed is small than %dKM/H\r\n", LDWS_WARN_MIN_SPEED);
            printed_low_speed_msg = AHC_TRUE;
        }

        return;
    }
#endif

    ADAS_CTL_GetLDWSAttr(&cur_ldws_attribute, &ulAlert);
    printed_low_speed_msg = AHC_FALSE;

#if defined(WIFI_PORT) && (WIFI_PORT == 1)
    if (AHC_Get_WiFi_Streaming_Status() == AHC_FALSE)
#endif
    {
        #if (OSD_SHOW_LDWS_LANE_LINE)
        static ldws_params_t last_ldws_attribute = {0};

        if (cur_ldws_attribute.state != LDWS_STATE_NODETECT)
        {
            if( last_ldws_attribute.left_lane[0].x != cur_ldws_attribute.left_lane[0].x  ||
                last_ldws_attribute.left_lane[0].y != cur_ldws_attribute.left_lane[0].y  ||
                last_ldws_attribute.left_lane[1].x != cur_ldws_attribute.left_lane[1].x  ||
                last_ldws_attribute.left_lane[1].y != cur_ldws_attribute.left_lane[1].y  ||
                last_ldws_attribute.right_lane[0].x != cur_ldws_attribute.right_lane[0].x  ||
                last_ldws_attribute.right_lane[0].y != cur_ldws_attribute.right_lane[0].y  ||
                last_ldws_attribute.right_lane[1].x != cur_ldws_attribute.right_lane[1].x  ||
                last_ldws_attribute.right_lane[1].y != cur_ldws_attribute.right_lane[1].y  )
            {
                if (ulAlert == LDWS_STATE_DEPARTURE_LEFT)
                {   // Left Shift
                    LDWS_DrawLaneLine( ubID, AHC_TRUE, &last_ldws_attribute, &cur_ldws_attribute, OSD_COLOR_RED, OSD_COLOR_BLUE );
                }
                else if(ulAlert == LDWS_STATE_DEPARTURE_RIGHT)
                {   // Right Shift
                    LDWS_DrawLaneLine( ubID, AHC_TRUE, &last_ldws_attribute, &cur_ldws_attribute, OSD_COLOR_BLUE, OSD_COLOR_RED );
                }
                else
                {
                    LDWS_DrawLaneLine( ubID, AHC_TRUE, &last_ldws_attribute, &cur_ldws_attribute, OSD_COLOR_BLUE, OSD_COLOR_BLUE);
                }

                last_ldws_attribute.left_lane[0].x = cur_ldws_attribute.left_lane[0].x;
                last_ldws_attribute.left_lane[0].y = cur_ldws_attribute.left_lane[0].y;
                last_ldws_attribute.left_lane[1].x = cur_ldws_attribute.left_lane[1].x;
                last_ldws_attribute.left_lane[1].y = cur_ldws_attribute.left_lane[1].y;
                last_ldws_attribute.right_lane[0].x = cur_ldws_attribute.right_lane[0].x;
                last_ldws_attribute.right_lane[0].y = cur_ldws_attribute.right_lane[0].y;
                last_ldws_attribute.right_lane[1].x = cur_ldws_attribute.right_lane[1].x;
                last_ldws_attribute.right_lane[1].y = cur_ldws_attribute.right_lane[1].y;
            }
        }

        // Draw Calibration Line
        #ifdef OSD_SHOW_LDWS_CALI_LANE_LINE
        if (OSD_SHOW_LDWS_CALI_LANE_LINE)
        {
            printc("Draw CALI Lane\r\n");
            cur_ldws_attribute.left_lane[0].x = ldwsTuningParms.laneCalibrationUpPointX;
            cur_ldws_attribute.left_lane[0].y = ldwsTuningParms.laneCalibrationUpPointY;
            cur_ldws_attribute.left_lane[1].x = ldwsTuningParms.laneCalibrationLeftPointX;
            cur_ldws_attribute.left_lane[1].y = ldwsTuningParms.laneCalibrationLeftPointY;
            cur_ldws_attribute.right_lane[0].x = ldwsTuningParms.laneCalibrationUpPointX;
            cur_ldws_attribute.right_lane[0].y = ldwsTuningParms.laneCalibrationUpPointY;
            cur_ldws_attribute.right_lane[1].x = ldwsTuningParms.laneCalibrationRightPointX;
            cur_ldws_attribute.right_lane[1].y = ldwsTuningParms.laneCalibrationRightPointY;

            LDWS_DrawLaneLine( ubID, AHC_FALSE, NULL, &cur_ldws_attribute, OSD_COLOR_WHITE, OSD_COLOR_WHITE);
        }
        #endif

        #endif
    }



#endif  // CUS_WMSG_LDWS_LINE
}
#endif

#if (ENABLE_ADAS_FCWS)
void AHC_WMSG_FCWS_Info(UINT16 ubID, AHC_BOOL needClean)
{
#ifdef CUS_WMSG_FCWS_INFO   // maybe defined in Config_SDK.h
#if defined(CCIR656_OUTPUT_ENABLE) && (CCIR656_OUTPUT_ENABLE)
    return;
#endif

#if (DRAW_WARNING_MSG_ENABLE == 0)
    return;
#endif

    CUS_WMSG_FCWS_INFO(needClean);

#else

#if defined(CCIR656_OUTPUT_ENABLE) && (CCIR656_OUTPUT_ENABLE)
    return;
#endif

#if (DRAW_WARNING_MSG_ENABLE == 0)
    return;
#endif

    if (((m_WMSGInfo != WMSG_NONE) && (m_WMSGInfo != WMSG_LDWS_RightShift) && (m_WMSGInfo != WMSG_LDWS_LeftShift)) || IN_MENU()) {
        return;
    }

    {
        UINT32 uiAdasFlag = 0;
        AHC_GetParam( PARAM_ID_ADAS, &uiAdasFlag );

        if ( uiAdasFlag & AHC_ADAS_ENABLED_FCWS ) {
            fcws_info_t cur_fcws_info = {0};

            ADAS_CTL_GetFCWSAttr(&cur_fcws_info);

#if (OSD_SHOW_FCWS_RESULT == 1) // Show FCWS result on display
            #if defined(WIFI_PORT) && (WIFI_PORT == 1)
            if (AHC_Get_WiFi_Streaming_Status() == AHC_FALSE)
            #endif
            {
                static fcws_info_t last_fcws_info = {0};


                BEGIN_LAYER(ubID);

                if (needClean) {
                    if (last_fcws_info.car_x || last_fcws_info.car_y || last_fcws_info.car_width || last_fcws_info.car_height) {
                        RECT     stRect;

                        AHC_OSDSetColor(ubID, AHC_OSDGetBkColor(ubID));
                        AHC_OSDSetPenSize(ubID, 4);

                        stRect.uiLeft = last_fcws_info.car_x;
                        stRect.uiTop = last_fcws_info.car_y;
                        stRect.uiWidth = last_fcws_info.car_width;
                        stRect.uiHeight = last_fcws_info.car_height;
                        MEMSET(&last_fcws_info, 0, sizeof(fcws_info_t));

                        AHC_OSDDrawFillRect2(ubID, &stRect);
                    }
                }

                if( cur_fcws_info.state == FCWS_STATE_FIND )
                {
                    GUI_COLOR displayColor = OSD_COLOR_GREEN;
                    GUI_RECT stGuiRect;
                    UINT8   sz[6] = {0};
                    UINT16 uiDispalyWidth;
                    UINT16 uiDispalyHeight;

                    if( cur_fcws_info.distance < FCWS_WARN_DISTANCE )
                        displayColor = OSD_COLOR_RED;

                    OSDDraw_GetDisplaySize( ubID, &uiDispalyWidth, &uiDispalyHeight );

                    AHC_OSDSetFont(ubID, &GUI_Font16B_1);
                    AHC_OSDSetColor(ubID, displayColor);
                    AHC_OSDSetPenSize(ubID, 4);

                    cur_fcws_info.car_x = ( cur_fcws_info.car_x * uiDispalyWidth  ) / gstLdwsCfg[PRM_SENSOR].width;
                    cur_fcws_info.car_y = ( cur_fcws_info.car_y * uiDispalyHeight ) / gstLdwsCfg[PRM_SENSOR].height;
                    cur_fcws_info.car_width =  ( cur_fcws_info.car_width * uiDispalyWidth  ) / gstLdwsCfg[PRM_SENSOR].width;
                    cur_fcws_info.car_height = ( cur_fcws_info.car_height * uiDispalyHeight ) / gstLdwsCfg[PRM_SENSOR].height;

                    // Make sure the text window is big enough
                    if( cur_fcws_info.car_width < 30)
                        cur_fcws_info.car_width = 30;

                    if( cur_fcws_info.car_height < 20)
                        cur_fcws_info.car_height = 20;

#ifdef FCWS_INFO_DISPLAY_REGION_LIMIT
                    {
                        UINT16 left_bound = 0, right_bound = 0xFFFF, top_bound = 0, bottom_bound = 0xFFFF;

                        #if (HDMI_ENABLE && defined(FCWS_INFO_DISPLAY_REGION_LIMIT_HDMI))
                        if (AHC_IsHdmiConnect()) {
                            left_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_HDMI_LB;
                            right_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_HDMI_RB;
                            top_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_HDMI_TB;
                            bottom_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_HDMI_BB;
                        }
                        else
                        #endif
                        #if (TVOUT_ENABLE && defined(FCWS_INFO_DISPLAY_REGION_LIMIT_TV))
                        if(AHC_IsTVConnectEx())
                        {
                            left_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_TV_LB;
                            right_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_TV_RB;
                            top_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_TV_TB;
                            bottom_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_TV_BB;
                        }
                        else
                        #endif
                        #if defined(FCWS_INFO_DISPLAY_REGION_LIMIT_LCD)
                        {
                            left_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_LCD_LB;
                            right_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_LCD_RB;
                            top_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_LCD_TB;
                            bottom_bound = FCWS_INFO_DISPLAY_REGION_LIMIT_LCD_BB;
                        }
                        #endif

                        if (cur_fcws_info.car_x < left_bound)
                            cur_fcws_info.car_x = left_bound;

                        if ((cur_fcws_info.car_x + cur_fcws_info.car_width) > right_bound) {
                            if (cur_fcws_info.car_width > (right_bound - left_bound)) {
                                cur_fcws_info.car_width = right_bound - left_bound;
                                cur_fcws_info.car_x = left_bound;
                            }
                            else {
                                cur_fcws_info.car_x = right_bound - cur_fcws_info.car_width;
                            }
                        }

                        if (cur_fcws_info.car_y < top_bound)
                            cur_fcws_info.car_y = top_bound;

                        if ((cur_fcws_info.car_y + cur_fcws_info.car_height) > bottom_bound) {
                            if (cur_fcws_info.car_height > (bottom_bound - top_bound)) {
                                cur_fcws_info.car_height = bottom_bound - top_bound;
                                cur_fcws_info.car_y = top_bound;
                            }
                            else {
                                cur_fcws_info.car_y = bottom_bound - cur_fcws_info.car_height;
                            }
                        }
                    }
#endif

                    cur_fcws_info.car_x &= 0xFFFFFFFE;
                    cur_fcws_info.car_y &= 0xFFFFFFFE;
                    cur_fcws_info.car_width &= 0xFFFFFFFE;
                    cur_fcws_info.car_height &= 0xFFFFFFFE;
                    MEMCPY(&last_fcws_info, &cur_fcws_info, sizeof(fcws_info_t));

                    stGuiRect.x0 = cur_fcws_info.car_x;
                    stGuiRect.y0 = cur_fcws_info.car_y;
                    stGuiRect.x1 = cur_fcws_info.car_x + cur_fcws_info.car_width;
                    stGuiRect.y1 = cur_fcws_info.car_y + cur_fcws_info.car_height;

                    AHC_OSDDrawRect(ubID, stGuiRect.x0, stGuiRect.y0, stGuiRect.x1, stGuiRect.y1);

                    if (cur_fcws_info.distance > 100)
                        cur_fcws_info.distance = 99;

                    sprintf((char *) &sz, "%02dm", cur_fcws_info.distance);
                    AHC_OSDDrawTextInRect(ubID, (UINT8 *) &sz, &stGuiRect, GUI_TA_HCENTER | GUI_TA_VCENTER);
                }
                else if (!needClean && (last_fcws_info.car_x || last_fcws_info.car_y || last_fcws_info.car_width || last_fcws_info.car_height)) {
                    RECT     stRect;

                    AHC_OSDSetColor(ubID, AHC_OSDGetBkColor(ubID));
                    AHC_OSDSetPenSize(ubID, 4);

                    stRect.uiLeft = last_fcws_info.car_x;
                    stRect.uiTop = last_fcws_info.car_y;
                    stRect.uiWidth = last_fcws_info.car_width;
                    stRect.uiHeight = last_fcws_info.car_height;
                    MEMSET(&last_fcws_info, 0, sizeof(fcws_info_t));

                    AHC_OSDDrawFillRect2(ubID, &stRect);
                }


                END_LAYER(ubID);
            }
#endif

            if ( cur_fcws_info.state == FCWS_STATE_FIND )
            {
                if ( cur_fcws_info.distance < FCWS_WARN_DISTANCE )
                {
                   // AHC_WMSG_Draw(AHC_TRUE, WMSG_FCWS, 1);
                   //AHC_PlaySoundEffect(AHC_SOUNDEFFECT_FCWS_WARNING); //cancel playsound by polling
                }
            }
        }
    }
#endif  // CUS_WMSG_FCWS_INFO
}
#endif

void AHC_WMSG_SetWMSG(AHC_WMSG_CFG* pSrcWMSG)
{
#if defined(CCIR656_OUTPUT_ENABLE) && (CCIR656_OUTPUT_ENABLE)
    return;
#endif

#if (DRAW_WARNING_MSG_ENABLE == 0)
    return;
#endif

    MEMCPY(&m_WMSG, pSrcWMSG, sizeof(AHC_WMSG_CFG));
}

void AHC_WMSG_GetWMSG(AHC_WMSG_CFG* pDestWMSG)
{
#if defined(CCIR656_OUTPUT_ENABLE) && (CCIR656_OUTPUT_ENABLE)
    return;
#endif

#if (DRAW_WARNING_MSG_ENABLE == 0)
    return;
#endif

    MEMCPY(pDestWMSG, &m_WMSG, sizeof(AHC_WMSG_CFG));
}

