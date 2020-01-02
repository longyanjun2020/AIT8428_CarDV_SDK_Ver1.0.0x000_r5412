//==============================================================================
//
//  File        : ahc_general.h
//  Description : INCLUDE File for the AHC general function porting.
//  Author      : 
//  Revision    : 1.0
//
//==============================================================================

#ifndef _AHC_GENERAL_H_
#define _AHC_GENERAL_H_

/*===========================================================================
 * Include files
 *===========================================================================*/ 

#include "AHC_Common.h"
#include "AHC_DCF.h"
#include "ait_bsp.h"
#include "config_fw.h"
#include "AHC_Config_SDK.h"
#include "AHC_Capture.h"
#include "AHC_General_CarDV.h"
#include "AHC_video.h"
#include "mmpf_pwm.h"

/*===========================================================================
 * Global variable
 *===========================================================================*/ 

#if defined(WIFI_PORT) && (WIFI_PORT == 1)
extern INT32				gslCGIFeedbackStatus;
extern UINT32				gulCGIFeedbackEvent;
#endif

/*===========================================================================
 * Macro define
 *===========================================================================*/ 

//Recference mmpf_player.c VIDEO_EVENT_XXX
#define AHC_VIDEO_PLAY         				(0x00000000)
#define AHC_VIDEO_PLAY_EOF     				(0x00000001)
#define AHC_VIDEO_PLAY_FF_EOS  				(0x00000002)
#define AHC_VIDEO_PLAY_BW_BOS  				(0x00000004)
#define AHC_VIDEO_PLAY_ERROR_STOP  			(0x00000008)
#define AHC_VIDEO_PLAY_UNSUPPORTED_AUDIO  	(0x00000010)

#define AHC_VIDEO_MAX_PROTECT_TIME  		(500)//ms
#define AHC_STILL_MAX_PROTECT_TIME  		(0)//ms

#define NON_CYCLING_TIME_LIMIT				(0xFFFFFFFF)

//For Video Record
#define SIGNLE_FILE_SIZE_LIMIT_4G			(0xFFFFFFFF)
#define SIGNLE_FILE_SIZE_LIMIT_3_75G		(0xF0000000)
#define SIGNLE_FILE_SIZE_LIMIT_3_5G			(0xE0000000)

//For Search File Format Char */
#define SEARCH_PHOTO						"JPG"
#define SEARCH_MOVIE						"AVI,MOV"
#define SEARCH_PHOTO_MOVIE					"AVI,MOV,JPG"

#define EXT_DOT                             "."
#define PHOTO_JPG_EXT                       "JPG"
#define MOVIE_3GP_EXT                       "MOV"
#define MOVIE_AVI_EXT                       "AVI"

//For Audio Callback Event [Ref:mmpf_audio_ctl.h]
#define AHC_AUDIO_EVENT_EOF     			(0x00000001)
#define AHC_AUDIO_EVENT_PLAY				(0x00000002)

//For SD Card
#define SLOW_MEDIA_CLASS				(4)//Class4
#define SLOW_MEDIA_CB_THD				(3)

#define MAX_FILE_NAME_LENGTH    (32)
#define MAX_ALLOWED_WORD_LENGTH (57)

#define AHC_PREVIEW_WINDOW_OP_MASK	 	0xff00
#define	AHC_PREVIEW_WINDOW_OP_GET 	 	0x0000
#define	AHC_PREVIEW_WINDOW_OP_SET 	 	0x0100
#define AHC_PREVIEW_WINDOW_MASK		 	0x00ff
#define	AHC_PREVIEW_WINDOW_FRONT 	 	0x0000
#define	AHC_PREVIEW_WINDOW_REAR		 	0x0001

/*===========================================================================
 * Structure define
 *===========================================================================*/ 

typedef struct _AHC_RTC_TIME
{
    UINT16 uwYear;		
    UINT16 uwMonth;
    UINT16 uwDay;		//The date of January is 1~31
    UINT16 uwDayInWeek; //Sunday ~ Saturday
    UINT16 uwHour;   
    UINT16 uwMinute;
    UINT16 uwSecond;
    UINT8 ubAmOrPm;		//am: 0, pm: 1, work only at b_12FormatEn = MMP_TURE
    UINT8 b_12FormatEn;  //for set time, to indacate which format, 0 for 24 Hours,   //1 for 12 Hours format
} AHC_RTC_TIME; //Sync it with AUTL_DATETIME

typedef struct _AHC_TIME_STRING
{
	INT8 byYear[8];
	INT8 byMonth[8];
	INT8 byDay[8];
	INT8 byHour[8];
	INT8 byMinute[8];
	INT8 bySecond[8]; 
} AHC_TIME_STRING;

typedef struct _AHC_QUEUE_MESSAGE 
{
	UINT32 ulMsgID; 
	UINT32 ulParam1;
	UINT32 ulParam2;
} AHC_QUEUE_MESSAGE;

typedef struct _AHC_MEMORY_LOCATION 
{
    UINT32 ulLocationAddr;
    UINT32 ulSize;
} AHC_MEMORY_LOCATION;

typedef struct _AHC_SOUND_FILE_ATTRIBUTE 
{
    char  	path[256];
    UINT32 	ulStartAddress;
    UINT32 	ulSize;
} AHC_SOUND_FILE_ATTRIBUTE;

/*===========================================================================
 * Enum define
 *===========================================================================*/

typedef enum _AHC_PIO_TRIGMODE {
	AHC_PIO_TRIGMODE_EDGE_H2L		=	0x0,
	AHC_PIO_TRIGMODE_EDGE_L2H		=	0x1,
	AHC_PIO_TRIGMODE_LEVEL_H		=	0x2,
	AHC_PIO_TRIGMODE_LEVEL_L		=	0x3,
	AHC_PIO_TRIGMODE_UNKNOWN		=	0xFF
} AHC_PIO_TRIGMODE;

typedef enum _AHC_MEMORY_LOCATION_ID {
	AHC_CCD_TG_MEM_LOCATION = 0,
	AHC_UART_WRITE_MEM_LOCATION,
	AHC_UART_READ_MEM_LOCATION,
	AHC_SET_PARAMARRAY_MEM_LOCATION,
	AHC_GET_PARAM_ARRAY_MEM_LOCATION,
	AHC_FLASH_BURST_MEM_LOCATION,
	AHC_GET_IMAGE_MEM_LOCATION,
	AHC_CURR_PATH_MEM_LOCATION,
	AHC_LUMA_HIST_MEM_LOCATION,
	AHC_GET_TIME_MEM_LOCATION,
	AHC_SET_FREE_PARAMS_MEM_LOCATION,
	AHC_GET_FREE_PARAMS_MEM_LOCATION,
	AHC_AWB_TAB_MEM_LOCATION ,
	AHC_TSMT_FILE_MEM_LOCATION,
	AHC_TSMT_THUMBNAIL_MEM_LOCATION,
	AHC_SET_EXIF_MEM_LOCATION,
	AHC_GET_EXIF_MEM_LOCATION,
	AHC_RCV_FILE_MEM_LOCATION,
	AHC_SET_DPOF_GET_HEADR_PARAM_MEM_LOCATION,
	AHC_SET_DPOF_GET_JOB_PARAM_STR_MEM_LOCATION,
	AHC_SET_DPOF_GET_SRC_PARAM_MEM_LOCATION,
	AHC_OSD_TEXT_OUT_MEM_LOCATION,
	AHC_OSD_DRAW_TEXT_MEM_LOCATION,
	AHC_WRITE_I2C_MEM_LOCATION,
	AHC_READ_I2C_MEM_LOCATION,
	AHC_DRAW_TEXT_MEM_LOCATION,
	AHC_MEMORY_LOCATION_MAX	
} AHC_MEMORY_LOCATION_ID;

typedef enum _AHC_MODE_ID {
    AHC_MODE_IDLE 				= 0x00,
    AHC_MODE_CAPTURE_PREVIEW 	= 0x10,
    AHC_MODE_DRAFT_CAPTURE 		= 0x11,
    AHC_MODE_STILL_CAPTURE 		= 0x20,
    AHC_MODE_C2C_CAPTURE 		= 0x21,
    AHC_MODE_SEQUENTIAL_CAPTURE = 0x22,
    AHC_MODE_LONG_TIME_CONTINUOUS_FIRST_CAPTURE = 0x23,
    AHC_MODE_LONG_TIME_CONTINUOUS_NEXT_CAPTURE = 0x24,
    AHC_MODE_LONG_TIME_CONTINUOUS_LAST_CAPTURE = 0x25,
    AHC_MODE_CONTINUOUS_CAPTURE = 0x26,
    AHC_MODE_USB_MASS_STORAGE 	= 0x30,
    AHC_MODE_USB_WEBCAM 		= 0x31,
    AHC_MODE_PLAYBACK  			= 0x40,
    AHC_MODE_THUMB_BROWSER 		= 0x50,
    AHC_MODE_VIDEO_RECORD 		= 0x60,
    AHC_MODE_RECORD_PREVIEW 	= 0x62,
    AHC_MODE_RECORD_PRERECD 	= 0x63,
    AHC_MODE_RAW_PREVIEW 		= 0x70,
    AHC_MODE_RAW_CAPTURE 		= 0x71,
    AHC_MODE_NET_PLAYBACK       = 0x80,
    AHC_MODE_CALIBRATION 		= 0xF0,
    AHC_MODE__NO_CHANGE             ,       // temp Special ID 
    AHC_MODE_MAX 				= 0xFF
}AHC_MODE_ID;

typedef enum _AHC_SYS_STATUS {
	AHC_SYS_VIDRECD_STATUS = 0x00,
    AHC_SYS_VIDRECD_START,				///< Video record operation, start
    AHC_SYS_VIDRECD_PAUSE,				///< Video record operation, pause
    AHC_SYS_VIDRECD_RESUME,				///< Video record operation, resume
    AHC_SYS_VIDRECD_STOP,				///< Video record operation, stop
    AHC_SYS_VIDRECD_PREVIEW_NORMAL,   
	AHC_SYS_VIDRECD_PREVIEW_ABNORMAL,
	
	AHC_SYS_VIDPLAY_STATUS = 0x10,
	AHC_SYS_VIDPLAY_INVALID,			/**< The component has detected that it's internal data 
                                		structures are corrupted to the point that
                                		it cannot determine it's state properly */
	AHC_SYS_VIDPLAY_LOADED,      		/**< The component has been loaded but has not completed
                                		initialization. */
    AHC_SYS_VIDPLAY_IDLE,        		/**< The component initialization has been completed
                                		successfully and the component is ready to start.*/
    AHC_SYS_VIDPLAY_EXECUTING,   		/**< The component has accepted the start command and
                                		is processing data (if data is available) */
    AHC_SYS_VIDPLAY_PAUSE,       		/**< The component has received pause command */
    
    
	AHC_SYS_USB_STATUS 		= 0x20,
	AHC_SYS_USB_NORMAL,
	AHC_SYS_USB_ISADAPTER,
	AHC_SYS_USB_IDLE,
	
	AHC_SYS_CAPTURE_STATUS 	= 0x30,
	
	AHC_SYS_AUDPLAY_STATUS			= 0x40,
	AHC_SYS_AUDPLAY_START			= 0x41,			
	AHC_SYS_AUDPLAY_PAUSE			= 0x42,	
	AHC_SYS_AUDPLAY_STOP			= 0x44,	
	AHC_SYS_AUDPLAY_INVALID			= 0x48,
	
	AHC_SYS_IDLE,
	AHC_SYS_ERROR,
	
	AHC_SYS_OPERATION_MAX
} AHC_SYS_STATUS;

typedef enum _AHC_PIO_REG 
{
    //===========================GPIO
    AHC_PIO_REG_GPIO0 	= 	0x0,
    AHC_PIO_REG_GPIO1	= 	0x1,
    AHC_PIO_REG_GPIO2	= 	0x2,
    AHC_PIO_REG_GPIO3	= 	0x3,
    AHC_PIO_REG_GPIO4	= 	0x4,
    AHC_PIO_REG_GPIO5	= 	0x5,
    AHC_PIO_REG_GPIO6	= 	0x6,
    AHC_PIO_REG_GPIO7	= 	0x7,
    AHC_PIO_REG_GPIO8	= 	0x8,
    AHC_PIO_REG_GPIO9	= 	0x9,
    AHC_PIO_REG_GPIO10  = 	0xa,
    AHC_PIO_REG_GPIO11	= 	0xb,
    AHC_PIO_REG_GPIO12	= 	0xc,
    AHC_PIO_REG_GPIO13	= 	0xd,
    AHC_PIO_REG_GPIO14	= 	0xe,
    AHC_PIO_REG_GPIO15	= 	0xf,
    AHC_PIO_REG_GPIO16	= 	0x10,
    AHC_PIO_REG_GPIO17	= 	0x11,
    AHC_PIO_REG_GPIO18	= 	0x12,
    AHC_PIO_REG_GPIO19	= 	0x13,
    AHC_PIO_REG_GPIO20  = 	0x14,
    AHC_PIO_REG_GPIO21	= 	0x15,
    AHC_PIO_REG_GPIO22	= 	0x16,
    AHC_PIO_REG_GPIO23	= 	0x17,
    AHC_PIO_REG_GPIO24	= 	0x18,
    AHC_PIO_REG_GPIO25	= 	0x19,
    AHC_PIO_REG_GPIO26	= 	0x1a,
    AHC_PIO_REG_GPIO27	= 	0x1b,
    AHC_PIO_REG_GPIO28	= 	0x1c,
    AHC_PIO_REG_GPIO29	= 	0x1d,
    AHC_PIO_REG_GPIO30  = 	0x1e,
    AHC_PIO_REG_GPIO31	= 	0x1f,
    AHC_PIO_REG_GPIO32	= 	0x20,
    AHC_PIO_REG_GPIO33	= 	0x21,
    AHC_PIO_REG_GPIO34	= 	0x22,
    AHC_PIO_REG_GPIO35	= 	0x23,
    AHC_PIO_REG_GPIO36	= 	0x24,
    AHC_PIO_REG_GPIO37	= 	0x25,
    AHC_PIO_REG_GPIO38	= 	0x26,
    AHC_PIO_REG_GPIO39	= 	0x27,
    AHC_PIO_REG_GPIO40  = 	0x28,
    AHC_PIO_REG_GPIO41	= 	0x29,
    AHC_PIO_REG_GPIO42	= 	0x2a,
    AHC_PIO_REG_GPIO43	= 	0x2b,
    AHC_PIO_REG_GPIO44	= 	0x2c,
    AHC_PIO_REG_GPIO45	= 	0x2d,
    AHC_PIO_REG_GPIO46	= 	0x2e,
    AHC_PIO_REG_GPIO47	= 	0x2f,
    AHC_PIO_REG_GPIO48	= 	0x30,
    AHC_PIO_REG_GPIO49	= 	0x31,
    AHC_PIO_REG_GPIO50  = 	0x32,
    AHC_PIO_REG_GPIO51	= 	0x33,
    AHC_PIO_REG_GPIO52	= 	0x34,
    AHC_PIO_REG_GPIO53	= 	0x35,
    AHC_PIO_REG_GPIO54	= 	0x36,
    AHC_PIO_REG_GPIO55	= 	0x37,
    AHC_PIO_REG_GPIO56	= 	0x38,
    AHC_PIO_REG_GPIO57	= 	0x39,
    AHC_PIO_REG_GPIO58	= 	0x3a,
    AHC_PIO_REG_GPIO59	= 	0x3b,
    AHC_PIO_REG_GPIO60	= 	0x3c,
    AHC_PIO_REG_GPIO61	= 	0x3d,
    AHC_PIO_REG_GPIO62	= 	0x3e,
    AHC_PIO_REG_GPIO63	= 	0x3f,
    AHC_PIO_REG_GPIO64  =   0x40,
    AHC_PIO_REG_GPIO65  =   0x41,
    AHC_PIO_REG_GPIO66  =   0x42,
    AHC_PIO_REG_GPIO67  =   0x43,
    AHC_PIO_REG_GPIO68  =   0x44,
    AHC_PIO_REG_GPIO69  =   0x45,
    AHC_PIO_REG_GPIO70  =   0x46,
    AHC_PIO_REG_GPIO71  =   0x47,
    AHC_PIO_REG_GPIO72  =   0x48,
    AHC_PIO_REG_GPIO73  =   0x49,
    AHC_PIO_REG_GPIO74  =   0x4A,
    AHC_PIO_REG_GPIO75  =   0x4B,
    AHC_PIO_REG_GPIO76  =   0x4C,
    AHC_PIO_REG_GPIO77  =   0x4D,
    AHC_PIO_REG_GPIO78  =   0x4E,
    AHC_PIO_REG_GPIO79  =   0x4F,
    AHC_PIO_REG_GPIO80  =   0x50,
    AHC_PIO_REG_GPIO81  =   0x51,
    AHC_PIO_REG_GPIO82  =   0x52,
    AHC_PIO_REG_GPIO83  =   0x53,
    AHC_PIO_REG_GPIO84  =   0x54,
    AHC_PIO_REG_GPIO85  =   0x55,
    AHC_PIO_REG_GPIO86  =   0x56,
    AHC_PIO_REG_GPIO87  =   0x57,
    AHC_PIO_REG_GPIO88  =   0x58,
    AHC_PIO_REG_GPIO89  =   0x59,
    AHC_PIO_REG_GPIO90  =   0x5A,
    AHC_PIO_REG_GPIO91  =   0x5B,
    AHC_PIO_REG_GPIO92  =   0x5C,
    AHC_PIO_REG_GPIO93  =   0x5D,
    AHC_PIO_REG_GPIO94  =   0x5E,
    AHC_PIO_REG_GPIO95  =   0x5F,
    AHC_PIO_REG_GPIO96  =   0x60,
    AHC_PIO_REG_GPIO97  =   0x61,
    AHC_PIO_REG_GPIO98  =   0x62,
    AHC_PIO_REG_GPIO99  =   0x63,
    AHC_PIO_REG_GPIO100 =   0x64,
    AHC_PIO_REG_GPIO101 =   0x65,
    AHC_PIO_REG_GPIO102 =   0x66,
    AHC_PIO_REG_GPIO103 =   0x67,
    AHC_PIO_REG_GPIO104 =   0x68,
    AHC_PIO_REG_GPIO105 =   0x69,
    AHC_PIO_REG_GPIO106 =   0x6A,
    AHC_PIO_REG_GPIO107 =   0x6B,
    AHC_PIO_REG_GPIO108 =   0x6C,
    AHC_PIO_REG_GPIO109 =   0x6D,
    AHC_PIO_REG_GPIO110 =   0x6E,
    AHC_PIO_REG_GPIO111 =   0x6F,
    AHC_PIO_REG_GPIO112 =   0x70,
    AHC_PIO_REG_GPIO113 =   0x71,
    AHC_PIO_REG_GPIO114 =   0x72,
    AHC_PIO_REG_GPIO115 =   0x73,
    AHC_PIO_REG_GPIO116 =   0x74,
    AHC_PIO_REG_GPIO117 =   0x75,
    AHC_PIO_REG_GPIO118 =   0x76,
    AHC_PIO_REG_GPIO119 =   0x77,
    AHC_PIO_REG_GPIO120 =   0x78,
    AHC_PIO_REG_GPIO121 =   0x79,
    AHC_PIO_REG_GPIO122 =   0x7A,
    AHC_PIO_REG_GPIO123 =   0x7B,
    AHC_PIO_REG_GPIO124 =   0x7C,
    AHC_PIO_REG_GPIO125 =   0x7D,
    AHC_PIO_REG_GPIO126 =   0x7E,
    AHC_PIO_REG_GPIO127 =   0x7F,
    AHC_PIO_REG_UNKNOWN =   0xFFF
} AHC_PIO_REG;

typedef enum _AHC_SNR_STATUS {
	AHC_SNR_NORMAL = 0,
	AHC_SNR_REVERSE,
	AHC_SNR_NOFLIP	
}AHC_SNR_STATUS;

typedef enum _AHC_SOUND_EFFECT_STATUS {
	AHC_SOUND_EFFECT_STATUS_START = 0,  ///< play start
    AHC_SOUND_EFFECT_STATUS_PAUSE ,     ///< play pause
    AHC_SOUND_EFFECT_STATUS_STOP  ,     ///< play stop
    AHC_SOUND_EFFECT_STATUS_INVALID		///< invalid status
}AHC_SOUND_EFFECT_STATUS;

typedef enum _AHC_PROTECTION_OP{
    AHC_PROTECTION_PRE_FILE = 0x01,// previous
    AHC_PROTECTION_CUR_FILE = 0x02,// current
    AHC_PROTECTION_BOTH_FILE = AHC_PROTECTION_PRE_FILE|AHC_PROTECTION_CUR_FILE// previous + current
}AHC_PROTECTION_OP;

typedef enum _AHC_PROTECT_TYPE {
	AHC_PROTECT_NONE    = 0x00, 
	AHC_PROTECT_G       = 0x01, //GSensor Lock
	AHC_PROTECT_MENU    = 0x02, //Menu lock
	AHC_PROTECT_D       = AHC_PROTECT_G|AHC_PROTECT_MENU,   //GSensor+Menu Lock
	AHC_PROTECT_MOVE  	= 0x04,  //Move File to another DIR
	AHC_PROTECT_NEXT  	= 0x08  //Move next File to another DIR
}AHC_PROTECT_TYPE;

typedef enum _AHC_ZOOM_DIRECTION
{
	AHC_SENSOR_ZOOM_IN = 0,
	AHC_SENSOR_ZOOM_OUT,
	AHC_SENSOR_ZOOM_STOP
} AHC_ZOOM_DIRECTION;

typedef struct _AHC_WINDOW_RECT {
    UINT16		usLeft;
    UINT16      usTop;
    UINT16      usWidth;
    UINT16      usHeight;
} AHC_WINDOW_RECT;

/*===========================================================================
 * Function prototype
 *===========================================================================*/

AHC_BOOL 	AHC_SetPreviewZoomConfig(UINT16 bySteps, UINT8 byMaxRatio);
AHC_BOOL 	AHC_SetPreviewZoom(AHC_CAPTURE_MODE CaptureMode, AHC_ZOOM_DIRECTION byDirection, UINT8 bySteps);
AHC_BOOL 	AHC_PlaybackZoom(UINT16 uwStartX, UINT16 uwStartY, UINT16 uwWidth, UINT16 uwHeight);
int			AHC_PreviewWindowOp(int op, AHC_WINDOW_RECT * pRect);

AHC_BOOL 	AHC_STICKER_TransRGB565toIndex8(UINT32 uiSrcAddr,  UINT32 uiSrcW,  UINT32 uiSrcH, 
									        UINT32 uiDestAddr, UINT32 uiDestW, UINT32 uiDestH,
									        UINT8  byForegroundColor, UINT8  byBackgroundColor);
AHC_BOOL 	AHC_STICKER_TransDateToString(AHC_RTC_TIME* psRtcTime, INT8* pbyDate, UINT8 byDateConfig, UINT8 byFormatConfig);

#define AHC_AUDIO_STREAM_ID 0 //should be 0 or 1.
#define AHC_AUDIO_STREAM_2_ID 1 //should be 0 or 1.

AHC_BOOL 	AHC_Deletion_Romove(AHC_BOOL bFirstTime);

AHC_BOOL    AHC_Protect_SpellFileName(char* FilePathName, INT8* pchDirName, INT8* pchFileName);
AHC_PROTECT_TYPE AHC_Protect_GetType(void);
void 		AHC_Protect_SetType(AHC_PROTECT_TYPE Type);
AHC_BOOL 	AHC_Protect_SetVRFile(AHC_PROTECTION_OP Op, AHC_PROTECT_TYPE Type);

// Video/Audio Playback Function
void 		AHC_SetVideoPlayStartTime(MMP_ULONG ulStartTime);
AHC_BOOL 	AHC_GetVideoPlayStopStatus(UINT32 *pwValue);
AHC_BOOL 	AHC_SetVideoPlayStopStatus(UINT32 Value);
AHC_BOOL 	AHC_GetAudioPlayStopStatus(UINT32 *pwValue);
AHC_BOOL 	AHC_SetAudioPlayStopStatus(UINT32 Value);
AHC_BOOL 	AIHC_GetCurrentPBFileType(UINT32 *pFileType);
AHC_BOOL 	AIHC_GetCurrentPBHeight(UINT16 *pHeight);
AHC_BOOL 	AIHC_StopPlaybackMode(void);
int         AHC_IsInVideoMode(void);

// RTC Function
AHC_ERR 	AHC_RTC_UpdateTime(void);
void        AHC_RTC_GetTime(AHC_RTC_TIME *psAHC_RTC_Time);
void        AHC_RTC_GetTimeEx(AHC_RTC_TIME *psAHC_RTC_Time, AHC_BOOL bUpdateRelativeTime);
AHC_BOOL    AHC_SetClock(UINT16 uiYear, UINT16 uiMonth, UINT16 uiDay, UINT16 uiDayInWeek, UINT16 uiHour, UINT16 uiMinute, UINT16 uiSecond,  UINT8 ubAmOrPm, UINT8 b_12FormatEn);
AHC_BOOL    AHC_GetClock(UINT16* puwYear, UINT16* puwMonth, UINT16* puwDay, UINT16* puwDayInWeek, UINT16* puwHour, UINT16* puwMinute, UINT16* puwSecond, UINT8* ubAmOrPm, UINT8* b_12FormatEn);
AHC_BOOL 	AHC_RtcSetWakeUpEn(AHC_BOOL bEnable, UINT16 uiYear, UINT16 uiMonth, UINT16 uiDay, UINT16 uiHour, UINT16 uiMinute, UINT16 uiSecond, void *phHandleFunc);

// General Function
AHC_BOOL 	AHC_Initialize(void);
AHC_BOOL 	AHC_SetMode(AHC_MODE_ID uiMode);
AHC_BOOL 	AHC_GetSystemStatus(UINT32* pwValue);
AHC_MODE_ID AHC_GetAhcSysMode(void);
void        AHC_SetAhcSysMode(AHC_MODE_ID uiMode);

AHC_BOOL    AHC_SendAHLMessageEnable(AHC_BOOL Enable);
AHC_BOOL 	AHC_SendAHLMessage(UINT32 ulMsgID, UINT32 ulParam1, UINT32 ulParam2);
AHC_BOOL 	AHC_SendAHLMessage_HP(UINT32 ulMsgID, UINT32 ulParam1, UINT32 ulParam2);
AHC_BOOL 	AHC_GetAHLMessage(UINT32* ulMsgID, UINT32* ulParam1, UINT32* ulParam2);
AHC_BOOL 	AHC_GetAHLMessage_HP(UINT32* ulMsgID, UINT32* ulParam1, UINT32* ulParam2);
void 		AHC_GetAHLHPMessageCount(UINT16 *usCount);
AHC_BOOL	AHC_DumpAHLMessage(void);
AHC_BOOL 	AIHC_InitAHLMessage(void);
AHC_BOOL 	AIHC_InitAHLMessage_HP(void);
AHC_BOOL 	AHC_WaitForBootComplete(void);
AHC_BOOL	AHC_UnloadSystemFile(void);

// GPIO Function
AHC_BOOL    AHC_GPIO_ConfigPad(AHC_PIO_REG piopin, MMP_UBYTE config);
AHC_BOOL 	AHC_GPIO_SetOutputMode(AHC_PIO_REG ahc_piopin, UINT8 bDirection);
AHC_BOOL 	AHC_GPIO_SetTrigMode(AHC_PIO_REG ahc_piopin, AHC_PIO_TRIGMODE bPolarity);
AHC_BOOL 	AHC_GPIO_EnableInterrupt(AHC_PIO_REG ahc_piopin, void* (phHandleFunc), UINT8 bEnable);
AHC_BOOL 	AHC_GPIO_SetData(AHC_PIO_REG ahc_piopin, UINT8 bState);
AHC_BOOL 	AHC_GPIO_GetData(AHC_PIO_REG ahc_piopin, UINT8 *pwState);

// PWM Function
AHC_BOOL 	AHC_PWM_Initialize(void);
AHC_BOOL 	AHC_PWM_OutputPulse(MMP_PWM_PIN pwm_pin, AHC_BOOL bEnableIoPin, UINT32 ulFrquency, AHC_BOOL bHigh2Low, AHC_BOOL bEnableInterrupt, void* pwm_callBack, UINT32 ulNumOfPulse);
AHC_BOOL 	AHC_PWM_OutputPulseEx(MMP_PWM_PIN pwm_pin, AHC_BOOL bEnableIoPin, UINT32 ulFrquency, UINT32 ulDuty, AHC_BOOL bHigh2Low, AHC_BOOL bEnableInterrupt, void* pwm_callBack, UINT32 ulNumOfPulse);

// Power Function
void 		AHC_PowerOff_Preprocess(void);
void        AHC_PowerOff_VirtualPath(void);
void	 	AHC_PowerOff_NormalPath(void);
AHC_ERR 	AHC_PowerOff_ShowPicture(INT8 *charStr);

// Menu Setting Function
AHC_BOOL    Menu_SetMotionDtcSensitivity(UINT8 val);

#if (SD_UPDATE_FW_EN)
void        AHC_SDUpdateMode(void);
#endif

// Others
void 		AHC_PMU_PowerOff(void);
UINT32 		AHC_GetChargerPowerOffTime(void);

// PIR
#if (ENABLE_PIR_MODE)
void        AHC_PIR_CheckStart(void);
void        AHC_PIR_SetEnable(AHC_BOOL bEnable); 
#endif
AHC_BOOL    AHC_PIR_IsStarted(void);

#if (MOTION_DETECTION_EN)
AHC_BOOL    AHC_GetMotionDetectionStatus(void);
AHC_BOOL    AHC_SetMotionDetectionStatus(AHC_BOOL bEn);
AHC_BOOL    AHC_GetMotionDtcSensitivity(UINT8 *pubMvTh, UINT8 *pubCntTh);
int         AHC_SetMotionDtcSensitivity(unsigned char mvth,  unsigned char cnth);
UINT32      AHC_GetVMDRecTimeLimit(void);
UINT32      AHC_GetVMDShotNumber(void);
void        AHC_AdjustVMDPreviewRes(int *w, int *h);
#endif

void        AHC_Charger_InitialIO(void);
void        AHC_Charger_SetEnable(AHC_BOOL bEn);
AHC_BOOL    AHC_Charger_GetEnable(void);
void        AHC_Charger_SetTempCtrlEnable(AHC_BOOL bEn);
AHC_BOOL    AHC_Charger_GetTempCtrlStatus(void);
AHC_BOOL    AHC_Charger_GetStatus(void);

void        VideoMediaErrorPlayStopHandler(void);
AHC_BOOL    AHC_Deletion_Confirm(DCF_DB_TYPE sDB, UINT32 uiTime);
AHC_BOOL    AHC_Deletion_ConfirmByNum(DCF_DB_TYPE sDB, UINT32 uiTime);
AHC_BOOL    AHC_Deletion_RemoveEx(DCF_DB_TYPE sDB, UINT32 uiTime);

AHC_BOOL    AHC_CheckSysCalibMode(void);

AHC_BOOL    AHC_SDMMC_BasicCheck(void);

void        AHC_BUZZER_Alert(UINT32 ulFrquency, UINT32 ulTimes, UINT32 ulMs);
void AHC_Brightness_Alert(UINT32 ulFrquency, UINT32 ulTimes, UINT32 ulMs);
void        AHC_PowerOff_NormalPathEx(AHC_BOOL bForce, AHC_BOOL bByebye, AHC_BOOL bSound);

void        AHC_SetFastAeAwbMode( AHC_BOOL bEnable );

void		uiSetParkingModeStateInit(void);
void		uiSetParkingModeEnable(UINT8 enable);
void		uiSetParkingModeMenuState(AHC_BOOL enable, AHC_BOOL ParkingStartDrawed);
void		uiSetParkingModeRecordState(AHC_BOOL enable);
void		uiSetParkingModeRecord(AHC_BOOL recorded);
AHC_BOOL	uiGetParkingModeRecord(void);
AHC_BOOL    uiGetParkingStartDrawed(void);
AHC_BOOL    uiSetBootUpUIMode(void);

UINT8		uiGetParkingModeEnable(void);
AHC_BOOL	AHC_StartADAS(AHC_BOOL bEn);
AHC_BOOL	AHC_IsADASStarted( void );

ULONG       AHC_CheckWiFiOnOffInterval(MMP_ULONG waiting_time);
AHC_BOOL    AHC_WiFi_Switch(AHC_BOOL wifi_switch_on);
void        AHC_WiFi_Toggle_StreamingMode(void);

// HDMI-out Focus
#if (SUPPORT_HDMI_OUT_FOCUS)
AHC_BOOL    AHC_CheckHdmiOutFocus(void);
#endif

#if defined(AIT_HW_WATCHDOG_ENABLE) && (AIT_HW_WATCHDOG_ENABLE)
void        AHC_WD_Enable(AHC_BOOL bEnable);
void        AHC_WD_Kick(void);
#endif

AHC_BOOL    AHC_VIDEO_ChangeCurFileTimeLimit(UINT32 ulTimeLimitMs);
AHC_BOOL    AHC_KeyEventIDCheckConflict(UINT32 ulCurKeyEventID);
void        AHC_SetCurKeyEventID(UINT32 ulCurKeyEventID);
UINT32      AHC_GetCurKeyEventID(void);
AHC_BOOL    AHC_RestoreFromDefaultSetting(void);

void        AHC_SetPlayBackRearCam(AHC_BOOL bIsRear);
AHC_BOOL    AHC_GetPlayBackRearCam(void);

#endif		// _AHC_GENERAL_H_

