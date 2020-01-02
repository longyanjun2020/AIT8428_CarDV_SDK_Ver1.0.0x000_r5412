//
// net_utility.c: Utility functions used by network. Typically CGIs.
// used to be netapp2.c
//

#include "AHC_MENU.h"
#include "AHC_Stream.h"
#include "AHC_Display.h"	// CarDV - Aaron
#include "AHC_Browser.h"
#include "MenuSetting.h"
#include "LedControl.h"

#include "amn_osal.h"
#include "amn_sysobjs.h"
#include "amn_event.h"
#include "ucos_osl_ext.h"

#include "netapp.h"
#include "net_utility.h"
#include "net_serv.h"

#include "mmpd_scaler.h"
#include "mmpf_dsc.h"
#include "mmpf_exif.h"
#include "mmp_dsc_inc.h"
#include "mmpd_dsc.h"
#include "mmpf_fs_api.h"
#include "mmps_vidplay.h"

/*******************************************************************
 *
 * Miscellaneous for NETWORK
 *
 *******************************************************************/
//
extern MMP_ULONG m_ulMediaFileNameAddr;
extern MMP_ULONG m_ulDSCFileID;
extern MMP_ULONG64 m_ulDscFileSize;

extern MMP_ERR MMPF_DSC_ReadJpegFile(MMP_UBYTE *ubTarget, MMP_ULONG ulSeekpos, MMP_ULONG ulReadsize);
MMP_UBYTE* MMPF_DSC_MallocBuf(MMP_USHORT usExifNodeId, MMP_ULONG ulSize, MMP_BOOL bForUpdate);

#define	MI_FILE_JPEG		0
#define	MI_FILE_VIDEO		1
#define CUR_READ_INSIDE_STRUCT (1)
typedef struct MI_THUMBNAIL{
	char			file[64];
	int				type;
	int				width;
	int				height;
	int				bufsize;
	int				jsize;
	#define THUMBNAIL_FOUND(pmi) ((pmi)->jsize > 0)
#if CUR_READ_INSIDE_STRUCT
	int             cur_read;
#endif
	unsigned char	*jbuf;
} MI_THUMBNAIL;
#if CUR_READ_INSIDE_STRUCT
#define CUR_READ (pmi->cur_read)
#else
	int             cur_read;
#define CUR_READ cur_read
#endif
#define	MAX_JPEG_SIZE		(70*1024)

static int miDigThumbnailInJpg(char *filename, unsigned char **ppbuf, int bufsize, int *width, int *height);
static int miDigThumbnailInMov(char *filename, unsigned char **ppbuf, int bufsize, int *width, int *height);
//int miGetJPEGLargeThumbnail(char *jpgFileName, unsigned char *pbuf, int bufsize, int *width, int *height);
static int miRebuildVideoThumbnail(char *movFileName, unsigned char *pbuf, int bufsize, int *width, int *height);
static int _miRebuildImageThumbnail(char *jpgFileName, unsigned char *pbuf, int bufsize, int *width, int *height, int type);
static int miRebuildImageThumbnail(char *jpgFileName, unsigned char *pbuf, int bufsize, int *width, int *height)
{
	*width  = NET_THUMBNAIL_WIDTH;
	*height = NET_THUMBNAIL_HEIGHT;
	return _miRebuildImageThumbnail(jpgFileName, pbuf, bufsize, width, height, MI_FILE_JPEG);
}

static int miIsValidThumbUiState(enum NS_MEDIA_TYPE ns_media_type)
{
	int ret;
	ret = ns_handle_event(NS_EVENT_QUERY_THUMB_STATE, (void*)ns_media_type);
	if (ret == NSE_CB_NOT_FOUND) {
#ifdef NET_SYNC_PLAYBACK_MODE //move to later step
		if(ncgi_get_ui_state_id() != UI_NET_PLAYBACK_STATE) {
			printd(FG_YELLOW("Need to be in UI_NET_PLAYBACK_STATE")"\r\n");
			return 0;
		}
#endif
	}
	else if (ANY_NS_ERR(ret)) {
		printc("Invalid UI State err:%d\r\n", ret);
		return 0;
	}
	return 1;
}
/*
 * Give a filename and buf/size, return thumbnail of jpeg, width,height and size.
 * Caller has to free *pbuf by osal_free if *pbuf is NULL and bufize is 0.
 * @param[out] error error type of enum vhand_authen_spec_t
 */
MI_THUMBNAIL* miOpenThumbnail(char *filename, int *error)
{
	MI_THUMBNAIL	*pmi;
	char *p;
	int (*DigThumbnailFunc)(char *filename, unsigned char **ppbuf, int bufsize, int *width, int *height) = NULL;
	int (*RebuildThumbnailFunc)(char *jpgFileName, unsigned char *pbuf, int bufsize, int *width, int *height) = NULL;
	enum NS_MEDIA_TYPE media_type;
	int header_size;
	*error = VHAND_ERR__ILLEGAL_URL_PARAM;

	pmi = NULL;
	
	// Check File Type
	p = strrchr(filename, '.');
	if (p == NULL) {
		return NULL;
	}
	pmi = (MI_THUMBNAIL*)osal_malloc(MAX_JPEG_SIZE);
	if (pmi == NULL) {
		*error = VHAND_ERR__INTERNAL;
		return NULL;
	}

	header_size = ALIGN32(sizeof(MI_THUMBNAIL));//align 32 is for hardware
	pmi->bufsize = MAX_JPEG_SIZE - header_size;
	pmi->jsize   = 0;
	pmi->jbuf    = (unsigned char*)pmi + header_size;
	strncpy(pmi->file, filename, sizeof(pmi->file));

	if (stricmp(p + 1, "JPG") == 0) {
		pmi->type = MI_FILE_JPEG;
		media_type = NS_MEDIA_JPEG;
		DigThumbnailFunc = miDigThumbnailInJpg;
		RebuildThumbnailFunc = miRebuildImageThumbnail;
	} else {
		pmi->type = MI_FILE_VIDEO;
		media_type = NS_MEDIA_MOV;
	#if (SUPPORT_VR_THUMBNAIL)
		DigThumbnailFunc = miDigThumbnailInMov;
	#endif
		RebuildThumbnailFunc = miRebuildVideoThumbnail;
	}
	if (DigThumbnailFunc) {
		pmi->jsize = DigThumbnailFunc(pmi->file, &pmi->jbuf, pmi->bufsize, &pmi->width, &pmi->height);
	}
	if (!THUMBNAIL_FOUND(pmi)) {
		// UI State Check
		if (miIsValidThumbUiState(media_type)) {
			// Create JPEG from JPEG
			if (netapp_cap_lock(LOCK_CAPTURE, LOCK_CAPTURE_TIMEOUT) == 0) {
				pmi->jsize = RebuildThumbnailFunc(pmi->file, pmi->jbuf, pmi->bufsize, &pmi->width, &pmi->height);
				netapp_cap_lock(UNLOCK_CAPTURE, 0);
			} else {
				*error = VHAND_ERR__INTERNAL;
			}
		} else {
			*error = VHAND_ERR__OUT_OF_SERVICE;
		}
	}
	CUR_READ = 0;
	if (THUMBNAIL_FOUND(pmi)) {
		*error = VHAND_OK__NO_CHECK_AUTH;
	} else {
		osal_free(pmi);
		pmi = NULL;
	}
	return pmi;
}

void miCloseThumbnail(MI_THUMBNAIL *handle)
{
	if (handle == NULL)
		return;
	osal_free(handle);
}

/* @brief Create a new JPEG thumbnail from JPEG File. It Decodes JPEG and Encodes into smaller JPEG.
 *
 * Give a filename and output a buffer of thumbnail
 *
 * @param[in] jpgFileName The file name of JPEG file.
 * @param[in,out] pbuf The output buffer of the JPEG.
 * @param[in] bufsize The size of the output JPEG buffer in bytes.
 * @param[out] width, height The resolution of the JPEG.
 * @param[in] type MI_FILE_JPEG Decode this JPEG file as source.
 *                 MI_FILE_VIDEO Use pbuf as JPEG source but use Video thumbnail resolution.
 * When pbuf is NULL return the thumbnail size and width/height
 * @warning This function requires extra buffer, a clean UI state for memory map, and fixed hardware image pipeline.
 *          Thus, it switches AHC mode inside! It also pauses streaming.
 *          If the project does not comply this behavior, it is better avoid using this function.
 *          e.g. The project need to get the thumbnail while video is previewing, then it does not comply.
 */
static int _miRebuildImageThumbnail(char *jpgFileName, unsigned char *pbuf, int bufsize, int *width, int *height, int type)
{
	MMP_DSC_JPEG_INFO       jpeginfo;
    MMP_GRAPHICS_BUF_ATTR   BufAttr;
    MMP_ERR				    err;
    MI_INFO				    miinfo;
    unsigned short		    w, h;
	MMP_ULONG			    ulJpegSize;
	AHC_MODE_ID			    save;
	int					    ret;
    
 	if (miGetMediaInfo(jpgFileName, &miinfo) == 0) {
 		return 0;
 	}
	ret = 0;
	save = AHC_GetAhcSysMode();
	if(AHC_MODE_IDLE != save){
		ncam_set_streaming_mode(AHC_STREAM_PAUSE);
		AHC_SetMode(AHC_MODE_IDLE);
	}
	//
 	w = (*width  + 15) & (~0xf);
 	h = (miinfo.height * (*width) / miinfo.width + 15) & (~0xf);

    MEMSET0(&jpeginfo);
#if (SUPPORT_VR_THUMBNAIL)
    if (type == MI_FILE_JPEG)
#endif        
    {    
        strcpy(jpeginfo.bJpegFileName, jpgFileName);
        jpeginfo.usJpegFileNameLength   = strlen(jpgFileName); //+1;
        jpeginfo.ulJpegBufAddr          = 0;
        jpeginfo.ulJpegBufSize          = 0;
    	jpeginfo.bDecodeThumbnail       = MMP_FALSE;
    	#if (DSC_SUPPORT_BASELINE_MP_FILE)
    	jpeginfo.bDecodeLargeThumb		= MMP_TRUE;
    	#endif
        jpeginfo.bValid                 = MMP_FALSE;
    }
#if (SUPPORT_VR_THUMBNAIL)
    else {
    	jpeginfo.bDecodeThumbnail       = MMP_TRUE;
    	#if (DSC_SUPPORT_BASELINE_MP_FILE)
    	jpeginfo.bDecodeLargeThumb		= MMP_TRUE;
    	#endif
        if (!bufsize) {
    	 	printc(BG_RED("%s %d")"\r\n", __func__, __LINE__);
    		goto End_miCreateJPEGThumbnail;
        }                     
        else {
            UINT32 ulVRThumbJpgW, ulVRThumbJpgH;
            MMPS_3GPRECD_GetVRThumbJpgSize(&ulVRThumbJpgW, &ulVRThumbJpgH);     
            jpeginfo.usJpegFileNameLength   = NULL;
            jpeginfo.ulJpegBufAddr          = (MMP_ULONG)(pbuf);
            jpeginfo.ulJpegBufSize          = bufsize;
            jpeginfo.usPrimaryWidth         = ulVRThumbJpgW;
            jpeginfo.usPrimaryHeight        = ulVRThumbJpgH;
        }
    }
#endif
    MMPS_DSC_SetCaptureJpegQuality(MMP_DSC_JPEG_RC_ID_CAPTURE, MMP_FALSE, MMP_FALSE, 200, 220, 3, MMP_DSC_CAPTURE_HIGH_QUALITY);

    err = MMPS_DSC_DecodeThumbFirst(&jpeginfo, 
                                    w,
                                    h,
                                    MMP_DISPLAY_COLOR_RGB565);

	if (err != MMP_ERR_NONE) {
	 	printc(BG_RED("%s %d")"\r\n", __func__, __LINE__);
		goto End_miCreateJPEGThumbnail;
	}
    MMPS_DSC_GetJpegDispBufAttr(&BufAttr);
	/* for debug -- Write raw to file
	printc(FG_YELLOW("Write to 001.raw %dx%d\n"), BufAttr.usWidth, BufAttr.usHeight);
	MMPD_DSC_SetFileName("SD:\\001.raw", strlen("SD:\\001.raw"));
	MMPD_DSC_JpegDram2Card(BufAttr.ulBaseAddr,
						   BufAttr.usWidth * BufAttr.usHeight,
						   MMP_TRUE, MMP_TRUE);
	*/

    {
    	MMP_SCAL_FIT_RANGE  fitrange;
    	MMP_SCAL_GRAB_CTRL  grabctl;
    	MMP_DSC_CAPTURE_BUF capturebuf;
    	MMP_GRAPHICS_RECT   rect;
    	MMP_ULONG           ulSramCurPos = 0x104000;
    	MMPD_FCTL_ATTR 		fctlAttr;
    	MMP_PIPE_LINK       m_fctlLinkCapture = {
    			MMP_SCAL_PIPE_0,
				MMP_ICO_PIPE_0,
				MMP_IBC_PIPE_0
    	};
    	extern MMP_ERR MMPD_System_GetSramEndAddr(MMP_ULONG *ulAddress);

    	MMPD_System_GetSramEndAddr(&ulSramCurPos);
    	capturebuf.ulCompressStart  = BufAttr.ulBaseAddr + ALIGN32(BufAttr.usWidth * 2) * BufAttr.usHeight;
		capturebuf.ulCompressEnd = ALIGN16(
				capturebuf.ulCompressStart + jpeginfo.usPrimaryWidth * 2 * jpeginfo.usPrimaryHeight);
    	capturebuf.ulLineStart      = ulSramCurPos;
    	MMPD_DSC_SetCaptureBuffers(&capturebuf);

    	rect.usLeft     		= 0;
    	rect.usTop      		= 0;
    	rect.usWidth    		= BufAttr.usWidth;
    	rect.usHeight   		= BufAttr.usHeight;

    	fitrange.fitmode     	= MMP_SCAL_FITMODE_OUT;
    	fitrange.scalerType		= MMP_SCAL_TYPE_SCALER;
    	fitrange.ulInWidth   	= BufAttr.usWidth;
    	fitrange.ulInHeight  	= BufAttr.usHeight;
    	fitrange.ulOutWidth  	= BufAttr.usWidth;
    	fitrange.ulOutHeight 	= BufAttr.usHeight;

    	fitrange.ulInGrabX		= 1;
    	fitrange.ulInGrabY		= 1;
    	fitrange.ulInGrabW		= fitrange.ulInWidth;
    	fitrange.ulInGrabH		= fitrange.ulInHeight;

    	MMPD_Scaler_GetGCDBestFitScale(&fitrange, &grabctl);

    	fctlAttr.fctllink       = m_fctlLinkCapture;
    	fctlAttr.fitrange       = fitrange;
    	fctlAttr.grabctl        = grabctl;
    	fctlAttr.scalsrc		= MMP_SCAL_SOURCE_GRA;
    	fctlAttr.bSetScalerSrc	= MMP_TRUE;

    	if (MMP_ERR_NONE != MMPD_DSC_RawBuf2Jpeg(&BufAttr, &rect, 1, &fctlAttr)) {
    		ret = -1;
    		goto End_miCreateJPEGThumbnail;
    	}
    	(void)MMPD_DSC_GetJpegSize(&ulJpegSize);

    	if (ulJpegSize > bufsize) {
    		printc(BG_RED("%s %d: ERROR JPEG size %d but buffer size %d\n"), __func__, __LINE__,
    				ulJpegSize, bufsize);
    		goto End_miCreateJPEGThumbnail;
    	}
    	/* for debug
	MMPD_DSC_SetFileName("SD:\\001.jpg", strlen("SD:\\001.jpg"));
	MMPD_DSC_JpegDram2Card(capturebuf.ulCompressStart, ulJpegSize, MMP_TRUE, MMP_TRUE);
    	 */
    	memcpy(pbuf, (void*)capturebuf.ulCompressStart, ulJpegSize);
    }
    *width  = w;
    *height = h;
 	ret = (int)ulJpegSize;
End_miCreateJPEGThumbnail:
	if(AHC_MODE_IDLE != save){
		AHC_SetMode(save);
		ncam_set_streaming_mode(AHC_STREAM_RESUME);
	}
	return ret;
}

/* @brief Claim the existing thumbnail data from handle.
 *
 * The handle must be successfully opened.
 *
 */
int miGetThumbnail(MI_THUMBNAIL *pmi, unsigned char **jpegbuf)
{
	
	if (!pmi) return 0;

	if (jpegbuf != NULL) {
		*jpegbuf = pmi->jbuf + CUR_READ;
		if ((pmi->jsize - CUR_READ) > 8192) {
			CUR_READ += 8192;
			return 8192;
		}
		return pmi->jsize - CUR_READ;
	}
	return pmi->jsize;
}

int miGetThumbnailSize(MI_THUMBNAIL *handle)
{
	MI_THUMBNAIL	*pmi;
	
	if (!handle) return 0;
	pmi = (MI_THUMBNAIL*)handle;
	return pmi->jsize;
}

int miGetThumbnailDimension(MI_THUMBNAIL *handle, int *pwidth, int *pheight)
{
	MI_THUMBNAIL	*pmi;
	
	if (!handle) return 0;
	pmi = (MI_THUMBNAIL*)handle;
	*pwidth  = pmi->width;
	*pheight = pmi->height;
	return 1;
}

/** @brief Extract/Dig JPEG thumbnail out from JPEG Image file
 *
 *  Extract JPEG thumbnail from JPEG file if there is any.
 *
 *  @note This function should be able to call without extra memory and could be used with any UI State.
 *  @warning This is not a thread-safe function because it uses common member variables in MMPF function.
 */
static int miDigThumbnailInJpg(char *filename, unsigned char **ppbuf, int bufsize, int *width, int *height)
{
	MMP_ULONG			offset, size;
	unsigned int		ret;
	unsigned short		usJpgWidth, usJpgHeight;
	MMP_ULONG			exifBufSize;
	MMP_UBYTE			*exifBuf = NULL;
	MMP_ULONG			cache, rsiz;
	MMP_DSC_JPEG_FMT	jpgFormat;
	MMP_DSC_JPEG_INFO 	sJpegInfo;
	unsigned char *     pbuf = *ppbuf;
	
	m_ulMediaFileNameAddr = (MMP_ULONG)filename;
	if (MMPF_DSC_OpenJpegDecFile() != MMP_ERR_NONE) {
		printc(BG_RED("ERROR: %s %d Open - %s")"\r\n", __func__, __LINE__, filename);
		return -1;
	}
	if (m_ulDscFileSize == 0) {
		printc(BG_RED("ERROR: %s %d %s")"\r\n", __func__, __LINE__, filename);
		ret = -1;
		goto END_miGetJPEGThumbnail;
	}
	
	MMPF_DSC_ResetPreSeekPos();
	
	MMPF_DSC_SetJpgDecOffset(0, m_ulDscFileSize);
	//
    MMPD_DSC_GetBufferSize(MMPD_DSC_EXIF_WORK_BUFFER, 0, 0 ,0, &exifBufSize);
    exifBuf = (MMP_UBYTE*)osal_malloc(exifBufSize);
    if (exifBuf == NULL) {
		printc(BG_RED("ERROR: %s %d Memory not enough %d")"\r\n", __func__, __LINE__, exifBufSize);
		ret = -1;
		goto END_miGetJPEGThumbnail;
    }
    // Give working buffer, but its size is less then MMPS flow,
    // and the buffer is from Network memory POOL (cachable).
    // So, it must flush/invalidate before/after writing data into buffer 
	MMPD_DSC_SetExifWorkingBuffer(EXIF_NODE_ID_PRIMARY, (MMP_UBYTE *)exifBuf, exifBufSize / 2, MMP_FALSE);
    MMPD_DSC_SetExifWorkingBuffer(EXIF_NODE_ID_PRIMARY, (MMP_UBYTE *)exifBuf + (exifBufSize / 2),
    								exifBufSize / 2, MMP_TRUE);
	cache = (MMP_ULONG)MMPF_DSC_MallocBuf(EXIF_NODE_ID_PRIMARY, 1024, MMP_TRUE);
    MMPF_DSC_SetDecodeInputBuf(cache, 1024, 0, 0);

	if (MMPF_EXIF_GetThumbInfo(&sJpegInfo, MMPF_DSC_ReadJpegFile) != MMP_ERR_NONE) {
		printc("%s %d: %s NO Thumbnail\n", __func__, __LINE__, filename);
		ret = 0;
		goto END_miGetJPEGThumbnail;
	}

	offset = sJpegInfo.ulThumbOffset;
	size = sJpegInfo.ulThumbSize;
	
	MMPF_DSC_SetJpgDecOffset(offset, offset + size);
	
	if (MMPF_DSC_ScanJpegMarker(offset, offset + size,
								&usJpgWidth, &usJpgHeight, &jpgFormat) != MMP_ERR_NONE) {
		printc("%s %d: %s Thumbnail ERROR!\n", __func__, __LINE__, filename);
		ret = -1;
		goto END_miGetJPEGThumbnail;
	}
	if (width != NULL && height != NULL) {
		*width  = (int)usJpgWidth;
		*height = (int)usJpgHeight;
	}
	// free exif buffer
	osal_free(exifBuf);
	exifBuf = NULL;
	//
	if (pbuf) {
		// alloc buffer of thumbnail for caller when caller not specify address.
		if (bufsize < size) {
			printc("%s %d:Buffer Error!\n", __func__, __LINE__);
			ret = -1;
			goto END_miGetJPEGThumbnail;
		}
		// Seek to thumbnail start position
		MMPF_FS_FSeek(m_ulDSCFileID, offset, MMP_FS_FILE_BEGIN);
		// flush all data in cache into physical memory
		osal_flush_dcache();
		if (MMPF_FS_FRead(m_ulDSCFileID, pbuf, size, &rsiz) != MMP_ERR_NONE ||
			rsiz != size) {
			printc("%s %d: %s Thumbnail Read ERROR!\n", __func__, __LINE__, filename);
			ret = -1;
			goto END_miGetJPEGThumbnail;
		}
		// Make D-cache is invalid to
		// make sure CPU(Program) can read correct data that in physical memory.
		//osal_invalidate_dcache(((unsigned int)(pbuf) & ~(MEM_ALIGNMENT_HEAP - 1)) , size);
		osal_invalidate_dcache(((unsigned int)(pbuf)) , size);
	}
	// return thumbnail size
	ret = size;
END_miGetJPEGThumbnail:
	MMPF_DSC_CloseJpegDecFile();
	if (exifBuf)
		osal_free(exifBuf);
	return ret;
}

#if (SUPPORT_VR_THUMBNAIL)
static int miDigThumbnailInMov(char *filename, unsigned char **pbuf, int bufsize, int *width, int *height)
{
	MMP_ULONG jpg_size = 0;
	MMP_ULONG fn_len;

	if (filename == NULL) return -1;
	fn_len = strlen(filename);
	//not implemented, the existing video thumbnail resolution remains unknown.
	*width = 0;
	*height = 0;

	//ignore return value because the size is always 0 when error.
	(void)MMPS_VIDPLAY_GetThumbnail(filename, fn_len, bufsize, pbuf, &jpg_size);
	return jpg_size;
}
#endif

/* @brief Create a new JPEG thumbnail from Video File. (Decode Video and Encode into JPEG)
 *
 * @param[in] movFileName The file name of the mov/mp4/3gp file.
 * @param[out] pbuf The output buffer of the JPEG.
 * @param[in] bufsize The size of the output JPEG buffer in bytes.
 * @param[out] width, height The resolution of the JPEG.
 * When pbuf is NULL return the thumbnail size and width/height
 * @warning This function requires extra buffer, a clean UI state for memory map, and fixed hardware image pipeline.
 *          Thus, it switches AHC mode inside! It also pauses streaming.
 *          If the project does not comply this behavior, it is better avoid using this function.
 *          e.g. The project need to get the thumbnail while video is previewing, then it does not comply.
 */
static int miRebuildVideoThumbnail(char *movFileName, unsigned char *pbuf, int bufsize, int *width, int *height)
{
	int			ret, size;
	MMP_UBYTE	*jpegBuf;
	AHC_MODE_ID	save;
	
	MMP_UBYTE	ubLCDBKPowerOff = 0;			// CarDV - Aaron
	AHC_DISPLAY_OUTPUTPANEL		OutputDevice;	// CarDV - Aaron

	if (pbuf == NULL) {
		jpegBuf = (MMP_UBYTE*)osal_malloc(MAX_JPEG_SIZE);
		if (jpegBuf == NULL) {
			printc(BG_RED("%s %d: ERROR at Memory Alloc\n"), __func__, __LINE__);
			ret = -1;
			goto END_miGetVideoThumbnail;
		}
		size = MAX_JPEG_SIZE;
	} else {
		jpegBuf = pbuf;
		size = bufsize;
	}
	save = AHC_MODE_MAX;
	if (ncgi_get_ui_state_id() != UI_NET_PLAYBACK_STATE) {

		/// CarDV - Aaron +++
		// Read media files, LCD will be huaping.
		AHC_GetDisplayOutputDev(&OutputDevice);
		if(OutputDevice == MMP_DISPLAY_SEL_MAIN_LCD)
		{
			ubLCDBKPowerOff = AHC_TRUE;
			LedCtrl_LcdBackLight(AHC_FALSE);
		}
		// CarDV - Aaron ---

		ncam_set_streaming_mode(AHC_STREAM_PAUSE);
		save = AHC_GetAhcSysMode();
		AHC_SetMode(AHC_MODE_IDLE);
	}
	// flush all data in cache into physical memory
	osal_flush_dcache();

	ret = AHC_Thumb_GetJPEGFromVidFF((UINT8*)jpegBuf, size, movFileName, NET_THUMBNAIL_WIDTH, NET_THUMBNAIL_HEIGHT);	
	if (save != AHC_MODE_MAX) {
		AHC_SetMode(save);
		ncam_set_streaming_mode(AHC_STREAM_RESUME);
	}
	if (ret < 0 || ret > size) {
		printc(BG_RED("%s %d: ERROR File - %s\n"), __func__, __LINE__, movFileName);
		ret = -1;
		goto END_miGetVideoThumbnail;
	}
	osal_invalidate_dcache(((unsigned int)(jpegBuf) & ~(MEM_ALIGNMENT_HEAP - 1)) , size);
	*width  = NET_THUMBNAIL_WIDTH;
	*height = NET_THUMBNAIL_HEIGHT;
	if (pbuf == NULL) {
		if (jpegBuf) osal_free(jpegBuf);
	}
END_miGetVideoThumbnail:
		// CarDV - Aaron +++
		AHC_GetDisplayOutputDev(&OutputDevice);

		if(OutputDevice == MMP_DISPLAY_SEL_MAIN_LCD)
		{
			if(ubLCDBKPowerOff)
			{
				if(!LedCtrl_GetBacklightStatus())
				{
					LedCtrl_LcdBackLight(AHC_TRUE);
				}
			}
		}
		// CarDV - Aaron ---
	return ret;
}

//
typedef	unsigned short JPEGTAG;
typedef struct {
	JPEGTAG			tag;
	unsigned short	size;
} JTAG_HEADER;
//
#define	TAG_JPEG	0xffd8
#define	TAG_APP1	0xffe1
#define	TAG_SOF		0xffc0
#define	TAG_QT		0xffdb
#define	MAKE_JTAG_HEADER(jh, src)	jh.tag  = (*(src)     << 8) + (*(src + 1));\
									jh.size = (*(src + 2) << 8) + (*(src + 3));
//
#define	ATOM(a, b, c, d)	((a) + (b << 8) + (c << 16) + (d << 24))
#define	ATOM_ftyp	ATOM('f','t','y','p')
#define	ATOM_moov	ATOM('m','o','o','v')
#define	ATOM_mvhd	ATOM('m','v','h','d')
#define	ATOM_trak	ATOM('t','r','a','k')
#define	ATOM_tkhd	ATOM('t','k','h','d')
#define	ATOM_mdia	ATOM('m','d','i','a')
#define	ATOM_minf	ATOM('m','i','n','f')
#define	ATOM_stbl	ATOM('s','t','b','l')
#define	ATOM_stsz	ATOM('s','t','s','z')
#define	ATOM_udta	ATOM('u','d','t','a')
//#define	ATOM_PANA	ATOM('P','A','N','A')
//
typedef	unsigned int	MP4ATOM;
typedef	struct {
	unsigned int	size;
	MP4ATOM			atom;
} ATOM_HEADER;
//
typedef struct {
	OSAL_FILE*		fhdl;
	unsigned int	fpos;	// file position of read
	unsigned int	bpos;
	unsigned int	rpos;	// file position of data in buf
	char*			buf;
	unsigned int	bufsize;
	ATOM_HEADER		ah;
} ACONTEXT;
//
#define	MAKE_ATOM_HEADER(ah, src)	ah.size =  (*(src)     << 24) +	\
											   (*(src + 1) << 16) + \
											   (*(src + 2) <<  8) + \
											   (*(src + 3));  \
									ah.atom =  (*(src + 4)) + \
											   (*(src + 5) <<  8) + \
											   (*(src + 6) << 16) + \
											   (*(src + 7) << 24);
											  
#define	MAKE_INT16(p)				(int)(*(p) << 8) + (int)(*(p + 1))
#define	MAKE_INT32(p)				(int)(*(p) << 24)    + (int)(*(p + 1) << 16) + \
									(int)(*(p + 2) << 8) + (int)(*(p + 3))

ACONTEXT* InitAtomContext(ACONTEXT *atxt, OSAL_FILE *hdl);
void UninitAtomContext(ACONTEXT *atxt);
ATOM_HEADER* Reach_Sibling_Atom(ACONTEXT *atxt, MP4ATOM theAtom);
ATOM_HEADER *Find_AtomString(ACONTEXT *atxt, MP4ATOM *astr);

//
JTAG_HEADER *Find_JPEG_Tag(ACONTEXT *jtxt, JPEGTAG theTag)
{
	while (1) {
		JTAG_HEADER	jh;

		MAKE_JTAG_HEADER(jh, jtxt->buf + (jtxt->bpos - jtxt->rpos));
		if (jh.size == 0) {
			printc(BG_RED("%s %d UNRECOGNIZE FORMAT\n"), __func__, __LINE__);
			return NULL;	// Unrecognize  Format
		}
		if (jh.tag == theTag) {
			memcpy(&jtxt->ah, &jh, sizeof(JTAG_HEADER));
			return (JTAG_HEADER*)&jtxt->ah;
		}
		// next sibling
		jtxt->bpos += (jh.size + sizeof(JPEGTAG));
		if (jtxt->bpos > jtxt->fpos - sizeof(JTAG_HEADER)) {
			unsigned int	rs;
		
			jtxt->rpos = jtxt->bpos & (~(512 - 1));
			if (osal_fseek(jtxt->fhdl, jtxt->rpos, MMP_FS_FILE_BEGIN) != 0) {
				break;
			}
			osal_flush_dcache();
			if ((rs = osal_fread(jtxt->buf, 1, jtxt->bufsize, jtxt->fhdl)) == 0) {
				break;
			}
			//osal_invalidate_dcache(((unsigned int)(jtxt->buf) & ~(MEM_ALIGNMENT_HEAP - 1)) , rs);
			osal_invalidate_dcache(((unsigned int)(jtxt->buf)) , rs);
			jtxt->fpos = jtxt->rpos + rs;
		} else if (jtxt->bpos < jtxt->rpos) {
			printc(BG_RED("%s %d UNRECOGNIZE FORMAT\n"), __func__, __LINE__);
			return NULL;
		}
	}
	return NULL;
}
//
int miGetJPGInfo(OSAL_FILE *hdl, MI_INFO *miInfo)
{
	ACONTEXT	context;
	int		  	ret;

	ret = 0;	
	if (InitAtomContext(&context, hdl) == NULL)
		goto ERR_miGetJPGInfo;
	context.bpos += sizeof(JPEGTAG);	// skip TAG_JPEG
	if (Find_JPEG_Tag(&context, TAG_SOF) == NULL)
		goto ERR_miGetJPGInfo;
	miInfo->width  = MAKE_INT16(context.buf + (context.bpos - context.rpos) + 0x7);
	miInfo->height = MAKE_INT16(context.buf + (context.bpos - context.rpos) + 0x5);
	miInfo->type   = 2; // Still Image
	ret = sizeof(MI_INFO);
ERR_miGetJPGInfo:
	UninitAtomContext(&context);
	return ret;
}

// frame rate = moov.mdia.stbl.stsz.samplecount /
//				(moov.trak.tkhd.duration / moov.mvhd.timescale)
int miGetMP4Info(OSAL_FILE *hdl, MI_INFO *miInfo)
{
	//#define	BUF_MP4SIZE		1024
	//#define	READ_MP4SIZE	1024
	//char			*buf;
	//size_t			rs;
	int				ret;
	//ATOM_HEADER		ah;
	ACONTEXT		context;
	MP4ATOM	mvhdstr[] = {ATOM_moov, ATOM_mvhd, 0};
	MP4ATOM tkhdstr[] = {			ATOM_trak, ATOM_tkhd, 0};
	MP4ATOM stszstr[] = {                      ATOM_mdia, ATOM_minf, ATOM_stbl, ATOM_stsz, 0};
	ret = 0;
	// |<-------------------------->|
	// |<----->r<---b---->f<------->|
	//
	// init context
	if (InitAtomContext(&context, hdl) == NULL)
		goto ERR_miGetMP4Info;
	// moov.mvhd, get timescale, duration
	if (Find_AtomString(&context, mvhdstr) == NULL)
		goto ERR_miGetMP4Info;
	miInfo->timescale = MAKE_INT32(context.buf + (context.bpos - context.rpos) + 0x14);
	miInfo->duration  = MAKE_INT32(context.buf + (context.bpos - context.rpos) + 0x18);
	// moov.trak.tkhd, get width, height
	// *NOTE*: the first trak MUST BE vide
	if (Find_AtomString(&context, tkhdstr) == NULL)
		goto ERR_miGetMP4Info;
	miInfo->width  = MAKE_INT16(context.buf + (context.bpos - context.rpos) + 0x54);
	miInfo->height = MAKE_INT16(context.buf + (context.bpos - context.rpos) + 0x58);
	// moov.trak.mdia.minf.stbl.stsz, get sample counts
	if (Find_AtomString(&context, stszstr) == NULL)
		goto ERR_miGetMP4Info;
	miInfo->samplecount = MAKE_INT32(context.buf + (context.bpos - context.rpos) + 0x10);
	// It's MP4.
	miInfo->type = 1;	//MP4
	ret = sizeof(MI_INFO);

ERR_miGetMP4Info:
	UninitAtomContext(&context);
	return ret;
}
/*
 * Init ATON Context
 */
ACONTEXT* InitAtomContext(ACONTEXT *atxt, OSAL_FILE *hdl)
{
	#define	BUF_MP4SIZE		1024
	//#define	READ_MP4SIZE	1024
	size_t			rs;
	
	// init context
	atxt->rpos = atxt->bpos = atxt->fpos = 0;
	atxt->fhdl = hdl;
	atxt->buf  = osal_malloc(BUF_MP4SIZE);
	atxt->bufsize = BUF_MP4SIZE;
	if (atxt->buf == NULL) {
		return NULL;
	}
	osal_fseek(atxt->fhdl, atxt->rpos, MMP_FS_FILE_BEGIN);
	osal_flush_dcache();
	if ((rs = osal_fread(atxt->buf, 1, atxt->bufsize, atxt->fhdl)) == 0) {
		osal_free(atxt->buf);
		return NULL;
	}
	//osal_invalidate_dcache(((unsigned int)(atxt->buf) & ~(MEM_ALIGNMENT_HEAP - 1)) , rs);
	osal_invalidate_dcache(((unsigned int)(atxt->buf)) , rs);
	atxt->fpos += rs;
	return atxt;
}
/*
 *
 */
void UninitAtomContext(ACONTEXT *atxt)
{
	if (atxt->buf)
		osal_free(atxt->buf);
}
/*
 *
 */
ATOM_HEADER* Reach_Sibling_Atom(ACONTEXT *atxt, MP4ATOM theAtom)
{
	while (1) {
		ATOM_HEADER	ah;

		MAKE_ATOM_HEADER(ah, atxt->buf + (atxt->bpos - atxt->rpos));
		if (ah.size == 0) {
			printc(BG_RED("%s %d UNRECOGNIZE FORMAT\n"), __func__, __LINE__);
			return NULL;	// Unrecognize  Format
		}
		if (ah.atom == theAtom) {
			memcpy(&atxt->ah, &ah, sizeof(ATOM_HEADER));
			return &atxt->ah;
		}
		// next sibling
		atxt->bpos += ah.size;
		if (atxt->bpos > atxt->fpos - sizeof(ATOM_HEADER)) {
			unsigned int	rs;
		
			atxt->rpos = atxt->bpos & (~(512 - 1));
			if (osal_fseek(atxt->fhdl, atxt->rpos, MMP_FS_FILE_BEGIN) != 0) {
				break;
			}
			osal_flush_dcache();
			if ((rs = osal_fread(atxt->buf, 1, atxt->bufsize, atxt->fhdl)) == 0) {
				break;
			}
			//osal_invalidate_dcache(((unsigned int)(atxt->buf) & ~(MEM_ALIGNMENT_HEAP - 1)) , rs);
			osal_invalidate_dcache(((unsigned int)(atxt->buf)) , rs);
			atxt->fpos = atxt->rpos + rs;
		} else if (atxt->bpos < atxt->rpos) {
			printc(BG_RED("%s %d UNRECOGNIZE FORMAT\n"), __func__, __LINE__);
			return NULL;
		}
	}
	return NULL;
}
/*
 *
 */
ATOM_HEADER *Find_AtomString(ACONTEXT *atxt, MP4ATOM *astr)
{
	MP4ATOM *pato;
	
	if (astr == NULL) return NULL;
	pato = astr;
	while (Reach_Sibling_Atom(atxt, *pato) != NULL) {
		pato++;
		if (*pato == NULL)
			return &atxt->ah;
		// to offspring
		atxt->bpos += sizeof(ATOM_HEADER);
	}
	return NULL;
}
/*
 * handle is file handle
 * Get Media File Info
 */
int miGetMediaInfo(char *filename, MI_INFO *miInfo)
{
	OSAL_FILE *hdl;
	char	  *p;
	int		  ret;
	
	ret = 0;
	p = strrchr(filename, '.');
	if (p == NULL)
		return ret;
	hdl = osal_fopen(filename, "rb");
	if (hdl == NULL)
		return ret;
	if (stricmp(p + 1, "JPG") == 0) {
		ret = miGetJPGInfo(hdl, miInfo);
	} else if (stricmp(p + 1, "MP4") == 0 ||
			   stricmp(p + 1, "MOV") == 0) {
		ret = miGetMP4Info(hdl, miInfo);
	}
	osal_fclose(hdl);
	return ret;
}

/*******************************************************************
 * Miscellaneous for H.264 SPS/PPS Generation
 * Because MMP might not be on and the code and coupled with MMPF
 * Port a version here.
 *******************************************************************/
#if 0
void ____SPS_PPS_Generation____() {}
#endif

#define BASELINE_PROFILE (66)
#define MAIN_PROFILE     (77)
#define FREXT_HP         (100)      ///< YUV 4:2:0/8 "High"
#define SUPPORT_POC_TYPE_1 (0)

typedef struct _MI_H264_SPS_INFO {
    MMP_BOOL    Valid; // indicates the parameter set is valid

    MMP_ULONG   profile_idc;                                    // u(8)
    MMP_BOOL    constrained_set0_flag;                          // u(1)
    MMP_BOOL    constrained_set1_flag;                          // u(1)
    MMP_BOOL    constrained_set2_flag;                          // u(1)
    MMP_BOOL    constrained_set3_flag;                          // u(1)
    MMP_BOOL    constrained_set4_flag;                          // u(1)
    MMP_BOOL    constrained_set5_flag;                          // u(1)
    MMP_BOOL    constrained_set6_flag;                          // u(1)
    MMP_ULONG   level_idc;                                      // u(8)
    MMP_ULONG   seq_parameter_set_id;                           // ue(v)
    MMP_ULONG   chroma_format_idc;                              // ue(v)

    MMP_BOOL    seq_scaling_matrix_present_flag;                // u(1) => always 0
        //MMP_LONG   seq_scaling_list_present_flag[12];         // u(1)

    MMP_ULONG   bit_depth_luma_minus8;                          // ue(v)
    MMP_ULONG   bit_depth_chroma_minus8;                        // ue(v)
    MMP_ULONG   log2_max_frame_num_minus4;                      // ue(v)
    MMP_ULONG   pic_order_cnt_type;
    // if( pic_order_cnt_type == 0 )
    MMP_ULONG   log2_max_pic_order_cnt_lsb_minus4;              // ue(v)
    // else if( pic_order_cnt_type == 1 )
    #if (SUPPORT_POC_TYPE_1 == 1)
    MMP_BOOL delta_pic_order_always_zero_flag;                  // u(1)
    MMP_LONG    offset_for_non_ref_pic;                     	// se(v)
    MMP_LONG    offset_for_top_to_bottom_field;             	// se(v)
    MMP_ULONG   num_ref_frames_in_pic_order_cnt_cycle;      	// ue(v)
    // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
    MMP_LONG    offset_for_ref_frame[MAX_REF_FRAME_IN_POC_CYCLE];   // se(v)
    #endif //(SUPPORT_POC_TYPE_1 == 1)
    MMP_ULONG   num_ref_frames;                                 // ue(v)
    MMP_BOOL    gaps_in_frame_num_value_allowed_flag;           // u(1)
    MMP_ULONG   pic_width_in_mbs_minus1;                        // ue(v)
    MMP_ULONG   pic_height_in_map_units_minus1;                 // ue(v)
    MMP_BOOL    frame_mbs_only_flag;                            // u(1)
    // if( !frame_mbs_only_flag )
    MMP_BOOL    mb_adaptive_frame_field_flag;                   // u(1)
    MMP_BOOL    direct_8x8_inference_flag;                      // u(1)
    MMP_BOOL    frame_cropping_flag;                            // u(1)
    MMP_ULONG   frame_cropping_rect_left_offset;                // ue(v)
    MMP_ULONG   frame_cropping_rect_right_offset;               // ue(v)
    MMP_ULONG   frame_cropping_rect_top_offset;                 // ue(v)
    MMP_ULONG   frame_cropping_rect_bottom_offset;              // ue(v)
    MMP_BOOL    vui_parameters_present_flag;                    // u(1)
    #if (SUPPORT_VUI_INFO)
        MMPF_H264ENC_VUI_INFO vui_seq_parameters;
    #endif
} MI_H264_SPS_INFO;
static MI_H264_SPS_INFO m_sps;

typedef struct _MMPF_H264ENC_BS_INFO {
    MMP_LONG byte_pos;          ///< current position in bitstream;
    MMP_LONG bits_to_go;        ///< current bit counter
    MMP_UBYTE byte_buf;         ///< current buffer for last written byte
    MMP_UBYTE *streamBuffer;    ///< actual buffer for written bytes
} MMPF_H264ENC_BS_INFO;

typedef enum _MMPF_H264ENC_NALU_TYPE {
    H264_NALU_TYPE_SLICE    = 1,
    H264_NALU_TYPE_DPA      = 2,
    H264_NALU_TYPE_DPB      = 3,
    H264_NALU_TYPE_DPC      = 4,
    H264_NALU_TYPE_IDR      = 5,
    H264_NALU_TYPE_SEI      = 6,
    H264_NALU_TYPE_SPS      = 7,
    H264_NALU_TYPE_PPS      = 8,
    H264_NALU_TYPE_AUD      = 9,
    H264_NALU_TYPE_EOSEQ    = 10,
    H264_NALU_TYPE_EOSTREAM = 11,
    H264_NALU_TYPE_FILL     = 12,
    H264_NALU_TYPE_SPSEXT   = 13,
    H264_NALU_TYPE_PREFIX   = 14,
    H264_NALU_TYPE_SUBSPS   = 15
} MMPF_H264ENC_NALU_TYPE;

typedef enum _MMPF_H264ENC_NAL_REF_IDC{
    H264_NALU_PRIORITY_HIGHEST     = 3,
    H264_NALU_PRIORITY_HIGH        = 2,
    H264_NALU_PRIORITY_LOW         = 1,
    H264_NALU_PRIORITY_DISPOSABLE  = 0
} MMPF_H264ENC_NAL_REF_IDC;

typedef struct _MMPF_H264ENC_NALU_INFO {
    MMPF_H264ENC_NALU_TYPE nal_unit_type;
    MMPF_H264ENC_NAL_REF_IDC nal_ref_idc;
    MMP_UBYTE   temporal_id;    ///< SVC extension
} MMPF_H264ENC_NALU_INFO;

typedef struct _MMPF_H264ENC_SYNTAX_ELEMENT {
    MMP_LONG    type;           //!< type of syntax element for data part.
    MMP_LONG    value1;         //!< numerical value of syntax element
    MMP_LONG    value2;         //!< for blocked symbols, e.g. run/level
    MMP_LONG    len;            //!< length of code
    MMP_LONG    inf;            //!< info part of UVLC code
    MMP_ULONG   bitpattern;     //!< UVLC bitpattern
    MMP_LONG    context;        //!< CABAC context

  //!< for mapping of syntaxElement to UVLC
  //void    (*mapping)(int value1, int value2, int* len_ptr, int* info_ptr);
} MMPF_H264ENC_SYNTAX_ELEMENT;

#define MAX_PARSET_BUF_SIZE     (256)
#define H264E_STARTCODE_LEN     (3)

static void sodb2rbsp(MMPF_H264ENC_BS_INFO *cur_bs)
{
	// check byte_aligned
#if 0//still generate 0x80 as HW does
	if (cur_bs->bits_to_go == 8)
    	return;
#endif

    // check this: if bits_to_go is 0

    cur_bs->byte_buf    <<= 1;
    cur_bs->byte_buf    |= 1;
    cur_bs->bits_to_go--;
    cur_bs->byte_buf    <<= cur_bs->bits_to_go;
    cur_bs->streamBuffer[cur_bs->byte_pos++] = cur_bs->byte_buf;
    cur_bs->bits_to_go  = 8;
    cur_bs->byte_buf    = 0;
}

static int rbsp2ebsp(unsigned char *ebsp, unsigned char *rbsp, int rbsp_size)
{
    int i, ebsp_len, zero_count;

    ebsp_len = 0;
    zero_count = 0;
    for (i = 0; i < rbsp_size; i++) {
        if(zero_count == 2 && !(rbsp[i] & 0xFC)) {
            ebsp[ebsp_len] = 0x03;
            ebsp_len++;
            zero_count = 0;
        }
        ebsp[ebsp_len] = rbsp[i];

        zero_count = (rbsp[i])? 0: zero_count+1;
        ebsp_len++;
    }
    return ebsp_len;
}

static int rbsp2nalu(unsigned char *nalu_buf, unsigned char *rbsp, int rbsp_size,
                     MMPF_H264ENC_NALU_INFO *nalu_inf)
{
    int i;

#if 0
    // Write AnnexB NALU
    for (i = 0; i < (H264E_STARTCODE_LEN-1); i++) //always make 4 byte startcode
        nalu_buf[i] = 0x00;
    nalu_buf[i++] = 0x01;
#else
    i = 0;
#endif
    nalu_buf[i++] = (unsigned char)((nalu_inf->nal_ref_idc<<5)|nalu_inf->nal_unit_type);
    return i + rbsp2ebsp((nalu_buf+i), rbsp, rbsp_size);
}


static void write_uvlc(MMPF_H264ENC_SYNTAX_ELEMENT *se, MMPF_H264ENC_BS_INFO *currStream)
{
    unsigned int mask = 1 << (se->len - 1);
    int i;

    // Add the new bits to the bitstream.
    // Write out a byte if it is full
    if ( se->len < 33 )
    {
        for (i = 0; i < se->len; i++)
        {
            currStream->byte_buf <<= 1;

            if (se->bitpattern & mask)
                currStream->byte_buf |= 1;

            mask >>= 1;

            if ((--currStream->bits_to_go) == 0)
            {
                currStream->bits_to_go = 8;
                currStream->streamBuffer[currStream->byte_pos++] = currStream->byte_buf;
                currStream->byte_buf = 0;
            }
        }
    }
    else
    {
        // zeros
        for (i = 0; i < (se->len - 32); i++)
        {
            currStream->byte_buf <<= 1;

            if ((--currStream->bits_to_go) == 0)
            {
                currStream->bits_to_go = 8;
                currStream->streamBuffer[currStream->byte_pos++] = currStream->byte_buf;
                currStream->byte_buf = 0;
            }
        }
        // actual info
        mask = ((unsigned int)1) << 31;
        for (i = 0; i < 32; i++)
        {
            currStream->byte_buf <<= 1;

            if (se->bitpattern & mask)
                currStream->byte_buf |= 1;

            mask >>= 1;

            if ((--currStream->bits_to_go) == 0)
            {
                currStream->bits_to_go = 8;
                currStream->streamBuffer[currStream->byte_pos++] = currStream->byte_buf;
                currStream->byte_buf = 0;
            }
        }
    }
}

/** @brief Makes code word and passes it back
A code word has the following format: 0 0 0 ... 1 Xn ...X2 X1 X0.
NOTE this function is called with sym->inf > (1<<(sym->len/2)).  The upper bits of inf are junk
*/
static int symbol2uvlc(MMPF_H264ENC_SYNTAX_ELEMENT *sym)
{
    int suffix_len = sym->len >> 1;
    //assert (suffix_len < 32);
    suffix_len = (1 << suffix_len);
    sym->bitpattern = suffix_len | (sym->inf & (suffix_len - 1));
    return 0;
}

/** @brief mapping for ue(v) syntax elements
*/
static void ue_linfo(int ue, int dummy, int *len, int *info)
{
    int i, nn =(ue+1)>>1;

    for (i=0; i < 33 && nn != 0; i++)
    {
        nn >>= 1;
    }
    *len  = (i << 1) + 1;
    *info = ue + 1 - (1 << i);
}

/** @brief ue_v, writes an ue(v) syntax element, returns the length in bits
*/
static int ue_v (int value, MMPF_H264ENC_BS_INFO *bitstream)
{
    MMPF_H264ENC_SYNTAX_ELEMENT symbol, *sym=&symbol;
    sym->value1 = value;
    sym->value2 = 0;

    ue_linfo(sym->value1,sym->value2,&(sym->len),&(sym->inf));
    symbol2uvlc(sym);

    write_uvlc (sym, bitstream);

    return (sym->len);
}

/** @brief u_1, writes a flag (u(1) syntax element, returns the length in bits, always 1
*/
static int u_1 (int value, MMPF_H264ENC_BS_INFO *bitstream)
{
    MMPF_H264ENC_SYNTAX_ELEMENT symbol, *sym=&symbol;

    sym->bitpattern = value;
    sym->value1 = value;
    sym->len = 1;

    //assert (bitstream->streamBuffer != NULL);
    write_uvlc(sym, bitstream);

    return (sym->len);
}

/** @brief u_v, writes a n bit fixed length syntax element, returns the length in bits,
*/
static int u_v (int n, int value, MMPF_H264ENC_BS_INFO *bitstream)
{
    MMPF_H264ENC_SYNTAX_ELEMENT symbol, *sym=&symbol;

    sym->bitpattern = value;
    sym->value1 = value;
    sym->len = n;

    //assert (bitstream->streamBuffer != NULL);
    write_uvlc(sym, bitstream);

    return (sym->len);
}

static __inline MMP_ULONG idc_level(MI_H264_SPS_INFO* sps)
{
	struct IDC_LEVELS {
		MMP_ULONG MBperSec;
		MMP_USHORT level;
	} IDC_LEVELS[] = {
		{1485, 10},
		{3000, 11},
		{6000, 12},
		{11880, 20},
		{19800, 21},
		{20250, 22},
		{40500, 30},
		{108000, 31},
		{216000, 32},
		{245760, 41}
	};//must from low to high
	int i;
	MMP_ULONG MBperSec = (sps->pic_height_in_map_units_minus1 + 1) *
			             (sps->pic_width_in_mbs_minus1 + 1) * 30;

	for (i = 0; i < sizeof(IDC_LEVELS)/sizeof(IDC_LEVELS[0]); ++i) {
		if (MBperSec < IDC_LEVELS[i].MBperSec) {
			return (MMP_ULONG) IDC_LEVELS[i].level;
		}
	}
	return 0;
}

//idx is currently not used
MI_H264_SPS_INFO* miH264GetSpsInfo(int idx)
{
	return &m_sps;
}

void miH264InitSpsInfo(MI_H264_SPS_INFO* sps)
{
	MEMSET(sps, 0, sizeof(MI_H264_SPS_INFO));
	sps->profile_idc = BASELINE_PROFILE;
	sps->constrained_set0_flag = 0;
	sps->constrained_set1_flag = 0;
	sps->constrained_set2_flag = 0;
	sps->constrained_set3_flag = 0;
	sps->constrained_set4_flag = 0;
	sps->constrained_set5_flag = 0;
	sps->constrained_set6_flag = 0;
	sps->level_idc = 40;
	sps->seq_parameter_set_id = 0;
	sps->log2_max_frame_num_minus4 = 0;
	sps->pic_order_cnt_type = 0;
	sps->log2_max_pic_order_cnt_lsb_minus4 = 0;
	sps->num_ref_frames = 1;
	sps->pic_width_in_mbs_minus1 = 119;
	sps->pic_height_in_map_units_minus1 = 67;
	sps->frame_mbs_only_flag = 1;

	sps->vui_parameters_present_flag = 0;
}

void miH264ModifySpsInfo(MI_H264_SPS_INFO* sps, MMP_USHORT w, MMP_USHORT h)
{
	if ((h & 0x0F) != 0) {
		sps->frame_cropping_flag = 1;
		sps->frame_cropping_rect_bottom_offset = (0x10 - (h & 0x0F)) >> 1;
	}
	if ((w & 0x0F) != 0) {
		sps->frame_cropping_flag = 1;
		sps->frame_cropping_rect_right_offset = (0x10 - (w & 0x0F)) >> 1;
	}
	sps->pic_height_in_map_units_minus1 = ((h + 0xF)>> 4) - 1;
	sps->pic_width_in_mbs_minus1 = ((w + 0xF) >> 4) - 1;

	sps->level_idc = idc_level(sps);

	//sps->level_idc = 40;
	//printt("level_idc = %d\r\n", sps->level_idc);
}

//return total generated SPS length
int miH264GenerateSps (nalu_buf_t *nalu_buf, MI_H264_SPS_INFO *sps)
{
    MMPF_H264ENC_BS_INFO *bitstream, rbsp_inf;
    MMPF_H264ENC_NALU_INFO nalu_inf;
    MMP_ULONG   len = 0;
    MMP_UBYTE m_bH264ParsetBuf[MAX_PARSET_BUF_SIZE];
    int ret_len;

    //nalu_inf.buf = nalu_buf; // init nalu buf addr
    bitstream = &rbsp_inf;
    MEMSET0(&rbsp_inf);
    MEMSET0(m_bH264ParsetBuf);
    bitstream->streamBuffer = m_bH264ParsetBuf;
    bitstream->bits_to_go = 8;

    //len+=u_1  (0, bitstream);
    //len+=u_1  (sps->nal_ref_idc, bitstream);

    len+=u_v  (8, sps->profile_idc, bitstream);

    len+=u_1  (sps->constrained_set0_flag, bitstream);
    len+=u_1  (sps->constrained_set1_flag, bitstream);
    len+=u_1  (sps->constrained_set2_flag, bitstream);
    len+=u_1  (sps->constrained_set3_flag, bitstream);
    len+=u_1  (sps->constrained_set4_flag, bitstream);
    len+=u_1  (sps->constrained_set5_flag, bitstream);
    len+=u_1  (sps->constrained_set6_flag, bitstream);
    len+=u_1  (0, bitstream); //reserve_zero_bit
    //len+=u_v  (4, 0, bitstream);

    len+=u_v  (8, sps->level_idc, bitstream);

    len+=ue_v (sps->seq_parameter_set_id, bitstream);

    // Fidelity Range Extensions stuff
    if(sps->profile_idc == FREXT_HP) {
        len+=ue_v (sps->chroma_format_idc, bitstream);
        len+=ue_v (sps->bit_depth_luma_minus8, bitstream);
        len+=ue_v (sps->bit_depth_chroma_minus8, bitstream);
        len+=u_1  (0, bitstream);
        len+=u_1 (sps->seq_scaling_matrix_present_flag, bitstream);
    }

    len+=ue_v (sps->log2_max_frame_num_minus4, bitstream);
    len+=ue_v (sps->pic_order_cnt_type, bitstream);

    switch (sps->pic_order_cnt_type) {
    #if (SUPPORT_POC_TYPE_1 == 1)
    case 1:
        len+=u_1  (sps->delta_pic_order_always_zero_flag, bitstream);
        len+=se_v (sps->offset_for_non_ref_pic, bitstream);
        len+=se_v (sps->offset_for_top_to_bottom_field, bitstream);
        len+=ue_v (sps->num_ref_frames_in_pic_order_cnt_cycle, bitstream);
        for (i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
            len+=se_v (sps->offset_for_ref_frame[i], bitstream);
        break;
    #endif
    case 2:
        break;
    case 0:
    default:
        len+=ue_v (sps->log2_max_pic_order_cnt_lsb_minus4, bitstream);
        break;
    }

    len+=ue_v (sps->num_ref_frames, bitstream);
    len+=u_1  (sps->gaps_in_frame_num_value_allowed_flag, bitstream);
    len+=ue_v (sps->pic_width_in_mbs_minus1, bitstream);
    len+=ue_v (sps->pic_height_in_map_units_minus1, bitstream);
    len+=u_1  (sps->frame_mbs_only_flag, bitstream);
    if (!sps->frame_mbs_only_flag) {
        len+=u_1  (sps->mb_adaptive_frame_field_flag, bitstream);
    }
    len+=u_1  (sps->direct_8x8_inference_flag, bitstream);

    len+=u_1  (sps->frame_cropping_flag, bitstream);
    if (sps->frame_cropping_flag) {
        len+=ue_v (sps->frame_cropping_rect_left_offset, bitstream);
        len+=ue_v (sps->frame_cropping_rect_right_offset, bitstream);
        len+=ue_v (sps->frame_cropping_rect_top_offset, bitstream);
        len+=ue_v (sps->frame_cropping_rect_bottom_offset, bitstream);
    }

    len+=u_1  (sps->vui_parameters_present_flag, bitstream);

    #if (SUPPORT_VUI_INFO)
    if (sps->vui_parameters_present_flag) {
        len += vui_parameter_generate(&(sps->vui_seq_parameters),
                bitstream);
    }
    #endif

    sodb2rbsp(bitstream); //byte aligned
    nalu_inf.nal_ref_idc   = H264_NALU_PRIORITY_HIGHEST;
    nalu_inf.nal_unit_type = H264_NALU_TYPE_SPS;
	ret_len = rbsp2nalu(nalu_buf->buf, bitstream->streamBuffer, bitstream->byte_pos, &nalu_inf);
	if (ret_len > nalu_buf->len) {
		printd(FG_RED("ERROR") " Insufficient NALU buffer size %d. SPS has been generated %d bytes and might have"
				" overwritten it.\r\n", nalu_buf->len, ret_len);
	}
    return ret_len;
}

int miH264GenerateSpsPpsNalu(nalu_buf_t * const sps_buf, nalu_buf_t *sps_pps_buf)
{
	unsigned char* ptr = sps_pps_buf->buf;
	unsigned char START_CODE[] = {0x00, 0x00, 0x00, 0x01};
	#define START_CODE_SIZE sizeof(START_CODE)
	unsigned char PPS[] = {0x68, 0xCE, 0x38, 0x80};
	#define PPS_SIZE sizeof(PPS)

	if (sps_pps_buf->len < sps_buf->len + START_CODE_SIZE * 2 + PPS_SIZE) {
		return -1;
	}

	MEMCPY(ptr, START_CODE, START_CODE_SIZE);
	ptr +=  START_CODE_SIZE;

	MEMCPY(ptr, sps_buf->buf, sps_buf->len);
	ptr += sps_buf->len;

	MEMCPY(ptr, START_CODE, START_CODE_SIZE);
	ptr +=  START_CODE_SIZE;

	MEMCPY(ptr, PPS, PPS_SIZE);
	ptr +=  PPS_SIZE;

	return ptr - sps_pps_buf->buf;
}



//
// Enum DCF File
#include "AHC_General.h"
#include "AHC_UF.h"
#include "fs_api.h"
//
// FILE Enum
//#include "fs_api.h"
typedef struct _FFINST {
	unsigned int	tot;
	unsigned int	idx;
	unsigned int	flen;
	int				fmt;	// AVI: 2, JPEG: 0
	int				rd;		// rd only:1, rw: 0
	int             new_file;    // 1: is a new file and wait for downloading
	void			*data;
	FS_FILETIME		ft;
} FFINST, *PFFINST;
static DCF_INFO _dcf_info;
static void copy_ffattr(PFFINST pffi, DCF_INFO *pinfo);
static FFINST	_ffi;

//int FindNextFile(PFFINST hinst, char *out, int bufsize, int *retsize, FFFILTER_FN* pfilter);

/*
 * check the input file is being written by recorder
 */
static int fileIsRecording(char* filename)
{
	if (AHC_GetAhcSysMode() == AHC_MODE_VIDEO_RECORD) {
		char	*fullname;
		
		fullname = (char*)AHC_VIDEO_GetCurRecFileName(AHC_TRUE);
		printc("Current Recording file %s\r\n", fullname);
		return 0;
	}
	return 0;
}
/*
 * Currently Support DCF only, \DCIM\*
 */
PFFINST FindFirstFile(char *inout, int bufsize, int *retsize, FFFILTER_FN* pfilter)
{
//	AHC_UF_SetFreeChar(0, DCF_SET_ALLOWED, (UINT8 *)"MP4,MOV,JPG,AVI");
//#if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_NORMAL)
	// nothing!!
//#else
//	AHC_UF_SelectDB(DCF_DB_TYPE_1ST_DB);
//#endif
	if (pfilter == NULL)
		return NULL;

#if USE_THREAD_SAFE_DCF_API
	if (!AHC_UF_GetTotalFileCountByDB(pfilter->arg.db, &_ffi.tot))
#else
	if (!AHC_UF_GetAllDBTotalFileCount(&_ffi.tot))
#endif
	{
		return NULL;
	}
	
	if(_ffi.tot <= 0)
	{
	    printc("%s,DB=%d,count=%d!!!\r\n",__func__,pfilter->arg.db,_ffi.tot);
	    return NULL;
	}
	_ffi.idx = -1;
	_ffi.data = (void*)&_dcf_info;
	if (FindNextFile(&_ffi, inout, bufsize, retsize, pfilter) == 0)
		return NULL;
	return &_ffi;
}

int FindNextFile(PFFINST hinst, char *out, int bufsize, int *retsize, FFFILTER_FN* pfilter)
{
	unsigned char	type;
	int				len;
	PFFINST			pffi;
	int				idx;
	DCF_INFO        *pInfo;
#if USE_THREAD_SAFE_DCF_API
	AHC_BOOL ret = AHC_FALSE;
	UINT8 op;
#endif
 	
	len = *retsize = 0;
	if (hinst == NULL || (pfilter == NULL)) {
		return 0;
	}
	pffi = (PFFINST)(hinst);
	if (pffi->idx < 0) {
		printc("Error idx!!!!\r\n");
		return 0;
	}
	idx  = pffi->idx;
	pffi->idx++;

__fnnext:
#if USE_THREAD_SAFE_DCF_API
	if (pfilter->arg.order == FF_BACKWARD) {
		if (pffi->idx == 0) {
			op = DCF_NODE_VISIT_NODETAIL;
		} else {
			op = DCF_NODE_VISIT_REWIND;
		}
	} else {
		if (pffi->idx == 0) {
			op = DCF_NODE_VISIT_NODEHEAD;
		} else {
			op = DCF_NODE_VISIT_FORWARD;
		}
	}

	if (pffi->data != NULL) {
		pInfo = (DCF_INFO*) pffi->data;
		ret = AHC_UF_GetFileInfoByVisitNode(pfilter->arg.db, pfilter->arg.cam, pInfo, op);
	}
	if (ret == AHC_FALSE) {
		pffi->idx = idx;
		return 0;
	}
#else
#if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_NORMAL)
	if (!AHC_UF_GetAllDBInfobyIndex(pffi->idx, &info)) {
		pffi->idx = idx;
		return 0;
	}
#else
{
 	AHC_RTC_TIME	ftime;
	if (AHC_UF_GetFilePathNamebyIndex(pffi->idx, info.FileName) == AHC_FALSE) {
		pffi->idx = idx;
		return 0;
	}

#if AHC_SHAREENC_SUPPORT //dual encode case
	if (AHC_FALSE ==  AHC_UF_GetFileSizebyFullName(info.FileName, &info.uiFileSize)) {
		printc("Unable to get file size\r\n");
		return 0;
	}
#else //fading code
	AHC_UF_GetFileSizebyIndex(pffi->idx, &info.uiFileSize);
#endif

	AHC_UF_GetFileTypebyIndex(pffi->idx, &info.FileType);
	AHC_UF_IsReadOnlybyIndex(pffi->idx, &info.bReadOnly);
	AHC_UF_GetFileTimebyIndex(pffi->idx, &ftime);
	info.uwYear = ftime.uwYear;
	info.uwMonth= ftime.uwMonth;
	info.uwDay  = ftime.uwDay;
	info.uwHour = ftime.uwHour;
	info.uwMinute=ftime.uwMinute;
	info.uwSecond=ftime.uwSecond;
}
#endif
#endif
	strncpy(out, pInfo->FilePath, bufsize);
	type = pInfo->FileType;
	if (pfilter) {
		if (pfilter->ffn) {
			if ((len = (pfilter->ffn)(hinst, out, (void*)type, &(pfilter->arg))) == 0) {
				pffi->idx++;
				goto __fnnext;
			}
		}
		if (pfilter->arg.new_file != FF_ATTR_ANY && pInfo->bIsFileDownload != pfilter->arg.new_file) {
			pffi->idx++;
			goto __fnnext;
		}
	}
	len = strlen(out);
	// don't send writing file to remote to be deleted or downloaded
	if (fileIsRecording(out)) {
		pffi->idx++;
		goto __fnnext;
	}
	copy_ffattr(pffi, pInfo);
	*retsize = (len > bufsize - 1)? bufsize : len;
	return *retsize;
}

int FindFileAttr(PFFINST hinst, char *in, unsigned int* size, char** fmt, int *rd, FS_FILETIME *ft, int *new_file)
{
	PFFINST			pffi;

	if (!hinst)
		return 0;
	pffi  = (PFFINST)(hinst);
	*size = pffi->flen;
	switch (pffi->fmt) {
	case 0:
		*fmt = "jpeg";
		break;
	case 2:
		*fmt = "AVI";
		break;
	case 3:
		*fmt = "MOV";
		break;
	case 4:
		*fmt = "MP4";
		break;
	default:
		*fmt = "BAD";
	}
	*rd   = pffi->rd;
	*ft   = pffi->ft;
	*new_file  = pffi->new_file;
	return 1;
}

int FindFileGroup(PFFINST hinst, FILE_GRPINFO *pinf)
{
	PFFINST			pffi;
	DCF_INFO		*pif;

	if (!hinst) {
		return 0;
	}
	pffi = (PFFINST)(hinst);
	/*
	if (AHC_UF_GetDirFileKeybyIndex(pffi->idx, &pinf->dkey, &pinf->fkey) != AHC_TRUE) {
		printc("ERROR:%s %d\n", __func__, __LINE__);
		return 0;
	}
	*/
	pif = (DCF_INFO*)pffi->data;
	pinf->dkey = pif->uwDirKey;
	pinf->fkey = pif->uwFileKey;
	#if (defined(DCF_GROUP_FUN) && DCF_GROUP_FUN != 0)
	if (AHC_UF_GetCurGrp(pinf->dkey, pinf->fkey, &pinf->grpid, &pinf->grpmo) != AHC_TRUE) {
		printc("ERROR:%s %d\n", __func__, __LINE__);
		return 0;
	}
	if (AHC_UF_GetGrpCurFileNum(pinf->dkey, pinf->fkey, pinf->grpid, &pinf->grpno) != AHC_TRUE) {
		printc("ERROR:%s %d\n", __func__, __LINE__);
		return 0;
	}
	if (AHC_UF_GetGrpTotalFileNum(pinf->dkey, pinf->grpid, &pinf->grpto) != AHC_TRUE) {
		printc("ERROR:%s %d\n", __func__, __LINE__);
		return 0;
	}
	#else
	//TBD
	#endif
	return 1;
}

int FindFileGetPosition(PFFINST hinst)
{
	PFFINST			pffi;

	if (!hinst)
		return 0;
	pffi = (PFFINST)(hinst);
	return pffi->idx;
}

/* FIXME To be reviewed. The target is to remove this define in case (1).
 * Use a temporary PSDATETIMEDCFDB structure here to implement backward from parameter.
 * It does not use DB lock and thus it is not tread-safe.
 * This function cooperates with the entire DIR command and might take up to seconds(?)
 * so it would be difficult to decide how to lock the DB.
 * Also it should use AHC_UF functions instead but which are not available now.*/
#define CR_DIR_BACKWARD_FROM (1)

/* FIXME To be reviewed. The target is to remove this define in case (1).
 * Also it should use AHC_UF functions instead but which are not available now.*/
#define CR_DIR_THREAD_SAFE_FROM (1)

PSDATETIMEFILE AIHC_DCFDT_GetParentNodeByIndex(PSDATETIMEDCFDB pDB, UINT uiIndex );
void *FindFileSetPosition(PFFINST hinst, int pos, FFFILTER_FN* pFilter)
{
	PFFINST			pffi;

	if (!hinst)
		return 0;
	pffi = (PFFINST)(hinst);
	if (pFilter == NULL)
		return NULL;
#if USE_THREAD_SAFE_DCF_API
	{
		PSDATETIMEDCFDB pDb;
		PSDATETIMEFILE node = NULL;
		#if (DCF_FILE_NAME_TYPE == DCF_FILE_NAME_TYPE_DATE_TIME)
		pDb = AIHC_DCFDT_GetDbByType(pFilter->arg.db);
		if (pDb) {
		#if CR_DIR_BACKWARD_FROM
			if (pFilter->arg.order == FF_BACKWARD) {
				UINT32 cnt;
				cnt = pDb->ulTotalObjectNumByCam[pFilter->arg.cam];
				pos = cnt - pos - 1;
				osal_dprint(WARN, "Backward listing is not thread-safe. (to IDX %d)", pos);
			}
		#endif
		#if CR_DIR_THREAD_SAFE_FROM
			node = AIHC_DCFDT_GetParentNodeByIndex(pDb, pos);
		#endif
		}
		if (node) {
			pDb->psCurrParentNode = node;
		}
		#else
			#error Not supported
		#endif
	}
#endif
	pffi->idx = pos;
	return hinst;
}

static void copy_ffattr(PFFINST pffi, DCF_INFO *pinfo)
{
	pffi->flen = pinfo->uiFileSize;
	pffi->rd = (int)(pinfo->bReadOnly == AHC_TRUE)? 1 : 0;	// ??
	pffi->fmt  = pinfo->FileType;
	pffi->ft.Year     =  pinfo->uwYear;
	pffi->ft.Month    =  pinfo->uwMonth;
	pffi->ft.Day      =  pinfo->uwDay;
	pffi->ft.Hour     =  pinfo->uwHour;
	pffi->ft.Minute   =  pinfo->uwMinute;
	pffi->ft.Second   =  pinfo->uwSecond;
	pffi->new_file = pinfo->bIsFileDownload;
}

/*
*Just for old DCF rule
*if we want to delete rear file,
*we must transfer the rear path to the front path
*Because DCF will delete relvant rear and front files
*/
int GetFrontFilename(char* front)
{
	char *p,*n;
	
	if(0 == (p = strrchr(front, '.')))
		return 0;
	p--;
	if (!strncmp(p,  "R", 1)) {
	
		if( 0 == (n = strrchr(front, '\\')))
			return 0;
		n--;
		if (!strncmp(n,  "R", 1)) {
			memcpy(n,"F",1);
			memcpy(p,"F",1);
			return 1;
		}
	}
	return 0;

}

int wildstrcmp(char *s1, const char* ws2, int ws2len)
{
	if (*(ws2 + ws2len - 1) == '.') {
		return strncmp(s1, ws2, ws2len);
	}
	return strcmp(s1, ws2);
}

#define STATION_SERVICE_PORT 49132//43
#define STATION_SERVICE_VERSION	0x0f0a //2015/10

extern struct netif main_netdev;
static int 	gSTAser_fd = -1;
static struct ammo_evmsg * gpSTAev = NULL;

//#define DBG_STA_SERV
static int station_handle_request(char * src,int revlen,char *pack)
{
	
	short reqtype; 
	char *prestype,*presdata;
	int reslen;
	
	if(revlen < 4) 
		return 0;
	
	if(*src != (STATION_SERVICE_VERSION >> 8) || *(src+1) != (STATION_SERVICE_VERSION & 0xff))
		return 0;
	
	#ifdef DBG_STA_SERV	
	{
		int i ;
		for(i=0 ; i < revlen ; i++){
			printc(FG_BLUE("0x%x "),*(src+i));
		}
		printc(FG_BLUE("\n"));
	}
	#endif
	
	reslen 		= 0;
	reqtype 	= ntohs(*(uint16*)(src + 2));
	prestype 	= pack + 2;
	presdata 	= pack + 4;
	
	printc("%s : reqtype=%x\n",__func__,reqtype);
	
	switch(reqtype)
	{
		case 0x0001:
		{//request ip
			*(uint16*)prestype = htons(0x0101);
			*(uint32*)presdata = htonl(main_netdev.ip_addr.addr);
			printc("ip = %x \n",main_netdev.ip_addr.addr);
			reslen = 8;
		}
		break;
		
		default:
		return 0;
	}
	
	*pack = STATION_SERVICE_VERSION >> 8;
	*(pack+1) = STATION_SERVICE_VERSION & 0xff;
	
	#ifdef DBG_STA_SERV	
	{
		int i ;
		for(i=0 ; i < reslen ; i++){
			printc(FG_RED("0x%x "),*(pack+i));
		}
		printc(FG_RED("\n"));
	}
	#endif
	
	return reslen;
	
}

#define DISCOVER_STR "DISCOVER.CAMERA.IP"
static void station_service_requset(struct ammo_evmsg *m, void *d)
{
	socklen_t fromlen;
	int rev_len;
	struct sockaddr_in sockaddr;
	char rec_buf[128];
	
	fromlen = sizeof(struct sockaddr_in);
	if((rev_len = lwip_recvfrom(gSTAser_fd, rec_buf, sizeof(rec_buf), 0 , (struct sockaddr *)&sockaddr, &fromlen)) > 0)
	{
		int sendlen;
		//printc("UDP receive data =%s,len=%d\n",rec_buf,rev_len);

		if (!strncasecmp(rec_buf,DISCOVER_STR,rev_len)) {
				ncgi_notify_uichange(NCGI_DEF_REASON);
		}

		if((sendlen = station_handle_request(rec_buf,rev_len,rec_buf)) != 0){
			int client_fd;	
			struct sockaddr_in bind_sockaddr;		
			int cnt;
			
			if((client_fd = lwip_socket( PF_INET, SOCK_DGRAM, 0 )) < 0)
				return;

			bind_sockaddr.sin_family      = AF_INET;
			bind_sockaddr.sin_addr.s_addr = 0;
			bind_sockaddr.sin_port        = 0;

			if( lwip_bind( client_fd, (struct sockaddr *)&bind_sockaddr, sizeof( sockaddr ) ) < 0 ) {
				osal_dprint(ERR, "fail to bind socket: %s",__func__);
				lwip_close( client_fd );
			}
			
			#ifdef DBG_STA_SERV	
			printc("rec ip = %x port = %d\n",sockaddr.sin_addr.s_addr,sockaddr.sin_port);
			#endif

			cnt = lwip_sendto(client_fd, rec_buf, sendlen, 0, (struct sockaddr*)&sockaddr, fromlen);
			lwip_close( client_fd );
		}
	}
}

/*
* Only for station Mode, listen to UDP port AIT_SERVICE_PORT
* to service request ,such as ip request ...
*/
void start_station_service(void)
{
	struct sockaddr_in sockaddr;
	
	if(gSTAser_fd != -1){
		printc("%s : socket has exsited\n",__func__);
		return; 
	}
	
	if((gSTAser_fd = lwip_socket( PF_INET, SOCK_DGRAM, 0 )) < 0){ 
	    return; 
	}	
	
	sockaddr.sin_family      = AF_INET;
	sockaddr.sin_addr.s_addr = IPADDR_ANY;//INADDR_BROADCAST;
	sockaddr.sin_port        = htons(STATION_SERVICE_PORT);

	if( lwip_bind( gSTAser_fd, (struct sockaddr *)&sockaddr, sizeof( sockaddr ) ) < 0 ) {
		osal_dprint(ERR, "fail to bind socket: %s", osal_strerror() );
		lwip_close( gSTAser_fd );
		return ;
	}
	
	//lwip_fcntl( gSTAser_fd, F_SETFL, O_NONBLOCK );
	
	gpSTAev = evmsg_new_FD( gSTAser_fd, 0, 0, station_service_requset, 0 );
}

void stop_station_service(void)
{
	
	if(gpSTAev){
		evmsg_remove(gpSTAev);
		gpSTAev = 0;
	}
	
	if(gSTAser_fd != -1){
		lwip_close(gSTAser_fd);
		gSTAser_fd = -1;
	}
	
}

/**
 *
 * @param[in,out] from Input string.
 * @param[out] to Output string. If this string pointer is NULL, then change `from` string.
 */
void ns_convert_fn_fs_to_web(char* from, char* to)
{
	size_t len;
	size_t i, j;

	len = strlen(from);
	if (to) {
		strncpy(to, from, len + 1);
	} else {
		to = from;
	}

	for (i = 0; i < len; ++i) {
		if ((to[i] == ':') && (to[i+1] == '\\')) {
			//dont' use strncpy(). the memory might be overlapped and the result will be undefined.
			for (j = i; j < len + 1; ++j) {
				to[j] = to[j+1];
			}
		}
		if (to[i] == ':' || (to [i] == '\\')) {
			to[i] = '/';
		}
	}
}
