#ifndef _PCAM_API_H_
#define _PCAM_API_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "os_wrap.h"
#include "mmp_lib.h"
#include "mmps_3gprecd.h"
#include "mmpf_audio_ctl.h"

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

enum {
    PCAM_ERROR_NONE = 0,
    /* System Error */
    PCAM_SYS_ERR,
    PCAM_SYS_TIMEOUT,
    /* Pcam Error */
    PCAM_USB_INIT_ERR,
    PCAM_USB_PREVIEW_ERR,
    PCAM_USB_CAPTURE_ERR,
    /* Misc */
    PCAM_USB_FIRMWARE_ERR,
    PCAM_ERROR_MAX
};

// Must be sync with MMPS_3GPRECD_VIDEO_FORMAT
typedef enum _PCAM_USB_VIDEO_FORMAT {
    PCAM_USB_VIDEO_FORMAT_OTHERS = 0x00,
    PCAM_USB_VIDEO_FORMAT_H264,
    PCAM_USB_VIDEO_FORMAT_MJPEG, 
    PCAM_USB_VIDEO_FORMAT_YUV422,
    PCAM_USB_VIDEO_FORMAT_YUV420, 
    PCAM_USB_VIDEO_FORMAT_NUM
} PCAM_USB_VIDEO_FORMAT;

typedef enum _PCAM_USB_VIDEO_QUALITY {
    PCAM_USB_HIGH_Q = 0,
    PCAM_USB_NORMAL_Q,
    PCAM_USB_LOW_Q,
    PCAM_USB_QUALITY_NUM
} PCAM_USB_VIDEO_QUALITY;

// Map to RES_TYPE_LIST
typedef enum _PCAM_USB_VIDEO_RES { 
    PCAM_USB_RESOL_640x360 = 0, 
    PCAM_USB_RESOL_640x480,
    PCAM_USB_RESOL_720x480,
    PCAM_USB_RESOL_800x600,
    PCAM_USB_RESOL_848x480,
    PCAM_USB_RESOL_960x720,
    PCAM_USB_RESOL_1024x576,
    PCAM_USB_RESOL_1024x600,
    PCAM_USB_RESOL_1024x768,
    PCAM_USB_RESOL_1280x720,
    PCAM_USB_RESOL_1280x960,
    PCAM_USB_RESOL_1600x1200,
    PCAM_USB_RESOL_1920x1080,
    PCAM_USB_RESOL_2048x1536,
    PCAM_USB_RESOL_2176x1224,
    PCAM_USB_RESOL_2560x1440,
    PCAM_USB_RESOL_2560x1920,
    PCAM_USB_RESOL_DUALSNR_1472x736,
    PCAM_USB_RESOL_DUALSNR_1920x960,
    PCAM_USB_RESOL_DUALSNR_2176x1088,
    PCAM_USB_RESOL_DUALSNR_2560x1280,
    PCAM_USB_RESOL_DUALSNR_3008x1504,
    PCAM_USB_RESOL_NUM
} PCAM_USB_VIDEO_RES;

typedef enum _PCAM_USB_AUDIO_FORMAT {
    PCAM_USB_AUDIO_FORMAT_AAC,
    PCAM_USB_AUDIO_FORMAT_AMR,
    PCAM_USB_AUDIO_FORMAT_NUM 
} PCAM_USB_AUDIO_FORMAT;

typedef enum _PCAM_USB_DEBAND {
    PCAM_USB_DEBAND_60HZ,
    PCAM_USB_DEBAND_50HZ
} PCAM_USB_DEBAND;

typedef enum _PCAM_USB_SETTING_CONTEXT {
    PCAM_USB_SETTING_H264_RES = 0,
    PCAM_USB_SETTING_VIDEO_RES,
    PCAM_USB_SETTING_VIDEO_FORMAT,
    PCAM_USB_SETTING_VIDEO_QUALITY,
    PCAM_USB_SETTING_DEBAND,
    PCAM_USB_SETTING_AUDIO_FORMAT,
    
    PCAM_USB_SETTING_SATURATION,
    PCAM_USB_SETTING_CONTRAST,
    PCAM_USB_SETTING_BRIGHTNESS,
    PCAM_USB_SETTING_HUE,
    PCAM_USB_SETTING_GAMMA,
    PCAM_USB_SETTING_BACKLIGHT,
    PCAM_USB_SETTING_SHARPNESS,
    PCAM_USB_SETTING_GAIN,
    PCAM_USB_SETTING_WB,
    PCAM_USB_SETTING_LENS,
    PCAM_USB_SETTING_AF,
    PCAM_USB_SETTING_AE,
    PCAM_USB_SETTING_WB_TEMP,
    PCAM_USB_SETTING_DIGZOOM,
    PCAM_USB_SETTING_DIGPAN,
    PCAM_USB_SETTING_OUTPUT_SENSOR, //Dual sensor output case.

    PCAM_USB_SETTING_ALL
} PCAM_USB_SETTING_CONTEXT;

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct _PCAM_USB_CTX {
    MMPS_3GPRECD_VIDEO_FORMAT 	videoFormat;
    MMP_USHORT 					videoQuality;
    MMP_USHORT 					videoRes;
    MMP_USHORT 					debandMode;
    MMPS_3GPRECD_AUDIO_FORMAT 	audioFormat;
    MMP_USHORT 					audioInPath;
    MMP_AUD_LINEIN_CH 			lineInChannel;
} PCAM_USB_CTX;

typedef struct _PCAM_USB_ZOOM {
    MMP_UBYTE 					Dir; 			// 0 : zoom in, 1 : zoom out, 2 : zoom stop
    MMP_UBYTE 					RangeStep; 		// How many step, defined at UVC.
    MMP_UBYTE 					RangeMin;  		// The zoom range for each step, floating
    MMP_UBYTE 					RangeMax;  		// The zoom range for each step, floating
} PCAM_USB_ZOOM;

typedef struct _PAN_USB_PANTILT {
    MMP_LONG    				PanMin;
    MMP_LONG    				PanMax;
    MMP_LONG    				TiltMin;
    MMP_LONG    				TiltMax;
    MMP_USHORT  				Steps;
} PCAM_USB_PANTILT;

typedef struct _PCAM_USB_INFO {
    PCAM_USB_VIDEO_FORMAT   	pCamVideoFormat;
    PCAM_USB_VIDEO_QUALITY  	pCamVideoQuality;
    PCAM_USB_VIDEO_RES      	pCamVideoRes;
    PCAM_USB_DEBAND         	pCamDebandMode;
    PCAM_USB_AUDIO_FORMAT   	pCamAudioFormat;
    
    MMP_USHORT          		pCamSaturation;
    MMP_USHORT        			pCamContrast;
    MMP_USHORT           		pCamSharpness;
    MMP_USHORT              	pCamGain;
    MMP_SHORT               	pCamBrightness;
    MMP_SHORT               	pCamHue;
    MMP_SHORT              	 	pCamGamma;
    MMP_USHORT              	pCamBacklight;
    MMP_UBYTE               	pCamWB;
    MMP_USHORT             	 	pCamWBTemp;
    MMP_USHORT              	pCamLensPos;
    MMP_BOOL               	 	pCamEnableAF;
    MMP_BOOL                	pCamEnableAE;
    
    PCAM_USB_ZOOM           	pCamDigZoom;
    PCAM_USB_PANTILT        	pCamDigPan;
} PCAM_USB_INFO;

// VC async. control block
typedef struct _PCAM_ASYNC_VC_CFG
{
    MMP_BOOL                    pCamEnableAsyncMode; // Enable Async mode or not
    MMP_UBYTE                   pCamOriginator;
    MMP_UBYTE                   pCamSelector;
    MMP_UBYTE                   pCamAttribute;
    MMP_UBYTE                   pCamValUnit;
} PCAM_ASYNC_VC_CFG;

typedef struct _PCAM_AHC_PREVIEW_INFO 
{
    MMP_BOOL                    bUserDefine;
    MMP_UBYTE                   ubPreviewMode;
    MMP_BOOL                    bUseCustBitrate;
    MMP_ULONG                   ulStreamBitrate;
} PCAM_AHC_PREVIEW_INFO;

//==============================================================================
//
//                              FUNCTION PROTOTYPE
//
//==============================================================================

#if (SUPPORT_UVC_FUNC)
MMP_USHORT pcam_usb_preview_start(void);
MMP_USHORT pcam_usb_preview_stop(void);
MMP_USHORT pcam_usb_exit(void);
MMP_USHORT pcam_usb_set_attributes(MMP_UBYTE pcamIndex, void *pcamValue ); 
MMP_USHORT pcam_usb_get_attributes(MMP_UBYTE pcamIndex, void *pcamValue);
PCAM_USB_INFO *pcam_usb_get_info(void);
MMP_USHORT pcam_usb_CustomedPreviewAttr(MMP_BOOL bUserConfig, MMP_UBYTE ubPrevSnrMode);
void pcam_usb_GetCustomedPreviewAttr(MMP_BOOL *pbUserConfig, MMP_UBYTE *pubPrevSnrMode);
MMP_USHORT pcam_usb_CustomedStreamAttr(MMP_BOOL bUserConfig, MMP_ULONG ulBitrate);
void pcam_usb_GetCustomedStreamAttr(MMP_BOOL *pbUserConfig, MMP_ULONG *pulBitrate);
MMP_USHORT pcam_usb_take_rawpicture(MMP_ULONG addr);
void pcam_usb_setGain_UAC2ADC(MMP_SHORT voldb);
void PCAM2MMP_SetUVCMixMode(MMP_BOOL bUVCMixEn);
MMP_BOOL PCAM2MMP_GetUVCMixMode(void);
MMP_ULONG PCAM2MMP_SetPcamBufferAddr(MMP_ULONG BufAddr, PCAM_USB_VIDEO_FORMAT Format, MMP_SHORT MaxW, MMP_SHORT MaxH);
#endif

#endif //_PCAM_API_H_
