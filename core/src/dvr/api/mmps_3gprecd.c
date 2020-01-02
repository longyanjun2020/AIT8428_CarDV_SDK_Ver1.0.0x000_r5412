/**
 @file mmps_3gprecd.c
 @brief 3GP Recorder Control Function
 @author Will Tseng
 @version 1.0
*/

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_lib.h"
#include "ait_utility.h"
#include "aitu_memdebug.h"
#include "hdr_cfg.h"
#include "snr_cfg.h"
#include "ptz_cfg.h"
#include "usb_cfg.h"
#include "ldc_cfg.h"
#include "mdtc_cfg.h"
#include "vidrec_cfg.h"
#include "mmp_icon_inc.h"
#include "mmp_reg_mci.h"
#include "mmp_ldc_inc.h"
#if (SUPPORT_COMPONENT_FLOW_CTL)
#include "mmp_component_ctl.h"
#endif
#include "mmps_iva.h"
#include "mmps_system.h"
#include "mmps_3gprecd.h"
#include "mmps_dsc.h"
#include "mmps_sensor.h"
#include "mmps_audio.h"
#include "mmps_fs.h"
#include "mmpd_system.h"
#include "mmpd_fctl.h"
#include "mmpd_dma.h"
#include "mmpd_usb.h"
#include "mmpd_ptz.h"
#include "mmpd_scaler.h"
#include "mmpd_rawproc.h"
#include "mmpd_bayerscaler.h"
#include "mmpd_mp4venc.h"
#include "mmpd_3gpmgr.h"
#include "mmpd_uvcrecd.h"
#include "mmpf_sensor.h"
#include "mmpf_mp4venc.h"
#include "mmpf_3gpmgr.h"
#include "mmpf_avimux.h"
#include "mmpf_audio_ctl.h"
#include "mmpf_mci.h"
#include "mmpf_ldc.h"
#include "mmpf_rawproc.h"
#include "mmpf_adas_ctl.h"
#include "mmpf_display.h"
#include "mmpf_jpeg_ctl.h"
#include "mmpf_usbh_ctl.h"
#if (SUPPORT_UVC_FUNC)
#include "pCam_api.h"
#endif
#include "mmp_register.h"
#include "mmp_reg_jpeg.h"
#include "mmpf_dsc.h"

/** @addtogroup MMPS_3GPRECD
@{
*/

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define DMA_DEINTERLACE_BUF_CNT         (2)
#define DMA_DEINTERLACE_DOUBLE_FIELDS   (2)

#define MAX_3GPRECD_FILENAME_LENGTH     (512)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

/// Video encode system mode
static MMPS_3GPRECD_RUNTIME_CFG m_VidRecdModes = {
    0,                              // usVideoPreviewMode
    VIDRECD_RESOL_1920x1088,        // usVideoMVBufResIdx
    {0, 0},                         // usVideoEncResIdx
    {{0, 0},{0, 0}},                // SnrInputFrameRate
    {{0, 0},{0, 0}},                // VideoEncFrameRate
    {{0, 0},{0, 0}},                // ContainerFrameRate
    {0, 0},                         // usPFrameCount
    {0, 0},                         // usBFrameCount
    {1048576, 0},                   // ulBitrate
    0,                              // ulAudBitrate
    0xFFFFFFFF,                     // ulSizeLimit
    0xFFFFFFFF,                     // ulTimeLimitMs
    0,                              // ulReservedSpace
    {VIDENC_SRCMODE_CARD,  VIDENC_SRCMODE_CARD},    // VideoSrcMode
    {H264ENC_HIGH_PROFILE, H264ENC_HIGH_PROFILE},   // VisualProfile
#if (DUALENC_SUPPORT) || (SUPPORT_H264_WIFI_STREAM)
    {VIDENC_CURBUF_FRAME,   VIDENC_CURBUF_FRAME},  	// VidCurBufMode
#else
    {VIDENC_CURBUF_RT,      VIDENC_CURBUF_RT},      // VidCurBufMode
#endif    
    MMP_FALSE,                      // bSlowMotionEn;
};

/// Video context handle
MMP_ULONG                       m_VidRecdID = INVALID_ENC_ID;
MMP_ULONG                       m_VidDualID = INVALID_ENC_ID;

#if (SUPPORT_H264_WIFI_STREAM)
static MMP_H264_WIFISTREAM_OBJ  m_sH264WifiStreamObj[VR_MAX_H264_STREAM_NUM];
static MMP_ULONG                m_ulH264WifiFrontID = INVALID_ENC_ID;
static MMP_ULONG                m_ulH264WifiRearID  = INVALID_ENC_ID;
#endif

/// Video encode config mode
static MMPS_3GPRECD_PRESET_CFG  m_VidRecdConfigs;

/// Status of video preview/Encode
MMP_BOOL                        m_bVidPreviewActive[VR_MAX_PREVIEW_NUM]	= {MMP_FALSE, MMP_FALSE};
MMP_BOOL                        m_bVidRecordActive[VR_MAX_ENCODE_NUM]   = {MMP_FALSE, MMP_FALSE};
#if (SUPPORT_H264_WIFI_STREAM)
MMP_BOOL                        m_bH264WifiStreamActive[VR_MAX_H264_STREAM_NUM] = {MMP_FALSE};
#endif

/// Parameters of preview/record zoom
static MMPS_DSC_ZOOM_INFO       m_VidPreviewZoomInfo;
static MMPS_DSC_ZOOM_INFO       m_VidRecordZoomInfo;
static MMP_USHORT               m_usVidStaticZoomIndex  = 0;

/// End of video preview memory
static MMP_ULONG                m_ulVideoPreviewEndAddr = 0;

/// Container format
static VIDMGR_CONTAINER_TYPE    m_VidRecdContainer      = VIDMGR_CONTAINER_UNKNOWN;

/// Parameters of video input buffers
static VIDENC_INPUT_BUF         m_VidRecdInputBuf[VR_MAX_ENCODE_NUM];
#if (SUPPORT_H264_WIFI_STREAM)
static VIDENC_INPUT_BUF         m_WifiEncInputBuf[VR_MAX_H264_STREAM_NUM];
#endif
static MMP_BOOL                 m_bInitFcamRecPipeFrmBuf = MMP_FALSE;

/// AHC parameters
static MMPS_3GPRECD_AHC_PREVIEW_INFO    m_sAhcVideoPrevInfo[VR_MAX_PREVIEW_NUM] = {{0},{0}};
static MMPS_3GPRECD_AHC_VIDEO_INFO      m_sAhcVideoRecdInfo[VR_MAX_ENCODE_NUM] = {{0},{0}};
#if (SUPPORT_H264_WIFI_STREAM)
static MMPS_3GPRECD_AHC_VIDEO_INFO      m_sAhcH264WifiInfo[VR_MAX_H264_STREAM_NUM] = {0};
#endif

/// For Decode MJPEG to Preview attribute
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
static MMPS_3GPRECD_AHC_PREVIEW_INFO    m_sAhcDecMjpegToPreviewInfo;
static MMP_USHORT                       m_usDecMjpegToPreviewSrcW   = 1280;
static MMP_USHORT                       m_usDecMjpegToPreviewSrcH   = 720;
static MMPS_3GPRECD_PREVIEW_BUFINFO     m_sDecMjpegPrevwBufInfo;
#endif

/// For Decode MJPEG to Encode attribute
#if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
static MMPS_3GPRECD_AHC_VIDEO_INFO      m_sAhcDecMjpegToEncodeInfo 	= {0};
static MMP_USHORT                       m_usAhcDecMjpegToEncodeSrcW = 1280;
static MMP_USHORT                       m_usAhcDecMjpegToEncodeSrcH = 720;
#endif

static MMP_BOOL                         m_bAhcConfigVideoRZoom  = MMP_FALSE;

static MMP_ULONG                m_ulVidRecEncodeAddr        = 0;
static MMP_ULONG                m_ulVidRecSramAddr          = 0;

static MMP_ULONG                m_ulVidRecCaptureSramAddr   = 0;
static MMP_ULONG                m_ulVidRecCaptureDramAddr   = 0;
static MMP_ULONG                m_ulVidRecCaptureEndSramAddr = 0;
static MMP_ULONG                m_ulVidRecCaptureEndDramAddr = 0;

#if (HANDLE_JPEG_EVENT_BY_QUEUE)
#if (SUPPORT_MJPEG_WIFI_STREAM)
static MMP_ULONG                m_ulFrontCamRsvdCompBufStart = 0;
static MMP_ULONG               	m_ulFrontCamRsvdCompBufSize	= MAX_FRONT_CAM_ENC_COMPBUF_SZ;
static MMP_ULONG                m_ulFrontCamEncLineBufAddr	= 0;
static MMP_ULONG               	m_ulFrontCamEncLineBufSize	= MAX_FRONT_CAM_ENC_LINEBUF_SZ;
static MMP_USHORT               m_usMJPEGMaxEncWidth        = 640;
static MMP_USHORT               m_usMJPEGMaxEncHeight       = 480;
#endif
static MMP_ULONG                m_ulVidRecCaptureFrmBufAddr = 0;
#endif

MMP_ULONG                       m_ulVidRecRearCamCaptFrmSyncCnt = 2;
static MMP_ULONG                m_ulVidRecRearCamCaptDramAddr 	= 0;
#if (SUPPORT_USB_HOST_FUNC)
static MMP_ULONG                m_ulVidRecRearCamCaptDramSize 	= ISO_MJPEG_MAX_VIDEO_FRM_SIZE;
#else
static MMP_ULONG                m_ulVidRecRearCamCaptDramSize 	= 512*1024;
#endif

static MMP_BOOL                 m_bSeamlessEnabled 		= MMP_FALSE;

static MMP_ULONG                m_ulVidShareMvAddr 	    = 0;
static MMP_ULONG                m_ulVidRecDramEndAddr 	= 0;
#if (SUPPORT_H264_WIFI_STREAM)
static MMP_ULONG                m_ulVidWifiStreamStartAddr = 0;
static MMP_ULONG                m_ulVidWifiStreamDramAddr  = 0;
#endif

static MMP_UBYTE                m_ub1stVRStreamSnrId    = PRM_SENSOR;
static MMP_UBYTE                m_ub2ndVRStreamSnrId    = SCD_SENSOR;//PRM_SENSOR;

static MMP_PIPE_LINK            m_RecordFctlLink        = {MMP_SCAL_PIPE_0, MMP_ICO_PIPE_0, MMP_IBC_PIPE_0};
static MMP_PIPE_LINK            m_2ndRecFctlLink        = {MMP_SCAL_PIPE_1, MMP_ICO_PIPE_1, MMP_IBC_PIPE_1};
#if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
static MMP_PIPE_LINK            m_DecMjpegToEncFctlLink = {MMP_SCAL_PIPE_1, MMP_ICO_PIPE_1, MMP_IBC_PIPE_1};
#endif
static MMP_PIPE_LINK            m_PreviewFctlLink 	    = {MMP_SCAL_PIPE_3, MMP_ICO_PIPE_3, MMP_IBC_PIPE_3};
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
static MMP_PIPE_LINK            m_DecMjpegToPrevwFctlLink = {MMP_SCAL_PIPE_2, MMP_ICO_PIPE_2, MMP_IBC_PIPE_2};
#endif
static MMP_PIPE_LINK            m_2ndPrewFctlLink       = {MMP_SCAL_PIPE_2, MMP_ICO_PIPE_2, MMP_IBC_PIPE_2};
static MMP_PIPE_LINK            m_WifiDSCFctlLink 		= {MMP_SCAL_PIPE_2, MMP_ICO_PIPE_2, MMP_IBC_PIPE_2};
static MMP_PIPE_LINK            m_LdcH264WifiVRFctlLink = {MMP_SCAL_PIPE_2, MMP_ICO_PIPE_2, MMP_IBC_PIPE_2};
static MMP_PIPE_LINK            m_LdcH264WifiDSCFctlLink = {MMP_SCAL_PIPE_0, MMP_ICO_PIPE_0, MMP_IBC_PIPE_0};
static MMP_PIPE_LINK            m_LdcSrcFctlLink 	    = {MMP_SCAL_PIPE_1, MMP_ICO_PIPE_1, MMP_IBC_PIPE_1};
static MMP_PIPE_LINK            m_LdcCaptureFctlLink 	= {MMP_SCAL_PIPE_2, MMP_ICO_PIPE_2, MMP_IBC_PIPE_2};
static MMP_PIPE_LINK            m_PreviewFctlLinkInUVCMixMode = {MMP_SCAL_PIPE_3, MMP_ICO_PIPE_3, MMP_IBC_PIPE_3};
#if (SUPPORT_MDTC)||(SUPPORT_ADAS)
static MMP_PIPE_LINK            m_MdtcFctlLink          = {MMP_SCAL_PIPE_4, MMP_ICO_PIPE_4, MMP_IBC_PIPE_4};
#endif

static MMP_UBYTE                m_ubLdcResMode          = 0;
static MMP_UBYTE                m_ubLdcFpsMode          = 0;
static MMP_ULONG                m_ulLdcMaxSrcWidth      = 1920;
static MMP_ULONG                m_ulLdcMaxSrcHeight     = 1080;
static MMP_ULONG                m_ulLdcMaxOutWidth      = 1920;
static MMP_ULONG                m_ulLdcMaxOutHeight     = 1080;
static MMP_UBYTE                m_ubLdcMaxOutBufNum     = 2;
static MMP_ULONG                m_ulLdcOutStoreYAddr[MAX_LDC_OUT_BUF_NUM];
static MMP_ULONG                m_ulLdcOutStoreUAddr[MAX_LDC_OUT_BUF_NUM];
static MMP_ULONG                m_ulLdcOutStoreVAddr[MAX_LDC_OUT_BUF_NUM];

static MMP_USHORT               m_usMaxStillJpegW       = VR_MAX_CAPTURE_WIDTH;
static MMP_USHORT               m_usMaxStillJpegH       = VR_MAX_CAPTURE_HEIGHT;

#if (SUPPORT_MDTC)||(SUPPORT_ADAS)
static MMPS_3GPRECD_Y_FRAME_TYPE m_YFrameType           = MMPS_3GPRECD_Y_FRAME_TYPE_NONE;
MMP_SCAL_FIT_RANGE              gsADASFitRange          = {0};
MMP_SCAL_GRAB_CTRL              gsADASGrabctl           = {0};
MMPD_FCTL_ATTR                  m_ADASFctlAttr          = {0};
#endif

static MMP_ULONG                m_ulVRPreviewW[VR_MAX_PREVIEW_NUM] 	= {0, 0};
static MMP_ULONG                m_ulVRPreviewH[VR_MAX_PREVIEW_NUM]	= {0, 0};
static MMP_ULONG                m_ulVREncodeW[VR_MAX_ENCODE_NUM] 	= {0, 0};
static MMP_ULONG                m_ulVREncodeH[VR_MAX_ENCODE_NUM]    = {0, 0};
static MMP_ULONG                m_ulVREncScalOutW[VR_MAX_ENCODE_NUM] = {0, 0};
static MMP_ULONG                m_ulVREncScalOutH[VR_MAX_ENCODE_NUM] = {0, 0};
#if (SUPPORT_H264_WIFI_STREAM)
static MMP_ULONG                m_ulWifiStreamW[VR_MAX_H264_STREAM_NUM]    = {0};
static MMP_ULONG                m_ulWifiStreamH[VR_MAX_H264_STREAM_NUM]    = {0};
static MMP_ULONG                m_ulWifiStreamScalOutW[VR_MAX_H264_STREAM_NUM] = {0};
static MMP_ULONG                m_ulWifiStreamScalOutH[VR_MAX_H264_STREAM_NUM] = {0};
#endif

#if (SUPPORT_COMPONENT_FLOW_CTL)
static MMP_UBYTE                m_ubCamPreviewListId[MAX_CAM_NUM] = {0xFF, 0xFF, 0xFF};
static MMP_UBYTE                m_ubCamEncodeListId[MAX_CAM_NUM] = {0xFF, 0xFF, 0xFF};
#endif

MMPD_FCTL_ATTR                  m_VRPreviewFctlAttr[VR_MAX_PREVIEW_NUM];
MMPD_FCTL_ATTR                  m_DecMjpegToPrevwFctlAttr;
MMPD_FCTL_ATTR                  m_VREncodeFctlAttr[VR_MAX_ENCODE_NUM];
MMPD_FCTL_ATTR                  m_DecMjpegToEncFctlAttr;

MMP_RAW_STORE_BUF               m_sRawBuf[MMP_RAW_MDL_NUM];
MMP_RAW_STORE_BUF               m_sRawEndBuf[MMP_RAW_MDL_NUM];
MMP_RAW_STORE_BUF               m_sDeinterlaceBuf[MMP_RAW_MDL_NUM];

static MMP_BOOL                 m_bDualEncEnable = MMP_FALSE;
#if (DUALENC_SUPPORT)
static MMP_ULONG                glEncID[2] = {INVALID_ENC_ID, INVALID_ENC_ID};
static MMP_UBYTE                gbTotalEncNum = 0;
#endif

MMP_BOOL                        gbDualBayerSnrInDSCMode = MMP_FALSE;
#if (EMERGENTRECD_SUPPORT)
MMP_3GPRECD_EMERG_ACTION        m_EmergActionType = MMP_3GPRECD_EMERG_DEFAULT_ACT;
#endif

MMP_BOOL                		m_bMuxer3gpConstantFps = MMP_TRUE;

//==============================================================================
//
//                              EXTERN VARIABLE
//
//==============================================================================

extern MMP_BOOL     m_bHdmiInterlace;
extern MMP_BOOL     m_bUVCRecdSupport;
#if (SUPPORT_SHARE_REC)
extern MMP_ULONG    glDualRecdTimeLimit;
extern MMP_ULONG    glDualPreEncDuration;
#endif
#if (SUPPORT_VR_THUMBNAIL)
extern MMP_ULONG    m_ulVRThumbWidth;
extern MMP_ULONG    m_ulVRThumbHeight;
extern MMP_ULONG    gulVRThumbMaxBufSize;
extern MMP_UBYTE    gubVRThumbEn;
extern MMP_UBYTE    gubIsCreatJpgFile;
#endif

extern void* H264UVCHdl[];
#if (SUPPORT_H264_WIFI_STREAM)
//extern void* m_sH264WifiHdl[];
#endif

extern MMP_UBYTE gbADASSrcYUV;
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

static MMP_ERR MMPS_3GPRECD_SetPreviewMemory(MMP_UBYTE ubSnrSel, 
											 MMP_USHORT usPreviewW, 
											 MMP_USHORT usPreviewH, 
											 MMP_ULONG *ulStackAddr, 
											 MMP_ULONG *ulFbAddr);
MMP_ERR MMPS_3GPRECD_SetH264MemoryMap(MMP_ULONG *ulEncId, MMP_USHORT usEncW, MMP_USHORT usEncH, MMP_ULONG ulFBufAddr, MMP_ULONG ulStackAddr);
MMP_ERR MMPS_3GPRECD_InitDecMjpegToEncode(VIDENC_INPUT_BUF *pInputBuf, MMP_ULONG ubEncId);

MMP_ERR MMPS_3GPRECD_SetDualH264MemoryMap(MMP_ULONG *ulEncId, MMP_USHORT usEncW, MMP_USHORT usEncH, MMP_ULONG ulFBufAddr, MMP_ULONG ulStackAddr);
#if(SUPPORT_H264_WIFI_STREAM)
MMP_ERR MMPS_H264_WIFI_SetStreamMemoryMap(MMP_ULONG *ulEncId, MMP_H264_WIFISTREAM_OBJ *pWifi, MMP_USHORT usEncW, MMP_USHORT usEncH, MMP_ULONG ulFBufAddr, MMP_ULONG ulStackAddr);
#endif
MMP_ERR MMPS_3GPRECD_StopAllPipeZoom(void);

static MMP_ERR MMPS_3GPRECD_InitDigitalZoomParam(MMP_UBYTE ubPipe);
static MMP_ERR MMPS_3GPRECD_RestoreDigitalZoomRange(MMP_UBYTE ubPipe);

static MMP_ULONG MMPS_3GPRECD_CalculteTargetFrmSize(MMP_ULONG ulEncId);
static MMP_ERR MMPS_3GPRECD_SetEncodeRes(MMP_UBYTE ubEncIdx);

static MMP_ERR MMPS_3GPRECD_Set2ndSnrPreviewMemory( MMP_UBYTE   ubSnrSel,
                                                    MMP_USHORT  usPreviewW,  
                                                    MMP_USHORT  usPreviewH, 
                                                    MMP_ULONG   *ulStackAddr);
MMP_ERR MMPS_3GPRECD_Enable2ndSnrPreviewPipe(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable, MMP_BOOL bCheckFrameEnd);

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetConfig
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get pointer to initialize customer configs.
 @retval Return pointer.
*/
MMPS_3GPRECD_PRESET_CFG* MMPS_3GPRECD_GetConfig(void)
{
    return (&m_VidRecdConfigs);
}

#if 0
void ____VR_Common_Preview_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetPreviewPipe
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetPreviewPipe(MMP_UBYTE ubSnrSel, MMP_UBYTE ubPipe)
{
    if (ubSnrSel == PRM_SENSOR) {
        FCTL_PIPE_TO_LINK(ubPipe, m_PreviewFctlLink);
    }
    else if (ubSnrSel == SCD_SENSOR) {
        FCTL_PIPE_TO_LINK(ubPipe, m_2ndPrewFctlLink);
    }
    #if(SUPPORT_DEC_MJPEG_TO_PREVIEW)
    else if (ubSnrSel == USBH_SENSOR) {
        FCTL_PIPE_TO_LINK(ubPipe, m_DecMjpegToPrevwFctlLink);
    }
    #endif
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_CustomedPreviewAttr
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set video preview resolution.
 @param[in] bUserConfig		Use user defined preview configuration.
 @param[in] bRotate			Use DMA rotate to rotate preview buffer.
 @param[in] ubRotateDir		DMA rotate direction.
 @param[in] sFitMode		Scaler fit mode.
 @param[in] usBufWidth		Preview buffer width.
 @param[in] usBufHeight		Preview buffer height.
 @param[in] usStartX 		The X Offset of the display window.
 @param[in] usStartY 		The Y Offset of the display window.
 @param[in] usWinWidth  	The width of the display window.
 @param[in] usWinHeight 	The height of the display window.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_CustomedPreviewAttr(MMP_UBYTE  ubSnrSel,
                                         MMP_BOOL 	bUserConfig,
										 MMP_BOOL 	bRotate,
										 MMP_UBYTE 	ubRotateDir,
										 MMP_UBYTE	sFitMode,
										 MMP_USHORT usBufWidth, MMP_USHORT usBufHeight, 
										 MMP_USHORT usStartX, 	MMP_USHORT usStartY,
                                      	 MMP_USHORT usWinWidth, MMP_USHORT usWinHeight)
{
    m_sAhcVideoPrevInfo[ubSnrSel].bUserDefine  	    = bUserConfig;
    m_sAhcVideoPrevInfo[ubSnrSel].bPreviewRotate    = bRotate;
    m_sAhcVideoPrevInfo[ubSnrSel].sPreviewDmaDir    = ubRotateDir;
    m_sAhcVideoPrevInfo[ubSnrSel].sFitMode          = sFitMode;
    m_sAhcVideoPrevInfo[ubSnrSel].ulPreviewBufW     = usBufWidth;
    m_sAhcVideoPrevInfo[ubSnrSel].ulPreviewBufH     = usBufHeight;
    m_sAhcVideoPrevInfo[ubSnrSel].ulDispStartX      = usStartX;
    m_sAhcVideoPrevInfo[ubSnrSel].ulDispStartY      = usStartY;
    m_sAhcVideoPrevInfo[ubSnrSel].ulDispWidth       = usWinWidth;
    m_sAhcVideoPrevInfo[ubSnrSel].ulDispHeight      = usWinHeight;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_AdjustPreviewRes
//  Description : This function need to called before MMPS_3GPRECD_PreviewStart()
//------------------------------------------------------------------------------
static MMP_ERR MMPS_3GPRECD_AdjustPreviewRes(MMP_UBYTE ubSnrSel)
{
    MMP_ULONG   ulScalInW, ulScalInH;
    MMP_ULONG   ulPreviewW, ulPreviewH;
    MMP_BOOL    bAdjustSize = MMP_TRUE;
    
    if (m_VidRecdConfigs.previewpath[ubSnrSel] == MMP_3GP_PATH_INVALID) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    MMPS_Sensor_GetCurPrevScalInputRes(ubSnrSel, &ulScalInW, &ulScalInH);
    printc("ubSnrSel=%d....ulScalInW=%d....ulScalInH=%d\r\n",ubSnrSel,ulScalInW,ulScalInH);
    // Calculate preview parameters.
    if (m_sAhcVideoPrevInfo[ubSnrSel].bUserDefine) {
        ulPreviewW = m_sAhcVideoPrevInfo[ubSnrSel].ulPreviewBufW;
        ulPreviewH = m_sAhcVideoPrevInfo[ubSnrSel].ulPreviewBufH;
    }
    else {
        ulPreviewW = m_VidRecdConfigs.previewdata[ubSnrSel].usVidPreviewBufW[m_VidRecdModes.usVideoPreviewMode];
        ulPreviewH = m_VidRecdConfigs.previewdata[ubSnrSel].usVidPreviewBufH[m_VidRecdModes.usVideoPreviewMode];
    }
   printc("ulPreviewW=%d....ulPreviewH=%d\r\n",ulPreviewW,ulPreviewH);
    if ((m_sAhcVideoPrevInfo[ubSnrSel].bUserDefine) &&
        (m_VidRecdModes.usVideoPreviewMode != VIDRECD_CCIR_PREVIEW_MODE)) {

        if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) || 
            (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
            bAdjustSize = (ubSnrSel == SCD_SENSOR) ? (MMP_FALSE) : (MMP_TRUE);
        }
        if ((CAM_CHECK_PRM(PRM_CAM_TV_DECODER)) || 
            (CAM_CHECK_PRM(PRM_CAM_YUV_SENSOR))) {
            bAdjustSize = (ubSnrSel == PRM_SENSOR) ? (MMP_FALSE) : (MMP_TRUE);
        }
        printc("bAdjustSize=%d\r\n",bAdjustSize);
        if (bAdjustSize)
        {
            MMPS_Display_AdjustScaleInSize( MMP_DISPLAY_PRM_CTL,
                                            ulScalInW,
                                            ulScalInH,
                                            ulPreviewW,
                                            ulPreviewH,
                                            &ulPreviewW,
                                            &ulPreviewH);
        }
    }

    if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
        if (MMP_IsVidLdcSupport()) {
            // NOP
        }
        else {
            ulPreviewW = (gbDualBayerSnrInDSCMode) ? (FLOOR32(ulScalInW) * 2) : (320); // TBD
            ulPreviewH = (gbDualBayerSnrInDSCMode) ? (FLOOR32(ulScalInH)) : (160);
	 printc("4444ulPreviewW=%d....ulPreviewH=%d\r\n",ulPreviewW,ulPreviewH);
        }
    }

    if (ubSnrSel != SCD_SENSOR) {
        m_ulVRPreviewW[ubSnrSel] = ulPreviewW;
        m_ulVRPreviewH[ubSnrSel] = ulPreviewH;
    }
   	else { // CHECK
        m_ulVRPreviewW[ubSnrSel] = 480;//ALIGN8(ulPreviewW);   liao
        m_ulVRPreviewH[ubSnrSel] = 320;//ALIGN8(ulPreviewH);
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set customized preview mode.
 @param[in] usPreviewMode Assign preview display mode. 
            0: VIDRECD_NORMAL_PREVIEW_MODE, 
            1: VIDRECD_FULL_PREVIEW_MODE, 
            2: VIDRECD_NTSC_PREVIEW_MODE, 
            3: VIDRECD_PAL_PREVIEW_MODE,
            4: VIDRECD_HDMI_PREVIEW_MODE
            5: VIDRECD_CCIR_PREVIEW_MODE
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetPreviewMode(MMP_USHORT usPreviewMode)
{
    m_VidRecdModes.usVideoPreviewMode = usPreviewMode;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetPreviewMode
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get customized preview mode.
 @param[in] usPreviewMode Assign preview display mode. 
            0: VIDRECD_NORMAL_PREVIEW_MODE, 
            1: VIDRECD_FULL_PREVIEW_MODE, 
            2: VIDRECD_NTSC_PREVIEW_MODE, 
            3: VIDRECD_PAL_PREVIEW_MODE,
            4: VIDRECD_HDMI_PREVIEW_MODE
 @retval usVideoPreviewMode
*/
MMP_USHORT MMPS_3GPRECD_GetPreviewMode(void)
{
    return m_VidRecdModes.usVideoPreviewMode;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetPreviewPipeStatus
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Return in video preview status or not.
 @param[out] bEnable preview enable.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetPreviewPipeStatus(MMP_UBYTE ubSnrSel, MMP_BOOL *bEnable)
{
    *bEnable = m_bVidPreviewActive[ubSnrSel];
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetPreviewPipe
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetPreviewPipe(MMP_UBYTE ubSnrSel, MMP_IBC_PIPEID *pPipe)
{
    if (ubSnrSel == PRM_SENSOR) {
        *pPipe = m_PreviewFctlLink.ibcpipeID;
    }
    else if (ubSnrSel == SCD_SENSOR && MMP_IsDualVifCamEnable()) {
        if (MMP_GetDualSnrPrevwType() == DUALSNR_DUAL_PREVIEW)
            *pPipe = m_2ndPrewFctlLink.ibcpipeID;
        else
            *pPipe = m_PreviewFctlLink.ibcpipeID;
    }
    else {
        RTNA_DBG_Str0("UNDEFINE SNR:");  RTNA_DBG_Byte0(ubSnrSel);  RTNA_DBG_Str0(" \r\n");
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetPreviewRes
//  Description : 
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetPreviewRes(MMP_UBYTE ubSnrSel, MMP_ULONG *pulWidth, MMP_ULONG *pulHeight)
{
    *pulWidth = m_ulVRPreviewW[ubSnrSel];
    *pulHeight = m_ulVRPreviewH[ubSnrSel];

    return MMP_ERR_NONE;
}

#if 0
void ____VR_1st_Preview_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetPreviewPipeConfig
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set pipe config.
 @param[in] *pPreviewBuf 	Pointer to Preview buffer.
 @param[in] *pLdcSrcBuf 	Pointer to LDC source buffer.
 @param[in] *pMdtcBuf 		Pointer to Motion detection buffer.
 @param[in] usPreviewW 		The preview buffer width.
 @param[in] usPreviewH 		The preview buffer height.
 @retval MMP_ERR_NONE Success.
*/
static MMP_ERR MMPS_3GPRECD_SetPreviewPipeConfig(MMPS_3GPRECD_PREVIEW_BUFINFO 	*pPreviewBuf,
										  		 MMP_LDC_SRC_BUFINFO 			*pLdcSrcBuf,
										  		 MMPS_3GPRECD_MDTC_BUFINFO		*pMdtcBuf,
										  		 MMP_USHORT 					usPreviewW, 
										  		 MMP_USHORT 					usPreviewH,
										  		 MMP_UBYTE                      ubSnrSel)
{
    MMP_USHORT				usModeIdx = m_VidRecdModes.usVideoPreviewMode;
    MMP_ULONG				ulScalInW, ulScalInH;
    MMP_ULONG          		ulRotateW, ulRotateH;
    MMP_BOOL				bDmaRotateEn;
    MMP_SCAL_FIT_MODE		sFitMode;
   	MMP_SCAL_FIT_RANGE		fitrange;
    MMP_SCAL_GRAB_CTRL   	previewGrabctl, DispGrabctl;
    MMP_DISPLAY_DISP_ATTR	dispAttr;
    MMP_DISPLAY_ROTATE_TYPE	ubDmaRotateDir;
    MMPD_FCTL_ATTR          fctlAttr;
    MMP_USHORT              i;
    MMP_ULONG             	ulDispStartX, ulDispStartY, ulDispWidth, ulDispHeight;
    MMP_SCAL_FIT_RANGE      sFitRangeBayer;
    MMP_SCAL_GRAB_CTRL      sGrabctlBayer;
    MMP_USHORT              usCurZoomStep = 0;
    MMP_ULONG               ulPreviewInW, ulPreviewInH;
    MMP_DISPLAY_WIN_ID      ePreviewWinID = GET_VR_PREVIEW_WINDOW(ubSnrSel);
    MMP_DISPLAY_DEV_TYPE    ePreviewDev = m_VidRecdConfigs.previewdata[0].DispDevice[usModeIdx];

	/* Parameter Check */
	if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID) {
		return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
	}

	if (pPreviewBuf == NULL) {
		return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
	}
	
	if ((m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE || 
	  	 m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI  ||
	  	 m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) && pLdcSrcBuf == NULL) {
		return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
	}

    #if (SUPPORT_MDTC)
    if (pMdtcBuf == NULL) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }
    #endif

    /* Get the preview display parameters */
    MMPS_Sensor_GetCurPrevScalInputRes(ubSnrSel, &ulScalInW, &ulScalInH);

    MMPD_BayerScaler_GetZoomInfo(MMP_BAYER_SCAL_DOWN, &sFitRangeBayer, &sGrabctlBayer); 

    if (m_sAhcVideoPrevInfo[ubSnrSel].bUserDefine) {
        bDmaRotateEn    = m_sAhcVideoPrevInfo[ubSnrSel].bPreviewRotate;
        ubDmaRotateDir	= m_sAhcVideoPrevInfo[ubSnrSel].sPreviewDmaDir;
        sFitMode 		= m_sAhcVideoPrevInfo[ubSnrSel].sFitMode;
        ulDispStartX  	= m_sAhcVideoPrevInfo[ubSnrSel].ulDispStartX;
        ulDispStartY  	= m_sAhcVideoPrevInfo[ubSnrSel].ulDispStartY;
        ulDispWidth   	= m_sAhcVideoPrevInfo[ubSnrSel].ulDispWidth;
        ulDispHeight  	= m_sAhcVideoPrevInfo[ubSnrSel].ulDispHeight;
    }
    else {
        bDmaRotateEn	= m_VidRecdConfigs.previewdata[0].bUseRotateDMA[usModeIdx];
        ubDmaRotateDir	= m_VidRecdConfigs.previewdata[0].ubDMARotateDir[usModeIdx];
        sFitMode 		= m_VidRecdConfigs.previewdata[0].sFitMode[usModeIdx];
        ulDispStartX  	= m_VidRecdConfigs.previewdata[0].usVidDispStartX[usModeIdx];
        ulDispStartY  	= m_VidRecdConfigs.previewdata[0].usVidDispStartY[usModeIdx];
        ulDispWidth   	= m_VidRecdConfigs.previewdata[0].usVidDisplayW[usModeIdx];
        ulDispHeight  	= m_VidRecdConfigs.previewdata[0].usVidDisplayH[usModeIdx];
    }

    /* Initial zoom relative config */ 
    MMPS_3GPRECD_InitDigitalZoomParam(m_PreviewFctlLink.scalerpath);

    MMPS_3GPRECD_RestoreDigitalZoomRange(m_PreviewFctlLink.scalerpath);

    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE) 
    {
        // Config Video Preview Pipe
        #if (TV_JAGGY_WORKAROUND)
        if (usModeIdx == VIDRECD_NTSC_PREVIEW_MODE || 
            usModeIdx == VIDRECD_PAL_PREVIEW_MODE)
        {
            ulPreviewInW = TV_JAGGY_1ST_OUT_W;
            ulPreviewInH = TV_JAGGY_1ST_OUT_H;
        }
        else
        #endif
        {
            if (MMP_IsVidPtzEnable()) {
                ulPreviewInW = sFitRangeBayer.ulOutWidth;
                ulPreviewInH = sFitRangeBayer.ulOutHeight;
            }
            else {
                ulPreviewInW = ulScalInW;
                ulPreviewInH = ulScalInH;
            }
        }
        
        fitrange.fitmode        = sFitMode;
        fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;
        fitrange.ulInWidth      = ulPreviewInW;
        fitrange.ulInHeight     = ulPreviewInH;
        
        if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
            fitrange.ulOutWidth  = (gbDualBayerSnrInDSCMode) ? FLOOR32(fitrange.ulInWidth) : (usPreviewW / 2);
            fitrange.ulOutHeight = (gbDualBayerSnrInDSCMode) ? FLOOR32(fitrange.ulInHeight) : (usPreviewH);
        }
        else {
            fitrange.ulOutWidth  = usPreviewW;
            fitrange.ulOutHeight = usPreviewH;
        }

        fitrange.ulInGrabX      = 1;
        fitrange.ulInGrabY      = 1;
        fitrange.ulInGrabW      = fitrange.ulInWidth;
        fitrange.ulInGrabH      = fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        
        MMPD_PTZ_InitPtzInfo(m_PreviewFctlLink.scalerpath,
                             fitrange.fitmode,
                             fitrange.ulInWidth, fitrange.ulInHeight,
                             fitrange.ulOutWidth, fitrange.ulOutHeight);

        MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);

        MMPD_PTZ_CalculatePtzInfo(m_PreviewFctlLink.scalerpath, usCurZoomStep, 0, 0);

        MMPD_PTZ_GetCurPtzInfo(m_PreviewFctlLink.scalerpath, &fitrange, &previewGrabctl);

        if (MMP_IsVidPtzEnable()) {
            MMPD_PTZ_ReCalculateGrabRange(&fitrange, &previewGrabctl);
        }
        
        fctlAttr.bRtModeOut     = MMP_FALSE;
        fctlAttr.colormode      = m_VidRecdConfigs.previewdata[0].DispColorFmt[usModeIdx];
        if (fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB565 || 
            fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB888) {
            fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_RGB;
        }
        else {
            #if (CCIR656_FORCE_SEL_BT601)
            fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
            #else
            fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_YUV_FULLRANGE;
            #endif
        }
        fctlAttr.fctllink       = m_PreviewFctlLink;
        fctlAttr.fitrange       = fitrange;
        fctlAttr.grabctl        = previewGrabctl;
        if ((m_VidRecdConfigs.bRawPreviewEnable[0]) &&
            (m_VidRecdConfigs.ubRawPreviewBitMode[0] == MMP_RAW_COLORFMT_YUV420 ||
             m_VidRecdConfigs.ubRawPreviewBitMode[0] == MMP_RAW_COLORFMT_YUV422))
        {
            fctlAttr.scalsrc    = MMP_SCAL_SOURCE_GRA;
        }
        else {
            fctlAttr.scalsrc    = MMP_SCAL_SOURCE_ISP;
        }
        fctlAttr.sScalDelay         = m_sFullSpeedScalDelay;
        fctlAttr.bSetScalerSrc      = MMP_TRUE;
        fctlAttr.ubPipeLinkedSnr    = ubSnrSel;
        fctlAttr.usBufCnt           = pPreviewBuf->usBufCnt;

        for (i = 0; i < fctlAttr.usBufCnt; i++) {
            fctlAttr.ulBaseAddr[i] = pPreviewBuf->ulYBuf[i];
            fctlAttr.ulBaseUAddr[i] = pPreviewBuf->ulUBuf[i];
            fctlAttr.ulBaseVAddr[i] = pPreviewBuf->ulVBuf[i];
        }

        if (bDmaRotateEn) {
            fctlAttr.bUseRotateDMA  = MMP_TRUE;
            fctlAttr.usRotateBufCnt = pPreviewBuf->usRotateBufCnt;
            
            for (i = 0; i < fctlAttr.usRotateBufCnt; i++) {
                fctlAttr.ulRotateAddr[i] = pPreviewBuf->ulRotateYBuf[i];
                fctlAttr.ulRotateUAddr[i] = pPreviewBuf->ulRotateUBuf[i];
                fctlAttr.ulRotateVAddr[i] = pPreviewBuf->ulRotateVBuf[i];
            }
        }
        else {
            fctlAttr.bUseRotateDMA = MMP_FALSE;
            fctlAttr.usRotateBufCnt = 0;
        }

        m_VRPreviewFctlAttr[0] = fctlAttr;

        #if (TV_JAGGY_WORKAROUND)
        if (usModeIdx == VIDRECD_NTSC_PREVIEW_MODE || 
            usModeIdx == VIDRECD_PAL_PREVIEW_MODE)
            MMPD_Fctl_SetPipeAttrForIbcFbForTV(ulScalInW, ulScalInH, &fctlAttr);
        else
        #endif
            MMPD_Fctl_SetPipeAttrForIbcFB(&fctlAttr);

        MMPD_Fctl_ClearPreviewBuf(m_PreviewFctlLink.ibcpipeID, 0xFFFFFF);

        if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
            MMPD_Fctl_LinkPipeToDisplay(m_PreviewFctlLink.ibcpipeID, 
                                        ePreviewWinID,
                                        ePreviewDev);
        }
        else {
        
            if (bDmaRotateEn) {
                MMPD_Fctl_LinkPipeToDma(m_PreviewFctlLink.ibcpipeID,
                                        ePreviewWinID,
                                        ePreviewDev,
                                        ubDmaRotateDir);
            }
            else {
                MMPD_Fctl_LinkPipeToDisplay(m_PreviewFctlLink.ibcpipeID, 
                                            ePreviewWinID,
                                            ePreviewDev);
            }
        }
        
        // Config Display Window
        if ((m_VidRecdConfigs.previewdata[0].VidDispDir[usModeIdx] == MMP_DISPLAY_ROTATE_NO_ROTATE) ||
            (m_VidRecdConfigs.previewdata[0].VidDispDir[usModeIdx] == MMP_DISPLAY_ROTATE_RIGHT_180)) {                        
            ulRotateW = usPreviewW;
            ulRotateH = usPreviewH;
        }
        else {
            ulRotateW = usPreviewH;
            ulRotateH = usPreviewW;
        }
        
        if (bDmaRotateEn) {
            // Rotate 90/270 for vertical panel
            ulRotateW = usPreviewH;
            ulRotateH = usPreviewW;
        }
        
        dispAttr.usStartX = 0;
        dispAttr.usStartY = 0;
        
        if (0/*m_sAhcVideoPrevInfo[ubSnrSel].bUserDefine*/) {
            dispAttr.usDisplayOffsetX = ulDispStartX;
            dispAttr.usDisplayOffsetY = ulDispStartY;
        }
        else {
            // Image at center
            dispAttr.usDisplayOffsetX = (ulDispWidth > ulRotateW) ? ((ulDispWidth - ulRotateW) >> 1) : (0);
            dispAttr.usDisplayOffsetY = (ulDispHeight > ulRotateH) ? ((ulDispHeight - ulRotateH) >> 1) : (0);
        }
        
        dispAttr.bMirror           = m_VidRecdConfigs.previewdata[0].bVidDispMirror[usModeIdx];

        if (bDmaRotateEn) {
            dispAttr.rotatetype    = MMP_DISPLAY_ROTATE_NO_ROTATE;
        }
        else {
            dispAttr.rotatetype    = m_VidRecdConfigs.previewdata[0].VidDispDir[usModeIdx];
        }

        dispAttr.usDisplayWidth    = ulRotateW;
        dispAttr.usDisplayHeight   = ulRotateH;

        if (m_bHdmiInterlace) {
            dispAttr.usDisplayHeight = ulRotateH / 2;
        }
        
        MMPD_Display_SetWinToDisplay(ePreviewWinID, &dispAttr);

        fitrange.fitmode        = sFitMode;
        fitrange.scalerType     = MMP_SCAL_TYPE_WINSCALER;
        fitrange.ulInWidth      = ulRotateW;
        fitrange.ulInHeight     = ulRotateH;
        fitrange.ulOutWidth     = ulRotateW;
        fitrange.ulOutHeight    = ulRotateH;

        fitrange.ulInGrabX      = 1;
        fitrange.ulInGrabY      = 1;
        fitrange.ulInGrabW      = fitrange.ulInWidth;
        fitrange.ulInGrabH      = fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;

        MMPD_Scaler_GetGCDBestFitScale(&fitrange, &DispGrabctl);

        MMPD_Display_SetWinScaling(ePreviewWinID, &fitrange, &DispGrabctl);

        MMPD_Fctl_UnLinkPipeToLdc(m_LdcSrcFctlLink.ibcpipeID);
    }
    else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE || 
             m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI  ||
             m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) 
    {
        MMP_SCAL_GRAB_CTRL  ldcGrabCtl;

        MMPF_LDC_SetLinkPipe(m_LdcSrcFctlLink.ibcpipeID,
							 m_PreviewFctlLink.ibcpipeID,
							 m_RecordFctlLink.ibcpipeID,
							 m_PreviewFctlLink.ibcpipeID,
							 m_LdcCaptureFctlLink.ibcpipeID);
        
        // Config pipe for graphic loopback
        fitrange.fitmode        = MMP_SCAL_FITMODE_OUT;
        fitrange.scalerType		= MMP_SCAL_TYPE_SCALER;
        fitrange.ulInWidth  	= ulScalInW;
        fitrange.ulInHeight 	= ulScalInH; 
        fitrange.ulOutWidth     = m_ulLdcMaxSrcWidth;
        fitrange.ulOutHeight    = m_ulLdcMaxSrcHeight;

        fitrange.ulInGrabX 		= 1;
        fitrange.ulInGrabY 		= 1;
        fitrange.ulInGrabW 		= fitrange.ulInWidth;
        fitrange.ulInGrabH 		= fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;

        MMPD_Scaler_GetGCDBestFitScale(&fitrange, &ldcGrabCtl);
        
        fctlAttr.bRtModeOut     = MMP_FALSE;
        fctlAttr.colormode      = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
        #if (CCIR656_FORCE_SEL_BT601)
        fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
        #else
        fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_YUV_FULLRANGE;
        #endif
        fctlAttr.fctllink       = m_LdcSrcFctlLink;
        fctlAttr.fitrange       = fitrange;
        fctlAttr.grabctl        = ldcGrabCtl;
        fctlAttr.scalsrc        = MMP_SCAL_SOURCE_ISP;
        fctlAttr.sScalDelay     = m_sFullSpeedScalDelay;
        fctlAttr.bSetScalerSrc  = MMP_TRUE;
        fctlAttr.ubPipeLinkedSnr = ubSnrSel;
        fctlAttr.usBufCnt       = pLdcSrcBuf->usInBufCnt;

        for (i = 0; i < pLdcSrcBuf->usInBufCnt; i++) {
            fctlAttr.ulBaseAddr[i]  = pLdcSrcBuf->ulInYBuf[i];
            fctlAttr.ulBaseUAddr[i] = pLdcSrcBuf->ulInUBuf[i];
            fctlAttr.ulBaseVAddr[i] = pLdcSrcBuf->ulInVBuf[i];
        }
        
        fctlAttr.bUseRotateDMA  = MMP_FALSE;
        fctlAttr.usRotateBufCnt = 0;

        MMPD_Fctl_SetPipeAttrForIbcFB(&fctlAttr);
        MMPD_Fctl_ClearPreviewBuf(m_LdcSrcFctlLink.ibcpipeID, 0xFFFFFF);
        MMPD_Fctl_LinkPipeToLdc(m_LdcSrcFctlLink.ibcpipeID);
        
        if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI ||
            m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE)
        {
            // Config pipe for preview first
            fitrange.fitmode        = sFitMode;
            fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;
            
            if (MMP_IsDualVifCamEnable() && MMP_GetParallelFrmStoreType() != PARALLEL_FRM_NOT_SUPPORT)
                fitrange.ulInWidth  = m_ulLdcMaxOutWidth * 2;
            else
                fitrange.ulInWidth  = m_ulLdcMaxOutWidth;
            
            fitrange.ulInHeight     = m_ulLdcMaxOutHeight;
            fitrange.ulOutWidth     = usPreviewW;
            fitrange.ulOutHeight    = usPreviewH;

            fitrange.ulInGrabX      = 1;
            fitrange.ulInGrabY      = 1;
            fitrange.ulInGrabW      = fitrange.ulInWidth;
            fitrange.ulInGrabH      = fitrange.ulInHeight;
            fitrange.ubChoseLit     = 0;
            
            MMPD_PTZ_InitPtzInfo(m_PreviewFctlLink.scalerpath,
                                 fitrange.fitmode,
                                 fitrange.ulInWidth, fitrange.ulInHeight, 
                                 fitrange.ulOutWidth, fitrange.ulOutHeight);

            MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);

            MMPD_PTZ_CalculatePtzInfo(m_PreviewFctlLink.scalerpath, usCurZoomStep, 0, 0);

            MMPD_PTZ_GetCurPtzInfo(m_PreviewFctlLink.scalerpath, &fitrange, &previewGrabctl);

            if (MMP_IsVidPtzEnable()) {
                MMPD_PTZ_ReCalculateGrabRange(&fitrange, &previewGrabctl);
            }
            
            fctlAttr.bRtModeOut         = MMP_FALSE;
            fctlAttr.colormode          = m_VidRecdConfigs.previewdata[0].DispColorFmt[usModeIdx];
            if (fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB565 || 
                fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB888) {
                fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_RGB;
            }
            else {
                #if (CCIR656_FORCE_SEL_BT601)
                fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
                #else
                fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_YUV_FULLRANGE;
                #endif
            }
            fctlAttr.fctllink           = m_PreviewFctlLink;
            fctlAttr.fitrange           = fitrange;
            fctlAttr.grabctl            = previewGrabctl;
            fctlAttr.scalsrc            = MMP_SCAL_SOURCE_LDC;
            fctlAttr.sScalDelay         = m_sFullSpeedScalDelay;
            fctlAttr.bSetScalerSrc      = MMP_TRUE;
            fctlAttr.ubPipeLinkedSnr    = ubSnrSel;
            fctlAttr.usBufCnt           = pPreviewBuf->usBufCnt;
            fctlAttr.bUseRotateDMA      = MMP_FALSE;
            fctlAttr.usRotateBufCnt     = 0;
            
            for (i = 0; i < fctlAttr.usBufCnt; i++) {
                fctlAttr.ulBaseAddr[i] = pPreviewBuf->ulYBuf[i];
                fctlAttr.ulBaseUAddr[i] = pPreviewBuf->ulUBuf[i];
                fctlAttr.ulBaseVAddr[i] = pPreviewBuf->ulVBuf[i];
            }

            MMPD_Fctl_SetPipeAttrForIbcFB(&fctlAttr);
            MMPD_Fctl_ClearPreviewBuf(m_PreviewFctlLink.ibcpipeID, 0xFFFFFF);
            MMPD_Fctl_LinkPipeToDisplay(m_PreviewFctlLink.ibcpipeID, 
                                        ePreviewWinID, 
                                        ePreviewDev);
            
            // Config pipe for loopback 
            MMPD_Fctl_ResetIBCLinkType(m_PreviewFctlLink.ibcpipeID);
            
            fitrange.fitmode        = MMP_SCAL_FITMODE_OUT;
            fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;
            
            if (MMP_IsDualVifCamEnable() && MMP_GetParallelFrmStoreType() != PARALLEL_FRM_NOT_SUPPORT)
                fitrange.ulInWidth  = m_ulLdcMaxOutWidth * 2;
            else
                fitrange.ulInWidth  = m_ulLdcMaxOutWidth;
            
            fitrange.ulInHeight     = m_ulLdcMaxOutHeight;
           	fitrange.ulOutWidth     = fitrange.ulInWidth;
            fitrange.ulOutHeight    = fitrange.ulInHeight;

            fitrange.ulInGrabX      = 1;
            fitrange.ulInGrabY      = 1;
            fitrange.ulInGrabW      = fitrange.ulInWidth;
            fitrange.ulInGrabH      = fitrange.ulInHeight;
            fitrange.ubChoseLit     = 0;

            MMPD_Scaler_GetGCDBestFitScale(&fitrange, &ldcGrabCtl);
            
            fctlAttr.bRtModeOut         = MMP_FALSE;
            fctlAttr.colormode          = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
            #if (CCIR656_FORCE_SEL_BT601)
            fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
            #else
            fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_YUV_FULLRANGE;
            #endif
            fctlAttr.fctllink           = m_PreviewFctlLink;
            fctlAttr.fitrange           = fitrange;
            fctlAttr.grabctl            = ldcGrabCtl;
            fctlAttr.scalsrc            = MMP_SCAL_SOURCE_LDC;
            fctlAttr.sScalDelay         = m_sFullSpeedScalDelay;
            fctlAttr.bSetScalerSrc      = MMP_TRUE;
            fctlAttr.ubPipeLinkedSnr    = ubSnrSel;
            fctlAttr.usBufCnt           = pPreviewBuf->usBufCnt;
            fctlAttr.bUseRotateDMA      = MMP_FALSE;
            fctlAttr.usRotateBufCnt     = 0;

            for (i = 0; i < fctlAttr.usBufCnt; i++) {
                fctlAttr.ulBaseAddr[i] = pPreviewBuf->ulYBuf[i];
                fctlAttr.ulBaseUAddr[i] = pPreviewBuf->ulUBuf[i];
                fctlAttr.ulBaseVAddr[i] = pPreviewBuf->ulVBuf[i];
            }
            
            MMPD_Fctl_SetPipeAttrForIbcFB(&fctlAttr);
            MMPD_Fctl_ClearPreviewBuf(m_PreviewFctlLink.ibcpipeID, 0xFFFFFF);		
            MMPD_Fctl_LinkPipeToLdc(m_PreviewFctlLink.ibcpipeID);
        }
        else
        {
            // Config pipe for preview
            fitrange.fitmode        = sFitMode;
            fitrange.scalerType 	= MMP_SCAL_TYPE_SCALER;
            fitrange.ulInWidth  	= m_ulLdcMaxOutWidth;
            fitrange.ulInHeight 	= m_ulLdcMaxOutHeight;
            fitrange.ulOutWidth     = usPreviewW;
            fitrange.ulOutHeight    = usPreviewH;

            fitrange.ulInGrabX 		= 1;
            fitrange.ulInGrabY 		= 1;
            fitrange.ulInGrabW 		= fitrange.ulInWidth;
            fitrange.ulInGrabH 		= fitrange.ulInHeight;
            fitrange.ubChoseLit     = 0;
            
	        MMPD_PTZ_InitPtzInfo(m_PreviewFctlLink.scalerpath,
	        					 fitrange.fitmode,
					             fitrange.ulInWidth, fitrange.ulInHeight, 
					             fitrange.ulOutWidth, fitrange.ulOutHeight);

			MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);

			MMPD_PTZ_CalculatePtzInfo(m_PreviewFctlLink.scalerpath, usCurZoomStep, 0, 0);

			MMPD_PTZ_GetCurPtzInfo(m_PreviewFctlLink.scalerpath, &fitrange, &previewGrabctl);

            if (MMP_IsVidPtzEnable()) {
                MMPD_PTZ_ReCalculateGrabRange(&fitrange, &previewGrabctl);
            }
            
            fctlAttr.bRtModeOut         = MMP_FALSE;
            fctlAttr.colormode          = m_VidRecdConfigs.previewdata[0].DispColorFmt[usModeIdx];
            if (fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB565 || 
                fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB888) {
                fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_RGB;
            }
            else {
                #if (CCIR656_FORCE_SEL_BT601)
                fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
                #else
                fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_YUV_FULLRANGE;
                #endif
            }
            fctlAttr.fctllink           = m_PreviewFctlLink;
            fctlAttr.fitrange           = fitrange;
            fctlAttr.grabctl            = previewGrabctl;
            fctlAttr.scalsrc            = MMP_SCAL_SOURCE_LDC;
            fctlAttr.sScalDelay         = m_sFullSpeedScalDelay;
            fctlAttr.bSetScalerSrc      = MMP_TRUE;
            fctlAttr.ubPipeLinkedSnr    = ubSnrSel;
            fctlAttr.usBufCnt           = pPreviewBuf->usBufCnt;
            
            for (i = 0; i < fctlAttr.usBufCnt; i++) {
                fctlAttr.ulBaseAddr[i] = pPreviewBuf->ulYBuf[i];
                fctlAttr.ulBaseUAddr[i] = pPreviewBuf->ulUBuf[i];
                fctlAttr.ulBaseVAddr[i] = pPreviewBuf->ulVBuf[i];
            }
            
	        if (bDmaRotateEn) {
	            fctlAttr.bUseRotateDMA  = MMP_TRUE;
	            fctlAttr.usRotateBufCnt = pPreviewBuf->usRotateBufCnt;
	            
	            for (i = 0; i < fctlAttr.usRotateBufCnt; i++) {
	                fctlAttr.ulRotateAddr[i]  = pPreviewBuf->ulRotateYBuf[i];
	                fctlAttr.ulRotateUAddr[i] = pPreviewBuf->ulRotateUBuf[i];
	                fctlAttr.ulRotateVAddr[i] = pPreviewBuf->ulRotateVBuf[i];
	            }
	        }
	        else {
	            fctlAttr.bUseRotateDMA = MMP_FALSE;
	            fctlAttr.usRotateBufCnt = 0;
	        }

	        MMPD_Fctl_SetPipeAttrForIbcFB(&fctlAttr);
			MMPD_Fctl_ClearPreviewBuf(m_PreviewFctlLink.ibcpipeID, 0xFFFFFF);

            if (fctlAttr.bUseRotateDMA) {
                MMPD_Fctl_LinkPipeToDma(m_PreviewFctlLink.ibcpipeID, 
                                        ePreviewWinID, 
                                        ePreviewDev,
                                        ubDmaRotateDir);
            }
            else {
                MMPD_Fctl_LinkPipeToDisplay(m_PreviewFctlLink.ibcpipeID, 
                                            ePreviewWinID, 
                                            ePreviewDev);
            }
        }
        
        // Config Display Window
        if ((m_VidRecdConfigs.previewdata[0].VidDispDir[usModeIdx] == MMP_DISPLAY_ROTATE_NO_ROTATE) ||
            (m_VidRecdConfigs.previewdata[0].VidDispDir[usModeIdx] == MMP_DISPLAY_ROTATE_RIGHT_180)) {                        
            ulRotateW = usPreviewW;
            ulRotateH = usPreviewH;
        }
        else {
            ulRotateW = usPreviewH;
            ulRotateH = usPreviewW;
        }
        
        if (bDmaRotateEn) {
        	// Rotate 90/270 for vertical panel
        	ulRotateW = usPreviewH;
            ulRotateH = usPreviewW;
        }
        
        dispAttr.usStartX          = 0;
        dispAttr.usStartY          = 0;
        dispAttr.usDisplayOffsetX  = (ulDispWidth > ulRotateW) ? ((ulDispWidth - ulRotateW) >> 1) : (0);
        dispAttr.usDisplayOffsetY  = (ulDispHeight > ulRotateH) ? ((ulDispHeight - ulRotateH) >> 1) : (0);
        dispAttr.bMirror           = m_VidRecdConfigs.previewdata[0].bVidDispMirror[usModeIdx];

        if (bDmaRotateEn) {
            dispAttr.rotatetype    = MMP_DISPLAY_ROTATE_NO_ROTATE;
        }
        else {
            dispAttr.rotatetype    = m_VidRecdConfigs.previewdata[0].VidDispDir[usModeIdx];
        }

        dispAttr.usDisplayWidth    = ulRotateW;
        dispAttr.usDisplayHeight   = ulRotateH;
        
        if (m_bHdmiInterlace) {
            dispAttr.usDisplayHeight = ulRotateH / 2;
        }
        
        MMPD_Display_SetWinToDisplay(ePreviewWinID, &dispAttr);

        fitrange.fitmode        = sFitMode;
        fitrange.scalerType     = MMP_SCAL_TYPE_WINSCALER;
        fitrange.ulInWidth      = ulRotateW;
        fitrange.ulInHeight     = ulRotateH;

        if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI ||
        	m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
        	/* Use display scaler to fit the desired output range */
	        fitrange.ulOutWidth  = m_ulVRPreviewW[ubSnrSel];
	        fitrange.ulOutHeight = m_ulVRPreviewH[ubSnrSel];
        }
        else {
            fitrange.ulOutWidth  = ulRotateW;
            fitrange.ulOutHeight = ulRotateH;
        }

        fitrange.ulInGrabX      = 1;
        fitrange.ulInGrabY 		= 1;
        fitrange.ulInGrabW 		= fitrange.ulInWidth;
        fitrange.ulInGrabH 	    = fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;

        MMPD_Scaler_GetGCDBestFitScale(&fitrange, &DispGrabctl);

        MMPD_Display_SetWinScaling(ePreviewWinID, &fitrange, &DispGrabctl);
    }

    // Config ADAS Pipe
    #if (SUPPORT_MDTC) || (SUPPORT_ADAS)
    gsADASFitRange.fitmode      = sFitMode; // ADAS fitmode refer by preview's mode
    gsADASFitRange.scalerType 	= MMP_SCAL_TYPE_SCALER;
    gsADASFitRange.ulInWidth    = ulScalInW;
    gsADASFitRange.ulInHeight   = ulScalInH;

    gsADASFitRange.ulInGrabX 	= 1;
    gsADASFitRange.ulInGrabY 	= 1;
    gsADASFitRange.ulInGrabW 	= ulPreviewInW;
    gsADASFitRange.ulInGrabH 	= ulPreviewInH;
    gsADASFitRange.ubChoseLit   = 0;
    
    #if (ADAS_OUTPUT_LOG == 1)
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "====ADAS Parameter Calculate Result1====");
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "====ADASFitRange Paramters====");
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "gsADASFitRange.fitmode");
    RTNA_DBG_Long(0, gsADASFitRange.fitmode);
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "gsADASFitRange.scalerType");
    RTNA_DBG_Long(0, gsADASFitRange.scalerType);
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "gsADASFitRange.ulInWidth");
    RTNA_DBG_Long(0, gsADASFitRange.ulInWidth);
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "gsADASFitRange.ulInHeight");
    RTNA_DBG_Long(0, gsADASFitRange.ulInHeight);
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "gsADASFitRange.ulInGrabX");
    RTNA_DBG_Long(0, gsADASFitRange.ulInGrabX);
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "gsADASFitRange.ulInGrabY");
    RTNA_DBG_Long(0, gsADASFitRange.ulInGrabY);
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "gsADASFitRange.ulInGrabW");
    RTNA_DBG_Long(0, gsADASFitRange.ulInGrabW);
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "gsADASFitRange.ulInGrabH");
    RTNA_DBG_Long(0, gsADASFitRange.ulInGrabH);
    RTNA_DBG_Str (0, "\r\n");
    RTNA_DBG_Str (0, "====End of ADASFitRange Paramters====");
    RTNA_DBG_Str (0, "\r\n");
    #endif
    
    MMPD_Scaler_GetGCDBestFitScale(&gsADASFitRange, &gsADASGrabctl);

    if (gbADASSrcYUV == 1)//YUV422
    {
    	fctlAttr.colormode      = MMP_DISPLAY_COLOR_YUV422;
    	m_MdtcFctlLink.scalerpath 	= MMP_SCAL_PIPE_2;
    	m_MdtcFctlLink.icopipeID 	= MMP_ICO_PIPE_2;
    	m_MdtcFctlLink.ibcpipeID 	= MMP_IBC_PIPE_2;
    }
    else
    {
    	fctlAttr.colormode      = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
    }
    fctlAttr.fctllink       = m_MdtcFctlLink;
    fctlAttr.fitrange       = gsADASFitRange;
    fctlAttr.grabctl        = gsADASGrabctl;
    fctlAttr.scalsrc		= MMP_SCAL_SOURCE_ISP;
    fctlAttr.bSetScalerSrc	= MMP_TRUE;
    fctlAttr.usBufCnt  		= pMdtcBuf->usBufCnt;
    fctlAttr.bUseRotateDMA 	= MMP_FALSE;
	fctlAttr.usRotateBufCnt = 0;

    for (i = 0; i < fctlAttr.usBufCnt; i++) {
        fctlAttr.ulBaseAddr[i] = pMdtcBuf->ulYBuf[i];
    }

    MEMCPY(&m_ADASFctlAttr, &fctlAttr, sizeof(fctlAttr));

    if (gbADASSrcYUV == 1)//YUV422
    {
    	MMPD_Fctl_SetPipeAttrForIbcFB(&fctlAttr);
    }
    else
    {
    	MMPD_Fctl_SetSubPipeAttr(&fctlAttr);
    }

    MMPS_Sensor_SetVMDPipe(PRM_SENSOR, m_MdtcFctlLink.ibcpipeID);

    #if (CPU_ID == CPU_A && SUPPORT_ADAS)
    {
        MMP_ULONG ulFocalLength = 0, ulPixelSize = 0, ulDzoomN = 0, ulDzoomM = 0;
        MMPF_SENSOR_RESOLUTION* pResTable = gsSensorFunction->MMPF_Sensor_GetResTable(ubSnrSel);
        MMP_USHORT usCurPreviewMode = 0;

        _ADAS_GetCustomerPara(&ulFocalLength, &ulPixelSize, &ulDzoomN, &ulDzoomM);   

        // ulDzoomN/ulDzoomM are based on Y direction in ADAS library.
        ulDzoomN = gsADASGrabctl.ulScaleYN;
        ulDzoomM = gsADASGrabctl.ulScaleYM;

        // Restore bayer scaler ratio.
        ulDzoomN *= sGrabctlBayer.ulScaleYN;
        ulDzoomM *= sGrabctlBayer.ulScaleYM;

        MMPF_Sensor_GetParam(ubSnrSel, MMPF_SENSOR_CURRENT_PREVIEW_MODE, &usCurPreviewMode);
        
        // Restore sensor binning ratio.
        ulDzoomN *= pResTable->ubHBinningN[usCurPreviewMode];
        ulDzoomM *= pResTable->ubHBinningM[usCurPreviewMode];

        // Get sensor pixel size.
        ulPixelSize = (pResTable->usPixelSize /*Unit:nm*/ * 100) / 1000; //unit: um*100

        #if (ADAS_OUTPUT_LOG == 1)
        RTNA_DBG_Str (0, "\r\n");
        RTNA_DBG_Str (0, "====ADAS Parameter Calculate Result2====");
        RTNA_DBG_Str (0, "\r\n");
        RTNA_DBG_Str (0, "====ScaleYN/YM of ADAS ====");
        RTNA_DBG_Str (0, "\r\n");
        RTNA_DBG_Str (0, "gsADASGrabctl.ulScaleYN");
        RTNA_DBG_Long(0, gsADASGrabctl.ulScaleYN);
        RTNA_DBG_Str (0, "\r\n");
        RTNA_DBG_Str (0, "gsADASGrabctl.ulScaleYM");
        RTNA_DBG_Long(0, gsADASGrabctl.ulScaleYM);
        RTNA_DBG_Str (0, "\r\n");
        RTNA_DBG_Str (0, "sGrabctlBayer.ulScaleYN");
        RTNA_DBG_Long(0, sGrabctlBayer.ulScaleYN);
        RTNA_DBG_Str (0, "\r\n");
        RTNA_DBG_Str (0, "sGrabctlBayer.ulScaleYM");
        RTNA_DBG_Long(0, sGrabctlBayer.ulScaleYM);
        RTNA_DBG_Str (0, "\r\n");
        RTNA_DBG_Str (0, "pResTable->ubHBinningN[usCurPreviewMode]");
        RTNA_DBG_Long(0, pResTable->ubHBinningN[usCurPreviewMode]);
        RTNA_DBG_Str (0, "\r\n");
        RTNA_DBG_Str (0, "pResTable->ubHBinningM[usCurPreviewMode]");
        RTNA_DBG_Long(0, pResTable->ubHBinningM[usCurPreviewMode]);
        RTNA_DBG_Str (0, "\r\n");

        RTNA_DBG_Str(0, "ADAS Focal Len:");
        RTNA_DBG_Long(0, ulFocalLength);
        RTNA_DBG_Str(0, ", PixelSize:");
        RTNA_DBG_Long(0, ulPixelSize);
        RTNA_DBG_Str(0, "\r\nZoom N:");
        RTNA_DBG_Long(0, ulDzoomN);
        RTNA_DBG_Str(0, ", Zoom M:");
        RTNA_DBG_Long(0, ulDzoomM);
        RTNA_DBG_Str(0, "\r\n");
        RTNA_DBG_Str (0, "====End of ScaleYN/YM of ADAS ====");
        RTNA_DBG_Str (0, "\r\n");
        #endif
                
        _ADAS_CustomerSpecifyPara(ulFocalLength, ulPixelSize, ulDzoomN, ulDzoomM);           
    }
    #endif
    #endif

    #if (SUPPORT_DEC_MJPEG_TO_PREVIEW == 0) // CHECK
    #if (HANDLE_JPEG_EVENT_BY_QUEUE)
    if (MMPF_JPEG_GetCtrlByQueueEnable()) {
        MMPD_System_EnableClock(MMPD_SYS_CLK_JPG,   MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYS_CLK_SCALE, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYS_CLK_ICON,  MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYS_CLK_IBC,   MMP_TRUE);
    }
    #endif
    #endif
    
    // Tune MCI priority
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE) {
        if (m_LdcSrcFctlLink.ibcpipeID == MMP_IBC_PIPE_0)
            MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_GRA_LDC_P0);
        else if (m_LdcSrcFctlLink.ibcpipeID == MMP_IBC_PIPE_1)
            MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_GRA_LDC_P1);
        else if (m_LdcSrcFctlLink.ibcpipeID == MMP_IBC_PIPE_2)
            MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_GRA_LDC_P2);
        else if (m_LdcSrcFctlLink.ibcpipeID == MMP_IBC_PIPE_3)
            MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_GRA_LDC_P3);

        MMPD_H264ENC_SetEncByteCnt(128);
    }
    else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI ||
             m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
        MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_GRA_LDC_P3P1_H264_P0);
        MMPD_H264ENC_SetEncByteCnt(128);
    }
    else
    {
        if (m_VidRecdConfigs.bRawPreviewEnable[0]) {
            if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_0)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_RAW0_P0);
            else if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_1)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_RAW0_P1);
            else if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_2)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_RAW0_P2);
            else if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_3)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_RAW0_P3); 
        }
        else if (m_VidRecdConfigs.previewdata[0].bUseRotateDMA[usModeIdx]) {
            if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_0)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_DMAR_H264_P0);
            else if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_1)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_DMAR_H264_P1);
            else if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_2)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_DMAR_H264_P2);
            else if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_3)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_DMAR_H264_P3);
        }
        else {
            if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_0)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_DMAR_H264_P0);
            else if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_1)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_DMAR_H264_P1);
            else if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_2)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_DMAR_H264_P2);
            else if (m_PreviewFctlLink.ibcpipeID == MMP_IBC_PIPE_3)
                MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_DMAR_H264_P3);
        }

        if (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
            MMPF_MCI_SetModuleMaxPriority(MCI_SRC_RAWS0);
            MMPF_MCI_SetModuleMaxPriority(MCI_SRC_RAWS1_JPGLB);
        }

        if (m_VidRecdConfigs.bStillCaptureEnable) {
            MMPD_VIDENC_TunePipeMaxMCIPriority(m_RecordFctlLink.ibcpipeID); // patch for catpure
        }            
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_EnablePreviewPipe
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Turn on and off preview for video encode.

 @param[in] bEnable             Enable and disable scaler path for video preview.
 @param[in] bCheckFrameEnd      When "bEnable" is MMP_TRUE, the setting means check frame end or not.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_EnablePreviewPipe(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable, MMP_BOOL bCheckFrameEnd)
{
    MMP_DISPLAY_WIN_ID  winId   = GET_VR_PREVIEW_WINDOW(ubSnrSel);
    MMP_UBYTE           ubVifId = MMPD_Sensor_GetVIFPad(ubSnrSel);
    MMP_ERR             sRet    = MMP_ERR_NONE;
    
    if (!(bEnable ^ m_bVidPreviewActive[ubSnrSel])) {
        return MMP_ERR_NONE;
    }

    if (bEnable) {
        // Reset all pipe
        MMP_UBYTE i = 0;

        for (i = MMP_IBC_PIPE_0; i < MMP_IBC_PIPE_MAX; i++) {
            sRet = MMPD_Icon_ResetModule(i);
            sRet = MMPD_IBC_ResetModule(i);
        }

        if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}
        
        // Enable video encode timer
        sRet = MMPD_VIDENC_EnableTimer(bEnable);
        if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}

        switch(m_VidRecdConfigs.previewpath[0]) {
            case MMP_3GP_PATH_2PIPE:
                sRet = MMPD_Fctl_EnablePreview(ubSnrSel, m_PreviewFctlLink.ibcpipeID, bEnable, bCheckFrameEnd);
            break;
            case MMP_3GP_PATH_LDC_LB_SINGLE:
            case MMP_3GP_PATH_LDC_LB_MULTI:
            case MMP_3GP_PATH_LDC_MULTISLICE:
                sRet = MMPD_Fctl_EnablePreview(ubSnrSel, m_LdcSrcFctlLink.ibcpipeID,  bEnable, MMP_FALSE); 
                sRet |= MMPD_Fctl_EnablePreview(ubSnrSel, m_PreviewFctlLink.ibcpipeID, bEnable, MMP_FALSE);
            break;
        }
        
        if (sRet != MMP_ERR_NONE){MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return MMP_3GPRECD_ERR_GENERAL_ERROR;}            

        #if (SUPPORT_MDTC)||(SUPPORT_ADAS)
        sRet |= MMPD_Fctl_EnablePreview(ubSnrSel, m_MdtcFctlLink.ibcpipeID, bEnable, MMP_FALSE);
        #endif
        if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}            
    }
    else 
    {
        sRet = MMPS_3GPRECD_StopAllPipeZoom();
        
        if (sRet != MMP_ERR_NONE){MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}            

#if (SUPPORT_MDTC)
      	if (MMPS_Sensor_IsVMDStarted(PRM_SENSOR)) {
       		MMPS_Sensor_StartVMD(PRM_SENSOR, MMP_FALSE);
            if(sRet != MMP_ERR_NONE){MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}                        
        }
#endif
#if (SUPPORT_ADAS)
      	if (MMPS_Sensor_IsADASStarted(PRM_SENSOR)) {
       		MMPS_Sensor_StartADAS(PRM_SENSOR, MMP_FALSE);
            if(sRet != MMP_ERR_NONE){MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}                        
        }
#endif

        switch(m_VidRecdConfigs.previewpath[0]) {
            case MMP_3GP_PATH_2PIPE:
                sRet = MMPD_Fctl_EnablePreview(ubSnrSel, m_PreviewFctlLink.ibcpipeID, bEnable, MMP_TRUE);
            break;
            case MMP_3GP_PATH_LDC_LB_SINGLE:
            case MMP_3GP_PATH_LDC_LB_MULTI:
            case MMP_3GP_PATH_LDC_MULTISLICE:
                sRet = MMPD_Fctl_EnablePreview(ubSnrSel, m_PreviewFctlLink.ibcpipeID, bEnable, MMP_TRUE);
                sRet |= MMPD_Fctl_EnablePreview(ubSnrSel, m_LdcSrcFctlLink.ibcpipeID,  bEnable, MMP_TRUE);
            break;
        }

        if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return MMP_3GPRECD_ERR_GENERAL_ERROR;}                                    

        #if (SUPPORT_MDTC)||(SUPPORT_ADAS)
        sRet |= MMPD_Fctl_EnablePreview(ubSnrSel, m_MdtcFctlLink.ibcpipeID, bEnable, MMP_TRUE);
        #endif

        // Disable video encode timer
        sRet |= MMPD_VIDENC_EnableTimer(MMP_FALSE);

        if (MMPS_Display_GetFreezeWinEn() == MMP_FALSE) {
        	sRet |= MMPD_Display_SetWinActive(winId, MMP_FALSE);
        }
        
        if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}                                    

        if (m_VidRecdConfigs.bRawPreviewEnable[0]) {
            sRet = MMPD_RAWPROC_EnablePreview(ubVifId, MMP_FALSE);

            if (m_VidRecdConfigs.ubRawPreviewBitMode[0] == MMP_RAW_COLORFMT_YUV420 ||
                m_VidRecdConfigs.ubRawPreviewBitMode[0] == MMP_RAW_COLORFMT_YUV422) {
                sRet |= MMPD_RAWPROC_EnablePath(ubSnrSel,
                                                ubVifId,
                                                MMP_FALSE,
                                                MMP_RAW_IOPATH_VIF2RAW,
                                                m_VidRecdConfigs.ubRawPreviewBitMode[0]);
            }
            else {    
                sRet |= MMPD_RAWPROC_EnablePath(ubSnrSel,
                                                ubVifId,
                                                MMP_FALSE,
                                                MMP_RAW_IOPATH_VIF2RAW | MMP_RAW_IOPATH_RAW2ISP,
                                                m_VidRecdConfigs.ubRawPreviewBitMode[0]);
            }
            if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}                                                
        }

        sRet |= MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_DEFAULT);
        if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}                                                        
    }

    m_bVidPreviewActive[ubSnrSel] = bEnable;

    if (bEnable == MMP_FALSE) {
        m_usVidStaticZoomIndex = 0;
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_PreviewStop
//  Description : Stop preview display mode.
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_PreviewStop(MMP_UBYTE ubSnrSel)
{
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    MMPS_3GPRECD_EnableRecordPipe(MMP_FALSE);
    MMPS_3GPRECD_EnablePreviewPipe(ubSnrSel, MMP_FALSE, MMP_FALSE);

    if (m_VidRecdModes.VidCurBufMode[0] == VIDENC_CURBUF_RT) {
        MMPD_IBC_SetH264RT_Enable(MMP_FALSE);
    }
   	
    /* This is a work around. After real-time H264 encode,
     * we have to reset IBC to avoid a corrupted still JPEG later.
     * Here we reset all of IBC pipes, instead of H.264 pipe only.
     */
    #if (MCR_V2_UNDER_DBG)
    MMPD_IBC_ResetModule(m_RecordFctlLink.ibcpipeID);
    MMPD_IBC_ResetModule(m_PreviewFctlLink.ibcpipeID);
    #endif

    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE   ||
        m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI    ||
        m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
        
        MMPD_IBC_ResetModule(m_LdcCaptureFctlLink.ibcpipeID);
        MMPD_IBC_ResetModule(m_LdcSrcFctlLink.ibcpipeID);
    }

    MMPD_Fctl_UnLinkPipeToLdc(m_LdcSrcFctlLink.ibcpipeID);

    if (gsHdrCfg.bVidEnable) {
        MMPD_System_ResetHModule(MMPD_SYS_MDL_VIF0, 	MMP_FALSE);
        MMPD_System_ResetHModule(MMPD_SYS_MDL_RAW_S0, 	MMP_TRUE);
        MMPD_System_ResetHModule(MMPD_SYS_MDL_RAW_S1, 	MMP_TRUE);
        MMPD_System_ResetHModule(MMPD_SYS_MDL_RAW_F, 	MMP_TRUE);
        MMPD_System_ResetHModule(MMPD_SYS_MDL_ISP, 		MMP_FALSE);	

        MMPD_HDR_UnInitModule(ubSnrSel);
    }

    // Reset the init frame buffer flag
    m_bInitFcamRecPipeFrmBuf = MMP_FALSE;

    #if (SUPPORT_COMPONENT_FLOW_CTL)
    MMP_CompCtl_UnLinkComponentList(m_ubCamPreviewListId[PRM_SENSOR], MMP_TRUE);
    #endif
 
    #if 1 // CHECK   
    // Reset the Buffer Address
    m_ulVidRecDramEndAddr   = 0;
    m_ulVidRecSramAddr      = 0;
    m_ulVideoPreviewEndAddr = 0;
    m_ulVidShareMvAddr      = 0;
    
    MMPD_3GPMGR_SetTempFileNameAddr(0, 0);
    #endif

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_PreviewStart
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set buffers and parameters. Then display preview mode.
 @param[in] bCheckFrameEnd 	The setting will check VIF frame end or not.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS Unsupport resolution.
*/
MMP_ERR MMPS_3GPRECD_PreviewStart(MMP_UBYTE ubSnrSel, MMP_BOOL bCheckFrameEnd)
{
    MMP_ERR     err;
    MMP_ULONG   ulStackAddr = 0;
    
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }
    
    if (m_ulVidRecDramEndAddr == 0 || m_ulVideoPreviewEndAddr == 0) {
        MMPS_Sensor_GetMemEnd(&ulStackAddr);
        m_ulVidRecDramEndAddr = m_ulVideoPreviewEndAddr = ulStackAddr;
    }
    
    MMPS_3GPRECD_AdjustPreviewRes(ubSnrSel);

    m_ub1stVRStreamSnrId = ubSnrSel;

    /* Only Raw path support PTZ or HDR functions */
    if (MMP_IsVidPtzEnable() || MMP_IsVidHDREnable()) {
        m_VidRecdConfigs.bRawPreviewEnable[0] = MMP_TRUE;
    }
    
    if (m_ulVidRecSramAddr == 0) {
        MMPD_System_GetSramEndAddr(&m_ulVidRecSramAddr);
    }
    
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI) {
        err = MMPS_3GPRECD_SetPreviewMemory(ubSnrSel,
                                            m_ulLdcMaxOutWidth,
                                            m_ulLdcMaxOutHeight,
                                            &ulStackAddr,
                                            &m_ulVidRecSramAddr);
    }
    else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
        err = MMPS_3GPRECD_SetPreviewMemory(ubSnrSel,
                                            m_ulVRPreviewW[ubSnrSel],
                                           	m_ulVRPreviewH[ubSnrSel],
                                            &ulStackAddr,
                                            &m_ulVidRecSramAddr);
    }
    else {
        err = MMPS_3GPRECD_SetPreviewMemory(ubSnrSel,
                                            m_ulVRPreviewW[ubSnrSel],
                                            m_ulVRPreviewH[ubSnrSel],
                                            &ulStackAddr,
                                            &m_ulVidRecSramAddr);
    }

    if (err) {
        RTNA_DBG_Str(0, "Alloc mem for video preview failed\r\n");
        return err;
    }
    
    MMPD_Display_ResetRotateDMABufIdx(MMP_DISPLAY_SRC_FRONTCAM);
    
    #if (SUPPORT_VR_THUMBNAIL)
    // TL: Due to Q-table was not initialized. The 1st VR thumbanil sizes is abnormal.
    {
        MMP_UBYTE i = 0;
    
        for (i = MMP_DSC_JPEG_RC_ID_CAPTURE; i < MMP_DSC_JPEG_RC_ID_NUM; i++) {
            MMPD_DSC_SetJpegQTableEx(i, (MMP_UBYTE *)MMPS_DSC_GetConfig()->encParams.ubDscQtable[MMP_DSC_CAPTURE_NORMAL_QUALITY],
                                        (MMP_UBYTE *)MMPS_DSC_GetConfig()->encParams.ubDscQtable[MMP_DSC_CAPTURE_NORMAL_QUALITY] + DSC_QTABLE_ARRAY_SIZE,
                                        (MMP_UBYTE *)MMPS_DSC_GetConfig()->encParams.ubDscQtable[MMP_DSC_CAPTURE_NORMAL_QUALITY] + DSC_QTABLE_ARRAY_SIZE,
                                        MMP_DSC_CAPTURE_NORMAL_QUALITY);        
        }
    }
    #endif

    #if (SUPPORT_COMPONENT_FLOW_CTL)
    if (CAM_CHECK_PRM(PRM_CAM_BAYER_SENSOR)) {
        
        if (m_ubCamPreviewListId[PRM_SENSOR] == 0xFF) {
            
            MMP_USHORT  usModeIdx   = m_VidRecdModes.usVideoPreviewMode;
            MMP_UBYTE   ubVifId     = MMPF_Sensor_GetVIFPad(m_VRPreviewFctlAttr[0].ubPipeLinkedSnr);
            MMP_UBYTE   ubPipeId    = m_VRPreviewFctlAttr[0].fctllink.ibcpipeID;
            MMP_UBYTE   ubCompIdArray[6];
            
            ubCompIdArray[0] = MMP_COMPONENT_ID_NULL;
            ubCompIdArray[1] = MMP_COMPONENT_ID_VIF2ISP;
            ubCompIdArray[2] = TRANS_PIPE_TO_PIPECOMP_ID(ubPipeId);
            ubCompIdArray[3] = TRANS_PIPE_TO_DRAMCOMP_ID(ubPipeId);
            ubCompIdArray[4] = MMP_COMPONENT_ID_DISP;
            ubCompIdArray[5] = MMP_COMPONENT_ID_NULL;
            
            if (MMP_FALSE == m_VidRecdConfigs.bRawPreviewEnable[0])
                MMP_CompCtl_RegisterVif2IspComponent(MMP_COMPONENT_USAGE_ID0, m_VRPreviewFctlAttr[0].ubPipeLinkedSnr, ubVifId);
            else
                MMP_CompCtl_RegisterRaw2IspComponent(MMP_COMPONENT_USAGE_ID0, m_VRPreviewFctlAttr[0].ubPipeLinkedSnr, ubVifId);
                    
            MMP_CompCtl_RegisterPipeComponent(MMP_COMPONENT_USAGE_ID0, (void*)&m_VRPreviewFctlAttr[0]);
            MMP_CompCtl_RegisterPipeOutDramComponent(MMP_COMPONENT_USAGE_ID0, (void*)&m_VRPreviewFctlAttr[0]);
            MMP_CompCtl_RegisterDisplayComponent(MMP_COMPONENT_USAGE_ID0,
                                                 GET_VR_PREVIEW_WINDOW(ubSnrSel),
                                                 m_VidRecdConfigs.previewdata[0].DispDevice[usModeIdx]);
            
            m_ubCamPreviewListId[PRM_SENSOR] = MMP_CompCtl_LinkComponentsEx(MMP_COMPONENT_USAGE_ID0, &ubCompIdArray[0], 6, NULL);
        }
    }
    #endif

    if (MMPS_3GPRECD_EnablePreviewPipe(ubSnrSel, MMP_TRUE, bCheckFrameEnd) != MMP_ERR_NONE) {
        RTNA_DBG_Str(0, "Enable Video Preview: Fail\r\n");
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetPreviewMemory
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set memory map for VideoR preview
 @param[in] usPreviewW Preview width.
 @param[in] usPreviewH Preview height.
 @param[in/out] ulStackAddr Available start address of dram buffer.
 @retval MMP_ERR_NONE Success.
*/
static MMP_ERR MMPS_3GPRECD_SetPreviewMemory(MMP_UBYTE ubSnrSel,
                                             MMP_USHORT usPreviewW,  MMP_USHORT usPreviewH, 
                                             MMP_ULONG *ulStackAddr, MMP_ULONG *ulFbAddr) 
{
    MMP_USHORT                      usModeIdx = m_VidRecdModes.usVideoPreviewMode; 
    MMP_USHORT                      i;
    MMP_ULONG                       ulCurAddr = *ulStackAddr; 
    MMP_ULONG                       ulTmpBufSize, ulYSize;
    MMP_ULONG                       ulPreviewBufSize    = 0;
    MMP_ULONG                       ulRawBufSize        = 0;
    MMP_ULONG                       ulLdcSrcBufSize     = 0;
    MMP_ULONG                       ulCaptureMaxBufSize;
    MMPS_3GPRECD_PREVIEW_DATA       *previewdata = &m_VidRecdConfigs.previewdata[0];
    MMPS_3GPRECD_PREVIEW_BUFINFO    previewbuf;
    MMP_RAW_STORE_BUF               rawBuf, rawEndBuf;
    MMP_LDC_SRC_BUFINFO             ldcSrcBuf;
    MMPS_3GPRECD_MDTC_BUFINFO       MdtcSrcBuf;
    MMP_UBYTE                       ubVifId = MMPD_Sensor_GetVIFPad(ubSnrSel);
    MMP_USHORT                      usOrigPreviewW = usPreviewW;
    MMP_USHORT                      usOrigPreviewH = usPreviewH;
    MMP_USHORT                      usPreviewBufW = 0, usPreviewBufH = 0;
    MMP_USHORT                      usMaxJpegW = 0, usMaxJpegH = 0;
    #if (SUPPORT_VR_REFIX_TAILINFO)
    MMP_USHORT                      usFixedTailInfoTempBufSize = 2048;
    MMP_ULONG                       ulAVRepackBufSize = 512 * 1024;
    MMP_ULONG                       ulTmpAddr[3];
    #endif
    
    AUTL_MEMDBG_BLOCK               sVRPrevwMemDbgBlk;
    MMP_UBYTE                       ubDbgItemIdx = 0;
    MMP_ULONG                       ulMemStart = 0;

    ///////////////////////////////////////////////////////////////////////////
    //
    //  Get Preview Config setting and calculate how many memory will be used    
    //
    ///////////////////////////////////////////////////////////////////////////
    
    // Get Preview buffer config and size
    previewbuf.usBufCnt = previewdata->usVidDispBufCnt[usModeIdx];

    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI) {
        if ((MMPF_LDC_MultiRunGetMaxLoopBackCount() + 1) > previewdata->usVidDispBufCnt[usModeIdx])
            previewbuf.usBufCnt = (MMPF_LDC_MultiRunGetMaxLoopBackCount() + 1);	
    }

    if (previewdata->bUseRotateDMA[usModeIdx])
        previewbuf.usRotateBufCnt = previewdata->usRotateBufCnt[usModeIdx];
    else
        previewbuf.usRotateBufCnt = 0;

    #if (HANDLE_JPEG_EVENT_BY_QUEUE)
    if (MMPF_JPEG_GetCtrlByQueueEnable()) {
        // These parameters are for allocate preview/rotate buffer 
        usPreviewW = (usOrigPreviewW > m_usMJPEGMaxEncWidth) ? (usOrigPreviewW) : (m_usMJPEGMaxEncWidth);
        usPreviewH = (usOrigPreviewH > m_usMJPEGMaxEncHeight) ? (usOrigPreviewH) : (m_usMJPEGMaxEncHeight);
    }
    #endif

    ulYSize = usPreviewW * usPreviewH;

    switch(previewdata->DispColorFmt[usModeIdx]) {
    case MMP_DISPLAY_COLOR_YUV420:
    case MMP_DISPLAY_COLOR_YUV420_INTERLEAVE:
        ulPreviewBufSize = ALIGN32(ulYSize) + ALIGN32(ulYSize >> 2) * 2;
        break;
    case MMP_DISPLAY_COLOR_YUV422:
    case MMP_DISPLAY_COLOR_RGB565:
        ulPreviewBufSize = ALIGN32(ulYSize) + ALIGN32(ulYSize >> 1) * 2;
        break;
    case MMP_DISPLAY_COLOR_RGB888:
        ulPreviewBufSize = ALIGN32(ulYSize) * 3;
        break;
    default:
        RTNA_DBG_Str(0, "Not Support Color Format\r\n");
        return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        break;
    }
    ulPreviewBufSize =  ALIGN32(ulPreviewBufSize);

    // Get Raw preview buffer config and size
    if (MMP_TRUE == m_VidRecdConfigs.bRawPreviewEnable[0]) {

        MMP_USHORT  usSensorW;
        MMP_USHORT  usSensorH;

        if (m_VidRecdConfigs.ulRawStoreBufCnt > VR_MAX_RAWSTORE_BUFFER_NUM) {
            m_VidRecdConfigs.ulRawStoreBufCnt = VR_MAX_RAWSTORE_BUFFER_NUM;
        }

        rawBuf.ulRawBufCnt = m_VidRecdConfigs.ulRawStoreBufCnt;
        rawEndBuf.ulRawBufCnt = rawBuf.ulRawBufCnt;
        
        MMPD_RAWPROC_GetStoreRange(ubVifId, &usSensorW, &usSensorH);

        if (gsHdrCfg.bVidEnable) {

            if (m_VidRecdConfigs.ulRawStoreBufCnt < VR_MIN_RAWSTORE_BUFFER_NUM) {
                rawBuf.ulRawBufCnt = m_VidRecdConfigs.ulRawStoreBufCnt = VR_MIN_RAWSTORE_BUFFER_NUM;
            }

            /* Plus 256 for buffer address alignment */
            if (gsHdrCfg.ubRawStoreBitMode == HDR_BITMODE_10BIT)
                ulRawBufSize = (2 * usSensorW * usSensorH * 4 / 3) + 256;
            else
                ulRawBufSize = (2 * usSensorW * usSensorH) + 256;
        }
        else {

            if (m_VidRecdConfigs.ubRawPreviewBitMode[0] == MMP_RAW_COLORFMT_BAYER10)
                ulRawBufSize = ALIGN32(usSensorW * usSensorH * 4 / 3);
            else
                ulRawBufSize = ALIGN32(usSensorW * usSensorH);
        }
    }
    else {
        rawBuf.ulRawBufCnt = 0;
        rawEndBuf.ulRawBufCnt = rawBuf.ulRawBufCnt;
        ulRawBufSize = 0;
    }
    
    // Get Graphics loopback source buffer config and size
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE   ||
        m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI) {
        ldcSrcBuf.usInBufCnt    = 2;
        ulLdcSrcBufSize         = ALIGN32(m_ulLdcMaxSrcWidth * m_ulLdcMaxSrcHeight * 3 / 2);
    }
    else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
        ldcSrcBuf.usInBufCnt    = (m_ubLdcResMode == MMP_LDC_RES_MODE_MS_736P) ? MAX_LDC_SRC_BUF_NUM : 2;
        ulLdcSrcBufSize         = ALIGN32(m_ulLdcMaxSrcWidth * m_ulLdcMaxSrcHeight * 3 / 2);
    }
    else {
        ldcSrcBuf.usInBufCnt    = 0;
        ulLdcSrcBufSize         = 0;
    }

    if (MMPS_Sensor_IsADASEnable(PRM_SENSOR)||MMPS_Sensor_IsVMDEnable(PRM_SENSOR))
	{
		// Get Motion detection buffer config and size
		#if (SUPPORT_MDTC)||(SUPPORT_ADAS)
		if (m_YFrameType == MMPS_3GPRECD_Y_FRAME_TYPE_ADAS) {
			#if (SUPPORT_ADAS)
			MMPS_Sensor_GetADASResolution(PRM_SENSOR, &gsADASFitRange.ulOutWidth, &gsADASFitRange.ulOutHeight);
			#endif
		}
		else {
			#if (SUPPORT_MDTC)
			MMPS_Sensor_GetVMDResolution(PRM_SENSOR, &gsADASFitRange.ulOutWidth, &gsADASFitRange.ulOutHeight);
			#endif
		}

		MdtcSrcBuf.usBufCnt = 2;
		//ulMdtcSrcBufSize    = ALIGN32(gsADASFitRange.ulOutWidth * gsADASFitRange.ulOutHeight);
		#else
		MdtcSrcBuf.usBufCnt = 0;
		//ulMdtcSrcBufSize    = 0;
		#endif
	}

    //////////////////////////////////////////////////////////////////////////
    //
    //  Allocate memory for preview buffer
    //
    //////////////////////////////////////////////////////////////////////////

    ulCurAddr = ALIGN32(*ulStackAddr);

    usPreviewBufW = usPreviewW;
    usPreviewBufH = usPreviewH;
    
    #if (SUPPORT_UVC_FUNC) && (HANDLE_JPEG_EVENT_BY_QUEUE)
    // For UVC mix mode source pipe (YUV)
    if (MMP_TRUE == PCAM2MMP_GetUVCMixMode() && MMPF_JPEG_GetCtrlByQueueEnable()) {
        usPreviewBufW = (usPreviewBufW < gusUvcYUY2MaxWidth) ? gusUvcYUY2MaxWidth : usPreviewBufW;
        usPreviewBufH = (usPreviewBufH < gusUvcYUY2MaxHeight) ? gusUvcYUY2MaxHeight : usPreviewBufH;
    }
    #endif

    AUTL_MemDbg_Init(&sVRPrevwMemDbgBlk, AUTL_MEMDBG_USAGE_ID_VR_PREVW);
    ulMemStart = ulCurAddr;

    // Allocate memory for preview buffer
    for (i = 0; i < previewbuf.usBufCnt; i++) {
        if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV420) {

            previewbuf.ulYBuf[i] = ulCurAddr;
            previewbuf.ulUBuf[i] = previewbuf.ulYBuf[i] + ALIGN32(ulYSize);
            previewbuf.ulVBuf[i] = previewbuf.ulUBuf[i] + ALIGN32(ulYSize >> 2);
        }
        else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV420_INTERLEAVE) {

            previewbuf.ulYBuf[i] = ulCurAddr;
            previewbuf.ulUBuf[i] = previewbuf.ulYBuf[i] + ALIGN32(ulYSize);
            previewbuf.ulVBuf[i] = previewbuf.ulUBuf[i];
        }
        else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV422 ||
                 previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_RGB565) {
            
            previewbuf.ulYBuf[i] = ulCurAddr;
            previewbuf.ulUBuf[i] = 0;
            previewbuf.ulVBuf[i] = 0;
        }
        else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_RGB888) {
            
            previewbuf.ulYBuf[i] = ulCurAddr;
            previewbuf.ulUBuf[i] = 0;
            previewbuf.ulVBuf[i] = 0;
        }
        else {
            RTNA_DBG_Str(0, "Not supported preview format\r\n");  
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }
        ulCurAddr += ulPreviewBufSize;
    }

    AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Preview");
    ulMemStart = ulCurAddr;

    // Allocate memory for rotate DMA destination buffer
	for (i = 0; i < previewbuf.usRotateBufCnt; i++) {
	
		if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV420) {
		    
		    previewbuf.ulRotateYBuf[i] = ulCurAddr;
            previewbuf.ulRotateUBuf[i] = previewbuf.ulRotateYBuf[i] + ALIGN32(ulYSize);
            previewbuf.ulRotateVBuf[i] = previewbuf.ulRotateUBuf[i] + ALIGN32(ulYSize >> 2);
	    }
		else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV420_INTERLEAVE) {

		    previewbuf.ulRotateYBuf[i] = ulCurAddr;
            previewbuf.ulRotateUBuf[i] = previewbuf.ulRotateYBuf[i] + ALIGN32(ulYSize);
            previewbuf.ulRotateVBuf[i] = previewbuf.ulRotateUBuf[i];
	    }
		else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV422 ||
				 previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_RGB565) {
		    
		    previewbuf.ulRotateYBuf[i] = ulCurAddr;
            previewbuf.ulRotateUBuf[i] = 0;
            previewbuf.ulRotateVBuf[i] = 0;
		}
        else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_RGB888) {
		    
		    previewbuf.ulRotateYBuf[i] = ALIGN32(ulCurAddr);
            previewbuf.ulRotateUBuf[i] = 0;
            previewbuf.ulRotateVBuf[i] = 0;
		}
		else {
		    RTNA_DBG_Str(0, "Not supported preview format\r\n");  
		    return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
		}
        ulCurAddr +=  ulPreviewBufSize;        
	}

	AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Rotate Preview");
	ulMemStart = ulCurAddr;

    // Allocate memory for raw preview buffer (include HDR)
    if (m_VidRecdConfigs.bRawPreviewEnable[0]) {

        MMP_USHORT usStoreW, usStoreH;

        if (gsHdrCfg.bVidEnable)
        {
            /* Use 256Bytes alignment address for HDR store buffer */
            for (i = 0; i < rawBuf.ulRawBufCnt; i++) {
                rawBuf.ulRawBufAddr[i] = ALIGN256(ulCurAddr);
                ulCurAddr = rawBuf.ulRawBufAddr[i] + ulRawBufSize;
            }

            MMPD_RAWPROC_SetStoreBuf(ubVifId, &rawBuf);

            MMPD_RAWPROC_EnablePreview(ubVifId, MMP_TRUE);

            MMPD_RAWPROC_GetStoreRange(ubVifId, &usStoreW, &usStoreH);
            MMPD_RAWPROC_SetFetchRange(0, 0, usStoreW * 2, usStoreH, usStoreW * 2);

            MMPD_HDR_InitModule(usStoreW, usStoreH);
			MMPD_HDR_SetBufEnd(ALIGN32(ulCurAddr));

			if (gsHdrCfg.ubRawStoreBitMode == HDR_BITMODE_10BIT)
		        MMPD_RAWPROC_EnablePath(ubSnrSel,
		                                ubVifId,
		                                MMP_TRUE,
						                MMP_RAW_IOPATH_ISP2RAW | MMP_RAW_IOPATH_RAW2ISP,
						                MMP_RAW_COLORFMT_BAYER10);
			else
		        MMPD_RAWPROC_EnablePath(ubSnrSel,
		                                ubVifId,
		                                MMP_TRUE,
						                MMP_RAW_IOPATH_ISP2RAW | MMP_RAW_IOPATH_RAW2ISP,
						                MMP_RAW_COLORFMT_BAYER8);
            
            m_sRawBuf[ubVifId]    = rawBuf;
            m_sRawEndBuf[ubVifId] = rawEndBuf;
        }
        else
        {
            for (i = 0; i < rawBuf.ulRawBufCnt; i++) {
                rawBuf.ulRawBufAddr[i] = ALIGN256(ulCurAddr);
                ulCurAddr = rawBuf.ulRawBufAddr[i] + ulRawBufSize;
                rawEndBuf.ulRawBufAddr[i] = ulCurAddr;
            }

            MMPD_RAWPROC_SetStoreBuf(ubVifId, &rawBuf);

            // Set Ring Buffer End Address
            MMPD_RAWPROC_SetStoreBufEnd(ubVifId, &rawEndBuf);
            
            MMPD_RAWPROC_EnableRingStore(ubVifId, MMP_RAW_STORE_PLANE0, MMP_TRUE);

            MMPD_HDR_UnInitModule(ubSnrSel);

            if (CAM_CHECK_PRM(PRM_CAM_YUV_SENSOR))
                MMPD_RAWPROC_SetStoreOnly(ubVifId, MMP_TRUE);
            else
                MMPD_RAWPROC_SetStoreOnly(ubVifId, MMP_FALSE);

            MMPD_RAWPROC_EnablePreview(ubVifId, MMP_TRUE);

            MMPD_RAWPROC_GetStoreRange(ubVifId, &usStoreW, &usStoreH);
           	MMPD_RAWPROC_SetFetchRange(0, 0, usStoreW, usStoreH, usStoreW);

            if (m_VidRecdConfigs.ubRawPreviewBitMode[0] == MMP_RAW_COLORFMT_YUV420 ||
                m_VidRecdConfigs.ubRawPreviewBitMode[0] == MMP_RAW_COLORFMT_YUV422) {
                MMPD_RAWPROC_EnablePath(ubSnrSel,
                                        ubVifId,
                                        MMP_TRUE,
                                        MMP_RAW_IOPATH_VIF2RAW,
                                        m_VidRecdConfigs.ubRawPreviewBitMode[0]);
            }
            else {
                MMPD_RAWPROC_EnablePath(ubSnrSel,
                                        ubVifId,
                                        MMP_TRUE,
                                        MMP_RAW_IOPATH_VIF2RAW | MMP_RAW_IOPATH_RAW2ISP,
                                        m_VidRecdConfigs.ubRawPreviewBitMode[0]);
            }

            m_sRawBuf[ubVifId]    = rawBuf;
            m_sRawEndBuf[ubVifId] = rawEndBuf;

            if (MMP_IsDualVifCamEnable() && MMP_GetDualSnrPrevwType() == DUALSNR_SINGLE_PREVIEW)
            {
                ubSnrSel    = SCD_SENSOR;
                ubVifId     = MMPD_Sensor_GetVIFPad(ubSnrSel);
                
                for (i = 0; i < rawBuf.ulRawBufCnt; i++) {
                    rawBuf.ulRawBufAddr[i] = ALIGN256(ulCurAddr);
                    ulCurAddr = rawBuf.ulRawBufAddr[i] + ulRawBufSize;
                    rawEndBuf.ulRawBufAddr[i] = ulCurAddr;
                }
                
                MMPD_RAWPROC_SetStoreBuf(ubVifId, &rawBuf);

                // Set Ring Buffer End Address
                MMPD_RAWPROC_SetStoreBufEnd(ubVifId, &rawEndBuf);
                
                MMPD_RAWPROC_EnableRingStore(ubVifId, MMP_RAW_STORE_PLANE0, MMP_TRUE);

                MMPD_HDR_UnInitModule(ubSnrSel);

                MMPD_RAWPROC_EnablePreview(ubVifId, MMP_TRUE);

                if (m_VidRecdConfigs.ubRawPreviewBitMode[0] == MMP_RAW_COLORFMT_YUV420 ||
                    m_VidRecdConfigs.ubRawPreviewBitMode[0] == MMP_RAW_COLORFMT_YUV422) {
                    MMPD_RAWPROC_EnablePath(ubSnrSel,
                                            ubVifId,
                                            MMP_TRUE,
                                            MMP_RAW_IOPATH_VIF2RAW,
                                            m_VidRecdConfigs.ubRawPreviewBitMode[0]);
                }
                else {
                    MMPD_RAWPROC_EnablePath(ubSnrSel,
                                            ubVifId,
                                            MMP_TRUE,
                                            MMP_RAW_IOPATH_VIF2RAW | MMP_RAW_IOPATH_RAW2ISP,
                                            m_VidRecdConfigs.ubRawPreviewBitMode[0]);
                }

                m_sRawBuf[ubVifId]    = rawBuf;
                m_sRawEndBuf[ubVifId] = rawEndBuf;

                /* Restore Sensor/VIF ID */
                ubSnrSel    = PRM_SENSOR;
                ubVifId     = MMPD_Sensor_GetVIFPad(ubSnrSel);
            }
        }
        
        ulCurAddr = ALIGN256(ulCurAddr);
    }
    else {

        MMPD_HDR_UnInitModule(ubSnrSel);

        MMPD_RAWPROC_EnablePreview(ubVifId, MMP_FALSE);
    }

    AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Raw Preview");
    ulMemStart = ulCurAddr;

    // Allocate memory for LDC source buffer (NV12)
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE   ||
        m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI    ||
        m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE)
    {
        ulTmpBufSize = m_ulLdcMaxSrcWidth * m_ulLdcMaxSrcHeight;

        for (i = 0; i < ldcSrcBuf.usInBufCnt; i++) {

            ldcSrcBuf.ulInYBuf[i] = ulCurAddr;
            ldcSrcBuf.ulInUBuf[i] = ldcSrcBuf.ulInYBuf[i] + ALIGN32(ulTmpBufSize);
            ldcSrcBuf.ulInVBuf[i] = ldcSrcBuf.ulInUBuf[i];

	        printc(">>LDC Src Buf[%d] Y %x U %x V %x\r\n",
	                i, ldcSrcBuf.ulInYBuf[i], ldcSrcBuf.ulInUBuf[i], ldcSrcBuf.ulInVBuf[i]);

	        ulCurAddr += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>1));
		}

		AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"LDC Source");
		ulMemStart = ulCurAddr;
		
		if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) 
		{
            #if (LDC_STITCH_BLENDING_EN)
            MMP_ULONG   ulLdcBlendBlockYAddr[LDC_STITCH_BLENDING_BLK_NUM][MAX_LDC_OUT_BUF_NUM];
            MMP_ULONG   ulLdcBlendBlockUAddr[LDC_STITCH_BLENDING_BLK_NUM][MAX_LDC_OUT_BUF_NUM];
            MMP_ULONG   ulLdcBlendBlockVAddr[LDC_STITCH_BLENDING_BLK_NUM][MAX_LDC_OUT_BUF_NUM];
            int         ulBlkIdx = 0;
            #endif
            
            if (MMP_IsDualVifCamEnable()) {
                if (MMP_GetParallelFrmStoreType() == PARALLEL_FRM_EQUIRETANGLE)
                    ulTmpBufSize = (5 * m_ulLdcMaxOutWidth / 2) * m_ulLdcMaxOutHeight;
                else if (MMP_GetParallelFrmStoreType() == PARALLEL_FRM_LEFT_RIGHT)
                    ulTmpBufSize = (2 * m_ulLdcMaxOutWidth) * m_ulLdcMaxOutHeight;
                else
                    ulTmpBufSize = m_ulLdcMaxOutWidth * m_ulLdcMaxOutHeight;
			}
			else {
				ulTmpBufSize = m_ulLdcMaxOutWidth * m_ulLdcMaxOutHeight;
			}
			
			m_ubLdcMaxOutBufNum = (m_ubLdcResMode == MMP_LDC_RES_MODE_MS_736P) ? MAX_LDC_OUT_BUF_NUM : 2;
			
			for (i = 0; i < m_ubLdcMaxOutBufNum; i++) {
				
				m_ulLdcOutStoreYAddr[i] = ulCurAddr;
				m_ulLdcOutStoreUAddr[i] = ulCurAddr + ALIGN32(ulTmpBufSize);
				m_ulLdcOutStoreVAddr[i] = ulCurAddr + ALIGN32(ulTmpBufSize);
				
			    MMPF_LDC_MultiSliceInitOutStoreBuf(i,
			    								   m_ulLdcOutStoreYAddr[i],
			                                       m_ulLdcOutStoreUAddr[i],
			                                       m_ulLdcOutStoreVAddr[i],
			                                       m_ubLdcMaxOutBufNum);
		        
		        MEMSET((void*)m_ulLdcOutStoreYAddr[i], 0x00, ulTmpBufSize);
		        MEMSET((void*)m_ulLdcOutStoreUAddr[i], 0x80, ulTmpBufSize / 2);
		        
			    ulCurAddr += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>1));
			}

			#if (LDC_STITCH_BLENDING_EN)
			ulTmpBufSize = LDC_STITCH_BLENDING_BLK_WIDTH * m_ulLdcMaxOutHeight;

			for (ulBlkIdx = 0; ulBlkIdx < LDC_STITCH_BLENDING_BLK_NUM; ulBlkIdx++) {
			
				for (i = 0; i < m_ubLdcMaxOutBufNum; i++) {
					
					ulLdcBlendBlockYAddr[ulBlkIdx][i] = ulCurAddr;
					ulLdcBlendBlockUAddr[ulBlkIdx][i] = ulCurAddr + ALIGN32(ulTmpBufSize);
					ulLdcBlendBlockVAddr[ulBlkIdx][i] = ulCurAddr + ALIGN32(ulTmpBufSize);
					
				    MMPF_LDC_MultiSliceInitBlendBlkBuf(ulBlkIdx,
				    								   i,
				    								   ulLdcBlendBlockYAddr[ulBlkIdx][i],
				                                       ulLdcBlendBlockUAddr[ulBlkIdx][i],
				                                       ulLdcBlendBlockVAddr[ulBlkIdx][i]);
			        
			        MEMSET((void*)ulLdcBlendBlockYAddr[ulBlkIdx][i], 0x00, ulTmpBufSize);
			        MEMSET((void*)ulLdcBlendBlockUAddr[ulBlkIdx][i], 0x80, ulTmpBufSize / 2);
			        
				    ulCurAddr += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>1));
				}
			}
			#endif
		}

		AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"LDC MS Out");
		ulMemStart = ulCurAddr;
	}
	
    if (MMPS_Sensor_IsADASEnable(PRM_SENSOR)||MMPS_Sensor_IsVMDEnable(PRM_SENSOR))
	{
		// Allocate memory for Motion detection source buffer (Y only)
		#if (SUPPORT_MDTC)||(SUPPORT_ADAS)
		ulCurAddr = ALIGN32(ulCurAddr);

    //Protection in case of ADAS is not enabled
    if( gsADASFitRange.ulOutWidth == 0 || gsADASFitRange.ulOutHeight == 0 )
    {
      //For MMPS_Sensor_AllocateVMDBuffer() below
      gsADASFitRange.ulOutWidth  = gstMdtcCfg[PRM_SENSOR].width;
      gsADASFitRange.ulOutHeight = gstMdtcCfg[PRM_SENSOR].height;
    }

    if (gbADASSrcYUV == 1)//YUV422
    {
    	ulTmpBufSize = gsADASFitRange.ulOutWidth * gsADASFitRange.ulOutHeight * 2;
    }else
    {
    	ulTmpBufSize = gsADASFitRange.ulOutWidth * gsADASFitRange.ulOutHeight;
    }

    #if (SUPPORT_ADAS)
    if (m_YFrameType == MMPS_3GPRECD_Y_FRAME_TYPE_ADAS) {
			if (MMPS_Sensor_IsADASEnable(PRM_SENSOR))
			{
			if (MMPS_Sensor_AllocateADASBuffer(PRM_SENSOR, &ulCurAddr, MMP_TRUE, ulTmpBufSize) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "Allocate ADAS buffer failed\r\n");
            return MMP_SENSOR_ERR_LDWS;
        }
    }
		}
    else
    #endif
    {
    	#if (SUPPORT_MDTC)
			if (MMPS_Sensor_IsVMDEnable(PRM_SENSOR))
			{
			if (MMPS_Sensor_AllocateVMDBuffer(PRM_SENSOR, &ulCurAddr, MMP_TRUE) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "Allocate MDTC buffer failed\r\n");
            return MMP_SENSOR_ERR_VMD;
        }
			}
        #endif
    }

    for (i = 0; i < MdtcSrcBuf.usBufCnt; i++) {
        MdtcSrcBuf.ulYBuf[i] = ulCurAddr;

        ulCurAddr += ALIGN32(ulTmpBufSize);
	}

	AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"ADAS/MDTC");
	ulMemStart = ulCurAddr;
	#endif
	}
	// Allocate memory for File name buffer
    MMPD_3GPMGR_SetTempFileNameAddr(ulCurAddr, MAX_3GPRECD_FILENAME_LENGTH);

    ulCurAddr += MAX_3GPRECD_FILENAME_LENGTH;
    
    AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FileName");
    ulMemStart = ulCurAddr;

    // Allocate memory for MV (Motion Vector) buffer
    MMPS_3GPRECD_AllocateMVBuffer(  &ulCurAddr,
                                    m_VidRecdConfigs.usEncWidth[m_VidRecdModes.usVideoMVBufResIdx],
                                    m_VidRecdConfigs.usEncHeight[m_VidRecdModes.usVideoMVBufResIdx]);

    AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"Motion Vector");
    ulMemStart = ulCurAddr;
    
    #if (defined(WIFI_PORT) && WIFI_PORT == 1)
    // Allocate memory for wifi stream
    {
        MMP_ULONG	ulWiFiH264CurAddr = ulCurAddr;
        MMP_ULONG	ulWiFiMJPEGCurAddr = ulCurAddr;
        
        #if (SUPPORT_H264_WIFI_STREAM)
        if (m_VidRecdConfigs.bH264WifiStreamEnable) {
            MMP_ERR status;
            
            status = MMPS_H264_WIFI_ReserveStreamBuf(&ulWiFiH264CurAddr, 
                                                     m_VidRecdConfigs.ulMaxH264WifiStreamWidth, 
                                                     m_VidRecdConfigs.ulMaxH264WifiStreamHeight,
                                                     VIDENC_CURBUF_FRAME);
            
            if (status != MMP_ERR_NONE) {
                RTNA_DBG_Str(0, "Allocate H264 Streaming Buffer failed!\r\n");	
            }
        }
        #endif
        
        #if (HANDLE_JPEG_EVENT_BY_QUEUE) && (SUPPORT_MJPEG_WIFI_STREAM)
        if (MMPF_JPEG_GetCtrlByQueueEnable()) {
            // For Front Cam Wifi Streaming
            m_ulFrontCamRsvdCompBufStart = ALIGN_X(ulWiFiMJPEGCurAddr, DSC_BUF_ALIGN_BASE);
            ulWiFiMJPEGCurAddr += m_ulFrontCamRsvdCompBufSize;
            
            m_ulFrontCamEncLineBufAddr = ALIGN_X(ulWiFiMJPEGCurAddr, DSC_BUF_ALIGN_BASE);
            ulWiFiMJPEGCurAddr += m_ulFrontCamEncLineBufSize;
        }
        #endif
        
        ulCurAddr = ulWiFiMJPEGCurAddr > ulWiFiH264CurAddr ? ulWiFiMJPEGCurAddr : ulWiFiH264CurAddr;

        AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam H264 Wifi Buffer");
        ulMemStart = ulCurAddr;
    }
    #endif

    // Allocate memory for still capture buffer
    if (m_VidRecdConfigs.bStillCaptureEnable
        #if (SUPPORT_UVC_FUNC)
        || (MMP_TRUE == PCAM2MMP_GetUVCMixMode()) // For UVC mix mode destination pipe (MJPEG)
        #endif
        ) 
    {
        ulCurAddr = ALIGN32(ulCurAddr);
        m_ulVidRecCaptureDramAddr = ulCurAddr;
        m_ulVidRecCaptureSramAddr = *ulFbAddr;

        // Pre-calculating maximum memory usage
        MMPS_DSC_SetShotMode(MMPS_DSC_SINGLE_SHOTMODE);
        #if (DSC_SUPPORT_BASELINE_MP_FILE)
        MMPS_DSC_EnableMultiPicFormat(MMP_FALSE, MMP_FALSE);
        #endif

        usMaxJpegW = m_usMaxStillJpegW;
        usMaxJpegH = m_usMaxStillJpegH;
        
        #if (SUPPORT_UVC_FUNC)
        if (usMaxJpegW < gusUvcYUY2MaxWidth) {
            MMP_PRINT_RET_ERROR(0, 0, "UVC MJPG width exceeds max JPG width!", 0);
            RTNA_DBG_Str(0, "Max JPG W:"); RTNA_DBG_Short(0, usMaxJpegW);
            RTNA_DBG_Str(0, " UVC MJPG W:"); RTNA_DBG_Short(0, gusUvcYUY2MaxWidth); RTNA_DBG_Str(0, "\r\n");
            usMaxJpegW = gusUvcYUY2MaxWidth;    
        }

        if (usMaxJpegH < gusUvcYUY2MaxHeight) {
            MMP_PRINT_RET_ERROR(0, 0, "UVC MJPG height exceeds max JPG height!", 0);
            RTNA_DBG_Str(0, "Max JPG H:"); RTNA_DBG_Short(0, usMaxJpegH);
            RTNA_DBG_Str(0, " UVC MJPG H:"); RTNA_DBG_Short(0, gusUvcYUY2MaxHeight); RTNA_DBG_Str(0, "\r\n");            
            usMaxJpegH = gusUvcYUY2MaxHeight;    
        }        
        #endif
        
        MMPS_DSC_SetJpegEncParam(DSC_RESOL_IDX_UNDEF, usMaxJpegW, usMaxJpegH, MMP_DSC_JPEG_RC_ID_CAPTURE);

        #ifdef MCR_V2_32MB
        MMPS_DSC_ConfigThumbnail(0, 0, MMP_DSC_THUMB_SRC_NONE); // Disable Thumbnail to get 800 KB extra DRAM space
        #else
        MMPS_DSC_ConfigThumbnail(VR_MAX_THUMBNAIL_WIDTH, VR_MAX_THUMBNAIL_HEIGHT, MMP_DSC_THUMB_SRC_DECODED_JPEG);
        #endif
        
        MMPS_DSC_SetSystemBuf(&ulCurAddr, MMP_FALSE, MMP_TRUE, MMP_TRUE);
        MMPS_DSC_SetCaptureBuf(ulFbAddr, &ulCurAddr, MMP_TRUE, MMP_FALSE, MMP_DSC_CAPTURE_NO_ROTATE, MMP_TRUE);

        m_ulVidRecCaptureEndSramAddr = *ulFbAddr;
        m_ulVidRecCaptureEndDramAddr = ulCurAddr;
        
        AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Still Capture");
        ulMemStart = ulCurAddr;

        // Maximum memory usage for rear cam capture
        if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264) || \
            CAM_CHECK_USB(USB_CAM_SONIX_MJPEG)) {
              
            if (m_VidRecdConfigs.bDualCaptureEnable) {
                ulCurAddr = ALIGN32(ulCurAddr);
                m_ulVidRecRearCamCaptDramAddr = ulCurAddr;
                ulCurAddr += m_ulVidRecRearCamCaptDramSize;

                AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Still Capture");
                ulMemStart = ulCurAddr;
            }
        }
        
        #if (HANDLE_JPEG_EVENT_BY_QUEUE)
        if (MMPF_JPEG_GetCtrlByQueueEnable())
        {
            #if (USE_H264_CUR_BUF_AS_CAPT_BUF)
            {
                MMP_ULONG   ulEncW, ulEncH;
                MMP_UBYTE*  pBuf = NULL;
                
                MMPS_3GPRECD_SetEncodeRes(MMPS_3GPRECD_FILESTREAM_NORMAL);

                ulEncW = m_ulVREncodeW[0];
                ulEncH = m_ulVREncodeH[0];
                
                m_VidRecdInputBuf[0].ulBufCnt = 2;

                if (usMaxJpegW < ulEncW) {
                    MMP_PRINT_RET_ERROR(0, 0, "VR encode width exceeds max JPG width!", 0);
                    RTNA_DBG_Str(0, "Max JPG W:"); RTNA_DBG_Short(0, usMaxJpegW);
                    RTNA_DBG_Str(0, " VR enc W:"); RTNA_DBG_Short(0, ulEncW); RTNA_DBG_Str(0, "\r\n");
                }
                if (usMaxJpegH < ulEncH) {
                    MMP_PRINT_RET_ERROR(0, 0, "VR encode height exceeds max JPG height!", 0);
                    RTNA_DBG_Str(0, "Max JPG H:"); RTNA_DBG_Short(0, usMaxJpegH);
                    RTNA_DBG_Str(0, " VR enc H:"); RTNA_DBG_Short(0, ulEncH); RTNA_DBG_Str(0, "\r\n");
                }
                
                for (i = 0; i < m_VidRecdInputBuf[0].ulBufCnt; i++) {
                    
                    ulTmpBufSize = ulEncW * ulEncH;

                    m_VidRecdInputBuf[0].ulY[i] = ALIGN32(ulCurAddr);
                    m_VidRecdInputBuf[0].ulU[i] = m_VidRecdInputBuf[0].ulY[i] + ulTmpBufSize;
                    m_VidRecdInputBuf[0].ulV[i] = m_VidRecdInputBuf[0].ulU[i];

                    ulCurAddr += (ulTmpBufSize * 3) / 2;
                    
                    // Workaround to reduce blur image in the bottom of frame
                    memset((MMP_UBYTE*)m_VidRecdInputBuf[0].ulY[i], 0, (ulTmpBufSize * 3) / 2 );
                    
                    // Fill the last 8 line as black color
                    pBuf = (MMP_UBYTE *)(m_VidRecdInputBuf[0].ulU[i] - (ulEncW * 8));
                    MEMSET(pBuf, 0x00, ulEncW * 8);
                    pBuf = (MMP_UBYTE *)(m_VidRecdInputBuf[0].ulU[i] + (ulEncW * ulEncH / 2) - (ulEncW * 4));
                    MEMSET(pBuf, 0x80, ulEncW * 4);
                }
                
                m_ulVidRecCaptureFrmBufAddr = m_VidRecdInputBuf[0].ulY[0];
            }
            
            #else // USE_H264_CUR_BUF_AS_CAPT_BUF
            
            if (usMaxJpegW < VR_MAX_CAPTURE_WIDTH) {
                MMP_PRINT_RET_ERROR(0, 0, "VR encode width exceeds max JPG width!", 0);
                RTNA_DBG_Str(0, "Max JPG W:"); RTNA_DBG_Short(0, usMaxJpegW); RTNA_DBG_Str(0, "\r\n");
            }
            if (usMaxJpegH < VR_MAX_CAPTURE_HEIGHT) {
                MMP_PRINT_RET_ERROR(0, 0, "VR encode height exceeds max JPG height!", 0);
                RTNA_DBG_Str(0, "Max JPG H:"); RTNA_DBG_Short(0, usMaxJpegH); RTNA_DBG_Str(0, "\r\n");
            }
            
            // For store front cam capture frame (NV12/I420)
            m_ulVidRecCaptureFrmBufAddr = ulCurAddr;
            ulTmpBufSize = (VR_MAX_CAPTURE_WIDTH * VR_MAX_CAPTURE_HEIGHT) * 3 / 2;

            ulCurAddr += ulTmpBufSize;
            MEMSET((MMP_UBYTE*)m_ulVidRecCaptureFrmBufAddr, 0x00, ulTmpBufSize);
            #endif
        }
        #endif

        AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Capture Buffer");
        ulMemStart = ulCurAddr;
    }
    else {
        m_ulVidRecCaptureDramAddr = 0;
        ulCaptureMaxBufSize = 0;
    }

    #if (SUPPORT_VR_REFIX_TAILINFO)
    ulCurAddr = ALIGN32(ulCurAddr);
    ulTmpAddr[0] = ulCurAddr;
    ulCurAddr += ALIGN32(usFixedTailInfoTempBufSize); 
    ulTmpAddr[1] = ulCurAddr;
    ulCurAddr += ALIGN32(ulAVRepackBufSize); 
    ulTmpAddr[2] = ulCurAddr;
    ulCurAddr += m_VidRecdConfigs.ulReFixTailBufSize;
    
    MMPD_3GPMGR_SetTempBuf2FixedTailInfo(tmp_addr[0], usFixedTailInfoTempBufSize,
                                         tmp_addr[1], ulAVRepackBufSize,
                                         tmp_addr[2], m_VidRecdConfigs.ulReFixTailBufSize);

    AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"ReFix Tail Buffer");
    ulMemStart = ulCurAddr;
    #endif

    #if (SUPPORT_UVC_FUNC)
    // Allocate PCAM slot buffer for UVC mix mode.
    if (MMP_TRUE == PCAM2MMP_GetUVCMixMode()) {
        MMP_ULONG UvcBufferSize = 0;
        
        // Reserve worst case buffer size.
        UvcBufferSize = PCAM2MMP_SetPcamBufferAddr(ulCurAddr, PCAM_USB_VIDEO_FORMAT_YUV422, gusUvcYUY2MaxWidth, gusUvcYUY2MaxHeight);
        ulCurAddr += UvcBufferSize;

        AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"UVC Preview Buffer");
        ulMemStart = ulCurAddr;
    }
    #endif

    AUTL_MemDbg_ShowAllItems(&sVRPrevwMemDbgBlk, MMP_TRUE);

    if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
	{
	    MMP_UBYTE sRet;
	    MMP_USHORT usStickerSrcWidth = 0, usStickerSrcHeight = 0, usDstStartx = 0, usDstStarty = 0;
                
	    sRet = MMPF_VIDENC_GetSWStickerAttribute(&usStickerSrcWidth, &usStickerSrcHeight, &usDstStartx, &usDstStarty);
	    ulCurAddr = ALIGN16(ulCurAddr); 
	    MEMSET((void*)(ulCurAddr), 0x0, usStickerSrcWidth * usStickerSrcHeight);               
	    sRet = MMPF_VIDENC_SetSWStickerAddress(ulCurAddr);        
	    if(sRet != MMP_ERR_NONE){MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk);}                                                        
	    ulCurAddr += usStickerSrcWidth * usStickerSrcHeight; 
	    MMP_PRINT_RET_ERROR(0, ulCurAddr, "",gubMmpDbgBk);
	}
	
    *ulStackAddr = ulCurAddr;
    m_ulVidRecDramEndAddr = m_ulVideoPreviewEndAddr = ulCurAddr;

    #if defined(ALL_FW)
    if (m_ulVidRecDramEndAddr > MMPS_System_GetMemHeapEnd()) {
        RTNA_DBG_Str(0, "\t= [HeapMemErr] Video preview =\r\n");
        return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
    }
    printc("End of 1st video preview buffers = 0x%X\r\n", m_ulVidRecDramEndAddr);
    #endif

    MMPS_3GPRECD_SetPreviewPipeConfig(&previewbuf, &ldcSrcBuf, &MdtcSrcBuf, usOrigPreviewW, usOrigPreviewH, ubSnrSel);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetPreviewEndAddr
//  Description :
//------------------------------------------------------------------------------
MMP_ULONG MMPS_3GPRECD_GetPreviewEndAddr(void)
{
    return m_ulVideoPreviewEndAddr;
}

#if 0
void ____VR_2nd_Preview_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_Set2ndSnrPreviewPipeConfig
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set pipe config.
 @param[in] *pPreviewBuf 	Pointer to Preview buffer.
 @param[in] usPreviewW 		The preview buffer width.
 @param[in] usPreviewH 		The preview buffer height.
 @retval MMP_ERR_NONE Success.
*/
static MMP_ERR MMPS_3GPRECD_Set2ndSnrPreviewPipeConfig( MMPS_3GPRECD_PREVIEW_BUFINFO 	*pPreviewBuf,
										  		        MMP_USHORT 					    usPreviewW, 
										  		        MMP_USHORT 					    usPreviewH,
										  		        MMP_UBYTE                       ubSnrSel)
{
    MMP_USHORT				usModeIdx = m_VidRecdModes.usVideoPreviewMode; 
    MMP_ULONG				ulScalInW, ulScalInH;
    MMP_ULONG          		ulRotateW, ulRotateH;
    MMP_BOOL				bDmaRotateEn;
    MMP_SCAL_FIT_MODE		sFitMode;
    MMP_SCAL_FIT_RANGE		fitrange;
    MMP_SCAL_GRAB_CTRL   	previewGrabctl, DispGrabctl;
    MMP_DISPLAY_DISP_ATTR	dispAttr;
    MMP_DISPLAY_ROTATE_TYPE	ubDmaRotateDir;
    MMPD_FCTL_ATTR          fctlAttr;
    MMP_USHORT              i;
    MMP_ULONG               ulDispStartX, ulDispStartY, ulDispWidth, ulDispHeight;
    MMP_SCAL_FIT_RANGE      sFitRangeBayer;
    MMP_SCAL_GRAB_CTRL      sGrabctlBayer;
    //MMP_USHORT            usCurZoomStep = 0;
    MMP_DISPLAY_WIN_ID      ePreviewWinID = GET_VR_PREVIEW_WINDOW(ubSnrSel);
    MMP_DISPLAY_DEV_TYPE    ePreviewDev = m_VidRecdConfigs.previewdata[1].DispDevice[usModeIdx];

    /* Parameter Check */
    if (m_VidRecdConfigs.previewpath[1] == MMP_3GP_PATH_INVALID) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    if (pPreviewBuf == NULL) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    /* Get the preivew display parameters */
    MMPS_Sensor_GetCurPrevScalInputRes(ubSnrSel, &ulScalInW, &ulScalInH);
    
    MMPD_BayerScaler_GetZoomInfo(MMP_BAYER_SCAL_DOWN, &sFitRangeBayer, &sGrabctlBayer); 

    if (m_sAhcVideoPrevInfo[ubSnrSel].bUserDefine) {
        bDmaRotateEn    = m_sAhcVideoPrevInfo[ubSnrSel].bPreviewRotate;
        ubDmaRotateDir	= m_sAhcVideoPrevInfo[ubSnrSel].sPreviewDmaDir;
        sFitMode 		= m_sAhcVideoPrevInfo[ubSnrSel].sFitMode;
        ulDispStartX  	= m_sAhcVideoPrevInfo[ubSnrSel].ulDispStartX;
        ulDispStartY  	= m_sAhcVideoPrevInfo[ubSnrSel].ulDispStartY;
        ulDispWidth   	= m_sAhcVideoPrevInfo[ubSnrSel].ulDispWidth;
        ulDispHeight  	= m_sAhcVideoPrevInfo[ubSnrSel].ulDispHeight;
    }
    else {
        bDmaRotateEn	= m_VidRecdConfigs.previewdata[1].bUseRotateDMA[usModeIdx];
        ubDmaRotateDir	= m_VidRecdConfigs.previewdata[1].ubDMARotateDir[usModeIdx];
        sFitMode 		= m_VidRecdConfigs.previewdata[1].sFitMode[usModeIdx];
        ulDispStartX  	= m_VidRecdConfigs.previewdata[1].usVidDispStartX[usModeIdx];
        ulDispStartY  	= m_VidRecdConfigs.previewdata[1].usVidDispStartY[usModeIdx];
        ulDispWidth   	= m_VidRecdConfigs.previewdata[1].usVidDisplayW[usModeIdx];
        ulDispHeight  	= m_VidRecdConfigs.previewdata[1].usVidDisplayH[usModeIdx];
    }

    /* Initial zoom relative config */ 
    MMPS_3GPRECD_InitDigitalZoomParam(m_2ndPrewFctlLink.scalerpath);

    MMPS_3GPRECD_RestoreDigitalZoomRange(m_2ndPrewFctlLink.scalerpath);

    if (m_VidRecdConfigs.previewpath[1] == MMP_3GP_PATH_2PIPE) 
    {
        // Config Video Preview Pipe
        fitrange.ulInWidth 	    = ulScalInW;
        fitrange.ulInHeight	    = ulScalInH;

        fitrange.fitmode    	= sFitMode;
        fitrange.scalerType		= MMP_SCAL_TYPE_SCALER;
        fitrange.ulOutWidth     = usPreviewW;
        fitrange.ulOutHeight    = usPreviewH;

        fitrange.ulInGrabX		= 1;
        fitrange.ulInGrabY		= 1;
        fitrange.ulInGrabW		= fitrange.ulInWidth;
        fitrange.ulInGrabH		= fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;

        #if 1
        MMPD_Scaler_GetGCDBestFitScale(&fitrange, &previewGrabctl);
        #else
        MMPD_PTZ_InitPtzInfo(m_2ndPrewFctlLink.scalerpath,
                             fitrange.fitmode,
                             fitrange.ulInWidth, fitrange.ulInHeight,
                             fitrange.ulOutWidth, fitrange.ulOutHeight);

        MMPD_PTZ_GetCurPtzStep(m_2ndPrewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);

        MMPD_PTZ_CalculatePtzInfo(m_2ndPrewFctlLink.scalerpath, usCurZoomStep, 0, 0);

        MMPD_PTZ_GetCurPtzInfo(m_2ndPrewFctlLink.scalerpath, &fitrange, &previewGrabctl);
        #endif
        
        fctlAttr.bRtModeOut         = MMP_FALSE;
        fctlAttr.colormode          = m_VidRecdConfigs.previewdata[1].DispColorFmt[usModeIdx];
        if (fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB565 || 
            fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB888) {
            fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_RGB;
        }
        else {
            #if (CCIR656_FORCE_SEL_BT601)
            fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
            #else
            fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_YUV_FULLRANGE;
            #endif
        }
        fctlAttr.fctllink           = m_2ndPrewFctlLink;
        fctlAttr.fitrange           = fitrange;
        fctlAttr.grabctl            = previewGrabctl;
        if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) || 
            (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
            fctlAttr.scalsrc        = MMP_SCAL_SOURCE_GRA;
        }
        else if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
            fctlAttr.scalsrc        = MMP_SCAL_SOURCE_ISP;
        }
        fctlAttr.sScalDelay         = m_sFullSpeedScalDelay;
        fctlAttr.bSetScalerSrc      = MMP_TRUE;
        fctlAttr.ubPipeLinkedSnr    = ubSnrSel;
        fctlAttr.usBufCnt           = pPreviewBuf->usBufCnt;

        for (i = 0; i < fctlAttr.usBufCnt; i++) {
            fctlAttr.ulBaseAddr[i] = pPreviewBuf->ulYBuf[i];
            fctlAttr.ulBaseUAddr[i] = pPreviewBuf->ulUBuf[i];
            fctlAttr.ulBaseVAddr[i] = pPreviewBuf->ulVBuf[i];
        }

        if (bDmaRotateEn) {
            fctlAttr.bUseRotateDMA  = MMP_TRUE;
            fctlAttr.usRotateBufCnt = pPreviewBuf->usRotateBufCnt;
            
            for (i = 0; i < fctlAttr.usRotateBufCnt; i++) {
                fctlAttr.ulRotateAddr[i] = pPreviewBuf->ulRotateYBuf[i];
                fctlAttr.ulRotateUAddr[i] = pPreviewBuf->ulRotateUBuf[i];
                fctlAttr.ulRotateVAddr[i] = pPreviewBuf->ulRotateVBuf[i];
            }
        }
        else {
            fctlAttr.bUseRotateDMA = MMP_FALSE;
            fctlAttr.usRotateBufCnt = 0;
        }
	
	    m_VRPreviewFctlAttr[1] = fctlAttr;
	
        MMPD_Fctl_SetPipeAttrForIbcFB(&fctlAttr);

		MMPD_Fctl_ClearPreviewBuf(m_2ndPrewFctlLink.ibcpipeID, 0xFFFFFF);

        if (bDmaRotateEn) {
            MMPD_Fctl_LinkPipeToDma(m_2ndPrewFctlLink.ibcpipeID, 
                                    ePreviewWinID, 
                                    ePreviewDev,
                                    ubDmaRotateDir);
        }
        else {
            MMPD_Fctl_LinkPipeToDisplay(m_2ndPrewFctlLink.ibcpipeID, 
                                        ePreviewWinID, 
                                        ePreviewDev);
        }

        // Config Display Window
        if ((m_VidRecdConfigs.previewdata[1].VidDispDir[usModeIdx] == MMP_DISPLAY_ROTATE_NO_ROTATE) ||
            (m_VidRecdConfigs.previewdata[1].VidDispDir[usModeIdx] == MMP_DISPLAY_ROTATE_RIGHT_180)) {                        
            ulRotateW = usPreviewW;
            ulRotateH = usPreviewH;
        }
        else {
            ulRotateW = usPreviewH;
            ulRotateH = usPreviewW;
        }
        
        if (bDmaRotateEn) {
        	// Rotate 90/270 for vertical panel
        	ulRotateW = usPreviewH;
            ulRotateH = usPreviewW;
        }
        
        dispAttr.usStartX = 0;
        dispAttr.usStartY = 0;
        if (m_sAhcVideoPrevInfo[ubSnrSel].bUserDefine) {
            dispAttr.usDisplayOffsetX = ulDispStartX;
            dispAttr.usDisplayOffsetY = ulDispStartY;
        }
        else {
            dispAttr.usDisplayOffsetX = (ulDispWidth > ulRotateW) ? ((ulDispWidth - ulRotateW) >> 1) : (0);
            dispAttr.usDisplayOffsetY = (ulDispHeight > ulRotateH) ? ((ulDispHeight - ulRotateH) >> 1) : (0);	
        }
        
        if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER))
        {
            #if 0//(TV_IMAGE_FLIP_MIRROR)
            MMP_ULONG ulCurMode = 0;
            
            if (ulCurMode == VIDEOPLAY_MODE_STD_LCD) {
                dispAttr.bMirror    = !m_VidRecdConfigs.previewdata[1].bVidDispMirror[usModeIdx];
            }
            else {
                dispAttr.bMirror    = m_VidRecdConfigs.previewdata[1].bVidDispMirror[usModeIdx];
            }
            #else
            dispAttr.bMirror        = m_VidRecdConfigs.previewdata[1].bVidDispMirror[usModeIdx];
            #endif
        }
        else {
            dispAttr.bMirror        = m_VidRecdConfigs.previewdata[1].bVidDispMirror[usModeIdx];
        }
        
        if (bDmaRotateEn) {
            dispAttr.rotatetype    = MMP_DISPLAY_ROTATE_NO_ROTATE;
        }
        else {
            dispAttr.rotatetype    = m_VidRecdConfigs.previewdata[1].VidDispDir[usModeIdx];
        }

        dispAttr.usDisplayWidth    = ulRotateW;
        dispAttr.usDisplayHeight   = ulRotateH;

        if (m_bHdmiInterlace) {
            dispAttr.usDisplayHeight = ulRotateH / 2;
        }
        
        MMPD_Display_SetWinToDisplay(ePreviewWinID, &dispAttr);

        fitrange.fitmode        = sFitMode;
        fitrange.scalerType 	= MMP_SCAL_TYPE_WINSCALER;
        fitrange.ulInWidth      = ulRotateW;
        fitrange.ulInHeight     = ulRotateH;
        fitrange.ulOutWidth     = ulRotateW;
        fitrange.ulOutHeight    = ulRotateH;
 
 		fitrange.ulInGrabX 		= 1;
        fitrange.ulInGrabY 		= 1;
        fitrange.ulInGrabW 		= fitrange.ulInWidth;
        fitrange.ulInGrabH 		= fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;

        MMPD_Scaler_GetGCDBestFitScale(&fitrange, &DispGrabctl);

        MMPD_Display_SetWinScaling(ePreviewWinID, &fitrange, &DispGrabctl);

        MMPS_Sensor_SetVMDPipe(SCD_SENSOR, m_2ndPrewFctlLink.ibcpipeID);

        // Tune TV IN preview MCI priority for Icon overflow issue.
        if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) || 
            (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
            #if (SUPPORT_COMPONENT_FLOW_CTL)
            MMPD_VIDENC_TunePipeMaxMCIPriority(m_2ndPrewFctlLink.ibcpipeID);
            #else
            MMPD_VIDENC_TunePipe2ndMCIPriority(m_2ndPrewFctlLink.ibcpipeID);
            #endif
        }
    }
    else {
        // TBD
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_Enable2ndSnrPreviewPipe
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Turn on and off preview for video encode.

 @param[in] bEnable 			Enable and disable scaler path for video preview.
 @param[in] bCheckFrameEnd 		When "bEnable" is MMP_TRUE, the setting means check frame end or not.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_Enable2ndSnrPreviewPipe(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable, MMP_BOOL bCheckFrameEnd)
{
    MMP_DISPLAY_WIN_ID  winId   = GET_VR_PREVIEW_WINDOW(ubSnrSel);
    MMP_UBYTE           ubVifId = MMPD_Sensor_GetVIFPad(ubSnrSel);
    
    if (!(bEnable ^ m_bVidPreviewActive[ubSnrSel])) {
        return MMP_ERR_NONE;
    }

    if (bEnable) 
    {
        if (MMPD_Fctl_EnablePreview(ubSnrSel, m_2ndPrewFctlLink.ibcpipeID, bEnable, bCheckFrameEnd)) {
            return MMP_3GPRECD_ERR_GENERAL_ERROR;
        }
    }
    else 
    {
        if (MMPD_Fctl_EnablePreview(ubSnrSel, m_2ndPrewFctlLink.ibcpipeID, bEnable, MMP_TRUE)) {
            return MMP_3GPRECD_ERR_GENERAL_ERROR;
        }

    	 if (MMPS_Display_GetFreezeWinEn() == MMP_FALSE)
    	 {
    		 MMPD_Display_SetWinActive(winId, MMP_FALSE);
    	 }

		if (MMPS_Sensor_IsVMDStarted(SCD_SENSOR)) {
			MMPS_Sensor_StartVMD(SCD_SENSOR, MMP_FALSE);
		}
        if (m_VidRecdConfigs.bRawPreviewEnable[1]) {
            MMPD_RAWPROC_EnablePreview(ubVifId, MMP_FALSE);
            
            if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) ||\
                (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
                MMPD_RAWPROC_EnablePath(ubSnrSel,
                                        ubVifId,
                                        MMP_FALSE,
                                        MMP_RAW_IOPATH_VIF2RAW,
                                        m_VidRecdConfigs.ubRawPreviewBitMode[1]);
            }                            
            else if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
                MMPD_RAWPROC_EnablePath(ubSnrSel,
                                        ubVifId,
                                        MMP_FALSE,
                                        MMP_RAW_IOPATH_VIF2RAW | MMP_RAW_IOPATH_RAW2ISP,
                                        m_VidRecdConfigs.ubRawPreviewBitMode[1]);
            }
        }
    }

    m_bVidPreviewActive[ubSnrSel] = bEnable;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_Set2ndSnrPreviewMemory
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set memory map for VideoR preview
 @param[in] usPreviewW Preview width.
 @param[in] usPreviewW Preview height.
 @param[in/out] ulStackAddr Available start address of dram buffer.
 @retval MMP_ERR_NONE Success.
*/
static MMP_ERR MMPS_3GPRECD_Set2ndSnrPreviewMemory( MMP_UBYTE   ubSnrSel,
                                                    MMP_USHORT  usPreviewW,  
                                                    MMP_USHORT  usPreviewH, 
									  		        MMP_ULONG   *ulStackAddr)
{
    MMP_USHORT                      usModeIdx = m_VidRecdModes.usVideoPreviewMode; 
    MMP_USHORT                      i;
    MMP_ULONG                       ulCurAddr = *ulStackAddr; 
    MMP_ULONG                       ulTmpBufSize;
    MMP_ULONG                       ulPreviewBufSize = 0;
    MMP_ULONG                       ulRawBufSize = 0;
    MMPS_3GPRECD_PREVIEW_DATA       *previewdata = &m_VidRecdConfigs.previewdata[1];
    MMPS_3GPRECD_PREVIEW_BUFINFO    previewbuf;
    MMP_RAW_STORE_BUF               rawBuf, rawEndBuf;
    MMP_UBYTE                       ubVifId = MMPD_Sensor_GetVIFPad(ubSnrSel);
    MMPS_3GPRECD_MDTC_BUFINFO       MdtcSrcBuf;
    AUTL_MEMDBG_BLOCK               sVRPrevwMemDbgBlk;
    MMP_UBYTE                       ubDbgItemIdx = 0;
    MMP_ULONG                       ulMemStart = 0;
    
    ///////////////////////////////////////////////////////////////////////////
    //
    //  Get Preview Config setting and calculate how many memory will be used    
    //
    ///////////////////////////////////////////////////////////////////////////
    
    // Get Preview buffer config and size
    previewbuf.usBufCnt = previewdata->usVidDispBufCnt[usModeIdx];

    if (previewdata->bUseRotateDMA[usModeIdx])
        previewbuf.usRotateBufCnt = previewdata->usRotateBufCnt[usModeIdx];
    else
        previewbuf.usRotateBufCnt = 0;

    switch(previewdata->DispColorFmt[usModeIdx]) {
    case MMP_DISPLAY_COLOR_YUV420:
        ulPreviewBufSize = ALIGN32(usPreviewW * usPreviewH) + ALIGN32((usPreviewW*usPreviewH)>>2)*2;
        break;
    case MMP_DISPLAY_COLOR_YUV420_INTERLEAVE:
        ulPreviewBufSize = ALIGN32(usPreviewW * usPreviewH) + ALIGN32(usPreviewW*(usPreviewH>>1));
        break;
    case MMP_DISPLAY_COLOR_YUV422:
    case MMP_DISPLAY_COLOR_RGB565:
        ulPreviewBufSize = ALIGN32(usPreviewW * 2 * usPreviewH);
        break;
    case MMP_DISPLAY_COLOR_RGB888:
        ulPreviewBufSize = ALIGN32(usPreviewW * 3 * usPreviewH);
        break;
    default:
        RTNA_DBG_Str(0, "Not Support Color Format\r\n");
        return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        break;
    }

    // Get Raw preview buffer config and size
    if (MMP_TRUE == m_VidRecdConfigs.bRawPreviewEnable[1]) {
    
        MMP_USHORT	usSensorW;
        MMP_USHORT	usSensorH;

        if (m_VidRecdConfigs.ulRawStoreBufCnt > VR_MAX_RAWSTORE_BUFFER_NUM) {
            m_VidRecdConfigs.ulRawStoreBufCnt = VR_MAX_RAWSTORE_BUFFER_NUM;
        }

        rawBuf.ulRawBufCnt = m_VidRecdConfigs.ulRawStoreBufCnt;
        rawEndBuf.ulRawBufCnt = rawBuf.ulRawBufCnt;

        MMPD_RAWPROC_GetStoreRange(ubVifId, &usSensorW, &usSensorH);

        if (m_VidRecdConfigs.ubRawPreviewBitMode[1] == MMP_RAW_COLORFMT_BAYER10) {
            ulRawBufSize = ALIGN32(usSensorW * usSensorH * 4 / 3);
        }
        else if (m_VidRecdConfigs.ubRawPreviewBitMode[1] == MMP_RAW_COLORFMT_YUV420) {
            ulRawBufSize = usSensorW * usSensorH; // Y plane only
        }
        else if (m_VidRecdConfigs.ubRawPreviewBitMode[1] == MMP_RAW_COLORFMT_YUV422) {
            if (0XBBBB1603 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                0XBBBB7150 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                0xBBB6124B == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                0xFFFF2643 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                0xBBBB1375 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                0xBBBB6750 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                0xBBBB2825 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                0XBBBB9992 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                0xFFFF1302 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR)) {

                ulRawBufSize = ALIGN32(usSensorW * usSensorH * 1);
            }
            else {
                ulRawBufSize = ALIGN32(usSensorW * usSensorH * 2);
            }
        }
        else {
            ulRawBufSize = ALIGN32(usSensorW * usSensorH);
        }
        
        #if 1 //20160920: Workaround for RAW Process data overwrites buffer (Reason: Unknown)
        ulRawBufSize += 256;
        #endif
    }
    else {
        rawBuf.ulRawBufCnt = 0;
        rawEndBuf.ulRawBufCnt = rawBuf.ulRawBufCnt;
        ulRawBufSize = 0;
    }

    if (MMPS_Sensor_IsADASEnable(SCD_SENSOR)||MMPS_Sensor_IsVMDEnable(SCD_SENSOR))
	{
		//MMPS_Sensor_GetVMDResolution(&gsADASFitRange.ulOutWidth, &gsADASFitRange.ulOutHeight);
		MdtcSrcBuf.usBufCnt = 2;
		//ulMdtcSrcBufSize    = ALIGN32(gsADASFitRange.ulOutWidth * gsADASFitRange.ulOutHeight);
	}
	else
	{
		MdtcSrcBuf.usBufCnt = 0;
	}
    //////////////////////////////////////////////////////////////////////////
    //
    //  Allocate memory for preview buffer
    //
    //////////////////////////////////////////////////////////////////////////

    ulCurAddr = ALIGN32(*ulStackAddr);
    
    // Allocate memory for preview buffer
    for (i = 0; i < previewbuf.usBufCnt; i++) {
        if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV420) {
            ulTmpBufSize = usPreviewW * usPreviewH;

            previewbuf.ulYBuf[i] = ulCurAddr;
            previewbuf.ulUBuf[i] = previewbuf.ulYBuf[i] + ALIGN32(ulTmpBufSize);
            previewbuf.ulVBuf[i] = previewbuf.ulUBuf[i] + ALIGN32(ulTmpBufSize >> 2);

            ulCurAddr += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>2)*2);
        }
        else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV420_INTERLEAVE) {
            ulTmpBufSize = usPreviewW * usPreviewH;
            
            previewbuf.ulYBuf[i] = ulCurAddr;
            previewbuf.ulUBuf[i] = previewbuf.ulYBuf[i] + ALIGN32(ulTmpBufSize);
            previewbuf.ulVBuf[i] = previewbuf.ulUBuf[i];

            ulCurAddr += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>1));
        }
        else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV422 ||
                 previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_RGB565) {
            ulTmpBufSize = ALIGN32(usPreviewW * 2 * usPreviewH);
            
            previewbuf.ulYBuf[i] = ulCurAddr;
            previewbuf.ulUBuf[i] = 0;
            previewbuf.ulVBuf[i] = 0;
            
            ulCurAddr += ulTmpBufSize;
        }
        else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_RGB888) {
            ulTmpBufSize = ALIGN32(usPreviewW * 3 * usPreviewH);
            
            previewbuf.ulYBuf[i] = ulCurAddr;
            previewbuf.ulUBuf[i] = 0;
            previewbuf.ulVBuf[i] = 0;

            ulCurAddr += ulTmpBufSize;
		}
		else {
		    RTNA_DBG_Str(0, "Not supported preview format\r\n");  
		    return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
		}
	}

    AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Preview");
    ulMemStart = ulCurAddr;

    // Allocate memory for rotate DMA destination buffer
	for (i = 0; i < previewbuf.usRotateBufCnt; i++) {
	
		if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV420) {
		    ulTmpBufSize = usPreviewW * usPreviewH;
		       
		    previewbuf.ulRotateYBuf[i] = ALIGN32(ulCurAddr);
            previewbuf.ulRotateUBuf[i] = previewbuf.ulRotateYBuf[i] + ALIGN32(ulTmpBufSize);
            previewbuf.ulRotateVBuf[i] = previewbuf.ulRotateUBuf[i] + ALIGN32(ulTmpBufSize >> 2);

            ulCurAddr += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>2)*2);
	    }
		else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV420_INTERLEAVE) {
		    ulTmpBufSize = usPreviewW * usPreviewH;

		    previewbuf.ulRotateYBuf[i] = ALIGN32(ulCurAddr);
            previewbuf.ulRotateUBuf[i] = previewbuf.ulRotateYBuf[i] + ALIGN32(ulTmpBufSize);
            previewbuf.ulRotateVBuf[i] = previewbuf.ulRotateUBuf[i];
 
            ulCurAddr += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>1));
	    }
		else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_YUV422 ||
				 previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_RGB565) {
		    ulTmpBufSize = ALIGN32(usPreviewW * 2 * usPreviewH);
		    
		    previewbuf.ulRotateYBuf[i] = ALIGN32(ulCurAddr);
            previewbuf.ulRotateUBuf[i] = 0;
            previewbuf.ulRotateVBuf[i] = 0;

            ulCurAddr += ulTmpBufSize;
		}
        else if (previewdata->DispColorFmt[usModeIdx] == MMP_DISPLAY_COLOR_RGB888) {
		    ulTmpBufSize = ALIGN32(usPreviewW * 3 * usPreviewH);
		    
		    previewbuf.ulRotateYBuf[i] = ALIGN32(ulCurAddr);
            previewbuf.ulRotateUBuf[i] = 0;
            previewbuf.ulRotateVBuf[i] = 0;

            ulCurAddr += ulTmpBufSize;
		}
		else {
		    RTNA_DBG_Str(0, "Not supported preview format\r\n");  
		    return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
		}
	}

    AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Rotate Preview");
    ulMemStart = ulCurAddr;

    // Allocate memory for raw preview buffer
    if (m_VidRecdConfigs.bRawPreviewEnable[1]) {

        for (i = 0; i < rawBuf.ulRawBufCnt; i++) {
            rawBuf.ulRawBufAddr[i] = ALIGN32(ulCurAddr);
            ulCurAddr = rawBuf.ulRawBufAddr[i] + ulRawBufSize;
            rawEndBuf.ulRawBufAddr[i] = ulCurAddr;
            
            if (m_VidRecdConfigs.ubRawPreviewBitMode[1] == MMP_RAW_COLORFMT_YUV420) {
                // For I420
                rawBuf.ulRawBufUAddr[i] = (ulCurAddr);
                ulCurAddr = rawBuf.ulRawBufUAddr[i] + (ulRawBufSize >> 2);
                rawEndBuf.ulRawBufUAddr[i] = ulCurAddr;
                rawBuf.ulRawBufVAddr[i] = (ulCurAddr);
                ulCurAddr = rawBuf.ulRawBufVAddr[i] + (ulRawBufSize >> 2);
                rawEndBuf.ulRawBufVAddr[i] = ulCurAddr;
            }
        }

        MMPD_RAWPROC_SetStoreBuf(ubVifId, &rawBuf);
        
        // Set Ring Buffer End Address 
        MMPD_RAWPROC_SetStoreBufEnd(ubVifId, &rawEndBuf);
        
        MMPD_RAWPROC_EnableRingStore(ubVifId, MMP_RAW_STORE_PLANE0, MMP_TRUE);
        
        if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) {
        
            if (MMP_GetTvDecSnrAttr()->bUseDMADeinterlace) { // For YUV422, YUV420 is TBD
            
                m_sDeinterlaceBuf[ubVifId].ulRawBufCnt = DMA_DEINTERLACE_BUF_CNT;
                
                for (i = 0; i < DMA_DEINTERLACE_BUF_CNT; i++) {
                    MMPD_RAWPROC_SetDeInterlaceBuf(ubVifId, i, ulCurAddr, 0, 0, DMA_DEINTERLACE_BUF_CNT);
                    m_sDeinterlaceBuf[ubVifId].ulRawBufAddr[i]  = ulCurAddr;
                    m_sDeinterlaceBuf[ubVifId].ulRawBufUAddr[i] = 0;
                    m_sDeinterlaceBuf[ubVifId].ulRawBufVAddr[i] = 0;
                    ulCurAddr += ALIGN32(ulRawBufSize * DMA_DEINTERLACE_DOUBLE_FIELDS); 
                }
                MMPD_RAWPROC_ResetDeInterlaceBufIdx(ubVifId);
            }
            
            MMPD_RAWPROC_SetStoreOnly(ubVifId, MMP_TRUE);
            
            MMPD_RAWPROC_EnablePreview(ubVifId, MMP_TRUE);

            MMPD_RAWPROC_EnablePath(ubSnrSel,
                                    ubVifId,
                                    MMP_TRUE,
                                    MMP_RAW_IOPATH_VIF2RAW,
                                    m_VidRecdConfigs.ubRawPreviewBitMode[1]);
        }
        else if (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
        
            MMPD_RAWPROC_SetStoreOnly(ubVifId, MMP_TRUE);
            
            MMPD_RAWPROC_EnablePreview(ubVifId, MMP_TRUE);

            MMPD_RAWPROC_EnablePath(ubSnrSel,
                                    ubVifId,
                                    MMP_TRUE,
                                    MMP_RAW_IOPATH_VIF2RAW,
                                    m_VidRecdConfigs.ubRawPreviewBitMode[1]);
        }
        else if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
        
            MMPD_RAWPROC_SetStoreOnly(ubVifId, MMP_FALSE);
            
            MMPD_RAWPROC_EnablePreview(ubVifId, MMP_TRUE);

            MMPD_RAWPROC_EnablePath(ubSnrSel,
                                    ubVifId,
                                    MMP_TRUE,
                                    MMP_RAW_IOPATH_VIF2RAW | MMP_RAW_IOPATH_RAW2ISP,
                                    m_VidRecdConfigs.ubRawPreviewBitMode[1]);
        }
        
        m_sRawBuf[ubVifId]    = rawBuf;
        m_sRawEndBuf[ubVifId] = rawEndBuf;
    }
    else {
        MMPD_RAWPROC_EnablePreview(ubVifId, MMP_FALSE);
    }

    if (MMPS_Sensor_IsADASEnable(SCD_SENSOR)||MMPS_Sensor_IsVMDEnable(SCD_SENSOR))
    {
		#if (SUPPORT_MDTC)

		ulCurAddr = ALIGN32(ulCurAddr);

		ulTmpBufSize = m_ulVRPreviewW[SCD_SENSOR] * m_ulVRPreviewH[SCD_SENSOR];
		printc("2nd ulTmpBufSize 0x%x  W %d   *  H %d\r\n", ulTmpBufSize, m_ulVRPreviewW[SCD_SENSOR], m_ulVRPreviewH[SCD_SENSOR]);
		{
			if (MMPS_Sensor_IsVMDEnable(SCD_SENSOR))
			{
				if (MMPS_Sensor_AllocateVMDBuffer(SCD_SENSOR, &ulCurAddr, MMP_TRUE) != MMP_ERR_NONE) {
					PRINTF("Allocate MDTC buffer failed\r\n");
					return MMP_SENSOR_ERR_VMD;
				}
			}
		}

		for (i = 0; i < MdtcSrcBuf.usBufCnt; i++) {
			MdtcSrcBuf.ulYBuf[i] = ulCurAddr;

			ulCurAddr += ALIGN32(ulTmpBufSize);
		}
    	#endif
    }

    AUTL_MemDbg_PushItem(&sVRPrevwMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Raw Preview");
    ulMemStart = ulCurAddr;

    *ulStackAddr = ulCurAddr;
    m_ulVidRecDramEndAddr = m_ulVideoPreviewEndAddr = ulCurAddr;
    
    #if defined(ALL_FW)
    if (m_ulVidRecDramEndAddr > MMPS_System_GetMemHeapEnd()) {
        RTNA_DBG_Str(0, "\t= [HeapMemErr] 2nd Video preview =\r\n");
        return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
    }
    printc("End of 2nd video preview buffers = 0x%X\r\n", m_ulVidRecDramEndAddr);
    #endif

    MMPS_3GPRECD_Set2ndSnrPreviewPipeConfig(&previewbuf, usPreviewW, usPreviewH, ubSnrSel);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_2ndSnrPreviewStop
//  Description : Stop preview display mode.
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_2ndSnrPreviewStop(MMP_UBYTE ubSnrSel)
{
    if (m_VidRecdConfigs.previewpath[1] == MMP_3GP_PATH_INVALID) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    MMPS_3GPRECD_Enable2ndSnrPreviewPipe(ubSnrSel, MMP_FALSE, MMP_FALSE);
   	
    /* This is a work around. After real-time H264 encode,
     * we have to reset IBC to avoid a corrupted still JPEG later.
     * Here we reset all of IBC pipes, instead of H.264 pipe only.
     */
    MMPD_IBC_ResetModule(m_2ndPrewFctlLink.ibcpipeID);
    MMPD_IBC_ResetModule(m_2ndRecFctlLink.ibcpipeID);

    #if (SUPPORT_COMPONENT_FLOW_CTL)
    MMP_CompCtl_UnLinkComponentList(m_ubCamPreviewListId[SCD_SENSOR], MMP_TRUE);
    #endif

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_2ndSnrPreviewStart
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set buffers and parameters. Then display preview mode.
 @param[in] bCheckFrameEnd 	The setting will check VIF frame end or not.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS Unsupport resolution.
*/
MMP_ERR MMPS_3GPRECD_2ndSnrPreviewStart(MMP_UBYTE ubSnrSel, MMP_BOOL bCheckFrameEnd)
{
    MMP_ULONG ulStackAddr = 0;
    
    if (m_VidRecdConfigs.previewpath[1] == MMP_3GP_PATH_INVALID) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }
    
    if (m_ulVidRecDramEndAddr == 0 || m_ulVideoPreviewEndAddr == 0) {
        MMPS_Sensor_GetMemEnd(&ulStackAddr);
        m_ulVidRecDramEndAddr = m_ulVideoPreviewEndAddr = ulStackAddr;
    }
    
    if (m_ulVidRecSramAddr == 0) {
        MMPD_System_GetSramEndAddr(&m_ulVidRecSramAddr);
    }

    MMPS_3GPRECD_AdjustPreviewRes(ubSnrSel);
    
    if (MMP_IsScdCamExist())
        m_VidRecdConfigs.bRawPreviewEnable[1] = MMP_TRUE;
    else
        m_VidRecdConfigs.bRawPreviewEnable[1] = MMP_FALSE;
    
    /* Use first sensor preivew buffer end as the buffer start */
    ulStackAddr = m_ulVidRecDramEndAddr = m_ulVideoPreviewEndAddr;

    if (MMPS_3GPRECD_Set2ndSnrPreviewMemory(ubSnrSel,
                                            m_ulVRPreviewW[ubSnrSel],
                                            m_ulVRPreviewH[ubSnrSel],
                                            &ulStackAddr))
    {
        RTNA_DBG_Str(0, "Alloc mem for 2nd preview failed\r\n");
        return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
    }
    
    MMPD_Display_ResetRotateDMABufIdx(MMP_DISPLAY_SRC_REARCAM);

    #if (SUPPORT_COMPONENT_FLOW_CTL)
    if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER) ||
        CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
        
        if (m_ubCamPreviewListId[SCD_SENSOR] == 0xFF) {
            
            MMP_USHORT              usModeIdx   = m_VidRecdModes.usVideoPreviewMode;
            MMP_UBYTE               ubVifId     = MMPF_Sensor_GetVIFPad(m_VRPreviewFctlAttr[1].ubPipeLinkedSnr);
            MMP_UBYTE               ubRawId     = ubVifId;
            MMP_UBYTE               ubPipeId    = m_VRPreviewFctlAttr[1].fctllink.ibcpipeID;
            MMP_UBYTE               ubCompIdArray[10];
            
            MMP_GRAPHICS_BUF_ATTR   srcBuf  = {0, };
            MMP_GRAPHICS_RECT       srcRect = {0, };
            MMP_UBYTE               ubPixDelayN = 20;
            MMP_UBYTE               ubPixDelayM = 20;
            MMP_USHORT              usLineDelay = 0;
            MMP_USHORT              usRawSWidth, usRawSHeight;

            ubCompIdArray[0] = MMP_COMPONENT_ID_NULL;
            ubCompIdArray[1] = TRANS_RAWS_TO_RAWSCOMP_ID(ubRawId);
            ubCompIdArray[2] = TRANS_RAWS_TO_DRAMCOMP_ID(ubRawId);
            ubCompIdArray[3] = MMP_COMPONENT_ID_DMA;
            ubCompIdArray[4] = TRANS_RAWS_TO_DMA_DRAMCOMP_ID(ubRawId);
            ubCompIdArray[5] = MMP_COMPONENT_ID_GRA;
            ubCompIdArray[6] = TRANS_PIPE_TO_PIPECOMP_ID(ubPipeId);
            ubCompIdArray[7] = TRANS_PIPE_TO_DRAMCOMP_ID(ubPipeId);
            ubCompIdArray[8] = MMP_COMPONENT_ID_DISP;
            ubCompIdArray[9] = MMP_COMPONENT_ID_NULL;
            
            MMP_CompCtl_RegisterRawStoreComponent(MMP_COMPONENT_USAGE_ID1,
                                                  ubRawId,
                                                  MMP_RAW_COLORFMT_YUV422);
            MMP_CompCtl_RegisterRawStoreDramComponent(MMP_COMPONENT_USAGE_ID1,
                                                      ubRawId,
                                                      m_sRawBuf[ubRawId].ulRawBufCnt,
                                                      &m_sRawBuf[ubRawId].ulRawBufAddr[0],
                                                      &m_sRawEndBuf[ubRawId].ulRawBufAddr[0]);

            if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER) &&
                MMP_GetTvDecSnrAttr()->bUseDMADeinterlace) {
                MMP_CompCtl_RegisterDMAComponent(MMP_COMPONENT_USAGE_ID1, 0, 0, 0); // TBD
                MMP_CompCtl_RegisterRawStoreDMADramComponent(MMP_COMPONENT_USAGE_ID1, 
                                                             ubRawId, 
                                                             m_sDeinterlaceBuf[ubRawId].ulRawBufCnt, 
                                                             &m_sDeinterlaceBuf[ubRawId].ulRawBufAddr[0]);
            }
            
            /* Get Graphics Loopback Attribute */
            MMPF_RAWPROC_GetStoreRange(ubRawId, &usRawSWidth, &usRawSHeight);

            if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER) && 
                MMP_GetTvDecSnrAttr()->bUseDMADeinterlace) {

                srcBuf.usWidth          = usRawSWidth;
                srcBuf.usHeight         = usRawSHeight * 2;
                srcBuf.usLineOffset     = srcBuf.usWidth * 2;
                srcBuf.colordepth       = MMP_GRAPHICS_COLORDEPTH_YUV422_UYVY;
                
                if (0XBBBB1603 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                    0XBBBB7150 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                    0XBBBB9992 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR)) {
                    srcBuf.usLineOffset = srcBuf.usWidth;
                }
                srcBuf.ulBaseAddr       = 0;
                srcBuf.ulBaseUAddr      = 0;
                srcBuf.ulBaseVAddr      = 0;
            }
            else {

                srcBuf.usWidth          = usRawSWidth;
                srcBuf.usHeight         = usRawSHeight;
                srcBuf.usLineOffset	    = srcBuf.usWidth * 2;
                srcBuf.colordepth       = MMP_GRAPHICS_COLORDEPTH_YUV422_UYVY;

                if (0XBBBB1603 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                    0XBBBB7150 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
                    0XBBBB9992 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR)) {
                    srcBuf.usLineOffset = srcBuf.usWidth;
                    srcBuf.colordepth   = MMP_GRAPHICS_COLORDEPTH_YUV422_UYVY;
                }
                else if (0xFFFF2643 == gsSensorFunction->MMPF_Sensor_GetSnrID(PRM_SENSOR) ||
                    0xFFFF2643 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR)) {
                    srcBuf.usLineOffset = srcBuf.usWidth;
                    srcBuf.colordepth   = MMP_GRAPHICS_COLORDEPTH_YUV422_YVYU;
                }
                else if (0xBBB6124B == gsSensorFunction->MMPF_Sensor_GetSnrID(PRM_SENSOR) ||
                    0xBBB6124B == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR)) {
                    srcBuf.usWidth      = usRawSWidth / 2; 
                    srcBuf.usLineOffset = srcBuf.usWidth * 2;
                    srcBuf.colordepth   = MMP_GRAPHICS_COLORDEPTH_YUV422_UYVY;
                }
				else if(0xBBBB6750 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR) ||
					0xBBBB2825 == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR)){
					srcBuf.usWidth      = usRawSWidth / 2; 
                    srcBuf.usLineOffset = srcBuf.usWidth * 2;
                    srcBuf.colordepth   = MMP_GRAPHICS_COLORDEPTH_YUV422_UYVY;
				}
                
                srcBuf.ulBaseAddr       = 0;
                srcBuf.ulBaseUAddr      = 0;
                srcBuf.ulBaseVAddr      = 0;

                if (0xBBB6124B == gsSensorFunction->MMPF_Sensor_GetSnrID(SCD_SENSOR)) {
                    srcBuf.ulBaseAddr  +=16; // Workaround to avoid garbage vertical lines in the left of image
                }
            }

            srcRect.usLeft      = 0;
            srcRect.usTop       = 0;
            srcRect.usWidth     = srcBuf.usWidth;
            srcRect.usHeight    = srcBuf.usHeight;

            // Slow down Graphic clock and delay for reduce Icon usage.
            if (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
                ubPixDelayN = 20;
                ubPixDelayM = 20;
                usLineDelay = 0x20;
            }
            else {
                ubPixDelayN = 8;
                ubPixDelayM = 20;
                usLineDelay = 0x20;
            }
            
            MMP_CompCtl_RegisterGraComponent(MMP_COMPONENT_USAGE_ID1, srcBuf, srcRect, ubPixDelayN, ubPixDelayM, usLineDelay);
            
            MMP_CompCtl_RegisterPipeComponent(MMP_COMPONENT_USAGE_ID1, (void*)&m_VRPreviewFctlAttr[1]);
            MMP_CompCtl_RegisterPipeOutDramComponent(MMP_COMPONENT_USAGE_ID1, (void*)&m_VRPreviewFctlAttr[1]);
            MMP_CompCtl_RegisterDisplayComponent(MMP_COMPONENT_USAGE_ID1,
                                                 GET_VR_PREVIEW_WINDOW(ubSnrSel), 
                                                 m_VidRecdConfigs.previewdata[1].DispDevice[usModeIdx]);
            
            m_ubCamPreviewListId[SCD_SENSOR] = MMP_CompCtl_LinkComponentsEx(MMP_COMPONENT_USAGE_ID1, &ubCompIdArray[0], 10, (void*)_TrigEmptyRawStoreBuffer);
            
            MMP_CompCtl_StartCheckListTimer(3); // TBD
        }
    }
    #endif

    if (MMPS_3GPRECD_Enable2ndSnrPreviewPipe(ubSnrSel, MMP_TRUE, bCheckFrameEnd) != MMP_ERR_NONE) {
        RTNA_DBG_Str(0, "Enable Video Preview: Fail\r\n");
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }
    
    return MMP_ERR_NONE;
}

#if 0
void ____VR_Preview_Misc_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetFrontCamBufForDualStreaming
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetFrontCamBufForDualStreaming(MMP_ULONG *pulCompAddr, MMP_ULONG *pulCompSize,
													MMP_ULONG *pulLineBuf,	MMP_ULONG *pulLineSize)
{
#if (HANDLE_JPEG_EVENT_BY_QUEUE) && (SUPPORT_MJPEG_WIFI_STREAM)
    *pulCompAddr = m_ulFrontCamRsvdCompBufStart;
    *pulCompSize = m_ulFrontCamRsvdCompBufSize;
    *pulLineBuf  = m_ulFrontCamEncLineBufAddr;
    *pulLineSize = m_ulFrontCamEncLineBufSize;
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetDualBayerSnrCaptureMode
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetDualBayerSnrCaptureMode(MMP_BOOL bDSCMode)
{
    gbDualBayerSnrInDSCMode = bDSCMode;

    return MMP_ERR_NONE;
}

#if 0
void ____VR_DecMJPEGToPreview_Record_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetUVCFBMemory
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetUVCFBMemory(void)
{
#if (SUPPORT_USB_HOST_FUNC)    
    MMP_ERR     sRet = MMP_ERR_NONE;
    MMP_ULONG   ulPreviewUVCMaxBufSize = 0, ulCurAddr = 0;
    MMP_UBYTE   ubUVCFBMemSet = 0;
    MMP_UBYTE   ubEncIdx = 0;
    MMP_BOOL    bEnable = MMP_FALSE;
    MMP_ULONG   ulStackAddr = 0;
    
    // Check the record pipe status
    for (ubEncIdx = 0; ubEncIdx < VR_MAX_ENCODE_NUM; ++ubEncIdx) {      
        sRet = MMPS_3GPRECD_GetRecordPipeStatus(ubEncIdx, &bEnable);   
        if (bEnable != MMP_FALSE) {
            MMP_PRINT_RET_ERROR(0, sRet, "Can not set UVC FB memory when recording!", gubMmpDbgBk);
            return MMP_SYSTEM_ERR_NOT_SUPPORT;
        }
    }
    
    sRet = MMPF_USBH_IsSetFBMemory(&ubUVCFBMemSet);
    if (1 == ubUVCFBMemSet) {
        RTNA_DBG_Str(0, FG_GREEN("MMPS_3GPRECD_SetUVCFBMemory has been set before!\r\n"));
        return sRet;
    }
    
    if (m_ulVidRecDramEndAddr == 0 || m_ulVideoPreviewEndAddr == 0) {
        
        if (MMP_IsPrmCamExist() || MMP_IsScdCamExist())
            MMPS_Sensor_GetMemEnd(&ulStackAddr);
        else
            MMPD_System_GetFWEndAddr(&ulStackAddr);
        
        m_ulVidRecDramEndAddr = m_ulVideoPreviewEndAddr = ulStackAddr;
    }
    
    if (m_ulVidRecSramAddr == 0) {
        MMPD_System_GetSramEndAddr(&m_ulVidRecSramAddr);
    }
    
    // Allocate buffer for EP data, USBH preview/record    
    ulCurAddr = (m_ulVidRecDramEndAddr >= m_ulVideoPreviewEndAddr) ? m_ulVidRecDramEndAddr : m_ulVideoPreviewEndAddr;  
    ulCurAddr = ALIGN32(ulCurAddr);

    sRet = MMPD_UVCRECD_AllocFBMemory(ulCurAddr, &ulPreviewUVCMaxBufSize);
    ulCurAddr += ulPreviewUVCMaxBufSize;   

    if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "SetUVCFBMemory error!",gubMmpDbgBk); return sRet;}

    m_ulVidRecDramEndAddr = m_ulVideoPreviewEndAddr = ulCurAddr;

    #if defined(ALL_FW)
    if (m_ulVidRecDramEndAddr > MMPS_System_GetMemHeapEnd()) {
        RTNA_DBG_Str(0, "\t= [HeapMemErr] UVC rear FB Memory =\r\n");
        return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
    }
    printc("End of UVC rear FB Memory = 0x%X\r\n", m_ulVidRecDramEndAddr);
    #endif
    
    return sRet;
#else
    MMP_PRINT_RET_ERROR(0, 0, "SetUVCFBMemory not support!", gubMmpDbgBk);
    return MMP_ERR_NONE;
#endif
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetUVCRearMJPGMemory
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetUVCRearMJPGMemory(void)
{
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    MMP_ERR     sRet = MMP_ERR_NONE;
    MMP_ULONG   ulDecPreviewBufSize = 0, ulCurAddr;
    MMP_UBYTE   ubMjpegToPreviewJpegSet = 0;
    MMP_UBYTE   ubEncIdx = 0;
    MMP_BOOL    bEnable = MMP_FALSE;
    
    if (!MMP_IsSupportDecMjpegToPreview()) {
        return MMP_ERR_NONE;
    }
    
    // Check record pipe status
    for (ubEncIdx = 0; ubEncIdx < VR_MAX_ENCODE_NUM; ++ubEncIdx) {      
        sRet = MMPS_3GPRECD_GetRecordPipeStatus(ubEncIdx, &bEnable);   
        if (bEnable != MMP_FALSE) {
            MMP_PRINT_RET_ERROR(0, sRet, "Can not set UVC Rear MJPG Memory when recording!", gubMmpDbgBk);
            return MMP_SYSTEM_ERR_NOT_SUPPORT;
        }
    }
    
    sRet = MMPF_USBH_IsSetDecMjpegToPreviewJpegBuf(&ubMjpegToPreviewJpegSet);
    if (1 == ubMjpegToPreviewJpegSet) {
        MMPF_USBH_InitDecMjpegToPreviewJpegBuf();
        RTNA_DBG_Str(0, FG_GREEN("MMPS_3GPRECD_SetUVCRearMJPGMemory has been set before!\r\n"));
        return sRet;
    }

    if ((0 == m_sAhcDecMjpegToPreviewInfo.ulPreviewBufW) || 
         0 == m_sAhcDecMjpegToPreviewInfo.ulPreviewBufH) {
        MMP_PRINT_RET_ERROR(0, 0, "Parameter error!", gubMmpDbgBk);
        sRet = MMP_SYSTEM_ERR_PARAMETER;
        return sRet;
    }
    
    ulCurAddr = (m_ulVidRecDramEndAddr >= m_ulVideoPreviewEndAddr) ? m_ulVidRecDramEndAddr : m_ulVideoPreviewEndAddr;  
    ulCurAddr = ALIGN32(ulCurAddr);

    sRet = MMPS_3GPRECD_SetDecMjpegToPreviewBuf(m_sAhcDecMjpegToPreviewInfo.ulPreviewBufW,
                                                m_sAhcDecMjpegToPreviewInfo.ulPreviewBufH,
                                                ulCurAddr,
                                                &ulDecPreviewBufSize,
                                                &m_sDecMjpegPrevwBufInfo);
    
    ulCurAddr += ulDecPreviewBufSize;

    sRet = MMPD_DSC_SetDecMjpegToPreviewJpegBuf(ulCurAddr, &ulDecPreviewBufSize);
    ulCurAddr += ulDecPreviewBufSize;

    #if (AIT_REAR_CAM_VR_CAPTURE_EN)
    if (CAM_CHECK_USB(USB_CAM_AIT) && MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_MJPEG_H264)
    {
        MMP_USHORT usW = 0, usH = 0;

        sRet = MMPS_3GPRECD_GetDecMjpegToPreviewSrcAttr(&usW, &usH);
        MMPF_JPEGCTL_Buf2StillJpegSetRawAddress(ulCurAddr, ulCurAddr + (usW * usH), ulCurAddr + (usW * usH) + ((usW * usH) >> 2));
        ulCurAddr += (usW * usH * 2); 
    }
    #endif

    if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "SetMJPG Memory error!",gubMmpDbgBk); return sRet;}

    m_ulVidRecDramEndAddr = m_ulVideoPreviewEndAddr = ulCurAddr;

    #if defined(ALL_FW)
    if (m_ulVidRecDramEndAddr > MMPS_System_GetMemHeapEnd()) {
        RTNA_DBG_Str(0, "\t= [HeapMemErr] UVC rear MJPG Video preview =\r\n");
        return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
    }
    printc("End of UVC rear MJPG video preview buffers = 0x%X\r\n", m_ulVidRecDramEndAddr);
    #endif
    
    return sRet;
#else
    MMP_PRINT_RET_ERROR(0, 0, "SetMJPG Memory not support!",gubMmpDbgBk);
    return MMP_ERR_NONE;
#endif
}

#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetDispColorFmtToJpgAttr
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetDispColorFmtToJpgAttr(MMP_UBYTE ubColorformat)
{
    extern MMP_UBYTE gubIBCColorFormat; // Decode JPEG to IBC color format
    extern MMP_UBYTE MMPF_IBC_MapDispColorToIBCFmt(MMP_UBYTE ubDispColor);
    
    if (!MMP_IsSupportDecMjpegToPreview()) {
        return MMP_ERR_NONE;
    }
    
    gubIBCColorFormat = MMPF_IBC_MapDispColorToIBCFmt(ubColorformat);
    
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetDispColorFmtToJpgAttr
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetDispColorFmtToJpgAttr(MMP_UBYTE *pColorformat)
{
    extern MMP_UBYTE gubIBCColorFormat;
    *pColorformat = gubIBCColorFormat;
    return 0;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetDecMjpegToPreviewPipeId
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE MMPS_3GPRECD_GetDecMjpegToPreviewPipeId(void)
{
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    return m_DecMjpegToPrevwFctlLink.ibcpipeID;
#else
    return 0;
#endif
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetDecMjpegToPreviewSrcAttr
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetDecMjpegToPreviewSrcAttr(MMP_USHORT *pusW, MMP_USHORT *pusH)
{
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    *pusW = m_usDecMjpegToPreviewSrcW;
    *pusH = m_usDecMjpegToPreviewSrcH;
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetDecMjpegToPreviewSrcAttr
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetDecMjpegToPreviewSrcAttr(MMP_USHORT usSrcW, MMP_USHORT usSrcH)
{
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    m_usDecMjpegToPreviewSrcW = usSrcW;
    m_usDecMjpegToPreviewSrcH = usSrcH;
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetDecMjpegToPreviewDispAttr
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetDecMjpegToPreviewDispAttr(  MMP_USHORT *pusDispWinId,
                                                    MMP_USHORT *pusWinOfstX,  MMP_USHORT *pusWinOfstY,
                                                    MMP_USHORT *pusWinWidth,  MMP_USHORT *pusWinHeight,
                                                    MMP_USHORT *pusDispWidth, MMP_USHORT *pusDispHeight,                                              
                                                    MMP_USHORT *pusDispColor)
{
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    *pusDispWinId   = GET_VR_PREVIEW_WINDOW(USBH_SENSOR);
    *pusWinOfstX    = m_sAhcDecMjpegToPreviewInfo.ulDispStartX;
    *pusWinOfstY    = m_sAhcDecMjpegToPreviewInfo.ulDispStartY;
    *pusWinWidth    = m_sAhcDecMjpegToPreviewInfo.ulPreviewBufW;
    *pusWinHeight   = m_sAhcDecMjpegToPreviewInfo.ulPreviewBufH;
    *pusDispWidth   = m_sAhcDecMjpegToPreviewInfo.ulDispWidth;
    *pusDispHeight  = m_sAhcDecMjpegToPreviewInfo.ulDispHeight;
    *pusDispColor   = m_sAhcDecMjpegToPreviewInfo.sDispColor;
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetDecMjpegToPreviewDispAttr
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetDecMjpegToPreviewDispAttr(  MMP_USHORT usDispWinId,
                                                    MMP_BOOL   bRotate,
                                                    MMP_UBYTE  ubRotateDir,
                                                    MMP_UBYTE  sFitMode,
                                                    MMP_USHORT usWinOfstX,  MMP_USHORT usWinOfstY,
                                                    MMP_USHORT usWinWidth,  MMP_USHORT usWinHeight,
                                                    MMP_USHORT usDispWidth, MMP_USHORT usDispHeight,
                                                    MMP_USHORT usDispColor)
{
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    m_sAhcDecMjpegToPreviewInfo.bPreviewRotate  = bRotate;
    m_sAhcDecMjpegToPreviewInfo.sPreviewDmaDir  = ubRotateDir;
    m_sAhcDecMjpegToPreviewInfo.sFitMode        = sFitMode;
    m_sAhcDecMjpegToPreviewInfo.ulPreviewBufW   = usWinWidth;
    m_sAhcDecMjpegToPreviewInfo.ulPreviewBufH   = usWinHeight;
    m_sAhcDecMjpegToPreviewInfo.ulDispStartX   	= usWinOfstX;
    m_sAhcDecMjpegToPreviewInfo.ulDispStartY   	= usWinOfstY;
    m_sAhcDecMjpegToPreviewInfo.ulDispWidth    	= usDispWidth;
    m_sAhcDecMjpegToPreviewInfo.ulDispHeight    = usDispHeight;
    m_sAhcDecMjpegToPreviewInfo.sDispColor      = (MMP_DISPLAY_COLORMODE)usDispColor;

#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetDecMjpegToPreviewBuf
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetDecMjpegToPreviewBuf(   MMP_USHORT                      usPreviewW,  
                                                MMP_USHORT                      usPreviewH, 
                                                MMP_ULONG                       ulAddr, 
                                                MMP_ULONG                       *pulSize,
                                                MMPS_3GPRECD_PREVIEW_BUFINFO    *pPreviewBuf)
{
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    MMP_ULONG           cur_buf = ulAddr;
    MMP_USHORT          i = 0;
    MMP_ULONG           ulTmpBufSize = 0;
    MMP_ULONG           ulYSize;
    MMP_BOOL            bIsPrevwResolSet = MMP_FALSE;
    MMP_DISPLAY_WIN_ID  ePreviewWinID = GET_VR_PREVIEW_WINDOW(USBH_SENSOR);
    
    #define YUV420_Y_BLACK_COLOR    (0x00)
    #define YUV420_UV_BLACK_COLOR   (0x80)

    if (!MMP_IsSupportDecMjpegToPreview()) {
        return MMP_ERR_NONE;
    }

    #if (HANDLE_JPEG_EVENT_BY_QUEUE) && (SUPPORT_MJPEG_WIFI_STREAM)
    if (MMPF_JPEG_GetCtrlByQueueEnable()) {
        /* Reserved for Max MJPEG resolution */
        usPreviewW  = (usPreviewW > m_usMJPEGMaxEncWidth) ? (usPreviewW) : (m_usMJPEGMaxEncWidth);
        usPreviewH  = (usPreviewH > m_usMJPEGMaxEncHeight) ? (usPreviewH) : (m_usMJPEGMaxEncHeight);
    }
    #endif

    /* Allocate preview buffer (window buffer) */
    pPreviewBuf->usBufCnt = 2;
    
    for (i = 0; i < pPreviewBuf->usBufCnt; i++)
    {
        switch (m_sAhcDecMjpegToPreviewInfo.sDispColor) {
        case MMP_DISPLAY_COLOR_RGB565:
        case MMP_DISPLAY_COLOR_YUV422:
            ulTmpBufSize = ALIGN32(usPreviewW * 2 * usPreviewH);
            
            pPreviewBuf->ulYBuf[i] = cur_buf;
            pPreviewBuf->ulUBuf[i] = 0;
            pPreviewBuf->ulVBuf[i] = 0;
            cur_buf += ulTmpBufSize;
            break;
        case MMP_DISPLAY_COLOR_RGB888:
            ulTmpBufSize = ALIGN32(usPreviewW * 3 * usPreviewH);
            
            pPreviewBuf->ulYBuf[i] = cur_buf;
            pPreviewBuf->ulUBuf[i] = 0;
            pPreviewBuf->ulVBuf[i] = 0;
            cur_buf += ulTmpBufSize;
            break;
        case MMP_DISPLAY_COLOR_YUV420:
            ulTmpBufSize = usPreviewW * usPreviewH;
            
            pPreviewBuf->ulYBuf[i] = cur_buf;
            pPreviewBuf->ulUBuf[i] = pPreviewBuf->ulYBuf[i] + ALIGN32(ulTmpBufSize);
            pPreviewBuf->ulVBuf[i] = pPreviewBuf->ulUBuf[i] + ALIGN32(ulTmpBufSize >> 2);
            cur_buf += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>2)*2);
            break;
        case MMP_DISPLAY_COLOR_YUV420_INTERLEAVE:
            ulTmpBufSize = usPreviewW * usPreviewH;
        
            pPreviewBuf->ulYBuf[i] = cur_buf;
            pPreviewBuf->ulUBuf[i] = pPreviewBuf->ulYBuf[i] + ALIGN32(ulTmpBufSize);
            pPreviewBuf->ulVBuf[i] = pPreviewBuf->ulUBuf[i];
            cur_buf += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>1));
            break;
        }
    }
    
    /* Preset/Clear window buffer */
    ulYSize = usPreviewW * usPreviewH;
    
    MMPD_UVCRECD_IsPrevwResolSet(&bIsPrevwResolSet);

    if (bIsPrevwResolSet == MMP_FALSE)
    {
        DBG_S(0, "[ERR]: PREVW (W, H) not set yet, incorrect seq.! \r\n");
    }
    else
    {
        /* Set buffer to black color */
        if (m_sAhcDecMjpegToPreviewInfo.sDispColor == MMP_DISPLAY_COLOR_YUV420_INTERLEAVE) {
            MEMSET((void*)(pPreviewBuf->ulYBuf[0]), YUV420_Y_BLACK_COLOR, ulYSize);
            MEMSET((void*)(pPreviewBuf->ulUBuf[0]), YUV420_UV_BLACK_COLOR, (ulYSize>>1));
            MEMSET((void*)(pPreviewBuf->ulYBuf[1]), YUV420_Y_BLACK_COLOR, ulYSize);
            MEMSET((void*)(pPreviewBuf->ulUBuf[1]), YUV420_UV_BLACK_COLOR, (ulYSize>>1));
        }
        else {
            // TBD
            DBG_S(0, "[ERR]: UNSUPPORTED COLOR!");
            DBG_B(0,m_sAhcDecMjpegToPreviewInfo.sDispColor);
            DBG_S(0, " #");  DBG_L(0,__LINE__);  DBG_S(0, "\r\n"); 
        }
        
        /* First time: Update Window Address */
        /* Fix refresh hang, set UVC preview address once buffer allocated */
        if (!m_sAhcDecMjpegToPreviewInfo.bPreviewRotate) {
            MMPD_Display_UpdateWinAddr( ePreviewWinID,
                                        pPreviewBuf->ulYBuf[1],
                                        pPreviewBuf->ulUBuf[1],
                                        pPreviewBuf->ulVBuf[1]);
        }
    }
    
    /* Allocate rotate buffer */
    if (m_sAhcDecMjpegToPreviewInfo.bPreviewRotate)
    {
        pPreviewBuf->usRotateBufCnt = 2;
        
        for (i = 0; i < pPreviewBuf->usRotateBufCnt; i++) 
        {
            switch (m_sAhcDecMjpegToPreviewInfo.sDispColor) {
            case MMP_DISPLAY_COLOR_RGB565:
            case MMP_DISPLAY_COLOR_YUV422:
                ulTmpBufSize = ALIGN32(usPreviewW * 2 * usPreviewH);

                pPreviewBuf->ulRotateYBuf[i] = cur_buf;
                pPreviewBuf->ulRotateUBuf[i] = 0;
                pPreviewBuf->ulRotateVBuf[i] = 0;
                cur_buf += ulTmpBufSize;
                break;
            case MMP_DISPLAY_COLOR_RGB888:
                ulTmpBufSize = ALIGN32(usPreviewW * 3 * usPreviewH);

    	    	pPreviewBuf->ulRotateYBuf[i] = cur_buf;
    	    	pPreviewBuf->ulRotateUBuf[i] = 0;
    	    	pPreviewBuf->ulRotateVBuf[i] = 0;
            	cur_buf += ulTmpBufSize;
    			break;
    		case MMP_DISPLAY_COLOR_YUV420:
    	    	ulTmpBufSize = usPreviewW * usPreviewH;

     	    	pPreviewBuf->ulRotateYBuf[i] = cur_buf;
     	    	pPreviewBuf->ulRotateUBuf[i] = pPreviewBuf->ulRotateYBuf[i] + ALIGN32(ulTmpBufSize);
     	    	pPreviewBuf->ulRotateVBuf[i] = pPreviewBuf->ulRotateUBuf[i] + ALIGN32(ulTmpBufSize >> 2);
            	cur_buf += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>2)*2);
    			break;
    		case MMP_DISPLAY_COLOR_YUV420_INTERLEAVE:
    	    	ulTmpBufSize = usPreviewW * usPreviewH;

     	    	pPreviewBuf->ulRotateYBuf[i] = cur_buf;
     	    	pPreviewBuf->ulRotateUBuf[i] = pPreviewBuf->ulRotateYBuf[i] + ALIGN32(ulTmpBufSize);
     	    	pPreviewBuf->ulRotateVBuf[i] = pPreviewBuf->ulRotateUBuf[i];
            	cur_buf += (ALIGN32(ulTmpBufSize) + ALIGN32(ulTmpBufSize>>1));
    			break;
    		}
		}
    
        if (bIsPrevwResolSet)
        {
            /* Set buffer to black color */
            if (m_sAhcDecMjpegToPreviewInfo.sDispColor == MMP_DISPLAY_COLOR_YUV420_INTERLEAVE) {
                MEMSET((void*)(pPreviewBuf->ulRotateYBuf[0]), YUV420_Y_BLACK_COLOR, ulYSize);
                MEMSET((void*)(pPreviewBuf->ulRotateUBuf[0]), YUV420_UV_BLACK_COLOR, (ulYSize>>1));
                MEMSET((void*)(pPreviewBuf->ulRotateYBuf[1]), YUV420_Y_BLACK_COLOR, ulYSize);
                MEMSET((void*)(pPreviewBuf->ulRotateUBuf[1]), YUV420_UV_BLACK_COLOR, (ulYSize>>1));
            }
            else {
                DBG_S(0, "[ERR]: UNSUPPORTED COLOR!");
                DBG_B(0,m_sAhcDecMjpegToPreviewInfo.sDispColor);
                DBG_S(0, " #");  DBG_L(0,__LINE__);  DBG_S(0, "\r\n");
            }
            
            /* First time: Update Window Address */
            /* Fix refresh hang, set UVC preview address once buffer allocated */
            MMPD_Display_UpdateWinAddr( ePreviewWinID,
                                        pPreviewBuf->ulRotateYBuf[1],
                                        pPreviewBuf->ulRotateUBuf[1],
                                        pPreviewBuf->ulRotateVBuf[1]);
        }
    }
    else {
        pPreviewBuf->usRotateBufCnt = 0;
    }
    
    *pulSize = ALIGN32(cur_buf - ulAddr);
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_InitDecMjpegToPreview
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_InitDecMjpegToPreview(MMP_USHORT usJpegSrcW, MMP_USHORT usJpegSrcH)
{
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    MMP_ERR                         retstatus;
    MMP_SCAL_FIT_RANGE              fitrange;
    MMP_SCAL_GRAB_CTRL              grabctl;
    MMP_SCAL_FIT_MODE               sFitmode;
    MMP_DSC_JPEG_INFO               sJpegInfo;
    MMPD_FCTL_ATTR                  fctlAttr;
    MMP_USHORT                      usModeIdx = m_VidRecdModes.usVideoPreviewMode;
    MMP_USHORT                      i = 0;
    
    MMP_ULONG                       ulWinOffsetX, ulWinOffsetY;
    MMP_ULONG                       ulPrevwBufW, ulPrevwBufH;
    MMP_ULONG                       ulRotateW, ulRotateH;
    MMP_ULONG                       ulDispWidth, ulDispHeight;
    MMP_DISPLAY_ROTATE_TYPE         RotateType 	= MMP_DISPLAY_ROTATE_NO_ROTATE;
    MMP_BOOL                        bDmaRotateEn;
    MMP_DISPLAY_COLORMODE           DispColor   = MMP_DISPLAY_COLOR_YUV422;
    MMP_DISPLAY_DISP_ATTR           dispAttr;
    MMPS_3GPRECD_PREVIEW_BUFINFO    *pPreviewBuf = &m_sDecMjpegPrevwBufInfo;
    MMP_DISPLAY_WIN_ID              ePreviewWinID = GET_VR_PREVIEW_WINDOW(USBH_SENSOR);
    MMP_DISPLAY_DEV_TYPE            ePreviewDev = m_VidRecdConfigs.previewdata[0].DispDevice[usModeIdx];
    
    if (!MMP_IsSupportDecMjpegToPreview()) {
        return MMP_ERR_NONE;
    }
    
    MMPD_System_EnableClock(MMPD_SYS_CLK_JPG, 	MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYS_CLK_SCALE, MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYS_CLK_ICON,	MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYS_CLK_IBC, 	MMP_TRUE);

    /* If we have known the Jpeg resolution, we don't need to parse it again */
    if (usJpegSrcW != 0 && usJpegSrcH != 0) {
        sJpegInfo.bValid = MMP_TRUE;
    
        sJpegInfo.usPrimaryWidth   = usJpegSrcW;
        sJpegInfo.usPrimaryHeight  = usJpegSrcH;
        sJpegInfo.jpgFormat        = MMP_DSC_JPEG_FMT422;//TBD
    }
    else {
        sJpegInfo.bValid = MMP_FALSE;
    }
    
    /* Get JPEG information */
    if (!sJpegInfo.bValid)
    {
        // Need to set sJpegInfo.ulJpegBufAddr and sJpegInfo.ulJpegBufSize for parse bitstream.
        if (MMPD_DSC_GetJpegInfo(&sJpegInfo) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "Parse JPEG Info\r\n");
            return MMP_DSC_ERR_JPEGINFO_FAIL;
        }

        if (sJpegInfo.jpgFormat == MMP_DSC_JPEG_FMT_NOT_BASELINE) {
            return MMP_DSC_ERR_JPEGINFO_FAIL;
        }
    }
    
    /* Inform USBH the source frame information */
    MMPD_DSC_SetDecMjpegToPreviewSrcAttr(sJpegInfo.usPrimaryWidth, sJpegInfo.usPrimaryHeight);

    MMPD_DSC_SetDecMjpegToPreviewPipe((MMP_UBYTE)m_DecMjpegToPrevwFctlLink.ibcpipeID);

    // For sonix rear cam and LCD is 854x480. Tune IBC 2 performance 
    // For WIFI preview sometimes enc fail and cause image shift issue.
    #if (SUPPORT_COMPONENT_FLOW_CTL)
    MMPD_VIDENC_TunePipeMaxMCIPriority((MMP_UBYTE)m_DecMjpegToPrevwFctlLink.ibcpipeID);
    #else
    MMPD_VIDENC_TunePipe2ndMCIPriority((MMP_UBYTE)m_DecMjpegToPrevwFctlLink.ibcpipeID);
    #endif

    /* Initial Decode Output Resolution */
    bDmaRotateEn    = m_sAhcDecMjpegToPreviewInfo.bPreviewRotate;
    RotateType 		= m_sAhcDecMjpegToPreviewInfo.sPreviewDmaDir;
    sFitmode		= m_sAhcDecMjpegToPreviewInfo.sFitMode;
    ulWinOffsetX 	= m_sAhcDecMjpegToPreviewInfo.ulDispStartX;
    ulWinOffsetY 	= m_sAhcDecMjpegToPreviewInfo.ulDispStartY;
    ulDispWidth   	= m_sAhcDecMjpegToPreviewInfo.ulDispWidth;
    ulDispHeight  	= m_sAhcDecMjpegToPreviewInfo.ulDispHeight;
    ulPrevwBufW     = m_sAhcDecMjpegToPreviewInfo.ulPreviewBufW;
    ulPrevwBufH     = m_sAhcDecMjpegToPreviewInfo.ulPreviewBufH;
    DispColor       = m_sAhcDecMjpegToPreviewInfo.sDispColor;
    
    /* Calculate the grab range of decode out image */
    fitrange.fitmode    	= sFitmode;
    fitrange.scalerType 	= MMP_SCAL_TYPE_SCALER;
    fitrange.ulInWidth  	= sJpegInfo.usPrimaryWidth;
    fitrange.ulInHeight 	= sJpegInfo.usPrimaryHeight;
    fitrange.ulOutWidth  	= ulPrevwBufW;
    fitrange.ulOutHeight 	= ulPrevwBufH;

    fitrange.ulInGrabX 		= 1;
    fitrange.ulInGrabY 		= 1;
    fitrange.ulInGrabW 		= fitrange.ulInWidth;
    fitrange.ulInGrabH 		= fitrange.ulInHeight;
    fitrange.ubChoseLit     = 0;

    if ((fitrange.ulOutWidth > fitrange.ulInWidth) && 
        (fitrange.ulOutHeight > fitrange.ulInHeight)) {
        
        fitrange.ulOutWidth  = fitrange.ulInWidth;
        fitrange.ulOutHeight = fitrange.ulInHeight;
    }
    
    MMPD_Scaler_GetGCDBestFitScale(&fitrange, &grabctl);
    
    fctlAttr.bRtModeOut         = MMP_FALSE;
    fctlAttr.colormode          = DispColor;
    if (fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB565 || 
        fctlAttr.colormode == MMP_DISPLAY_COLOR_RGB888) {
        fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_RGB;
    }
    else {
        #if (CCIR656_FORCE_SEL_BT601)
        fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
        #else
        fctlAttr.eScalColorRange = MMP_SCAL_COLRMTX_YUV_FULLRANGE;
        #endif
    }
    fctlAttr.fctllink           = m_DecMjpegToPrevwFctlLink;
    fctlAttr.fitrange           = fitrange;
    fctlAttr.grabctl            = grabctl;
    fctlAttr.scalsrc            = MMP_SCAL_SOURCE_JPG;
    fctlAttr.sScalDelay         = m_sFullSpeedScalDelay;
    fctlAttr.bSetScalerSrc      = MMP_TRUE;
    fctlAttr.ubPipeLinkedSnr    = USBH_SENSOR;
    fctlAttr.usBufCnt           = pPreviewBuf->usBufCnt;

    for (i = 0; i < fctlAttr.usBufCnt; i++) {
        fctlAttr.ulBaseAddr[i]  = pPreviewBuf->ulYBuf[i];
        fctlAttr.ulBaseUAddr[i] = pPreviewBuf->ulUBuf[i];
        fctlAttr.ulBaseVAddr[i] = pPreviewBuf->ulVBuf[i];
    }

    if (bDmaRotateEn) {
        fctlAttr.bUseRotateDMA  = MMP_TRUE;
        fctlAttr.usRotateBufCnt = pPreviewBuf->usRotateBufCnt;
        
        for (i = 0; i < fctlAttr.usRotateBufCnt; i++) {
            fctlAttr.ulRotateAddr[i]  = pPreviewBuf->ulRotateYBuf[i];
            fctlAttr.ulRotateUAddr[i] = pPreviewBuf->ulRotateUBuf[i];
            fctlAttr.ulRotateVAddr[i] = pPreviewBuf->ulRotateVBuf[i];
        }
    }
    else {
        fctlAttr.bUseRotateDMA = MMP_FALSE;
        fctlAttr.usRotateBufCnt = 0;
    }
    
    m_DecMjpegToPrevwFctlAttr = fctlAttr;
    
    retstatus = MMPD_Fctl_SetPipeAttrForIbcFB(&fctlAttr);

    if (retstatus != MMP_ERR_NONE) {
        return retstatus;
    }

    MMPD_Fctl_ClearPreviewBuf(m_DecMjpegToPrevwFctlLink.ibcpipeID, 0xFFFFFF);

    if (bDmaRotateEn) {
        MMPD_Fctl_LinkPipeToDma(m_DecMjpegToPrevwFctlLink.ibcpipeID,
                                ePreviewWinID, 
                                ePreviewDev,
                                RotateType);
    }
    else {
        MMPD_Fctl_LinkPipeToDisplay(m_DecMjpegToPrevwFctlLink.ibcpipeID, 
                                    ePreviewWinID, 
                                    ePreviewDev);
    }
    
    /* Set display window parameter */
    if (bDmaRotateEn) {
        // Rotate 90/270 for vertical panel
        ulRotateW = ulPrevwBufH;
        ulRotateH = ulPrevwBufW;
    }
    else {
        ulRotateW = ulPrevwBufW;
        ulRotateH = ulPrevwBufH;
    }
    
    dispAttr.usStartX           = 0;
    dispAttr.usStartY           = 0;
    dispAttr.usDisplayOffsetX   = ulWinOffsetX;
   	dispAttr.usDisplayOffsetY   = ulWinOffsetY;
    dispAttr.usDisplayWidth     = ulRotateW;
    dispAttr.usDisplayHeight    = ulRotateH;
    dispAttr.bMirror            = MMP_FALSE;
    
    if (bDmaRotateEn) {
        dispAttr.rotatetype     = MMP_DISPLAY_ROTATE_NO_ROTATE;
    }
    else {
        dispAttr.rotatetype     = m_sAhcDecMjpegToPreviewInfo.sPreviewDmaDir;
    }
       
    MMPD_Display_SetWinToDisplay(ePreviewWinID, &dispAttr);

    /* Set window scaling function */
    do {
        
        fitrange.fitmode     = sFitmode;
        fitrange.scalerType	 = MMP_SCAL_TYPE_WINSCALER;
        fitrange.ulInWidth   = dispAttr.usDisplayWidth;
        fitrange.ulInHeight  = dispAttr.usDisplayHeight;
        fitrange.ulOutWidth  = m_sAhcDecMjpegToPreviewInfo.ulDispWidth;
        fitrange.ulOutHeight = m_sAhcDecMjpegToPreviewInfo.ulDispHeight;
        
        fitrange.ulInGrabX	 = 1;
        fitrange.ulInGrabY	 = 1;
        fitrange.ulInGrabW	 = fitrange.ulInWidth;
        fitrange.ulInGrabH	 = fitrange.ulInHeight;
        fitrange.ubChoseLit  = 0;

        MMPD_Scaler_GetGCDBestFitScale(&fitrange, &grabctl);

        retstatus =	MMPD_Display_SetWinScaling(ePreviewWinID, &fitrange, &grabctl);

    } while (retstatus == MMP_DISPLAY_ERR_OVERRANGE);

    /* Enable encode/decode interrupt, must set before preview start. */
    MMPF_USBH_InitDecMjpegToPreview();

#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_DecMjpegPreviewStart
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_DecMjpegPreviewStart(MMP_UBYTE ubSnrSel)
{
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW) 
    MMP_ERR sRet = MMP_ERR_NONE;
    
    if (!MMP_IsSupportDecMjpegToPreview()) {
        return MMP_ERR_NONE;
    }
    
    MMPD_Display_ResetRotateDMABufIdx(MMP_DISPLAY_SRC_REARCAM);

    #if (SUPPORT_COMPONENT_FLOW_CTL)
    if (CAM_CHECK_USB(USB_CAM_AIT) ||
        CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264)) {
        
        if (m_ubCamPreviewListId[USBH_SENSOR] == 0xFF) {
            
            MMP_UBYTE   ubPipeId  = m_DecMjpegToPrevwFctlAttr.fctllink.ibcpipeID;
            MMP_USHORT  usModeIdx = m_VidRecdModes.usVideoPreviewMode;
            MMP_UBYTE   ubCompIdArray[7];

            ubCompIdArray[0] = MMP_COMPONENT_ID_NULL;
            ubCompIdArray[1] = MMP_COMPONENT_ID_DRAM_JPGDEC_IN;
            ubCompIdArray[2] = MMP_COMPONENT_ID_JPGDEC;
            ubCompIdArray[3] = TRANS_PIPE_TO_PIPECOMP_ID(ubPipeId);
            ubCompIdArray[4] = TRANS_PIPE_TO_DRAMCOMP_ID(ubPipeId);
            ubCompIdArray[5] = MMP_COMPONENT_ID_DISP;
            ubCompIdArray[6] = MMP_COMPONENT_ID_NULL;

            //MMP_CompCtl_RegisterJpegDecDramComponent(MMP_COMPONENT_USAGE_ID2, 2, NULL); // Move to mmpf_usbh_ctl.c
            MMP_CompCtl_RegisterJpegDecComponent(MMP_COMPONENT_USAGE_ID2, m_usDecMjpegToPreviewSrcW, m_usDecMjpegToPreviewSrcH);
            
            MMP_CompCtl_RegisterPipeComponent(MMP_COMPONENT_USAGE_ID2, (void*)&m_DecMjpegToPrevwFctlAttr);
            MMP_CompCtl_RegisterPipeOutDramComponent(MMP_COMPONENT_USAGE_ID2, (void*)&m_DecMjpegToPrevwFctlAttr);
            MMP_CompCtl_RegisterDisplayComponent(MMP_COMPONENT_USAGE_ID2,
                                                 GET_VR_PREVIEW_WINDOW(ubSnrSel), 
                                                 m_VidRecdConfigs.previewdata[0].DispDevice[usModeIdx]);
            
            m_ubCamPreviewListId[USBH_SENSOR] = MMP_CompCtl_LinkComponentsEx(MMP_COMPONENT_USAGE_ID2, &ubCompIdArray[0], 7, (void*)_TrigEmptyJpegBSBuffer);
        }
    }
    #endif

#if (CCIR656_FORCE_SEL_BT601) // Workaround for MMPF_Display_FrameDoneTrigger handle
    sRet = MMPD_Fctl_EnablePreview(SCD_SENSOR, m_DecMjpegToPrevwFctlLink.ibcpipeID, MMP_TRUE, MMP_FALSE);
#else
    #if (SUPPORT_COMPONENT_FLOW_CTL) // Temp workaround, prevent from record stop to close VIF of SCD sensor
    sRet = MMPD_Fctl_EnablePreview(SCD_SENSOR, m_DecMjpegToPrevwFctlLink.ibcpipeID, MMP_TRUE, MMP_TRUE);
    #else
    sRet = MMPD_Fctl_EnablePreview(ubSnrSel, m_DecMjpegToPrevwFctlLink.ibcpipeID, MMP_TRUE, MMP_TRUE); 
    #endif
#endif

    if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_DecMjpegPreviewStop
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_DecMjpegPreviewStop(MMP_UBYTE ubSnrSel)
{
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    MMP_ERR sRet = MMP_ERR_NONE;

    if (!MMP_IsSupportDecMjpegToPreview()) {
        return MMP_ERR_NONE;
    }

    sRet = MMPD_Fctl_EnablePreview(ubSnrSel, m_DecMjpegToPrevwFctlLink.ibcpipeID, MMP_FALSE, MMP_FALSE); 
    
    if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}     
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetDecMjpegToEncodeAttr
//  Description : Need to set before MMPS_3GPRECD_InitDecMjpegToEncode()
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetDecMjpegToEncodeAttr(MMP_UBYTE sFitMode, 
                                             MMP_USHORT usSrcWidth, MMP_USHORT usSrcHeight, 
                                             MMP_USHORT usEncWidth, MMP_USHORT usEncHeight)
{
#if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
    m_sAhcDecMjpegToEncodeInfo.bUserDefine  = MMP_TRUE;
    m_sAhcDecMjpegToEncodeInfo.sFitMode		= sFitMode;
    m_sAhcDecMjpegToEncodeInfo.ulVideoEncW  = usEncWidth;
    m_sAhcDecMjpegToEncodeInfo.ulVideoEncH  = usEncHeight;
    
    m_usAhcDecMjpegToEncodeSrcW = usSrcWidth;
    m_usAhcDecMjpegToEncodeSrcH = usSrcHeight;
#endif
    return MMP_ERR_NONE;
}

MMP_ERR MMPS_3GPRECD_GetMjpegVideoEncWH(MMP_UBYTE ubEncIdx, MMP_ULONG *usEncWidth, MMP_ULONG *usEncHeight)
{
#if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
	*usEncWidth  = m_sAhcDecMjpegToEncodeInfo.ulVideoEncW;
	*usEncHeight = m_sAhcDecMjpegToEncodeInfo.ulVideoEncH;
#endif
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_InitDecMjpegToEncode
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set video record pipe configuration.
 @param[in] *pInputBuf 		Pointer to HW used buffer.
 @param[in] ubEncId         Encode instance ID
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_InitDecMjpegToEncode(VIDENC_INPUT_BUF *pInputBuf, MMP_ULONG ubEncId)
{
#if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
    MMP_SCAL_FIT_RANGE      fitrange;
    MMP_SCAL_GRAB_CTRL      EncodeGrabctl;
    MMPD_FCTL_ATTR          fctlAttr;
    MMP_USHORT              i;

    if (!MMP_IsSupportDecMjpegToEncH264()) {
        return MMP_ERR_NONE;
    }
    
    /* Parameter Check */
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    if (pInputBuf == NULL) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    MMPD_DSC_SetDecMjpegToEncodePipe((MMP_UBYTE)m_DecMjpegToEncFctlLink.ibcpipeID);

    /* Config Video Record Pipe */
    fitrange.fitmode        = m_sAhcDecMjpegToEncodeInfo.sFitMode;
    fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;
    fitrange.ulInWidth      = m_usAhcDecMjpegToEncodeSrcW;
    fitrange.ulInHeight     = m_usAhcDecMjpegToEncodeSrcH;
    fitrange.ulOutWidth     = m_sAhcDecMjpegToEncodeInfo.ulVideoEncW;
    fitrange.ulOutHeight    = m_sAhcDecMjpegToEncodeInfo.ulVideoEncH;

    fitrange.ulInGrabX      = 1;
    fitrange.ulInGrabY      = 1;
    fitrange.ulInGrabW      = fitrange.ulInWidth;
    fitrange.ulInGrabH      = fitrange.ulInHeight;
    fitrange.ubChoseLit     = 0;

    MMPD_Scaler_GetGCDBestFitScale(&fitrange, &EncodeGrabctl);
    
    fctlAttr.colormode          = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
    fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
    fctlAttr.fctllink           = m_DecMjpegToEncFctlLink;
    fctlAttr.fitrange           = fitrange;
    fctlAttr.grabctl            = EncodeGrabctl;
    fctlAttr.scalsrc            = MMP_SCAL_SOURCE_JPG;
    fctlAttr.bSetScalerSrc      = MMP_TRUE;
    fctlAttr.ubPipeLinkedSnr    = USBH_SENSOR;
    fctlAttr.usBufCnt           = pInputBuf->ulBufCnt;
    fctlAttr.bUseRotateDMA 	    = MMP_FALSE;

    for (i = 0; i < fctlAttr.usBufCnt; i++) {
        if (pInputBuf->ulY[i] != 0)
            fctlAttr.ulBaseAddr[i] = pInputBuf->ulY[i];
        if (pInputBuf->ulU[i] != 0)	
            fctlAttr.ulBaseUAddr[i] = pInputBuf->ulU[i];
        if (pInputBuf->ulV[i] != 0)
            fctlAttr.ulBaseVAddr[i] = pInputBuf->ulV[i];
    }
    
    /* Dual H264 Only support frame mode */
    if (m_VidRecdModes.VidCurBufMode[1] == VIDENC_CURBUF_RT) {
        fctlAttr.bRtModeOut = MMP_TRUE;
        RTNA_DBG_Str0("Dual H264 Only support frame mode\r\n");
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }
    else {
        fctlAttr.sScalDelay = m_sFullSpeedScalDelay;
        fctlAttr.bRtModeOut = MMP_FALSE;
        MMPD_Fctl_SetPipeAttrForH264FB(&fctlAttr);
    }
    
    m_DecMjpegToEncFctlAttr = fctlAttr;
    
    MMPD_Fctl_LinkPipeToVideo(m_DecMjpegToEncFctlLink.ibcpipeID, ubEncId);

    #if (SUPPORT_COMPONENT_FLOW_CTL)
    if (CAM_CHECK_USB(USB_CAM_AIT) ||
        CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264)) {
        
        if (m_ubCamEncodeListId[USBH_SENSOR] == 0xFF) {
            
            MMP_UBYTE ubPipeId  = m_DecMjpegToEncFctlAttr.fctllink.ibcpipeID;
            MMP_UBYTE ubCompIdArray[7];

            ubCompIdArray[0] = MMP_COMPONENT_ID_NULL;
            ubCompIdArray[1] = MMP_COMPONENT_ID_DRAM_JPGDEC_IN;
            ubCompIdArray[2] = MMP_COMPONENT_ID_JPGDEC;
            ubCompIdArray[3] = TRANS_PIPE_TO_PIPECOMP_ID(ubPipeId);
            ubCompIdArray[4] = TRANS_PIPE_TO_DRAMCOMP_ID(ubPipeId);
            ubCompIdArray[5] = MMP_COMPONENT_ID_H264ENC;
            ubCompIdArray[6] = MMP_COMPONENT_ID_NULL;

            //MMP_CompCtl_RegisterJpegDecDramComponent(MMP_COMPONENT_USAGE_ID2, 2, NULL); // Move to mmpf_usbh_ctl.c
            MMP_CompCtl_RegisterJpegDecComponent(MMP_COMPONENT_USAGE_ID2, m_usAhcDecMjpegToEncodeSrcW, m_usAhcDecMjpegToEncodeSrcH);
            
            MMP_CompCtl_RegisterPipeComponent(MMP_COMPONENT_USAGE_ID2, (void*)&m_DecMjpegToEncFctlAttr);
            MMP_CompCtl_RegisterPipeOutDramComponent(MMP_COMPONENT_USAGE_ID2, (void*)&m_DecMjpegToEncFctlAttr);
            MMP_CompCtl_RegisterH264Component(MMP_COMPONENT_USAGE_ID2, (void*)&m_DecMjpegToEncFctlAttr);

            m_ubCamEncodeListId[USBH_SENSOR] = MMP_CompCtl_LinkComponentsEx(MMP_COMPONENT_USAGE_ID2, &ubCompIdArray[0], 7, (void*)_TrigEmptyJpegBSBuffer);
        }
    }
    #endif

    /* Tune MCI priority of encode pipe for frame based mode */
    if (m_VidRecdModes.VidCurBufMode[1] == VIDENC_CURBUF_FRAME) {
        MMPD_VIDENC_TunePipe2ndMCIPriority(m_DecMjpegToEncFctlLink.ibcpipeID);
    }

#endif
    return MMP_ERR_NONE;
}

#if 0
void ____VR_Common_Capture_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetStillCaptureAddr
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetStillCaptureAddr(MMP_ULONG *pulSramAddr, MMP_ULONG *pulDramAddr)
{
    *pulSramAddr = m_ulVidRecCaptureSramAddr;
    *pulDramAddr = m_ulVidRecCaptureDramAddr;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetStillCaptureEndAddr
//  Description : CHECK
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetStillCaptureEndAddr(MMP_ULONG *pulSramBufAddr, MMP_ULONG *pulDramBufAddr)
{
    *pulSramBufAddr = m_ulVidRecCaptureEndSramAddr;
    *pulDramBufAddr = m_ulVidRecCaptureEndDramAddr;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetStillCaptureMaxRes
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set max still capture resolution.
 @param[in] usJpegW max still JPEG width
 @param[in] usJpegH max still JPEG height
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetStillCaptureMaxRes(MMP_USHORT usJpegW, MMP_USHORT usJpegH)
{
    m_usMaxStillJpegW = usJpegW;
    m_usMaxStillJpegH = usJpegH;
    
    return MMP_ERR_NONE;
}

#if (HANDLE_JPEG_EVENT_BY_QUEUE)
#if (USE_H264_CUR_BUF_AS_CAPT_BUF)
//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_PresetStillCaptureBuffer
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR MMPS_3GPRECD_PresetStillCaptureBuffer(VIDENC_INPUT_BUF *pInputBuf)
{
    MMP_ERR	    mmpstatus;
    MMP_USHORT  i;
    
    for (i = 0; i < pInputBuf->ulBufCnt; i++) 
	{
	    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);
        MMPH_HIF_SetParameterW(GRP_IDX_FLOWCTL, 0, m_RecordFctlLink.ibcpipeID);
        MMPH_HIF_SetParameterW(GRP_IDX_FLOWCTL, 2, i);
        MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 4, pInputBuf->ulY[i]);
        MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 8, pInputBuf->ulU[i]);
        MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 12, pInputBuf->ulV[i]);
        mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_SET_PREVIEW_BUF | BUFFER_ADDRESS);
	    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);
	}
    return mmpstatus;
}
#endif
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_DualBayerSnrStillCapture
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_DualBayerSnrStillCapture(MMPS_3GPRECD_STILL_CAPTURE_INFO *pVRCaptInfo)
{
    MMP_ULONG               ulJpegSize;
    VIDENC_FW_STATUS        status_fw;
    MMPS_DSC_CAPTURE_INFO 	captureinfo;
    MMP_IBC_PIPEID     		ubCaptPipe;
    MMP_SCAL_FIT_RANGE   	fitrange;
    MMP_SCAL_GRAB_CTRL 		grabctl;
    MMP_SCAL_SOURCE			ScalerSrc   = MMP_SCAL_SOURCE_ISP;
    MMP_ERR 				err         = MMP_ERR_NONE;
    MMP_ULONG				ulSramAddr  = m_ulVidRecCaptureSramAddr;
    MMP_ULONG				ulDramAddr  = m_ulVidRecCaptureDramAddr;

    /* Parameter Check */
    if (!MMP_IsDualVifCamEnable()) {
        return MMP_ERR_NONE;
    }

    if (!m_VidRecdConfigs.bStillCaptureEnable) {
        return err;
    }
    
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID ||
        m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI/*TBD*/) {
        return err;
    }
    
    if (pVRCaptInfo->sCaptSrc & MMP_3GPRECD_CAPTURE_SRC_FRONT_CAM)
    {
        /* Check the pipe is alive or not */
        if (!m_bVidPreviewActive[0]) {
            return MMP_3GPRECD_ERR_STILL_CAPTURE;
        }

        if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE) // For DSC mode use
            ubCaptPipe = m_RecordFctlLink.ibcpipeID;
        else
            ubCaptPipe = m_LdcCaptureFctlLink.ibcpipeID;

        fitrange.fitmode    = MMP_SCAL_FITMODE_OUT;
        fitrange.scalerType = MMP_SCAL_TYPE_SCALER;

        if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE) {
            MMPS_Sensor_GetCurPrevScalInputRes(PRM_SENSOR, &fitrange.ulInWidth, &fitrange.ulInHeight);
            fitrange.ulInWidth = FLOOR32(fitrange.ulInWidth);
            fitrange.ulInHeight = FLOOR32(fitrange.ulInHeight);
            
            fitrange.ulInWidth *= 2;
        }
        else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
            if (MMP_GetParallelFrmStoreType() != PARALLEL_FRM_NOT_SUPPORT)
                fitrange.ulInWidth = m_ulLdcMaxOutWidth * 2;
            else
                fitrange.ulInWidth = m_ulLdcMaxOutWidth;
            
            fitrange.ulInHeight	= m_ulLdcMaxOutHeight;
        }
        else {
            fitrange.ulInWidth  = m_ulLdcMaxOutWidth;
            fitrange.ulInHeight	= m_ulLdcMaxOutHeight;
        }
        
        fitrange.ulOutWidth	    = pVRCaptInfo->usWidth;
        fitrange.ulOutHeight	= pVRCaptInfo->usHeight;

        fitrange.ulInGrabX  	= 1;
        fitrange.ulInGrabY  	= 1;
        fitrange.ulInGrabW  	= fitrange.ulInWidth;
        fitrange.ulInGrabH  	= fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        
        MMPD_Scaler_GetGCDBestFitScale(&fitrange, &grabctl);
        
        #if (HANDLE_JPEG_EVENT_BY_QUEUE)
        if (MMPF_JPEG_GetCtrlByQueueEnable()) {
            MMPD_System_EnableClock(MMPD_SYS_CLK_JPG, MMP_TRUE);
            
            err = MMPS_DSC_CaptureByQueue(ubCaptPipe, 
                                          &captureinfo,
                                          ScalerSrc,
                                          &fitrange, 
                                          &grabctl,
                                          &ulJpegSize,
                                          ulDramAddr,
                                          ulSramAddr,
                                          m_ulVidRecCaptureFrmBufAddr,
                                          pVRCaptInfo,
                                          MMP_FALSE/*bRecording*/);
        }
        else {

            MMPS_DSC_SetShotMode(MMPS_DSC_SINGLE_SHOTMODE);
            MMPS_DSC_SetJpegEncParam(DSC_RESOL_IDX_UNDEF, pVRCaptInfo->usWidth, pVRCaptInfo->usHeight, MMP_DSC_JPEG_RC_ID_CAPTURE);
            
            MMPS_DSC_ConfigThumbnail(pVRCaptInfo->usThumbWidth, pVRCaptInfo->usThumbHeight, MMP_DSC_THUMB_SRC_NONE);

            MMPS_DSC_SetCaptureJpegQuality(MMP_DSC_JPEG_RC_ID_CAPTURE,
                                           pVRCaptInfo->bTargetCtl, pVRCaptInfo->bLimitCtl, pVRCaptInfo->bTargetSize,
                                           pVRCaptInfo->bLimitSize, pVRCaptInfo->bMaxTrialCnt, pVRCaptInfo->Quality);

            MMPS_DSC_SetSystemBuf(&ulDramAddr, MMP_FALSE, MMP_TRUE, MMP_TRUE);
            MMPS_DSC_SetCaptureBuf(&ulSramAddr, &ulDramAddr, MMP_TRUE, MMP_FALSE, MMP_DSC_CAPTURE_NO_ROTATE, MMP_TRUE);

            captureinfo.bFirstShot 	= MMP_TRUE;
            captureinfo.bExif 	    = pVRCaptInfo->bExifEn;
            captureinfo.bThumbnail 	= MMP_FALSE;
            
            if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE) { // TBD, Move to CaptureAfterDualBayerSnrReady()
                ScalerSrc = MMP_SCAL_SOURCE_LDC;
                err = MMPS_DSC_CaptureAfterSrcReady(ubCaptPipe, &captureinfo, ScalerSrc, &fitrange, &grabctl, &ulJpegSize, MMP_FALSE);
            }
            else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
                ScalerSrc = MMP_SCAL_SOURCE_GRA;
                err = MMPS_DSC_CaptureAfterDualBayerSnrReady(ubCaptPipe, &captureinfo, ScalerSrc, &fitrange, &grabctl, &ulJpegSize, MMP_TRUE);
            }
            else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE) {
                ScalerSrc = MMP_SCAL_SOURCE_GRA;
                err = MMPS_DSC_CaptureAfterDualBayerSnrReady(ubCaptPipe, &captureinfo, ScalerSrc, &fitrange, &grabctl, &ulJpegSize, MMP_FALSE);
            }
        }
        #endif
 
        if (err == MMP_ERR_NONE) 
        {
            captureinfo.ubFilename      = pVRCaptInfo->bFileName;
            captureinfo.usFilenamelen   = pVRCaptInfo->ulFileNameLen;
            captureinfo.bFirstShot      = MMP_TRUE;
            captureinfo.ulExtraBufAddr	= 0;

            MMPD_VIDENC_GetStatus(VIDENC_STREAMTYPE_VIDRECD, &status_fw);
            
            if ((status_fw == VIDENC_FW_STATUS_START) ||
                (status_fw == VIDENC_FW_STATUS_PAUSE) ||
                (status_fw == VIDENC_FW_STATUS_RESUME)) {
                
                /* Inform encoder the reserved space is reduced */
                if ((err = MMPD_3GPMGR_MakeRoom(VIDENC_STREAMTYPE_VIDRECD, ulJpegSize)) != MMP_ERR_NONE) {
                    RTNA_DBG_Str(0, "FrontCam MakeRoom Err:");
                    RTNA_DBG_Long(0, err);
                    RTNA_DBG_Str(0, "\r\n");
                    return MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE;
                }
            }
            
            if ((err = MMPS_DSC_JpegDram2Card(&captureinfo)) != MMP_ERR_NONE) {
                RTNA_DBG_Str(0, "FrontCam Save Card Err:");
                RTNA_DBG_Long(0, err);
                RTNA_DBG_Str(0, ", ");
                RTNA_DBG_Str(0, captureinfo.ubFilename);
                RTNA_DBG_Str(0, "\r\n");
                return MMP_3GPRECD_ERR_STILL_CAPTURE;
            }
        }
        else {
            RTNA_DBG_Str(0, "FrontCam Capture After SrcReady Err:");
            RTNA_DBG_Long(0, err);
            RTNA_DBG_Str(0, "\r\n");
            return MMP_3GPRECD_ERR_STILL_CAPTURE;
        }
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_LdcStillCapture
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_LdcStillCapture(MMPS_3GPRECD_STILL_CAPTURE_INFO *pVRCaptInfo)
{
    MMP_ULONG               ulJpegSize;
    VIDENC_FW_STATUS        status_fw;
    MMPS_DSC_CAPTURE_INFO 	captureinfo;
    MMP_IBC_PIPEID     		ubCaptPipe;
    MMP_SCAL_FIT_RANGE   	fitrange;
    MMP_SCAL_GRAB_CTRL 		grabctl;
    MMP_SCAL_SOURCE			ScalerSrc   = MMP_SCAL_SOURCE_ISP;
    MMP_ERR 				err         = MMP_ERR_NONE;
    MMP_ULONG				ulSramAddr  = m_ulVidRecCaptureSramAddr;
    MMP_ULONG				ulDramAddr  = m_ulVidRecCaptureDramAddr;

    /* Parameter Check */
    if (!MMP_IsVidLdcSupport()) {
        return MMP_ERR_NONE;
    }

    if (!m_VidRecdConfigs.bStillCaptureEnable) {
        return err;
    }
    
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID ||
        m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE ||
        m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI/*TBD*/) {
        return err;
    }
    
    if (pVRCaptInfo->sCaptSrc & MMP_3GPRECD_CAPTURE_SRC_FRONT_CAM)
    {
        /* Check the pipe is alive or not */
        if (!m_bVidPreviewActive[0]) {
            return MMP_3GPRECD_ERR_STILL_CAPTURE;
        }

        ubCaptPipe = m_LdcCaptureFctlLink.ibcpipeID;

        fitrange.fitmode        = MMP_SCAL_FITMODE_OUT;
        fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;
        fitrange.ulInWidth      = m_ulLdcMaxOutWidth;
        fitrange.ulInHeight	    = m_ulLdcMaxOutHeight;
        fitrange.ulOutWidth	    = pVRCaptInfo->usWidth;
        fitrange.ulOutHeight	= pVRCaptInfo->usHeight;

        fitrange.ulInGrabX  	= 1;
        fitrange.ulInGrabY  	= 1;
        fitrange.ulInGrabW  	= fitrange.ulInWidth;
        fitrange.ulInGrabH  	= fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        
        MMPD_Scaler_GetGCDBestFitScale(&fitrange, &grabctl);

        MMPS_DSC_SetShotMode(MMPS_DSC_SINGLE_SHOTMODE);
        MMPS_DSC_SetJpegEncParam(DSC_RESOL_IDX_UNDEF, pVRCaptInfo->usWidth, pVRCaptInfo->usHeight, MMP_DSC_JPEG_RC_ID_CAPTURE);
        
        MMPS_DSC_ConfigThumbnail(pVRCaptInfo->usThumbWidth, pVRCaptInfo->usThumbHeight, MMP_DSC_THUMB_SRC_NONE);

        MMPS_DSC_SetCaptureJpegQuality(MMP_DSC_JPEG_RC_ID_CAPTURE,
                                       pVRCaptInfo->bTargetCtl, pVRCaptInfo->bLimitCtl, pVRCaptInfo->bTargetSize,
        							   pVRCaptInfo->bLimitSize, pVRCaptInfo->bMaxTrialCnt, pVRCaptInfo->Quality);

        MMPS_DSC_SetSystemBuf(&ulDramAddr, MMP_FALSE, MMP_TRUE, MMP_TRUE);
        MMPS_DSC_SetCaptureBuf(&ulSramAddr, &ulDramAddr, MMP_TRUE, MMP_FALSE, MMP_DSC_CAPTURE_NO_ROTATE, MMP_TRUE);

        captureinfo.bFirstShot 	= MMP_TRUE;
        captureinfo.bExif 	    = pVRCaptInfo->bExifEn;
        captureinfo.bThumbnail 	= MMP_FALSE;
        
        if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE) { // TBD, Move to CaptureAfterLdcReady()
            ScalerSrc = MMP_SCAL_SOURCE_LDC;
            err = MMPS_DSC_CaptureAfterSrcReady(ubCaptPipe, &captureinfo, ScalerSrc, &fitrange, &grabctl, &ulJpegSize, MMP_FALSE);
        }
        else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
            ScalerSrc = MMP_SCAL_SOURCE_GRA;
            err = MMPS_DSC_CaptureAfterLdcReady(ubCaptPipe, &captureinfo, ScalerSrc, &fitrange, &grabctl, &ulJpegSize);
        }
 
        if (err == MMP_ERR_NONE) 
        {
            captureinfo.ubFilename      = pVRCaptInfo->bFileName;
            captureinfo.usFilenamelen   = pVRCaptInfo->ulFileNameLen;
            captureinfo.bFirstShot 		= MMP_TRUE;	
            captureinfo.ulExtraBufAddr	= 0;

            MMPD_VIDENC_GetStatus(VIDENC_STREAMTYPE_VIDRECD, &status_fw);
            
            if ((status_fw == VIDENC_FW_STATUS_START) ||
                (status_fw == VIDENC_FW_STATUS_PAUSE) ||
                (status_fw == VIDENC_FW_STATUS_RESUME)) {
                
                /* Inform encoder the reserved space is reduced */
                if ((err = MMPD_3GPMGR_MakeRoom(VIDENC_STREAMTYPE_VIDRECD, ulJpegSize)) != MMP_ERR_NONE) {
                    RTNA_DBG_Str(0, "FrontCam MakeRoom Err:");
                    RTNA_DBG_Long(0, err);
                    RTNA_DBG_Str(0, "\r\n");
                    return MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE;
                }
            }

            if ((err = MMPS_DSC_JpegDram2Card(&captureinfo)) != MMP_ERR_NONE) {
                RTNA_DBG_Str(0, "FrontCam Save Card Err:");
                RTNA_DBG_Long(0, err);
                RTNA_DBG_Str(0, ", ");
                RTNA_DBG_Str(0, captureinfo.ubFilename);
                RTNA_DBG_Str(0, "\r\n");
                return MMP_3GPRECD_ERR_STILL_CAPTURE;
            }
        }
        else {
            RTNA_DBG_Str(0, "FrontCam Capture After SrcReady Err:");
            RTNA_DBG_Long(0, err);
            RTNA_DBG_Str(0, "\r\n");
            return MMP_3GPRECD_ERR_STILL_CAPTURE;
        }
    }

    return MMP_ERR_NONE;
}


extern MMPF_JPG_CTL_ATTR   m_sGra2JpgAttr[MMP_IBC_PIPE_MAX];
extern MMP_ULONG m_ulPrimaryJpegLineStart;

int MMPS_3GPRECD_StillCapture_NoPrmCam_SonixRearCamOnly_CB(void* pArg)
{
	MMP_ERR 	RearCamErr  = MMP_ERR_NONE;
	MMP_ULONG   ulJpegSize;
	AITPS_JPG   pJPG        = AITC_BASE_JPG;

	ulJpegSize = pJPG->JPG_ENC_FRAME_SIZE;
//	printc("MMPS_3GPRECD_StillCapture_NoPrmCam_SonixRearCamOnly_CB ulJpegSize %d \r\n\n", ulJpegSize);

    RearCamErr = MMPD_DSC_JpegDram2Card(m_ulVidRecRearCamCaptDramAddr,
                                        ulJpegSize,
                                        MMP_TRUE,
                                        MMP_TRUE);
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StillCapture
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_StillCapture(MMPS_3GPRECD_STILL_CAPTURE_INFO *pVRCaptInfo)
{
    MMP_USHORT              ulInWidth, ulInHeight;
    MMP_ULONG               ulJpegSize;
    MMP_BOOL                bDualCamCapture = MMP_FALSE;
    VIDENC_FW_STATUS        status_fw;
    MMPS_DSC_CAPTURE_INFO 	captureinfo;
    MMP_IBC_PIPEID     		ibc_pipe;
    MMP_SCAL_FIT_RANGE   	fitrange;
    MMP_SCAL_GRAB_CTRL 		grabctl;
    MMP_SCAL_SOURCE			ScalerSrc   = MMP_SCAL_SOURCE_ISP;
    MMP_ERR 				err         = MMP_ERR_NONE;
    MMP_ERR 				RearCamErr  = MMP_ERR_NONE;
    MMP_ULONG				ulSramAddr  = m_ulVidRecCaptureSramAddr;
    MMP_ULONG				ulDramAddr  = m_ulVidRecCaptureDramAddr;
    #if (HANDLE_JPEG_EVENT_BY_QUEUE)
    MMP_BOOL                bRecording  = MMP_FALSE;
    #endif
    #if (VR_STILL_USE_REF_FRM)
    MMP_BOOL                bVidRefFrmAsSrc = MMP_FALSE;
    MMP_GRAPHICS_BUF_ATTR	bufAttr = {0, };
    MMP_GRAPHICS_RECT		srcrect = {0, };
    #endif

    /* Parameter Check */
    if (!m_VidRecdConfigs.bStillCaptureEnable) {
        return err;
    }

    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID) {
        return err;
    }

    if (pVRCaptInfo->sCaptSrc & MMP_3GPRECD_CAPTURE_SRC_FRONT_CAM)
    {
        ScalerSrc   = MMP_SCAL_SOURCE_ISP;

        // Check the pipe is alive or not 
        if (!m_bVidPreviewActive[0]) {
            return MMP_3GPRECD_ERR_STILL_CAPTURE;
        }

        if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE || 
            m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI  ||
            m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
            
            if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
                return MMPS_3GPRECD_DualBayerSnrStillCapture(pVRCaptInfo);
            }
            else {
                return MMPS_3GPRECD_LdcStillCapture(pVRCaptInfo);
            }
        }
        else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE) {
            if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
                return MMPS_3GPRECD_DualBayerSnrStillCapture(pVRCaptInfo);
            }
            else {
                // Assign JPEG source pipe for non-recording case if HANDLE_JPEG_EVENT_BY_QUEUE is 1
                // Assign JPEG source pipe for recording case if HANDLE_JPEG_EVENT_BY_QUEUE is 0
                if (((CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_NONE))) || \
                    CAM_CHECK_USB(USB_CAM_AIT) || \
                    CAM_CHECK_SCD(SCD_CAM_TV_DECODER) || \
                    CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
                    #if (HANDLE_JPEG_EVENT_BY_QUEUE)
                    if (MMPF_JPEG_GetCtrlByQueueEnable()) {
                        ibc_pipe = m_RecordFctlLink.ibcpipeID;
                    }
                    else {
                        ibc_pipe = m_PreviewFctlLink.ibcpipeID;
                    }
                    #else
                    	ibc_pipe =m_PreviewFctlLink.ibcpipeID;
                    #endif
                }
                else if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264)) {
                    ibc_pipe = m_RecordFctlLink.ibcpipeID;
                }

                #if (VR_STILL_USE_REF_FRM)
                /* Use H264 reference frame to encode JPEG */
                MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);
                
                if ((status_fw == VIDENC_FW_STATUS_START) ||
                    (status_fw == VIDENC_FW_STATUS_PREENCODE)) {
                    bVidRefFrmAsSrc = MMP_TRUE;
                }
                #endif
            }
        }

        #if (VR_STILL_USE_REF_FRM)
        if (bVidRefFrmAsSrc) {
            ulInWidth   = m_ulVREncodeBufW[0];
            ulInHeight  = m_ulVREncodeBufH[0];
        }
        else {
            MMPD_Scaler_GetGrabRange(m_PreviewFctlLink.scalerpath,
                                     MMP_SCAL_GRAB_STAGE_LPF,
                                     &ulInWidth, &ulInHeight);
        }
        #else
        MMPS_Sensor_GetCurPrevScalInputRes(m_ub1stVRStreamSnrId, (MMP_ULONG *)&ulInWidth, (MMP_ULONG *)&ulInHeight);
        #endif

        fitrange.fitmode    	= MMP_SCAL_FITMODE_OUT;
        fitrange.scalerType 	= MMP_SCAL_TYPE_SCALER;
        fitrange.ulInWidth	    = ulInWidth;
        fitrange.ulInHeight 	= ulInHeight;
        fitrange.ulOutWidth	    = pVRCaptInfo->usWidth;
        fitrange.ulOutHeight	= pVRCaptInfo->usHeight;

        fitrange.ulInGrabX  	= 1;
        fitrange.ulInGrabY  	= 1;
        fitrange.ulInGrabW  	= fitrange.ulInWidth;
        fitrange.ulInGrabH  	= fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;

        MMPD_Scaler_GetGCDBestFitScale(&fitrange, &grabctl);

        #if (HANDLE_JPEG_EVENT_BY_QUEUE)
        if (MMPF_JPEG_GetCtrlByQueueEnable()) {
        
            #if (USE_H264_CUR_BUF_AS_CAPT_BUF)
            MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);
            
            if ((status_fw == VIDENC_FW_STATUS_START) ||
                (status_fw == VIDENC_FW_STATUS_PREENCODE)) {
                
                bRecording = MMP_TRUE;
            }
            else {
                bRecording = MMP_FALSE;
                  
                if (!m_bInitFcamRecPipeFrmBuf) {
                    m_bInitFcamRecPipeFrmBuf = MMP_TRUE;
                    MMPS_3GPRECD_PresetStillCaptureBuffer(&m_VidRecdInputBuf[0]);
                }
                
                MMPF_Display_GetCurIbcFrameBuffer(m_RecordFctlLink.ibcpipeID,
                                                  0/*ubUsageId*/,
                                                  &m_ulVidRecCaptureFrmBufAddr,
                                                  NULL,
                                                  NULL);
            }
            #endif
            
            MMPD_System_EnableClock(MMPD_SYS_CLK_JPG, MMP_TRUE);
            
            err = MMPS_DSC_CaptureByQueue(ibc_pipe, 
                                          &captureinfo,
                                          ScalerSrc,
                                          &fitrange, 
                                          &grabctl,
                                          &ulJpegSize,
                                          ulDramAddr,
                                          ulSramAddr,
                                          m_ulVidRecCaptureFrmBufAddr,
                                          pVRCaptInfo,
                                          bRecording);
        }
        else {

            MMPS_DSC_SetShotMode(MMPS_DSC_SINGLE_SHOTMODE);
            MMPS_DSC_SetJpegEncParam(DSC_RESOL_IDX_UNDEF, pVRCaptInfo->usWidth, pVRCaptInfo->usHeight, MMP_DSC_JPEG_RC_ID_CAPTURE);

            // No thumbnail in Video capture mode.
            MMPS_DSC_ConfigThumbnail(pVRCaptInfo->usThumbWidth, pVRCaptInfo->usThumbHeight, MMP_DSC_THUMB_SRC_NONE);
            
            MMPS_DSC_SetCaptureJpegQuality(MMP_DSC_JPEG_RC_ID_CAPTURE,
                                           pVRCaptInfo->bTargetCtl, pVRCaptInfo->bLimitCtl, pVRCaptInfo->bTargetSize,
                                           pVRCaptInfo->bLimitSize, pVRCaptInfo->bMaxTrialCnt, pVRCaptInfo->Quality);

            MMPS_DSC_SetSystemBuf(&ulDramAddr, MMP_FALSE, MMP_TRUE, MMP_TRUE);
            MMPS_DSC_SetCaptureBuf(&ulSramAddr, &ulDramAddr, MMP_TRUE, MMP_FALSE, MMP_DSC_CAPTURE_NO_ROTATE, MMP_TRUE);

            captureinfo.bFirstShot 	= MMP_TRUE;
            captureinfo.bExif 	    = pVRCaptInfo->bExifEn;
            captureinfo.bThumbnail  = pVRCaptInfo->bThumbEn;

            #if (VR_STILL_USE_REF_FRM)
            if (bVidRefFrmAsSrc) {
                /* Setup graphic settings for loop-back */
                ScalerSrc = MMP_SCAL_SOURCE_GRA;

                bufAttr.usWidth 		= m_ulVREncScalOutW[0];
                bufAttr.usHeight		= m_ulVREncScalOutH[0];
                bufAttr.usLineOffset	= bufAttr.usWidth;
                bufAttr.colordepth		= MMP_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE;

                srcrect.usLeft			= 0;
                srcrect.usTop			= 0;
                srcrect.usWidth 		= bufAttr.usWidth;
                srcrect.usHeight		= bufAttr.usHeight;

                MMPD_Graphics_SetScaleAttr(&bufAttr, &srcrect, 1);
                MMPD_Graphics_SetDelayType(MMP_GRAPHICS_DELAY_CHK_SCA_BUSY);
                MMPD_Graphics_SetPixDelay(10, 20);
                MMPD_Graphics_SetLineDelay(0);
            }
            else {
                ScalerSrc = MMP_SCAL_SOURCE_ISP;
            }
            #else
            ScalerSrc = MMP_SCAL_SOURCE_ISP;
            #endif

            if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) // 20160920: Workaround for UV shift after capture (Reason: Unknown)
                err = MMPS_DSC_CaptureAfterSrcReady(m_2ndPrewFctlLink.ibcpipeID, &captureinfo, ScalerSrc, &fitrange, &grabctl, &ulJpegSize, bDualCamCapture);
            else
                err = MMPS_DSC_CaptureAfterSrcReady(ibc_pipe, &captureinfo, ScalerSrc, &fitrange, &grabctl, &ulJpegSize, bDualCamCapture);
        }
        #endif // (HANDLE_JPEG_EVENT_BY_QUEUE)

        if (err == MMP_ERR_NONE) 
        {
            captureinfo.ubFilename      = pVRCaptInfo->bFileName;
            captureinfo.usFilenamelen   = pVRCaptInfo->ulFileNameLen;
            captureinfo.bFirstShot 		= MMP_TRUE;	
            captureinfo.ulExtraBufAddr	= 0;

            MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);
            
            if ((status_fw == VIDENC_FW_STATUS_START) ||
                (status_fw == VIDENC_FW_STATUS_PAUSE) ||
                (status_fw == VIDENC_FW_STATUS_RESUME)) {
                
                /* Inform encoder the reserved space is reduced */
                if ((err = MMPD_3GPMGR_MakeRoom(m_VidRecdID, ulJpegSize)) != MMP_ERR_NONE) {
                    RTNA_DBG_Str(0, "FrontCam MakeRoom Err:");
                    RTNA_DBG_Long(0, err);
                    RTNA_DBG_Str(0, "\r\n");
                    return MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE;
                }
            }

            if ((err = MMPS_DSC_JpegDram2Card(&captureinfo)) != MMP_ERR_NONE) {
                RTNA_DBG_Str(0, "FrontCam Save Card Err:");
                RTNA_DBG_Long(0, err);
                RTNA_DBG_Str(0, ", ");
                RTNA_DBG_Str(0, captureinfo.ubFilename);
                RTNA_DBG_Str(0, "\r\n");
                return MMP_3GPRECD_ERR_STILL_CAPTURE;
            }
        }
        else {
            RTNA_DBG_Str(0, "FrontCam Capture After SrcReady Err:");
            RTNA_DBG_Long(0, err);
            RTNA_DBG_Str(0, "\r\n");
            return MMP_3GPRECD_ERR_STILL_CAPTURE;
        }
    }

    if ((pVRCaptInfo->sCaptSrc & MMP_3GPRECD_CAPTURE_SRC_REAR_CAM) &&
        (m_VidRecdConfigs.bDualCaptureEnable))
    {
        if (CAM_CHECK_USB(USB_CAM_AIT)) {
 			ScalerSrc	= MMP_SCAL_SOURCE_ISP;       
            #if (AIT_REAR_CAM_VR_CAPTURE_EN)
            #if(SUPPORT_DEC_MJPEG_TO_PREVIEW)
            if (MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_MJPEG_H264) {
                
                MMP_ERR     sRet = MMP_ERR_NONE;
                MMP_BOOL    bStillJPGDoneFlag = 0;
                MMP_ULONG   ulTimeout = 2000; 
                MMP_ULONG   ulLineBuf, ulCompBuf, ulCompBufEnd;
                
                extern MMP_ERR MMPF_USBH_BulkIn_StartMJPG2VRCapture(void); //TBD

                MMPS_DSC_GetCaptureBuf(&ulLineBuf, &ulCompBuf, &ulCompBufEnd);
                
                MMPF_JPEGCTL_Buf2StillJpegSetCompressAddress(ulLineBuf, ulCompBuf, ulCompBufEnd);
                MMPF_JPEGCTL_Buf2StillJpegSetWidthHeight(m_usDecMjpegToPreviewSrcW, m_usDecMjpegToPreviewSrcH);
                MMPF_JPEGCTL_Buf2StillJpegDoneSetFlag(MMP_FALSE);
                
                sRet = MMPF_USBH_BulkIn_StartMJPG2VRCapture();
                
                do {
                    MMPF_OS_Sleep(1);
                    MMPF_JPEGCTL_Buf2StillJpegDoneGetFlag(&bStillJPGDoneFlag);   
                } while((MMP_FALSE == bStillJPGDoneFlag) && ((--ulTimeout) > 0));
                
                if (0 == ulTimeout) {
                    MMP_PRINT_RET_ERROR(0, sRet, "", gubMmpDbgBk);                                                                    
                }
                else {
                    MMP_ULONG ulJpegLineStart = 0, ulJpegCompStart = 0, ulJpegCompEnd = 0;
                    MMP_ULONG ulJpegSize = 0;

                    MMPF_JPEGCTL_Buf2StillJpegGetCompressAddress(&ulJpegLineStart, &ulJpegCompStart, &ulJpegCompEnd);            
                    MMPF_JPEGCTL_Buf2StillJpegGetSize(&ulJpegSize);

                    MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);
                    
                    if ((status_fw == VIDENC_FW_STATUS_START) ||
                        (status_fw == VIDENC_FW_STATUS_PAUSE) ||
                        (status_fw == VIDENC_FW_STATUS_RESUME)) {
                        
                        /* Inform encoder the reserved space is reduced */
                        if ((RearCamErr = MMPD_3GPMGR_MakeRoom(m_VidRecdID, ulJpegSize)) != MMP_ERR_NONE) {
                            RTNA_DBG_Str(0, "RearCam MakeRoom Err:");
                            RTNA_DBG_Long(0, RearCamErr);
                            RTNA_DBG_Str(0, "\r\n");
                            return MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE;
                        }
                    }

                    sRet = MMPD_DSC_SetFileName(pVRCaptInfo->bRearFileName, pVRCaptInfo->ulRearFileNameLen);
                    sRet |= MMPD_DSC_JpegDram2Card(ulJpegCompStart, ulJpegSize, MMP_TRUE, MMP_TRUE);              
                    if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk);}                                                                    
                }
            }
            else
            #endif 
            {
                // TBD
                RTNA_DBG_Str(0, FG_RED("AIT rear cam with NV12+H264 format does not support VR capture!\r\n"));
            }
            #endif
        }
        else if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264) || \
                 CAM_CHECK_USB(USB_CAM_SONIX_MJPEG)) {
	        ScalerSrc   = MMP_SCAL_SOURCE_JPG;
        
	        MEMCPY(pVRCaptInfo->bFileName, pVRCaptInfo->bRearFileName, pVRCaptInfo->ulFileNameLen);
	        pVRCaptInfo->ulFileNameLen = pVRCaptInfo->ulRearFileNameLen;

			// Check the pipe is alive or not 
			if (!m_bVidPreviewActive[0]) {
				return MMP_3GPRECD_ERR_STILL_CAPTURE;
			}

			if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE ||
				m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE) {
				ibc_pipe =m_2ndRecFctlLink.ibcpipeID;
			}
			else
			{
            	ibc_pipe =m_PreviewFctlLink.ibcpipeID;
			}
			
			MMPS_Sensor_GetCurPrevScalInputRes(1, (MMP_ULONG *)&ulInWidth, (MMP_ULONG *)&ulInHeight);

			fitrange.fitmode		= m_sAhcDecMjpegToEncodeInfo.sFitMode;
			fitrange.scalerType 	= MMP_SCAL_TYPE_SCALER;
			fitrange.ulInWidth		= m_usAhcDecMjpegToEncodeSrcW;
			fitrange.ulInHeight 	= m_usAhcDecMjpegToEncodeSrcH;
			fitrange.ulOutWidth 	= m_sAhcDecMjpegToEncodeInfo.ulVideoEncW;
			fitrange.ulOutHeight	= m_sAhcDecMjpegToEncodeInfo.ulVideoEncH;
			
			fitrange.ulInGrabX		= 1;
			fitrange.ulInGrabY		= 1;
			fitrange.ulInGrabW		= fitrange.ulInWidth;
			fitrange.ulInGrabH		= fitrange.ulInHeight;
			fitrange.ubChoseLit 	= 0;

			MMPD_Scaler_GetGCDBestFitScale(&fitrange, &grabctl);

#if (HANDLE_JPEG_EVENT_BY_QUEUE)
			if (MMPF_JPEG_GetCtrlByQueueEnable()) {
			
			#if (USE_H264_CUR_BUF_AS_CAPT_BUF)
				MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);
				
				if ((status_fw == VIDENC_FW_STATUS_START) ||
					(status_fw == VIDENC_FW_STATUS_PREENCODE)) {
					
					bRecording = MMP_TRUE;
				}
				else {
					bRecording = MMP_FALSE;
					  
					if (!m_bInitFcamRecPipeFrmBuf) {
						m_bInitFcamRecPipeFrmBuf = MMP_TRUE;
						MMPS_3GPRECD_PresetStillCaptureBuffer(&m_VidRecdInputBuf[0]);
					}
					#if 0
					MMPF_Display_GetCurIbcFrameBuffer(m_RecordFctlLink.ibcpipeID,
													  0/*ubUsageId*/,
													  &m_ulVidRecCaptureFrmBufAddr,
													  NULL,
													  NULL);
					#endif
				}
			#endif

				MMPD_System_EnableClock(MMPD_SYS_CLK_JPG, MMP_TRUE);
				
				err = MMPS_DSC_CaptureByQueue(ibc_pipe, 
											  &captureinfo,
											  ScalerSrc,
											  &fitrange, 
											  &grabctl,
											  &ulJpegSize,
											  ulDramAddr,
											  ulSramAddr,
											  m_ulVidRecCaptureFrmBufAddr,
											  pVRCaptInfo,
											  bRecording);
				MMPD_System_EnableClock(MMPD_SYS_CLK_JPG, MMP_FALSE);	 	
			}

#endif // (HANDLE_JPEG_EVENT_BY_QUEUE)

			//capture done, start to save JPG to card
			if (err == MMP_ERR_NONE) 
			{
				captureinfo.ubFilename		= pVRCaptInfo->bFileName;
				captureinfo.usFilenamelen	= pVRCaptInfo->ulFileNameLen;
				captureinfo.bFirstShot		= MMP_TRUE; 
				captureinfo.ulExtraBufAddr	= 0;
	
				MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);
				
				if ((status_fw == VIDENC_FW_STATUS_START) ||
					(status_fw == VIDENC_FW_STATUS_PAUSE) ||
					(status_fw == VIDENC_FW_STATUS_RESUME)) {
					
					/* Inform encoder the reserved space is reduced */
					if ((err = MMPD_3GPMGR_MakeRoom(m_VidRecdID, ulJpegSize)) != MMP_ERR_NONE) {
						RTNA_DBG_Str(0, "RearCam MakeRoom Err:");
						RTNA_DBG_Long(0, err);
						RTNA_DBG_Str(0, "\r\n");
						return MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE;
					}
				}
				if ((err = MMPS_DSC_JpegDram2Card(&captureinfo)) != MMP_ERR_NONE) {
					RTNA_DBG_Str(0, "RearCam Save Card Err:");
					RTNA_DBG_Long(0, err);
					RTNA_DBG_Str(0, ", ");
					RTNA_DBG_Str(0, captureinfo.ubFilename);
					RTNA_DBG_Str(0, "\r\n");
					return MMP_3GPRECD_ERR_STILL_CAPTURE;
				}
			}
			else {
				RTNA_DBG_Str(0, "RearCam Capture After SrcReady Err:");
				RTNA_DBG_Long(0, err);
				RTNA_DBG_Str(0, "\r\n");
				return MMP_3GPRECD_ERR_STILL_CAPTURE;
			}		
        }
        else if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) || 
                 (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)))
        {
            extern MMPS_DSC_MULTISHOT_INFO m_MultiShotInfo;
            MMP_ULONG ulCaptureWidth, ulCaptureHeight;
        
            ibc_pipe = m_2ndPrewFctlLink.ibcpipeID;

            MMPD_Scaler_GetGrabRange(m_2ndPrewFctlLink.scalerpath, MMP_SCAL_GRAB_STAGE_LPF, &ulInWidth, &ulInHeight);

            MMPS_3GPRECD_GetRecordPipeBufWH(MMPS_3GPRECD_FILESTREAM_DUAL, &ulCaptureWidth, &ulCaptureHeight);

            fitrange.fitmode        = MMP_SCAL_FITMODE_OUT;
            fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;
            fitrange.ulInWidth      = ulInWidth;
            fitrange.ulInHeight     = ulInHeight;
            fitrange.ulOutWidth	    = ulCaptureWidth; //ulInWidth;
            fitrange.ulOutHeight    = ulCaptureHeight; //ulInHeight;

            fitrange.ulInGrabX      = 1;
            fitrange.ulInGrabY      = 1;
            fitrange.ulInGrabW      = fitrange.ulInWidth;
            fitrange.ulInGrabH      = fitrange.ulInHeight;
            fitrange.ubChoseLit     = 0;

            MMPD_Scaler_GetGCDBestFitScale(&fitrange, &grabctl);

            pVRCaptInfo->usWidth    = ulCaptureWidth; //ulInWidth;
            pVRCaptInfo->usHeight   = ulCaptureHeight; //ulInHeight;

            MMPS_DSC_SetShotMode(MMPS_DSC_SINGLE_SHOTMODE);
            MMPS_DSC_SetJpegEncParam(DSC_RESOL_IDX_UNDEF, pVRCaptInfo->usWidth, pVRCaptInfo->usHeight, MMP_DSC_JPEG_RC_ID_CAPTURE);

            if((pVRCaptInfo->usWidth  % 16 != 0)||
               (pVRCaptInfo->usHeight %  8 != 0))
            {
                printc(FG_RED("ERROR!!!! MMPS_3GPRECD_StillCapture(): Capture width & height must be multiple of 16 & 8. (%d x %d)\r\n"), pVRCaptInfo->usWidth, pVRCaptInfo->usHeight);
            }
            
            MMPS_DSC_ConfigThumbnail(pVRCaptInfo->usThumbWidth, pVRCaptInfo->usThumbHeight, MMP_DSC_THUMB_SRC_NONE);
            MMPS_DSC_SetCaptureJpegQuality(MMP_DSC_JPEG_RC_ID_CAPTURE,
                                           pVRCaptInfo->bTargetCtl, pVRCaptInfo->bLimitCtl, pVRCaptInfo->bTargetSize, 
                                           pVRCaptInfo->bLimitSize, pVRCaptInfo->bMaxTrialCnt, pVRCaptInfo->Quality);
            
            MMPS_DSC_SetSystemBuf(&ulDramAddr, MMP_FALSE, MMP_TRUE, MMP_TRUE);
            MMPS_DSC_SetCaptureBuf(&ulSramAddr, &ulDramAddr, MMP_TRUE, MMP_FALSE, MMP_DSC_CAPTURE_NO_ROTATE, MMP_TRUE);

            captureinfo.bFirstShot 	= MMP_TRUE;
            captureinfo.bExif 	    = pVRCaptInfo->bExifEn;
            captureinfo.bThumbnail  = MMP_FALSE;

            ScalerSrc = MMP_SCAL_SOURCE_GRA;

            err = MMPS_DSC_CaptureAfterSrcReady(ibc_pipe, &captureinfo, ScalerSrc, &fitrange, &grabctl, &ulJpegSize, bDualCamCapture);
            
            if (err == MMP_ERR_NONE) {
                captureinfo.ubFilename      = pVRCaptInfo->bFileName;
                captureinfo.usFilenamelen   = pVRCaptInfo->ulFileNameLen;
                captureinfo.bFirstShot 		= MMP_TRUE;	
                captureinfo.ulExtraBufAddr	= 0;

                MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);
                
                if ((status_fw == VIDENC_FW_STATUS_START) ||
                    (status_fw == VIDENC_FW_STATUS_PAUSE) ||
                    (status_fw == VIDENC_FW_STATUS_RESUME)) {
                    
                    /* Inform encoder the reserved space is reduced */
                    if ((err = MMPD_3GPMGR_MakeRoom(m_VidRecdID, ulJpegSize)) != MMP_ERR_NONE) {
                        RTNA_DBG_Str(0, "FrontCam MakeRoom Err:");
                        RTNA_DBG_Long(0, err);
                        RTNA_DBG_Str(0, "\r\n");
                        return MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE;
                    }
                }
                 
                err = MMPD_DSC_SetFileName(pVRCaptInfo->bRearFileName, pVRCaptInfo->ulRearFileNameLen);
                err = MMPD_DSC_JpegDram2Card(m_MultiShotInfo.ulPrimaryJpegAddr[0], 
                                             m_MultiShotInfo.ulPrimaryJpegSize[0],
                                             MMP_TRUE, MMP_TRUE);
            }
            else {
                RTNA_DBG_Str(0, "FrontCam Capture After SrcReady Err:");
                RTNA_DBG_Long(0, err);
                RTNA_DBG_Str(0, "\r\n");
                return MMP_3GPRECD_ERR_STILL_CAPTURE;
            }
        }
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_EnableVRThumbnail
//  Description : 
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_EnableVRThumbnail(MMP_UBYTE ubEnable, MMP_UBYTE ubIsCreateJpg)
{
#if (SUPPORT_COMPONENT_FLOW_CTL)
    ubEnable = MMP_FALSE;
    ubIsCreateJpg = MMP_FALSE;
#endif
	return MMPD_VIDENC_EnableVrThumbnail(ubEnable, ubIsCreateJpg);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetVRThumbRingBufNum
//  Description : 
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetVRThumbRingBufNum(MMP_UBYTE ubRingBufNum)
{
    return MMPD_VIDENC_SetVrThumbRingBufNum(ubRingBufNum);
}

#if (SUPPORT_VR_THUMBNAIL)
//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetVRThumbJpgSize
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetVRThumbJpgSize(MMP_ULONG ulJpegW, MMP_ULONG ulJpegH)
{
    m_ulVRThumbWidth = ulJpegW;
    m_ulVRThumbHeight = ulJpegH;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetVRThumbJpgSize
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetVRThumbJpgSize(MMP_ULONG *pulJpegW, MMP_ULONG *pulJpegH)
{
    *pulJpegW = m_ulVRThumbWidth;
    *pulJpegH = m_ulVRThumbHeight;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetVRThumbnailSts
//  Description : 
//------------------------------------------------------------------------------
MMP_UBYTE MMPS_3GPRECD_GetVRThumbnailSts(void)
{
    return MMPD_VIDENC_GetVrThumbnailSts();
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StoreJpgThumb
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_StoreJpgThumb(MMP_BYTE ubFilename[], MMP_USHORT usLength, VIDENC_STREAMTYPE ulStreamType)
{
    MMP_ERR                 err = MMP_ERR_NONE;
    MMPS_DSC_CAPTURE_INFO 	captureinfo;

    if (!gubIsCreatJpgFile) {
        return MMP_ERR_NONE;
    }
    
    captureinfo.ubFilename      = (MMP_BYTE*)ubFilename;
    captureinfo.usFilenamelen   = usLength;
    captureinfo.bFirstShot      = MMP_TRUE;
    captureinfo.bExif           = MMP_FALSE;
    captureinfo.ulExtraBufAddr  = 0;
    
    if ((err = MMPS_DSC_ThumbDram2Card(&captureinfo, (MMP_ULONG)ulStreamType)) != MMP_ERR_NONE) {
        RTNA_DBG_Str0(FG_RED("VR Thumb Save Card Err:"));
        RTNA_DBG_Long0(err);
        RTNA_DBG_Str0(", ");
        RTNA_DBG_Str0(captureinfo.ubFilename);
        RTNA_DBG_Str0("\r\n");
        return MMP_3GPRECD_ERR_STILL_CAPTURE;
    }
    
    return MMP_ERR_NONE;
}
#endif

#if 0
void ____VR_Common_Record_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetPreviewPipe
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetEncodePipe(MMP_UBYTE ubSnrSel, MMP_UBYTE ubPipe)
{
    if (ubSnrSel == PRM_SENSOR) {
        FCTL_PIPE_TO_LINK(ubPipe, m_RecordFctlLink);
    }
    else if (ubSnrSel == SCD_SENSOR) {
        FCTL_PIPE_TO_LINK(ubPipe, m_2ndRecFctlLink);
    }
    #if(SUPPORT_DEC_MJPEG_TO_PREVIEW)
    else if (ubSnrSel == USBH_SENSOR) {
        FCTL_PIPE_TO_LINK(ubPipe, m_DecMjpegToEncFctlLink);
    }
    #endif
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_CustomedEncResol
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set VideoR encode resolution
 @param[in] ubEncIdx    encode instance ID
 @param[in] sFitMode  	scaler fit mode
 @param[in] usWidth  	encode width
 @param[in] usHeight 	encode height
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_CustomedEncResol(MMP_UBYTE ubEncIdx, MMP_UBYTE sFitMode, MMP_USHORT usWidth, MMP_USHORT usHeight)
{
    m_sAhcVideoRecdInfo[ubEncIdx].bUserDefine  	= MMP_TRUE;
    m_sAhcVideoRecdInfo[ubEncIdx].sFitMode		= sFitMode;
    m_sAhcVideoRecdInfo[ubEncIdx].ulVideoEncW  	= usWidth;
    m_sAhcVideoRecdInfo[ubEncIdx].ulVideoEncH 	= usHeight;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetEncodeRes
//  Description : This function need to called before MMPS_3GPRECD_RecordStart()
//------------------------------------------------------------------------------
static MMP_ERR MMPS_3GPRECD_SetEncodeRes(MMP_UBYTE ubEncIdx)
{
    MMP_ULONG 	ulScalInW, ulScalInH;
    MMP_ULONG   ulEncWidth, ulEncHeight;
    
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    if (ubEncIdx == MMPS_3GPRECD_FILESTREAM_NORMAL)
        MMPS_Sensor_GetCurPrevScalInputRes(m_ub1stVRStreamSnrId, &ulScalInW, &ulScalInH);
    #if (DUALENC_SUPPORT)
    else if (ubEncIdx == MMPS_3GPRECD_FILESTREAM_DUAL)
        MMPS_Sensor_GetCurPrevScalInputRes(m_ub2ndVRStreamSnrId, &ulScalInW, &ulScalInH);
    #endif
    else
        return MMP_ERR_NONE;

    /* Calculate encode parameters */
    if (m_sAhcVideoRecdInfo[ubEncIdx].bUserDefine) {
        ulEncWidth  = m_sAhcVideoRecdInfo[ubEncIdx].ulVideoEncW;
        ulEncHeight = m_sAhcVideoRecdInfo[ubEncIdx].ulVideoEncH;
    }
    else {
        ulEncWidth  = m_VidRecdConfigs.usEncWidth[m_VidRecdModes.usVideoEncResIdx[ubEncIdx]];
        ulEncHeight = m_VidRecdConfigs.usEncHeight[m_VidRecdModes.usVideoEncResIdx[ubEncIdx]];
    }

    m_ulVREncodeW[ubEncIdx] = ulEncWidth;
    m_ulVREncodeH[ubEncIdx] = ulEncHeight;

	if (ulEncWidth & 0x0F || ulEncHeight & 0x0F) {
		RTNA_DBG_Str0("The Encode Width/Height must be 16x\r\n");
	}

	// The RT mode need padding 8 lines and Frame/RT mode need set crop 8 line
    ulEncHeight = (ulEncWidth == 1920 && ulEncHeight == 1088) ? 1080 : ulEncHeight;
    ulEncHeight = (ulEncWidth == 1440 && ulEncHeight == 1088) ? 1080 : ulEncHeight;
    ulEncHeight = (ulEncWidth == 640 && ulEncHeight == 368) ? 360 : ulEncHeight;

    if ((ulEncWidth > ulScalInW) || (ulEncHeight > ulScalInH)) {
        RTNA_DBG_Str0("Check the input/output setting!\r\n");
    }

    m_ulVREncScalOutW[ubEncIdx] = ulEncWidth;
    m_ulVREncScalOutH[ubEncIdx] = ulEncHeight;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetRecordPipeStatus
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Return in video record status or not.
 @param[in]  ubEncIdx   encode instance ID.
 @param[out] bEnable record enable.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetRecordPipeStatus(MMP_UBYTE ubEncIdx, MMP_BOOL *bEnable)
{
    *bEnable = m_bVidRecordActive[ubEncIdx];
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetRecordPipeBufWH
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetRecordPipeBufWH(MMP_UBYTE ubEncIdx, MMP_ULONG *pulBufW, MMP_ULONG *pulBufH)
{
    if (m_sAhcVideoRecdInfo[ubEncIdx].bUserDefine) {
        *pulBufW = m_sAhcVideoRecdInfo[ubEncIdx].ulVideoEncW;
        *pulBufH = m_sAhcVideoRecdInfo[ubEncIdx].ulVideoEncH;
    }
    else {
        *pulBufW = m_VidRecdConfigs.usEncWidth[m_VidRecdModes.usVideoEncResIdx[ubEncIdx]];
        *pulBufH = m_VidRecdConfigs.usEncHeight[m_VidRecdModes.usVideoEncResIdx[ubEncIdx]];
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetRecordPipe
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetRecordPipe(MMP_UBYTE ubEncIdx, MMP_IBC_PIPEID *pPipe)
{
    if (ubEncIdx == 0)
        *pPipe = m_RecordFctlLink.ibcpipeID;
    else
        *pPipe = m_2ndRecFctlLink.ibcpipeID;
          
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetMVBufResIdx
//  Description : CHECK
//------------------------------------------------------------------------------
/**
 @brief Set MV buffer resolution.
 @param[in] usResol Resolution for record video.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetMVBufResIdx(MMP_USHORT usResol)
{
    m_VidRecdModes.usVideoMVBufResIdx = usResol;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_AllocateMVBuffer
//  Description : Reserve memory for Motion Vector.
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_AllocateMVBuffer(MMP_ULONG *ulCurBufPos, MMP_ULONG ulMVWidth, MMP_ULONG ulMVHeight)
{
    MMP_ULONG	ulCurAddr = *ulCurBufPos;
    MMP_ULONG   MVBufSize;
    
    ulMVWidth	= MAX(ulMVWidth, VR_MIN_MV_WIDTH);
    ulMVHeight	= MAX(ulMVHeight, VR_MIN_MV_HEIGHT);
    
    ulCurAddr = ALIGN64(*ulCurBufPos);
    
    // Reserve MV buffer, #MB/Frame * #MVs/MB * #byte/MV
    m_ulVidShareMvAddr  = ulCurAddr;
    
    MVBufSize = (ulMVWidth >> 4) * (ulMVHeight >> 4) * 16 * 4;
    ulCurAddr	+= MVBufSize;

    *ulCurBufPos = ulCurAddr;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_CalculteTargetFrmSize
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get record target frame size according to the frame rate & bitrate.
 @retval Record target frame size.
*/
static MMP_ULONG MMPS_3GPRECD_CalculteTargetFrmSize(MMP_ULONG ulEncId)
{
    MMP_ULONG64 ullBitrate64;
    MMP_ULONG64 ullTimeIncr64;
    MMP_ULONG64 ullTimeResol64;
    MMP_ULONG   ulTargetFrameSize;

    ullBitrate64        = m_VidRecdModes.ulBitrate[ulEncId];
    ullTimeIncr64       = m_VidRecdModes.ContainerFrameRate[ulEncId].usVopTimeIncrement;
    ullTimeResol64      = m_VidRecdModes.ContainerFrameRate[ulEncId].usVopTimeIncrResol;
    
    ulTargetFrameSize   = (MMP_ULONG)((ullBitrate64 * ullTimeIncr64)/(ullTimeResol64 * 8));

    return ulTargetFrameSize;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetExpectedRecordTime
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get the expected video encoding time in unit of second according to
        the specified storage space.
 @param[in] ulSpace the available free space
 @param[in] ulVidBitRate Video bitrate in bit per second.
 @param[in] ulAudBitRate Audio bitrate in bit per second (set to 0 if audio disabled).
 @retval the expected recording time, negative value indicates error.
*/
MMP_LONG MMPS_3GPRECD_GetExpectedRecordTime(MMP_ULONG64 ullSpace, MMP_ULONG ulVidBitRate, MMP_ULONG ulAudBitRate)
{
    MMP_LONG    lRecordSec;
    MMP_ULONG   ulContainerOverhead;

    if ((ulVidBitRate + ulAudBitRate) < 8)
        return 0;

    switch (m_VidRecdContainer) {
    case VIDMGR_CONTAINER_3GP:
    case VIDMGR_CONTAINER_NONE:
        ulContainerOverhead = 0; // TODO: set to 3GP header size
        break;
    case VIDMGR_CONTAINER_AVI:
        switch(MMPD_3GPMGR_GetAudioFormat()) {
        case MMPD_3GPMGR_AUDIO_FORMAT_AAC:
            ulContainerOverhead = AVI_AACLC_HEADERS_SIZE;
            break;
        case MMPD_3GPMGR_AUDIO_FORMAT_ADPCM:
            ulContainerOverhead = AVI_WAVE_HEADERS_SIZE;
            break;
        case MMPD_3GPMGR_AUDIO_FORMAT_MP3:
            ulContainerOverhead = AVI_MP3_HEADERS_SIZE;
            break;
        default:
            ulContainerOverhead = AVI_NO_AUD_HEADERS_SIZE;
            break;
        }
        break;
    default:
        return -1;
    }

    if (ullSpace <= ulContainerOverhead)
        return 0;
    else
        ullSpace -= ulContainerOverhead;

    lRecordSec = (MMP_LONG)(ullSpace / ((ulVidBitRate + ulAudBitRate) >> 3));

    if (lRecordSec < VR_MIN_TIME_TO_RECORD)
        return 0;

    return (lRecordSec - VR_MIN_TIME_TO_RECORD);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetEncResIdx
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set encoded resolution.
 @param[in] usResol Resolution for record video.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetEncResIdx(MMP_ULONG ulEncId, MMP_USHORT usResol)
{
    m_VidRecdModes.usVideoEncResIdx[ulEncId] = usResol;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetBitrate
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set the recording video bit-rate according to the frame rate info in
        container. i.e. set the video bit-rate for playback

 a. For normal speed recording case:
    The bit-rate settings are identical for recording & playback.

 b. For slow motion recording case:
    The bit-rate settings should be set based on the container fps.
    e.g. Sensor inputs 60fps, and encode frame rate is also 60fps, but
           the container frame rate is set as 30fps to achieve 2x slow motion.
           For normal recording, we may set bit-rate to 16Mb for 720p 60fps;
           for 2x slow motion recording, please set bit-rate as 8Mb to achieve
           the same video quality.

 c. For fast action/time-lapse recording case:
    The bit-rate settings should be set also based on the container fps.
    e.g. Sensor inputs 60fps, and encode frame rate is set to 30fps only,
           the container frame rate is set as 60fps to achieve 2x fast action.
           For normal recording, we may set bit-rate to 8Mb for 720p 30fps;
           for 2x fast action recording, please set bit-rate as 16Mb to achieve
           the same video quality.

 @param[in] ulBitrate Bit per second of the video bitstream in recorded file.
 @retval MMP_ERR_NONE Success.
 @note It must be set after choosing resolution and format.
*/
MMP_ERR MMPS_3GPRECD_SetBitrate(MMP_ULONG ulEncId, MMP_ULONG ulBitrate)
{
    m_VidRecdModes.ulBitrate[ulEncId] = ulBitrate;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetFrameRatePara
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set frame rate for recorded video.
 @param[in] snr_fps Sensor input frame rate
 @param[in] enc_fps expected encode frame rate (normal/timelapse/slow motion)
 @param[in] container_fps Frame rate for playback
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetFrameRatePara(MMP_ULONG                 ulEncId,
                                      MMPS_3GPRECD_FRAMERATE    *snr_fps,
                                      MMPS_3GPRECD_FRAMERATE    *enc_fps,
                                      MMPS_3GPRECD_FRAMERATE    *container_fps)
{
    if ((snr_fps->usVopTimeIncrement == 0) ||
        (snr_fps->usVopTimeIncrResol == 0) ||
        (enc_fps->usVopTimeIncrement == 0) ||
        (enc_fps->usVopTimeIncrResol == 0) ||
        (container_fps->usVopTimeIncrement == 0) ||
        (container_fps->usVopTimeIncrResol == 0)) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }
    
    m_VidRecdModes.SnrInputFrameRate[ulEncId].usVopTimeIncrement = snr_fps->usVopTimeIncrement;
    m_VidRecdModes.SnrInputFrameRate[ulEncId].usVopTimeIncrResol = snr_fps->usVopTimeIncrResol;
    
    m_VidRecdModes.VideoEncFrameRate[ulEncId].usVopTimeIncrement = enc_fps->usVopTimeIncrement;
    m_VidRecdModes.VideoEncFrameRate[ulEncId].usVopTimeIncrResol = enc_fps->usVopTimeIncrResol;
    
    m_VidRecdModes.ContainerFrameRate[ulEncId].usVopTimeIncrement = container_fps->usVopTimeIncrement;
    m_VidRecdModes.ContainerFrameRate[ulEncId].usVopTimeIncrResol = container_fps->usVopTimeIncrResol;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetFrameRatePara
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get frame rate for recorded video.
 @param[in] snr_fps Sensor input frame rate
 @param[in] enc_fps expected encode frame rate (normal/timelapse/slow motion)
 @param[in] container_fps Frame rate for playback
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetFrameRatePara(MMP_ULONG                 ulEncId,
                                      MMPS_3GPRECD_FRAMERATE    *snr_fps,
                                      MMPS_3GPRECD_FRAMERATE    *enc_fps,
                                      MMPS_3GPRECD_FRAMERATE    *container_fps)
{       
    snr_fps->usVopTimeIncrement = m_VidRecdModes.SnrInputFrameRate[ulEncId].usVopTimeIncrement;
    snr_fps->usVopTimeIncrResol = m_VidRecdModes.SnrInputFrameRate[ulEncId].usVopTimeIncrResol;
    
    enc_fps->usVopTimeIncrement = m_VidRecdModes.VideoEncFrameRate[ulEncId].usVopTimeIncrement;
    enc_fps->usVopTimeIncrResol = m_VidRecdModes.VideoEncFrameRate[ulEncId].usVopTimeIncrResol;
    
    container_fps->usVopTimeIncrement = m_VidRecdModes.ContainerFrameRate[ulEncId].usVopTimeIncrement;
    container_fps->usVopTimeIncrResol = m_VidRecdModes.ContainerFrameRate[ulEncId].usVopTimeIncrResol;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetPFrameCount
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set P frame count of one cycle.
 @param[in] usFrameCount Count of P frame.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetPFrameCount(MMP_ULONG ulEncId, MMP_USHORT usFrameCount)
{
    m_VidRecdModes.usPFrameCount[ulEncId] = usFrameCount;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetBFrameCount
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set B frame count per P frame.
 @param[in] usFrameCount Count of B frame.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetBFrameCount(MMP_ULONG ulEncId, MMP_USHORT usFrameCount)
{
    if (usFrameCount) {
        RTNA_DBG_Str(0, "MMPS_3GPRECD_SetBFrameCount Err\r\n");
        // For single current frame mode, B-frame enc is not supported
        return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
    }

    m_VidRecdModes.usBFrameCount[ulEncId] = usFrameCount;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetProfile
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set profile for video encode.
 @param[in] profile Visual profile for the specified encoder.
 @retval none.
*/
MMP_ERR MMPS_3GPRECD_SetProfile(MMP_ULONG ulEncId, VIDENC_PROFILE profile)
{
    m_VidRecdModes.VisualProfile[ulEncId] = profile;
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetCurBufMode
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set encoded video current buffer mode.
 @param[in] CurBufMode Video encode current buffer mode for record video.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetCurBufMode(MMP_ULONG ulEncId, VIDENC_CURBUF_MODE CurBufMode)
{
    MMP_ULONG           ulVidNumOpening = 0;
    VIDENC_FW_STATUS    status_fw;
    MMP_ULONG           ulInstId;
    
    MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);

    if (ulEncId == MMPS_3GPRECD_FILESTREAM_NORMAL)
        ulInstId = m_VidRecdID;
    else
        ulInstId = m_VidDualID;
    
    if (ulInstId == INVALID_ENC_ID)
        status_fw = VIDENC_FW_STATUS_NONE;
    else
        MMPD_VIDENC_GetStatus(ulInstId, &status_fw);

    if ((ulVidNumOpening == VIDENC_MAX_STREAM_NUM) ||
        (status_fw == VIDENC_FW_STATUS_START) ||
        (status_fw == VIDENC_FW_STATUS_PAUSE) ||
        (status_fw == VIDENC_FW_STATUS_RESUME)) {
        RTNA_DBG_Str0("Can't change CurBufMode when recording\r\n");
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }
    
    m_VidRecdModes.VidCurBufMode[ulEncId] = CurBufMode;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetContainerType
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetContainerType(VIDMGR_CONTAINER_TYPE type)
{
    switch (type) {
    case VIDMGR_CONTAINER_3GP:
        m_VidRecdContainer = VIDMGR_CONTAINER_3GP;
        MMPD_3GPMGR_SetContainerType(VIDMGR_CONTAINER_3GP);
        break;
    case VIDMGR_CONTAINER_AVI:
        m_VidRecdContainer = VIDMGR_CONTAINER_AVI;
        MMPD_3GPMGR_SetContainerType(VIDMGR_CONTAINER_AVI);
        break;
    case VIDMGR_CONTAINER_NONE:
        m_VidRecdContainer = VIDMGR_CONTAINER_NONE;
        MMPD_3GPMGR_SetContainerType(VIDMGR_CONTAINER_NONE);
        break;    
    default:
        m_VidRecdContainer = VIDMGR_CONTAINER_UNKNOWN;
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetFileName
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Send encoded 3GP file name to firmware for card mode and creat file time.
 @param[in] bFilename File name.
 @param[in] usLength Length of file name in unit of byte.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetFileName(VIDENC_STREAMTYPE usStreamType, MMP_BYTE bFilename[], MMP_USHORT usLength)
{
    MMP_ULONG ulFileNameAddr = 0;
    MMP_ULONG ulFileNameSize = 0;
    MMP_ULONG ulCurAddr = m_ulVideoPreviewEndAddr;
    
    // Allocate memory for File name buffer
    MMPD_3GPMGR_GetTempFileNameAddr(&ulFileNameAddr, &ulFileNameSize);

    if (ulFileNameAddr == 0) {
        RTNA_DBG_Str(0, "Allocate FileName Buffer First!\r\n");
        
        MMPD_3GPMGR_SetTempFileNameAddr(ulCurAddr, MAX_3GPRECD_FILENAME_LENGTH);

        ulCurAddr += MAX_3GPRECD_FILENAME_LENGTH;

        m_ulVideoPreviewEndAddr = ulCurAddr;
    }

    MMPS_FS_SetCreationTime();
    MMPD_3GPMGR_SetFileName(usStreamType, bFilename, usLength);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetUserDataAtom
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Send user data atom to firmware for card mode.
 @param[in] AtomName atom name, maximum 4 bytes.
 @param[in] UserDataBuf user data atom buffer.
 @param[in] UserDataLength Length of user data atom.
 @param[in] usStreamType file type.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetUserDataAtom(VIDENC_STREAMTYPE usStreamType, MMP_BYTE AtomName[],MMP_BYTE UserDataBuf[], MMP_USHORT UserDataLength)
{
    return MMPD_3GPMGR_SetUserDataAtom(usStreamType, AtomName, UserDataBuf, UserDataLength);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetStoragePath
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Save encoded 3GP file to card mode or memory mode.
 @param[in] SrcMode Card or memory mode.
 @retval MMP_ERR_NONE Success.
 @note
 The parameter @a SrcMode can be:
 - VIDENC_SRCMODE_MEM : save file to memory.
 - VIDENC_SRCMODE_CARD : save file to storage.
*/
MMP_ERR MMPS_3GPRECD_SetStoragePath(VIDENC_SRCMODE SrcMode)
{
    m_VidRecdModes.VideoSrcMode[0] = SrcMode;
    
    MMPD_3GPMGR_SetStoragePath((MMP_UBYTE)m_VidRecdModes.VideoSrcMode[0]);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetReservedStorageSpace
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set reserved storage space which recorder or other system can not use it
        during recording. Ex. if user wants to reserve 10MB while 100MB free space,
        then total 90MB remained for recording & capture in recording to share.
 @pre MMPS_3GPRECD_SetStoragePath(VIDENC_SRCMODE_CARD) was called, for card mode only
 @param[in] ulReservedSize Reserved stroage space.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetReservedStorageSpace(MMP_ULONG ulReservedSize)
{
    if (m_VidRecdModes.VideoSrcMode[0] == VIDENC_SRCMODE_CARD){
        m_VidRecdModes.ulReservedSpace = ulReservedSize;
    }
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetMaxPreEncDuration
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get the max pre-encode duration in MS.
 It returns the max pre-encode duration allowed
*/
MMP_ULONG MMPS_3GPRECD_GetMaxPreEncDuration(void)
{
    MMP_ULONG64 ullBitrate64        = m_VidRecdModes.ulBitrate[0];
    MMP_ULONG64 ullTimeIncr64       = m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrement;
    MMP_ULONG64 ullTimeResol64      = m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrResol;
    MMP_ULONG   ulTargetFrameSize   = (MMP_ULONG)((ullBitrate64 * ullTimeIncr64)/(ullTimeResol64 * 8));
    MMP_ULONG   ulExpectBufSize     = 0;
    MMP_ULONG   ulMaxPreEncMs       = 0;

    ulExpectBufSize = m_VidRecdConfigs.ulVideoCompBufSize - (ulTargetFrameSize * 3);
    ulMaxPreEncMs = ((ulExpectBufSize) * 800) / (m_VidRecdModes.ulBitrate[0] >> 3);
    
    return ulMaxPreEncMs;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetMaxFileSizeTimeLimit
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get the maximum 3GP file time and size limit for video encoding.
 @param[in] ulFileLimit Maximum file size.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetMaxFileSizeTimeLimit(MMP_UBYTE ubEncIdx, MMP_ULONG64 *ulMaxSizeLimit, MMP_ULONG64 *ulMaxTimeLimit)
{
    MMP_ULONG       audSampleFreq;
    MMP_ULONG       vidByteSec, audByteSec, totalByteSec;
    MMP_ULONG64     totalsec = 0, totalsize = 0;

    MMPD_3GPMGR_GetAudioParam(ubEncIdx, &audSampleFreq);

    vidByteSec = 8 * ((m_VidRecdModes.ContainerFrameRate[ubEncIdx].usVopTimeIncrResol / m_VidRecdModes.ContainerFrameRate[ubEncIdx].usVopTimeIncrement) + 1);
    audByteSec = 4 * (audSampleFreq / 1024);

    totalByteSec = vidByteSec + audByteSec;
    totalsec = (m_VidRecdConfigs.ulTailBufSize / totalByteSec) - 1;

    *ulMaxTimeLimit = totalsec * 1000;   //ms
    
    totalsize = totalsec * m_VidRecdModes.ulBitrate[ubEncIdx];
    totalsize = totalsize >> 3;
    
    *ulMaxSizeLimit = totalsize;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetFileSizeLimit
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set the maximum 3GP file size for video encoding.
 @param[in] ulFileLimit Maximum file size.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetFileSizeLimit(MMP_ULONG ulSizeLimit)
{
    MMP_UBYTE ubEncIdx = 0;
    MMP_ULONG64 ulMaxTimeLimit = 0, ulMaxSizeLimit = 0;

    if (m_VidRecdID != INVALID_ENC_ID)
        MMPS_3GPRECD_GetMaxFileSizeTimeLimit(m_VidRecdID, &ulMaxSizeLimit, &ulMaxTimeLimit);
    else
        MMPS_3GPRECD_GetMaxFileSizeTimeLimit(ubEncIdx, &ulMaxSizeLimit, &ulMaxTimeLimit);

    if ((ulMaxSizeLimit > 2*1024*1024) && (ulMaxSizeLimit < ulSizeLimit)) //one clip at least 2 MB
    {
        m_VidRecdModes.ulSizeLimit = ulMaxSizeLimit;
        RTNA_DBG_Str(0, "Max Size: ");
        RTNA_DBG_Long(0, ulMaxSizeLimit);
        RTNA_DBG_Str(0, " bytes\r\n");
    }
    else {
        m_VidRecdModes.ulSizeLimit = ulSizeLimit;
    }
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetFileTimeLimit
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set the maximum 3GP file time for video encoding.
 @param[in] ulTimeLimitMs Maximum file time in unit of ms.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE Space not enough.

 @warn If ulTimeLimitMs is set to 0, then it doesn't take any effect.
*/
MMP_ERR MMPS_3GPRECD_SetFileTimeLimit(MMP_ULONG ulTimeLimitMs)
{
    if (ulTimeLimitMs) {
        m_VidRecdModes.ulTimeLimitMs = ulTimeLimitMs;
        return MMPD_3GPMGR_SetTimeLimit(ulTimeLimitMs);
    }

    return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_ChangeCurFileTimeLimit
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Change the maximum 3GP file time for current video encoding .
 @param[in] ulTimeLimitMs Maximum file time in unit of ms.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE Space not enough.
*/
MMP_ERR MMPS_3GPRECD_ChangeCurFileTimeLimit(MMP_ULONG ulTimeLimitMs)
{
    if (ulTimeLimitMs) {
        return MMPD_3GPMGR_SetTimeDynamicLimit(ulTimeLimitMs);
    }
    
    return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_Set3GPCreateModifyTimeInfo
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set creation time and modification time of 3GP file.
 @param[in] addr address of temp buffer.
 @param[in] size size of temp buffer.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_Set3GPCreateModifyTimeInfo(VIDENC_STREAMTYPE usStreamType, AUTL_DATETIME datetimenew)
{
    MMP_ULONG       CreateTime, ModifyTime;
    MMP_ULONG       ulseconds = 0; 
    AUTL_DATETIME   datetime = {2015, 3, 3, 2, 0, 0, 0, 0, 0};
    
    datetime.usYear     = 1904;
    datetime.usMonth    = 1;
    datetime.usDay      = 1;
    datetime.usHour     = 0;
    datetime.usMinute   = 0;
    datetime.usSecond   = 0;
    
    ulseconds  = AUTL_Calendar_DateToSeconds(&datetimenew, &datetime);
    CreateTime = ulseconds;
    ModifyTime = ulseconds; 
    
    RTNA_DBG_Str(0, "create & modify time:");
    RTNA_DBG_Long(0, ulseconds);
    RTNA_DBG_Str(0, "\r\n");
    
    MMPD_3GPMGR_Set3GPCreateModifyTimeInfo(usStreamType, CreateTime, ModifyTime);
    
    return MMP_ERR_NONE;  
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetEncodeCompBuf
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get start address and size of firmware compressed buffer.
 @param[in]  usStreamType: file type
 @param[out] *bufaddr : compress buffer address
 @param[out] *bufsize : compress buffer size
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetEncodeCompBuf(VIDENC_STREAMTYPE usStreamType, MMP_ULONG *bufaddr, MMP_ULONG *bufsize)
{
    return MMPD_3GPMGR_GetEncodeCompBuf(usStreamType, bufaddr, bufsize);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_UpdateParameter
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Update record parameter dynamically. This function should not be used if
        recording is not yet started.
 @param[in] type    the specific parameter type.
 @param[in] param   Parameter value to update.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_UpdateParameter(MMP_ULONG ulStreamType, MMPS_3GPRECD_PARAMETER type, void *param)
{
    MMP_ULONG               ulEncId;
    MMP_ERR                 err;
    VIDENC_FW_STATUS        status_vid;
    MMP_ULONG				ulBitrate;
    MMPS_3GPRECD_GOP		*gop;
    MMPS_3GPRECD_FRAMERATE  *fps;

    if (ulStreamType == VIDENC_STREAMTYPE_VIDRECD) {
        if (m_VidRecdID == INVALID_ENC_ID)
            status_vid = VIDENC_FW_STATUS_NONE;
        else
            MMPD_VIDENC_GetStatus(m_VidRecdID, &status_vid);

        ulEncId = m_VidRecdID;
    }
    #if(DUALENC_SUPPORT)
    else if (ulStreamType == VIDENC_STREAMTYPE_DUALENC) {
        if (m_VidDualID == INVALID_ENC_ID)
            status_vid = VIDENC_FW_STATUS_NONE;
        else
            MMPD_VIDENC_GetStatus(m_VidDualID, &status_vid);

        ulEncId = m_VidDualID;
    }
    #endif
    #if (SUPPORT_H264_WIFI_STREAM)
    else if (ulStreamType == VIDENC_STREAMTYPE_WIFIFRONT) {
        if (m_ulH264WifiFrontID == INVALID_ENC_ID)
            status_vid = VIDENC_FW_STATUS_NONE;
        else
            MMPD_VIDENC_GetStatus(m_ulH264WifiFrontID, &status_vid);

        ulEncId = m_ulH264WifiFrontID;
    }
    else if (ulStreamType == VIDENC_STREAMTYPE_WIFIREAR) {
        if (m_ulH264WifiRearID == INVALID_ENC_ID)
            status_vid = VIDENC_FW_STATUS_NONE;
        else
            MMPD_VIDENC_GetStatus(m_ulH264WifiRearID, &status_vid);

        ulEncId = m_ulH264WifiRearID;
    }
    #endif

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP)) {
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }

    switch (type) {
    case MMPS_3GPRECD_PARAMETER_BITRATE:
        if ((MMP_ULONG)param == 0) {
            return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
        }
        
        if (ulStreamType == VIDENC_STREAMTYPE_VIDRECD) {
            m_VidRecdModes.ulBitrate[0] = (MMP_ULONG)param;

            ulBitrate = m_VidRecdModes.ulBitrate[0];
        }
        #if (DUALENC_SUPPORT)
        else if (ulStreamType == VIDENC_STREAMTYPE_DUALENC) {
            m_VidRecdModes.ulBitrate[1] = (MMP_ULONG)param;

            ulBitrate = m_VidRecdModes.ulBitrate[1];
        }
        #endif
        #if (SUPPORT_H264_WIFI_STREAM)
        else if (ulStreamType == VIDENC_STREAMTYPE_WIFIFRONT) {
            m_sH264WifiStreamObj[0].WifiEncModes.ulBitrate = (MMP_ULONG)param;

            ulBitrate = m_sH264WifiStreamObj[0].WifiEncModes.ulBitrate;
        }
        else if (ulStreamType == VIDENC_STREAMTYPE_WIFIREAR) {
            m_sH264WifiStreamObj[1].WifiEncModes.ulBitrate = (MMP_ULONG)param;

            ulBitrate = m_sH264WifiStreamObj[1].WifiEncModes.ulBitrate;
        }
        #endif
        
        err = MMPD_VIDENC_SetBitrate(ulEncId, ulBitrate);
        break;
    case MMPS_3GPRECD_PARAMETER_FRAME_RATE:
        fps = (MMPS_3GPRECD_FRAMERATE *)param;
        
        if ((fps->usVopTimeIncrement == 0) || (fps->usVopTimeIncrResol == 0)) {
            return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
        }
        
        // Update encode frame rate
        err = MMPD_VIDENC_UpdateEncFrameRate(ulEncId,
                                             fps->usVopTimeIncrement, 
                                             fps->usVopTimeIncrResol);

        if (ulStreamType == VIDENC_STREAMTYPE_VIDRECD) {
            m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrement = fps->usVopTimeIncrement;
            m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrResol = fps->usVopTimeIncrResol;
        }
        #if (DUALENC_SUPPORT) && (SUPPORT_SHARE_REC == 0)
        else if (ulStreamType == VIDENC_STREAMTYPE_DUALENC) {
            m_VidRecdModes.VideoEncFrameRate[1].usVopTimeIncrement = fps->usVopTimeIncrement;
            m_VidRecdModes.VideoEncFrameRate[1].usVopTimeIncrResol = fps->usVopTimeIncrResol;
        }
        #endif
        #if (SUPPORT_H264_WIFI_STREAM)
        else if (ulStreamType == VIDENC_STREAMTYPE_WIFIFRONT) {
            m_sH264WifiStreamObj[0].WifiEncModes.VideoEncFrameRate.usVopTimeIncrement = fps->usVopTimeIncrement;
            m_sH264WifiStreamObj[0].WifiEncModes.VideoEncFrameRate.usVopTimeIncrResol = fps->usVopTimeIncrResol;
        }
        else if (ulStreamType == VIDENC_STREAMTYPE_WIFIREAR) {
            m_sH264WifiStreamObj[1].WifiEncModes.VideoEncFrameRate.usVopTimeIncrement = fps->usVopTimeIncrement;
            m_sH264WifiStreamObj[1].WifiEncModes.VideoEncFrameRate.usVopTimeIncrResol = fps->usVopTimeIncrResol;
        }
        #endif
        break;
    case MMPS_3GPRECD_PARAMETER_GOP:
        gop = (MMPS_3GPRECD_GOP *)param;
        err = MMPD_VIDENC_SetGOP(ulEncId, gop->usPFrameCount, gop->usBFrameCount);
        break;
    default:
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetRecordSpeed
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetRecordSpeed(VIDENC_SPEED_MODE SpeedMode, VIDENC_SPEED_RATIO SpeedRatio)
{
    if (SpeedMode == VIDENC_SPEED_SLOW) {
        PRINTF("Not support H264 slow record mode !\r\n");
        return MMPD_3GPMGR_SetRecordSpeed(VIDENC_SPEED_NORMAL, VIDENC_SPEED_1X);
    }
    
    return MMPD_3GPMGR_SetRecordSpeed(SpeedMode, SpeedRatio);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StartSeamless
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Start video seamless recording, 
        can be called before or after @ref MMPS_3GPRECD_StartRecord
 @param[in] bStart start or stop
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_StartSeamless(MMP_BOOL bStart)
{
    if (m_VidRecdConfigs.bSeamlessMode == MMP_TRUE) {
        m_bSeamlessEnabled = bStart;
        return MMPD_3GPMGR_SetSeamless(bStart);
    }

    return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_RegisterCallback
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set callbacks for recording events

The function sets callbacks for the following events.
    * media full  : record stopped due to card full (Task)
    * file full   : record stopped due to the specified time/space limitation (Task)
    * media slow  : record stopped due to the insufficient speed of media (ISR)
    * media error : record stopped due to some error in media writting (Task)
    * seamless    : to trigger the next recording in seamless mode (Task)
    * encode start: encoder started (Task/ISR)
    * encode stop : encoder stopped (ISR)
    * postprocess : for appending the user data in the file tail (Task)

@param[in] Event    Specified the event for register.
@param[in] CallBack The callback to be executed when the specified event happens.

@return It reports the status of the operation.

@warn The registered callback will not be auto deleted, please register a NULL
      to delete it.
*/
MMP_ERR MMPS_3GPRECD_RegisterCallback(VIDMGR_EVENT Event, void *CallBack)
{
    return MMPD_3GPMGR_RegisterCallback(Event, CallBack);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetSkipCntThreshold
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set the threshold of skip frame counts.

The function specified the threshold of skip frame counts to define what called
a slow media.

@param[in] threshold number of skip frames happen in 1 sec represents a slow media
@return It reports the status of the operation.
*/
MMP_ERR MMPS_3GPRECD_SetSkipCntThreshold(MMP_USHORT threshold)
{
    return MMPD_3GPMGR_SetSkipCntThreshold(threshold);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetVidRecdSkipModeParam
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Enalbe video record skip mode to set total skip count and continuous skip count.

 @param[in] ulTotalCount  - limitation of total skip count
 @param[in] ulContinCount - limitation of continuous skip count
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_SetVidRecdSkipModeParam(MMP_ULONG ulTotalCount, MMP_ULONG ulContinCount)
{
    return MMPD_3GPMGR_SetVidRecdSkipModeParas(ulTotalCount, ulContinCount);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetH264EncUseMode
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetH264EncUseMode(VIDENC_STREAMTYPE usStreamType, MMP_ULONG type)
{
    MMPD_3GPMGR_SetH264EnableEncMode(usStreamType ,type);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetMuxer3gpConstantFps
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetMuxer3gpConstantFps(MMP_BOOL bEnable)
{
    m_bMuxer3gpConstantFps = bEnable;
    MMPD_3GPMGR_SetMuxer3gpConstantFps(bEnable);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetAVSyncMethod
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetAVSyncMethod(VIDMGR_AVSYNC_METHOD usAVSyncMethod)
{
    MMPD_3GPMGR_SetAVSyncMethod(usAVSyncMethod);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetAVSyncMethod
//  Description :
//------------------------------------------------------------------------------
VIDMGR_AVSYNC_METHOD MMPS_3GPRECD_GetAVSyncMethod(void)
{
    return MMPD_3GPMGR_GetAVSyncMethod();    
}

#if 0
void ____VR_1st_Record_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetRecdPipeConfig
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set video record pipe configuration.
 @param[in] *pInputBuf 		Pointer to HW used buffer.
 @param[in] usEncInputW 	The encode buffer width (for scaler out stage).
 @param[in] usEncInputH 	The encode buffer height (for scaler out stage).
 @param[in] ubEncId         encode instance ID
 @retval MMP_ERR_NONE Success.
*/
static MMP_ERR MMPS_3GPRECD_SetRecdPipeConfig(VIDENC_INPUT_BUF 	    *pInputBuf,
											  MMP_USHORT		    usEncInputW,
											  MMP_USHORT		    usEncInputH,
											  MMP_USHORT            ubEncId)
{
    MMP_ULONG				ulScalInW, ulScalInH;
    MMP_SCAL_FIT_MODE		sFitMode;
    MMP_SCAL_FIT_RANGE  	fitrange;
    MMP_SCAL_GRAB_CTRL  	EncodeGrabctl;
    MMPD_FCTL_ATTR 			fctlAttr;
    MMP_USHORT				i;
	MMP_SCAL_FIT_RANGE 		sFitRangeBayer;
	MMP_SCAL_GRAB_CTRL		sGrabctlBayer;
	MMP_USHORT				usCurZoomStep = 0;

	/* Parameter Check */
	if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID) {
		return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
	}

	if (pInputBuf == NULL) {
		return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
	}

	/* Get the sensor parameters */
    MMPS_Sensor_GetCurPrevScalInputRes(m_ub1stVRStreamSnrId, &ulScalInW, &ulScalInH);

	MMPD_BayerScaler_GetZoomInfo(MMP_BAYER_SCAL_DOWN, &sFitRangeBayer, &sGrabctlBayer); 

    if (m_sAhcVideoRecdInfo[0].bUserDefine) {
    	sFitMode = m_sAhcVideoRecdInfo[0].sFitMode;
    }
    else {
    	sFitMode = MMP_SCAL_FITMODE_OUT;
    }

	/* Initial zoom relative config */ 
	MMPS_3GPRECD_InitDigitalZoomParam(m_RecordFctlLink.scalerpath);

	MMPS_3GPRECD_RestoreDigitalZoomRange(m_RecordFctlLink.scalerpath);

    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE)
    {
        // Config Video Record Pipe
        fitrange.fitmode        = sFitMode;
        fitrange.scalerType 	= MMP_SCAL_TYPE_SCALER;

        if (MMP_IsVidPtzEnable()) {
            fitrange.ulInWidth	= sFitRangeBayer.ulOutWidth;
            fitrange.ulInHeight	= sFitRangeBayer.ulOutHeight;
        }
        else {
            fitrange.ulInWidth 	= ulScalInW;
            fitrange.ulInHeight	= ulScalInH;
        }

        if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
            if (MMP_GetDualSnrEncodeType() == DUALSNR_DUAL_ENCODE) {
                fitrange.ulInWidth 	    = ulScalInW;
                fitrange.ulInHeight	    = ulScalInH;
                fitrange.ulOutWidth     = usEncInputW;
                fitrange.ulOutHeight    = usEncInputH;
            }
            else {
                fitrange.ulInWidth 	    = ulScalInW;
                fitrange.ulInHeight	    = ulScalInH;
                fitrange.ulOutWidth     = usEncInputW / 2;
                fitrange.ulOutHeight    = usEncInputH;
            }
        }
        else {
            fitrange.ulOutWidth     = usEncInputW;
            fitrange.ulOutHeight    = usEncInputH;

            if (usEncInputW == 1920 && usEncInputH == 1088) {
                fitrange.ulOutHeight = 1080;
            }
            else if (usEncInputW == 1600 && usEncInputH == 912) {
                fitrange.ulOutHeight = 904;
            }
        }
        
        fitrange.ulInGrabX      = 1;
        fitrange.ulInGrabY      = 1;
        fitrange.ulInGrabW      = fitrange.ulInWidth;
        fitrange.ulInGrabH      = fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        
        MMPD_PTZ_InitPtzInfo(m_RecordFctlLink.scalerpath,
                             fitrange.fitmode,
                             fitrange.ulInWidth, fitrange.ulInHeight,
                             fitrange.ulOutWidth, fitrange.ulOutHeight);
        
        // Be sync with preview path : TBD
        MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);

        if (usCurZoomStep > m_VidRecordZoomInfo.usMaxZoomSteps) {
            usCurZoomStep = m_VidRecordZoomInfo.usMaxZoomSteps;
        }
        
        MMPD_PTZ_CalculatePtzInfo(m_RecordFctlLink.scalerpath, usCurZoomStep, 0, 0);

        MMPD_PTZ_GetCurPtzInfo(m_RecordFctlLink.scalerpath, &fitrange, &EncodeGrabctl);
        
        if (MMP_IsVidPtzEnable()) {
            MMPD_PTZ_ReCalculateGrabRange(&fitrange, &EncodeGrabctl);
        }
        
        fctlAttr.colormode          = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
        fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
        fctlAttr.fctllink           = m_RecordFctlLink;
        fctlAttr.fitrange           = fitrange;
        fctlAttr.grabctl            = EncodeGrabctl;
        fctlAttr.scalsrc            = MMP_SCAL_SOURCE_ISP;
        fctlAttr.bSetScalerSrc      = MMP_TRUE;
        fctlAttr.ubPipeLinkedSnr    = PRM_SENSOR;
        fctlAttr.usBufCnt           = pInputBuf->ulBufCnt;
        fctlAttr.bUseRotateDMA      = MMP_FALSE;

        if (CAM_CHECK_PRM(PRM_CAM_TV_DECODER)) {
            if (m_VidRecdConfigs.bRawPreviewEnable[0] == MMP_TRUE) {
                fctlAttr.scalsrc		= MMP_SCAL_SOURCE_GRA;
            }	
        }

        for (i = 0; i < fctlAttr.usBufCnt; i++) {
            if (pInputBuf->ulY[i] != 0)
                fctlAttr.ulBaseAddr[i] = pInputBuf->ulY[i];
            if (pInputBuf->ulU[i] != 0)	
                fctlAttr.ulBaseUAddr[i] = pInputBuf->ulU[i];
            if (pInputBuf->ulV[i] != 0)
                fctlAttr.ulBaseVAddr[i] = pInputBuf->ulV[i];
        }

        if (m_VidRecdModes.VidCurBufMode[0] == VIDENC_CURBUF_RT) {
            fctlAttr.bRtModeOut = MMP_TRUE;
            MMPD_Fctl_SetPipeAttrForH264Rt(&fctlAttr, usEncInputW);
        }
        else {
            fctlAttr.bRtModeOut = MMP_FALSE;
            fctlAttr.sScalDelay = m_sFullSpeedScalDelay;
            MMPD_Fctl_SetPipeAttrForH264FB(&fctlAttr);
            
            m_bInitFcamRecPipeFrmBuf = MMP_TRUE;
        }
        
        MMPD_Fctl_LinkPipeToVideo(m_RecordFctlLink.ibcpipeID, ubEncId);
        
        // For raw zoom case. If zoom in happens change scaler delay and raw fetch line delay.
        if (m_VidRecdConfigs.bRawPreviewEnable[0] == MMP_TRUE) {
            MMP_BOOL ubCkScalarUp;
            
            MMPF_Scaler_CheckScaleUp(m_RecordFctlLink.scalerpath, &ubCkScalarUp);

            if (ubCkScalarUp) {
                MMPF_Scaler_SetPixelDelay(m_RecordFctlLink.ibcpipeID, 17, 20);
                MMPF_RAWPROC_SetFetchLineDelay(0xC);
            }
            else {
                MMPF_Scaler_SetPixelDelay(m_RecordFctlLink.ibcpipeID, 1, 1);
                MMPF_RAWPROC_SetFetchLineDelay(0x20);
            }
        }
    }
    else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE ||
    		 m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI 	||
    		 m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE)
    {
        // Config pipe for Encode
        fitrange.fitmode        = MMP_SCAL_FITMODE_OUT;
        fitrange.scalerType 	= MMP_SCAL_TYPE_SCALER;
        
        if (MMP_IsDualVifCamEnable() && MMP_GetParallelFrmStoreType() != PARALLEL_FRM_NOT_SUPPORT)
        	fitrange.ulInWidth  = m_ulLdcMaxOutWidth * 2;
        else
        	fitrange.ulInWidth  = m_ulLdcMaxOutWidth;
        
        fitrange.ulInHeight     = m_ulLdcMaxOutHeight;
        fitrange.ulOutWidth     = usEncInputW;
        fitrange.ulOutHeight	= usEncInputH;
        if(usEncInputW == 1920 && usEncInputH == 1088){
			fitrange.ulOutHeight = 1080;
        }

	    fitrange.ulInGrabX 		= 1;
	    fitrange.ulInGrabY 		= 1;
	    fitrange.ulInGrabW 		= fitrange.ulInWidth;
	    fitrange.ulInGrabH 		= fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        
        MMPD_PTZ_InitPtzInfo(m_RecordFctlLink.scalerpath,
        					 fitrange.fitmode,
				             fitrange.ulInWidth, fitrange.ulInHeight, 
				             fitrange.ulOutWidth, fitrange.ulOutHeight);

		// Be sync with preview path : TBD
		MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);

        if (usCurZoomStep > m_VidRecordZoomInfo.usMaxZoomSteps) {
            usCurZoomStep = m_VidRecordZoomInfo.usMaxZoomSteps;
        }

		MMPD_PTZ_CalculatePtzInfo(m_RecordFctlLink.scalerpath, usCurZoomStep, 0, 0);

		MMPD_PTZ_GetCurPtzInfo(m_RecordFctlLink.scalerpath, &fitrange, &EncodeGrabctl);

        if (MMP_IsVidPtzEnable()) {
            MMPD_PTZ_ReCalculateGrabRange(&fitrange, &EncodeGrabctl);
        }
        
        fctlAttr.colormode          = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
        fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
        fctlAttr.fctllink           = m_RecordFctlLink;
        fctlAttr.fitrange           = fitrange;
        fctlAttr.grabctl            = EncodeGrabctl;
        fctlAttr.usBufCnt           = pInputBuf->ulBufCnt;
        fctlAttr.scalsrc            = MMP_SCAL_SOURCE_LDC;
        fctlAttr.bSetScalerSrc      = MMP_TRUE;
        fctlAttr.ubPipeLinkedSnr    = PRM_SENSOR;
        fctlAttr.bUseRotateDMA      = MMP_FALSE;
        fctlAttr.usRotateBufCnt     = 0;

        if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
            fctlAttr.scalsrc		= MMP_SCAL_SOURCE_GRA;
        }

        for (i = 0; i < fctlAttr.usBufCnt; i++) {
            if (pInputBuf->ulY[i] != 0)
                fctlAttr.ulBaseAddr[i] = pInputBuf->ulY[i];
            if (pInputBuf->ulU[i] != 0)	
                fctlAttr.ulBaseUAddr[i] = pInputBuf->ulU[i];
            if (pInputBuf->ulV[i] != 0)
                fctlAttr.ulBaseVAddr[i] = pInputBuf->ulV[i];
        }

        if (m_VidRecdModes.VidCurBufMode[0] == VIDENC_CURBUF_RT) {
            fctlAttr.bRtModeOut = MMP_TRUE;
            MMPD_Fctl_SetPipeAttrForH264Rt(&fctlAttr, usEncInputW);
        }
        else {
            fctlAttr.bRtModeOut = MMP_FALSE;
            fctlAttr.sScalDelay = m_sFullSpeedScalDelay;
            MMPD_Fctl_SetPipeAttrForH264FB(&fctlAttr);
            
            m_bInitFcamRecPipeFrmBuf = MMP_TRUE;
        }
        
        MMPD_Fctl_LinkPipeToVideo(m_RecordFctlLink.ibcpipeID, ubEncId);
    }

    m_VREncodeFctlAttr[0] = fctlAttr;
    
    // Tune MCI priority of encode pipe for frame based mode
    if (m_VidRecdModes.VidCurBufMode[0] == VIDENC_CURBUF_FRAME) {
        MMPD_VIDENC_TunePipeMaxMCIPriority(m_RecordFctlLink.ibcpipeID);
        
        if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264)) {
            #if (HANDLE_JPEG_EVENT_BY_QUEUE)
            if (MMPF_JPEG_GetCtrlByQueueEnable()) {
                // ICON0 may overflow with sensor OV2710. Decrease other pipe priority setting.
                MMPD_VIDENC_TunePipe2ndMCIPriority(MMPS_3GPRECD_GetDecMjpegToPreviewPipeId());
            }
            #endif
        }
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_EnableRecordPipe
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Turn on and off record for video encode.

 @param[in] bEnable Enable and disable scaler path for video encode.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_EnableRecordPipe(MMP_BOOL bEnable)
{
    if (m_bVidRecordActive[0] ^ bEnable)
    {
        if (bEnable) {
            MMPD_Fctl_EnablePreview(m_ub1stVRStreamSnrId, m_RecordFctlLink.ibcpipeID, bEnable, MMP_FALSE);
        }
        else {
            MMPD_Fctl_EnablePreview(m_ub1stVRStreamSnrId, m_RecordFctlLink.ibcpipeID, bEnable, MMP_TRUE);
        }

        m_bVidRecordActive[0] = bEnable;
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StopRecord
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Stop video recording and fill 3GP tail.

 It works after video start, pause and resume.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
 @retval MMP_3GPRECD_ERR_OPEN_FILE_FAILURE Open file failed.
 @retval MMP_3GPRECD_ERR_CLOSE_FILE_FAILURE Close file failed.
*/
MMP_ERR MMPS_3GPRECD_StopRecord(void)
{
    MMP_ULONG           ulVidNumOpening = 0;
    VIDENC_FW_STATUS    status_fw, status_fw_old;
    MMP_ULONG           ulTimeout = VR_QUERY_STATES_TIMEOUT; 

    #if (DUALENC_SUPPORT)
    if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
        (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)) {
        MMPS_3GPRECD_StopDualH264();
        RTNA_DBG_Str(0, "Dual H264 Stoped\r\n");
    }
    #endif

    MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);

    if (m_VidRecdID == INVALID_ENC_ID)
        status_fw = VIDENC_FW_STATUS_NONE;
    else
        MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);

    status_fw_old = status_fw;

    if ((status_fw == VIDENC_FW_STATUS_START)   ||
        (status_fw == VIDENC_FW_STATUS_PAUSE)   ||
        (status_fw == VIDENC_FW_STATUS_RESUME)  ||
        (status_fw == VIDENC_FW_STATUS_STOP)    ||
        (status_fw == VIDENC_FW_STATUS_PREENCODE)) {
    	extern MMP_BOOL            gbSensorDead[2];
        MMPD_3GPMGR_StopCapture(m_VidRecdID, VIDENC_STREAMTYPE_VIDRECD);

		do {
	        if (gbSensorDead[0] == MMP_TRUE || gbSensorDead[1] == MMP_TRUE)
	        {
	        	RTNA_DBG_Str(0, "MMPS_3GPRECD_StopRecord()::MMPF_VIDENC_StopRecordForStorageAll \r\n");
	        	MMPF_VIDENC_StopRecordForStorageAll();
	        	break;
	        }
			MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);
			if (status_fw != VIDENC_FW_STATUS_STOP) {
				MMPF_OS_Sleep(1);
			}
		} while ((status_fw != VIDENC_FW_STATUS_STOP) && ((--ulTimeout) > 0));

		if (0 == ulTimeout) {
			MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);
		}

        // CHECK
        if ((status_fw_old == VIDENC_FW_STATUS_STOP) && (ulVidNumOpening > 0))
        {
            MMPD_VIDENC_DeInitInstance(m_VidRecdID);
        }
        
        MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);
        
        // Deinit H264 module if all stream are stoped.
        if (ulVidNumOpening == 0) {
            if (MMPD_VIDENC_IsModuleInit()) {
                MMPD_VIDENC_DeinitModule();
            }
        }

        m_VidRecdID = INVALID_ENC_ID;
        #if (DUALENC_SUPPORT)
        if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
            (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)) {
            glEncID[0] = m_VidRecdID;
            if (gbTotalEncNum > 0)
                gbTotalEncNum -= 1;
        }
        #endif
        
        MMPS_3GPRECD_EnableRecordPipe(MMP_FALSE);

        if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
        {
            MMPS_3GPRECD_EnableDualH264Pipe(m_ub2ndVRStreamSnrId, MMP_FALSE);
        }
        
        if (ulVidNumOpening == 0) {
            MMPD_VIDENC_EnableClock(MMP_FALSE);
        }

        if (m_VidRecdConfigs.bAsyncMode) {
            MMPD_AUDIO_SetRecordSilence(MMP_FALSE);
        }
    }
    else if (status_fw == VIDENC_FW_STATUS_NONE) {

        MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);

        if ((ulVidNumOpening > 0) && (m_VidRecdID != INVALID_ENC_ID)) {

            MMPD_VIDENC_DeInitInstance(m_VidRecdID);

            m_VidRecdID = INVALID_ENC_ID;
            #if (DUALENC_SUPPORT)
            if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
                (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)) {
                glEncID[0]  = m_VidRecdID;
                if (gbTotalEncNum > 0)
                    gbTotalEncNum -= 1;
            }
            #endif
            
            MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);
            
            if (ulVidNumOpening == 0) {
                if (MMPD_VIDENC_IsModuleInit()) {
                    MMPD_VIDENC_DeinitModule();
                }
            }
        }
    }
    else {
        // Restore DRAM end address to preview end address
        m_ulVidRecDramEndAddr = m_ulVideoPreviewEndAddr;

        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }

    // Restore DRAM end address to preview end address
    m_ulVidRecDramEndAddr = m_ulVideoPreviewEndAddr;
    
    #if (SUPPORT_COMPONENT_FLOW_CTL)
    MMP_CompCtl_UnLinkComponentList(m_ubCamEncodeListId[PRM_SENSOR], MMP_TRUE);
    #endif

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_PauseRecord
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Pause video recording. It works after video start and resume.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_PauseRecord(void)
{
    VIDENC_FW_STATUS    status_fw;
    MMP_ULONG           ulTimeout = VR_QUERY_STATES_TIMEOUT; 

    MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);

    if ((status_fw == VIDENC_FW_STATUS_START) ||
        (status_fw == VIDENC_FW_STATUS_RESUME)) {

        MMPD_3GPMGR_PauseCapture();

        do {
            MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);
            if (status_fw == VIDENC_FW_STATUS_STOP) {
                return MMP_ERR_NONE;
            }
            else if (status_fw != VIDENC_FW_STATUS_PAUSE) {
                MMPF_OS_Sleep(1);
            }
        } while ((status_fw != VIDENC_FW_STATUS_PAUSE) && ((--ulTimeout) > 0));

        if (0 == ulTimeout) {
            MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);
        }

        return MMP_ERR_NONE;
    }
    else {
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_ResumeRecord
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Resume video recording. It works after video pause.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_ResumeRecord(void)
{
    VIDENC_FW_STATUS    status_fw;
    MMP_ULONG           ulTimeout = VR_QUERY_STATES_TIMEOUT; 

    MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);

    if (status_fw == VIDENC_FW_STATUS_PAUSE) {

        MMPD_3GPMGR_ResumeCapture();

        do {
            MMPD_VIDENC_GetStatus(m_VidRecdID, &status_fw);
            if (status_fw == VIDENC_FW_STATUS_STOP) {
                return MMP_ERR_NONE;
            }            
            else if (status_fw != VIDENC_FW_STATUS_RESUME) {
                MMPF_OS_Sleep(1);
            }
        } while ((status_fw != VIDENC_FW_STATUS_RESUME) && ((--ulTimeout) > 0));

        if (0 == ulTimeout) {
            MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);
        }

        return MMP_ERR_NONE;
    }
    else {
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_PreRecord
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Pre start video recording.

 It start record without enable file saving
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_PreRecord(MMP_ULONG ulPreCaptureMs)
{
    MMP_ULONG           enc_id = 0;
    VIDENC_FW_STATUS    status_vid;
    MMP_ERR             status, sRet;
    MMP_ULONG           ulFps;
    MMP_ULONG           EncWidth, EncHeight;
    MMP_ULONG64         ullBitrate64 		= m_VidRecdModes.ulBitrate[0];
    MMP_ULONG64         ullTimeIncr64 		= m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrement;
    MMP_ULONG64         ullTimeResol64 		= m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrResol;
    MMP_ULONG           ulTargetFrameSize 	= (MMP_ULONG)((ullBitrate64 * ullTimeIncr64)/(ullTimeResol64 * 8));
    MMP_ULONG           ulExpectBufSize 	= 0;
    MMP_ULONG           ulMaxPreEncMs 		= 0;
    #if (EMERGENTRECD_SUPPORT)
    MMP_ULONG           ulMaxAudPreEncMs 	= 0;
    #endif
    MMP_ULONG           ulTimeout = VR_QUERY_STATES_TIMEOUT; 
    MMP_BOOL            bEncTimerEn = MMP_FALSE;
    
    MMPD_VIDENC_IsTimerEnabled(&bEncTimerEn);
    if (!bEncTimerEn) {
        MMPD_VIDENC_EnableTimer(MMP_TRUE);
    }

    MMPS_3GPRECD_SetEncodeRes(MMPS_3GPRECD_FILESTREAM_NORMAL);

    EncWidth  = m_ulVREncodeW[0];
    EncHeight = m_ulVREncodeH[0];

    ulFps = (m_VidRecdModes.SnrInputFrameRate[0].usVopTimeIncrResol - 1 +
            m_VidRecdModes.SnrInputFrameRate[0].usVopTimeIncrement) /
            m_VidRecdModes.SnrInputFrameRate[0].usVopTimeIncrement;

    status = MMPD_VIDENC_CheckCapability(EncWidth, EncHeight, ulFps);

    if (status != MMP_ERR_NONE) {
        return status;
    }
    
    // Calculate max pre-encode time limit
    #if (EMERGENTRECD_SUPPORT)
    ulMaxAudPreEncMs = (m_VidRecdConfigs.ulAudioCompBufSize) / (m_VidRecdModes.ulAudBitrate >> 3) * 1000 - 1000;
    #endif

    ulExpectBufSize = m_VidRecdConfigs.ulVideoCompBufSize - (ulTargetFrameSize * 3); /* Reserve 3 frame space */

    #if (EMERGENTRECD_SUPPORT)
    ulMaxPreEncMs = (MMP_ULONG)(((MMP_ULONG64)ulExpectBufSize * 1000) / (m_VidRecdModes.ulBitrate[0] >> 3)) - 1000;
    #else
    ulMaxPreEncMs = (MMP_ULONG)(((MMP_ULONG64)ulExpectBufSize * 900) / (m_VidRecdModes.ulBitrate[0] >> 3));
    #endif

    #if (EMERGENTRECD_SUPPORT)
    if (ulMaxPreEncMs > ulMaxAudPreEncMs) {
        ulMaxPreEncMs = ulMaxAudPreEncMs;
    }
    #endif
    
    if (ulPreCaptureMs > ulMaxPreEncMs) {
        PRINTF("The pre-record duration %d is over preferred %d ms\r\n", ulPreCaptureMs, ulMaxPreEncMs);
        ulPreCaptureMs = ulMaxPreEncMs;
    }

    if (m_VidRecdID == INVALID_ENC_ID)
        status_vid = VIDENC_FW_STATUS_NONE;
    else
        MMPD_VIDENC_GetStatus(m_VidRecdID, &status_vid);

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP)) {

        if (!MMPD_VIDENC_IsModuleInit()) {
            MMP_ERR	mmpstatus;

            mmpstatus = MMPD_VIDENC_InitModule();
            if (mmpstatus != MMP_ERR_NONE)
                return mmpstatus;
        }
        
        /* Reset Icon/IBC to prevent frame shift or broken */
        /* Reset record pipe cause preview image broken. Don't know why yet. So, reset preview pipe first.*/
        sRet |= MMPD_Scaler_ResetModule(m_PreviewFctlLink.scalerpath);  
        sRet |= MMPD_Icon_ResetModule(m_PreviewFctlLink.icopipeID);
        sRet |= MMPD_IBC_ResetModule(m_PreviewFctlLink.ibcpipeID);
        sRet |= MMPD_Scaler_ResetModule(m_RecordFctlLink.scalerpath);
        sRet |= MMPD_Icon_ResetModule(m_RecordFctlLink.icopipeID);
        sRet |= MMPD_IBC_ResetModule(m_RecordFctlLink.ibcpipeID);  

        if (MMPS_3GPRECD_SetH264MemoryMap(  &enc_id,
                                            EncWidth,
                                            EncHeight,
                                            m_ulVidRecSramAddr,
                                            m_ulVideoPreviewEndAddr))
        {
            RTNA_DBG_Str(0, "Alloc mem for video pre-record failed\r\n");
            return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
        }
        
        // Set current buffer mode before MMPF_Display_StartPreview() use it
        switch (m_VidRecdModes.VidCurBufMode[0]) {
        case VIDENC_CURBUF_FRAME:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_FRAME);
            break;
        case VIDENC_CURBUF_RT:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_RT);
            break;
        default:
            return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
        }

        if (MMPS_3GPRECD_EnableRecordPipe(MMP_TRUE) != MMP_ERR_NONE) {
            PRINTF("Enable Video Record: Fail\r\n");
            return MMP_3GPRECD_ERR_GENERAL_ERROR;
        }

        // Set encoder and merger parameters
        if (((EncWidth == 3200) && (EncHeight == 1808)) ||
            ((EncWidth == 1920) && (EncHeight == 1088)) ||
            ((EncWidth == 1440) && (EncHeight == 1088)) ||
            ((EncWidth == 640) && (EncHeight == 368))) {
            MMPD_VIDENC_SetCropping(enc_id, 0, 8, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_REPEAT, 8);
        }
        else {
            MMPD_VIDENC_SetCropping(enc_id, 0, 0, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_NONE, 0);
        }

        if (MMPD_VIDENC_SetResolution(enc_id, EncWidth, EncHeight) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "MMPD_VIDENC_SetResolution Err\r\n");
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }

        if (MMPD_VIDENC_SetProfile(enc_id, m_VidRecdModes.VisualProfile[0]) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "MMPD_VIDENC_SetProfile Err\r\n");
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }
        
        MMPD_VIDENC_SetEncodeMode();

        MMPD_3GPMGR_SetGOP(VIDENC_STREAMTYPE_VIDRECD, m_VidRecdModes.usPFrameCount[0], m_VidRecdModes.usBFrameCount[0]);
        MMPD_VIDENC_SetGOP(enc_id, m_VidRecdModes.usPFrameCount[0], m_VidRecdModes.usBFrameCount[0]);

        MMPD_VIDENC_SetQuality(enc_id, ulTargetFrameSize, (MMP_ULONG)ullBitrate64);

        MMPD_VIDENC_SetSnrFrameRate(enc_id,
                                    VIDENC_STREAMTYPE_VIDRECD,
                                    m_VidRecdModes.SnrInputFrameRate[0].usVopTimeIncrement,
                                    m_VidRecdModes.SnrInputFrameRate[0].usVopTimeIncrResol);

        MMPD_VIDENC_SetEncFrameRate(enc_id,
                                    m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrement,
                                    m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrResol);

        MMPD_3GPMGR_SetFrameRate(VIDENC_STREAMTYPE_VIDRECD,
                                 m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrResol,
                                 m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrement);
        
        // Start pre-encode
        MMPD_VIDENC_EnableClock(MMP_TRUE);
        
        MMPD_3GPMGR_PreCapture(VIDENC_STREAMTYPE_VIDRECD, ulPreCaptureMs);

        do {
            MMPD_VIDENC_GetStatus(enc_id, &status_vid);
            if (status_vid != VIDENC_FW_STATUS_PREENCODE) {
                MMPF_OS_Sleep(1);
            }
        } while ((status_vid != VIDENC_FW_STATUS_PREENCODE) && ((--ulTimeout) > 0));

        if (0 == ulTimeout) {
            MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);
        }

        #if (DUALENC_SUPPORT == 1)
        if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
            (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)){
            glEncID[gbTotalEncNum] = m_VidRecdID;
            gbTotalEncNum ++;
        }
        #endif
        
        #if (UVC_VIDRECD_SUPPORT)
        if (m_bUVCRecdSupport) { 
            MMPD_UVCRECD_EnableRecd();
        }
        #endif
    }
    else {
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StartRecord
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Start video recording.

 It can saves the 3GP file to host memory or memory card.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_StartRecord(void)
{
    MMP_ULONG           enc_id = 0;
    VIDENC_FW_STATUS    status_vid;
    MMP_ERR             status;
    MMP_ULONG           EncWidth, EncHeight;
    MMP_ULONG64         ullBitrate64 = m_VidRecdModes.ulBitrate[0];
    void                *pSeamlessCB;
    MMP_ULONG           ulAvaSize;
    MMP_ULONG           status_tx;
    MMP_ULONG           ulFps;
    MMP_ULONG           ulVidNumOpening = 0;
    MMP_ULONG           ulTimeout = VR_QUERY_STATES_TIMEOUT; 
    MMP_BOOL            bEncTimerEn = MMP_FALSE;
    
    // Add error handle for start record.
    // Root cause: Slow card happens during seamless record. Will call seamless flow and cause gbTotalEncNum still have value.
    // need to call stop record here to correct kernel state.
    #if (DUALENC_SUPPORT) // CHECK
    if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
        (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)) {
        if (gbTotalEncNum != 0) {
            MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);
            if (ulVidNumOpening == 0) {
                MMPS_3GPRECD_StopRecord();
                RTNA_DBG_Str(0, "start record and h264 instance not match\r\n");
            }
        }
    }
    #endif

    MMPD_VIDENC_IsTimerEnabled(&bEncTimerEn);
    if (!bEncTimerEn) {
        MMPD_VIDENC_EnableTimer(MMP_TRUE);
    }

    MMPS_3GPRECD_SetEncodeRes(MMPS_3GPRECD_FILESTREAM_NORMAL);

    EncWidth  = m_ulVREncodeW[0];
    EncHeight = m_ulVREncodeH[0];

    DBG_AutoTestPrint(ATEST_ACT_REC_0x0001, ATEST_STS_RSOL_SIZE_0x0004, EncWidth, EncHeight, gubMmpDbgBk);

    ulFps = (m_VidRecdModes.SnrInputFrameRate[0].usVopTimeIncrResol - 1 +
            m_VidRecdModes.SnrInputFrameRate[0].usVopTimeIncrement) /
            m_VidRecdModes.SnrInputFrameRate[0].usVopTimeIncrement;

    status = MMPD_VIDENC_CheckCapability(EncWidth, EncHeight, ulFps);

    if (status != MMP_ERR_NONE) {
        return status;
    }
    
    if (m_VidRecdID == INVALID_ENC_ID)
    	status_vid = VIDENC_FW_STATUS_NONE;
    else
    	MMPD_VIDENC_GetStatus(m_VidRecdID, &status_vid);

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP)) {
        
        // For sync live stream audio pointer
        MMPF_AUDIO_WaitForSync();

        if (!MMPD_VIDENC_IsModuleInit()) {
            MMP_ERR	mmpstatus;

            mmpstatus = MMPD_VIDENC_InitModule();
            if (mmpstatus != MMP_ERR_NONE) {
                RTNA_DBG_Str(0, "module init fail\r\n");
                return mmpstatus;
           	}
        }
        
        /* Now stop record will not stop preview (all pipes actually)
         * User can call MMPS_3GPRECD_StopRecord() and then call
         * MMPS_3GPRECD_StartRecord() to re-start recording without
         * stop/start preview. So we have to reset start address of mem
         * for encoder here.
         */
        if (MMPS_3GPRECD_SetH264MemoryMap(  &enc_id,
                                            EncWidth,
                                            EncHeight,
                                            m_ulVidRecSramAddr,
                                            m_ulVideoPreviewEndAddr))
        {
            RTNA_DBG_Str(0, "Alloc mem for video record failed\r\n");
            return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
        }
        
        // Set current buffer mode before MMPF_Display_StartPreview() use it
        switch (m_VidRecdModes.VidCurBufMode[0]) {
        case VIDENC_CURBUF_FRAME:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_FRAME);
            break;
        case VIDENC_CURBUF_RT:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_RT);
            break;
        default:
            return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
        }

        if (MMPS_3GPRECD_EnableRecordPipe(MMP_TRUE) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "Enable Video Record: Fail\r\n");
            return MMP_3GPRECD_ERR_GENERAL_ERROR;
        }
        
        // Set encoder and merger parameters
        if (((EncWidth == 3200) && (EncHeight == 1808)) ||
            ((EncWidth == 1920) && (EncHeight == 1088)) ||
            ((EncWidth == 1440) && (EncHeight == 1088)) ||
            ((EncWidth == 640) && (EncHeight == 368))) {
            
            MMPD_VIDENC_SetCropping(enc_id, 0, 8, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_REPEAT, 8);
        }
        else if (((EncWidth == 1600) && (EncHeight == 912))) {
            MMPD_VIDENC_SetCropping(enc_id, 0, 12, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_REPEAT, 12);
        }
        else {
            MMPD_VIDENC_SetCropping(enc_id, 0, 0, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_NONE, 0);
        }

        if (MMPD_VIDENC_SetResolution(enc_id, EncWidth, EncHeight) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "MMPD_VIDENC_SetResolution Err\r\n");
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }
        if (MMPD_VIDENC_SetProfile(enc_id, m_VidRecdModes.VisualProfile[0]) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "MMPD_VIDENC_SetProfile Err\r\n");
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }
        
        MMPD_VIDENC_SetEncodeMode();

        MMPD_3GPMGR_SetGOP(VIDENC_STREAMTYPE_VIDRECD, m_VidRecdModes.usPFrameCount[0], m_VidRecdModes.usBFrameCount[0]);
        MMPD_VIDENC_SetGOP(enc_id, m_VidRecdModes.usPFrameCount[0], m_VidRecdModes.usBFrameCount[0]);

        MMPD_VIDENC_SetQuality(enc_id, MMPS_3GPRECD_CalculteTargetFrmSize(0), (MMP_ULONG)ullBitrate64);

        MMPD_VIDENC_SetSnrFrameRate(enc_id,
                                    VIDENC_STREAMTYPE_VIDRECD,
                                    m_VidRecdModes.SnrInputFrameRate[0].usVopTimeIncrement,
                                    m_VidRecdModes.SnrInputFrameRate[0].usVopTimeIncrResol);

        MMPD_VIDENC_SetEncFrameRate(enc_id,
                                    m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrement,
                                    m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrResol);

        MMPD_3GPMGR_SetFrameRate(VIDENC_STREAMTYPE_VIDRECD,
                                 m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrResol,
                                 m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrement);
    }

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP) ||
        (status_vid == VIDENC_FW_STATUS_PREENCODE)) {
        
    #if (SUPPORT_SHARE_REC == 0)
        // Change start capture to MMPS_3GPRECD_StartAllRecord() for dual encode slow card issue.
        if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
            (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)) {
            
            // Just keep encode ID and encode total number here.
            if ((status_vid == VIDENC_FW_STATUS_NONE) ||
                (status_vid == VIDENC_FW_STATUS_STOP)) {
                if (gbTotalEncNum > 2) {
                    printc("ERR : some enc not disable: %x\r\n", gbTotalEncNum);
                }
                glEncID[gbTotalEncNum] = m_VidRecdID;
                gbTotalEncNum++;
            }
            return MMP_ERR_NONE;
        }
        else
    #endif
        { // Non dual encode case.
            
            MMPD_3GPMGR_SetFileLimit(m_VidRecdModes.ulSizeLimit, m_VidRecdModes.ulReservedSpace, &ulAvaSize);

            if (MMPS_3GPRECD_GetExpectedRecordTime(ulAvaSize, m_VidRecdModes.ulBitrate[0], m_VidRecdModes.ulAudBitrate) <= 0) {
                RTNA_DBG_Str(0, "MMPS_3GPRECD_GetExpectedRecordTime: Fail\r\n");
                return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
            }
            else {
                MMPD_3GPMGR_SetTimeLimit(m_VidRecdModes.ulTimeLimitMs);
            }

            if (m_VidRecdConfigs.bSeamlessMode == MMP_TRUE) {
                // Seamless callback must be registered if seamless mode is enabled.
                MMPD_3GPMGR_GetRegisteredCallback(VIDMGR_EVENT_SEAMLESS, &pSeamlessCB);

                if (m_bSeamlessEnabled && (pSeamlessCB == NULL)) {
                    RTNA_DBG_Str(0, "Get seamless: Fail\r\n");
                    return MMP_3GPRECD_ERR_GENERAL_ERROR;
                }
            }

            if (status_vid != VIDENC_FW_STATUS_PREENCODE) {
                MMPD_VIDENC_EnableClock(MMP_TRUE);
            }
            else {
                /*
                 * In pre-encode case,m_VidRecdID is always valid,
                 * besides enc_id will not be assigned by MMPS_3GPRECD_SetH264MemoryMap
                 * so dricetly assign m_VidRecdID to enc_id here!
                 */
                enc_id = m_VidRecdID;
            }

            if (MMPD_3GPMGR_StartCapture(m_VidRecdID, VIDENC_STREAMTYPE_VIDRECD) != MMP_ERR_NONE) {
                MMPD_VIDENC_EnableClock(MMP_FALSE);
                return MMP_3GPRECD_ERR_GENERAL_ERROR;
            }

            do {
                MMPD_VIDENC_GetMergerStatus(&status, &status_tx);
                MMPD_VIDENC_GetStatus(enc_id, &status_vid);
                
                if (status_vid == VIDENC_FW_STATUS_STOP){
                    return MMP_3GPRECD_ERR_STOPRECD_SLOWCARD;
                }
                if ((status_vid != VIDENC_FW_STATUS_START) && (status != MMP_FS_ERR_OPEN_FAIL)){
                    MMPF_OS_Sleep(1);
                }
            } while ((status_vid != VIDENC_FW_STATUS_START) && (status != MMP_FS_ERR_OPEN_FAIL) && ((--ulTimeout) > 0));

            if (0 == ulTimeout) {
                MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);
            }

            if (status == MMP_FS_ERR_OPEN_FAIL) {
                RTNA_DBG_Str(0, "Get MGR file open Fail\r\n");
                return status;
            }

            #if (SUPPORT_SHARE_REC)
            glEncID[0] = m_VidRecdID;
            if (status_vid == VIDENC_FW_STATUS_START) {
                gbTotalEncNum = 1;
            }
            #endif

            if (status_vid == VIDENC_FW_STATUS_START) {
                return MMP_ERR_NONE;
            }
            else if (status == MMP_FS_ERR_OPEN_FAIL) {
                return MMP_3GPRECD_ERR_OPEN_FILE_FAILURE;
            }
            else {
                return MMP_ERR_NONE;
            }
        }
    }
    else {
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetAudioFormat
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set audio mode to be AMR or AAC.
 @param[in] Format Assign the audio type is AMR or AAC with video encoder.
 @param[in] Option Assign audio sampling rate and bit rate.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_INVALID_PARAMETERS Invalid parameter.
 @note

 The parameter @a Option support:
 - MMPS_3GPRECD_AUDIO_AAC_22d05K_64K : 22.05KHz with 64kbps
 - MMPS_3GPRECD_AUDIO_AAC_22d05K_128K : 22.05KHz with 128kbps
 - MMPS_3GPRECD_AUDIO_AAC_32K_64K : 32KHz with 64kbps
 - MMPS_3GPRECD_AUDIO_AAC_32K_128K : 32KHz with 128kbps
 - MMPS_3GPRECD_AUDIO_AMR_4d75K : 4.75 KHz
 - MMPS_3GPRECD_AUDIO_AMR_5d15K : 5.15 KHz
 - MMPS_3GPRECD_AUDIO_AMR_12d2K : 12.2 KHz
*/
MMP_ERR MMPS_3GPRECD_SetAudioFormat(MMPS_3GPRECD_AUDIO_FORMAT Format, MMPS_3GPRECD_AUDIO_OPTION Option)
{
    MMP_ULONG   audioparam = 0;
    MMP_USHORT  audiomode = MMPS_AUDIO_AAC_RECORD_32K_128K;
    MMP_ERR     ret = MMP_ERR_NONE;

    if (Format == MMPS_3GPRECD_AUDIO_FORMAT_AAC) {
    
        MMPD_AUDIO_SetEncodeFormat(MMP_AUDIO_VAAC_ENCODE);

        switch (Option) {
        case MMPS_3GPRECD_AUDIO_AAC_16K_32K:
            audiomode = MMPS_AUDIO_AAC_RECORD_16K_32K;
            audioparam = 16000;
            m_VidRecdModes.ulAudBitrate = 32000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        case MMPS_3GPRECD_AUDIO_AAC_16K_64K:
            audiomode = MMPS_AUDIO_AAC_RECORD_16K_64K;
            audioparam = 16000;
            m_VidRecdModes.ulAudBitrate = 64000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        case MMPS_3GPRECD_AUDIO_AAC_22d05K_64K:
            audiomode 	= MMPS_AUDIO_AAC_RECORD_22K_64K;
            audioparam 	= 22050;
            m_VidRecdModes.ulAudBitrate = 64000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        case MMPS_3GPRECD_AUDIO_AAC_22d05K_128K:
            audiomode 	= MMPS_AUDIO_AAC_RECORD_22K_128K;
            audioparam	= 22050;
            m_VidRecdModes.ulAudBitrate = 128000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        case MMPS_3GPRECD_AUDIO_AAC_32K_64K:
            audiomode 	= MMPS_AUDIO_AAC_RECORD_32K_64K;
            audioparam 	= 32000;
            m_VidRecdModes.ulAudBitrate = 64000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        case MMPS_3GPRECD_AUDIO_AAC_32K_128K:
            audiomode 	= MMPS_AUDIO_AAC_RECORD_32K_128K;
            audioparam 	= 32000;
            m_VidRecdModes.ulAudBitrate = 128000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        case MMPS_3GPRECD_AUDIO_AAC_48K_128K:
            audiomode 	= MMPS_AUDIO_AAC_RECORD_48K_128K;
            audioparam 	= 48000;
            m_VidRecdModes.ulAudBitrate = 128000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        default:
            ret = MMP_3GPMGR_ERR_PARAMETER;
            break;
        }
        
        MMPD_AUDIO_SetEncodeFileSize(0xFFFFFFFF);
        MMPD_AUDIO_SetEncodeMode(audiomode);
        MMPD_3GPMGR_SetAudioParam(audioparam, MMPD_3GPMGR_AUDIO_FORMAT_AAC);
    }
    else if (Format == MMPS_3GPRECD_AUDIO_FORMAT_AMR) {
    
        MMPD_AUDIO_SetEncodeFormat(MMP_AUDIO_VAMR_ENCODE);
        MMPD_AUDIO_SetPLL(8000);

        switch (Option) {
        case MMPS_3GPRECD_AUDIO_AMR_4d75K:
            audiomode 	= MMPS_AUDIO_AMR_MR475_ENCODE_MODE;
            audioparam 	= 13;
            m_VidRecdModes.ulAudBitrate = 4750;
            break;
        case MMPS_3GPRECD_AUDIO_AMR_5d15K:
            audiomode 	= MMPS_AUDIO_AMR_MR515_ENCODE_MODE;
            audioparam 	= 14;
            m_VidRecdModes.ulAudBitrate = 5150;
            break;
        case MMPS_3GPRECD_AUDIO_AMR_12d2K:
            audiomode 	= MMPS_AUDIO_AMR_MR122_ENCODE_MODE;
            audioparam 	= 32;
            m_VidRecdModes.ulAudBitrate = 1220;
            break;
        default:
            ret = MMP_3GPMGR_ERR_PARAMETER;
            break;
        }
        
        MMPD_AUDIO_SetEncodeMode(audiomode);
        ret |= MMPD_3GPMGR_SetAudioParam(audioparam, MMPD_3GPMGR_AUDIO_FORMAT_AMR);
    }
    else if (Format == MMPS_3GPRECD_AUDIO_FORMAT_ADPCM) {
    
        MMPD_AUDIO_SetEncodeFormat(MMP_AUDIO_VADPCM_ENCODE);

        switch (Option) {
        case MMPS_3GPRECD_AUDIO_ADPCM_32K_22K:
            audiomode 	= MMPS_3GPRECD_AUDIO_ADPCM_32K_22K;
            audioparam 	= 32000;
            m_VidRecdModes.ulAudBitrate = 256000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        case MMPS_3GPRECD_AUDIO_ADPCM_44d1K_22K:
            audiomode 	= MMPS_3GPRECD_AUDIO_ADPCM_44d1K_22K;
            audioparam 	= 44100;
            m_VidRecdModes.ulAudBitrate = 352800;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        case MMPS_3GPRECD_AUDIO_ADPCM_48K_22K:
            audiomode 	= MMPS_3GPRECD_AUDIO_ADPCM_48K_22K;
            audioparam 	= 48000;
            m_VidRecdModes.ulAudBitrate = 384000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        default:
            ret = MMP_3GPMGR_ERR_PARAMETER;
            break;
        }
        
        MMPD_AUDIO_SetEncodeFileSize(0xFFFFFFFF);
        MMPD_AUDIO_SetEncodeMode(audiomode);
        MMPD_3GPMGR_SetAudioParam(audioparam, MMPD_3GPMGR_AUDIO_FORMAT_ADPCM);
    }
    else if (Format == MMPS_3GPRECD_AUDIO_FORMAT_MP3) {
    
        MMPD_AUDIO_SetEncodeFormat(MMP_AUDIO_VMP3_ENCODE);

        switch (Option) {
        case MMPS_3GPRECD_AUDIO_MP3_32K_128K:
            audiomode 	= MMPS_AUDIO_MP3_RECORD_32K_128K;
            audioparam 	= 32000;
            m_VidRecdModes.ulAudBitrate = 128000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        case MMPS_3GPRECD_AUDIO_MP3_44d1K_128K:
            audiomode 	= MMPS_AUDIO_MP3_RECORD_44d1K_128K;
            audioparam 	= 44100;
            m_VidRecdModes.ulAudBitrate = 128000;
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        default:
            ret = MMP_3GPMGR_ERR_PARAMETER;
            break;
        }
        
        MMPD_AUDIO_SetEncodeFileSize(0xFFFFFFFF);
        MMPD_AUDIO_SetEncodeMode(audiomode);
        MMPD_3GPMGR_SetAudioParam(audioparam, MMPD_3GPMGR_AUDIO_FORMAT_MP3);
    }
    else if (Format == MMPS_3GPRECD_AUDIO_FORMAT_PCM) {
        
        MMPD_AUDIO_SetEncodeFormat(MMP_AUDIO_VPCM_ENCODE);

        switch (Option) {
        case MMPS_3GPRECD_AUDIO_PCM_16K:
            audiomode 	= MMPS_AUDIO_WAV_RECORD_16K;
            audioparam 	= 16000;
            m_VidRecdModes.ulAudBitrate = 16000 * 2 * 2; // 2ch 16-bit sample
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        case MMPS_3GPRECD_AUDIO_PCM_32K:
            audiomode 	= MMPS_AUDIO_WAV_RECORD_32K;
            audioparam 	= 32000;
            m_VidRecdModes.ulAudBitrate = 32000 * 2 * 2; // 2ch 16-bit sample
            MMPD_AUDIO_SetPLL(audioparam);
            break;
        default:
            ret = MMP_3GPMGR_ERR_PARAMETER;
            break;
        }
        
        MMPD_AUDIO_SetEncodeFileSize(0xFFFFFFFF);
        MMPD_AUDIO_SetEncodeMode(audiomode);
        MMPD_3GPMGR_SetAudioParam(audioparam, MMPD_3GPMGR_AUDIO_FORMAT_PCM);
    }
    else {
        ret = MMP_3GPMGR_ERR_PARAMETER;
    }

    if (ret == MMP_ERR_NONE) {
        return MMP_ERR_NONE;
    }
    else {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetAudioRecMode
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Enable audio encode.
 @param[in] bEnable Enable video record with audio.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetAudioRecMode(MMPS_3GPRECD_AUDIO_DATATYPE mode)
{
	if (mode != MMPS_3GPRECD_NO_AUDIO_DATA) {
        MMPD_AUDIO_SetInPath(MMPS_Audio_GetConfig()->AudioInPath);
        MMPD_3GPMGR_EnableAVSyncEncode(MMP_TRUE);
	}
    else {
        m_VidRecdModes.ulAudBitrate = 0;
        MMPD_3GPMGR_EnableAVSyncEncode(MMP_FALSE);
    }

    if (mode == MMPS_3GPRECD_SILENCE_AUDIO_DATA)
        MMPD_AUDIO_SetRecordSilence(MMP_TRUE);
    else
        MMPD_AUDIO_SetRecordSilence(MMP_FALSE);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetRecordTime
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get how many ms passed after starting recording.
 @param[out] ulTime Recording time in unit of ms.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetRecordTime(MMP_ULONG *ulTime)
{
    #if (EMERGENTRECD_SUPPORT)
    {
        extern MMP_BOOL m_bStartEmerVR;
        if ((m_bStartEmerVR) && (MMPS_3GPRECD_GetEmergActionType() == MMP_3GPRECD_EMERG_SWITCH_FILE)) {
            return MMPD_3GPMGR_GetRecordingTime(VIDENC_STREAMTYPE_EMERGENCY, ulTime);
        }
    }
    #endif
    
    return MMPD_3GPMGR_GetRecordingTime(VIDENC_STREAMTYPE_VIDRECD, ulTime);  
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetRecordedSize
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get current size of encoded 3GP file.

 This size only counts file header and current AV bitstream.
 @param[out] ulSize Current file size.
 @return Error status.
*/
MMP_ERR MMPS_3GPRECD_GetRecordedSize(MMP_ULONG *ulSize)
{
    return MMPD_3GPMGR_Get3gpFileCurSize(ulSize);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_Get3GPFileSize
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get final size of encoded 3GP file.
 @param[out] ulfilesize Final 3GP file size.
 @return Error status.
*/
MMP_ERR MMPS_3GPRECD_Get3GPFileSize(MMP_ULONG *ulFileSize)
{
    return MMPD_3GPMGR_Get3gpFileSize(ulFileSize);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_CalTailBufSize
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Calculate the container tail buffer size according to the specified time.
 @param[in] time The specified time in unit of second.
 @return The size of tail buffer needed.
*/
static MMP_ULONG MMPS_3GPRECD_CalTailBufSize(MMP_ULONG ulTime)
{
    MMP_ULONG ulTailVidSize = 0;
    MMP_ULONG ulTailAudSize = 0;
    MMP_ULONG ulSlowMoRatio = 1;

    /* Calculate tail info size for video frames */
    ulTailVidSize = (MMP_ULONG)(((MMP_ULONG64)ulTime *
                    m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrResol) /
                    m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrement);
                 
    if (m_bMuxer3gpConstantFps) {                
    	ulTailVidSize = ulTailVidSize << 2; // 4-byte for size info
    }
    else {
    	ulTailVidSize = ulTailVidSize << 3; // 4-byte each for time & size info
    }
    
    /* Calculate tail info size for audio frames */
    if (MMPD_3GPMGR_GetAVSyncEncode()) {

        if (m_VidRecdModes.bSlowMotionEn) {
            ulSlowMoRatio = (m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrResol *
                            m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrement) /
                            (m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrResol *
                            m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrement);
            
            if (ulSlowMoRatio != 0) {
                ulTime = ulTime * ulSlowMoRatio;
            }
        }
        
        MMPD_AUDIO_GetRecordFrameCntInSeconds(ulTime, &ulTailAudSize);
        
        ulTailAudSize = ulTailAudSize << 2; // 4-byte for size info
    }

    return (ulTailVidSize + ulTailAudSize);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetContainerTailBufSize
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get the container tail buffer size.
 @return The size of tail buffer needed.

 @pre Container type is already set by MMPS_3GPRECD_SetContainerType();
      File size is already set by MMPS_3GPRECD_SetFileSizeLimit();
      File time is already set by MMPS_3GPRECD_SetFileTimeLimit();
      Video frame rate is already set by MMPS_3GPRECD_SetFrameRatePara();
      Audio mode is alreay selected by MMPS_3GPRECD_SetAudioRecMode();
      Audio format is already set by MMPS_3GPRECD_SetAudioFormat().
*/
MMP_ULONG MMPS_3GPRECD_GetContainerTailBufSize(void)
{
    MMP_LONG  lSpace2FTime;
    MMP_ULONG ulSizeByFTime = 0;
    MMP_ULONG ulSizeByFSize = 0;
    MMP_ULONG ulMarginTime;
    MMP_ULONG ulAudBitrate = 0;

    switch (m_VidRecdContainer) {
    case VIDMGR_CONTAINER_3GP:
        /* Currently only 3GP needs tail buffer */
        break;
    case VIDMGR_CONTAINER_NONE:
    case VIDMGR_CONTAINER_AVI:
    default:
        return 0;
    }

    /* If file time limit is set, calculate tail buffer size by time limit */
    if (m_VidRecdModes.ulTimeLimitMs != 0xFFFFFFFF) {
        ulMarginTime = (m_VidRecdModes.ulTimeLimitMs * 6) / 5000; // 20% margin
        ulSizeByFTime = MMPS_3GPRECD_CalTailBufSize(ulMarginTime);
    }

    /* Calculate tail buffer size by size limit*/
    if (MMPD_3GPMGR_GetAVSyncEncode()) {
        ulAudBitrate = m_VidRecdModes.ulAudBitrate;
    }
    
    lSpace2FTime = MMPS_3GPRECD_GetExpectedRecordTime(  m_VidRecdModes.ulSizeLimit,
                                                        m_VidRecdModes.ulBitrate[0],
                                                        ulAudBitrate);
                                                        
    ulMarginTime = (lSpace2FTime * 27) / 20; // 35% margin
    ulSizeByFSize = MMPS_3GPRECD_CalTailBufSize(ulMarginTime);

    /* Select the smaller one */
    if (ulSizeByFTime)
        return (ulSizeByFTime < ulSizeByFSize) ? ulSizeByFTime : ulSizeByFSize;
    else
        return ulSizeByFSize;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetRecordStatus
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get status of firmware video recorder.
 @param[out] retstatus Firmware operation status.
 @return Error status.
 @note

 The parameter @a retstatus can be:
 - VIDENC_FW_STATUS_NONE Idle.
 - VIDENC_FW_STATUS_START Under recording.
 - VIDENC_FW_STATUS_PAUSE Pause recording.
 - VIDENC_FW_STATUS_RESUME Restart recording.
 - VIDENC_FW_STATUS_STOP Stop recording.
*/
MMP_ERR MMPS_3GPRECD_GetRecordStatus(VIDENC_FW_STATUS *retstatus)
{
    MMP_ERR             ret;
    VIDENC_FW_STATUS    status_vid;

    if (m_VidRecdID == INVALID_ENC_ID) {
        status_vid  = VIDENC_FW_STATUS_NONE;
        ret         = MMP_ERR_NONE;
    }
    else {
        ret = MMPD_VIDENC_GetStatus(m_VidRecdID, &status_vid);
    }
    
    *retstatus = status_vid;

    return ret;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetH264MemoryMap
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set memory layout by different resolution and memory type for H264.

 Depends on encoded resolution and memory type to map buffers. It supports two types
 of memory, INTER(823) and STACK(821).
 @param[in] usEncW Encode width.
 @param[in] usEncH Encode height.
 @param[in] ulFBufAddr Available start address of frame buffer.
 @param[in] ulStackAddr Available start address of dram buffer.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetH264MemoryMap(MMP_ULONG *ulEncId, MMP_USHORT usEncW, MMP_USHORT usEncH, MMP_ULONG ulFBufAddr, MMP_ULONG ulStackAddr)
{
    const MMP_ULONG                 AVSyncBufSize         = 32;
    #if (EMERGENTRECD_SUPPORT)
    const MMP_ULONG                	VideoSizeTableSize    = 1024 * 4; // Normal record and emergent record
    const MMP_ULONG                 VideoTimeTableSize    = 1024 * 2; // Normal record and emergent record
    #else
    const MMP_ULONG                	VideoSizeTableSize    = 1024;   // Aux size table
    const MMP_ULONG                 VideoTimeTableSize    = 1024;   // Aux time table
    #endif
    const MMP_ULONG                 AudioCompBufSize      = m_VidRecdConfigs.ulAudioCompBufSize;
    const MMP_ULONG                 SPSSize               = 48;
    const MMP_ULONG                 PPSSize               = 16;
    const MMP_ULONG                 VideoCompBufSize      = m_VidRecdConfigs.ulVideoCompBufSize;
    #if (EMERGENTRECD_SUPPORT)
    const MMP_ULONG                	AVRepackBufSize       = 512 * 1024 * 2;
    #else
    #if (SUPPORT_SHARE_REC)
    const MMP_ULONG                	AVRepackBufSize       = 1024 * 1024;
    #else
    const MMP_ULONG                	AVRepackBufSize       = 512 * 1024;
    #endif
    #endif
    const MMP_ULONG                 SliceLengBufSize      = ALIGN_X((4 * ((usEncH>>4) + 2)), 64);
    const MMP_ULONG                 MVBufSize             = ((usEncW>>4) * (usEncH>>4)) * 16 * 4; //MBs * (Blocks per MB)* (4 Bytes)   
    MMP_USHORT                      i;
    MMP_ULONG                       ulCurAddr;
    MMPD_MP4VENC_VIDEOBUF           videohwbuf;
    MMPD_H264ENC_HEADER_BUF         headerbuf;
    MMPD_3GPMGR_AV_COMPRESS_BUF	    mergercompbuf;
    MMPD_3GPMGR_REPACKBUF           repackmiscbuf;
    MMP_ULONG                       bufferEndAddr;
    MMP_ULONG                       ulEncFrameSize;
    #if (UVC_VIDRECD_SUPPORT)
    MMPD_H264ENC_HEADER_BUF         UVCheaderbuf;
    MMPD_3GPMGR_AV_COMPRESS_BUF     UVCmergercompbuf;
    const MMP_ULONG                 UVCVidCompBufSize      = m_VidRecdConfigs.ulUVCVidCompBufSize;
    #if (VIDRECD_MULTI_TRACK == 0)
    MMPD_3GPMGR_REPACKBUF           UVCrepackmiscbuf;
    const MMP_ULONG                 UVCRepackBufSize       = 512 * 1024;
    const MMP_ULONG                 UVCVideoSizeTableSize  = 1024;
    const MMP_ULONG                 UVCVideoTimeTableSize  = 1024;
    #endif // VIDRECD_MULTI_TRACK == 0
    #endif // UVC_VIDRECD_SUPPORT
    #if (UVC_EMERGRECD_SUPPORT)
    MMPD_3GPMGR_REPACKBUF           UVCE_repackmiscbuf;
    const MMP_ULONG                 UVCE_RepackBufSize       = 512 * 1024;
    const MMP_ULONG                 UVCE_VideoSizeTableSize  = 1024;
    const MMP_ULONG                 UVCE_VideoTimeTableSize  = 1024;
    #endif // UVC_EMERGRECD_SUPPORT
    #if (DUAL_EMERGRECD_SUPPORT)
    MMPD_3GPMGR_REPACKBUF           DualE_repackmiscbuf;
    const MMP_ULONG                 DualE_RepackBufSize       = 512 * 1024;
    const MMP_ULONG                 DualE_VideoSizeTableSize  = 1024;
    const MMP_ULONG                 DualE_VideoTimeTableSize  = 1024;
    #endif // DUAL_EMERGRECD_SUPPORT

    AUTL_MEMDBG_BLOCK               sVRRecdMemDbgBlk;
    MMP_UBYTE                       ubDbgItemIdx = 0;
    MMP_ULONG                       ulMemStart = 0;

    m_ulVidRecEncodeAddr = ulCurAddr = ulStackAddr;

    if (((usEncW>>4) * (usEncH>>4)) > 34560) { //4096x2160
        return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
    }

    AUTL_MemDbg_Init(&sVRRecdMemDbgBlk, AUTL_MEMDBG_USAGE_ID_VR_RECD);
    ulMemStart = ulCurAddr;

    // Allocate memory for MV (Motion Vector) buffer
    if ((m_ulVidShareMvAddr == 0) || (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))) {
        MMPS_3GPRECD_AllocateMVBuffer(  &ulCurAddr,
                                        m_VidRecdConfigs.usEncWidth[m_VidRecdModes.usVideoMVBufResIdx],
                                        m_VidRecdConfigs.usEncHeight[m_VidRecdModes.usVideoMVBufResIdx]);

        AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"Motion Vector");
        ulMemStart = ulCurAddr;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    //  Get Record Config setting and calculate how many memory will be used    
    //
    ///////////////////////////////////////////////////////////////////////////

    // Get Encode buffer config and size (NV12)
    if (m_VidRecdModes.VidCurBufMode[0] == VIDENC_CURBUF_RT) {
        m_VidRecdInputBuf[0].ulBufCnt = 2;
    }
    else {
        m_VidRecdInputBuf[0].ulBufCnt = 2 + VIDENC_MAX_B_FRAME_NUMS;
    }

    ulEncFrameSize = ALIGN32((usEncW * usEncH * 3) / 2);

    //////////////////////////////////////////////////////////////////////////
    //
    //  Allocate memory for record buffer
    //
    //////////////////////////////////////////////////////////////////////////
 
    //*************************
    // AV Sync ............ 32
    // Frame Table ........ 1k
    // Time Table ......... 1k
    // Audio BS ........... 16k
    //*************************

    // Set av sync buffer
    repackmiscbuf.ulVideoEncSyncAddr    = ulCurAddr;
    ulCurAddr                      		+= AVSyncBufSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam AV Sync");
    ulMemStart = ulCurAddr;

    // Set aux size table buffer, must be 512 byte alignment
    repackmiscbuf.ulVideoSizeTableAddr  = ulCurAddr;
    repackmiscbuf.ulVideoSizeTableSize  = VideoSizeTableSize;
    ulCurAddr                     		+= VideoSizeTableSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Size Table");
    ulMemStart = ulCurAddr;

    // Set aux time table buffer, must be 512 byte alignment
    repackmiscbuf.ulVideoTimeTableAddr  = ulCurAddr;
    repackmiscbuf.ulVideoTimeTableSize  = VideoTimeTableSize;
    ulCurAddr                      		+= VideoTimeTableSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Time Table");
    ulMemStart = ulCurAddr;

    // Set audio compressed buffer
    mergercompbuf.ulAudioCompBufStart   = ulCurAddr;
    ulCurAddr                           += AudioCompBufSize;
    mergercompbuf.ulAudioCompBufEnd     = ulCurAddr;
    
    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Audio Compress");
    ulMemStart = ulCurAddr;
    
    // Initialize H264 instance
    #if (H264ENC_ICOMP_TEST)
    if (MMPD_VIDENC_InitInstance(ulEncId, VIDENC_STREAMTYPE_VIDRECD, VIDENC_RC_MODE_CQP) != MMP_ERR_NONE)
    #else
    if (MMPD_VIDENC_InitInstance(ulEncId, VIDENC_STREAMTYPE_VIDRECD, VIDENC_RC_MODE_CBR) != MMP_ERR_NONE)
    #endif
    {
        RTNA_DBG_Str(0, "Err not available h264 instance\r\n");
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    m_VidRecdID = *ulEncId;

    // Set REF/GEN buffer
    ulCurAddr = ALIGN32(ulCurAddr);

    MMPD_H264ENC_CalculateRefBuf(usEncW, usEncH, &(videohwbuf.refgenbd), &ulCurAddr);

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Ref/Gen Buffer");
    ulMemStart = ulCurAddr;

    // Set current encoder buffer (NV12)
    if (m_VidRecdModes.VidCurBufMode[0] == VIDENC_CURBUF_RT) {
        for (i = 0; i < m_VidRecdInputBuf[0].ulBufCnt; i++) {
            m_VidRecdInputBuf[0].ulY[i] = ALIGN32(ulFBufAddr);
            ulFBufAddr += (usEncW << 4);
            m_VidRecdInputBuf[0].ulU[i] = ulFBufAddr;
            m_VidRecdInputBuf[0].ulV[i] = m_VidRecdInputBuf[0].ulU[i] + (usEncW << 2); // CHECK
            ulFBufAddr += (usEncW << 3);
        }
        MMPD_H264ENC_SetSourcePPBuf(&m_VidRecdInputBuf[0]);
    }
    else {
        //#if (USE_H264_CUR_BUF_AS_CAPT_BUF)
        // NOP
        //#else
        #if 0
        if (MMPF_JPEG_GetCtrlByQueueEnable() && m_VidRecdConfigs.bStillCaptureEnable == MMP_TRUE)
        #else
        if (!CAM_CHECK_PRM(PRM_CAM_NONE) && MMPF_JPEG_GetCtrlByQueueEnable() && m_VidRecdConfigs.bStillCaptureEnable == MMP_TRUE)
        #endif
        {
            // NOP. Allocate m_VidRecdInputBuf in MMPS_3GPRECD_SetPreviewMemory.
        }
        else {
            MMP_UBYTE*  pBuf = NULL;
            MMP_ULONG   ulTmpBufSize;
            MMP_USHORT  usMaxJpegW = 0, usMaxJpegH = 0;
            MMP_ULONG   *ulFbAddr;

            // Allocate memory for still capture buffer
            ulCurAddr = ALIGN32(ulCurAddr);
            if (m_ulVidRecSramAddr == 0) {
                MMPD_System_GetSramEndAddr(&m_ulVidRecSramAddr);
            }

            ulFbAddr = &m_ulVidRecSramAddr;
            m_ulVidRecCaptureDramAddr = ulCurAddr;
            m_ulVidRecCaptureSramAddr = *ulFbAddr;

            // Pre-calculating maximum memory usage
            MMPS_DSC_SetShotMode(MMPS_DSC_SINGLE_SHOTMODE);
            #if (DSC_SUPPORT_BASELINE_MP_FILE)
            MMPS_DSC_EnableMultiPicFormat(MMP_FALSE, MMP_FALSE);
            #endif
            usMaxJpegW = m_usMaxStillJpegW;
            usMaxJpegH = m_usMaxStillJpegH;

            MMPS_DSC_SetJpegEncParam(DSC_RESOL_IDX_UNDEF, usMaxJpegW, usMaxJpegH, MMP_DSC_JPEG_RC_ID_CAPTURE);

            #ifdef MCR_V2_32MB
            MMPS_DSC_ConfigThumbnail(0, 0, MMP_DSC_THUMB_SRC_NONE); // Disable Thumbnail to get 800 KB extra DRAM space
            #else
            MMPS_DSC_ConfigThumbnail(VR_MAX_THUMBNAIL_WIDTH, VR_MAX_THUMBNAIL_HEIGHT, MMP_DSC_THUMB_SRC_DECODED_JPEG);
            #endif
        
            MMPS_DSC_SetSystemBuf(&ulCurAddr, MMP_FALSE, MMP_TRUE, MMP_TRUE);
            MMPS_DSC_SetCaptureBuf(ulFbAddr, &ulCurAddr, MMP_TRUE, MMP_FALSE, MMP_DSC_CAPTURE_NO_ROTATE, MMP_TRUE);
            m_ulVidRecCaptureEndSramAddr = *ulFbAddr;
            m_ulVidRecCaptureEndDramAddr = ulCurAddr;
            ulMemStart = ulCurAddr;
            
            // Maximum memory usage for rear cam capture
            if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264) || \
                CAM_CHECK_USB(USB_CAM_SONIX_MJPEG)) {
              
                if (m_VidRecdConfigs.bDualCaptureEnable) {
                    ulCurAddr = ALIGN32(ulCurAddr);
                    m_ulVidRecRearCamCaptDramAddr = ulCurAddr;
                    ulCurAddr += m_ulVidRecRearCamCaptDramSize;
            
                    ulMemStart = ulCurAddr;
                }
            }            

            for (i = 0; i < m_VidRecdInputBuf[0].ulBufCnt; i++) {
                ulTmpBufSize = usEncW * usEncH;

                m_VidRecdInputBuf[0].ulY[i] = ALIGN32(ulCurAddr);
                m_VidRecdInputBuf[0].ulU[i] = m_VidRecdInputBuf[0].ulY[i] + ulTmpBufSize;
                m_VidRecdInputBuf[0].ulV[i] = m_VidRecdInputBuf[0].ulU[i];
                
                ulCurAddr += (ulTmpBufSize * 3)/2;
                
                // Workaround to reduce blur image in the bottom of frame
                memset((MMP_UBYTE*)m_VidRecdInputBuf[0].ulY[i], 0, (ulTmpBufSize * 3) / 2 );

                // Fill the last 8 line as black color
                pBuf = (MMP_UBYTE *)(m_VidRecdInputBuf[0].ulU[i] - (usEncW * 8));
                MEMSET(pBuf, 0x00, usEncW * 8);
                pBuf = (MMP_UBYTE *)(m_VidRecdInputBuf[0].ulU[i] + (usEncW * usEncH / 2) - (usEncW * 4));
                MEMSET(pBuf, 0x80, usEncW * 4);
            }
        }
        //#endif
    }

    #if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
    if (MMP_IsSupportDecMjpegToEncH264() && CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
        MMPS_3GPRECD_InitDecMjpegToEncode(&m_VidRecdInputBuf[0], *ulEncId);    
    #endif
    
    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Frame Buffer");
    ulMemStart = ulCurAddr;
    
    // Set Record pipe
    MMPS_3GPRECD_SetRecdPipeConfig(&m_VidRecdInputBuf[0], m_ulVREncScalOutW[0], m_ulVREncScalOutH[0], *ulEncId);

    // Set Slice Length Buffer, align32
    videohwbuf.miscbuf.ulSliceLenBuf    = ulCurAddr;
    ulCurAddr                        	= ulCurAddr + SliceLengBufSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Slice Length Buf");
    ulMemStart = ulCurAddr;

    // Set MV buffer (#MB/Frame * #MVs/MB * #byte/MV)
    ulCurAddr                           = ALIGN_X(ulCurAddr, 64);
    #if (DUALENC_SUPPORT)
    if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
        (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)) {
        videohwbuf.miscbuf.ulMVBuf      = m_ulVidShareMvAddr; // CHECK
    }
    else {
        videohwbuf.miscbuf.ulMVBuf      = ulCurAddr;
        ulCurAddr                       += MVBufSize;
    }
    #else
    videohwbuf.miscbuf.ulMVBuf          = ulCurAddr;
    ulCurAddr                           += MVBufSize;
    #endif

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam MV Buf");
    ulMemStart = ulCurAddr;

    // Set SPS buffer
    ulCurAddr = ALIGN32(ulCurAddr);

    headerbuf.ulSPSStart                = ulCurAddr;
    headerbuf.ulSPSSize                 = SPSSize;
    ulCurAddr                      		+= headerbuf.ulSPSSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam SPS");
    ulMemStart = ulCurAddr;    
    
    #if (SUPPORT_VUI_INFO)
    // Rebuild-SPS
    headerbuf.ulTmpSPSSize              = SPSSize + 16;
    headerbuf.ulTmpSPSStart             = ulCurAddr;
    ulCurAddr                      		+= headerbuf.ulTmpSPSSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Rebuild-SPS");
    ulMemStart = ulCurAddr; 
    #endif
    
    // Set PPS buffer
    headerbuf.ulPPSStart                = ulCurAddr;
    headerbuf.ulPPSSize                 = PPSSize;
    ulCurAddr                       	+= headerbuf.ulPPSSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam PPS");
    ulMemStart = ulCurAddr; 

    #if (UVC_VIDRECD_SUPPORT)
    if (m_bUVCRecdSupport) {
        // Set h264 parameter set buf
        ulCurAddr = ALIGN32(ulCurAddr);
        UVCheaderbuf.ulSPSSize          = SPSSize;
        UVCheaderbuf.ulPPSSize          = PPSSize;
        // SPS
        UVCheaderbuf.ulSPSStart         = ulCurAddr;
        ulCurAddr                      	+= UVCheaderbuf.ulSPSSize;
        // PPS
        UVCheaderbuf.ulPPSStart         = ulCurAddr;
        ulCurAddr                   	+= UVCheaderbuf.ulPPSSize;

        AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVC SPS/PPS");
        ulMemStart = ulCurAddr; 
    }
    #endif

	// Set video compressed buffer, 32 byte alignment
	ulCurAddr                           = ALIGN32(ulCurAddr);
	videohwbuf.bsbuf.ulStart            = ulCurAddr;
	mergercompbuf.ulVideoCompBufStart   = ulCurAddr;
	ulCurAddr                        	+= VideoCompBufSize;
	videohwbuf.bsbuf.ulEnd              = ulCurAddr;
	mergercompbuf.ulVideoCompBufEnd     = ulCurAddr;
	
	AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Video Compress");
	ulMemStart = ulCurAddr; 
	
	#if (UVC_VIDRECD_SUPPORT)
	if (m_bUVCRecdSupport) { 
		UVCmergercompbuf.ulVideoCompBufStart   = ulCurAddr;
		ulCurAddr += UVCVidCompBufSize;
		UVCmergercompbuf.ulVideoCompBufEnd     = ulCurAddr;

		AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVC Video Compress");
		ulMemStart = ulCurAddr; 
	}	
	#endif
        
	// Set av repack buffer
	repackmiscbuf.ulAvRepackStartAddr   = ulCurAddr;
	repackmiscbuf.ulAvRepackSize        = AVRepackBufSize;
	ulCurAddr                        	+= repackmiscbuf.ulAvRepackSize;
	
	AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam AV Repack");
	ulMemStart = ulCurAddr; 
	
	#if (UVC_VIDRECD_SUPPORT)
	if (m_bUVCRecdSupport) { 
		#if (VIDRECD_MULTI_TRACK == 0)
		UVCrepackmiscbuf.ulAvRepackStartAddr = ulCurAddr;
		UVCrepackmiscbuf.ulAvRepackSize      = UVCRepackBufSize;
		ulCurAddr                        	+= repackmiscbuf.ulAvRepackSize;

		AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVC AV Repack");
		ulMemStart = ulCurAddr; 

		// Set frame table buffer, must be 512 byte alignment
	   	UVCrepackmiscbuf.ulVideoSizeTableAddr  = ulCurAddr;
	   	UVCrepackmiscbuf.ulVideoSizeTableSize  = UVCVideoSizeTableSize;
	    ulCurAddr                             += UVCVideoSizeTableSize;

		AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVC Size Table");
		ulMemStart = ulCurAddr; 

	   	// Set time table buffer, must be 512 byte alignment
    	UVCrepackmiscbuf.ulVideoTimeTableAddr  = ulCurAddr;
	    UVCrepackmiscbuf.ulVideoTimeTableSize  = UVCVideoTimeTableSize;
    	ulCurAddr                             += UVCVideoTimeTableSize;

		AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVC Time Table");
		ulMemStart = ulCurAddr; 
		#endif
	}	
    #endif
	
	#if (UVC_EMERGRECD_SUPPORT) && (VIDRECD_MULTI_TRACK == 0)
	if (m_bUVCRecdSupport) { 
		#if (VIDRECD_MULTI_TRACK == 0)
		UVCE_repackmiscbuf.ulAvRepackStartAddr 	= ulCurAddr;
		UVCE_repackmiscbuf.ulAvRepackSize      	= UVCE_RepackBufSize;
		ulCurAddr                           	+= UVCE_repackmiscbuf.ulAvRepackSize;

		AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVCE AV Repack");
		ulMemStart = ulCurAddr; 
		
		// Set frame table buffer, must be 512 byte alignment
	   	UVCE_repackmiscbuf.ulVideoSizeTableAddr  	= ulCurAddr;
	   	UVCE_repackmiscbuf.ulVideoSizeTableSize  	= UVCE_VideoSizeTableSize;
	    ulCurAddr                                  += UVCE_VideoSizeTableSize;

		AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVCE Size Table");
		ulMemStart = ulCurAddr; 

	   	// Set time table buffer, must be 512 byte alignment
    	UVCE_repackmiscbuf.ulVideoTimeTableAddr  	= ulCurAddr;
	    UVCE_repackmiscbuf.ulVideoTimeTableSize  	= UVCE_VideoTimeTableSize;
    	ulCurAddr                                  += UVCE_VideoTimeTableSize;  	

		AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVCE Time Table");
		ulMemStart = ulCurAddr; 
		#endif
	}	
	#endif 
    #if (DUAL_EMERGRECD_SUPPORT) && (VIDRECD_MULTI_TRACK == 0)
    if (1) { //(m_bUVCRecdSupport) { 
        #if (VIDRECD_MULTI_TRACK == 0)
        DualE_repackmiscbuf.ulAvRepackStartAddr 	= ulCurAddr;
        DualE_repackmiscbuf.ulAvRepackSize      	= DualE_RepackBufSize;
        ulCurAddr                           	   += DualE_repackmiscbuf.ulAvRepackSize;
        
        AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam DualE AV Repack");
        ulMemStart = ulCurAddr; 
        
        // Set frame table buffer, must be 512 byte alignment
       	DualE_repackmiscbuf.ulVideoSizeTableAddr  	= ulCurAddr;
       	DualE_repackmiscbuf.ulVideoSizeTableSize  	= DualE_VideoSizeTableSize;
        ulCurAddr                                  += DualE_VideoSizeTableSize;
        
        AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam DualE Size Table");
        ulMemStart = ulCurAddr; 

        // Set time table buffer, must be 512 byte alignment
        DualE_repackmiscbuf.ulVideoTimeTableAddr  	= ulCurAddr;
        DualE_repackmiscbuf.ulVideoTimeTableSize  	= DualE_VideoTimeTableSize;
        ulCurAddr                                  += DualE_VideoTimeTableSize;  	

        AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam DualE Time Table");
        ulMemStart = ulCurAddr; 
        #endif
    }
    #endif 
    
    MMPD_3GPMGR_SetRepackMiscBuf(VIDENC_STREAMTYPE_VIDRECD, &repackmiscbuf);
    MMPD_3GPMGR_SetEncodeCompBuf(VIDENC_STREAMTYPE_VIDRECD, &mergercompbuf);
    
    #if (UVC_VIDRECD_SUPPORT)
    if (m_bUVCRecdSupport) { 
        MMPD_3GPMGR_SetEncodeCompBuf(VIDENC_STREAMTYPE_UVCRECD, &UVCmergercompbuf);
        #if (VIDRECD_MULTI_TRACK == 0)
        MMPD_UVCRECD_SetRepackMiscBuf(VIDENC_STREAMTYPE_UVCRECD, &UVCrepackmiscbuf);
        #endif
        MMPD_H264ENC_SetUVCHdrBuf(&UVCheaderbuf);
    }
    #endif
    
    #if (UVC_EMERGRECD_SUPPORT)
    if (m_bUVCRecdSupport) { 
        #if (VIDRECD_MULTI_TRACK == 0)
        MMPD_UVCRECD_SetRepackMiscBuf(VIDENC_STREAMTYPE_UVCEMERG, &UVCE_repackmiscbuf);
        #endif
    }
    #endif
    #if (DUAL_EMERGRECD_SUPPORT)
    if (1) {
        #if (VIDRECD_MULTI_TRACK == 0)
        MMPD_3GPMGR_SetRepackMiscBuf(VIDENC_STREAMTYPE_DUALEMERG, &DualE_repackmiscbuf);
        #endif
    }
    #endif

    MMPD_H264ENC_SetHeaderBuf(*ulEncId, &headerbuf);
    MMPD_H264ENC_SetBitstreamBuf(*ulEncId, &(videohwbuf.bsbuf));
    MMPD_H264ENC_SetRefGenBound(*ulEncId, &(videohwbuf.refgenbd));
    MMPD_H264ENC_SetMiscBuf(*ulEncId, &(videohwbuf.miscbuf));
    
    MMPD_AUDIO_SetEncodeBuf(mergercompbuf.ulAudioCompBufStart,
                           (mergercompbuf.ulAudioCompBufEnd - mergercompbuf.ulAudioCompBufStart));
    
    MMPS_System_GetPreviewFrameStart(&bufferEndAddr);

    // Set front-cam tail info buffer
    {
        MMP_ULONG ulTailBufSize;

        if (m_VidRecdConfigs.ulTailBufSize)
            ulTailBufSize = m_VidRecdConfigs.ulTailBufSize;
        else
            ulTailBufSize = MMPS_3GPRECD_GetContainerTailBufSize();

        ulCurAddr = ALIGN16(ulCurAddr);
        
        ulTailBufSize = ALIGN32(ulTailBufSize);
        #if (EMERGENTRECD_SUPPORT)
        ulTailBufSize <<= 2;
        #endif

        if ((ulCurAddr + ulTailBufSize) > bufferEndAddr) {
            PRINTF("Can't use high speed videor\r\n");
            MMPD_3GPMGR_SetRecordTailSpeed(VIDENC_STREAMTYPE_VIDRECD, MMP_FALSE, 0, 0);
        }
        else {
            PRINTF("USE AIT high speed videor\r\n");
            MMPD_3GPMGR_SetRecordTailSpeed(VIDENC_STREAMTYPE_VIDRECD, MMP_TRUE, ulCurAddr, ulTailBufSize);
            ulCurAddr += ulTailBufSize;
            m_ulVidRecDramEndAddr = ulCurAddr; 
        }

        AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"FCam Tail Info");
        ulMemStart = ulCurAddr;
    }

    #if (UVC_VIDRECD_SUPPORT) && (VIDRECD_MULTI_TRACK == 0)
    // Set rear-cam tail info  buffer
	if (m_bUVCRecdSupport) { 
		
		ulCurAddr = ALIGN16(ulCurAddr);
		
		if ((ulCurAddr + m_VidRecdConfigs.ulUVCTailBufSize) > bufferEndAddr) {
			PRINTF("Can't use high speed videor for UVC recording\r\n");
			MMPD_3GPMGR_SetRecordTailSpeed(VIDENC_STREAMTYPE_UVCRECD, MMP_FALSE, 0, 0);
		}
		else {
			PRINTF("USE AIT high speed videor for UVC recording\r\n");
			MMPD_3GPMGR_SetRecordTailSpeed(VIDENC_STREAMTYPE_UVCRECD, MMP_TRUE, ulCurAddr, m_VidRecdConfigs.ulUVCTailBufSize);
			ulCurAddr += m_VidRecdConfigs.ulUVCTailBufSize;
			m_ulVidRecDramEndAddr = ulCurAddr; 
        }

		AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVC Tail Info");
		ulMemStart = ulCurAddr;
    }
    #endif
    
    #if (UVC_EMERGRECD_SUPPORT == 1) && (VIDRECD_MULTI_TRACK == 0)
    // Set rear-cam tail info buffer
	if (m_bUVCRecdSupport) { 
		
		ulCurAddr = ALIGN16(ulCurAddr); // not necessary, but ALIGN4 is must
		
		if ((ulCurAddr + m_VidRecdConfigs.ulUVCEmergTailBufSize) > bufferEndAddr) {
			PRINTF("Can't use high speed videor for UVC recording\r\n");
			MMPD_3GPMGR_SetRecordTailSpeed(VIDENC_STREAMTYPE_UVCEMERG, MMP_FALSE, 0, 0);
		}
		else {
			PRINTF("USE AIT high speed videor for UVC recording\r\n");
			MMPD_3GPMGR_SetRecordTailSpeed(VIDENC_STREAMTYPE_UVCEMERG, MMP_TRUE, ulCurAddr, m_VidRecdConfigs.ulUVCEmergTailBufSize);
			ulCurAddr += m_VidRecdConfigs.ulUVCEmergTailBufSize;
			m_ulVidRecDramEndAddr = ulCurAddr; 
        }

		AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVCE Tail Info");
		ulMemStart = ulCurAddr;
    }
    #endif   
    #if (DUAL_EMERGRECD_SUPPORT == 1) && (VIDRECD_MULTI_TRACK == 0)
    // Set rear-cam tail info buffer
    if (1) { //(m_bUVCRecdSupport) { 
        
        ulCurAddr = ALIGN16(ulCurAddr); // not necessary, but ALIGN4 is must
        
        if ((ulCurAddr + m_VidRecdConfigs.ulDualEmergTailBufSize) > bufferEndAddr) {
            PRINTF("Can't use high speed videor for DualE recording\r\n");
            MMPD_3GPMGR_SetRecordTailSpeed(VIDENC_STREAMTYPE_DUALEMERG, MMP_FALSE, 0, 0);
        }
        else {
            PRINTF("USE AIT high speed videor for DualE recording\r\n");
            MMPD_3GPMGR_SetRecordTailSpeed(VIDENC_STREAMTYPE_DUALEMERG, MMP_TRUE, ulCurAddr, m_VidRecdConfigs.ulDualEmergTailBufSize);
            ulCurAddr += m_VidRecdConfigs.ulDualEmergTailBufSize;
            m_ulVidRecDramEndAddr = ulCurAddr; 
        }
        
        AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam UVCE Tail Info");
        ulMemStart = ulCurAddr;
    }
    #endif
    
    #if (SUPPORT_VR_THUMBNAIL)
    // Set video thumbnail buffer
    if (MMPS_3GPRECD_GetVRThumbnailSts()) {
        
        MMP_ULONG  ThumbJpegSize;
        MMP_UBYTE  i, j;
        
        ulCurAddr = ALIGN32(ulCurAddr);
        
        ThumbJpegSize = ALIGN32((m_ulVRThumbWidth * m_ulVRThumbHeight * 4) >> 1);
        gulVRThumbMaxBufSize = ThumbJpegSize;
        
        for (i = 0; i < VIDENC_THUMB_MAX_TYPE; i++) {
            for (j = 0; j < gusVRThumbBufNum[i]; j++) { 
                gsVRThumbAttr[i][j].uladdr = ulCurAddr + (ThumbJpegSize * j);
            }
            ulCurAddr += (gusVRThumbBufNum[i] * ThumbJpegSize);
        }
     
        for (i = VIDENC_THUMB_RING_BUF_FRONT; i <= VIDENC_THUMB_RING_BUF_DUAL; i++) {
            for (j = 0; j < gusVRThumbBufNum[i]; j++) {
                gsVRThumbAttr[i][j].ulsize = 0x00000000;
                gsVRThumbAttr[i][j].ulprog_cnt = INVALID_THUMB_PROG_CNT;
                gsVRThumbAttr[i][j].ulidx = 0x00000000;
            }
        }
        
        AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"VR Thumbnail");
        ulMemStart = ulCurAddr;
    }
    #endif

    m_ulVidRecDramEndAddr = ulCurAddr;

    AUTL_MemDbg_ShowAllItems(&sVRRecdMemDbgBlk, MMP_TRUE);
    
    #if defined(ALL_FW)
    if (m_ulVidRecDramEndAddr > MMPS_System_GetMemHeapEnd()) {
        printc("\t= [HeapMemErr] Video record = %x, %x\r\n", m_ulVidRecDramEndAddr, MMPS_System_GetMemHeapEnd());
        return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
    }
    printc("End of video record buffers = 0x%X\r\n", m_ulVidRecDramEndAddr);
    #endif

    #if (SUPPORT_COMPONENT_FLOW_CTL)
    if (CAM_CHECK_PRM(PRM_CAM_BAYER_SENSOR)) {
        
        if (m_ubCamEncodeListId[PRM_SENSOR] == 0xFF) {
            
            MMP_UBYTE ubVifId   = MMPF_Sensor_GetVIFPad(m_VREncodeFctlAttr[0].ubPipeLinkedSnr);
            MMP_UBYTE ubPipeId  = m_VREncodeFctlAttr[0].fctllink.ibcpipeID;
            MMP_UBYTE ubCompIdArray[6];

            ubCompIdArray[0] = MMP_COMPONENT_ID_NULL;
            ubCompIdArray[1] = MMP_COMPONENT_ID_VIF2ISP;
            ubCompIdArray[2] = TRANS_PIPE_TO_PIPECOMP_ID(ubPipeId);
            ubCompIdArray[3] = TRANS_PIPE_TO_DRAMCOMP_ID(ubPipeId);
            ubCompIdArray[4] = MMP_COMPONENT_ID_H264ENC;
            ubCompIdArray[5] = MMP_COMPONENT_ID_NULL;

            if (MMP_FALSE == m_VidRecdConfigs.bRawPreviewEnable[0])
                MMP_CompCtl_RegisterVif2IspComponent(MMP_COMPONENT_USAGE_ID0, m_VREncodeFctlAttr[0].ubPipeLinkedSnr, ubVifId);
            else
                MMP_CompCtl_RegisterRaw2IspComponent(MMP_COMPONENT_USAGE_ID0, m_VREncodeFctlAttr[0].ubPipeLinkedSnr, ubVifId);
            
            MMP_CompCtl_RegisterPipeComponent(MMP_COMPONENT_USAGE_ID0, (void*)&m_VREncodeFctlAttr[0]);
            MMP_CompCtl_RegisterPipeOutDramComponent(MMP_COMPONENT_USAGE_ID0, (void*)&m_VREncodeFctlAttr[0]);
            MMP_CompCtl_RegisterH264Component(MMP_COMPONENT_USAGE_ID0, (void*)&m_VREncodeFctlAttr[0]);
            
            m_ubCamEncodeListId[PRM_SENSOR] = MMP_CompCtl_LinkComponentsEx(MMP_COMPONENT_USAGE_ID0, &ubCompIdArray[0], 6, NULL);
        }
    }
    #endif

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetParameter
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Return parameter with specific type.
 @param[in] SetParam Specific parameter.
 @param[out] usValue Parameter value.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetParameter(MMPS_3GPRECD_PARAMETER SetParam, MMP_ULONG *ulValue)
{
    MMP_ULONG ulParam;

    switch (SetParam) {
    case MMPS_3GPRECD_PARAMETER_PREVIEWMODE:
        ulParam = m_VidRecdModes.usVideoPreviewMode;
        break;
    case MMPS_3GPRECD_PARAMETER_SRCMODE:
        ulParam = m_VidRecdModes.VideoSrcMode[0];
        break;
    case MMPS_3GPRECD_PARAMETER_VIDEO_FORMAT:
        ulParam = MMPS_3GPRECD_VIDEO_FORMAT_H264;
        break;
    case MMPS_3GPRECD_PARAMETER_AUDIO_FORMAT:
        switch (MMPD_3GPMGR_GetAudioFormat()) {
        case MMPD_3GPMGR_AUDIO_FORMAT_AAC:
            ulParam = MMPS_3GPRECD_AUDIO_FORMAT_AAC;
            break;
        case MMPD_3GPMGR_AUDIO_FORMAT_AMR:
            ulParam = MMPS_3GPRECD_AUDIO_FORMAT_AMR;
            break;
        case MMPD_3GPMGR_AUDIO_FORMAT_ADPCM:
            ulParam = MMPS_3GPRECD_AUDIO_FORMAT_ADPCM;
            break;
        case MMPD_3GPMGR_AUDIO_FORMAT_MP3:
            ulParam = MMPS_3GPRECD_AUDIO_FORMAT_MP3;
            break;
        case MMPD_3GPMGR_AUDIO_FORMAT_PCM:
            ulParam = MMPS_3GPRECD_AUDIO_FORMAT_PCM;
            break;
        default:
            ulParam = 0xFFFFFFFF;
            break;
        }
        break;
    case MMPS_3GPRECD_PARAMETER_RESOLUTION:
        ulParam = m_VidRecdModes.usVideoEncResIdx[0];
        break;
    case MMPS_3GPRECD_PARAMETER_BITRATE:
        ulParam = m_VidRecdModes.ulBitrate[0];
        break;
    case MMPS_3GPRECD_PARAMETER_FRAME_RATE:
        ulParam = (m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrement << 16) |
                   m_VidRecdModes.VideoEncFrameRate[0].usVopTimeIncrResol;
        break;
    case MMPS_3GPRECD_PARAMETER_PROFILE:
        ulParam = m_VidRecdModes.VisualProfile[0];
        break;
    case MMPS_3GPRECD_PARAMETER_DRAM_END:
        // Can not get correct end address of allocated memory until start record!
        // When preview start, the value specifies the allocated memory for preview only,
        // encoder buffers are allocated in starting record.
        ulParam = m_ulVidRecDramEndAddr;
        break;
    default:
        ulParam = 0xFFFFFFFF;
        break;
    }

    *ulValue = ulParam;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetFrame
//  Description :
//------------------------------------------------------------------------------
/**
@brief Return the last video preview frame in RGB format
@param[in] usTargetW The width of the requested frame.
@param[in] usTargetH The height of the requested frame.
@param[out] pusOutBuf Pointer to the output buffer.
@param[in,out] ulSize The size of output buffer which contains requested frame.
@retval MMP_ERR_NONE Success.
@retval MMP_MP4VE_ERR_NOT_SUPPORTED_PARAMETERS No support input parameter
*/
MMP_ERR MMPS_3GPRECD_GetFrame(MMP_USHORT usTargetW, MMP_USHORT usTargetH, MMP_USHORT *pusOutBuf, MMP_ULONG *pulSize)
{
    MMP_GRAPHICS_BUF_ATTR   srcBufAttr, dstBufAttr;
    MMP_GRAPHICS_RECT       rect;
    MMP_PIPE_LINK           fctllink;
    MMPD_FCTL_ATTR          fctlAttr;
    
    // For H264 frame mode only
    if (m_VidRecdModes.VidCurBufMode[0] != VIDENC_CURBUF_FRAME) {
        return MMP_ERR_NONE;
    }

    MMPD_Fctl_GetAttributes(m_RecordFctlLink.ibcpipeID, &fctlAttr);

    fctllink = fctlAttr.fctllink;
          
    srcBufAttr.usWidth  	= m_ulVREncodeW[0];
    srcBufAttr.usHeight 	= m_ulVREncodeH[0];
    srcBufAttr.usLineOffset = srcBufAttr.usWidth;
    srcBufAttr.colordepth   = MMP_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE;
    srcBufAttr.ulBaseAddr  	= fctlAttr.ulBaseAddr[0];
    srcBufAttr.ulBaseUAddr 	= fctlAttr.ulBaseUAddr[0];
    srcBufAttr.ulBaseVAddr 	= fctlAttr.ulBaseVAddr[0];

    rect.usLeft   			= 0;
    rect.usTop    			= 0;
    rect.usWidth  			= srcBufAttr.usWidth;
    rect.usHeight 			= srcBufAttr.usHeight;

    *pulSize = usTargetW * usTargetH * 2;
    
    // Be careful that destination address will overwrite the using buffer.
    if (*pulSize <= (srcBufAttr.usWidth * srcBufAttr.usHeight * 3 / 2)) {
        if (fctlAttr.usBufCnt > 1) {
            dstBufAttr.ulBaseAddr = fctlAttr.ulBaseAddr[1];
        }
        else if ((fctlAttr.usBufCnt == 1) && (fctlAttr.usRotateBufCnt > 0)) {
            dstBufAttr.ulBaseAddr = fctlAttr.ulRotateAddr[0];
        }
    }
    else {

        dstBufAttr.ulBaseAddr = m_ulVideoPreviewEndAddr;
    }

    dstBufAttr.ulBaseUAddr  = 0;
    dstBufAttr.ulBaseVAddr  = 0;
    dstBufAttr.usWidth      = usTargetW;
    dstBufAttr.usHeight     = usTargetH;
    dstBufAttr.usLineOffset = usTargetW * 2;
    dstBufAttr.colordepth   = MMP_GRAPHICS_COLORDEPTH_16; // RGB565

    if (MMPD_Fctl_RawBuf2IbcBuf(&fctllink, &srcBufAttr, &rect, &dstBufAttr, 1)) {
        return MMP_3GPRECD_ERR_PARAMETER;
    }

    // Copy the image back to the host
    if (MMPD_DMA_MoveData(dstBufAttr.ulBaseAddr, (MMP_ULONG)pusOutBuf, *pulSize, NULL, NULL)) {
        return MMP_3GPRECD_ERR_PARAMETER;
    }

    return MMP_ERR_NONE;
}

#if (SUPPORT_VUI_INFO)
//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetSEIShutterMode
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetSEIShutterMode(MMPS_SEI_SHUTTER_TYPE ulMode)
{
	return MMPD_3GPMGR_SetSEIShutterMode((MMP_ULONG)ulMode);
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_Get3gpRecordingOffset
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get recording time offset.
 @param[out] ulTime Recording time offset in unit of ms.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_Get3gpRecordingOffset(MMP_ULONG *ulTime)
{
    return MMPD_3GPMGR_GetRecordingOffset(VIDENC_STREAMTYPE_VIDRECD, ulTime);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetAllEncPreRecordTime
//  Description :
//------------------------------------------------------------------------------
/**
 @brief For dual encode. Get all preencode time and select the smaller.

 Get all encode stream preencode time and select the smaller.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_GetAllEncPreRecordTime(MMP_ULONG ulPreCaptureMs, MMP_ULONG *ulRealPreCaptureMs)
{
    MMP_ULONG64 ullBitrate64 		= 0;
    MMP_ULONG64 ullTimeIncr64 		= 0;
    MMP_ULONG64 ullTimeResol64 		= 0;
    MMP_ULONG   ulTargetFrameSize 	= 0;
    MMP_ULONG   ulExpectBufSize 	= 0;
    MMP_ULONG   ulFrontMaxPreEncMs 	= 0;
    #if (EMERGENTRECD_SUPPORT)
    MMP_ULONG   ulMaxAudPreEncMs 	= 0;
    #endif
	MMP_ULONG   ulRearMaxPreEncMs 	= 0;
	MMP_ULONG   ulMaxPreEncMs 		= 0;
	#if (UVC_VIDRECD_SUPPORT && SUPPORT_USB_HOST_FUNC)
    MMPF_USBH_UVC_STREAM_CFG    *pUVCCfg    = MMPF_USBH_GetUVCCFG();
	#endif

	// Check front cam record information.
    ullBitrate64 		= m_VidRecdModes.ulBitrate[0];
    ullTimeIncr64 		= m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrement;
    ullTimeResol64 		= m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrResol;
    ulTargetFrameSize 	= (MMP_ULONG)((ullBitrate64 * ullTimeIncr64)/(ullTimeResol64 * 8));	
    ulExpectBufSize     = m_VidRecdConfigs.ulVideoCompBufSize - (ulTargetFrameSize * 3);

    #if (EMERGENTRECD_SUPPORT)
    ulMaxAudPreEncMs    = (m_VidRecdConfigs.ulAudioCompBufSize) / (m_VidRecdModes.ulAudBitrate >> 3) * 1000 - 1000;    
    ulFrontMaxPreEncMs  = (MMP_ULONG)(((MMP_ULONG64)ulExpectBufSize * 1000) / (m_VidRecdModes.ulBitrate[0] >> 3)) - 2000;
    #else
    ulFrontMaxPreEncMs  = (MMP_ULONG)(((MMP_ULONG64)ulExpectBufSize * 900) / (m_VidRecdModes.ulBitrate[0] >> 3));
    #endif
    
    printc("FrontCam PreEncode: %x, %x, %x, %x\r\n", ulFrontMaxPreEncMs, ulExpectBufSize, m_VidRecdModes.ulBitrate[0], ulTargetFrameSize);

	ulMaxPreEncMs = ulFrontMaxPreEncMs;

    #if (EMERGENTRECD_SUPPORT)
    if (ulMaxPreEncMs > ulMaxAudPreEncMs) {
        ulMaxPreEncMs = ulMaxAudPreEncMs;
    }
    #endif
    
    #if (UVC_VIDRECD_SUPPORT && SUPPORT_USB_HOST_FUNC)
    // Check rear cam record information.    
    if (((MMP_IsUSBCamExist() && MMP_IsUSBCamIsoMode()) || (MMP_IsScdCamExist())) && m_VidRecdModes.ulBitrate[1])
    {
        ullBitrate64 		= m_VidRecdModes.ulBitrate[1];
        ullTimeIncr64 		= m_VidRecdModes.ContainerFrameRate[1].usVopTimeIncrement;
        ullTimeResol64 		= m_VidRecdModes.ContainerFrameRate[1].usVopTimeIncrResol;
        ulTargetFrameSize 	= (MMP_ULONG)((ullBitrate64 * ullTimeIncr64)/(ullTimeResol64 * 8));	
        ulExpectBufSize     = m_VidRecdConfigs.ulVideoCompBufSize - (ulTargetFrameSize * 3); // CHECK : Not use ul2ndVideoCompBufSize ?

        #if (EMERGENTRECD_SUPPORT)
        ulRearMaxPreEncMs   = (MMP_ULONG)(((MMP_ULONG64)ulExpectBufSize * 1000) / (ullBitrate64 >> 3)) - 1000;
        #else
        ulRearMaxPreEncMs   = (MMP_ULONG)(((MMP_ULONG64)ulExpectBufSize * 900) / (ullBitrate64 >> 3));
        #endif
    
        printc("USBCam PreEncode: %x, %x, %x, %x\r\n", ulRearMaxPreEncMs, ulExpectBufSize, m_VidRecdModes.ulBitrate[1], ulTargetFrameSize);

        if (ulRearMaxPreEncMs > ulFrontMaxPreEncMs) {
            ulMaxPreEncMs = ulFrontMaxPreEncMs;
        }
        else {
            ulMaxPreEncMs = ulRearMaxPreEncMs;
        }		
    }

    if (MMP_IsUSBCamExist() && !MMP_IsUSBCamIsoMode() && pUVCCfg->mRecd.ulBitrate)
    {
        ullBitrate64 		= pUVCCfg->mRecd.ulBitrate;
        ullTimeIncr64 		= pUVCCfg->mRecd.usTimeIncrement; //m_VidRecdModes.ContainerFrameRate[1].usVopTimeIncrement;
        ullTimeResol64 		= pUVCCfg->mRecd.usTimeIncrResol; //m_VidRecdModes.ContainerFrameRate[1].usVopTimeIncrResol;
        ulTargetFrameSize 	= (MMP_ULONG)((ullBitrate64 * ullTimeIncr64)/(ullTimeResol64 * 8));	
        ulExpectBufSize     = m_VidRecdConfigs.ulVideoCompBufSize - (ulTargetFrameSize * 3); // CHECK : Not use ul2ndVideoCompBufSize ?

        #if (EMERGENTRECD_SUPPORT)
        ulRearMaxPreEncMs   = (MMP_ULONG)(((MMP_ULONG64)ulExpectBufSize * 1000) / (ullBitrate64 >> 3)) - 1000;
        #else
        ulRearMaxPreEncMs   = (MMP_ULONG)(((MMP_ULONG64)ulExpectBufSize * 900) / (ullBitrate64 >> 3));
        #endif
    
        printc("USBCam PreEncode: %x, %x, %x, %x\r\n", ulRearMaxPreEncMs, ulExpectBufSize, m_VidRecdModes.ulBitrate[1], ulTargetFrameSize);

        if (ulRearMaxPreEncMs > ulFrontMaxPreEncMs) {
            ulMaxPreEncMs = ulFrontMaxPreEncMs;
        }
        else {
            ulMaxPreEncMs = ulRearMaxPreEncMs;
        }		
    }
    #endif
    
    #if (EMERGENTRECD_SUPPORT)
    if (ulMaxPreEncMs > ulMaxAudPreEncMs) {
        ulMaxPreEncMs = ulMaxAudPreEncMs;
    }
    #endif
    
    if (ulPreCaptureMs > ulMaxPreEncMs) {
        PRINTF("The pre-record duration %d is over preferred %d ms\r\n", ulPreCaptureMs, ulMaxPreEncMs);
        ulPreCaptureMs = ulMaxPreEncMs;
    }
    
    *ulRealPreCaptureMs = ulPreCaptureMs;
    
    return MMP_ERR_NONE;
}

#if 0
void ____VR_2nd_Record_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_EnableDualRecd
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Enable dual video recording.

 Enable emergent video recording.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_EnableDualRecd(MMP_BOOL bEnable)
{
    m_bDualEncEnable = bEnable;

    #if (DUALENC_SUPPORT)
    MMPD_3GPMGR_EnableDualRecd(bEnable);
    #endif
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetDualH264SnrId
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetDualH264SnrId(MMP_UBYTE ubSnrSel)
{
    m_ub2ndVRStreamSnrId = ubSnrSel;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetDualH264PipeConfig
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set video record pipe configuration.
 @param[in] *pInputBuf 		Pointer to HW used buffer.
 @param[in] usEncInputW 	The encode buffer width (for scaler out stage).
 @param[in] usEncInputH 	The encode buffer height (for scaler out stage).
 @param[in] ubEncId         Encode instance ID.
 @retval MMP_ERR_NONE Success.
*/
static MMP_ERR MMPS_3GPRECD_SetDualH264PipeConfig(  VIDENC_INPUT_BUF 	*pInputBuf,
                                                    MMP_USHORT			usEncInputW,
                                                    MMP_USHORT			usEncInputH,
                                                    MMP_USHORT          ubEncId)
{
    MMP_ULONG				ulScalInW, ulScalInH;
    MMP_SCAL_FIT_MODE		sFitMode;
    MMP_SCAL_FIT_RANGE  	fitrange;
    MMP_SCAL_GRAB_CTRL  	EncodeGrabctl;
    MMPD_FCTL_ATTR 			fctlAttr;
    MMP_USHORT				i;
    MMP_SCAL_FIT_RANGE 		sFitRangeBayer;
    MMP_SCAL_GRAB_CTRL		sGrabctlBayer;
    MMP_USHORT				usCurZoomStep = 0;

    /* Parameter Check */
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    if (pInputBuf == NULL) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    /* Get the sensor parameters */
    MMPS_Sensor_GetCurPrevScalInputRes(m_ub2ndVRStreamSnrId, &ulScalInW, &ulScalInH);

    MMPD_BayerScaler_GetZoomInfo(MMP_BAYER_SCAL_DOWN, &sFitRangeBayer, &sGrabctlBayer);

    if (m_sAhcVideoRecdInfo[1].bUserDefine) {
        sFitMode = m_sAhcVideoRecdInfo[1].sFitMode;
    }
    else {
        sFitMode = MMP_SCAL_FITMODE_OUT;
    }

    /* Initial zoom relative config */
    MMPS_3GPRECD_InitDigitalZoomParam(m_2ndRecFctlLink.scalerpath);

    MMPS_3GPRECD_RestoreDigitalZoomRange(m_2ndRecFctlLink.scalerpath);
    
    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE)
    {
        // Config Video Record Pipe
        fitrange.fitmode        = sFitMode;
        fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;

        if (MMP_IsVidPtzEnable()) {
            fitrange.ulInWidth  = sFitRangeBayer.ulOutWidth;
            fitrange.ulInHeight = sFitRangeBayer.ulOutHeight;
        }
        else {
            fitrange.ulInWidth  = ulScalInW;
            fitrange.ulInHeight = ulScalInH;
        }

        fitrange.ulOutWidth     = usEncInputW;
        fitrange.ulOutHeight    = usEncInputH;

        fitrange.ulInGrabX      = 1;
        fitrange.ulInGrabY      = 1;
        fitrange.ulInGrabW      = fitrange.ulInWidth;
        fitrange.ulInGrabH      = fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        
        MMPD_PTZ_InitPtzInfo(m_2ndRecFctlLink.scalerpath,
                             fitrange.fitmode,
                             fitrange.ulInWidth, fitrange.ulInHeight,
                             fitrange.ulOutWidth, fitrange.ulOutHeight);

        // Be sync with preview path : TBD
        if (m_ub2ndVRStreamSnrId == PRM_SENSOR) {
            MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
        }
        else if (m_ub2ndVRStreamSnrId == SCD_SENSOR && MMP_IsDualVifCamEnable()) {
            if (MMP_GetDualSnrPrevwType() == DUALSNR_DUAL_PREVIEW)
                MMPD_PTZ_GetCurPtzStep(m_2ndPrewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
            else
                MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
        }

        if (usCurZoomStep > m_VidRecordZoomInfo.usMaxZoomSteps) {
            usCurZoomStep = m_VidRecordZoomInfo.usMaxZoomSteps;
        }

        MMPD_PTZ_CalculatePtzInfo(m_2ndRecFctlLink.scalerpath, usCurZoomStep, 0, 0);

        MMPD_PTZ_GetCurPtzInfo(m_2ndRecFctlLink.scalerpath, &fitrange, &EncodeGrabctl);
        
        if (MMP_IsVidPtzEnable()) {
            MMPD_PTZ_ReCalculateGrabRange(&fitrange, &EncodeGrabctl);
        }
        
        fctlAttr.colormode          = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
        fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
        fctlAttr.fctllink           = m_2ndRecFctlLink;
        fctlAttr.fitrange           = fitrange;
        fctlAttr.grabctl            = EncodeGrabctl;
        
        if (m_ub2ndVRStreamSnrId == PRM_SENSOR) {
            fctlAttr.scalsrc        = MMP_SCAL_SOURCE_ISP;
        }
        else if (m_ub2ndVRStreamSnrId == SCD_SENSOR) {
            if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) || 
                (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
                fctlAttr.scalsrc    = MMP_SCAL_SOURCE_GRA;
            }
            else if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
                fctlAttr.scalsrc    = MMP_SCAL_SOURCE_ISP;
            }
        }

        fctlAttr.bSetScalerSrc      = MMP_TRUE;
        fctlAttr.ubPipeLinkedSnr    = m_ub2ndVRStreamSnrId;
        fctlAttr.usBufCnt           = pInputBuf->ulBufCnt;
        fctlAttr.bUseRotateDMA      = MMP_FALSE;

        for (i = 0; i < fctlAttr.usBufCnt; i++) {
            if (pInputBuf->ulY[i] != 0)
                fctlAttr.ulBaseAddr[i] = pInputBuf->ulY[i];
            if (pInputBuf->ulU[i] != 0)
                fctlAttr.ulBaseUAddr[i] = pInputBuf->ulU[i];
            if (pInputBuf->ulV[i] != 0)
                fctlAttr.ulBaseVAddr[i] = pInputBuf->ulV[i];
        }

        if (m_VidRecdModes.VidCurBufMode[1] == VIDENC_CURBUF_RT) {
            fctlAttr.bRtModeOut = MMP_TRUE;
            MMPD_Fctl_SetPipeAttrForH264Rt(&fctlAttr, usEncInputW);
        }
        else {
            fctlAttr.bRtModeOut = MMP_FALSE;
            fctlAttr.sScalDelay = m_sFullSpeedScalDelay;
            MMPD_Fctl_SetPipeAttrForH264FB(&fctlAttr);
        }
        
        MMPD_Fctl_LinkPipeToVideo(m_2ndRecFctlLink.ibcpipeID, ubEncId);
    }
    else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE  ||
             m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI   ||
             m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE)
    {
        // Config pipe for Encode
        fitrange.fitmode        = MMP_SCAL_FITMODE_OUT;
        fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;
        
        if (MMP_IsDualVifCamEnable() && MMP_GetParallelFrmStoreType() != PARALLEL_FRM_NOT_SUPPORT)
            fitrange.ulInWidth  = m_ulLdcMaxOutWidth * 2;
        else
            fitrange.ulInWidth  = m_ulLdcMaxOutWidth;
        
        fitrange.ulInHeight     = m_ulLdcMaxOutHeight;
        fitrange.ulOutWidth     = usEncInputW;
        fitrange.ulOutHeight    = usEncInputH;

        fitrange.ulInGrabX      = 1;
        fitrange.ulInGrabY      = 1;
        fitrange.ulInGrabW      = fitrange.ulInWidth;
        fitrange.ulInGrabH      = fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        
        MMPD_PTZ_InitPtzInfo(m_2ndRecFctlLink.scalerpath,
                             fitrange.fitmode,
                             fitrange.ulInWidth, fitrange.ulInHeight, 
                             fitrange.ulOutWidth, fitrange.ulOutHeight);

        // Be sync with preview path : TBD
        if (m_ub2ndVRStreamSnrId == PRM_SENSOR) {
            MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
        }
        else if (m_ub2ndVRStreamSnrId == SCD_SENSOR && MMP_IsDualVifCamEnable()) {
            if (MMP_GetDualSnrPrevwType() == DUALSNR_DUAL_PREVIEW)
                MMPD_PTZ_GetCurPtzStep(m_2ndPrewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
            else
                MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
        }
        
        if (usCurZoomStep > m_VidRecordZoomInfo.usMaxZoomSteps) {
            usCurZoomStep = m_VidRecordZoomInfo.usMaxZoomSteps;
        }

        MMPD_PTZ_CalculatePtzInfo(m_2ndRecFctlLink.scalerpath, usCurZoomStep, 0, 0);

        MMPD_PTZ_GetCurPtzInfo(m_2ndRecFctlLink.scalerpath, &fitrange, &EncodeGrabctl);

        if (MMP_IsVidPtzEnable()) {
            MMPD_PTZ_ReCalculateGrabRange(&fitrange, &EncodeGrabctl);
        }
        
        fctlAttr.colormode          = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
        fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
        fctlAttr.fctllink           = m_2ndRecFctlLink;
        fctlAttr.fitrange           = fitrange;
        fctlAttr.grabctl            = EncodeGrabctl;
        fctlAttr.usBufCnt           = pInputBuf->ulBufCnt;
        fctlAttr.scalsrc            = MMP_SCAL_SOURCE_LDC;
        fctlAttr.bSetScalerSrc      = MMP_TRUE;
        fctlAttr.ubPipeLinkedSnr    = m_ub2ndVRStreamSnrId;
        fctlAttr.bUseRotateDMA      = MMP_FALSE;
        fctlAttr.usRotateBufCnt     = 0;

        if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
            fctlAttr.scalsrc        = MMP_SCAL_SOURCE_GRA;
        }

        for (i = 0; i < fctlAttr.usBufCnt; i++) {
            if (pInputBuf->ulY[i] != 0)
                fctlAttr.ulBaseAddr[i] = pInputBuf->ulY[i];
            if (pInputBuf->ulU[i] != 0)
                fctlAttr.ulBaseUAddr[i] = pInputBuf->ulU[i];
            if (pInputBuf->ulV[i] != 0)
                fctlAttr.ulBaseVAddr[i] = pInputBuf->ulV[i];
        }

        if (m_VidRecdModes.VidCurBufMode[1] == VIDENC_CURBUF_RT) {
            fctlAttr.bRtModeOut = MMP_TRUE;
            MMPD_Fctl_SetPipeAttrForH264Rt(&fctlAttr, usEncInputW);
        }
        else {
            fctlAttr.bRtModeOut = MMP_FALSE;
            fctlAttr.sScalDelay = m_sFullSpeedScalDelay;
            MMPD_Fctl_SetPipeAttrForH264FB(&fctlAttr);
            
            m_bInitFcamRecPipeFrmBuf = MMP_TRUE; // CHECK
        }

        MMPD_Fctl_LinkPipeToVideo(m_2ndRecFctlLink.ibcpipeID, ubEncId);
    }

    m_VREncodeFctlAttr[1] = fctlAttr;

    // Tune MCI priority of encode pipe for frame based mode
    if (m_VidRecdModes.VidCurBufMode[1] == VIDENC_CURBUF_FRAME) {
        MMPD_VIDENC_TunePipeMaxMCIPriority(m_2ndRecFctlLink.ibcpipeID);
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_EnableDualH264Pipe
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Turn on and off record for video encode.

 @param[in] bEnable Enable and disable scaler path for video encode.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_EnableDualH264Pipe(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
	#if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
	if (MMP_IsSupportDecMjpegToEncH264() && CAM_CHECK_PRM(PRM_CAM_NONE))
	{
		m_2ndRecFctlLink.scalerpath = m_DecMjpegToEncFctlLink.scalerpath;
		m_2ndRecFctlLink.icopipeID = m_DecMjpegToEncFctlLink.icopipeID;
		m_2ndRecFctlLink.ibcpipeID = m_DecMjpegToEncFctlLink.ibcpipeID;
	}
	#endif

    if (m_bVidRecordActive[1] ^ bEnable)
    {
        if (bEnable) {
            MMPD_Fctl_EnablePreview(ubSnrSel, m_2ndRecFctlLink.ibcpipeID, bEnable, MMP_FALSE);
        }
        else {
            #if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
            if (MMP_IsSupportDecMjpegToEncH264())
                MMPD_Fctl_EnablePreview(ubSnrSel, m_2ndRecFctlLink.ibcpipeID, bEnable, MMP_FALSE);
            else
            #endif
                MMPD_Fctl_EnablePreview(ubSnrSel, m_2ndRecFctlLink.ibcpipeID, bEnable, MMP_TRUE);
        }

        m_bVidRecordActive[1] = bEnable;
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_DualEncPreRecord
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Pre start video recording for 2nd H264-Encode.

 It start record without enable file saving
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_DualEncPreRecord(MMP_ULONG ulPreCaptureMs)
{
    MMP_ULONG           enc_id = 1;
    VIDENC_FW_STATUS    status_vid;
    MMP_ERR             status;
    MMP_ULONG           ulFps;
    MMP_ULONG           EncWidth, EncHeight;
    MMP_ULONG64         ullBitrate64        = m_VidRecdModes.ulBitrate[1];
    MMP_ULONG64         ullTimeIncr64       = m_VidRecdModes.ContainerFrameRate[1].usVopTimeIncrement;
    MMP_ULONG64         ullTimeResol64      = m_VidRecdModes.ContainerFrameRate[1].usVopTimeIncrResol;
    MMP_ULONG           ulTargetFrameSize   = (MMP_ULONG)((ullBitrate64 * ullTimeIncr64)/(ullTimeResol64 * 8));
    MMP_ULONG           ulExpectBufSize     = 0;
    MMP_ULONG           ulMaxPreEncMs       = 0;
    #if (EMERGENTRECD_SUPPORT)
    MMP_ULONG           ulMaxAudPreEncMs    = 0;
    #endif
    MMP_ULONG           ulTimeout = VR_QUERY_STATES_TIMEOUT; 
    MMP_ERR             sRet = MMP_ERR_NONE;

    MMPS_3GPRECD_SetEncodeRes(MMPS_3GPRECD_FILESTREAM_DUAL);

    EncWidth  = m_ulVREncodeW[1];
    EncHeight = m_ulVREncodeH[1];

    ulFps = (m_VidRecdModes.SnrInputFrameRate[1].usVopTimeIncrResol - 1 +
            m_VidRecdModes.SnrInputFrameRate[1].usVopTimeIncrement) /
            m_VidRecdModes.SnrInputFrameRate[1].usVopTimeIncrement;

    status = MMPD_VIDENC_CheckCapability(EncWidth, EncHeight, ulFps);

    if (status != MMP_ERR_NONE) {
        return status;
    }
    
    ulExpectBufSize = m_VidRecdConfigs.ul2ndVideoCompBufSize - (ulTargetFrameSize * 3);

    #if (EMERGENTRECD_SUPPORT)
    ulMaxAudPreEncMs = (m_VidRecdConfigs.ulAudioCompBufSize) / (m_VidRecdModes.ulAudBitrate >> 3) * 1000 - 1000;
    ulMaxPreEncMs = (MMP_ULONG)(((MMP_ULONG64)ulExpectBufSize * 1000) / (m_VidRecdModes.ulBitrate[1] >> 3)) - 1000;
    #else
    ulMaxPreEncMs = (MMP_ULONG)(((MMP_ULONG64)ulExpectBufSize * 900) / (m_VidRecdModes.ulBitrate[1] >> 3));
    #endif

    #if (EMERGENTRECD_SUPPORT)
    if (ulMaxPreEncMs > ulMaxAudPreEncMs) {
        ulMaxPreEncMs = ulMaxAudPreEncMs;
    }
    #endif

    if (ulPreCaptureMs > ulMaxPreEncMs) {
        PRINTF("The pre-record duration %d is over preferred %d ms\r\n", ulPreCaptureMs, ulMaxPreEncMs);
        ulPreCaptureMs = ulMaxPreEncMs;
    }

    if (m_VidDualID == INVALID_ENC_ID)
        status_vid = VIDENC_FW_STATUS_NONE;
    else
        MMPD_VIDENC_GetStatus(m_VidDualID, &status_vid);

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP)) {

        if (!MMPD_VIDENC_IsModuleInit()) {
            MMP_ERR	mmpstatus;

            mmpstatus = MMPD_VIDENC_InitModule();
            if (mmpstatus != MMP_ERR_NONE) {
                return mmpstatus;
            }
        }

        /* Reset Icon/IBC to prevent frame shift or broken */
        sRet |= MMPD_Scaler_ResetModule(m_2ndRecFctlLink.scalerpath);
        sRet |= MMPD_Icon_ResetModule(m_2ndRecFctlLink.icopipeID);
        sRet |= MMPD_IBC_ResetModule(m_2ndRecFctlLink.ibcpipeID);

        if (MMPS_3GPRECD_SetDualH264MemoryMap(  &enc_id,
                                                EncWidth,
                                                EncHeight,
                                                m_ulVidRecSramAddr,
                                                m_ulVidRecDramEndAddr))
        {
            PRINTF("Alloc mem for video pre-record failed\r\n");
            return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
        }

        switch (m_VidRecdModes.VidCurBufMode[1]) {
        case VIDENC_CURBUF_FRAME:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_FRAME);
            break;
        case VIDENC_CURBUF_RT:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_RT);
            break;
        default:
            return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
        }

        if (MMPS_3GPRECD_EnableDualH264Pipe(m_ub2ndVRStreamSnrId, MMP_TRUE) != MMP_ERR_NONE) {
            PRINTF("Enable Video Record: Fail\r\n");
            return MMP_3GPRECD_ERR_GENERAL_ERROR;
        }

        // Set encoder and merger parameters
        if (((EncWidth == 3200) && (EncHeight == 1808)) ||
            ((EncWidth == 1920) && (EncHeight == 1088)) ||
            ((EncWidth == 1440) && (EncHeight == 1088)) ||
            ((EncWidth == 640) && (EncHeight == 368))) {
            
            MMPD_VIDENC_SetCropping(enc_id, 0, 8, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_REPEAT, 8);
        }
        else if ((EncWidth == 1600) && (EncHeight == 912)) {
            MMPD_VIDENC_SetCropping(enc_id, 0, 12, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_REPEAT, 12);
        }
        else {
            MMPD_VIDENC_SetCropping(enc_id, 0, 0, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_NONE, 0);
        }

        if (MMPD_VIDENC_SetResolution(enc_id, EncWidth, EncHeight) != MMP_ERR_NONE) {
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }

        if (MMPD_VIDENC_SetProfile(enc_id, m_VidRecdModes.VisualProfile[1]) != MMP_ERR_NONE) {
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }
        
        MMPD_VIDENC_SetEncodeMode();

        MMPD_3GPMGR_SetGOP(VIDENC_STREAMTYPE_DUALENC, m_VidRecdModes.usPFrameCount[1], m_VidRecdModes.usBFrameCount[1]);
        
        MMPD_VIDENC_SetGOP(enc_id, m_VidRecdModes.usPFrameCount[1], m_VidRecdModes.usBFrameCount[1]);

        MMPD_VIDENC_SetQuality(enc_id, ulTargetFrameSize, (MMP_ULONG)ullBitrate64);

        MMPD_VIDENC_SetSnrFrameRate(enc_id,
                                    VIDENC_STREAMTYPE_DUALENC,
                                    m_VidRecdModes.SnrInputFrameRate[1].usVopTimeIncrement,
                                    m_VidRecdModes.SnrInputFrameRate[1].usVopTimeIncrResol);

        MMPD_VIDENC_SetEncFrameRate(enc_id,
                                    m_VidRecdModes.VideoEncFrameRate[1].usVopTimeIncrement,
                                    m_VidRecdModes.VideoEncFrameRate[1].usVopTimeIncrResol);

        MMPD_3GPMGR_SetFrameRate(VIDENC_STREAMTYPE_DUALENC,
                                 m_VidRecdModes.ContainerFrameRate[1].usVopTimeIncrResol,
                                 m_VidRecdModes.ContainerFrameRate[1].usVopTimeIncrement);

        MMPD_VIDENC_EnableClock(MMP_TRUE);
        
        MMPD_3GPMGR_PreCapture(VIDENC_STREAMTYPE_DUALENC, ulPreCaptureMs);

        do {
            MMPD_VIDENC_GetStatus(enc_id, &status_vid);
            if (status_vid != VIDENC_FW_STATUS_PREENCODE) {
                MMPF_OS_Sleep(1);
            }
        } while ((status_vid != VIDENC_FW_STATUS_PREENCODE) && ((--ulTimeout) > 0));

        if (0 == ulTimeout) {
            MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);
        }

        if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
            (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)){
            glEncID[gbTotalEncNum] = m_VidDualID;
            gbTotalEncNum ++;
        }
        
    }
    else {
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StartDualH264
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Start multi video encode.
*/
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
extern MMP_USHORT gsMjpegToPreviewSrcWidth;
extern MMP_USHORT gsMjpegToPreviewSrcHeight;
#endif
MMP_ERR MMPS_3GPRECD_StartDualH264(void)
{
    MMP_ULONG           enc_id = 0;
    VIDENC_FW_STATUS    status_vid;
    MMP_ERR             status;
    MMP_ULONG           EncWidth, EncHeight;
    MMP_ULONG64         ullBitrate64 = m_VidRecdModes.ulBitrate[1];
    MMP_ULONG           ulFps;
    MMP_ULONG           ulAvaSize;
    MMP_ULONG           ulTimeout = VR_QUERY_STATES_TIMEOUT; 
    VIDENC_STREAMTYPE   sStreamType = VIDENC_STREAMTYPE_MAX;
    MMP_ULONG           uFileStream = MMPS_3GPRECD_FILESTREAM_MAX_NUM;
    MMP_BOOL            bEncTimerEn = MMP_FALSE;

    if (!CAM_CHECK_PRM(PRM_CAM_NONE))
    {
        sStreamType = VIDENC_STREAMTYPE_DUALENC;
        uFileStream = MMPS_3GPRECD_FILESTREAM_DUAL;
    }
    else {
		if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264)) 
		{
	        sStreamType = VIDENC_STREAMTYPE_VIDRECD;
    	    uFileStream = MMPS_3GPRECD_FILESTREAM_NORMAL;
        	MMPD_VIDENC_IsTimerEnabled(&bEncTimerEn);
	        if (!bEncTimerEn) {
    	        MMPD_VIDENC_EnableTimer(MMP_TRUE);
        	}

	        m_VidDualID = m_VidRecdID;        
			}
    }
    
    MMPS_3GPRECD_SetEncodeRes(uFileStream);

    EncWidth  = m_ulVREncodeW[1];
    EncHeight = m_ulVREncodeH[1];

    ulFps = (m_VidRecdModes.SnrInputFrameRate[1].usVopTimeIncrResol - 1 +
            m_VidRecdModes.SnrInputFrameRate[1].usVopTimeIncrement) /
            m_VidRecdModes.SnrInputFrameRate[1].usVopTimeIncrement;

	// for only sonix
	#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
    {
        EncWidth = gsMjpegToPreviewSrcWidth;
        EncHeight = gsMjpegToPreviewSrcHeight;                
        ulFps = 25;
        MMPD_3GPMGR_SetGOP(VIDENC_STREAMTYPE_VIDRECD, m_VidRecdModes.usPFrameCount[0], m_VidRecdModes.usBFrameCount[0]);
                      
        MMPD_3GPMGR_SetFrameRate(VIDENC_STREAMTYPE_VIDRECD,
                                 m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrResol,
                                 m_VidRecdModes.ContainerFrameRate[0].usVopTimeIncrement);        
    }
	#endif

    status = MMPD_VIDENC_CheckCapability(EncWidth, EncHeight, ulFps);

    if (status != MMP_ERR_NONE) {
        return status;
    }
    
    if (m_VidDualID == INVALID_ENC_ID)
        status_vid = VIDENC_FW_STATUS_NONE;
    else
        MMPD_VIDENC_GetStatus(m_VidDualID, &status_vid);

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP)) {

        if (!MMPD_VIDENC_IsModuleInit()) {
            MMP_ERR	mmpstatus;

            mmpstatus = MMPD_VIDENC_InitModule();
            if (mmpstatus != MMP_ERR_NONE)
                return mmpstatus;
        }
        
        /* Now stop record will not stop preview (all pipes actually)
         * User can call MMPS_3GPRECD_StopRecord() and then call
         * MMPS_3GPRECD_StartRecord() to re-start recording without
         * stop/start preview. So we have to reset start address of mem
         * for encoder here.
         */
        if (!CAM_CHECK_PRM(PRM_CAM_NONE))
        {
        if (MMPS_3GPRECD_SetDualH264MemoryMap(  &enc_id,
                                                EncWidth,
                                                EncHeight,
                                                m_ulVidRecSramAddr,
                                                m_ulVidRecDramEndAddr))
        {
            RTNA_DBG_Str(0, "Alloc mem for 2nd record failed\r\n");
            return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
        }
        }
        else {
			if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264)) 
			{
	            if (MMPS_3GPRECD_SetH264MemoryMap(  &enc_id,
    	                                            EncWidth,
        	                                        EncHeight,
            	                                    m_ulVidRecSramAddr,
                	                                m_ulVideoPreviewEndAddr))
            	{
                	RTNA_DBG_Str(0, "Alloc mem for video record failed\r\n");
	                return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
    	        }
			}
        }

        if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264) && m_VidRecdID != INVALID_ENC_ID)
            m_VidDualID = m_VidRecdID;
            
        // Set current buffer mode
        switch (m_VidRecdModes.VidCurBufMode[1]) {
        case VIDENC_CURBUF_FRAME:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_FRAME);
            break;
        case VIDENC_CURBUF_RT:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_RT);
            break;
        default:
            return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
        }

        if (MMPS_3GPRECD_EnableDualH264Pipe(m_ub2ndVRStreamSnrId, MMP_TRUE) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "MMPS_3GPRECD_EnableDualH264Pipe Err2\r\n");
            return MMP_3GPRECD_ERR_GENERAL_ERROR;
        }

        if (((EncWidth == 3200) && (EncHeight == 1808)) ||
            ((EncWidth == 1920) && (EncHeight == 1088)) ||
            ((EncWidth == 1440) && (EncHeight == 1088)) ||
            ((EncWidth == 640) && (EncHeight == 368))) {
            
            MMPD_VIDENC_SetCropping(enc_id, 0, 8, 0, 0);
            
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_REPEAT, 8);
        }
        else {
            MMPD_VIDENC_SetCropping(enc_id, 0, 0, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_NONE, 0);
        }
        
        if (MMPD_VIDENC_SetResolution(enc_id, EncWidth, EncHeight) != MMP_ERR_NONE) {
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }

        if (CAM_CHECK_PRM(PRM_CAM_NONE) && CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
        {
            MMPD_VIDENC_SetEncodeMode();
            MMPD_VIDENC_SetGOP(enc_id, m_VidRecdModes.usPFrameCount[0], m_VidRecdModes.usBFrameCount[0]);
            MMPD_VIDENC_SetQuality(enc_id, MMPS_3GPRECD_CalculteTargetFrmSize(0), (MMP_ULONG)m_VidRecdModes.ulBitrate[0]);
        }

        if (!CAM_CHECK_PRM(PRM_CAM_NONE))
        {
        if (MMPD_VIDENC_SetProfile(enc_id, m_VidRecdModes.VisualProfile[1]) != MMP_ERR_NONE) {
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }
        
        MMPD_VIDENC_SetEncodeMode();

        MMPD_3GPMGR_SetGOP(sStreamType, m_VidRecdModes.usPFrameCount[1], m_VidRecdModes.usBFrameCount[1]);
        MMPD_VIDENC_SetGOP(enc_id, m_VidRecdModes.usPFrameCount[1], m_VidRecdModes.usBFrameCount[1]);

        MMPD_VIDENC_SetQuality(enc_id, MMPS_3GPRECD_CalculteTargetFrmSize(1), (MMP_ULONG)ullBitrate64);

        MMPD_VIDENC_SetSnrFrameRate(enc_id,
                                    sStreamType,
                                    m_VidRecdModes.SnrInputFrameRate[1].usVopTimeIncrement,
                                    m_VidRecdModes.SnrInputFrameRate[1].usVopTimeIncrResol);

        MMPD_VIDENC_SetEncFrameRate(enc_id,
                                    m_VidRecdModes.VideoEncFrameRate[1].usVopTimeIncrement,
                                    m_VidRecdModes.VideoEncFrameRate[1].usVopTimeIncrResol);

        MMPD_3GPMGR_SetFrameRate(sStreamType,
                                 m_VidRecdModes.ContainerFrameRate[1].usVopTimeIncrResol,
                                 m_VidRecdModes.ContainerFrameRate[1].usVopTimeIncrement);
        }
    }

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP) ||
        (status_vid == VIDENC_FW_STATUS_PREENCODE)) {

	#if (SUPPORT_SHARE_REC == 0)
        // Change start capture to MMPS_3GPRECD_StartAllRecord() for dual encode slow card issue.
        if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
            (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)) {
            // Just save file ID and encid and enc number.
            if ((status_vid == VIDENC_FW_STATUS_NONE) ||
                (status_vid == VIDENC_FW_STATUS_STOP)) {
                if (!CAM_CHECK_PRM(PRM_CAM_NONE))
                glEncID[gbTotalEncNum] = m_VidDualID;
                else {
                    if (CAM_CHECK_SCD(SCD_CAM_NONE) && CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))
                    {
                        glEncID[gbTotalEncNum] = m_VidRecdID;
                        m_VidDualID = INVALID_ENC_ID;
                    }
                }
                gbTotalEncNum++;
            }
            
            return MMP_ERR_NONE;
        }
        else 
    #endif
        {
            
            MMPD_3GPMGR_SetFileLimit(m_VidRecdModes.ulSizeLimit, m_VidRecdModes.ulReservedSpace, &ulAvaSize);

            if (MMPS_3GPRECD_GetExpectedRecordTime(ulAvaSize, m_VidRecdModes.ulBitrate[1], m_VidRecdModes.ulAudBitrate) <= 0) {
                return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
            }
            
            if (status_vid != VIDENC_FW_STATUS_PREENCODE) {
                MMPD_VIDENC_EnableClock(MMP_TRUE);
            }
            else {
                /*
                 * In pre-encode case,m_VidDualID is always valid,
                 * besides enc_id will not be assigned by MMPS_3GPRECD_SetH264MemoryMap
                 * so dricetly assign m_VidDualID to enc_id here!
                 */
                enc_id = m_VidDualID;
            }

            if (MMPD_3GPMGR_StartCapture(m_VidDualID, sStreamType) != MMP_ERR_NONE) {
                MMPD_VIDENC_EnableClock(MMP_FALSE);
                return MMP_3GPRECD_ERR_GENERAL_ERROR;
            }

            do {
                MMPD_VIDENC_GetStatus(enc_id, &status_vid);
                if (status_vid != VIDENC_FW_STATUS_START) {
                    MMPF_OS_Sleep(1);
                }
            } while ((status_vid != VIDENC_FW_STATUS_START) && (--ulTimeout) > 0);

            if (ulTimeout == 0) {
                RTNA_DBG_Str(0, "Dual H264 NG...\r\n");
                return MMP_3GPRECD_ERR_GENERAL_ERROR;
            }
            
            if (status == MMP_FS_ERR_OPEN_FAIL) {
                return status;
            }

            #if (SUPPORT_SHARE_REC)
            glEncID[1] = m_VidDualID;
            if (status_vid == VIDENC_FW_STATUS_START) {
                gbTotalEncNum = 2;
            }
            #endif

            if (status_vid == VIDENC_FW_STATUS_START) {
                return MMP_ERR_NONE;
            }
            else if (status == MMP_FS_ERR_OPEN_FAIL) {
                return MMP_3GPRECD_ERR_OPEN_FILE_FAILURE;
            }
            else {
                return MMP_ERR_NONE;
            }
        }
    }
    else {
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetDualH264MemoryMap
//  Description :
//------------------------------------------------------------------------------
/**
     @brief Set memory layout by different resolution and memory type for H264.

     Depends on encoded resolution and memory type to map buffers. It supports two types
     of memory, INTER(823) and STACK(821).
     @param[in] usEncW Encode width.
     @param[in] usEncH Encode height.
     @param[in] ulFBufAddr Available start address of frame buffer.
     @param[in] ulStackAddr Available start address of dram buffer.
     @retval MMP_ERR_NONE Success.
 */
MMP_ERR MMPS_3GPRECD_SetDualH264MemoryMap(MMP_ULONG *ulEncId, MMP_USHORT usEncW, MMP_USHORT usEncH, MMP_ULONG ulFBufAddr, MMP_ULONG ulStackAddr)
{
    const MMP_ULONG                 AVSyncBufSize         = 32;
    #if (VIDRECD_MULTI_TRACK == 0)
    const MMP_ULONG                	VideoSizeTableSize    = 1024;
    const MMP_ULONG                 VideoTimeTableSize    = 1024;
    #endif
    const MMP_ULONG                 AudioCompBufSize      = 0;
    const MMP_ULONG                 SPSSize               = 48;
    const MMP_ULONG                 PPSSize               = 16;
    const MMP_ULONG                 VideoCompBufSize      = m_VidRecdConfigs.ul2ndVideoCompBufSize;
    #if (VIDRECD_MULTI_TRACK == 0)
    const MMP_ULONG                 AVRepackBufSize       = 512 * 1024;
    #endif 
    const MMP_ULONG                 SliceLengBufSize      = ((4 * ((usEncH>>4) + 2) + 63) >> 6) << 6;//align 64

    MMP_USHORT						i;
    MMP_ULONG						ulCurAddr;
    MMP_ULONG						ulTmpBufSize;
    MMPD_MP4VENC_VIDEOBUF			videohwbuf;
    MMPD_H264ENC_HEADER_BUF         headerbuf;
    MMPD_3GPMGR_AV_COMPRESS_BUF	    mergercompbuf;
    MMPD_3GPMGR_REPACKBUF			repackmiscbuf;
    MMP_ULONG                       bufferEndAddr;
    MMP_ULONG                       ulEncFrameSize;

    AUTL_MEMDBG_BLOCK				sVRRecdMemDbgBlk;
    MMP_UBYTE						ubDbgItemIdx = 0;
    MMP_ULONG						ulMemStart = 0;

    m_ulVidRecEncodeAddr = ulCurAddr = ulStackAddr;

    if (((usEncW>>4)*(usEncH>>4)) > 34560) {//4096x2160
        return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
    }

    AUTL_MemDbg_Init(&sVRRecdMemDbgBlk, AUTL_MEMDBG_USAGE_ID_VR_RECD);
    ulMemStart = ulCurAddr;

    ///////////////////////////////////////////////////////////////////////////
    //
    //  Get Record Config setting and calculate how many memory will be used    
    //
    ///////////////////////////////////////////////////////////////////////////

    // Get Encode buffer config and size (YUV420)
    if (m_VidRecdModes.VidCurBufMode[1] == VIDENC_CURBUF_RT) {
        m_VidRecdInputBuf[1].ulBufCnt = 2;
    }
    else {
        m_VidRecdInputBuf[1].ulBufCnt = 2 + VIDENC_MAX_B_FRAME_NUMS;
    }

    ulEncFrameSize = ALIGN32((usEncW * usEncH * 3) / 2);
    
    //////////////////////////////////////////////////////////////////////////
    //
    //  Allocate memory for record buffer
    //
    //////////////////////////////////////////////////////////////////////////

    //*************************
    // AV Sync ............ 32
    // Frame Table ........ 1k
    // Time Table ......... 1k
    // Audio BS ........... 16k
    //*************************

    // Set av sync buffer
    repackmiscbuf.ulVideoEncSyncAddr    = ulCurAddr;
    ulCurAddr                           += AVSyncBufSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam AV Sync");
    ulMemStart = ulCurAddr;

    #if (VIDRECD_MULTI_TRACK == 0)
    // Set aux size table buffer, must be 512 byte alignment
    repackmiscbuf.ulVideoSizeTableAddr  = ulCurAddr;
    repackmiscbuf.ulVideoSizeTableSize  = VideoSizeTableSize;
    ulCurAddr                           += VideoSizeTableSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Size Table");
    ulMemStart = ulCurAddr;

    // Set aux time table buffer, must be 512 byte alignment
    repackmiscbuf.ulVideoTimeTableAddr  = ulCurAddr;
    repackmiscbuf.ulVideoTimeTableSize  = VideoTimeTableSize;
    ulCurAddr                           += VideoTimeTableSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Time Table");
    ulMemStart = ulCurAddr;
    #endif

    // Set audio compressed buffer
    mergercompbuf.ulAudioCompBufStart   = ulCurAddr;
    ulCurAddr                           += AudioCompBufSize;
    mergercompbuf.ulAudioCompBufEnd     = ulCurAddr;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Audio Compress");
    ulMemStart = ulCurAddr;
    
    // Initialize H264 instance
    if (MMPD_VIDENC_InitInstance(ulEncId, VIDENC_STREAMTYPE_DUALENC, VIDENC_RC_MODE_CBR) != MMP_ERR_NONE)
    {
        RTNA_DBG_Str(0, "DualEnc Err not available h264 instance\r\n");
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    m_VidDualID = *ulEncId;

    // Set REF/GEN buffer
    ulCurAddr = ALIGN32(ulCurAddr);

    MMPD_H264ENC_CalculateRefBuf(usEncW, usEncH, &(videohwbuf.refgenbd), &ulCurAddr);

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Ref/Gen Buffer");
    ulMemStart = ulCurAddr;

    // Set current encoder buffer (NV12)
    if (m_VidRecdModes.VidCurBufMode[1] == VIDENC_CURBUF_RT) {
        if (/*m_bVidRecordActive[0]*/1 && m_VidRecdModes.VidCurBufMode[0] == VIDENC_CURBUF_RT) {
            for (i = 0; i < m_VidRecdInputBuf[0].ulBufCnt; i++) {
                m_VidRecdInputBuf[1].ulY[i] = m_VidRecdInputBuf[0].ulY[i];
                m_VidRecdInputBuf[1].ulU[i] = m_VidRecdInputBuf[0].ulU[i];
                m_VidRecdInputBuf[1].ulV[i] = m_VidRecdInputBuf[0].ulV[i];
            }
        }
        else {
            for (i = 0; i < m_VidRecdInputBuf[1].ulBufCnt; i++) {
                m_VidRecdInputBuf[1].ulY[i] = ALIGN32(ulFBufAddr);
                ulFBufAddr += (usEncW << 4);
                m_VidRecdInputBuf[1].ulU[i] = ulFBufAddr;
                m_VidRecdInputBuf[1].ulV[i] = m_VidRecdInputBuf[1].ulU[i] + (usEncW << 2); // CHECK
                ulFBufAddr += (usEncW << 3);
            }
            MMPD_H264ENC_SetSourcePPBuf(&m_VidRecdInputBuf[1]);
        }
    }
    else {
        MMP_UBYTE*  pBuf = NULL;
       
        for (i = 0; i < m_VidRecdInputBuf[1].ulBufCnt; i++) {
            ulTmpBufSize = usEncW * usEncH;

            m_VidRecdInputBuf[1].ulY[i] = ALIGN32(ulCurAddr);
            m_VidRecdInputBuf[1].ulU[i] = m_VidRecdInputBuf[1].ulY[i] + ulTmpBufSize;
            m_VidRecdInputBuf[1].ulV[i] = m_VidRecdInputBuf[1].ulU[i];
            
            ulCurAddr += (ulTmpBufSize * 3)/2;

            // Workaround to reduce blur image in the bottom of frame
            memset((MMP_UBYTE*)m_VidRecdInputBuf[1].ulY[i], 0, (ulTmpBufSize * 3) / 2 );

            // Fill the last 8 line as black color
            pBuf = (MMP_UBYTE *)(m_VidRecdInputBuf[1].ulU[i] - (usEncW * 8));
            MEMSET(pBuf, 0x00, usEncW * 8);
            pBuf = (MMP_UBYTE *)(m_VidRecdInputBuf[1].ulU[i] + (usEncW * usEncH / 2) - (usEncW * 4));
            MEMSET(pBuf, 0x80, usEncW * 4);
        }
    }

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Frame Buffer");
    ulMemStart = ulCurAddr;

    // Set Record pipe
    #if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
    if (MMP_IsSupportDecMjpegToEncH264())
        MMPS_3GPRECD_InitDecMjpegToEncode(&m_VidRecdInputBuf[1], *ulEncId);
    else
    #endif
        MMPS_3GPRECD_SetDualH264PipeConfig(&m_VidRecdInputBuf[1], m_ulVREncScalOutW[1], m_ulVREncScalOutH[1], *ulEncId);

    // Set Slice Length Buffer, align32
    videohwbuf.miscbuf.ulSliceLenBuf    = ulCurAddr;
    ulCurAddr                           = ulCurAddr + SliceLengBufSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Slice Length Buf");
    ulMemStart = ulCurAddr;

    // Set MV buffer, #MB/Frame * #MVs/MB * #byte/MV
    videohwbuf.miscbuf.ulMVBuf          = m_ulVidShareMvAddr;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam MV Buf");
    ulMemStart = ulCurAddr;

    // Set SPS buffer
    ulCurAddr = ALIGN32(ulCurAddr);
    
    headerbuf.ulSPSStart                = ulCurAddr;
    headerbuf.ulSPSSize                 = SPSSize;
    ulCurAddr                           += headerbuf.ulSPSSize;
    
    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam SPS");
    ulMemStart = ulCurAddr;
    
    #if (SUPPORT_VUI_INFO)
    // Rebuild-SPS
    headerbuf.ulTmpSPSSize              = SPSSize+16;
    headerbuf.ulTmpSPSStart             = ulCurAddr;
    ulCurAddr                           += headerbuf.ulTmpSPSSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Rebuild-SPS");
    ulMemStart = ulCurAddr; 
    #endif

    // Set PPS buffer
    headerbuf.ulPPSStart                = ulCurAddr;
    headerbuf.ulPPSSize                 = PPSSize;
    ulCurAddr                           += headerbuf.ulPPSSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam PPS");
    ulMemStart = ulCurAddr; 

    // Set video compressed buffer, 32 byte alignment
    ulCurAddr                           = ALIGN32(ulCurAddr);
    videohwbuf.bsbuf.ulStart            = ulCurAddr;
    mergercompbuf.ulVideoCompBufStart   = ulCurAddr;
    ulCurAddr                           += VideoCompBufSize;
    videohwbuf.bsbuf.ulEnd              = ulCurAddr;
    mergercompbuf.ulVideoCompBufEnd     = ulCurAddr;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam Video Compress");
    ulMemStart = ulCurAddr; 

    #if (VIDRECD_MULTI_TRACK == 0)
    // Set av repack buffer
    repackmiscbuf.ulAvRepackStartAddr   = ulCurAddr;
    repackmiscbuf.ulAvRepackSize        = AVRepackBufSize;
    ulCurAddr                           += repackmiscbuf.ulAvRepackSize;

    AUTL_MemDbg_PushItem(&sVRRecdMemDbgBlk, ubDbgItemIdx++, ulMemStart, ulCurAddr, ulCurAddr - ulMemStart, (MMP_UBYTE*)"RCam AV Repack");
    ulMemStart = ulCurAddr; 
    #endif

    #if (VIDRECD_MULTI_TRACK == 0)
    MMPD_3GPMGR_SetRepackMiscBuf(VIDENC_STREAMTYPE_DUALENC, &repackmiscbuf);
    #endif
    MMPD_3GPMGR_SetEncodeCompBuf(VIDENC_STREAMTYPE_DUALENC, &mergercompbuf);

    MMPD_H264ENC_SetHeaderBuf(*ulEncId, &headerbuf);
    MMPD_H264ENC_SetBitstreamBuf(*ulEncId, &(videohwbuf.bsbuf));
    MMPD_H264ENC_SetRefGenBound(*ulEncId, &(videohwbuf.refgenbd));
    MMPD_H264ENC_SetMiscBuf(*ulEncId, &(videohwbuf.miscbuf));

    MMPS_System_GetPreviewFrameStart(&bufferEndAddr);

    ulCurAddr = ALIGN16(ulCurAddr);
        
    // Set Rear-cam tail info buffer    
	#if (VIDRECD_MULTI_TRACK == 0)
	if ((ulCurAddr + m_VidRecdConfigs.ulTailBufSize) > bufferEndAddr) {
		PRINTF("Can't use high speed videor\r\n");
		MMPD_3GPMGR_SetRecordTailSpeed(VIDENC_STREAMTYPE_DUALENC, MMP_FALSE, 0, 0);
	}
	else {
		PRINTF("USE AIT high speed videor\r\n");
		MMPD_3GPMGR_SetRecordTailSpeed(VIDENC_STREAMTYPE_DUALENC, MMP_TRUE, ulCurAddr, m_VidRecdConfigs.ulTailBufSize);
		ulCurAddr += m_VidRecdConfigs.ulTailBufSize;
		m_ulVidRecDramEndAddr = ulCurAddr;
	}
	#endif

	m_ulVidRecDramEndAddr = ulCurAddr;
    
    AUTL_MemDbg_ShowAllItems(&sVRRecdMemDbgBlk, MMP_TRUE);
    
    #if defined(ALL_FW)
    if (m_ulVidRecDramEndAddr > MMPS_System_GetMemHeapEnd()) {
        printc("\t= [HeapMemErr] 2nd Video record %x/%x\r\n",
                m_ulVidRecDramEndAddr,MMPS_System_GetMemHeapEnd());
        return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
    }
    printc("End of 2nd video record buffers = 0x%X\r\n", m_ulVidRecDramEndAddr);
    #endif

    #if (SUPPORT_COMPONENT_FLOW_CTL)
    if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER) ||
        CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
        
        if (m_ubCamEncodeListId[SCD_SENSOR] == 0xFF) {
            
            MMP_UBYTE ubVifId = MMPF_Sensor_GetVIFPad(m_VREncodeFctlAttr[1].ubPipeLinkedSnr);
            MMP_UBYTE ubRawId = ubVifId;
            MMP_UBYTE ubPipeId = m_VREncodeFctlAttr[1].fctllink.ibcpipeID;
            MMP_UBYTE ubCompIdArray[10];

            ubCompIdArray[0] = MMP_COMPONENT_ID_NULL;
            ubCompIdArray[1] = TRANS_RAWS_TO_RAWSCOMP_ID(ubRawId);
            ubCompIdArray[2] = TRANS_RAWS_TO_DRAMCOMP_ID(ubRawId);
            ubCompIdArray[3] = MMP_COMPONENT_ID_DMA;
            ubCompIdArray[4] = TRANS_RAWS_TO_DMA_DRAMCOMP_ID(ubRawId);
            ubCompIdArray[5] = MMP_COMPONENT_ID_GRA;
            ubCompIdArray[6] = TRANS_PIPE_TO_PIPECOMP_ID(ubPipeId);
            ubCompIdArray[7] = TRANS_PIPE_TO_DRAMCOMP_ID(ubPipeId);
            ubCompIdArray[8] = MMP_COMPONENT_ID_H264ENC;
            ubCompIdArray[9] = MMP_COMPONENT_ID_NULL;

            #if 0 // Not to register again
            MMP_CompCtl_RegisterRawStoreComponent(MMP_COMPONENT_USAGE_ID1, 
                                                  ubRawId,
                                                  MMP_RAW_COLORFMT_YUV422);
            MMP_CompCtl_RegisterRawStoreDramComponent(MMP_COMPONENT_USAGE_ID1,
                                                      ubRawId,
                                                      m_sRawBuf[ubRawId].ulRawBufCnt,
                                                      &m_sRawBuf[ubRawId].ulRawBufAddr[0],
                                                      &m_sRawEndBuf[ubRawId].ulRawBufAddr[0]);

            if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER) &&
                MMP_GetTvDecSnrAttr()->bUseDMADeinterlace) {
                MMP_CompCtl_RegisterDMAComponent(MMP_COMPONENT_USAGE_ID1, 0, 0, 0); // TBD
                MMP_CompCtl_RegisterRawStoreDMADramComponent(MMP_COMPONENT_USAGE_ID1, 
                                                             ubRawId, 
                                                             m_sDeinterlaceBuf[ubRawId].ulRawBufCnt, 
                                                             &m_sDeinterlaceBuf[ubRawId].ulRawBufAddr[0]);
            }

            MMP_CompCtl_RegisterGraComponent(MMP_COMPONENT_USAGE_ID1, srcBuf, srcRect, ubPixDelayN, ubPixDelayM, usLineDelay);
            #endif
            
            MMP_CompCtl_RegisterPipeComponent(MMP_COMPONENT_USAGE_ID1, (void*)&m_VREncodeFctlAttr[1]);
            MMP_CompCtl_RegisterPipeOutDramComponent(MMP_COMPONENT_USAGE_ID1, (void*)&m_VREncodeFctlAttr[1]);
            MMP_CompCtl_RegisterH264Component(MMP_COMPONENT_USAGE_ID1, (void*)&m_VREncodeFctlAttr[1]);

            m_ubCamEncodeListId[SCD_SENSOR] = MMP_CompCtl_LinkComponentsEx(MMP_COMPONENT_USAGE_ID1, &ubCompIdArray[0], 10, (void*)_TrigEmptyRawStoreBuffer);
        }
    }
    #endif

	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StopDualH264
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Stop video recording and fill 3GP tail.

 It works after video start, pause and resume.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
 @retval MMP_3GPRECD_ERR_OPEN_FILE_FAILURE Open file failed.
 @retval MMP_3GPRECD_ERR_CLOSE_FILE_FAILURE Close file failed.
*/
MMP_ERR MMPS_3GPRECD_StopDualH264(void)
{
    MMP_ULONG           ulVidNumOpening = 0;
    VIDENC_FW_STATUS    status_fw;
    MMP_ULONG           ulTimeout = VR_QUERY_STATES_TIMEOUT; 

    if (m_VidDualID == INVALID_ENC_ID)
        status_fw = VIDENC_FW_STATUS_NONE;
    else
        MMPD_VIDENC_GetStatus(m_VidDualID, &status_fw);

    if ((status_fw == VIDENC_FW_STATUS_START)   ||
        (status_fw == VIDENC_FW_STATUS_PAUSE)   ||
        (status_fw == VIDENC_FW_STATUS_RESUME)  ||
        (status_fw == VIDENC_FW_STATUS_STOP)    ||
        (status_fw == VIDENC_FW_STATUS_PREENCODE)) {
        
        MMPD_3GPMGR_StopCapture(m_VidDualID, VIDENC_STREAMTYPE_DUALENC);

        do {
            MMPD_VIDENC_GetStatus(m_VidDualID, &status_fw);
            if (status_fw != VIDENC_FW_STATUS_STOP) {
                MMPF_OS_Sleep(1);
            }
        } while ((status_fw != VIDENC_FW_STATUS_STOP) && ((--ulTimeout) > 0));

        if (0 == ulTimeout) {
            MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk); 
        }
        
        MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);
        
        // Deinit H264 module if all stream are stoped.
        if (ulVidNumOpening == 0) {
            if (MMPD_VIDENC_IsModuleInit()) {
                MMPD_VIDENC_DeinitModule();
            }
        }

        m_VidDualID = INVALID_ENC_ID;
        #if (DUALENC_SUPPORT)
        if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
            (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)) {
            glEncID[1] = m_VidDualID;
            gbTotalEncNum -= 1;
        }
        #endif 

        MMPS_3GPRECD_EnableDualH264Pipe(m_ub2ndVRStreamSnrId, MMP_FALSE);
        
        if (ulVidNumOpening == 0) {
            MMPD_VIDENC_EnableClock(MMP_FALSE);
        }
    }
    else if (status_fw == VIDENC_FW_STATUS_NONE) {
    
        MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);
    
        if ((ulVidNumOpening > 0) && (m_VidDualID != INVALID_ENC_ID)) {
            
            MMPD_VIDENC_DeInitInstance(m_VidDualID);

            m_VidDualID = INVALID_ENC_ID;
            #if (DUALENC_SUPPORT) && (SUPPORT_SHARE_REC == 0)
            if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
                (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)) {
                glEncID[1] = m_VidDualID;
                gbTotalEncNum -= 1; 
            }
            #endif
            
            MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);
            
            if (ulVidNumOpening == 0) {
                if (MMPD_VIDENC_IsModuleInit()) {
                    MMPD_VIDENC_DeinitModule();
                }
            }
        }
    }
    else {
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }

    #if (SUPPORT_COMPONENT_FLOW_CTL)
    MMP_CompCtl_UnLinkComponentList(m_ubCamEncodeListId[SCD_SENSOR], MMP_TRUE);
    #endif

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StartAllRecord
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Start all record with MGR functoin.
 @return Error status.
*/
MMP_ERR MMPS_3GPRECD_StartAllRecord(void) 
{
#if (DUALENC_SUPPORT)
    MMP_ERR             startCaptureStatus = MMP_ERR_NONE;
    VIDENC_FW_STATUS    status_vid;
    MMP_ULONG           ulAvaSize;  
    void                *pSeamlessCB;  
    MMP_ULONG           ulTimeout = VR_QUERY_STATES_TIMEOUT; 
    MMP_ERR             status;
    MMP_ULONG           status_tx;
    
    // Start normal record
    if (m_VidRecdID == INVALID_ENC_ID)
        status_vid = VIDENC_FW_STATUS_NONE;
    else
        MMPD_VIDENC_GetStatus(m_VidRecdID, &status_vid);

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP) ||
        (status_vid == VIDENC_FW_STATUS_PREENCODE)) {
        
        MMPD_3GPMGR_SetFileLimit(m_VidRecdModes.ulSizeLimit, m_VidRecdModes.ulReservedSpace, &ulAvaSize);

        if (MMPS_3GPRECD_GetExpectedRecordTime(ulAvaSize, m_VidRecdModes.ulBitrate[0], m_VidRecdModes.ulAudBitrate) <= 0) {
            RTNA_DBG_Str(0, "All start front get except time error\r\n");
            return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
        }
        else {
            MMPD_3GPMGR_SetTimeLimit(m_VidRecdModes.ulTimeLimitMs);
        }

        if (m_VidRecdConfigs.bSeamlessMode == MMP_TRUE) {
            // Seamless callback must be registered if seamless mode is enabled.
            MMPD_3GPMGR_GetRegisteredCallback(VIDMGR_EVENT_SEAMLESS, &pSeamlessCB);

            if (m_bSeamlessEnabled && (pSeamlessCB == NULL)) {
                RTNA_DBG_Str(0, "Get seamless: Fail\r\n");
                return MMP_3GPRECD_ERR_GENERAL_ERROR;
            }
        }

        if (status_vid != VIDENC_FW_STATUS_PREENCODE) {
            MMPD_VIDENC_EnableClock(MMP_TRUE);
        }
    }
    
	// Start dual record
	if (gbTotalEncNum == 2) {
	
	    if (m_VidDualID == INVALID_ENC_ID)
	    	status_vid = VIDENC_FW_STATUS_NONE;
	    else
	    	MMPD_VIDENC_GetStatus(m_VidDualID, &status_vid);
	
	    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
	        (status_vid == VIDENC_FW_STATUS_STOP) ||
	        (status_vid == VIDENC_FW_STATUS_PREENCODE)) {
	        
			MMPD_3GPMGR_SetFileLimit(m_VidRecdModes.ulSizeLimit, m_VidRecdModes.ulReservedSpace, &ulAvaSize);

	        if (MMPS_3GPRECD_GetExpectedRecordTime(ulAvaSize, m_VidRecdModes.ulBitrate[1], m_VidRecdModes.ulAudBitrate) <= 0) {
	        	RTNA_DBG_Str(0, "All start dual get except time error\r\n");
	            return MMP_3GPRECD_ERR_MEM_EXHAUSTED;
	        }
			
	        if (status_vid != VIDENC_FW_STATUS_PREENCODE) {
	            MMPD_VIDENC_EnableClock(MMP_TRUE);
	        }
		}
	}
	
    #if (DUALENC_SUPPORT)
    if ((MMP_CheckVideoDualRecordEnabled(CAM_TYPE_SCD) & DUAL_REC_ENCODE_H264) ||
        (MMP_CheckVideoDualRecordEnabled(CAM_TYPE_USB) & DUAL_REC_ENCODE_H264)) {
        startCaptureStatus = MMPD_3GPMGR_StartAllCapture(gbTotalEncNum, &glEncID[0]);
    }
    #endif
    
    do {
        MMPD_VIDENC_GetMergerStatus(&status, &status_tx);
        MMPD_VIDENC_GetStatus(glEncID[0], &status_vid);
        
        if (status_vid == VIDENC_FW_STATUS_STOP) {
            RTNA_DBG_Str(0, "All start meet slow card Fail-1\r\n");
            return MMP_3GPRECD_ERR_STOPRECD_SLOWCARD;
        }
        else if ((status_vid != VIDENC_FW_STATUS_START) && (status != MMP_FS_ERR_OPEN_FAIL)) {
            MMPF_OS_Sleep(1);
        }
    } while ((status_vid != VIDENC_FW_STATUS_START) && (status != MMP_FS_ERR_OPEN_FAIL) && ((--ulTimeout) > 0));

    if (0 == ulTimeout) {
        MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);
    }

    if (status == MMP_FS_ERR_OPEN_FAIL) {
        RTNA_DBG_Str(0, "All start file open Fail\r\n");
        return status;
    }

    if (status_vid == VIDENC_FW_STATUS_START) {
        return MMP_ERR_NONE;
    }
    else if (status == MMP_FS_ERR_OPEN_FAIL) {
        return MMP_3GPRECD_ERR_OPEN_FILE_FAILURE;
    }

    if (gbTotalEncNum == 2) {
    
        do {
	        MMPD_VIDENC_GetMergerStatus(&status, &status_tx);
            MMPD_VIDENC_GetStatus(glEncID[1], &status_vid);
            
	        if (status_vid == VIDENC_FW_STATUS_STOP) {
	        	RTNA_DBG_Str(0, "All start meet slow card Fail-2\r\n");
	        	return MMP_3GPRECD_ERR_STOPRECD_SLOWCARD;
	        }
        } while ((status_vid != VIDENC_FW_STATUS_START) && (status != MMP_FS_ERR_OPEN_FAIL) && (--ulTimeout) > 0);

        if (ulTimeout == 0) {
            RTNA_DBG_Str(0, "Dual H264 NG...\r\n");
            return MMP_3GPRECD_ERR_GENERAL_ERROR;
        }
        
        if (status == MMP_FS_ERR_OPEN_FAIL) {
	    	RTNA_DBG_Str(0, "All start DUAL file open Fail\r\n");    
            return status;
        }

        if (status_vid == VIDENC_FW_STATUS_START) {
            return MMP_ERR_NONE;
        }
        else if (status == MMP_FS_ERR_OPEN_FAIL) {
            return MMP_3GPRECD_ERR_OPEN_FILE_FAILURE;
        }
    }
#endif    
    return MMP_ERR_NONE;
}

#if 0
void ____VR_Share_Record_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetShareFileTimeLimit
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set the maximum 3GP file time for video encoding.
 @param[in] ulTimeLimitMs Maximum file time in unit of ms.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE Space not enough.
*/
MMP_ERR MMPS_3GPRECD_SetShareFileTimeLimit(MMP_ULONG ulTimeLimitMs)
{
    #if (SUPPORT_SHARE_REC)
    if (ulTimeLimitMs) {
        glDualRecdTimeLimit = ulTimeLimitMs;
        return MMP_ERR_NONE;
    }
    #endif

    return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetSharePreEncTimeLimit
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set the the pre-encoding time limit.
 @param[in] ulTimeLimitMs Maximum file time in ms.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE Space not enough.
*/
MMP_ERR MMPS_3GPRECD_SetSharePreEncTimeLimit(MMP_ULONG ulTimeLimitMs)
{
    #if (SUPPORT_SHARE_REC)
    if (ulTimeLimitMs) {
        glDualPreEncDuration = ulTimeLimitMs;
        return MMP_ERR_NONE;
    }
    #endif

    return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetSharePreEncTimeLimit
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get the the pre-encoding time limit.
 @param[in] ulTimeLimitMs Maximum file time in ms.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE Space not enough.
*/
MMP_ERR MMPS_3GPRECD_GetSharePreEncTimeLimit(MMP_ULONG *ulTimeLimitMs)
{
    #if (SUPPORT_SHARE_REC)
    if (glDualPreEncDuration > 0) {
        *ulTimeLimitMs = glDualPreEncDuration;
        return MMP_ERR_NONE;
    }
    #endif

    return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetShareRecordingTime
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get dual recording time.
 @param[out] ulTime Recording time in unit of ms.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetShareRecordingTime(MMP_ULONG *ulTime)
{
    #if (SUPPORT_SHARE_REC)
    return MMPD_3GPMGR_GetRecordingTime(VIDENC_STREAMTYPE_DUALENC, ulTime);
    #else
    return MMP_ERR_NONE;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetShareRecordingOffset
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get dual recording time offset.
 @param[out] ulTime Recording time offset in unit of ms.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetShareRecordingOffset(MMP_ULONG *ulTime)
{
    #if (SUPPORT_SHARE_REC)
    return MMPD_3GPMGR_GetRecordingOffset(VIDENC_STREAMTYPE_DUALENC, ulTime);
    #else
    return MMP_ERR_NONE;
    #endif
}

#if 0
void ____VR_Wifi_Stream_Function____(){ruturn;} //dummy
#endif

#if (SUPPORT_H264_WIFI_STREAM)
//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_CustomedResol
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set H264 Wifi resolution
 @param[in] sFitMode  	scaler fit mode
 @param[in] usWidth  	encode width
 @param[in] usHeight 	encode height
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_H264_WIFI_CustomedResol(MMP_UBYTE ubSnrSel, MMP_UBYTE sFitMode, MMP_USHORT usWidth, MMP_USHORT usHeight, MMP_BOOL bEnable)
{
    if (bEnable) {
        m_sAhcH264WifiInfo[ubSnrSel].bUserDefine    = MMP_TRUE;
        m_sAhcH264WifiInfo[ubSnrSel].sFitMode       = sFitMode;
        m_sAhcH264WifiInfo[ubSnrSel].ulVideoEncW    = usWidth;
        m_sAhcH264WifiInfo[ubSnrSel].ulVideoEncH 	= usHeight;
    }
    else {
        m_sAhcH264WifiInfo[ubSnrSel].bUserDefine    = MMP_FALSE;
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_SetStreamRes
//  Description : This function need to called before MMPS_H264_WIFI_StartStream()
//------------------------------------------------------------------------------
static MMP_ERR MMPS_H264_WIFI_SetStreamRes(void* WifiHandle)
{
    MMP_ULONG               ulScalInW, ulScalInH;
    MMP_ULONG               ulEncWidth, ulEncHeight;
    MMP_H264_WIFISTREAM_OBJ *pWifi = (MMP_H264_WIFISTREAM_OBJ*)WifiHandle;

    if (pWifi->ubWifiSnrSel > SENSOR_MAX_NUM) {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    if (pWifi->ulStreamType == VIDENC_STREAMTYPE_WIFIREAR) {
        
        if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG) ||
            CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264)) {
            #if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
            if (MMP_IsSupportDecMjpegToEncH264()) {
                ulScalInW = m_usAhcDecMjpegToEncodeSrcW;
                ulScalInH = m_usAhcDecMjpegToEncodeSrcH;
            }
            else
            #endif
            {
                #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
                if (MMP_IsSupportDecMjpegToPreview()) {
                    ulScalInW = m_usDecMjpegToPreviewSrcW;
                    ulScalInH = m_usDecMjpegToPreviewSrcH;
                }
                #endif
            }
        }
        else if (CAM_CHECK_USB(USB_CAM_AIT)) {
            #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
            if (MMP_IsSupportDecMjpegToPreview()) {
                ulScalInW = m_usDecMjpegToPreviewSrcW;
                ulScalInH = m_usDecMjpegToPreviewSrcH;
            }
            else
            #endif
            {
                #if (SUPPORT_USB_HOST_FUNC)
                ulScalInW = gusUsbhMaxYuvWidth;
                ulScalInH = gusUsbhMaxYuvHeight;
                #endif
            }
        }
        else if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) ||
                 (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) ||
                 (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
            MMPS_Sensor_GetCurPrevScalInputRes(pWifi->ubWifiSnrSel, &ulScalInW, &ulScalInH);        
        }
    }
    else {
        MMPS_Sensor_GetCurPrevScalInputRes(pWifi->ubWifiSnrSel, &ulScalInW, &ulScalInH);
    }

    /* Calculate encode parameters */
    if (m_sAhcH264WifiInfo[pWifi->ubWifiSnrSel].bUserDefine) {
        ulEncWidth  = m_sAhcH264WifiInfo[pWifi->ubWifiSnrSel].ulVideoEncW;
        ulEncHeight = m_sAhcH264WifiInfo[pWifi->ubWifiSnrSel].ulVideoEncH;
    }
    else {
        ulEncWidth  = m_VidRecdConfigs.usEncWidth[pWifi->WifiEncModes.usVideoEncResIdx];
        ulEncHeight = m_VidRecdConfigs.usEncHeight[pWifi->WifiEncModes.usVideoEncResIdx];
    }

    m_ulWifiStreamW[pWifi->ubWifiSnrSel] = ulEncWidth;
    m_ulWifiStreamH[pWifi->ubWifiSnrSel] = ulEncHeight;

    if (ulEncWidth & 0x0F || ulEncHeight & 0x0F) {
        RTNA_DBG_Str0("The Encode Width/Height must be 16x\r\n");
    }

    // The RT mode need padding 8 lines and Frame/RT mode need set crop 8 line
    ulEncHeight = (ulEncWidth == 1920 && ulEncHeight == 1088) ? 1080 : ulEncHeight;
    ulEncHeight = (ulEncWidth == 1440 && ulEncHeight == 1088) ? 1080 : ulEncHeight;
    ulEncHeight = (ulEncWidth == 640 && ulEncHeight == 368) ? 360 : ulEncHeight;

    if ((ulEncWidth > ulScalInW) || (ulEncHeight > ulScalInH)) {
        RTNA_DBG_Str0("[Wifi] Check the input/output setting!\r\n");
    }

    m_ulWifiStreamScalOutW[pWifi->ubWifiSnrSel] = ulEncWidth;
    m_ulWifiStreamScalOutH[pWifi->ubWifiSnrSel] = ulEncHeight;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_SetGraToH264WifiPipeConfig
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set rear cam H264 WiFi(live view) pipe configuration.
 @param[in] *pInputBuf 		Pointer to HW used buffer.
 @param[in] usEncInputW 	The encode buffer width (for scaler out stage).
 @param[in] usEncInputH 	The encode buffer height (for scaler out stage).
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_H264_WIFI_SetGraToH264WifiPipeConfig(MMP_H264_WIFISTREAM_OBJ   *pWifi, 
                                                  VIDENC_INPUT_BUF	        *pInputBuf, 
                                                  MMP_USHORT			 	usEncInputW,
                                                  MMP_USHORT			 	usEncInputH,
                                                  MMP_ULONG			 	    ubEncId)
{
#if (SUPPORT_USB_HOST_FUNC)
    MMP_SCAL_FIT_RANGE          fitrange;
    MMP_SCAL_GRAB_CTRL          EncodeGrabctl;
    MMPD_FCTL_ATTR              fctlAttr;
    MMP_USHORT                  i;
    MMP_GRAPHICS_BUF_ATTR       srcBuf      = {0, };
    MMP_GRAPHICS_RECT           srcRect     = {0, };
    MMPF_USBH_UVC_STREAM_CFG    *pUVCCfg    = MMPF_USBH_GetUVCCFG(); // TBD
    
    /* Parameter Check */
    if (pWifi->ubWifiSnrSel != USBH_SENSOR) {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    if (pInputBuf == NULL) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID) {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    /* Config Video Record Pipe */
    fitrange.fitmode        = MMP_SCAL_FITMODE_OPTIMAL;
    fitrange.scalerType     = MMP_SCAL_TYPE_SCALER;
    fitrange.ulInWidth      = pUVCCfg->mPrevw.usWidth;
    fitrange.ulInHeight     = pUVCCfg->mPrevw.usHeight;
    fitrange.ulOutWidth     = usEncInputW;
    fitrange.ulOutHeight    = usEncInputH;

    fitrange.ulInGrabX      = 1;
    fitrange.ulInGrabY      = 1;
    fitrange.ulInGrabW      = fitrange.ulInWidth;
    fitrange.ulInGrabH      = fitrange.ulInHeight;
    fitrange.ubChoseLit     = 0;

    MMPD_Scaler_GetGCDBestFitScale(&fitrange, &EncodeGrabctl);
    
    fctlAttr.colormode          = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
    fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
    fctlAttr.fctllink           = pWifi->FctlLink;
    fctlAttr.fitrange           = fitrange;
    fctlAttr.grabctl            = EncodeGrabctl;
    fctlAttr.scalsrc            = MMP_SCAL_SOURCE_GRA;
    fctlAttr.bSetScalerSrc      = MMP_TRUE;
    fctlAttr.usBufCnt           = pInputBuf->ulBufCnt;
    fctlAttr.bUseRotateDMA      = MMP_FALSE;

    for (i = 0; i < fctlAttr.usBufCnt; i++) {
        if (pInputBuf->ulY[i] != 0)
            fctlAttr.ulBaseAddr[i] = pInputBuf->ulY[i];
        if (pInputBuf->ulU[i] != 0)
            fctlAttr.ulBaseUAddr[i] = pInputBuf->ulU[i];
        if (pInputBuf->ulV[i] != 0)
            fctlAttr.ulBaseVAddr[i] = pInputBuf->ulV[i];
    }
    
    /* Dual H264 P1 Only support frame mode */
    if (pWifi->WifiEncModes.VidCurBufMode == VIDENC_CURBUF_RT) {
        fctlAttr.bRtModeOut = MMP_TRUE;
        MMPD_Fctl_SetPipeAttrForH264Rt(&fctlAttr, usEncInputW);
    }
    else {
        fctlAttr.bRtModeOut = MMP_FALSE;
        fctlAttr.sScalDelay = m_sFullSpeedScalDelay;
        MMPD_Fctl_SetPipeAttrForH264FB(&fctlAttr);
    }

    /* Config Graphics Module */
    srcBuf.usWidth          = fitrange.ulInWidth;
    srcBuf.usHeight         = fitrange.ulInHeight;
    #if (DEVICE_YUV_TYPE == ST_YUY2)
    srcBuf.usLineOffset     = srcBuf.usWidth * 2;
    srcBuf.colordepth       = MMP_GRAPHICS_COLORDEPTH_YUV422_UYVY;
    #else
    srcBuf.usLineOffset     = srcBuf.usWidth * 1;
    srcBuf.colordepth       = MMP_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE;
    #endif
    srcBuf.ulBaseAddr       = 0;
    srcBuf.ulBaseUAddr      = 0;
    srcBuf.ulBaseVAddr      = 0;

    srcRect.usLeft          = 0;
    srcRect.usTop           = 0;
    srcRect.usWidth         = srcBuf.usWidth;
    srcRect.usHeight        = srcBuf.usHeight;

    MMPD_Graphics_SetScaleAttr(&srcBuf, &srcRect, 1);
    MMPD_Graphics_SetDelayType(MMP_GRAPHICS_DELAY_CHK_SCA_BUSY);
    MMPD_Graphics_SetPixDelay(13, 20);
    MMPD_Graphics_SetLineDelay(0);
    
    /* Set Link Type */
    MMPD_Fctl_LinkPipeToWifi(pWifi->FctlLink.ibcpipeID, ubEncId);

    /* Tune MCI priority of encode pipe for frame based mode */
    if (pWifi->WifiEncModes.VidCurBufMode == VIDENC_CURBUF_FRAME) {
        MMPD_VIDENC_TunePipeMaxMCIPriority(pWifi->FctlLink.ibcpipeID);
    }
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_SetDecMjpegToH264WifiPipeConfig
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set rear cam H264 WiFi(live view) pipe configuration.
 @param[in] *pInputBuf 		Pointer to HW used buffer.
 @param[in] usEncInputW 	The encode buffer width (for scaler out stage).
 @param[in] usEncInputH 	The encode buffer height (for scaler out stage).
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_H264_WIFI_SetDecMjpegToH264WifiPipeConfig(MMP_H264_WIFISTREAM_OBJ  *pWifi,
                                                       VIDENC_INPUT_BUF	        *pInputBuf,
                                                       MMP_USHORT               usEncInputW,
                                                       MMP_USHORT               usEncInputH,
                                                       MMP_ULONG                ubEncId)
{
#if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
    MMP_SCAL_FIT_RANGE      fitrange;
    MMP_SCAL_GRAB_CTRL      EncodeGrabctl;
    MMPD_FCTL_ATTR          fctlAttr;
    MMP_USHORT              i;

    if (!MMP_IsSupportDecMjpegToEncH264()) {
        return MMP_ERR_NONE;
    }
    
    /* Parameter Check */
    if (pWifi->ubWifiSnrSel != USBH_SENSOR) {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    if (pInputBuf == NULL) {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_INVALID) {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    MMPD_DSC_SetDecMjpegToEncodePipe((MMP_UBYTE)pWifi->FctlLink.ibcpipeID);

    /* Config Video Record Pipe */
    fitrange.fitmode        = m_sAhcDecMjpegToEncodeInfo.sFitMode;
    fitrange.scalerType 	= MMP_SCAL_TYPE_SCALER;
    fitrange.ulInWidth 	    = m_usAhcDecMjpegToEncodeSrcW;
    fitrange.ulInHeight	    = m_usAhcDecMjpegToEncodeSrcH;
    fitrange.ulOutWidth     = usEncInputW;
    fitrange.ulOutHeight    = usEncInputH;

    fitrange.ulInGrabX 		= 1;
    fitrange.ulInGrabY 		= 1;
    fitrange.ulInGrabW 		= fitrange.ulInWidth;
    fitrange.ulInGrabH 		= fitrange.ulInHeight;
    fitrange.ubChoseLit     = 0;

    MMPD_Scaler_GetGCDBestFitScale(&fitrange, &EncodeGrabctl);
    
    fctlAttr.colormode          = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
    fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
    fctlAttr.fctllink           = pWifi->FctlLink;
    fctlAttr.fitrange           = fitrange;
    fctlAttr.grabctl            = EncodeGrabctl;
    fctlAttr.scalsrc            = MMP_SCAL_SOURCE_JPG;
    fctlAttr.bSetScalerSrc      = MMP_TRUE;
    fctlAttr.usBufCnt           = pInputBuf->ulBufCnt;
    fctlAttr.bUseRotateDMA      = MMP_FALSE;

    for (i = 0; i < fctlAttr.usBufCnt; i++) {
        if (pInputBuf->ulY[i] != 0)
            fctlAttr.ulBaseAddr[i] = pInputBuf->ulY[i];
        if (pInputBuf->ulU[i] != 0)
            fctlAttr.ulBaseUAddr[i] = pInputBuf->ulU[i];
        if (pInputBuf->ulV[i] != 0)
            fctlAttr.ulBaseVAddr[i] = pInputBuf->ulV[i];
    }
    
    /* Dual H264 P1 Only support frame mode */
    if (pWifi->WifiEncModes.VidCurBufMode == VIDENC_CURBUF_RT) {
        fctlAttr.bRtModeOut = MMP_TRUE;
        MMPD_Fctl_SetPipeAttrForH264Rt(&fctlAttr, usEncInputW);
    }
    else {
        fctlAttr.bRtModeOut = MMP_FALSE;
        fctlAttr.sScalDelay = m_sFullSpeedScalDelay;
        MMPD_Fctl_SetPipeAttrForH264FB(&fctlAttr);
    }
    
    #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    if (MMP_IsSupportDecMjpegToPreview()) {
        // WIFI mode need change color format for NV12
        MMPS_3GPRECD_SetDispColorFmtToJpgAttr(MMP_DISPLAY_COLOR_YUV420_INTERLEAVE);
    }
    #endif

    MMPD_Fctl_LinkPipeToWifi(pWifi->FctlLink.ibcpipeID, ubEncId);

    /* Tune MCI priority of encode pipe for frame based mode */
    if (pWifi->WifiEncModes.VidCurBufMode == VIDENC_CURBUF_FRAME) {
        MMPD_VIDENC_TunePipeMaxMCIPriority(pWifi->FctlLink.ibcpipeID);
    }

#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_SetStreamPipeConfig
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set video record pipe configuration.
 @param[in] *pInputBuf 		Pointer to HW used buffer.
 @param[in] usEncInputW 	The encode buffer width (for scaler out stage).
 @param[in] usEncInputH 	The encode buffer height (for scaler out stage).
 @retval MMP_ERR_NONE Success.
*/
/*static*/ MMP_ERR MMPS_H264_WIFI_SetStreamPipeConfig( 	MMP_H264_WIFISTREAM_OBJ	*pWifi, 
													VIDENC_INPUT_BUF 	    *pInputBuf,
													MMP_USHORT			 	usEncInputW,
											     	MMP_USHORT			 	usEncInputH,
											     	MMP_USHORT             	ubEncId)
{
    MMP_ULONG				ulScalInW, ulScalInH;
    MMP_SCAL_FIT_MODE		sFitMode;
    MMP_SCAL_FIT_RANGE  	fitrange;
    MMP_SCAL_GRAB_CTRL  	EncodeGrabctl;
    MMPD_FCTL_ATTR 			fctlAttr;
    MMP_USHORT				i;
    MMP_SCAL_FIT_RANGE 		sFitRangeBayer;
    MMP_SCAL_GRAB_CTRL		sGrabctlBayer;
    MMP_USHORT				usCurZoomStep = 0;

    /* Parameter Check */
    if (pWifi->ubWifiSnrSel > SCD_SENSOR) {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    if (pInputBuf == NULL) {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    /* Get the sensor parameters */
    MMPS_Sensor_GetCurPrevScalInputRes(pWifi->ubWifiSnrSel, &ulScalInW, &ulScalInH);

    MMPD_BayerScaler_GetZoomInfo(MMP_BAYER_SCAL_DOWN, &sFitRangeBayer, &sGrabctlBayer);

    if (m_sAhcH264WifiInfo[pWifi->ubWifiSnrSel].bUserDefine) {
        sFitMode = m_sAhcH264WifiInfo[pWifi->ubWifiSnrSel].sFitMode;
    }
    else {
        sFitMode = MMP_SCAL_FITMODE_OUT;
    }

    /* Initial zoom relative config */
    MMPS_3GPRECD_InitDigitalZoomParam(pWifi->FctlLink.scalerpath);

    MMPS_3GPRECD_RestoreDigitalZoomRange(pWifi->FctlLink.scalerpath);

    if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE)
    {
        // Config Wifi Streaming Pipe
        fitrange.fitmode        = sFitMode;
        fitrange.scalerType 	= MMP_SCAL_TYPE_SCALER;

        if (MMP_IsVidPtzEnable()) {
            fitrange.ulInWidth	= sFitRangeBayer.ulOutWidth;
            fitrange.ulInHeight	= sFitRangeBayer.ulOutHeight;
        }
        else {
            fitrange.ulInWidth 	= ulScalInW;
            fitrange.ulInHeight	= ulScalInH;
        }

        if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
        
            if (gbDualBayerSnrInDSCMode == MMP_FALSE) {
                fitrange.ulOutWidth     = usEncInputW / 2;
                fitrange.ulOutHeight    = usEncInputH; 
            }
            else {
                fitrange.ulInWidth 	    = FLOOR32(ulScalInW) * 2;
                fitrange.ulInHeight	    = FLOOR32(ulScalInH);
                fitrange.ulOutWidth     = usEncInputW;
                fitrange.ulOutHeight    = usEncInputH;
            }
        }
        else {
            fitrange.ulOutWidth     = usEncInputW;
            fitrange.ulOutHeight    = usEncInputH;
        }

        fitrange.ulInGrabX      = 1;
        fitrange.ulInGrabY      = 1;
        fitrange.ulInGrabW      = fitrange.ulInWidth;
        fitrange.ulInGrabH      = fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        
        MMPD_PTZ_InitPtzInfo(pWifi->FctlLink.scalerpath,
                             fitrange.fitmode,
                             fitrange.ulInWidth, fitrange.ulInHeight,
                             fitrange.ulOutWidth, fitrange.ulOutHeight);

		// Be sync with preview path : TBD
		if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
	    	MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
	    }
	    else if (pWifi->ubWifiSnrSel == SCD_SENSOR && MMP_IsDualVifCamEnable()) {
			#if 0 // TBD not to use this first
	    	if (MMP_GetDualSnrPrevwType() == DUALSNR_DUAL_PREVIEW)
	        	MMPD_PTZ_GetCurPtzStep(m_2ndPrewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
	    	else
	    	#endif
	    	    MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
	    }

	    if (usCurZoomStep > m_VidRecordZoomInfo.usMaxZoomSteps) {
	        usCurZoomStep = m_VidRecordZoomInfo.usMaxZoomSteps;
	    }

        MMPD_PTZ_CalculatePtzInfo(pWifi->FctlLink.scalerpath, usCurZoomStep, 0, 0);

        MMPD_PTZ_GetCurPtzInfo(pWifi->FctlLink.scalerpath, &fitrange, &EncodeGrabctl);

        if (MMP_IsVidPtzEnable()) {
            MMPD_PTZ_ReCalculateGrabRange(&fitrange, &EncodeGrabctl);
        }
        
        fctlAttr.colormode          = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
        fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
        fctlAttr.fctllink           = pWifi->FctlLink;
        fctlAttr.fitrange           = fitrange;
        fctlAttr.grabctl            = EncodeGrabctl;
        
        if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
            fctlAttr.scalsrc        = (gbDualBayerSnrInDSCMode) ? MMP_SCAL_SOURCE_GRA : MMP_SCAL_SOURCE_ISP;
        }
        else {
            fctlAttr.scalsrc        = pWifi->LinkAttr.scalerSrc; // Change for TV DEC need graphic src and SONIX rear cam need jpeg src.
        }
        
        fctlAttr.bSetScalerSrc      = MMP_TRUE;
        fctlAttr.ubPipeLinkedSnr    = pWifi->ubWifiSnrSel;
        fctlAttr.usBufCnt           = pInputBuf->ulBufCnt;
        fctlAttr.bUseRotateDMA      = MMP_FALSE;

        for (i = 0; i < fctlAttr.usBufCnt; i++) {
            if (pInputBuf->ulY[i] != 0)
                fctlAttr.ulBaseAddr[i] = pInputBuf->ulY[i];
            if (pInputBuf->ulU[i] != 0)
                fctlAttr.ulBaseUAddr[i] = pInputBuf->ulU[i];
            if (pInputBuf->ulV[i] != 0)
                fctlAttr.ulBaseVAddr[i] = pInputBuf->ulV[i];
        }

        if (pWifi->WifiEncModes.VidCurBufMode == VIDENC_CURBUF_RT) {
            fctlAttr.bRtModeOut = MMP_TRUE;
            MMPD_Fctl_SetPipeAttrForH264Rt(&fctlAttr, usEncInputW);
        }
        else {
            fctlAttr.bRtModeOut = MMP_FALSE;
            fctlAttr.sScalDelay = m_sFullSpeedScalDelay;
            MMPD_Fctl_SetPipeAttrForH264FB(&fctlAttr);
            
            if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) || 
                (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
                if (pWifi->ubWifiSnrSel == SCD_SENSOR && MMP_IsDualVifCamEnable()) {
                    MMPD_Scaler_SetLineDelay(pWifi->FctlLink.scalerpath, 0xC0);
                }
            }
        }

        MMPD_Fctl_LinkPipeToWifi(pWifi->FctlLink.ibcpipeID, ubEncId);
    }
    else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE ||
             m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI 	||
             m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE)
    {
        // Config Wifi Streaming Pipe
        fitrange.fitmode        = MMP_SCAL_FITMODE_OUT;
        fitrange.scalerType 	= MMP_SCAL_TYPE_SCALER;
        
        if (MMP_IsDualVifCamEnable() && MMP_GetParallelFrmStoreType() != PARALLEL_FRM_NOT_SUPPORT)
            fitrange.ulInWidth  = m_ulLdcMaxOutWidth * 2;
        else
            fitrange.ulInWidth  = m_ulLdcMaxOutWidth;
        
        fitrange.ulInHeight     = m_ulLdcMaxOutHeight;
        fitrange.ulOutWidth     = usEncInputW;
        fitrange.ulOutHeight	= usEncInputH;

        fitrange.ulInGrabX 		= 1;
        fitrange.ulInGrabY 		= 1;
        fitrange.ulInGrabW 		= fitrange.ulInWidth;
        fitrange.ulInGrabH 		= fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        
        MMPD_PTZ_InitPtzInfo(pWifi->FctlLink.scalerpath,
        					 fitrange.fitmode,
				             fitrange.ulInWidth, fitrange.ulInHeight, 
				             fitrange.ulOutWidth, fitrange.ulOutHeight);

		// Be sync with preview path : TBD
		if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
			MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
	    }
	    else if (pWifi->ubWifiSnrSel == SCD_SENSOR && MMP_IsDualVifCamEnable()) {
	        if (MMP_GetDualSnrPrevwType() == DUALSNR_DUAL_PREVIEW)
	        	MMPD_PTZ_GetCurPtzStep(m_2ndPrewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
	    	else
	    		MMPD_PTZ_GetCurPtzStep(m_PreviewFctlLink.scalerpath, NULL, &usCurZoomStep, NULL, NULL);
	    }

        if (usCurZoomStep > m_VidRecordZoomInfo.usMaxZoomSteps) {
            usCurZoomStep = m_VidRecordZoomInfo.usMaxZoomSteps;
        }

        MMPD_PTZ_CalculatePtzInfo(pWifi->FctlLink.scalerpath, usCurZoomStep, 0, 0);

        MMPD_PTZ_GetCurPtzInfo(pWifi->FctlLink.scalerpath, &fitrange, &EncodeGrabctl);

        if (MMP_IsVidPtzEnable()) {
            MMPD_PTZ_ReCalculateGrabRange(&fitrange, &EncodeGrabctl);
        }
        
        fctlAttr.colormode          = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
        fctlAttr.eScalColorRange    = MMP_SCAL_COLRMTX_FULLRANGE_TO_BT601;
        fctlAttr.fctllink           = pWifi->FctlLink;
        fctlAttr.fitrange           = fitrange;
        fctlAttr.grabctl            = EncodeGrabctl;
        fctlAttr.usBufCnt           = pInputBuf->ulBufCnt;
        fctlAttr.scalsrc            = MMP_SCAL_SOURCE_LDC;
        fctlAttr.bSetScalerSrc      = MMP_TRUE;
        fctlAttr.ubPipeLinkedSnr    = PRM_SENSOR;
        fctlAttr.bUseRotateDMA      = MMP_FALSE;
        fctlAttr.usRotateBufCnt     = 0;

        if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
            fctlAttr.scalsrc        = MMP_SCAL_SOURCE_GRA;
        }

        for (i = 0; i < fctlAttr.usBufCnt; i++) {
            if (pInputBuf->ulY[i] != 0)
                fctlAttr.ulBaseAddr[i] = pInputBuf->ulY[i];
            if (pInputBuf->ulU[i] != 0)	
                fctlAttr.ulBaseUAddr[i] = pInputBuf->ulU[i];
            if (pInputBuf->ulV[i] != 0)
                fctlAttr.ulBaseVAddr[i] = pInputBuf->ulV[i];
        }

        if (pWifi->WifiEncModes.VidCurBufMode == VIDENC_CURBUF_RT) {
            fctlAttr.bRtModeOut = MMP_TRUE;
            MMPD_Fctl_SetPipeAttrForH264Rt(&fctlAttr, usEncInputW);
        }
        else {
            fctlAttr.bRtModeOut = MMP_FALSE;
            fctlAttr.sScalDelay = m_sFullSpeedScalDelay;
            MMPD_Fctl_SetPipeAttrForH264FB(&fctlAttr);
        }

        MMPD_Fctl_LinkPipeToWifi(pWifi->FctlLink.ibcpipeID, ubEncId);
    }
    
    // Tune MCI priority of encode pipe for frame based mode
    if (pWifi->WifiEncModes.VidCurBufMode == VIDENC_CURBUF_FRAME) {
        MMPD_VIDENC_TunePipeMaxMCIPriority(pWifi->FctlLink.ibcpipeID);
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_EnableStreamPipe
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Turn on and off record for video encode.

 @param[in] bEnable : Enable and disable scaler path for video encode.
 @param[in] bForceEn : For some case,need to disable streaming pipe without checking status.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_H264_WIFI_EnableStreamPipe(MMP_H264_WIFISTREAM_OBJ *pWifi, MMP_BOOL bEnable ,MMP_BOOL bForceEn)
{
    MMP_ERR sRet = MMP_ERR_NONE;

    if ((m_bH264WifiStreamActive[pWifi->ubWifiSnrSel] ^ bEnable) || bForceEn){
#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
        if ((MMP_IsSupportDecMjpegToPreview()) && (pWifi->ubWifiSnrSel == USBH_SENSOR)) {
            if (bEnable){
                sRet = MMPD_JPEG_Ctl_ResumePreview();
            }
            else{
                sRet = MMPD_JPEG_Ctl_PausePreview(); // PAUSE JPEG DECODE TO PREVIEW.
            }
            sRet = MMPD_Fctl_EnablePreview(USBH_SENSOR, pWifi->FctlLink.ibcpipeID, bEnable, MMP_FALSE);
        }
        else 
#endif
        {
            sRet = MMPD_Fctl_EnablePreview(pWifi->ubWifiSnrSel, pWifi->FctlLink.ibcpipeID, bEnable, MMP_FALSE);
        }

        m_bH264WifiStreamActive[pWifi->ubWifiSnrSel] = bEnable;
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_ReserveStreamBuf
//  Description : Reserve memory for wifi stream.
//------------------------------------------------------------------------------
MMP_ERR MMPS_H264_WIFI_ReserveStreamBuf(MMP_ULONG *ulCurBufPos, MMP_USHORT usRsvdMaxW, MMP_USHORT usRsvdMaxH, MMP_UBYTE ubEncBufMode)
{
    const MMP_ULONG AVSyncBufSize         = 32;
    const MMP_ULONG VideoSizeTableSize    = 1024;
    const MMP_ULONG VideoTimeTableSize    = 1024;
    const MMP_ULONG SPSSize               = 48;
    const MMP_ULONG PPSSize               = 16;
    const MMP_ULONG H264HeaderSize        = ((SPSSize + PPSSize + 31) >> 5) << 5;
    const MMP_ULONG VideoCompBufSize      = m_VidRecdConfigs.ulWifiVideoCompBufSize;
    const MMP_ULONG SliceLengBufSize      = ALIGN_X((4 * ((usRsvdMaxH>>4) + 2)), 64);

    MMP_ULONG	    ulCurAddr = *ulCurBufPos;
    MMP_ULONG	    ulEncBufCnt = 2;
    MMP_ULONG       MergerBufSize;
    MMP_ULONG       ulEncFrameSize;
    MMP_ULONG       TotalMemSize;
    
    m_ulVidWifiStreamStartAddr = ulCurAddr = ALIGN32(ulCurAddr);
	
    MergerBufSize = AVSyncBufSize + VideoSizeTableSize + VideoTimeTableSize;
    MergerBufSize = ALIGN32(MergerBufSize);

    // Get Encode buffer config and size (NV12)
    if (ubEncBufMode == VIDENC_CURBUF_FRAME) {
        ulEncBufCnt = 2;
    }
    else {
        ulEncBufCnt = 0;
    }
    ulEncFrameSize = ALIGN32((usRsvdMaxW * usRsvdMaxH * 3) / 2);

    // Calculate total memory for encode
	TotalMemSize = MergerBufSize             +
				   #if (SHARE_REF_GEN_BUF == 1)
				   (ulEncFrameSize + 32) * (ulEncBufCnt + 1) + 
				   #else
				   (ulEncFrameSize + 32) * (ulEncBufCnt + 2) +
				   #endif
				   H264HeaderSize            +
				   VideoCompBufSize          +
				   SliceLengBufSize;

    *ulCurBufPos = ulCurAddr + TotalMemSize;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_AssignObjEntity
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_H264_WIFI_AssignObjEntity(MMP_UBYTE ubSnrSel, void** pHandle)
{
    MMP_ERR sRet = MMP_ERR_NONE;
    MMP_H264_WIFISTREAM_OBJ *pWifi;

    *pHandle = (void*)(&m_sH264WifiStreamObj[ubSnrSel]);
    MEMSET(&m_sH264WifiStreamObj[ubSnrSel], 0, sizeof(MMP_H264_WIFISTREAM_OBJ));
    pWifi = (MMP_H264_WIFISTREAM_OBJ *)*pHandle;
    pWifi->ubWifiSnrSel = ubSnrSel;
    
    return sRet;    
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_GetPreviewFctlLink
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_H264_WIFI_GetPreviewFctlLink(MMP_UBYTE ubSnrSel, MMP_PIPE_LINK *pWifiFctlLink, MMP_BOOL *pbNeedBackupPipeSetting)
{
    MMP_ERR sRet = MMP_ERR_NONE;

    *pbNeedBackupPipeSetting = MMP_TRUE;
        
    if (ubSnrSel == PRM_SENSOR) {
        *pWifiFctlLink = m_PreviewFctlLink;
    }
    else if (ubSnrSel == SCD_SENSOR) {
    
        if (MMP_IsDualVifCamEnable()) {
            if (CAM_CHECK_SCD(SCD_CAM_TV_DECODER) || 
                CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR)) {
                *pWifiFctlLink = m_2ndPrewFctlLink;
            }
            else if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
                *pWifiFctlLink = m_2ndPrewFctlLink; // TBD
            }
        }
    }
    else if (ubSnrSel == USBH_SENSOR) {
    
        #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
        if (MMP_IsSupportDecMjpegToPreview()) {
            // Change to use JPEG preview FCTL link.
            *pWifiFctlLink = m_DecMjpegToPrevwFctlLink;
        }
        else {
            *pWifiFctlLink = m_2ndPrewFctlLink; // TBD, for AIT NV12 format
            *pbNeedBackupPipeSetting = MMP_FALSE;
        }
        #endif
    }
    else{
        MMP_PRINT_RET_ERROR(0, 0, "", 0);
    }
    
    return sRet;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_BackupPreviewPipe
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_H264_WIFI_BackupPreviewPipe(void** pHandle)
{
    MMP_H264_WIFISTREAM_OBJ 	**ppHeadWiFiHandler = (MMP_H264_WIFISTREAM_OBJ**)pHandle;
    MMP_H264_WIFISTREAM_OBJ 	*pWifi;    
    MMP_ERR sRet = MMP_ERR_NONE;

    pWifi = *ppHeadWiFiHandler;    
    if(NULL == pWifi){ MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk); return sRet;}
    
    // Backup scaler source path.
    MMPD_Scaler_GetPath(pWifi->FctlLink.scalerpath, &(pWifi->LinkAttr.scalerSrc));
    
    // Backup scaler attribute.
    MMPD_Scaler_BackupAttributes(pWifi->FctlLink.scalerpath);
    
    // Backup IBC link attribute
    MMPD_Fctl_GetIBCLinkAttr(pWifi->FctlLink.scalerpath, &(pWifi->LinkAttr.IBCLinkType), &(pWifi->LinkAttr.previewDev), &(pWifi->LinkAttr.winID), &(pWifi->LinkAttr.rotateDir));

    // Backup IBC output color format.
    #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    if (MMP_IsUSBCamIsoMode() && MMP_IsSupportDecMjpegToPreview()) {
        MMPS_3GPRECD_GetDispColorFmtToJpgAttr(&pWifi->LinkAttr.IBCColorFormat);
    }
    #endif

    return sRet;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_ResetPreviewPipe
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_H264_WIFI_ResetPreviewPipe(void** pHandle)
{
    MMP_H264_WIFISTREAM_OBJ 	**ppHeadWiFiHandler = (MMP_H264_WIFISTREAM_OBJ**)pHandle;
    MMP_H264_WIFISTREAM_OBJ 	*pWifi;    
    MMP_ERR sRet = MMP_ERR_NONE;

    pWifi = *ppHeadWiFiHandler;    
    if(NULL == pWifi){ MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk); return sRet;}
    
    MMPD_Fctl_ResetIBCLinkType(pWifi->FctlLink.ibcpipeID);

    #if 1 // Change to scaler disable.
    MMPD_Scaler_SetEnable(pWifi->FctlLink.scalerpath, MMP_FALSE);
    MMPD_System_Sleep(40);
    #else
    MMPD_Scaler_SetPath(WifiFctlLink.scalerpath, MMP_SCAL_SOURCE_JPG, MMP_TRUE);
    #endif

    return sRet;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_GetStreamActive
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_H264_WIFI_GetStreamActive(MMP_UBYTE ubSnrSel, MMP_UBYTE *pbEnable)
{
    MMP_ERR sRet = MMP_ERR_NONE;

    *pbEnable = MMP_FALSE;

    if (m_bH264WifiStreamActive[ubSnrSel]) {
        *pbEnable = MMP_TRUE;
    }

    return sRet;        
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_FastSwitchSensorStream
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_H264_WIFI_FastSwitchSensorStream(void** pNewHandle, void** pCurHandle)
{
    MMP_H264_WIFISTREAM_OBJ 	**ppCurWiFiHandler = (MMP_H264_WIFISTREAM_OBJ**)pCurHandle;
    MMP_H264_WIFISTREAM_OBJ 	**ppNewWiFiHandler = (MMP_H264_WIFISTREAM_OBJ**)pNewHandle;
    MMP_H264_WIFISTREAM_OBJ 	*pCurWifi;    
    MMP_H264_WIFISTREAM_OBJ 	*pNewWifi;    
    MMP_ERR sRet = MMP_ERR_NONE;

    pCurWifi = *ppCurWiFiHandler;    
    pNewWifi = *ppNewWiFiHandler;    

    if((NULL == pNewWifi) || (NULL == pCurWifi)){ MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk); return sRet; }
    if(pNewWifi->ubWifiSnrSel == pCurWifi->ubWifiSnrSel){ MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk); return sRet; }
    
    //Disable current sensor stream.
    sRet = MMPS_H264_WIFI_EnableStreamPipe(pCurWifi, MMP_FALSE, MMP_FALSE);
    sRet = MMPS_H264_WIFI_ResetPreviewPipe(pCurHandle);
    sRet = MMPS_H264_WIFI_ResetPreviewPipe(pNewHandle);

    //Copy current attribute to new stream.
    pNewWifi->usEncID = pCurWifi->usEncID;
    m_WifiEncInputBuf[pNewWifi->ubWifiSnrSel] = m_WifiEncInputBuf[pCurWifi->ubWifiSnrSel];
    m_ulWifiStreamScalOutW[pNewWifi->ubWifiSnrSel] = m_ulWifiStreamScalOutW[pCurWifi->ubWifiSnrSel];
    m_ulWifiStreamScalOutH[pNewWifi->ubWifiSnrSel] = m_ulWifiStreamScalOutH[pCurWifi->ubWifiSnrSel];     
            
    sRet = MMPS_H264_WIFI_SetStreamPipeConfig(pNewWifi, 
                                        &m_WifiEncInputBuf[pNewWifi->ubWifiSnrSel], 
                                        m_ulWifiStreamScalOutW[pNewWifi->ubWifiSnrSel], 
                                        m_ulWifiStreamScalOutH[pNewWifi->ubWifiSnrSel], 
                                        pNewWifi->usEncID);
    if(sRet != MMP_ERR_NONE){ MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk); return sRet;}

    //Enable new sensor stream.
    sRet = MMPS_H264_WIFI_EnableStreamPipe(pNewWifi, MMP_TRUE, MMP_FALSE);
    if(sRet != MMP_ERR_NONE){ MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk); return sRet;}
  
    return sRet;        
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_OpenStream
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_H264_WIFI_OpenStream(MMP_UBYTE ubSnrSel, void** pHandle, MMP_BOOL bUsePrevwPipe, MMP_UBYTE bVRMode)
{
    MMP_H264_WIFISTREAM_OBJ *pWifi;
    MMP_PIPE_LINK           WifiFctlLink;
    MMP_BOOL                bBackupPipeSetting = MMP_TRUE;
    MMP_ERR sRet = MMP_ERR_NONE;

    if (ubSnrSel >= SENSOR_MAX_NUM) {
        return MMP_WIFI_ERR_PARAMETER;
    }

    *pHandle = NULL;
    sRet = MMPS_H264_WIFI_AssignObjEntity(ubSnrSel, pHandle);
    pWifi    = (MMP_H264_WIFISTREAM_OBJ *)*pHandle;
    
    pWifi->bEnableWifi	= MMP_TRUE;
    //pWifi->ubWifiSnrSel = ubSnrSel;

    if (bUsePrevwPipe) {
        sRet = MMPS_H264_WIFI_GetPreviewFctlLink(ubSnrSel, &WifiFctlLink, &bBackupPipeSetting);
        pWifi->FctlLink		= WifiFctlLink;
        
        if (bBackupPipeSetting) {            
            sRet = MMPS_H264_WIFI_BackupPreviewPipe(pHandle);
            //We use LCD preview pipe as WIFI preview pipe and LCD preview pipe is always on,
            //so we must force to stop preview pipe here. 
            sRet = MMPS_H264_WIFI_EnableStreamPipe(pWifi, MMP_FALSE, MMP_TRUE);
            sRet = MMPS_H264_WIFI_ResetPreviewPipe(pHandle);
        }
    }
    else {
        if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE   ||
            m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI    ||
            m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) {
            WifiFctlLink = (bVRMode) ? (m_LdcH264WifiVRFctlLink) : (m_LdcH264WifiDSCFctlLink);
        }
        else {
            WifiFctlLink = m_WifiDSCFctlLink;
        }

        pWifi->FctlLink		= WifiFctlLink;
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_StartStream
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Start Wifi stream video encode.
*/
MMP_ERR MMPS_H264_WIFI_StartStream(void* WifiHandle)
{
    MMP_ULONG           	enc_id = 0;
    VIDENC_FW_STATUS  	    status_vid;
    MMP_ERR             	status;
    MMP_ULONG           	EncWidth, EncHeight;
    MMP_ULONG64         	ullBitrate64, ullTimeIncr64, ullTimeResol64;
    MMP_ULONG           	ulTargetFrameSize;
    MMP_ULONG           	ulFps;
    MMP_ULONG           	ulWifiID;
    MMP_ULONG           	ulTimeout = VR_QUERY_STATES_TIMEOUT;
    MMP_H264_WIFISTREAM_OBJ *pWifi = (MMP_H264_WIFISTREAM_OBJ*)WifiHandle;

    if (pWifi && !pWifi->bEnableWifi) {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }
    
   /* if (m_bH264WifiStreamActive[pWifi->ubWifiSnrSel]) {
        MMPS_H264_WIFI_StopStream(&m_sH264WifiHdl[0]);
        RTNA_DBG_Str(0, "Stop stream before start it\r\n");
    }*/
    
    /* Assign StreamType and ID */
    if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
        ulWifiID = m_ulH264WifiFrontID;
    }
    else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
             pWifi->ubWifiSnrSel == USBH_SENSOR) {
        ulWifiID = m_ulH264WifiRearID;
    }
    else {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    status = MMPS_H264_WIFI_SetStreamRes(WifiHandle);

    if (status != MMP_ERR_NONE) {
        return status;
    }
    
    EncWidth  = m_ulWifiStreamW[pWifi->ubWifiSnrSel];
    EncHeight = m_ulWifiStreamH[pWifi->ubWifiSnrSel];

    ullBitrate64      = pWifi->WifiEncModes.ulBitrate;
    ullTimeIncr64     = pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrement;
    ullTimeResol64    = pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrResol;
    ulTargetFrameSize = (MMP_ULONG)((ullBitrate64 * ullTimeIncr64)/(ullTimeResol64 * 8));

    ulFps = (pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrResol - 1 +
             pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrement) /
             pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrement;

    status = MMPD_VIDENC_CheckCapability(EncWidth, EncHeight, ulFps);

    if (status != MMP_ERR_NONE) {
        return status;
    }
    
    if (ulWifiID == INVALID_ENC_ID)
        status_vid = VIDENC_FW_STATUS_NONE;
    else
        MMPD_VIDENC_GetStatus(ulWifiID, &status_vid);

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP)) {

        if (!MMPD_VIDENC_IsModuleInit()) {
            MMP_ERR	mmpstatus;

            mmpstatus = MMPD_VIDENC_InitModule();
            if (mmpstatus != MMP_ERR_NONE) {
                return mmpstatus;
            }
        }
        
        if (MMPS_H264_WIFI_SetStreamMemoryMap(  &enc_id,
                                                pWifi,
                                                EncWidth,
                                                EncHeight,
                                                m_ulVidRecSramAddr,
                                                m_ulVidWifiStreamStartAddr))
        {
            RTNA_DBG_Str(0, "Alloc mem for wifi stream failed\r\n");
            return MMP_WIFI_ERR_MEM_EXHAUSTED;
        }

        // Set current buffer mode before MMPF_Display_StartPreview() use it
        switch (pWifi->WifiEncModes.VidCurBufMode) {
        case VIDENC_CURBUF_FRAME:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_FRAME);
            break;
        case VIDENC_CURBUF_RT:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_RT);
            break;
        default:
            return MMP_WIFI_ERR_INVALID_PARAMETERS;
        }

        if (MMPS_H264_WIFI_EnableStreamPipe(pWifi, MMP_TRUE, MMP_FALSE) != MMP_ERR_NONE) {
            PRINTF("Enable Video Record: Fail\r\n");
            return MMP_WIFI_ERR_PARAMETER;
        }

        // Set encoder and merger parameters
        if (((EncWidth == 3200) && (EncHeight == 1808)) ||
            ((EncWidth == 1920) && (EncHeight == 1088)) ||
            ((EncWidth == 1440) && (EncHeight == 1088)) ||
            ((EncWidth == 640) && (EncHeight == 368))) {
            
            MMPD_VIDENC_SetCropping(enc_id, 0, 8, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_REPEAT, 8);
        }
        else {
            MMPD_VIDENC_SetCropping(enc_id, 0, 0, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_NONE, 0);
        }

        if (MMPD_VIDENC_SetResolution(enc_id, EncWidth, EncHeight) != MMP_ERR_NONE) {
            return MMP_WIFI_ERR_UNSUPPORTED_PARAMETERS;
        }
        
        if (MMPD_VIDENC_SetProfile(enc_id, pWifi->WifiEncModes.VisualProfile) != MMP_ERR_NONE) {
            return MMP_WIFI_ERR_UNSUPPORTED_PARAMETERS;
        }

        MMPD_VIDENC_SetEncodeMode();

        if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
            MMPD_3GPMGR_SetGOP(VIDENC_STREAMTYPE_WIFIFRONT, pWifi->WifiEncModes.usPFrameCount, pWifi->WifiEncModes.usBFrameCount);
            MMPD_VIDENC_SetGOP(enc_id, pWifi->WifiEncModes.usPFrameCount, pWifi->WifiEncModes.usBFrameCount);
        }
        else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
                 pWifi->ubWifiSnrSel == USBH_SENSOR) {
            MMPD_3GPMGR_SetGOP(VIDENC_STREAMTYPE_WIFIREAR, pWifi->WifiEncModes.usPFrameCount, pWifi->WifiEncModes.usBFrameCount);
            MMPD_VIDENC_SetGOP(enc_id, pWifi->WifiEncModes.usPFrameCount, pWifi->WifiEncModes.usBFrameCount);
        }

        MMPD_VIDENC_SetQuality(enc_id, ulTargetFrameSize, (MMP_ULONG)ullBitrate64);
        
        if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
            MMPD_VIDENC_SetSnrFrameRate(enc_id,
                                        VIDENC_STREAMTYPE_WIFIFRONT,
                                        pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrement,
                                        pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrResol);
        }
        else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
                 pWifi->ubWifiSnrSel == USBH_SENSOR) {
            MMPD_VIDENC_SetSnrFrameRate(enc_id,
                                        VIDENC_STREAMTYPE_WIFIREAR,
                                        pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrement,
                                        pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrResol);
        }
        
        MMPD_VIDENC_SetEncFrameRate(enc_id,
                                    pWifi->WifiEncModes.VideoEncFrameRate.usVopTimeIncrement,
                                    pWifi->WifiEncModes.VideoEncFrameRate.usVopTimeIncrResol);

        if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
            MMPD_3GPMGR_SetFrameRate(VIDENC_STREAMTYPE_WIFIFRONT,
                                     pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrResol,
                                     pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrement);
        }
        else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
                 pWifi->ubWifiSnrSel == USBH_SENSOR) {
            MMPD_3GPMGR_SetFrameRate(VIDENC_STREAMTYPE_WIFIREAR,
                                     pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrResol,
                                     pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrement);
        }
    }

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP) ||
        (status_vid == VIDENC_FW_STATUS_PREENCODE)) {

        if (status_vid != VIDENC_FW_STATUS_PREENCODE) {
            MMPD_VIDENC_EnableClock(MMP_TRUE);
        }

        if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
            if (MMPD_VIDENC_StartStreaming(m_ulH264WifiFrontID, VIDENC_STREAMTYPE_WIFIFRONT) != MMP_ERR_NONE) {
                MMPD_VIDENC_EnableClock(MMP_FALSE);
                return MMP_3GPRECD_ERR_GENERAL_ERROR;
            }
        }
        else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
                 pWifi->ubWifiSnrSel == USBH_SENSOR) {
            if (MMPD_VIDENC_StartStreaming(m_ulH264WifiRearID, VIDENC_STREAMTYPE_WIFIREAR) != MMP_ERR_NONE) {
                MMPD_VIDENC_EnableClock(MMP_FALSE);
                return MMP_3GPRECD_ERR_GENERAL_ERROR;
            }
        }
        
        do {
            MMPD_VIDENC_GetStatus(enc_id, &status_vid);
            if (status_vid != VIDENC_FW_STATUS_START) {
                MMPF_OS_Sleep(1);
            }
        } while ((status_vid != VIDENC_FW_STATUS_START) && (--ulTimeout) > 0);

        if (ulTimeout == 0) {
            RTNA_DBG_Str(0, "Wifi stream H264 NG...\r\n");
            return MMP_WIFI_ERR_PARAMETER;
        }

        pWifi->usEncID = enc_id;
        
        if (status_vid == VIDENC_FW_STATUS_START) {
            return MMP_ERR_NONE;
        }
        else {
            return MMP_ERR_NONE;
        }
    }
    else {
        return MMP_WIFI_ERR_PARAMETER;
    }
}

#if(SUPPORT_H264_WIFI_STREAM)
//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_SetStreamMemoryMap
//  Description :
//------------------------------------------------------------------------------
/**
     @brief Set memory layout by different resolution and memory type for H264.

     Depends on encoded resolution and memory type to map buffers.
     @param[in] usEncW Encode width.
     @param[in] usEncH Encode height.
     @param[in] ulFBufAddr Available start address of frame buffer.
     @param[in] ulStackAddr Available start address of dram buffer.
     @retval MMP_ERR_NONE Success.
 */
MMP_ERR MMPS_H264_WIFI_SetStreamMemoryMap(MMP_ULONG *ulEncId, MMP_H264_WIFISTREAM_OBJ *pWifi, MMP_USHORT usEncW, MMP_USHORT usEncH, MMP_ULONG ulFBufAddr, MMP_ULONG ulStackAddr)
{
    const MMP_ULONG                 AVSyncBufSize         = 32;
    const MMP_ULONG                	VideoSizeTableSize    = 1024;
    const MMP_ULONG                 VideoTimeTableSize    = 1024;
    const MMP_ULONG                 AudioCompBufSize      = 0;
    const MMP_ULONG                 SPSSize               = 48;
    const MMP_ULONG                 PPSSize               = 16;
    const MMP_ULONG                 VideoCompBufSize      = m_VidRecdConfigs.ulWifiVideoCompBufSize;
    const MMP_ULONG                 SliceLengBufSize      = ((4 * ((usEncH>>4) + 2) + 63) >> 6) << 6;//align 64

    MMP_USHORT                      i;
    MMP_ULONG                       ulCurAddr;
    MMP_ULONG                       ulTmpBufSize;
    MMPD_MP4VENC_VIDEOBUF           videohwbuf;
    MMPD_H264ENC_HEADER_BUF         headerbuf;
    MMPD_3GPMGR_AV_COMPRESS_BUF	    mergercompbuf;
    MMPD_3GPMGR_REPACKBUF           repackmiscbuf;
    MMP_ULONG                       bufferEndAddr;
    MMP_ULONG                       ulEncFrameSize;
    
    m_ulVidRecEncodeAddr = ulCurAddr = ulStackAddr;

    if (((usEncW>>4)*(usEncH>>4)) > 34560) { // 4096x2160
        return MMP_WIFI_ERR_UNSUPPORTED_PARAMETERS;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    //  Get Stream Config setting and calculate how many memory will be used    
    //
    ///////////////////////////////////////////////////////////////////////////

    // Get Encode buffer config and size (NV12)
    if (m_sH264WifiStreamObj[pWifi->ubWifiSnrSel].WifiEncModes.VidCurBufMode == VIDENC_CURBUF_RT) {
        m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulBufCnt = 2;
    }
    else {
        m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulBufCnt = 2 + VIDENC_MAX_B_FRAME_NUMS;
    }

    ulEncFrameSize = ALIGN32((usEncW * usEncH * 3) / 2);

    //////////////////////////////////////////////////////////////////////////
    //
    //  Allocate memory for stream buffer
    //
    //////////////////////////////////////////////////////////////////////////
    
    //*************************
    // AV Sync ............ 32
    // Frame Table ........ 1k
    // Time Table ......... 1k
    // Audio BS ........... 16k
    //*************************
    
    // Set av sync buffer
    repackmiscbuf.ulVideoEncSyncAddr    = ulCurAddr;
    ulCurAddr                           += AVSyncBufSize;

    // Set aux size table buffer, must be 512 byte alignment
    repackmiscbuf.ulVideoSizeTableAddr  = ulCurAddr;
    repackmiscbuf.ulVideoSizeTableSize  = VideoSizeTableSize;
    ulCurAddr                           += VideoSizeTableSize;

    // Set aux time table buffer, must be 512 byte alignment
    repackmiscbuf.ulVideoTimeTableAddr  = ulCurAddr;
    repackmiscbuf.ulVideoTimeTableSize  = VideoTimeTableSize;
    ulCurAddr                           += VideoTimeTableSize;

    // Set audio compressed buffer
    mergercompbuf.ulAudioCompBufStart   = ulCurAddr;
    ulCurAddr                           += AudioCompBufSize;
    mergercompbuf.ulAudioCompBufEnd     = ulCurAddr;
    
    // Initialize H264 instance
    if (pWifi->ubWifiSnrSel == PRM_SENSOR) 
    {
        if (MMPD_VIDENC_InitInstance(ulEncId, VIDENC_STREAMTYPE_WIFIFRONT, VIDENC_RC_MODE_CBR) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "WifiStream Err not available h264 instance\r\n");
            return MMP_WIFI_ERR_INVALID_PARAMETERS;
        }
        m_ulH264WifiFrontID = *ulEncId;
    }
    else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
             pWifi->ubWifiSnrSel == USBH_SENSOR) 
    {
        if (MMPD_VIDENC_InitInstance(ulEncId, VIDENC_STREAMTYPE_WIFIREAR, VIDENC_RC_MODE_CBR) != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "WifiStream Err not available h264 instance\r\n");
            return MMP_WIFI_ERR_INVALID_PARAMETERS;
        }
        m_ulH264WifiRearID = *ulEncId;
    }

    // Set REF/GEN buffer
    ulCurAddr = ALIGN32(ulCurAddr);

    MMPD_H264ENC_CalculateRefBuf(usEncW, usEncH, &(videohwbuf.refgenbd), &ulCurAddr);

    // Set current encoder buffer (NV12)
    if (m_sH264WifiStreamObj[pWifi->ubWifiSnrSel].WifiEncModes.VidCurBufMode == VIDENC_CURBUF_RT) {
        if (/*m_bVidRecordActive[0]*/0 && m_VidRecdModes.VidCurBufMode[0] == VIDENC_CURBUF_RT) {
            for (i = 0; i < m_VidRecdInputBuf[0].ulBufCnt; i++) {
                m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulY[i] = m_VidRecdInputBuf[0].ulY[i];
                m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulU[i] = m_VidRecdInputBuf[0].ulU[i];
                m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulV[i] = m_VidRecdInputBuf[0].ulV[i];
            }
        }
        else {
            for (i = 0; i < m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulBufCnt; i++) {
                m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulY[i] = ALIGN32(ulFBufAddr);
                ulFBufAddr += (usEncW << 4);
                m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulU[i] = ulFBufAddr;
                m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulV[i] = m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulU[i] + (usEncW << 2); // CHECK
                ulFBufAddr += (usEncW << 3);
            }
            MMPD_H264ENC_SetSourcePPBuf(&m_WifiEncInputBuf[pWifi->ubWifiSnrSel]);
        }
    }
    else {
        for (i = 0; i < m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulBufCnt; i++) {
            ulTmpBufSize = usEncW * usEncH;

            m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulY[i] = ALIGN32(ulCurAddr);
            m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulU[i] = m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulY[i] + ulTmpBufSize;
            m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulV[i] = m_WifiEncInputBuf[pWifi->ubWifiSnrSel].ulU[i];
            
            ulCurAddr += (ulTmpBufSize * 3) / 2;
        }
    }

    // Set Stream pipe
    if (pWifi->ulStreamType == VIDENC_STREAMTYPE_WIFIREAR) {

        if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264)) {
            #if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
            if (MMP_IsSupportDecMjpegToEncH264()) {
                MMPS_H264_WIFI_SetDecMjpegToH264WifiPipeConfig( pWifi, 
                                                                &m_WifiEncInputBuf[pWifi->ubWifiSnrSel], 
                                                                m_ulWifiStreamScalOutW[pWifi->ubWifiSnrSel], 
                                                                m_ulWifiStreamScalOutH[pWifi->ubWifiSnrSel], 
                                                                *ulEncId);
            }
            #endif
        }
        else if (CAM_CHECK_USB(USB_CAM_AIT)) {
            if (MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_NV12_H264) {
                    MMPS_H264_WIFI_SetGraToH264WifiPipeConfig(  pWifi, 
                                                                &m_WifiEncInputBuf[pWifi->ubWifiSnrSel], 
                                                                m_ulWifiStreamScalOutW[pWifi->ubWifiSnrSel], 
                                                                m_ulWifiStreamScalOutH[pWifi->ubWifiSnrSel], 
                                                                *ulEncId);
            }
            else { // CHECK
                MMPS_H264_WIFI_SetDecMjpegToH264WifiPipeConfig( pWifi, 
                                                                &m_WifiEncInputBuf[pWifi->ubWifiSnrSel], 
                                                                m_ulWifiStreamScalOutW[pWifi->ubWifiSnrSel], 
                                                                m_ulWifiStreamScalOutH[pWifi->ubWifiSnrSel], 
                                                                *ulEncId);
            }
        }
        else {
            MMPS_H264_WIFI_SetStreamPipeConfig( pWifi, 
                                                &m_WifiEncInputBuf[pWifi->ubWifiSnrSel], 
                                                m_ulWifiStreamScalOutW[pWifi->ubWifiSnrSel], 
                                                m_ulWifiStreamScalOutH[pWifi->ubWifiSnrSel], 
                                                *ulEncId);
        }                                              
    }
    else if (pWifi->ulStreamType == VIDENC_STREAMTYPE_WIFIFRONT) {
        MMPS_H264_WIFI_SetStreamPipeConfig( pWifi, 
                                            &m_WifiEncInputBuf[pWifi->ubWifiSnrSel], 
                                            m_ulWifiStreamScalOutW[pWifi->ubWifiSnrSel], 
                                            m_ulWifiStreamScalOutH[pWifi->ubWifiSnrSel], 
                                            *ulEncId);
    }

    // Set Slice Length Buffer, align32
    videohwbuf.miscbuf.ulSliceLenBuf    = ulCurAddr;
    ulCurAddr                           = ulCurAddr + SliceLengBufSize;

    // Set MV buffer, #MB/Frame * #MVs/MB * #byte/MV
    videohwbuf.miscbuf.ulMVBuf          = m_ulVidShareMvAddr;

    // Set SPS buffer
    ulCurAddr = ALIGN32(ulCurAddr);
    
    headerbuf.ulSPSStart                = ulCurAddr;
    headerbuf.ulSPSSize                 = SPSSize;	
    ulCurAddr                           += headerbuf.ulSPSSize;
    
    #if (SUPPORT_VUI_INFO)
    // Rebuild-SPS
    headerbuf.ulTmpSPSSize              = SPSSize + 16;
    headerbuf.ulTmpSPSStart             = ulCurAddr;
    ulCurAddr                           += headerbuf.ulTmpSPSSize;
    #endif

    // Set PPS buffer
    headerbuf.ulPPSStart                = ulCurAddr;
    headerbuf.ulPPSSize                 = PPSSize;
    ulCurAddr                           += headerbuf.ulPPSSize;

    // Set video compressed buffer, 32 byte alignment
    ulCurAddr                           = ALIGN32(ulCurAddr);
    videohwbuf.bsbuf.ulStart            = ulCurAddr;
    mergercompbuf.ulVideoCompBufStart   = ulCurAddr;
    ulCurAddr                           += VideoCompBufSize;
    videohwbuf.bsbuf.ulEnd              = ulCurAddr;
    mergercompbuf.ulVideoCompBufEnd     = ulCurAddr;

    if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
        MMPD_3GPMGR_SetRepackMiscBuf(VIDENC_STREAMTYPE_WIFIFRONT, &repackmiscbuf);
        MMPD_3GPMGR_SetEncodeCompBuf(VIDENC_STREAMTYPE_WIFIFRONT, &mergercompbuf);
    }
    else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
             pWifi->ubWifiSnrSel == USBH_SENSOR) {
        MMPD_3GPMGR_SetRepackMiscBuf(VIDENC_STREAMTYPE_WIFIREAR, &repackmiscbuf);
        MMPD_3GPMGR_SetEncodeCompBuf(VIDENC_STREAMTYPE_WIFIREAR, &mergercompbuf);
    }

    MMPD_H264ENC_SetHeaderBuf(*ulEncId, &headerbuf);
    MMPD_H264ENC_SetBitstreamBuf(*ulEncId, &(videohwbuf.bsbuf));
    MMPD_H264ENC_SetRefGenBound(*ulEncId, &(videohwbuf.refgenbd));
    MMPD_H264ENC_SetMiscBuf(*ulEncId, &(videohwbuf.miscbuf));

    MMPS_System_GetPreviewFrameStart(&bufferEndAddr);

    m_ulVidWifiStreamDramAddr = ulCurAddr;

    #if defined(ALL_FW)
    if (m_ulVidWifiStreamDramAddr > MMPS_System_GetMemHeapEnd()) {
        RTNA_DBG_Str(0, "\t= [HeapMemErr] 1st Wifi stream =\r\n");
        return MMP_WIFI_ERR_MEM_EXHAUSTED;
    }
    printc("End of 1st wifi stream buffers = 0x%X\r\n", m_ulVidWifiStreamDramAddr);
    #endif

	return MMP_ERR_NONE;
}
#endif
//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_StopStream
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Stop video recording and fill 3GP tail.

 It works after video start, pause and resume.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
 @retval MMP_3GPRECD_ERR_OPEN_FILE_FAILURE Open file failed.
 @retval MMP_3GPRECD_ERR_CLOSE_FILE_FAILURE Close file failed.
*/
MMP_ERR MMPS_H264_WIFI_StopStream(void** WifiHandle)
{
    MMP_ULONG           		ulWifiID;
    MMP_ULONG					i;
    MMP_ULONG           		ulVidNumOpening = 0;
    MMP_H264_WIFISTREAM_OBJ 	**ppHeadWiFiHandler = (MMP_H264_WIFISTREAM_OBJ**)WifiHandle;
    MMP_H264_WIFISTREAM_OBJ 	*pWifi;
    VIDENC_FW_STATUS  		    status_fw;
    MMP_ULONG                   ulTimeout = VR_QUERY_STATES_TIMEOUT; 

    if (!ppHeadWiFiHandler){
        MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);     
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }
    for (i = 0; i < VR_MAX_H264_STREAM_NUM ; i++){
    	if(*ppHeadWiFiHandler && (*ppHeadWiFiHandler)->bEnableWifi)
    		break;
    	
    	if (i == VR_MAX_H264_STREAM_NUM){
            MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);                 
    		return MMP_WIFI_ERR_INVALID_PARAMETERS;
       }
    	else
    		ppHeadWiFiHandler++;
    }

	pWifi = *ppHeadWiFiHandler;
	 
    if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
        ulWifiID = m_ulH264WifiFrontID;
    }
    else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
             pWifi->ubWifiSnrSel == USBH_SENSOR) {
        ulWifiID = m_ulH264WifiRearID;
    }
    else {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    if (ulWifiID == INVALID_ENC_ID)
        status_fw = VIDENC_FW_STATUS_NONE;
    else
        MMPD_VIDENC_GetStatus(ulWifiID, &status_fw);

    if ((status_fw == VIDENC_FW_STATUS_START)   ||
        (status_fw == VIDENC_FW_STATUS_PAUSE)   ||
        (status_fw == VIDENC_FW_STATUS_RESUME)  ||
        (status_fw == VIDENC_FW_STATUS_STOP)    ||
        (status_fw == VIDENC_FW_STATUS_PREENCODE)) {

        if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
            MMPD_VIDENC_StopStreaming(ulWifiID, VIDENC_STREAMTYPE_WIFIFRONT);
        }
        else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
                 pWifi->ubWifiSnrSel == USBH_SENSOR) {
            MMPD_VIDENC_StopStreaming(ulWifiID, VIDENC_STREAMTYPE_WIFIREAR);
        }

        do {
            MMPD_VIDENC_GetStatus(ulWifiID, &status_fw);
            if (status_fw != VIDENC_FW_STATUS_STOP){
                MMPF_OS_Sleep(1);
            }              
        } while ((status_fw != VIDENC_FW_STATUS_STOP) && ((--ulTimeout) > 0));
        
        if (0 == ulTimeout) {
            MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);                                                                    
        }

        MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);

        // Deinit H264 module if all stream are stoped.
        if (ulVidNumOpening == 0) {
            if (MMPD_VIDENC_IsModuleInit()) {
                MMPD_VIDENC_DeinitModule();
            }
        }

        if (pWifi->ubWifiSnrSel == PRM_SENSOR)
            m_ulH264WifiFrontID = INVALID_ENC_ID;
        else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
                 pWifi->ubWifiSnrSel == USBH_SENSOR)
            m_ulH264WifiRearID = INVALID_ENC_ID;
        else
            return MMP_WIFI_ERR_INVALID_PARAMETERS;
        
        MMPS_H264_WIFI_EnableStreamPipe(pWifi, MMP_FALSE, MMP_FALSE);
        
        if (ulVidNumOpening == 0) {
            MMPD_VIDENC_EnableClock(MMP_FALSE);
        }
    }
    else if (status_fw == VIDENC_FW_STATUS_NONE) {
        
        MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);
        
        if ((ulVidNumOpening > 0) && (ulWifiID != INVALID_ENC_ID)) {

            MMPD_VIDENC_DeInitInstance(ulWifiID);

            if (pWifi->ubWifiSnrSel == PRM_SENSOR)
                m_ulH264WifiFrontID = INVALID_ENC_ID;
            else if (pWifi->ubWifiSnrSel == SCD_SENSOR ||
                     pWifi->ubWifiSnrSel == USBH_SENSOR)
                m_ulH264WifiRearID = INVALID_ENC_ID;
            else
                return MMP_WIFI_ERR_INVALID_PARAMETERS;

            MMPD_VIDENC_GetNumOpen(&ulVidNumOpening);

            if (ulVidNumOpening == 0) {
                if (MMPD_VIDENC_IsModuleInit()) {
                    MMPD_VIDENC_DeinitModule();
                }
            }
        }
    }
    else {
        return MMP_WIFI_ERR_PARAMETER;
    }
    
    MMPS_H264_WIFI_Return2Display(pWifi);
	MMPD_JPEG_Ctl_ResumePreview();
	
    pWifi->bEnableWifi = MMP_FALSE;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_H264_WIFI_Return2Display
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_H264_WIFI_Return2Display(void* WifiHandle)
{
    MMP_H264_WIFISTREAM_OBJ *pWifi  = (MMP_H264_WIFISTREAM_OBJ*)WifiHandle;
    MMP_BOOL                bDSCOn  = MMP_FALSE;

#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE)// Pcam Mix Mode ... UVC function work under Video/DSC mode
    //if (pWifi && !pWifi->bEnableWifi) {
    //    return MMP_WIFI_ERR_INVALID_PARAMETERS;
    //}
#else
    if (pWifi && !pWifi->bEnableWifi) {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }
#endif
        
    MMPS_DSC_GetPreviewStatus(&bDSCOn);
    
    /* Restore the setting of Scaler */
    if (bDSCOn) {
        return MMP_ERR_NONE;
    }

    /* Disable scaler because it might still previewing or recording */
    MMPD_Scaler_SetEnable(pWifi->FctlLink.scalerpath, MMP_FALSE);
    
    /* Change the path to JPEG in order to keep the setting of scaler normal */
    MMPD_Scaler_SetPath(pWifi->FctlLink.scalerpath, MMP_SCAL_SOURCE_JPG, MMP_TRUE);

	if (pWifi->ulStreamType == VIDENC_STREAMTYPE_WIFIFRONT) {
		// Fix WIFI preview -> LCD preview have white line issue.   
	    MMPD_IBC_SetStoreEnable(pWifi->FctlLink.ibcpipeID, MMP_FALSE);
	    // IBC can't sync with ISR, wait 40 msec for IBC store done.
		MMPF_OS_Sleep (40);
		
		MMPD_Fctl_ResetIBCBufIdx(pWifi->FctlLink.ibcpipeID);
        MMPD_Fctl_SetPipeAttrForIbcFB(&m_VRPreviewFctlAttr[0]);
	}
	else if (pWifi->ulStreamType == VIDENC_STREAMTYPE_WIFIREAR) {
        
        if (CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264) ||\
            CAM_CHECK_USB(USB_CAM_SONIX_MJPEG)) {
            // For restore gsPreviewBufWidth and gsPreviewBufHeight. (TBD)	
            MMPD_Fctl_SetPipeAttrForIbcFB(&m_DecMjpegToPrevwFctlAttr);
            #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
            if (MMP_IsSupportDecMjpegToPreview()) {
                MMPS_3GPRECD_SetDispColorFmtToJpgAttr(pWifi->LinkAttr.IBCColorFormat);
            }
            #endif
        }
        else if (CAM_CHECK_USB(USB_CAM_AIT)) {
            #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
            if (MMP_IsSupportDecMjpegToPreview()) {
                // For restore gsPreviewBufWidth and gsPreviewBufHeight. (TBD)	
                MMPD_Fctl_SetPipeAttrForIbcFB(&m_DecMjpegToPrevwFctlAttr);
            }
            else {
                RTNA_DBG_Str(0, "No need to restore pipe setting!\r\n");
                return MMP_ERR_NONE;
            }
            #else
            RTNA_DBG_Str(0, "No need to restore pipe setting!\r\n");
            return MMP_ERR_NONE;
            #endif
        }
        else if ((CAM_CHECK_SCD(SCD_CAM_TV_DECODER)) || 
                 (CAM_CHECK_SCD(SCD_CAM_YUV_SENSOR))) {
            // Fix WIFI preview -> LCD preview have white line issue.   
            MMPD_IBC_SetStoreEnable(pWifi->FctlLink.ibcpipeID, MMP_FALSE);
            // IBC can't sync with ISR, wait 40 msec for IBC store done.
            MMPF_OS_Sleep (40);
            
            MMPD_Fctl_ResetIBCBufIdx(pWifi->FctlLink.ibcpipeID);
            MMPD_Fctl_SetPipeAttrForIbcFB(&m_VRPreviewFctlAttr[1]);
            MMPD_Scaler_SetLineDelay(pWifi->FctlLink.scalerpath, 0);
        }
        else if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
            // TBD
        }
    }

    /* Restore the setting of IBC */
    MMPD_Fctl_RestoreIBCLinkAttr(pWifi->FctlLink.ibcpipeID, 
                                 pWifi->LinkAttr.IBCLinkType, 
                                 pWifi->LinkAttr.previewDev, 
                                 pWifi->LinkAttr.winID,
                                 pWifi->LinkAttr.rotateDir);

    MMPD_IBC_ClearFrameEnd(pWifi->FctlLink.ibcpipeID);
    MMPD_Scaler_SetPath(pWifi->FctlLink.ibcpipeID, pWifi->LinkAttr.scalerSrc, MMP_TRUE);
    MMPD_Scaler_SetEnable(pWifi->FctlLink.ibcpipeID, MMP_TRUE);
    MMPD_IBC_CheckFrameEnd(pWifi->FctlLink.ibcpipeID);
    
	if (pWifi->ulStreamType == VIDENC_STREAMTYPE_WIFIFRONT) {
		MMPD_Display_ResetRotateDMABufIdx(MMP_DISPLAY_SRC_FRONTCAM);
		MMPD_Fctl_EnablePreview(PRM_SENSOR, pWifi->FctlLink.ibcpipeID, MMP_TRUE, MMP_FALSE);
	}
	else {
		MMPD_Display_ResetRotateDMABufIdx(MMP_DISPLAY_SRC_REARCAM);
		
		if (MMP_IsScdCamExist())
			MMPD_Fctl_EnablePreview(SCD_SENSOR, pWifi->FctlLink.ibcpipeID, MMP_TRUE, MMP_FALSE);
		else
			MMPD_Fctl_EnablePreview(USBH_SENSOR, pWifi->FctlLink.ibcpipeID, MMP_TRUE, MMP_FALSE);	
	}
	
    return MMP_ERR_NONE;
}
#endif

#if 0
void ____VR_UVC_Stream_Function____(){ruturn;} //dummy
#endif

#if(SUPPORT_UVC_FUNC)
//------------------------------------------------------------------------------
//  Function    : MMPS_H264_UVC_StartStream
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_H264_UVC_StartStream(void* WifiHandle)
{
    MMP_ULONG           	enc_id = 0;
    VIDENC_FW_STATUS  	    status_vid;
    MMP_ERR             	status;
    MMP_ULONG           	EncWidth, EncHeight;
    MMP_ULONG64         	ullBitrate64, ullTimeIncr64, ullTimeResol64;
    MMP_ULONG           	ulTargetFrameSize;
    MMP_ULONG           	ulFps;
    MMP_ULONG           	ulWifiID;
    MMP_ULONG           	ulTimeout = VR_QUERY_STATES_TIMEOUT;
    MMP_H264_WIFISTREAM_OBJ *pWifi = (MMP_H264_WIFISTREAM_OBJ*)WifiHandle;

    if (pWifi && !pWifi->bEnableWifi) {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    if (m_bH264WifiStreamActive[pWifi->ubWifiSnrSel]) {
        MMPS_H264_WIFI_StopStream(&H264UVCHdl[0]);
        RTNA_DBG_Str(0, "Stop stream before start it\r\n");
    }

    /* Assign StreamType and ID */
    if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
        ulWifiID = m_ulH264WifiFrontID;
    }
    else if (pWifi->ubWifiSnrSel == SCD_SENSOR) {
        ulWifiID = m_ulH264WifiRearID;
    }
    else {
        return MMP_WIFI_ERR_INVALID_PARAMETERS;
    }

    status = MMPS_H264_WIFI_SetStreamRes(WifiHandle);

    if (status != MMP_ERR_NONE) {
        return status;
    }

    EncWidth  = m_ulWifiStreamW[pWifi->ubWifiSnrSel];
    EncHeight = m_ulWifiStreamH[pWifi->ubWifiSnrSel];

    ullBitrate64      = pWifi->WifiEncModes.ulBitrate;
    ullTimeIncr64     = pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrement;
    ullTimeResol64    = pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrResol;
    ulTargetFrameSize = (MMP_ULONG)((ullBitrate64 * ullTimeIncr64)/(ullTimeResol64 * 8));

    ulFps = (pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrResol - 1 +
             pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrement) /
             pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrement;

    status = MMPD_VIDENC_CheckCapability(EncWidth, EncHeight, ulFps);

    if (status != MMP_ERR_NONE) {
        return status;
    }

    if (ulWifiID == INVALID_ENC_ID)
        status_vid = VIDENC_FW_STATUS_NONE;
    else
        MMPD_VIDENC_GetStatus(ulWifiID, &status_vid);

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP)) {

        if (!MMPD_VIDENC_IsModuleInit()) {
            MMP_ERR	mmpstatus;

            mmpstatus = MMPD_VIDENC_InitModule();
            if (mmpstatus != MMP_ERR_NONE) {
                return mmpstatus;
            }
        }

        if (MMPS_H264_WIFI_SetStreamMemoryMap(  &enc_id,
        										pWifi,
        		                           		EncWidth,
												EncHeight,
												m_ulVidRecSramAddr,
												m_ulVidWifiStreamStartAddr))
        {
            RTNA_DBG_Str(0, "Alloc mem for wifi stream failed\r\n");
            return MMP_WIFI_ERR_MEM_EXHAUSTED;
        }

        // Set current buffer mode before MMPF_Display_StartPreview() use it
        switch (pWifi->WifiEncModes.VidCurBufMode) {
        case VIDENC_CURBUF_FRAME:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_FRAME);
            break;
        case VIDENC_CURBUF_RT:
            MMPD_VIDENC_SetCurBufMode(enc_id, VIDENC_CURBUF_RT);
            break;
        default:
            return MMP_WIFI_ERR_INVALID_PARAMETERS;
        }

        if (MMPS_H264_WIFI_EnableStreamPipe(pWifi, MMP_TRUE, MMP_FALSE) != MMP_ERR_NONE) {
            PRINTF("Enable Video Record: Fail\r\n");
            return MMP_WIFI_ERR_PARAMETER;
        }

        // Set encoder and merger parameters
        if (((EncWidth == 1920) && (EncHeight == 1088)) ||
            ((EncWidth == 1440) && (EncHeight == 1088)) ||
            ((EncWidth == 640) && (EncHeight == 368))) {

            MMPD_VIDENC_SetCropping(enc_id, 0, 8, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_REPEAT, 8);
        }
        else {
            MMPD_VIDENC_SetCropping(enc_id, 0, 0, 0, 0);
            MMPD_H264ENC_SetPadding(enc_id, H264ENC_PADDING_NONE, 0);
        }

        if (MMPD_VIDENC_SetResolution(enc_id, EncWidth, EncHeight) != MMP_ERR_NONE) {
            return MMP_WIFI_ERR_UNSUPPORTED_PARAMETERS;
        }

        if (MMPD_VIDENC_SetProfile(enc_id, pWifi->WifiEncModes.VisualProfile) != MMP_ERR_NONE) {
            return MMP_WIFI_ERR_UNSUPPORTED_PARAMETERS;
        }

        MMPD_VIDENC_SetEncodeMode();

        if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
            MMPD_3GPMGR_SetGOP(pWifi->ulStreamType, pWifi->WifiEncModes.usPFrameCount, pWifi->WifiEncModes.usBFrameCount);
            MMPD_VIDENC_SetGOP(enc_id, pWifi->WifiEncModes.usPFrameCount, pWifi->WifiEncModes.usBFrameCount);
        }
        else if (pWifi->ubWifiSnrSel == SCD_SENSOR) {
            MMPD_3GPMGR_SetGOP(VIDENC_STREAMTYPE_WIFIREAR, pWifi->WifiEncModes.usPFrameCount, pWifi->WifiEncModes.usBFrameCount);
            MMPD_VIDENC_SetGOP(enc_id, pWifi->WifiEncModes.usPFrameCount, pWifi->WifiEncModes.usBFrameCount);
        }

        MMPD_VIDENC_SetQuality(enc_id, ulTargetFrameSize, (MMP_ULONG)ullBitrate64);

        if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
            MMPD_VIDENC_SetSnrFrameRate(enc_id,
        								pWifi->ulStreamType,
										pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrement,
										pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrResol);
        }
        else if (pWifi->ubWifiSnrSel == SCD_SENSOR) {
       		MMPD_VIDENC_SetSnrFrameRate(enc_id,
        								VIDENC_STREAMTYPE_WIFIREAR,
										pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrement,
										pWifi->WifiEncModes.SnrInputFrameRate.usVopTimeIncrResol);
		}

        MMPD_VIDENC_SetEncFrameRate(enc_id,
        							pWifi->WifiEncModes.VideoEncFrameRate.usVopTimeIncrement,
									pWifi->WifiEncModes.VideoEncFrameRate.usVopTimeIncrResol);

        if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
        	MMPD_3GPMGR_SetFrameRate(pWifi->ulStreamType,
        							 pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrResol,
									 pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrement);
        }
        else if (pWifi->ubWifiSnrSel == SCD_SENSOR) {
        	MMPD_3GPMGR_SetFrameRate(VIDENC_STREAMTYPE_WIFIREAR,
        							 pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrResol,
									 pWifi->WifiEncModes.ContainerFrameRate.usVopTimeIncrement);
        }
        
        MMPD_VIDENC_SetForceI(enc_id, 3);
    }

    if ((status_vid == VIDENC_FW_STATUS_NONE) ||
        (status_vid == VIDENC_FW_STATUS_STOP) ||
        (status_vid == VIDENC_FW_STATUS_PREENCODE)) {

        if (status_vid != VIDENC_FW_STATUS_PREENCODE) {
            MMPD_VIDENC_EnableClock(MMP_TRUE);
        }

        if (pWifi->ubWifiSnrSel == PRM_SENSOR) {
        	if (/*MMPD_3GPMGR_StartCapture*/MMPD_VIDENC_StartStreaming(m_ulH264WifiFrontID, pWifi->ulStreamType) != MMP_ERR_NONE) {
            	MMPD_VIDENC_EnableClock(MMP_FALSE);
            	return MMP_3GPRECD_ERR_GENERAL_ERROR;
            }
        }
        else if (pWifi->ubWifiSnrSel == SCD_SENSOR) {
         	if (/*MMPD_3GPMGR_StartCapture*/MMPD_VIDENC_StartStreaming(m_ulH264WifiRearID, VIDENC_STREAMTYPE_WIFIREAR) != MMP_ERR_NONE) {
            	MMPD_VIDENC_EnableClock(MMP_FALSE);
            	return MMP_3GPRECD_ERR_GENERAL_ERROR;
            }
        }

        do {
            MMPD_VIDENC_GetStatus(enc_id, &status_vid);
            if (status_vid != VIDENC_FW_STATUS_START) {
                MMPF_OS_Sleep(1);
            }
        } while ((status_vid != VIDENC_FW_STATUS_START) && (--ulTimeout) > 0);

        if (ulTimeout == 0) {
            RTNA_DBG_Str(0, "UVC stream H264 NG...\r\n");
            return MMP_WIFI_ERR_PARAMETER;
        }

        pWifi->usEncID = enc_id;

        if (status_vid == VIDENC_FW_STATUS_START) {
            return MMP_ERR_NONE;
        }
        else {
            return MMP_ERR_NONE;
        }
    }
    else {
        return MMP_WIFI_ERR_PARAMETER;
    }
}
#endif

#if 0
void ____VR_Zoom_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetZoomConfig
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set zoom config

The function set zoom config, total step and max zoom multiplier

@param[in] usMaxSteps 	max zoom steps.
@param[in] usMaxRatio 	max zoom ratio.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_3GPRECD_SetZoomConfig(MMP_USHORT usMaxSteps, MMP_USHORT usMaxRatio)
{
    m_VidPreviewZoomInfo.usMaxZoomSteps	= usMaxSteps;
    m_VidPreviewZoomInfo.usMaxZoomRatio	= usMaxRatio;
    
    m_VidRecordZoomInfo.usMaxZoomSteps	= usMaxSteps;
    m_VidRecordZoomInfo.usMaxZoomRatio	= usMaxRatio;
    
    m_bAhcConfigVideoRZoom = MMP_TRUE;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_InitDigitalZoomParam
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR MMPS_3GPRECD_InitDigitalZoomParam(MMP_UBYTE ubPipe)
{
	if (ubPipe == m_PreviewFctlLink.scalerpath) 
	{
		// Initial preview zoom parameters
	    m_VidPreviewZoomInfo.scalerpath 	= m_PreviewFctlLink.scalerpath;
	    m_VidPreviewZoomInfo.usCurZoomStep 	= 0;
	    m_VidPreviewZoomInfo.usMaxZoomRatio	= gsVidPtzCfg.usMaxZoomRatio;
		m_VidPreviewZoomInfo.usMaxZoomSteps	= gsVidPtzCfg.usMaxZoomSteps;
		m_VidPreviewZoomInfo.sMaxPanSteps	= gsVidPtzCfg.sMaxPanSteps;
		m_VidPreviewZoomInfo.sMinPanSteps	= gsVidPtzCfg.sMinPanSteps;
		m_VidPreviewZoomInfo.sMaxTiltSteps	= gsVidPtzCfg.sMaxTiltSteps;
		m_VidPreviewZoomInfo.sMinTiltSteps	= gsVidPtzCfg.sMinTiltSteps;

	    MMPD_PTZ_InitPtzRange(m_VidPreviewZoomInfo.scalerpath,
				              m_VidPreviewZoomInfo.usMaxZoomRatio,
				              m_VidPreviewZoomInfo.usMaxZoomSteps,
				              m_VidPreviewZoomInfo.sMaxPanSteps,
				              m_VidPreviewZoomInfo.sMinPanSteps,
				              m_VidPreviewZoomInfo.sMaxTiltSteps,
				              m_VidPreviewZoomInfo.sMinTiltSteps);

	    MMPD_PTZ_SetDigitalZoom(m_VidPreviewZoomInfo.scalerpath, MMP_PTZ_ZOOMSTOP, MMP_TRUE);
	}
	else if (ubPipe == m_RecordFctlLink.scalerpath)
	{
	    // Initail video zoom parameters
        m_VidRecordZoomInfo.scalerpath      = m_RecordFctlLink.scalerpath;
        m_VidRecordZoomInfo.usCurZoomStep   = 0;
	    m_VidRecordZoomInfo.usMaxZoomRatio	= gsVidPtzCfg.usMaxZoomRatio;
		m_VidRecordZoomInfo.usMaxZoomSteps	= gsVidPtzCfg.usMaxZoomSteps;
		m_VidRecordZoomInfo.sMaxPanSteps	= gsVidPtzCfg.sMaxPanSteps;
		m_VidRecordZoomInfo.sMinPanSteps	= gsVidPtzCfg.sMinPanSteps;
		m_VidRecordZoomInfo.sMaxTiltSteps	= gsVidPtzCfg.sMaxTiltSteps;
		m_VidRecordZoomInfo.sMinTiltSteps	= gsVidPtzCfg.sMinTiltSteps;

		MMPD_PTZ_InitPtzRange(m_VidRecordZoomInfo.scalerpath,
				              m_VidRecordZoomInfo.usMaxZoomRatio,
				              m_VidRecordZoomInfo.usMaxZoomSteps,
				              m_VidRecordZoomInfo.sMaxPanSteps,
				              m_VidRecordZoomInfo.sMinPanSteps,
				              m_VidRecordZoomInfo.sMaxTiltSteps,
				              m_VidRecordZoomInfo.sMinTiltSteps);

	    MMPD_PTZ_SetDigitalZoom(m_VidRecordZoomInfo.scalerpath, MMP_PTZ_ZOOMSTOP, MMP_TRUE);
	}
	
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_RestoreDigitalZoomRange
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR MMPS_3GPRECD_RestoreDigitalZoomRange(MMP_UBYTE ubPipe)
{
    if (m_usVidStaticZoomIndex) 
    {
		if (ubPipe == m_PreviewFctlLink.scalerpath)
		{
	        m_VidPreviewZoomInfo.usCurZoomStep = m_usVidStaticZoomIndex;

			MMPD_PTZ_SetTargetPtzStep(m_VidPreviewZoomInfo.scalerpath, 
	    							  MMP_PTZ_ZOOM_INC_IN, 
	    							  m_VidPreviewZoomInfo.usCurZoomStep, 0, 0);
			
	        MMPD_PTZ_SetDigitalZoomOP(m_ub1stVRStreamSnrId,
	                                  m_VidPreviewZoomInfo.scalerpath,
	                                  MMP_PTZ_ZOOMIN,
	                                  MMP_TRUE);
		}
		else if (ubPipe == m_RecordFctlLink.scalerpath)
		{
	        m_VidRecordZoomInfo.usCurZoomStep = m_usVidStaticZoomIndex;

			MMPD_PTZ_SetTargetPtzStep(m_VidRecordZoomInfo.scalerpath, 
	    							  MMP_PTZ_ZOOM_INC_IN, 
	    							  m_VidRecordZoomInfo.usCurZoomStep, 0, 0);

	        MMPD_PTZ_SetDigitalZoomOP(m_ub1stVRStreamSnrId,
	                                  m_VidRecordZoomInfo.scalerpath,
	                                  MMP_PTZ_ZOOMIN,
	                                  MMP_TRUE);
		} 

        // Reset zoom index
 		m_usVidStaticZoomIndex = 0;
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetCurZoomStep
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetCurZoomStep(MMP_UBYTE ubPipe, MMP_USHORT *usCurZoomStep)
{
    MMP_USHORT	usCurStep;
    MMP_ERR     status = MMP_ERR_NONE;
    
    if (MMP_IsVidPtzEnable() == MMP_FALSE) {
        usCurZoomStep = 0;
        return status;
    }
    
    if (ubPipe == m_PreviewFctlLink.scalerpath) 
    {
        status = MMPD_PTZ_GetCurPtzStep(m_VidPreviewZoomInfo.scalerpath,
                                        NULL, &usCurStep, NULL, NULL);

        *usCurZoomStep = usCurStep;

        if (m_bVidPreviewActive[0]) {
            m_VidPreviewZoomInfo.usCurZoomStep = usCurStep;
        }
    }
    else if (ubPipe == m_RecordFctlLink.scalerpath) 
    {
        status = MMPD_PTZ_GetCurPtzStep(m_VidRecordZoomInfo.scalerpath,
                                        NULL, &usCurStep, NULL, NULL);
    
        *usCurZoomStep = usCurStep;

        if (m_bVidRecordActive[0]) {
            m_VidRecordZoomInfo.usCurZoomStep = usCurStep;
        }
    }

    return status;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetCurZoomStatus
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE MMPS_3GPRECD_GetCurZoomStatus(void)
{
	return MMPD_PTZ_GetCurPtzStatus();
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetPreviewZoom
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetPreviewZoom(MMPS_3GPRECD_ZOOM_PATH sPath, MMP_PTZ_ZOOM_DIR sZoomDir, MMP_USHORT usCurZoomStep)
{
    MMP_BOOL 			bComplete[2] = {MMP_FALSE, MMP_FALSE};
	MMP_PTZ_ZOOM_INC  	sZoomInc;
	MMP_UBYTE			ubPipe;
	MMP_USHORT			usMaxStep = 0;
	
    if (m_bVidPreviewActive[0] != MMP_TRUE) {
        m_usVidStaticZoomIndex = usCurZoomStep;
        return MMP_ERR_NONE;
    }

	/* Decide the zoom step increment */
	if (sZoomDir == MMP_PTZ_ZOOMIN) {
        sZoomInc = MMP_PTZ_ZOOM_INC_IN;
    }
    else if (sZoomDir == MMP_PTZ_ZOOMOUT) {
        sZoomInc = MMP_PTZ_ZOOM_INC_OUT;
    }
	
	if ((sPath == MMPS_3GPRECD_ZOOM_PATH_PREV && m_bVidPreviewActive[0]) || 
		(sPath == MMPS_3GPRECD_ZOOM_PATH_RECD && m_bVidRecordActive[0]))
	{
		if (sPath == MMPS_3GPRECD_ZOOM_PATH_PREV) {
			ubPipe 		= m_VidPreviewZoomInfo.scalerpath;
			usMaxStep 	= m_VidPreviewZoomInfo.usMaxZoomSteps;
		}
		else {
			ubPipe 		= m_VidRecordZoomInfo.scalerpath;
			usMaxStep 	= m_VidRecordZoomInfo.usMaxZoomSteps;
		}

	    MMPD_PTZ_CheckZoomComplete(ubPipe, bComplete);
	    
	    if (!bComplete[0]) {

	        MMPD_PTZ_SetDigitalZoom(ubPipe, MMP_PTZ_ZOOMSTOP, MMP_TRUE);

	        do {
	            MMPD_PTZ_CheckZoomComplete(ubPipe, bComplete);
	        } while (!bComplete[0]);
	    }

		if (sZoomDir == MMP_PTZ_ZOOMIN) {
			MMPD_PTZ_SetTargetPtzStep(ubPipe, 
		    						  sZoomInc, 
		    						  usMaxStep, 0, 0);
		}
		else if (sZoomDir == MMP_PTZ_ZOOMOUT) {
			MMPD_PTZ_SetTargetPtzStep(ubPipe, 
		    						  sZoomInc, 
		    						  0, 0, 0);
		}

	    MMPD_PTZ_SetDigitalZoom(ubPipe, sZoomDir, MMP_TRUE);
	}
	else if (sPath == MMPS_3GPRECD_ZOOM_PATH_BOTH && m_bVidPreviewActive[0] && m_bVidRecordActive[0])
	{
	    MMPD_PTZ_CheckZoomComplete(m_VidPreviewZoomInfo.scalerpath, bComplete);
	    MMPD_PTZ_CheckZoomComplete(m_VidRecordZoomInfo.scalerpath, bComplete + 1);

	    if (!bComplete[0]) {

	        MMPD_PTZ_SetDigitalZoom(m_VidPreviewZoomInfo.scalerpath, MMP_PTZ_ZOOMSTOP, MMP_TRUE);
			MMPD_PTZ_SetDigitalZoom(m_VidRecordZoomInfo.scalerpath, MMP_PTZ_ZOOMSTOP, MMP_TRUE);

	        do {
	            MMPD_PTZ_CheckZoomComplete(m_VidPreviewZoomInfo.scalerpath, bComplete);
	            MMPD_PTZ_CheckZoomComplete(m_VidRecordZoomInfo.scalerpath, bComplete + 1);
	        } while (!bComplete[0] && !bComplete[1]);
	    }

		if (sZoomDir == MMP_PTZ_ZOOMIN) {
			MMPD_PTZ_SetTargetPtzStep(m_VidPreviewZoomInfo.scalerpath, 
		    						  sZoomInc, 
		    						  m_VidPreviewZoomInfo.usMaxZoomSteps, 0, 0);
		}
		else if (sZoomDir == MMP_PTZ_ZOOMOUT) {
			MMPD_PTZ_SetTargetPtzStep(m_VidPreviewZoomInfo.scalerpath, 
		    						  sZoomInc, 
		    						  0, 0, 0);
		}

	    MMPD_PTZ_SetDigitalZoom(m_VidPreviewZoomInfo.scalerpath, sZoomDir, MMP_TRUE);

		if (sZoomDir == MMP_PTZ_ZOOMIN) {
			MMPD_PTZ_SetTargetPtzStep(m_VidRecordZoomInfo.scalerpath, 
		    						  sZoomInc, 
		    						  m_VidRecordZoomInfo.usMaxZoomSteps, 0, 0);
		}
		else if (sZoomDir == MMP_PTZ_ZOOMOUT) {
			MMPD_PTZ_SetTargetPtzStep(m_VidRecordZoomInfo.scalerpath, 
		    						  sZoomInc, 
		    						  0, 0, 0);
		}

	    MMPD_PTZ_SetDigitalZoom(m_VidRecordZoomInfo.scalerpath, sZoomDir, MMP_TRUE);
	}

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StopAllPipeZoom
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_StopAllPipeZoom(void)
{
	MMP_BOOL bComplete[2] = {MMP_FALSE, MMP_FALSE};

	return MMP_ERR_NONE; //TBD

	if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_2PIPE) 
	{
        MMPD_PTZ_CheckZoomComplete(m_VidPreviewZoomInfo.scalerpath, bComplete);
        MMPD_PTZ_CheckZoomComplete(m_VidRecordZoomInfo.scalerpath, bComplete + 1);
        
        if ((!bComplete[0]) || (!bComplete[1])) {
            MMPD_PTZ_SetDigitalZoom(m_VidPreviewZoomInfo.scalerpath, MMP_PTZ_ZOOMSTOP, MMP_TRUE);
            MMPD_PTZ_SetDigitalZoom(m_VidRecordZoomInfo.scalerpath, MMP_PTZ_ZOOMSTOP, MMP_TRUE);

            do {
                MMPD_PTZ_CheckZoomComplete(m_VidPreviewZoomInfo.scalerpath, bComplete);
                MMPD_PTZ_CheckZoomComplete(m_VidRecordZoomInfo.scalerpath, bComplete + 1);
            } while((!bComplete[0]) || (!bComplete[1]));
        }
	}
    else if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_SINGLE || 
    		 m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_LB_MULTI	||
    		 m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE) 
    {
        MMPD_PTZ_CheckZoomComplete(m_VidPreviewZoomInfo.scalerpath, bComplete);
        MMPD_PTZ_CheckZoomComplete(m_VidRecordZoomInfo.scalerpath, bComplete + 1);
        
        if ((!bComplete[0]) || (!bComplete[1])) {
            MMPD_PTZ_SetDigitalZoom(m_VidPreviewZoomInfo.scalerpath, MMP_PTZ_ZOOMSTOP, MMP_TRUE);
            MMPD_PTZ_SetDigitalZoom(m_VidRecordZoomInfo.scalerpath, MMP_PTZ_ZOOMSTOP, MMP_TRUE);

            do {
                MMPD_PTZ_CheckZoomComplete(m_VidPreviewZoomInfo.scalerpath, bComplete);
                MMPD_PTZ_CheckZoomComplete(m_VidRecordZoomInfo.scalerpath, bComplete + 1);
            } while((!bComplete[0]) || (!bComplete[1]));
        }
	}

	return MMP_ERR_NONE;
}

#if 0
void ____VR_LDC_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetLdcRunMode
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetLdcRunMode(MMP_UBYTE ubRunMode)
{
	if (!MMP_IsVidLdcSupport()) {
		return MMP_ERR_NONE;
	}

	MMPF_LDC_SetRunMode(ubRunMode);
	
	if (ubRunMode == MMP_LDC_RUN_MODE_MULTI_RUN) {
		MMPF_LDC_MultiRunSetLoopBackCount(MMPF_LDC_MultiRunGetMaxLoopBackCount());
	}
	
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetLdcResMode
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetLdcResMode(MMP_UBYTE ubResMode, MMP_UBYTE ubFpsMode)
{
	if (!MMP_IsVidLdcSupport()) {
		return MMP_ERR_NONE;
	}

	m_ubLdcResMode = ubResMode;
	m_ubLdcFpsMode = ubFpsMode;
	
	if (m_VidRecdConfigs.previewpath[0] == MMP_3GP_PATH_LDC_MULTISLICE)
	{
		switch(ubResMode)
		{
			case MMP_LDC_RES_MODE_MS_736P:
			    m_ulLdcMaxSrcWidth 	= 736;
			    m_ulLdcMaxSrcHeight	= 736;
				m_ulLdcMaxOutWidth 	= 736;
				m_ulLdcMaxOutHeight	= 736;
			break;
			case MMP_LDC_RES_MODE_MS_1080P: // For AR0330
			    m_ulLdcMaxSrcWidth 	= 2304;
			    m_ulLdcMaxSrcHeight	= 1296;
				m_ulLdcMaxOutWidth 	= 1920;
				m_ulLdcMaxOutHeight	= 1080;
			break;
			case MMP_LDC_RES_MODE_MS_1536P:
			    m_ulLdcMaxSrcWidth 	= 1536;
			    m_ulLdcMaxSrcHeight	= 1536;
				m_ulLdcMaxOutWidth 	= 1536;
				m_ulLdcMaxOutHeight	= 1536;
			break;
			default:
				return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
			break;
		}	
	}
	else
	{
		switch(ubResMode)
		{
			case MMP_LDC_RES_MODE_FHD:
				m_ulLdcMaxSrcWidth 	= 1920;
				m_ulLdcMaxSrcHeight	= 1080;
				m_ulLdcMaxOutWidth 	= 1920;
				m_ulLdcMaxOutHeight	= 1080;
			break;
			case MMP_LDC_RES_MODE_HD:
				m_ulLdcMaxSrcWidth 	= 1280;
				m_ulLdcMaxSrcHeight	= 720;
				m_ulLdcMaxOutWidth 	= 1280;
				m_ulLdcMaxOutHeight	= 720;
			break;
			case MMP_LDC_RES_MODE_WVGA:
				m_ulLdcMaxSrcWidth 	= 848;
				m_ulLdcMaxSrcHeight	= 480;
				m_ulLdcMaxOutWidth 	= 848;
				m_ulLdcMaxOutHeight	= 480;
			break;
			default:
				return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
			break;
		}
	}
	
	MMPF_LDC_SetResMode(ubResMode);
	MMPF_LDC_SetFpsMode(ubFpsMode);
	
	MMPF_LDC_SetFrameRes(m_ulLdcMaxSrcWidth, m_ulLdcMaxSrcHeight,
						 m_ulLdcMaxOutWidth, m_ulLdcMaxOutHeight);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetLdcMaxOutRes
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_GetLdcMaxOutRes(MMP_ULONG *pulMaxW, MMP_ULONG *pulMaxH)
{
	if (!MMP_IsVidLdcSupport()) {
		*pulMaxW = 0;
		*pulMaxH = 0;
		return MMP_ERR_NONE;
	}

    *pulMaxW = m_ulLdcMaxOutWidth;
	*pulMaxH = m_ulLdcMaxOutHeight;

    return MMP_ERR_NONE;
}

#if 0
void ____VR_Sticker_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetSticker
//  Description :
//------------------------------------------------------------------------------
/** @brief The function sets the attributes to the video sticker

The function sets the attributes to the specified video sticker with 
its sticker ID.

  @param[in] stickerID the sticker ID
  @param[in] stickerAtrribute the sticker attributes
  @return It reports the status of the operation.
*/
MMP_ERR MMPS_3GPRECD_SetSticker(MMP_STICKER_ATTR *pStickerAtrr)
{
	MMP_ERR err = MMP_ERR_NONE;

    if (pStickerAtrr != NULL) {
        err = MMPD_Icon_SetAttributes(pStickerAtrr->ubStickerId, pStickerAtrr);
    }
    else {
        return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
    }

    return err;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_EnableSticker
//  Description :
//------------------------------------------------------------------------------
/** @brief The function enable or disable video recording with sticker

The function enables or disables the specified video sticker with its sticker ID.

@param[in] sStickerID specified the sticker id for video recording
@param[in] bEnable enable or disable the sticker of the specified ID.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_3GPRECD_EnableSticker(MMP_STICKER_ID stickerID, MMP_BOOL bEnable)
{
    return MMPD_Icon_SetEnable(stickerID, bEnable);
}

#if 0
void ____VR_TimeLapse_Function____(){ruturn;} //dummy
#endif

#if (SUPPORT_TIMELAPSE)
MMPS_3GPRECD_AVI_LIST AVIRiff = 
{
    MKTAG('R','I','F','F'),
    327904,
    MKTAG('A','V','I',' ')
};

MMPS_3GPRECD_AVI_LIST AVIHdrl = 
{
    MKTAG('L','I','S','T'),
    192,
    MKTAG('h','d','r','l')
};

MMPS_3GPRECD_AVI_LIST AVIStrl = 
{
    MKTAG('L','I','S','T'),
    116,
    MKTAG('s','t','r','l')
};

MMPS_3GPRECD_AVI_LIST AVIMovi = 
{
    MKTAG('L','I','S','T'),
    4,
    MKTAG('m','o','v','i')
};

MMPS_3GPRECD_AVI_MainHeader AVIMainHdr = 
{
    MKTAG('a','v','i','h'),
    0x38,
    33333,
    20000,
    0,
    0x00000010,
    0,
    0,
    1,  // Stream number
    0,
    1280,  // width
    720,   // height
    0,
    0,
    0,
    0
};

MMPS_3GPRECD_AVI_StreamHeader AVIStrHdr = 
{
    MKTAG('s','t','r','h'),
    0x38,
    MKTAG('v','i','d','s'),
    MKTAG('M','J','P','G'),
    0,
    0,
    0,
    1000,
    30000,
    0,
    0,
    0,
    0xFFFFFFFF,
    0,
    0,
    0
};

MMPS_3GPRECD_AVI_StreamFormat AVIStrFormat = 
{
    MKTAG('s','t','r','f'),
    0x28,
    0x28,
    1280,
    720,
    0x00180001,
    MKTAG('M','J','P','G'),
    1843200,
    0,
    0,
    0,
    0
};

#define AVIINDEX_BUFFER_SIZE (0x50000)

MMPS_3GPRECD_AVI_Header AVIIndex = 
{
    MKTAG('i','d','x','1'),
    AVIINDEX_BUFFER_SIZE
};

MMP_UBYTE m_IndexBuff[512];

MMP_ULONG m_ulHeaderSizePos = 0x04;
MMP_ULONG m_ulMoviSizePos = AVIINDEX_BUFFER_SIZE + 0xE0;
MMP_ULONG m_ulMainHdrFNumPos = 0x30;
MMP_ULONG m_ulStrHdrLengthPos = 0x8C;
MMP_ULONG m_ulIndexStartPos = 0xDC;

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_InitAVIFile
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_InitAVIFile(MMP_BYTE *bAviName, MMP_ULONG ulNameSize, MMP_ULONG ulWidth, MMP_ULONG ulHeight,
                            	 MMP_ULONG CoedcFourCC, MMP_ULONG FrameRate, MMP_ULONG ulBitRate, MMP_BOOL bInit, MMP_ULONG *FileID)
{
    MMP_ERR error;
    MMP_ULONG ulWriteCnt = 0, i, BufSize = sizeof(m_IndexBuff);

    if (!bInit) {
        return MMPS_FS_FileOpen(bAviName, ulNameSize, "r+b", sizeof("r+b"), FileID);
    } 
    else {

        error = MMPS_FS_FileOpen(bAviName, ulNameSize, "w+b", sizeof("w+b"), FileID);
    
        if (error != MMP_ERR_NONE || *FileID == 0) {
            *FileID = 0;
            return error;
        }
        
        memset(m_IndexBuff, 0, BufSize);
        
        if (ulWidth != 0) {
            AVIMainHdr.ulWidth = ulWidth;
            AVIStrFormat.ulWidth = ulWidth;
        }
        if (ulHeight != 0) {
            AVIMainHdr.ulHeight = ulHeight;
            AVIStrFormat.ulHeight = ulHeight;
        }
        if (CoedcFourCC != 0) {
            AVIStrHdr.Handler = CoedcFourCC;
            AVIStrFormat.ubCompression = CoedcFourCC;
        }
        if (FrameRate != 0) {
            AVIStrHdr.ulRate = FrameRate;
            AVIMainHdr.ulMSecPreFrame = (1000*1000*1000) / FrameRate;
        }
        if (ulBitRate != 0) {
            AVIMainHdr.ulMaxByteRate = ulBitRate/8;
        }

        AVIStrFormat.ulImageSize = (AVIMainHdr.ulWidth * AVIMainHdr.ulHeight) << 1;
        
        error = MMPS_FS_FileWrite(*FileID, (MMP_UBYTE *)&AVIRiff, sizeof(AVIRiff), &ulWriteCnt);
        if (error != MMP_ERR_NONE) {
            return error;
        }
        
        error = MMPS_FS_FileWrite(*FileID, (MMP_UBYTE *)&AVIHdrl, sizeof(AVIHdrl), &ulWriteCnt);
        if (error != MMP_ERR_NONE) {
            return error;
        }
        
        error = MMPS_FS_FileWrite(*FileID, (MMP_UBYTE *)&AVIMainHdr, sizeof(AVIMainHdr), &ulWriteCnt);
        if (error != MMP_ERR_NONE) {
            return error;
        }
        
        error = MMPS_FS_FileWrite(*FileID, (MMP_UBYTE *)&AVIStrl, sizeof(AVIStrl), &ulWriteCnt);
        if (error != MMP_ERR_NONE) {
            return error;
        }
        
        error = MMPS_FS_FileWrite(*FileID, (MMP_UBYTE *)&AVIStrHdr, sizeof(AVIStrHdr), &ulWriteCnt);
        if (error != MMP_ERR_NONE) {
            return error;
        }
        
        error = MMPS_FS_FileWrite(*FileID, (MMP_UBYTE *)&AVIStrFormat, sizeof(AVIStrFormat), &ulWriteCnt);
        if (error != MMP_ERR_NONE) {
            return error;
        }
        
        error = MMPS_FS_FileWrite(*FileID, (MMP_UBYTE *)&AVIIndex, sizeof(AVIIndex), &ulWriteCnt);
        if (error != MMP_ERR_NONE) {
            return error;
        }

        for (i = 0; i < AVIIndex.ulSize; i += BufSize) {
            if ((AVIIndex.ulSize - i) < BufSize) {
                error = MMPS_FS_FileWrite(*FileID, m_IndexBuff, (AVIIndex.ulSize - i), &ulWriteCnt);
                if (error != MMP_ERR_NONE) {
                    return error;
                }
            } 
            else {
                error = MMPS_FS_FileWrite(*FileID, m_IndexBuff, BufSize, &ulWriteCnt);
                if (error != MMP_ERR_NONE) {
                    return error;
                }
            }
        }

        error = MMPS_FS_FileWrite(*FileID, (MMP_UBYTE *)&AVIMovi, sizeof(AVIMovi), &ulWriteCnt);
        if (error != MMP_ERR_NONE) {
            return error;
        }
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_AVIAppendFrame
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_AVIAppendFrame(MMP_ULONG ulFID, MMP_UBYTE *pData, MMP_ULONG ulSize, MMP_ULONG64 *ulFileSize, MMP_ULONG *ulFrameNum)
{
    MMP_ERR                     error;
    MMP_ULONG                   ulWriteCnt = 0;
    MMP_ULONG64                 ulFSize = 0, ulTempSize, ulTempLen;
    MMPS_3GPRECD_AVI_Header     Chunk;
    MMP_UBYTE                   TempData[4];
    MMPS_3GPRECD_AVI_IndexEntry Entry;
    
    Chunk.ubFourCC = MKTAG('0','0','d','c');
    Chunk.ulSize = ulSize;
    ulSize = ulSize + (ulSize&0x01);

    MMPS_FS_FileGetSize(ulFID, &ulFSize);

    if (((MMP_ULONG64)((MMP_ULONG64)ulFSize + (MMP_ULONG64)ulSize)) > 0xFFFFFFFF) {
        *ulFileSize = ulFSize;
        return MMP_FS_ERR_WRITE_FAIL;
    }
    
    MMPS_FS_FileSeek(ulFID, m_ulMainHdrFNumPos, MMPS_FS_SEEK_SET);
    MMPS_FS_FileRead(ulFID, TempData, 4, &ulWriteCnt);

    ulTempLen = TempData[0] | TempData[1] << 8 | TempData[2] << 16 | TempData[3] << 24;
    if (ulTempLen >= (AVIIndex.ulSize / 16)) {
        *ulFrameNum = ulTempLen;
        return MMP_FS_ERR_WRITE_FAIL;
    }
    
    MMPS_FS_FileSeek(ulFID, 0, MMPS_FS_SEEK_END);
    
    error = MMPS_FS_FileWrite(ulFID, (MMP_UBYTE *)&Chunk, sizeof(Chunk), &ulWriteCnt);
    if (error != MMP_ERR_NONE) {
        return error;
    }
    error = MMPS_FS_FileWrite(ulFID, pData, ulSize, &ulWriteCnt);
    if (error != MMP_ERR_NONE) {
        return error;
    }

    *ulFileSize = ulFSize + ulSize + sizeof(Chunk);
    MMPS_FS_FileSeek(ulFID, m_ulHeaderSizePos, MMPS_FS_SEEK_SET);
    ulTempSize = *ulFileSize - m_ulHeaderSizePos - 4;

    error = MMPS_FS_FileWrite(ulFID, (MMP_UBYTE *)&ulTempSize, 4, &ulWriteCnt);
    if (error != MMP_ERR_NONE) {
        return error;
    }

    // Set Main header frame number
    MMPS_FS_FileSeek(ulFID, m_ulMainHdrFNumPos, MMPS_FS_SEEK_SET);
    MMPS_FS_FileRead(ulFID, TempData, 4, &ulWriteCnt);
    ulTempLen = TempData[0] | TempData[1] << 8 | TempData[2] << 16 | TempData[3] << 24;
    ulTempLen++;
    *ulFrameNum = ulTempLen;

    MMPS_FS_FileSeek(ulFID, m_ulMainHdrFNumPos, MMPS_FS_SEEK_SET);
    error = MMPS_FS_FileWrite(ulFID, (MMP_UBYTE *)&ulTempLen, 4, &ulWriteCnt);
    if (error != MMP_ERR_NONE) {
        return error;
    }

    // Set Stream header frame number
    MMPS_FS_FileSeek(ulFID, m_ulStrHdrLengthPos, MMPS_FS_SEEK_SET);
    error = MMPS_FS_FileWrite(ulFID, (MMP_UBYTE *)&ulTempLen, 4, &ulWriteCnt);
    if (error != MMP_ERR_NONE) {
        return error;
    }
    
    // Set Index entry
    Entry.ubFourCC = MKTAG('0','0','d','c');
    Entry.ulFlag = 0x00000010;
    Entry.ulPos = ulFSize;
    Entry.ulSize = Chunk.ulSize;
    ulTempLen--;

    MMPS_FS_FileSeek(ulFID, (m_ulIndexStartPos + (ulTempLen*16)), MMPS_FS_SEEK_SET);
    error = MMPS_FS_FileWrite(ulFID, (MMP_UBYTE *)&Entry, sizeof(Entry), &ulWriteCnt);
    if (error != MMP_ERR_NONE) {
        return error;
    }
        
    // Set Movi Size
    MMPS_FS_FileSeek(ulFID, m_ulMoviSizePos, MMPS_FS_SEEK_SET);
    ulTempSize = *ulFileSize - m_ulMoviSizePos - 4;
    error = MMPS_FS_FileWrite(ulFID, (MMP_UBYTE *)&ulTempSize, 4, &ulWriteCnt);
    if (error != MMP_ERR_NONE) {
        return error;
    }

    return error;
}
#endif

#if 0
void ____VR_Emergent_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetEmergActionType
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set emergency record flow type.
 @retval MMP_ERR_NONE Success.
*/
MMP_BOOL MMPS_3GPRECD_SetEmergActionType(MMP_3GPRECD_EMERG_ACTION emergact)
{
    #if (EMERGENTRECD_SUPPORT)
    m_EmergActionType = emergact;
    #endif
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetEmergActionType
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Check what emergency record flow type is.
 @retval MMP_ERR_NONE Success.
*/
MMP_3GPRECD_EMERG_ACTION MMPS_3GPRECD_GetEmergActionType(void)
{
    #if (EMERGENTRECD_SUPPORT)
    return m_EmergActionType;
    #endif
    
    return MMP_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetEmergFileName
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Send emergent 3GP file name to firmware for card mode and creat file time.
 @param[in] bFilename File name.
 @param[in] usLength Length of file name in unit of byte.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetEmergFileName(MMP_BYTE bFilename[], MMP_USHORT usLength)
{
    #if (EMERGENTRECD_SUPPORT)
    MMPS_FS_SetCreationTime();
    MMPD_3GPMGR_SetFileName(VIDENC_STREAMTYPE_EMERGENCY, bFilename, usLength);
    #endif
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_EnableEmergentRecd
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Enable emergent video recording.

 Enable emergent video recording.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_EnableEmergentRecd(MMP_BOOL bEnable)
{
    #if (EMERGENTRECD_SUPPORT)
    MMPD_3GPMGR_EnableEmergentRecd(bEnable);
    #endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_EnableUVCEmergentRecd
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Enable UVC emergent video recording.

 Enable uvc emergent video recording.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_EnableUVCEmergentRecd(MMP_BOOL bEnable)
{
    #if (UVC_EMERGRECD_SUPPORT)
    MMPD_UVCRECD_EnableEmergentRecd(bEnable);
    #endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_EnableDualEmergentRecd
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Enable Dual emergent video recording.

 Enable dual emergent video recording.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_EnableDualEmergentRecd(MMP_BOOL bEnable)
{
    #if (DUAL_EMERGRECD_SUPPORT)
    MMPD_3GPMGR_EnableDualEmergentRecd(bEnable);
    #endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_IsDualEmergentRecdEnable
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMPS_3GPRECD_IsDualEmergentRecdEnable(void)
{
    #if (DUAL_EMERGRECD_SUPPORT)
    return MMPD_3GPMGR_IsDualEmergentRecdEnable();
    #endif
    return MMP_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StartEmergentRecd
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Start emergent video recording.

 Start to save the 3GP file.
 @param[in] bStopNormRecd: stop normal record, keep emergent record
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_StartEmergentRecd(MMP_BOOL bStopNormRecd)
{
    #if (EMERGENTRECD_SUPPORT)
    return MMPD_3GPMGR_StartEmergentRecd(bStopNormRecd);
    #else
    return MMP_3GPMGR_ERR_UNSUPPORT;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_StopEmergentRecd
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Stop emergent video recording.

 Stop to save the 3GP file.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPS_3GPRECD_StopEmergentRecd(MMP_BOOL bBlocking)
{
    #if (EMERGENTRECD_SUPPORT)
    MMP_BOOL    bEnable, bRecord;
    MMP_ULONG   ulTimeout = VR_QUERY_STATES_TIMEOUT; 

    MMPD_3GPMGR_GetEmergentRecStatus(&bEnable, &bRecord);

    if (bEnable && bRecord) {
        MMPD_3GPMGR_StopEmergentRecd();

        if (!bBlocking) {
            return MMP_ERR_NONE;
        }
        
        do {
            MMPD_3GPMGR_GetEmergentRecStatus(&bEnable, &bRecord);
            if (bRecord != MMP_FALSE) {
                MMPF_OS_Sleep(1);
            }
        } while ((bRecord != MMP_FALSE) && ((--ulTimeout) > 0));

        if (0 == ulTimeout) {
            MMP_PRINT_RET_ERROR(0, 0, "", gubMmpDbgBk);
        }
    }
    else {
        return MMP_3GPMGR_ERR_INVLAID_STATE;
    }
    #endif
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetEmergentFileTimeLimit
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set the maximum 3GP file time for video encoding.
 @param[in] ulTimeLimitMs Maximum file time in unit of ms.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE Space not enough.
*/
MMP_ERR MMPS_3GPRECD_SetEmergentFileTimeLimit(MMP_ULONG ulTimeLimitMs)
{
    #if (EMERGENTRECD_SUPPORT)
    if (ulTimeLimitMs) {
        return MMPD_3GPMGR_SetEmergentTimeLimit(ulTimeLimitMs);
    }
    #endif

    return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetEmergentFileSizeLimit
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set the maximum video file size for video encoding.
 @param[in] ulSizeLimitBytes Maximum file size in unit of byte.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_INVALID_PARAMETERS Wrong parameter.
*/
MMP_ERR MMPS_3GPRECD_SetEmergentFileSizeLimit(MMP_ULONG ulSizeLimitBytes)
{
    #if (EMERGENTRECD_SUPPORT)
    if (ulSizeLimitBytes) {
        return MMPD_3GPMGR_SetEmergentSizeLimit(ulSizeLimitBytes);
    }
    #endif

    return MMP_3GPRECD_ERR_INVALID_PARAMETERS;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetEmergPreEncTimeLimit
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set the the pre-encoding time limit.
 @param[in] ulTimeLimitMs Maximum file time in ms.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_NOT_ENOUGH_SPACE Space not enough.
*/
MMP_ERR MMPS_3GPRECD_SetEmergPreEncTimeLimit(MMP_ULONG ulTimeLimitMs)
{
    #if (EMERGENTRECD_SUPPORT)
    if (ulTimeLimitMs) {
        return MMPD_3GPMGR_SetEmergPreEncTimeLimit(ulTimeLimitMs);
    }
    #endif

    return MMP_ERR_NONE;//MMP_3GPRECD_ERR_INVALID_PARAMETERS;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetEmergentRecordingTime
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get emergent recording time.
 @param[out] ulTime Recording time in unit of ms.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetEmergentRecordingTime(MMP_ULONG *ulTime)
{
	#if (EMERGENTRECD_SUPPORT)
    return MMPD_3GPMGR_GetRecordingTime(VIDENC_STREAMTYPE_EMERGENCY, ulTime);
	#else
	return MMP_ERR_NONE;
	#endif	
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_GetEmergentRecordingOffset
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get emergent recording time offset.
 @param[out] ulTime Recording time offset in unit of ms.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetEmergentRecordingOffset(MMP_ULONG *ulTime)
{
	#if (EMERGENTRECD_SUPPORT)
    return MMPD_3GPMGR_GetRecordingOffset(VIDENC_STREAMTYPE_EMERGENCY, ulTime);
	#else
	return MMP_ERR_NONE;
	#endif
}

#if 0
void ____VR_Misc_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetTime2FlushFSCache
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set max time to flush FS cache buffer to SD.

 @param[in] time Set max time (s).
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetTime2FlushFSCache(MMP_ULONG ulTime)
{
	MMPD_3GPMGR_SetTime2FlushFSCache(ulTime*1000);
	
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_CheckFile2Refix
//  Description :
//------------------------------------------------------------------------------
/**
 @brief check if there is a file need to be refixed.

 @retval MMP_ERR_NONE Success.
*/
#if (SUPPORT_VR_REFIX_TAILINFO)
MMP_ERR MMPS_3GPRECD_CheckFile2Refix(void)
{
	MMPD_3GPMGR_CheckFile2Refix();
	
    return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetYFrameType
//  Description : Configure the usage of Y-frame pipe (ex: Motion Detection or LDWS)
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetYFrameType(MMPS_3GPRECD_Y_FRAME_TYPE enType)
{
#if (SUPPORT_MDTC)||(SUPPORT_ADAS)
    m_YFrameType = enType;
#endif
    return MMP_ERR_NONE;
}

#if (DUALENC_SUPPORT)
//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetSWStickerAttribute
//  Description : Use for set SW sticker attribute.
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetSWStickerAttribute(MMP_USHORT usStickerSrcWidth, MMP_USHORT usStickerSrcHeight, 
                                           MMP_USHORT usDstStartx, MMP_USHORT usDstStarty)
{
    return MMPD_VIDENC_SetSWStickerAttr(usStickerSrcWidth, usStickerSrcHeight,
                                        usDstStartx, usDstStarty);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_SetSWStickerAddress
//  Description : Use for set SW sticker address.
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_SetSWStickerAddress(MMP_ULONG ulStickerSrcAddr)
{
    return MMPD_VIDENC_SetSWStickerAddress(ulStickerSrcAddr);
}
#endif

/**
 @brief Add Atom IDIT for GVP, the atom store starting time of recording.

 @retval size of atom IDIT
*/
#if (MGR_SUPPORT_AVI == 1)
#if (AVI_IDIT_CHUNK_EN == 1)

#define	_3GP_FOURCC(b0, b1, b2, b3) (((unsigned long)(b0)      ) +	\
                                     ((unsigned long)(b1) <<  8) +	\
                                     ((unsigned long)(b2) << 16) +	\
                                     ((unsigned long)(b3) << 24))

#define	ATOM_IDIT		_3GP_FOURCC('I', 'D', 'I', 'T')
#define	ATOM_IDIT_SIZE	sizeof(struct idit_t)
#define	ATOM_IDIT_DACB	20

typedef struct idit_t {
    unsigned long	atombc;
    unsigned long	atomid;
    unsigned char	tmstr[ATOM_IDIT_DACB];
} IDIT_T;

typedef struct idit_avi {
    unsigned long	atomid;
    unsigned long	atombc;
    unsigned char	tmstr[ATOM_IDIT_DACB];
} IDIT_AVI;

extern AUTL_DATETIME gVidRecdRTCTime;

__inline unsigned int CONVERT_ENDIAN(unsigned int d)
{
    unsigned int t = d;
    
    *((char*)&d + 3) = *((char*)&t + 0);
    *((char*)&d + 2) = *((char*)&t + 1);
    *((char*)&d + 1) = *((char*)&t + 2);
    *((char*)&d + 0) = *((char*)&t + 3);

    return d;
}

unsigned int MMPS_3GPMUX_Build_IDIT(void **ptr)
{
    extern int snprintf(char * /*s*/, size_t /*n*/, const char * /*format*/, ...);

    static IDIT_T	idit;
    
    idit.atombc = CONVERT_ENDIAN(ATOM_IDIT_SIZE);
    idit.atomid = ATOM_IDIT;
    
    snprintf((void*)idit.tmstr, ATOM_IDIT_DACB, 
             "%04d-%02d-%02d %02d:%02d:%02d",
             gVidRecdRTCTime.usYear,
             gVidRecdRTCTime.usMonth,
             gVidRecdRTCTime.usDay,
             gVidRecdRTCTime.usHour,
             gVidRecdRTCTime.usMinute,
            gVidRecdRTCTime.usSecond);
            
    *ptr = &idit;
    
    return ATOM_IDIT_SIZE;
}

unsigned int MMPS_AVIMUX_Build_IDIT(void **ptr)
{
    extern int snprintf(char * /*s*/, size_t /*n*/, const char * /*format*/, ...);

    static IDIT_AVI	idit;
    
    idit.atombc = ATOM_IDIT_DACB;
    idit.atomid = MAKE_FCC('I','D','I','T');;
    
    snprintf((void*)idit.tmstr, ATOM_IDIT_DACB, 
             "%04d-%02d-%02d %02d:%02d:%02d",
             gVidRecdRTCTime.usYear,
             gVidRecdRTCTime.usMonth,
             gVidRecdRTCTime.usDay,
             gVidRecdRTCTime.usHour,
             gVidRecdRTCTime.usMinute,
            gVidRecdRTCTime.usSecond);
            
    *ptr = &idit;
    
    return ATOM_IDIT_SIZE;
}

#endif //#if (AVI_IDIT_CHUNK_EN == 1
#endif //#if (MGR_SUPPORT_AVI == 1)

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_PreviewPipeInUVCMixModeEnable
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_3GPRECD_PreviewPipeInUVCMixModeEnable(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    MMP_ERR sRet = MMP_ERR_NONE;
    static MMP_PIPE_LINK sPreviewFctlLinkBackup = {0};

    if (ubSnrSel == PRM_SENSOR) {
        if (bEnable) {
            sPreviewFctlLinkBackup = m_PreviewFctlLink;
            m_PreviewFctlLink = m_PreviewFctlLinkInUVCMixMode;
        }
        else {
            m_PreviewFctlLink = sPreviewFctlLinkBackup;
        }
    }
    else if (ubSnrSel == SCD_SENSOR) {
        if (bEnable) { // UVC mix mode does not support second sensor.
            sRet = MMP_HIF_ERR_PARAMETER;
        }
        
        if (sRet != MMP_ERR_NONE) {MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk);}                                                               
    }

    return sRet;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_3GPRECD_ModifyAVIListAtom
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Modify LIST Atom for AVI format.

 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_ModifyAVIListAtom(MMP_BOOL bEnable, MMP_BYTE *pStr)
{
    return MMPD_3GPMGR_ModifyAVIListAtom(bEnable, pStr);
}

/// @}
