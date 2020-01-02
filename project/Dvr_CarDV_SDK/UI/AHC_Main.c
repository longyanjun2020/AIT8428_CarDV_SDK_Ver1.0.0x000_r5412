/*===========================================================================
 * Include file 
 *===========================================================================*/ 

#include "customer_config.h" 
#include "AHC_Common.h"
#include "AHC_Capture.h"
#include "AHC_General.h"
#include "AHC_Message.h"
#include "AHC_Sensor.h"
#include "AHC_Utility.h"
#include "AHC_GUI.h"
#include "AHC_Os.h"
#include "AHC_Menu.h"
#include "AHC_Version.h"
#include "AHC_Fs.h"
#include "AHC_Media.h"
#include "AHC_Parameter.h"
#include "AHC_Warningmsg.h"
#include "AHC_Display.h"
#include "AHC_General_CarDV.h"
#include "AHC_Video.h"
#include "AHC_USB.h"
#include "LedControl.h"
#include "ZoomControl.h"
#include "StateCameraFunc.h"
#include "dsc_charger.h"
#if (SUPPORT_GSENSOR) 
#include "GSensor_ctrl.h"
#endif
#if (SUPPORT_IR_CONVERTER)
#include "IR_Ctrl.h"
#endif
#if (SUPPORT_GPS) 
#include "GPS_ctl.h"
#endif
#if (TOUCH_UART_FUNC_EN)
#include "Touch_Uart_ctrl.h"
#endif
#include "disp_drv.h"
#include "AIT_Init.h"
#include "AHC_task.h"

#include "StateTVFunc.h"
#include "StateHDMIFunc.h"
#include "hdr_cfg.h"
#include "snr_cfg.h"
#include "mmpf_system.h" //For task monitor //CarDV...
#include "PMUCtrl.h" 
#include "IconPosition.h"
#include "ColorDefine.h"
#include "MenuDrawCommon.h"
#include "AHC_Callback.h"
#include "AHC_UF.h"

#include "fm_i2c.h"
#include "Gsensor_da380andsc7a30e.h"// 4-17

#include "dsc_Key.h"// 4-24
#include "gps_nmea0183.h" // 4-25
#include "mmpf_pio.h"

#include "commanddefs.h"

#if (defined(SUPPORT_SPEECH_RECOG)&&(SUPPORT_SPEECH_RECOG))
#include <math.h>
#include "api_wakeup.h"
#include "aitu_ringbuf.h"
#include "mmpf_vmd.h"
#endif
/*===========================================================================
 * Global variable
 *===========================================================================*/ 

AHC_OS_SEMID 	ahc_System_Ready;
AHC_BOOL		gAHC_InitialDone = AHC_FALSE;
AHC_BOOL        gAHC_UpdateMenuDone = AHC_FALSE;
AHC_BOOL Menu_To_Video_One=AHC_FALSE;// liao
#if (TASK_MONITOR_ENABLE)
MMPF_TASK_MONITOR gsTaskMonitorAhcMain;
MMPF_TASK_MONITOR gsTaskMonitorAHCWorkStack;
MMPF_TASK_MONITOR gsTaskMonitorAHCWorkJob;
#endif
UINT32 ulTimess;


//===============lyj=========

extern MMP_ULONG R,G,B;
AHC_BOOL app_flag = 0;
AHC_BOOL lamp_mode = 0;
MMP_ULONG Freq_config = 22000;//6000;

AHC_BOOL twikle_flag = 0;
AHC_BOOL breath_flag = 0;
short i = 0; 
short j = 0;
//short j1 = 101;
short jp = 101;

AHC_BOOL exit_flag =0;

int  mainVflag_count = 0;
int sub_count_ex	         =0;

AHC_BOOL volume_M = 0;
//AHC_BOOL volume_B = 0;

MMP_USHORT Tfreq = 8750;
unsigned short OldPlayTime = 0;


SetAllFlag SelfAllFlag = {0};

//static INT8 SendMessageMCU = 0;

/*lyj 20190605 define*/

//PowerUpFlag PowerOnFlag ={100,100,100,100,100};
PowerOnSaveTheStatus PowerStatus={0,100,100};

extern MMP_USHORT FMCurruntFreq; // lyj 20190606

AHC_BOOL bl_flag =0; // lyj20190711

/*===========================================================================
 * Extern variable
 *===========================================================================*/ 

extern UARTCOMMAND 	sAhcUartCommand[];

extern MMP_BYTE 		recv_data[15];

//extern AHC_BOOL    bluetooth_key;

extern AHC_BOOL flag_twinkle;

extern AHC_BOOL flag_color;

extern AHC_BOOL	flag_add;
extern  AHC_BOOL	flag_sub;
extern AHC_BOOL sub_flag_ex;

extern AHC_BOOL switch_search;
extern MMP_USHORT radio[31];
extern MMP_USHORT radio_am[21];

extern u8   Valid;
extern __u8    Radiocnt;
extern __u8    RadiocntAm;
extern u16  Freq;
extern AHC_BOOL mcu_flag;
extern AHC_BOOL Mute_ON_OFF;

//extern AHC_BOOL 			display_radio;

extern AHC_BOOL Task_delay_flag;
extern UINT16 wDispID;
AHC_BOOL Task_lockOn_flag = 0;
extern AHC_BOOL     BlueStause;

AHC_BOOL BlueENpin = 0;
//AHC_BOOL 		BlueStauseEx = 100;
AHC_BOOL PowerOnVoiceMode = 0; // lyj 20190622

extern unsigned short play_time_HtoL;
extern unsigned short play_time_T;
/*===========================================================================
 * Extern function
 *===========================================================================*/
extern void 	AIHC_SetTempDCFMemStartAddr(UINT32 addr);
extern void 	AIHC_SetTempBrowserMemStartAddr(UINT32 addr);
extern void		InitKeyPad(void);
extern int      pf_HDR_EnSet(AHC_BOOL value);
extern void 	AHC_PrintAllTask(void);
extern void		AHC_PrintUITask(void);
#if defined(SUPPORT_SPEECH_RECOG)&&(SUPPORT_SPEECH_RECOG)
extern MMP_ULONG LivePCMTransferCB(AUTL_RINGBUF *pPCMOutRing, MMP_ULONG ulUnReadSampleCnt);
extern MMP_ULONG SpeechPCMTransferCB(AUTL_RINGBUF *pPCMOutRing, MMP_ULONG ulUnReadSampleCnt);
#endif

extern void  MenuStateExecutionCommon1(UINT32 ulPosition);// lyj~~~
extern void Menu_Set_Page(AHC_BOOL Flag);

extern AHC_BOOL Menu_Get_Page(void);
extern void twinkle_led(MMP_ULONG ulFreq,UINT32 uiMiliSecond,MMP_ULONG rled,MMP_ULONG gled,MMP_ULONG bled);

extern void Color_chang(MMP_ULONG ulFreq,MMP_ULONG rled,MMP_ULONG gled,MMP_ULONG bled);
extern void get_data_RGB(float *R1,float *G1, float *B1);

extern  void BD3490_reginit(void);

extern void Draw_Mute_icon(RECT tmpRECT6,AHC_BOOL iconFlag);
extern void breathe_lamp(MMP_ULONG lightBar);
extern void Candle_lamp(void);

extern void auto_exit(UINT16 bID0);
extern void White_light_bar_off(void);
extern void Right_left_code(AHC_BOOL codeflag);
extern void InitEncodeGpio( MMP_GPIO_PIN piopin,GpioCallBackFunc* CallBackFunc,MMP_GPIO_TRIG flag);
extern void Draw_Main_volum_icon(int * vicon, AHC_BOOL clear_flag,UINT16 bID0);
unsigned char dealDataUSB(unsigned char passBit[]/*void*/);
/*===========================================================================
 * Main body
 *===========================================================================*/ 

void Clear_array_data(void)
{
	int i;
		for(i = 0; i<15; i++)
		{
			recv_data[i] = 0;

		}

}

#if (TVOUT_PREVIEW_EN)
void InitOSDCustom_Tv(U32 colorFormat, AHC_OSD_ROTATE_DRAW_MODE ahcOSDRotateDraw)
{
    UINT16		uwTVWidth, uwTVHeight;
    UINT16		uwTVOffX, uwTVOffY;
    UINT32		pos[2];
    UINT32		uwDispScale = 0;
    UINT16		uwTVTempWidth, uwTVTempHeight;
    UINT8       ubTVSystem = MenuSettingConfig()->uiTVSystem;
    UINT16 uwDisplayID = 0;
 
    AHC_Display_GetWidthHdight(&uwTVWidth, &uwTVHeight);

	if(ubTVSystem == TV_SYSTEM_PAL)
	{
		if ((uiGetCurrentState() == UI_CAMERA_STATE)||
			(uiGetCurrentState() == UI_VIDEO_STATE)||
			(uiGetCurrentState() == UI_CLOCK_SETTING_STATE)||
			(uiGetCurrentState() == UI_USBSELECT_MENU_STATE))
		{
			uwDispScale = 1;
			uwTVTempWidth = uwTVWidth / uwDispScale;
			uwTVTempHeight = uwTVHeight / uwDispScale;
			uwTVOffX = 0;
			uwTVOffY = 0;
		}else
		if (uiGetCurrentState() == UI_BROWSER_STATE)
		{
			uwDispScale = 1;
			uwTVTempWidth = POS_TV_PAL_BROWSER_OSD_W;
			uwTVTempHeight = POS_TV_PAL_BROWSER_OSD_H;
			uwTVOffX = 0;
			uwTVOffY = 0;//68;
		}else
		//if (uiGetCurrentState() == UI_PLAYBACK_STATE)	 //By default,All of other state use this setting, include UI_PLAYBACK_STATE
		{
			uwDispScale = 1;
			uwTVTempWidth = POS_TV_PLAY_OSD_W;
			uwTVTempHeight = POS_TV_PLAY_OSD_H;
			uwTVOffX = uwTVWidth- uwTVTempWidth - 40;
			uwTVOffY = 68;
		}
	}
	else if(ubTVSystem == TV_SYSTEM_NTSC)
	{
		if ((uiGetCurrentState() == UI_CAMERA_STATE)||
			(uiGetCurrentState() == UI_VIDEO_STATE)||
			(uiGetCurrentState() == UI_CLOCK_SETTING_STATE)||
			(uiGetCurrentState() == UI_USBSELECT_MENU_STATE))
		{
			uwDispScale = 1;
			uwTVTempWidth = uwTVWidth / uwDispScale;
			uwTVTempHeight = uwTVHeight / uwDispScale;
			uwTVOffX = 0;
			uwTVOffY = 0;
		}else
		if (uiGetCurrentState() == UI_BROWSER_STATE)
		{
			uwDispScale = 1;
			uwTVTempWidth = POS_TV_NTSC_BROWSER_OSD_W;
			uwTVTempHeight = POS_TV_NTSC_BROWSER_OSD_H;
			uwTVOffX = 0;
			uwTVOffY = 0;//20;
		}else
		//if (uiGetCurrentState() == UI_PLAYBACK_STATE)	 //All of other state use this setting, include UI_PLAYBACK_STATE
		{
			uwDispScale = 1;
			uwTVTempWidth = POS_TV_PLAY_OSD_W;
			uwTVTempHeight = POS_TV_PLAY_OSD_H;
			uwTVOffX = uwTVWidth- uwTVTempWidth - 20;
			uwTVOffY = 20;
		}
	}

    {
        //** Set TV OSD#1 by uwDisplayID = 17
        uwDisplayID = TVFunc_GetUImodeOsdID();   
        AHC_OSDCreateBuffer(uwDisplayID ,uwTVTempWidth, uwTVTempHeight, OSD_COLOR_RGB565);
        pos[0] = uwTVOffX;
        pos[1] = uwTVOffY;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_DISPLAY_OFFSET, pos);
        pos[0] = uwDispScale - 1;
        pos[1] = uwDispScale - 1;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_DISPLAY_SCALE, pos);
        pos[0] = 1;
        pos[1] = OSD_COLOR_TRANSPARENT;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_TRANSPARENT_ENABLE, pos);

        pos[0] = OSD_ROTATE_DRAW_NONE;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_ROTATE_BY_GUI, pos);

        pos[0] = OSD_FLIP_DRAW_NONE_ENABLE;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_FLIP_BY_GUI, pos);

        //** Set TV OSD by uwDisplayID = 17
        //   	AHC_OSDCreateBuffer(TV_UI_OSD2_ID ,uwTVTempWidth, uwTVTempHeight, OSD_COLOR_RGB565);
        //   	pos[0] = uwTVOffX;
        //   	pos[1] = uwTVOffY;
        //   	AHC_OSDSetDisplayAttr(TV_UI_OSD2_ID, AHC_OSD_ATTR_DISPLAY_OFFSET, pos);
        //   	pos[0] = uwDispScale - 1;
        //   	pos[1] = uwDispScale - 1;
        //   	AHC_OSDSetDisplayAttr(TV_UI_OSD2_ID, AHC_OSD_ATTR_DISPLAY_SCALE, pos);
        //   	pos[0] = 1;
        //   	pos[1] = OSD_COLOR_TRANSPARENT;
        //   	AHC_OSDSetDisplayAttr(TV_UI_OSD2_ID, AHC_OSD_ATTR_TRANSPARENT_ENABLE, pos);

        //   	pos[0] = OSD_ROTATE_DRAW_NONE;
        //   	AHC_OSDSetDisplayAttr(TV_UI_OSD2_ID, AHC_OSD_ATTR_ROTATE_BY_GUI, pos);

        //   	pos[0] = OSD_FLIP_DRAW_NONE_ENABLE;
        //    AHC_OSDSetDisplayAttr(TV_UI_OSD2_ID, AHC_OSD_ATTR_FLIP_BY_GUI, pos);
        //--------------------------------------------------------
    }

#if (TV_MENU_EN)
    {
        UINT32		uwDispScale = 0;
        UINT16		uwTVTempWidth, uwTVTempHeight;	
        UINT16 uwDisplayID = 0, uwDisplayID2 = 0;

        uwDispScale = OSD_DISPLAY_SCALE_TV;

        uwTVTempWidth = uwTVWidth / OSD_DISPLAY_SCALE_TV;
        uwTVTempHeight = uwTVHeight / OSD_DISPLAY_SCALE_TV;

        uwDisplayID = TVFunc_GetMenuOsdID();
        uwDisplayID2 = TVFunc_GetMenuOsd2ID();
        //** Set TV MENU#1 by uwDisplayID = 18,19
        AHC_OSDCreateBuffer(uwDisplayID, uwTVTempWidth, uwTVTempHeight, OSD_COLOR_RGB565);
        AHC_OSDCreateBuffer(uwDisplayID2, uwTVTempWidth, uwTVTempHeight, OSD_COLOR_RGB565);
        pos[0] = 0;//uwTVOffX;
        pos[1] = 0;//uwTVOffY;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_DISPLAY_OFFSET, pos);
        AHC_OSDSetDisplayAttr(uwDisplayID2, AHC_OSD_ATTR_DISPLAY_OFFSET, pos);
        pos[0] = uwDispScale - 1;
        pos[1] = uwDispScale - 1;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_DISPLAY_SCALE, pos);
        AHC_OSDSetDisplayAttr(uwDisplayID2, AHC_OSD_ATTR_DISPLAY_SCALE, pos);
        pos[0] = OSD_FLIP_DRAW_NONE_ENABLE;                
        AHC_OSDSetDisplayAttr(uwDisplayID,  AHC_OSD_ATTR_FLIP_BY_GUI, pos);
        AHC_OSDSetDisplayAttr(uwDisplayID2,  AHC_OSD_ATTR_FLIP_BY_GUI, pos);
        pos[0] = OSD_ROTATE_DRAW_NONE;
        AHC_OSDSetDisplayAttr(uwDisplayID,  AHC_OSD_ATTR_ROTATE_BY_GUI, pos);
        AHC_OSDSetDisplayAttr(uwDisplayID2,  AHC_OSD_ATTR_ROTATE_BY_GUI, pos);
        AHC_OSDClearBuffer(uwDisplayID);
        AHC_OSDClearBuffer(uwDisplayID2);
    }
#endif
}
#endif

#if (HDMI_ENABLE)
void InitOSDCustom_Hdmi(U32 colorFormat, AHC_OSD_ROTATE_DRAW_MODE ahcOSDRotateDraw)
{
    UINT16		uwHDMIWidth, uwHDMIHeight;
    UINT16		uwHDMIOffX, uwHDMIOffY;
    UINT32		pos[2];
    UINT32		uwDispScale = 0;
    UINT16		uwHDMITempWidth, uwHDMITempHeight;

    AHC_GetHdmiDisplayWidthHeight(&uwHDMIWidth, &uwHDMIHeight);


	if((MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080I)||(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080P))
	{
		if ((uiGetCurrentState() == UI_CAMERA_STATE)||
			(uiGetCurrentState() == UI_VIDEO_STATE)||
			(uiGetCurrentState() == UI_CLOCK_SETTING_STATE)||
			(uiGetCurrentState() == UI_USBSELECT_MENU_STATE))
		{
			uwDispScale = OSD_DISPLAY_SCALE_HDMI_FHD;
			uwHDMITempWidth = uwHDMIWidth / uwDispScale;
			uwHDMITempHeight = uwHDMIHeight / uwDispScale;
			uwHDMIOffX = 0;
			uwHDMIOffY = 0;
		}else
		if (uiGetCurrentState() == UI_BROWSER_STATE)
		{
			uwDispScale = OSD_PREVIEW_SCALE_HDMI;
			uwHDMITempWidth = POS_HDMI_1080P_BROWSER_OSD_W;
			uwHDMITempHeight = POS_HDMI_1080P_BROWSER_OSD_H;
			uwHDMIOffX = 0;
			uwHDMIOffY = 15;
		}else
		//if (uiGetCurrentState() == UI_PLAYBACK_STATE)		//By default,All of other state use this setting, include UI_PLAYBACK_STATE
		{
			uwDispScale = OSD_PREVIEW_SCALE_HDMI;
			uwHDMITempWidth = POS_HDMI_PLAY_OSD_W;
			uwHDMITempHeight = POS_HDMI_PLAY_OSD_H;
			uwHDMIOffX = uwHDMIWidth- uwHDMITempWidth - 50;
			uwHDMIOffY = 25;
		}
	}
	else if(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_720P)
	{
		if ((uiGetCurrentState() == UI_CAMERA_STATE)||
			(uiGetCurrentState() == UI_VIDEO_STATE)||
			(uiGetCurrentState() == UI_CLOCK_SETTING_STATE)||
			(uiGetCurrentState() == UI_USBSELECT_MENU_STATE))
		{
			uwDispScale = OSD_DISPLAY_SCALE_HDMI_HD;
			uwHDMITempWidth = uwHDMIWidth / uwDispScale;
			uwHDMITempHeight = uwHDMIHeight / uwDispScale;
			uwHDMIOffX = 0;
			uwHDMIOffY = 0;
		}else
		if (uiGetCurrentState() == UI_BROWSER_STATE)
		{
			uwDispScale = OSD_PREVIEW_SCALE_HDMI;
			uwHDMITempWidth = POS_HDMI_720P_BROWSER_OSD_W;
			uwHDMITempHeight = POS_HDMI_720P_BROWSER_OSD_H;
			uwHDMIOffX = 0;
			uwHDMIOffY = 20;
		}else
		//if (uiGetCurrentState() == UI_PLAYBACK_STATE)		//All of other state use this setting
		{
			uwDispScale = OSD_PREVIEW_SCALE_HDMI;
			uwHDMITempWidth = POS_HDMI_PLAY_OSD_W;
			uwHDMITempHeight = POS_HDMI_PLAY_OSD_H;
			uwHDMIOffX = uwHDMIWidth- uwHDMITempWidth - 40;
			uwHDMIOffY = 25;
		}
	}
	else if(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_480P)
	{
		if ((uiGetCurrentState() == UI_CAMERA_STATE)||
			(uiGetCurrentState() == UI_VIDEO_STATE)||
			(uiGetCurrentState() == UI_CLOCK_SETTING_STATE)||
			(uiGetCurrentState() == UI_USBSELECT_MENU_STATE))
		{
			uwDispScale = OSD_DISPLAY_SCALE_TV;
			uwHDMITempWidth = uwHDMIWidth / uwDispScale;
			uwHDMITempHeight = uwHDMIHeight / uwDispScale;
			uwHDMIOffX = 0;
			uwHDMIOffY = 0;
		}else
		if (uiGetCurrentState() == UI_BROWSER_STATE)
		{
			uwDispScale = OSD_PREVIEW_SCALE_HDMI;
			uwHDMITempWidth = POS_HDMI_480P_BROWSER_OSD_W;
			uwHDMITempHeight = POS_HDMI_480P_BROWSER_OSD_H;
			uwHDMIOffX = 0;
			uwHDMIOffY = 20;
		}else
		//if (uiGetCurrentState() == UI_PLAYBACK_STATE)		//CAUTION:: All of other state use this setting
		{
			uwDispScale = OSD_PREVIEW_SCALE_HDMI;
			uwHDMITempWidth = POS_HDMI_PLAY_OSD_W;
			uwHDMITempHeight = POS_HDMI_PLAY_OSD_H;
			uwHDMIOffX = uwHDMIWidth- uwHDMITempWidth - 40;
			uwHDMIOffY = 25;
		}
	}
#if OSD_PREVIEW_SCALE_HDMI
    {
        UINT16 uwDisplayID = 0, uwDisplayID2 = 0;
        
        uwDisplayID = HDMIFunc_GetUImodeOsdID();
        uwDisplayID2 = HDMIFunc_GetUImodeOsd2ID();
        //** Set HDMI OSD#1 by uwDisplayID = 16
        AHC_OSDCreateBuffer(uwDisplayID ,uwHDMITempWidth, uwHDMITempHeight, OSD_COLOR_RGB565);
        pos[0] = uwHDMIOffX;
        pos[1] = uwHDMIOffY;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_DISPLAY_OFFSET, pos);
        pos[0] = uwDispScale - 1;
        pos[1] = uwDispScale - 1;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_DISPLAY_SCALE, pos);
        pos[0] = 1;
        pos[1] = OSD_COLOR_TRANSPARENT;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_TRANSPARENT_ENABLE, pos);

        pos[0] = OSD_ROTATE_DRAW_NONE;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_ROTATE_BY_GUI, pos);

        pos[0] = OSD_FLIP_DRAW_NONE_ENABLE;
        AHC_OSDSetDisplayAttr(uwDisplayID, AHC_OSD_ATTR_FLIP_BY_GUI, pos);

        //** Set HDMI OSD by uwDisplayID = 17
        AHC_OSDCreateBuffer(uwDisplayID2 ,uwHDMITempWidth, uwHDMITempHeight, OSD_COLOR_RGB565);
        pos[0] = uwHDMIOffX;
        pos[1] = uwHDMIOffY;
        AHC_OSDSetDisplayAttr(uwDisplayID2, AHC_OSD_ATTR_DISPLAY_OFFSET, pos);
        pos[0] = uwDispScale - 1;
        pos[1] = uwDispScale - 1;
        AHC_OSDSetDisplayAttr(uwDisplayID2, AHC_OSD_ATTR_DISPLAY_SCALE, pos);
        pos[0] = 1;
        pos[1] = OSD_COLOR_TRANSPARENT;
        AHC_OSDSetDisplayAttr(uwDisplayID2, AHC_OSD_ATTR_TRANSPARENT_ENABLE, pos);

        pos[0] = OSD_ROTATE_DRAW_NONE;
        AHC_OSDSetDisplayAttr(uwDisplayID2, AHC_OSD_ATTR_ROTATE_BY_GUI, pos);

        pos[0] = OSD_FLIP_DRAW_NONE_ENABLE;
        AHC_OSDSetDisplayAttr(uwDisplayID2, AHC_OSD_ATTR_FLIP_BY_GUI, pos);
        //--------------------------------------------------------
    }
#endif

#if (HDMI_MENU_EN)
    {
        UINT16 uwMenuID = 0, uwMenuID2 = 0;

        uwMenuID = HDMIFunc_GetMenuOsdID();
        uwMenuID2 = HDMIFunc_GetMenuOsd2ID();
    	if((MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080I)||(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080P))
    	{
    		if (uiGetCurrentState() == UI_BROWSER_STATE)
    		{
    			uwDispScale = OSD_DISPLAY_SCALE_HDMI_FHD;
    			uwHDMITempWidth = uwHDMIWidth / uwDispScale;
    			uwHDMITempHeight = uwHDMIHeight / uwDispScale;
    			uwHDMIOffX = 0;
    			uwHDMIOffY = 0;
    		}
    	}
        else if((MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_720P))
        {
    		if (uiGetCurrentState() == UI_BROWSER_STATE)
    		{
    			uwDispScale = OSD_DISPLAY_SCALE_HDMI_HD;
    			uwHDMITempWidth = uwHDMIWidth / uwDispScale;
    			uwHDMITempHeight = uwHDMIHeight / uwDispScale;
    			uwHDMIOffX = 0;
    			uwHDMIOffY = 0;
    		}
    	}
    	//** Set HDMI OSD#1 by uwDisplayID = 18,19
        AHC_OSDCreateBuffer(uwMenuID, uwHDMITempWidth, uwHDMITempHeight, OSD_COLOR_RGB565);

        pos[0] = uwHDMIOffX;
        pos[1] = uwHDMIOffY;
        AHC_OSDSetDisplayAttr(uwMenuID, AHC_OSD_ATTR_DISPLAY_OFFSET, pos);
        pos[0] = uwDispScale - 1;
        pos[1] = uwDispScale - 1;
        AHC_OSDSetDisplayAttr(uwMenuID, AHC_OSD_ATTR_DISPLAY_SCALE, pos);
        pos[0] = OSD_ROTATE_DRAW_NONE;
        AHC_OSDSetDisplayAttr(uwMenuID,  AHC_OSD_ATTR_ROTATE_BY_GUI, pos);
        AHC_OSDClearBuffer(uwMenuID);

        AHC_OSDCreateBuffer(uwMenuID2, uwHDMITempWidth, uwHDMITempHeight, OSD_COLOR_RGB565);
        pos[0] = uwHDMIOffX;
        pos[1] = uwHDMIOffY;
        AHC_OSDSetDisplayAttr(uwMenuID2, AHC_OSD_ATTR_DISPLAY_OFFSET, pos);
        pos[0] = uwDispScale - 1;
        pos[1] = uwDispScale - 1;
        AHC_OSDSetDisplayAttr(uwMenuID2, AHC_OSD_ATTR_DISPLAY_SCALE, pos);
        pos[0] = OSD_ROTATE_DRAW_NONE;
        AHC_OSDSetDisplayAttr(uwMenuID2,  AHC_OSD_ATTR_ROTATE_BY_GUI, pos);
        AHC_OSDClearBuffer(uwMenuID2);
    }
#endif
}
#endif

void InitOSDCustom_Lcd(U32 colorFormat, AHC_OSD_ROTATE_DRAW_MODE ahcOSDRotateDraw)
{
    UINT32 i;
    UINT32 iVal[10];
    INT32  bAlphaBlendingEn = 0;

    // LAYER 0 - ID 0 ~ 15
    // LAYER 1 - ID 16 ~ 31
    // Default layer is display id 0 & 16    

    if(OSD_COLOR_RGB565 == colorFormat){
        colorFormat = OSD_COLOR_RGB565_CCW; //use RGB565 CCW internally.
    }
    else if(OSD_COLOR_ARGB32 == colorFormat){
        colorFormat = OSD_COLOR_ARGB32_CCW; //use ARGB CCW internally.
    }
    
    for(i=1; i<2; i++){
        AHC_OSDCreateBuffer(i,
                    RTNA_LCD_GetAttr()->usPanelW / OSD_DISPLAY_SCALE_LCD, RTNA_LCD_GetAttr()->usPanelH / OSD_DISPLAY_SCALE_LCD,
                    colorFormat/*OSD_COLOR_RGB565_CCW*//*OSD_COLOR_ARGB32_CCW*/);
#if 0        
        if (OSD_DISPLAY_SCALE_LCD) {
            AHC_OSDCreateBuffer(i,
            RTNA_LCD_GetAttr()->usPanelW / OSD_DISPLAY_SCALE_LCD, RTNA_LCD_GetAttr()->usPanelH / OSD_DISPLAY_SCALE_LCD,
            OSD_COLOR_RGB565_CCW/*OSD_COLOR_ARGB32_CCW*/);
        }
        else {
            AHC_OSDCreateBuffer(i, RTNA_LCD_GetAttr()->usPanelW, RTNA_LCD_GetAttr()->usPanelH, colorFormat);
        }
#endif

        iVal[0] = ahcOSDRotateDraw;        
        AHC_OSDSetDisplayAttr(i,  AHC_OSD_ATTR_ROTATE_BY_GUI, iVal);
        iVal[0] = OSD_FLIP_DRAW_NONE_ENABLE;       
        AHC_OSDSetDisplayAttr(i,  AHC_OSD_ATTR_FLIP_BY_GUI, iVal);        

        if (OSD_DISPLAY_SCALE_LCD) {
            iVal[0] = OSD_DISPLAY_SCALE_LCD - 1;    // remapping to enum MMP_DISPLAY_DUPLICATE
            iVal[1] = OSD_DISPLAY_SCALE_LCD - 1;
        }
        else {
            iVal[0] = MMP_DISPLAY_DUPLICATE_1X;
            iVal[1] = MMP_DISPLAY_DUPLICATE_1X;
        }

        AHC_OSDSetDisplayAttr(i, AHC_OSD_ATTR_DISPLAY_SCALE, iVal);
        AHC_OSDClearBuffer(i);
    }

    for(i=16; i<=20; i++)
    {
        AHC_OSDCreateBuffer(i,
                RTNA_LCD_GetAttr()->usPanelW / OSD_DISPLAY_SCALE_LCD, RTNA_LCD_GetAttr()->usPanelH / OSD_DISPLAY_SCALE_LCD,
                colorFormat);

        iVal[0] = ahcOSDRotateDraw;                
        AHC_OSDSetDisplayAttr(i,  AHC_OSD_ATTR_ROTATE_BY_GUI, iVal);
        iVal[0] = OSD_FLIP_DRAW_NONE_ENABLE;                
        AHC_OSDSetDisplayAttr(i,  AHC_OSD_ATTR_FLIP_BY_GUI, iVal);          

        if (OSD_DISPLAY_SCALE_LCD) {
            iVal[0] = OSD_DISPLAY_SCALE_LCD - 1;    // remapping to enum MMP_DISPLAY_DUPLICATE
            iVal[1] = OSD_DISPLAY_SCALE_LCD - 1;
        }
        else {
            iVal[0] = MMP_DISPLAY_DUPLICATE_1X;
            iVal[1] = MMP_DISPLAY_DUPLICATE_1X;
        }    
        AHC_OSDSetDisplayAttr(i, AHC_OSD_ATTR_DISPLAY_SCALE, iVal);
        AHC_OSDClearBuffer(i);
    }

    // Set transparent color
    iVal[0] = 1;         	// Enable
    iVal[1] = 0x00000000; 	// Black color
    AHC_OSDSetDisplayAttr(0, AHC_OSD_ATTR_TRANSPARENT_ENABLE, iVal);
    AHC_OSDSetDisplayAttr(1, AHC_OSD_ATTR_TRANSPARENT_ENABLE, iVal);

    for(i=0;i<5;i++){
        AHC_OSDSetDisplayAttr(i+16, AHC_OSD_ATTR_TRANSPARENT_ENABLE, iVal);
    }

    if(bAlphaBlendingEn){
        iVal[0] = 1;        // Enable
        iVal[1] = AHC_OSD_ALPHA_ARGB;

        for(i=0; i<8; i++) {
            iVal[2+i] = i * 256 / 8;  // Alpha weighting 
        }

        AHC_OSDSetDisplayAttr(0, AHC_OSD_ATTR_ALPHA_BLENDING_ENABLE, iVal);
        AHC_OSDSetDisplayAttr(1, AHC_OSD_ATTR_ALPHA_BLENDING_ENABLE, iVal);
        AHC_OSDSetDisplayAttr(2, AHC_OSD_ATTR_ALPHA_BLENDING_ENABLE, iVal);

        for(i=0;i<5;i++){
            AHC_OSDSetDisplayAttr(i+16, AHC_OSD_ATTR_ALPHA_BLENDING_ENABLE, iVal);
        }
    }    

    AHC_OSDSetColor(1,  0x80000000); // Alpha value is 0x80
    AHC_OSDSetColor(0,  0x80000000); // Alpha value is 0x80
    AHC_OSDSetColor(20, 0x80000000); // Alpha value is 0x80

    AHC_OSDSetCurrentDisplay(1);    
    AHC_OSDSetCurrentDisplay(0);
    AHC_OSDSetCurrentDisplay(20);

    // Set MAIN Inactive & OVERLAY WINDOW Active
    AHC_OSDSetActive(0, 0);
    AHC_OSDSetActive(20, 1);
    //printc("### %x\r\n\r\n", OSTimeGet());

}

#if (TVOUT_ENABLE)//(TVOUT_PREVIEW_EN)
void InitOSD_TVout(void)
{
    U32 colorFormat;
    AHC_OSD_ROTATE_DRAW_MODE ahcOSDRotateDraw;

    U32 ulOSDWidth, ulOSDHeight, ulThumbWidth, ulThumbHeight; 
    AHC_BOOL bMainBuffFull;

    UINT16 		uwTVWidth, uwTVHeight;

    printc("### %s,%d -\r\n", __func__, __LINE__);

    GUI_RegisterExternalFillRectCB((void *)AHC_OSD_ExternalFillRectCB);

#if 0//TBD
    if(MenuSettingConfig()->uiTVSystem==TV_SYSTEM_PAL)
    AHC_SetDisplayOutputDev(DISP_OUT_TV_PAL, AHC_DISPLAY_DUPLICATE_1X);
    else
    AHC_SetDisplayOutputDev(DISP_OUT_TV_NTSC, AHC_DISPLAY_DUPLICATE_1X);
#endif
#if (TVOUT_PREVIEW_EN)
    ulOSDWidth = 320; //TBD
    ulOSDHeight = 240;
#else
    ulOSDWidth = 0; //TBD
    ulOSDHeight = 0
#endif
    if (uiGetCurrentState() == UI_BROWSER_STATE){
        AHC_DISPLAY_OUTPUTPANEL pCurDevice;

        AHC_GetDisplayOutputDev(&pCurDevice);    
        //TBD
        if(((MenuSettingConfig()->uiTVSystem == TV_SYSTEM_PAL) && (pCurDevice != AHC_DISPLAY_PAL_TV)) ||
        ((MenuSettingConfig()->uiTVSystem == TV_SYSTEM_NTSC) && (pCurDevice != AHC_DISPLAY_NTSC_TV))){   
            switch(MenuSettingConfig()->uiTVSystem) 
            {
                case TV_SYSTEM_PAL:
                    AHC_SetDisplayOutputDev(DISP_OUT_TV_PAL, AHC_DISPLAY_DUPLICATE_1X);
                    AHC_SetCurrentDisplayEx(MMP_DISPLAY_SEL_PAL_TV);
                    break;	    	    
                    
                case TV_SYSTEM_NTSC:
                default:
                    AHC_SetDisplayOutputDev(DISP_OUT_TV_NTSC, AHC_DISPLAY_DUPLICATE_1X);
                    AHC_SetCurrentDisplayEx(MMP_DISPLAY_SEL_NTSC_TV);
                    break;
            }    							
        }
        AHC_Display_GetWidthHdight(&uwTVWidth, &uwTVHeight);	   
        switch(MenuSettingConfig()->uiTVSystem) {
            case TV_SYSTEM_NTSC:
                ulThumbWidth 	= uwTVWidth;
                ulThumbHeight	= uwTVHeight - POS_TV_NTSC_BROWSER_OSD_H;	
                break;

            case TV_SYSTEM_PAL:
                ulThumbWidth 	= uwTVWidth;
                ulThumbHeight	= uwTVHeight - POS_TV_PAL_BROWSER_OSD_H;
                break;			

            default:
                printc("TVFunc_ChangeOSDStatus Error!\r\n");
                break;
        }

    }else{
        ulThumbWidth = 0;
        ulThumbHeight = 0;
    }     

    bMainBuffFull = AHC_FALSE;

    colorFormat = WMSG_LAYER_WIN_COLOR_FMT;
    ahcOSDRotateDraw = OSD_ROTATE_DRAW_NONE;
    AHC_OSD_RegisterInitOSDCustomCB((void *)InitOSDCustom_Tv);

#if	defined(MALLOC_OSDBUFF_DOWN_GROWTH) && (MALLOC_OSDBUFF_DOWN_GROWTH)
	AIHC_SetGUIMemStartAddr( AHC_UF_GetTempBaseAddr());
#else
    AIHC_SetGUIMemStartAddr(AHC_GUI_TEMP_BASE_ADDR);
#endif

    AHC_OSDInit(ulOSDWidth, ulOSDHeight, 
                ulThumbWidth, ulThumbHeight,
                PRIMARY_DATESTAMP_WIDTH, PRIMARY_DATESTAMP_HEIGHT, 
                THUMB_DATESTAMP_WIDTH, THUMB_DATESTAMP_HEIGHT, 
                colorFormat, bMainBuffFull, ahcOSDRotateDraw);

}
#endif

#if (HDMI_ENABLE)//(HDMI_PREVIEW_EN)
void InitOSD_HDMI(void)
{
    U32 colorFormat;
    AHC_OSD_ROTATE_DRAW_MODE ahcOSDRotateDraw;

    U32 ulOSDWidth, ulOSDHeight, ulThumbWidth, ulThumbHeight; 
    AHC_BOOL bMainBuffFull;

    UINT16 		uwHDMIWidth, uwHDMIHeight;
    //UINT16		uwHDMIOffX,  uwHDMIOffY;
    AHC_DISPLAY_HDMIOUTPUTMODE 	HdmiMode;

    printc("### %s,%d -\r\n", __func__, __LINE__);
    GUI_RegisterExternalFillRectCB((void *)AHC_OSD_ExternalFillRectCB);
	//bHaveInitOsdHdmi = AHC_TRUE;
	
#if (HDMI_PREVIEW_EN)
    ulOSDWidth = 320;
    ulOSDHeight = 240;
#else
    ulOSDWidth = 0;
    ulOSDHeight = 0;
#endif
    if (uiGetCurrentState() == UI_BROWSER_STATE)
    {
        AHC_GetHdmiDisplayWidthHeight(&uwHDMIWidth, &uwHDMIHeight);
        AHC_GetHDMIOutputMode(&HdmiMode);

        switch(HdmiMode)
        {
            case AHC_DISPLAY_HDMIOUTPUT_1920X1080P:
                ulThumbWidth 	= uwHDMIWidth;
                ulThumbHeight 	= uwHDMIHeight - POS_HDMI_1080I_BROWSER_OSD_H;// - POS_HDMI_1080I_BROWSER_OSD_H;
                break;
                
            case AHC_DISPLAY_HDMIOUTPUT_1920X1080I:
                ulThumbWidth 	= uwHDMIWidth;
                ulThumbHeight 	= (uwHDMIHeight<<1) - POS_HDMI_1080I_BROWSER_OSD_H;
                break;
                
            case AHC_DISPLAY_HDMIOUTPUT_1280X720P:
                ulThumbWidth 	= uwHDMIWidth;
                ulThumbHeight 	= uwHDMIHeight - POS_HDMI_720P_BROWSER_OSD_H;
                break;
                
            case AHC_DISPLAY_HDMIOUTPUT_640X480P:
            case AHC_DISPLAY_HDMIOUTPUT_720X480P:
            case AHC_DISPLAY_HDMIOUTPUT_720X576P:
            case AHC_DISPLAY_HDMIOUTPUT_1280X720P_50FPS:
            case AHC_DISPLAY_HDMIOUTPUT_1920X1080P_30FPS:
            default:
                printc("HDMIFunc_ChangeOSDStatus Error!\r\n");
                break;
        }
    }else{
        ulThumbWidth = 0;
        ulThumbHeight = 0;
    }

    bMainBuffFull = AHC_FALSE;

    //AHC_SetDisplayOutputDev(DISP_OUT_HDMI, AHC_DISPLAY_DUPLICATE_1X);
    colorFormat = WMSG_LAYER_WIN_COLOR_FMT;
    ahcOSDRotateDraw = OSD_ROTATE_DRAW_NONE;        
    AHC_OSD_RegisterInitOSDCustomCB((void *)InitOSDCustom_Hdmi);

#if	defined(MALLOC_OSDBUFF_DOWN_GROWTH)&&(MALLOC_OSDBUFF_DOWN_GROWTH)
	AIHC_SetGUIMemStartAddr( AHC_UF_GetTempBaseAddr());
#else
    AIHC_SetGUIMemStartAddr(AHC_GUI_TEMP_BASE_ADDR_HDMI);
#endif

    AHC_OSDInit(ulOSDWidth, ulOSDHeight, 
                ulThumbWidth, ulThumbHeight,
                PRIMARY_DATESTAMP_WIDTH, PRIMARY_DATESTAMP_HEIGHT, 
                THUMB_DATESTAMP_WIDTH, THUMB_DATESTAMP_HEIGHT, 
                colorFormat, bMainBuffFull, ahcOSDRotateDraw);
}
#endif

void InitOSD_LCD(void)
{
    U32 colorFormat;
    AHC_OSD_ROTATE_DRAW_MODE ahcOSDRotateDraw;

    U32 ulOSDWidth, ulOSDHeight, ulThumbWidth, ulThumbHeight; 
    AHC_BOOL bMainBuffFull;

   // printc("### %s,%d -\r\n", __func__, __LINE__);

    GUI_RegisterExternalFillRectCB((void *)AHC_OSD_ExternalFillRectCB);

    ulOSDWidth = RTNA_LCD_GetAttr()->usPanelW / OSD_DISPLAY_SCALE_LCD;
    ulOSDHeight = RTNA_LCD_GetAttr()->usPanelH / OSD_DISPLAY_SCALE_LCD;
    ulThumbWidth = RTNA_LCD_GetAttr()->usPanelW / OSD_DISPLAY_SCALE_LCD;
    ulThumbHeight = RTNA_LCD_GetAttr()->usPanelH / OSD_DISPLAY_SCALE_LCD;
    bMainBuffFull = AHC_TRUE;

    //Force OSD format as RGB565 for saving memory and drawing time.
    colorFormat = OSD_COLOR_RGB565;
    //colorFormat = OSD_COLOR_ARGB32;
#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
    ahcOSDRotateDraw = OSD_ROTATE_DRAW_NONE;
#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90)
    ahcOSDRotateDraw = OSD_ROTATE_DRAW_RIGHT_90;
#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
    ahcOSDRotateDraw = OSD_ROTATE_DRAW_RIGHT_270;
#endif        
    AHC_OSD_RegisterInitOSDCustomCB((void *)InitOSDCustom_Lcd);

#if	defined(MALLOC_OSDBUFF_DOWN_GROWTH)&&(MALLOC_OSDBUFF_DOWN_GROWTH)
	AIHC_SetGUIMemStartAddr( AHC_UF_GetTempBaseAddr());
#else
    AIHC_SetGUIMemStartAddr(AHC_GUI_TEMP_BASE_ADDR);
#endif

    if(OSD_DISPLAY_SCALE_LCD == 0){
        AHC_PRINT_RET_ERROR(0,0);
        return;
    }

    AHC_OSDInit(ulOSDWidth, ulOSDHeight, 
	    ulThumbWidth, ulThumbHeight,
	    PRIMARY_DATESTAMP_WIDTH, PRIMARY_DATESTAMP_HEIGHT, 
	    THUMB_DATESTAMP_WIDTH, THUMB_DATESTAMP_HEIGHT, 
	    colorFormat, bMainBuffFull, ahcOSDRotateDraw);
}

void InitOSD_None(void)
{
    printc("### %s,%d -\r\n", __func__, __LINE__);

    GUI_RegisterExternalFillRectCB((void *)AHC_OSD_ExternalFillRectCB);

    //TBD
}

void InitOSD(void)
{
#if (TVOUT_ENABLE)//(TVOUT_PREVIEW_EN)
    if(AHC_IsTVConnectEx()){
        InitOSD_TVout();
    }
    else
#endif	
#if (HDMI_ENABLE)//(HDMI_PREVIEW_EN)
    if(AHC_IsHdmiConnect()){
        InitOSD_HDMI();
    }
    else
#endif
    {
        InitOSD_LCD();
    }
}

#if (defined(SUPPORT_SPEECH_RECOG)&&(SUPPORT_SPEECH_RECOG))
int speech_handler(void *obj, wakeup_status_t status, char *json, int bytes)
{
    if (status == 0) {
    	printc("not wakeup.\r\n");
    } else {
    	printc(json);
    }
    return 0;
}
void SpeechRecogRunCB(MMP_ULONG ID)
{
	printc("SpeechRecogCB=%X.\r\n",ID);
}
#endif
UINT8 AHC_WaitForSystemReady(void)
{
    UINT16 SemCount = 0;

    AHC_OS_QuerySem(ahc_System_Ready, &SemCount); 

	return (0 == SemCount)?(0):(1);
}

MMP_UBYTE bDCFInitDone = 0; //Andy Liu. Remove it later...
void AHC_DCF_Init(void)
{
    AHC_AUDIO_I2S_CFG   ConfigAudioI2S;
    AHC_BOOL 			bSD_inserted = AHC_FALSE;

	printc("### %s -\r\n", __func__);
	printc("~~~AHC_DCF_Init == ~~~~~~%d~~~~~~\n",MenuSettingConfig()->uiMediaSelect);
	AIHC_SetTempDCFMemStartAddr(AHC_DCF_TEMP_BASE_ADDR);
	AIHC_SetTempBrowserMemStartAddr(AHC_THUMB_TEMP_BASE_ADDR);

    #if (AHC_SHAREENC_SUPPORT)
    AHC_UF_RegisterCallback(AHC_UFCB_CREATE_FORMAT_FREE_FILE, (void *)AHC_UF_CreateShareFileCB);
    #endif

    if(MEDIA_SETTING_SD_CARD == MenuSettingConfig()->uiMediaSelect)
    {
        bSD_inserted = AHC_IsSDInserted();

        if(bSD_inserted){
			Enable_SD_Power(1 /* Power On */);
            AHC_SDMMC_SetState(SDMMC_IN);
            
            #if (FS_FORMAT_FREE_ENABLE)
            if( AHC_CheckMedia_FormatFree( AHC_MEDIA_MMC ) == AHC_FALSE )
            {
                SystemSettingConfig()->byNeedToFormatMediaAsFormatFree = 1;
            }
            else            
            #endif
                AHC_RemountDevices(MenuSettingConfig()->uiMediaSelect);
        }
        else{
			Enable_SD_Power(0 /* Power Off */);
            AHC_SDMMC_SetState(SDMMC_OUT);
    	}
    }
    else
    {
    	AHC_RemountDevices(MenuSettingConfig()->uiMediaSelect);
	}

    ConfigAudioI2S.workingMode      = AHC_AUDIO_I2S_MASTER_MODE;
    ConfigAudioI2S.mclkMode         = AHC_AUDIO_I2S_256FS_MODE;
    ConfigAudioI2S.outputBitSize    = AHC_AUDIO_I2S_OUT_16BIT;
    ConfigAudioI2S.lrckPolarity     = AHC_AUDIO_I2S_LRCK_L_LOW;

    AHC_ConfigAudioExtDevice(&ConfigAudioI2S, 44100);
	//MIMF_CUSTOMIZED_SORT_TYPE use for customized sorting type, implement by AHC_MIMF.c
#if(MIMF_CUSTOMIZED_SORT_TYPE)    
    AHC_MIMF_RegisterCusCallback();
#endif  

    bDCFInitDone = 1;
}

#if (TASK_MONITOR_ENABLE)
void AHC_Main_Task_ReSetMonitorTime(UINT32 ulTime)
{
    gsTaskMonitorAhcMain.ulExecTime = ulTime;
}
#endif
#if(ENCODE_GPIO_ISR_off)
MMP_UBYTE returnValue2=0;
void Encode_RIGHT_ISR(MMP_GPIO_PIN piopin)
{
	Right_left_code(1);// lyj
	printc("~~~~break_1~~~~~~~~\r\n");
}
void Encode_LEFT_ISR(MMP_GPIO_PIN piopin)
{

	Right_left_code(0);// lyj
	printc("~~~~break_2~~~~~~~~\r\n");
}
#endif

//==================ISR===
#if 0
void Encode_RIGHT_T_ISR(MMP_GPIO_PIN piopin)
{
	MMP_UBYTE LeftValue =5;
	MMP_UBYTE RightValue = 5;
	MMPF_PIO_GetData(ENCODE_GPIO_RIGHT,&LeftValue);//ENCODE_GPIO_LEFT
	MMPF_PIO_GetData(ENCODE_GPIO_LEFT,&RightValue);
	printc("~~~~T_ISR-->~~~LeftValue = %d~~~~RightValue = %d~~~~~~\r\n",LeftValue,RightValue);
	if(LeftValue == 1 && RightValue == 0)
	{	
		//sub_count_ex++;
	//	if(sub_count_ex > 10)
		//	sub_count_ex = 10;
		 //Draw_Main_volum_icon(sub_count_ex,1);
		 printc("~~~~~Encode_RIGHT_T_ISR~~~~~~~\r\n");
	}
	else if(LeftValue == 0 && RightValue == 1)
	{
		printc("~~~~~Encode_LEFT_T_ISR~~~~~~~\r\n");
	}
	else
	{

	}

}
#endif

void Encode_LEFT_T_ISR(MMP_GPIO_PIN piopin)
{

	{
		sub_count_ex--;	
		if(sub_count_ex < 0)
			sub_count_ex = 0;
		 //Draw_Main_volum_icon(sub_count_ex,1);
		  printc("~~~~~Encode_LEFT_T_ISR~~~~~~~\r\n");
	}

}//lyj

//===========================
 
void AHC_Backlight_Pwm(void)
{
	INT32 s32Contrast_t = 0;
	
	 pf_General_EnGet(COMMON_KEY_CONTRAST, &s32Contrast_t);
	 printc("s32Contrast_t=%d\r\n",s32Contrast_t);
	 switch(s32Contrast_t)
	 	{
		case 1 : ulTimess=90; break ;
		case 2 : ulTimess=80; break ;
		case 3 : ulTimess=72; break ;
		case 4 : ulTimess=68; break ;
		case 5 : ulTimess=66; break ;
		case 6 : ulTimess=64; break ;
		case 7 : ulTimess=62; break ;
		case 8 : ulTimess=60; break ;
		case 9 : ulTimess=58; break ;
		case 10 : ulTimess=30; break ;
		case 11 : ulTimess=15; break ;
		case 12 : ulTimess=8; break ;
		case 13 : ulTimess=2; break ;

	 }
	  printc("ulTimess=%d\r\n",ulTimess);
	 MMPF_PWM_OutputPulse(LCD_BACKLIGHT_PWM_UNIT_PIN, MMP_TRUE, 50000,ulTimess, MMP_TRUE, MMP_FALSE, NULL, 0);
	

}

#if 0
static void App_AHC_Backlight_Pwm(MMP_UBYTE s32Contrast_t)
{

	


	 switch(s32Contrast_t)
	 	{
		case 1 : ulTimess=76; break ;
		case 2 : ulTimess=70; break ;
		case 3 : ulTimess=64; break ;
		case 4 : ulTimess=58; break ;
		case 5 : ulTimess=52; break ;
		case 6 : ulTimess=46; break ;
		case 7 : ulTimess=40; break ;
		case 8 : ulTimess=34; break ;
		case 9 : ulTimess=28; break ;
		case 10 : ulTimess=22; break ;
		case 11 : ulTimess=16; break ;
		case 12 : ulTimess=10; break ;
		case 13 : ulTimess=4; break ;

	 }
	  printc("ulTimess=%d\r\n",ulTimess);
	  if(s32Contrast_t > 0 && s32Contrast_t < 14)
	 	MMPF_PWM_OutputPulse(LCD_BACKLIGHT_PWM_UNIT_PIN, MMP_TRUE, 50000,ulTimess, MMP_TRUE, MMP_FALSE, NULL, 0);
	

}
#endif


void app_contrl_Backlight(void)
{

	if(recv_data[4] < 8)
	{
		//App_AHC_Backlight_Pwm(1);
		Freq_config = 100;
	}
	else if(recv_data[4] < 16)
	{
		//App_AHC_Backlight_Pwm(2);
		Freq_config = 1000;
	}
	else if(recv_data[4] < 24)
	{
		//App_AHC_Backlight_Pwm(3);
		Freq_config = 1800;
	}
	else if(recv_data[4] < 32)
	{
		//App_AHC_Backlight_Pwm(4);
		Freq_config = 2500;
	}
	else if(recv_data[4] < 40)
	{
		//App_AHC_Backlight_Pwm(5);
		Freq_config = 3000;
	}
	else if(recv_data[4] < 48)
	{
		//App_AHC_Backlight_Pwm(6);
		Freq_config = 3500;
	}
	else if(recv_data[4] < 56)
	{
		//App_AHC_Backlight_Pwm(7);
		Freq_config = 5000;
	}
	else if(recv_data[4] < 62)
	{
		//App_AHC_Backlight_Pwm(8);
		Freq_config = 6000;
	}
	else if(recv_data[4] < 70)
	{
		//App_AHC_Backlight_Pwm(9);
		Freq_config = 20000;
	}
	else if(recv_data[4] < 78)
	{
		//App_AHC_Backlight_Pwm(10);
		Freq_config = 40000;
	}
	else if(recv_data[4] < 86)
	{
		//App_AHC_Backlight_Pwm(11);
		Freq_config = 60000;
	}
	else if(recv_data[4] <= 92)
	{
		//App_AHC_Backlight_Pwm(12);
		Freq_config = 80000;
	}
	else if(recv_data[4] <= 100)
	{
		//App_AHC_Backlight_Pwm(13);
		Freq_config = 100000;
	}

	recv_data[4] = 255;


}

static void twikle_speed(MMP_UBYTE dev)
{
		if(twikle_flag == 0)
		{
			j++;

			if(j % dev == 0)
			{
		
				twinkle_led(Freq_config,500,R,G,B);
				twikle_flag = 1;
		
				if(j == 100 || j == 200 || j == 130 || j == 150 || j == 160)
				{
					j = 0;
				}
				
			}
			

		}
		
		

			if(twikle_flag == 1)
			{
				i++;
				if(i % dev == 0)
				{
					White_light_bar_off();
					twikle_flag = 0;

					if(i == 100 || i == 200 || i == 130 || i == 150 || i == 160)
					{
						i = 0;
					}
				}
		
			}



}



//==============================spi=========
#if 0

 unsigned short serialNum = 9;  
unsigned char Send_Data[]={0x56,0x55,0x12,0x98,0x23,0x39,0x34,0x78,0x32,0x11,0x59,0x78,0x87,0x25,0x46};

int Flag_len=1;
int SendNum=0;
int flag_22222=0;
int BitCount(unsigned short n)
{
    unsigned int c =0 ;
    for (c =0; n; ++c)
    {
        n &= (n -1) ; // 清除最低位的1
    }
    return c ;
}
void CS_ISR(MMP_GPIO_PIN piopin)
{

	
	serialNum=9;
	SendNum=0;
	flag_22222=1;
}
void CLK_ISR(MMP_GPIO_PIN piopin)
{
unsigned char  SPI_MOSI;
unsigned short length=0x0;//char length;
unsigned short Data=0;
unsigned short Data33=0;
 if(flag_22222){
length=sizeof(Send_Data)/sizeof(char);
   if(Flag_len){
	//length=sizeof(Send_Data)/sizeof(char);
	--serialNum;
	if((BitCount(length)%2)==0)
		{
		length=length+0x100;
	//	printc(" 0x%x    偶数\r\n",length);
	}else
		{
	//	printc(" 0x%x  奇数\r\n",length);
	}
	
	SPI_MOSI = (length>>serialNum)&0x01; 
	MMPF_PIO_SetData(/*MMP_GPIO60*/MMP_GPIO18, SPI_MOSI,MMP_TRUE);
	if(serialNum<=0){
		serialNum=9;
		//Senddata(length);
		Flag_len=0;
	}
   }
   else

   	{
  // 	SendNum++;
//	Senddata(Send_Data[SendNum]);
	
	if(SendNum<length){
		//SendNum++;
		Data=Send_Data[SendNum];
		--serialNum;
		if((BitCount(Data)%2)==0)
			{
			Data=Data+0x100;
		//	Data33=Data&0xff00;
		//	printc(" 0x%x  偶数\r\n",Data);
		}else
			{
		//    printc(" 0x%x  奇数\r\n",Data);
		}
		SPI_MOSI = (Data>>serialNum)&0x01; 
		MMPF_PIO_SetData(/*MMP_GPIO60*/MMP_GPIO18, SPI_MOSI,MMP_TRUE);

		if(serialNum<=0){
			serialNum=9;
		//	RTNA_DBG_Str(0,"Data=");
		//	RTNA_DBG_Byte(0,Data);
		//	RTNA_DBG_Str(0,"\r\n");
			++SendNum;
			if(SendNum==length)
				{
				Flag_len=1;
				SendNum=0;
				flag_22222=0;
		///		length=0;
				
				
			//	MMPF_PIO_SetData(MMP_GPIO60, 0,MMP_TRUE);
			}
			//Senddata(length);
		//	Flag_len=1;
		}
   }
  
}
 	}
}


void InitSPIGpio( MMP_GPIO_PIN piopin,GpioCallBackFunc* CallBackFunc,MMP_GPIO_TRIG flag)
{
    if(piopin == MMP_GPIO_MAX)
		return;
    
    printc("%s \r\n", __func__);
//	MMPF_PIO_EnableOutputMode(MMP_GPIO21, 0, MMP_TRUE);
//	MMPF_PIO_EnableOutputMode(MMP_GPIO31, 1, MMP_TRUE);
    //Set GPIO as input mode
    MMPF_PIO_EnableOutputMode(piopin, MMP_FALSE, MMP_TRUE);

    // Set the trigger mode.  设置触发模式
    MMPF_PIO_EnableTrigMode(piopin, flag, MMP_TRUE, MMP_TRUE);

    //Enable Interrupt  使能中断
    MMPF_PIO_EnableInterrupt(piopin, MMP_TRUE, 0, (GpioCallBackFunc *)CallBackFunc, MMP_TRUE);

}






#endif

//==============================end==============

void AHC_Main_Task(void *p_arg)
{
    AHC_SWPACK_TaskInit();
    
    ahc_System_Ready = AHC_OS_CreateSem(0);

    AHC_USBDetectHandler();

	

#if (SUPPORT_IR_CONVERTER)
    #if 1
    printc("--I-- IR function is TBD\r\n");
    #else
    IRConverter_Initialize();
    // To Clear IR status of PowerOn Key avoid power off
    IRConverter_WriteCommand(IR_CONVERTER_CLEAR_POWERON, 0x01);
    #endif
#endif
	
    AHC_Initialize();
#if defined(CFG_BOOT_BYPASS_CLOCK_CHECK)
	CheckRtcAndAutoSetRtc();
#endif
	AHC_WaitForBootComplete();
   
#ifdef USB_LABLE_STRING
	AHC_SetUSBLable(USB_LABLE_STRING);
#endif
	/* Update Menu Setting */ 
	uiCheckDefaultMenuExist();
	gAHC_UpdateMenuDone = AHC_TRUE;

	MenuSettingConfig()->uiMediaSelect = MEDIA_SETTING_SD_CARD;


	//====================lyj===
	 mainVflag_count = MenuSettingConfig()->uiEV;
	 sub_count_ex = MenuSettingConfig()->uiSpeedStamp;
	//===================
	Menu_To_Video_One=AHC_TRUE;// liao 20180316
    #if (AHC_DRAM_SIZE == COMMON_DRAM_SIZE_32MB)
    pf_FCWS_EnSet(FCWS_EN_OFF);
    pf_SAG_EnSet(SAG_EN_OFF);
    pf_HDR_EnSet(HDR_EN_OFF);
    MMP_EnableVidHDR(MMP_FALSE);
    #endif

	#if defined(SUPPORT_ONCHIP_IRDA)&&(SUPPORT_ONCHIP_IRDA)
	MMPS_IrDA_Initialize();
	MMPS_IrDA_SetIrEn(MMP_TRUE);
    #endif

    #if SNR_CLK_POWER_SAVING_SETTING
    pf_HDR_EnSet(COMMON_HDR_EN_OFF);
    #endif

	Menu_SetVolume(MenuSettingConfig()->uiVolume);

	AHC_SetMenuFlashLEDStatus(MenuSettingConfig()->uiLEDFlash);	

#if (SUPPORT_GPS)
	AHC_OS_SetFlags(UartCtrlFlag, GPS_FLAG_INITIAL, AHC_OS_FLAG_SET);
#endif
#if (TOUCH_UART_FUNC_EN)
	AHC_OS_SetFlags(UartCtrlFlag, Touch_Uart_FLAG_INITIAL, AHC_OS_FLAG_SET);
#endif
	/*let power on logo off*/
	//AHC_SetDisplayMode(DISPLAY_MODE_DISABLE); //move to other loc
    // Remove this and initial this with jobdispatch task.//CZ patch...20160204
    //AHC_DCF_Init();

    #ifdef CFG_TURN_OFF_POWER_LED_WHEN_BOOT_FINISH
    LedCtrl_PowerLed(AHC_FALSE);
    #else
    LedCtrl_PowerLed(AHC_TRUE);
    #endif

   // AHC_PowerOn_Welcome();
    AHC_Backlight_Pwm();

	// ~~~~~~~~~~~~~~~~~~lyj~~~~~~~~
	#if 1 // lyj 20190917
	MMPF_PIO_PadConfig(MMP_GPIO62, PAD_OUT_DRIVING(0), MMP_TRUE);// lyj 20180730
	MMPF_PIO_EnableOutputMode(MMP_GPIO62, MMP_TRUE, MMP_FALSE);
	MMPF_PIO_SetData(MMP_GPIO62, 1, MMP_TRUE);
	#endif

	//~~~~~~~~~~~~~~~end~~~~~~~~~~~~~
	#ifndef	NO_PANEL
	// InitOSD will make LCD off, Shutdown LCD backlight must before,
	// Otherwise, Some kind of LCD will show ugly white lines
	//LedCtrl_LcdBackLight(AHC_FALSE);
	//LedCtrl_LcdBackLightLock(AHC_TRUE);
	#endif	// NO_PANEL

    //InitOSD(); //Move to CE_JOB_DISPATCH_Task for fast boot.

    AHC_OS_ReleaseSem(ahc_System_Ready);

    //InitKeyPad(); //Move to CE_JOB_DISPATCH_Task for fast boot.
	
    AHC_UartRegisterUartCmdList(sAhcUartCommand);

#ifdef CFG_BOOT_FLASH_MODE //may be defined in config_xxx.h, could be FLASH_OFF or FLASH_ON
	// Always set Flash ON or OFF when system start up.
	#if CFG_BOOT_FLASH_MODE
	    MenuSettingConfig()->uiFlashMode = FLASH_ON;
	#else
	    MenuSettingConfig()->uiFlashMode = FLASH_OFF;
	#endif
#endif
#ifdef CFG_BOOT_MOV_CLIP_TIME //may be defined in config_xxx.h, could be MOVIE_CLIP_TIME_OFF or MOVIE_CLIP_TIME_xMIN
	MenuSettingConfig()->uiMOVClipTime 	 = CFG_BOOT_MOV_CLIP_TIME;
#endif
#ifdef CFG_BOOT_SLIDESHOW_FILE //may be defined in config_xxx.h, could be SLIDESHOW_FILE_STILL
	// Force slideshow photo only
	MenuSettingConfig()->uiSlideShowFile = SLIDESHOW_FILE_STILL; 
#endif
#ifdef CFG_BOOT_CHECK_USB_KEY_FILE //may be defined in config_xxx.h, a file name such as "SD:\\aituvc.txt"
	// Check USB Key file to run PCCAM
	AHC_CheckUSB_KeyFile(CFG_BOOT_CHECK_USB_KEY_FILE, 1 /* 1 for PCCAM */);
#endif
#ifdef CFG_BOOT_LCD_ROTATE //may be defined in config_xxx.h, could be LCD_ROTATE_OFF
    // Init LCD direction, always is normal
	MenuSettingConfig()->uiLCDRotate = CFG_BOOT_LCD_ROTATE;
#endif
#ifdef CFG_BOOT_FORCE_TURN_OFF_WIFI //	defined in config_xxx.h, for project of key control Wi-Fi On/Off
	Setpf_WiFi(WIFI_MODE_OFF);
#endif
#ifdef CFG_BOOT_FORCE_TURN_ON_WIFI	//	defined in config_xxx.h, for project of key control Wi-Fi On/Off
	Setpf_WiFi(WIFI_MODE_ON);
#endif
#ifdef CFG_VIDEO_FORMAT_SELECT_BY_SWITCH
	CFG_VIDEO_FORMAT_SELECT_BY_SWITCH();
#endif

#if(MENU_REARCAM_TYPE_EN)
	if(SCD_CAM_NONE != MMP_GetScdCamType()){
		pf_RearCamType_EnSet(USB_CAM_SONIX_MJPEG2H264 + MMP_GetScdCamType());
	}
	else {
		pf_RearCamType_EnSet(MMP_GetUSBCamType());
	}
#endif
    
    //Setpf_WiFi(WIFI_MODE_ON);
    
    uiStateSystemStart();

	AHC_PreSetLCDDirection();

    #if 0//#ifdef CFG_BOOT_CHECK_USB_UPDATE_FILE //may be defined in config_xxx.h, a file name such as "SD:\\FLYTECAM.bin"
    if(AHC_IsUsbConnect()) {
	    if(IsAdapter_Connect()) {
		   SDUpdateCheckFileExisted(CFG_BOOT_CHECK_USB_UPDATE_FILE);
	    } 
	    else {
		    NotifyUSB_HwStatus(1);
	    }
    }	
    #endif

#if (VR_VIDEO_TYPE==COMMON_VR_VIDEO_TYPE_3GP)
    AHC_VIDEO_SetMovieConfig(0, AHC_CLIP_CONTAINER_TYPE     , AHC_MOVIE_CONTAINER_3GP);
#elif (VR_VIDEO_TYPE==COMMON_VR_VIDEO_TYPE_AVI)
    AHC_VIDEO_SetMovieConfig(0, AHC_CLIP_CONTAINER_TYPE     , AHC_MOVIE_CONTAINER_AVI);
#endif

#if (TASK_MONITOR_ENABLE)
    memcpy(&gsTaskMonitorAhcMain.TaskName[0], __func__, TASK_MONITOR_MAXTASKNAMELEN);
    gsTaskMonitorAhcMain.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_WAITING;
    gsTaskMonitorAhcMain.ulExecTime = 0;
    memset((void *)gsTaskMonitorAhcMain.ParaArray, 0x0, sizeof(gsTaskMonitorAhcMain.ParaArray));
    gsTaskMonitorAhcMain.ulParaLength = 0;    
    gsTaskMonitorAhcMain.pTimeoutCB = (TASK_MONITOR_TimeoutCallback *)NULL;
    MMPF_TaskMonitor_RegisterTask(&gsTaskMonitorAhcMain);
    //MMPF_TaskMonitor_RegisterGblCB((void *)&AHC_PrintUITask);
    MMPF_TaskMonitor_RegisterGblCB((void *) NULL);
#endif

#if defined(AIT_HW_WATCHDOG_ENABLE) && (AIT_HW_WATCHDOG_ENABLE)
        AHC_WD_Enable(AHC_TRUE);
#endif

#ifndef	NO_PANEL
		// InitOSD will make LCD off, Shutdown LCD backlight must before,
		// Otherwise, Some kind of LCD will show ugly white lines
		//LedCtrl_LcdBackLight(AHC_FALSE);
		//LedCtrl_LcdBackLightLock(AHC_TRUE);
#endif	// NO_PANEL

#if defined(SUPPORT_SPEECH_RECOG)&&(SUPPORT_SPEECH_RECOG)
	MMPS_Sensor_StartSpeechRecog(0,MMP_TRUE);
	MMPS_AUDIO_InitLiveRecord(MMP_AUDIO_PCM_ENCODE, 0, 16000, (MMP_LivePCMCB)SpeechPCMTransferCB);
	MMPS_LiveAudio_StartRecord();
	MMPS_StartSpeechRecog((MMP_SpeechRecogCbFunc)SpeechRecogRunCB);
#endif
	Main_Set_Page(MENU_MAIN_FLAG);// LIAO

	//MMPF_PIO_EnableOutputMode(USB_EN_SELECT,MMP_TRUE,MMP_TRUE);// long 4-20

       // MMPF_PIO_SetData(USB_EN_SELECT, 1,MMP_TRUE);  //long 4-20




	
	

//FM==========long======
	//printc("~~~~~long~~test123~~~~\r\n");
	//fm7703_init();
	//BD3490_init();
//	uart_to_mcu();
	//AHC_OS_SleepMs(500);
	BD3490_reginit();
	//SendMessageMCU = 1; // lyj 20190917

		 #if 1
	 {
	 	
	 	//main
	 	volume_conver_size(0xc0,mainVflag_count);
		//sub
		//AHC_OS_SleepMs(100);
		volume_conver_size(0xc7,sub_count_ex);
		volume_conver_size(0xc8,sub_count_ex);
		// fornt
		//AHC_OS_SleepMs(100);
		volume_conver_size(0xc1,MenuSettingConfig()->uiVolume);
		volume_conver_size(0xc2,MenuSettingConfig()->uiVolume);
		//rear
		//AHC_OS_SleepMs(100);
		volume_conver_size(0xc3,MenuSettingConfig()->uiMOVPowerOffTime);
		volume_conver_size(0xc4,MenuSettingConfig()->uiMOVPowerOffTime);
		//output
		//AHC_OS_SleepMs(100);
		volume_conver_size(0xc5,MenuSettingConfig()->uiEffect);
		volume_conver_size(0xc6,MenuSettingConfig()->uiEffect);
	 }//lyj 20181022
	 #endif


	
	//printc("~~~~~longteset124~~~~~~~~~\r\n");
    
//================long===    

      key_24g_parser_vlevel();
		#if 0
		{
			printc("~a~~MenuSettingConfig()->uiMOVQuality = %d~~~~~\r\n",MenuSettingConfig()->uiMOVQuality);
			printc("~a~~MenuSettingConfig()->uiVMDRecTime  = %d~~~~~\r\n",MenuSettingConfig()->uiVMDRecTime );
			printc("~a~~MenuSettingConfig()->uiEV  = %d~~~~~\r\n",MenuSettingConfig()->uiEV);
			printc("~a~~MenuSettingConfig()->uiSpeedStamp  = %d~~~~~\r\n",MenuSettingConfig()->uiSpeedStamp);
			printc("~~a~MenuSettingConfig()->uiEffect  = %d~~~~~\r\n",MenuSettingConfig()->uiEffect);
			printc("~~a~MenuSettingConfig()->uiBatteryVoltage  = %d~~~~~\r\n",MenuSettingConfig()->uiBatteryVoltage);
			printc("~a~~MenuSettingConfig()->uiVolume  = %d~~~~~\r\n",MenuSettingConfig()->uiVolume);
			printc("~a~~MenuSettingConfig()->uiMOVPowerOffTime  = %d~~~~~\r\n",MenuSettingConfig()->uiMOVPowerOffTime);
			printc("~~~MenuSettingConfig()->uiIMGSize  = %d~~~~~\r\n",MenuSettingConfig()->uiIMGSize);
			printc("~~~MenuSettingConfig()->uiBeep  = %d~~~~~\r\n",MenuSettingConfig()->uiBeep);
			printc("~~~MenuSettingConfig()->uiMOVClipTime  = %d~~~~~\r\n",MenuSettingConfig()->uiMOVClipTime);//uiLCDBrightness
			printc("~~~MenuSettingConfig()->uiMOVClipTime  = %d~~~~~\r\n",MenuSettingConfig()->uiLCDBrightness);
			 printc("~~~MenuSettingConfig()->uiMOVClipTime  = %d~~~~~\r\n",MenuSettingConfig()->uiTimeZone); 

		}
		#endif
			//printc("~~~MenuSettingConfig()->uiPowerUpFlag  = %d~~~~~\r\n",MenuSettingConfig()->uiPowerUpFlag);
	//	printc("~~~MenuSettingConfig()->uiRGBstatus  = %d~~~~~\r\n",MenuSettingConfig()->uiRGBstatus);
		//printc("~~~MenuSettingConfig()->uiBiglangStauts  = %d~~~~~\r\n",MenuSettingConfig()->uiBiglangStauts);
	//	printc("~~~MenuSettingConfig()->uiSpeedRGB  = %d~~~~~\r\n",MenuSettingConfig()->uiSpeedRGB);
		//printc("~~~MenuSettingConfig()->uiRGBmode  = %d~~~~~\r\n",MenuSettingConfig()->uiRGBmode);
		AHC_OS_SleepMs(5);
		ReadTheStatus(); // lyj 20190606

		if(BlueENpin == 0)
			{
			BlueENpin = 1;
			AHC_OS_SleepMs(10);
			PowerOnReadTheE2promToAM_FM();
	}
		
		//printc("~~~MenuSettingConfig()->uiMOVClipTime  = %d~~~~~\r\n",MenuSettingConfig()->uiLCDBrightness);
			// printc("~~~MenuSettingConfig()->uiTimeZone  = %d~~~~~\r\n",MenuSettingConfig()->uiTimeZone); 
		 SelfAllFlag.Speedmode = MenuSettingConfig()->uiSpeedRGB ;
		SelfAllFlag.onceFlag = MenuSettingConfig()->uiRGBmode ; // lyj 20191021
		//==============lyj 20191021===============
		if(MenuSettingConfig()->uiRGBstatus ==1)
			flag_color = 1,app_flag =2;
		if(MenuSettingConfig()->uiRGBstatus ==2)
			SelfAllFlag.SlowBecome = 1,app_flag=2;// lyj 20191021
		{
			extern AHC_BOOL			lightBarFlag_1;
			extern AHC_BOOL			lightBarFlag_2;
			extern AHC_BOOL			lightBarFlag_3;
			extern AHC_BOOL			lightBarFlag_4;
			extern AHC_BOOL			lightBarFlag_5;
			extern AHC_BOOL			lightBarFlag_6;
			extern void light_Bar_fun(MMP_GPIO_PIN pin,AHC_BOOL vel);

			AHC_BOOL dex_i = 0;
			/*switch(MenuSettingConfig()->uiBiglangStauts)
			{
					case 1:
						light_Bar_fun(MMP_GPIO43,0);
						break;
					
					case 2:
						light_Bar_fun(MMP_GPIO44,0);
						break;

					case 3:
						light_Bar_fun(MMP_GPIO45,0);
						break;
					case 4:
						light_Bar_fun(MMP_GPIO46,0);
						break;
					case 5:
						light_Bar_fun(MMP_GPIO47,0);
						break;
					case 6:
						light_Bar_fun(MMP_GPIO48,0);
						break;
			}*/

			for(dex_i=0; dex_i<6;dex_i++)
			{
				
				if((MenuSettingConfig()->uiBiglangStauts >> dex_i)&0x01)
				{
					//AHC_BOOL lamstatus = MenuSettingConfig()->uiBiglangStauts >> dex_i;
						switch(dex_i)
						{
							case 0:
								lightBarFlag_1 =1;
							light_Bar_fun(MMP_GPIO43,0);
							break;
						
							case 1:
								lightBarFlag_2 =1;
								light_Bar_fun(MMP_GPIO44,0);
								break;

							case 2:
								lightBarFlag_3 =1;
								light_Bar_fun(MMP_GPIO45,0);
								break;
							case 3:
								lightBarFlag_4 =1;
								light_Bar_fun(MMP_GPIO46,0);
								break;
							case 4:
								lightBarFlag_5 =1;
								light_Bar_fun(MMP_GPIO47,0);
								break;
							case 5:
								lightBarFlag_6 =1;
								light_Bar_fun(MMP_GPIO48,0);
								break;


						}


				}



			}

			

		}// lyj 20191021 lamp status

    while(1)
    {
        UINT32 uiMsgId, uiParam1, uiParam2;
		UINT16 usCount;

        while (1) {
    		AHC_GetAHLHPMessageCount(&usCount);

    		if(usCount < AHC_HP_MSG_QUEUE_SIZE) {
    			if(AHC_GetAHLMessage_HP( &uiMsgId, &uiParam1, &uiParam2) == AHC_FALSE) {
    			    if (AHC_GetAHLMessage( &uiMsgId, &uiParam1, &uiParam2) == AHC_FALSE) {
        			    continue;
    			    }

			        break;
    			}

			    break;
    		}
    		else if (AHC_GetAHLMessage( &uiMsgId, &uiParam1, &uiParam2) == AHC_FALSE) {
		        continue;
    		}

		    break;
        }

//0-->0xaa  1-->0x55  2-->0x03  3-->0x0f  4-->0x02  5-->0x0d  6-->0x0a  flash 
//0xFE,0xFB,0x89 1,2,46
#if	0//(RGB_DATA_EN_DEBUG)

if((recv_data[1] == 0x55 && recv_data[3] == 0x0F && recv_data[4] == 0x01 && recv_data[5] == 0x0D) )
{
	app_flag = 1;
	i=0;
	j=0;
	j1=101;
	Clear_array_data();
	White_light_bar_off();
}
else if(recv_data[1] == 0x55 &&  recv_data[3] == 0x0F && recv_data[4] == 0x00 && recv_data[5] == 0x0D)
{
	app_flag = 0;
	
	Clear_array_data();
	
}

if(app_flag == 0)
{
	if((recv_data[7] == 0x0D && recv_data[8] == 0x0A && recv_data[5] != 0x0D))
	{
			float R2, G2, B2;
			
			get_data_RGB(&R2,&G2,&B2);
			if(R2 == 100 && G2 == 100 &&B2 == 100)
				{
					R2 = 46; G2 = 1; B2 = 2;

				}
			Color_chang(Freq_config,(MMP_ULONG)R2,(MMP_ULONG)G2,(MMP_ULONG)B2);
			if(lamp_mode ==1)
				Clear_array_data();
	}

	if((recv_data[1] == 0x55 &&  recv_data[3] == 0x0F && recv_data[4] == 0x04))
	{
		
		Candle_lamp();
		Clear_array_data();
	}

}
else if( flag_color == 1)
{
		float R2, G2, B2;
		flag_color = 0;	
			get_data_RGB(&R2,&G2,&B2);
			if(R2 == 100 && G2 == 100 &&B2 == 100)
				{
					R2 = 46; G2 = 1; B2 = 2;

				}
			Color_chang(Freq_config,(MMP_ULONG)R2,(MMP_ULONG)G2,(MMP_ULONG)B2);

} // lyj 20181022
	
#if 0 // lyj 20190118
	if(recv_data[0] == 0xAA && recv_data[1] == 0xAC)
	{

		UINT32 ulPosition;
	//	bluetooth_key = AHC_TRUE;
		
		
		if(recv_data[3] == 0xF2)
		{
			ulPosition = 0;
			Menu_Set_Page(0);
			MenuStateExecutionCommon1(ulPosition);
			printc("~~~~~F2F2F2F2F2F2F2F2F2F2~~~~~~~~~~\n");
		}
		else if(recv_data[3] == 0xF4)
		{
			ulPosition = 1;
			Menu_Set_Page(1);
			MenuStateExecutionCommon1(ulPosition);
		}
		else if(recv_data[3] == 0xF5)
		{
			ulPosition = 2;
			Menu_Set_Page(2);
			MenuStateExecutionCommon1(ulPosition);
		}
		else if(recv_data[3] == 0xF1)
		{
			ulPosition = 5;
			Menu_Set_Page(5);
			MenuStateExecutionCommon1(ulPosition);
		}
		else if(recv_data[3] == 0xF3)
		{
			ulPosition = 6;
			Menu_Set_Page(6);
			MenuStateExecutionCommon1(ulPosition);
		}
		#if 1
		else if(recv_data[3] == 0xF6)
		{
			//bluetooth_key = AHC_FALSE;
				if(recv_data[4] == 0x00)
				{
					
					Mute_ON_OFF = 1;
					mcu_flag = 1;
					//APP_Draw_Mute_icon(1);
				}
				else  if(recv_data[4] == 0x01)
				{
					
					Mute_ON_OFF = 2;
					mcu_flag = 0;
					//APP_Draw_Mute_icon(0);
				}
		}
		#endif

		Clear_array_data();
		

	}
	#endif
	#if 0
	else if (recv_data[1] == 0x55 && recv_data[4] == 0xFF && recv_data[10] == 0xFF)
	{

			UINT32 ulPosition;
			//bluetooth_key = AHC_TRUE;
			if(recv_data[3] == 0xF3 )
			{
				ulPosition = 7;
				Menu_Set_Page(7);	
				MenuStateExecutionCommon1(ulPosition);
			}

			//MenuStateExecutionCommon1(ulPosition);

			Clear_array_data();
		
	}
	#endif

#endif




	
		

#if (TASK_MONITOR_ENABLE)
        MMPF_OS_GetTime(&gsTaskMonitorAhcMain.ulExecTime);
        gsTaskMonitorAhcMain.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_RUNNING;    

        *(MMP_ULONG *)(gsTaskMonitorAhcMain.ParaArray) = (MMP_ULONG)uiMsgId;
        gsTaskMonitorAhcMain.ulParaLength = sizeof(MMP_ULONG);  

        *(MMP_ULONG *)(gsTaskMonitorAhcMain.ParaArray + gsTaskMonitorAhcMain.ulParaLength) = (MMP_ULONG)uiParam1;
        gsTaskMonitorAhcMain.ulParaLength += sizeof(MMP_ULONG);  
        
        *(MMP_ULONG *)(gsTaskMonitorAhcMain.ParaArray + gsTaskMonitorAhcMain.ulParaLength) = (MMP_ULONG)uiParam2;
        gsTaskMonitorAhcMain.ulParaLength += sizeof(MMP_ULONG);          
#endif

        AHC_OS_AcquireSem(ahc_System_Ready, 0);

		uiStateMachine(uiMsgId, uiParam1, uiParam2);//ui Init

        AHC_OS_ReleaseSem(ahc_System_Ready);

#if (TASK_MONITOR_ENABLE)
        gsTaskMonitorAhcMain.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_WAITING;
#endif
    }
}
#if (MENU_BLUETOOTH_PROGRESS_BAR)
//UINT16 wSecond;
//extern UINT16 wDispID;
unsigned char lengthBull = 0,str_first6[52]={/*0x9e,0xa6,0x75,0x30,0x6b,0x23,0x79,0xd1,0x62,0x80,0x67,0x09,0x96,0x50,0x51,0x6c,0x53,0xf8,0x00,0x2e,0x00,0x6d,0x00,0x70,0x00,0x33};// {0x53,0x43,0x5e,0x74,0x7b,0x49,0x4e,0x00,0x56,0xde*/'\0'};

//extern AHC_BOOL      ReciveDataLength;
#endif
void AHC_WorkStack_Task(void *p_arg)
{
	#define WORK_STACK_WAIT_UINT       50   // ms
	#define RTC_UPDATE_PERIOD          1    // ms, RTC_UPDATE_PERIOD * WORK_STACK_WAIT_UINT
	#define CHARGING_CHECK_PERIOD      3    // ms, CHARGING_CHECK_PERIOD * WORK_STACK_WAIT_UINT
//-------------------------------
#if (MENU_BLUETOOTH_PROGRESS_BAR)
	RECT tmpRECT1 = {25,   265,   43, 30};
	RECT tmpRECT2 = {356,   265,   43, 30};
	RECT tmpRECT3 = {50,   200,   350, 30};
	RECT tmpRECT4 = {100,   240,   300, 30};
	RECT tmpRECT8 = {100,   290,   300, 25};
	//posite data[]
	/*RECT tmpRECT_data[15] = {{100,   200,   20, 30},{120,   200,   20, 30},{140,   200,   20, 30},{160,   200,   20, 30},{180,   200,   20, 30},{200,   200,   20, 30},{220,   200,   20, 30},{240,   200,   20, 30},{260,   200,   20, 30},\
	{280,   200,   20, 30},{300,   200,   20, 30},{320,   200,   20, 30},{340,   200,   20, 30},{360,   200,   20, 30},{380,   200,   20, 30}};*/
	RECT tmpRECT_data[30] = {{50,   200,   20, 30},{65,   200,   20, 30},{80,   200,   20, 30},{95,   200,   20, 30},{110,   200,   20, 30},{125,   200,   20, 30},{140,   200,   20, 30},{155,   200,   20, 30},{170,   200,   20, 30},\
	{185,   200,   20, 30},{200,   200,   20, 30},{215,   200,   20, 30},{230,   200,   20, 30},{245,   200,   20, 30},{260,   200,   20, 30},\
	{275,   200,   20, 30},{290,   200,   20, 30},{305,   200,   20, 30},{320,   200,   20, 30},{335,   200,   20, 30},{350,   200,   20, 30},\
	{365,   200,   20, 30},{380,   200,   20, 30},{395,   200,   20, 30}};
	 
	

	RECT tmpRECT5 = {17,   15,   44, 36};// {18,   16,   44, 36}
	 char    szv[16];
	// char    szv_1[64];
	 GUI_COLOR Color_2;
	 UINT16          BarPos = 0  /*,OldBarPos */,OldTotalTime = 10000,/*OldPlayTime = 0,*/countMusic = 0;
	
	 RECT            rc = RECT_MENU_ADJUST_BAR_ITEM;
#endif
//----------------------------
    UINT32 uiTaskCounter = 0;

	AHC_WaitForBootComplete();

#if (TASK_MONITOR_ENABLE)
    memcpy(&gsTaskMonitorAHCWorkStack.TaskName[0], __func__, TASK_MONITOR_MAXTASKNAMELEN);
    gsTaskMonitorAHCWorkStack.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_WAITING;
    gsTaskMonitorAHCWorkStack.ulExecTime = 0;
    memset((void *)gsTaskMonitorAHCWorkStack.ParaArray, 0x0, sizeof(gsTaskMonitorAHCWorkStack.ParaArray)); 
    gsTaskMonitorAHCWorkStack.ulParaLength = 0;    
    gsTaskMonitorAHCWorkStack.pTimeoutCB = (TASK_MONITOR_TimeoutCallback *)NULL;       
    MMPF_TaskMonitor_RegisterTask(&gsTaskMonitorAHCWorkStack);
#endif
    
    while(1)
    {
    	// TODO: This task only for draw Battery Icon!?
	//printc("---------------Battery Icon!?-----------\r\n");
        uiTaskCounter++;
#if (TASK_MONITOR_ENABLE)
        MMPF_OS_GetTime(&gsTaskMonitorAHCWorkStack.ulExecTime);
        gsTaskMonitorAHCWorkStack.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_RUNNING;    
        *(MMP_ULONG *)(gsTaskMonitorAHCWorkStack.ParaArray) = (MMP_ULONG)uiTaskCounter;
        gsTaskMonitorAHCWorkStack.ulParaLength = sizeof(MMP_ULONG); 
#endif  
//--------------------------------------
#if (MENU_BLUETOOTH_PROGRESS_BAR)// USB与Bluetooth 时间显示

if(/*Main_Get_Page() == BLUETOOH_FLAG||Main_Get_Page() == USB_FLAG*/Menu_Get_Page() == 6)
{
	if(Main_Get_Page() == USB_FLAG && lengthBull !=0)
	{
	#if (MENU_BLUETOOTH_PROGRESS_BAR)// 进度条与读秒
	//------------------------------------------
	//OldBarPos = BarPos;
	if(play_time_HtoL > 900)
		play_time_HtoL = 1;
	#if 0
	if(play_time_HtoL > 900)
		play_time_HtoL = 0;
	//if(play_time_T > 900)
	//	play_time_T = 0;
	//if(play_time_T > 0)
		//play_time_T -= 1; 
	if(OldPlayTime +1 == play_time_HtoL)
		BarPos = play_time_HtoL*rc.uiWidth/play_time_T;
	else
		BarPos = 0;
	//printc("playTime = %d,totalTime = %d \n",play_time_HtoL,play_time_T);
	//OldBarPos = BarPos;
	if(BarPos >260 /*|| BarPos > OldBarPos + 50*/ /*|| (OldBarPos > 50 &&BarPos < OldBarPos - 50)*/)
	{
			BarPos = 2;
	}
	#endif

    AHC_OSDSetColor(wDispID, OSD_COLOR_USB);
    AHC_OSDSetPenSize(wDispID, 1);

	#if 0
    AHC_OSDDrawLine(wDispID, rc.uiLeft, rc.uiLeft+rc.uiWidth, rc.uiTop, rc.uiTop);
    AHC_OSDDrawLine(wDispID, rc.uiLeft, rc.uiLeft+rc.uiWidth, rc.uiTop+rc.uiHeight, rc.uiTop+rc.uiHeight);
    AHC_OSDDrawLine(wDispID, rc.uiLeft, rc.uiLeft, rc.uiTop, rc.uiTop+rc.uiHeight);
    AHC_OSDDrawLine(wDispID, rc.uiLeft+rc.uiWidth, rc.uiLeft+rc.uiWidth, rc.uiTop, rc.uiTop+rc.uiHeight);
	#endif
    //Draw Progress Bar
if(OldTotalTime == play_time_T)
{
    AHC_OSDSetColor(wDispID, OSD_COLOR_ORANGE_2);//选定
    AHC_OSDDrawFillRect(wDispID, rc.uiLeft+1, rc.uiTop+1, (rc.uiLeft+1)+BarPos, rc.uiTop+rc.uiHeight-1);

    //Draw Progress Bar
    AHC_OSDSetColor(wDispID, OSD_COLOR_DARKGRAY);//未选定

    AHC_OSDDrawFillRect(wDispID, (rc.uiLeft+1)+BarPos, rc.uiTop+1, rc.uiLeft+rc.uiWidth+1, rc.uiTop+rc.uiHeight-1);

#endif


	switch(Main_Get_Page())
	{
		//case BLUETOOH_FLAG :   Color_2 = OSD_COLOR_BLUETOOTH;  break;
		case USB_FLAG :             Color_2 = OSD_COLOR_USB;  break;
	}
	
     
	// 当前时间
	if(OldPlayTime +1 == play_time_HtoL)
	{
		BarPos = play_time_HtoL*rc.uiWidth/play_time_T;
		//else
		//BarPos = 0;
		if(BarPos >260 )
		{
				BarPos = 2;
		}
	        sprintf(szv, "%02d:%02d", play_time_HtoL/60,play_time_HtoL%60);
			AHC_OSDSetColor(wDispID, Color_2); 
		AHC_OSDSetFont(wDispID, &GUI_Font20_1);
		  AHC_OSDDrawFillRect(wDispID, tmpRECT1.uiLeft, tmpRECT1.uiTop, 68, 295);
		OSD_ShowString( wDispID,szv, NULL, tmpRECT1, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
	}
	else if(play_time_HtoL == 0 || play_time_HtoL == 1 || play_time_HtoL == 2)
	{
		BarPos = play_time_HtoL*rc.uiWidth/play_time_T;
	        sprintf(szv, "%02d:%02d", play_time_HtoL/60,play_time_HtoL%60);
			AHC_OSDSetColor(wDispID, Color_2); 
		AHC_OSDSetFont(wDispID, &GUI_Font20_1);
		  AHC_OSDDrawFillRect(wDispID, tmpRECT1.uiLeft, tmpRECT1.uiTop, 68, 295);
		OSD_ShowString( wDispID,szv, NULL, tmpRECT1, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);

	}
	// 总时间
	 sprintf(szv, "%02d:%02d", play_time_T/60,play_time_T%60);
	 AHC_OSDSetColor(wDispID, Color_2); 
	AHC_OSDSetFont(wDispID, &GUI_Font20_1);
	  AHC_OSDDrawFillRect(wDispID, tmpRECT2.uiLeft, tmpRECT2.uiTop, 399, 295);
	OSD_ShowString( wDispID,szv, NULL, tmpRECT2, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);

	// 歌曲名
	 //sprintf(szv_1, /*"Name Of The Song Liao Rong"*/"take me to your heart"/*"朋友"*/);
	 AHC_OSDSetColor(wDispID, Color_2); 
	AHC_OSDSetFont(wDispID, &GUI_Font20_1);
	  //AHC_OSDDrawFillRect(wDispID, tmpRECT3.uiLeft, tmpRECT3.uiTop, tmpRECT3.uiLeft+tmpRECT3.uiWidth, tmpRECT3.uiTop+tmpRECT3.uiHeight);


	if(countMusic++ == 9)
	{
		unsigned char k/*,lengthBull = 0*/,posite_i;
		//int tmp;//=0x96CD-0x4E00;
	       unsigned char str_first[2];
		//   char str[]={0x96,0xcd};
		  unsigned short tmp1,temp,tmp;
		   // lengthBull = dealDataUSB(str_first6);
			countMusic = 0;
		   AHC_OSDDrawFillRect(wDispID, tmpRECT3.uiLeft, tmpRECT3.uiTop, tmpRECT3.uiLeft+tmpRECT3.uiWidth, tmpRECT3.uiTop+tmpRECT3.uiHeight);
		   for(k=0,posite_i =0;k<lengthBull;k=k+2,posite_i++)
		   {
			 tmp1=(str_first6[k]<<8)+str_first6[k+1];
			//tmp = (tmp1>>8);
			#if 1
			if(str_first6[k]>=0x4e)
			{
				ShowOSD_SetLanguage(wDispID);
				tmp=tmp1-0x4e00;
				temp=tmp+0xA1A1;
				str_first[0]=(temp>>8);
		      		str_first[1]=(unsigned char)temp;
		
			
					if(posite_i != 0)
					{
						if(tmpRECT_data[posite_i -1].uiWidth == 10)
							tmpRECT_data[posite_i].uiLeft= tmpRECT_data[posite_i -1].uiLeft +10;
						else
							tmpRECT_data[posite_i].uiLeft= tmpRECT_data[posite_i -1].uiLeft +18;
					}
					tmpRECT_data[posite_i].uiWidth = 18;
					OSD_ShowString(wDispID, (char*)str_first, NULL, tmpRECT_data[posite_i], OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT, GUI_TA_HCENTER|GUI_TA_VCENTER); 
		
			}
			else if(str_first6[k]==0x00)
			{
		
					if(posite_i != 0)
					{
						if(tmpRECT_data[posite_i - 1].uiWidth == 18)
							tmpRECT_data[posite_i].uiLeft= tmpRECT_data[posite_i -1].uiLeft +18;	
						else
							tmpRECT_data[posite_i].uiLeft= tmpRECT_data[posite_i -1].uiLeft +10;	
					}
					tmpRECT_data[posite_i].uiWidth = 10;
					if(str_first6[k+1] != 0x20)
					OSD_ShowString(wDispID, /*(char*)str_first6[k+1]*/ConverString(str_first6[k+1]), NULL, tmpRECT_data[posite_i], OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT, GUI_TA_HCENTER|GUI_TA_VCENTER); 
			
					
			}
			#endif
		   }
	
	  }
	}
	//OldTotalTime = play_time_T;
	OldTotalTime = play_time_T;
	//OldPlayTime = play_time_HtoL;
}
	
}
	OldPlayTime = play_time_HtoL;

#endif

//===============================lyj==========
#if 0 // have problem
if(recv_data[3] == 0xF7 && recv_data[1] == 0xAC)
{
		if(recv_data[4] == 0x00)
		{
			
			Mute_ON_OFF = 1;
			mcu_flag = 1;
			//APP_Draw_Mute_icon(1);
		}
		else if(recv_data[4] == 0x01)
		{
			
			Mute_ON_OFF = 2;
			mcu_flag = 0;
			//APP_Draw_Mute_icon(0);
		}
}
#endif

if(mcu_flag == 0 )
{	// 44 ,36
	Draw_Mute_icon(tmpRECT5,0);
}
else if(mcu_flag == 1)
{

	Draw_Mute_icon(tmpRECT5,1);

}

#if 0
if(BlueStause ==1)
{
	if(BlueStauseEx ==1)
	{
		DrawBluetoothLinkIcon(1);// lyj 20181025
	}
	else if(BlueStauseEx ==0 )
	{
		DrawBluetoothLinkIcon(0);// lyj 20181025
	}
}
#endif

if(app_flag == 0)
{
	
	if(recv_data[1] == 0xAB &&  recv_data[3] == 0xF6 && recv_data[6] == 0x0A)
	{
		//app_contrl_Backlight();		
	}
	
	
}

#if 1
{

	if(sub_flag_ex == 1)
	{
		if(flag_add == 1)
		{
			sub_count_ex++;
			
			 Draw_Main_volum_icon(&sub_count_ex,1,wDispID);
			// if(sub_count_ex > 10)
				//sub_count_ex = 10;
			flag_add = 0;
		}
		if(flag_sub== 1)
		{
			sub_count_ex--;	
			//if(sub_count_ex < 0)
				//sub_count_ex = 0;
			 Draw_Main_volum_icon(&sub_count_ex,1,wDispID);
			 flag_sub = 0;

		}

	}
	else
	{
		if(flag_add == 1)
		{
			mainVflag_count++;
			//if(mainVflag_count > 10)
				//mainVflag_count = 10;
			 Draw_Main_volum_icon(&mainVflag_count,0,wDispID);
			flag_add = 0;
			if(mainVflag_count != 0) // lyj 20190530
			{
				Mute_ON_OFF = 1;
					mcu_flag = 1;
			}
		}
		if(flag_sub== 1)
		{
			mainVflag_count--;	
			//if(mainVflag_count < 0)
				//mainVflag_count = 0;
			 Draw_Main_volum_icon(&mainVflag_count,0,wDispID);
			 flag_sub = 0;
			 if(mainVflag_count != 0)  // lyj 20190530
			{
				Mute_ON_OFF = 1;
					mcu_flag = 1;
			}

		}

	}



}


if(recv_data[0] == 0xAA && recv_data[1] == 0xAC)
	{

		UINT32 ulPosition;
	//	bluetooth_key = AHC_TRUE;
		
		
		if(recv_data[3] == 0xF2)
		{
			ulPosition = 0;
			Menu_Set_Page(0);
			MenuStateExecutionCommon1(ulPosition);
			//printc("~~~~~F2F2F2F2F2F2F2F2F2F2~~~~~~~~~~\n");
		}
		else if(recv_data[3] == 0xF4)
		{
			ulPosition = 1;
			Menu_Set_Page(1);
			MenuStateExecutionCommon1(ulPosition);
		}
		else if(recv_data[3] == 0xF5)
		{
			ulPosition = 2;
			Menu_Set_Page(2);
			MenuStateExecutionCommon1(ulPosition);
		}
		else if(recv_data[3] == 0xF1)
		{
			ulPosition = 5;
			Menu_Set_Page(5);
			MenuStateExecutionCommon1(ulPosition);
		}
		else if(recv_data[3] == 0xF3)
		{
			ulPosition = 6;
			Menu_Set_Page(6);
			MenuStateExecutionCommon1(ulPosition);
		}
		#if 1
		else if(recv_data[3] == 0xF6)
		{
			//bluetooth_key = AHC_FALSE;
				/*if(recv_data[4] == 0x00)
				{
					
					Mute_ON_OFF = 1;
					mcu_flag = 1;
					//APP_Draw_Mute_icon(1);
				}
				else  if(recv_data[4] == 0x01)
				{
					
					Mute_ON_OFF = 2;
					mcu_flag = 0;
					//APP_Draw_Mute_icon(0);
				}*/

			if(mcu_flag)
			{
				//uart_to_mcu();
				//Key24_switch();
				mcu_flag = 0;
				Mute_ON_OFF = 2;
			}	
			else
			{
				//uart_to_mcu_off();
				mcu_flag = 1;
				Mute_ON_OFF = 1;
			}
		}
		#endif

		Clear_array_data();
		

	}
#endif


//===============================end==========
//----------------------------------------
        ///////////////////////////////////////////////////////////////////////////////
        // RTC Update Counter
        ///////////////////////////////////////////////////////////////////////////////
        if((uiTaskCounter % RTC_UPDATE_PERIOD) == 0) {
            AHC_SetParam(PARAM_ID_USE_RTC, 1);
            AHC_RTC_UpdateTime();
        }

        #if (OSD_SHOW_BATTERY_STATUS)
        if ((uiTaskCounter % CHARGING_CHECK_PERIOD) == 0 && gAHC_InitialDone) {
    		if ((AHC_TRUE == AHC_Charger_GetStatus()))
    		{
    			AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_UPDATE_CHARGE_ICON, 0);
    		}
        }
        #endif

        #if (TASK_MONITOR_ENABLE)
        gsTaskMonitorAHCWorkStack.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_WAITING;        
        #endif
        
        AHC_OS_SleepMs(WORK_STACK_WAIT_UINT);
    }
}

//extern AHC_BOOL ReadMusicFlag;
void AHC_WorkJob_Task(void *p_arg)
{
#if (MENU_BLUETOOTH_PROGRESS_BAR)

 char    szv[16];
 RECT tmpRECT1;
AHC_RTC_TIME 	CurRtctime;
GUI_COLOR Color_1;
static MMP_UBYTE acc_det_old = 100;
static int acc_detCount = 0;
static INT8 MCU_message = 0; // lyj 20190917
 INT32		level=0;
// INT32 	OldLevel = 0;
	 UINT16		level1=0;

#endif
	static MMP_ULONG Count = 0;
    static MMP_ULONG Count1 = 0,Task_delay_count = 0;
	static MMP_UBYTE 	acc_det=0;
	static MMP_UBYTE 	stausedamp=100; // lyj 20181025
	//static MMP_UBYTE 	Oldstausedamp=0; // lyj 20181025
#if (GPS_CONNECT_ENABLE)    
    static MMP_ULONG ulGPSInfoRefreshCnt;
#endif

extern AHC_BOOL Yaokongqi_flag; // lyj 20190423
	#if 0
	// lyj 20180829
	//MMPF_PIO_EnableOutputMode(MMP_GPIO63, MMP_FALSE, MMP_FALSE);
	 MMPF_PIO_EnableOutputMode(MMP_GPIO25, MMP_FALSE, MMP_TRUE);
	#endif
	#if 0// lyj 20190226
	MMPF_PIO_PadConfig(USB_EN_SELECT, PAD_OUT_DRIVING(0), MMP_TRUE);// lyj 20180929
	MMPF_PIO_EnableOutputMode(USB_EN_SELECT, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_SetData(USB_EN_SELECT,1,MMP_TRUE);
	#endif
#if (TASK_MONITOR_ENABLE)
    memcpy(&gsTaskMonitorAHCWorkStack.TaskName[0], __func__, TASK_MONITOR_MAXTASKNAMELEN);
    gsTaskMonitorAHCWorkStack.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_WAITING;
    gsTaskMonitorAHCWorkStack.ulExecTime = 0;
    memset((void *)gsTaskMonitorAHCWorkStack.ParaArray, 0x0, sizeof(gsTaskMonitorAHCWorkStack.ParaArray)); 
    gsTaskMonitorAHCWorkStack.ulParaLength = 0;    
    gsTaskMonitorAHCWorkStack.pTimeoutCB = (TASK_MONITOR_TimeoutCallback *)NULL;       
    MMPF_TaskMonitor_RegisterTask(&gsTaskMonitorAHCWorkStack);
#endif

    while(1)
    {    


	//=============Task delay=============
	if(Task_delay_flag)
	{
		Task_delay_count++;
		if(Task_delay_count > 3)
		{
			Task_delay_count = 1;
			Task_delay_flag = 0;
			Task_lockOn_flag = 1;
		}

	}
	#if 0
	else
	{
		Task_delay_count = 1;

	}
	#endif
	if(Yaokongqi_flag > 0 && Main_Get_Page()!= PREVIEW_FLAG)
	{
		switch(Yaokongqi_flag)
		{
			extern AHC_BOOL return_flag;

			case 1:
				Yaokongqi_flag =0; // prev
				//if(Main_Get_Page() == BLUETOOH_FLAG)
				if(SelfAllFlag.upDwonUB & 0x01)
					BlueMusicPrev();
				//else if(Main_Get_Page() == USB_FLAG)
				else if(SelfAllFlag.upDwonUB & 0x02)
					USBMusicPrev();
				else if(Main_Get_Page() == RADIO_FM_FLAG && return_flag == 1)
					//SetKeyPadEvent(BUTTON_LEFT_REL);
					SetKeyPadEvent(BUTTON_RIGHT_REL);
				break;


				case 2:		
				Yaokongqi_flag =0;	 // next
				//if(Main_Get_Page() == BLUETOOH_FLAG)
				if(SelfAllFlag.upDwonUB & 0x01)
					BlueMusicNext();
				//else if(Main_Get_Page() == USB_FLAG)
				else if(SelfAllFlag.upDwonUB & 0x02)
					USBMusicNext();
				else if(Main_Get_Page() == RADIO_FM_FLAG  && return_flag == 1)
					//SetKeyPadEvent(BUTTON_RIGHT_REL);
					SetKeyPadEvent(BUTTON_LEFT_REL);
				break;

				case 3:
				Yaokongqi_flag =0;	// play
				//if(Main_Get_Page() == BLUETOOH_FLAG)
				if(SelfAllFlag.upDwonUB & 0x01)
					BlueMusicPlay_Pause();
				//else if(Main_Get_Page() == USB_FLAG)
				if(SelfAllFlag.upDwonUB & 0x02)
					USBMusicPlay_Pause();
				//else if(Main_Get_Page() == RADIO_FM_FLAG)
					//SetKeyPadEvent(BUTTON_SET_PRESS);// lyj 20190427
				
				break;
				
		default:
			Yaokongqi_flag =0;
			break;


		}


	}

	
	#if  1
	//if(BlueStause == 1)
	{
		Count1++;
		if(Count1 > 100)
			Count1 = 0;
	//if( ReadMusicFlag	== 1)
		lengthBull = dealDataUSB(str_first6);
		if(Count1%5 == 0)
		{
			//if(str_first1[0] != 0xaa /*&& str_first1[2] != 0xf5&& str_first1[3] != 0xa7*/)
			//{
				// str_first1[0] = 0;
				 SPI_CS();
			//}
		}
		if(Count1%20 == 0)
		{
			MMPF_PIO_GetData(MMP_GPIO63,&stausedamp);
			//if(BlueENpin == 0)
		//	{
			//	BlueENpin = 1;
				//MMPF_PIO_SetData(MMP_GPIO66, 1,MMP_TRUE);
				//MMPF_PIO_PadConfig(MMP_GPIO66, PAD_OUT_DRIVING(0), MMP_TRUE);
				//MMPF_PIO_EnableOutputMode(MMP_GPIO66, MMP_TRUE, MMP_TRUE); // lyj 20190112 BT En pin
				//MMPF_PIO_SetData(MMP_GPIO66, 1,MMP_TRUE); // lyj 20190423
			//	PowerOnReadTheE2promToAM_FM();
		//	}
				#if 0
				if(PowerOnVoiceMode ==1)
				{
					PowerOnVoiceMode =0;
					converseChannel(0xF3);
					printc("~~~~~~@_@~~~~3333~~~~~~~~~\n");
				}
				else if(PowerOnVoiceMode ==2)
				{
					PowerOnVoiceMode =0;
					converseChannel(0xF3);
				}
				else if(PowerOnVoiceMode ==2)
				{
					PowerOnVoiceMode =0;
					converseChannel(0xF3);
				}
				#endif
				switch(PowerOnVoiceMode)
				{
					case 6:
						PowerOnVoiceMode =0;
						IIC_write_data(0x04,0x02,1);//U盘
						break;
					case 5:
						PowerOnVoiceMode =0;
						converseChannel(0xF5);
						printc("~~~~~~@_@~~~~3335~~~~~~~~~\n");
						break;
				
					case 4:

						PowerOnVoiceMode =0;
						converseChannel(0xF4);
						printc("~~~~~~@_@~~~~3334~~~~~~~~~\n");

						break;
					case 3:
						PowerOnVoiceMode =0;
						converseChannel(0xF2);
						printc("~~~~~~@_@~~~~3331~~~~~~~~~\n");

						break;

						case 2:
							PowerOnVoiceMode =0;
							converseChannel(0xF1);
							printc("~~~~~~@_@~~~~3332~~~~~~~~~\n");
						break;

						case 1:
							PowerOnVoiceMode =0;
							converseChannel(0xF3);
							printc("~~~~~~@_@~~~~3333~~~~~~~~~\n");
						break;

						default:
							break;
				}

				
			#if 0
			if(/*Oldstausedamp != stausedamp*/BlueStause == 1)
			{

				DrawBluetoothLinkIcon(1);// lyj 20181025
			}
			else if(/*Oldstausedamp == stausedamp*/BlueStause == 0)
			{
				DrawBluetoothLinkIcon(0);// lyj 20181025
			}
			#endif

			#if 1
			if(stausedamp == 0)
			{
				//连上蓝牙
				//BlueStauseEx = 1;
				DrawBluetoothLinkIcon(1);// lyj 20181025
			}
			else if(stausedamp == 1)
			{
				//断开蓝牙
				//BlueStauseEx = 0;
				DrawBluetoothLinkIcon(0);// lyj 20181025
			}
			#endif
			//Oldstausedamp = stausedamp;
			//printc("~~~aa~~~led on~stausedamp== %d~~~~~~~\r\n",stausedamp);
		}
		//printc("~~~aa~~~led on~stausedamp== %d~~~~~~~\r\n",stausedamp);
		/*if(Count1 %30 && BlueENpin ==1)
		{
			BlueENpin = 2;
			PowerOnReadTheE2promToAM_FM();

		}// lyj 20190114*/
		

	}
	#endif
	//=========================
//----------------------------------------------------
#if (MENU_BLUETOOTH_PROGRESS_BAR)


		if(/*Main_Get_Page()!= RGB_FLAG&&*/Main_Get_Page()!= PREVIEW_FLAG)
		{
		//AHC_RTC_GetTime(&CurRtctime);
		//wSecond=CurRtctime.uwSecond;
	//	printc("---------time=%d---------\r\n",wSecond);
		/*
		AHC_OSDSetFont(wDispID, &GUI_Font20B_1);
		AHC_OSDDrawDec( wDispID,wSecond, 240,40, 2);
		DrawAudPb_ProgressBar_Menu(wDispID, wSecond, 60, AHC_FALSE);
*/

		{
			 pf_General_EnGet(COMMON_KEY_B_VOL, &level);
			// printc("~~~~~~~level = %d~~~~~~~~\r\n",level);sfefaefaef
				if(level<820)
					{
						level1=(level*100/791)%10;//791  830
						level=level*10/791;	
					}
					else if(level<1300)
					{
						//level=level+30;
						level1=(level*100/758  )%10;// 758  831
						level=level*10/758  ;

					}
		}

#if 1   // lyj 20180604
		sprintf(szv, "%d.%dV", /*(INT32)wSecond*/level,level1);
	    tmpRECT1.uiLeft     = 415;
	    tmpRECT1.uiTop     = 55; 
	    tmpRECT1.uiWidth  = 60;
	    tmpRECT1.uiHeight = 35;
	
	switch(Main_Get_Page())
		{
		case MENU_MAIN_FLAG :        Color_1 = OSD_COLOR_MAIN_PAGE;  break;
		case BLUETOOH_FLAG  :         Color_1 = OSD_COLOR_BLUETOOTH;  break;
		case RADIO_FM_FLAG :          Color_1 = OSD_COLOR_FM;  break;
		case RADIO_AM_FLAG :         Color_1 = OSD_COLOR_AM;  break;
		case VOLUME_FLAG :             Color_1 = OSD_COLOR_VOL;  break;
		case LIGHT_BAR_FLAG :       Color_1 = OSD_COLOR_BAR;  break;
		case AUX_FLAG :                 Color_1 = OSD_COLOR_AUX;  break;
		case USB_FLAG : 			 Color_1 = OSD_COLOR_USB;  break;
		case RGB_FLAG :  			Color_1 = OSD_COLOR_RGB;  break; // lyj 20191018
		case BRIGHTNESS_FLAG :     Color_1 = OSD_COLOR_BRIGHTNESS;  break;
	}
	
	AHC_OSDSetColor(wDispID, Color_1); 
	AHC_OSDSetFont(wDispID, &GUI_Font20_1);
  	 AHC_OSDDrawFillRect(wDispID, tmpRECT1.uiLeft, tmpRECT1.uiTop, 475, 90);
	OSD_ShowString( wDispID,szv, NULL, tmpRECT1, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);

#endif
	}


#endif


//===============================lyj==================
if(app_flag == 0)
{

	if(recv_data[1] == 0x55 &&  recv_data[3] == 0xF3 && recv_data[5] == 0x0D  && lamp_mode ==1)
	{

		//speed_Select();
		#if 1
		switch(recv_data[4])
		{
		case 1:
			twikle_speed(20);//	20
			break;

		case 2:
			twikle_speed(16);// 16
			break;

		case 3:
			twikle_speed(13);// 13
			break;

		case 4:
			twikle_speed(10);// 10
			break;

		case 5:
			twikle_speed(8);// 8
			break;
		case 6:
			twikle_speed(6);// 6
			break;

		case 7:
			twikle_speed(4);// 4
			break;

		case 8:
			twikle_speed(2);// 2
			break;
		}

		#endif

		//twikle_speed(2);
	

	}

	if(recv_data[1] == 0x55 &&  recv_data[3] == 0x0F && recv_data[4] == 0x02)
	{
		lamp_mode = 1;
	}
	else if(recv_data[1] == 0x55 &&  recv_data[3] == 0x0F && recv_data[4] != 0x02 /*&& recv_data[8] == 0 && recv_data[13] == 0*/)
	{
		lamp_mode = 0;
	}

	if(lamp_mode == 1)
	{
		twikle_speed(5);
	}

// breathe lamp void breathe_lamp(void)
	if((recv_data[1] == 0x55 &&  recv_data[3] == 0x0F && recv_data[4] == 0x03))
	{


		if(breath_flag == 0)
		{

			i++;

			breathe_lamp((MMP_ULONG)i);
			
			if(i == 101)
			{
				i = 0;
				breath_flag = 1;
				printc("~~~~aaaaaaaaaa~~~~i = %d~~\r\n",i);
			}
		
				
				
			
				
		}
		
		
		if(breath_flag == 1)
		{

			jp--;

			breathe_lamp((MMP_ULONG)jp);
			if(jp== 0)
			{
				jp = 101;
				breath_flag = 0;
			}
		
				
							
		}

		
	}

}
#if 0
else if(flag_twinkle == 1)
{
	if(!SelfAllFlag.Speedmode)
			twikle_speed(5);
		else if(SelfAllFlag.Speedmode ==1)
			twikle_speed(10);
		else
			twikle_speed(2); // lyj 20191018
} // lyj 20181022
#endif

#if 1
if(volume_M == 1)
{

	if(exit_flag == 0)
	{
		if( i != 0)
		{
			i = 0;
			exit_flag = 1;
		}
	}
	if(i == 30)
	{
		i = 0;
		auto_exit(wDispID);
		volume_M = 0;
		//volume_B  = 0;
		sub_flag_ex = 0;
	}
	i++;

}
#endif
if(switch_search == 1)
{

	if(Menu_Get_Page() == 1)
	{
		fm_si47xx_auto_search(Tfreq*10);

		if(Valid)
		{
			static int countFm = 0;// lyj 20190610
			radio[Radiocnt++] = Freq;
			MenuSettingConfig()->uiTimeZone= Radiocnt;
			AutoDisplayTheFreq(Freq,wDispID,countFm); // lyj 20190610
			/*lyj 20190610 define */
			if(++countFm >=10)
				countFm = 0;// lyj 20190610
			
		}

		//if(volume_M == 0)
		//{
			if(switch_search == 1)
			Draw_FM_AM_icon_Ex(Tfreq,wDispID);
	//	}
		
		Tfreq +=20; // 10

		if(Tfreq == 10790)
		{
			switch_search = 0;
			//Radiocnt = 0;

			if(radio[0] != 0)
			{
				extern MMP_USHORT FMCurruntFreq;// lyj 20190923
				si47xxFMRX_tune(radio[0]);
				Draw_FM_AM_icon_Ex(radio[0],wDispID);
				FMCurruntFreq = radio[0];
				//return_flag =0;
				bl_flag = 1;
				SetKeyPadEvent(BUTTON_RIGHT_REL); // lyj 20190711
				//return_flag =1;
			}
		}
		
	}
	else if(Menu_Get_Page() == 2)
	{
		fm_si47xx_auto_search(Tfreq*10);

		if(Valid)
		{
			static int countAm = 0;// lyj 20190610
			radio_am[RadiocntAm++] = Freq;
			MenuSettingConfig()->uiLCDBrightness= RadiocntAm;
			AutoDisplayTheFreq(Freq,wDispID,countAm); // lyj 20190610
			if(++countAm >=10)
				countAm = 0;// lyj 20190610
		}

		//if(volume_M == 0)
		//{
			if(switch_search == 1)
			Draw_AM_icon_Ex(Tfreq,wDispID);
		//}
		
		//Tfreq +=9;
		Tfreq +=10;//lyj 20190216
		if(Tfreq == 1710)
		{
			switch_search = 0;
			//Radiocnt = 0;

			if(radio_am[0] != 0)
			{
				extern MMP_USHORT AMCurruntFreq;
				si47xxAMRX_tune(radio_am[0]);
				Draw_AM_icon_Ex(Tfreq,wDispID);
				AMCurruntFreq = radio_am[0];
				bl_flag = 1;
				SetKeyPadEvent(BUTTON_RIGHT_REL); // lyj 20190711
			}
		}


	}


}

//==================================end===============

//	AHC_Brightness_Alert(30000, ulTimess,1);
//-----------------------------------------------
#if (TASK_MONITOR_ENABLE)
        MMPF_OS_GetTime(&gsTaskMonitorAHCWorkStack.ulExecTime);
        gsTaskMonitorAHCWorkStack.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_RUNNING;    
#endif

#if 1


		
		

		if(acc_detCount < 6) // 5
		{
			acc_detCount++;
			MMPF_PIO_GetData(MMP_GPIO25, &acc_det_old);

		}
	else
	//MMPF_PIO_GetData(MMP_GPIO25, &acc_det);
	//if(acc_detCount >=5) // 5 lyj 20190627
	{
		MMPF_PIO_GetData(MMP_GPIO25, &acc_det);	
		if(acc_det == 0)
			acc_det_old =0;
	}
	
	if(acc_det && acc_det_old != 1)
	{
		// power_off massenge
		printc("~~MMP_GPIO63~~~acc_det = %d~~~~~~\r\n",acc_det);
		AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_POWER_LPRESS, 0);
		
	}
#endif
			 #if 0
			 if(SendMessageMCU &&MCU_message < 19)
			 {
			 		MCU_message++;
			 	
				if(MCU_message == 13)
					uart_to_mcu();
				//main
				if(MCU_message == 14)
			 		volume_conver_size(0xc0,mainVflag_count);
				//sub
				if(MCU_message == 15)
				{
					volume_conver_size(0xc7,sub_count_ex);
					volume_conver_size(0xc8,sub_count_ex);
				}
				// fornt
				if(MCU_message == 16)
				{
					volume_conver_size(0xc1,MenuSettingConfig()->uiVolume);
					volume_conver_size(0xc2,MenuSettingConfig()->uiVolume);
				}
				//rear
				if(MCU_message == 17)
				{
					volume_conver_size(0xc3,MenuSettingConfig()->uiMOVPowerOffTime);
					volume_conver_size(0xc4,MenuSettingConfig()->uiMOVPowerOffTime);
				}
				//output
				if(MCU_message == 18)
				{
					volume_conver_size(0xc5,MenuSettingConfig()->uiEffect);
					volume_conver_size(0xc6,MenuSettingConfig()->uiEffect);
				}
			 }//lyj 20181022
			 #endif
		



	#if 1 //lyj 20181025  add facebook 20190814
        if(AHC_GetWaitIconState()) 
        {
    	    AHC_DrawWaitIcon();  
        }
        
        if(AHC_WMSG_States()) 
        {
            Count++;
            if(AHC_WMSG_IsTimeOut(Count*100)) {
                Count = 0;
            }    
        }
	#endif
#if 0//(GPS_CONNECT_ENABLE) 
		if( uiGetCurrentState() >= UI_CAMERA_MENU_STATE    &&
			uiGetCurrentState() <= UI_CLOCK_SETTING_STATE)
		{
			ulGPSInfoRefreshCnt++;
			if(ulGPSInfoRefreshCnt == 10)
			{
				ulGPSInfoRefreshCnt = 0;
				AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_UPDATE_MESSAGE, 0);
			}
		}
#endif	     

#if defined(AIT_HW_WATCHDOG_ENABLE) && (AIT_HW_WATCHDOG_ENABLE)
        AHC_WD_Kick();
#endif

#if (TASK_MONITOR_ENABLE)
        gsTaskMonitorAHCWorkStack.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_WAITING;        
#endif

		#if (defined(CUS_ADAS_OUTPUT_LOG) && ADAS_OUTPUT_LOG == 1)
		{
            extern MMP_BOOL FlagSetWrAdasLog;
            extern void ADAS_write_log(void);
            if (FlagSetWrAdasLog)
            {
                FlagSetWrAdasLog = MMP_FALSE;
                ADAS_write_log();
            }
		}
		#endif

        AHC_OS_SleepMs(100);
    }
}

#if 0
void ________Uart_Function_________(){ruturn;} //dummy
#endif

void UartCmdSetMode( char* szParam )
{
    int iMode;

    sscanfl( szParam, "%x",&iMode);

    if( iMode == 0xFF )
    {
        printc("AHC_MODE_IDLE = 0x00,              \n");
        printc("AHC_MODE_CAPTURE_PREVIEW = 0x10,   \n");
        printc("AHC_MODE_DRAFT_CAPTURE = 0x11,     \n");
        printc("AHC_MODE_STILL_CAPTURE = 0x20,     \n");
        printc("AHC_MODE_C2C_CAPTURE = 0x21,       \n");
        printc("AHC_MODE_SEQUENTIAL_CAPTURE = 0x22,\n");
        printc("AHC_MODE_USB_MASS_STORAGE = 0x30,  \n");
        printc("AHC_MODE_USB_WEBCAM = 0x31,        \n");
        printc("AHC_MODE_PLAYBACK  = 0x40,         \n");
        printc("AHC_MODE_THUMB_BROWSER = 0x50,     \n");
        printc("AHC_MODE_VIDEO_RECORD = 0x60,      \n");
        printc("AHC_MODE_RECORD_PREVIEW = 0x62,    \n");
        printc("AHC_MODE_CALIBRATION = 0xF0,       \n");
        printc("AHC_MODE_MAX = 0xFF                \n");
    }
    else
    {
        AHC_SetMode( iMode );
    }  
}

void UartCmdSetUiMode( char* szParam )
{
    int uiMode;

    sscanfl( szParam, "%x",&uiMode);

    if( uiMode == 0xFF )
    {
        printc("UI_CAMERA_STATE    		= 0x00, \n");
        printc("UI_VIDEO_STATE     		= 0x01, \n");
        printc("UI_BROWSER_STATE   		= 0x02, \n");
        printc("UI_PLAYBACK_STATE  		= 0x03, \n");
        printc("UI_SLIDESHOW_STATE 		= 0x04, \n");
        printc("UI_MSDC_STATE      		= 0x05, \n");
        //printc("UI_HDMI_STATE      		= 0x06, \n");
        //printc("UI_TVOUT_STATE     		= 0x07, \n");
        printc("UI_CAMERA_MENU_STATE   	= 0x08, \n");
        printc("UI_VIDEO_MENU_STATE    	= 0x09, \n");
        printc("UI_PLAYBACK_MENU_STATE 	= 0x0A, \n");
    }
    else
    {
        StateSwitchMode( uiMode );
    }   
}

void UartCmd_Main(char* szParam)
{
    UINT16 uiItem   = 0;
    UINT16 uiParam1 = 0;
    UINT16 uiParam2 = 0;
    UINT16 uiParam3 = 0;
    UINT16 uiParam4 = 0;
    UINT16 uiParam5 = 0;
    UINT16 uiParam6 = 0;
	
	sscanfl( szParam, "%d %d %d %d %d %d %d",&uiItem, &uiParam1, &uiParam2, &uiParam3, &uiParam4, &uiParam5, &uiParam6 );

    switch( uiItem ) 
    {
        case 0:     
            break;
            
        case 1: 
            break;
                
        case 2: 
            break;
    }
}

UARTCOMMAND sAhcUartCommand[] =
{
    { "mode",          " Mode ",   		   "AHC_SetMode",      			UartCmdSetMode }, 
    { "uimode",        " uiMode ",   	   "UI_SetMode",      			UartCmdSetUiMode },
    { "ucm",           "",                 "Test Uart command",         UartCmd_Main},
    {0,0,0,0}
};

AHC_BOOL MenuFunc_CheckAvailableSpace( UINT8 uiCameraState )
{
    AHC_BOOL bIsAvailableSpace = AHC_TRUE;

    printc("### %s -\r\n", __func__);
        
    switch( uiCameraState )
    {
    case UI_VIDEO_STATE:
        {
            UBYTE bHour = 0, bMin = 0, bSec = 0;

            AHC_VIDEO_AvailableRecTime(&bHour, &bMin, &bSec);
            
            if ((bHour + bMin + bSec) == 0)
            {
                bIsAvailableSpace = AHC_FALSE;
            }
        }
        break;

    case UI_CAMERA_BURST_STATE:
        /*
        // TBD
        if( CameraSettingConfig()->uiRemainingShots < CaptureBurstFunc_GetShotNumber( MenuSettingConfig()->uiBurstShot ) )
        {
            bIsAvailableSpace = AHC_FALSE;
        }
        */
        break;

    default:
        {
            UINT64 freeBytes = 0;
            UINT32 remainCaptureNum = 0;
            
            CaptureFunc_CheckMemSizeAvailable(&freeBytes, &remainCaptureNum);

            if (remainCaptureNum)
            {
                bIsAvailableSpace = AHC_FALSE;
            }
        }
        break;
    }

    return bIsAvailableSpace;
}

