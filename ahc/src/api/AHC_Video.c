//==============================================================================
//
//  File        : AHC_video.c
//  Description : AHC video function
//  Author      :
//  Revision    : 1.0
//
//==============================================================================

/*===========================================================================
 * Include files
 *===========================================================================*/

#include "Customer_config.h"
#include "AIHC_DCF.h"
#include "AHC_Video.h"
#include "AHC_Audio.h"
#include "AHC_Capture.h"
#include "AHC_DateTime.h"
#include "AHC_Gsensor.h"
#include "AHC_Message.h"
#include "AHC_Stream.h"
#include "AHC_USB.h"
#include "AHC_USBHost.h"
#include "AHC_Media.h"
#include "AHC_Sensor.h"
#include "AHC_Config_SDK.h"
#include "AHC_UF.h"
#include "AHC_DCF.h"
#include "AHC_GUI.h"
#include "AHC_Callback.h"
#include "AHC_General.h"
#include "AHC_Menu.h"
#include "AHC_Utility.h"
#include "vidrec_cfg.h"
#include "ptz_cfg.h"
#include "ldc_cfg.h"
#include "snr_cfg.h"
#include "mmu.h"
#include "mmps_iva.h"
#include "mmps_3gprecd.h"
#include "mmps_audio.h"
#include "mmps_aui.h"
#include "mmps_fs.h"
#include "mmps_sensor.h"
#include "mmpf_display.h"
#include "mmpf_usbh_uvc.h"
#include "mmpf_jpeg_ctl.h"
#include "mmpf_ldc.h"

#if (GPS_CONNECT_ENABLE)
#include "GPS_ctl.h"
#endif
#if (GSENSOR_CONNECT_ENABLE)
#include "GSensor_ctrl.h"
#endif
#include "MenuSetting.h"
#include "StateVideoFunc.h"
#include "DrawStateVideoFunc.h"
#include "ParkingModeCtrl.h"

#if defined(WIFI_PORT) && (WIFI_PORT == 1)
#include "netapp.h"
#endif

/*===========================================================================
 * Macro Define
 *===========================================================================*/

// Max number of videos used in this code base
#define AHC_MAX_VIDEO	        (2)
#define AHC_MAX_AUDIO	        (2)

typedef UINT16 AHC_MEDIA_USAGE;

// Definition of AHC_MEDIA_USAGE. bit 0 is for switching the media on or off.
// bit 1 ~ 7 is for bit mapped configuration
// @Warning Be careful about the condition check, 0x40 still means this media is disabled.
#define AHC_MEDIA_DISABLE       ((AHC_MEDIA_USAGE)0x00)  // General disabling
#define AHC_MEDIA_ENABLE        ((AHC_MEDIA_USAGE)0x01)  // General enabling
#define AHC_MEDIA_STREAMING     ((AHC_MEDIA_USAGE)0x10)  // Output to network

/*===========================================================================
 * Structure
 *===========================================================================*/

// Properties of a single video
typedef struct _AHC_VIDEO_PROPERTY {
    AHC_MEDIA_USAGE                 Usage;
    AHC_VIDEO_FORMAT                CodecType; 			// Map to MMPS_3GPRECD_VIDEO_FORMAT
    AHC_VIDEO_FORMAT_SETTING        sCodecTypeSetting;  // Map to VIDENC_PROFILE or other format setting.
    AHC_VIDEO_CURBUF_ENCODE_MODE    sVidEncMode; 		// Map to VIDENC_CURBUF_MODE.
    UINT32                          SampleRate; 		// FPS, e.g. VIDRECD_FRAMERATE_30FPS
    UINT32                          FPSx10; 			// FPSx10, 75 means 7.5FPS
    UINT32                          Bitrate; 			// Video[0] always uses 3GP config as bitrate. Only other videos use this field.
    UINT32                          nMaxP; 				// Max number of P frames
    UINT32                          nMaxB; 				// Max number of B frames
    UINT32                          nMaxQP; 			// Max QP 51
    UINT32                          CompressRatio; 		// Compression ratio, e.g. VIDRECD_QUALITY_HIGH
    UINT32                          ReservedSize;		// Reserved size
    UINT32                          Resolution; 		// Encode resolution, e.g. VIDRECD_RESOL_1920x1088
    AHC_BOOL                        bStreaming; 		// Back up the status only. for quicker status reference
} AHC_VIDEO_PROPERTY;

// Properties of a single audio
typedef struct _AHC_AUDIO_PROPERTY {
    AHC_MEDIA_USAGE 				Usage;
    AHC_AUDIO_FORMAT 				CodecType; 			// Map to MMPS_3GPRECD_AUDIO_FORMAT;
    UINT32 							SampleRate;
    UINT32 							Bitrate;
    UINT32 							ChannelType;
    AHC_BOOL 						bPreRecordDAC;
    AHC_BOOL 						bMute;
    AHC_BOOL 						bStreaming; 		// Back up the status only. for quicker status reference
} AHC_AUDIO_PROPERTY;

// Property sets of the video used in this. This video includes audio information.
typedef struct _AHC_MOVIE {
    UINT16 							nVideo;				// Number of video
    UINT16 							nAudio;				// Number of audio
    AHC_CONTAINER 					ContainerType;		// Map to VIDMGR_CONTAINER_TYPE.
    AHC_BOOL 						PreRecordStatus;
    UINT32 							PreRecordLength; 	// PreEncode length, Unit ms
    AHC_VIDEO_PROPERTY 				v[AHC_MAX_VIDEO];
    AHC_AUDIO_PROPERTY 				a[AHC_MAX_AUDIO];
} AHC_MOVIE;

/*===========================================================================
 * Structure define
 *===========================================================================*/

// This function type should aligned with MMPF_VIDMGR_WriteFile()
typedef MMP_ERR (VR_WRITEFUNC)(void *buf, MMP_ULONG size, MMP_ULONG *wr_size, MMP_ULONG ulFileID);
AHC_BOOL AHC_VIDEO_ConfigRecTimeStamp(UINT32 TimeStampOp, AHC_RTC_TIME* sRtcTime, AHC_BOOL bInitIconBuf);

/*===========================================================================
 * Global variable
 *===========================================================================*/

static AHC_MOVIE m_sMovieConfig = 
{
	0, 							// nVideo
	0, 							// nAudio
	VIDMGR_CONTAINER_3GP, 		// ContainerType
	AHC_FALSE, 					// PreRecordStatus
    10000, 						// PreRecordLength
    {
        {   AHC_MEDIA_DISABLE,				// Usage
            MMPS_3GPRECD_VIDEO_FORMAT_H264,	// CodecType
            H264ENC_BASELINE_PROFILE,		// sCodecTypeSetting
            VIDENC_CURBUF_RT,				// sVidEncMode
            VIDRECD_FRAMERATE_30FPS,		// SampleRate
            300,							// FPSx10
            0, 								// Bitrate
            14,								// nMaxP 
            0,								// nMaxB
            51,								// nMaxQP
            VIDRECD_QUALITY_HIGH,			// CompressRatio
            2*1024*1024,					// ReservedSize
            VIDRECD_RESOL_1920x1088, 		// Resolution
            AHC_FALSE						// bStreaming
        },
        #if (AHC_MAX_VIDEO >= 2)
        {   AHC_MEDIA_DISABLE,				// Usage
            MMPS_3GPRECD_VIDEO_FORMAT_MJPEG,// CodecType
            H264ENC_BASELINE_PROFILE,		// sCodecTypeSetting, No used
            VIDENC_CURBUF_RT,         		// sVidEncMode, No used
            VIDRECD_FRAMERATE_30FPS,		// SampleRate
            300,							// FPSx10
            0,								// Bitrate
            0,								// nMaxP  
            0,								// nMaxB
            51,								// nMaxQP
            VIDRECD_QUALITY_HIGH,			// CompressRatio
            1*1024*1024,					// ReservedSize
            VIDRECD_RESOL_640x368,			// Resolution
            AHC_FALSE 						// bStreaming
        },
        #endif
    },
    {   AHC_MEDIA_DISABLE,					// Usage
        MMPS_3GPRECD_AUDIO_FORMAT_AAC, 		// CodecType
        16000,								// SampleRate
        32000,								// Bitrate
        1,									// ChannelType
        AHC_TRUE,							// bPreRecordDAC
        AHC_FALSE,							// bMute
    	AHC_FALSE 							// bStreaming
    }
};

static AHC_BOOL					m_bNightModeEnable = MMP_FALSE;

static MMPS_3GPRECD_FRAMERATE   m_sSlowMotionContainerFps = {1000, 30000};
static AHC_BOOL                 m_bSlowMotionEnable = MMP_FALSE;

#if (defined(VIDEO_REC_TIMELAPSE_EN) && VIDEO_REC_TIMELAPSE_EN)
static MMPS_3GPRECD_FRAMERATE   m_sTimeLapseFps = {1000, 30000};
static AHC_BOOL                 m_bTimeLapseEnable = MMP_FALSE;
#endif

static UINT8                    m_byTvSrcType = MMP_SNR_TVDEC_SRC_NO_READY;

#if (VR_SLOW_CARD_DETECT)
static AHC_BOOL                 m_bCardSlowDetected = AHC_FALSE;
#endif

static VR_WRITEFUNC             *VR_WriteFunc = NULL;

static MMP_BYTE                 m_CurVRFullName[MAX_FILE_NAME_SIZE];
static MMP_BYTE                 m_PrevVRFullName[MAX_FILE_NAME_SIZE];
#if (VIDRECD_MULTI_TRACK == 0)
static MMP_BYTE                 m_CurVRRearFullName[MAX_FILE_NAME_SIZE];
static MMP_BYTE                 m_CurVRUSBHFullName[MAX_FILE_NAME_SIZE];
#if (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) //(EMER_RECORD_DUAL_WRITE_ENABLE == 1)
static MMP_BYTE                 m_EmrVRRearFullName[MAX_FILE_NAME_SIZE];
static MMP_BYTE                 m_EmrVRUSBHFullName[MAX_FILE_NAME_SIZE];
#endif
#endif

static UINT8                    m_VideoRFileName[64];
#if (VIDRECD_MULTI_TRACK == 0)
static UINT8                    m_RearVideoRFileName[64];
static UINT8                    m_USBHVideoRFileName[64];
#if (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) //(EMER_RECORD_DUAL_WRITE_ENABLE == 1)
static UINT8                    m_EmrRearVideoRFileName[64];
static UINT8                    m_EmrUSBHVideoRFileName[64];
#endif
#endif

static UINT16                   gusCurVideoDirKey = 0;
static UINT8                    *gpbCurVideoFileName = &m_VideoRFileName[0];
#if (VIDRECD_MULTI_TRACK == 0)
static UINT8                    *gpbCurVideoRearFileName = &m_RearVideoRFileName[0];
static UINT8                    *gpbCurVideoUSBHFileName = &m_USBHVideoRFileName[0];
#endif

#if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE)
static UINT32                   glThumbFileId = NULL;
static UINT8                    m_ThumbJpgFileName[64];
static MMP_BYTE                 m_CurThumbJpgFullName[MAX_FILE_NAME_SIZE];
static UINT8                    m_chEmerThumbFileName[64];
static MMP_BYTE                 m_chEmerThumbFullName[MAX_FILE_NAME_SIZE];
static UINT8                    m_ThumbJpgFileName_Rear[64];
static MMP_BYTE                 m_CurThumbJpgFullName_Rear[MAX_FILE_NAME_SIZE];
static UINT8                    m_chEmerThumbFileName_Rear[64];
static MMP_BYTE                 m_chEmerThumbFullName_Rear[MAX_FILE_NAME_SIZE];
static UINT8                    m_ThumbJpgFileName_USBH[64];
static MMP_BYTE                 m_CurThumbJpgFullName_USBH[MAX_FILE_NAME_SIZE];
static UINT8                    m_chEmerThumbFileName_USBH[64];
static MMP_BYTE                 m_chEmerThumbFullName_USBH[MAX_FILE_NAME_SIZE];
#endif

#if (AHC_SHAREENC_SUPPORT)
char                            m_VideoRFullFileName[MAX_FILE_NAME_SIZE];
static UINT8                    m_ShareRecFileName[64];
static UINT8                    m_2ndVideoParkFileName[64];
static char                     m_ShareRecFullFileName[MAX_FILE_NAME_SIZE];
//static AHC_BOOL                 m_bStartShareVR = AHC_FALSE;
static AHC_BOOL                 m_bShareVRPostDone = AHC_TRUE;

MMP_BOOL                        m_bFirstShareFile = AHC_FALSE;
static UINT32                   m_uiShareRecInterval = 0;
AHC_BOOL                        m_bStartShareRec = AHC_FALSE;
AHC_BOOL                        m_bShareRecPostDone = AHC_TRUE;
AHC_RTC_TIME                    m_DualRecStartRtcTime;
static MMP_USHORT               m_usShareRecResolIdx = VIDRECD_RESOL_640x480;
static MMP_ULONG                m_ulDualPreEncodeTime = 0;
#endif

#if (AHC_EMERGENTRECD_SUPPORT)//(EMER_RECORD_DUAL_WRITE_ENABLE == 1)
static AHC_KERNAL_EMERGENCY_STOP_STEP   m_bKernalEmergencyStopStep = AHC_KERNAL_EMERGENCY_AHC_DONE;
static AHC_BOOL                 m_bEmerVRPostDone = AHC_TRUE;
static UINT32                   m_uiEmerRecordInterval;

static UINT16                   m_uwEmerVRDirKey;
static UINT8                    m_chEmerVRFileName[64];
static MMP_BYTE                 m_chEmerVRFullName[MAX_FILE_NAME_SIZE];
#endif

MMP_BOOL                        m_bStartEmerVR = AHC_FALSE;

#if (AHC_EMERGENTRECD_SUPPORT)
static AHC_BOOL                 m_bNormal2Emergency = AHC_FALSE;
#endif
static UINT8                    m_bDelNormRecAfterEmr = AHC_FALSE;

#if (AHC_ENABLE_VIDEO_STICKER)
UINT32                          m_uiVideoStampBufIndex  = 0;
#endif

AHC_BOOL                        m_bSeamlessRecord = AHC_FALSE;

static AHC_RTC_TIME             m_VideoRecStartRtcTime;
#if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
static AHC_RTC_TIME             m_EmerRecStartRtcTime;
#endif

static UINT8                    m_ubRecordAGain = AHC_DEFAULT_VR_MIC_AGAIN;
static UINT8                    m_ubRecordDGain = AHC_DEFAULT_VR_MIC_DGAIN; // 8328 chip 0x51 is 0DB for Dgain.

static AHC_BOOL                 m_VidRecdCardSlowStop   = AHC_FALSE;
static AHC_BOOL                 m_APStopVideoRecord     = AHC_FALSE;
static ULONG                    m_APStopVideoRecord_VidTime = 0;
static ULONG                    m_APStopVideoRecord_AudTime = 0;

#if (((GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO)) || \
         ((GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)))
MMP_USHORT  filestreamtype  = 0xFF;
#endif

AHC_BOOL                        gbFirstRecordSetFile = AHC_TRUE;
UINT8                           gbAudioOn = 0;
AHC_BOOL                        gbDeleteFile = AHC_FALSE;
UINT32                          gulVidRecdExceptionCode = AHC_VIDRECD_MODE_API_MAX;

pfAHC_VIDEO_SetRecordModeAPI    pfAhcVidSetRecModeInit = AHC_VIDEO_SetRecordModeInit;
pfAHC_VIDEO_SetRecordModeAPI    pfAhcVidSetRecModeUnInit = AHC_VIDEO_SetRecordModeUnInit;
   
AHC_VIDRECD_MODE_ACTION_LIST    gpfAhcVidSetRecModeActoinList[AHC_VIDRECD_FLOW_TYPE_MAX] = 
{    
    { //AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_RECORD
        NULL,                                       //AHC_VIDRECD_MODE_API_CLOSE_PREVIOUS_FILE = 0,
        NULL,                                       //AHC_VIDRECD_MODE_API_CYCLIC_DELETE_FILES,
        AHC_VIDEO_SetRecordModeMiscPreprocess,      //AHC_VIDRECD_MODE_API_MISC_PREPROCESS,
        AHC_VIDEO_SetRecordModeSetProfile,          //AHC_VIDRECD_MODE_API_SET_PROFILE,
        AHC_VIDEO_SetRecordModeSetBitRate,          //AHC_VIDRECD_MODE_API_SET_BITRATE,
        AHC_VIDEO_SetRecordModeSetContainerType,    //AHC_VIDRECD_MODE_API_SET_CONTAINER_TYPE,
        AHC_VIDEO_SetRecordModeSetVideoCodecType,   //AHC_VIDRECD_MODE_API_SET_VIDEO_CODEC_TYPE,
        AHC_VIDEO_SetRecordModeSetH264BufferMode,   //AHC_VIDRECD_MODE_API_SET_H264_BUFFER_MODE,
        AHC_VIDEO_SetRecordModeSetFrameRate,        //AHC_VIDRECD_MODE_API_SET_FRAMERATE,
        AHC_VIDEO_SetRecordModeSetP_BFrameCount,    //AHC_VIDRECD_MODE_API_SET_P_BFRAME_COUNT,
        AHC_VIDEO_SetRecordModeSetAudioEncode,      //AHC_VIDRECD_MODE_API_SET_AUDIO_ENCODE,
        AHC_VIDEO_SetRecordModeSetTimeLimit,        //AHC_VIDRECD_MODE_API_SET_TIME_LIMIT,
        AHC_VIDEO_SetRecordModeSetDualEncode,       //AHC_VIDRECD_MODE_API_SET_DUALENCODE,
        NULL,                                       //AHC_VIDRECD_MODE_API_START_PRE_ENCODE
        AHC_VIDEO_SetRecordModePreSetFilename,      //AHC_VIDRECD_MODE_API_PRESET_FILENAME,
        AHC_VIDEO_SetRecordModeRegisterCallback,    //AHC_VIDRECD_MODE_API_REGISTER_CALLBACK,
        AHC_VIDEO_SetRecordModeSetSeamless,         //AHC_VIDRECD_MODE_API_SET_SEAMLESS,
        AHC_VIDEO_SetRecordModeSetEmergency,        //AHC_VIDRECD_MODE_API_SET_EMERGENCY,
        AHC_VIDEO_SetRecordModePreAddFilename,      //AHC_VIDRECD_MODE_API_PREADD_FILENAME,
        AHC_VIDEO_SetRecordModeStart,               //AHC_VIDRECD_MODE_API_START_RECORD,
        AHC_VIDEO_SetRecordModePostSetFilename,     //AHC_VIDRECD_MODE_API_POSTSET_FILENAME,
        AHC_VIDEO_SetRecordModeMiscPostprocess,     //AHC_VIDRECD_MODE_API_MISC_POSTPROCESS, 
        AHC_VIDEO_SetRecordExceptionHandler,        //AHC_VIDRECD_MODE_API_EXCEPTION_HANDLER
    },

    { //AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD
        NULL,                                       //AHC_VIDRECD_MODE_API_CLOSE_PREVIOUS_FILE = 0,
        NULL,                                       //AHC_VIDRECD_MODE_API_CYCLIC_DELETE_FILES,
        AHC_VIDEO_SetRecordModeMiscPreprocess,      //AHC_VIDRECD_MODE_API_MISC_PREPROCESS,
        AHC_VIDEO_SetRecordModeSetProfile,          //AHC_VIDRECD_MODE_API_SET_PROFILE,
        AHC_VIDEO_SetRecordModeSetBitRate,          //AHC_VIDRECD_MODE_API_SET_BITRATE,
        AHC_VIDEO_SetRecordModeSetContainerType,    //AHC_VIDRECD_MODE_API_SET_CONTAINER_TYPE,
        AHC_VIDEO_SetRecordModeSetVideoCodecType,   //AHC_VIDRECD_MODE_API_SET_VIDEO_CODEC_TYPE,
        AHC_VIDEO_SetRecordModeSetH264BufferMode,   //AHC_VIDRECD_MODE_API_SET_H264_BUFFER_MODE,
        AHC_VIDEO_SetRecordModeSetFrameRate,        //AHC_VIDRECD_MODE_API_SET_FRAMERATE,
        AHC_VIDEO_SetRecordModeSetP_BFrameCount,    //AHC_VIDRECD_MODE_API_SET_P_BFRAME_COUNT,
        AHC_VIDEO_SetRecordModeSetAudioEncode,      //AHC_VIDRECD_MODE_API_SET_AUDIO_ENCODE,
        AHC_VIDEO_SetRecordModeSetTimeLimit,        //AHC_VIDRECD_MODE_API_SET_TIME_LIMIT,
        AHC_VIDEO_SetRecordModeSetDualEncode,       //AHC_VIDRECD_MODE_API_SET_DUALENCODE,
        AHC_VIDEO_SetRecordEnterPreEncode,          //AHC_VIDRECD_MODE_API_START_PRE_ENCODE
        NULL,                                       //AHC_VIDRECD_MODE_API_PRESET_FILENAME,
        NULL,                                       //AHC_VIDRECD_MODE_API_REGISTER_CALLBACK,
        AHC_VIDEO_SetRecordModeSetSeamless,         //AHC_VIDRECD_MODE_API_SET_SEAMLESS,
        AHC_VIDEO_SetRecordModeSetEmergency,        //AHC_VIDRECD_MODE_API_SET_EMERGENCY,
        NULL,                                       //AHC_VIDRECD_MODE_API_PREADD_FILENAME,
        NULL,                                       //AHC_VIDRECD_MODE_API_START_RECORD,
        NULL,                                       //AHC_VIDRECD_MODE_API_POSTSET_FILENAME,
        NULL,                                       //AHC_VIDRECD_MODE_API_MISC_POSTPROCESS,    
        NULL,                                       //AHC_VIDRECD_MODE_API_EXCEPTION_HANDLER
    },
        
    { //AHC_VIDRECD_FLOW_TYPE_PRERECORD_TO_RECORD
        NULL,                                       //AHC_VIDRECD_MODE_API_CLOSE_PREVIOUS_FILE = 0,
        NULL,                                       //AHC_VIDRECD_MODE_API_CYCLIC_DELETE_FILES,
        NULL,                                       //AHC_VIDRECD_MODE_API_MISC_PREPROCESS,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_PROFILE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_BITRATE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_CONTAINER_TYPE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_VIDEO_CODEC_TYPE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_H264_BUFFER_MODE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_FRAMERATE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_P_BFRAME_COUNT,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_AUDIO_ENCODE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_TIME_LIMIT,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_DUALENCODE,
        NULL,                                       //AHC_VIDRECD_MODE_API_START_PRE_ENCODE
        AHC_VIDEO_SetRecordModePreSetFilename,      //AHC_VIDRECD_MODE_API_PRESET_FILENAME,
        AHC_VIDEO_SetRecordModeRegisterCallback,    //AHC_VIDRECD_MODE_API_REGISTER_CALLBACK,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_SEAMLESS,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_EMERGENCY,
        AHC_VIDEO_SetRecordModePreAddFilename,      //AHC_VIDRECD_MODE_API_PREADD_FILENAME,
        AHC_VIDEO_SetRecordModeStart,               //AHC_VIDRECD_MODE_API_START_RECORD,
        AHC_VIDEO_SetRecordModePostSetFilename,     //AHC_VIDRECD_MODE_API_POSTSET_FILENAME,
        AHC_VIDEO_SetRecordModeMiscPostprocess,     //AHC_VIDRECD_MODE_API_MISC_POSTPROCESS,            
        AHC_VIDEO_SetRecordExceptionHandler,        //AHC_VIDRECD_MODE_API_EXCEPTION_HANDLER
    },    
        
    { //AHC_VIDRECD_FLOW_TYPE_SEAMLESS_START_NEXT_RECORD
        AHC_VIDEO_ClosePreviousFile,                //AHC_VIDRECD_MODE_API_CLOSE_PREVIOUS_FILE = 0,
        AHC_VIDEO_CyclicDeleteFiles,                //AHC_VIDRECD_MODE_API_CYCLIC_DELETE_FILES,
        AHC_VIDEO_SetRecordModeMiscPreprocess,      //AHC_VIDRECD_MODE_API_MISC_PREPROCESS,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_PROFILE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_BITRATE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_CONTAINER_TYPE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_VIDEO_CODEC_TYPE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_H264_BUFFER_MODE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_FRAMERATE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_P_BFRAME_COUNT,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_AUDIO_ENCODE,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_TIME_LIMIT,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_DUALENCODE,
        NULL,                                       //AHC_VIDRECD_MODE_API_START_PRE_ENCODE
        AHC_VIDEO_SetRecordModePreSetFilename,      //AHC_VIDRECD_MODE_API_PRESET_FILENAME,
        NULL,                                       //AHC_VIDRECD_MODE_API_REGISTER_CALLBACK,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_SEAMLESS,
        NULL,                                       //AHC_VIDRECD_MODE_API_SET_EMERGENCY,
        AHC_VIDEO_SetRecordModePreAddFilename,      //AHC_VIDRECD_MODE_API_PREADD_FILENAME,
        AHC_VIDEO_SetRecordModeStart,               //AHC_VIDRECD_MODE_API_START_RECORD,
        AHC_VIDEO_SetRecordModePostSetFilename,     //AHC_VIDRECD_MODE_API_POSTSET_FILENAME,
        NULL,                                       //AHC_VIDRECD_MODE_API_MISC_POSTPROCESS, 
        AHC_VIDEO_SetRecordExceptionHandler,        //AHC_VIDRECD_MODE_API_EXCEPTION_HANDLER
    }    
};

AHC_VIDEO_RECD_CMD_STATE gAhcVidRecdCmdStateIdle = 
{
    AHC_VIDRECD_IDLE,
    AHC_VIDEO_RecdCmdStateExecIdle,
    AHC_VIDEO_GetCmdState
};

AHC_VIDEO_RECD_CMD_STATE gAhcVidRecdCmdStateInit = 
{
    AHC_VIDRECD_INIT,
    AHC_VIDEO_RecdCmdStateExecInit,        
    AHC_VIDEO_GetCmdState
};

AHC_VIDEO_RECD_CMD_STATE gAhcVidRecdCmdStateStart = 
{
    AHC_VIDRECD_START,
    AHC_VIDEO_RecdCmdStateExecStart,
    AHC_VIDEO_GetCmdState    
};

AHC_VIDEO_RECD_CMD_STATE gAhcVidRecdCmdStatePreRecd = 
{
    AHC_VIDRECD_PRERECD,
    AHC_VIDEO_RecdCmdStateExecPreRecd,
    AHC_VIDEO_GetCmdState    
};

AHC_VIDEO_RECD_CMD_STATE gAhcVidRecdCmdStateStop = 
{
    AHC_VIDRECD_STOP,
    AHC_VIDEO_RecdCmdStateExecStop,
    AHC_VIDEO_GetCmdState    
};

AHC_VIDEO_RECD_CONTEXT gAhcVidRecdContext = 
{
    NULL,
    AHC_VIDEO_RecdContextSetState,
    AHC_VIDEO_RecdContextGetState,
    AHC_VIDEO_RecdContextSetRecordMode
};

/*===========================================================================
 * Extern varible
 *===========================================================================*/ 

#if (SUPPORT_ADAS)
extern MMP_ULONG    ulADASTimeLimit;
#endif

extern AHC_BOOL     gbAhcDbgBrk;

extern MMP_USHORT   gsAhcPrmSensor;
extern MMP_USHORT   gsAhcScdSensor;

#if(UVC_HOST_VIDEO_ENABLE)
extern MMP_USHORT   gsAhcUsbhSensor;
#endif

extern MMP_USHORT   gsStillZoomIndex;

/*===========================================================================
 * Extern function
 *===========================================================================*/ 

MMP_ERR MMPF_Display_StopFlushWin(MMP_UBYTE ubSnrSel, MMP_USHORT usIBCPipe, MMP_BOOL bEnable);
UINT32 AHC_VIDEO_RecordPostWriteInfo(UINT32 ulFileID);
AHC_BOOL AIHC_DCFDT_DeleteFileInFS(PSDATETIMEDCFDB pDB, char* szFullPath, AHC_BOOL bIsFileReadOnly );

/*===========================================================================
 * Main body
 *===========================================================================*/

#if 0
void _____Internal_Functions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : AIHC_VIDEO_MapFromRawVRes
//  Description : Map Integer Video resolutions to AHC resolution
//------------------------------------------------------------------------------ 
AHC_BOOL AIHC_VIDEO_MapFromRawVRes(UINT16 w, UINT16 h, AHC_VIDRECD_RESOL *AhcRes)
{
    if (w >= 2688) {
        *AhcRes = AHC_VIDRECD_RESOL_3200x1808;
    } 
    else if (w >= 2560) {
        *AhcRes = AHC_VIDRECD_RESOL_2560x1440;
    }
    else if (w >= 2304) {
        *AhcRes = AHC_VIDRECD_RESOL_2304x1296;
    }
    else if (w >= 1920) {
        *AhcRes = AHC_VIDRECD_RESOL_1920x1088;
    }
    else if (w >= 1600) {
        *AhcRes = AHC_VIDRECD_RESOL_1600x900;
    }
    else if (w >= 1440) {
        *AhcRes = AHC_VIDRECD_RESOL_1440x1088;
    }
    else if (w >= 1280) {
        if (h >= 960) {
            *AhcRes = AHC_VIDRECD_RESOL_1280x960;
        }
        else {
            *AhcRes = AHC_VIDRECD_RESOL_1280x720;
        }
    }
    else if (w >= 720) {
        *AhcRes = AHC_VIDRECD_RESOL_720x480;
    }
    else if (w >= 640) {
        if (h == 360)
            *AhcRes = AHC_VIDRECD_RESOL_640x360;
        else
            *AhcRes = AHC_VIDRECD_RESOL_640x480;
    }
    else {
        *AhcRes = AHC_VIDRECD_RESOL_MAX;
        return AHC_FALSE;
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AIHC_VIDEO_MapToRawVRes
//  Description : Map AHC resolution to Integer Video resolutions
//------------------------------------------------------------------------------
AHC_BOOL AIHC_VIDEO_MapToRawVRes(AHC_VIDRECD_RESOL res, UINT16 *w, UINT16 *h)
{
    switch(res) {
    case AHC_VIDRECD_RESOL_640x360:
        *w = 640;
        *h = 360;
        break;
    case AHC_VIDRECD_RESOL_640x480:
        *w = 640;
        *h = 480;
        break;
    case AHC_VIDRECD_RESOL_720x480:
        *w = 720;
        *h = 480;
        break;
    case AHC_VIDRECD_RESOL_1280x720:
        *w = 1280;
        *h = 720;
        break;
    case AHC_VIDRECD_RESOL_1280x960:
        *w = 1280;
        *h = 960;
        break;     
    case AHC_VIDRECD_RESOL_1440x1088:
        *w = 1440;
        *h = 1080;
        break; 
    case AHC_VIDRECD_RESOL_1600x900:
        *w = 1600;
        *h = 912;
        break;      
    case AHC_VIDRECD_RESOL_1920x1088:
        *w = 1920;
        *h = 1080;
        break;
    case AHC_VIDRECD_RESOL_2304x1296:
        *w = 2304;
        *h = 1296;
        break;
    case AHC_VIDRECD_RESOL_2560x1440:
        *w = 2560;
        *h = 1440;
        break;
    case AHC_VIDRECD_RESOL_3200x1808:
        *w = 3200;
        *h = 1800;
        break;		  
    default:
        *h = 0; 
        *w = 0;
        return MMP_FALSE;
        break;
    }
    return MMP_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AIHC_VIDEO_GetMovieCfgEx
//  Description :
//------------------------------------------------------------------------------
/**
 * Get the config parameter of the movie
 * @param i[in] index of the media track if required.
 * @param wCommand[in] parameter to be get
 * @param wOp[out] The pointer of the parameter. Typically UINT32 type.
 * @return AHC_TRUE if supported
 */
AHC_BOOL AIHC_VIDEO_GetMovieCfgEx(UINT16 i, UINT32 wCommand, UINT32 *wOp)
{
    AHC_BOOL    bCheckAudio = AHC_FALSE;
    AHC_BOOL    bCheckVideo = AHC_FALSE;
    UINT32      value = 0;

    switch(wCommand) {
    case AHC_MAX_PFRAME_NUM:
        bCheckVideo = AHC_TRUE;
        value = m_sMovieConfig.v[i].nMaxP;
        break;
    case AHC_MAX_BFRAME_NUM:
        bCheckVideo = AHC_TRUE;
        value = m_sMovieConfig.v[i].nMaxB;
        break;
    case AHC_MAX_BRC_QSCALE:
        bCheckVideo = AHC_TRUE;
        value = m_sMovieConfig.v[i].nMaxQP;
        break;
    case AHC_VIDEO_RESOLUTION:
        bCheckVideo = AHC_TRUE;
        value = m_sMovieConfig.v[i].Resolution;
        break;
    case AHC_FRAME_RATE:
        bCheckVideo = AHC_TRUE;
        value = m_sMovieConfig.v[i].SampleRate;
        break;
    case AHC_FRAME_RATEx10:
        bCheckVideo = AHC_TRUE;
        value = m_sMovieConfig.v[i].FPSx10;
        break;
    case AHC_VIDEO_BITRATE:
        if (i == 1) {	// i is 1 for network video streaming
            value = m_sMovieConfig.v[i].Bitrate;
            break;
        }
        
        bCheckVideo = AHC_TRUE;
        
        {
            UINT32 ulBitrate = 0;

        #if (MENU_MOVIE_FRAME_RATE_EN)
			switch(MenuSettingConfig()->uiMOVFrameRate)
		    {
			#if (MENU_MOVIE_FRAME_RATE_30_EN)
			    case MOVIE_FRAME_RATE_30FPS:
					ulBitrate = MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[m_sMovieConfig.v[i].Resolution][m_sMovieConfig.v[i].CompressRatio];
		    	break;
		    #endif
		    #if (MENU_MOVIE_FRAME_RATE_20_EN)
			    case MOVIE_FRAME_RATE_20FPS:
					ulBitrate = MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[m_sMovieConfig.v[i].Resolution][m_sMovieConfig.v[i].CompressRatio]*2/3;
			    break;
			#endif
			#if (MENU_MOVIE_FRAME_RATE_15_EN)
				case MOVIE_FRAME_RATE_15FPS:
					ulBitrate = MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[m_sMovieConfig.v[i].Resolution][m_sMovieConfig.v[i].CompressRatio]/2;
				break;
			#endif
			#if (MENU_MOVIE_FRAME_RATE_10_EN)
			    case MOVIE_FRAME_RATE_10FPS:
					ulBitrate = MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[m_sMovieConfig.v[i].Resolution][m_sMovieConfig.v[i].CompressRatio]/3;
			    break;
			#endif
		    }
		#else
			if (VIDRECD_FRAMERATE_30FPS == m_sMovieConfig.v[i].SampleRate) {
	            ulBitrate = MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[m_sMovieConfig.v[i].Resolution][m_sMovieConfig.v[i].CompressRatio];
	        }
	        else {
	            ulBitrate = MMPS_3GPRECD_GetConfig()->ulFps60BitrateMap[m_sMovieConfig.v[i].Resolution][m_sMovieConfig.v[i].CompressRatio];
	        }
		#endif

			if (MenuSettingConfig()->uiMOVPreRecord == MOVIE_PRE_RECORD_ON)
			{
				if (ulBitrate > AHC_VIDEO_MAXBITRATE_PRERECORD)
				{
					ulBitrate = AHC_VIDEO_MAXBITRATE_PRERECORD;
				}
			}

        	value = ulBitrate;
        }
        break;
    #if (AHC_SHAREENC_SUPPORT)
    case AHC_VIDEO_2ND_BITRATE:
        bCheckVideo = AHC_TRUE;
        if (VIDRECD_FRAMERATE_30FPS == m_sMovieConfig.v[i].SampleRate) {
            value = MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[m_sMovieConfig.v[i].Resolution][m_sMovieConfig.v[i].CompressRatio];
        }
        else {
            value = MMPS_3GPRECD_GetConfig()->ulFps60BitrateMap[m_sMovieConfig.v[i].Resolution][m_sMovieConfig.v[i].CompressRatio];
        }
        break;
    #endif
    case AHC_VIDEO_CODEC_TYPE:
        bCheckVideo = AHC_TRUE;
        value = m_sMovieConfig.v[i].CodecType;
        break;
    case AHC_VIDEO_CODEC_TYPE_SETTING:
        bCheckVideo = AHC_TRUE;

        switch (m_sMovieConfig.v[i].CodecType) {
            case MMPS_3GPRECD_VIDEO_FORMAT_H264:
                value = (UINT32)m_sMovieConfig.v[i].sCodecTypeSetting;
            case MMPS_3GPRECD_VIDEO_FORMAT_MJPEG:
                break;
            case MMPS_3GPRECD_VIDEO_FORMAT_YUV422:
                break;
            case MMPS_3GPRECD_VIDEO_FORMAT_YUV420:
                break;
            default:
                break;
        }
        break;
    case AHC_VIDEO_ENCODE_MODE:
        bCheckVideo = AHC_TRUE;
        value = (UINT32)m_sMovieConfig.v[i].sVidEncMode;
        break;
    case AHC_VIDEO_COMPRESSION_RATIO:
        bCheckVideo = AHC_TRUE;
        value = m_sMovieConfig.v[i].CompressRatio;
        break;
    case AHC_VIDEO_RESERVED_SIZE:
        bCheckVideo = AHC_TRUE;
        value= m_sMovieConfig.v[i].ReservedSize;
        break;
    case AHC_VIDEO_STREAMING:
        bCheckVideo = AHC_TRUE;
        value = m_sMovieConfig.v[i].bStreaming;
        break;
    case AHC_VIDEO_USAGE:
    	bCheckVideo = AHC_TRUE;
    	value = m_sMovieConfig.v[i].Usage;
    	break;
    /* Audio Config */
    case AHC_AUD_MUTE_END:
        bCheckAudio = AHC_TRUE;
        value = m_sMovieConfig.a[i].bMute;
        break;
    case AHC_AUD_SAMPLE_RATE:
        bCheckAudio = AHC_TRUE;
        value = m_sMovieConfig.a[i].SampleRate;
        break;
    case AHC_AUD_CHANNEL_CONFIG:
        bCheckAudio = AHC_TRUE;
        value = m_sMovieConfig.a[i].ChannelType;
        break;
    case AHC_AUD_BITRATE:
        bCheckAudio = AHC_TRUE;
        value = m_sMovieConfig.a[i].Bitrate;
        break;
    case AHC_AUD_CODEC_TYPE:
        bCheckAudio = AHC_TRUE;
        value = m_sMovieConfig.a[i].CodecType;
        break;
    case AHC_AUD_PRERECORD_DAC:
        bCheckAudio = AHC_TRUE;
        value = m_sMovieConfig.a[i].bPreRecordDAC;
        break;       
    case AHC_AUD_STREAMING:
    	bCheckAudio = AHC_TRUE;
        value = m_sMovieConfig.a[i].bStreaming;
        break;
   	/* Movie Config */
    case AHC_CLIP_CONTAINER_TYPE:
        value = m_sMovieConfig.ContainerType;
        break;
 	case AHC_VIDEO_PRERECORD_LENGTH:
        value = m_sMovieConfig.PreRecordLength;
        break;
    case AHC_VIDEO_PRERECORD_STATUS:
        value = m_sMovieConfig.PreRecordStatus;
        break;
    default:
        return AHC_FALSE;
        break;
    }

    if (bCheckAudio) {
        if (i >= AHC_MAX_AUDIO) {
            return AHC_FALSE;
        }
    }
    if (bCheckVideo) {
        if (i >= AHC_MAX_VIDEO) {
            return AHC_FALSE;
        }
    }
    
    *wOp = value;

    return AHC_TRUE;
}

#if 0
void _____Movie_Config_Functions_____(){}
#endif

#if defined(VIDEO_REC_TIMELAPSE_EN) && (VIDEO_REC_TIMELAPSE_EN)
//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetTimeLapseBitrate
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_GetTimeLapseBitrate(UINT32 ulFrameRate, UINT32 ulTimeLapseSetting, UINT32 *pulVidBitRate, UINT32 *pulAudBitRate)
{
    UINT32 ulTimeLapseRatio = 0;

    switch(ulTimeLapseSetting){
        case COMMON_VR_TIMELAPSE_5SEC:
            ulTimeLapseRatio = 5;
            break;
        case COMMON_VR_TIMELAPSE_10SEC:
            ulTimeLapseRatio = 10;
            break;
        case COMMON_VR_TIMELAPSE_30SEC:
            ulTimeLapseRatio = 30;
            break;
        case COMMON_VR_TIMELAPSE_60SEC:
            ulTimeLapseRatio = 60;
            break;
        default:
        case COMMON_VR_TIMELAPSE_1SEC:
            ulTimeLapseRatio = 1;
            break;
    }
    
    ulFrameRate = ulFrameRate * ulTimeLapseRatio;
    
    *pulVidBitRate = (*pulVidBitRate + ulFrameRate - 1) / ulFrameRate;
    *pulAudBitRate = 0;   // Mute
}
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetMovieConfig
//  Description :
//------------------------------------------------------------------------------
/**
 * Set the config parameter of the movie

 This API configures both record and playback movie files

 Parameters:

 @param[in] i index of the media track if required.
 @param[in] wCommand Config type.
 @param[in] wOp      config value.
 @retval AHC_TRUE Success.
*/
AHC_BOOL AHC_VIDEO_SetMovieConfig(UINT16 i, AHC_MOVIE_CONFIG wCommand, UINT32 wOp)
{
    char *szErr;

    if (wCommand < AHC_VIDEO_CONFIG_MAX) {
        if (i >= AHC_MAX_VIDEO) {
            szErr = "video stream";
            goto config_error;
        }
    } 
    else if (wCommand < AHC_AUD_CONFIG_MAX) {
        if (i >= AHC_MAX_AUDIO) {
            szErr = "audio stream";
            goto config_error;
        }
    }

    switch(wCommand){
    case AHC_MAX_PFRAME_NUM:
        m_sMovieConfig.v[i].nMaxP = wOp;
        break;
    case AHC_MAX_BFRAME_NUM:
        if (wOp != 0)
            m_sMovieConfig.v[i].nMaxB = 2;
        else
            m_sMovieConfig.v[i].nMaxB = 0;
        break;
    case AHC_MAX_BRC_QSCALE:
        m_sMovieConfig.v[i].nMaxQP = wOp;
        break;
    case AHC_VIDEO_RESOLUTION:
        m_sMovieConfig.v[i].Resolution = wOp;
        break;
    case AHC_FRAME_RATE:
    case AHC_FRAME_RATEx10:
        if (wCommand == AHC_FRAME_RATEx10) {
            wOp /= 10;
        }
        m_sMovieConfig.v[i].SampleRate = wOp;
        m_sMovieConfig.v[i].FPSx10 = wOp * 10;

        switch (wOp) {
		case 7:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_7d5FPS;
            m_sMovieConfig.v[i].FPSx10 = 75;
		break;
		case 10:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_10FPS;
		break;
		case 12:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_12FPS;
		break;
		case 15:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_15FPS;
		break;
		case 20:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_20FPS;
		break;
		case 24:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_24FPS;
		break;
		case 25:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_25FPS;
		break;
		case 30:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_30FPS;
		break;
		case 50:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_50FPS;
		break;
		case 60:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_60FPS;
		break;
		case 100:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_100FPS;
		break;
		case 120:
			m_sMovieConfig.v[i].SampleRate = VIDRECD_FRAMERATE_120FPS;
		break;
        }
        break;
    case AHC_VIDEO_BITRATE:
        m_sMovieConfig.v[i].Bitrate = wOp;
        break;
    case AHC_VIDEO_CODEC_TYPE:
        switch (wOp) {
        case AHC_MOVIE_VIDEO_CODEC_H264:
            wOp = MMPS_3GPRECD_VIDEO_FORMAT_H264;
            break;
        case AHC_MOVIE_VIDEO_CODEC_MJPEG:
            wOp = MMPS_3GPRECD_VIDEO_FORMAT_MJPEG;
            break;
        case AHC_MOVIE_VIDEO_CODEC_YUV422:
            wOp = MMPS_3GPRECD_VIDEO_FORMAT_YUV422;
            break;
		case AHC_MOVIE_VIDEO_CODEC_NONE:
			wOp = MMPS_3GPRECD_VIDEO_FORMAT_OTHERS;
			break;
        default:
            return AHC_FALSE;
            break;
        }
        
        m_sMovieConfig.v[i].CodecType = wOp;
        break;
    case AHC_VIDEO_CODEC_TYPE_SETTING:
        switch (m_sMovieConfig.v[i].CodecType) {
            case MMPS_3GPRECD_VIDEO_FORMAT_H264:
                wOp = (VIDENC_PROFILE)wOp;
                break;
            case MMPS_3GPRECD_VIDEO_FORMAT_MJPEG:
                break;
            case MMPS_3GPRECD_VIDEO_FORMAT_YUV422:
                break;
            case MMPS_3GPRECD_VIDEO_FORMAT_YUV420:
                break;
            default:
                break;
        }

        m_sMovieConfig.v[i].sCodecTypeSetting = wOp;
        break;
    case AHC_VIDEO_ENCODE_MODE:
        m_sMovieConfig.v[i].sVidEncMode = (VIDENC_CURBUF_MODE)wOp;
        break;
    case AHC_VIDEO_COMPRESSION_RATIO:
        m_sMovieConfig.v[i].CompressRatio = wOp;
        break;
    case AHC_VIDEO_RESERVED_SIZE:
        m_sMovieConfig.v[i].ReservedSize = wOp;
        break; 
    case AHC_VIDEO_STREAMING:
        m_sMovieConfig.v[i].bStreaming = wOp;
        break;
    case AHC_VIDEO_USAGE:
    	m_sMovieConfig.v[i].Usage = wOp;
        break;
    /* Audio Config */
    case AHC_AUD_MUTE_END:
        // Always ignore this value
        break;
    case AHC_AUD_SAMPLE_RATE:
        if ((wOp == 22050) || (wOp == 32000) || (wOp == 44100) || (wOp == 48000)) {
        	m_sMovieConfig.a[i].SampleRate = wOp;
        }	
        break;
    case AHC_AUD_CHANNEL_CONFIG:
        if (AHC_AUDIO_CHANNEL_STEREO == wOp) {
            m_sMovieConfig.a[i].ChannelType = MMPS_AUDIO_LINEIN_DUAL;
        }
        else if (AHC_AUDIO_CHANNEL_MONO_R == wOp) {
            m_sMovieConfig.a[i].ChannelType = MMPS_AUDIO_LINEIN_R;
        }
        else if (AHC_AUDIO_CHANNEL_MONO_L == wOp) {
            m_sMovieConfig.a[i].ChannelType = MMPS_AUDIO_LINEIN_L;
        }
        else {
            return AHC_FALSE;
        }
        break;
    case AHC_AUD_BITRATE:
        m_sMovieConfig.a[i].Bitrate = wOp;
        break;
    case AHC_AUD_CODEC_TYPE:
        if (AHC_MOVIE_AUDIO_CODEC_AAC == wOp) {
            m_sMovieConfig.a[i].CodecType = MMPS_3GPRECD_AUDIO_FORMAT_AAC;
        }
        else if (AHC_MOVIE_AUDIO_CODEC_ADPCM == wOp) {
            m_sMovieConfig.a[i].CodecType = MMPS_3GPRECD_AUDIO_FORMAT_ADPCM;
        }
        else if (AHC_MOVIE_AUDIO_CODEC_MP3 == wOp) {
            m_sMovieConfig.a[i].CodecType = MMPS_3GPRECD_AUDIO_FORMAT_MP3;
        }
        #if 0
        else if (AHC_MOVIE_AUDIO_CODEC_G711 == wOp) {
            m_sMovieConfig.a[i].CodecType = wOp;
        }
        #endif
        else if (AHC_MOVIE_AUDIO_CODEC_PCM == wOp) {
            m_sMovieConfig.a[i].CodecType = MMPS_3GPRECD_AUDIO_FORMAT_PCM;
        }
        else {
        	printc(FG_RED("unknown audio format AHC:%d\r\n"), wOp);
            return AHC_FALSE;
        }
        break;
    case AHC_AUD_PRERECORD_DAC:
        m_sMovieConfig.a[i].bPreRecordDAC = wOp;
        break;
    case AHC_AUD_STREAMING:
        m_sMovieConfig.a[i].bStreaming = wOp;
        break;
	/* Movie Config */
    case AHC_CLIP_CONTAINER_TYPE:
        if (AHC_MOVIE_CONTAINER_AVI == wOp) {
            m_sMovieConfig.ContainerType = VIDMGR_CONTAINER_AVI;
        }
        else if (AHC_MOVIE_CONTAINER_3GP == wOp) {
            m_sMovieConfig.ContainerType = VIDMGR_CONTAINER_3GP;
        }
        else {
            return AHC_FALSE;
        }
        break;
    case AHC_VIDEO_PRERECORD_LENGTH:
        m_sMovieConfig.PreRecordLength = wOp;
        break;
    case AHC_VIDEO_PRERECORD_STATUS:
        m_sMovieConfig.PreRecordStatus = wOp;
        break;
    default:
        szErr = "command";

config_error:
        printc(FG_RED("Invalid config %s.") "stream:%d command:%d\r\n", szErr, i, wCommand);
        return MMP_FALSE;
        break;
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_CaptureClipCmd
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief This API configures both record and playback movie files

 This API configures both record and playback movie files

 Parameters:

 @param[in] wCommand capture command.
 @param[in] wOp      command value.
 @retval AHC_TRUE Success.
*/
AHC_BOOL AHC_VIDEO_CaptureClipCmd(UINT16 wCommand, UINT16 wOp)
{
    AHC_BOOL ahc_ret = AHC_TRUE;
    
    switch(wCommand) {

    case AHC_CAPTURE_CLIP_PAUSE:
        MMPS_3GPRECD_PauseRecord();
        break;
    case AHC_CAPTURE_CLIP_RESUME:
        MMPS_3GPRECD_ResumeRecord();
        break;
    case AHC_CAPTURE_SNAPSHOT:
    {
        MMP_ERR     err = MMP_ERR_NONE;
        MMP_BYTE    bJpgFileName[MAX_FILE_NAME_SIZE];
        MMP_BYTE    DirName[16];
        UINT16      CaptureDirKey;
        UINT8       CaptureFileName[32];
        UINT8       bCreateNewDir;
        UINT32      ThumbWidth, ThumbHeight;
        AHC_BOOL    bRet = AHC_TRUE;
        UINT32      movieSize;
        AHC_BOOL    bAddRearCamtoDB = AHC_FALSE;
        
        VIDENC_FW_STATUS sMergerStatus = VIDENC_FW_STATUS_NONE;
        MMP_ERR     sRet = MMP_ERR_NONE;

        MMPS_3GPRECD_STILL_CAPTURE_INFO VideoCaptureInfo = {
            NULL,       // bFileName
            0,          // ulFileNameLen
            1920,       // Jpeg width
            1080,       // Jpeg height
            160,        // Thumbnail width
            120,        // Thumbnail height
            MMP_TRUE,   // Thumbnail Enable
            MMP_TRUE,   // EXIF Enable
            MMP_FALSE,  // bTargetCtl
            MMP_FALSE,  // bLimitCtl
            200,        // bTargetSize
            220,        // bLimitSize
            1,          // bMaxTrialCnt
            MMP_DSC_CAPTURE_NORMAL_QUALITY,
            MMP_3GPRECD_CAPTURE_SRC_FRONT_CAM,
            NULL,       // bRearFileName
            0           // ulRearFileNameLen
        };

        #if ( FS_FORMAT_FREE_ENABLE )
        MMP_UBYTE   byFFWriteEnable;
    	MMPS_FORMATFREE_GetWriteEnable(&byFFWriteEnable);
    	MMPS_FORMATFREE_EnableWrite(1);
        #endif
        
        // Due to encode thumbnail need use GRA engine which conflict with LDC/JPEG Queue usage.
        if (MMP_IsVidLdcSupport() && MMP_GetVidLdcRunMode() != LDC_RUN_MODE_DISABLE) {
            VideoCaptureInfo.bThumbEn = MMP_FALSE;
        }

        if (MMPF_JPEG_GetCtrlByQueueEnable()) {
            VideoCaptureInfo.bThumbEn = MMP_FALSE;
        }

        AHC_Menu_SettingGetCB((char *) COMMON_KEY_MOVIE_SIZE, &movieSize);

        switch(movieSize) {
        case MOVIE_SIZE_4K_15P:
            VideoCaptureInfo.usWidth  = 3840;
            VideoCaptureInfo.usHeight = 2160;
        break;
        case MOVIE_SIZE_4K_24P:
            VideoCaptureInfo.usWidth  = 3200;
            VideoCaptureInfo.usHeight = 1800;
        break;		
        case MOVIE_SIZE_2d7K_30P:
            VideoCaptureInfo.usWidth  = 2704;
            VideoCaptureInfo.usHeight = FLOOR8(1524); //Not 8x
        break;
        case MOVIE_SIZE_1440_30P:
            VideoCaptureInfo.usWidth  = 2560;
            VideoCaptureInfo.usHeight = 1440;
        break;
        case MOVIE_SIZE_SHD_30P:
            VideoCaptureInfo.usWidth  = 2304;
            VideoCaptureInfo.usHeight = 1296;
        break;
        case MOVIE_SIZE_1080_30P:
        case MOVIE_SIZE_1080_50P:
        case MOVIE_SIZE_1080_60P:
        case MOVIE_SIZE_1080_30P_HDR:
            VideoCaptureInfo.usWidth  = 1920;
            VideoCaptureInfo.usHeight = 1080;
        break;
        #if (MENU_MOVIE_SIZE_900P_30P_EN)
        case MOVIE_SIZE_900P_30P:
            VideoCaptureInfo.usWidth  = 1600;
            VideoCaptureInfo.usHeight = 912;
            break;
        #endif
        #if (MENU_MOVIE_SIZE_960P_30P_EN)
        case MOVIE_SIZE_960P_30P:
            VideoCaptureInfo.usWidth  = 1280;
            VideoCaptureInfo.usHeight = 960;
            break;
        #endif
        case MOVIE_SIZE_720_30P:
        case MOVIE_SIZE_720_60P:
        case MOVIE_SIZE_720_100P:
        case MOVIE_SIZE_720_120P:
            VideoCaptureInfo.usWidth  = 1280;
            VideoCaptureInfo.usHeight = 720;
        break;
        case MOVIE_SIZE_VGA_30P:
        case MOVIE_SIZE_VGA_120P:
            VideoCaptureInfo.usWidth  = 640;
            VideoCaptureInfo.usHeight = 480;
        break;
        default:
        case MOVIE_SIZE_360_30P:
            VideoCaptureInfo.usWidth  = 640;
            VideoCaptureInfo.usHeight = 360;
        break;
        }

		// for only sonix
        if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
        {
            VideoCaptureInfo.usWidth = 1280;
            VideoCaptureInfo.usHeight = 720;
        }

        if (CAM_CHECK_PRM(PRM_CAM_BAYER_SENSOR) &&
            CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) { // TBD
            
            if (MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
                VideoCaptureInfo.usWidth        = 3008;
                VideoCaptureInfo.usHeight       = 1504;
                VideoCaptureInfo.bTargetSize    = 500; // KB
                VideoCaptureInfo.Quality        = MMP_DSC_CAPTURE_HIGH_QUALITY;
            }
            if (MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_2PIPE) {
                VideoCaptureInfo.usWidth        = 3008;
                VideoCaptureInfo.usHeight       = 1504;
                VideoCaptureInfo.bTargetSize    = 500; // KB
                VideoCaptureInfo.Quality        = MMP_DSC_CAPTURE_HIGH_QUALITY;
            }
        }

        if (MMP_TRUE == VideoCaptureInfo.bExifEn) {
            
            AHC_ConfigEXIF_RTC();
            AHC_ConfigEXIF_SystemInfo();
            AHC_ConfigEXIF_MENU();
        }

        AHC_GetCaptureConfig(ACC_CUS_THUMB_W, &ThumbWidth);
        AHC_GetCaptureConfig(ACC_CUS_THUMB_H, &ThumbHeight);
        
        VideoCaptureInfo.usThumbWidth  = ThumbWidth;
        VideoCaptureInfo.usThumbHeight = ThumbHeight;
        
        DBG_AutoTestPrint(ATEST_ACT_SNAPSHOP_0x0002, ATEST_STS_START_0x0001, 0, 0, gbAhcDbgBrk);
        DBG_AutoTestPrint(ATEST_ACT_SNAPSHOP_0x0002, ATEST_STS_RSOL_SIZE_0x0004, VideoCaptureInfo.usWidth, VideoCaptureInfo.usHeight, gbAhcDbgBrk);

        MEMSET(bJpgFileName   , 0, sizeof(bJpgFileName));
        MEMSET(DirName        , 0, sizeof(DirName));
        MEMSET(CaptureFileName, 0, sizeof(CaptureFileName));

        AHC_UF_SetFreeChar(0, DCF_SET_FREECHAR, (UINT8*)SNAP_DEFAULT_FILE_FREECHAR);

    #if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_NORMAL)
        bRet = AHC_UF_GetName(&CaptureDirKey, (INT8*)DirName, (INT8*)CaptureFileName, &bCreateNewDir);
        
        if (bRet == AHC_TRUE) {
            printc("CaptureDirKey=%d  CaptureFileName=%s\r\n", CaptureDirKey, CaptureFileName);

            STRCPY(bJpgFileName, (MMP_BYTE*)AHC_UF_GetRootName());
            STRCAT(bJpgFileName, "\\");
            STRCAT(bJpgFileName, DirName);

            if (bCreateNewDir) {
                MMPS_FS_DirCreate((INT8*)bJpgFileName, STRLEN(bJpgFileName));
                AHC_UF_AddDir(DirName);
            }

            STRCAT(bJpgFileName, "\\");
            STRCAT(bJpgFileName, (INT8*)CaptureFileName);
            STRCAT(bJpgFileName, EXT_DOT);
            STRCAT(bJpgFileName, PHOTO_JPG_EXT);
            
            VideoCaptureInfo.bFileName = bJpgFileName;
            VideoCaptureInfo.ulFileNameLen = STRLEN(bJpgFileName);
            
            err = MMPS_3GPRECD_StillCapture(&VideoCaptureInfo);

            if (MMP_ERR_NONE == err)
            {
                STRCAT((INT8*)CaptureFileName, EXT_DOT);
                STRCAT((INT8*)CaptureFileName, PHOTO_JPG_EXT);

                AHC_UF_PreAddFile(CaptureDirKey, (INT8*)CaptureFileName);
                AHC_UF_PostAddFile(CaptureDirKey, (INT8*)CaptureFileName);
            }
            else {
                printc("-E- MMPS_3GPRECD_StillCapture return 0x%X\r\n", __func__);
            }
        }
        else {
            err = MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE;
        }

    #elif (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
        {
            AHC_RTC_TIME        RtcTime;
            AHC_BOOL            bRearFlag;			
            USB_DETECT_PHASE    USBCurrentStates = 0;
            MMP_BYTE            bRearCamJpgFileName[MAX_FILE_NAME_SIZE];
            UINT8               bRearCamCaptureFileName[32];
            
            AHC_RTC_GetTime(&RtcTime);

            AHC_UF_GetRearCamPathFlag(&bRearFlag);
            AHC_UF_SetRearCamPathFlag(AHC_FALSE);             
            bRet = AHC_UF_GetName2(&RtcTime, (INT8*)bJpgFileName, (INT8*)CaptureFileName, &bCreateNewDir);
            AHC_UF_SetRearCamPathFlag(bRearFlag);
            if (bRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,bRet); return AHC_FALSE;}             

            STRCAT(bJpgFileName, EXT_DOT);
            STRCAT(bJpgFileName, PHOTO_JPG_EXT);
            STRCAT((INT8*)CaptureFileName, EXT_DOT);
            STRCAT((INT8*)CaptureFileName, PHOTO_JPG_EXT);

            printc("bJpgFileName: %s\r\n",bJpgFileName);
            
            VideoCaptureInfo.sCaptSrc = 0;
            VideoCaptureInfo.bFileName = bJpgFileName;
            VideoCaptureInfo.ulFileNameLen = STRLEN(bJpgFileName);

            if (CAM_CHECK_PRM(PRM_CAM_NONE)) {
				if (CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
	                VideoCaptureInfo.sCaptSrc |= MMP_3GPRECD_CAPTURE_SRC_REAR_CAM;
			}
            else {
                VideoCaptureInfo.sCaptSrc |= MMP_3GPRECD_CAPTURE_SRC_FRONT_CAM;
			}

            #if ( FS_FORMAT_FREE_ENABLE )
            ahc_ret = AHC_FF_SetFileNameSlot( VideoCaptureInfo.bFileName , DCF_CAM_FRONT);
			if (ahc_ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahc_ret); /*return AHC_FALSE;*/}
            #endif
            
            if (MMP_IsUSBCamExist()) {
                
                AHC_USBGetStates(&USBCurrentStates, AHC_USB_GET_PHASE_CURRENT);
                
                if ((USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) && 
                    (MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_MJPEG_H264) &&  // Only "MJPEG" AIT Rear Cam supports capture
                    (AHC_TRUE == AHC_HostUVCVideoIsEnabled())) {
                    VideoCaptureInfo.sCaptSrc |= MMP_3GPRECD_CAPTURE_SRC_REAR_CAM;
                    bAddRearCamtoDB = AHC_TRUE;
                }
                else {
                    bAddRearCamtoDB = AHC_FALSE;
                }
            }
            else if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) {
                if (AHC_TRUE == AHC_SNR_GetTvDecSnrCnntStatus()) {
                    bAddRearCamtoDB = AHC_TRUE;
                    VideoCaptureInfo.sCaptSrc |= MMP_3GPRECD_CAPTURE_SRC_REAR_CAM;
                }
                else {
                    bAddRearCamtoDB = AHC_FALSE;
                }
            }
            else if (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
                if (1) {
                    bAddRearCamtoDB = AHC_TRUE;
                    VideoCaptureInfo.sCaptSrc |= MMP_3GPRECD_CAPTURE_SRC_REAR_CAM;
                }
                else {
                    bAddRearCamtoDB = AHC_FALSE;
                }
            }
    
            if (bAddRearCamtoDB) {
                AHC_UF_SetRearCamPathFlag(AHC_TRUE);
                
                ahc_ret = AHC_UF_GetName2(&RtcTime, (INT8*)bRearCamJpgFileName, (INT8*)bRearCamCaptureFileName, &bCreateNewDir);            
                if (ahc_ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahc_ret); /*return AHC_FALSE;*/} 
                
                STRCAT(bRearCamJpgFileName, EXT_DOT);
                STRCAT(bRearCamJpgFileName, PHOTO_JPG_EXT);
                STRCAT((INT8*)bRearCamCaptureFileName, EXT_DOT);
                STRCAT((INT8*)bRearCamCaptureFileName, PHOTO_JPG_EXT);

                VideoCaptureInfo.bRearFileName      = bRearCamJpgFileName;
                VideoCaptureInfo.ulRearFileNameLen  = STRLEN(bRearCamJpgFileName);

                if (!CAM_CHECK_PRM(PRM_CAM_NONE))
                {
                    printc("bRearCamJpgFileName: %s\r\n",bRearCamJpgFileName);
                    #if ( FS_FORMAT_FREE_ENABLE )
                   	ahc_ret = AHC_FF_SetFileNameSlot( VideoCaptureInfo.bRearFileName, DCF_CAM_REAR);
					if (ahc_ret != AHC_TRUE){ 
						AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahc_ret);
						bAddRearCamtoDB = AHC_FALSE;
                    	VideoCaptureInfo.sCaptSrc &= ~(MMP_3GPRECD_CAPTURE_SRC_REAR_CAM);
					} 
                    #endif

                    ahc_ret = AHC_UF_PreAddFile(CaptureDirKey, (INT8*)bRearCamCaptureFileName);
                    if (ahc_ret == AHC_FALSE) {
                        RTNA_DBG_Str0(FG_RED("AHC_VIDEO_CaptureClipCmd Error!\r\n"));
                    }
                }
            }

            AHC_UF_SetRearCamPathFlag(AHC_FALSE);

            ahc_ret = AHC_UF_PreAddFile(CaptureDirKey, (INT8*)CaptureFileName);
            if (ahc_ret != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahc_ret); /*return AHC_FALSE;*/} 

            #if (AHC_ENABLE_VIDEO_STICKER)
            sRet = MMPS_3GPRECD_GetRecordStatus(&sMergerStatus);
            if ((sRet != MMP_ERR_NONE) && 
               ((sMergerStatus == VIDENC_FW_STATUS_NONE) || (sMergerStatus == VIDENC_FW_STATUS_PAUSE) || (sMergerStatus == VIDENC_FW_STATUS_STOP))) {
                UpdateVideoCurrentTimeStamp();
            }
            #endif

            if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) { // CHECK
                MMPF_Display_StopFlushWin(gsAhcScdSensor, MMP_IBC_PIPE_2, MMP_TRUE);
            }
            
            err = MMPS_3GPRECD_StillCapture(&VideoCaptureInfo);

            if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) {
                MMPF_Display_StopFlushWin(gsAhcScdSensor, MMP_IBC_PIPE_2, MMP_FALSE);
            }
            
            if (MMP_ERR_NONE == err)
            {
                AHC_UF_SetRearCamPathFlag(AHC_FALSE);
                
                ahc_ret = AHC_UF_PostAddFile(CaptureDirKey,(INT8*)CaptureFileName);
                if (ahc_ret != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahc_ret); /*return AHC_FALSE;*/} 

                // Post add rear cam file
                if ((bAddRearCamtoDB) && !CAM_CHECK_PRM(PRM_CAM_NONE)) {
                    ahc_ret = AHC_UF_PostAddFile(CaptureDirKey,(INT8*)bRearCamCaptureFileName);
                    if (ahc_ret != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk, ahc_ret); /*return AHC_FALSE;*/} 
                }
            }
            else {
                printc("-E- MMPS_3GPRECD_StillCapture %s line %d return 0x%X\r\n", __func__, __LINE__, err);
            }
        }
    #endif

        AHC_UF_SetFreeChar(0, DCF_SET_FREECHAR, (UINT8*)VIDEO_DEFAULT_FLIE_FREECHAR);

        DBG_AutoTestPrint(ATEST_ACT_SNAPSHOP_0x0002, ATEST_STS_END_0x0003, (err & 0xFFFF0000)>>16, (err & 0xFFFF), gbAhcDbgBrk);        

    	#if(FS_FORMAT_FREE_ENABLE)
    	// Set original setting back
    	MMPS_FORMATFREE_EnableWrite(byFFWriteEnable);
    	#endif
    }
        break;
    default:
        break;
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetPreRecordStatus
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetPreRecordStatus(UINT32 *ulStatus)
{
    AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_PRERECORD_STATUS, ulStatus);
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetVideoBitrate
//  Description : 
//------------------------------------------------------------------------------
UINT32 AHC_VIDEO_GetVideoBitrate(int type)
{
	UINT32 ulResIdx = 0;
	UINT32 ulFps = 30;

	switch (type) {
	#if(MENU_MOVIE_SIZE_4K_24P_EN)
    case MOVIE_SIZE_4K_24P:
    	ulResIdx	= VIDRECD_RESOL_3200x1808;
    	ulFps		= 24;
    	break;
	#endif
	#if(MENU_MOVIE_SIZE_1440_30P_EN)
    case MOVIE_SIZE_1440_30P:
    	ulResIdx	= VIDRECD_RESOL_2560x1440;
    	ulFps		= 30;
    	break;
	#endif
    #if(MENU_MOVIE_SIZE_SHD_30P_EN)
    case MOVIE_SIZE_SHD_30P:
    	ulResIdx	= VIDRECD_RESOL_2304x1296;
    	ulFps		= 30;
    	break;
	#endif
	#if(MENU_MOVIE_SIZE_SHD_25P_EN)
    case MOVIE_SIZE_SHD_25P:
    	ulResIdx	= VIDRECD_RESOL_2304x1296;
    	ulFps		= 25;
    	break;
	#endif
    #if(MENU_MOVIE_SIZE_SHD_24P_EN)
    case MOVIE_SIZE_SHD_24P:
    	ulResIdx	= VIDRECD_RESOL_2304x1296;
    	ulFps		= 24;
    	break;
	#endif
    #if (MENU_MOVIE_SIZE_1080_60P_EN)
    case MOVIE_SIZE_1080_60P:
    	ulResIdx 	= VIDRECD_RESOL_1920x1088;
    	ulFps 		= 60;
    	break;
	#endif
    #if (MENU_MOVIE_SIZE_1080_24P_EN)
    case MOVIE_SIZE_1080_24P:
        ulResIdx    = VIDRECD_RESOL_1920x1088;
        ulFps       = 24;
        break;
    #endif
	#if (MENU_MOVIE_SIZE_1080P_EN)
    case MOVIE_SIZE_1080P:
    	ulResIdx 	= VIDRECD_RESOL_1920x1088;
    	ulFps 		= 30;
    	break;
    #endif
    #if (MENU_MOVIE_SIZE_900P_30P_EN)
    case MOVIE_SIZE_900P_30P:
        ulResIdx 	= VIDRECD_RESOL_1600x900;
        ulFps 		= 30;
        break;
    #endif 
    #if (MENU_MOVIE_SIZE_960P_30P_EN)
    case MOVIE_SIZE_960P_30P:
        ulResIdx 	= VIDRECD_RESOL_1280x960;
        ulFps 		= 30;
        break;
    #endif    
    #if (MENU_MOVIE_SIZE_720_60P_EN)
    case MOVIE_SIZE_720_60P:
		ulResIdx 	= VIDRECD_RESOL_1280x720;
    	ulFps 		= 60;
		break;
	#endif
    #if (MENU_MOVIE_SIZE_720_24P_EN)
    case MOVIE_SIZE_720_24P:
        ulResIdx    = VIDRECD_RESOL_1280x720;
        ulFps       = 24;
        break;
    #endif
	#if(MENU_MOVIE_SIZE_720P_EN)
	case MOVIE_SIZE_720P:
		ulResIdx 	= VIDRECD_RESOL_1280x720;
    	ulFps 		= 30;
		break;
	#endif
	#if(MENU_MOVIE_SIZE_VGA30P_EN)
	case MOVIE_SIZE_VGA_30P:
		ulResIdx 	= VIDRECD_RESOL_640x480;
    	ulFps 		= 30;
		break;
	#endif
    case MOVIE_SIZE_360_30P:
        ulResIdx 	= VIDRECD_RESOL_640x368;
     	ulFps 		= 30;
        break;
	default:
    	ulResIdx 	= VIDRECD_RESOL_1920x1088;
     	ulFps 		= 30;
		printc("ERROR: movie size\r\n");
		break;
	}
	
	if (ulFps == 60) {
		return MMPS_3GPRECD_GetConfig()->ulFps60BitrateMap[ulResIdx][MenuSettingConfig()->uiMOVQuality];
	}
	else {
		return MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[ulResIdx][MenuSettingConfig()->uiMOVQuality];
	}
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetVideoRealFpsX1000
//  Description : 
//------------------------------------------------------------------------------
UINT32 AHC_VIDEO_GetVideoRealFpsX1000(UINT32 FrateIdx)
{
    UINT32 RealFrameRate;

    switch(FrateIdx)
    {
	case VIDRECD_FRAMERATE_7d5FPS:
        RealFrameRate = 7500;
        break;
    case VIDRECD_FRAMERATE_10FPS:
        RealFrameRate = 10000;
        break;
	case VIDRECD_FRAMERATE_12FPS:
        RealFrameRate = 12000;
        break;
    case VIDRECD_FRAMERATE_15FPS:
        RealFrameRate = 15000;
        break;
	case VIDRECD_FRAMERATE_20FPS:
        RealFrameRate = 20000;
        break;
	case VIDRECD_FRAMERATE_24FPS:
        RealFrameRate = 24000;
        break;
    case VIDRECD_FRAMERATE_25FPS:
        RealFrameRate = 25000;
        break;
	case VIDRECD_FRAMERATE_30FPS:
        RealFrameRate = 30000;
        break;
	case VIDRECD_FRAMERATE_50FPS:
        RealFrameRate = 50000;
        break;
    case VIDRECD_FRAMERATE_60FPS:
        RealFrameRate = 60000;
        break;
    case VIDRECD_FRAMERATE_100FPS:
        RealFrameRate = 100000;
        break;
	case VIDRECD_FRAMERATE_120FPS:
        RealFrameRate = 120000;
        break;
    default:
        RealFrameRate = 30000;//TBD
        break;
    }

    return RealFrameRate;
}

#if 0
void _____VR_Callback_Function_________(){ruturn;} //dummy
#endif

#define VR_WRITE_FUNC_CACHE  (1)

#if (VR_WRITE_FUNC_CACHE == 1)
void VR_WriteFunc_Ex(UINT8* pData, UINT32 ulNumBytes, UINT32 *pulWrittenBytes, UINT32 ulFileID, AHC_BOOL bLastData)
{
    #define VR_WRITE_BUFFER_LENGTH (64*1024)  //64KB
    
    static UINT8 pbyDataBuffer[VR_WRITE_BUFFER_LENGTH]; //Allocate 64KB as temp buffer
    static UINT32 ulDataNumInBuffer = 0;
    UINT32 ulWritteneSize = 0;

    *pulWrittenBytes = 0;
    
    if (bLastData == AHC_FALSE)
    {
        if (ulNumBytes > VR_WRITE_BUFFER_LENGTH) //Write all data to media
        {
            VR_WriteFunc((void *)pbyDataBuffer, ulDataNumInBuffer, &ulWritteneSize, ulFileID);
            *pulWrittenBytes = ulWritteneSize;
            ulDataNumInBuffer = 0;
            VR_WriteFunc(pData, ulNumBytes, &ulWritteneSize, ulFileID);
            *pulWrittenBytes += ulWritteneSize;
        }
        else if ((ulDataNumInBuffer + ulNumBytes) >= VR_WRITE_BUFFER_LENGTH) //Write 64KB data to media
        {
            UINT32 ulRemainingSegment = (ulDataNumInBuffer + ulNumBytes) - VR_WRITE_BUFFER_LENGTH;
            UINT32 ulWriteSegment = ulNumBytes - ulRemainingSegment;
            
            memcpy( (UINT8*)(pbyDataBuffer+ulDataNumInBuffer), pData, ulWriteSegment );
            VR_WriteFunc((void *)pbyDataBuffer, VR_WRITE_BUFFER_LENGTH, &ulWritteneSize, ulFileID);
            *pulWrittenBytes = ulWritteneSize;
            memcpy( (UINT8*)pbyDataBuffer, (UINT8*)(pData+ulWriteSegment), ulRemainingSegment );
            ulDataNumInBuffer = ulRemainingSegment;
        }
        else  //Keep all data in buffer
        {
            memcpy( (UINT8*)(pbyDataBuffer+ulDataNumInBuffer), pData, ulNumBytes );
            ulDataNumInBuffer += ulNumBytes;
            *pulWrittenBytes = 0;
        }
    }
    else //Write all data to media
    {
        if ((ulDataNumInBuffer + ulNumBytes) > VR_WRITE_BUFFER_LENGTH)
        {
            VR_WriteFunc((void *)pbyDataBuffer, ulDataNumInBuffer, &ulWritteneSize, ulFileID);
            *pulWrittenBytes = ulWritteneSize;
            ulDataNumInBuffer = 0;
            VR_WriteFunc(pData, ulNumBytes, &ulWritteneSize, ulFileID);
            *pulWrittenBytes += ulWritteneSize;
        }
        else
        {
            if (pData != NULL && ulNumBytes != 0)
            {
                memcpy( (UINT8*)(pbyDataBuffer+ulDataNumInBuffer), pData, ulNumBytes );
                ulDataNumInBuffer += ulNumBytes;
            }
            VR_WriteFunc((void *)pbyDataBuffer, ulDataNumInBuffer, &ulWritteneSize, ulFileID);
            *pulWrittenBytes = ulWritteneSize;
            ulDataNumInBuffer = 0;
        }
    }
}
#else
void VR_WriteFunc_Ex(UINT8* pData, UINT32 ulNumBytes, UINT32 *pulWrittenBytes, UINT32 ulFileID, AHC_BOOL bLastData)
{
    VR_WriteFunc(pData, ulNumBytes, pulWrittenBytes, ulFileID);
}
#endif

//------------------------------------------------------------------------------
//  Function    : SNRTvSrcTypeCB
//  Description : This Callback function will be called once TV source is changed or removed.
//  InParam     : byTvSrcType - TV type: 0 - None, 1 - PAL, 2 - NTSC
//                byFalg - Reserved
//------------------------------------------------------------------------------
void SNRTvSrcTypeCB(MMP_UBYTE byTvSrcType, MMP_UBYTE byFalg)
{
    if (!CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) {
        return;
    }
    
    printc(FG_GREEN("<SNRTvSrcTypeCB> %d\r\n"), byTvSrcType);

    m_byTvSrcType = byTvSrcType;

    if (byTvSrcType == MMP_SNR_TVDEC_SRC_NO_READY)
        MMPS_Sensor_SetPreviewDelayCount(gsAhcScdSensor, DISP_DISABLE_PREVIEW_DELAY_COUNT);
    else
        MMPS_Sensor_SetPreviewDelayCount(gsAhcScdSensor, DISP_PREVIEW_DELAY_COUNT_SCD_SENSOR);

    AHC_SetCurKeyEventID(BUTTON_TV_DECODER_SRC_TYPE);
    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_TV_DECODER_SRC_TYPE, 0);
}

//------------------------------------------------------------------------------
//  Function    : VRCardSlowStopEncCB
//  Description :
//------------------------------------------------------------------------------
void VRCardSlowStopEncCB(void)
{
    printc(FG_RED("<VRCardSlowStopEncCB>\r\n"));

    AHC_VIDEO_SetRecCardSlowStop(AHC_TRUE);

    #if (GPS_CONNECT_ENABLE)
    if (AHC_GPS_Module_Attached()) {
        UINT8 bGPS_en;

        if ((AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) && 
            (bGPS_en != COMMON_GPS_REC_INFO_OFF)) {
            MMPF_OS_SetFlags(UartCtrlFlag, GPS_FLAG_SWITCHBUFFER_CLOSE, MMPF_OS_FLAG_SET);
        }
    }
    #endif
}

#if 0
//------------------------------------------------------------------------------
//  Function    : VRAPStopRecordCB
//  Description :
//------------------------------------------------------------------------------
void VRAPStopRecordCB(ULONG ulLastRecdVidTime, ULONG ulLastRecdAudTime)
{
    printc("VRAPStopRecordCB  ulLastRecdVidTime=%d  ulLastRecdAudTime=%d\n", ulLastRecdVidTime, ulLastRecdAudTime);

    AHC_VIDEO_SetAPStopRecord(AHC_TRUE);
    AHC_VIDEO_SetAPStopRecordTime(ulLastRecdVidTime, ulLastRecdAudTime);

    #if (GPS_CONNECT_ENABLE)
    if (AHC_GPS_Module_Attached())
    {
        UINT8 bGPS_en;

        if ((AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) && 
            (bGPS_en != COMMON_GPS_REC_INFO_OFF)) {
            MMPF_OS_SetFlags(UartCtrlFlag, GPS_FLAG_SWITCHBUFFER_CLOSE, MMPF_OS_FLAG_SET);
        }
    }
    #endif

    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_AP_STOP_VIDEO_RECD, 0);
}
#endif

//------------------------------------------------------------------------------
//  Function    : VRMediaFullCB
//  Description :
//------------------------------------------------------------------------------
void VRMediaFullCB(void)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    printc(FG_GREEN("<VRMediaFullCB>\r\n"));

    ahcRet = AHC_KeyEventIDCheckConflict(BUTTON_VRCB_MEDIA_FULL);
    if (ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return;}      
    
    AHC_SetCurKeyEventID(BUTTON_VRCB_MEDIA_FULL);
    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_MEDIA_FULL, 0);
}

//------------------------------------------------------------------------------
//  Function    : VRFileFullCB
//  Description :
//------------------------------------------------------------------------------
void VRFileFullCB(void)
{
    AHC_BOOL            ahcRet = AHC_TRUE;   
    VIDENC_FW_STATUS    sMergerStatus = VIDENC_FW_STATUS_NONE;
    MMP_ERR             sRet = MMP_ERR_NONE;
    MMP_BOOL            bDevConSts = MMP_FALSE;
    extern MMP_BOOL     gbUVCRecdStart;

    printc(FG_GREEN("<VRFileFullCB>\r\n"));
    MMPS_USB_GetDevConnSts(&bDevConSts);

    #if (SUPPORT_USB_HOST_FUNC)
    if ((MMP_IsUSBCamExist() && !MMP_IsUSBCamIsoMode()) && (!bDevConSts) && (!gbUVCRecdStart))
    {
        printc(FG_RED("%s, UVC cable drop\r\n"), __func__);
    }
    else
    #endif
    {
	    ahcRet = AHC_KeyEventIDCheckConflict(BUTTON_VRCB_FILE_FULL);
	    if (ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(0,ahcRet); return;}      
    }
    
    sRet = MMPS_3GPRECD_GetRecordStatus(&sMergerStatus);
    if ((sRet != MMP_ERR_NONE) || (sMergerStatus != VIDENC_FW_STATUS_PREENCODE)) {
        printc(FG_RED("sMergerStatus:%d\r\n"), sMergerStatus);        
        AHC_PRINT_RET_ERROR(0, sRet);
        return;
    }

    AHC_SetCurKeyEventID(BUTTON_VRCB_FILE_FULL);
    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_FILE_FULL, 0);
}

//------------------------------------------------------------------------------
//  Function    : VRLongTimeFileFullCB
//  Description :
//------------------------------------------------------------------------------
void VRLongTimeFileFullCB(void)
{
    printc(FG_GREEN("<VRLongTimeFileFullCB>\r\n"));

    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_LONG_TIME_FILE_FULL, 0);
}

//------------------------------------------------------------------------------
//  Function    : VRMediaSlowCB
//  Description :
//------------------------------------------------------------------------------
void VRMediaSlowCB(void)
{
    printc(FG_RED("<VRMediaSlowCB>\r\n"));
    
    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_MEDIA_SLOW, 0);
}

#if (VR_SLOW_CARD_DETECT)
//------------------------------------------------------------------------------
//  Function    : VRPreGetTimeWhenCardSlowCB
//  Description :
//------------------------------------------------------------------------------
void VRPreGetTimeWhenCardSlowCB(AHC_BOOL bNorRecdEnable, UINT32 ulNorRecdTime, AHC_BOOL bEmergRecdEnable, UINT32 ulEmergRecdTime)
{
    printc(FG_RED("<VRPreGetTimeWhenCardSlowCB>\r\n"));
    
    printc("bNorRecdEnable = %d ulNorRecdTime = %d\r\n", bNorRecdEnable, ulNorRecdTime);
    printc("bEmergRecdEnable = %d ulEmergRecdTime = %d\r\n", bEmergRecdEnable, ulEmergRecdTime);

    m_bCardSlowDetected = AHC_TRUE;

#ifdef SLOW_CARD_RESTART
    AHC_WMSG_Draw(AHC_TRUE, WMSG_CARD_SLOW, 3);
    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_MEDIA_SLOW, 0);
#endif	
}

//------------------------------------------------------------------------------
//  Function    : VRRecdStopWhenCardSlowCB
//  Description :
//------------------------------------------------------------------------------
void VRRecdStopWhenCardSlowCB(void)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    
    printc(FG_GREEN("<VRRecdStopWhenCardSlowCB>\r\n"));

    m_bCardSlowDetected = AHC_FALSE;
    
    AHC_VIDEO_SetAPStopRecord(AHC_FALSE);
    
    if ((MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE)  ||
        (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE) )
    {
        AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_AHC_DONE);
    }

    ahcRet = AHC_KeyEventIDCheckConflict(BUTTON_VRCB_RECDSTOP_CARDSLOW);
    if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return; }    
    
    AHC_SetCurKeyEventID(BUTTON_VRCB_RECDSTOP_CARDSLOW);
    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_RECDSTOP_CARDSLOW, 0);
}
#endif

//------------------------------------------------------------------------------
//  Function    : VRMediaErrorCB
//  Description :
//------------------------------------------------------------------------------
void VRMediaErrorCB(AHC_ERR VideoStatus, AHC_ERR VideoFileErr)
{
    printc(FG_RED("<VRMediaErrorCB>\r\n"));
    printc("VideoStatus  : %x \r\n", VideoStatus);
    printc("VideoFileErr : %x \r\n", VideoFileErr);
    
    AHC_WMSG_Draw(AHC_TRUE, WMSG_INSERT_SD_AGAIN, 2);
    
    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_MEDIA_ERROR, 0);
}

//------------------------------------------------------------------------------
//  Function    : VRStartEncCB
//  Description :
//------------------------------------------------------------------------------
void VRStartEncCB(void)
{
    printc(FG_GREEN("<VRStartEncCB>\r\n"));

    AHC_VIDEO_RecordStartWriteInfo();
}

//------------------------------------------------------------------------------
//  Function    : VRStopEncCB
//  Description :
//------------------------------------------------------------------------------
static void VRStopEncCB(void)
{
    #if (AHC_SHAREENC_SUPPORT)
    if ((AHC_VIDEO_IsShareRecStarted() == AHC_TRUE) && (AHC_VIDEO_IsSharePostDone() == AHC_FALSE))
    {
        RTNA_DBG_Str(0, "<VRCB_SHARE_DONE>\r\n");
        m_bShareVRPostDone = AHC_TRUE;
        AHC_VIDEO_SetShareRecStarted(AHC_FALSE);
    }
    #endif

#if (GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO)
    if (AHC_GPS_Module_Attached()) 
    {
        UINT8 bGPS_en;
        
        if ((AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) && 
            (bGPS_en != COMMON_GPS_REC_INFO_OFF)) {

            if (AHC_GPS_NeedToFlushBackupBuffer() == AHC_TRUE) {
                GPSCtrl_EndRecord_Backup();
            }
            #if (AHC_SHAREENC_SUPPORT)
            else if (m_bStartShareRec)
            {
                GPSCtrl_EndRecordEx();
            }            
            #endif            
            else 
            {
                GPSCtrl_EndRecord();
            }
        }
    }
#endif

#if (GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)
    if (AHC_Gsensor_Module_Attached())
    {
        if (AHC_Gsensor_NeedToFlushBackupBuffer() == AHC_TRUE) {
            AHC_Gsensor_EndRecord_Backup();
        }
        #if (AHC_SHAREENC_SUPPORT)
        else if (m_bStartShareRec)
        {
            AHC_Gsensor_EndRecordEx();
        }            
        #endif          
        else 
        {
            AHC_Gsensor_EndRecord();
        }
    }
#endif

#if (VR_VIDEO_TYPE == COMMON_VR_VIDEO_TYPE_3GP)
    #if (GPS_CONNECT_ENABLE)
    if (AHC_GPS_Module_Attached())
    {
        UINT8 bGPS_en;
        
        if ((AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) && 
            (bGPS_en != COMMON_GPS_REC_INFO_OFF)) {
            MMPF_OS_SetFlags(UartCtrlFlag, GPS_FLAG_SWITCHBUFFER, MMPF_OS_FLAG_SET);
        }
    }
    #endif
#endif
}

//------------------------------------------------------------------------------
//  Function    : VRPostProcessCB
//  Description :
//------------------------------------------------------------------------------
static UINT32 VRPostProcessCB(MMP_ERR (*MuxWrite)(void *buf, MMP_ULONG size, MMP_ULONG *wr_size, MMP_ULONG ul_FileID),MMP_ULONG ulbackFileID)
{
    extern UINT32 m_ulEventPreRecordTime;
    extern UINT32 m_ulEventHappenTime;
    extern AHC_BOOL m_bCurrentTimeLessThanPreRecord;

    printc(FG_GREEN("<VRPostProcessCB>\r\n"));

    VR_WriteFunc = MuxWrite;

    // Add below code ( #if ~ #endif ) since VRStopEncCB() did not be called after Emergency recording finished
    // Remove it after VRStopEncCB() is called from driver
    #if (GPS_RAW_FILE_ENABLE == 0 && GSENSOR_RAW_FILE_ENABLE == 0)
    if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE)
    {
        #if (GPS_CONNECT_ENABLE && GSENSOR_CONNECT_ENABLE)
        if ((AHC_VIDEO_GetKernalEmergStopStep() == AHC_KERNAL_EMERGENCY_RECORD) &&
           (AHC_GPS_NeedToFlushBackupBuffer() == AHC_FALSE && 
            AHC_Gsensor_NeedToFlushBackupBuffer() == AHC_FALSE))
        {
            VRStopEncCB();
        }
        #endif
    }
    #endif

    // Blocking action for 3gp merger task.
    AHC_VIDEO_RecordPostWriteInfo(ulbackFileID);

#if (GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO) && (GPS_USE_FILE_AS_DATABUF)
    GPSCtrl_CloseInfoFile();

    if ((MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE) ||
        (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE) )
    {
        if (AHC_GPS_Module_Attached() == AHC_TRUE && AHC_VIDEO_GetKernalEmergStopStep() == AHC_KERNAL_EMERGENCY_RECORD) {
            AHC_GPS_TriggerRestorePreRecordInfo(AHC_TRUE, (m_ulEventPreRecordTime + (OSTimeGet() - m_ulEventHappenTime))*1000/ OS_TICKS_PER_SEC, m_bCurrentTimeLessThanPreRecord );
        }
    }
#endif

#if (GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO) && (GSNR_USE_FILE_AS_DATABUF)
    AHC_Gsensor_CloseInfoFile();
    
    if ((MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE)  ||
        (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE) )
    {
        if (AHC_Gsensor_Module_Attached() == AHC_TRUE && AHC_VIDEO_GetKernalEmergStopStep() == AHC_KERNAL_EMERGENCY_RECORD) {
            AHC_Gsensor_TriggerRestorePreRecordInfo(AHC_TRUE, (m_ulEventPreRecordTime + (OSTimeGet() - m_ulEventHappenTime))*1000/ OS_TICKS_PER_SEC, m_bCurrentTimeLessThanPreRecord );
        }
    }
#endif
    
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : VRSeamlessCB
//  Description :
//------------------------------------------------------------------------------
void VRSeamlessCB(void)
{
    printc(FG_GREEN("<VRSeamlessCB>\r\n"));
	
    AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_SEAM_LESS, 0);
}

//------------------------------------------------------------------------------
//  Function    : VREmerDoneCB
//  Description :
//------------------------------------------------------------------------------
void VREmerDoneCB(void)
{
    printc(FG_GREEN("<VREmerDoneCB>\n"));

    #if (VR_SLOW_CARD_DETECT)
    if (m_bCardSlowDetected) {
        return;
    }    
    #endif

    #if (EMER_RECORD_DUAL_WRITE_ENABLE == 1)
    
    if ((MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE) ||
        (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE) )
    {
        AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_STOP);
    }

    //for UVC cam
    #if (UVC_VIDRECD_SUPPORT && SUPPORT_USB_HOST_FUNC)
    if (MMP_IsUSBCamExist() && !MMP_IsUSBCamIsoMode())
    {
        #if (VIDRECD_MULTI_TRACK == 0) && (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) 
        MMP_BOOL bDevConSts = MMP_FALSE;
        MMPS_USB_GetDevConnSts(&bDevConSts);    

        if ((m_EmrVRUSBHFullName[0] != 0 && bDevConSts)         || 
            ((m_EmrVRUSBHFullName[0] == 0))                     )
        {
            AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_EMER_DONE, 0);
        }
        #else
        AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_EMER_DONE, 0);
        #endif
    }
    else
    #endif
    {
        #if (DUAL_EMERGRECD_SUPPORT == 1 && VIDRECD_MULTI_TRACK == 1)
        MMP_SNR_TVDEC_SRC_TYPE sSnrTVSrc = MMP_SNR_TVDEC_SRC_PAL;

        if (MMP_GetScdCamType() == SCD_CAM_TV_DECODER) {
            MMPS_Sensor_GetTVDecSrcType(&sSnrTVSrc);
        }
        
        if ((sSnrTVSrc == MMP_SNR_TVDEC_SRC_NO_READY) && (MMPS_3GPRECD_IsDualEmergentRecdEnable())) {
            // If MMP_SNR_TVDEC_SRC_NO_READY, SNRTvSrcTypeCB() will be called to stop video recording.
            // Skip BUTTON_VRCB_EMER_DONE to avoid double stop video recording which will let system stuck.
            printc(" #TV DEC drop\r\n");
        }
        else
        #endif
        AHC_SendAHLMessage_HP(AHLM_UI_NOTIFICATION, BUTTON_VRCB_EMER_DONE, 0);
    }
        
    #endif
}

//------------------------------------------------------------------------------
//  Function    : VRDualEncFileFullCB
//  Description :
//------------------------------------------------------------------------------
void VRDualEncFileFullCB(void)
{
#if (AHC_SHAREENC_SUPPORT)
    extern MMP_ULONG m_DualWriteBytes;
    //extern MMP_ULONG m_Dual2WriteBytes;

    DCF_DB_TYPE sDB = AHC_UF_GetDB();
    
    RTNA_DBG_Str0("## Dual-Enc File Full CB ##\r\n");
    printc("Dual fs = %ld bytes\r\n", m_DualWriteBytes);

    AHC_UF_SelectDB(DCF_DB_FORMAT_FREE_DB);
    AHC_UF_SizeinFile_Update((INT8*)m_ShareRecFileName);
    AHC_UF_PostAddFile(0, (INT8*)m_ShareRecFileName);
    AHC_UF_SelectDB(sDB);

    m_bFirstShareFile = AHC_FALSE;
    m_bStartShareRec = AHC_FALSE;
    m_bShareRecPostDone = AHC_TRUE;

    #if (0)//defined(WIFI_PORT) && (WIFI_PORT == 1)    
    ncgi_notify_fn(NCGI_SHORT_FN, (char*)m_ShareRecFullFileName);
    #endif
#else
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : VRDualEncStartCB
//  Description :
//------------------------------------------------------------------------------
void VRDualEncStartCB(void)
{
    printc(FG_GREEN("<VRDualEncStartCB>\n"));
}

//------------------------------------------------------------------------------
//  Function    : VRDualEncStopCB
//  Description :
//------------------------------------------------------------------------------
void VRDualEncStopCB(void)
{
    printc(FG_GREEN("<VRDualEncStopCB>\n"));
}

//------------------------------------------------------------------------------
//  Function    : VRStreamingCB
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL VRStreamingCB(MMPS_3GPRECD_STREAMCB_TYPE cbmode, MMP_ULONG framesize, MMPS_3GPRECD_STREAM_INFO* moveinfo)
{
    printc(FG_GREEN("<VRStreamingCB>\n"));
    return MMP_TRUE;
}

#if 0
void _____VR_Zoom_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetCurZoomStep
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetCurZoomStep(UINT16 *usZoomStepNum)
{
    MMP_IBC_PIPEID ubPipe;

    MMPS_3GPRECD_GetPreviewPipe(gsAhcPrmSensor, &ubPipe);

    if (!MMPS_3GPRECD_GetCurZoomStep(ubPipe, usZoomStepNum))
	    return AHC_FALSE;
	else
	    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetMaxZoomStep
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetMaxZoomStep(UINT16 *usMaxZoomStep)
{
    *usMaxZoomStep = gsVidPtzCfg.usMaxZoomSteps;

	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetMaxZoomRatio
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetMaxZoomRatio(UINT16 *usMaxZoomRatio)
{
     *usMaxZoomRatio = gsVidPtzCfg.usMaxZoomRatio;

     return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetCurZoomStatus
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE AHC_VIDEO_GetCurZoomStatus(void)
{
	return MMPS_3GPRECD_GetCurZoomStatus();
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetDigitalZoomRatio
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetDigitalZoomRatio(UINT32 *usZoomRatio)
{
    UINT16 usMaxZoomRatio, usCurrentStep, usMaxZoomStep;

    AHC_VIDEO_GetMaxZoomRatio(&usMaxZoomRatio);

    AHC_VIDEO_GetCurZoomStep(&usCurrentStep);
    AHC_VIDEO_GetMaxZoomStep(&usMaxZoomStep);

    *usZoomRatio = ((usMaxZoomRatio > 1) && (usMaxZoomStep > 0)) ? (100 + (usCurrentStep * (usMaxZoomRatio - 1) * 100) / usMaxZoomStep) : 100;
    
    return AHC_TRUE;
}

#if 0
void _____VR_Common_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetPrevRecFullName
//  Description :
//------------------------------------------------------------------------------
UINT8* AHC_VIDEO_GetPrevRecFullName(void)
{
    return (UINT8*)m_PrevVRFullName;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetCurRecFileName
//  Description :
//------------------------------------------------------------------------------
UINT8* AHC_VIDEO_GetCurRecFileName(AHC_BOOL bFullName)
{
    if (bFullName) {
        return (UINT8*)m_CurVRFullName;
	}
    else {
        return gpbCurVideoFileName;
    }
}

#if (VIDRECD_MULTI_TRACK == 0)
//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetCurRecRearFileName
//  Description :
//------------------------------------------------------------------------------
UINT8* AHC_VIDEO_GetCurRecRearFileName(AHC_BOOL bFullName)
{
    if (bFullName) {
        return (UINT8*)m_CurVRRearFullName;
	}
    else {
        return gpbCurVideoRearFileName;
    }    
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetCurRecUSBHFileName
//  Description :
//------------------------------------------------------------------------------
UINT8* AHC_VIDEO_GetCurRecUSBHFileName(AHC_BOOL bFullName)
{
    if (bFullName) {
        return (UINT8*)m_CurVRUSBHFullName;
	}
    else {
        return gpbCurVideoUSBHFileName;
    }    
}

#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetCurRecDirKey
//  Description :
//------------------------------------------------------------------------------
UINT16 AHC_VIDEO_GetCurRecDirKey(void)
{
    return gusCurVideoDirKey;
}

//------------------------------------------------------------------------------
//  Function    : AHC_FF_SetFileNameSlot
//  Description : Search a hidden / oldest filename slot for next recorded file
//------------------------------------------------------------------------------
/**
 * @brief   		Search a hidden / oldest filename slot for next recorded file
 *                  Note: This API only uses for Format Free.
 * @param[in] 		byFilename    New filename for next recorded file
 * @retval 			AHC_TRUE      Success.
 */
#if ( FS_FORMAT_FREE_ENABLE )
AHC_BOOL AHC_FF_SetFileNameSlot( MMP_BYTE *byFilename , DCF_CAM_ID byCamID )
{
    UINT16              uwYear,uwMonth,uwDay,uwDayInWeek,uwHour,uwMinute,uwSecond;
    UINT8               ubAmOrPm, b_12FormatEn;
    MMPS_FS_FILETIME    sFsFileTime;
	AHC_BOOL            ahcRet;
    char byOldFilename[MAX_FILE_NAME_SIZE] = {0};
        
    ahcRet = AHC_UF_SearchAvailableFileSlot( byCamID, byOldFilename );
	if (ahcRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return ahcRet;} 
    printc("--FF: Old File-- %s\r\n", byOldFilename);
    
    MMPS_FS_FileDirRename( (MMP_BYTE*)byOldFilename, strlen(byOldFilename), byFilename, strlen(byFilename) );

    AHC_GetClock(&uwYear, &uwMonth, &uwDay, &uwDayInWeek, &uwHour, &uwMinute, &uwSecond, &ubAmOrPm, &b_12FormatEn);
    sFsFileTime.usYear   = uwYear;
    sFsFileTime.usMonth  = uwMonth;
    sFsFileTime.usDay    = uwDay;
    sFsFileTime.usHour   = uwHour;
    sFsFileTime.usMinute = uwMinute;
    sFsFileTime.usSecond = uwSecond;
    MMPS_FS_FileDirSetTime((MMP_BYTE*)byFilename, strlen(byFilename), &sFsFileTime);

    // Set Non-hidden attribute to let recorded file can be seen.
    {
        AHC_FS_ATTRIBUTE sAttrib;
        AHC_FS_FileDirGetAttribute( byFilename, strlen(byFilename), &sAttrib );
        if( sAttrib & AHC_FS_ATTR_HIDDEN )
        {
            sAttrib &= ~AHC_FS_ATTR_HIDDEN;
            AHC_FS_FileDirSetAttribute( byFilename, strlen(byFilename), sAttrib );
        }
    }
    
    return AHC_TRUE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetFileName
//  Description : This function is used to set file name before video recording
//------------------------------------------------------------------------------
MMP_ERR AHC_VIDEO_SetFileName(MMP_BYTE byFilename[], UINT16 usLength, VIDENC_STREAMTYPE eStreamType, DCF_CAM_ID byCamID)
{
    #if (FS_FORMAT_FREE_ENABLE)
    MMPS_FORMATFREE_EnableWrite(1);
    AHC_FF_SetFileNameSlot( byFilename , byCamID );
    #endif

    if (eStreamType == VIDENC_STREAMTYPE_EMERGENCY) {
        MMPS_3GPRECD_SetEmergFileName(byFilename, usLength);
    }
    else {
        MMPS_3GPRECD_SetFileName(eStreamType, byFilename, usLength);
    }

	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_WaitVideoWriteFileFinish
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_WaitVideoWriteFileFinish(void)
{
	UINT32 				ulTimeOut = 20;	// unit is 100ms
	VIDENC_FW_STATUS 	status_vid;

	do {
		if (0 == ulTimeOut) {
			printc(FG_RED("--E-- Waiting for video file write to SD - Timeout !!!\r\n"));
			break;
		}

		ulTimeOut--;

		MMPD_VIDENC_GetStatus(0/*ulEncId*/, &status_vid);
		AHC_OS_SleepMs(100);

	} while ((status_vid != VIDENC_FW_STATUS_NONE) && 
	         (status_vid != VIDENC_FW_STATUS_STOP) && 
	         (status_vid != VIDENC_FW_STATUS_PREENCODE));

	AHC_OS_SleepMs(100);

	return (ulTimeOut ? AHC_TRUE : AHC_FALSE);
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_DeleteDcfMinKeyFile
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_DeleteDcfMinKeyFile(AHC_BOOL bFirstTime, const char *ext)
{
	/*
	 * FIXME: DON'T set CHECK_CROSS_LINK be 1, there is a BUG!!
	 *		  It is wrong in check free space before and after file deleted when
	 *		  that file is last one (index is 9999) in folder. The folder will be
	 *		  delete too, free space will be release more then only file deleted
	 *		  It makes fault of cross link in checking!!
	 * For CarDV never stop recording even something wrong, but popup a warning
	 * message box until user checked.
	 */
	#define CHECK_CROSS_LINK	(0)

 	AHC_BOOL			ahc_ret 		= AHC_TRUE;
   	UINT32				MaxObjIdx		= 0;
    UINT32      		CurObjIdx		= 0;
    UINT32              uiFileSize;
    AHC_BOOL            bReadOnly;
    AHC_BOOL            bCharLock;
    UINT64      		ulFreeSpace 	= 0;
	UINT32 				ulAudBitRate 	= 128000;//128K
    UINT32      		ulVidBitRate	= 0;
	UINT32				ulTimelimit		= 0;
	UINT64      		ulSpaceNeeded 	= 0;
	UINT32				InitObjIdx 		= 0;
#if (CHECK_CROSS_LINK)
	UINT64				ulFreeSpace1	= 0;
	UINT32				ulCluseterSize	= 0;
	AHC_FS_DISK_INFO 	volInfo;

	AHC_FS_GetVolumeInfo("SD:\\", strlen("SD:\\"), &volInfo);

	ulCluseterSize = (volInfo.usBytesPerSector) * (volInfo.usSecPerCluster);
#endif

	AHC_UF_SetFreeChar(0, DCF_SET_ALLOWED, (UINT8 *) ext);

	AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_BITRATE, &ulVidBitRate);

	ulTimelimit = AHC_VIDEO_GetRecTimeLimitEx();

	if (ulTimelimit == NON_CYCLING_TIME_LIMIT)
	{
		ulSpaceNeeded = 0x24000000;/* 576MB */
	}
	else
	{
        #if (defined(VIDEO_REC_TIMELAPSE_EN) && VIDEO_REC_TIMELAPSE_EN)
        UINT32 slVRTimelapse = 0;

        if ((AHC_Menu_SettingGetCB(COMMON_KEY_VR_TIMELAPSE, &slVRTimelapse) == AHC_TRUE) && 
            (slVRTimelapse != COMMON_VR_TIMELAPSE_OFF)) {
            
            UINT32 Framerate, FrateIdx;

            AIHC_VIDEO_GetMovieCfgEx(0, AHC_FRAME_RATE, &FrateIdx);
            Framerate = AHC_VIDEO_GetVideoRealFpsX1000(FrateIdx) / AHC_VIDRECD_TIME_INCREMENT_BASE;
            
            AHC_VIDEO_GetTimeLapseBitrate(Framerate, slVRTimelapse, &ulVidBitRate, &ulAudBitRate);
        }
        else
        #endif    
        {
            if (MenuSettingConfig()->uiMOVSoundRecord == MOVIE_SOUND_RECORD_OFF) {
                ulAudBitRate = 0;
            }
        }

        AHC_VIDEO_GetRecStorageSpaceNeed(ulVidBitRate, ulAudBitRate, ulTimelimit, &ulSpaceNeeded);

		if (bFirstTime) {
			ulSpaceNeeded *= 2;
		}
	}

	#if (!CHECK_CROSS_LINK)
    AHC_Media_GetFreeSpace(&ulFreeSpace);
	#endif

	do {
		#if (CHECK_CROSS_LINK)
		AHC_Media_GetFreeSpace(&ulFreeSpace);
		#endif

		if (ulFreeSpace >= ulSpaceNeeded)
		{
			printc(FG_BLUE("FreeSpace is Enough [SpaceNeeded %dKB]\r\n"),(ulSpaceNeeded>>10));
			return AHC_TRUE;
		}

	   	AHC_UF_GetTotalFileCount(&MaxObjIdx);

		if (MaxObjIdx == 0)
		{
			printc("No More DCF File for Delete!\r\n");
			return AHC_FALSE;
		}

		if (InitObjIdx >= MaxObjIdx)
		{
			printc("All %d File Can't Delete!!\r\n",MaxObjIdx);
			return AHC_FALSE;
		}

		AHC_UF_SetCurrentIndex(InitObjIdx);
		AHC_UF_GetCurrentIndex(&CurObjIdx);

        if (AHC_UF_GetFileSizebyIndex(CurObjIdx, &uiFileSize) == AHC_FALSE)
        {
		    printc("AHC_UF_GetFileSizebyIndex Error\r\n");
            return AHC_FALSE;
        }
        
        AHC_UF_IsReadOnlybyIndex(CurObjIdx, &bReadOnly);
        AHC_UF_IsCharLockbyIndex(CurObjIdx, &bCharLock);

		if (bReadOnly
		    #if	(PROTECT_FILE_TYPE == PROTECT_FILE_RENAME)
			|| bCharLock
		    #endif
            )
		{
			InitObjIdx++;
			continue;
		}

        ahc_ret = AHC_UF_FileOperation_ByIdx(CurObjIdx, DCF_FILE_DELETE_ALL_CAM, NULL, NULL);
        
	    if (ahc_ret == AHC_FALSE)
	    {
	    	printc(FG_RED("AHC_DCF_FileOperation Delete Error\r\n"));
	    	return AHC_FALSE;
	    }
	    else
	    {
	    	#if (CHECK_CROSS_LINK)
			AHC_Media_GetFreeSpace(&ulFreeSpace1);

			if (((ulFreeSpace + uiFileSize - ulCluseterSize) < ulFreeSpace1) &&
				((ulFreeSpace + uiFileSize + ulCluseterSize) > ulFreeSpace1))
			{
				// Check Pass
			}
			else
			{
				printc("FS Cross Link!!!!!\r\n");
				AHC_WMSG_Draw(AHC_TRUE, WMSG_FORMAT_SD_CARD, 3);
				return AHC_FALSE;
			}
			#else
	    	ulFreeSpace += uiFileSize;
	    	#endif
	    }
    } while(1);

	return ahc_ret;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecStorageSpaceNeed
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_GetRecStorageSpaceNeed(UINT32 ulVidBitRate, UINT32 ulAudBitRate, UINT32 ulTimelimit, UINT64 *pulSpaceNeeded) 
{
    UINT64 ulSpaceNeeded = 0;

    ulSpaceNeeded = ((((UINT64)ulVidBitRate + (UINT64)ulAudBitRate) >> 3) * (UINT64)ulTimelimit >> 3) * 9; // More 1/8 bitstream size

    // Modify TV in case media full issue. Need check free space with TV in record bitstream.
    // Suggest to check bitrate of video if need more correct size. (ex: TV in bitrate is 3Mb)
    if (MMP_IsUSBCamExist() || 
        CAM_CHECK_SCD(SCD_CAM_TV_DECODER) ||
        CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))
    {
        MMP_BOOL bStatus = MMP_TRUE;
        
        if (MMP_IsUSBCamExist()) {
            #if (USB_EN) && (UVC_HOST_VIDEO_ENABLE)
            MMPF_USBH_GetUVCPrevwSts(&bStatus);
            #endif
        }
        
        if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER) ||
            CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
            bStatus = MMP_TRUE;
        }
        
        // For Rear Cam
        if (MMP_TRUE == bStatus) {
            if (ulSpaceNeeded <= (SIGNLE_FILE_SIZE_LIMIT_3_75G / 2))
                ulSpaceNeeded <<= 1;
            else
                ulSpaceNeeded = SIGNLE_FILE_SIZE_LIMIT_3_75G;
        }
    }
    
    if (ulSpaceNeeded > SIGNLE_FILE_SIZE_LIMIT_3_75G) {
        ulSpaceNeeded = SIGNLE_FILE_SIZE_LIMIT_3_75G;
    }

    if (pulSpaceNeeded) {
        *pulSpaceNeeded = ulSpaceNeeded;
    }

    printc("Free space %dMB is need\r\n", ulSpaceNeeded >> 20);
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetNightMode
//  Description : ulMinFPS: 30, 24, 12, 8 ,4 (for IQ Table)
//------------------------------------------------------------------------------
void AHC_VIDEO_SetNightMode(AHC_BOOL bEnable, MMP_ULONG ulMinFPS)
{
    m_bNightModeEnable = bEnable;

    MMPS_Sensor_SetNightMode(bEnable, ulMinFPS);
    if(bEnable) {
    	MMPS_3GPRECD_SetMuxer3gpConstantFps(MMP_FALSE);
		MMPS_3GPRECD_SetAVSyncMethod(VIDMGR_AVSYNC_REF_AUD);
    }
    else {
    	MMPS_3GPRECD_SetMuxer3gpConstantFps(MMP_TRUE);
		MMPS_3GPRECD_SetAVSyncMethod(VIDMGR_AVSYNC_REF_VID);
    }
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetNightMode
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_GetNightMode(AHC_BOOL *pbEnable)
{
    *pbEnable = m_bNightModeEnable;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetSlowMotionFPS
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_SetSlowMotionFPS(AHC_BOOL bEnable, UINT32 usTimeIncrement, UINT32 usTimeIncrResol)
{
    m_bSlowMotionEnable = bEnable;

    m_sSlowMotionContainerFps.usVopTimeIncrement = usTimeIncrement;
    m_sSlowMotionContainerFps.usVopTimeIncrResol = usTimeIncrResol;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetSlowMotionFPS
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_GetSlowMotionFPS(AHC_BOOL *pbEnable, UINT32 *pusTimeIncrement, UINT32 *pusTimeIncrResol)
{
    *pbEnable = m_bSlowMotionEnable;

    *pusTimeIncrement = m_sSlowMotionContainerFps.usVopTimeIncrement;
    *pusTimeIncrResol = m_sSlowMotionContainerFps.usVopTimeIncrResol;
}

#if (defined(VIDEO_REC_TIMELAPSE_EN) && VIDEO_REC_TIMELAPSE_EN == 1)
//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetTimeLapseFPS
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_SetTimeLapseFPS(AHC_BOOL bEnable, UINT32 usTimeIncrement, UINT32 usTimeIncrResol)
{
    m_bTimeLapseEnable = bEnable;

    m_sTimeLapseFps.usVopTimeIncrement = usTimeIncrement;
    m_sTimeLapseFps.usVopTimeIncrResol = usTimeIncrResol;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetTimeLapseFPS
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_GetTimeLapseFPS(AHC_BOOL *pbEnable, UINT32 *pusTimeIncrement, UINT32 *pusTimeIncrResol)
{
    *pbEnable = m_bTimeLapseEnable;

    *pusTimeIncrement = m_sTimeLapseFps.usVopTimeIncrement;
    *pusTimeIncrResol = m_sTimeLapseFps.usVopTimeIncrResol;
}
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecCardSlowStop
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_SetRecCardSlowStop(AHC_BOOL bState)
{
    m_VidRecdCardSlowStop = bState;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecCardSlowStop
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetRecCardSlowStop(void)
{
    return m_VidRecdCardSlowStop;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetAPStopRecord
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_SetAPStopRecord(AHC_BOOL bState)
{
    m_APStopVideoRecord = bState;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetAPStopRecord
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetAPStopRecord(void)
{
    return m_APStopVideoRecord;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetAPStopRecordTime
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_SetAPStopRecordTime(ULONG ulLastRecdVidTime, ULONG ulLastRecdAudTime)
{
    m_APStopVideoRecord_VidTime = ulLastRecdVidTime;
    m_APStopVideoRecord_AudTime = ulLastRecdAudTime;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetAPStopRecordTime
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetAPStopRecordTime(ULONG *ulLastRecdVidTime, ULONG *ulLastRecdAudTime)
{
    *ulLastRecdVidTime = m_APStopVideoRecord_VidTime; // Unit: ms
    *ulLastRecdAudTime = m_APStopVideoRecord_AudTime; // Unit: ms
    
    return AHC_TRUE;
}

MMP_USHORT  AHC_VIDEO_FileIdToStreamType(UINT32 ulFileID)
{
    char   fname[MAX_FILE_NAME_SIZE];
    MMP_USHORT  st = VIDENC_STREAMTYPE_MAX;
    
    STRCPY(fname, (char *)((FS_FILE *)ulFileID)->FileNameMark);
    if (strcmp(fname, m_CurVRFullName) == 0)
    {
        st = VIDENC_STREAMTYPE_VIDRECD;
    }
    #if (VIDRECD_MULTI_TRACK == 0)
    #if (UVC_VIDRECD_SUPPORT == 1) || (DUALENC_SUPPORT == 1)
    else if (strcmp(fname, m_CurVRRearFullName) == 0) 
    {
        #if (SUPPORT_USB_HOST_FUNC) 
        if ((MMP_IsUSBCamExist() && MMP_IsUSBCamIsoMode()) || (MMP_IsScdCamExist()))
        #else
        if (MMP_IsScdCamExist())
        #endif
            st = VIDENC_STREAMTYPE_DUALENC;
        #if (SUPPORT_USB_HOST_FUNC)     
        else if (MMP_IsUSBCamExist() && !MMP_IsUSBCamIsoMode())
            st = VIDENC_STREAMTYPE_UVCRECD;
        #endif    
    }
    #endif
    #endif
    #if (AHC_EMERGENTRECD_SUPPORT)
    else if (strcmp(fname, m_chEmerVRFullName) == 0)
    {
        st = VIDENC_STREAMTYPE_EMERGENCY;
    }
    #endif
    #if (VIDRECD_MULTI_TRACK == 0)
    #if (AHC_UVC_EMERGRECD_SUPPORT == 1) || (AHC_DUAL_EMERGRECD_SUPPORT == 1)
    else if (strcmp(fname, m_EmrVRRearFullName) == 0)
    {
    	#if (SUPPORT_USB_HOST_FUNC)
        if (MMP_IsUSBCamExist() && MMP_IsUSBCamIsoMode())
            st = VIDENC_STREAMTYPE_DUALEMERG;
        else if (MMP_IsUSBCamExist() && !MMP_IsUSBCamIsoMode())
            st = VIDENC_STREAMTYPE_UVCEMERG;
        #else
            st = VIDENC_STREAMTYPE_MAX;    
        #endif    
    }
    #endif
    #endif
    #if (AHC_SHAREENC_SUPPORT)
    else if (strcmp(fname, m_ShareRecFullFileName) == 0)
    {
        st = VIDENC_STREAMTYPE_DUALENC;
    }
    #endif
    
    return st;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RecordStartWriteInfo
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_RecordStartWriteInfo( void )
{
#if (GPS_CONNECT_ENABLE && (GPS_FUNC & FUNC_VIDEOSTREM_INFO))
    if (AHC_GPS_Module_Attached()) {
        UINT8 bGPS_en;
        
        if ((AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) && 
            (bGPS_en != COMMON_GPS_REC_INFO_OFF)) {
            GPSCtrl_StartRecord();
        }
    }
#endif

#if (GSENSOR_CONNECT_ENABLE && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO))
    if (AHC_Gsensor_Module_Attached()) {
        AHC_Gsensor_StartRecord();
    }
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RecordPostWriteInfo
//  Description :
//------------------------------------------------------------------------------
UINT32 AHC_VIDEO_RecordPostWriteInfo(UINT32 ulFileID)
{
    #if (((GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO)) || \
         ((GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)))
    UINT32 ulWriteSize = 0;
    UINT32 ulTotalWSize = 0;
    #endif
    
    #if (((GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO)) || \
         ((GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)))
    filestreamtype = AHC_VIDEO_FileIdToStreamType(ulFileID);
    #endif

#if ((GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO))
    #if (AVI_IDIT_CHUNK_EN == 1)
    {
        void            *ptr;
        unsigned int    cb, wr;

        #if (VR_CONTAINER_TYPE == COMMON_VR_VIDEO_TYPE_3GP)
        cb = MMPS_3GPMUX_Build_IDIT(&ptr);
        #else //COMMON_VR_VIDEO_TYPE_AVI
        cb = MMPS_AVIMUX_Build_IDIT(&ptr);
        #endif
        
        VR_WriteFunc_Ex(ptr, cb, &wr, ulFileID, AHC_FALSE);
        ulTotalWSize += wr;
    }
    #endif

    if (AHC_GPS_Module_Attached())
    {
        UINT8 bGPS_en;
        
        if ((AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) && 
            (bGPS_en != COMMON_GPS_REC_INFO_OFF)) {
            
            GPSCtrl_SetHeaderForAviChuck_GPSConfig();
            VR_WriteFunc_Ex((void *)GPSCtrl_HeaderAddressForAviChuck(0), 8, &ulWriteSize, ulFileID, AHC_FALSE);
            ulTotalWSize += ulWriteSize;
            VR_WriteFunc_Ex((void *)GPSCtrl_AttributeAddressForAviChuck(), sizeof(GPS_ATTRIBUTE), &ulWriteSize, ulFileID, AHC_FALSE);
            ulTotalWSize += ulWriteSize;

            #if (AHC_SHAREENC_SUPPORT) && (VIDRECD_MULTI_TRACK == 0)
            if ((m_bStartShareRec) && (filestreamtype == VIDENC_STREAMTYPE_DUALENC))
                GPSCtrl_SetHeaderForAviChuckEx();    
            else
            #endif
            GPSCtrl_SetHeaderForAviChuck();
            VR_WriteFunc_Ex((void *)GPSCtrl_HeaderAddressForAviChuck(1), 8, &ulWriteSize, ulFileID, AHC_FALSE);
            ulTotalWSize += ulWriteSize;

            #if (GPS_USE_FILE_AS_DATABUF)
            {
                extern UINT32 ulGPSInfoFileByte, ulGPSInfoLastByte;
                extern UINT32 ulGPSInfoFileId;

                UINT8  temp[256];
                UINT32 ultotalRSize, ulReadCount;
                pGPSInfoChuck InfoAddr;

                if (filestreamtype == VIDENC_STREAMTYPE_VIDRECD)
                {
                    AHC_FS_FileSeek(ulGPSInfoFileId, 0, AHC_FS_SEEK_SET);

                    // Read data from temp file then write to AVI.
                    for (ultotalRSize = ulGPSInfoFileByte; (int)ultotalRSize > 0; )
                    {
                        if (AHC_FS_FileRead(ulGPSInfoFileId,
                                            (UINT8 *)temp,
                                            sizeof(temp) / sizeof(UINT8),
                                            &ulReadCount) != AHC_ERR_NONE ||
                            ulReadCount == 0) {
                            break;
                        }

                        VR_WriteFunc_Ex((void *)temp, ulReadCount, &ulWriteSize, ulFileID, AHC_FALSE);
                        ulTotalWSize += ulWriteSize;
                        ultotalRSize -= ulReadCount;
                    }
                }
                
                // Flush last data from ping-pong buffer
                if (AHC_GPS_NeedToFlushBackupBuffer() == AHC_TRUE) {
                    AHC_GPS_FlushBackupBuffer(AHC_FALSE);
                    InfoAddr = (pGPSInfoChuck)GPSCtrl_GetBackupDataAddr();
                }
                #if (AHC_SHAREENC_SUPPORT) && (VIDRECD_MULTI_TRACK == 0)
                else if ((m_bStartShareRec) && (filestreamtype == VIDENC_STREAMTYPE_DUALENC))
                {
                    InfoAddr = (pGPSInfoChuck)GPSCtrl_GetLastDataAddrEx();
                    ulGPSInfoLastByte = GPSCtrl_InfoSizeForAviChuckEx();
                }
                #endif
                else {
                    InfoAddr = (pGPSInfoChuck)GPSCtrl_GetLastDataAddr();
                }
                
                if (InfoAddr != NULL) {
                    VR_WriteFunc_Ex((void *)InfoAddr, ulGPSInfoLastByte, &ulWriteSize, ulFileID, AHC_FALSE);
                    ulTotalWSize += ulWriteSize;
                }
            }
            #else
                #if (AHC_SHAREENC_SUPPORT) && (VIDRECD_MULTI_TRACK == 0)
            if ((m_bStartShareRec) && (filestreamtype == VIDENC_STREAMTYPE_DUALENC))
            {
                VR_WriteFunc_Ex((void *)GPSCtrl_InfoAddressForAviChuckEx(), GPSCtrl_InfoSizeForAviChuckEx(), &ulWriteSize, ulFileID, AHC_FALSE);
            }
            else
                #endif
            {
                VR_WriteFunc_Ex((void *)GPSCtrl_InfoAddressForAviChuck(), GPSCtrl_InfoSizeForAviChuck(), &ulWriteSize, ulFileID, AHC_FALSE);
            }

            ulTotalWSize += ulWriteSize;            
            #endif
        }
    }
#endif

#if (GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)
    if (AHC_Gsensor_Module_Attached())
    {
        AHC_Gsensor_SetAttribChuckHeader();
        VR_WriteFunc_Ex((void *)AHC_Gsensor_GetChuckHeaderAddr(0), 8, &ulWriteSize, ulFileID, AHC_FALSE);
        ulTotalWSize += ulWriteSize;
        VR_WriteFunc_Ex((void *)AHC_Gsensor_GetAttributeAddr(), sizeof(GSNR_ATTRIBUTE), &ulWriteSize, ulFileID, AHC_FALSE);
        ulTotalWSize += ulWriteSize;

        #if (AHC_SHAREENC_SUPPORT) && (VIDRECD_MULTI_TRACK == 0)
        if ((m_bStartShareRec) && (filestreamtype == VIDENC_STREAMTYPE_DUALENC))
            AHC_Gsensor_SetInfoChuckHeaderEx();
        else
        #endif
        AHC_Gsensor_SetInfoChuckHeader();
        VR_WriteFunc_Ex((void *)AHC_Gsensor_GetChuckHeaderAddr(1), 8, &ulWriteSize, ulFileID, AHC_FALSE);
        ulTotalWSize += ulWriteSize;

        #if (GSNR_USE_FILE_AS_DATABUF)
        {
            extern UINT32 m_ulInfoFileByte, m_ulInfoLastByte;
            extern UINT32 m_ulInfoFileId;

            UINT8  temp[256];
            UINT32 ultotalRSize, ulReadCount;
            PGSNR_INFOCHUCK InfoAddr;
            AHC_ERR err;

            if (filestreamtype == VIDENC_STREAMTYPE_VIDRECD)
            {
                AHC_FS_FileSeek(m_ulInfoFileId, 0, AHC_FS_SEEK_SET);

                // Read data from temp file then write to AVI.
                for (ultotalRSize = m_ulInfoFileByte; ultotalRSize > 0; )
                {
                    err = AHC_FS_FileRead(  m_ulInfoFileId,
                                            (UINT8 *)temp,
                                            256,
                                            &ulReadCount);

                    if (err != AHC_ERR_NONE || ulReadCount == 0) {
                        break;
                    }

                    VR_WriteFunc_Ex((void *)temp, ulReadCount, &ulWriteSize, ulFileID, AHC_FALSE);
                    ulTotalWSize += ulWriteSize;
                    ultotalRSize -= ulReadCount;
                }
            }
            
            // Flush last data from ping-pong buffer
            if (AHC_Gsensor_NeedToFlushBackupBuffer() == AHC_TRUE) {
                AHC_Gsensor_FlushBackupBuffer( AHC_FALSE );
                InfoAddr = (PGSNR_INFOCHUCK)AHC_Gsensor_GetBackupDataAddr();
            }
            #if (AHC_SHAREENC_SUPPORT) && (VIDRECD_MULTI_TRACK == 0)
            else if ((m_bStartShareRec) && (filestreamtype == VIDENC_STREAMTYPE_DUALENC))
            {
                InfoAddr = (PGSNR_INFOCHUCK)AHC_Gsensor_GetLastDataAddrEx();
                m_ulInfoLastByte = AHC_Gsensor_GetInfoChuckSizeEx();
            }
            #endif            
            else {
                InfoAddr = (PGSNR_INFOCHUCK)AHC_Gsensor_GetLastDataAddr();
            }
            
            if (InfoAddr != NULL) {
                VR_WriteFunc_Ex((void *)InfoAddr, m_ulInfoLastByte, &ulWriteSize, ulFileID, AHC_FALSE);
                ulTotalWSize += ulWriteSize;
            }                
        }
        #else
            #if (AHC_SHAREENC_SUPPORT) && (VIDRECD_MULTI_TRACK == 0)
        if ((m_bStartShareRec) && (filestreamtype == VIDENC_STREAMTYPE_DUALENC))
        {
            VR_WriteFunc_Ex((void *)AHC_Gsensor_GetInfoChuckAddrEx(),AHC_Gsensor_GetInfoChuckSizeEx(),&ulWriteSize, ulFileID, AHC_FALSE);
        }
        else
            #endif
        {    
            VR_WriteFunc_Ex((void *)AHC_Gsensor_GetInfoChuckAddr(),AHC_Gsensor_GetInfoChuckSize(),&ulWriteSize, ulFileID, AHC_FALSE);
        }
        
        ulTotalWSize += ulWriteSize;        
        #endif
    }
#endif

#if (FS_FORMAT_FREE_ENABLE == 1)
    // Add some dummy data to avoid some PC tools need a long time to parse the file (with lots of 0x00 data in the tail of file).
    // Root cause: Unknown
    {
        UINT8 byFormatFreeDummyData[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA};
        VR_WriteFunc_Ex((void *)byFormatFreeDummyData, sizeof(byFormatFreeDummyData), &ulWriteSize, ulFileID, AHC_FALSE);
        ulTotalWSize += ulWriteSize;
    }
#endif

    // Use below code to write all data from buffer to media
    VR_WriteFunc_Ex(NULL, 0, &ulWriteSize, ulFileID, AHC_TRUE);
    ulTotalWSize += ulWriteSize;
    
    // Close GPS Raw/KML File for normal record
#if (GPS_CONNECT_ENABLE == 1)
    if (AHC_GPS_Module_Attached())
    {
        #if (GPS_RAW_FILE_ENABLE == 1)
        UINT8 bGPS_en;

        if (AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) {
            switch (bGPS_en) {
            case RECODE_GPS_OFF:
            case RECODE_GPS_IN_VIDEO:
                // NOP
                break;
            default:
                GPSCtrl_GPSRawClose();
                break;
            }
        }
        #endif
        #if (GPS_KML_FILE_ENABLE == 1)
        GPSCtrl_KMLGen_Write_TailAndClose();
        #endif
    }
#endif

    #if (GSENSOR_RAW_FILE_ENABLE == 1 && GPS_CONNECT_ENABLE == 0)
    GPSCtrl_GPSRawClose();
    #endif

    // Close GPS Raw/KML File for emergency record
    #if (GPS_CONNECT_ENABLE == 1)
    if (AHC_GPS_Module_Attached() && !AHC_VIDEO_IsEmergRecStarted())
    {
        #if (GPS_RAW_FILE_EMER_EN == 1)
        UINT8 bGPS_en;
        
        if (AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) {
            switch (bGPS_en) {
            case RECODE_GPS_OFF:
            case RECODE_GPS_IN_VIDEO:
                // NOP
                break;
            default:
                GPSCtrl_GPSRawClose_Emer();
                break;
            }
        }
        #endif
    }
    #endif

    #if (GSENSOR_RAW_FILE_ENABLE == 1 && GPS_CONNECT_ENABLE == 0 && GPS_RAW_FILE_EMER_EN == 1)
    if (!AHC_VIDEO_IsEmergRecStarted())
        GPSCtrl_GPSRawClose_Emer();
    #endif

    #if (AHC_SHAREENC_SUPPORT)      
    {
    	//extern MMP_BOOL m_bStartGPS_SHARE, m_bStartGsnr_SHARE;
        UINT8  tmpzero[8] = {0};
        if (m_bStartShareRec)
        {              	
            #if (GPS_CONNECT_ENABLE && (GPS_FUNC & FUNC_VIDEOSTREM_INFO))
          	AHC_GPS_SetSHAREIndex();
  	        #endif
      	    #if (GSENSOR_CONNECT_ENABLE && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO))
            AHC_Gsensor_SetSHAREIndex();
          	#endif    

            #if ( ((GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO))     || \
              ((GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)) )
          	VR_WriteFunc((void *)tmpzero, sizeof(tmpzero), &ulWriteSize, ulFileID);
          	ulTotalWSize += ulWriteSize;
      	    #endif
      	}
    }
    #endif
    
    #if ( ((GPS_CONNECT_ENABLE) && (GPS_FUNC & FUNC_VIDEOSTREM_INFO))     || \
          ((GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO)) )
    return ulTotalWSize;
    #else
    return 0;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_IsVRSeamless
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_SetVRSeamless(AHC_BOOL bSeamless)
{
    m_bSeamlessRecord = bSeamless;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_IsVRSeamless
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_IsVRSeamless(void)
{
    return m_bSeamlessRecord;
}

#if 0
void _____VR_Time_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_AvailableRecTime
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_AvailableRecTime(UBYTE* Hour, UBYTE* Min, UBYTE* Sec)
{
    AHC_VIDEO_AvailableRecTimeEx(0, Hour, Min, Sec);
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_AvailableRecTimeEx
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_AvailableRecTimeEx(UINT32 bitrate, UBYTE* Hour, UBYTE* Min, UBYTE* Sec)
{
    VIDMGR_CONTAINER_TYPE ContainerType;
    UINT32      Param;
    UINT64      ulFreeSpace = 0;
    UINT32      ulTotalTime = 0;
    UINT32      ulVideoReservedSize;
    UINT32      ulVideoBitRate, ulAudioBitRate;
    UBYTE       ubHour;
    #if (AHC_SHAREENC_SUPPORT)
    UINT32      ul2ndVideoBitRate;
    #endif

    AIHC_VIDEO_GetMovieCfgEx(0, AHC_CLIP_CONTAINER_TYPE, &Param);
    ContainerType = Param;
    
    MMPS_3GPRECD_SetContainerType(ContainerType);

    /* Get Media Free Space */
    AHC_Media_GetFreeSpace(&ulFreeSpace);

    AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_RESERVED_SIZE, &ulVideoReservedSize);
    
    if (bitrate == 0)
        AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_BITRATE, &ulVideoBitRate);
    else
        ulVideoBitRate = bitrate;
        
    #if (AHC_SHAREENC_SUPPORT)
    AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_2ND_BITRATE, &ul2ndVideoBitRate);
    ulVideoBitRate += ul2ndVideoBitRate;
    #endif
    
    AIHC_VIDEO_GetMovieCfgEx(0, AHC_AUD_BITRATE, &ulAudioBitRate);

    if (ulFreeSpace <= ulVideoReservedSize) {
        *Hour = 0;
        *Min  = 0;
        *Sec  = 0;
        return;
    }

    ulTotalTime = MMPS_3GPRECD_GetExpectedRecordTime(ulFreeSpace - ulVideoReservedSize, ulVideoBitRate, ulAudioBitRate);

    ubHour = ulTotalTime / (60*60);

    *Hour = ubHour;

    if (ubHour < 100) {
        *Min  = (ulTotalTime / 60) - (ubHour * 60);
        *Sec  = ulTotalTime % 60;
    }
    else {
        *Hour = 99;
        *Min  = 59;
        *Sec  = 59;
    }
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetCurRecordingTime
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetCurRecordingTime(UINT32 *ulTime)
{
    if (!MMPS_3GPRECD_GetRecordTime(ulTime))
        return AHC_FALSE;
    else
        return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_ChangeCurFileTimeLimit
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_ChangeCurFileTimeLimit(UINT32 ulTimeLimitMs)
{
    DBG_AutoTestPrint(ATEST_ACT_EMERGENCY_0x0009, ATEST_STS_START_0x0001, 0, 0, gbAhcDbgBrk);
    
	if (MMP_ERR_NONE == MMPS_3GPRECD_ChangeCurFileTimeLimit(ulTimeLimitMs)) {
		return AHC_TRUE;
	}
	return AHC_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecTimeLimitEx
//  Description :
//------------------------------------------------------------------------------
UINT32 AHC_VIDEO_GetRecTimeLimitEx(void)
{
    #define SEC_PER_MIN  (60)

    UINT32	m_VRTimeLimit = 0;  // Unit:seconds
    UINT32  clipTime;

    if (AHC_Menu_SettingGetCB((char*)COMMON_KEY_VR_CLIP_TIME, &clipTime) == AHC_FALSE) {
        RTNA_DBG_Str(0, FG_RED("Get COMMON_KEY_VR_CLIP_TIME error - using default\r\n"));
        clipTime = MOVIE_CLIP_TIME_OFF;
    }
    
	printc("clipTime %d\r\n",clipTime);
    
    switch (clipTime)
    {
    case COMMON_MOVIE_CLIP_TIME_6SEC:
        m_VRTimeLimit = 6;
        break;
    case COMMON_MOVIE_CLIP_TIME_1MIN:
        m_VRTimeLimit = 1*SEC_PER_MIN;
        break;
    case COMMON_MOVIE_CLIP_TIME_2MIN:
        m_VRTimeLimit = 2*SEC_PER_MIN;
        break;
    case COMMON_MOVIE_CLIP_TIME_3MIN:
        m_VRTimeLimit = 3*SEC_PER_MIN;
        break;
    case COMMON_MOVIE_CLIP_TIME_5MIN:
        m_VRTimeLimit = 5*SEC_PER_MIN;
        break;
    case COMMON_MOVIE_CLIP_TIME_10MIN:
        m_VRTimeLimit = 10*SEC_PER_MIN;
        break;
    case COMMON_MOVIE_CLIP_TIME_25MIN:
        m_VRTimeLimit = 25*SEC_PER_MIN;
        break;
    case COMMON_MOVIE_CLIP_TIME_30MIN:
        m_VRTimeLimit = 30*SEC_PER_MIN;
        break;
    case COMMON_MOVIE_CLIP_TIME_OFF:
    default:
        m_VRTimeLimit = NON_CYCLING_TIME_LIMIT;
        break;
    }

    #if (MOTION_DETECTION_EN)
    #if (SET_MOTION_DETECTION_MOVIE_CLIP_TIME)
    {
        extern AHC_BOOL m_ubMotionDtcEn;

		if (m_ubMotionDtcEn)
        {
			m_VRTimeLimit = MOTION_DETECTION_MOVIE_CLIP_TIME;
		}
	}
	#endif
    #endif

#ifdef VIDEO_REC_TIMELAPSE_EN
    #if (VIDEO_REC_TIMELAPSE_EN) // CHECK
    {
        UINT32 ulTimeLapseSetting;
        UINT32 Frate;
        UINT64 ullFramerate_1000Times;
        
        AIHC_VIDEO_GetMovieCfgEx(0, AHC_FRAME_RATE, &Frate);
        ullFramerate_1000Times = (AHC_VIDEO_GetVideoRealFpsX1000(Frate)*1000) / AHC_VIDRECD_TIME_INCREMENT_BASE;
        
        if ((AHC_Menu_SettingGetCB(COMMON_KEY_VR_TIMELAPSE, &ulTimeLapseSetting) == AHC_TRUE) && 
            (ulTimeLapseSetting != COMMON_VR_TIMELAPSE_OFF) &&
		    (m_VRTimeLimit != NON_CYCLING_TIME_LIMIT)) {
		    
            switch(ulTimeLapseSetting){
            case COMMON_VR_TIMELAPSE_5SEC:
                m_VRTimeLimit = (UINT32)(((UINT64)m_VRTimeLimit * ullFramerate_1000Times * 5)/1000);
                break;
            case COMMON_VR_TIMELAPSE_10SEC:
                m_VRTimeLimit = (UINT32)(((UINT64)m_VRTimeLimit * ullFramerate_1000Times * 10)/1000);
                break;
            case COMMON_VR_TIMELAPSE_30SEC:
                m_VRTimeLimit = (UINT32)(((UINT64)m_VRTimeLimit * ullFramerate_1000Times * 30)/1000);                    
                break;
            case COMMON_VR_TIMELAPSE_60SEC:
                m_VRTimeLimit = (UINT32)(((UINT64)m_VRTimeLimit * ullFramerate_1000Times * 60)/1000);                    
                break;
            case COMMON_VR_TIMELAPSE_1SEC:
            default:
                m_VRTimeLimit = (UINT32)(((UINT64)m_VRTimeLimit * ullFramerate_1000Times * 1)/1000);                    
                break;
            }
        }
    }
    #endif
#endif

    return m_VRTimeLimit;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecTimeLimit
//  Description :
//------------------------------------------------------------------------------
UINT32 AHC_VIDEO_GetRecTimeLimit(void)
{
    return AHC_VIDEO_GetRecTimeLimitEx();
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecTimeLimit
//  Description :
//------------------------------------------------------------------------------
UINT32 AHC_VIDEO_SetRecTimeLimit(UINT32 ulTimeLimit)
{
    if (ulTimeLimit < ((0x7FFFFFFF - VR_TIME_LIMIT_OFFSET) / 1000)) {
        ulTimeLimit = ulTimeLimit * 1000 + VR_TIME_LIMIT_OFFSET;
    }

    MMPS_3GPRECD_SetFileTimeLimit(ulTimeLimit);
    
    return 0;
}

void AHC_VIDEO_AvoidDuplicatedFileName(AHC_RTC_TIME *psAHC_RTC_Time)
{
    static AHC_RTC_TIME sVideoRecStartRtcTime_Last[DCF_DB_TYPE_MAX_NUMBER] = {0};
    DCF_DB_TYPE emCurrDcfDB = AHC_UF_GetDB();

    // Avoid the same filename for un-expected DCF / File System issue
    if ( sVideoRecStartRtcTime_Last[emCurrDcfDB].uwHour   == psAHC_RTC_Time->uwHour   &&
         sVideoRecStartRtcTime_Last[emCurrDcfDB].uwMinute == psAHC_RTC_Time->uwMinute &&
         sVideoRecStartRtcTime_Last[emCurrDcfDB].uwSecond == psAHC_RTC_Time->uwSecond ) 
    {
        printc("WARNING!!! The same DateTime filename as previous file!!\r\n");
        // Add one second in filename
        AHC_DT_ShiftRtc(psAHC_RTC_Time, 1);
    }

    sVideoRecStartRtcTime_Last[emCurrDcfDB].uwHour   = psAHC_RTC_Time->uwHour;
    sVideoRecStartRtcTime_Last[emCurrDcfDB].uwMinute = psAHC_RTC_Time->uwMinute;
    sVideoRecStartRtcTime_Last[emCurrDcfDB].uwSecond = psAHC_RTC_Time->uwSecond;
}

#if 0
void _____VR_DualRec_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetDualEncSetting
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_SetDualEncSetting(void)
{
    VIDENC_PROFILE              sProfile = H264ENC_BASELINE_PROFILE;
    MMP_USHORT                  usResoIdx = VIDRECD_RESOL_720x480;
    MMP_ULONG                   ulEncW, ulEncH;
    VIDENC_CURBUF_MODE          sVideoCurBufMode = VIDENC_CURBUF_FRAME;
    MMPS_3GPRECD_FRAMERATE      sSensorInFps  = {AHC_VIDRECD_TIME_INCREMENT_BASE_NTSC, 30000};
    MMPS_3GPRECD_FRAMERATE      sEncodeFps    = {AHC_VIDRECD_TIME_INCREMENT_BASE_NTSC, 30000};
    MMPS_3GPRECD_FRAMERATE      sContainerFps = {AHC_VIDRECD_TIME_INCREMENT_BASE_NTSC, 30000};
    MMP_USHORT                  usQuality = VIDRECD_QUALITY_LOW;
    MMP_USHORT                  usPFrameCount = 14, usBFrameCount = 0;
    MMP_SNR_TVDEC_SRC_TYPE      sTVDecSrc = MMP_SNR_TVDEC_SRC_NO_READY;
    MMP_ULONG                   ubVidTimeIncBase = AHC_VIDRECD_TIME_INCREMENT_BASE_NTSC;
    MMP_ERR                     sRet;

    if ((CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) ||\
        (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
        
        if (DUALSNR_DUAL_ENCODE == MMP_GetDualSnrEncodeType()) {
            
            if (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
                #if (SCALE_UP_REC_HD_TO_FHD_TEST)
                usResoIdx = VIDRECD_RESOL_1920x1088;
                #else
                if (0xFFFF1302 == gsSensorFunction->MMPF_Sensor_GetSnrID(gsAhcScdSensor)) {
                    usResoIdx = VIDRECD_RESOL_1920x1088;
                }
                else {
                    usResoIdx = VIDRECD_RESOL_1280x720;
                }
                #endif
            }    
            else if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
                usResoIdx = VIDRECD_RESOL_1920x1088;
            }    
            
            if (GET_VR_ENCODE_BUFMODE(gsAhcScdSensor) == VIDREC_CURBUF_FRAME) {
                sVideoCurBufMode = VIDENC_CURBUF_FRAME;
            }
            else {
                sVideoCurBufMode = VIDENC_CURBUF_RT;
            }
        }
    }
    else if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) {
		MMPS_Sensor_GetTVDecSrcType(&sTVDecSrc);
		
		switch(sTVDecSrc) {
		case MMP_SNR_TVDEC_SRC_PAL:
		case MMP_SNR_TVDEC_SRC_NTSC:
			usResoIdx = VIDRECD_RESOL_720x480;
			break;
		case MMP_SNR_TVDEC_SRC_HD:
			usResoIdx = VIDRECD_RESOL_1280x720;
			break;
		case MMP_SNR_TVDEC_SRC_FHD:
			usResoIdx = VIDRECD_RESOL_1920x1088;
			break;
		case MMP_SNR_TVDEC_SRC_NO_READY:
		default:
			usResoIdx = VIDRECD_RESOL_720x480;
			AHC_PRINT_RET_ERROR(0, 0); 
			break;
		}
		
        sVideoCurBufMode = VIDENC_CURBUF_FRAME;
    }
    else {
        return;
    }
    
    /* Set 2nd Video Record Attribute */
    sRet = MMPS_3GPRECD_SetEncResIdx(MMPS_3GPRECD_FILESTREAM_DUAL, usResoIdx);

    sRet |= MMPS_3GPRECD_SetProfile(MMPS_3GPRECD_FILESTREAM_DUAL, sProfile);
    sRet |= MMPS_3GPRECD_SetCurBufMode(MMPS_3GPRECD_FILESTREAM_DUAL, sVideoCurBufMode);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return;} 
    
    if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) {
        MMPS_Sensor_GetTVDecSrcType(&sTVDecSrc);
        
        switch(sTVDecSrc) {
        case MMP_SNR_TVDEC_SRC_PAL:
            ubVidTimeIncBase = AHC_VIDRECD_TIME_INCREMENT_BASE_PAL;
            break;
        case MMP_SNR_TVDEC_SRC_NTSC:
            ubVidTimeIncBase = AHC_VIDRECD_TIME_INCREMENT_BASE_NTSC;
            break;
		case MMP_SNR_TVDEC_SRC_HD:
		case MMP_SNR_TVDEC_SRC_FHD:
            ubVidTimeIncBase = AHC_VIDRECD_TIME_INCREMENT_BASE;
            break;
        case MMP_SNR_TVDEC_SRC_NO_READY:
        default:
            AHC_PRINT_RET_ERROR(0, 0); return; 
            break;
        }
    }
    else {
        ubVidTimeIncBase = AHC_VIDRECD_TIME_INCREMENT_BASE;
    }
    
    sSensorInFps.usVopTimeIncrement  = ubVidTimeIncBase;
    sSensorInFps.usVopTimeIncrResol  = 30000; // TBD
    sEncodeFps.usVopTimeIncrement    = sSensorInFps.usVopTimeIncrement;
    sEncodeFps.usVopTimeIncrResol    = sSensorInFps.usVopTimeIncrResol;
    sContainerFps.usVopTimeIncrement = sSensorInFps.usVopTimeIncrement;
    sContainerFps.usVopTimeIncrResol = sSensorInFps.usVopTimeIncrResol;

    sRet = MMPS_3GPRECD_SetFrameRatePara(MMPS_3GPRECD_FILESTREAM_DUAL, &sSensorInFps, &sEncodeFps, &sContainerFps);
    #if (SUPPORT_VR_FHD_60FPS)
    sRet |= MMPS_3GPRECD_SetBitrate(MMPS_3GPRECD_FILESTREAM_DUAL, MMPS_3GPRECD_GetConfig()->ulFps60BitrateMap[usResoIdx][usQuality]);
    #else
    sRet |= MMPS_3GPRECD_SetBitrate(MMPS_3GPRECD_FILESTREAM_DUAL, MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[usResoIdx][usQuality]);
    #endif

    sRet |= MMPS_3GPRECD_SetPFrameCount(MMPS_3GPRECD_FILESTREAM_DUAL, usPFrameCount);
    sRet |= MMPS_3GPRECD_SetBFrameCount(MMPS_3GPRECD_FILESTREAM_DUAL, usBFrameCount);
    
    if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
        if (DUALSNR_DUAL_ENCODE == MMP_GetDualSnrEncodeType()) {
            AHC_GetImageSize(VIDEO_CAPTURE_MODE, &ulEncW, &ulEncH);
        }
    }
    else if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER) ||
             CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
        ulEncW = MMPS_3GPRECD_GetConfig()->usEncWidth[usResoIdx];
        ulEncH = MMPS_3GPRECD_GetConfig()->usEncHeight[usResoIdx];
    }

    sRet |= MMPS_3GPRECD_CustomedEncResol(MMPS_3GPRECD_FILESTREAM_DUAL,
                                          MMP_SCAL_FITMODE_OPTIMAL,
                                          (MMP_USHORT)ulEncW,
                                          (MMP_USHORT)ulEncH);
    
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return;}
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_DualRecordStart
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_DualRecordStart(void)
{
    MMP_ERR sRet;
    
    AHC_VIDEO_SetDualEncSetting();

    sRet = MMPS_3GPRECD_StartDualH264();
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(0,sRet); return AHC_FALSE;} 

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_CheckDualRecEnabled
//  Description :
//------------------------------------------------------------------------------
UINT8 AHC_VIDEO_CheckDualRecEnabled(UINT8 ubCamType, AHC_BOOL bChkConnect)
{
    if (CAM_CHECK_SCD(SCD_CAM_NONE) && 
        CAM_CHECK_USB(USB_CAM_NONE)) {
        #if (AHC_SHAREENC_SUPPORT)
        return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
        #else
        return DUAL_REC_DISABLE;
        #endif
    }

    if (ubCamType == CAM_TYPE_SCD) {
    
        switch(CAM_GET_SCD)
        {
        case SCD_CAM_TV_DECODER:
            if (bChkConnect) {
                if (AHC_TRUE == AHC_SNR_GetTvDecSnrCnntStatus())
                    return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
                else
                    return DUAL_REC_DISABLE;
            }
            else {
                return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
            }
        break;
        case SCD_CAM_BAYER_SENSOR:
            if (DUALSNR_DUAL_ENCODE == MMP_GetDualSnrEncodeType())
                return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
            else
                return DUAL_REC_DISABLE;
        break;
        case SCD_CAM_YUV_SENSOR:
            if (DUALSNR_DUAL_ENCODE == MMP_GetDualSnrEncodeType())
                return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
            else
                return DUAL_REC_DISABLE;
        break;
        default:
            return DUAL_REC_DISABLE;
        break;
        }
    }
    else if (ubCamType == CAM_TYPE_USB) {
    
        switch(CAM_GET_USB)
        {
        case USB_CAM_AIT:
            if (bChkConnect) {
                if (AHC_TRUE == AHC_HostUVCVideoIsEnabled())
                    return DUAL_REC_STORE_FILE;
                else
                    return DUAL_REC_DISABLE;
            }
            else {
                return DUAL_REC_STORE_FILE;
            }
        break;
        case USB_CAM_SONIX_MJPEG:
            if (bChkConnect) {
                if (AHC_TRUE == AHC_HostUVCVideoIsEnabled())
                    return DUAL_REC_STORE_FILE;
                else
                    return DUAL_REC_DISABLE;
            }
            else {
                return DUAL_REC_STORE_FILE;
            }
        break;
        case USB_CAM_SONIX_MJPEG2H264:
            if (bChkConnect) {
                if (AHC_TRUE == AHC_HostUVCVideoIsEnabled())
                    return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
                else
                    return DUAL_REC_DISABLE;
            }
            else {
                return DUAL_REC_STORE_FILE | DUAL_REC_ENCODE_H264;
            }
        break;
        default:
            return DUAL_REC_DISABLE;
        break;
        }
    }
    
    return DUAL_REC_DISABLE;
}

#if 0
void _____VR_EmergRec_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_IsEmergRecStarted
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_IsEmergRecStarted(void)
{
#if (EMER_RECORD_DUAL_WRITE_ENABLE)
    return m_bStartEmerVR;
#else
    return AHC_FALSE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetEmergRecStarted
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetEmergRecStarted(AHC_BOOL bEmerRecordStarted)
{
#if (EMER_RECORD_DUAL_WRITE_ENABLE)
    m_bStartEmerVR = bEmerRecordStarted;
    return AHC_TRUE;
#else
    return AHC_FALSE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_IsEmergPostDone
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_IsEmergPostDone(void)
{
#if (EMER_RECORD_DUAL_WRITE_ENABLE)
    return m_bEmerVRPostDone;
#else
    return AHC_FALSE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetEmergPostDone
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetEmergPostDone(AHC_BOOL bDone)
{
#if (AHC_EMERGENTRECD_SUPPORT)
    m_bEmerVRPostDone = bDone;
    return AHC_TRUE;
#else
    return AHC_FALSE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetKernalEmergStopStep
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_STOP_STEP bKernalEmergencyStopStep)
{
    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    m_bKernalEmergencyStopStep = bKernalEmergencyStopStep;
    return AHC_TRUE;
    #else
    return AHC_FALSE;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetKernalEmergStopStep
//  Description :
//------------------------------------------------------------------------------
AHC_KERNAL_EMERGENCY_STOP_STEP AHC_VIDEO_GetKernalEmergStopStep(void)
{
    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    return m_bKernalEmergencyStopStep;
    #else
    return AHC_FALSE;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetEmergRecFileName
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetEmergRecFileName(UINT8 ** filename, AHC_BOOL bFullFileName)
{
#if (EMER_RECORD_DUAL_WRITE_ENABLE)
	if (filename == NULL) {
		return AHC_FALSE;
	}
	
	if (bFullFileName) {
		*filename = (UINT8*)m_chEmerVRFullName;
	} 
	else {
		*filename = (UINT8*)m_chEmerVRFileName;
	}
	return AHC_TRUE;
#else
	return AHC_FALSE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_StartEmergRecord
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_StartEmergRecord(void)
{
    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    DCF_DB_TYPE sCurType;
    MMP_BYTE    chDirName[16];
    UINT8       bCreateNewDir;
    VIDMGR_CONTAINER_TYPE       ContainerType;
    UINT32      Param;
    AHC_BOOL    bRet;
    #if (GPS_RAW_FILE_EMER_EN == 1 || GSENSOR_RAW_FILE_ENABLE == 1)
    MMP_BYTE    bGPSRawFileName_Emer[MAX_FILE_NAME_SIZE];
    #endif
    MMP_ERR     err;
    MMP_BOOL    bDevConSts = MMP_FALSE;

    #if (GPS_RAW_FILE_EMER_EN == 1)
    GPSCtrl_SetGPSRawStart_Emer(AHC_TRUE);
    GPSCtrl_SetGPSRawWriteFirst(AHC_FALSE);
    #endif

    MEMSET(m_chEmerVRFullName, 0, sizeof(m_chEmerVRFullName));
    MEMSET(chDirName         , 0, sizeof(chDirName));
    MEMSET(m_chEmerVRFileName, 0, sizeof(m_chEmerVRFileName));

    #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
    MEMSET(m_chEmerThumbFileName, 0, sizeof(m_chEmerThumbFileName));
    MEMSET(m_chEmerThumbFullName, 0, sizeof(m_chEmerThumbFullName));
    MEMSET(m_chEmerThumbFileName_Rear, 0, sizeof(m_chEmerThumbFileName_Rear));
    MEMSET(m_chEmerThumbFullName_Rear, 0, sizeof(m_chEmerThumbFullName_Rear));
    MEMSET(m_chEmerThumbFileName_USBH, 0, sizeof(m_chEmerThumbFileName_USBH));
    MEMSET(m_chEmerThumbFullName_USBH, 0, sizeof(m_chEmerThumbFullName_USBH));
    #endif

    #if (VIDRECD_MULTI_TRACK == 0) && (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) 
    if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
        MEMSET(m_EmrVRRearFullName, 0, sizeof(m_EmrVRRearFullName));
        MEMSET(m_EmrRearVideoRFileName, 0, sizeof(m_EmrRearVideoRFileName));
    }
    if (!CAM_CHECK_USB(USB_CAM_NONE)) {
        MEMSET(m_EmrVRUSBHFullName, 0, sizeof(m_EmrVRUSBHFullName));
        MEMSET(m_EmrUSBHVideoRFileName, 0, sizeof(m_EmrUSBHVideoRFileName));
    }    
    #endif
    
    sCurType = AHC_UF_GetDB();

    AHC_UF_SelectDB(DCF_DB_TYPE_3RD_DB);

#if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_NORMAL)
    
    bRet = AHC_UF_GetName(&m_uwEmerVRDirKey, (INT8*)chDirName, (INT8*)m_chEmerVRFileName, &bCreateNewDir);
    
    if (bRet == AHC_TRUE) {
        STRCPY(m_chEmerVRFullName, (INT8*)AHC_UF_GetRootDirName());
        STRCAT(m_chEmerVRFullName, "\\");
        STRCAT(m_chEmerVRFullName, chDirName);

        if (bCreateNewDir) {
            MMPS_FS_DirCreate((INT8*)m_chEmerVRFullName, STRLEN(m_chEmerVRFullName));
            AHC_UF_AddDir(chDirName);
        }

        STRCAT(m_chEmerVRFullName, "\\");
        STRCAT(m_chEmerVRFullName, (INT8*)m_chEmerVRFileName);
    }
#elif (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
{
    MMP_ULONG   ulEmergentRecordingOffset;
    int         nSecondOffset = 0;

    AHC_RTC_GetTime(&m_EmerRecStartRtcTime);
    
    MMPS_3GPRECD_GetEmergentRecordingOffset(&ulEmergentRecordingOffset);

    #if 0
    nSecondOffset = -1*(ulEmergentRecordingOffset/1000);
    #else
    nSecondOffset = -1*(EMER_RECORD_DUAL_WRITE_PRETIME);
    #endif
    
    AHC_DT_ShiftRtc( &m_EmerRecStartRtcTime, nSecondOffset);

    AHC_VIDEO_AvoidDuplicatedFileName( &m_EmerRecStartRtcTime );
    
    bRet = AHC_UF_GetName2(&m_EmerRecStartRtcTime, (INT8*)m_chEmerVRFullName, (INT8*)m_chEmerVRFileName, &bCreateNewDir);
    
    #if (VIDRECD_MULTI_TRACK == 0) && (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) 
    if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
        AHC_UF_SetRearCamPathFlag(AHC_TRUE);
        AHC_UF_GetName2(&m_EmerRecStartRtcTime, (INT8*)m_EmrVRRearFullName, (INT8*)m_EmrRearVideoRFileName, &bCreateNewDir);
        AHC_UF_SetRearCamPathFlag(AHC_FALSE);
    }

    MMPS_USB_GetDevConnSts(&bDevConSts);
    if ((!CAM_CHECK_USB(USB_CAM_NONE)) && (bDevConSts)) {
        AHC_UF_SetRearCamPathFlag(AHC_TRUE);
        AHC_UF_GetName2(&m_EmerRecStartRtcTime, (INT8*)m_EmrVRUSBHFullName, (INT8*)m_EmrUSBHVideoRFileName, &bCreateNewDir);
        AHC_UF_SetRearCamPathFlag(AHC_FALSE);
    }
    #endif
}
#endif

    if (bRet == AHC_TRUE) {

        AIHC_VIDEO_GetMovieCfgEx(0, AHC_CLIP_CONTAINER_TYPE, &Param);
        ContainerType = Param;

        STRCAT((INT8*)m_chEmerVRFullName, EXT_DOT);
        STRCAT((INT8*)m_chEmerVRFileName, EXT_DOT);
        #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
        MEMCPY(m_chEmerThumbFullName, m_chEmerVRFullName, sizeof(m_chEmerThumbFullName));
        MEMCPY(m_chEmerThumbFileName, m_chEmerVRFileName, sizeof(m_chEmerThumbFileName));
        #endif
        
        #if (VIDRECD_MULTI_TRACK == 0) && (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) 
        if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
            STRCAT((INT8*)m_EmrVRRearFullName, EXT_DOT);
            STRCAT((INT8*)m_EmrRearVideoRFileName, EXT_DOT);
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            MEMCPY(m_chEmerThumbFullName_Rear, m_EmrVRRearFullName, sizeof(m_chEmerThumbFullName_Rear));
            MEMCPY(m_chEmerThumbFileName_Rear, m_EmrRearVideoRFileName, sizeof(m_chEmerThumbFileName_Rear));
            #endif            
        }

        if ((!CAM_CHECK_USB(USB_CAM_NONE)) && (bDevConSts)) {
            STRCAT((INT8*)m_EmrVRUSBHFullName, EXT_DOT);
            STRCAT((INT8*)m_EmrUSBHVideoRFileName, EXT_DOT);
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            MEMCPY(m_chEmerThumbFullName_USBH, m_EmrVRUSBHFullName, sizeof(m_chEmerThumbFullName_USBH));
            MEMCPY(m_chEmerThumbFileName_USBH, m_EmrUSBHVideoRFileName, sizeof(m_chEmerThumbFileName_USBH));
            #endif            
        }
        #endif
        
        #if (GPS_CONNECT_ENABLE == 1)
        if (AHC_GPS_Module_Attached())
        {
            #if (GPS_RAW_FILE_ENABLE == 1 && GPS_RAW_FILE_EMER_EN == 1) 
            MEMSET(bGPSRawFileName_Emer, 0, sizeof(bGPSRawFileName_Emer));
            STRCPY(bGPSRawFileName_Emer, m_chEmerVRFullName);
            STRCAT(bGPSRawFileName_Emer, GPS_RAW_FILE_EXTENTION);
            #endif
        }
        #endif

        #if (GSENSOR_RAW_FILE_ENABLE == 1 && GPS_CONNECT_ENABLE == 0)
        MEMSET(bGPSRawFileName_Emer, 0, sizeof(bGPSRawFileName_Emer));
        STRCPY(bGPSRawFileName_Emer, m_chEmerVRFullName);
        STRCAT(bGPSRawFileName_Emer, GPS_RAW_FILE_EXTENTION);
        #endif

        if (ContainerType == VIDMGR_CONTAINER_3GP) {
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            STRCAT((INT8*)m_chEmerThumbFullName, PHOTO_JPG_EXT);
            STRCAT((INT8*)m_chEmerThumbFileName, PHOTO_JPG_EXT);
            #endif
            STRCAT((INT8*)m_chEmerVRFullName, MOVIE_3GP_EXT);
            STRCAT((INT8*)m_chEmerVRFileName, MOVIE_3GP_EXT);
            #if (VIDRECD_MULTI_TRACK == 0) && (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) 
            if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
                #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
                STRCAT((INT8*)m_chEmerThumbFullName_Rear, PHOTO_JPG_EXT);
                STRCAT((INT8*)m_chEmerThumbFileName_Rear, PHOTO_JPG_EXT);
                #endif
                STRCAT(m_EmrVRRearFullName, MOVIE_3GP_EXT);
                STRCAT((char *)m_EmrRearVideoRFileName, MOVIE_3GP_EXT);
            }

            if ((!CAM_CHECK_USB(USB_CAM_NONE)) && (bDevConSts)) {
                #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
                STRCAT((INT8*)m_chEmerThumbFullName_USBH, PHOTO_JPG_EXT);
                STRCAT((INT8*)m_chEmerThumbFileName_USBH, PHOTO_JPG_EXT);
                #endif
                STRCAT(m_EmrVRUSBHFullName, MOVIE_3GP_EXT);
                STRCAT((char *)m_EmrUSBHVideoRFileName, MOVIE_3GP_EXT);
            }
            #endif
        }
        else {
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            STRCAT((INT8*)m_chEmerThumbFullName, PHOTO_JPG_EXT);
            STRCAT((INT8*)m_chEmerThumbFileName, PHOTO_JPG_EXT);
            #endif
            STRCAT((INT8*)m_chEmerVRFullName, MOVIE_AVI_EXT);
            STRCAT((INT8*)m_chEmerVRFileName, MOVIE_AVI_EXT);
            
            #if (VIDRECD_MULTI_TRACK == 0) && (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) 
            if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
                #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
                STRCAT((INT8*)m_chEmerThumbFullName_Rear, PHOTO_JPG_EXT);
                STRCAT((INT8*)m_chEmerThumbFileName_Rear, PHOTO_JPG_EXT);
                #endif
                STRCAT(m_EmrVRRearFullName, MOVIE_AVI_EXT);
                STRCAT((char *)m_EmrRearVideoRFileName, MOVIE_AVI_EXT);
            }

            if ((!CAM_CHECK_USB(USB_CAM_NONE)) && (bDevConSts)) {
                #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
                STRCAT((INT8*)m_chEmerThumbFullName_USBH, PHOTO_JPG_EXT);
                STRCAT((INT8*)m_chEmerThumbFileName_USBH, PHOTO_JPG_EXT);
                #endif
                STRCAT(m_EmrVRUSBHFullName, MOVIE_AVI_EXT);
                STRCAT((char *)m_EmrUSBHVideoRFileName, MOVIE_AVI_EXT);
            }
            #endif
        }

        #if (GPS_CONNECT_ENABLE == 1)
        if (AHC_GPS_Module_Attached())
        {
            #if (GPS_RAW_FILE_ENABLE == 1)
            UINT8 bGPS_en;

            if (AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) {
                switch (bGPS_en) {
                case RECODE_GPS_OFF:
                case RECODE_GPS_IN_VIDEO:
                    // NOP
                    break;
                default:
                {
                	#if (GPS_RAW_FILE_EMER_EN == 1)
                    MMP_ULONG ulEmergentRecordingOffset = 0;
                    
                    MMPS_3GPRECD_GetEmergentRecordingOffset(&ulEmergentRecordingOffset);
                    
                    GPSCtrl_SetGPSRawFileName_Emer(bGPSRawFileName_Emer, STRLEN(bGPSRawFileName_Emer));
                    #if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
                    GPSCtrl_SetGPSRawBufTime_Emer(ulEmergentRecordingOffset/1000);
                    #endif
                    #endif
                }
                    break;
                }
            }
            #endif
        }
        #endif

        #if (GSENSOR_RAW_FILE_ENABLE == 1 && GPS_CONNECT_ENABLE == 0)
        {
            MMP_ULONG ulEmergentRecordingOffset = 0;
            
            MMPS_3GPRECD_GetEmergentRecordingOffset(&ulEmergentRecordingOffset);
            
            GPSCtrl_SetGPSRawFileName_Emer(bGPSRawFileName_Emer,STRLEN(bGPSRawFileName_Emer));
            #if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
            GPSCtrl_SetGPSRawBufTime_Emer(ulEmergentRecordingOffset/1000);
            #endif
        }
        #endif

        AHC_VIDEO_SetFileName(m_chEmerVRFullName, STRLEN(m_chEmerVRFullName), VIDENC_STREAMTYPE_EMERGENCY, DCF_CAM_FRONT);
        printc("Emerg Filename=%s\r\n", m_chEmerVRFullName);
        
        #if (VIDRECD_MULTI_TRACK == 0) && (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) 
        if (MMP_IsUSBCamExist()) {
            if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_TRUE) & DUAL_REC_ENCODE_H264) {
                AHC_VIDEO_SetFileName(m_EmrVRUSBHFullName, STRLEN(m_EmrVRUSBHFullName), VIDENC_STREAMTYPE_DUALEMERG, DCF_CAM_REAR);
                printc("Dual Emerg Filename=%s\r\n", m_EmrVRUSBHFullName);
            }
            else if ((AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_TRUE) & DUAL_REC_STORE_FILE) && (bDevConSts)) {
                AHC_VIDEO_SetFileName(m_EmrVRUSBHFullName, STRLEN(m_EmrVRUSBHFullName), VIDENC_STREAMTYPE_UVCEMERG, DCF_CAM_REAR);
                printc("UVC Emerg Filename=%s\r\n", m_EmrVRUSBHFullName);
            }           
        }
        
        if (MMP_IsScdCamExist()) {
            if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_STORE_FILE) {
                AHC_VIDEO_SetFileName(m_EmrVRRearFullName, STRLEN(m_EmrVRRearFullName), VIDENC_STREAMTYPE_DUALEMERG, DCF_CAM_REAR);
            }
        }
        #endif

        AHC_UF_PreAddFile(m_uwEmerVRDirKey, (INT8*)m_chEmerVRFileName);

        m_uiEmerRecordInterval = EMER_RECORD_DUAL_WRITE_INTERVAL;
        AHC_VIDEO_SetEmergRecInterval(m_uiEmerRecordInterval);

        if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE)
            err = MMPS_3GPRECD_StartEmergentRecd(AHC_TRUE);
        else
            err = MMPS_3GPRECD_StartEmergentRecd(AHC_FALSE);    

        if (err != MMP_ERR_NONE)
        {
            PSDATETIMEDCFDB pDB = AIHC_DCFDT_GetDbByType(AHC_DCFDT_GetDB());
            
            m_bStartEmerVR = AHC_FALSE;
            m_bEmerVRPostDone = AHC_TRUE;
            
            #if (GPS_RAW_FILE_EMER_EN == 1)
            GPSCtrl_SetGPSRawStart_Emer(AHC_FALSE);
            #endif

            // Note: Below code should be also workable for Standard DCF
            AIHC_DCFDT_DeleteFileInFS( pDB, m_chEmerVRFullName, AHC_FALSE );

            if (err == MMP_3GPMGR_ERR_INVLAID_STATE)
            {
                printc(FG_RED("Failed to start Emergency record because it's not in video record mode (2)!!!\r\n"));
                // Wait a while, then trigger Emergency record again.
                AHC_OS_Sleep(300);
                AHC_VIDEO_SetEmergRecStarted(AHC_FALSE);
                AHC_SendAHLMessage(AHLM_UI_NOTIFICATION, BUTTON_LOCK_FILE_G, 0);
            }
            AHC_UF_SelectDB(sCurType);
            return AHC_FALSE;
        }
        
        m_bStartEmerVR = AHC_TRUE;
        m_bEmerVRPostDone = AHC_FALSE;
    }

    AHC_UF_SelectDB(sCurType);

    #endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetEmergRecTime
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetEmergRecTime(UINT32 *uiTime)
{
    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    MMPS_3GPRECD_GetEmergentRecordingTime(uiTime);
    #endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetEmergRecTimeOffset
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetEmergRecTimeOffset(UINT32 *uiTime)
{
    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    MMPS_3GPRECD_GetEmergentRecordingOffset(uiTime);
    #endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetEmergRecInterval
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetEmergRecInterval(UINT32 uiInterval)
{
    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    m_uiEmerRecordInterval = uiInterval;

    if (uiInterval < ((0x7FFFFFFF - EM_VR_TIME_LIMIT_OFFSET) / 1000))
        uiInterval = uiInterval * 1000 + EM_VR_TIME_LIMIT_OFFSET;

    MMPS_3GPRECD_SetEmergentFileTimeLimit(uiInterval);
    printc("Max Interval : %d ms\n", uiInterval);

    // Set Emergency file size limit = 20 MB
    //MMPS_3GPRECD_SetEmergentFileSizeLimit( 20*1024*1024 );
    #endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetEmergRecInterval
//  Description :
//------------------------------------------------------------------------------
UINT32 AHC_VIDEO_GetEmergRecInterval(void)
{
    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    return m_uiEmerRecordInterval;
    #else
    return 0;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_EmergRecPostProcess
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_EmergRecPostProcess(void)
{
    #if (GPS_RAW_FILE_EMER_EN == 1)
    GPSCtrl_GPSRawWriteFlushBuf_Emer();
    #endif
    
    #if (GPS_RAW_FILE_ENABLE == 1)
    if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE)
    {
        if (GPSCtrl_GPSRawWriteFirst_Normal())
            GPSCtrl_GPSRawResetBuffer();
    }
    #endif

    #if (GPS_CONNECT_ENABLE == 1)
    if (AHC_GPS_Module_Attached())
    {
        #if (GPS_RAW_FILE_EMER_EN == 1)
        UINT8 bGPS_en;
        
        if (AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) {
            switch (bGPS_en) {
            case RECODE_GPS_OFF:
            case RECODE_GPS_IN_VIDEO:
                // NOP
                break;
            default:
            	if (!AHC_VIDEO_IsEmergRecStarted())
                    GPSCtrl_GPSRawClose_Emer();
                break;
            }
        }
        #endif
    }
    #endif

    #if (GSENSOR_RAW_FILE_ENABLE == 1 && GPS_CONNECT_ENABLE == 0 && GPS_RAW_FILE_EMER_EN == 1)
    if (!AHC_VIDEO_IsEmergRecStarted())
        GPSCtrl_GPSRawClose_Emer();
    #endif
    
    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    if (m_bEmerVRPostDone == AHC_FALSE) {
        DCF_DB_TYPE sType;

        sType = AHC_UF_GetDB();

        AHC_UF_SelectDB(DCF_DB_TYPE_3RD_DB);

        AHC_UF_PostAddFile(m_uwEmerVRDirKey, (INT8*)m_chEmerVRFileName);
        #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
        AHC_VIDEO_StoreJpgThumbnail(VIDENC_STREAMTYPE_EMERGENCY);
        #endif
        
        #if (VIDRECD_MULTI_TRACK == 0) && (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) 
        if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
            AHC_UF_PostAddFile(m_uwEmerVRDirKey,(INT8*)m_EmrRearVideoRFileName);
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            AHC_VIDEO_StoreJpgThumbnail(VIDENC_STREAMTYPE_DUALEMERG);
            #endif
        }
        
        if (!CAM_CHECK_USB(USB_CAM_NONE) && (m_EmrUSBHVideoRFileName[0] != 0)) {
            AHC_UF_PostAddFile(m_uwEmerVRDirKey,(INT8*)m_EmrUSBHVideoRFileName);
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            AHC_VIDEO_StoreJpgThumbnail(VIDENC_STREAMTYPE_UVCEMERG);
            #endif
        }        
        #endif
        
        AHC_UF_SelectDB(sType);
        
        m_bEmerVRPostDone = AHC_TRUE;

        #if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
        {
            AHC_RTC_TIME    RtcTime;
            int             nSecondOffset;
            MMP_ULONG       ulEmergentRecordingTime;

            AHC_RTC_GetTime(&RtcTime);
            
            if ((m_EmerRecStartRtcTime.uwYear <= RTC_DEFAULT_YEAR) && (RtcTime.uwYear > RTC_DEFAULT_YEAR)) {
                
                ulEmergentRecordingTime = AHC_VIDEO_GetEmergRecInterval();
                printc("ulEmergentRecordingTime=%d\r\n", ulEmergentRecordingTime);

                nSecondOffset = -1*ulEmergentRecordingTime;
                AHC_DT_ShiftRtc(&RtcTime, nSecondOffset);

                printc("AHC_VIDEO_EmergRecPostProcess:AHC_UF_Rename::Old:%d-%d-%d:%d-%d-%d New:%d-%d-%d:%d-%d-%d \r\n",
                        m_EmerRecStartRtcTime.uwYear,
                        m_EmerRecStartRtcTime.uwMonth,
                        m_EmerRecStartRtcTime.uwDay,
                        m_EmerRecStartRtcTime.uwHour,
                        m_EmerRecStartRtcTime.uwMinute,
                        m_EmerRecStartRtcTime.uwSecond,
                        RtcTime.uwYear,RtcTime.uwMonth,RtcTime.uwDay,RtcTime.uwHour,RtcTime.uwMinute,RtcTime.uwSecond);

                AHC_UF_Rename(AHC_UF_GetDB(), &m_EmerRecStartRtcTime, &RtcTime);
            }
        }
        #endif
    }
    #endif

    #if (GPS_RAW_FILE_EMER_EN == 1)
    GPSCtrl_SetGPSRawStart_Emer(AHC_FALSE);
    GPSCtrl_SetGPSRawWriteFirst(AHC_TRUE);
    #endif

    if (AHC_PostEmergencyDone != NULL) {
	    AHC_PostEmergencyDone();
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_EmergRecPostProcessMediaError
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_EmergRecPostProcessMediaError(void)
{
    #if (GPS_RAW_FILE_EMER_EN == 1)
    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    m_bStartEmerVR = AHC_FALSE;
    #endif
    GPSCtrl_GPSRawWriteFlushBuf_Emer();
    #endif
    
    #if (GPS_RAW_FILE_ENABLE == 1)
    if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE)
    {
        if (GPSCtrl_GPSRawWriteFirst_Normal())
            GPSCtrl_GPSRawResetBuffer();
    }
    #endif

    #if (GPS_CONNECT_ENABLE == 1)
    if (AHC_GPS_Module_Attached())
    {
        #if (GPS_RAW_FILE_EMER_EN == 1)
        UINT8 bGPS_en;
        
        if (AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) {
            switch (bGPS_en) {
            case RECODE_GPS_OFF:
            case RECODE_GPS_IN_VIDEO:
                // NOP
                break;
            default:
                GPSCtrl_GPSRawClose_Emer();
                GPSCtrl_SetGPSRawStart_Emer(AHC_FALSE);
                break;
            }
        }
        #endif
    }
    #endif

    #if (GSENSOR_RAW_FILE_ENABLE == 1 && GPS_CONNECT_ENABLE == 0 && GPS_RAW_FILE_EMER_EN == 1)
    GPSCtrl_GPSRawClose_Emer();
    #endif
    
    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    if (m_bEmerVRPostDone == AHC_FALSE) {
        DCF_DB_TYPE sType;

        sType = AHC_UF_GetDB();

        AHC_UF_SelectDB(DCF_DB_TYPE_3RD_DB);

        AHC_UF_PostAddFile(m_uwEmerVRDirKey,(INT8*)m_chEmerVRFileName);
        #if (VIDRECD_MULTI_TRACK == 0) && (AHC_DUAL_EMERGRECD_SUPPORT == 1 || AHC_UVC_EMERGRECD_SUPPORT == 1) 
        if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
            AHC_UF_PostAddFile(m_uwEmerVRDirKey,(INT8*)m_EmrRearVideoRFileName);
        }
        if (!CAM_CHECK_USB(USB_CAM_NONE) && (m_EmrUSBHVideoRFileName[0] != 0)) {
            AHC_UF_PostAddFile(m_uwEmerVRDirKey,(INT8*)m_EmrUSBHVideoRFileName);
        }
        #endif
        AHC_UF_SelectDB(sType);

        m_bEmerVRPostDone = AHC_TRUE;
        m_bStartEmerVR = AHC_FALSE;

        if ((MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_DUAL_FILE) ||
            (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE))
        {
            AHC_VIDEO_SetKernalEmergStopStep(AHC_KERNAL_EMERGENCY_AHC_DONE);
        }
    }
    #endif

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_StopEmergRecord
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_StopEmergRecord(void)
{
    #if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
    MMP_ULONG ulEmergentRecordingTime;

    MMPS_3GPRECD_GetEmergentRecordingTime(&ulEmergentRecordingTime);
    #endif

    #if (EMERGENTRECD_SUPPORT)
    MMPS_3GPRECD_StopEmergentRecd(AHC_TRUE);
    #endif

    #if (EMER_RECORD_DUAL_WRITE_ENABLE)
    m_bStartEmerVR = AHC_FALSE;
    #endif

    #if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
    {
        AHC_RTC_TIME    RtcTime;
        int             nSecondOffset;

        AHC_RTC_GetTime(&RtcTime);
        
        if ((m_EmerRecStartRtcTime.uwYear <= RTC_DEFAULT_YEAR) && (RtcTime.uwYear > RTC_DEFAULT_YEAR)) {
            
            nSecondOffset = -1*(ulEmergentRecordingTime/1000);
            AHC_DT_ShiftRtc(&RtcTime, nSecondOffset);

            printc("AHC_VIDEO_StopEmergRecord:AHC_UF_Rename::Old:%d-%d-%d:%d-%d-%d New:%d-%d-%d:%d-%d-%d \r\n",
                    m_EmerRecStartRtcTime.uwYear,
                    m_EmerRecStartRtcTime.uwMonth,
                    m_EmerRecStartRtcTime.uwDay,
                    m_EmerRecStartRtcTime.uwHour,
                    m_EmerRecStartRtcTime.uwMinute,
                    m_EmerRecStartRtcTime.uwSecond,
                    RtcTime.uwYear,RtcTime.uwMonth,RtcTime.uwDay,RtcTime.uwHour,RtcTime.uwMinute,RtcTime.uwSecond);

            AHC_UF_Rename(AHC_UF_GetDB(), &m_EmerRecStartRtcTime, &RtcTime);
        }
    }
    #endif

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetNormal2Emergency
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_SetNormal2Emergency(AHC_BOOL bState)
{
    #if (AHC_EMERGENTRECD_SUPPORT)
    m_bNormal2Emergency = bState;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetNormal2Emergency
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetNormal2Emergency(void)
{
    #if (AHC_EMERGENTRECD_SUPPORT)
    return m_bNormal2Emergency;
    #endif

    return AHC_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetDelNormFileAfterEmerg
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_SetDelNormFileAfterEmerg(UINT8 bEnable)
{
	m_bDelNormRecAfterEmr = bEnable;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_NeedDeleteNormalAfterEmerg
//  Description :
//------------------------------------------------------------------------------
UINT8 AHC_VIDEO_NeedDeleteNormalAfterEmerg(void)
{
	return m_bDelNormRecAfterEmr;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_DeleteLatestNormalFile
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_DeleteLatestNormalFile(void)
{
	UINT32 ulIndex = 0;
	
	AHC_UF_FileOperation_ByIdx(ulIndex, DCF_FILE_DELETE_ALL_CAM, NULL, NULL);
    AHC_VIDEO_SetDelNormFileAfterEmerg(0);
}

#if 0
void _____VR_ShareRec_Function_________(){ruturn;} //dummy
#endif

#if (AHC_SHAREENC_SUPPORT)
//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_IsSharePostDone
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_IsSharePostDone(void)
{
#if (AHC_SHAREENC_SUPPORT)
    return m_bShareVRPostDone;
#else
    return AHC_FALSE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetSharePostDone
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetSharePostDone(AHC_BOOL bDone)
{
#if (AHC_SHAREENC_SUPPORT)
    m_bShareVRPostDone = bDone;
    return AHC_TRUE;
#else
    return AHC_FALSE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetShareRecdRes
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetShareRecdRes(MMP_USHORT ResIdx)
{
    #if (AHC_SHAREENC_SUPPORT)
    m_usShareRecResolIdx = ResIdx;
    return AHC_TRUE;
    #endif
    return AHC_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_IsShareRecStarted
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_IsShareRecStarted(void)
{
    #if (AHC_SHAREENC_SUPPORT)
    return m_bStartShareRec;
    #else
    return AHC_FALSE;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetShareRecStarted
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetShareRecStarted(AHC_BOOL bDualRecordStarted)
{
    #if (AHC_SHAREENC_SUPPORT)
    m_bStartShareRec = bDualRecordStarted;
    return AHC_TRUE;
    #endif
    return AHC_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetShareRecStarted
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetShareRecFileName(UINT8 * filename, AHC_BOOL bFullFileName)
{
#if (AHC_SHAREENC_SUPPORT)
	char *pFileName;
	
	if (filename == NULL) {
		return AHC_FALSE;
	}
	
	if (bFullFileName) {
		pFileName = m_ShareRecFullFileName;
	} 
	else {
		pFileName = (char*)m_ShareRecFileName;
	}
	strncpy(pFileName, (char*)filename, MAX_FILE_NAME_SIZE);
	
	return AHC_TRUE;
#else
	return AHC_FALSE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetShareRecFileName
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetShareRecFileName(UINT8 ** filename, AHC_BOOL bFullFileName)
{
	if (filename == NULL) {
		return AHC_FALSE;
	}
	
	if (bFullFileName) {
		*filename = (UINT8*)m_ShareRecFullFileName;
	} 
	else {
		*filename = m_ShareRecFileName;
	}
	
	if (filename[0] == '\0') {
		return AHC_FALSE;
	}
	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_StartShareRecord
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_StartShareRecord(void)
{
    #if (AHC_SHAREENC_SUPPORT)    
    MMP_ERR		        err;
    MMP_ULONG           ulShareRecordingOffset;
    MMP_BYTE            bShareFullName[MAX_FILE_NAME_SIZE];
    MMP_BYTE            byOldFilename[MAX_FILE_NAME_SIZE];
    AHC_FS_ATTRIBUTE    sAttrib;
    AHC_RTC_TIME        RtcTime;
    UINT8               bCreateNewDir;
    DCF_DB_TYPE         sType = AHC_UF_GetDB();   
    AHC_BOOL            status;
    int                 nSecondOffset = 0;

    m_bStartShareRec = AHC_TRUE;
    nSecondOffset = -1*(EMER_RECORD_DUAL_WRITE_PRETIME);

    if (m_bFirstShareFile == AHC_FALSE)
    {
        AHC_UF_SelectDB(DCF_DB_FORMAT_FREE_DB);
        
        AHC_RTC_GetTime(&RtcTime);
        AHC_DT_ShiftRtc(&RtcTime, nSecondOffset);
        
        MEMSET(m_ShareRecFileName, 0, sizeof(m_ShareRecFileName));
        AHC_UF_GetName2(&RtcTime, (INT8*)bShareFullName, (INT8*)m_ShareRecFileName, &bCreateNewDir);
        
        STRCAT(bShareFullName, EXT_DOT);        
        STRCAT((INT8*)bShareFullName, MOVIE_3GP_EXT);                    
        STRCAT((INT8*)m_ShareRecFileName, EXT_DOT);        
        STRCAT((INT8*)m_ShareRecFileName, MOVIE_3GP_EXT);                            
        printc("\r\nShare FileName %s ,%d\r\n",bShareFullName,__LINE__);

        status = AHC_UF_SearchAvailableFileSlot(DCF_CAM_FRONT, byOldFilename);
        
        if (status == AHC_FALSE) {
            AHC_VIDEO_SetShareRecStarted(AHC_FALSE);
            AHC_UF_SelectDB(sType);
			return AHC_FALSE;
		}
		
        MMPS_FS_FileDirRename(byOldFilename, strlen(byOldFilename), bShareFullName, strlen(bShareFullName));

        if (!AHC_UF_PreAddFile(0, (INT8*)m_ShareRecFileName)) {
            printc("PreAdd failed\r\n");
        }
        
        AHC_FS_FileDirGetAttribute(bShareFullName, strlen(bShareFullName), &sAttrib);
        
        if (sAttrib & AHC_FS_ATTR_HIDDEN) {
            sAttrib &= ~AHC_FS_ATTR_HIDDEN;
            AHC_FS_FileDirSetAttribute(bShareFullName, strlen(bShareFullName), sAttrib);
        }
        
        AHC_UF_SelectDB(sType); 
        
        AHC_VIDEO_SetFileName(bShareFullName, STRLEN(bShareFullName), VIDENC_STREAMTYPE_DUALENC, DCF_CAM_FRONT);
    }
    else {
        m_bFirstShareFile = AHC_FALSE;
    }
    
    MMPS_3GPRECD_GetShareRecordingOffset(&ulShareRecordingOffset);

    AHC_VIDEO_SetShareRecInterval(DUAL_RECORD_WRITE_INTERVAL);        

    err = MMPS_3GPRECD_StartDualH264();

	if (err != MMP_ERR_NONE) {
		AHC_VIDEO_SetShareRecStarted(AHC_FALSE);
		return AHC_FALSE;
	}

	(void)AHC_VIDEO_SetShareRecFileName((UINT8*)bShareFullName, AHC_TRUE);
	
    m_bShareRecPostDone = AHC_FALSE;
    #endif
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetShareRecTimeOffset
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetShareRecTimeOffset(UINT32 *uiTime)
{
    #if (AHC_SHAREENC_SUPPORT)
    MMPS_3GPRECD_GetShareRecordingOffset(uiTime);
    #endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetShareRecInterval
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetShareRecInterval(UINT32 uiInterval)
{
    #if (AHC_SHAREENC_SUPPORT)
    m_uiShareRecInterval = uiInterval;

    if (uiInterval < ((0x7FFFFFFF - EM_VR_TIME_LIMIT_OFFSET) / 1000))
        uiInterval = uiInterval * 1000 + EM_VR_TIME_LIMIT_OFFSET;

    MMPS_3GPRECD_SetShareFileTimeLimit(uiInterval);
    printc("Max Dual Interval : %d ms\n", uiInterval);
    #endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_ShareRecPostProcess
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_ShareRecPostProcess(void)
{
    #if (AHC_SHAREENC_SUPPORT)
    if (m_bShareRecPostDone == AHC_FALSE){
        printc("Post Dual Done\r\n");
        m_bShareRecPostDone = AHC_TRUE;
    }

    if (AHC_PostDualRecdDone != NULL) {
	    AHC_PostDualRecdDone();
    }
    #endif

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_ShareRecPostProcessMediaError
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_ShareRecPostProcessMediaError(void)
{  
#if (AHC_SHAREENC_SUPPORT)
    if (m_bShareRecPostDone == AHC_FALSE) {
        printc("Post Dual Done\r\n");
        m_bShareRecPostDone = AHC_TRUE;
        m_bStartShareRec = AHC_FALSE;        
    }

    return AHC_TRUE;
#endif
    return AHC_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_StopShareRecord
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_StopShareRecord(void)
{
#if (AHC_SHAREENC_SUPPORT)
    MMP_ULONG ulDualRecordingTime;

    MMPS_3GPRECD_GetShareRecordingTime(&ulDualRecordingTime);	
    
    MMPS_3GPRECD_StopDualH264();   

	printc("Dual rec time = %d\r\n", ulDualRecordingTime);

    AHC_VIDEO_SetShareRecStarted(AHC_FALSE);
#endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetSharePreEncTimeLimit
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetSharePreEncTimeLimit(MMP_ULONG ulDurationMs)
{    
    #if (AHC_SHAREENC_SUPPORT)   
    MMP_ULONG PreLimitMs = DUAL_RECORD_WRITE_PRETIME_LIMIT*1000 + EM_VR_TIME_LIMIT_OFFSET;
    
    if (ulDurationMs > PreLimitMs)
        return MMPS_3GPRECD_SetSharePreEncTimeLimit(PreLimitMs);
    else
        return MMPS_3GPRECD_SetSharePreEncTimeLimit(ulDurationMs);
    #endif
    return AHC_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetSharePreEncTimeLimit
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetSharePreEncTimeLimit(MMP_ULONG *ulDurationMs)
{
    #if (AHC_SHAREENC_SUPPORT)
    MMPS_3GPRECD_GetSharePreEncTimeLimit(ulDurationMs);
    return AHC_TRUE;
    #endif
    return AHC_FALSE;
}
#endif

#if 0
void _____VR_Audio_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecVolumeParam
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecVolumeParam(UINT8 ubDgain, UINT8 ubAGain)
{
    m_ubRecordDGain = ubDgain;
    m_ubRecordAGain = ubAGain;

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecVolumeParam
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetRecVolumeParam(UINT8* pubDgain, UINT8* pubAGain)
{
    *pubDgain = m_ubRecordDGain;
    *pubAGain = m_ubRecordAGain;

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecVolumeToFW
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecVolumeToFW(AHC_BOOL bEnable)
{
    if (bEnable) {
        printc("D gain : 0x%x\r\n", m_ubRecordDGain);
        printc("A gain : 0x%x\r\n", m_ubRecordAGain);

        MMPS_AUDIO_SetRecordVolume(m_ubRecordDGain, m_ubRecordAGain);
    }
    else {
        printc("AHC_VIDEO_SetRecVolumeToFW - Mute !!!\r\n");
        MMPS_AUDIO_SetRecordVolume(0, 0);
    }
    return MMP_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecVolumeByMenuSetting
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecVolumeByMenuSetting(AHC_BOOL bEnable)
{
    UINT32 u32DGain = 0;
    UINT32 u32AGain = 0;

    if (bEnable) {
    	AHC_GetParam(PARAM_ID_AUDIO_IN_DIGITAL_GAIN, &u32DGain);
    	AHC_GetParam(PARAM_ID_AUDIO_IN_GAIN, &u32AGain);
    }

    AHC_VIDEO_SetRecVolumeParam((UINT8)u32DGain, (UINT8)u32AGain);
    
    MMPS_AUDIO_SetRecordVolume((UINT8)u32DGain, (UINT8)u32AGain);

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecVolume
//  Description :
//------------------------------------------------------------------------------
/**
Use Uart command "rg <D gain> <A gain> <Booster on>" to adjust the gain while video record.

The default values are 0x3F, 0xFF and 1 for the three parameters.

The definition of D gain value:
6・b 11 1111~ 6・b 10 0000: 0.5dB per step (0dB~ -15.5dB)
6・b 01 1111~ 6・b 00 0000: 1 dB per step (-16dB~-46dB)

The definition of A gain value:
4 high bits are for Left channel, 4 low bits are for Right channel.
0000~1111: 2dB per step (-10dB~20dB)

e.x.
UART>rg 0x3f 0xff 0
Means set D gain 0x3f(0dB), A gain 0xff(20dB for each channel).
*/
AHC_BOOL AHC_VIDEO_GetRecVolume(ULONG* piDGain, ULONG* piLAGain, ULONG* piRAGain)
{
    ULONG LAGain;
    ULONG RAGain;
    ULONG DGain;

    /*
    6・b 11 1111~ 6・b 10 0000: 0.5dB per step (0dB~ -15.5dB)
    */
    #define DGAIN_BITSIGN       (0x20)
    #define DGAIN_STEP_DEFAULT  (0x3F)  ///< 0b11 1111
    #define DGAIN_STEP_BASE     (0x20)  ///< 0b10 0000
    #define DGAIN_DELTA         (0.5)   ///< 0.5dB
    #define DGAIN_BASE          (-15.5) ///< -15.5dB

    /*
    6・b 01 1111~ 6・b 00 0000: 1 dB per step (-16dB~-46dB)
    */
    #define DGAIN_STEP_DEFAULT2  (0x1F)  ///< 0b01 1111
    #define DGAIN_STEP_BASE2     (0x00)  ///< 0b00 0000
    #define DGAIN_DELTA2         (1)     ///< 1dB
    #define DGAIN_BASE2          (-46)   ///< -46dB

    if ((m_ubRecordDGain & DGAIN_BITSIGN) != 0) {
        printc("DGain : 0dB ~ -15.5dB \n");
        DGain = DGAIN_BASE + (m_ubRecordDGain - DGAIN_STEP_BASE) * DGAIN_DELTA;
    }
    else {
        printc("DGain : -16dB ~ -46dB \n");
        DGain = DGAIN_BASE2 + (m_ubRecordDGain - DGAIN_STEP_BASE2) * DGAIN_DELTA2;
    }

    printc("DGain : %d dB \n", DGain);

    /*
    The definition of A gain value:
    4 high bits are for Left channel, 4 low bits are for Right channel.
    0000~1111: 2dB per step (-10dB~20dB)
    */
    #define AGAIN_LMASK(a)      ((a >> 4) & 0xF)
    #define AGAIN_RMASK(a)      (a & 0xF)

    #define AGAIN_STEP_DEFAULT  (0xF)   ///< 0b01 1111
    #define AGAIN_STEP_BASE     (0x0)   ///< 0b00 0000
    #define AGAIN_DELTA         (2)     ///< 1dB
    #define AGAIN_BASE          (-10)   ///< -16dB

    LAGain = AGAIN_BASE + (AGAIN_LMASK(m_ubRecordAGain)  - AGAIN_STEP_BASE) * AGAIN_DELTA;
    RAGain = AGAIN_BASE + (AGAIN_RMASK(m_ubRecordAGain)  - AGAIN_STEP_BASE) * AGAIN_DELTA;

    printc("AGain : -10dB ~ 20dB \n");
    printc("LAGain : %d dB \n", LAGain);
    printc("RAGain : %d dB \n", RAGain);

    *piDGain    = DGain;
    *piLAGain   = LAGain;
    *piRAGain   = RAGain;
    
    return MMP_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetAudioSamplingRate
//  Description :
//------------------------------------------------------------------------------
/** @brief Get the audio sampling rate
   Typically this is for UI or network so that they could know the setting of this build.
*/
UINT32 AHC_VIDEO_GetAudioSamplingRate(UINT8 uiAudioType)
{
    switch(uiAudioType)
    {
    case VR_AUDIO_TYPE_AAC:
        if ((AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_16K_32K) ||
            (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_16K_64K))
            return 16000;
        else if ((AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_22d05K_64K) ||
                 (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_22d05K_128K))
            return 22050;
        else if ((AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_32K_64K) ||
                 (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_32K_128K))
            return 32000;
        else if (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_48K_128K)
            return 48000;
        else if ((AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_44d1K_64K) ||
                 (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_44d1K_128K))
            return 44100;
        else
        {
            printc(FG_RED("!!! WARNING !!! Un-considered AAC option (1)\r\n"));
            return 32000;
        }
    case VR_AUDIO_TYPE_MP3:
        if (AHC_MP3_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_MP3_32K_128K)
            return 32000;
        else if (AHC_MP3_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_MP3_44d1K_128K)
            return 44100;
        else
        {
            printc(FG_RED("!!! WARNING !!! Un-considered MP3 option (1)\r\n"));
            return 32000;
        }
    case VR_AUDIO_TYPE_ADPCM:
        if (AHC_ADPCM_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_ADPCM_16K_22K)
            return 16000;
        else if (AHC_ADPCM_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_ADPCM_32K_22K)
            return 32000;
        else if (AHC_ADPCM_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_ADPCM_44d1K_22K)
            return 44100;
        else
        {
            printc(FG_RED("!!! WARNING !!! Un-considered ADPCM option  (1)\r\n"));
            return 32000;
        }
    case VR_AUDIO_TYPE_AMR:
        if (AHC_AMR_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AMR_4d75K)
            return 4750;
        else if (AHC_AMR_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AMR_5d15K)
            return 5150;
        else if (AHC_AMR_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AMR_12d2K)
            return 12200;
        else
        {
            printc(FG_RED("!!! WARNING !!! Un-considered AMR option  (1)\r\n"));
            return 32000;
        }
    case VR_AUDIO_TYPE_PCM:
        if ( AHC_PCM_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_PCM_32K)
            return 32000;
        else if ( AHC_PCM_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_PCM_16K)
            return 16000;
        else
        {
            printc(FG_RED("!!! WARNING !!! Un-considered PCM option  (1)\r\n"));
            return 32000;
        }
    default:
        printc(FG_RED("!!! WARNING !!! Un-supported audio type: %d  (1)\r\n"), uiAudioType);
        return 32000;
    }
    
    return 32000;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetAudioBitRate
//  Description :
//------------------------------------------------------------------------------
/** @brief Get the audio bit rate
   Typically this is for UI or network so that they could know the setting of this build.
*/
UINT32 AHC_VIDEO_GetAudioBitRate( UINT8 uiAudioType )
{
    switch(uiAudioType)
    {
    case VR_AUDIO_TYPE_AAC:
        if (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_16K_32K)
            return 32000;
        else if ((AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_16K_64K)       ||
                 (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_22d05K_64K)    ||
                 (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_32K_64K)       ||
                 (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_44d1K_64K))
            return 64000;
        else if ((AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_22d05K_128K) ||
                 (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_32K_128K)    ||
                 (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_48K_128K)    ||
                 (AHC_AAC_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AAC_44d1K_128K))
            return 128000;
        else
        {
            printc(FG_RED("!!! WARNING !!! Un-considered AAC option (2)\r\n"));
            return 128000;
        }
    case VR_AUDIO_TYPE_MP3:
        if ((AHC_MP3_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_MP3_32K_128K) ||
            (AHC_MP3_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_MP3_44d1K_128K))
            return 128000;
        else
        {
            printc(FG_RED("!!! WARNING !!! Un-considered MP3 option (2)\r\n"));
            return 128000;
        }
    case VR_AUDIO_TYPE_ADPCM:
        if ((AHC_ADPCM_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_ADPCM_16K_22K) ||
            (AHC_ADPCM_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_ADPCM_32K_22K) ||
            (AHC_ADPCM_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_ADPCM_44d1K_22K))
            return 22000;
        else
        {
            printc(FG_RED("!!! WARNING !!! Un-considered ADPCM option (2)\r\n"));
            return 22000;
        }
    case VR_AUDIO_TYPE_PCM:
        if ((AHC_PCM_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_PCM_16K) ||
            (AHC_PCM_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_PCM_32K))
            return 22000;
        else
        {
            printc(FG_RED("!!! WARNING !!! Un-considered PCM option (2)\r\n"));
            return 22000;
        }
    case VR_AUDIO_TYPE_AMR:
        if ((AHC_AMR_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AMR_4d75K) ||
            (AHC_AMR_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AMR_5d15K) ||
            (AHC_AMR_AUDIO_OPTION == MMPS_3GPRECD_AUDIO_AMR_12d2K))
            return 8000;
        else
        {
            printc(FG_RED("!!! WARNING !!! Un-considered AMR option (2)\r\n"));
            return 8000;
        }
    default:
        printc(FG_RED("!!! WARNING !!! Un-supported audio type: %d (2)\r\n"), uiAudioType);
        return 128000;
    }
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetDefaultAudioSamplingRate
//  Description :
//------------------------------------------------------------------------------
/** @brief Get the default audio sampling rate
   Typically this is for UI or network so that they could know the default setting of this build.
   Example usage: RTSP default SDP for audio sampling rate. At that moment the audio parameters are not set.
*/
UINT32 AHC_VIDEO_GetDefaultAudioSamplingRate(void)
{
    return AHC_VIDEO_GetAudioSamplingRate(VR_AUDIO_TYPE);
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_ConfigAudio
//  Description :
//------------------------------------------------------------------------------
/** @brief Config audio stream

  @note Audio bitrate is defined in something like AHC_AAC_AUDIO_OPTION and AHC_MP3_AUDIO_OPTION
        so sampling rate and bit rate will be ignored. Those fields are for back up status only.

  @param stream[in] The audio stream ID
  @param aFormat[in] audio format
  @param channelConfig channel configuration
*/
AHC_BOOL AHC_VIDEO_ConfigAudio( UINT16                     stream, 
                                AHC_AUDIO_FORMAT           aFormat,
                                AHC_AUDIO_CHANNEL_CONFIG   channelConfig)
{
    AHC_BOOL    bMute;
    INT32       iSoundRecord;
    MMP_ULONG   bitRate, samplingRate;
    MMPS_3GPRECD_AUDIO_FORMAT format;

    // These 2 keeps the value of the setting for quicker reference only. The real settings
    // automatically applied in MMPS
    switch (aFormat) {
    case AHC_MOVIE_AUDIO_CODEC_AAC:
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_BITRATE, AHC_VIDEO_GetAudioBitRate(VR_AUDIO_TYPE_AAC));
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_SAMPLE_RATE, AHC_VIDEO_GetAudioSamplingRate(VR_AUDIO_TYPE_AAC));
        break;
    case AHC_MOVIE_AUDIO_CODEC_MP3:
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_BITRATE, AHC_VIDEO_GetAudioBitRate(VR_AUDIO_TYPE_MP3));
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_SAMPLE_RATE, AHC_VIDEO_GetAudioSamplingRate(VR_AUDIO_TYPE_MP3));
        break;
    case AHC_MOVIE_AUDIO_CODEC_ADPCM:
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_BITRATE, AHC_VIDEO_GetAudioBitRate(VR_AUDIO_TYPE_ADPCM));
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_SAMPLE_RATE, AHC_VIDEO_GetAudioSamplingRate(VR_AUDIO_TYPE_ADPCM));
        break;
    case AHC_MOVIE_AUDIO_CODEC_G711:
        (void)AHC_Audio_ParseOptions(AHC_AUDIO_OPTION, &format, &samplingRate, &bitRate);
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_BITRATE, 8000);//TBD
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_SAMPLE_RATE, samplingRate);
        break;
    case AHC_MOVIE_AUDIO_CODEC_PCM:
        #if 0
        (void)AHC_Audio_ParseOptions(AHC_AUDIO_OPTION, &format, &samplingRate, &bitRate);
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_BITRATE, samplingRate << 4); // mono, 16-bit per sample
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_SAMPLE_RATE, samplingRate);
        #else
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_BITRATE, AHC_VIDEO_GetAudioBitRate(VR_AUDIO_TYPE_PCM));
        AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_SAMPLE_RATE, AHC_VIDEO_GetAudioSamplingRate(VR_AUDIO_TYPE_PCM));
        #endif
    	break;
    default:
        return AHC_FALSE;
        break;
    }
    
    if (MMP_FALSE == AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_CODEC_TYPE, aFormat)) {
        return MMP_FALSE;
    }
    
    AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_CHANNEL_CONFIG, channelConfig);
    
    if (AHC_Menu_SettingGetCB(COMMON_KEY_RECD_SOUND, &iSoundRecord) && iSoundRecord == MOVIE_SOUND_RECORD_ON) {
        bMute = AHC_FALSE;
    } 
    else {
        if (MMPS_3GPRECD_GetAVSyncMethod() == VIDMGR_AVSYNC_REF_AUD)
        {
            bMute = AHC_FALSE;
        }
        else
            bMute = AHC_TRUE;
    }
    
    AHC_VIDEO_SetMovieConfig(stream, AHC_AUD_MUTE_END, bMute);
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecWithWNR
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_SetRecWithWNR(AHC_BOOL bEnable)
{
#if (WNR_EN)
    if (bEnable) {
       MMPS_AUDIO_EnableWNR();
    }
    else {
       MMPS_AUDIO_DisableWNR();
    }
#endif    
}

#if 0
void _____VR_NormalRec_Function_________(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeAudioOn
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_SetRecordModeAudioOn(UINT8 bAudioOn)
{
    gbAudioOn = bAudioOn;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecordModeAudioOn
//  Description : 
//------------------------------------------------------------------------------
UINT8 AHC_VIDEO_GetRecordModeAudioOn(void)
{
    return gbAudioOn;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeDeleteFile
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_SetRecordModeDeleteFile(AHC_BOOL bDeleteFile)
{
    gbDeleteFile = bDeleteFile;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecordModeDeleteFile
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetRecordModeDeleteFile(void)
{
    return gbDeleteFile;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeFirstTimeRecord
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_SetRecordModeFirstTimeRecord(AHC_BOOL bFirstRecordSetFile)
{
    gbFirstRecordSetFile = bFirstRecordSetFile;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecordModeFirstTimeRecord
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_GetRecordModeFirstTimeRecord(void)
{
    return gbFirstRecordSetFile;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecordModeClearException
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_GetRecordModeClearException(void)
{
    gulVidRecdExceptionCode = AHC_VIDRECD_MODE_API_MAX;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecordModeThrow
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_GetRecordModeThrow(UINT32 ulExceptionCode)
{
    gulVidRecdExceptionCode = ulExceptionCode;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetRecordModeCatch
//  Description : 
//------------------------------------------------------------------------------
UINT32 AHC_VIDEO_GetRecordModeCatch(void)
{
    return gulVidRecdExceptionCode;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_ClosePreviousFile
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_ClosePreviousFile(void)
{
    AHC_FS_ATTRIBUTE    attribute;
    AHC_FS_FILETIME     timestructure;
    UINT32              ulpFileSize;

    AHC_UF_PostAddFile(gusCurVideoDirKey, (INT8*)gpbCurVideoFileName);

    #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
    RTNA_DBG_Str0(FG_YELLOW("AHC_VIDEO_ClosePreviousFile!\r\n"));
    AHC_VIDEO_StoreJpgThumbnail(VIDENC_STREAMTYPE_VIDRECD);
    #endif

    #if (VIDRECD_MULTI_TRACK == 0)
    if (!CAM_CHECK_SCD(SCD_CAM_NONE))
    {
        if (AHC_FS_FileDirGetInfo((INT8*)m_CurVRRearFullName, 
                                  STRLEN((INT8*)m_CurVRRearFullName), 
                                  &attribute, 
                                  &timestructure, 
                                  &ulpFileSize) == AHC_ERR_NONE) {
                                  
            AHC_UF_PostAddFile(gusCurVideoDirKey, (INT8*)m_RearVideoRFileName);
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            AHC_VIDEO_StoreJpgThumbnail(VIDENC_STREAMTYPE_UVCRECD); // CHECK
            #endif
        }
    }
    
    if (!CAM_CHECK_USB(USB_CAM_NONE) && (m_CurVRUSBHFullName[0] != 0))
    {
        if (AHC_FS_FileDirGetInfo((INT8*)m_CurVRUSBHFullName, 
                                  STRLEN((INT8*)m_CurVRUSBHFullName), 
                                  &attribute, 
                                  &timestructure, 
                                  &ulpFileSize) == AHC_ERR_NONE) {
                                  
            AHC_UF_PostAddFile(gusCurVideoDirKey, (INT8*)m_USBHVideoRFileName);
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            AHC_VIDEO_StoreJpgThumbnail(VIDENC_STREAMTYPE_UVCRECD);
            #endif
        }
    }    
    #endif

    if (AHC_UF_GetDB() == DCF_DB_TYPE_1ST_DB && AHC_VIDEO_NeedDeleteNormalAfterEmerg()) {
        AHC_VIDEO_DeleteLatestNormalFile();
    }
    
#if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
{
    AHC_RTC_TIME    RtcTime;
    int             nSecondOffset;

    AHC_RTC_GetTime(&RtcTime);
    
    if ((m_VideoRecStartRtcTime.uwYear <= RTC_DEFAULT_YEAR) && (RtcTime.uwYear > RTC_DEFAULT_YEAR)) {
        
        nSecondOffset = -1*AHC_VIDEO_GetRecTimeLimit();
        
        AHC_DT_ShiftRtc(&RtcTime, nSecondOffset);

        printc("AHC_VIDEO_RestartRecMode:AHC_UF_Rename::Old:%d-%d-%d:%d-%d-%d New:%d-%d-%d:%d-%d-%d \r\n",
                m_VideoRecStartRtcTime.uwYear,
                m_VideoRecStartRtcTime.uwMonth,
                m_VideoRecStartRtcTime.uwDay,
                m_VideoRecStartRtcTime.uwHour,
                m_VideoRecStartRtcTime.uwMinute,
                m_VideoRecStartRtcTime.uwSecond,
                RtcTime.uwYear,
                RtcTime.uwMonth,
                RtcTime.uwDay,
                RtcTime.uwHour,
                RtcTime.uwMinute,
                RtcTime.uwSecond);

        AHC_UF_Rename(AHC_UF_GetDB(), &m_VideoRecStartRtcTime, &RtcTime);
        
        m_VideoRecStartRtcTime = RtcTime;
    }
}
#endif

#if (AHC_EMERGENTRECD_SUPPORT)
    if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_MOVE_FILE)
    {
        if (AHC_VIDEO_GetNormal2Emergency()) {
        
            #if 0
            UINT32          CurrentObjIdx;
            #else
            UINT8           bCreateNewDir;
            UINT32          Param;
            VIDMGR_CONTAINER_TYPE      ContainerType;
            #endif
            UINT16          DirKey;
            char            FilePathName[MAX_FILE_NAME_SIZE];
            INT8            DirName[32];
            INT8            FileName[32];

            AHC_VIDEO_SetNormal2Emergency(AHC_FALSE);
            //LedCtrl_ForceCloseTimer(AHC_FALSE);

            #if 0
            AHC_UF_GetCurrentIndex(&CurrentObjIdx);
            MEMSET(FilePathName, 0, sizeof(FilePathName));
            AHC_UF_GetFilePathNamebyIndex(CurrentObjIdx, FilePathName);
            AHC_UF_GetDirKeybyIndex(CurrentObjIdx, &DirKey);
            #else
            MEMSET(FilePathName, 0, sizeof(FilePathName));
            MEMSET(FileName, 0, sizeof(FileName));
            
            AHC_UF_GetName2(&m_VideoRecStartRtcTime, FilePathName, FileName, &bCreateNewDir);
            
            AIHC_VIDEO_GetMovieCfgEx(0, AHC_CLIP_CONTAINER_TYPE, &Param);
            ContainerType = Param;

            STRCAT(FilePathName, EXT_DOT);
            if (ContainerType == VIDMGR_CONTAINER_3GP) {
                STRCAT(FilePathName, MOVIE_3GP_EXT);
            }
            else {
                STRCAT(FilePathName, MOVIE_AVI_EXT);
            }
            #endif

            printc("Normal2Emer File: %s \n", FilePathName);

            MEMSET(DirName, 0, sizeof(DirName));
            MEMSET(FileName, 0, sizeof(FileName));

            MEMCPY(DirName, FilePathName, 10);       // SD:\Normal
            GetPathFileNameStr(FileName, sizeof(FileName), FilePathName);

            AHC_UF_MoveFile(DCF_DB_TYPE_1ST_DB, DCF_DB_TYPE_3RD_DB, DirKey, FileName, AHC_TRUE);
        }
    }
#endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_DelPreviousFile
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_DelPreviousFile(void)
{
#if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
    VIDMGR_CONTAINER_TYPE ContainerType;
    UINT32  Param;
    UINT8   bCreateNewDir;
    INT8    DirName[32];
    INT8    FileName[32];

    AHC_UF_GetName2(&m_VideoRecStartRtcTime, (INT8*)m_CurVRFullName, (INT8*)gpbCurVideoFileName, &bCreateNewDir);

    MEMSET(DirName, 0, sizeof(DirName));
    MEMSET(FileName, 0, sizeof(FileName));

    AIHC_VIDEO_GetMovieCfgEx(0, AHC_CLIP_CONTAINER_TYPE, &Param);
    ContainerType = Param;

    if (ContainerType == VIDMGR_CONTAINER_3GP) {
        STRCAT((INT8*)gpbCurVideoFileName, EXT_DOT);
        STRCAT((INT8*)gpbCurVideoFileName, MOVIE_3GP_EXT);
    }
    else {
        STRCAT((INT8*)gpbCurVideoFileName, EXT_DOT);
        STRCAT((INT8*)gpbCurVideoFileName, MOVIE_AVI_EXT);
    }

    GetPathDirStr(DirName, sizeof(DirName), m_CurVRFullName);

    memcpy(FileName, gpbCurVideoFileName, sizeof(FileName) - 1);    

    printc("AHC_VIDEO_DelPreviousFile\r\n");
    printc("DirName: %s \n", DirName);
    printc("FileName: %s \n", FileName);
    printc("FileName2: %s \n", gpbCurVideoFileName);
  
    AHC_UF_FileOperation((UINT8*)DirName, (UINT8*)FileName, DCF_FILE_DELETE_ALL_CAM, NULL, NULL);

#endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_CyclicDeleteFiles
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_CyclicDeleteFiles(void)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    AHC_BOOL b_delete = 1;

    b_delete = AHC_VIDEO_GetRecordModeDeleteFile();
    
    /* Protect File Flow */
#ifdef CFG_CUS_VIDEO_PROTECT_PROC
    CFG_CUS_VIDEO_PROTECT_PROC();
#else
    if (AHC_Protect_GetType() != AHC_PROTECT_NONE)
    {
        AHC_Protect_SetVRFile(AHC_PROTECTION_CUR_FILE, AHC_Protect_GetType());
        AHC_Protect_SetType(AHC_PROTECT_NONE);
    }
#endif

    #if (FS_FORMAT_FREE_ENABLE)
    b_delete = AHC_FALSE;
    #endif
    
    /* Delete File Flow */
    if (b_delete){
        if (AHC_Deletion_Romove(AHC_FALSE) == AHC_FALSE) {
            printc(FG_RED("AHC_Deletion_Romove Error\r\n"));
            return AHC_FALSE;
        }
    }
    
    return ahcRet;    
}    

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeInitParameters
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeInitParameters(void)
{
    UINT32                  EncWidth, EncHeight, i, ulMinDiff = 0;
    MMPS_3GPRECD_PRESET_CFG *VideoConfig = MMPS_3GPRECD_GetConfig();
    MMP_USHORT              usResolIdx;        
    MMP_ERR                 sRet = MMP_ERR_NONE;
    AHC_BOOL                ahcRet = AHC_TRUE;

    #if (AHC_ENABLE_VIDEO_STICKER == 0)
    AHC_ConfigCapture(ACC_DATE_STAMP, AHC_ACC_TIMESTAMP_DISABLE);
    #endif
    
    /* Set Record Parameters */
    AHC_GetImageSize(VIDEO_CAPTURE_MODE, &EncWidth, &EncHeight);
    
    MMPS_3GPRECD_SetStillCaptureMaxRes((MMP_USHORT)EncWidth, (MMP_USHORT)EncHeight);
   
    ulMinDiff = (UINT32)(-1);

    for (i = 0; i < VIDRECD_RESOL_MAX_NUM; i++) {
        UINT32 ulCurDiff = 0;

        if ((EncWidth <= VideoConfig->usEncWidth[i]) && (EncHeight <= VideoConfig->usEncHeight[i])) {
            ulCurDiff = (VideoConfig->usEncWidth[i] - EncWidth) + (VideoConfig->usEncHeight[i] - EncHeight);
            if (ulCurDiff < ulMinDiff) {
                usResolIdx = i;
                ulMinDiff = ulCurDiff;
                break;
            }
        }
    }

    printc("VR, EncWidth:%d, EncHeight:%d, usResolIdx:%d\r\n", EncWidth, EncHeight, usResolIdx);
    AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_RESOLUTION, usResolIdx);

    sRet = MMPS_3GPRECD_SetEncResIdx(MMPS_3GPRECD_FILESTREAM_NORMAL, usResolIdx);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}  

	#if (SUPPORT_ADAS)
    ulADASTimeLimit = AHC_VIDEO_GetRecTimeLimit();
    #endif
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetBitRate
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetBitRate(void)
{
    MMPS_3GPRECD_PRESET_CFG *VideoConfig = MMPS_3GPRECD_GetConfig();
    MMP_USHORT              usResolIdx = 0;
    MMP_USHORT              usQuality = 0;
    UINT32                  uiFrameRateType = 0;    
    UINT32                  Param;
    AHC_BOOL                ahcRet = AHC_TRUE;
    MMP_ERR                 sRet = MMP_ERR_NONE;

    switch (MenuSettingConfig()->uiMOVQuality) {
    case QUALITY_SUPER_FINE:
        usQuality = VIDRECD_QUALITY_HIGH;
        break;
    case QUALITY_FINE:
        usQuality = VIDRECD_QUALITY_MID;
        break;
    default:
        usQuality = VIDRECD_QUALITY_HIGH;
        break;
    }
    
    AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_COMPRESSION_RATIO, usQuality);

    AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_COMPRESSION_RATIO, &Param);
    usQuality = Param;

    AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_RESOLUTION, &Param);
    usResolIdx = (MMP_USHORT)Param;
        
    #if (EMER_RECORD_DUAL_WRITE_ENABLE == 1)
    if (MenuSettingConfig()->uiGsensorSensitivity != GSENSOR_SENSITIVITY_OFF)
    {
        sRet = MMPS_3GPRECD_SetBitrate(MMPS_3GPRECD_FILESTREAM_NORMAL, AHC_VIDEO_MAXBITRATE_PRERECORD);
    }
    else
    #endif    
    {
        AIHC_VIDEO_GetMovieCfgEx(0, AHC_FRAME_RATE, &uiFrameRateType);

        if (uiFrameRateType == VIDRECD_FRAMERATE_60FPS) {
            sRet = MMPS_3GPRECD_SetBitrate(MMPS_3GPRECD_FILESTREAM_NORMAL, VideoConfig->ulFps60BitrateMap[usResolIdx][usQuality]);
        }
        else {
            sRet = MMPS_3GPRECD_SetBitrate(MMPS_3GPRECD_FILESTREAM_NORMAL, VideoConfig->ulFps30BitrateMap[usResolIdx][usQuality]);
        }
    }
    
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetProfile
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetProfile(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;

    if (GET_VR_ENCODE_PROFILE(gsAhcPrmSensor) == VIDREC_H264_HIGH_PROFILE) {
        AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_CODEC_TYPE_SETTING, H264ENC_HIGH_PROFILE);
        sRet = MMPS_3GPRECD_SetProfile(MMPS_3GPRECD_FILESTREAM_NORMAL, H264ENC_HIGH_PROFILE);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
    }
    else if (GET_VR_ENCODE_PROFILE(gsAhcPrmSensor) == VIDREC_H264_BASELINE_PROFILE) {
        AHC_VIDEO_SetMovieConfig(0, AHC_VIDEO_CODEC_TYPE_SETTING, H264ENC_BASELINE_PROFILE);
        sRet = MMPS_3GPRECD_SetProfile(MMPS_3GPRECD_FILESTREAM_NORMAL, H264ENC_BASELINE_PROFILE);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
    }
    else {
        // TBD
    }

    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetContainerType
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetContainerType(void)
{
    VIDMGR_CONTAINER_TYPE   ContainerType;
    UINT32                  Param;
    AHC_BOOL                ahcRet = AHC_TRUE;
    MMP_ERR                 sRet = MMP_ERR_NONE;
    
    AIHC_VIDEO_GetMovieCfgEx(0, AHC_CLIP_CONTAINER_TYPE, &Param);
    ContainerType = Param;

    sRet = MMPS_3GPRECD_SetContainerType(ContainerType);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetVideoCodecType
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetVideoCodecType(void)
{
    MMPS_3GPRECD_VIDEO_FORMAT   VideoFmt;
    UINT32                      Param;
    AHC_BOOL                    ahcRet = AHC_TRUE;

    // Default as H264.
    AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_CODEC_TYPE, &Param);
    VideoFmt = Param;
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetH264BufferMode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetH264BufferMode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;

    #if (AHC_SHAREENC_SUPPORT)
    sRet = MMPS_3GPRECD_SetCurBufMode(MMPS_3GPRECD_FILESTREAM_NORMAL, VIDENC_CURBUF_FRAME);
    #else
    if (GET_VR_ENCODE_BUFMODE(gsAhcPrmSensor) == VIDREC_CURBUF_FRAME) {
        sRet = MMPS_3GPRECD_SetCurBufMode(MMPS_3GPRECD_FILESTREAM_NORMAL, VIDENC_CURBUF_FRAME);
    }
    else {
        sRet = MMPS_3GPRECD_SetCurBufMode(MMPS_3GPRECD_FILESTREAM_NORMAL, VIDENC_CURBUF_RT);
    }
    #endif
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetFrameRate
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetFrameRate(void)
{
    AHC_BOOL                bSlowMotionEnable = AHC_FALSE;
    AHC_BOOL                bVideoTimeLapseEnable = AHC_FALSE;
    UINT32                  usSlowMotionTimeIncrement, usSlowMotionTimeIncrResol;
    UINT32                  usTimeLapseTimeIncrement, usTimeLapseTimeIncrResol;
    INT32                   FrameRate;
    MMP_SNR_TVDEC_SRC_TYPE  sTVDecSrc = MMP_SNR_TVDEC_SRC_NO_READY; //Need to check...
    MMPS_3GPRECD_FRAMERATE  sSensorFps, sEncodeFps, sContainerFps;
    UINT32                  Param;    
    AHC_BOOL                ahcRet = AHC_TRUE;
    MMP_ERR                 sRet = MMP_ERR_NONE;

    AIHC_VIDEO_GetMovieCfgEx(0, AHC_FRAME_RATE, &Param);
    FrameRate = Param;

    FrameRate = AHC_VIDEO_GetVideoRealFpsX1000(FrameRate);
          
    if (CAM_CHECK_PRM(PRM_CAM_TV_DECODER)) {
        MMPS_Sensor_GetTVDecSrcType(&sTVDecSrc);
        
        if (sTVDecSrc == MMP_SNR_TVDEC_SRC_PAL)
            sSensorFps.usVopTimeIncrement = AHC_VIDRECD_TIME_INCREMENT_BASE_PAL;
        else if (sTVDecSrc == MMP_SNR_TVDEC_SRC_NTSC)
            sSensorFps.usVopTimeIncrement = AHC_VIDRECD_TIME_INCREMENT_BASE_NTSC;
		else if (sTVDecSrc == MMP_SNR_TVDEC_SRC_HD)
            sSensorFps.usVopTimeIncrement = AHC_VIDRECD_TIME_INCREMENT_BASE;
		else if (sTVDecSrc == MMP_SNR_TVDEC_SRC_FHD)
            sSensorFps.usVopTimeIncrement = AHC_VIDRECD_TIME_INCREMENT_BASE;
		else
			AHC_PRINT_RET_ERROR(0, 0);
    }
    else {
        sSensorFps.usVopTimeIncrement = AHC_VIDRECD_TIME_INCREMENT_BASE;
    }
    
    sSensorFps.usVopTimeIncrResol = FrameRate;
    
    DBG_AutoTestPrint(ATEST_ACT_REC_0x0001, ATEST_STS_FRAME_RATE_0x0006, 0, sSensorFps.usVopTimeIncrResol, gbAhcDbgBrk);

    AHC_VIDEO_GetSlowMotionFPS(&bSlowMotionEnable, &usSlowMotionTimeIncrement, &usSlowMotionTimeIncrResol);
    #if (defined(VIDEO_REC_TIMELAPSE_EN) && VIDEO_REC_TIMELAPSE_EN)
    AHC_VIDEO_GetTimeLapseFPS(&bVideoTimeLapseEnable, &usTimeLapseTimeIncrement, &usTimeLapseTimeIncrResol);
    #endif

    if (MMP_IsUSBCamExist())
    {
        USB_DETECT_PHASE USBCurrentStates = 0;

        AHC_USBGetStates(&USBCurrentStates,AHC_USB_GET_PHASE_CURRENT);
        
        if ((USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) && 
            (AHC_TRUE == AHC_HostUVCVideoIsEnabled())) {
            bSlowMotionEnable = AHC_FALSE;
            bVideoTimeLapseEnable = AHC_FALSE;
            printc(FG_RED("%s,%d Now dual cam does not support timelapse and slowmotion!\r\n"),__func__,__LINE__);
        }
    }
    
    if (MMP_IsScdCamExist()) {
        bSlowMotionEnable = AHC_FALSE;
        bVideoTimeLapseEnable = AHC_FALSE;
        printc(FG_RED("%s,%d Now dual cam does not support timelapse and slowmotion!\r\n"),__func__,__LINE__);
    }
    
    if (CAM_CHECK_PRM(PRM_CAM_TV_DECODER) ||
        CAM_CHECK_PRM(PRM_CAM_YUV_SENSOR)) {
        bSlowMotionEnable = AHC_FALSE;
        bVideoTimeLapseEnable = AHC_FALSE;
        printc(FG_RED("%s,%d Now TV decoder Snr does not support timelapse and slowmotion!\r\n"),__func__,__LINE__);
    }

    if (bSlowMotionEnable) {
        // Slow Motion Record
        sContainerFps.usVopTimeIncrement = usSlowMotionTimeIncrement;
        sContainerFps.usVopTimeIncrResol = usSlowMotionTimeIncrResol;        
        if ((sSensorFps.usVopTimeIncrement * sSensorFps.usVopTimeIncrResol) < (sContainerFps.usVopTimeIncrement * sContainerFps.usVopTimeIncrResol)){
            printc(FG_RED("%s,%d SlowMotion error!%d,%d,%d,%d\r\n"), sSensorFps.usVopTimeIncrement, sSensorFps.usVopTimeIncrResol, sContainerFps.usVopTimeIncrement, sContainerFps.usVopTimeIncrResol);
            sContainerFps.usVopTimeIncrement = sSensorFps.usVopTimeIncrement;
            sContainerFps.usVopTimeIncrResol = sSensorFps.usVopTimeIncrResol;   
        }
    }
    else {
        // Normal Record (Non-Slow Motion)
        sContainerFps.usVopTimeIncrement = sSensorFps.usVopTimeIncrement;
        sContainerFps.usVopTimeIncrResol = sSensorFps.usVopTimeIncrResol;
		if(/*(usResolIdx == VIDRECD_RESOL_3200x1808) &&*/ (sSensorFps.usVopTimeIncrResol == 24000)) {
			sContainerFps.usVopTimeIncrResol = 30000;
		}	
    }

    if (bVideoTimeLapseEnable) {
        // Time Lapse Record
        sEncodeFps.usVopTimeIncrement = usTimeLapseTimeIncrement;
        sEncodeFps.usVopTimeIncrResol = usTimeLapseTimeIncrResol;     
        if ((sSensorFps.usVopTimeIncrement * sSensorFps.usVopTimeIncrResol) < (sEncodeFps.usVopTimeIncrement * sEncodeFps.usVopTimeIncrResol)){
            printc(FG_RED("%s,%d VideoTimeLapse error!%d,%d,%d,%d\r\n"), sSensorFps.usVopTimeIncrement, sSensorFps.usVopTimeIncrResol, sEncodeFps.usVopTimeIncrement, sEncodeFps.usVopTimeIncrResol);
            sEncodeFps.usVopTimeIncrement = sSensorFps.usVopTimeIncrement;
            sEncodeFps.usVopTimeIncrResol = sSensorFps.usVopTimeIncrResol;   
        }
    }
    else {
        // Normal Record (Non-VideoTimeLapse)
        sEncodeFps.usVopTimeIncrement = sSensorFps.usVopTimeIncrement;
        sEncodeFps.usVopTimeIncrResol = sSensorFps.usVopTimeIncrResol;
		if(/*(usResolIdx == VIDRECD_RESOL_3200x1808) &&*/ (sSensorFps.usVopTimeIncrResol == 24000)) {
			sEncodeFps.usVopTimeIncrResol = 30000;
		}		
    }
    
    //printc("VR, sSensorFps: %d / %d\r\n", sSensorFps.usVopTimeIncrement, sSensorFps.usVopTimeIncrResol);
    //printc("VR, sEncodeFps: %d / %d\r\n", sEncodeFps.usVopTimeIncrement, sEncodeFps.usVopTimeIncrResol);
    //printc("VR, sContainerFps: %d / %d\r\n", sContainerFps.usVopTimeIncrement, sContainerFps.usVopTimeIncrResol);

    sRet = MMPS_3GPRECD_SetFrameRatePara(MMPS_3GPRECD_FILESTREAM_NORMAL, &sSensorFps, &sEncodeFps, &sContainerFps);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetP_BFrameCount
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetP_BFrameCount(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;
    INT32       PFrameNum, BFrameNum;
    UINT32      Param;

    AIHC_VIDEO_GetMovieCfgEx(0, AHC_MAX_PFRAME_NUM, &Param);
    PFrameNum = Param;

    AIHC_VIDEO_GetMovieCfgEx(0, AHC_MAX_BFRAME_NUM, &Param);
    BFrameNum = Param;

    //printc("VR:...PFrameNum=%d \r\n", PFrameNum);
    //printc("VR:...BFrameNum=%d \r\n", BFrameNum);

    sRet = MMPS_3GPRECD_SetBFrameCount(MMPS_3GPRECD_FILESTREAM_NORMAL, BFrameNum);

    sRet = MMPS_3GPRECD_SetPFrameCount(MMPS_3GPRECD_FILESTREAM_NORMAL, PFrameNum);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetAudioEncode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetAudioEncode(void)
{
    MMPS_3GPRECD_AUDIO_FORMAT   AudioFmt;
    MMPS_AUDIO_LINEIN_CHANNEL   AudioLineIn;
    MMP_UBYTE                   ubWMR;
    MMP_UBYTE                   ubRecordWithAudio;    
    MMP_ULONG                   ulSamplerate;
    UINT32                      Param;
    UINT8                       bAudioOn = 1; //Andy Liu TBD.
    AHC_BOOL                    ahcRet = AHC_TRUE;
    MMP_ERR                     sRet = MMP_ERR_NONE;

    bAudioOn = AHC_VIDEO_GetRecordModeAudioOn();
    
    if (bAudioOn) {
        AIHC_InitAudioExtDACIn();
    }

    AIHC_VIDEO_GetMovieCfgEx(0, AHC_AUD_CODEC_TYPE, &Param);
    AudioFmt = Param;
    //printc("VR:...AudioFmt=%d \r\n", AudioFmt);   

    AIHC_VIDEO_GetMovieCfgEx(0, AHC_AUD_SAMPLE_RATE, &Param);
    ulSamplerate = Param;
    //printc("VR:...ulSamplerate=%d \r\n", ulSamplerate);
    
    AIHC_VIDEO_GetMovieCfgEx(0, AHC_AUD_CHANNEL_CONFIG, &Param);
    AudioLineIn = Param;
    //printc("VR:...AudioLineIn=%d \r\n", AudioLineIn);

    if (bAudioOn) {
        sRet = MMPS_3GPRECD_SetAudioRecMode(MMPS_3GPRECD_REC_AUDIO_DATA);
    }
    else {
        sRet = MMPS_3GPRECD_SetAudioRecMode(MMPS_3GPRECD_NO_AUDIO_DATA);
    }
    
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    MMPS_AUDIO_SetLineInChannel(AudioLineIn);

    if (AudioFmt == MMPS_3GPRECD_AUDIO_FORMAT_AMR) {
        sRet = MMPS_3GPRECD_SetAudioFormat(AudioFmt, AHC_AMR_AUDIO_OPTION);
    }
    else if (AudioFmt == MMPS_3GPRECD_AUDIO_FORMAT_AAC) {
        sRet = MMPS_3GPRECD_SetAudioFormat(AudioFmt, AHC_AAC_AUDIO_OPTION);
    }
    else if (AudioFmt == MMPS_3GPRECD_AUDIO_FORMAT_ADPCM) {
        sRet = MMPS_3GPRECD_SetAudioFormat(AudioFmt, AHC_ADPCM_AUDIO_OPTION);
    }
    else if (AudioFmt == MMPS_3GPRECD_AUDIO_FORMAT_MP3) {
        sRet = MMPS_3GPRECD_SetAudioFormat(AudioFmt, AHC_MP3_AUDIO_OPTION);
    }
    else if (AudioFmt == MMPS_3GPRECD_AUDIO_FORMAT_PCM) {
        sRet = MMPS_3GPRECD_SetAudioFormat(AudioFmt, AHC_PCM_AUDIO_OPTION);
    }
    
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    
    AHC_Menu_SettingGetCB((char *)COMMON_KEY_WNR_EN, &ubWMR);
    #ifdef COMMON_KEY_RECORD_WITH_AUDIO
    AHC_Menu_SettingGetCB((char *)COMMON_KEY_RECORD_WITH_AUDIO, &ubRecordWithAudio);
    #else
    AHC_Menu_SettingGetCB((char *)"RecordWithAudio", &ubRecordWithAudio);
    #endif

    if ((ubWMR == WNR_ON) && bAudioOn && (ubRecordWithAudio == MOVIE_SOUND_RECORD_ON)) {
        AHC_VIDEO_SetRecWithWNR(AHC_TRUE);
    }
    else {
        AHC_VIDEO_SetRecWithWNR(AHC_FALSE);
    }

#if (VIDEO_REC_WITH_MUTE_EN)
    #if ((MENU_MOVIE_EN && MENU_MOVIE_SOUND_RECORD_EN) || (MENU_SINGLE_EN))
    if (MenuSettingConfig()->uiMOVSoundRecord == MOVIE_SOUND_RECORD_ON)
    #else
    if (1)
    #endif
    #if (SETTING_VR_VOLUME_EVERYTIME)
    {
        AHC_VIDEO_SetRecVolumeToFW(AHC_TRUE);
    }
    else 
    {
        AHC_VIDEO_SetRecVolumeToFW(AHC_FALSE);
    }
    #endif
#endif
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetTimeLimit
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetTimeLimit(void)
{
    UINT32      TimeLimit = 0;
    AHC_BOOL    ahcRet = AHC_TRUE;

    TimeLimit = AHC_VIDEO_GetRecTimeLimit();
    AHC_VIDEO_SetRecTimeLimit(TimeLimit);
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetDualEncode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetDualEncode(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;

    if ((AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_ENCODE_H264) ||
        (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_TRUE) & DUAL_REC_ENCODE_H264)) {
        sRet = MMPS_3GPRECD_EnableDualRecd(MMP_TRUE);
    }
    else {
        sRet = MMPS_3GPRECD_EnableDualRecd(MMP_FALSE);
    }
    
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/} 
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModePreSetFilename
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModePreSetFilename(void)
{
    VIDMGR_CONTAINER_TYPE   ContainerType;
    UINT8                   bCreateNewDir;
    MMP_BYTE                DirName[16]; 
    AHC_BOOL                bRet;
    UINT32                  Param;
    AHC_BOOL                ahcRet = AHC_TRUE;
    MMP_ERR                 sRet = MMP_ERR_NONE;    
    #if (GPS_KML_FILE_ENABLE == 1)
    MMP_BYTE                bKMLFileName[MAX_FILE_NAME_SIZE];
    #endif
    #if (GPS_RAW_FILE_ENABLE == 1)
    MMP_BYTE                bGPSRawFileName[MAX_FILE_NAME_SIZE];
    #endif
    #if ((GSENSOR_RAW_FILE_ENABLE == 1) && (GPS_CONNECT_ENABLE == 0))
    MMP_BYTE                bGPSRawFileName[MAX_FILE_NAME_SIZE];
    #endif
    #if (AHC_SHAREENC_SUPPORT)    
    UINT32                  uiFrameRateType = 0; //Need to check...
    MMP_UBYTE               *uByteTemp;
    #endif
    MMP_BOOL                bStatus = MMP_TRUE;
    
    // First record
    if (AHC_VIDEO_GetRecordModeFirstTimeRecord() == MMP_TRUE) {
        sRet = MMPS_3GPRECD_SetStoragePath(VIDENC_SRCMODE_CARD);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

        // First record
        MEMSET(m_PrevVRFullName, 0, sizeof(m_PrevVRFullName));
    }
    else {
        // Restart record
        MEMCPY(m_PrevVRFullName, m_CurVRFullName, sizeof(m_CurVRFullName));
    }
    
    /* Get Record FileName */
    AIHC_VIDEO_GetMovieCfgEx(0, AHC_CLIP_CONTAINER_TYPE, &Param);
    ContainerType = Param;
	
	/* Reset Full Name Array */
    MEMSET(m_CurVRFullName, 0, sizeof(m_CurVRFullName));
    #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
    MEMSET(m_CurThumbJpgFullName, 0, sizeof(m_CurThumbJpgFullName));
    #endif
    
    #if (VIDRECD_MULTI_TRACK == 0)
    if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
        MEMSET(m_CurVRRearFullName, 0, sizeof(m_CurVRRearFullName));
        #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
        MEMSET(m_CurThumbJpgFullName_Rear, 0, sizeof(m_CurThumbJpgFullName_Rear));
        #endif
    }
    
    if (!CAM_CHECK_USB(USB_CAM_NONE)) {
        MEMSET(m_CurVRUSBHFullName, 0, sizeof(m_CurVRUSBHFullName));
        #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
        MEMSET(m_CurThumbJpgFullName_USBH, 0, sizeof(m_CurThumbJpgFullName_USBH));
        #endif
    }    
    #endif
    
    /* Reset Dir/File Name Array */
    MEMSET(DirName, 0, sizeof(DirName));
    MEMSET(gpbCurVideoFileName, 0, sizeof(gpbCurVideoFileName));
    
    #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
    MEMSET(m_ThumbJpgFileName, 0, sizeof(m_ThumbJpgFileName));
    #endif

    #if (VIDRECD_MULTI_TRACK == 0)
    if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
        MEMSET(m_RearVideoRFileName, 0, sizeof(m_RearVideoRFileName));
        #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
        MEMSET(m_ThumbJpgFileName_Rear, 0, sizeof(m_ThumbJpgFileName_Rear));
        #endif
    }

    if (!CAM_CHECK_USB(USB_CAM_NONE)) {
        MEMSET(m_USBHVideoRFileName, 0, sizeof(m_USBHVideoRFileName));
        #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
        MEMSET(m_ThumbJpgFileName_USBH, 0, sizeof(m_ThumbJpgFileName_USBH));
        #endif
    }
    #endif

#if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_NORMAL)

    bRet = AHC_UF_GetName(&gusCurVideoDirKey, (INT8*)DirName, (INT8*)gpbCurVideoFileName, &bCreateNewDir);

    if (bRet  == AHC_TRUE) {

        STRCPY(m_CurVRFullName, (INT8*)AHC_UF_GetRootDirName());
        STRCAT(m_CurVRFullName, "\\");
        STRCAT(m_CurVRFullName, DirName);

        if (bCreateNewDir) {
            MMPS_FS_DirCreate((INT8*)m_CurVRFullName, STRLEN(m_CurVRFullName));
            AHC_UF_AddDir(DirName);
        }

        STRCAT(m_CurVRFullName, "\\");
        STRCAT(m_CurVRFullName, (INT8*)gpbCurVideoFileName);
    }
#elif (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
    {
        MMP_ULONG   ulRecordingOffset = 0;
        int         nSecondOffset;

        AHC_RTC_GetTime(&m_VideoRecStartRtcTime);

        sRet = MMPS_3GPRECD_Get3gpRecordingOffset(&ulRecordingOffset);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

        nSecondOffset = -1*(ulRecordingOffset/1000);
        AHC_DT_ShiftRtc(&m_VideoRecStartRtcTime, nSecondOffset);

        AHC_VIDEO_AvoidDuplicatedFileName( &m_VideoRecStartRtcTime );

        {
            AHC_BOOL bRearFlag;

            AHC_UF_GetRearCamPathFlag(&bRearFlag);
            AHC_UF_SetRearCamPathFlag(AHC_FALSE);
            bRet = AHC_UF_GetName2(&m_VideoRecStartRtcTime, (INT8*)m_CurVRFullName, (INT8*)gpbCurVideoFileName, &bCreateNewDir);
            AHC_UF_SetRearCamPathFlag(bRearFlag);
        }

        #if (VIDRECD_MULTI_TRACK == 0)
        {
            MMP_BOOL bDevConSts = MMP_TRUE;

            MMPS_USB_GetDevConnSts(&bDevConSts);
            if ((!CAM_CHECK_SCD(SCD_CAM_NONE)) && (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_STORE_FILE))
            {
                MEMSET(m_RearVideoRFileName, 0, sizeof(m_RearVideoRFileName));
                AHC_UF_SetRearCamPathFlag(AHC_TRUE);
                AHC_UF_GetName2(&m_VideoRecStartRtcTime, (INT8*)m_CurVRRearFullName, (INT8*)m_RearVideoRFileName, &bCreateNewDir);
                AHC_UF_SetRearCamPathFlag(AHC_FALSE);
            }
        
            if ((!CAM_CHECK_USB(USB_CAM_NONE)) && (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_TRUE) & DUAL_REC_STORE_FILE) &&
                (bDevConSts))
            {
                MEMSET(m_USBHVideoRFileName, 0, sizeof(m_USBHVideoRFileName));
                AHC_UF_SetRearCamPathFlag(AHC_TRUE);
                AHC_UF_GetName2(&m_VideoRecStartRtcTime, (INT8*)m_CurVRUSBHFullName, (INT8*)m_USBHVideoRFileName, &bCreateNewDir);
                AHC_UF_SetRearCamPathFlag(AHC_FALSE);
            }
        }
        #endif
    }
#endif

    if (bRet != AHC_TRUE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}  

    STRCAT(m_CurVRFullName, EXT_DOT);
    #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
    STRCPY(m_CurThumbJpgFullName, m_CurVRFullName);
    #endif
    
    #if (VIDRECD_MULTI_TRACK == 0)  
    if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
        STRCAT(m_CurVRRearFullName, EXT_DOT);
        #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
        STRCPY(m_CurThumbJpgFullName_Rear, m_CurVRRearFullName);
        #endif
    }

    if (!CAM_CHECK_USB(USB_CAM_NONE) && (m_CurVRUSBHFullName[0] != 0)) {
        #if (SUPPORT_COMPONENT_FLOW_CTL)
        STRCAT(m_CurVRUSBHFullName, "U");
        #endif
        STRCAT(m_CurVRUSBHFullName, EXT_DOT);
        #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
        STRCPY(m_CurThumbJpgFullName_USBH, m_CurVRUSBHFullName);
        #endif
    }
    #endif

    #if (GPS_CONNECT_ENABLE == 1)
    if (AHC_GPS_Module_Attached())
    {
        #if (GPS_KML_FILE_ENABLE == 1)
        MEMSET(bKMLFileName, 0, sizeof(bKMLFileName));
        STRCPY(bKMLFileName, m_CurVRFullName);
        STRCAT(bKMLFileName, GPS_KML_FILE_EXTENTION);
        #endif
        #if (GPS_RAW_FILE_ENABLE == 1)
        MEMSET(bGPSRawFileName, 0, sizeof(bGPSRawFileName));
        STRCPY(bGPSRawFileName, m_CurVRFullName);
        STRCAT(bGPSRawFileName, GPS_RAW_FILE_EXTENTION);
        #endif
    }
    #endif

    #if (GSENSOR_RAW_FILE_ENABLE == 1 && GPS_CONNECT_ENABLE == 0)
    MEMSET(bGPSRawFileName, 0, sizeof(bGPSRawFileName));
    STRCPY(bGPSRawFileName, m_CurVRFullName);
    STRCAT(bGPSRawFileName, GPS_RAW_FILE_EXTENTION);
    #endif

    if (ContainerType == VIDMGR_CONTAINER_3GP) {
        STRCAT(m_CurVRFullName, MOVIE_3GP_EXT);
        
        #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
        STRCAT(m_CurThumbJpgFullName, PHOTO_JPG_EXT);

        MEMCPY(m_ThumbJpgFileName, gpbCurVideoFileName, sizeof(m_ThumbJpgFileName));
        STRCAT((INT8*)m_ThumbJpgFileName, EXT_DOT);
        STRCAT((INT8*)m_ThumbJpgFileName, PHOTO_JPG_EXT);
        #endif
        
        #if (VIDRECD_MULTI_TRACK == 0)
        if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
            STRCAT(m_CurVRRearFullName, MOVIE_3GP_EXT);
            
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            STRCAT(m_CurThumbJpgFullName_Rear, PHOTO_JPG_EXT);

            MEMCPY(m_ThumbJpgFileName_Rear, m_RearVideoRFileName, sizeof(m_ThumbJpgFileName_Rear));
            STRCAT((INT8*)m_ThumbJpgFileName_Rear, EXT_DOT);
            STRCAT((INT8*)m_ThumbJpgFileName_Rear, PHOTO_JPG_EXT);
            #endif
        }

        if (!CAM_CHECK_USB(USB_CAM_NONE) && (m_CurVRUSBHFullName[0] != 0)) {
            STRCAT(m_CurVRUSBHFullName, MOVIE_3GP_EXT);
            
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            STRCAT(m_CurThumbJpgFullName_USBH, PHOTO_JPG_EXT);

            MEMCPY(m_ThumbJpgFileName_USBH, m_USBHVideoRFileName, sizeof(m_ThumbJpgFileName_USBH));
            STRCAT((INT8*)m_ThumbJpgFileName_USBH, EXT_DOT);
            STRCAT((INT8*)m_ThumbJpgFileName_USBH, PHOTO_JPG_EXT);
            #endif
        }
        #endif
        
        STRCAT((INT8*)gpbCurVideoFileName, EXT_DOT);
        STRCAT((INT8*)gpbCurVideoFileName, MOVIE_3GP_EXT);
        
        #if (VIDRECD_MULTI_TRACK == 0)
        if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
            STRCAT((INT8*)m_RearVideoRFileName, EXT_DOT);
            STRCAT((INT8*)m_RearVideoRFileName, MOVIE_3GP_EXT);
        }

        if (!CAM_CHECK_USB(USB_CAM_NONE) && (m_USBHVideoRFileName[0] != 0)) {
            #if (SUPPORT_COMPONENT_FLOW_CTL)
            STRCAT((INT8*)m_USBHVideoRFileName,"U");
            #endif
            STRCAT((INT8*)m_USBHVideoRFileName, EXT_DOT);
            STRCAT((INT8*)m_USBHVideoRFileName, MOVIE_3GP_EXT);
        }
        #endif
        
        #if (AHC_SHAREENC_SUPPORT)
        uByteTemp = &m_ShareRecFileName[0];
        if ((*uByteTemp != 0) && (!m_bStartShareRec))  {
            STRCAT((INT8*)m_ShareRecFileName, EXT_DOT);
            STRCAT((INT8*)m_ShareRecFileName, MOVIE_3GP_EXT);	
            // For parking mode.
            //STRCAT((INT8*)m_2ndVideoParkFileName, EXT_DOT);
            //STRCAT((INT8*)m_2ndVideoParkFileName, MOVIE_3GP_EXT);
        }
        #endif
    }
    else {
        STRCAT(m_CurVRFullName, MOVIE_AVI_EXT);
        
        #if (VIDRECD_MULTI_TRACK == 0)
        if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
            STRCAT(m_CurVRRearFullName, MOVIE_AVI_EXT);
        }

        if (!CAM_CHECK_USB(USB_CAM_NONE) && (m_CurVRUSBHFullName[0] != 0)) {
            STRCAT(m_CurVRUSBHFullName, MOVIE_AVI_EXT);
        }
        
        #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
        MEMCPY(m_ThumbJpgFileName, gpbCurVideoFileName, sizeof(m_ThumbJpgFileName));
        STRCAT((INT8*)m_ThumbJpgFileName, EXT_DOT);
        STRCAT((INT8*)m_ThumbJpgFileName, PHOTO_JPG_EXT);
        #endif
        #endif
        
        STRCAT((INT8*)gpbCurVideoFileName, EXT_DOT);
        STRCAT((INT8*)gpbCurVideoFileName, MOVIE_AVI_EXT);
        
        #if (VIDRECD_MULTI_TRACK == 0)
        if (!CAM_CHECK_SCD(SCD_CAM_NONE)) {
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            MEMCPY(m_ThumbJpgFileName_Rear, m_RearVideoRFileName, sizeof(m_ThumbJpgFileName_Rear));
            STRCAT((INT8*)m_ThumbJpgFileName_Rear, EXT_DOT);
            STRCAT((INT8*)m_ThumbJpgFileName_Rear, PHOTO_JPG_EXT);
            #endif                    
            STRCAT((INT8*)m_RearVideoRFileName, EXT_DOT);
            STRCAT((INT8*)m_RearVideoRFileName, MOVIE_AVI_EXT);
        }
        
        if (!CAM_CHECK_USB(USB_CAM_NONE) && (m_USBHVideoRFileName[0] != 0)) {
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            MEMCPY(m_ThumbJpgFileName_USBH, m_USBHVideoRFileName, sizeof(m_ThumbJpgFileName_USBH));
            STRCAT((INT8*)m_ThumbJpgFileName_USBH, EXT_DOT);
            STRCAT((INT8*)m_ThumbJpgFileName_USBH, PHOTO_JPG_EXT);
            #endif                    
            STRCAT((INT8*)m_USBHVideoRFileName, EXT_DOT);
            STRCAT((INT8*)m_USBHVideoRFileName, MOVIE_AVI_EXT);
        }
        #endif
        
        #if (AHC_SHAREENC_SUPPORT)
        uByteTemp = &m_ShareRecFileName[0];
        if (*uByteTemp != 0)  {
            STRCAT((INT8*)m_ShareRecFileName, EXT_DOT);
            STRCAT((INT8*)m_ShareRecFileName, MOVIE_AVI_EXT);
            // For parking mode.
            //STRCAT((INT8*)m_2ndVideoParkFileName, EXT_DOT);
            //STRCAT((INT8*)m_2ndVideoParkFileName, MOVIE_AVI_EXT);                    
        }
        #endif
    }

    {
        if (!CAM_CHECK_PRM(PRM_CAM_NONE))
        {
            sRet = AHC_VIDEO_SetFileName(m_CurVRFullName, STRLEN(m_CurVRFullName), VIDENC_STREAMTYPE_VIDRECD, DCF_CAM_FRONT);
            if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}         
        }
        
        #if (AHC_SHAREENC_SUPPORT)
        {
            MMP_ULONG                   ul2ndBitRate = 0;
            MMPS_3GPRECD_FRAMERATE      sSensorFps, sEncodeFps, sContainerFps;
            MMPS_3GPRECD_FRAMERATE      twinenc_fps, twincontainer_fps;
            MMPS_3GPRECD_PRESET_CFG     *VideoConfig = MMPS_3GPRECD_GetConfig();
            INT32                       PFrameNum, BFrameNum;
            MMP_USHORT                  usQuality;
            UINT32                      Param;
            
            MMPS_3GPRECD_GetFrameRatePara(MMPS_3GPRECD_FILESTREAM_NORMAL, &sSensorFps, &sEncodeFps, &sContainerFps);
            twinenc_fps.usVopTimeIncrement = sEncodeFps.usVopTimeIncrement;
            twinenc_fps.usVopTimeIncrResol = sEncodeFps.usVopTimeIncrResol;
            twincontainer_fps.usVopTimeIncrement = sContainerFps.usVopTimeIncrement;
            twincontainer_fps.usVopTimeIncrResol = sContainerFps.usVopTimeIncrResol;  
            
            AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_COMPRESSION_RATIO, &Param);
            usQuality = Param;
            
            AIHC_VIDEO_GetMovieCfgEx(0, AHC_MAX_PFRAME_NUM, &Param);
            PFrameNum = Param;
            
            AIHC_VIDEO_GetMovieCfgEx(0, AHC_MAX_BFRAME_NUM, &Param);
            BFrameNum = Param;
                          
            MMPS_3GPRECD_SetH264EncUseMode(VIDENC_STREAMTYPE_DUALENC, VIDRECD_USEMODE_RECD);
            MMPS_3GPRECD_SetEncResIdx(MMPS_3GPRECD_FILESTREAM_DUAL, m_usShareRecResolIdx);
            
            if (GET_VR_ENCODE_PROFILE(gsAhcPrmSensor) == H264ENC_HIGH_PROFILE) {
                MMPS_3GPRECD_SetProfile(MMPS_3GPRECD_FILESTREAM_DUAL, H264ENC_HIGH_PROFILE);
            }
            else if (GET_VR_ENCODE_PROFILE(gsAhcPrmSensor) == H264ENC_BASELINE_PROFILE) {
                MMPS_3GPRECD_SetProfile(MMPS_3GPRECD_FILESTREAM_DUAL, H264ENC_BASELINE_PROFILE);
            }
            
            MMPS_3GPRECD_SetCurBufMode(MMPS_3GPRECD_FILESTREAM_DUAL, VIDENC_CURBUF_FRAME);
            MMPS_3GPRECD_SetFrameRatePara(MMPS_3GPRECD_FILESTREAM_DUAL, &sSensorFps, &twinenc_fps, &twincontainer_fps);
        
            if (uiFrameRateType == VIDRECD_FRAMERATE_60FPS)            
                ul2ndBitRate = VideoConfig->ulFps60BitrateMap[m_usShareRecResolIdx][usQuality];
            else
                ul2ndBitRate = VideoConfig->ulFps30BitrateMap[m_usShareRecResolIdx][usQuality];

            MMPS_3GPRECD_SetBitrate(MMPS_3GPRECD_FILESTREAM_DUAL, ul2ndBitRate);
            MMPS_3GPRECD_SetPFrameCount(MMPS_3GPRECD_FILESTREAM_DUAL, PFrameNum);
            MMPS_3GPRECD_SetBFrameCount(MMPS_3GPRECD_FILESTREAM_DUAL, BFrameNum);
            
            MMPS_3GPRECD_CustomedEncResol(MMPS_3GPRECD_FILESTREAM_DUAL, 
                                          MMP_SCAL_FITMODE_OPTIMAL, 
                                          MMPS_3GPRECD_GetConfig()->usEncWidth[m_usShareRecResolIdx],
                                          MMPS_3GPRECD_GetConfig()->usEncHeight[m_usShareRecResolIdx]);
        }
        #endif
        
        #if (VIDRECD_MULTI_TRACK == 0)
        MMPS_USB_GetDevConnSts(&bStatus);
        if (MMP_IsUSBCamExist()) {
            if (AHC_TRUE == AHC_HostUVCVideoIsEnabled()) {
                VIDENC_STREAMTYPE sStreamType = VIDENC_STREAMTYPE_UVCRECD;
                                
                if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_TRUE) & DUAL_REC_ENCODE_H264) {
                    if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
                    	sStreamType = VIDENC_STREAMTYPE_VIDRECD;
                    else
                    	sStreamType = VIDENC_STREAMTYPE_DUALENC;
                }

                if (!CAM_CHECK_PRM(PRM_CAM_NONE))
                {
	                sRet = AHC_VIDEO_SetFileName(m_CurVRUSBHFullName, STRLEN(m_CurVRUSBHFullName), sStreamType, DCF_CAM_REAR);
    	            if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}         
                }
                else {                
					if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
					{
	                    sRet = AHC_VIDEO_SetFileName(m_CurVRFullName, STRLEN(m_CurVRFullName), sStreamType, DCF_CAM_FRONT);
    	                if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}         
        	            MEMSET(m_CurVRUSBHFullName, 0, sizeof(m_CurVRUSBHFullName));
            	        MEMSET(m_USBHVideoRFileName, 0, sizeof(m_USBHVideoRFileName));
					}
                }
            }
        }
        #if (SUPPORT_USB_HOST_FUNC)
        if (MMP_IsUSBCamExist() && !MMP_IsUSBCamIsoMode() && !bStatus) {
            MEMSET(m_CurVRUSBHFullName, 0, sizeof(m_CurVRUSBHFullName));
            #if (AHC_UVC_EMERGRECD_SUPPORT)
            MEMSET(m_EmrUSBHVideoRFileName, 0, sizeof(m_EmrUSBHVideoRFileName));
            #endif
        }
        #endif
        
        if (MMP_IsScdCamExist()) {
            if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_STORE_FILE) {
                sRet = AHC_VIDEO_SetFileName(m_CurVRRearFullName, STRLEN(m_CurVRRearFullName), VIDENC_STREAMTYPE_DUALENC, DCF_CAM_REAR);
                if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
            }
        }
        else {
            MEMSET(m_CurVRRearFullName, 0, sizeof(m_CurVRRearFullName));
        }
        #endif
    }    

    #if (GPS_CONNECT_ENABLE == 1)
    if (AHC_GPS_Module_Attached())
    {
        #if (GPS_KML_FILE_ENABLE == 1)
        GPSCtrl_SetKMLFileName(bKMLFileName,STRLEN(bKMLFileName));
        #endif
        
        #if (GPS_RAW_FILE_ENABLE == 1)
        {
            UINT8 bGPS_en;

            if (AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGPS_en) == AHC_TRUE) {
                switch (bGPS_en) {
                case RECODE_GPS_OFF:
                case RECODE_GPS_IN_VIDEO:
                    // NOP
                    break;
                default:                      
                    if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE && (uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE))
                    {   
                        MMP_ULONG ulRecordingOffset = 0;
                        
                        sRet = MMPS_3GPRECD_Get3gpRecordingOffset(&ulRecordingOffset);
                        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
                        #if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
                        GPSCtrl_SetGPSRawBufTime_Normal(ulRecordingOffset/1000);
                        #endif    
                    }

                    GPSCtrl_SetGPSRawFileName(bGPSRawFileName, STRLEN(bGPSRawFileName));
                    break;
                }
            }
        }
        #endif
    }
    #endif

    #if (GSENSOR_RAW_FILE_ENABLE == 1 && GPS_CONNECT_ENABLE == 0)
    {
        if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE && (uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE))
        {
            MMP_ULONG ulRecordingOffset = 0;
            
            sRet = MMPS_3GPRECD_Get3gpRecordingOffset(&ulRecordingOffset);
            if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
            #if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
            GPSCtrl_SetGPSRawBufTime_Normal(ulRecordingOffset/1000);
            #endif                
        }

        GPSCtrl_SetGPSRawFileName(bGPSRawFileName,STRLEN(bGPSRawFileName));
    }
    #endif

    if(uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE)
    {
	    #if (GPS_RAW_FILE_ENABLE == 1)
	    if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE)
	    {
	        GPSCtrl_GPSRawWriteFirst_Normal();
	        GPSCtrl_GPSRawResetBuffer();
	    }
	    #endif
	}

    RTNA_DBG_Str(0, "Prm:");
    RTNA_DBG_Str(0, m_CurVRFullName);
    RTNA_DBG_Str(0, "\r\n");
    #if (VIDRECD_MULTI_TRACK == 0)
    if (bStatus)
    {
        RTNA_DBG_Str(0, "Scd:");
        RTNA_DBG_Str(0, m_CurVRRearFullName);
        RTNA_DBG_Str(0, "\r\n");
        RTNA_DBG_Str(0, "USBH:");
        RTNA_DBG_Str(0, m_CurVRUSBHFullName);
        RTNA_DBG_Str(0, "\r\n");    
    }
    #endif
    #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
    RTNA_DBG_Str(0, "Thumb:");
    RTNA_DBG_Str(0, m_CurThumbJpgFullName);
    RTNA_DBG_Str(0, "\r\n");
    #endif

    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModePostSetFilename
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModePostSetFilename(void)
{
    #if (GPS_CONNECT_ENABLE && GPS_USE_FILE_AS_DATABUF)
    MMP_BYTE    bGpsInfoFileName[MAX_FILE_NAME_SIZE];
    MMP_ULONG   ulGpsInfoDirID;
    #endif
    AHC_BOOL    ahcRet = AHC_TRUE;

    /* Open GPS/Gsensor Temp File */
    #if (FS_FORMAT_FREE_ENABLE == 0) // Do NOT support GPS / Gsensor temp file in Format Free.
    #if (GPS_CONNECT_ENABLE && GPS_USE_FILE_AS_DATABUF)
    if (AHC_GPS_Module_Attached())
    {
        STRCPY(bGpsInfoFileName, (INT8*)AHC_UF_GetRootName());
        STRCAT(bGpsInfoFileName, GPS_TEMP_INFOFILE_DIR);

        if (AHC_FS_DirOpen(bGpsInfoFileName, sizeof(bGpsInfoFileName), &ulGpsInfoDirID) != AHC_ERR_NONE) {                
            AHC_ERR err;
            err = AHC_FS_DirCreate(bGpsInfoFileName, sizeof(bGpsInfoFileName));
            printc("AHC_FS_DirCreate :%x\r\n",err);
            AHC_FS_FileDirSetAttribute(bGpsInfoFileName, sizeof(bGpsInfoFileName), AHC_FS_ATTR_HIDDEN);		
        }
        else {
            AHC_FS_DirClose(ulGpsInfoDirID);
        }

        STRCAT(bGpsInfoFileName, "\\");
        STRCAT(bGpsInfoFileName, (INT8*)GPS_TEMP_INFOFILE_NAME);
        
        GPSCtrl_OpenInfoFile(bGpsInfoFileName);
    }
    #endif

    #if ((GSENSOR_CONNECT_ENABLE) && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO) && GSNR_USE_FILE_AS_DATABUF)
    if (AHC_Gsensor_Module_Attached())
    {
        MMP_BYTE    bGsensorInfoFileName[MAX_FILE_NAME_SIZE] = {0};
        MMP_ULONG   ulGsensorInfoDirID;

        STRCPY(bGsensorInfoFileName, (INT8*)AHC_UF_GetRootName());
        STRCAT(bGsensorInfoFileName, GSNR_TEMP_INFOFILE_DIR);

        if (AHC_FS_DirOpen(bGsensorInfoFileName, sizeof(bGsensorInfoFileName),&ulGsensorInfoDirID) != AHC_ERR_NONE) {
            AHC_ERR err;
            err = AHC_FS_DirCreate(bGsensorInfoFileName, sizeof(bGsensorInfoFileName));
            printc("AHC_FS_DirCreate :%x\r\n",err);
            AHC_FS_FileDirSetAttribute(bGsensorInfoFileName, sizeof(bGsensorInfoFileName), AHC_FS_ATTR_HIDDEN);
        }
        else {
            AHC_FS_DirClose(ulGsensorInfoDirID);
        }

        STRCAT(bGsensorInfoFileName, "\\");
        STRCAT(bGsensorInfoFileName, (INT8*)GSNR_TEMP_INFOFILE_NAME);
        
        AHC_Gsensor_OpenInfoFile(bGsensorInfoFileName);
    }
    #endif
    #endif

    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeRegisterCallback
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeRegisterCallback(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;

    #if (VR_SLOW_CARD_DETECT)
    sRet = MMPS_3GPRECD_SetVidRecdSkipModeParam(VRCB_TOTAL_SKIP_FRAME, VRCB_CONTINUOUS_SKIP_FRAME);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    #endif
    
    /* Register Callback Function */
    sRet = MMPS_3GPRECD_RegisterCallback (VIDMGR_EVENT_RECDSTOP_CARDSLOW,  (void*)VRCardSlowStopEncCB);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_MEDIA_FULL,           (void*)VRMediaFullCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_FILE_FULL,            (void*)VRFileFullCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}         

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_LONG_TIME_FILE_FULL,  (void*)VRLongTimeFileFullCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_MEDIA_ERROR,          (void*)VRMediaErrorCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}         

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_MEDIA_SLOW,           (void*)VRMediaSlowCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    #if (VR_SLOW_CARD_DETECT)
    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_PREGETTIME_CARDSLOW,  (void*)VRPreGetTimeWhenCardSlowCB); 
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_RECDSTOP_CARDSLOW,    (void*)VRRecdStopWhenCardSlowCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}         
    #endif

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_SEAMLESS ,            (void*)VRSeamlessCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_ENCODE_START,         (void*)VRStartEncCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_ENCODE_STOP,          (void*)VRStopEncCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_POSTPROCESS,          (void*)VRPostProcessCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_EMERGFILE_FULL,       (void*)VREmerDoneCB);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    #if (DUALENC_SUPPORT)
    if ((AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_FALSE) & DUAL_REC_ENCODE_H264) ||
        (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) & DUAL_REC_ENCODE_H264)) {
    
        sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_DUALENC_FILE_FULL,    (void *)VRDualEncFileFullCB);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}

        sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_DUALENCODE_START, 	(void *)VRDualEncStartCB);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

        sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_DUALENCODE_STOP, 	    (void *)VRDualEncStopCB);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

        sRet = MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT_STREAMCB, 		    (void *)VRStreamingCB);        
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    }
    #endif
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeMiscPreprocess
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeMiscPreprocess(void)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    if (AHC_VIDEO_GetRecordModeFirstTimeRecord() == AHC_TRUE) {
        // Disable Fast AE/AWB when recording    
        AHC_SetFastAeAwbMode(AHC_FALSE);
    }
    
    #if (AHC_ENABLE_VIDEO_STICKER)
    UpdateVideoCurrentTimeStamp();
    #endif
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeMiscPostprocess
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeMiscPostprocess(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;
    
    /* Check the Memory Usage */
    {
        MMP_ULONG ulVRDramEnd;
        
        sRet = MMPS_3GPRECD_GetParameter(MMPS_3GPRECD_PARAMETER_DRAM_END, &ulVRDramEnd);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}             
        printc(FG_BLUE("@@@ Dram Memory End Addr of Video Record: 0x%X")"\r\n", ulVRDramEnd);
    }

    // Refresh timestamp immediately after record start.
    #if (AHC_ENABLE_VIDEO_STICKER) && (DUALENC_SUPPORT)
    if ((AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_ENCODE_H264) ||
        (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_TRUE) & DUAL_REC_ENCODE_H264)) { 
        UpdateVideoCurrentTimeStamp();
    }
    #endif
 
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModePreAddFilename
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModePreAddFilename(void)
{
    AHC_BOOL bStatus;
    AHC_BOOL ahcRet = AHC_TRUE;   

    {
    #if (AHC_SHAREENC_SUPPORT)
        DCF_DB_TYPE sDB = AHC_UF_GetDB();
        
        if (sDB != DCF_DB_TYPE_1ST_DB) {
            printc(FG_RED("%s, wrong db: %d, %d\r\n"),__func__, sDB, __LINE__);
            AHC_UF_SelectDB(DCF_DB_TYPE_1ST_DB);
        }
        
        bStatus = AHC_UF_PreAddFile(gusCurVideoDirKey, (INT8*)gpbCurVideoFileName);
        AHC_UF_SelectDB(sDB);
    #else
        bStatus = AHC_UF_PreAddFile(gusCurVideoDirKey, (INT8*)gpbCurVideoFileName);
    #endif
    }

    AHC_PRINT_RET_ERROR(0, 0);
    printc(FG_RED("Need to add rear cam PreAddFile!!!\r\n"));
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModePreAddFilenameFails
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModePreAddFilenameFails(void)
{
    AHC_BOOL    bStatus; // Need to catch from start record
    AHC_BOOL    ahcRet = AHC_TRUE;   
    INT8        DirName[64]; 
	
    AHC_PRINT_RET_ERROR(gbAhcDbgBrk, bStatus);
    
    if (GetPathDirStr(DirName, sizeof(DirName), m_CurVRFullName))
    {
        if(!AHC_UF_FileOperation((UINT8*)DirName, (UINT8*)gpbCurVideoFileName, DCF_FILE_DELETE_ALL_CAM, NULL, NULL)) {
        	#if ((GSENSOR_RAW_FILE_ENABLE == 1) && (GPS_CONNECT_ENABLE == 0))
        	AHC_FS_FileRemove(bGPSRawFileName,STRLEN(bGPSRawFileName));	
        	#endif
        }	
    }

    AHC_PRINT_RET_ERROR(0, 0);
    printc(FG_RED("Need to add rear cam deletion!!!\r\n"));

    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetSeamless
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetSeamless(void)
{
    UINT32      TimeLimit = 0;
    AHC_BOOL    ahcRet = AHC_TRUE;   
    MMP_ERR     sRet = MMP_ERR_NONE;
    
    TimeLimit = AHC_VIDEO_GetRecTimeLimit();
    printc("%s,%d,TimeLimit:%d\r\n",__func__,__LINE__,TimeLimit);

    if (AHC_VIDEO_SetRecTimeLimit(TimeLimit) == 0) {
    
        /* Set File Size, Time Limit */
        sRet = MMPS_3GPRECD_SetFileSizeLimit(SIGNLE_FILE_SIZE_LIMIT_3_75G);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

        if (TimeLimit == NON_CYCLING_TIME_LIMIT/*Non cycle record*/){
            UINT64 ulStorageFreeSpace = 0;

            AHC_Media_GetFreeSpace(&ulStorageFreeSpace);

            if (ulStorageFreeSpace < SIGNLE_FILE_SIZE_LIMIT_4G) {
                printc(FG_RED("--W-- Free Space Not Enough to Enable Seamless-2\r\n"));
                sRet = MMPS_3GPRECD_StartSeamless(MMP_FALSE);
                if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
                
                AHC_VIDEO_SetVRSeamless(AHC_FALSE);
                return ahcRet;
            }
        }
    }

    sRet = MMPS_3GPRECD_StartSeamless(MMP_TRUE);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    
    AHC_VIDEO_SetVRSeamless(AHC_TRUE);
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeSetEmergency
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeSetEmergency(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;   
    MMP_ERR     sRet = MMP_ERR_NONE;
    #if (DUAL_EMERGRECD_SUPPORT) && (AHC_DUAL_EMERGRECD_SUPPORT)
    MMP_SNR_TVDEC_SRC_TYPE sSnrTVSrc = MMP_SNR_TVDEC_SRC_PAL;    
    #endif

    #if ((UVC_HOST_VIDEO_ENABLE) && (UVC_EMERGRECD_SUPPORT) && (AHC_UVC_EMERGRECD_SUPPORT))
    USB_DETECT_PHASE USBCurrentStates = 0;
    
    AHC_USBGetStates(&USBCurrentStates, AHC_USB_GET_PHASE_CURRENT);
    
    #if (SUPPORT_USB_HOST_FUNC)
    if ((USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) && 
        (AHC_TRUE == AHC_HostUVCVideoIsEnabled()) &&
        (AHC_FALSE== MMP_IsUSBCamIsoMode()))
    {
        sRet = MMPS_3GPRECD_EnableUVCEmergentRecd(MMP_TRUE);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    }
    #endif
    #endif
    
    #if (DUAL_EMERGRECD_SUPPORT) && (AHC_DUAL_EMERGRECD_SUPPORT)
    if (MMP_GetScdCamType() == SCD_CAM_TV_DECODER)
    {
        MMPS_Sensor_GetTVDecSrcType(&sSnrTVSrc);   
    }
    
    if ((MMP_IsScdCamExist() && (sSnrTVSrc != MMP_SNR_TVDEC_SRC_NO_READY)) ||
        ((MMP_IsUSBCamExist()) && (AHC_TRUE == AHC_HostUVCVideoIsEnabled()) && (AHC_TRUE == MMP_IsUSBCamIsoMode())))
    {
        sRet = MMPS_3GPRECD_EnableDualEmergentRecd(MMP_TRUE);
    }
    else
    {
        sRet = MMPS_3GPRECD_EnableDualEmergentRecd(MMP_FALSE);
    }
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    #endif
    
    #if (AHC_EMERGENTRECD_SUPPORT)
    if (MMP_IsPrmCamExist())
    {
        sRet = MMPS_3GPRECD_EnableEmergentRecd(AHC_TRUE);
    }
    else
    {
        sRet = MMPS_3GPRECD_EnableEmergentRecd(MMP_FALSE);
    }
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    #if (EMER_RECORD_DUAL_WRITE_PRETIME > 0)
    sRet = MMPS_3GPRECD_SetEmergPreEncTimeLimit(EMER_RECORD_DUAL_WRITE_PRETIME * 1000 + EM_VR_TIME_LIMIT_OFFSET);
    #endif
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 

    AHC_VIDEO_SetEmergRecInterval(EMER_RECORD_DUAL_WRITE_INTERVAL);
    #endif
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeInitCommon
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeInitCommon(void)
{
    MMP_USHORT              usVideoPreviewMode;
    AHC_DISPLAY_OUTPUTPANEL OutputDevice;
    MMP_USHORT              zoomIndex;
    UINT32                  ulTVDecTypeTimeout = 100;
    MMP_SNR_TVDEC_SRC_TYPE  sTVDecSrc = MMP_SNR_TVDEC_SRC_NO_READY;
    UINT32                  ulFlickerMode = COMMON_FLICKER_50HZ;
    UINT8                   ubEv = COMMON_EVVALUE_00;
    AHC_BOOL                ubSnrFlipEn;
    AHC_WINDOW_RECT         rect;
    AHC_BOOL                ahcRet = AHC_TRUE;
    MMP_ERR                 sRet = MMP_ERR_NONE;

    /* Reset Icon Link Source */
    MMPD_Icon_InitLinkSrc();

    if ((ahcRet = AHC_VIDEO_SetRecordModeInitParameters()) == AHC_FALSE) {
        return ahcRet;
    }
            
    #if (defined(WIFI_PORT) && WIFI_PORT == 1)
    if (AHC_GetStreamingMode() != AHC_STREAM_OFF) {
        printc(FG_RED("Need to reallocate preview memory, so force to stop streaming!"));
        UNLOCK_AHC_MODE();
        AHC_SetStreamingMode(AHC_STREAM_OFF);
        LOCK_AHC_MODE();
    }
    #endif
    
    /* Stop Preview First */
    #if (UVC_HOST_VIDEO_ENABLE)
    if (MMP_IsUSBCamExist()) {
		printc("-----------------LLLLLLLLL-----------------\r\n");
        AIHC_UVCPreviewStop();
    }
    #endif

    // Move from AIHC_UVCPreviewStop().
    #if(UVC_HOST_VIDEO_ENABLE)
    if (MMP_IsUSBCamExist()) {
        sRet = MMPS_Display_SetWinActive(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), MMP_FALSE);
    }
    #endif
    if (MMP_IsScdCamExist()) {
		printc("-----------------GET_VR_PREVIEW_WINDOW(gsAhcScdSensor)=%d----------------\r\n",GET_VR_PREVIEW_WINDOW(gsAhcScdSensor));
        sRet = MMPS_Display_SetWinActive(GET_VR_PREVIEW_WINDOW(gsAhcScdSensor), MMP_FALSE);
    }
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}

    #if (UVC_HOST_VIDEO_ENABLE)
    if (MMP_IsUSBCamExist()) {
        ahcRet = AHC_HostUVCResetFBMemory();
        #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
        if (MMP_IsSupportDecMjpegToPreview()) {
			printc("-----------------KKKKK-----------------\r\n");
            ahcRet = AHC_HostUVCResetMjpegToPreviewJpegBuf();
        }
        #endif
    }
    #endif

    if (MMP_IsPrmCamExist()) {
        sRet = MMPS_3GPRECD_PreviewStop(gsAhcPrmSensor);
    }
    
    if (MMP_IsScdCamExist()) {
        if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) {
			printc("-----------------TV_DECODER-----------------\r\n");
            MMPS_Sensor_EnableTVDecSrcTypeDetection(MMP_FALSE);
            sRet |= MMPS_3GPRECD_2ndSnrPreviewStop(gsAhcScdSensor);
        }
        else if (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
            sRet |= MMPS_3GPRECD_2ndSnrPreviewStop(gsAhcScdSensor);
        }
    }
    
    // Add for sticker sometimes abnormal issue.
    MMPS_3GPRECD_EnableSticker(MMP_STICKER_PIPE0_ID0, MMP_FALSE);
    
    AHC_VIDEO_SetRecWithWNR(AHC_FALSE);
       
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}             
    
    /* Initialize Sensor */
    if (MMPS_Sensor_IsInitialized(gsAhcPrmSensor) == MMP_FALSE)
    {
    	MMP_UBYTE ubPreviewMode_Scd;
		
        MMPS_Sensor_SetPreviewDelayCount(gsAhcPrmSensor, DISP_PREVIEW_DELAY_COUNT_PRM_SENSOR);
        
        if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) {
            // Init second sensor before primary sensor for real board issue.  
            // Root casue is unknown yet. TBD.
            printc("-----------------233333-----------------\r\n");
            MMPS_Sensor_RegisterCallback(SCD_SENSOR, MMPS_IVA_EVENT_TV_SRC_TYPE, (void*)SNRTvSrcTypeCB);

			ubPreviewMode_Scd = MMPS_Sensor_GetDefPreviewMode(gsAhcScdSensor);
			sRet = MMPS_Sensor_Initialize(gsAhcScdSensor, ubPreviewMode_Scd, MMP_SNR_VID_PRW_MODE);
            if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }
        }

        if (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
            // Use ubDefPreviewMode in sensor driver as 2nd sensor Preview mode
            ubPreviewMode_Scd = MMPS_Sensor_GetDefPreviewMode(gsAhcScdSensor);
            
            sRet = MMPS_Sensor_Initialize(gsAhcScdSensor, ubPreviewMode_Scd, MMP_SNR_VID_PRW_MODE);
            if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }
        }
        else if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
            sRet = MMPS_Sensor_Initialize(gsAhcScdSensor, AHC_SNR_GetPresetSnrMode(), MMP_SNR_VID_PRW_MODE);
            if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
        }

        if (MMP_IsPrmCamExist()) {
            sRet = MMPS_Sensor_Initialize(gsAhcPrmSensor, AHC_SNR_GetPresetSnrMode(), MMP_SNR_VID_PRW_MODE);
            if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
        }        
    }

    if (AHC_Menu_SettingGetCB((char *)COMMON_KEY_FLICKER, &ulFlickerMode) == AHC_TRUE) {
        if (COMMON_FLICKER_60HZ == ulFlickerMode) {
            AHC_SNR_SetLightFreq(AHC_SENSOR_VIDEO_DEBAND_60HZ);
        }
        else {
            AHC_SNR_SetLightFreq(AHC_SENSOR_VIDEO_DEBAND_50HZ);
        }
    }
    else {
        AHC_SNR_SetLightFreq(AHC_SENSOR_VIDEO_DEBAND_60HZ);
    }

    if (AHC_Menu_SettingGetCB((char *)COMMON_KEY_EV, &ubEv) == AHC_TRUE) {
        AHC_SetAeEvBiasMode(Menu_EV_To_AE_EV_BIAS(ubEv));
    }
    
    /* Set Display Device */
    AHC_GetDisplayOutputDev(&OutputDevice);
    sRet = MMPS_Display_SetOutputPanel(MMP_DISPLAY_PRM_CTL, OutputDevice);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
printc("OutputDevice=%d\r\n",OutputDevice);
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

    sRet = MMPS_3GPRECD_SetPreviewMode(usVideoPreviewMode);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}             
    
    /* Set Preview Zoom Configuration */
    {
	    MMP_IBC_PIPEID ubPipe;

        sRet = MMPS_3GPRECD_GetPreviewPipe(gsAhcPrmSensor, &ubPipe);            
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
        
        if (gsStillZoomIndex == 0xFFFF) {
            MMPS_3GPRECD_GetCurZoomStep(ubPipe, &zoomIndex);
        }
        else {
            zoomIndex = gsStillZoomIndex;
            gsStillZoomIndex = 0xFFFF;
        }

        #if (VIDEO_CAMERA_DZOOM_SYNC)
        sRet = MMPS_3GPRECD_SetPreviewZoom(MMPS_3GPRECD_ZOOM_PATH_PREV, MMP_PTZ_ZOOMIN, zoomIndex);
        #else
        sRet = MMPS_3GPRECD_SetPreviewZoom(MMPS_3GPRECD_ZOOM_PATH_PREV, MMP_PTZ_ZOOMOUT, 0);
        #endif
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    }
    
    /* Decide the Preview Path Type */
    if (MMP_IsVidLdcSupport() && MMP_GetVidLdcRunMode() != LDC_RUN_MODE_DISABLE)
    {
        UINT32 EncWidth, EncHeight;
      
        AHC_GetImageSize(VIDEO_CAPTURE_MODE, &EncWidth, &EncHeight);
          printc("---------------EncWidth=%d--EncHeight=%d--------------\r\n",EncWidth,EncHeight);
        if ((EncWidth == 1920 && EncHeight == 1088) ||
            (EncWidth == 1280 && EncHeight == 720) ||
            (EncWidth == 848 && EncHeight == 480)) {

            MMPS_3GPRECD_GetConfig()->previewpath[0] = MMP_GetVidLdcRunMode()+MMP_3GP_PATH_LDC_LB_SINGLE;

            if (EncWidth == 1920 && EncHeight == 1088) {
				MMPS_3GPRECD_GetConfig()->previewpath[0] = MMP_3GP_PATH_LDC_LB_SINGLE;//single loop back always.
                sRet = MMPS_3GPRECD_SetLdcResMode(MMP_LDC_RES_MODE_FHD, MMP_LDC_FPS_MODE_30P);
            }
            else if (EncWidth == 1280 && EncHeight == 720) {
                sRet = MMPS_3GPRECD_SetLdcResMode(MMP_LDC_RES_MODE_HD, MMP_LDC_FPS_MODE_60P);
            }
            else if (EncWidth == 848 && EncHeight == 480) {
                sRet = MMPS_3GPRECD_SetLdcResMode(MMP_LDC_RES_MODE_WVGA, MMP_LDC_FPS_MODE_30P);
            }

            if (MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE) {
                sRet = MMPS_3GPRECD_SetLdcRunMode(MMP_LDC_RUN_MODE_SINGLE_RUN);
            }
            else if (MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI) {
                sRet = MMPS_3GPRECD_SetLdcRunMode(MMP_LDC_RUN_MODE_MULTI_RUN);
            }
            else if (MMPS_3GPRECD_GetConfig()->previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
                sRet = MMPS_3GPRECD_SetLdcRunMode(MMP_LDC_RUN_MODE_MULTI_SLICE);
            }
        }
        else {
            MMPS_3GPRECD_GetConfig()->previewpath[0] = MMP_3GP_PATH_2PIPE;
            sRet = MMPS_3GPRECD_SetLdcRunMode(MMP_LDC_RUN_MODE_DISABLE);
        }

        if (usVideoPreviewMode == VIDRECD_HDMI_PREVIEW_MODE) {
            MMPS_3GPRECD_GetConfig()->previewpath[0] = MMP_3GP_PATH_2PIPE;
            sRet = MMPS_3GPRECD_SetLdcRunMode(MMP_LDC_RUN_MODE_DISABLE);
        }
        
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    }
    else {
        MMPS_3GPRECD_GetConfig()->previewpath[0] = MMP_3GP_PATH_2PIPE;
        sRet = MMPS_3GPRECD_SetLdcRunMode(MMP_LDC_RUN_MODE_DISABLE);
    }
    
    /* Set Sensor Flip Direction */  
    ubSnrFlipEn = AHC_CheckSNRFlipEn(CTRL_BY_ALL) && AHC_GetSensorStatus();
    AHC_SetKitDirection(AHC_LCD_NOFLIP, AHC_FALSE, AHC_GetSensorStatus(), ubSnrFlipEn);
	
	#if (MENU_GENERAL_LCD_ROTATE_EN)
	if(MenuSettingConfig()->uiLCDRotate == LCD_ROTATE_ON){
		AHC_SNR_SetFlipDir(PRM_SENSOR, SENSOR_180_DEGREE);
	}
	#endif
    
    /* Set USB Cam Preview Attribute */
    #if (UVC_HOST_VIDEO_ENABLE)
    if(AHC_IsTVConnectEx()||AHC_IsHdmiConnect()){
    	sRet = MMPS_UVCRECD_SetUVCPrevwRote(MMP_GRAPHICS_ROTATE_NO_ROTATE);
    }
    else
    {
		#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
		sRet = MMPS_UVCRECD_SetUVCPrevwRote(MMP_GRAPHICS_ROTATE_NO_ROTATE);
		#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90)
		sRet = MMPS_UVCRECD_SetUVCPrevwRote(MMP_GRAPHICS_ROTATE_RIGHT_90);
		#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
		sRet = MMPS_UVCRECD_SetUVCPrevwRote(MMP_GRAPHICS_ROTATE_RIGHT_270);
		#endif
		if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
    }
    #endif

    #if (CCIR656_OUTPUT_ENABLE) 
    // MMPS_CCIR_DisableDisplay();// To avoid display abnormal at first of switch preview mode
    #endif
    
    
#if (FRONT_MOTION_DETECTION_EN)
    if (!CAM_CHECK_PRM(PRM_CAM_NONE))
    {
	    MMPS_Sensor_EnableVMD(PRM_SENSOR, MMP_TRUE);
    	sRet |= MMPS_Sensor_InitializeVMD(PRM_SENSOR, &gstMdtcCfg[PRM_SENSOR]);

	    if (sRet != MMP_ERR_NONE) {
    		RTNA_DBG_Str(0, "Initialize video motion detection config failed\r\n");
    	}
    }
#endif

#if (FRONT_ADAS_EN)
    if (!CAM_CHECK_PRM(PRM_CAM_NONE))
    {
	    MMPS_Sensor_EnableADAS(PRM_SENSOR, MMP_TRUE);
    	sRet |= MMPS_Sensor_InitializeADAS(PRM_SENSOR, &gstLdwsCfg[PRM_SENSOR]);

	    if (sRet != MMP_ERR_NONE) {
    		RTNA_DBG_Str(0, "Initialize video ADAS config failed\r\n");
    	}
    }
#endif

    /* Start [Prm] TV Decoder Detect Flow */
    if (CAM_CHECK_PRM(PRM_CAM_TV_DECODER)) {
        
        ulTVDecTypeTimeout = 100;
        
        // Wait until TV Src type ready.
        do {
            MMPS_Sensor_CheckTVDecSrcType(gsAhcPrmSensor, &sTVDecSrc);
            if (MMP_SNR_TVDEC_SRC_NO_READY == sTVDecSrc) {
                AHC_OS_Sleep(100);
            }
            else {
                break;
            }
        } while ((MMP_SNR_TVDEC_SRC_NO_READY == sTVDecSrc) && (--ulTVDecTypeTimeout > 0));
        
        switch(sTVDecSrc)
        {
        case MMP_SNR_TVDEC_SRC_PAL:
            AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_PAL_25FPS);
            MMPS_Sensor_ChangePreviewMode(gsAhcPrmSensor, AHC_SNR_GetPresetSnrMode(), 10);
            break;
        case MMP_SNR_TVDEC_SRC_NTSC:
            AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_NTSC_30FPS);
            MMPS_Sensor_ChangePreviewMode(gsAhcPrmSensor, AHC_SNR_GetPresetSnrMode(), 10);
            break;
        case MMP_SNR_TVDEC_SRC_HD:
            AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_AHDHD_25FPS);
            MMPS_Sensor_ChangePreviewMode(gsAhcPrmSensor, AHC_SNR_GetPresetSnrMode(), 10);
            break;
        case MMP_SNR_TVDEC_SRC_FHD:
            AHC_SNR_PresetSnrMode(AHC_SENSOR_MODE_AHDFHD_25FPS);
            MMPS_Sensor_ChangePreviewMode(gsAhcPrmSensor, AHC_SNR_GetPresetSnrMode(), 10);
            break;
        default:
            printc(FG_RED("%s,%d warning! SNR check signal type TimeOut.\r\n"), __func__, __LINE__);
            break;
        }
    }
    
	if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
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
    
    /* Start Prm Sensor Preview Flow */
    if (MMP_IsPrmCamExist() && GET_VR_PREVIEW_ACTIVE(gsAhcPrmSensor)) {
        sRet = MMPS_3GPRECD_PreviewStart(gsAhcPrmSensor, MMP_FALSE);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
    }
    
    {
        #ifdef CUS_PREVIEW_WINDOW_POS
        // TBD
        #else
        MMP_ULONG dw, dh;
        
        MMPS_3GPRECD_GetPreviewRes(gsAhcPrmSensor, &dw, &dh);

        rect.usLeft     = 0;
        rect.usTop      = 0;
	#if (TVOUT_PREVIEW_EN)
		if (AHC_IsTVConnectEx())
		{
			rect.usWidth	= dw;
			rect.usHeight	= dh;
		}
		else
	#endif
		{
	        #if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
	        rect.usWidth    = dw;
	        rect.usHeight   = dh;
	        #elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
		printc("dh=%d.dw=%d\r\n",dh,dw);
	        rect.usWidth    = dh;
	        rect.usHeight   = dw;
	        #endif
        }
        AHC_PreviewWindowOp(AHC_PREVIEW_WINDOW_OP_SET | AHC_PREVIEW_WINDOW_FRONT, &rect);
        #endif
    }
    
#if (REAR_MOTION_DETECTION_EN)
	    MMPS_Sensor_EnableVMD(SCD_SENSOR, MMP_TRUE);
	    sRet |= MMPS_Sensor_InitializeVMD(SCD_SENSOR, &gstMdtcCfg[SCD_SENSOR]);



	    if (sRet != MMP_ERR_NONE) {
	    	RTNA_DBG_Str(0, "Initialize video motion detection config failed\r\n");
	    }
#endif

#if (REAR_ADAS_EN)
	    MMPS_Sensor_EnableADAS(SCD_SENSOR, MMP_TRUE);
	    sRet |= MMPS_Sensor_InitializeADAS(SCD_SENSOR, &gstLdwsCfg[SCD_SENSOR]);

	    if (sRet != MMP_ERR_NONE) {
	    	RTNA_DBG_Str(0, "Initialize video ADAS config failed\r\n");
        }
#endif
    
    /* Start Scd Sensor Preview Flow */
    if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER))
    {
        ulTVDecTypeTimeout = 10;

        do {
            MMPS_Sensor_CheckTVDecSrcType(gsAhcScdSensor, &sTVDecSrc);
            if (MMP_SNR_TVDEC_SRC_NO_READY == sTVDecSrc) {
                AHC_OS_Sleep(10);
            }
            else {
                break;
            }
        } while ((MMP_SNR_TVDEC_SRC_NO_READY == sTVDecSrc) && (--ulTVDecTypeTimeout > 0));

        if (MMP_SNR_TVDEC_SRC_PAL == sTVDecSrc) {
            AHC_SNR_PresetTVDecSrcType(AHC_SENSOR_MODE_PAL_25FPS);
            MMPS_Sensor_SetPreviewMode(gsAhcScdSensor, AHC_SNR_GetPresetTVDecSrcType());
        }
        else if (MMP_SNR_TVDEC_SRC_NTSC == sTVDecSrc) {
            AHC_SNR_PresetTVDecSrcType(AHC_SENSOR_MODE_NTSC_30FPS);
            MMPS_Sensor_SetPreviewMode(gsAhcScdSensor, AHC_SNR_GetPresetTVDecSrcType());
        }
		else if (MMP_SNR_TVDEC_SRC_HD == sTVDecSrc) {
			printc(FG_GREEN("%s,%d,Don`t change SNR2 preview mode.\r\n"), __func__, __LINE__);//use sensor`s ubDefPreviewMode
            //AHC_SNR_PresetTVDecSrcType(AHC_SENSOR_MODE_AHDHD_25FPS);
            //MMPS_Sensor_SetPreviewMode(gsAhcScdSensor, AHC_SNR_GetPresetTVDecSrcType());
        }
        else {
            printc(FG_GREEN("%s,%d warning! SNR2 check signal type TimeOut.\r\n"), __func__, __LINE__);
        }

        MMPS_Sensor_EnableTVDecSrcTypeDetection(MMP_TRUE);

        if (AHC_FALSE == AHC_SNR_GetTvDecSnrCnntStatus())
            MMPS_Sensor_SetPreviewDelayCount(gsAhcScdSensor, DISP_DISABLE_PREVIEW_DELAY_COUNT);
        else
            MMPS_Sensor_SetPreviewDelayCount(gsAhcScdSensor, DISP_PREVIEW_DELAY_COUNT_SCD_SENSOR);
    }
    
    if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER) ||
        CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
        
        if (GET_VR_PREVIEW_ACTIVE(gsAhcScdSensor)) {
        
            sRet = MMPS_3GPRECD_2ndSnrPreviewStart(gsAhcScdSensor, MMP_FALSE);
            if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}  //TBD.  If rear sensor is not connected, skip second stream record.              
            
            #if defined(WIFI_PORT) && (WIFI_PORT == 1) 
            // Add this for WIFI APP need this for switch sensor
            ncam_set_rear_cam_ready(AHC_TRUE);
            #endif
            
            {
                #ifdef CUS_PREVIEW_WINDOW_POS
                // TBD
                #else
                MMP_ULONG dw, dh;
                printc("gsAhcScdSensor=%d\r\n",gsAhcScdSensor);
                MMPS_3GPRECD_GetPreviewRes(gsAhcScdSensor, &dw, &dh);
                
                rect.usLeft     = 0;
                rect.usTop      = 0;
				#if (TVOUT_PREVIEW_EN)
				if (AHC_IsTVConnectEx())
				{
					rect.usWidth	= dw;
					rect.usHeight	= dh;
				}
				else
				#endif
				{
	                #if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
	                rect.usWidth    = dw;
	                rect.usHeight   = dh;
	                #elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
					printc("111111dh=%d.dw=%d\r\n",dh,dw);
	                rect.usWidth    = dh;
	                rect.usHeight   = dw;
	                #endif
				}
				printc("222dh=%d.dw=%d\r\n",rect.usHeight,rect.usWidth);
				
                AHC_PreviewWindowOp(/*AHC_PREVIEW_WINDOW_OP_SET | */AHC_PREVIEW_WINDOW_REAR, &rect);
                #endif
            }
        }
        
        #if (DUALENC_SUPPORT)
        if ((AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_ENCODE_H264) ||
            (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_TRUE) & DUAL_REC_ENCODE_H264)) {
            if (sRet == MMP_ERR_NONE) {
                sRet = MMPS_3GPRECD_SetDualH264SnrId(gsAhcScdSensor);
                if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
            }
        }
        #endif
    }

    #if (SUPPORT_SHARE_REC)
    sRet = MMPS_3GPRECD_SetDualH264SnrId(gsAhcPrmSensor);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
    #endif
    
    UpdateVideoCurrentTimeStamp();

    /* Start USBH Cam Preview Flow */
    #if (UVC_HOST_VIDEO_ENABLE)
    if (MMP_IsUSBCamExist()) {
		printc("------------SSSSS------------\r\n");
        AIHC_UVCPreviewStart(usVideoPreviewMode, OutputDevice, ulFlickerMode);
    }
    #endif
    
    AHC_SetFastAeAwbMode(AHC_TRUE);
  
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeInit
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeInit(void)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    
    ahcRet = AHC_VIDEO_SetRecordModeInitCommon();
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeUnInit
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeUnInit(void)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    
    // NOP, TBD
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeStart
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeStart(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;

    /* Start Normal/Dual/UVC Record */
    // Patch for AIT rear cam slow card issue. Open file for UVC here.
    // Seamless is ok because of TX start by m_container.
    #if (UVC_HOST_VIDEO_ENABLE) && (UVC_VIDRECD_SUPPORT)
    if ((AHC_VIDEO_GetRecordModeFirstTimeRecord() == AHC_TRUE) && 
        (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) == DUAL_REC_STORE_FILE)) {
        
        USB_DETECT_PHASE USBCurrentStates = 0;

        AHC_USBGetStates(&USBCurrentStates, AHC_USB_GET_PHASE_CURRENT);

        if ((USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) && 
            (AHC_TRUE == AHC_HostUVCVideoIsEnabled())) {
            MMPS_UVCRECD_OpenRecdFile();
        }
    }
    #endif
    
    if (MMP_IsPrmCamExist()) {
    
        sRet = MMPS_3GPRECD_StartRecord();
        if (sRet != MMP_ERR_NONE) {
            extern MMP_ULONG m_VidRecdID;

            sRet = MMPS_3GPRECD_StopRecord();
            
            if (m_VidRecdID != INVALID_ENC_ID) { // Workaround
                MMPD_VIDENC_DeInitInstance(m_VidRecdID);
            }
            AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet);    
            AHC_VIDEO_GetRecordModeThrow(AHC_VIDRECD_MODE_API_START_RECORD);
            return AHC_FALSE;
        }
    }
    
    #if (AHC_SHAREENC_SUPPORT)                
    if (AHC_VIDEO_GetRecordModeFirstTimeRecord() == AHC_TRUE) {
        m_ulDualPreEncodeTime = DUAL_RECORD_WRITE_PRETIME*1000 + EM_VR_TIME_LIMIT_OFFSET;
        printc("Enable Dual Pre Record: %d ms\r\n", m_ulDualPreEncodeTime);
        AHC_VIDEO_SetSharePreEncTimeLimit(m_ulDualPreEncodeTime);
        MMPS_3GPRECD_DualEncPreRecord(m_ulDualPreEncodeTime);
        #if (GPS_CONNECT_ENABLE && (GPS_FUNC & FUNC_VIDEOSTREM_INFO))
        AHC_GPS_StartSHAREPreRec(m_ulDualPreEncodeTime);
        #endif
        #if (GSENSOR_CONNECT_ENABLE && (GSENSOR_FUNC & FUNC_VIDEOSTREM_INFO))
        AHC_Gsensor_StartSHAREPreRec(m_ulDualPreEncodeTime);
        #endif
    }
    #endif
            
    if (MMP_IsUSBCamExist()) {
    
        USB_DETECT_PHASE USBCurrentStates = 0;

        AHC_USBGetStates(&USBCurrentStates, AHC_USB_GET_PHASE_CURRENT);
        
        if ((AHC_VIDEO_GetRecordModeFirstTimeRecord() == AHC_TRUE) &&
            (USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) && 
            (AHC_TRUE == AHC_HostUVCVideoIsEnabled())) {
            
            if (!AHC_HostUVCVideoRecordStart()){ 
                AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); 
                AHC_VIDEO_GetRecordModeThrow(AHC_VIDRECD_MODE_API_START_RECORD);
                return AHC_FALSE;
            }
        }
    }
    
    if (MMP_IsScdCamExist()) {
        
        if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_ENCODE_H264) {
            if (!AHC_VIDEO_DualRecordStart()) {
                AHC_PRINT_RET_ERROR(0,0); 
                AHC_VIDEO_GetRecordModeThrow(AHC_VIDRECD_MODE_API_START_RECORD);
                return AHC_FALSE;
            }
        }
    }

#if (DUALENC_SUPPORT) && (SUPPORT_SHARE_REC == 0)
    // No matter rear cam is connected or not, MMPS_3GPRECD_StartAllRecord must be called in DUALENC_SUPPORT case.
    if ((AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_FALSE) & DUAL_REC_ENCODE_H264) ||
        (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) & DUAL_REC_ENCODE_H264)) {
        // Dual encode start record together
        sRet = MMPS_3GPRECD_StartAllRecord();
        if (sRet != MMP_ERR_NONE){ 
            AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet);
            AHC_VIDEO_GetRecordModeThrow(AHC_VIDRECD_MODE_API_START_RECORD);
            return AHC_FALSE;
        } 
    }
#endif
               
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordWaitDCFDone
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordWaitDCFDone(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    UINT32      ulDCFInitTimeout = 0;
    AHC_BOOL    bDCFInited = AHC_FALSE;
    
    /* Initialize DCF DB */
    ulDCFInitTimeout = 600; // Timeout value is 6 sec.
    
    do {
        AHC_OS_SleepMs(10);
        bDCFInited = AHC_UF_IsDBMounted();
    } while ((AHC_FALSE == bDCFInited) && (--ulDCFInitTimeout > 0));

    if (0 == ulDCFInitTimeout) { printc(FG_RED("[Wait DCF init timeout!]\r\n")); }
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordEnterPreEncode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordEnterPreEncode(void)
{
    UINT32      Param = 0;
    MMP_ULONG   ulRealPreencodeTime = 0;
    MMP_ULONG	ulConfigPreRecordTime = 0;
    MMP_ERR     sRet = MMP_ERR_NONE;
    AHC_BOOL    ahcRet = AHC_TRUE;
    
    /* Enter PreEncode Flow */
    AIHC_VIDEO_GetMovieCfgEx(0, AHC_VIDEO_PRERECORD_LENGTH, &Param);
    
    if((uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE) && (uiGetParkingModeEnable() == AHC_TRUE))
    	ulConfigPreRecordTime = uiGetParkingCfg()->ulPreRecordLengthInMs;
    else
    	ulConfigPreRecordTime = Param;
    
    #if (UVC_HOST_VIDEO_ENABLE) && (DUALENC_SUPPORT) 
    if (MMP_IsUSBCamExist()) {
        if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) & DUAL_REC_ENCODE_H264) {
        
            USB_DETECT_PHASE USBCurrentStates = 0;
        
            AHC_USBGetStates(&USBCurrentStates, AHC_USB_GET_PHASE_CURRENT);
        
            if ((USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) && 
                (AHC_TRUE == AHC_HostUVCVideoIsEnabled())) {
                
                AHC_HostUVCVideoDualEncodeSetting();
            }
        }
    }
    #endif

    if (MMP_IsScdCamExist()) {
        if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_ENCODE_H264) {
            AHC_VIDEO_SetDualEncSetting();
        }
    }

    MMPS_3GPRECD_GetAllEncPreRecordTime(ulConfigPreRecordTime, &ulRealPreencodeTime);    	
    printc("Enable Video Pre Record: %d ms\r\n", ulRealPreencodeTime);

    sRet = MMPS_3GPRECD_PreRecord(ulRealPreencodeTime);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    
    if (MMP_IsUSBCamExist())
    {
        #if (UVC_HOST_VIDEO_ENABLE) && (DUALENC_SUPPORT)
        if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) & DUAL_REC_ENCODE_H264) 
        {
            USB_DETECT_PHASE USBCurrentStates = 0;

            AHC_USBGetStates(&USBCurrentStates,AHC_USB_GET_PHASE_CURRENT);
            
            if ((USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) && 
                (AHC_TRUE == AHC_HostUVCVideoIsEnabled())) {
            
                if (MMPS_3GPRECD_DualEncPreRecord(ulRealPreencodeTime)) { 
                    AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); 
                    return AHC_FALSE;
                }
            }
        }
        #endif
    }
    
    if (MMP_IsScdCamExist()) {
        if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_ENCODE_H264) {
            if (MMPS_3GPRECD_DualEncPreRecord(ulRealPreencodeTime)) { 
                AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); 
                return AHC_FALSE;
            }
        }
    }
    
    AHC_VIDEO_RecordStartWriteInfo();
        
    printc("========AHC_VIDRECD_PRERECD=============\r\n");

    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordExceptionHandler
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordExceptionHandler(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    UINT32      ulExceptionCode;

    ulExceptionCode = AHC_VIDEO_GetRecordModeCatch();
    
    switch(ulExceptionCode) {
    case AHC_VIDRECD_MODE_API_START_RECORD:
        AHC_VIDEO_SetRecordModePreAddFilenameFails();
        break;
    default:
        printc(FG_RED("There is no corresponding exception handler!\r\n"));
        break;
    }
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeRegisterInit
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeRegisterInit(void *pfExternalAPI)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    if (NULL == pfExternalAPI) {
        AHC_PRINT_RET_ERROR(0, 0);
        ahcRet = AHC_FALSE;
        return ahcRet;
    }

    pfAhcVidSetRecModeInit = (pfAHC_VIDEO_SetRecordModeAPI)pfExternalAPI;
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeRegisterUnInit
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeRegisterUnInit(void *pfExternalAPI)
{
    AHC_BOOL ahcRet = AHC_TRUE;

    if (NULL == pfExternalAPI) {
        AHC_PRINT_RET_ERROR(0, 0);
        ahcRet = AHC_FALSE;
        return ahcRet;
    }

    pfAhcVidSetRecModeUnInit = (pfAHC_VIDEO_SetRecordModeAPI)pfExternalAPI;
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeRegisterAction
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeRegisterAction( AHC_VIDRECD_FLOW_TYPE   ahcVidRecFlowType, 
                                                AHC_VIDRECD_MODE_API    ahcVidRecModeAPI, 
                                                void*                   pfExternalAPI)
{
    AHC_BOOL ahcRet = AHC_TRUE;
    
    if (ahcVidRecFlowType >= AHC_VIDRECD_FLOW_TYPE_MAX) {
        AHC_PRINT_RET_ERROR(0, 0);
        ahcRet = AHC_FALSE;
        return ahcRet;
    }

    if (NULL == pfExternalAPI) {
        AHC_PRINT_RET_ERROR(0, 0);
        ahcRet = AHC_FALSE;
        return ahcRet;
    }
    
    gpfAhcVidSetRecModeActoinList[ahcVidRecFlowType].pfahcVidSetRecModeAPI[ahcVidRecModeAPI] = (pfAHC_VIDEO_SetRecordModeAPI)pfExternalAPI;
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordModeExecActionList
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_SetRecordModeExecActionList(AHC_VIDRECD_FLOW_TYPE ahcVidRecFlowType)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    UINT32      ulLoop = 0;
    pfAHC_VIDEO_SetRecordModeAPI pfahcCurVidSetRecModeAPI = NULL;
    
    if (ahcVidRecFlowType >= AHC_VIDRECD_FLOW_TYPE_MAX) {
        AHC_PRINT_RET_ERROR(0, 0);
        ahcRet = AHC_FALSE;
        return ahcRet;
    }

    AHC_VIDEO_GetRecordModeClearException();
    
    for (ulLoop = 0; ulLoop < AHC_VIDRECD_MODE_API_EXCEPTION_HANDLER; ++ulLoop) {
        
        pfahcCurVidSetRecModeAPI = gpfAhcVidSetRecModeActoinList[ahcVidRecFlowType].pfahcVidSetRecModeAPI[ulLoop];
        
        if (NULL == pfahcCurVidSetRecModeAPI) { 
            continue; 
        }

        ahcRet = pfahcCurVidSetRecModeAPI();
        
        if (ahcRet != AHC_TRUE) {
            AHC_PRINT_RET_ERROR(0, 0);
            printc("Flow Type:%d, API:%d Error\r\n", ahcVidRecFlowType, ulLoop);
            
            pfahcCurVidSetRecModeAPI = gpfAhcVidSetRecModeActoinList[ahcVidRecFlowType].pfahcVidSetRecModeAPI[AHC_VIDRECD_MODE_API_EXCEPTION_HANDLER];
            
            if (NULL != pfahcCurVidSetRecModeAPI){
                // Exception handler.
                ahcRet = pfahcCurVidSetRecModeAPI();
            }
            
            ahcRet = AHC_FALSE;
            return ahcRet; 
        }
    }
    
    return ahcRet;
}
     
//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_GetCmdState
//  Description : Member function of gAhcVidRecdCmdStateXXXX
//------------------------------------------------------------------------------
AHC_VIDRECD_CMD AHC_VIDEO_GetCmdState(void* pthis)
{
    AHC_VIDEO_RECD_CMD_STATE *pAhcVidRecdCmdState = (AHC_VIDEO_RECD_CMD_STATE *)pthis;
    
    return pAhcVidRecdCmdState->ahcVidRecdCmdState;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RecdCmdStateExecIdle
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_RecdCmdStateExecIdle(void* pthis, AHC_VIDRECD_CMD ahcNewVidRecdCmdState)
{
    AHC_VIDEO_RECD_CONTEXT  *pVidRecdContext = (AHC_VIDEO_RECD_CONTEXT *)pthis;
    AHC_BOOL                ahcRet = AHC_TRUE;
    
    static UINT8            bStateIdleDone = 0;
    
    if (NULL == pthis) { AHC_PRINT_RET_ERROR(0, 0); return;}

    switch(ahcNewVidRecdCmdState) {
    case AHC_VIDRECD_IDLE:               
        if (1 == bStateIdleDone) { AHC_PRINT_RET_ERROR(0, 0); return;}
        // Stop preview
        ahcRet = AHC_VIDEO_StopRecordModeExStopPreview();
        ahcRet = AHC_VIDEO_StopRecordModeExStopSensor();

        ahcRet = pfAhcVidSetRecModeUnInit();
        bStateIdleDone = 1;
        return;
        break;
    case AHC_VIDRECD_INIT:
        pVidRecdContext->pfSetState(&gAhcVidRecdCmdStateInit);
        bStateIdleDone = 0;
        break;        
    case AHC_VIDRECD_START:           
    case AHC_VIDRECD_PRERECD:
    case AHC_VIDRECD_STOP: // TBD                        
    default:
        AHC_PRINT_RET_ERROR(0, 0);
        return;
        break;            
    }

    // Transfer to next state.
    pVidRecdContext->pfSetRecordMode(pthis, ahcNewVidRecdCmdState);
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RecdCmdStateExecInit
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_RecdCmdStateExecInit(void* pthis, AHC_VIDRECD_CMD ahcNewVidRecdCmdState)
{
    AHC_VIDEO_RECD_CONTEXT  *pVidRecdContext = (AHC_VIDEO_RECD_CONTEXT *)pthis;
    AHC_BOOL                ahcRet = AHC_TRUE;
    
    static UINT8            bStateInitDone = 0;
    
    if (NULL == pthis) { AHC_PRINT_RET_ERROR(0, 0); return;}

    switch(ahcNewVidRecdCmdState) {
    case AHC_VIDRECD_IDLE:     
        pVidRecdContext->pfSetState(&gAhcVidRecdCmdStateIdle);
        bStateInitDone = 0;
        break;   
    case AHC_VIDRECD_INIT:
        if (1 == bStateInitDone) { AHC_PRINT_RET_ERROR(0, 0); return;}
        ahcRet = pfAhcVidSetRecModeInit();
        bStateInitDone = 1;
        return;
        break;
    case AHC_VIDRECD_START:
        pVidRecdContext->pfSetState(&gAhcVidRecdCmdStateStart);
        bStateInitDone = 0;
        break;  
    case AHC_VIDRECD_PRERECD:
        pVidRecdContext->pfSetState(&gAhcVidRecdCmdStatePreRecd);
        bStateInitDone = 0;
        break;      
    case AHC_VIDRECD_STOP: // TBD                        
    default:
        AHC_PRINT_RET_ERROR(0, 0);
        return;
        break;            
    }

    // Transfer to next state.
    pVidRecdContext->pfSetRecordMode(pthis, ahcNewVidRecdCmdState);
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RecdCmdStateExecStart
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_RecdCmdStateExecStart(void* pthis, AHC_VIDRECD_CMD ahcNewVidRecdCmdState)
{
    AHC_VIDEO_RECD_CONTEXT  *pVidRecdContext = (AHC_VIDEO_RECD_CONTEXT *)pthis;
    AHC_BOOL                ahcRet = AHC_TRUE;
    VIDENC_FW_STATUS        sMergerStatus;
    
    static UINT8            bStateStartDone = 0;
    
    if (NULL == pthis) { AHC_PRINT_RET_ERROR(0, 0); return;}

    switch(ahcNewVidRecdCmdState) {            
    case AHC_VIDRECD_START:           
        switch(bStateStartDone) {
        case 0: // First time record.
            ahcRet = AHC_VIDEO_SetRecordWaitDCFDone();       

            AHC_VIDEO_SetRecordModeFirstTimeRecord(AHC_TRUE);
            
            MMPS_3GPRECD_GetRecordStatus(&sMergerStatus);
            
            if (VIDENC_FW_STATUS_PREENCODE == sMergerStatus){
                ahcRet = AHC_VIDEO_SetRecordModeExecActionList(AHC_VIDRECD_FLOW_TYPE_PRERECORD_TO_RECORD);   
            }
            else {
                ahcRet = AHC_VIDEO_SetRecordModeExecActionList(AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_RECORD);   
            }

            bStateStartDone = 1;
            return;
            break;
        case 1: // Seamless record.
            AHC_VIDEO_SetRecordModeFirstTimeRecord(AHC_FALSE);
            ahcRet = AHC_VIDEO_SetRecordModeExecActionList(AHC_VIDRECD_FLOW_TYPE_SEAMLESS_START_NEXT_RECORD);
            bStateStartDone = 1;
            return;
            break;
        default:  
            AHC_PRINT_RET_ERROR(0, 0); 
            return;
            break;
        }
        return;
        break;
    case AHC_VIDRECD_STOP:                                
        pVidRecdContext->pfSetState(&gAhcVidRecdCmdStateStop);
        bStateStartDone = 0;
        break;
    case AHC_VIDRECD_IDLE:     
    case AHC_VIDRECD_INIT:
    case AHC_VIDRECD_PRERECD:
    default:
        AHC_PRINT_RET_ERROR(0, 0);
        return;
        break;            
    }

    // Transfer to next state.
    pVidRecdContext->pfSetRecordMode(pthis, ahcNewVidRecdCmdState);   
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RecdCmdStateExecPreRecd
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_RecdCmdStateExecPreRecd(void* pthis, AHC_VIDRECD_CMD ahcNewVidRecdCmdState)
{
    AHC_VIDEO_RECD_CONTEXT  *pVidRecdContext = (AHC_VIDEO_RECD_CONTEXT *)pthis;
    AHC_BOOL                ahcRet = AHC_TRUE;

    static UINT8            bStatePreRecdDone = 0;
    
    if (NULL == pthis) { AHC_PRINT_RET_ERROR(0, 0); return;}

    switch(ahcNewVidRecdCmdState) {    
    case AHC_VIDRECD_PRERECD:        
        if (1 == bStatePreRecdDone) { AHC_PRINT_RET_ERROR(0, 0); return;}

        AHC_VIDEO_SetRecordModeFirstTimeRecord(AHC_TRUE);
        ahcRet = AHC_VIDEO_SetRecordModeExecActionList(AHC_VIDRECD_FLOW_TYPE_PREVIEW_TO_PRERECORD);                                
        bStatePreRecdDone = 1;
        return;
        break;
    case AHC_VIDRECD_START:                                
        pVidRecdContext->pfSetState(&gAhcVidRecdCmdStateStart);
        bStatePreRecdDone = 0;
        break;      
    case AHC_VIDRECD_STOP:                                
        pVidRecdContext->pfSetState(&gAhcVidRecdCmdStateStop);
        bStatePreRecdDone = 0;
        break;      
    case AHC_VIDRECD_IDLE:     
    case AHC_VIDRECD_INIT:
    default:
        AHC_PRINT_RET_ERROR(0, 0);
        return;
        break;            
    }

    // Transfer to next state.
    pVidRecdContext->pfSetRecordMode(pthis, ahcNewVidRecdCmdState);   
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RecdCmdStateExecStop
//  Description : 
//------------------------------------------------------------------------------
void AHC_VIDEO_RecdCmdStateExecStop(void* pthis, AHC_VIDRECD_CMD ahcNewVidRecdCmdState)
{
    AHC_VIDEO_RECD_CONTEXT  *pVidRecdContext = (AHC_VIDEO_RECD_CONTEXT *)pthis;
    AHC_BOOL                ahcRet = AHC_TRUE;

    static UINT8            bStateStopDone = 0;
    
    if (NULL == pthis) { AHC_PRINT_RET_ERROR(0, 0); return;}

    switch(ahcNewVidRecdCmdState) {                
    case AHC_VIDRECD_STOP:   
        if (1 == bStateStopDone) { AHC_PRINT_RET_ERROR(0, 0); return;}
        
        ahcRet = AHC_VIDEO_StopRecordModeExStopRec();
        bStateStopDone = 1;
        return;
        break;
    case AHC_VIDRECD_IDLE:    
        pVidRecdContext->pfSetState(&gAhcVidRecdCmdStateIdle);
        bStateStopDone = 0;            
        break;
    case AHC_VIDRECD_START:
        pVidRecdContext->pfSetState(&gAhcVidRecdCmdStateStart);
        bStateStopDone = 0;
        break;
    case AHC_VIDRECD_PRERECD:
        pVidRecdContext->pfSetState(&gAhcVidRecdCmdStatePreRecd);
        bStateStopDone = 0;
        break;      
    case AHC_VIDRECD_INIT:
    default:            
        AHC_PRINT_RET_ERROR(0, 0);
        return;
        break;            
    }

    // Transfer to next state.
    pVidRecdContext->pfSetRecordMode(pthis, ahcNewVidRecdCmdState);   
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RecdContextSetState
//  Description : Member function of gAhcVidRecdContext
//------------------------------------------------------------------------------
void AHC_VIDEO_RecdContextSetState(AHC_VIDEO_RECD_CMD_STATE *pNextState)
{
    gAhcVidRecdContext.pAhcCurVidRecdCmdState = pNextState;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RecdContextGetState
//  Description : Member function of gAhcVidRecdContext
//------------------------------------------------------------------------------
AHC_VIDEO_RECD_CMD_STATE* AHC_VIDEO_RecdContextGetState(void)
{
    return gAhcVidRecdContext.pAhcCurVidRecdCmdState;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RecdContextSetRecordMode
//  Description : Member function of gAhcVidRecdContext
//------------------------------------------------------------------------------
void AHC_VIDEO_RecdContextSetRecordMode(void* pthis, AHC_VIDRECD_CMD bStartRecord)
{
    AHC_VIDEO_RECD_CONTEXT *pVidRecdContext = (AHC_VIDEO_RECD_CONTEXT *)pthis;
    
    if (NULL == pthis) { AHC_PRINT_RET_ERROR(0, 0); return;}

    pVidRecdContext->pAhcCurVidRecdCmdState->pfStateExec(pthis, bStartRecord);
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_RestartRecMode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_RestartRecMode(void)
{
    AHC_VIDRECD_CMD ahcVidRecdCmd = AHC_VIDRECD_IDLE;
    AHC_BOOL        ahcRet;
   
    // Make sure Video record state is already in START.
    ahcVidRecdCmd = AHC_VIDEO_GetCmdState((void *)(AHC_VIDEO_RecdContextGetState()));    
    if (AHC_VIDRECD_START != ahcVidRecdCmd) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); return AHC_FALSE;}
    
    ahcRet = AHC_VIDEO_SetRecordMode(AHC_VIDRECD_START);
    
    return ahcRet;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_SetRecordMode
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set VideoR previrw / record

 Set VideoR previrw / record

 @param[in] bAudioOn        encode audio or not
 @param[in] bStartRecord    Preview or record
 @retval AHC_BOOL
*/
AHC_BOOL AHC_VIDEO_SetRecordMode(AHC_VIDRECD_CMD eVidRecCmd)
{
    AHC_BOOL ahcRet = AHC_TRUE;
       
    // Initiate pAhcCurVidRecdCmdState as gAhcVidRecdCmdStateIdle
    if (NULL == gAhcVidRecdContext.pfGetState()) {
        gAhcVidRecdContext.pfSetState(&gAhcVidRecdCmdStateIdle);
    }

    gAhcVidRecdContext.pfSetRecordMode((void *)(&gAhcVidRecdContext), eVidRecCmd);    

    return ahcRet;    
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_StopRecordModeExStopRec
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_StopRecordModeExStopRec(void)
{
    #if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
    MMP_ULONG 	        ulRecordingTime;
    #endif
    UINT32 		        TimeStampOp;
    AHC_FS_ATTRIBUTE    attribute;
    AHC_FS_FILETIME     timestructure;
    UINT32              ulpFileSize;
    AHC_BOOL            ahcRet = AHC_TRUE;

    /* Stop Seamless */
    if (AHC_VIDEO_IsVRSeamless() == AHC_TRUE) {
        MMPS_3GPRECD_StartSeamless(MMP_FALSE);
        AHC_VIDEO_SetVRSeamless(AHC_FALSE);
    }

	#if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
    MMPS_3GPRECD_GetRecordTime(&ulRecordingTime);
	#endif
    
    /* Stop Record Flow */
	if (!(CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_NONE))) {
        MMPS_3GPRECD_StopRecord();
    }
    
	AHC_VIDEO_SetMovieConfig(0, AHC_AUD_PRERECORD_DAC, AHC_TRUE);

	#if (UVC_HOST_VIDEO_ENABLE)
	if (MMP_IsUSBCamExist()) {
        AHC_HostUVCVideoRecordStop();
	}
	#endif
    
    /* Close GPS/Gsensor File */
	#if (GPS_CONNECT_ENABLE == 1)
    if (AHC_GPS_Module_Attached())
    {
        #if (GPS_RAW_FILE_ENABLE == 1)
        UINT8 bGps_En;

        if (AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGps_En) == AHC_TRUE) {
            switch (bGps_En) {
            case RECODE_GPS_OFF:
            case RECODE_GPS_IN_VIDEO:
                // NOP
                break;
            default:
                GPSCtrl_GPSRawClose();
                break;
            }
        }
        #endif

        #if (GPS_KML_FILE_ENABLE == 1)
        GPSCtrl_KMLGen_Write_TailAndClose();
        #endif
    }
	#endif

    #if (GPS_RAW_FILE_ENABLE == 1)
    GPSCtrl_GPSRawWrite_Close();
    #endif
    #if (GSENSOR_RAW_FILE_ENABLE == 1 && GPS_CONNECT_ENABLE == 0)
    GPSCtrl_GPSRawClose();
    #endif

    #if (GPS_CONNECT_ENABLE == 1)
    if (AHC_GPS_Module_Attached())
    {
        #if (GPS_RAW_FILE_EMER_EN == 1)
        UINT8 bGps_En;

        if (AHC_Menu_SettingGetCB((char*)COMMON_KEY_GPS_RECORD_ENABLE, &bGps_En) == AHC_TRUE) {
            switch (bGps_En) {
            case RECODE_GPS_OFF:
            case RECODE_GPS_IN_VIDEO:
                // NOP
                break;
            default:
                GPSCtrl_GPSRawClose_Emer();
                break;
            }
        }
        #endif
    }
    #endif

    #if(GSENSOR_RAW_FILE_ENABLE == 1 && GPS_CONNECT_ENABLE == 0 && GPS_RAW_FILE_EMER_EN == 1)
    GPSCtrl_GPSRawClose_Emer();
    #endif
    
    /* Add File to DCF DB */
    if (!uiGetParkingModeEnable())
    {
        #if (DCF_DB_COUNT > 1)
        AHC_UF_SelectDB(DCF_DB_TYPE_1ST_DB);
		#endif
		
		if (!CAM_CHECK_PRM(PRM_CAM_NONE)) {
            AHC_UF_PostAddFile(gusCurVideoDirKey, (INT8*)gpbCurVideoFileName);
            #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
            AHC_VIDEO_StoreJpgThumbnail(VIDENC_STREAMTYPE_VIDRECD);
            #endif
        }
        
        #if (VIDRECD_MULTI_TRACK == 0)
        if (!CAM_CHECK_SCD(SCD_CAM_NONE))
        {
            if (AHC_FS_FileDirGetInfo((INT8*)m_CurVRRearFullName, STRLEN((INT8*)m_CurVRRearFullName), &attribute, &timestructure, &ulpFileSize) == AHC_ERR_NONE) {
                AHC_UF_PostAddFile(gusCurVideoDirKey, (INT8*)m_RearVideoRFileName);
                #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
                AHC_VIDEO_StoreJpgThumbnail(VIDENC_STREAMTYPE_DUALENC);
                #endif                    
            }
        }

        if (!CAM_CHECK_USB(USB_CAM_NONE) && (m_CurVRUSBHFullName[0] != 0))
        {
            if (AHC_FS_FileDirGetInfo((INT8*)m_CurVRUSBHFullName, STRLEN((INT8*)m_CurVRUSBHFullName), &attribute, &timestructure, &ulpFileSize) == AHC_ERR_NONE) {
                AHC_UF_PostAddFile(gusCurVideoDirKey, (INT8*)m_USBHVideoRFileName);
                #if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
                AHC_VIDEO_StoreJpgThumbnail(VIDENC_STREAMTYPE_UVCRECD);
                #endif                    
            }
        }
        #endif
    }
    
    /* Modify the RTC Time and FileName */
    #if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
    {
        AHC_RTC_TIME    RtcTime;
        int             nSecondOffset;

        AHC_RTC_GetTime(&RtcTime);

        if ((m_VideoRecStartRtcTime.uwYear <= RTC_DEFAULT_YEAR) && (RtcTime.uwYear > RTC_DEFAULT_YEAR)) {
            
            nSecondOffset = -1*(ulRecordingTime/1000);
            AHC_DT_ShiftRtc(&RtcTime, nSecondOffset);

            printc("AHC_VIDEO_StopRecordMode:AHC_UF_Rename::Old:%d-%d-%d:%d-%d-%d New:%d-%d-%d:%d-%d-%d \r\n",
                    m_VideoRecStartRtcTime.uwYear,
                    m_VideoRecStartRtcTime.uwMonth,
                    m_VideoRecStartRtcTime.uwDay,
                    m_VideoRecStartRtcTime.uwHour,
                    m_VideoRecStartRtcTime.uwMinute,
                    m_VideoRecStartRtcTime.uwSecond,
                    RtcTime.uwYear,
                    RtcTime.uwMonth,
                    RtcTime.uwDay,
                    RtcTime.uwHour,
                    RtcTime.uwMinute,
                    RtcTime.uwSecond);

            AHC_UF_Rename(AHC_UF_GetDB(), &m_VideoRecStartRtcTime, &RtcTime);

            m_VideoRecStartRtcTime = RtcTime;
        }
    }
    #endif

	#if (AHC_EMERGENTRECD_SUPPORT) 
	if (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_MOVE_FILE)
	{
        if (AHC_VIDEO_GetNormal2Emergency())
        {
            UINT8           bCreateNewDir;
            UINT32          Param;
            VIDMGR_CONTAINER_TYPE ContainerType;
            UINT16          DirKey;
            char            FilePathName[MAX_FILE_NAME_SIZE];
            INT8            DirName[32];
            INT8            FileName[32];
            UINT32          StrLen;
            
            AHC_VIDEO_SetNormal2Emergency(AHC_FALSE);

            MEMSET(FilePathName, 0, sizeof(FilePathName));
            MEMSET(FileName, 0, sizeof(FileName));

            AHC_UF_GetName2(&m_VideoRecStartRtcTime, FilePathName, FileName, &bCreateNewDir);
            
            AIHC_VIDEO_GetMovieCfgEx(0, AHC_CLIP_CONTAINER_TYPE, &Param);
            ContainerType = Param;

            STRCAT(FilePathName, EXT_DOT);

            if (ContainerType == VIDMGR_CONTAINER_3GP) {
                STRCAT(FilePathName, MOVIE_3GP_EXT);
            }
            else {
                STRCAT(FilePathName, MOVIE_AVI_EXT);
            }

            printc("Normal2Emer File: %s \n", FilePathName);

            MEMSET(DirName,0,sizeof(DirName));
            MEMSET(FileName,0,sizeof(FileName));
            
            GetPathDirStr(DirName, sizeof(DirName), FilePathName);
            
            StrLen = AHC_StrLen(FilePathName) - AHC_StrLen(DirName) - 1;
            
            MEMCPY(FileName, FilePathName + AHC_StrLen(DirName) + 1, StrLen);

            AHC_UF_MoveFile(DCF_DB_TYPE_1ST_DB, DCF_DB_TYPE_3RD_DB, DirKey, FileName, AHC_TRUE);
        }
    }
	#endif
    
    /* Protect File if Needed */
    if (AHC_Protect_GetType() != AHC_PROTECT_NONE)
    {
        #ifdef CFG_CUS_VIDEO_PROTECT_PROC
        CFG_CUS_VIDEO_PROTECT_PROC();
        #else
        AHC_Protect_SetVRFile(AHC_PROTECTION_CUR_FILE, AHC_Protect_GetType());
        AHC_Protect_SetType(AHC_PROTECT_NONE);
        #endif
    }

    /* Disable Sticker */
    AHC_GetCaptureConfig(ACC_DATE_STAMP, &TimeStampOp);

    if (TimeStampOp & AHC_ACC_TIMESTAMP_ENABLE_MASK) {
        MMPS_DSC_SetSticker(NULL, NULL); // CHECK
    }
    
    #if (FS_FORMAT_FREE_ENABLE)
    // Note: Below setting must be set after MMPS_3GPRECD_StopRecord() which will return after finished writing video file.
    MMPS_FORMATFREE_EnableWrite(0); // Disable "Format Free Write" which will not update FAT table
    #endif
    
    return ahcRet;    
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_StopRecordModeExStopPreview
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_StopRecordModeExStopPreview(void)
{
	UINT32		i = 0;
    AHC_BOOL	ahcRet = AHC_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;

    /* Stop Preview Flow */       
    #if (UVC_HOST_VIDEO_ENABLE)
    if (MMP_IsUSBCamExist()) {
    
        USB_DETECT_PHASE USBCurrentStates = 0;

        AHC_USBGetStates(&USBCurrentStates, AHC_USB_GET_PHASE_CURRENT);

        if (USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) {
            AIHC_UVCPreviewStop();
        }
   
        #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
        if (MMP_IsSupportDecMjpegToPreview()) {
            sRet = MMPS_3GPRECD_DecMjpegPreviewStop(gsAhcPrmSensor); // CHECK,not use USBH_SENSOR ?
            if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
        }
        #endif 
        
        ahcRet = AHC_HostUVCResetFBMemory();
        
        #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
        if (MMP_IsSupportDecMjpegToPreview()) {
            ahcRet = AHC_HostUVCResetMjpegToPreviewJpegBuf();
        }
        #endif
    }
    #endif
    
    if (MMP_IsPrmCamExist()) {
        sRet = MMPS_3GPRECD_PreviewStop(gsAhcPrmSensor);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    }

    if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) {
        MMPS_Sensor_EnableTVDecSrcTypeDetection(MMP_FALSE);
        sRet = MMPS_3GPRECD_2ndSnrPreviewStop(gsAhcScdSensor);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}
    }
    else if (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
        sRet = MMPS_3GPRECD_2ndSnrPreviewStop(gsAhcScdSensor);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}      
    }
    
    AHC_VIDEO_SetRecWithWNR(AHC_FALSE);

    // Disable all Icon, so that the first shot of still capture mode can update Icon attribute.
    for (i = 0 ; i < MMP_STICKER_ID_NUM; i++) {
    	MMPS_3GPRECD_EnableSticker(i, MMP_FALSE);
    }
    
    return ahcRet;    
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_StopRecordModeExStopSensor
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_StopRecordModeExStopSensor(void)
{
    AHC_BOOL    ahcRet = AHC_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;
    
    /* Shut Down Sensor */
    sRet = MMPS_Sensor_PowerDown(gsAhcPrmSensor);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    
    if (MMP_IsScdCamExist()) {
        sRet = MMPS_Sensor_PowerDown(gsAhcScdSensor);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    }
    
    return ahcRet;    
}

#if 0
void _____VR_Sticker_Function_________(){ruturn;} //dummy
#endif

#if ( VIDEO_STICKER_INDEX1_EN == 1 )

#define ICON_WIDTH_LIMIT (1920)
static UINT8 byStickerZeroLine[ICON_WIDTH_LIMIT] = {0};

//------------------------------------------------------------------------------
//  Function    : TransferStampFromIndex8ToIndex1
//  Description : Transfer Stamp buffer from Index8 to Index1
//                NOTE: Since it's done by SW, it may take lots of time.
//                      Please reduce buffer size as small as possible.
//------------------------------------------------------------------------------
void TransferStampFromIndex8ToIndex1(UINT8* pbyStampAddr, UINT16 uwStampW, UINT16 uwStampH)
{
    UINT32 i, j;
    UINT32 ulLineStartAddress = 0;
    UINT32 ulLineEndAddress = 0;
    UINT32 ulLineStartAddress_Index1 = 0;
    //UINT32 ulTime0 = OSTimeGet();

    if( uwStampW > ICON_WIDTH_LIMIT )
    {
        printc(FG_RED("Warning: The width of DateTime Stamp buffer is over HW limitation.(%d)\r\n"), uwStampW);
    }

    if( uwStampW%8 != 0 )
    {
        printc(FG_RED("Warning: The width of DateTime Stamp buffer is not multiple of 8. (%d)\r\n"), uwStampW);
    }
    
    for( j=0; j < uwStampH; j++ )
    {
        ulLineStartAddress = uwStampW*j;
        
        //Check if any color in this line
        if( memcmp( byStickerZeroLine, &pbyStampAddr[ulLineStartAddress], uwStampW ) != 0 )
        {
            ulLineEndAddress = ulLineStartAddress + uwStampW;
            ulLineStartAddress_Index1 = ulLineStartAddress / 8;
            
            for( i=ulLineStartAddress; i < ulLineEndAddress; i+=8 )
            {
                pbyStampAddr[i/8] = \
                    ((pbyStampAddr[i+7])? 0x80 : 0x00) | \
                    ((pbyStampAddr[i+6])? 0x40 : 0x00) | \
                    ((pbyStampAddr[i+5])? 0x20 : 0x00) | \
                    ((pbyStampAddr[i+4])? 0x10 : 0x00) | \
                    ((pbyStampAddr[i+3])? 0x08 : 0x00) | \
                    ((pbyStampAddr[i+2])? 0x04 : 0x00) | \
                    ((pbyStampAddr[i+1])? 0x02 : 0x00) | \
                    ((pbyStampAddr[i+0])? 0x01 : 0x00) ;
            }
        }
        else
        {
            memset( &pbyStampAddr[ulLineStartAddress/8], 0x0, uwStampW/8 );
        }
    }

    //Flush data from cache to DRAM
    MMPF_MMU_FlushDCacheMVA( (MMP_ULONG)pbyStampAddr, uwStampW*uwStampH/8 );

    //printc("TransferStampFromIndex8ToIndex1: %d ms\r\n", OSTimeGet()-ulTime0);
}

#endif

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_ConfigRecTimeStamp
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_ConfigRecTimeStamp(UINT32 TimeStampOp, AHC_RTC_TIME* sRtcTime, AHC_BOOL bInitIconBuf)
{
#if (AHC_ENABLE_VIDEO_STICKER)

    MMP_STICKER_ATTR    StickerAttribute;
    UINT32              uiSrcAddr;
    UINT16              uwSrcW;
    UINT16              uwSrcH;
    UINT16              uwTempColorFormat;
    UINT16              uwColorFormat = MMP_ICON_COLOR_INDEX8;
    UINT32              StickerX, StickerY;
    #if (DUALENC_SUPPORT)
    UINT32              ulDualStickerX, ulDualStickerY, ulLoop;
    MMP_ULONG           ulDualStickerAddr = 0;
    MMP_ERR             sRet = MMP_ERR_NONE;
    UINT16              usGuiW, usGuiH, usGuiFormat;
    UINT32              ulGuiAddr, ulStickerSrcAddr;
    #endif
    UINT32              uiOSDid, uiSubOSDid;
    static AHC_BOOL     bVRTimeStampRunning = AHC_FALSE;

    OS_CRITICAL_INIT();
   
    OS_ENTER_CRITICAL();

    // Add a protection to avoid this function is called by multi-task and make broken stamp
    if (bVRTimeStampRunning == AHC_TRUE)
    {
        OS_EXIT_CRITICAL();
        return AHC_FALSE;
    }

    bVRTimeStampRunning = AHC_TRUE;
    OS_EXIT_CRITICAL();

    AIHC_DrawReservedOSD(AHC_TRUE);

    m_uiVideoStampBufIndex ^= 1;

    if (m_uiVideoStampBufIndex == 0) {
        uiOSDid = DATESTAMP_PRIMARY_OSD_ID;
        uiSubOSDid = DATESTAMP_PRIMARY_OSD_ID_SUB;
    }
    else {
        uiOSDid = DATESTAMP_THUMB_OSD_ID;
        uiSubOSDid = DATESTAMP_THUMB_OSD_ID_SUB;
    }
    
    #if (DUALENC_SUPPORT)
    // Warning: We should spearate AIT MJPEG+H264 or NV12+H264
    if ((AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_ENCODE_H264) ||
        (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_TRUE) & DUAL_REC_ENCODE_H264)) {
        DrawVideo_GetDualTimeStampPosition(uiOSDid, TimeStampOp, &uiSrcAddr, &uwSrcW, &uwSrcH, &uwTempColorFormat, &ulDualStickerX, &ulDualStickerY);
    }
    else if (CAM_CHECK_USB(USB_CAM_AIT)) {
        if (MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_MJPEG_H264) {
            DrawVideo_GetDualTimeStampPosition(uiOSDid, TimeStampOp, &uiSrcAddr, &uwSrcW, &uwSrcH, &uwTempColorFormat, &ulDualStickerX, &ulDualStickerY);
        }
    }
    #endif
    
    DrawVideo_TimeStamp(uiOSDid, TimeStampOp, sRtcTime, 
                        &uiSrcAddr, &uwSrcW, &uwSrcH, 
                        &uwTempColorFormat, 
                        &StickerX,
                        &StickerY);

    AIHC_DrawReservedOSD(AHC_FALSE);
    
    #if (SW_STICKER_EN == 1)
    AHC_SWSticker_SetPosition(  uwSrcW,        uwSrcH,
                                CaptureWidth,  CaptureHeight,
                                StickerX,      StickerY,
                                DATESTAMP_PRIMARY_OSD_ID, DATESTAMP_THUMB_OSD_ID);

    if (bInitIconBuf == AHC_TRUE) {
        AHC_SWSticker_SetCBFuncPtr((void*)AHC_SWSticker_MoveBuf);
        AHC_OSDLoadWinPalette(DATESTAMP_PRIMARY_OSD_ID);
    }

    #else

    #if ( VIDEO_STICKER_INDEX1_EN == 1 )
    uwColorFormat = MMP_ICON_COLOR_INDEX1;
    TransferStampFromIndex8ToIndex1( (UINT8*)uiSrcAddr, uwSrcW, uwSrcH );
    #endif

    StickerAttribute.ubStickerId	  = MMP_STICKER_PIPE0_ID0;
    StickerAttribute.ulBaseAddr       = uiSrcAddr;
    StickerAttribute.usStartX         = StickerX;
    StickerAttribute.usStartY         = StickerY;
    StickerAttribute.usWidth          = uwSrcW;
    StickerAttribute.usHeight         = uwSrcH;
    StickerAttribute.colorformat      = uwColorFormat;
    StickerAttribute.ulTpColor        = GUI_BLACK;
    StickerAttribute.bTpEnable        = MMP_TRUE;
    StickerAttribute.bSemiTpEnable    = MMP_FALSE;
    StickerAttribute.ubIconWeight     = 16;
    StickerAttribute.ubDstWeight      = 0;

 	#if (DUALENC_SUPPORT)
    if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
    {
    	//Change SW sticker to HW sticker when Sonix rear cam only.
    }
    else if ((AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_SCD, AHC_TRUE) & DUAL_REC_ENCODE_H264) ||
        (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_TRUE) & DUAL_REC_ENCODE_H264) ||
        (CAM_CHECK_USB(USB_CAM_AIT) && MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_MJPEG_H264))// Warning: We should spearate AIT MJPEG+H264 or NV12+H264
    #else
    if (0)
    #endif
    {
        sRet = MMPS_3GPRECD_SetSWStickerAttribute(StickerAttribute.usWidth, StickerAttribute.usHeight, (MMP_USHORT)ulDualStickerX, (MMP_USHORT)ulDualStickerY);
        if (sRet != MMP_ERR_NONE){AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet);}
        
        AIHC_DrawReservedOSD(AHC_TRUE);
        
        AHC_OSDGetBufferAttr((MMP_UBYTE)(uiOSDid+ 2), &usGuiW, &usGuiH, &usGuiFormat, &ulGuiAddr);  

        ulDualStickerAddr = DRAM_CACHE_VA(ulGuiAddr);
        ulStickerSrcAddr = DRAM_CACHE_VA(StickerAttribute.ulBaseAddr);

        for (ulLoop = 0; ulLoop < (StickerAttribute.usWidth * StickerAttribute.usHeight); ++ulLoop) {
            *((MMP_UBYTE *)ulDualStickerAddr + ulLoop) = (*((MMP_UBYTE *)ulStickerSrcAddr + ulLoop) != 0) ? 0xFF : 0x00;
        }

        MMPF_MMU_FlushDCache();
        MMPS_3GPRECD_SetSWStickerAddress(ulGuiAddr);
        
        AIHC_DrawReservedOSD(AHC_FALSE);
    }
    #endif // SW_STICKER_EN

	#if ( VIDEO_STICKER_INDEX1_EN == 1 )
    StickerAttribute.colorformat = MMP_ICON_COLOR_INDEX1;
    TransferStampFromIndex8ToIndex1((UINT8*)uiSrcAddr, uwSrcW, uwSrcH);
    #endif
    
    MMPS_3GPRECD_SetSticker(&StickerAttribute);

    if (bInitIconBuf == AHC_TRUE) {
        AHC_OSDLoadIconIndexColorTable(uiOSDid, StickerAttribute.ubStickerId, StickerAttribute.colorformat);
        AHC_OSDLoadWinPalette(uiOSDid);
    }

    MMPS_3GPRECD_EnableSticker(StickerAttribute.ubStickerId, MMP_TRUE);

    bVRTimeStampRunning = AHC_FALSE;

#endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_UpdateRecTimeStamp
//  Description :
//------------------------------------------------------------------------------
void AHC_VIDEO_UpdateRecTimeStamp(AHC_RTC_TIME* sRtcTime)
{
#if (AHC_ENABLE_VIDEO_STICKER)
    UINT32      TimeStampOp;
    UINT8       ubCurUIState = 0;
    UI_STATE_ID ubParentUIState = 0;    
    
    ubCurUIState = uiGetCurrentState();
    StateModeGetParent(ubCurUIState, &ubParentUIState);

    if (UI_STATE_UNSUPPORTED != ubParentUIState) {
        ubCurUIState = ubParentUIState;
    }
    
    if (!AHC_IsInVideoMode()) {
		return;
	}
    
    AHC_GetCaptureConfig(ACC_DATE_STAMP, &TimeStampOp);

    if (TimeStampOp & AHC_ACC_TIMESTAMP_ENABLE_MASK) {
        AHC_VIDEO_ConfigRecTimeStamp(TimeStampOp, sRtcTime, MMP_TRUE);
    }
#endif
}

#if 0
void _____VR_Misc_Function_________(){ruturn;} //dummy
#endif

#if (AHC_VR_THUMBNAIL_CREATE_JPG_FILE == 1)
//------------------------------------------------------------------------------
//  Function    : AHC_VIDEO_StoreJpgThumbnail
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_VIDEO_StoreJpgThumbnail(VIDENC_STREAMTYPE ulStreamType)
{
#if (SUPPORT_VR_THUMBNAIL)
    if (ulStreamType == VIDENC_STREAMTYPE_VIDRECD) {
        RTNA_DBG_Str0(FG_GREEN("[NOR0]Store "));
        RTNA_DBG_Str0(m_CurThumbJpgFullName);
        RTNA_DBG_Str0("\r\n");
        MMPS_3GPRECD_StoreJpgThumb(m_CurThumbJpgFullName, STRLEN(m_CurThumbJpgFullName), VIDENC_STREAMTYPE_VIDRECD);
    }
    else if (ulStreamType == VIDENC_STREAMTYPE_EMERGENCY) {
        RTNA_DBG_Str0(FG_GREEN("[EMG0]Store "));
        RTNA_DBG_Str0(m_chEmerThumbFullName);
        RTNA_DBG_Str0("\r\n");
        MMPS_3GPRECD_StoreJpgThumb(m_chEmerThumbFullName, STRLEN(m_chEmerThumbFullName), VIDENC_STREAMTYPE_EMERGENCY);        
    }
    else if (ulStreamType == VIDENC_STREAMTYPE_UVCRECD) {
        RTNA_DBG_Str0(FG_GREEN("[NOR1]Store "));
        RTNA_DBG_Str0(m_CurThumbJpgFullName_USBH);
        RTNA_DBG_Str0("\r\n");
        MMPS_3GPRECD_StoreJpgThumb(m_CurThumbJpgFullName_USBH, STRLEN(m_CurThumbJpgFullName_USBH), VIDENC_STREAMTYPE_UVCRECD);        
    }
    else if (ulStreamType == VIDENC_STREAMTYPE_UVCEMERG) {
        RTNA_DBG_Str0(FG_GREEN("[EMG1]Store "));
        RTNA_DBG_Str0(m_chEmerThumbFullName_USBH);
        RTNA_DBG_Str0("\r\n");
        MMPS_3GPRECD_StoreJpgThumb(m_chEmerThumbFullName_USBH, STRLEN(m_chEmerThumbFullName_USBH), VIDENC_STREAMTYPE_UVCEMERG);        
    }
    else if (ulStreamType == VIDENC_STREAMTYPE_DUALEMERG) {
        RTNA_DBG_Str0(FG_GREEN("[EMG1]Store "));
        RTNA_DBG_Str0(m_chEmerThumbFullName_Rear);
        RTNA_DBG_Str0("\r\n");
        MMPS_3GPRECD_StoreJpgThumb(m_chEmerThumbFullName_Rear, STRLEN(m_chEmerThumbFullName_Rear), VIDENC_STREAMTYPE_DUALEMERG);        
    }    
#endif
    return AHC_TRUE;
}
#endif // AHC_VR_THUMBNAIL_CREATE_JPG_FILE
