/*===========================================================================
 * Include file 
 *===========================================================================*/ 

#include "Customer_Config.h"
#include "config_fw.h"
#include "vidrec_cfg.h"

#include "dsc_key.h"
#include "AHC_Message.h"
#include "AHC_Menu.h"
#include "AHC_General.h"
#include "AHC_Parameter.h"
#include "AHC_Warningmsg.h"
#include "AHC_Media.h"
#include "AHC_Display.h"
#include "AHC_OS.h"
#include "AHC_General.h"
#include "AHC_Video.h"
#include "AHC_FS.h"
#include "AHC_Config_SDK.h"
#include "AHC_USB.h"
#if (UVC_HOST_VIDEO_ENABLE )
#include "AHC_USBHost.h"
#endif
#include "AHC_GSensor.h"
#include "AHC_PMU.h"
#include "AHC_SARADC.h"
#include "dsc_charger.h"
#include "UartShell.h"
#include "mmpf_timer.h"
#include "mmps_vidplay.h"
#include "mmps_3gprecd.h"
#include "mmph_hif.h"
#include "mmpf_pio.h"
#include "mmpf_i2cm.h"
#include "mmpf_rtc.h"
#include "mmpf_SARADC.h"
#include "mmpf_saradc.h"
#include "MenuSetting.h"
#include "StateHDMIfunc.h"
#include "StateTVfunc.h"
#include "StateVideoFunc.h"
#include "LedControl.h"
#include "PMUCtrl.h"
#include "MediaPlaybackCtrl.h"
#include "lib_retina.h"
#include "Mmpf_system.h" //For task monitor //CarDV...
#include "ColorDefine.h"// liao

//#include "mmp_gpio_inc.h"// long

#if (SUPPORT_GSENSOR) 
#include "GSensor_ctrl.h"
#endif
#if (SUPPORT_IR_CONVERTER)
#include "IR_Ctrl.h"
#endif
#if (SUPPORT_TOUCH_PANEL) 
#include "AHC_TouchPanel.h"
#endif
#if (GPS_CONNECT_ENABLE)
#include "GPS_ctl.h"
#if (GPS_MODULE == GPS_MODULE_NMEA0183) 
#include "gps_nmea0183.h"
#elif (GPS_MODULE == GPS_MODULE_GMC1030) 
#include "GpsRadar_GMC1030.h"
#endif
#endif

// for NETWORK
#if defined(WIFI_PORT) && (WIFI_PORT == 1)
#include "wlan.h"
#include "netapp.h"
#endif

// For CDV
#define	CDV_KEYPAD_FLAG		0x00000001
#define	CDV_TIME_FLAG		0x00000002
#if (UPDATE_UI_USE_MULTI_TASK)
#define	CDV_UI_FLAG			0x00000004
#endif

MMP_ULONG R,G,B;

//==================spi===========
unsigned char length = 0;

//char str[100];
unsigned char str_first1[63]={'\0'};

MMP_UBYTE flag_1=0;
MMP_UBYTE flag_2=0;
AHC_BOOL	flag_add = 0;
AHC_BOOL	flag_sub = 0;

AHC_BOOL     BlueStause = 0;
AHC_BOOL      ReciveDataLength = 0;

unsigned short play_time_HtoL = 0;
unsigned short play_time_T = 0;
//============================

AHC_BOOL Yaokongqi_flag = 0; // lyj 20190423


AHC_BOOL VideoFunc_LockFileEnabled(void);
AHC_BOOL Menu_Get_Page(void);
AHC_BOOL Main_Get_Page(void);
//void Volum_detion(void);
extern int  mainVflag_count;
extern int sub_count_ex;
/*===========================================================================
 * Global varible : ADC Key
 *===========================================================================*/

#if(ADC_KEY_EN==1) 

SADCBUTTON sADCButton[] =
{
#if 1	
    {KEY_PRESS_UP, 		KEY_REL_UP, 	KEY_LPRESS_UP, 		KEY_LREL_UP,		ADC_KEY_VOLT_U, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_U, 		ADC_STATE_ID_D,			0, KEYFLAG_UP,		"ADC_UP"},
    {KEY_PRESS_DOWN, 	KEY_REL_DOWN, 	KEY_LPRESS_DOWN, 	KEY_LREL_DOWN ,		ADC_KEY_VOLT_D, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_D, 		ADC_STATE_ID_U,			0, KEYFLAG_DOWN,	"ADC_DOWN"},
    {KEY_PRESS_LEFT, 	KEY_REL_LEFT, 	KEY_LPRESS_LEFT, 	KEY_LREL_LEFT ,		ADC_KEY_VOLT_L, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_L, 		ADC_STATE_ID_R,			0, KEYFLAG_LEFT,	"ADC_LEFT"},
    {KEY_PRESS_RIGHT, 	KEY_REL_RIGHT, 	KEY_LPRESS_RIGHT, 	KEY_LREL_RIGHT,		ADC_KEY_VOLT_R, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_R, 		ADC_STATE_ID_L,			0, KEYFLAG_RIGHT,	"ADC_RIGHT"},
    {KEY_PRESS_OK, 		KEY_REL_OK, 	KEY_LPRESS_OK, 		KEY_LREL_OK, 		ADC_KEY_VOLT_ENTER, 	ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_ENTER,	ADC_STATE_ID_INVALID, 	0, KEYFLAG_OK,		"ADC_ENTER"},
	#if (MORE_ADC_KEY==1)
    {KEY_PRESS_REC, 	KEY_REL_REC, 	KEY_LPRESS_REC, 	KEY_LREL_REC,		ADC_KEY_VOLT_REC, 		ADC_KEY_VOLT_MARGIN_REC, ADC_STATE_ID_REC, 		ADC_STATE_ID_INVALID,	0, KEYFLAG_REC,		"ADC_REC"},
    {KEY_PRESS_MENU, 	KEY_REL_MENU, 	KEY_LPRESS_MENU, 	KEY_LREL_MENU,		ADC_KEY_VOLT_MENU, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_MENU, 	ADC_STATE_ID_INVALID,	0, KEYFLAG_MENU,	"ADC_MENU"},
    {KEY_PRESS_PLAY, 	KEY_REL_PLAY, 	KEY_LPRESS_PLAY, 	KEY_LREL_PLAY,		ADC_KEY_VOLT_PLAY, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_PLAY, 	ADC_STATE_ID_INVALID,	0, KEYFLAG_PLAY,	"ADC_PLAY"},
    {KEY_PRESS_MODE, 	KEY_REL_MODE, 	KEY_LPRESS_MODE, 	KEY_LREL_MODE,		ADC_KEY_VOLT_MODE, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_MODE, 	ADC_STATE_ID_INVALID,	0, KEYFLAG_MODE,	"ADC_MODE"},
    {KEY_PRESS_SOS, 	KEY_REL_SOS, 	KEY_LPRESS_SOS, 	KEY_LREL_SOS, 		ADC_KEY_VOLT_SOS, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_SOS,		ADC_STATE_ID_INVALID, 	0, KEYFLAG_SOS,		"ADC_SOS"},
    {KEY_PRESS_MUTE, 	KEY_REL_MUTE, 	KEY_LPRESS_MUTE, 	KEY_LREL_MUTE, 		ADC_KEY_VOLT_MUTE, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_MUTE,		ADC_STATE_ID_INVALID, 	0, KEYFLAG_MUTE,	"ADC_MUTE"},
    {KEY_PRESS_CAPTURE, KEY_REL_CAPTURE,KEY_LPRESS_CAPTURE, KEY_LREL_CAPTURE, 	ADC_KEY_VOLT_CAPTURE, 	ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_CAPTURE,	ADC_STATE_ID_INVALID, 	0, KEYFLAG_CAPTURE,	"ADC_CAPTURE"},
    {KEY_PRESS_FUNC1, 	KEY_REL_FUNC1, 	KEY_LPRESS_FUNC1, 	KEY_LREL_FUNC1, 	ADC_KEY_VOLT_FUNC1, 	ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_FUNC1,	ADC_STATE_ID_INVALID, 	0, KEYFLAG_FUNC1,	"ADC_FUNC1"},
    {KEY_PRESS_FUNC2, 	KEY_REL_FUNC2, 	KEY_LPRESS_FUNC2, 	KEY_LREL_FUNC2, 	ADC_KEY_VOLT_FUNC2, 	ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_FUNC2,	ADC_STATE_ID_INVALID, 	0, KEYFLAG_FUNC2,	"ADC_FUNC2"},
    {KEY_PRESS_FUNC3, 	KEY_REL_FUNC3, 	KEY_LPRESS_FUNC3, 	KEY_LREL_FUNC3, 	ADC_KEY_VOLT_FUNC3, 	ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_FUNC2,	ADC_STATE_ID_INVALID, 	0, KEYFLAG_FUNC3,	"ADC_FUNC3"},    
	#endif
#else //For BiTeck TV Decoder testing
    {KEY_PRESS_UP, 		KEY_REL_UP, 	KEY_LPRESS_UP, 		KEY_LREL_UP,		ADC_KEY_VOLT_U, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_U, 		ADC_STATE_ID_D,			0, KEYFLAG_UP,		"ADC_UP"},
    {KEY_PRESS_DOWN, 	KEY_REL_DOWN, 	KEY_LPRESS_DOWN, 	KEY_LREL_DOWN ,		ADC_KEY_VOLT_D, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_D, 		ADC_STATE_ID_U,			0, KEYFLAG_DOWN,	"ADC_DOWN"},
    {KEY_PRESS_LEFT, 	KEY_REL_LEFT, 	KEY_LPRESS_LEFT, 	KEY_LREL_LEFT ,		ADC_KEY_VOLT_L, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_L, 		ADC_STATE_ID_INVALID,	0, KEYFLAG_LEFT,	"ADC_LEFT"},
    {KEY_PRESS_RIGHT, 	KEY_REL_RIGHT, 	KEY_LPRESS_RIGHT, 	KEY_LREL_RIGHT,		ADC_KEY_VOLT_R, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_R, 		ADC_STATE_ID_INVALID,	0, KEYFLAG_RIGHT,	"ADC_RIGHT"},
    {KEY_PRESS_OK, 		KEY_REL_OK, 	KEY_LPRESS_OK, 		KEY_LREL_OK, 		ADC_KEY_VOLT_ENTER, 	ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_ENTER,	ADC_STATE_ID_INVALID, 	0, KEYFLAG_OK,		"ADC_ENTER"},
    {KEY_PRESS_REC, 	KEY_REL_REC, 	KEY_LPRESS_REC, 	KEY_LREL_REC,		ADC_KEY_VOLT_REC, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_REC, 		ADC_STATE_ID_REC,	    0, KEYFLAG_REC,		"ADC_REC"},
    {KEY_PRESS_MENU, 	KEY_REL_MENU, 	KEY_LPRESS_MENU, 	KEY_LREL_MENU,		ADC_KEY_VOLT_MENU, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_MENU, 	ADC_STATE_ID_MENU,	    0, KEYFLAG_MENU,	"ADC_MENU"},
    {KEY_PRESS_PLAY, 	KEY_REL_PLAY, 	KEY_LPRESS_PLAY, 	KEY_LREL_PLAY,		ADC_KEY_VOLT_PLAY, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_PLAY, 	ADC_STATE_ID_INVALID,	0, KEYFLAG_PLAY,	"ADC_PLAY"},
    {KEY_PRESS_MODE, 	KEY_REL_MODE, 	KEY_LPRESS_MODE, 	KEY_LREL_MODE,		ADC_KEY_VOLT_MODE, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_MODE, 	ADC_STATE_ID_MODE,	    0, KEYFLAG_MODE,	"ADC_MODE"},
    {KEY_PRESS_SOS, 	KEY_REL_SOS, 	KEY_LPRESS_SOS, 	KEY_LREL_SOS, 		ADC_KEY_VOLT_SOS, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_SOS,		ADC_STATE_ID_INVALID, 	0, KEYFLAG_SOS,		"ADC_SOS"},
    {KEY_PRESS_MUTE, 	KEY_REL_MUTE, 	KEY_LPRESS_MUTE, 	KEY_LREL_MUTE, 		ADC_KEY_VOLT_MUTE, 		ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_MUTE,		ADC_STATE_ID_INVALID, 	0, KEYFLAG_MUTE,	"ADC_MUTE"},
    {KEY_PRESS_CAPTURE, KEY_REL_CAPTURE,KEY_LPRESS_CAPTURE, KEY_LREL_CAPTURE, 	ADC_KEY_VOLT_CAPTURE, 	ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_CAPTURE,	ADC_STATE_ID_INVALID, 	0, KEYFLAG_CAPTURE,	"ADC_CAPTURE"},
    {KEY_PRESS_FUNC1, 	KEY_REL_FUNC1, 	KEY_LPRESS_FUNC1, 	KEY_LREL_FUNC1, 	ADC_KEY_VOLT_FUNC1, 	ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_FUNC1,	ADC_STATE_ID_INVALID, 	0, KEYFLAG_FUNC1,	"ADC_FUNC1"},
    {KEY_PRESS_FUNC2, 	KEY_REL_FUNC2, 	KEY_LPRESS_FUNC2, 	KEY_LREL_FUNC2, 	ADC_KEY_VOLT_FUNC2, 	ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_FUNC2,	ADC_STATE_ID_INVALID, 	0, KEYFLAG_FUNC2,	"ADC_FUNC2"},
    {KEY_PRESS_FUNC3, 	KEY_REL_FUNC3, 	KEY_LPRESS_FUNC3, 	KEY_LREL_FUNC3, 	ADC_KEY_VOLT_FUNC3, 	ADC_KEY_VOLT_MARGIN, ADC_STATE_ID_FUNC2,	ADC_STATE_ID_INVALID, 	0, KEYFLAG_FUNC3,	"ADC_FUNC3"},    
#endif
};

static MMP_ULONG	gNumOfADCKey 	    = sizeof(sADCButton) / sizeof(SADCBUTTON);
static MMP_UBYTE 	gADCKeyStatus 	    = ADCPRESS_MAX;
static MMP_USHORT 	gADCKeyPressCnt     = 0;
static MMP_UBYTE	gADCKeyLongPress 	= MMP_FALSE;
#endif		// #if(ADC_KEY_EN==1) 
#if(ENCODE_GPIO_ISR)
MMP_UBYTE Quiescent_State=2;
#endif
/*===========================================================================
 * Global varible : Battery Measure
 *===========================================================================*/

static MMP_ULONG 	uBatteryDetectCounter 	= 0;
static MMP_ULONG 	uVoltageLevelSum	   	= 0; 
static MMP_UBYTE 	uSkipDetectNum   		= (0xFF - 0x04); 

/*===========================================================================
 * Global variable : IR Key
 *===========================================================================*/

#if(IR_KEY_EN==1) 
#ifndef	_OEM_IR_KEY_
/* In case OEM_IR_KEY enabled in Config_xxx.h, the sIRButton is defined in Oem_Key_xxx.c
 */
SIRBUTTON sIRButton[] =
{
//   iPressId            iReleaseId          iLongPressId        iLongReleaseId      ulKeyValue          ubKeyStateId            ubKeyPairId    ubKeyCount ubPressStatus ubkeyname
    {KEY_PRESS_UP, 		KEY_REL_UP, 		KEY_LPRESS_UP, 		KEY_LREL_UP,		IR_VALUE_U, 		IR_STATE_ID_U, 			IR_STATE_ID_D,			0,			0,	"IR_UP"},
    {KEY_PRESS_DOWN, 	KEY_REL_DOWN, 		KEY_LPRESS_DOWN, 	KEY_LREL_DOWN,		IR_VALUE_D, 		IR_STATE_ID_D, 			IR_STATE_ID_U,			0,			0,	"IR_DOWN"},
    {KEY_PRESS_LEFT, 	KEY_REL_LEFT, 		KEY_LPRESS_LEFT, 	KEY_LREL_LEFT,		IR_VALUE_L, 		IR_STATE_ID_L, 			IR_STATE_ID_R,			0,			0,	"IR_LEFT"},
    {KEY_PRESS_RIGHT, 	KEY_REL_RIGHT, 		KEY_LPRESS_RIGHT, 	KEY_LREL_RIGHT,		IR_VALUE_R, 		IR_STATE_ID_R, 			IR_STATE_ID_L,			0,			0,	"IR_RIGHT"},
    {KEY_PRESS_OK, 		KEY_REL_OK, 		KEY_LPRESS_OK, 		KEY_LREL_OK, 		IR_VALUE_ENTER, 	IR_STATE_ID_ENTER,		IR_STATE_ID_INVALID, 	0,			0,	"IR_ENTER"},
    {KEY_PRESS_REC, 	KEY_REL_REC, 		KEY_LPRESS_REC, 	KEY_LREL_REC,		IR_VALUE_REC, 		IR_STATE_ID_REC, 		IR_STATE_ID_INVALID,	0,			0,	"IR_REC"},
    {KEY_PRESS_MENU, 	KEY_REL_MENU, 		KEY_LPRESS_MENU, 	KEY_LREL_MENU,		IR_VALUE_MENU, 		IR_STATE_ID_MENU,		IR_STATE_ID_INVALID,	0,			0,	"IR_MENU"},
    {KEY_PRESS_PLAY, 	KEY_REL_PLAY, 		KEY_LPRESS_PLAY, 	KEY_LREL_PLAY,		IR_VALUE_PLAY, 		IR_STATE_ID_PLAY,		IR_STATE_ID_INVALID,	0,			0,	"IR_PLAY"},
    {KEY_PRESS_MODE, 	KEY_REL_MODE, 		KEY_LPRESS_MODE, 	KEY_LREL_MODE,		IR_VALUE_MODE, 		IR_STATE_ID_MODE, 		IR_STATE_ID_INVALID,	0,			0,	"IR_MODE"},
    {KEY_PRESS_SOS, 	KEY_REL_SOS, 		KEY_LPRESS_SOS, 	KEY_LREL_SOS, 		IR_VALUE_SOS, 		IR_STATE_ID_SOS,		IR_STATE_ID_INVALID, 	0,			0,	"IR_SOS"},
    {KEY_PRESS_MUTE, 	KEY_REL_MUTE, 		KEY_LPRESS_MUTE, 	KEY_LREL_MUTE, 		IR_VALUE_MUTE,		IR_STATE_ID_MUTE,		IR_STATE_ID_INVALID, 	0,			0,	"IR_MUTE"},
	{KEY_PRESS_CAPTURE, KEY_REL_CAPTURE, 	KEY_LPRESS_CAPTURE, KEY_LREL_CAPTURE, 	IR_VALUE_CAPTURE,	IR_STATE_ID_CAPTURE,	IR_STATE_ID_INVALID, 	0,			0,	"IR_CAPTURE"},
	{KEY_PRESS_TELE, 	KEY_REL_TELE, 		KEY_LPRESS_TELE, 	KEY_LREL_TELE, 		IR_VALUE_TELE,		IR_STATE_ID_TELE,		IR_STATE_ID_INVALID, 	0,			0,	"IR_ZOOMIN"},
	{KEY_PRESS_WIDE, 	KEY_REL_WIDE, 		KEY_LPRESS_WIDE, 	KEY_LREL_WIDE, 		IR_VALUE_WIDE,		IR_STATE_ID_WIDE,		IR_STATE_ID_INVALID, 	0,			0,	"IR_ZOOMOUT"},
	{KEY_PRESS_FUNC1, 	KEY_REL_FUNC1, 		KEY_LPRESS_FUNC1, 	KEY_LREL_FUNC1, 	IR_VALUE_FUNC1,		IR_STATE_ID_FUNC1,		IR_STATE_ID_INVALID, 	0,			0,	"IR_FUNC1"},
	{KEY_PRESS_FUNC2, 	KEY_REL_FUNC2, 		KEY_LPRESS_FUNC2, 	KEY_LREL_FUNC2, 	IR_VALUE_FUNC2,		IR_STATE_ID_FUNC2,		IR_STATE_ID_INVALID, 	0,			0,	"IR_FUNC2"},
	{KEY_PRESS_FUNC3, 	KEY_REL_FUNC3, 		KEY_LPRESS_FUNC3, 	KEY_LREL_FUNC3, 	IR_VALUE_FUNC3,		IR_STATE_ID_FUNC3,		IR_STATE_ID_INVALID, 	0,			0,	"IR_FUNC3"},
	{KEY_PRESS_POWER, 	KEY_REL_POWER,		KEY_LPRESS_POWER, 	KEY_LREL_POWER, 	IR_VALUE_POWER,		IR_STATE_ID_POWER,		IR_STATE_ID_INVALID, 	0,			0,	"IR_POWER"},
};

#define	NUM_IR_KEY	(sizeof(sIRButton) / sizeof(SIRBUTTON))

#else
// Declare in Oem_Key_xxx.c
extern SIRBUTTON sIRButton[];
extern int			_irNumKey;
#define	NUM_IR_KEY	_irNumKey
#endif
#endif

/*===========================================================================
 * Global variable : GPIO Key/Device
 *===========================================================================*/

#define	INVALID_KV		        	(0xFF)

SGPIOBUTTON sButton[] =
{
//	iPressId			iReleaseId			iLongPressId		iLongReleaseId		piopin		ulKeyLastTime	ulContinuousTime		ulPressLevel			ulKeyLastValue			ulKeyCount	ubKeyStateId	ubKeyPairId			ulKeyFlag			ubkeyname[16]	ubLPRepeatTime															ubkeyname	ubLPRepeatEn
//																										ulDebounceTime																			ubLongPress																ubLPRepeatEn
    {KEY_PRESS_REC, 	KEY_REL_REC, 		KEY_LPRESS_REC, 	KEY_LREL_REC,		KEY_GPIO_REC, 		0, 30, KEY_LPRESS_TIME_REC, 	KEY_PRESSLEVEL_REC,  	KEY_PRESSLEVEL_REC^1,  		0, 0, 	GPIOID_REC,   	KEY_GPIO_ID_INVALID, KEYFLAG_REC, 		"REC",		0,	0},
    {KEY_PRESS_MENU, 	KEY_REL_MENU, 		KEY_LPRESS_MENU, 	KEY_LREL_MENU,		KEY_GPIO_MENU, 		0, 30, KEY_LPRESS_TIME_MENU, 	KEY_PRESSLEVEL_MENU, 	KEY_PRESSLEVEL_MENU^1, 		0, 0, 	GPIOID_MENU,  	KEY_GPIO_ID_INVALID, KEYFLAG_MENU, 		"MENU",		0,	0},
    {KEY_PRESS_PLAY, 	KEY_REL_PLAY, 		KEY_LPRESS_PLAY, 	KEY_LREL_PLAY,		KEY_GPIO_PLAY, 		0, 30, KEY_LPRESS_TIME_PLAY, 	KEY_PRESSLEVEL_PLAY, 	KEY_PRESSLEVEL_PLAY^1, 		0, 0, 	GPIOID_PLAY,  	KEY_GPIO_ID_INVALID, KEYFLAG_PLAY,		"PLAY",		0,	0},
    {KEY_PRESS_MODE, 	KEY_REL_MODE, 		KEY_LPRESS_MODE, 	KEY_LREL_MODE,		KEY_GPIO_MODE, 		0, 30, KEY_LPRESS_TIME_MODE, 	KEY_PRESSLEVEL_MODE, 	KEY_PRESSLEVEL_MODE^1, 		0, 0, 	GPIOID_MODE,  	KEY_GPIO_ID_INVALID, KEYFLAG_MODE,		"MODE",		0,	0},
    {KEY_PRESS_POWER, 	KEY_REL_POWER, 		KEY_LPRESS_POWER, 	KEY_LREL_POWER, 	KEY_GPIO_POWER, 	0, 30, KEY_LPRESS_TIME_POWER, 	KEY_PRESSLEVEL_POWER, 	KEY_PRESSLEVEL_POWER^1, 	0, 0,	GPIOID_POWER, 	KEY_GPIO_ID_INVALID, KEYFLAG_POWER,		"POWER",	1,	1},
//    {KEY_PRESS_OK, 		KEY_REL_OK, 		KEY_LPRESS_OK, 		KEY_LREL_OK,		KEY_GPIO_OK, 		0, 30, KEY_LPRESS_TIME_OK,		KEY_PRESSLEVEL_OK, 		KEY_PRESSLEVEL_OK^1, 		0, 0, 	GPIOID_OK,    	KEY_GPIO_ID_INVALID, KEYFLAG_OK,		"OK",		0,	0},
#if (MORE_GPIO_KEY==1)
    {KEY_PRESS_UP, 		KEY_REL_UP, 		KEY_LPRESS_UP, 		KEY_LREL_UP,		KEY_GPIO_UP, 		0, 30, KEY_LPRESS_TIME_UP,		KEY_PRESSLEVEL_UP, 		KEY_PRESSLEVEL_UP^1, 		0, 0, 	GPIOID_UP,    	GPIOID_DOWN, 	 	 KEYFLAG_UP,		"UP",		1,	1},
    {KEY_PRESS_DOWN, 	KEY_REL_DOWN, 		KEY_LPRESS_DOWN, 	KEY_LREL_DOWN,		KEY_GPIO_DOWN, 		0, 30, KEY_LPRESS_TIME_DOWN, 	KEY_PRESSLEVEL_DOWN, 	KEY_PRESSLEVEL_DOWN^1, 		0, 0, 	GPIOID_DOWN,  	GPIOID_UP,      	 KEYFLAG_DOWN,		"DOWN",		1,	1},
    {KEY_PRESS_LEFT, 	KEY_REL_LEFT, 		KEY_LPRESS_LEFT, 	KEY_LREL_LEFT,		KEY_GPIO_LEFT, 		0, 30, KEY_LPRESS_TIME_LEFT, 	KEY_PRESSLEVEL_LEFT, 	KEY_PRESSLEVEL_LEFT^1, 		0, 0, 	GPIOID_LEFT, 	GPIOID_RIGHT,   	 KEYFLAG_LEFT,		"LEFT",		1,	1},
    {KEY_PRESS_RIGHT, 	KEY_REL_RIGHT, 		KEY_LPRESS_RIGHT, 	KEY_LREL_RIGHT,		KEY_GPIO_RIGHT, 	0, 30, KEY_LPRESS_TIME_RIGHT, 	KEY_PRESSLEVEL_RIGHT, 	KEY_PRESSLEVEL_RIGHT^1, 	0, 0, 	GPIOID_RIGHT, 	GPIOID_LEFT,    	 KEYFLAG_RIGHT,		"RIGHT",	1,	1},
    {KEY_PRESS_OK, 		KEY_REL_OK, 		KEY_LPRESS_OK, 		KEY_LREL_OK,		KEY_GPIO_OK, 		0, 30, KEY_LPRESS_TIME_OK,		KEY_PRESSLEVEL_OK, 		KEY_PRESSLEVEL_OK^1, 		0, 0, 	GPIOID_OK,    	KEY_GPIO_ID_INVALID, KEYFLAG_OK,		"OK",		0,	0},
	{KEY_PRESS_SOS, 	KEY_REL_SOS, 		KEY_LPRESS_SOS, 	KEY_LREL_SOS,		KEY_GPIO_SOS, 		0, 30, KEY_LPRESS_TIME_SOS, 	KEY_PRESSLEVEL_SOS, 	KEY_PRESSLEVEL_SOS^1, 		0, 0, 	GPIOID_SOS, 	KEY_GPIO_ID_INVALID, KEYFLAG_SOS,		"SOS",		0,	0},
	{KEY_PRESS_MUTE, 	KEY_REL_MUTE, 		KEY_LPRESS_MUTE, 	KEY_LREL_MUTE,		KEY_GPIO_MUTE, 		0, 30, KEY_LPRESS_TIME_MUTE, 	KEY_PRESSLEVEL_MUTE, 	KEY_PRESSLEVEL_MUTE^1, 		0, 0, 	GPIOID_MUTE, 	KEY_GPIO_ID_INVALID, KEYFLAG_MUTE,		"MUTE",		0,	0},
    {KEY_PRESS_CAPTURE, KEY_REL_CAPTURE, 	KEY_LPRESS_CAPTURE, KEY_LREL_CAPTURE,	KEY_GPIO_CAPTURE, 	0, 30, KEY_LPRESS_TIME_CAPTURE, KEY_PRESSLEVEL_CAPTURE, KEY_PRESSLEVEL_CAPTURE^1,	0, 0, 	GPIOID_CAPTURE, KEY_GPIO_ID_INVALID, KEYFLAG_CAPTURE,	"CAPTURE",	0,	0},
    {KEY_PRESS_FUNC1, 	KEY_REL_FUNC1, 		KEY_LPRESS_FUNC1, 	KEY_LREL_FUNC1,		KEY_GPIO_FUNC1, 	0, 30, KEY_LPRESS_TIME_FUNC1, 	KEY_PRESSLEVEL_FUNC1, 	KEY_PRESSLEVEL_FUNC1^1, 	0, 0, 	GPIOID_FUNC1, 	KEY_GPIO_ID_INVALID, KEYFLAG_FUNC1,		"FUNC1",	0,	0},
    {KEY_PRESS_FUNC2, 	KEY_REL_FUNC2, 		KEY_LPRESS_FUNC2, 	KEY_LREL_FUNC2,		KEY_GPIO_FUNC2, 	0, 30, KEY_LPRESS_TIME_FUNC2, 	KEY_PRESSLEVEL_FUNC2, 	KEY_PRESSLEVEL_FUNC2^1, 	0, 0, 	GPIOID_FUNC2, 	KEY_GPIO_ID_INVALID, KEYFLAG_FUNC2,		"FUNC2",	0,	0},
    {KEY_PRESS_FUNC3, 	KEY_REL_FUNC3, 		KEY_LPRESS_FUNC3, 	KEY_LREL_FUNC3,		KEY_GPIO_FUNC3, 	0, 30, KEY_LPRESS_TIME_FUNC3, 	KEY_PRESSLEVEL_FUNC3, 	KEY_PRESSLEVEL_FUNC3^1, 	0, 0, 	GPIOID_FUNC3, 	KEY_GPIO_ID_INVALID, KEYFLAG_FUNC3,		"FUNC3",	0,	0},
#endif
#if (SUPPORT_TOUCH_PANEL && defined(TOUCH_PANEL_INT_GPIO))
    #if (DSC_TOUCH_PANEL != TOUCH_PANEL_FT6X36)
	{TOUCH_PANEL_PRESS, TOUCH_PANEL_REL,    TOUCH_PANEL_MOVE,   TOUCH_PANEL_REL, TOUCH_PANEL_INT_GPIO,  0, 120,                  100,   KEY_PRESSLEVEL_TOUCH,   KEY_PRESSLEVEL_TOUCH^1, 	0, 0, 	GPIOID_TOUCH, 	KEY_GPIO_ID_INVALID, KEYFLAG_DUMMY,     "TOUCH",	0,	0}
    #endif
#endif
};

SGPIOBUTTON sDeviceStatus[] =
{
   //                                                                                                       ulContinuousTime   ulKeyLastValue     ubKeyStateId  
   //                                                                                                  ulDebounceTime;      ulPressLevel;      ubLongPress; 
   //iPressId;           iReleaseId;         iLongPressId;  iLongReleaseId; piopin;                 ulKeyLastTime;                          ubKeyCount;             ubKeyPairId;         ulKeyFlag      ubkeyname[16]
#if (DETECT_SD_BY_TIMER==0) 
    {SD_CARD_IN	 	,  	 SD_CARD_OUT, 	   	 KEYPAD_NONE,	KEYPAD_NONE, 	DEVICE_GPIO_SD_PLUG, 	0, 200, 4294967295LL, 	0, INVALID_KV,	0, 0, DEVID_SDMMC, 		KEY_GPIO_ID_INVALID, KEYFLAG_DUMMY, "SDMMC"},
#endif

#ifdef CFG_CUS_BUTTON_LCD_COVER //may be defined in config_xxx.h
    CFG_BUTTON_CUS_LCD_COVER,
#else
    {DEVICE_LCD_OPEN, 	 DEVICE_LCD_COVERED, KEYPAD_NONE, 	KEYPAD_NONE, 	DEVICE_GPIO_LCD_COV, 	0, 200, 1200, 			0, INVALID_KV,  0, 0, DEVID_LCDCOV, 	KEY_GPIO_ID_INVALID, KEYFLAG_DUMMY,	"LCD COVER"},
#endif

    {DEVICE_LCD_INV, 	 DEVICE_LCD_NOR, 	 KEYPAD_NONE, 	KEYPAD_NONE, 	DEVICE_GPIO_LCD_INV, 	0, 200, 1200, 			0, INVALID_KV, 	0, 0, DEVID_LCDINV, 	KEY_GPIO_ID_INVALID, KEYFLAG_DUMMY,	"LCD ROTATE"},
	// All of Devices Initialized, send DEVICES_READY to Turn ON LCD backlight
	// The event sent only once.
	{DEVICES_READY,		 DEVICES_READY,		 KEYPAD_NONE, 	KEYPAD_NONE, 	MMP_GPIO_MAX,		    0, 200, 1200, 			0, INVALID_KV, 	0, 0, DEVID_LCDINV, 	KEY_GPIO_ID_INVALID, KEYFLAG_DUMMY,	"DEVICE READY"},  
	//

    {DC_CABLE_IN, 	 	 DC_CABLE_OUT, 		 KEYPAD_NONE, 	KEYPAD_NONE, 	DEVICE_GPIO_DC_INPUT, 	0, 200, 4294967295LL, 	DEVICE_GPIO_DC_INPUT_LEVEL, !DEVICE_GPIO_DC_INPUT_LEVEL,		0, 0, DEVID_DC_CABLE, 	KEY_GPIO_ID_INVALID, KEYFLAG_DUMMY,	"DC CABLE"}, 

#if defined(DEVICE_GPIO_CUS_SW1) && (DEVICE_GPIO_CUS_SW1 != MMP_GPIO_MAX)
    {CUS_SW1_ON,         CUS_SW1_OFF,        KEYPAD_NONE, 	KEYPAD_NONE, 	DEVICE_GPIO_CUS_SW1, 	0, 200, 4294967295LL, 	DEVICE_GPIO_CUS_SW1_ON, 1, 			0, 0, DEVID_CUS_SW1, 	KEY_GPIO_ID_INVALID, KEYFLAG_DUMMY,	"CUS SW1"}, 
#endif

#if defined(DEVICE_GPIO_CUS_SW2) && (DEVICE_GPIO_CUS_SW2 != MMP_GPIO_MAX)
    {CUS_SW2_ON,         CUS_SW2_OFF,        KEYPAD_NONE, 	KEYPAD_NONE, 	DEVICE_GPIO_CUS_SW2, 	0, 200, 4294967295LL, 	DEVICE_GPIO_CUS_SW2_ON, 1, 			0, 0, DEVID_CUS_SW2, 	KEY_GPIO_ID_INVALID, KEYFLAG_DUMMY,	"CUS SW2"}, 
#endif

};

static MMP_ULONG	gNumOfGPIOKey = sizeof(sButton) / sizeof(SGPIOBUTTON);
static MMP_ULONG	gNumOfDevice  = sizeof(sDeviceStatus) / sizeof(SGPIOBUTTON);
static MMP_ULONG   	gKeyEvent 	  = KEYPAD_NONE;
static MMP_ULONG	gComboKey	  = KEYFLAG_DUMMY;

#if (UVC_HOST_VIDEO_ENABLE )
static AHC_BOOL gbRearState = AHC_FALSE;
#endif

/*===========================================================================
 * Global variable
 *===========================================================================*/ 

#define SPEAKER_OFF_TIME                (5)     // SEC
#define POWER_SETTING_COUNTER_5MIN 		(2380)
#define TICKS_PER_SECOND				(1000) 
#define TICKS_PER_MINUTE				(60 * 1000) 

static MMP_BOOL  	UITaskReady				 = MMP_FALSE;
static MMP_ULONG 	gAutoPowerOffCounter 	 = 0;
static MMP_ULONG 	gVideoPowerOffCounter    = 0; 
static MMP_ULONG 	gLCDPowerSaveCounter  	 = 0;

#if (CHARGE_FULL_NOTIFY)
AHC_BOOL			bChargeFull				 = AHC_FALSE;
#endif

#if (NO_CARD_POWER_OFF_EN)
AHC_BOOL			gbNoCardPowerOff     	 = AHC_FALSE;
#endif

#if (SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
AHC_BOOL    ubGsnrPwrOnActStart     = AHC_FALSE;
AHC_BOOL    ubGsnrPwrOnFirstREC     = AHC_TRUE;
#endif

UINT8 ubSDMMCLeftSpaceLevel = 0;

#if (TASK_MONITOR_ENABLE)
MMPF_TASK_MONITOR gsTaskMonitorUIKey;
MMPF_TASK_MONITOR gsTaskMonitorRealIDUIKey;
#endif


AHC_BOOL flag_twinkle = 0;

/*===========================================================================
 * Extern variable
 *===========================================================================*/ 

extern AHC_BOOL 		PowerOff_InProcess; 
extern AHC_BOOL 		bForce_PowerOff;
extern AHC_BOOL			gAHC_InitialDone;  
extern MMPF_OS_FLAGID   CDV_UI_Flag;

#ifdef CFG_LCD_POWER_SAVE_WHEN_MOTION //may be defined in config_xxx.h, typically 10*10000
extern AHC_BOOL			m_ubMotionDtcEn;
#endif
extern AHC_BOOL         gbAhcDbgBrk;

extern MMP_USHORT       gsAhcPrmSensor;
extern MMP_USHORT       gsAhcUsbhSensor;


extern MMP_BYTE 		recv_data[15]; //long 5 -4



/*===========================================================================
 * Local function
 *===========================================================================*/ 

void SetKeyPadEvent(MMP_ULONG keyevent);
void SDMMCTimerOpen(void);

/*===========================================================================
 * Extern function
 *===========================================================================*/ 

extern AHC_BOOL AHC_IsUsbBooting(void);
extern AHC_BOOL AHC_IsDCInBooting(void);
extern UINT8 	AHC_WaitForSystemReady(void);
extern void 	SlideString(void);
extern void 	SlideSubMenu(void);
extern int		RemoveMenu_GsensorSensitivity(void);
#ifdef SLIDE_MENU
extern UINT8	IsSlidingMenu(void);
#endif
extern AHC_BOOL ubUIInitDone(void);
extern MMP_BOOL EDOGCtrl_IsInitial(void);
extern void EDOGCtrl_RestoesPOItoFile_Manual(void);
#if (EDOG_ENABLE)
extern void EDOGCtrl_Handler(MMP_HANDLE *h);

extern MMP_BOOL EDOGCtrl_SetSpeedLvl(MMP_ULONG);
extern MMP_BOOL EDOGCtrl_SetSpeedDist(MMP_ULONG);

extern MMP_BOOL EDOGCtrl_SetCheckAngleEn(MMP_BOOL);
extern MMP_BOOL EDOGCtrl_SetSearchAngleErr(MMP_UBYTE);
extern MMP_BOOL EDOGCtrl_SetSearchAngle(MMP_USHORT);
extern MMP_BOOL EDOGCtrl_SetSearchXAngle(MMP_USHORT);
extern MMP_BOOL EDOGCtrl_SetPoiDataType(MMP_UBYTE);
extern MMP_BOOL EDOGCtrl_EnAddSpeedCamPOI(MMP_UBYTE);
#endif
void DrawStateVideoRecUpdate(UINT32 ubEvent);

/*===========================================================================
 * Main body
 *===========================================================================*/ 

#ifdef CFG_KEY_DO_COVER_ACTION //may be defined in config_xxx.h
void doCoverAction(SGPIOBUTTON *sb, MMP_UBYTE kv, MMP_ULONG tm)
{
	if (kv == sb->ulPressLevel)
		SetKeyPadEvent(sb->iPressId);	// Close Cover
	else 
		SetKeyPadEvent(sb->iReleaseId);	// Open Cover

	printc("Cover %s\r\n", (kv == sb->ulPressLevel)? "Closed" : "Opened"); 
	sb->ulKeyLastValue = kv;
	sb->ulKeyLastTime  = tm;
}
#endif

#if 0
void _____ComboKey_Function_________(){ruturn;} //dummy
#endif

void SetComboKey(int key)
{

	gComboKey |= key;

}

void ClearComboKey(int key)
{
	gComboKey &= ~(key);
}

void PollingComboKey(void)
{
#if (COMBO_KEY_EN==1)

	if(gComboKey == KEYFLAG_DUMMY)
		return;

	switch(gComboKey)
	{
		case COMBO_KEY_FLAGSET1:
			printc(">>>>COMBO_KEY_FLAGSET1\r\n");
			if(COMBO_KEY_FLAGSET1!=KEYFLAG_DUMMY) 
				AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_COMBO_SET1, 0);

		break;	
		case COMBO_KEY_FLAGSET2:
			printc(">>>>COMBO_KEY_FLAGSET2\r\n");
			if(COMBO_KEY_FLAGSET2!=KEYFLAG_DUMMY) 
				AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_COMBO_SET2, 0);
		break;	
		default:
		break;
	}
#endif	
}

#if 0
void _____PowerSave_Function_________(){ruturn;} //dummy
#endif

void AutoPowerOffCounterReset(void)
{
	MMP_ULONG	t;

#if (SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN)
	if(ubGsnrPwrOnActStart)
	{
		gAutoPowerOffCounter = POWER_ON_GSNR_IDLE_TIME * TICKS_PER_SECOND;
	}
	else
#endif
	{
		switch(MenuSettingConfig()->uiAutoPowerOff)
		{
		#if (MENU_GENERAL_AUTO_POWEROFF_30SEC_EN)
			case AUTO_POWER_OFF_30SEC:
				gAutoPowerOffCounter = (30*1000);
			break;
		#endif	
        #if (MENU_GENERAL_AUTO_POWEROFF_15SEC_EN)
			case AUTO_POWER_OFF_15SEC:
				gAutoPowerOffCounter = (15*1000);
			break;
		#endif
		#if (MENU_GENERAL_AUTO_POWEROFF_1MIN_EN)
			case AUTO_POWER_OFF_1MIN:
				gAutoPowerOffCounter = TICKS_PER_MINUTE;
			break;
		#endif
		#if (MENU_GENERAL_AUTO_POWEROFF_2MIN_EN)	
			case AUTO_POWER_OFF_2MINS:
				gAutoPowerOffCounter = 2 * TICKS_PER_MINUTE;	
			break;
		#endif
		#if (MENU_GENERAL_AUTO_POWEROFF_3MIN_EN)
			case AUTO_POWER_OFF_3MINS:
				gAutoPowerOffCounter = 3 * TICKS_PER_MINUTE;
			break;
		#endif
		#if (MENU_GENERAL_AUTO_POWEROFF_5MIN_EN)	
			case AUTO_POWER_OFF_5MINS:
				gAutoPowerOffCounter = 5 * TICKS_PER_MINUTE;
			break;
		#endif	
			default:
				gAutoPowerOffCounter = 0;
				return;
			break;
		}
	}
	
#if (NO_CARD_POWER_OFF_EN)
    if (gbNoCardPowerOff)
    {		
		#ifdef CFG_POWER_CUS_AUTO_OFF_TIME //may be defined in config_xxx.h
		printc("NO SD and %d sec power off\r\n", CFG_POWER_CUS_AUTO_OFF_TIME / 1000);
		gAutoPowerOffCounter = CFG_POWER_CUS_AUTO_OFF_TIME;
		#else
		RTNA_DBG_Str(0, "NO SD and 10 sec power off\r\n");
		gAutoPowerOffCounter = (10*1000);
    	#endif
    }
#endif
	
	MMPF_OS_GetTime(&t);
	gAutoPowerOffCounter += t;
	
	if (0 == gAutoPowerOffCounter) {
		// overflow
		gAutoPowerOffCounter++;
	}
}

void LCDPowerSaveCounterReset(void)
{
	MMP_ULONG	t;

    #ifdef CFG_LCD_POWER_SAVE_WHEN_MOTION //may be defined in config_xxx.h, typically 10*10000
	if(m_ubMotionDtcEn && uiGetCurrentState()==UI_VIDEO_STATE)
	{
		gLCDPowerSaveCounter = CFG_LCD_POWER_SAVE_WHEN_MOTION;
	}
	else
	#endif
	{
		switch(MenuSettingConfig()->uiLCDPowerSave)
		{
		#if (MENU_GENERAL_LCD_POWER_SAVE_1MIN_EN)
			case LCD_POWER_SAVE_1MIN:
				gLCDPowerSaveCounter = TICKS_PER_MINUTE;
			break;
		#endif
		#if (MENU_GENERAL_LCD_POWER_SAVE_3MIN_EN)	
			case LCD_POWER_SAVE_3MIN:
				gLCDPowerSaveCounter = 3 * TICKS_PER_MINUTE;
			break;
		#endif	
			default:
				gLCDPowerSaveCounter = 0;
				return;
			break;
		}
	}

	MMPF_OS_GetTime(&t);
	gLCDPowerSaveCounter += t;
}

void VideoPowerOffCounterReset(void)
{
	UINT32	p;
	
	p = AHC_GetVideoPowerOffTime();

	MMPF_OS_GetTime(&gVideoPowerOffCounter);
	
	#ifdef OFF_TIME_DURING_LOCK
    {
    	if(VideoFunc_LockFileEnabled())
    	{
    		gVideoPowerOffCounter += (OFF_TIME_DURING_LOCK * TICKS_PER_SECOND);
    	}
    	else
    	{
    		gVideoPowerOffCounter += (p * TICKS_PER_SECOND);
    	}
    }
	#else
	gVideoPowerOffCounter += (p * TICKS_PER_SECOND);
	#endif
}

void PowerSavingModeDetection(UINT32 tick)
{
	UINT8		cur_state;	
	
	cur_state = uiGetCurrentState();
    
    if (cur_state==UI_MSDC_STATE || cur_state==UI_PCCAM_STATE)
    {
		AutoPowerOffCounterReset();
		return;
	}
	else if(VideoFunc_RecordStatus()==AHC_TRUE)// video status
	{
		AutoPowerOffCounterReset(); // been running
		return;
	}	
    else if(cur_state==UI_PLAYBACK_STATE && (MOV_STATE_PLAY == GetMovConfig()->iState || MOV_STATE_FF == GetMovConfig()->iState) )
    {
		AutoPowerOffCounterReset();
		return;
	}

	if (gAutoPowerOffCounter != 0)
	{
		INT32 time_diff = (INT32) (tick - gAutoPowerOffCounter);

		// 21600000(ms): avoid overflow case, set system tolerance 6-HR
		// need modify it when UI setting except to 6-HR
		if ((time_diff >= 0) && (time_diff < 21600000)) {
			RTNA_DBG_Str(0, BG_RED("\r\n!!! Entering Power saving mode")"\r\n");
			bForce_PowerOff = AHC_TRUE;
			SetKeyPadEvent(KEY_POWER_OFF);
			gAutoPowerOffCounter = 0; // Never enter again
		}
	}
}

void LCDPowerSaveDetection(UINT32 tick)
{
#ifdef CFG_SURPRESS_POWER_SAVE_WHEN_PLAYING //may be defined in config_xxx.h
    if(uiGetCurrentState()==UI_PLAYBACK_STATE && (MOV_STATE_PLAY == GetMovConfig()->iState || MOV_STATE_FF == GetMovConfig()->iState) )
    {
		LCDPowerSaveCounterReset();
		return;
	}
#endif

#if (UVC_HOST_VIDEO_ENABLE )
    if (gLCDPowerSaveCounter != 0 && gbRearState == AHC_FALSE) 
#else
	if (gLCDPowerSaveCounter != 0)
#endif
	{
		if ((int)(tick - gLCDPowerSaveCounter) >= 0) 
		{
			gLCDPowerSaveCounter = 0;
			
			if(LedCtrl_GetBacklightStatus())
				LedCtrl_LcdBackLight(AHC_FALSE);
		}
	}
#if (UVC_HOST_VIDEO_ENABLE )
	else if(gbRearState == AHC_TRUE)
	{
		LCDPowerSaveCounterReset();
	}
#endif
}

void VideoPowerOnOffDetection(UINT32 tick)
{
    UINT8 ubBootupWithCharger = 0;
	static AHC_BOOL bPoweroff = AHC_FALSE; 
    
    if(AHC_TRUE == AHC_GetShutdownByChargerOut())
    {
		#if (RESET_POWER_CNT_DURING_LOCK)
    	{
    		if(VideoFunc_LockFileEnabled())
    		{
    			VideoPowerOffCounterReset();
    			return;
    		}
    	}
		#endif
		
		#if (CHARGER_OUT_ACT_OTHER_MODE==ACT_FORCE_POWER_OFF)
		if(!VideoFunc_RecordStatus())
		{
			tick = gVideoPowerOffCounter+1;
		}
		#endif
		#if (CHARGER_OUT_ACT_VIDEO_REC==ACT_FORCE_POWER_OFF)
		if(VideoFunc_RecordStatus())
		{
			tick = gVideoPowerOffCounter+1;
		}
		#endif		

    	if (gVideoPowerOffCounter == 0)
    		return;

    	tick = tick - gVideoPowerOffCounter;

        if ((int)tick > 0 && bPoweroff==AHC_FALSE)
        {
        	bPoweroff = AHC_TRUE;
            RTNA_DBG_Str(0, FG_RED("!!! Auto Power off\r\n"));
            bForce_PowerOff = AHC_TRUE;
		#ifdef CFG_MENU_SETTING_BY_CUSTOMER_CARD
             AHC_SetMode(AHC_MODE_IDLE);
             AHC_PowerOff_NormalPath();
		#endif
       //     AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, KEY_POWER_OFF, 0);
        }
    }
    else if((ubBootupWithCharger = AHC_GetRecordByChargerIn()) > 0)
    {
        AHC_SetRecordByChargerIn(--ubBootupWithCharger);

        if(!ubBootupWithCharger)
        {
            AHC_BOOL sendVideoRecord = AHC_FALSE;

			if (uiGetCurrentState() == UI_STATE_UNSUPPORTED) {
				// Delay more time
				AHC_SetRecordByChargerIn(3);
			}
			else if (uiGetCurrentState() == UI_VIDEO_STATE) {
                sendVideoRecord = AHC_TRUE;
            }
#if 0		//Andy Liu TBD	
#if (TVOUT_PREVIEW_EN)
            else if ((uiGetCurrentState() == UI_TVOUT_STATE) && \
                     (AHC_TV_VIDEO_PREVIEW_STATUS == TVFunc_Status())) {
                    sendVideoRecord = AHC_TRUE;
            }
#endif
#if (HDMI_PREVIEW_EN)
            else if ((uiGetCurrentState() == UI_HDMI_STATE) && \
                     (AHC_HDMI_VIDEO_PREVIEW_STATUS == HDMIFunc_Status())) {
                    sendVideoRecord = AHC_TRUE;
            }
#endif
#endif

            if (AHC_TRUE == sendVideoRecord)
            {
             //   SetKeyPadEvent(KEY_VIDEO_RECORD);
                VideoPowerOffCounterReset();

                if (AHC_GetShutdownByChargerOut() == AHC_TRUE)
                    AHC_SetShutdownByChargerOut(AHC_FALSE);
                
    			#if ((SUPPORT_GSENSOR && POWER_ON_BY_GSENSOR_EN) && (GSNR_PWRON_MOVIE_SHAKED_ACT == GSNR_PWRON_MOVIE_LOCKED))
    			if ((AHC_GetBootingSrc() & PWR_ON_BY_GSENSOR) == PWR_ON_BY_GSENSOR)
    			{
				    extern AHC_BOOL ubGsnrPwrOnFirstREC;
                    extern AHC_BOOL m_ubGsnrIsObjMove;
                    
				    if (AHC_TRUE == ubGsnrPwrOnFirstREC) {
					    m_ubGsnrIsObjMove = AHC_TRUE;
					    //ubGsnrPwrOnFirstREC = AHC_FALSE;
					}
    			}
			    #endif
            }
        }
    }
}

void SpeakerPowerDetection(void)
{
    static MMP_ULONG ulpreDetectTime = 0;
    MMP_ULONG ulcurDetectTime = 0;

    if (AHC_FALSE == AHC_IsSpeakerEnable()) {
		if (MMPF_RTC_CTL_IsValid())
        	ulpreDetectTime = MMPF_RTC_CTL_ReadTime_InSeconds();
        return;    
    }

    ulcurDetectTime = MMPF_RTC_CTL_ReadTime_InSeconds();

    if (ulcurDetectTime >= ulpreDetectTime) {
        ulcurDetectTime -= ulpreDetectTime;
    }
    else {
        ulcurDetectTime = ((MMP_ULONG) -1) - ulpreDetectTime + ulcurDetectTime;
    }

    if (ulcurDetectTime < SPEAKER_OFF_TIME)
        return;

    ulpreDetectTime = MMPF_RTC_CTL_ReadTime_InSeconds();
    
    {
        UINT8 ubSoundStatus = 0;

        AHC_GetSoundEffectStatus(&ubSoundStatus);
        
        if((AHC_SOUND_EFFECT_STATUS_STOP == ubSoundStatus) || (AHC_SOUND_EFFECT_STATUS_INVALID == ubSoundStatus))
        {    
            if (AHC_GetAhcSysMode() != AHC_MODE_PLAYBACK) {
                AHC_SpeakerEnable(SPEAKER_ENABLE_GPIO, AHC_FALSE);
                printc("!!! Speaker turn-off by %s\r\n", __func__);
            }
        }
    }
}

#if 0
void _____SDMMC_Function_________(){ruturn;} //dummy
#endif


#if (DETECT_SD_BY_TIMER==1)
 
extern MMP_BOOL bFirstTimeGetFreeSpace;

void SDMMCTimerHandler(void)
{
#if (DETECT_SD_BY_TIMER==1)
    MMP_UBYTE bSD_Inserted = AHC_FALSE;
    
    static MMP_ULONG bCardDetCnt = 1, bCardRemoveCnt = 1;

    if(!gAHC_InitialDone)
        return;
  
    bSD_Inserted = AHC_IsSDInserted();
    
    if ((bSD_Inserted == AHC_TRUE) && (AHC_SDMMC_GetState() == SDMMC_OUT)) 
    {
        bCardDetCnt = (bCardDetCnt + 1) & 0x3f;
        bCardRemoveCnt = 1;
        
        if(0 == bCardDetCnt)
        {
            AHC_SDMMC_SetState(SDMMC_IN);
			bFirstTimeGetFreeSpace = MMP_TRUE;
            printc("\r\n+++SD_CARD_IN+++\r\n");
            SetKeyPadEvent(SD_CARD_IN);
        }
    }
    else if ((bSD_Inserted == AHC_FALSE) && (AHC_SDMMC_GetState() == SDMMC_IN))
    {
        bCardRemoveCnt = (bCardRemoveCnt + 1) & 0x05;
        bCardDetCnt = 1;
        
        if(0 ==bCardRemoveCnt)
        {
            AHC_SDMMC_SetState(SDMMC_OUT);
            bFirstTimeGetFreeSpace = MMP_TRUE;
            printc("\r\n+++SD_CARD_OUT+++\r\n");
            SetKeyPadEvent(SD_CARD_OUT);
        }
    }
    else
    {
        bCardRemoveCnt = 1;
        bCardDetCnt = 1;
    }

    #ifdef CFG_NOSD_POWEROFF
	{
		static MMP_UBYTE  bPoweroff = AHC_FALSE;
		bSD_Inserted = AHC_IsSDInserted();

		if (!bSD_Inserted && !bPoweroff)
		{
			//AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, KEY_POWER_OFF, 0);
			SetKeyPadEvent(BUTTON_FUNC1_LPRESS);
			printc("KEY_POWER_OFF %d\r\n");
			bPoweroff = AHC_TRUE;
		}
	}
    #endif
#endif
}

void SDMMCTimerHandlerForOs(void *tmr, void *arg)
{
    #if ((DETECT_SD_BY_TIMER==1) && (DETECT_SD_BY_OS_TIMER == 1))
    SDMMCTimerHandler();
    #endif
}

#if ((DETECT_SD_BY_TIMER==1) && (DETECT_SD_BY_OS_TIMER == 1))
static AHC_OS_TMRID ulSDMMC_TMRID = 0xFF;
#endif

void SDMMCTimerOpen(void)
{
#if (DETECT_SD_BY_TIMER==1)
    #if (DETECT_SD_BY_OS_TIMER == 1)
    ulSDMMC_TMRID = AHC_OS_StartTimer(40, MMPF_OS_TMR_OPT_PERIODIC, &SDMMCTimerHandlerForOs, NULL);
    #else
    MMPF_TIMER_ATTR TimerAttribute;
    
	TimerAttribute.TimePrecision = MMPF_TIMER_MILLI_SEC;
	TimerAttribute.ulTimeUnits   = 40;
	TimerAttribute.Func          = SDMMCTimerHandler;
	TimerAttribute.bIntMode      = MMPF_TIMER_PERIODIC;
    MMPF_Timer_Start(MMPF_TIMER_2, &TimerAttribute);
    #endif
#endif
}

void SDMMCTimerClose(void)
{
#if (DETECT_SD_BY_TIMER==1)
    #if (DETECT_SD_BY_OS_TIMER == 1)
    if (ulSDMMC_TMRID < 0xFE) {
        AHC_OS_StopTimer(ulSDMMC_TMRID, MMPF_OS_TMR_OPT_PERIODIC);
        ulSDMMC_TMRID = 0xFF;
    }
    #else
	MMPF_Timer_Stop(MMPF_TIMER_2);
    #endif
#endif
}

UINT8 SDMMCSpaceCalculation(void)
{
	UINT64 ulFreeByte,ulTotalByte,ulAvailableByte;	
 	
 	AHC_FS_GetStorageFreeSpace(AHC_MEDIA_MMC, &ulFreeByte);
	AHC_FS_GetTotalSpace("SD:\\", sizeof("SD:\\"), &ulTotalByte);
	ulAvailableByte = (ulFreeByte*100) / ulTotalByte;
	if((ulAvailableByte/25)>=3)
	{
		ubSDMMCLeftSpaceLevel = 4;
	}
	else if((ulAvailableByte/25)>=2)
	{
		ubSDMMCLeftSpaceLevel = 3;
	}
	else if((ulAvailableByte/25)>=1)
	{
		ubSDMMCLeftSpaceLevel = 2;
	}
	else
		ubSDMMCLeftSpaceLevel = 1;

	if(ulFreeByte==0)
		AHC_BUZZER_Alert(4, 1, 100);
		
	return ubSDMMCLeftSpaceLevel;
}

#endif

#if 0
void _____KeyPad_Device_Function_________(){ruturn;} //dummy
#endif

void InitButtonGpio(MMP_GPIO_PIN piopin, GpioCallBackFunc* CallBackFunc)
{
	
		
	if(piopin == MMP_GPIO_MAX)
		return;
	
		
    MMPF_PIO_EnableOutputMode(piopin, MMP_FALSE, MMP_TRUE);
	


    if (CallBackFunc) {

        MMPF_PIO_EnableTrigMode(piopin, GPIO_H2L_EDGE_TRIG, MMP_TRUE, MMP_TRUE);
        MMPF_PIO_EnableTrigMode(piopin, GPIO_L2H_EDGE_TRIG, MMP_TRUE, MMP_TRUE);
        MMPF_PIO_EnableInterrupt(piopin, MMP_TRUE, 0, (GpioCallBackFunc *) CallBackFunc, MMP_TRUE);

		
    }
    else
        MMPF_PIO_EnableInterrupt(piopin, MMP_FALSE, 0, (GpioCallBackFunc *) NULL, MMP_TRUE);    
}

// long 4-25
void InitButton24G(MMP_GPIO_PIN piopin, GpioCallBackFunc* CallBackFunc,MMP_GPIO_TRIG flag)
{
	MMP_UBYTE revalback; // 4-25

	//printc("22222222222222~~piopin=~~%d~~~~\r\n",piopin);// 4-25
		
	if(piopin == MMP_GPIO_MAX)
		return;

	//printc("~~22222222222222222~~0x%d~~~~\r\n",CallBackFunc); // 4-25

   // printc("~MMPF_PIO_EnableOutputMode = %d~~~~~~~~~~\r\n",MMPF_PIO_EnableOutputMode(piopin, MMP_FALSE, MMP_TRUE));
		
    MMPF_PIO_EnableOutputMode(piopin, MMP_FALSE, MMP_TRUE);

	MMPF_PIO_GetData(piopin,&revalback);

 //  printc("~MMPF_PIO_GetData = %d~~~~~~~~~~\r\n",MMPF_PIO_GetData(piopin,&revalback));
	
	//printc("2222222222222222~revalback=%d~~~~\r\n",revalback); // 4-25

    if (CallBackFunc) {
	//printc("~22222222222~~~~%s~~~~~~~~~~\r\n",__func__);// long 4--25

	#if 0
       if( MMPF_PIO_EnableTrigMode(piopin, GPIO_H2L_EDGE_TRIG, MMP_TRUE, MMP_FALSE) &&
        MMPF_PIO_EnableTrigMode(piopin, GPIO_L2H_EDGE_TRIG, MMP_TRUE, MMP_FALSE) &&
        MMPF_PIO_EnableInterrupt(piopin, MMP_TRUE, 0, (GpioCallBackFunc *) CallBackFunc, MMP_FALSE))

	 #endif

	 
	//printc("~MMPF_PIO_EnableTrigMode~~~~%d~~~~~~~~~~\r\n",MMPF_PIO_EnableTrigMode(piopin, flag, MMP_TRUE, MMP_TRUE));

	 MMPF_PIO_EnableTrigMode(piopin, flag, MMP_TRUE, MMP_TRUE);
	//printc("~~MMPF_PIO_EnableTrigMode~~~%d~~~~~~~~~~\r\n",MMPF_PIO_EnableTrigMode(piopin, flag, MMP_TRUE, MMP_TRUE));
	//printc("~~MMPF_PIO_EnableInterrupt~~~%d~~~~~~~~~~\r\n",MMPF_PIO_EnableInterrupt(piopin, MMP_TRUE, 0, (GpioCallBackFunc *) CallBackFunc, MMP_TRUE));

	MMPF_PIO_EnableInterrupt(piopin, MMP_TRUE, 0, (GpioCallBackFunc *) CallBackFunc, MMP_TRUE);

	//printc("~22222222222222~~debug~~%s~~~~~~~~~~\r\n",__func__);// long 4--25
    }
    else
        MMPF_PIO_EnableInterrupt(piopin, MMP_FALSE, 0, (GpioCallBackFunc *) NULL, MMP_TRUE);    



}

void SetPrevNextPin(MMP_UBYTE level)
{
	MMPF_PIO_SetData(USB_EN_SELECT,level,MMP_TRUE);// lyj 20190226

	MMPF_PIO_SetData(MMP_GPIO67, level, MMP_TRUE);
	MMPF_PIO_SetData(MMP_GPIO68, level, MMP_TRUE);
	MMPF_PIO_SetData(MMP_GPIO69, level, MMP_TRUE);
	if(!level)
		SelfAllFlag.upDwonUB = 1;// bluetooth
	else
		SelfAllFlag.upDwonUB = 2;// usb
}

void CloseTheUSBPower(void)
{
	MMPF_PIO_SetData(USB_EN_SELECT,0,MMP_TRUE);// lyj 20190226
	SelfAllFlag.upDwonUB = 0;
}

void CloseThePreviwVideoPower(void)
{
	MMPF_PIO_SetData(MMP_GPIO50, 0, MMP_TRUE);

}
void OpenThePreviwVideoPower(void)
{

	MMPF_PIO_SetData(MMP_GPIO50, MMP_TRUE, MMP_TRUE);
}

void Bluetooth_LEd(void)
{
	#if 1 // lyj 20190423
	//MMPF_PIO_PadConfig(MMP_GPIO66, PAD_OUT_DRIVING(0), MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO66, MMP_TRUE, MMP_TRUE);

	MMPF_PIO_SetData(MMP_GPIO66, 1,MMP_TRUE);
	//printc("~~~~~~led on~~~~~~~~\r\n");

	BlueStause = 1;
	#endif
}

void Bluetooth_LEdoff(void)
{
	#if 1// lyj 20190423
	MMPF_PIO_PadConfig(MMP_GPIO66, PAD_OUT_DRIVING(0), MMP_TRUE);
	MMPF_PIO_EnableOutputMode(MMP_GPIO66, MMP_TRUE, MMP_TRUE);

	MMPF_PIO_SetData(MMP_GPIO66, 0,MMP_TRUE);
	BlueStause = 0;
	//printc("~~~~~~led off~~~~~~~~\r\n");
	#endif

}

void BlueMusicPrev(void)
{
	//MMPF_PIO_PadConfig(MMP_GPIO39, PAD_OUT_DRIVING(0), MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO39, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_SetData(MMP_GPIO68, 1,MMP_TRUE);
	AHC_OS_SleepMs(250);
	MMPF_PIO_SetData(MMP_GPIO68, 0,MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO37, MMP_FALSE, MMP_TRUE);
	printc("~~~~~~Music Prev~~~~~~~~\r\n");

}

void BlueMusicPlay_Pause(void)
{
	//MMPF_PIO_PadConfig(MMP_GPIO38, PAD_OUT_DRIVING(0), MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO38, MMP_TRUE, MMP_TRUE);

	
	
	MMPF_PIO_SetData(MMP_GPIO67, 1,MMP_TRUE);
	AHC_OS_SleepMs(250);
	MMPF_PIO_SetData(MMP_GPIO67, 0,MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO38, MMP_FALSE, MMP_TRUE);
	printc("~~~~~~Music play~~~~~~~~\r\n");

}
void BlueMusicNext(void)
{
	//MMPF_PIO_PadConfig(MMP_GPIO37, PAD_OUT_DRIVING(0), MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO37, MMP_TRUE, MMP_TRUE);

	MMPF_PIO_SetData(MMP_GPIO69, 1,MMP_TRUE);
	AHC_OS_SleepMs(250);
	MMPF_PIO_SetData(MMP_GPIO69, 0,MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO37, MMP_FALSE, MMP_TRUE);
	printc("~~~~~~Music next~~~~~~~~\r\n");
}// lyj 20181026

void USBMusicPrev(void)
{
	//MMPF_PIO_PadConfig(MMP_GPIO39, PAD_OUT_DRIVING(0), MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO39, MMP_TRUE, MMP_TRUE);
	//MMPF_PIO_SetData(MMP_GPIO69, 1,MMP_TRUE);
	MMPF_PIO_SetData(MMP_GPIO69, 0,MMP_TRUE);
	AHC_OS_SleepMs(300);
	MMPF_PIO_SetData(MMP_GPIO69, 1,MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO37, MMP_FALSE, MMP_TRUE);
	printc("~~~USB~~~Music Prev~~~~~~~~\r\n");

	//str_first1[0] = 0;
	//str_first1[1] = 0;
	//str_first1[2] = 0;
	//str_first1[3] = 0;
}

void USBMusicNext(void)//USBMusicNext
{
	//MMPF_PIO_PadConfig(MMP_GPIO38, PAD_OUT_DRIVING(0), MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO38, MMP_TRUE, MMP_TRUE);

	
//	MMPF_PIO_SetData(MMP_GPIO68, 1,MMP_TRUE);
	MMPF_PIO_SetData(MMP_GPIO68, 0,MMP_TRUE);
	AHC_OS_SleepMs(300);
	MMPF_PIO_SetData(MMP_GPIO68, 1,MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO38, MMP_FALSE, MMP_TRUE);
	printc("~~~USB~~~Music next~~~~~~~~\r\n");
	//str_first1[0] = 0;
	//str_first1[1] = 0;
	//str_first1[2] = 0;
	//str_first1[3] = 0;

}
void USBMusicPlay_Pause(void)
{
	//MMPF_PIO_PadConfig(MMP_GPIO37, PAD_OUT_DRIVING(0), MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO37, MMP_TRUE, MMP_TRUE);
	//MMPF_PIO_SetData(MMP_GPIO67, 1,MMP_TRUE);
	MMPF_PIO_SetData(MMP_GPIO67, 0,MMP_TRUE);
	AHC_OS_SleepMs(300);
	MMPF_PIO_SetData(MMP_GPIO67, 1,MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMP_GPIO37, MMP_FALSE, MMP_TRUE);
	printc("~~~USB~~~Music play~~~~~~~~\r\n");
}// lyj 20181115


#if 0
void Bluetooth_power(void)
{
	MMPF_PIO_EnableOutputMode(MMP_GPIO32, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_SetData(MMP_GPIO32, 0, MMP_TRUE); 
	AHC_OS_SleepMs(1);
	MMPF_PIO_SetData(MMP_GPIO32, 1, MMP_TRUE); 
}
#endif
void print_recvdata(void)
{
	int i = 0;
	#if 1
	if(recv_data[0] == 0xAA && recv_data[1] == 0xAC && recv_data[2] == 0x03)
	{
		for(i = 0;i<14;i++)
		{
			printc("-->0x%x  ",recv_data[i]);
		}
		printc("\r\n");
	}

	if(recv_data[0] == 0xAA && recv_data[5] == 0x0D && recv_data[6] == 0x0A)
	{
		for(i = 0;i<7;i++)
		{
			printc("-->0x%x  ",recv_data[i]);
		}
		printc("\r\n");
	}

	if(recv_data[6] != 0x0A && recv_data[1] == 0x55 && recv_data[8] == 0x0A)
	{
		for(i = 0;i<9;i++)
		{
			printc("-->0x%x  ",recv_data[i]);
		}
		printc("\r\n");
	}	
	#endif
	#if 0
	for(i = 0;i<14;i++)
		{
			printc("i-->0x%x  ",i,recv_data[i]);
		}
		printc("\r\n");
	#endif
}



/// ===========end=======

void InitKeyPad(void)
{
    int i;

    //e.g. #define CFG_CUS_KEYPAD_INIT 	AHC_GPIO_ConfigPad(KEY_LED_MUTE,PAD_PULL_HIGH | PAD_E8_CURRENT);//*((char*)(0x80009D21)) = 0x44;
    #ifdef CFG_CUS_KEYPAD_INIT //may be defined in config_xxx.h
    CFG_CUS_KEYPAD_INIT
    #endif

#if (SUPPORT_GSENSOR)
    #if (POWER_ON_BY_GSENSOR_EN) && defined(GSENSOR_INT)
	if ((AHC_GetBootingSrc() & PWR_ON_BY_GSENSOR) == PWR_ON_BY_GSENSOR)
	{
		if(	GSNR_PWRON_ACT == GSNR_PWRON_IDLE && \
			(GSNR_PWRON_REC_BY && GSNR_PWRON_REC_BY_KEY))
			ubGsnrPwrOnActStart = AHC_FALSE;
		else if(GSNR_PWRON_ACT == GSNR_PWRON_REC_AUTO)
			ubGsnrPwrOnActStart = AHC_TRUE;
			
		printc(FG_BLUE("Power On by Gsensor\r\n"));
    }
    #endif

	if (AHC_Gsensor_IOControl(GSNR_CMD_RESET, NULL) != AHC_TRUE) {
		//No GSensor in device
	    #ifndef CFG_MENU_ALWAYS_KEEP_GSENSOR_SENSITIVITY //may be defined in config_xxx.h
		RemoveMenu_GsensorSensitivity();
		#endif
	} 
	else {
		AHC_Gsensor_IOControl(GSNR_CMD_CALIBRATION, NULL);
		//Set which axis into ground to use it to detect kit position
		AHC_Gsensor_IOControl(GSNR_CMD_SET_GROUND_AXIS, (MMP_UBYTE*)GSNR_GROUND_AXIS);
        AHC_Gsensor_SetIntThreshold();
	}
#endif//SUPPORT_GSENSOR

#if defined(TV_OUT_DETECT_GPIO)
    InitButtonGpio(TV_OUT_DETECT_GPIO, NULL);
#endif

#if (SUPPORT_TOUCH_PANEL)
    AHC_TouchPanel_Init();
    #if defined(TOUCH_PANEL_USE_INTR_MODE)&&(TOUCH_PANEL_USE_INTR_MODE)
    //#warning    touchpanel use interrupt mode
    #else
    #if defined(TOUCH_PANEL_INT_GPIO)
    if (TOUCH_PANEL_INT_GPIO != MMP_GPIO_MAX)
        MMPS_PIO_PadConfig(TOUCH_PANEL_INT_GPIO, 0x24);//Set CGPIO20 PU for HDK C version
    #endif
    #endif
#endif

#ifdef CFG_CUS_GPIO_CONFIG
	CFG_CUS_GPIO_CONFIG();
#endif

    for( i=0; i<gNumOfDevice; i++ )
    {
        InitButtonGpio( sDeviceStatus[i].piopin, NULL);

#if defined(DEVICE_GPIO_DC_INPUT)
        if (DEVICE_GPIO_DC_INPUT != MMP_GPIO_MAX) {
            if((DEVICE_GPIO_DC_INPUT == sDeviceStatus[i].piopin) && AHC_IsDCInBooting())
            {
                printc("Force DEVICE_GPIO_DC_INPUT as plug in status\r\n");
                sDeviceStatus[i].ulKeyLastValue = DEVICE_GPIO_DC_INPUT_LEVEL;       //Force it as plug in status.
            }
        }
#endif
    }

    for( i=0; i<gNumOfGPIOKey; i++ )
    { 
        InitButtonGpio( sButton[i].piopin, NULL );
    }

#if (DETECT_SD_BY_TIMER==1)
    SDMMCTimerOpen();
#endif
}

MMP_ERR GetKeyPadEvent(MMP_ULONG *keyevent)
{
    *keyevent = gKeyEvent;
    gKeyEvent = KEYPAD_NONE;
    
    return MMP_ERR_NONE;
}

void SetKeyPadEvent(MMP_ULONG keyevent)
{
	gKeyEvent = keyevent;
	MMPF_OS_SetFlags(CDV_UI_Flag, CDV_KEYPAD_FLAG, MMPF_OS_FLAG_SET);
}

void SetUITimeEvent(void)
{
	MMPF_OS_SetFlags(CDV_UI_Flag, CDV_TIME_FLAG, MMPF_OS_FLAG_SET);
}

#if (UPDATE_UI_USE_MULTI_TASK)
//void SetUIUpdateEvent(void)
//{
//	MMPF_OS_SetFlags(CDV_UI_Flag, CDV_UI_FLAG, MMPF_OS_FLAG_SET);
//}
#endif

#if (HDMI_ENABLE)
AHC_BOOL HDMI_Cable_Detection(void)
{
    AHC_BOOL		bIsHDMIConnect = AHC_FALSE;
    static AHC_BOOL m_bHdmiCableIn = AHC_FALSE; 

    //Andy Liu. Note: detection function should not include system flow.    
    bIsHDMIConnect = AHC_IsHdmiConnect();

    if(m_bHdmiCableIn == bIsHDMIConnect) {
        //No state changed.
        return bIsHDMIConnect;//AHC_TRUE;
    }

    m_bHdmiCableIn = bIsHDMIConnect;

    if (bIsHDMIConnect == AHC_TRUE) {
        //HDMI cable in
        RTNA_DBG_Str(0, "@@@ HDMI_CABLE_IN\r\n");
        SetKeyPadEvent(HDMI_CABLE_IN);
    }    
    else {
        //HDMI cable out 
        RTNA_DBG_Str(0, "@@@ HDMI_CABLE_OUT\r\n");
        SetKeyPadEvent(HDMI_CABLE_OUT);
    } 

    return bIsHDMIConnect;//AHC_FALSE;
}
#endif

#if (TVOUT_ENABLE)
AHC_BOOL TV_Cable_Detection(void)
{
    AHC_BOOL 		bIsTVConnect = AHC_FALSE;
    static AHC_BOOL m_bTvCableIn = AHC_FALSE;

#ifdef TV_ONLY
    return AHC_TRUE;
#endif

    //Andy Liu. Note: detection function should not include system flow.
    bIsTVConnect = AHC_IsTVConnectEx(); 

    if(m_bTvCableIn == bIsTVConnect){
        //No state changed.
        return bIsTVConnect;//AHC_TRUE;        
    }

    m_bTvCableIn = bIsTVConnect;

    if (bIsTVConnect == AHC_TRUE) {
        //TV cable in   
        RTNA_DBG_Str(0, "@@@ TV_CABLE_IN\r\n");
        SetKeyPadEvent(TV_CABLE_IN);
    }
    else {
        //TV cable out
        RTNA_DBG_Str(0, "@@@ TV_CABLE_OUT\r\n");
        SetKeyPadEvent(TV_CABLE_OUT);
    }  

    return bIsTVConnect;
}
#endif

void GPIO_Key_Detection(void)
{
    MMP_ULONG 	i, ulNow;
	UINT32		uiLCDstatus;
	  static AHC_BOOL Power_key_dec = 0; // lyj 20190427
	  static MMP_ULONG Poweranxiajishu = 0;
    if (!AHC_KeyUIWorking())
    	return;
    	
    MMPF_OS_GetTime(&ulNow);   
    
    AHC_GetParam(PARAM_ID_LCD_STATUS, &uiLCDstatus);
    
    for( i=0; i<gNumOfGPIOKey ; i++ )
    {
		MMP_UBYTE tempValue = 0;
		MMP_UBYTE ubKeyId   = i;

		if(sButton[i].piopin == MMP_GPIO_MAX)
			continue;
		if(Power_key_dec < 60)
		{
			Power_key_dec ++;
			if(sButton[i].piopin == KEY_GPIO_POWER)
			{
				//printc("~~~~~~^_^~~~~~==~~~%d~~~~~~~~~ \r\n",Power_key_dec);			
					continue; // lyj 20190427
			}
			

		}
		
	
		MMPF_PIO_GetData(sButton[i].piopin, &tempValue);

		/*
		*author: lyj 20190814
		*menu键+ 电源键进入测试模式
		*/
		if(i == 1 )
		{
			MMP_UBYTE PowerValue = 100;
			 
			 extern AHC_BOOL compress_key;
			if(tempValue)
			{
				Poweranxiajishu++;
					//printc("##-----------------^_^---Poweranxiajishu == %d------------###\r\n",Poweranxiajishu);	
				if(Poweranxiajishu > 20 )
				{
					MMPF_PIO_GetData(sButton[4].piopin, &PowerValue);	
					if(!PowerValue)
					{
						// 组合键按下.	
						printc("##-----------------^_^---------------###\r\n");	
						Poweranxiajishu =0;
						compress_key = 1;
						AHC_WMSG_DrawBMP(WMSG_WAIT_INITIAL_DONE,2);
						
					}
					
				}
				
			}

			
		}

#if (HALL_SNR_FLIP_SELECT & FLIP_OSD) || (KEY_FLIP_SELECT & FLIP_OSD) || (KEYPAD_ROTATE_EN==1)
		ubKeyId = (!uiLCDstatus)?(i):(sButton[i].ubKeyPairId);
#endif		
		if(ubKeyId == KEY_GPIO_ID_INVALID)
			ubKeyId = i;
		
		// Continuous key
		if( sButton[ubKeyId].ulContinuousTime	!=	0								&&
			sButton[ubKeyId].ulKeyLastValue		==	sButton[ubKeyId].ulPressLevel	&&
			sButton[ubKeyId].ulKeyLastValue		==	tempValue )
		{
			if( ulNow - sButton[ubKeyId].ulKeyLastTime > sButton[ubKeyId].ulContinuousTime )
			{
				sButton[ubKeyId].ubLongPress 		= 1;
                sButton[ubKeyId].ulKeyLastValue 	= tempValue;
                sButton[ubKeyId].ulKeyLastTime  	= ulNow;
                sButton[ubKeyId].ulKeyCount++;
                    
				if(KEY_LPRESS_POWER == sButton[ubKeyId].iLongPressId)
				{
					if(sButton[ubKeyId].ulKeyCount==POWER_KEY_LONG_PRESS_CNT)
					{
			        	#if defined(WIFI_PORT) && (WIFI_PORT == 1) && defined(CFG_WIFI_STREAMING_DISABLE_REC_KEY)
			        	if ((AHC_STREAM_OFF != AHC_GetStreamingMode()) && 
							#if (POWER_OFF_CONFIRM_PAGE_EN)
							(PowerOff_InProcess == AHC_FALSE) &&
							#endif
			                (KEY_VIDEO_RECORD == sButton[ubKeyId].iLongPressId))
						{
							continue;
						}
						#endif
							
						SetKeyPadEvent(sButton[ubKeyId].iLongPressId);
						printc("1Long Press Key %s\r\n",sButton[ubKeyId].ubkeyname);	
					}
					else if (sButton[ubKeyId].ubLPRepeatEn && sButton[ubKeyId].ubLPRepeatTime) {
						if (POWER_KEY_LONG_PRESS_CNT < sButton[ubKeyId].ulKeyCount) {
							if (0 == ((sButton[ubKeyId].ulKeyCount - POWER_KEY_LONG_PRESS_CNT) % (POWER_KEY_LONG_PRESS_CNT * sButton[ubKeyId].ubLPRepeatTime))) {
					        	#if defined(WIFI_PORT) && (WIFI_PORT == 1) && defined(CFG_WIFI_STREAMING_DISABLE_REC_KEY)
					        	if ((AHC_STREAM_OFF != AHC_GetStreamingMode()) && 
									#if (POWER_OFF_CONFIRM_PAGE_EN)
									(PowerOff_InProcess == AHC_FALSE) &&
									#endif
						        	(KEY_VIDEO_RECORD == sButton[ubKeyId].iLongPressId))
						        {
									continue;
								}
								#endif
							
								SetKeyPadEvent(sButton[ubKeyId].iLongPressId);
								printc("Repeat Long Press Key %s\r\n", sButton[ubKeyId].ubkeyname);	
							}
						}
					} 	                    		
				}
				else
				{
					if(sButton[ubKeyId].ulKeyCount==OTHER_KEY_LONG_PRESS_CNT)
					{
			        	#if defined(WIFI_PORT) && (WIFI_PORT == 1) && defined(CFG_WIFI_STREAMING_DISABLE_REC_KEY)
			        	if ((AHC_STREAM_OFF != AHC_GetStreamingMode()) && 
							#if (POWER_OFF_CONFIRM_PAGE_EN)
							(PowerOff_InProcess == AHC_FALSE) &&
							#endif
			        	    (KEY_VIDEO_RECORD == sButton[ubKeyId].iLongPressId))
			        	{
							continue;
						}
						#endif
							
						SetKeyPadEvent(sButton[ubKeyId].iLongPressId);
						AHC_SetButtonStatus(sButton[ubKeyId].iLongPressId);
						SetComboKey(sButton[ubKeyId].ulKeyFlag << 16);
						printc("2Long Press Key %s\r\n",sButton[ubKeyId].ubkeyname);
					}	 						
					else if (sButton[ubKeyId].ubLPRepeatEn && sButton[ubKeyId].ubLPRepeatTime) {
						if (OTHER_KEY_LONG_PRESS_CNT < sButton[ubKeyId].ulKeyCount) {
							if (0 == (sButton[ubKeyId].ulKeyCount % OTHER_KEY_LONG_PRESS_CNT)) {
					        	#if defined(WIFI_PORT) && (WIFI_PORT == 1) && defined(CFG_WIFI_STREAMING_DISABLE_REC_KEY)
					        	if ((AHC_STREAM_OFF != AHC_GetStreamingMode()) && 
									#if (POWER_OFF_CONFIRM_PAGE_EN)
									(PowerOff_InProcess == AHC_FALSE) &&
									#endif
					        	    (KEY_VIDEO_RECORD == sButton[ubKeyId].iLongPressId))
					        	{
									continue;
								}
								#endif
									
								SetKeyPadEvent(sButton[ubKeyId].iLongPressId);
								printc("Repeat Long Press Key %s\r\n", sButton[ubKeyId].ubkeyname);	
							}
						}
					} 	                    		
				}
            }
            //break;
		}
		else if( tempValue != sButton[ubKeyId].ulKeyLastValue &&      // Check if the key is lost
				 ulNow - sButton[ubKeyId].ulKeyLastTime > sButton[ubKeyId].ulDebounceTime )
		{
			// If state change
			if( tempValue == sButton[ubKeyId].ulPressLevel )
			{
	        	#if defined(WIFI_PORT) && (WIFI_PORT == 1) && defined(CFG_WIFI_STREAMING_DISABLE_REC_KEY)
	        	if ((AHC_STREAM_OFF != AHC_GetStreamingMode()) && 
					#if (POWER_OFF_CONFIRM_PAGE_EN)
					(PowerOff_InProcess == AHC_FALSE) &&
					#endif
	        		(KEY_VIDEO_RECORD == sButton[ubKeyId].iPressId))
	        	{
					continue;
				}
				#endif
					
				sButton[ubKeyId].ubLongPress = 0;
				SetKeyPadEvent(sButton[ubKeyId].iPressId);          	
				AHC_SetButtonStatus(sButton[ubKeyId].iPressId);
				SetComboKey(sButton[ubKeyId].ulKeyFlag);      	
				printc("Press Key %s\r\n",sButton[ubKeyId].ubkeyname);                    
			}
			else
			{
				if ((!sButton[ubKeyId].ubLongPress) ||
				    ((KEY_LPRESS_POWER == sButton[ubKeyId].iLongPressId) && (POWER_KEY_LONG_PRESS_CNT > sButton[ubKeyId].ulKeyCount)) ||
					((KEY_LPRESS_POWER != sButton[ubKeyId].iLongPressId) && (OTHER_KEY_LONG_PRESS_CNT > sButton[ubKeyId].ulKeyCount)))
				{
		        	#if defined(WIFI_PORT) && (WIFI_PORT == 1) && defined(CFG_WIFI_STREAMING_DISABLE_REC_KEY)
		        	if ((AHC_STREAM_OFF != AHC_GetStreamingMode()) && 
						#if (POWER_OFF_CONFIRM_PAGE_EN)
						(PowerOff_InProcess == AHC_FALSE) &&
						#endif
			        	(KEY_VIDEO_RECORD == sButton[ubKeyId].iReleaseId))
			        {
						continue;
					}
					#endif
						
					SetKeyPadEvent(sButton[ubKeyId].iReleaseId);
					ClearComboKey(sButton[ubKeyId].ulKeyFlag);	
					printc("Release Key %s\r\n",sButton[ubKeyId].ubkeyname);
				}
				else
				{
		        	#if defined(WIFI_PORT) && (WIFI_PORT == 1) && defined(CFG_WIFI_STREAMING_DISABLE_REC_KEY)
		        	if ((AHC_STREAM_OFF != AHC_GetStreamingMode()) && 
						#if (POWER_OFF_CONFIRM_PAGE_EN)
						(PowerOff_InProcess == AHC_FALSE) &&
						#endif
		        		(KEY_VIDEO_RECORD == sButton[ubKeyId].iLongReleaseId))
		        	{
						continue;
					}
					#endif
						
					sButton[ubKeyId].ubLongPress = 0;
					SetKeyPadEvent(sButton[ubKeyId].iLongReleaseId);	
					ClearComboKey(sButton[ubKeyId].ulKeyFlag << 16);
					printc("Release Long Key %s\r\n",sButton[ubKeyId].ubkeyname);
					Poweranxiajishu = 0; // lyj 20190814
				}
						
				sButton[ubKeyId].ulKeyCount = 0;
			}
			
			sButton[ubKeyId].ulKeyLastValue = tempValue;
			sButton[ubKeyId].ulKeyLastTime  = ulNow;
            //break;
        }
    }

	if (gTmKeypadHook)
		(TMHOOK)gTmKeypadHook();
		
#ifdef CFG_KEY_MEASURE_ADX2003 //may be defined in config_dv178.h
	// Clear ADX2003 power key status
	ADX2003_Measure_Power_Key();
#endif
}
void IR_Key_Detection(void)
{
#if (SUPPORT_IR_CONVERTER)
	MMP_UBYTE	ubKeyId;
	MMP_UBYTE 	i;
	int			ir_code;
	MMP_ULONG	uiLCDstatus;
	static int	int_cnt = 0;
	static int	pre_ubKeyId = -1;
	static int	idle_cnt = 0;
		
    if (!AHC_KeyUIWorking())
    	return;
    	
	AHC_GetParam(PARAM_ID_LCD_STATUS, &uiLCDstatus);
	ir_code = IR_CheckInterruptGpio();
	
	if ((ir_code != 0) && ((ir_code >> 16) != 0xff)) {
		printc("GOT IR 0x%08x, \r\n", ir_code);
		int_cnt++;
		idle_cnt = 0;

		for(i=0; i<NUM_IR_KEY; i++) {
			ubKeyId = (!uiLCDstatus)?(i):(sIRButton[i].ubKeyPairId);

			if(ubKeyId == IR_STATE_ID_INVALID)
				ubKeyId = i;

			if (sIRButton[ubKeyId].ulKeyValue == (unsigned short)ir_code /* little endian */) {
				if (pre_ubKeyId != ubKeyId) {
					if (pre_ubKeyId != -1) {
						printc("1. IR RELEASE - %s\r\n", sIRButton[(MMP_UBYTE) pre_ubKeyId].ubkeyname);
						SetKeyPadEvent(sIRButton[pre_ubKeyId].iReleaseId);
					}

					SetKeyPadEvent(sIRButton[ubKeyId].iPressId);
					pre_ubKeyId = ubKeyId;
					printc("IR Key Press %s Sent %d\r\n", sIRButton[ubKeyId].ubkeyname, sIRButton[ubKeyId].iPressId);
					int_cnt = 1;
				}
				else
				{ 
					if (int_cnt == 1) {
						if (pre_ubKeyId != -1) {
							printc("2. IR RELEASE - %s\r\n", sIRButton[(MMP_UBYTE) pre_ubKeyId].ubkeyname);
							SetKeyPadEvent(sIRButton[pre_ubKeyId].iReleaseId);
						}

						SetKeyPadEvent(sIRButton[ubKeyId].iPressId);
						pre_ubKeyId = ubKeyId;
						printc("IR Key Press %s Sent %d\r\n", sIRButton[ubKeyId].ubkeyname, sIRButton[ubKeyId].iPressId);
					}
					else if ((int_cnt % 10) == 0)
					{
						SetKeyPadEvent(sIRButton[ubKeyId].iLongPressId);
						printc("IR Key LongPress %s Sent %d\r\n", sIRButton[ubKeyId].ubkeyname, sIRButton[ubKeyId].iLongPressId);
					}
				}
			}
		}
	} 
	else {
		if (int_cnt != 0)
		{
			if ((pre_ubKeyId != -1) && (idle_cnt > 4))
			{
				printc("3. IR RELEASE - %s\r\n", sIRButton[pre_ubKeyId].ubkeyname);
				SetKeyPadEvent(sIRButton[pre_ubKeyId].iReleaseId);

				pre_ubKeyId = -1;
				idle_cnt = 0;
				int_cnt = 0;
			}
			else	
				idle_cnt++;		
		}
	}
#endif
}


//PC/adaptor plugs in/out.
void USB_Cable_In(UINT8 in)
{
    //case USB_CABLE_IN:
    if(in){
        VideoPowerOffCounterReset();

        if(AHC_GetShutdownByChargerOut()==AHC_TRUE)
            AHC_SetShutdownByChargerOut(AHC_FALSE);

        SetKeyPadEvent(USB_CABLE_IN);
    }
    else{
        SetKeyPadEvent(USB_CABLE_OUT);
    }
}

void USB_BDevice_In(UINT8 in)
{
    AHC_USB_PauseDetection(1);
    
    if(in){
        AHC_SetCurKeyEventID(BUTTON_USB_B_DEVICE_DETECTED);
        AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_USB_B_DEVICE_DETECTED, 0);
    }
    else{
        AHC_SetCurKeyEventID(BUTTON_USB_B_DEVICE_REMOVED);
        AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_USB_B_DEVICE_REMOVED, 0);
    }
}


void Device_Detection(void)
{
    UINT32 		i;
    MMP_ULONG 	ulNow;
    UINT32		uiLCDstatus;
    static MMP_UBYTE ubDetectCnt=0;

    if(!gAHC_InitialDone)
        return;

    ubDetectCnt = (ubDetectCnt+1)%5;
    
    if(ubDetectCnt>0)
        return;
    
    MMPF_OS_GetTime(&ulNow);   
    AHC_GetParam(PARAM_ID_LCD_STATUS,&uiLCDstatus);
    
    for( i=0; i<gNumOfDevice; i++ )
    {
        MMP_UBYTE tempValue = 0;
        
		if(sDeviceStatus[i].piopin == MMP_GPIO_MAX) {
		    if (sDeviceStatus[i].iPressId == DEVICES_READY) {
		    	sDeviceStatus[i].iPressId = 0;
		    	// Send an event after ALL of devices initialized
		    	SetKeyPadEvent(DEVICES_READY);
		    }
			continue;
		}
        
        MMPS_PIO_GetData( sDeviceStatus[i].piopin, &tempValue);

        if( sDeviceStatus[i].ulKeyLastValue == tempValue )
        {
            //NOP.  Keep the current status.
            sDeviceStatus[i].ulKeyLastTime = ulNow;
        }
        else if ( (ulNow - sDeviceStatus[i].ulKeyLastTime) > sDeviceStatus[i].ulDebounceTime )
        {
            // If state change
            #ifdef CFG_KEY_DO_COVER_ACTION //may be defined in config_xxx.h
            if (sDeviceStatus[i].iPressId == DEVICE_LCD_COVERED) {
            	doCoverAction(&sDeviceStatus[i], tempValue, ulNow);
            	continue;
            }
			#endif
            
            if( tempValue == sDeviceStatus[i].ulPressLevel )
            {
            	switch(sDeviceStatus[i].iPressId)
            	{
            		case DC_CABLE_IN:
            			VideoPowerOffCounterReset();
            			
            			if(AHC_GetShutdownByChargerOut()==AHC_TRUE)
            				AHC_SetShutdownByChargerOut(AHC_FALSE);

            			SetKeyPadEvent(sDeviceStatus[i].iPressId);
            		break;

            		#if (FLIP_CTRL_METHOD & CTRL_BY_HALL_SNR) 
            		case DEVICE_LCD_INV:
            			AHC_SetRotateSrc(SRC_HALL_SNR);
            			if(MenuSettingConfig()->uiLCDRotate==LCD_ROTATE_ON)
            				SetKeyPadEvent(DEVICE_LCD_NOR);
            			else
            				SetKeyPadEvent(DEVICE_LCD_INV);
            		break;
            		#endif
            		
            		default:
            			SetKeyPadEvent(sDeviceStatus[i].iPressId);
            		break;
            	}
            	
                printc("@@@ Device %s is plugged in\r\n",sDeviceStatus[i].ubkeyname);
            }
            else
            {
            	switch(sDeviceStatus[i].iReleaseId)
            	{
            		#if (FLIP_CTRL_METHOD & CTRL_BY_HALL_SNR) 
            		case DEVICE_LCD_NOR :
            			AHC_SetRotateSrc(SRC_HALL_SNR);
	            		if(MenuSettingConfig()->uiLCDRotate==LCD_ROTATE_ON)
	            			SetKeyPadEvent(DEVICE_LCD_INV);
	            		else
	            			SetKeyPadEvent(DEVICE_LCD_NOR);
	            	break;
	            	#endif
	            	
	            	default:
	            		SetKeyPadEvent(sDeviceStatus[i].iReleaseId);
	            	break;
            	}
               
                printc("@@@ Device %s is plugged out\r\n",sDeviceStatus[i].ubkeyname);                    
            }

            sDeviceStatus[i].ulKeyLastValue = tempValue;
            sDeviceStatus[i].ulKeyLastTime  = ulNow;
        }
    }
}

void ADC_AUX1_Detection_Once(void)
{	

#if (BATTERY_DETECTION_EN)
	MMP_UBYTE i = 0;
	MMP_ULONG ulVoltageLevelSum	   	 = 0;
	MMP_ULONG ulBatteryDetectCounter = 0;

    if(!gAHC_InitialDone)
        return;
    
	for (i = 0; i < 4; i++)
	{
        MMP_ULONG ulLevel = 0;
        MMP_UBYTE ubSource = 0;

        AHC_PMUCtrl_ADC_Measure_Key(&ulLevel, &ubSource, BATTERY_DETECT_ADC_CHANNEL);
        ulVoltageLevelSum += ulLevel * REAL_BAT_VOLTAGE_RATIO_M / REAL_BAT_VOLTAGE_RATIO_N;
        ulBatteryDetectCounter++;
        MMPF_OS_Sleep_MS(10);
	}
	
	if (ulBatteryDetectCounter >= 4)
	{	  
		ulVoltageLevelSum /= ulBatteryDetectCounter;

		if(ulVoltageLevelSum > BATT_FULL_LEVEL)
			MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_FULL;
		else if((ulVoltageLevelSum > BATT_LEVEL_2) && (ulVoltageLevelSum <= BATT_FULL_LEVEL))
			MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_LEVEL_2;
		else if((ulVoltageLevelSum > BATT_LEVEL_1) && (ulVoltageLevelSum <= BATT_LEVEL_2))
			MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_LEVEL_1;
		else if((ulVoltageLevelSum > BATT_LEVEL_0) && (ulVoltageLevelSum <= BATT_LEVEL_1))
			MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_LEVEL_0;
		else if((ulVoltageLevelSum <= BATT_LEVEL_0)) 
			MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_EMPTY;	

		SetKeyPadEvent(BUTTON_BATTERY_DETECTION);
	}
#else
	MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_FULL;
#endif
}


void ADC_AUX1_Detection(void)
{	
    MMP_UBYTE battery_level_temp = 0;
	//UINT32 level = 0;
    //UINT8 source;
    if(!gAHC_InitialDone)
        return;

	// AHC_PMUCtrl_ADC_Measure_Key(&level, &source, BATTERY_DETECT_ADC_CHANNEL);

  

// do not controlled by BATTERY_DETECTION_EN
// Just test SAR-ADC function
//	printc("---level---source--------------\r\n");
#if 1//(ENABLE_BATTERY_DET_DEBUG)
    {
        static skipMonitorTimes = BATTERY_DETECT_PERIOD >> 1;
        static preBatteryVoltage = 0;
        UINT32 level = 0;
        UINT8 source;

        if(skipMonitorTimes > 0xF0)
        {
            skipMonitorTimes++;
        }
        else if (skipMonitorTimes > 0)
        {
            skipMonitorTimes--;
            return;
        }
        else
        {
            skipMonitorTimes = BATTERY_DETECT_PERIOD >> 1;
        }

        AHC_PMUCtrl_ADC_Measure_Key(&level, &source, BATTERY_DETECT_ADC_CHANNEL);
		//printc("---level=%d---source=%d---------------\r\n",level,source);
		  	 pf_General_EnSet(COMMON_KEY_B_VOL,level);
        level = level * REAL_BAT_VOLTAGE_RATIO_M / REAL_BAT_VOLTAGE_RATIO_N;
       // printc("Monitor Battery Level = %d\r\n", level);
        if (preBatteryVoltage != level) {
        //    printc("Monitor Battery Level = %d\r\n", level); long 4-25
            preBatteryVoltage = level;
        }
    }
#endif

#if 0

#if (BATTERY_DETECTION_EN == 0)
    return;
#endif
#if 0
#ifdef CFG_DRAW_ALWAYS_FULL_CHARGE_ICON
	if (AHC_TRUE == AHC_Charger_GetStatus()) {
		uVoltageLevelSum = BATTERY_VOLTAGE_FULL;
		MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_FULL;
		uBatteryDetectCounter = 0;
        uSkipDetectNum = 0;
		#if (OSD_SHOW_BATTERY_STATUS==1)
		AHC_SetChargingID(3);
		#endif
		
		return;
	}
#elif (CHARGE_FULL_NOTIFY==0)
    if(AHC_TRUE == AHC_Charger_GetStatus())
    {
        uSkipDetectNum = 0;
        MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_CHARGING; //TBD
        return;
    }
#endif

    if(uSkipDetectNum > 0xF0)
    {    
        uSkipDetectNum++;
    }
    else if(uSkipDetectNum > 0)
    {
        uSkipDetectNum--;
        return;
    }
    else
    {    
        uSkipDetectNum = BATTERY_DETECT_PERIOD;
    }

#if (CHARGE_FULL_NOTIFY)

    if(AHC_TRUE == AHC_Charger_GetStatus() || UI_MSDC_STATE == uiGetCurrentState() || UI_PCCAM_STATE == uiGetCurrentState())
    {
        MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_CHARGING; //TBD

		{
			MMP_UBYTE ubChargeStatus = !CHARGE_FULL_STATUS;
			
            #ifdef GPIO_CHARGE_STATUS
            if (GPIO_CHARGE_STATUS != MMP_GPIO_MAX) {
                AHC_GPIO_ConfigPad(GPIO_CHARGE_STATUS, PAD_OUT_DRIVING(0) | PAD_SCHMITT_TRIG | PAD_PULL_UP);
                AHC_GPIO_SetOutputMode(GPIO_CHARGE_STATUS, MMP_FALSE);
                AHC_GPIO_GetData(GPIO_CHARGE_STATUS, &ubChargeStatus);
            }
			#endif
			
			if(ubChargeStatus != CHARGE_FULL_STATUS)
			{
				bChargeFull = AHC_FALSE;
				#if OSD_SHOW_BATTERY_STATUS
				AHC_EnableChargingIcon(AHC_TRUE);
				#endif
			}
			else
			{	
				if(!bChargeFull)
				{
					printc(FG_GREEN("CHARGING COMPLETE\r\n"));

					#ifdef CHARGE_FULL_LED
					/* When USB plug out to OFF it? */
					LedCtrl_ButtonLed(CHARGE_FULL_LED, AHC_TRUE);
					#endif
					#if (OSD_SHOW_BATTERY_STATUS==1)
					AHC_SetChargingID(3);
					#endif
					
					#if (OSD_SHOW_BATTERY_STATUS==0)
					bChargeFull = AHC_TRUE;
					#else
					/*
					if(AHC_FALSE==AHC_DrawChargingIcon())//Return AHC_FALSE means not use task to drawing charging icons. 
					{
						MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_FULL;
						SetKeyPadEvent(BUTTON_BATTERY_DETECTION);
					}
					*/
					bChargeFull = AHC_TRUE;
					AHC_EnableChargingIcon(AHC_FALSE);
					#endif
				}
			}
		}

        return;
    }
#endif

#endif

#if 1
    {
        UINT32 level = 0;
        UINT8 source;

        AHC_PMUCtrl_ADC_Measure_Key(&level, &source, BATTERY_DETECT_ADC_CHANNEL);
		
        level = level * REAL_BAT_VOLTAGE_RATIO_M / REAL_BAT_VOLTAGE_RATIO_N;
        battery_level_temp = MenuSettingConfig()->uiBatteryVoltage;

        if ((MenuSettingConfig()->uiBatteryVoltage == BATTERY_VOLTAGE_EMPTY) ||
            (MenuSettingConfig()->uiBatteryVoltage == BATTERY_VOLTAGE_LEVEL_0))
        {
            uBatteryDetectCounter += 2;
            uVoltageLevelSum += 2*level;
            
        }
        else
        {
            uBatteryDetectCounter++;
            uVoltageLevelSum += level;
        }

        //uVoltageLevelSum += level;
        
        if (uBatteryDetectCounter >= 4)
        {
            uVoltageLevelSum = uVoltageLevelSum / uBatteryDetectCounter;

            if(uVoltageLevelSum > BATT_FULL_LEVEL)
                MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_FULL;
            else if((uVoltageLevelSum > BATT_LEVEL_2) && (uVoltageLevelSum <= BATT_FULL_LEVEL))
                MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_LEVEL_2;
            else if((uVoltageLevelSum > BATT_LEVEL_1) && (uVoltageLevelSum <= BATT_LEVEL_2))
                MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_LEVEL_1;
            else if((uVoltageLevelSum > BATT_LEVEL_0) && (uVoltageLevelSum <= BATT_LEVEL_1))
                MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_LEVEL_0;
            else if((uVoltageLevelSum <= BATT_LEVEL_0)) 
            {
                RTNA_DBG_Str(0, "\r\n--W-- Low Battery !!!\r\n \
                                 --W-- Low Battery !!!\r\n \
                                 --W-- Low Battery !!!\r\n\r\n");

#ifdef CFG_POWER_CUS_EMPTY_BATTERY_FUNC //may be defined in config_xxx.h
                CFG_POWER_CUS_EMPTY_BATTERY_FUNC();
#else
				#ifdef CFG_BATTERY_EMPTY_IMMEDIATE_POWER_OFF
				// Do not check MenuSettingConfig()->uiBatteryVoltage
				#else
                if(MenuSettingConfig()->uiBatteryVoltage == BATTERY_VOLTAGE_EMPTY)
                #endif
                {
                	extern void uiBlockEvent(int arg);

					// Stop video record ASAP
					if (VideoFunc_RecordStatus())
					{
						StateVideoRecMode_StopRecordingProc(EVENT_VIDEO_KEY_RECORD);
						AHC_VIDEO_WaitVideoWriteFileFinish();
					}

					uiBlockEvent(1);	// In Power Off progress, not to service any message any more.
				    RTNA_DBG_Str(0, "\r\nSystem will be shutdown ...\r\n");

					// Waiting for UI idle - In DSC Capture will cause system dead lock
					while (0 == AHC_KeyUIWorking()) {
						AHC_OS_SleepMs(100);
						RTNA_DBG_Str(0, "z");
					}

                	AHC_WMSG_Draw(AHC_TRUE, WMSG_GOTO_POWER_OFF, BATTERY_LOW_WARNNING_TIME);  // liao
                	AHC_OS_SleepMs(CFG_BATTERY_EMPTY_WARNING_DELAY_TIME);

					// Waiting for UI idle - In DSC Capture will cause system dead lock
					while (0 == AHC_KeyUIWorking()) {
						AHC_OS_SleepMs(100);
						RTNA_DBG_Str(0, "z");
					}
						
    				// printc("BATTERY_VOLTAGE_EMPTY. Turn off system.\r\n");
                    // Call Power Off directly, because to send a POWER OFF message may not be serviced!
                    // when there are some messages are waiting in queue!!
                    AHC_PowerOff_NormalPathEx(AHC_TRUE, AHC_TRUE, AHC_TRUE);  //liao
                    //SetKeyPadEvent(KEY_POWER_OFF);
                }

                MenuSettingConfig()->uiBatteryVoltage = BATTERY_VOLTAGE_EMPTY;
#endif
            }

#ifdef CFG_CUS_BAT_SHOW_BY_LED_FUNC
			CFG_CUS_BAT_SHOW_BY_LED_FUNC();
#endif

#if (ENABLE_BATTERY_DET_DEBUG)
		    printc("\r\nMenuSettingConfig()->uiBatteryVoltage = %d\r\n", MenuSettingConfig()->uiBatteryVoltage);
		    printc("Average Battery Level = %d\r\n\r\n", uVoltageLevelSum);
#endif
            
            if((MenuSettingConfig()->uiBatteryVoltage != BATTERY_VOLTAGE_EMPTY) && (battery_level_temp != MenuSettingConfig()->uiBatteryVoltage))
            {
                SetKeyPadEvent(BUTTON_BATTERY_DETECTION);
            }

			uVoltageLevelSum = 0;
			uBatteryDetectCounter = 0;
        }
    } 
#endif
#endif
}
//int countADC = 0; // lyj 20181228
void ADC_AUX2_Detection(void)
{
#if (ADC_KEY_EN==1)

    MMP_USHORT 	saradc_data = 0;
	UINT32		uiLCDstatus;
	MMP_UBYTE 	i, ubOutRegionCnt = 0;
	
    if (!AHC_KeyUIWorking())
    	return;

    AHC_SARADC_ReadData(ADC_KEY_SARADC_CHANNEL, &saradc_data);

    //Print below to check ADC key voltage, ex: ADC_KEY_VOLT_REC
   //printc("ADC voltage = %d\r\n", saradc_data);
   
    
    AHC_GetParam(PARAM_ID_LCD_STATUS, &uiLCDstatus);

    for (i = 0; i < gNumOfADCKey; i++)
    {
        MMP_ULONG UpBound  = sADCButton[i].ulKeyVolt + sADCButton[i].ulKeyMargin;
        MMP_ULONG LowBound = sADCButton[i].ulKeyVolt - sADCButton[i].ulKeyMargin;
        MMP_UBYTE ubKeyId  = i;
        
        if (sADCButton[i].ubKeyStateId == ADCPRESS_NONE)
        {
            ubOutRegionCnt++;
            continue;
        }
        
        if (sADCButton[i].ulKeyVolt < sADCButton[i].ulKeyMargin)
            LowBound = 0;
        
        if ( saradc_data <= UpBound && saradc_data >= LowBound )
        {
        
#if (HALL_SNR_FLIP_SELECT & FLIP_OSD) || (KEY_FLIP_SELECT & FLIP_OSD) || (KEYPAD_ROTATE_EN==1)      
            if (gADCKeyLongPress == AHC_FALSE)
                ubKeyId = (!uiLCDstatus)?(i):(sADCButton[i].ubKeyPairId);
            else
                ubKeyId	= gADCKeyStatus;
#endif
            
            if (ubKeyId == ADC_STATE_ID_INVALID)
                ubKeyId = i;

#if (DIR_KEY_CYCLIC_LPRESS_EN)
            if ((sADCButton[ubKeyId].ubKeyStateId <= ADCPRESS_R) && //For Direction Key
                (sADCButton[ubKeyId].ubKeyStateId > ADCPRESS_U || sADCButton[ubKeyId].ubKeyStateId == ADCPRESS_U))
            {
				if((gADCKeyStatus != sADCButton[ubKeyId].ubKeyStateId) || 
				  ((gADCKeyStatus == sADCButton[ubKeyId].ubKeyStateId) && (gADCKeyPressCnt == 0)) )
				{
					gADCKeyStatus 	= sADCButton[ubKeyId].ubKeyStateId;
					gADCKeyPressCnt = 0;
					printc("1111ADC Press %s %d\r\n",sADCButton[ubKeyId].ubkeyname, sADCButton[ubKeyId].ubKeyCount);
					SetKeyPadEvent(sADCButton[ubKeyId].iPressId);
					AHC_SetButtonStatus(sADCButton[ubKeyId].iPressId);
					SetComboKey(sADCButton[ubKeyId].ulKeyFlag);
		        }
		        
				if((gADCKeyStatus == sADCButton[ubKeyId].ubKeyStateId) 	&&
					(ADC_KEY_LONG_PRESS_CNT*2==sADCButton[ubKeyId].ubKeyCount) )
				{
					gADCKeyLongPress = MMP_TRUE;
					printc("222ADC LPress %s %d\r\n",sADCButton[ubKeyId].ubkeyname, sADCButton[ubKeyId].ubKeyCount);
					SetKeyPadEvent(sADCButton[ubKeyId].iLongPressId);
					AHC_SetButtonStatus(sADCButton[ubKeyId].iLongPressId);
					SetComboKey(sADCButton[ubKeyId].ulKeyFlag << 16);
				}
			}
			else
#endif			
			{
				if(gADCKeyStatus != sADCButton[ubKeyId].ubKeyStateId)
				{
					gADCKeyStatus 	= sADCButton[ubKeyId].ubKeyStateId;
					gADCKeyPressCnt = 0;
					printc("3333ADC Press %s %d\r\n",sADCButton[ubKeyId].ubkeyname, sADCButton[ubKeyId].ubKeyCount);
					SetKeyPadEvent(sADCButton[ubKeyId].iPressId);
					AHC_SetButtonStatus(sADCButton[ubKeyId].iPressId);
					SetComboKey(sADCButton[ubKeyId].ulKeyFlag);
		        }
				else if((gADCKeyStatus == sADCButton[ubKeyId].ubKeyStateId) 	&&
						(ADC_KEY_LONG_PRESS_CNT*2==sADCButton[ubKeyId].ubKeyCount) )
				{
					gADCKeyStatus 	 = sADCButton[ubKeyId].ubKeyStateId;
					gADCKeyLongPress = MMP_TRUE;
					printc("444ADC LPress %s %d\r\n",sADCButton[ubKeyId].ubkeyname, sADCButton[ubKeyId].ubKeyCount);
					SetKeyPadEvent(sADCButton[ubKeyId].iLongPressId);
					AHC_SetButtonStatus(sADCButton[ubKeyId].iLongPressId);
					SetComboKey(sADCButton[ubKeyId].ulKeyFlag << 16);
				}
			}
	
			gADCKeyPressCnt = (gADCKeyPressCnt + 1) % ADC_KEY_LONG_PRESS_CNT;  
			sADCButton[ubKeyId].ubKeyCount++;
            break;
		}
		else
		{
			ubOutRegionCnt++;
		}
	}

	if(ubOutRegionCnt==gNumOfADCKey && gADCKeyStatus!=ADCPRESS_MAX && gADCKeyStatus!=ADCPRESS_NONE)
	{
		printc("ADC Release %s\r\n",sADCButton[gADCKeyStatus].ubkeyname);
		
		if(gADCKeyLongPress)
		{
			SetKeyPadEvent(sADCButton[gADCKeyStatus].iLongReleaseId);
			ClearComboKey(sADCButton[gADCKeyStatus].ulKeyFlag << 16);
		}
		else
		{		
			SetKeyPadEvent(sADCButton[gADCKeyStatus].iReleaseId);
			ClearComboKey(sADCButton[gADCKeyStatus].ulKeyFlag);
		}
		
		sADCButton[gADCKeyStatus].ubKeyCount = 0;
		gADCKeyStatus 		= ADCPRESS_NONE;
		gADCKeyPressCnt		= 0;
		gADCKeyLongPress 	= MMP_FALSE;
		ubOutRegionCnt		= 0;
    }    
        
#endif    	
}

void ADC_BatteryTemperature_Detection(void)
{
#if 0//(MENU_BLUETOOTH_PROGRESS_BAR)
	extern UINT16 wDispID;
	 char    szv[16];
	 RECT tmpRECT1;
	AHC_RTC_TIME 	CurRtctime;
	GUI_COLOR Color_1;
	UINT16 wSecond;


//--------------------------------------------

	//	if(wDispID!=0)
		{
		AHC_RTC_GetTime(&CurRtctime);
		wSecond=CurRtctime.uwSecond;
	//	printc("---------time=%d---------\r\n",wSecond);
		/*
		AHC_OSDSetFont(wDispID, &GUI_Font20B_1);
		AHC_OSDDrawDec( wDispID,wSecond, 240,40, 2);
		DrawAudPb_ProgressBar_Menu(wDispID, wSecond, 60, AHC_FALSE);
*/
		sprintf(szv, "%dV", (INT32)wSecond);
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
		case RADIO_WB_FLAG :       Color_1 = OSD_COLOR_WB;  break;
		case LIGHT_BAR_FLAG :       Color_1 = OSD_COLOR_BAR;  break;
		case AUX_FLAG :                 Color_1 = OSD_COLOR_AUX;  break;
		case USB_FLAG : 			 Color_1 = OSD_COLOR_USB;  break;
		case RGB_FLAG :  			Color_1 = OSD_COLOR_MAIN_PAGE;  break;
		case BRIGHTNESS_FLAG :     Color_1 = OSD_COLOR_BRIGHTNESS;  break;
	}
	
	AHC_OSDSetColor(wDispID, Color_1); 
	AHC_OSDSetFont(wDispID, &GUI_Font20_1);
       AHC_OSDDrawFillRect(wDispID, tmpRECT1.uiLeft, tmpRECT1.uiTop, 475, 90);
	OSD_ShowString( wDispID,szv, NULL, tmpRECT1, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);

	}
#endif

//--------------------------------------------
#ifdef CFG_MONITOR_BATTERY_TEMPERATURE
    static MMP_ULONG uBatteryTempDetectCounter = 0;
    static MMP_ULONG uTempVoltageLevelSum = 0;
    static MMP_UBYTE uSkipTempDetectNum = (0xFF - 0x04);
    MMP_USHORT saradc_data = 0;

    if(!gAHC_InitialDone)
        return;

    if (uSkipTempDetectNum > 0xF0)
    {    
        uSkipTempDetectNum++;
    }
    else if (uSkipTempDetectNum > 0)
    {
        uSkipTempDetectNum--;

        return;
    }
    else
    {    
        uSkipTempDetectNum = BATTERY_DETECT_PERIOD;
    }

    AHC_SARADC_ReadData(TEMPERATURE_SARADC_CHANNEL, &saradc_data);
	if (saradc_data)
        saradc_data = (UINT32) saradc_data * (UINT32) SARADC_REFERENCE_VOLTAGE / (UINT32) SARADC_FULL_SCALE;

    uBatteryTempDetectCounter++;
    uTempVoltageLevelSum += saradc_data;

    if (uBatteryTempDetectCounter >= 4)
    {
        uTempVoltageLevelSum = uTempVoltageLevelSum / uBatteryTempDetectCounter;

        #if (ENABLE_BATTERY_TEMP_DEBUG) 
        printc("Battery Temperature Level = %d\r\n", uTempVoltageLevelSum);
        #endif
        
        #ifdef GPIO_CHARGE_ENABLE
        if (GPIO_CHARGE_ENABLE != MMP_GPIO_MAX) {
            AHC_GPIO_ConfigPad(GPIO_CHARGE_ENABLE, PAD_OUT_DRIVING(0));
            AHC_GPIO_SetOutputMode(GPIO_CHARGE_ENABLE, AHC_TRUE); 

            if (uTempVoltageLevelSum < BATTERY_HIGH_TEMP_LEVEL) {
                RTNA_DBG_Str(0, "--W-- Battery Temperature is too high !!!\r\n");
                RTNA_DBG_Str(0, "--W-- Shutdown battery charger !!!\r\n");
            	AHC_GPIO_SetData(GPIO_CHARGE_ENABLE, (0 == GPIO_CHARGE_ENABLE_LEVEL) ? 1 : 0);
            }
            else {
            	AHC_GPIO_SetData(GPIO_CHARGE_ENABLE, (0 == GPIO_CHARGE_ENABLE_LEVEL) ? 0 : 1);
            }
        }
        #endif
        
		uTempVoltageLevelSum = 0;
		uBatteryTempDetectCounter = 0;
    }
#endif
}

void UpdateVideoCurrentTimeStamp(void);
void UIKeyTask(void)
{
    MMP_ULONG      ulKeyEvent;
    MMPF_OS_FLAGS  flags;
	MMPF_OS_FLAGS  waitflags;
    MMP_ULONG      ulNow;

#if (KEYPAD_DETECT_METHOD == KEYPAD_DETECT_TASK)
    UITaskReady = MMP_TRUE;
#endif

#if (TASK_MONITOR_ENABLE)
    memcpy(&gsTaskMonitorUIKey.TaskName[0], __func__, TASK_MONITOR_MAXTASKNAMELEN);
    gsTaskMonitorUIKey.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_WAITING;
    gsTaskMonitorUIKey.ulExecTime = 0;
    memset((void *)gsTaskMonitorUIKey.ParaArray, 0x0, sizeof(gsTaskMonitorUIKey.ParaArray)); 
    gsTaskMonitorUIKey.ulParaLength = 0;    
    gsTaskMonitorUIKey.pTimeoutCB = (TASK_MONITOR_TimeoutCallback *)NULL;       
    MMPF_TaskMonitor_RegisterTask(&gsTaskMonitorUIKey);
#endif

	waitflags = CDV_KEYPAD_FLAG | CDV_TIME_FLAG;
	#if (UPDATE_UI_USE_MULTI_TASK)
	waitflags |= CDV_UI_FLAG;
	#endif
	
    while(1){
        MMPF_OS_WaitFlags(CDV_UI_Flag, waitflags, 
        MMPF_OS_FLAG_WAIT_SET_ANY|MMPF_OS_FLAG_CONSUME, 0, &flags);

#if (TASK_MONITOR_ENABLE)
        MMPF_OS_GetTime(&gsTaskMonitorUIKey.ulExecTime);
        gsTaskMonitorUIKey.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_RUNNING;    
        *(MMP_ULONG *)(gsTaskMonitorUIKey.ParaArray) = (MMP_ULONG)flags;
        gsTaskMonitorUIKey.ulParaLength = sizeof(MMP_ULONG); 
#endif

        if (flags & CDV_TIME_FLAG) {
            // The KeyTask priority is higher then NETWORK to update Video Time Stamp for better performance.
            UpdateVideoCurrentTimeStamp();
        }

		#if (UPDATE_UI_USE_MULTI_TASK)
        if (flags & CDV_UI_FLAG) {
            DrawStateVideoRecUpdate(EVENT_VIDREC_UPDATE_MESSAGE);
        }		
		#endif

        if (flags & CDV_KEYPAD_FLAG) {
            MMPF_OS_GetTime(&ulNow);

            GetKeyPadEvent(&ulKeyEvent);

#if (SUPPORT_TOUCH_PANEL)
            if(ulKeyEvent == TOUCH_PANEL_PRESS || ulKeyEvent == TOUCH_PANEL_REL)
            {
                UINT32	dir;
                MMP_UBYTE	finger;
                MMP_ERR status;

                AHC_GetParam(PARAM_ID_LCD_STATUS, &dir);
                status = AHC_TouchPanel_CheckUpdate(&ulKeyEvent, &ulNow, dir, &finger);
                /*if(finger  == FINGER_NONE) {
                    printc("Skip This KeyEvent [%d]\r\n", ulKeyEvent);
					ulKeyEvent = KEYPAD_NONE;
                }*/
            }
#endif

            if( ulKeyEvent != KEYPAD_NONE ) {
                AHC_SendAHLMessage(AHLM_GPIO_BUTTON_NOTIFICATION, ulKeyEvent, ulNow);
            }
        }

#if (TASK_MONITOR_ENABLE)
        gsTaskMonitorUIKey.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_WAITING;
#endif
    }
}

#if 0
void _____EDOG_Function_________(){ruturn;} //dummy
#endif

extern MMPF_OS_FLAGID   	UartCtrlFlag;
extern AHC_BOOL gbBlockRealIDUIKeyTask;

#if (EDOG_ENABLE == 1)
extern MMP_HANDLE        *hGps;
//Eog I/O wrap functions.
//====================================
#include "EDOG_ctl.h"

EDOG_ERR EDOGIO_FS_OPEN_Wrap(char *pFileName, char *pMode, unsigned int *ulFileID)
{
    AHC_ERR sRet = AHC_ERR_NONE;
    
    sRet = AHC_FS_FileOpen(pFileName, AHC_StrLen(pFileName), pMode, AHC_StrLen(pMode), ulFileID);
    if(sRet != AHC_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return EDOG_ERR_FS_FAIL;}   
    return EDOG_ERR_NONE;
}

EDOG_ERR EDOGIO_FS_CLOSE_Wrap(unsigned int ulFileID)
{
    AHC_ERR sRet = AHC_ERR_NONE;

    sRet = AHC_FS_FileClose(ulFileID);
    if(sRet != AHC_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return EDOG_ERR_FS_FAIL;}   
    return EDOG_ERR_NONE;    
}

EDOG_ERR EDOGIO_FS_READ_Wrap(unsigned int ulFileID, unsigned char *pData, unsigned int NumBytes, unsigned int *read_count)
{
    AHC_ERR sRet = AHC_ERR_NONE;

    sRet = AHC_FS_FileRead(ulFileID, pData, NumBytes, read_count);
    if(sRet != AHC_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return EDOG_ERR_FS_FAIL;}   
    return EDOG_ERR_NONE;    
}

EDOG_ERR EDOGIO_FS_WRITE_Wrap(unsigned int ulFileID, unsigned char *pData, unsigned int NumBytes, unsigned int *write_count)
{
    AHC_ERR sRet = AHC_ERR_NONE;

    sRet = AHC_FS_FileWrite(ulFileID, pData, NumBytes, write_count);
    if(sRet != AHC_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return EDOG_ERR_FS_FAIL;}   
    return EDOG_ERR_NONE;    
}

EDOG_ERR EDOGIO_FS_SEEK_Wrap(unsigned int ulFileID, long long llOffset, int lOrigin)
{
    AHC_ERR sRet = AHC_ERR_NONE;

    //Note 
    //EDOG_FS_SEEK_SET -> AHC_FS_SEEK_SET
    //EDOG_FS_SEEK_CUR -> AHC_FS_SEEK_CUR
    //EDOG_FS_SEEK_END -> AHC_FS_SEEK_END
    sRet = AHC_FS_FileSeek(ulFileID, llOffset, lOrigin);
    if(sRet != AHC_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return EDOG_ERR_FS_FAIL;}   
    return EDOG_ERR_NONE;    
}

EDOG_ERR EDOGIO_FS_GetFileSize_Wrap(unsigned int ulFileID, unsigned int *ulFileSize)
{
    AHC_ERR sRet = AHC_ERR_NONE;

    sRet = AHC_FS_FileGetSize(ulFileID, ulFileSize);
    if(sRet != AHC_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return EDOG_ERR_FS_FAIL;}   
    return EDOG_ERR_NONE;    
}

//pEDOGNmeaInfo EDOGIO_GPS_GetInfor_Wrap(void)
//{
//    //If customer's GPS format is not matched with pEDOGNmeaInfo, convert data here!    
//    return (pEDOGNmeaInfo)GPS_Information();
//}

EDOG_BOOL EDOGIO_GPS_IsAttached_Wrap(void)
{
    if(AHC_TRUE == AHC_GPS_Module_Attached()){
        return EDOG_TRUE;
    }
    return EDOG_FALSE;
}

void EDOGIO_LED_LcdBacklight_Wrap(AHC_BOOL bOn)
{
    LedCtrl_LcdBackLight(bOn);
}

EDOG_BOOL EDOGIO_LED_GetBacklightStatus_Wrap(void)
{
    if(AHC_TRUE == LedCtrl_GetBacklightStatus()){
        return EDOG_TRUE;
    }
    return EDOG_FALSE;
}

#if (SUPPORT_GPS == 1)
EDOG_BOOL EDOGIO_GPS_IsValidValue_Wrap(void)
{
    if(AHC_TRUE == GPSCtrl_GPS_IsValidValue())
    {
        return EDOG_TRUE;
    }
    return EDOG_FALSE;
}

void EDOGIO_RTC_GetTime_Wrap(EDOG_RTC_TIME *sEdogRTC_Time)
{
    //If customer's RTC format is not matched with EDOG_RTC_TIME, convert data here!    
    AHC_RTC_GetTime((AHC_RTC_TIME *)sEdogRTC_Time);
}

double EDOGIO_GPS_Dmm2Degree_DBL_Wrap(double dblLatLonDmm)
{
    return GpsDmmToDegree_double(dblLatLonDmm);
}

double EDOGIO_GPS_Degree2Dmm_DBL_Wrap(double dblLatLonDmm)
{
    return DegreeToGpsDmm_double(dblLatLonDmm);
}

float EDOGIO_GPS_Dmm2Degree_Float_Wrap(float dflLatLonDmm)
{
    return GpsDmmToDegree_float(dflLatLonDmm);
}

float EDOGIO_GPS_Degree2Dmm_Float_Wrap(float dflLatLonDmm)
{
    return DegreeToGpsDmm_float(dflLatLonDmm);
}

EDOG_BOOL EDOGIO_GPS_IsEof(void)
{
#if (GPS_CONFIG_NMEA_FILE)
    return GPS_IsEOF();
#else
    return EDOG_FALSE;
#endif
}

void EDOGIO_GPS_ResetEof(void)
{
#if (GPS_CONFIG_NMEA_FILE)
    GPS_ResetEOF();
#endif
}
#endif //#if (SUPPORT_GPS == 1)
#endif //#if (EDOG_ENABLE == 1)

extern MMP_USHORT m_ulPollADCIntervalInMs;
void CyclicJobTask(void)
{
	AHC_WaitForBootComplete();

	while(1) 
    {
    	ADC_AUX2_Detection();
        AHC_OS_SleepMs(m_ulPollADCIntervalInMs);
	};
}
//MMP_UBYTE LeftValue;
//	MMP_UBYTE RightValue;

//===========================================spi===========

unsigned char SPI_MISO(void)
{
	MMP_UBYTE returnValue;
	MMPF_PIO_GetData(/*MMP_GPIO21*/MMP_GPIO18, &returnValue);//MMP_GPIO19
	return returnValue;
}
void SPI_CS(/*MMP_UBYTE Value*/void)
{
	
	 //MMPF_PIO_EnableOutputMode(/*MMP_GPIO31*/MMP_GPIO19, 1, MMP_TRUE);// cs
	//MMPF_PIO_SetData(MMP_GPIO19, Value, MMP_TRUE);//cs MMP_GPIO19 MMP_GPIO17
	//printc("~CCC~~~CS = %d ~~~~~~~\r\n",Value);
	//return returnValue;
	if(str_first1[0] != 0xaa /*&& str_first1[2] != 0xf5&& str_first1[3] != 0xa7*/)
	{
		str_first1[0] = 0;
		MMPF_PIO_SetData(MMP_GPIO19, 0, MMP_TRUE);
		 //RTNA_WAIT_US(3000);
		 AHC_OS_SleepMs(200);
		 MMPF_PIO_SetData(MMP_GPIO19, 1, MMP_TRUE);
		 //printc("~reset finish ~~~~~~~\r\n");
	}
}
void SPIInit_1(void) 
{        
	MMPF_PIO_Enable(MMP_GPIO16,MMP_TRUE);
	MMPF_PIO_Enable(MMP_GPIO19,MMP_TRUE);
	MMPF_PIO_Enable(MMP_GPIO18,MMP_TRUE);
	MMPF_PIO_PadConfig(MMP_GPIO16, PAD_OUT_DRIVING(0), MMP_TRUE);
	MMPF_PIO_PadConfig(MMP_GPIO19, PAD_OUT_DRIVING(0), MMP_TRUE);
	
		
	//MMPF_PIO_PadConfig(MMP_GPIO18, PAD_OUT_DRIVING(0), MMP_TRUE)
	
	 MMPF_PIO_EnableOutputMode(MMP_GPIO16, 1, MMP_TRUE);// clk
	 MMPF_PIO_EnableOutputMode(MMP_GPIO19, 1, MMP_TRUE);// cs
	 MMPF_PIO_EnableOutputMode(MMP_GPIO18, 0, MMP_TRUE);// in
	 MMPF_PIO_SetData(MMP_GPIO16, 1, MMP_TRUE);// clk
	 MMPF_PIO_SetData(MMP_GPIO19, 1, MMP_TRUE);//cs

}
#if 0  // 从高位开始读取
unsigned char SPIReceiveByte(void)  
{     
    unsigned char serialNum = 0;  
    unsigned char dataValue=0x0;  
  
     MMPF_PIO_SetData(MMP_GPIO42, 0, MMP_TRUE);// clk     //设置SCK端口为低电平  
  //   SPI_CS(0);//cs      //设置CS端口为低电平，CS有效  
  
    for(serialNum=0;serialNum<=7;serialNum++)//以MSB方式按位接收一个字节数据,上升沿一位数据被存入移位寄存器  
    {  
        MMPF_PIO_SetData(MMP_GPIO42, 1, MMP_TRUE);// clk
        RTNA_WAIT_US(5);
        if(SPI_MISO()) dataValue|=(0x80>>serialNum);  
         MMPF_PIO_SetData(MMP_GPIO42,0, MMP_TRUE);// clk  
         RTNA_WAIT_US(1);   
    }  
  
//   SPI_CS(1);//cs     //设置CS端口为高电平，CS无效  
  
    return dataValue;  
}  
#else
int BitCount(unsigned short n)
{
    unsigned int c =0 ;
    for (c =0; n; ++c)
    {
        n &= (n -1) ; // 清除最低位的1
    }
    return c ;
}
unsigned short SPIReceiveByte(void)  
{     
    unsigned char serialNum = 0;  
    unsigned char dataValue=0;  
  
  //  MMPF_PIO_SetData(MMP_GPIO42, 0, MMP_TRUE);// clk     //设置SCK端口为低电平  
  //   SPI_CS(0);//cs      //设置CS端口为低电平，CS有效  
  
    for(serialNum=0;serialNum<8;serialNum++)//以MSB方式按位接收一个字节数据,上升沿一位数据被存入移位寄存器  
    {  
   //	 SPI_CS(0);//cs 
   	dataValue=dataValue<<1;
	
    RTNA_WAIT_US(80);//100
    MMPF_PIO_SetData(MMP_GPIO16, 0, MMP_TRUE);// clk
    RTNA_WAIT_US(170);// 250
	MMPF_PIO_SetData(MMP_GPIO16,1, MMP_TRUE);// clk 
	
	if(SPI_MISO()) dataValue|=0x01;//(0x80>>serialNum);  
        else dataValue&=0xfe;

		 //MMPF_OS_Sleep_MS(1);
		// MMPF_PIO_SetData(MMP_GPIO16,1, MMP_TRUE);
       
		//RTNA_WAIT_US(1);
      //   MMPF_PIO_SetData(MMP_GPIO42,0, MMP_TRUE);// clk  
    //      SPI_CS(1);//cs 
         //RTNA_WAIT_US(1);   
    }  
  
//   SPI_CS(1);//cs     //设置CS端口为高电平，CS无效  
  	//RTNA_WAIT_US(100000);
    return dataValue;  
}  
void SPISendByte(unsigned char sendData)  
{  
    unsigned char serialNum = 0;  
  	 unsigned char  SPI_MOSI;
   MMPF_PIO_SetData(MMP_GPIO16, 0, MMP_TRUE);// clk //设置SCK端口为低电平  
   
  
    for(serialNum=8;serialNum>=1;serialNum--)   //以MSB方式按位发送一个字节数据,上升沿一位数据被存入移位寄存器  
    {  
        SPI_MOSI = (sendData>>(serialNum-1))&0x01;  
		MMPF_PIO_SetData(MMP_GPIO17, SPI_MOSI, MMP_TRUE);//cs
        RTNA_WAIT_US(2);   
       MMPF_PIO_SetData(MMP_GPIO16, 1, MMP_TRUE);// clk 
        RTNA_WAIT_US(2);  
        MMPF_PIO_SetData(MMP_GPIO16, 0, MMP_TRUE);// clk  
        RTNA_WAIT_US(2); 
    }  
 
} 

#endif
#if 0
void Set_data_ENISR(MMP_GPIO_PIN ENpin)
{
	MMPF_PIO_PadConfig(ENpin, PAD_OUT_DRIVING(0), MMP_TRUE);
	MMPF_PIO_EnableOutputMode(ENpin, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_SetData(ENpin, MMP_TRUE, MMP_TRUE);
}
#endif
MMP_UBYTE Get_data_ENISR(MMP_GPIO_PIN ENpin,MMP_UBYTE * Value_Get)
{
	//MMPF_PIO_EnableOutputMode(ENpin, MMP_FALSE, MMP_TRUE);
	MMPF_PIO_GetData(ENpin,Value_Get);
	return (*Value_Get);

} // lyj 20181018




//=======================================end================

extern  AHC_BOOL exit_flag;

void RealIDUIKeyTask(void)
{
    MMP_UBYTE uCount;
//==========================spi==========
	 unsigned char dataValue=0;
	//  unsigned char data=0;
	 unsigned char dataValue1=0;
	//----------------
	int k;
//	 char str_first1[]={0x96,0xC6,0x96,0xcd};
	 //----------------
	MMP_UBYTE returnValue;
	MMP_UBYTE u =0;
	MMP_UBYTE Read_Data11=0;

//================================================
	
#if (GPS_CONNECT_ENABLE)
    MMP_ULONG uCountForGPSTask = 0;
#endif

    AHC_WaitForBootComplete();
	
    
    /*
     * Start to devices detect earlier to make all of devices initialized earlier,
     * to make shorter period of lcd black
     */
    Device_Detection();

    if (UITaskReady == MMP_TRUE)
    {
    #if (EDOG_ENABLE)
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_PRINTF, (void *)printc);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_OPEN, (void *)EDOGIO_FS_OPEN_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_CLOSE, (void *)EDOGIO_FS_CLOSE_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_READ, (void *)EDOGIO_FS_READ_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_WRITE, (void *)EDOGIO_FS_WRITE_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_SEEK, (void *)EDOGIO_FS_SEEK_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_GET_FILESIZE, (void *)EDOGIO_FS_GetFileSize_Wrap); 
        //EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_GETINFORMATION, (void *)EDOGIO_GPS_GetInfor_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_ISATTACHED, (void *)EDOGIO_GPS_IsAttached_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_LED_CTRL_LCDBACKLIGHT, (void *)EDOGIO_LED_LcdBacklight_Wrap);  
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_LED_CTRL_BACKLIGHT_STATUS, (void *)EDOGIO_LED_GetBacklightStatus_Wrap);          
        
        #if (SUPPORT_GPS == 1)
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_ISVALIDVALUE, (void *)EDOGIO_GPS_IsValidValue_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_DMM2DEGREE_DBL, (void *)EDOGIO_GPS_Dmm2Degree_DBL_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_DEGREE2DMM_DBL, (void *)EDOGIO_GPS_Degree2Dmm_DBL_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_DMM2DEGREE_FLOAT, (void *)EDOGIO_GPS_Dmm2Degree_Float_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_DEGREE2DMM_FLOAT, (void *)EDOGIO_GPS_Degree2Dmm_Float_Wrap);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_ISEOF, (void *)EDOGIO_GPS_IsEof);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_RESETEOF, (void *)EDOGIO_GPS_ResetEof);
        EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_RTC_GETTIME, EDOGIO_RTC_GetTime_Wrap);
        #endif

        #if (GPS_CONFIG_NMEA_FILE)
        EDOGCtrl_SetEnGpsSimulator(MMP_TRUE);
        #endif
        
        #if 1
        EDOGCtrl_SetPoiDataType(1); //1:for China
        EDOGCtrl_SetBaseName(MAPLAYOUT_BASENAME);
        EDOGCtrl_SetExtName(MAPLAYOUT_EXTNAME);
        EDOGCtrl_SetMapHeaderName(MAPLAYOUT_HEADERNAME);
        #else
        EDOGCtrl_SetPoiDataType(0); //0:for other
        EDOGCtrl_SetSpCamDBFName(SPEED_CAM_DB_FILE_NAME);
        #endif
        
        EDOGCtrl_SetDistMethod(DIST_METHOD_GBL_CIRCLE_DIST);
        EDOGCtrl_SetSpeedLvl(0);
        EDOGCtrl_SetSpeedDist(1000);
    #endif

        #if (TASK_MONITOR_ENABLE)
        memcpy(&gsTaskMonitorRealIDUIKey.TaskName[0], __func__, TASK_MONITOR_MAXTASKNAMELEN);
        gsTaskMonitorRealIDUIKey.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_WAITING;
        gsTaskMonitorRealIDUIKey.ulExecTime = 0;
        memset((void *)gsTaskMonitorRealIDUIKey.ParaArray, 0x0, sizeof(gsTaskMonitorRealIDUIKey.ParaArray)); 
        gsTaskMonitorRealIDUIKey.ulParaLength = 0;    
        gsTaskMonitorRealIDUIKey.pTimeoutCB = (TASK_MONITOR_TimeoutCallback *)NULL;       
        MMPF_TaskMonitor_RegisterTask(&gsTaskMonitorRealIDUIKey);
        #endif

		 SPIInit_1();
        
        while(1)
        {
            uCount++;

//====================spi===================

//if(Main_Get_Page() == USB_FLAG)
{
	flag_2++;
	if(flag_2==4) // 5
		{
		flag_2=0;
		flag_1=1;

	}


	if(flag_1==1)
		{

			{

			 MMPF_PIO_SetData(MMP_GPIO16, 1, MMP_TRUE);// clk 

			dataValue=SPIReceiveByte();

			 if((( u > 7) && dataValue == 0xaa&& str_first1[u -5] == 0xa7) || (dataValue == 0xaa&& str_first1[0] != 0xaa) )
			 	u = 0;
			 str_first1[u] = dataValue;
			 if(str_first1[0] == 0xaa)
			 {
				u++;
				if(u > 62)
				{
					str_first1[0] = 0;
					u = 0;
				}
			 }
			 #if 1
			 if(u > 30)
			 {
				play_time_T = 0;
				play_time_HtoL = 0;
			 }
			 if(( u > 15) &&str_first1[1] !=0 && str_first1[0] == 0xaa && str_first1[u - 2] == 0xf5  && str_first1[u -1] == 0xa7 && (str_first1[u] != 0 || str_first1[u+1] != 0))
			 {
			 	//if(str_first1[u+1] != 0 && str_first1[u+2] != 0)
			 	//{
					play_time_T = 	(str_first1[u]<<8) + str_first1[u+1] -1;
					//play_time_T = play_time_T<<8;
					//play_time_T = str_first1[u+2];

					play_time_HtoL = (str_first1[u+2]<<8) +str_first1[u+3];
					//play_time_HtoL = play_time_HtoL<<8;
					//play_time_HtoL = str_first1[u+4];
			 	//}
			 }
			 else if(( u > 5) &&str_first1[1] !=0 && str_first1[0] == 0xaa && str_first1[u - 4] == 0xf5  && str_first1[u -3] == 0xa7 && (str_first1[u-2] != 0 || str_first1[u-1] != 0))
			 {
				play_time_T = 	(str_first1[u-2]<<8) + str_first1[u-1]-1;
				play_time_HtoL = (str_first1[u]<<8) +str_first1[u+1];
			 }
			 #endif

			//if(str_first1[0] != 0xaa && str_first1[61] != 0x00&& str_first1[62] != 0x00)
			#if 0
			if(str_first1[0] != 0xaa /*&& str_first1[2] != 0xf5&& str_first1[3] != 0xa7*/)
			{
				// str_first1[61] = 0;
				// str_first1[62] = 0;
				 str_first1[0] = 0;
				u = 0;
				 SPI_CS();
			}
			#endif
			 
			 #if 0
			 if(str_first1[0] == 0xf0 && str_first1[1] != 0x00)
			 {
				ReciveDataLength = str_first1[1];
			 }
			 #endif
			 #if 0
			 if(str_first1[u -1] == 0xf5 && str_first1[u] == 0xff)
			 {
				u = 0;
			 }
			 #endif
	

			//printc("~~Get~~ == 0x%02x---> %d~\r\n",/*str_first1[u-1]*/dataValue,u-1);

			
			dataValue=0;
			dataValue1=0;

		}
	
		//SPI_CS(1);//cs  
		flag_1=0;

	}
}





//===================end=====================


			
		//-------------------螺旋编码器------------
		//--------------------------------------
            #if (TASK_MONITOR_ENABLE)
            MMPF_OS_GetTime(&gsTaskMonitorRealIDUIKey.ulExecTime);
            gsTaskMonitorRealIDUIKey.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_RUNNING;    
            *(gsTaskMonitorRealIDUIKey.ParaArray) = uCount;
            gsTaskMonitorRealIDUIKey.ulParaLength = sizeof(MMP_UBYTE); 
            #endif
            
        #if (GPS_CONNECT_ENABLE)
            #if (GPS_CONFIG_NMEA_FILE == 0)
            if ((uCountForGPSTask++ % 5) == 0)
            #else
            if ((uCountForGPSTask++ % 6) == 0)
            #endif
            {
                MMPF_OS_SetFlags(UartCtrlFlag, GPS_FLAG_GETGPSPACK, MMPF_OS_FLAG_SET);
            }
        #endif

            if (AHC_FALSE == gbBlockRealIDUIKeyTask) {
                switch (uCount)
                {
                    default:
                        #ifdef CFG_KEY_SPEED_UP_AUX_DETECTION //may be defined in config_xxx.h
                        ADC_AUX1_Detection();
                        uCount = 0;
                        break;
                        #else
                        uCount = 1;     // Enter case 1 directly when the case of default is NOP.
                        #endif

                    case 1:
                        #ifdef SLIDE_MENU	// Fix OSD Collapses when pressing MENU button during Slide Menu is moving.
                        if (IsSlidingMenu() == 0)
                        #endif
                        {
                            #ifdef CFG_BLOCK_KEY_EVENT_BEFORE_UI_INIT_DONE
                            if (ubUIInitDone())
                            #endif
                            {
                                GPIO_Key_Detection();
                                IR_Key_Detection();
                                PollingComboKey();
                            }
                        }

                        #ifdef SLIDE_STRING
                        SlideString();
                        #endif
                        break; 

                    case 2:
                        Device_Detection();

                        #if (SUPPORT_GSENSOR)
                        AHC_Gsensor_Polling(MOVE_BY_ACCELERATION);
                        #endif

                        #ifdef CFG_KEY_SPEED_UP_AUX_DETECTION //may be defined in config_xxx.h
                        // Customer ask to do more once to speed up detection cycle
                        ADC_AUX1_Detection();
                        #endif

                        #if (SUPPORT_GPS && defined(GPS_STATUS_LED))
                        if (GPSCtrl_ModuleConnected()) 
                        {
                            static int	_ledcnt = 0;

                            if (GPS_IsValidValue_NMEA0183()) {
                                LedCtrl_ButtonLed(GPS_STATUS_LED, _ledcnt & 0x04);
                            } else {
                                LedCtrl_ButtonLed(GPS_STATUS_LED, _ledcnt & 0x01);
                            }
                            _ledcnt++;
                        } 
                        else 
                        {
                            LedCtrl_ButtonLed(GPS_STATUS_LED, AHC_TRUE);
                        }
                        #endif
                        break;

                    case 3:
                        #ifdef SLIDE_MENU	// Fix OSD Collapses when pressing MENU button during Slide Menu is moving.
                        if (IsSlidingMenu() == 0)
                        #endif
                        {
                            #if (SUPPORT_GSENSOR)
                            AHC_Gsensor_Polling(Z_AXIS_GRAVITY_CAHNGE);
                            #endif
                        }
                        break;
                        
                    case 4:
                        ADC_AUX1_Detection();
                        ADC_BatteryTemperature_Detection();

                        #ifdef SLIDE_STRING
                        SlideString();
                        #endif
                        #ifdef SLIDE_MENU
                        SlideSubMenu();
                        #endif
                        break;
                    
                    case 5:
                    {
                        UINT32          curtick;
                        #if (UVC_HOST_VIDEO_ENABLE )
                        static UINT32   tickcount = 0;
                        MMP_BOOL        ubActive;
                        #endif
                        
                        #if (ADX2015_EN)
                        ADX2015_IsrHandler(0);
                        #endif

                        #if (HDMI_ENABLE)
                        if (HDMI_Cable_Detection() == AHC_FALSE)
                        #endif
                        {
                            #if (TVOUT_ENABLE)
                            TV_Cable_Detection();
                            #endif
                        }

                        MMPF_OS_GetTime(&curtick);
                        PowerSavingModeDetection(curtick);
                        LCDPowerSaveDetection(curtick);
                        VideoPowerOnOffDetection(curtick);
                        SpeakerPowerDetection();

                        #if (EDOG_ENABLE)
                        {
                            static UINT8 ubEdogCount = 0;
                            
                            ubEdogCount += 1;
                            
                            if (ubEdogCount == 5) {
                                if (uiGetCurrentState() != UI_SD_UPDATE_STATE && uiGetCurrentState() != UI_MSDC_STATE) {
                                    EDOGCtrl_Handler(hGps);
                                }
                                ubEdogCount = 0;
                            }
                        }
                        #endif

                        #if (UVC_HOST_VIDEO_ENABLE )
                        MMPD_Display_GetWinActive(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), &ubActive);
                        
                        if ((tickcount > 10) && (AHC_HostUVCVideoIsEnabled()) && ubActive)
                        {
                            UINT16 Width;
                            UINT16 Height;
                            
                            AHC_Display_GetWidthHdight(&Width, &Height);
                            
                            if ((MMP_IsUSBCamIsoMode() && gbRearState == AHC_FALSE && AHC_TRUE == AHC_HostUVCSonixReverseGearDetection()) ||
                                (!MMP_IsUSBCamIsoMode() && gbRearState == AHC_FALSE && AHC_TRUE == AHC_HostUVC_GetReversingGearStatus())) // CHECK
                            {
                                gbRearState = AHC_TRUE;

                                MMPS_Display_SetWinPriority(MMP_DISPLAY_WIN_OSD, MMP_DISPLAY_WIN_MAIN, UPPER_IMAGE_WINDOW_ID, LOWER_IMAGE_WINDOW_ID);
                                
                                AHC_HostUVCVideoSetWinAttribute(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), Width, Height, 0, 0, MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);
                                
                                MMPS_Display_SetWinActive(GET_VR_PREVIEW_WINDOW(gsAhcPrmSensor), MMP_FALSE);
                                MMPS_Display_SetWinActive(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), AHC_TRUE);
                                
                                SetKeyPadEvent(USB_REARCAM_REVERSE_GEAR);
                                
                                RTNA_DBG_Str(0, ">>>>>>USB_REARCAM_REVERSE_GEAR-1\r\n");
                            }
                            else if ((MMP_IsUSBCamIsoMode() && gbRearState == AHC_TRUE && AHC_FALSE == AHC_HostUVCSonixReverseGearDetection()) ||
                                     (!MMP_IsUSBCamIsoMode() && gbRearState == AHC_TRUE && AHC_FALSE == AHC_HostUVC_GetReversingGearStatus()))
                            {
                                gbRearState = AHC_FALSE;
                                
                                MMPS_Display_SetWinPriority(MMP_DISPLAY_WIN_OSD, MMP_DISPLAY_WIN_MAIN, UPPER_IMAGE_WINDOW_ID, LOWER_IMAGE_WINDOW_ID);

                                #if (TVOUT_PREVIEW_EN || HDMI_PREVIEW_EN)
                                if (AHC_IsTVConnectEx() || AHC_IsHdmiConnect()) {
                                    AHC_HostUVCVideoSetWinAttribute(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), Width/2, Height/2, Width/2, Height/2, MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE); 
                                }
                                else
                                #endif
                                {
                                    #if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0) 
                                    AHC_HostUVCVideoSetWinAttribute(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), Width/2, Height/2, Width/2, Height/2, MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE); 
                                    #elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90)
                                    AHC_HostUVCVideoSetWinAttribute(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), Width/2, Height/2, 0, 0,MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE);
                                    #elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
                                    AHC_HostUVCVideoSetWinAttribute(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), Width/2, Height/2, Width/2, 0, MMP_SCAL_FITMODE_OPTIMAL, AHC_FALSE); 
                                    #endif
                                }
                                
                                MMPS_Display_SetWinActive(GET_VR_PREVIEW_WINDOW(gsAhcPrmSensor), AHC_TRUE);
                                MMPS_Display_SetWinActive(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), AHC_TRUE);
                                
                                RTNA_DBG_Str(0, ">>>>>>USB_REARCAM_REVERSE_GEAR-2\r\n");
                            }
                            tickcount = 0;
                        }
                        else if (!AHC_HostUVCVideoIsEnabled() && gbRearState) {
                             gbRearState = AHC_FALSE;
                        }
                        
                        tickcount++;
                        #endif
                        break;
                    }
                }
            }
            
            #if (TASK_MONITOR_ENABLE)
            gsTaskMonitorRealIDUIKey.sTaskMonitorStates = MMPF_TASK_MONITOR_STATES_WAITING;        
            #endif

//===============================================================
//~~~ Test only ~~~ Test only ~~~ Test only ~~~ Test only ~~~ Test only ~~~ Test only ~~~ 
//-------------------------------------- Begin --------------------------------------
//===============================================================
#if defined(SUPPORT_HID_FUNC) && (SUPPORT_HID_FUNC)   
#if (USB_HID_FUNC_USAGE_TABLE == USB_HID_FUNC_KEYBOARD) 
                {
                    static MMP_ULONG ulTestKeyBoardCnt = 200;
                    static MMP_ULONG ulTestKeyBoardChar = 0;
                    
                    if(--ulTestKeyBoardCnt == 0){
                        ulTestKeyBoardCnt = 200;
                        
                        switch(ulTestKeyBoardChar){
                            case 0: 
                                usb_hid_trigger(0x16, 0x00); //'s'
                                ulTestKeyBoardChar++;
                                break;

                            case 1: 
                                usb_hid_trigger(0x17, 0x00); //'t'
                                ulTestKeyBoardChar++;
                                break;

                            case 2: 
                                usb_hid_trigger(0x04, 0x00); //'a'
                                ulTestKeyBoardChar++;
                                break;

                            case 3: 
                                usb_hid_trigger(0x15, 0x00); //'r'
                                ulTestKeyBoardChar++;
                                break;

                            default: 
                                usb_hid_trigger(0x2c, 0x00); //' '
                                ulTestKeyBoardChar = 0;
                                break;
                            }

                            //MMPF_OS_Sleep(iHIDSpeed);
                            //clear all key
                            usb_hid_trigger(0, 0x0);
                        
                    }
                }
#endif

#if (USB_HID_FUNC_USAGE_TABLE == USB_HID_FUNC_MOUSE) 
                //TBD
#endif

#if (USB_HID_FUNC_USAGE_TABLE == USB_HID_FUNC_VENDOR_DEFINED)  
                {
                    static MMP_ULONG ulTestCustomHIDCnt = 200, ulTestCustomHIDPattern = 0;
                    MMP_UBYTE ubTestCustomHIDTx[8] = {0};
                    
                    if(--ulTestCustomHIDCnt == 0){
                        ulTestCustomHIDCnt = 200;
                        
                        switch(ulTestCustomHIDPattern){
                            case 0: 
                                ubTestCustomHIDTx[0] = 0x00;
                                ubTestCustomHIDTx[1] = 0x01;
                                ubTestCustomHIDTx[2] = 0x02;
                                ubTestCustomHIDTx[3] = 0x03;
                                ubTestCustomHIDTx[4] = 0x04;
                                ubTestCustomHIDTx[5] = 0x05;
                                ubTestCustomHIDTx[6] = 0x06;
                                ubTestCustomHIDTx[7] = 0x07;
                                ulTestCustomHIDPattern++;
                                break;

                            case 1: 
                                ubTestCustomHIDTx[0] = 0x07;
                                ubTestCustomHIDTx[1] = 0x06;
                                ubTestCustomHIDTx[2] = 0x05;
                                ubTestCustomHIDTx[3] = 0x04;
                                ubTestCustomHIDTx[4] = 0x03;
                                ubTestCustomHIDTx[5] = 0x02;
                                ubTestCustomHIDTx[6] = 0x01;
                                ubTestCustomHIDTx[7] = 0x00;
                                ulTestCustomHIDPattern++;
                                break;

                            case 2: 
                                ubTestCustomHIDTx[0] = 0x00;
                                ubTestCustomHIDTx[1] = 0x55;
                                ubTestCustomHIDTx[2] = 0xAA;
                                ubTestCustomHIDTx[3] = 0xFF;
                                ubTestCustomHIDTx[4] = 0x05;
                                ubTestCustomHIDTx[5] = 0xA0;
                                ubTestCustomHIDTx[6] = 0x0F;
                                ubTestCustomHIDTx[7] = 0xF0;
                                ulTestCustomHIDPattern++;
                                break;

                            default: 
                                ubTestCustomHIDTx[0] = 0x01;
                                ubTestCustomHIDTx[1] = 0x02;
                                ubTestCustomHIDTx[2] = 0x04;
                                ubTestCustomHIDTx[3] = 0x08;
                                ubTestCustomHIDTx[4] = 0x10;
                                ubTestCustomHIDTx[5] = 0x20;
                                ubTestCustomHIDTx[6] = 0x40;
                                ubTestCustomHIDTx[7] = 0x80;
                                ulTestCustomHIDPattern = 0;
                                break;
                        }

                        usb_hid_trigger(ubTestCustomHIDTx, 0x08);
                    }
                }
#endif

#endif
//===============================================================
//~~~ Test only ~~~ Test only ~~~ Test only ~~~ Test only ~~~ Test only ~~~ Test only ~~~ 
//--------------------------------------- End ---------------------------------------
//===============================================================

            MMPF_OS_Sleep_MS(10);
        }
    }
}

void NotifyUSB_HwStatus(int sts)
{
	switch(sts)
	{
		case 0: /* SUSPEND */
			#ifdef CFG_KEY_NOTIFY_CABLE_OUT //may be defined in config_xxx.h
			SetKeyPadEvent(USB_CABLE_OUT);
			#endif	
		break;
		
		case 1: /* RESET */
			// Resend USB Cable In to check variable usIsAdapter of USB ISR
			SetKeyPadEvent(USB_CABLE_IN);
		break;
		
		default:
		break;
	}
}

/*
 * Enable_SD_Power 0: PowerOff, not 0: PowerOn SD
 * NOTE: Don't call the function in ISR, GPIOs are protected by OS, they cannot been written
 */
int Enable_SD_Power(int bEnable)
{
	return bEnable;
/* have to review more, it may cause SD not to work!*/
//#ifdef	DEVICE_GPIO_SD_POWER
#if 0

    AHC_GPIO_SetOutputMode(DEVICE_GPIO_SD_POWER, AHC_TRUE);

	if (bEnable) {
	    // LOW to turn on SD power
		/*
		AHC_GPIO_SetOutputMode(CONFIG_PIO_REG_GPIO54, AHC_TRUE);
		AHC_GPIO_SetOutputMode(CONFIG_PIO_REG_GPIO55, AHC_TRUE);
		AHC_GPIO_SetOutputMode(CONFIG_PIO_REG_GPIO56, AHC_TRUE);
		AHC_GPIO_SetOutputMode(CONFIG_PIO_REG_GPIO57, AHC_TRUE);
		AHC_GPIO_SetOutputMode(CONFIG_PIO_REG_GPIO58, AHC_TRUE);
		*/
	    AHC_GPIO_SetData(DEVICE_GPIO_SD_POWER, AHC_FALSE);
	} else {
	    // HIGH to turn off SD power
	    AHC_GPIO_SetData(DEVICE_GPIO_SD_POWER, AHC_TRUE);
	}
#endif
	return bEnable;
}

#if 0
void InitButtonGpio(MMP_GPIO_PIN piopin, GpioCallBackFunc* CallBackFunc)
{
	if(piopin == MMP_GPIO_MAX)
		return;
		
    MMPF_PIO_EnableOutputMode(piopin, MMP_FALSE, MMP_TRUE);

    if (CallBackFunc) {
        MMPF_PIO_EnableTrigMode(piopin, GPIO_H2L_EDGE_TRIG, MMP_TRUE, MMP_TRUE);
        MMPF_PIO_EnableTrigMode(piopin, GPIO_L2H_EDGE_TRIG, MMP_TRUE, MMP_TRUE);
        MMPF_PIO_EnableInterrupt(piopin, MMP_TRUE, 0, (GpioCallBackFunc *) CallBackFunc, MMP_TRUE);
    }
    else
        MMPF_PIO_EnableInterrupt(piopin, MMP_FALSE, 0, (GpioCallBackFunc *) NULL, MMP_TRUE);    
}
#endif
#if 1
void GpioCallBackFunc1(MMP_ULONG gpio )
{

		#if  0
		UINT8 *data = NULL;
		
		printc("~~~~~~~yaokongqi~~~1~~~~~~\r\n");
		if(!MMPF_PIO_GetData(MMP_GPIO24, data ))
			printc("~MMP_GPIO24 = %d~~~~~\r\n",*data);

		#else

		printc("~~~~~~~yaokongqi~~~24~~~~~~\r\n");
		
		Yaokongqi_flag = 3; // play pause

		#endif


}

void GpioCallBackFunc2(void )
{

		#if 0
		UINT8 *data = NULL;
		
		printc("~~~~~~~yaokongqi~~~28~~~~~~\r\n");
		if(!MMPF_PIO_GetData(MMP_GPIO28, data ))
			printc("~~~~MMP_GPIO28 = %d~~~~~\r\n",*data);

		#else
		printc("~~~~~~~yaokongqi~~~28~~~~~~\r\n");
			Yaokongqi_flag = 1; // prev 

		#endif


}

void GpioCallBackFunc3(void )
{
		UINT8 *data = NULL;
		
		printc("~~~~~~~yaokongqi~~~~~12~~~~\r\n");
			MMPF_PIO_GetData(MMP_GPIO26, data );
			printc("~~~~MMP_GPIO26 = %d~~~~~\r\n",*data);


}

void GpioCallBackFunc4(void )
{
	#if 0
		UINT8 *data = NULL;
		
		printc("~~~~~~~yaokongqi~~~~~3~~~~\r\n");
		if(!MMPF_PIO_GetData(MMP_GPIO27, data ))
			printc("~~~~MMP_GPIO27 = %d~~~~~\r\n",*data);
	#else
		printc("~~~~~~~yaokongqi~~~~~27~~~~\r\n");
		SetKeyPadEvent(BUTTON_POWER_LPRESS); // lyj 20190423

	#endif

}

void GpioCallBackFunc5(void )
{

	#if 0
		UINT8 *data = NULL;
		
		printc("~~~~~~~yaokongqi~~~~~~~~~\r\n");
		if(!MMPF_PIO_GetData(MMP_GPIO29, data ))
			printc("~~~~MMP_GPIO29 = %d~~~~~\r\n",*data);
	#else
		printc("~~~~~~~yaokongqi~~~29~~~~~~\r\n");
			flag_add = 1;
			exit_flag = 0;

	#endif


}

void GpioCallBackFunc6(void )
{

	#if 0
		UINT8 *data = NULL;
		
		printc("~~~~~~~yaokongqi~~~4~~~~~~\r\n");
		if(!MMPF_PIO_GetData(MMP_GPIO30, data ))
			printc("~~~~MMP_GPIO30 = %d~~~~~\r\n",*data);
	#else
			printc("~~~~~~~yaokongqi~~~30~~~~~~\r\n");
			flag_sub= 1;
			exit_flag = 0;

	#endif

}


void GpioCallBackFunc7(void )
{

		#if  0
		UINT8 *data = NULL;
		
		printc("~~~~~~~yaokongqi~~~5~~~~~~\r\n");
		if(!MMPF_PIO_GetData(MMP_GPIO31, data ))
			printc("~~~~MMP_GPIO31 = %d~~~~~\r\n",*data);

		#else

		printc("~~~~~~~yaokongqi~~~31~~~~~~\r\n");
		
		Yaokongqi_flag = 2; // next
		#endif
}




void key_24g_parser_vlevel(void)
{



		InitButton24G(MMP_GPIO24,(GpioCallBackFunc*)GpioCallBackFunc1,GPIO_L2H_EDGE_TRIG);
		//printc("~~~~~~key~~~~test~~~~\r\n");
		//InitButton24G(MMP_GPIO25,(GpioCallBackFunc*)GpioCallBackFunc2,GPIO_H2L_EDGE_TRIG);
		InitButton24G(MMP_GPIO28,(GpioCallBackFunc*)GpioCallBackFunc2,GPIO_L2H_EDGE_TRIG);
		//InitButton24G(MMP_GPIO26,(GpioCallBackFunc*)GpioCallBackFunc3,GPIO_H2L_EDGE_TRIG);
		InitButton24G(MMP_GPIO27,(GpioCallBackFunc*)GpioCallBackFunc4,GPIO_H2L_EDGE_TRIG|GPIO_L2H_EDGE_TRIG);
		InitButton24G(MMP_GPIO29,(GpioCallBackFunc*)GpioCallBackFunc5,GPIO_L2H_EDGE_TRIG);
		InitButton24G(MMP_GPIO30,(GpioCallBackFunc*)GpioCallBackFunc6,GPIO_L2H_EDGE_TRIG);
		InitButton24G(MMP_GPIO31,(GpioCallBackFunc*)GpioCallBackFunc7,GPIO_L2H_EDGE_TRIG);
		//printc("~~~~~~key~~~~test~~init~~~~\r\n");

	

	
}
#endif
void Key24_switch(void)
{
	MMP_UBYTE revalback1 = 100;

	MMPF_PIO_EnableOutputMode(MMP_GPIO26, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_SetData(MMP_GPIO26,1,MMP_TRUE);
	RTNA_WAIT_MS(150);
	MMPF_PIO_SetData(MMP_GPIO26,0,MMP_TRUE);
	RTNA_WAIT_MS(150);
	MMPF_PIO_SetData(MMP_GPIO26,1,MMP_TRUE);
	RTNA_WAIT_MS(150);
	MMPF_PIO_SetData(MMP_GPIO26,0,MMP_TRUE);
	RTNA_WAIT_MS(150);
	MMPF_PIO_SetData(MMP_GPIO26,1,MMP_TRUE);
	RTNA_WAIT_MS(150);
	MMPF_PIO_SetData(MMP_GPIO26,0,MMP_TRUE);
	RTNA_WAIT_MS(150);
	MMPF_PIO_SetData(MMP_GPIO26,1,MMP_TRUE);
	RTNA_WAIT_MS(150);
	MMPF_PIO_SetData(MMP_GPIO26,0,MMP_TRUE);
	MMPF_PIO_EnableOutputMode(MMP_GPIO26, MMP_FALSE, MMP_TRUE);

	//MMPF_PIO_GetData(MMP_GPIO26,&revalback1);
	
	//RTNA_WAIT_MS(3000);
	
	
	printc("key24 switch~revalback=%d~~~~\r\n",revalback1); // 4-25
	

}


void White_light_bar(MMP_ULONG lightBar)
{
			MMPF_PWM_Initialize();
			
			MMPF_PWM_OutputPulse(MMP_PWM3_PIN_AGPIO4,MMP_TRUE,22000,lightBar,  MMP_TRUE,  MMP_FALSE, NULL,0 );//设置脉冲 green  改blue
			MMPF_PWM_OutputPulse(MMP_PWM0_PIN_AGPIO1,MMP_TRUE,22000,lightBar,  MMP_TRUE,  MMP_FALSE, NULL,0);//red
			MMPF_PWM_OutputPulse(MMP_PWM1_PIN_AGPIO2,MMP_TRUE,22000,lightBar,  MMP_TRUE,  MMP_FALSE, NULL,0 );//设置脉冲 blue	
}

void White_light_bar_off(void)
{
			MMPF_PWM_Initialize();
			
			MMPF_PWM_OutputPulse(MMP_PWM3_PIN_AGPIO4,MMP_TRUE,22000,101,  MMP_TRUE,  MMP_FALSE, NULL,0 );//设置脉冲 green  改blue
			MMPF_PWM_OutputPulse(MMP_PWM0_PIN_AGPIO1,MMP_TRUE,22000,101,  MMP_TRUE,  MMP_FALSE, NULL,0);//red
			MMPF_PWM_OutputPulse(MMP_PWM1_PIN_AGPIO2,MMP_TRUE,22000,101,  MMP_TRUE,  MMP_FALSE, NULL,0 );//设置脉冲 blue	
			//printc("~~~~~~~~~~~~~~~~led_off~~~~~~~\r\n");

}


void get_data_RGB(float *R1,float *G1, float *B1)
{
		float Rled = ((255 - ((float)recv_data[4]))/255)*100;
		float Gled = ((255 - ((float)recv_data[5]))/255)*100;
		float Bled = ((255 - ((float)recv_data[6]))/255)*100;

		*R1 = Rled;
		*G1 = Gled;
		*B1 = Bled;

		printc("~~0000~~~~R = %d,G = %d,B = %d ~~~~\r\n",(MMP_ULONG)Rled,(MMP_ULONG)Gled,(MMP_ULONG)Bled);

}


#if 1
void Color_chang(MMP_ULONG ulFreq,MMP_ULONG rled,MMP_ULONG gled,MMP_ULONG bled)
{


			//MMP_PWM_ATTR pwm_attribute;
			int i;
			float a,b,c;

			//pwm_attribute.ulClkDuty_Peroid = 255;
			//pwm_attribute.uPulseID = MMP_PWM_PULSE_ID_B;



			#if 0

			pwm_attribute.ulClkDuty_Peroid = 255;
			pwm_attribute.uPulseID = MMP_PWM_PULSE_ID_A;


			pwm_attribute.ubID			= MMP_PWM3_PIN_AGPIO4 | MMP_PWM0_PIN_AGPIO1 | MMP_PWM1_PIN_AGPIO2;
			pwm_attribute.ulClkDuty_T0	= 255;
			pwm_attribute.ulClkDuty_T1	= 255;
			pwm_attribute.ulClkDuty_T2	= 255;
			pwm_attribute.ulClkDuty_T3	= 255;
			pwm_attribute.ubNumOfPulses = 1;
			#endif
	
			MMPF_PWM_Initialize();

			//MMPF_PWM_SetAttribe(&pwm_attribute);

			
		
			
			MMPF_PWM_OutputPulse(MMP_PWM3_PIN_AGPIO4,MMP_TRUE,ulFreq,bled/*rled*/,  MMP_TRUE,  MMP_FALSE, NULL,0 );//设置脉冲 green  改blue
			MMPF_PWM_OutputPulse(MMP_PWM0_PIN_AGPIO1,MMP_TRUE,ulFreq,rled/*bled*/,  MMP_TRUE,  MMP_FALSE, NULL,0);//red   
			MMPF_PWM_OutputPulse(MMP_PWM1_PIN_AGPIO2,MMP_TRUE,ulFreq,gled,  MMP_TRUE,  MMP_FALSE, NULL,0 );//设置脉冲 blue   green

	if(recv_data[7] == 0x0D && recv_data[8] == 0x0A && recv_data[5] != 0x0D)
	{
			
			get_data_RGB(&a,&b,&c);
			R = (MMP_ULONG)a;
			G = (MMP_ULONG)b;
			B = (MMP_ULONG)c;
			//printc("~~~~111~~R = %d,G = %d,B = %d ~~~~\r\n",R,G,B);

	
	}
#if 0 // 20181022
	if(recv_data[1] == 0x55 &&  recv_data[3] == 0x0F && recv_data[4] == 0x02)
		{}else if(recv_data[1] == 0x55 &&  recv_data[3] == 0xF3 )
		{}
		else
		
	{	
	for(i = 0; i<15; i++)
		{
				recv_data[i] = 0;

		}
	
	}
#endif	
}

#endif

void twinkle_led(MMP_ULONG ulFreq,UINT32 uiMiliSecond,MMP_ULONG rled,MMP_ULONG gled,MMP_ULONG bled)
{

		Color_chang(ulFreq,rled,gled,bled);
		//AHC_OS_SleepMs(uiMiliSecond);
		//White_light_bar_off();
		//AHC_OS_SleepMs(uiMiliSecond);

		//printc("~~~~~~~~lyj1111111122~~~\r\n");
}


//===================Freq display=======



//=====================end==================

void light_Bar_fun(MMP_GPIO_PIN pin,AHC_BOOL vel)
{
	MMPF_PIO_EnableOutputMode(pin,MMP_TRUE,MMP_TRUE);
	MMPF_PIO_SetData(pin,vel,MMP_TRUE);

}

//breathe lamp
void breathe_lamp(MMP_ULONG lightBar)
{
	
		White_light_bar(lightBar);

	printc("~~~~~~~end~~~\r\n");
}

//candle lamp
void Candle_lamp(void)
{
	MMPF_PWM_Initialize();
			
	MMPF_PWM_OutputPulse(MMP_PWM3_PIN_AGPIO4,MMP_TRUE,22000,1,  MMP_TRUE,  MMP_FALSE, NULL,0 );//设置脉冲 green  改blue
	MMPF_PWM_OutputPulse(MMP_PWM0_PIN_AGPIO1,MMP_TRUE,22000,46,  MMP_TRUE,  MMP_FALSE, NULL,0);//red
	MMPF_PWM_OutputPulse(MMP_PWM1_PIN_AGPIO2,MMP_TRUE,22000,2,  MMP_TRUE,  MMP_FALSE, NULL,0 );//设置脉冲 blue	
}
extern unsigned short  OldPlayTime;
unsigned char  dealDataUSB(unsigned char passBit[])
{
	if(str_first1[0] == 0xaa /*&& str_first1[1] == 0xfa*/ &&str_first1[2] != 0xf5 && str_first1[3] != 0xa7 && str_first1[str_first1[1] + 2] == 0xf5 && str_first1[str_first1[1] + 3] == 0xa7/*&& (str_first1[str_first1[1] + 1] != 0x00 || str_first1[str_first1[1]] != 0x00) */)
	{
		MMP_UBYTE i;
		for(i = 0; i < str_first1[1];i++)
		{
			passBit[i] = str_first1[i+2];
			//printc("get the USB data passBit[%d] = 0x%02x~~\r\n",i,passBit[i]);
		}

		//ReciveDataLength = str_first1[1];
		str_first1[str_first1[1] + 2] = 0;
		OldPlayTime = 0;
		if(str_first1[1] > 48)
			str_first1[1] = 48;
		return str_first1[1];

	}
	else if(str_first1[0] == 0xaa /*&& str_first1[1] == 0xfa*/ &&str_first1[2] == 0xf5 && str_first1[3] == 0xa7 && str_first1[1] !=0)
	{
		str_first1[str_first1[1] + 2] = 0x00;
		if(str_first1[1] > 48)
			str_first1[1] = 48;
		return str_first1[1];
	}
		
		return 0;

}

char * ConverString(char ch)
{
	//char * StringTatale[] = "*";
	switch(ch)
	{

		case 40:
			return "(";
			break;
		case 41:
			return ")";
			break;
		case 45:
			return "-";
			break;
		case 126:
			return "~";
			break;
		case 48:
			return "0";
			break;
		case 49:
			return "1";
			break;
		case 50:
			return "2";
			break;
		case 51:
			return "3";
			break;
		case 52:
			return "4";
			break;
		case 53:
			return "5";
			break;
		case 54:
			return "6";
			break;
		case 55:
			return "7";
			break;
		case 56:
			return "8";
			break;
		case 57:
			return "9";
			break;

		case 95:
			return "_";
			break;
		case 46:
			return ".";
			break;
		case 64:
			return "@";
			break;
		case 65:
			return "A";
			break;
		case 66:
			return "B";
			break;
		case 67:
			return "C";
			break;
		case 68:
			return "D";
			break;
		case 69:
			return "E";
			break;
		case 70:
			return "F";
			break;
		case 71:
			return "G";
			break;
		case 72:
			return "H";
			break;
		case 73:
			return "I";
			break;


		case 74:
			return "J";
			break;
		case 75:
			return "K";
			break;
		case 76:
			return "L";
			break;
		case 77:
			return "M";
			break;
		case 78:
			return "N";
			break;
		case 79:
			return "O";
			break;
		case 80:
			return "P";
			break;
		case 81:
			return "Q";
			break;
		case 82:
			return "R";
			break;
			
		case 83:
			return "S";
			break;
		case 84:
			return "T";
			break;
		case 85:
			return "U";
			break;
		case 86:
			return "V";
			break;
		case 87:
			return "W";
			break;
		case 88:
			return "X";
			break;
		case 89:
			return "Y";
			break;
		case 90:
			return "Z";
			break;
		case 39:
			return "'";
			break;	

		case 97:
			return "a";
			break;
		case 98:
			return "b";
			break;
		case 99:
			return "c";
			break;
		case 100:
			return "d";
			break;
		case 101:
			return "e";
			break;
		case 102:
			return "f";
			break;
	
		case 103:
			return "g";
			break;
		case 104:
			return "h";
			break;


		case 105:
			return "i";
			break;
		case 106:
			return "j";
			break;
		case 107:
			return "k";
			break;
		case 108:
			return "l";
			break;
		case 109:
			return "m";
			break;
		case 110:
			return "n";
			break;
		case 111:
			return "o";
			break;
		case 112:
			return "p";
			break;
		case 113:
			return "q";
			break;
			
		case 114:
			return "r";
			break;
		case 115:
			return "s";
			break;
		case 116:
			return "t";
			break;
		case 117:
			return "u";
			break;
		case 118:
			return "v";
			break;
		case 119:
			return "w";
			break;
		case 120:
			return "x";
			break;
		case 121:
			return "y";
			break;
		case 122:
			return "z";
			break;
		


		default:
			return "*";
			break;
			


	}

}



