/**
 @file mmps_3gprecd.h
 @brief Header File for the Host 3GP RECORDER API.
 @author Will Tseng
 @version 1.0
*/

#ifndef _MMPS_3GPRECD_H_
#define _MMPS_3GPRECD_H_

//===============================================================================
//
//                               INCLUDE FILE
//
//===============================================================================

#include "mmp_lib.h"
#include "config_fw.h"
#include "mmp_graphics_inc.h"
#include "mmp_rawproc_inc.h"
#include "mmp_icon_inc.h"
#include "mmp_snr_inc.h"
#include "mmp_mjpeg_inc.h"
#include "mmp_media_def.h"
#include "mmp_vidrec_inc.h"
#include "mmps_dsc.h"
#include "mmps_rtc.h"
#include "mmps_system.h"
#include "mmpd_mp4venc.h"
#include "mmpd_3gpmgr.h"
#include "mmpf_mp4venc.h"

//===============================================================================
//
//                               MACRO DEFINE
//
//===============================================================================

#define VR_MAX_RAWSTORE_BUFFER_NUM      (3)
#define VR_MIN_RAWSTORE_BUFFER_NUM      (2)

#define VR_MAX_PREVIEW_NUM              (VIF_SENSOR_MAX_NUM)
#define VR_MAX_ENCODE_NUM               (2)

#define VR_MAX_H264_STREAM_NUM          (SENSOR_MAX_NUM)//(MAX_WIFI_STREAM_NUM)

#define VR_MAX_CAPTURE_WIDTH            (1920)
#define VR_MAX_CAPTURE_HEIGHT           (1080)

#define VR_MAX_THUMBNAIL_WIDTH          (640)
#define VR_MAX_THUMBNAIL_HEIGHT         (480)

#define VR_MIN_MV_WIDTH                 (1920)
#define VR_MIN_MV_HEIGHT                (1088)

#define VR_MIN_TIME_TO_RECORD           (2)     ///< 2 seconds

#define VR_QUERY_STATES_TIMEOUT         (15000) ///< millisecond.

//===============================================================================
//
//                               ENUMERATION
//
//===============================================================================

// Video Stream Index (Record File)
typedef enum _MMPS_3GPRECD_FILESTREAM_ID {
    MMPS_3GPRECD_FILESTREAM_NORMAL = 0x00,
    MMPS_3GPRECD_FILESTREAM_DUAL,
    MMPS_3GPRECD_FILESTREAM_MAX_NUM
} MMPS_3GPRECD_FILESTREAM_ID;

// Video Format (Must be sync with PCAM_USB_VIDEO_FORMAT)
typedef enum _MMPS_3GPRECD_VIDEO_FORMAT {
    MMPS_3GPRECD_VIDEO_FORMAT_OTHERS = 0x00,			///< Video format, none
    MMPS_3GPRECD_VIDEO_FORMAT_H264,                     ///< Video format, H.264
    MMPS_3GPRECD_VIDEO_FORMAT_MJPEG,                    ///< Video format, MJPEG
    MMPS_3GPRECD_VIDEO_FORMAT_YUV422,                   ///< Video format, YUV422
    MMPS_3GPRECD_VIDEO_FORMAT_YUV420
} MMPS_3GPRECD_VIDEO_FORMAT;

// Audio Format, Sample rate, Bit rate
typedef enum _MMPS_3GPRECD_AUDIO_OPTION {
    MMPS_3GPRECD_AUDIO_AAC_BASE 		= 0x00,			///< Audio AAC format
    MMPS_3GPRECD_AUDIO_AAC_16K_32K		= 0x02,			///< AAC    16KHz with 32kbps
    MMPS_3GPRECD_AUDIO_AAC_16K_64K,						///< AAC    16KHz with 64kbps
    MMPS_3GPRECD_AUDIO_AAC_22d05K_64K,					///< AAC 22.05KHz with 64kbps
    MMPS_3GPRECD_AUDIO_AAC_22d05K_128K,					///< AAC 22.05KHz with 128kbps
    MMPS_3GPRECD_AUDIO_AAC_32K_64K,						///< AAC 32KHz with 64kbps
    MMPS_3GPRECD_AUDIO_AAC_32K_128K,					///< AAC 32KHz with 128kbps
    MMPS_3GPRECD_AUDIO_AAC_48K_128K,					///< AAC 48KHz with 128kbps
    MMPS_3GPRECD_AUDIO_AAC_44d1K_64K,					///< AAC 44.1KHz with 64kbps
    MMPS_3GPRECD_AUDIO_AAC_44d1K_128K,					///< AAC 44.1KHz with 128kbps

    MMPS_3GPRECD_AUDIO_AMR_BASE 		= 0x10,			///< Audio AMR format
    MMPS_3GPRECD_AUDIO_AMR_4d75K,						///< AMR 4.75KHz with 8kbps
    MMPS_3GPRECD_AUDIO_AMR_5d15K,						///< AMR 5.15KHz with 8kbps
    MMPS_3GPRECD_AUDIO_AMR_12d2K,						///< AMR 12.2KHz with 8kbps

    //ADPCM section, need to sync with MMPF_SetADPCMEncMode
    MMPS_3GPRECD_AUDIO_ADPCM_16K_22K,					///< ADPCM 16KHz with 22kbps
    MMPS_3GPRECD_AUDIO_ADPCM_32K_22K,					///< ADPCM 32KHz with 22kbps
    MMPS_3GPRECD_AUDIO_ADPCM_44d1K_22K,					///< ADPCM 44.1KHz with 22kbps
    MMPS_3GPRECD_AUDIO_ADPCM_48K_22K,                   ///< ADPCM 48KHz with 22kbps

    MMPS_3GPRECD_AUDIO_MP3_32K_128K,                    ///< MP3 32KHz with 128kbps
    MMPS_3GPRECD_AUDIO_MP3_44d1K_128K,                  ///< MP3 44.1KHz with 128kbps

    MMPS_3GPRECD_AUDIO_PCM_16K,                         ///< Raw PCM 16KHz
    MMPS_3GPRECD_AUDIO_PCM_32K                          ///< Raw PCM 32KHz
} MMPS_3GPRECD_AUDIO_OPTION;

// Audio Format
typedef enum _MMPS_3GPRECD_AUDIO_FORMAT {
    MMPS_3GPRECD_AUDIO_FORMAT_AAC = 0x00,               ///< Video encode with AAC audio
    MMPS_3GPRECD_AUDIO_FORMAT_AMR,                      ///< Video encode with AMR audio
    MMPS_3GPRECD_AUDIO_FORMAT_ADPCM,           	        ///< Video encode with ADPCM audio
    MMPS_3GPRECD_AUDIO_FORMAT_MP3,                      ///< video encode with MP3 audio
    MMPS_3GPRECD_AUDIO_FORMAT_PCM                       ///< video encode with raw PCM audio
} MMPS_3GPRECD_AUDIO_FORMAT;

// Audio Data Type
typedef enum _MMPS_3GPRECD_AUDIO_DATATYPE {
    MMPS_3GPRECD_NO_AUDIO_DATA = 0,
    MMPS_3GPRECD_REC_AUDIO_DATA,
    MMPS_3GPRECD_SILENCE_AUDIO_DATA
} MMPS_3GPRECD_AUDIO_DATATYPE;

// Encode Parameters
typedef enum _MMPS_3GPRECD_PARAMETER {
    MMPS_3GPRECD_PARAMETER_PREVIEWMODE = 0x0,			///< Encode sensor mode
    MMPS_3GPRECD_PARAMETER_SRCMODE,						///< Card or memory mode
    MMPS_3GPRECD_PARAMETER_VIDEO_FORMAT,				///< Video format
    MMPS_3GPRECD_PARAMETER_AUDIO_FORMAT,				///< Audio format
    MMPS_3GPRECD_PARAMETER_RESOLUTION,					///< Video resolution
    MMPS_3GPRECD_PARAMETER_BITRATE,
    MMPS_3GPRECD_PARAMETER_FRAME_RATE,                  ///< Video frame rate
    MMPS_3GPRECD_PARAMETER_GOP,
    MMPS_3GPRECD_PARAMETER_PROFILE,                     ///< Video codec profile
    MMPS_3GPRECD_PARAMETER_DRAM_END                     ///< End of DRAM address for video encode
} MMPS_3GPRECD_PARAMETER;

// Stream Callback Type
typedef enum _MMPS_3GPRECD_STREAMCB_TYPE {
    MMPS_3GPRECD_STREAMCB_VIDMOVE = 0,                  ///< move video stream
    MMPS_3GPRECD_STREAMCB_VIDUPDATE,                    ///< update video stream
    MMPS_3GPRECD_STREAMCB_AUDMOVE,                      ///< move audio stream
    MMPS_3GPRECD_STREAMCB_AUDUPDATE                     ///< update audio stream
} MMPS_3GPRECD_STREAMCB_TYPE;

#if (SUPPORT_VUI_INFO)
// Shutter Type
typedef enum _MMPS_SEI_SHUTTER_TYPE {
	MMPS_SEI_SHUTTER_1920x1080_29d97 = 0,
	MMPS_SEI_SHUTTER_1280x720_59d94,
	MMPS_SEI_SHUTTER_1280x720_29d97,
	MMPS_SEI_SHUTTER_848x480_29d97,
	MMPS_SEI_SHUTTER_NONE
} MMPS_SEI_SHUTTER_TYPE;
#endif

// Y-Pipe Frame Type
typedef enum _MMPS_3GPRECD_Y_FRAME_TYPE {
    MMPS_3GPRECD_Y_FRAME_TYPE_NONE = 0,
    MMPS_3GPRECD_Y_FRAME_TYPE_VMD,
    MMPS_3GPRECD_Y_FRAME_TYPE_ADAS,
    MMPS_3GPRECD_Y_FRAME_TYPE_MAX
} MMPS_3GPRECD_Y_FRAME_TYPE;

// Zoom Path
typedef enum _MMPS_3GPRECD_ZOOM_PATH {
    MMPS_3GPRECD_ZOOM_PATH_NONE = 0,
    MMPS_3GPRECD_ZOOM_PATH_PREV,
    MMPS_3GPRECD_ZOOM_PATH_RECD,
    MMPS_3GPRECD_ZOOM_PATH_BOTH,
    MMPS_3GPRECD_ZOOM_PATH_MAX
} MMPS_3GPRECD_ZOOM_PATH;

// For 3GP preview/encode path selection
typedef enum _MMP_3GPRECD_PATH {
    MMP_3GP_PATH_2PIPE 			= 0,
    MMP_3GP_PATH_LDC_LB_SINGLE,
    MMP_3GP_PATH_LDC_LB_MULTI,
    MMP_3GP_PATH_LDC_MULTISLICE, 
    MMP_3GP_PATH_INVALID 		= 0xFF
} MMP_3GPRECD_PATH;

// For 3GP still capture source selection
typedef enum _MMP_3GPRECD_CAPTURE_SRC {
    MMP_3GPRECD_CAPTURE_SRC_FRONT_CAM   = 0x01,
    MMP_3GPRECD_CAPTURE_SRC_REAR_CAM    = 0x02
} MMP_3GPRECD_CAPTURE_SRC;

// For Emergency recording action flow
typedef enum _MMP_3GPRECD_EMERG_ACTION {
    MMP_3GPRECD_EMERG_NO_ACT = 0,
    MMP_3GPRECD_EMERG_DUAL_FILE,
    MMP_3GPRECD_EMERG_SWITCH_FILE,
    MMP_3GPRECD_EMERG_MOVE_FILE,
    MMP_3GPRECD_EMERG_DEFAULT_ACT = MMP_3GPRECD_EMERG_NO_ACT
} MMP_3GPRECD_EMERG_ACTION;

//===============================================================================
//
//                               STRUCTURES
//
//===============================================================================

// Stream callback buffer information
typedef struct _MMPS_3GPRECD_STREAM_INFO {
    MMP_ULONG               baseaddr;
    MMP_ULONG               segsize;
    MMP_ULONG               startaddr;
    MMP_ULONG               availsize;
} MMPS_3GPRECD_STREAM_INFO;

// Frame rate
typedef struct _MMPS_3GPRECD_FRAMERATE {
    MMP_ULONG               usVopTimeIncrement;                             ///< Video encode VOP time increment
    MMP_ULONG               usVopTimeIncrResol;                             ///< Video encode VOP time resolution
} MMPS_3GPRECD_FRAMERATE;

// GOP
typedef struct _MMPS_3GPRECD_GOP {
    MMP_USHORT              usPFrameCount;                                  ///< P frame count in 1 GOP
    MMP_USHORT              usBFrameCount;                                  ///< Consecutive B frame count
} MMPS_3GPRECD_GOP;

// Still Capture information
typedef struct _MMPS_3GPRECD_STILL_CAPTURE_INFO {
    MMP_BYTE                *bFileName;
    MMP_ULONG               ulFileNameLen;
    MMP_USHORT              usWidth;
    MMP_USHORT              usHeight;
    MMP_USHORT              usThumbWidth;
    MMP_USHORT              usThumbHeight;
    MMP_BOOL                bThumbEn;
    MMP_BOOL                bExifEn;
    MMP_BOOL                bTargetCtl;
    MMP_BOOL                bLimitCtl;
    MMP_ULONG               bTargetSize;
    MMP_ULONG               bLimitSize;
    MMP_USHORT              bMaxTrialCnt;
    MMP_DSC_JPEG_QUALITY    Quality;
    MMP_3GPRECD_CAPTURE_SRC sCaptSrc;
    MMP_BYTE                *bRearFileName;
    MMP_ULONG               ulRearFileNameLen;
} MMPS_3GPRECD_STILL_CAPTURE_INFO;

// Preview buffer information
typedef struct _MMPS_3GPRECD_PREVIEW_BUFINFO {
    MMP_ULONG               ulYBuf[4];                                      ///< Video encode preview Y buffer
    MMP_ULONG               ulUBuf[4];                                      ///< Video encode preview U buffer
    MMP_ULONG               ulVBuf[4];                                      ///< Video encode preview V buffer
    MMP_ULONG               ulRotateYBuf[4];                                ///< Video preview rotate DMA dst Y buffer
    MMP_ULONG               ulRotateUBuf[4];                                ///< Video preview rotate DMA dst U buffer
    MMP_ULONG               ulRotateVBuf[4];                                ///< Video preview rotate DMA dst V buffer
    MMP_USHORT              usRotateBufCnt;                                 ///< Video preview rotate DMA dst buffer count
    MMP_USHORT              usBufCnt;                                       ///< Video encode preview buffer count
} MMPS_3GPRECD_PREVIEW_BUFINFO;

// Motion Detection buffer information
typedef struct _MMPS_3GPRECD_MDTC_BUFINFO {
    MMP_ULONG               ulYBuf[2];                                      ///< MDTC source Y buffer (Pipe4)
    MMP_USHORT              usBufCnt;                                       ///< MDTC source buffer count
} MMPS_3GPRECD_MDTC_BUFINFO;

// Preview and display configuration
typedef struct _MMPS_3GPRECD_PREVIEW_DATA {
    MMP_USHORT  			usVidPreviewBufW[VIDRECD_PREV_MODE_MAX_NUM];    ///< Specify preview buffer width
    MMP_USHORT  			usVidPreviewBufH[VIDRECD_PREV_MODE_MAX_NUM];    ///< Specify preview buffer height
    MMP_SCAL_FIT_MODE		sFitMode[VIDRECD_PREV_MODE_MAX_NUM];            ///< Specify scaler fit mode

    //++ For Display Window/Device Attribute
    MMP_DISPLAY_COLORMODE 	DispColorFmt[VIDRECD_PREV_MODE_MAX_NUM];        ///< Preview color format
    MMP_DISPLAY_DEV_TYPE 	DispDevice[VIDRECD_PREV_MODE_MAX_NUM];          ///< Display mode of NORMAL, FLM or TV
    //MMP_DISPLAY_WIN_ID 	DispWinId[VIDRECD_PREV_MODE_MAX_NUM];           ///< Display window index
    MMP_USHORT 				usVidDispBufCnt[VIDRECD_PREV_MODE_MAX_NUM];     ///< preview pipe buffer count
    MMP_USHORT 				usVidDisplayW[VIDRECD_PREV_MODE_MAX_NUM];       ///< Preview pipe display window width
    MMP_USHORT 				usVidDisplayH[VIDRECD_PREV_MODE_MAX_NUM];       ///< Preview pipe display window height    
    MMP_USHORT 				usVidDispStartX[VIDRECD_PREV_MODE_MAX_NUM];     ///< Preview display window X offset
    MMP_USHORT 				usVidDispStartY[VIDRECD_PREV_MODE_MAX_NUM];     ///< Preview display window Y offset
    MMP_BOOL   				bVidDispMirror[VIDRECD_PREV_MODE_MAX_NUM];      ///< Mirror mode
    MMP_DISPLAY_ROTATE_TYPE VidDispDir[VIDRECD_PREV_MODE_MAX_NUM];          ///< Rotate mode
    //-- For Display Window/Device Attribute
    
    //++ For DMA rotate
    MMP_BOOL   				bUseRotateDMA[VIDRECD_PREV_MODE_MAX_NUM];       ///< Preview uses rotate DMA to rotate or not
    MMP_USHORT 				usRotateBufCnt[VIDRECD_PREV_MODE_MAX_NUM];      ///< Preview rotate DMA dst buffer count
    MMP_DISPLAY_ROTATE_TYPE	ubDMARotateDir[VIDRECD_PREV_MODE_MAX_NUM];	    ///< Preview rotate DMA direction
    //-- For DMA rotate
} MMPS_3GPRECD_PREVIEW_DATA;

// Record run-time change configuration
typedef struct _MMPS_3GPRECD_RUNTIME_CFG {
    MMP_USHORT              usVideoPreviewMode;                             ///< Index of video encode preview modes
    MMP_USHORT              usVideoMVBufResIdx;                             ///< Index of video encode MV buffer index
    MMP_USHORT              usVideoEncResIdx[VR_MAX_ENCODE_NUM];            ///< Index of video encode resolutions
    MMPS_3GPRECD_FRAMERATE  SnrInputFrameRate[VR_MAX_ENCODE_NUM];           ///< Sensor input frame rate settings
    MMPS_3GPRECD_FRAMERATE  VideoEncFrameRate[VR_MAX_ENCODE_NUM];           ///< Video encode frame rate settings
    MMPS_3GPRECD_FRAMERATE  ContainerFrameRate[VR_MAX_ENCODE_NUM];          ///< Container frame rate settings
    MMP_USHORT              usPFrameCount[VR_MAX_ENCODE_NUM];               ///< P frame count in 1 GOP
    MMP_USHORT              usBFrameCount[VR_MAX_ENCODE_NUM];               ///< Consecutive B frame count
    MMP_ULONG               ulBitrate[VR_MAX_ENCODE_NUM];                   ///< Video bitrate
    MMP_ULONG               ulAudBitrate;                                   ///< Audio bitrate
    MMP_ULONG               ulSizeLimit;                                    ///< Video stream size limit
    MMP_ULONG               ulTimeLimitMs;                                  ///< Video stream time limit in unit of ms
    MMP_ULONG               ulReservedSpace;                                ///< Reserved storage space which can't be used by recorder or other application
    VIDENC_SRCMODE          VideoSrcMode[VR_MAX_ENCODE_NUM];                ///< Memory mode or card mode
    VIDENC_PROFILE          VisualProfile[VR_MAX_ENCODE_NUM];               ///< Visual profile
    VIDENC_CURBUF_MODE      VidCurBufMode[VR_MAX_ENCODE_NUM];               ///< Video current buffer mode
    MMP_BOOL                bSlowMotionEn;                                  ///< Slow motion enable
} MMPS_3GPRECD_RUNTIME_CFG;

// Record preset configuration
typedef struct _MMPS_3GPRECD_PRESET_CFG {
    //++ For Preview 
    MMP_UBYTE            	previewpath[VR_MAX_PREVIEW_NUM];
    MMPS_3GPRECD_PREVIEW_DATA   previewdata[VR_MAX_PREVIEW_NUM];            ///< Customized preview configuration
    //-- For Preview 

    //++ For Encode 
    MMP_USHORT              usEncWidth[VIDRECD_RESOL_MAX_NUM];              ///< Video encode width by resolution
    MMP_USHORT              usEncHeight[VIDRECD_RESOL_MAX_NUM];             ///< Video encode height by resolution
    MMP_ULONG               ulFps30BitrateMap[VIDRECD_RESOL_MAX_NUM][VIDRECD_QLEVEL_MAX_NUM];	///< Bitrate map for 30fps
    MMP_ULONG               ulFps60BitrateMap[VIDRECD_RESOL_MAX_NUM][VIDRECD_QLEVEL_MAX_NUM];   ///< Bitrate map for 60fps
    MMPS_3GPRECD_FRAMERATE  framerate[VIDRECD_FPS_MAX_NUM];                 ///< Video encode frame rate settings
    //-- For Encode

    //++ For Feature
    MMP_BOOL                bAsyncMode;                                     ///< Async mode enable or not
    MMP_BOOL                bSeamlessMode;                                  ///< Support seamless recording or not
    MMP_BOOL                bFdtcEnable;                                    ///< Face detection enable or not
    MMP_BOOL                bRawPreviewEnable[VR_MAX_PREVIEW_NUM];          ///< Raw preview enable or not (Per Raw module)
    MMP_UBYTE			    ubRawPreviewBitMode[VR_MAX_PREVIEW_NUM];        ///< Raw preview bit mode (Per Raw module)
    MMP_BOOL                bStillCaptureEnable;                            ///< Still capture enable or not
    MMP_BOOL                bDualCaptureEnable;                             ///< Dual camera capture enable or not
    MMP_BOOL			    bH264WifiStreamEnable;                          ///< H264 WIFI Streaming Enable
    #if (SUPPORT_H264_WIFI_STREAM)
    MMP_ULONG			    ulMaxH264WifiStreamWidth;
    MMP_ULONG			    ulMaxH264WifiStreamHeight;
    #endif
    //-- For Feature

    //++ For Buffer Reserved Size 
    MMP_ULONG               ulRawStoreBufCnt;
    MMP_ULONG               ulTailBufSize;
    MMP_ULONG               ulVideoCompBufSize;
    MMP_ULONG               ul2ndVideoCompBufSize;
    MMP_ULONG               ulWifiVideoCompBufSize;
    MMP_ULONG               ulAudioCompBufSize;
    MMP_ULONG               ulUVCVidCompBufSize;
    #if (VIDRECD_MULTI_TRACK == 0)
    MMP_ULONG               ulUVCTailBufSize;
    #endif
    #if (UVC_EMERGRECD_SUPPORT == 1 && VIDRECD_MULTI_TRACK == 0)
    MMP_ULONG               ulUVCEmergTailBufSize;
    #endif
    #if (DUAL_EMERGRECD_SUPPORT == 1 && VIDRECD_MULTI_TRACK == 0)
    MMP_ULONG               ulDualEmergTailBufSize;
    #endif
    #if (SUPPORT_VR_REFIX_TAILINFO)
    MMP_ULONG               ulReFixTailBufSize;
    #endif
	//-- For Buffer Reserved Size 
} MMPS_3GPRECD_PRESET_CFG;

#if (SUPPORT_H264_WIFI_STREAM)
// Wifi stream run-time change configuration
typedef struct _MMP_H264_WIFI_RUNTIME_CFG {
    MMP_USHORT              usVideoEncResIdx;                               ///< Index of video encode resolutions
    MMPS_3GPRECD_FRAMERATE  SnrInputFrameRate;                              ///< Sensor input frame rate settings
    MMPS_3GPRECD_FRAMERATE  VideoEncFrameRate;                              ///< Video encode frame rate settings
    MMPS_3GPRECD_FRAMERATE  ContainerFrameRate;                             ///< Container frame rate settings
    MMP_USHORT              usPFrameCount;                                  ///< P frame count in 1 GOP
    MMP_USHORT              usBFrameCount;                                  ///< Consecutive B frame count
    MMP_ULONG               ulBitrate;                                      ///< Video bitrate
    VIDENC_SRCMODE          VideoSrcMode;                                   ///< Memory mode or card mode
    VIDENC_PROFILE          VisualProfile;                                  ///< Visual profile
    VIDENC_CURBUF_MODE      VidCurBufMode;                                  ///< Video Current Buffer Mode
} MMP_H264_WIFI_RUNTIME_CFG;

typedef struct _MMP_H264_WIFI_LINKATTR {
    MMP_IBC_PIPE_ATTR       IBCPipeAttr;
    MMP_IBC_LINK_TYPE       IBCLinkType;
    MMP_DISPLAY_DEV_TYPE    previewDev;
    MMP_DISPLAY_WIN_ID 	    winID;
    MMP_DISPLAY_ROTATE_TYPE rotateDir;
    MMP_SCAL_SOURCE         scalerSrc;
    MMP_UBYTE               IBCColorFormat;
} MMP_H264_WIFI_LINKATTR;

typedef struct _MMP_H264_WIFISTREAM_OBJ {
    MMP_SHORT               usEncID;
    MMP_BOOL                bEnableWifi;
    MMP_UBYTE               ubWifiSnrSel;
    MMP_ULONG               ulStreamType;
    MMP_PIPE_LINK           FctlLink;
    MMP_H264_WIFI_RUNTIME_CFG WifiEncModes;
    MMP_H264_WIFI_LINKATTR  LinkAttr;
} MMP_H264_WIFISTREAM_OBJ;
#endif

#if (SUPPORT_TIMELAPSE)

#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))

// For time lapse
typedef struct _MMPS_3GPRECD_AVI_LIST {
    MMP_ULONG               List;
    MMP_ULONG               ulSize;
    MMP_ULONG               FourCC;
} MMPS_3GPRECD_AVI_LIST;

typedef struct _MMPS_3GPRECD_AVI_MainHeader {
    MMP_ULONG               FourCC;
    MMP_ULONG               ulSize;   // always 0x38bytes
    MMP_ULONG               ulMSecPreFrame;
    MMP_ULONG               ulMaxByteRate;
    MMP_ULONG               ulReServed;
    MMP_ULONG               ulFlag;
    MMP_ULONG               ulFrameNum;
    MMP_ULONG               ulInitFrame;
    MMP_ULONG               ulStreamNum;
    MMP_ULONG               ulBufferSize;
    MMP_ULONG               ulWidth;
    MMP_ULONG               ulHeight;
    MMP_ULONG               ulScale;
    MMP_ULONG               ulRate;
    MMP_ULONG               ulStart;
    MMP_ULONG               ulLength;
} MMPS_3GPRECD_AVI_MainHeader;

typedef struct _MMPS_3GPRECD_AVI_StreamHeader {
    MMP_ULONG               FourCC;
    MMP_ULONG               ulSize;     // always 0x38bytes
    MMP_ULONG               Type;
    MMP_ULONG               Handler;
    MMP_ULONG               ulFlag;
    MMP_ULONG               ulReServed;
    MMP_ULONG               InitFrame;
    MMP_ULONG               ulScale;
    MMP_ULONG               ulRate;
    MMP_ULONG               ulStart;
    MMP_ULONG               ulLength;
    MMP_ULONG               ulBufferSize;
    MMP_ULONG               ulQuality;
    MMP_ULONG               ulSampleSize;
    MMP_ULONG               ulReServed2;
    MMP_ULONG               ulReServed3;
} MMPS_3GPRECD_AVI_StreamHeader;

typedef struct _MMPS_3GPRECD_AVI_StreamFormat {
    MMP_ULONG               FourCC;
    MMP_ULONG               ulSize;     // always 0x28bytes
    MMP_ULONG               ulSize2;    // always 0x28
    MMP_ULONG               ulWidth;
    MMP_ULONG               ulHeight;
    MMP_ULONG               ulPlaneAndBitCount;
    MMP_ULONG               ubCompression;
    MMP_ULONG               ulImageSize;
    MMP_ULONG               ulXPelPerMeter;
    MMP_ULONG               ulYPelPerMeter;
    MMP_ULONG               ulColorUse;
    MMP_ULONG               ulColorBit;
} MMPS_3GPRECD_AVI_StreamFormat;

typedef struct _MMPS_3GPRECD_AVI_Header {
    MMP_ULONG               ubFourCC;
    MMP_ULONG               ulSize;     // always 327680bytes
} MMPS_3GPRECD_AVI_Header;

typedef struct _MMPS_3GPRECD_AVI_IndexEntry {
    MMP_ULONG               ubFourCC;
    MMP_ULONG               ulFlag;
    MMP_ULONG               ulPos;
    MMP_ULONG               ulSize;
} MMPS_3GPRECD_AVI_IndexEntry;
#endif

// AHC Parameters
typedef struct _MMPS_3GPRECD_AHC_PREVIEW_INFO {
    MMP_BOOL                bUserDefine;
    MMP_BOOL             	bPreviewRotate;
    MMP_DISPLAY_ROTATE_TYPE	sPreviewDmaDir;
    MMP_SCAL_FIT_MODE		sFitMode;
    MMP_ULONG               ulPreviewBufW;
    MMP_ULONG              	ulPreviewBufH;
    MMP_ULONG               ulDispStartX;
    MMP_ULONG               ulDispStartY;
    MMP_ULONG               ulDispWidth;
    MMP_ULONG               ulDispHeight;
    MMP_DISPLAY_COLORMODE   sDispColor;
} MMPS_3GPRECD_AHC_PREVIEW_INFO;

typedef struct _MMPS_3GPRECD_AHC_VIDEO_INFO {
    MMP_BOOL                bUserDefine;
    MMP_SCAL_FIT_MODE       sFitMode;
    MMP_ULONG               ulVideoEncW;
    MMP_ULONG               ulVideoEncH;
} MMPS_3GPRECD_AHC_VIDEO_INFO;

//===============================================================================
//
//                               FUNCTION PROTOTYPES
//
//===============================================================================

/** @addtogroup MMPS_3GPRECD
@{
*/

MMPS_3GPRECD_PRESET_CFG* MMPS_3GPRECD_GetConfig(void);

/* Preview Function */
MMP_ERR MMPS_3GPRECD_SetPreviewPipe(MMP_UBYTE ubSnrSel, MMP_UBYTE ubPipe);
MMP_ERR MMPS_3GPRECD_CustomedPreviewAttr(MMP_UBYTE  ubSnrSel,
                                         MMP_BOOL 	bUserConfig,
										 MMP_BOOL 	bRotate,
										 MMP_UBYTE 	ubRotateDir,
										 MMP_UBYTE	sFitMode,
										 MMP_USHORT usBufWidth, MMP_USHORT usBufHeight, 
										 MMP_USHORT usStartX, 	MMP_USHORT usStartY,
                                      	 MMP_USHORT usWinWidth, MMP_USHORT usWinHeight);
MMP_ERR MMPS_3GPRECD_EnablePreviewPipe(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable, MMP_BOOL bCheckFrameEnd);
MMP_ERR MMPS_3GPRECD_PreviewStop(MMP_UBYTE ubSnrSel);
MMP_ERR MMPS_3GPRECD_PreviewStart(MMP_UBYTE ubSnrSel, MMP_BOOL bCheckFrameEnd);
MMP_ERR MMPS_3GPRECD_SetPreviewMode(MMP_USHORT usPreviewMode);
MMP_USHORT MMPS_3GPRECD_GetPreviewMode(void);
MMP_ULONG MMPS_3GPRECD_GetPreviewEndAddr(void);
MMP_ERR MMPS_3GPRECD_GetPreviewPipe(MMP_UBYTE ubSnrSel, MMP_IBC_PIPEID *pPipe);
MMP_ERR MMPS_3GPRECD_GetPreviewRes(MMP_UBYTE ubSnrSel, MMP_ULONG *width, MMP_ULONG *height);
MMP_ERR MMPS_3GPRECD_GetPreviewPipeStatus(MMP_UBYTE ubSnrSel, MMP_BOOL *bEnable);
MMP_ERR MMPS_3GPRECD_GetFrontCamBufForDualStreaming(MMP_ULONG *pulCompAddr, MMP_ULONG *pulCompSize, 
                                                    MMP_ULONG *pulLineBuf,	MMP_ULONG *pulLineSize);

/* 2nd Preview Function */
MMP_ERR MMPS_3GPRECD_2ndSnrPreviewStop(MMP_UBYTE ubSnrSel);
MMP_ERR MMPS_3GPRECD_2ndSnrPreviewStart(MMP_UBYTE ubSnrSel, MMP_BOOL bCheckFrameEnd);

/* Record Function */
MMP_ERR MMPS_3GPRECD_SetEncodePipe(MMP_UBYTE ubSnrSel, MMP_UBYTE ubPipe);
MMP_ERR MMPS_3GPRECD_AllocateMVBuffer(MMP_ULONG *ulCurBufPos, MMP_ULONG ulMVWidth, MMP_ULONG ulMVHeight);
MMP_ERR MMPS_3GPRECD_GetRecordPipeBufWH(MMP_UBYTE ubEncIdx, MMP_ULONG *pulBufW, MMP_ULONG *pulBufH);
MMP_ERR MMPS_3GPRECD_GetRecordPipe(MMP_UBYTE ubEncIdx, MMP_IBC_PIPEID *pPipe);
MMP_ERR MMPS_3GPRECD_GetStillCaptureAddr(MMP_ULONG *pulSramAddr, MMP_ULONG *pulDramAddr);
MMP_ERR MMPS_3GPRECD_GetStillCaptureEndAddr(MMP_ULONG *pulSramBufAddr, MMP_ULONG *pulDramBufAddr);
MMP_ERR MMPS_3GPRECD_SetMVBufResIdx(MMP_USHORT usResol);
MMP_ERR MMPS_3GPRECD_SetDecMjpegToEncodeAttr(MMP_UBYTE sFitMode, 
                                             MMP_USHORT usSrcWidth, MMP_USHORT usSrcHeight, 
                                             MMP_USHORT usEncWidth, MMP_USHORT usEncHeight);

MMP_ERR MMPS_3GPRECD_CustomedEncResol(MMP_UBYTE ubEncIdx, MMP_UBYTE sFitMode, MMP_USHORT usWidth, MMP_USHORT usHeight);
MMP_ERR MMPS_3GPRECD_EnableRecordPipe(MMP_BOOL bEnable);
MMP_ERR MMPS_3GPRECD_GetRecordPipeStatus(MMP_UBYTE ubEncIdx, MMP_BOOL *bEnable);
MMP_ERR MMPS_3GPRECD_SetFileName(VIDENC_STREAMTYPE usStreamType, MMP_BYTE bFileName[], MMP_USHORT usLength);
MMP_ERR MMPS_3GPRECD_SetUserDataAtom(VIDENC_STREAMTYPE usStreamType, MMP_BYTE AtomName[], MMP_BYTE UserDataBuf[], MMP_USHORT UserDataLength);
MMP_ERR MMPS_3GPRECD_SetStoragePath(VIDENC_SRCMODE SrcMode);
MMP_ERR MMPS_3GPRECD_SetReservedStorageSpace(MMP_ULONG ulReservedSize);
MMP_ERR MMPS_3GPRECD_SetFileSizeLimit(MMP_ULONG ulFileLimit);
MMP_ERR MMPS_3GPRECD_SetFileTimeLimit(MMP_ULONG ulTimeLimitMs);
MMP_ERR MMPS_3GPRECD_Set3GPCreateModifyTimeInfo(VIDENC_STREAMTYPE usStreamType, AUTL_DATETIME datetimenew);
MMP_ERR MMPS_3GPRECD_ChangeCurFileTimeLimit(MMP_ULONG ulTimeLimitMs);
MMP_ERR MMPS_3GPRECD_PreRecord(MMP_ULONG ulPreCaptureMs);
MMP_ERR MMPS_3GPRECD_StartRecord(void);
MMP_ERR MMPS_3GPRECD_StopRecord(void);
MMP_ERR MMPS_3GPRECD_PauseRecord(void);
MMP_ERR MMPS_3GPRECD_ResumeRecord(void);
MMP_ERR MMPS_3GPRECD_GetRecordedSize(MMP_ULONG *ulSize);
MMP_ERR MMPS_3GPRECD_Get3GPFileSize(MMP_ULONG *ulFileSize);
MMP_ERR MMPS_3GPRECD_GetEncodeCompBuf(VIDENC_STREAMTYPE usStreamType, MMP_ULONG *bufaddr, MMP_ULONG *bufsize);
MMP_ERR MMPS_3GPRECD_GetParameter(MMPS_3GPRECD_PARAMETER type, MMP_ULONG *ulValue);
MMP_ERR MMPS_3GPRECD_UpdateParameter(MMP_ULONG ulStreamType, MMPS_3GPRECD_PARAMETER type, void *param);
MMP_LONG MMPS_3GPRECD_GetExpectedRecordTime(MMP_ULONG64 ullSpace, MMP_ULONG ulVidBitRate, MMP_ULONG ulAudBitRate);
MMP_ERR MMPS_3GPRECD_GetRecordTime(MMP_ULONG *ulTime);
MMP_ERR MMPS_3GPRECD_SetFrameRatePara(MMP_ULONG                 ulEncId,
                                      MMPS_3GPRECD_FRAMERATE    *snr_fps,
                                      MMPS_3GPRECD_FRAMERATE    *enc_fps,
                                      MMPS_3GPRECD_FRAMERATE    *container_fps);
MMP_ERR MMPS_3GPRECD_SetEncResIdx(MMP_ULONG ulEncId, MMP_USHORT usResol);
MMP_ERR MMPS_3GPRECD_ModifyAVIListAtom(MMP_BOOL bEnable, MMP_BYTE *pStr);
MMP_ERR MMPS_3GPRECD_SetBitrate(MMP_ULONG ulEncId, MMP_ULONG ulBitrate);
MMP_ERR MMPS_3GPRECD_SetCurBufMode(MMP_ULONG ulEncId, VIDENC_CURBUF_MODE CurBufMode);
MMP_ERR MMPS_3GPRECD_SetAudioFormat(MMPS_3GPRECD_AUDIO_FORMAT Format, MMPS_3GPRECD_AUDIO_OPTION Option);
MMP_ERR MMPS_3GPRECD_SetAudioRecMode(MMPS_3GPRECD_AUDIO_DATATYPE mode);
MMP_ERR MMPS_3GPRECD_GetRecordStatus(VIDENC_FW_STATUS *retstatus);
MMP_ULONG MMPS_3GPRECD_GetContainerTailBufSize(void);
MMP_ERR MMPS_3GPRECD_SetPFrameCount(MMP_ULONG ulEncId, MMP_USHORT usFrameCount);
MMP_ERR MMPS_3GPRECD_SetBFrameCount(MMP_ULONG ulEncId, MMP_USHORT usFrameCount);
MMP_ERR MMPS_3GPRECD_SetProfile(MMP_ULONG ulEncId, VIDENC_PROFILE profile);
MMP_ERR MMPS_3GPRECD_GetFrame(MMP_USHORT usTargetWidth, MMP_USHORT usTargetHeight,
                              MMP_USHORT *pOutBuf, MMP_ULONG *ulSize);
MMP_ERR MMPS_3GPRECD_SetRecordSpeed(VIDENC_SPEED_MODE ubSpeedMode, VIDENC_SPEED_RATIO ubSpeedRatio);
MMP_ERR MMPS_3GPRECD_StartSeamless(MMP_BOOL bStart);
MMP_ERR MMPS_3GPRECD_SetStillCaptureMaxRes(MMP_USHORT usJpegW, MMP_USHORT usJpegH);
MMP_ERR MMPS_3GPRECD_StillCapture(MMPS_3GPRECD_STILL_CAPTURE_INFO *pCaptureInfo);
MMP_ERR MMPS_3GPRECD_SetContainerType(VIDMGR_CONTAINER_TYPE type);
MMP_ERR MMPS_3GPRECD_RegisterCallback (VIDMGR_EVENT Event, void *CallBack);
MMP_ERR MMPS_3GPRECD_SetSkipCntThreshold(MMP_USHORT threshold);
MMP_ERR MMPS_3GPRECD_SetVidRecdSkipModeParam(MMP_ULONG ulTotalCount, MMP_ULONG ulContinCount);
MMP_ERR MMPS_3GPRECD_Get3gpRecordingOffset(MMP_ULONG *ulTime);
MMP_ERR MMPS_3GPRECD_StartAllRecord(void);
MMP_ERR MMPS_3GPRECD_GetAllEncPreRecordTime(MMP_ULONG ulPreCaptureMs, MMP_ULONG *ulRealPreCaptureMs);
#if (SUPPORT_VUI_INFO)
MMP_ERR MMPS_3GPRECD_SetSEIShutterMode(MMPS_SEI_SHUTTER_TYPE ulMode);
#endif
MMP_ERR MMPS_3GPRECD_SetTime2FlushFSCache(MMP_ULONG time);
#if (SUPPORT_VR_REFIX_TAILINFO)
MMP_ERR MMPS_3GPRECD_CheckFile2Refix(void);
#endif

MMP_ERR MMPS_3GPRECD_SetH264EncUseMode(VIDENC_STREAMTYPE usStreamType, MMP_ULONG type);
MMP_ERR MMPS_3GPRECD_SetMuxer3gpConstantFps(MMP_BOOL bEnable);
MMP_ERR MMPS_3GPRECD_SetAVSyncMethod(VIDMGR_AVSYNC_METHOD usAVSyncMethod);
VIDMGR_AVSYNC_METHOD MMPS_3GPRECD_GetAVSyncMethod(void);

/* Zoom Function */
MMP_ERR     MMPS_3GPRECD_SetPreviewZoom(MMPS_3GPRECD_ZOOM_PATH sPath, MMP_PTZ_ZOOM_DIR sZoomDir, MMP_USHORT usCurZoomStep);
MMP_ERR     MMPS_3GPRECD_GetCurZoomStep(MMP_UBYTE ubPipe, MMP_USHORT *usZoomStepNum);
MMP_ERR     MMPS_3GPRECD_SetZoomConfig(MMP_USHORT usMaxSteps, MMP_USHORT usMaxRatio);
MMP_UBYTE   MMPS_3GPRECD_GetCurZoomStatus(void);

/* LDC Function */
MMP_ERR MMPS_3GPRECD_SetLdcRunMode(MMP_UBYTE ubRunMode);
MMP_ERR MMPS_3GPRECD_SetLdcResMode(MMP_UBYTE ubResMode, MMP_UBYTE ubFpsMode);
MMP_ERR MMPS_3GPRECD_GetLdcMaxOutRes(MMP_ULONG *pulMaxW, MMP_ULONG *pulMaxH);

/* Sticker Function */
MMP_ERR MMPS_3GPRECD_SetSticker(MMP_STICKER_ATTR *pStickerAtrribute);
MMP_ERR MMPS_3GPRECD_EnableSticker(MMP_STICKER_ID stickerID, MMP_BOOL bEnable);

/* TimeLapse Function */
#if (SUPPORT_TIMELAPSE)
MMP_ERR MMPS_3GPRECD_InitAVIFile(MMP_BYTE *bAviName, MMP_ULONG ulNameSize, MMP_ULONG ulWidth, MMP_ULONG ulHeight,
                                 MMP_ULONG CoedcFourCC, MMP_ULONG FrameRate, MMP_ULONG ulBitRate, MMP_BOOL bInit, MMP_ULONG *FileID);
MMP_ERR MMPS_3GPRECD_AVIAppendFrame(MMP_ULONG ulFID, MMP_UBYTE *pData, MMP_ULONG ulSize, MMP_ULONG64 *ulFileSize, MMP_ULONG *ulFrameNum);
#endif

/* Emergency Record Function */
MMP_ERR MMPS_3GPRECD_SetEmergFileName(MMP_BYTE bFileName[], MMP_USHORT usLength);
MMP_ERR MMPS_3GPRECD_EnableEmergentRecd(MMP_BOOL bEnable);
MMP_ERR MMPS_3GPRECD_StartEmergentRecd(MMP_BOOL bStopNormRecd);
MMP_ERR MMPS_3GPRECD_StopEmergentRecd(MMP_BOOL bBlocking);
MMP_ERR MMPS_3GPRECD_SetEmergentFileTimeLimit(MMP_ULONG ulTimeLimitMs);
MMP_ERR MMPS_3GPRECD_SetEmergentFileSizeLimit(MMP_ULONG ulSizeLimitBytes);
MMP_ERR MMPS_3GPRECD_SetEmergPreEncTimeLimit(MMP_ULONG ulTimeLimitMs);
MMP_ERR MMPS_3GPRECD_GetEmergentRecordingTime(MMP_ULONG *ulTime);
MMP_ERR MMPS_3GPRECD_GetEmergentRecordingOffset(MMP_ULONG *ulTime);
#if (UVC_EMERGRECD_SUPPORT)
MMP_ERR MMPS_3GPRECD_EnableUVCEmergentRecd(MMP_BOOL bEnable);
#endif
#if (DUAL_EMERGRECD_SUPPORT)
MMP_ERR     MMPS_3GPRECD_EnableDualEmergentRecd(MMP_BOOL bEnable);
MMP_BOOL    MMPS_3GPRECD_IsDualEmergentRecdEnable(void);
#endif
MMP_BOOL MMPS_3GPRECD_SetEmergActionType(MMP_3GPRECD_EMERG_ACTION emergact);
MMP_3GPRECD_EMERG_ACTION MMPS_3GPRECD_GetEmergActionType(void);

/* MultiStream Record Function */
MMP_ERR MMPS_3GPRECD_EnableDualRecd(MMP_BOOL bEnable);
MMP_ERR MMPS_3GPRECD_SetDualH264SnrId(MMP_UBYTE ubSnrSel);
MMP_ERR MMPS_3GPRECD_EnableDualH264Pipe(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable);
MMP_ERR MMPS_3GPRECD_DualEncPreRecord(MMP_ULONG ulPreCaptureMs);
MMP_ERR MMPS_3GPRECD_StartDualH264(void);
MMP_ERR MMPS_3GPRECD_StopDualH264(void);
MMP_ERR MMPS_3GPRECD_GetFrameRatePara(MMP_ULONG                 ulEncId,
                                      MMPS_3GPRECD_FRAMERATE    *snr_fps,
                                      MMPS_3GPRECD_FRAMERATE    *enc_fps,
                                      MMPS_3GPRECD_FRAMERATE    *container_fps);

/* Share Record Function */
#if (SUPPORT_SHARE_REC)
MMP_ERR MMPS_3GPRECD_GetShareRecordingTime(MMP_ULONG *ulTime);
MMP_ERR MMPS_3GPRECD_GetShareRecordingOffset(MMP_ULONG *ulTime);
MMP_ERR MMPS_3GPRECD_SetShareFileTimeLimit(MMP_ULONG ulTimeLimitMs);
MMP_ERR MMPS_3GPRECD_SetSharePreEncTimeLimit(MMP_ULONG ulTimeLimitMs);
MMP_ERR MMPS_3GPRECD_GetSharePreEncTimeLimit(MMP_ULONG *ulTimeLimitMs);
#endif

/* Wifi Streaming (H264) Function */
#if (SUPPORT_H264_WIFI_STREAM)
MMP_ERR MMPS_H264_WIFI_CustomedResol(MMP_UBYTE ubSnrSel, MMP_UBYTE sFitMode, MMP_USHORT usWidth, MMP_USHORT usHeight, MMP_BOOL bEnable);
MMP_ERR MMPS_H264_WIFI_EnableStreamPipe(MMP_H264_WIFISTREAM_OBJ *pWifi, MMP_BOOL bEnable,MMP_BOOL bForceEn);
MMP_ERR MMPS_H264_WIFI_ReserveStreamBuf(MMP_ULONG *ulCurBufPos, MMP_USHORT usResvMaxwW,  MMP_USHORT usResvMaxH, MMP_UBYTE ubEncBufMode);
#if 1//defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE)// Pcam Mix Mode ... UVC function work under Video/DSC mode
MMP_ERR MMPS_H264_WIFI_AssignObjEntity(MMP_UBYTE ubSnrSel, void** pHandle);
MMP_ERR MMPS_H264_WIFI_GetPreviewFctlLink(MMP_UBYTE ubSnrSel, MMP_PIPE_LINK *pWifiFctlLink, MMP_BOOL *pbNeedBackupPipeSetting);
MMP_ERR MMPS_H264_WIFI_BackupPreviewPipe(void** pHandle);
MMP_ERR MMPS_H264_WIFI_ResetPreviewPipe(void** pHandle);
MMP_ERR MMPS_H264_WIFI_GetStreamActive(MMP_UBYTE ubSnrSel, MMP_UBYTE *pbEnable);
MMP_ERR MMPS_H264_WIFI_FastSwitchSensorStream(void** pNewHandle, void** pCurHandle);
#endif
MMP_ERR MMPS_H264_WIFI_OpenStream(MMP_UBYTE ubSnrSel, void** pHandle, MMP_BOOL bUsePrevwPipe, MMP_UBYTE bVRMode);
MMP_ERR MMPS_H264_WIFI_StartStream(void* WifiHandle);
MMP_ERR MMPS_H264_WIFI_StopStream(void** WifiHandle);
MMP_ERR MMPS_H264_WIFI_Return2Display(void* WifiHandle);
#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE)// Pcam Mix Mode ... UVC function work under Video/DSC mode
MMP_ERR MMPS_H264_WIFI_SetStreamPipeConfig( 	MMP_H264_WIFISTREAM_OBJ	*pWifi, 
													VIDENC_INPUT_BUF 	    *pInputBuf,
													MMP_USHORT			 	usEncInputW,
											     	MMP_USHORT			 	usEncInputH,
											     	MMP_USHORT             	ubEncId);
#endif
#endif

#if (SUPPORT_UVC_FUNC)
/* UVC Streaming (H264) Function */
MMP_ERR MMPS_H264_UVC_StartStream(void* UVCHandle);
#endif

/* UVC Record Function */
MMP_ERR MMPS_3GPRECD_SetUVCFBMemory(void);
MMP_ERR MMPS_3GPRECD_SetUVCRearMJPGMemory(void);

/* Decode MJPEG to Preview/Encode Function */
MMP_UBYTE MMPS_3GPRECD_GetDecMjpegToPreviewPipeId(void);
MMP_ERR MMPS_3GPRECD_GetDecMjpegToPreviewSrcAttr(MMP_USHORT *pusW, MMP_USHORT *pusH);
MMP_ERR MMPS_3GPRECD_SetDecMjpegToPreviewSrcAttr(MMP_USHORT usSrcW, MMP_USHORT usSrcH);
MMP_ERR MMPS_3GPRECD_GetDecMjpegToPreviewDispAttr(  MMP_USHORT *pusDispWinId,
                                                    MMP_USHORT *pusWinOfstX,  MMP_USHORT *pusWinOfstY,
                                                    MMP_USHORT *pusWinWidth,  MMP_USHORT *pusWinHeight,
                                                    MMP_USHORT *pusDispWidth, MMP_USHORT *pusDispHeight,                                              
                                                    MMP_USHORT *pusDispColor);
MMP_ERR MMPS_3GPRECD_SetDecMjpegToPreviewDispAttr(  MMP_USHORT usDispWinId,
            									    MMP_BOOL   bRotate,
            									    MMP_UBYTE  ubRotateDir,
            									    MMP_UBYTE  sFitMode,
                                                    MMP_USHORT usWinOfstX,  MMP_USHORT usWinOfstY,
                                                    MMP_USHORT usWinWidth,  MMP_USHORT usWinHeight,
                                                    MMP_USHORT usDispWidth, MMP_USHORT usDispHeight,
                                                    MMP_USHORT usDispColor);
MMP_ERR MMPS_3GPRECD_SetDecMjpegToPreviewBuf(   MMP_USHORT                      usPreviewW,  
                                                MMP_USHORT                      usPreviewH, 
                                                MMP_ULONG                       ulAddr, 
                                                MMP_ULONG                       *pulSize,
                                                MMPS_3GPRECD_PREVIEW_BUFINFO    *pPreviewBuf);
MMP_ERR MMPS_3GPRECD_InitDecMjpegToPreview(MMP_USHORT usJpegSrcW, MMP_USHORT usJpegSrcH);
MMP_ERR MMPS_3GPRECD_DecMjpegPreviewStart(MMP_UBYTE ubSnrSel);
MMP_ERR MMPS_3GPRECD_DecMjpegPreviewStop(MMP_UBYTE ubSnrSel);
MMP_ERR MMPS_3GPRECD_SetDispColorFmtToJpgAttr(MMP_UBYTE ubColorformat);

/* Wifi Streaming (MJPEG) Function */
#if defined(ALL_FW)
MMP_ERR MMPS_MJPEG_OpenStream(MMP_USHORT usEncID, MMP_USHORT usMode, MMP_MJPEG_OBJ_PTR *ppHandle);
MMP_ERR MMPS_MJPEG_StartFrontCamStream(     MMP_UBYTE           ubSnrSel,
                                            MMP_UBYTE           ubMode,
                                            MMP_MJPEG_OBJ_PTR   pHandle,
                                            MMP_MJPEG_ENC_INFO  *pMjpegInfo,
                                            MMP_MJPEG_RATE_CTL  *pRateCtrl);
MMP_ERR MMPS_MJPEG_StartRearCamStream(      MMP_UBYTE           ubSnrSel,
                                            MMP_UBYTE           ubMode,
                                            MMP_MJPEG_OBJ_PTR   pHandle,
                                            MMP_MJPEG_ENC_INFO  *pMjpegInfo,
                                            MMP_MJPEG_RATE_CTL  *pRateCtrl);
MMP_ERR MMPS_MJPEG_StopStream(MMP_MJPEG_OBJ_PTR pHandle);
MMP_ERR MMPS_MJPEG_CloseStream(MMP_MJPEG_OBJ_PTR* ppHandle);
MMP_ERR MMPS_MJPEG_Return2Display(MMP_MJPEG_OBJ_PTR pHandle);
MMP_ERR MMPS_MJPEG_SetCaptureAddr(MMP_ULONG ulDramAddr, MMP_ULONG ulSramAddr);
MMP_ERR MMPS_MJPEG_GetCaptureAddr(MMP_ULONG *pulDram, MMP_ULONG *pulSram);
#endif

/* VR Thumbnail Function */
#if (SUPPORT_VR_THUMBNAIL)
MMP_ERR MMPS_3GPRECD_SetVRThumbJpgSize(MMP_ULONG ulJpegW, MMP_ULONG ulJpegH);
MMP_ERR MMPS_3GPRECD_GetVRThumbJpgSize(MMP_ULONG *pulJpegW, MMP_ULONG *pulJpegH);
MMP_ERR MMPS_3GPRECD_EnableVRThumbnail(MMP_UBYTE ubEnable, MMP_UBYTE ubIsCreateJpg);
MMP_ERR MMPS_3GPRECD_SetVRThumbRingBufNum(MMP_UBYTE ubRingBufNum);
MMP_UBYTE MMPS_3GPRECD_GetVRThumbnailSts(void);
MMP_ERR MMPS_3GPRECD_StoreJpgThumb(MMP_BYTE ubFilename[], MMP_USHORT usLength, VIDENC_STREAMTYPE ulStreamType);
#endif

/* Misc Function */
MMP_ERR MMPS_3GPRECD_SetYFrameType(MMPS_3GPRECD_Y_FRAME_TYPE enType);

#if (MGR_SUPPORT_AVI == 1)
#if (AVI_IDIT_CHUNK_EN == 1)
unsigned int MMPS_3GPMUX_Build_IDIT(void **ptr);
unsigned int MMPS_AVIMUX_Build_IDIT(void **ptr);
#endif
#endif

#if (DUALENC_SUPPORT)
MMP_ERR MMPS_3GPRECD_SetSWStickerAttribute(MMP_USHORT usStickerSrcWidth, MMP_USHORT usStickerSrcHeight, 
                                           MMP_USHORT usDstStartx, MMP_USHORT usDstStarty);
MMP_ERR MMPS_3GPRECD_SetSWStickerAddress(MMP_ULONG ulStickerSrcAddr);
#endif

MMP_ERR MMPS_3GPRECD_PreviewPipeInUVCMixModeEnable(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable);
MMP_ERR MMPS_3GPRECD_SetDualBayerSnrCaptureMode(MMP_BOOL bDSCMode);

/// @}

#endif //  _MMPS_3GPRECD_H_

