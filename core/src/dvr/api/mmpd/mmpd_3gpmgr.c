/// @ait_only
/**
 @file mmpd_3gpmgr.c
 @brief Retina 3GP Merger Control Driver Function
 @author Will Tseng
 @version 1.0
*/

/** @addtogroup MMPD_3GPMGR
 *  @{
 */

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_lib.h"
#include "lib_retina.h"
#include "mmpd_3gpmgr.h"
#include "mmpd_mp4venc.h"
#include "mmph_hif.h"
#include "mmpf_3gpmgr.h"

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================

static MMP_UBYTE    m_ub3gpMgrStoragePath;      ///< save card mode or memory mode
static MMP_BOOL     m_bAVSyncEncode;            ///< encode with audio
static MMP_ULONG    m_ulTempFileNameBufAddr = 0;///< temp address for file name
static MMP_ULONG    m_ulTempFileNameBufSize = 0;///< size of temp buf for file name

static MMPD_3GPMGR_AUDIO_FORMAT m_3gpAudioFmt;  ///< audio mode

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_PreCapture
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Pre-capture audio/video without saving
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_PreCapture(MMP_USHORT usStreamType, MMP_ULONG ulPreCaptureMs)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usStreamType);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulPreCaptureMs);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_PRECAPTURE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_StartCapture
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Start capture audio/video
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_StartCapture(MMP_ULONG ubEncId, MMP_USHORT usStreamType)
{
    MMP_ERR	status = MMP_ERR_NONE;

    if (usStreamType == VIDENC_STREAMTYPE_VIDRECD) {
        MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubEncId);
        MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usStreamType);
        MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_START);
        status = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    }
    #if (DUALENC_SUPPORT) || (SUPPORT_H264_WIFI_STREAM)
    else if (usStreamType == VIDENC_STREAMTYPE_DUALENC ||
             usStreamType == VIDENC_STREAMTYPE_WIFIFRONT ||
             usStreamType == VIDENC_STREAMTYPE_WIFIREAR) {
        MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubEncId);
        MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usStreamType);
        MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_MULTIENC_START);
        status = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    }
    #endif

	return status;
}

#if (DUALENC_SUPPORT)
//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_StartAllCapture
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Start all encoder audio/video together.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_StartAllCapture(MMP_UBYTE ubTotalEncCnt, MMP_ULONG *pEncID)
{
    MMP_ERR norm_status = MMP_ERR_NONE;
    MMP_ERR dual_status = MMP_ERR_NONE;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubTotalEncCnt);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, pEncID[0]);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, pEncID[1]);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_ALL_START);
    norm_status = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);
    dual_status = MMPH_HIF_GetParameterL(GRP_IDX_VID, 8);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    if (norm_status != MMP_ERR_NONE)
        return norm_status;
    if (dual_status != MMP_ERR_NONE)
        return dual_status;

    return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_StopCapture
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Stop capture audio/video
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_StopCapture(MMP_ULONG ubEncId, MMP_USHORT usStreamType)
{
    #if (DUALENC_SUPPORT) && (SUPPORT_SHARE_REC) 
    if (usStreamType != VIDENC_STREAMTYPE_DUALENC) {
        MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubEncId);
        MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usStreamType);

        MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_STOP);
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    }
    else {
        MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubEncId);
        MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_MULTIENC_STOP);
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    }
    #else
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usStreamType);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_STOP);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    #endif
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_PauseCapture
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Pause capture audio/video
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_PauseCapture(void)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_PAUSE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_ResumeCapture
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Resume capture audio/video
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_ResumeCapture(void)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_RESUME);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetContainerType
//  Description : 
//------------------------------------------------------------------------------
MMP_ERR MMPD_3GPMGR_SetContainerType(VIDMGR_CONTAINER_TYPE type)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, type);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | SET_CONTAINER_TYPE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetRecordSpeed
//  Description : 
//------------------------------------------------------------------------------
MMP_ERR MMPD_3GPMGR_SetRecordSpeed(VIDENC_SPEED_MODE SpeedMode, VIDENC_SPEED_RATIO SpeedRatio)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, SpeedMode);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 1, SpeedRatio);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | ENCODE_SPEED_CTL);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetStoragePath
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Save encoded 3GP file to card mode or memory mode.
 @param[in] ubEnable Enable to card or disable to memory.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetStoragePath(MMP_UBYTE ubSrcMode)
{
    m_ub3gpMgrStoragePath = ubSrcMode;
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, m_ub3gpMgrStoragePath);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | ENCODE_STORAGE_PATH);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetStoragePath
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Check encoded 3GP file if card mode or memory mode.
 @return Card or memory mode.
*/
MMP_UBYTE MMPD_3GPMGR_GetStoragePath(void)
{
    return m_ub3gpMgrStoragePath;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetTempFileNameAddr
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set temp address for filename buffer
 @param[in] addr address
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_GetTempFileNameAddr(MMP_ULONG* pulAddr, MMP_ULONG* pulSize)
{
    *pulAddr = m_ulTempFileNameBufAddr;
    *pulSize = m_ulTempFileNameBufSize;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetTempFileNameAddr
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set temp address for filename buffer
 @param[in] addr address
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetTempFileNameAddr(MMP_ULONG ulAddr, MMP_ULONG ulSize)
{
    m_ulTempFileNameBufAddr = ulAddr;
    m_ulTempFileNameBufSize = ulSize;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetFileName
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Send encoded 3GP file name to firmware for card mode.
 @param[in] bFileName File name.
 @param[in] usLength Length of file name in unit of byte.
 @retval MMP_ERR_NONE Success.
 @note It just use video compressed buffer to store file name. Then it should be set after MMPD_3GPMGR_SetEncodeCompBuf.
*/
MMP_ERR MMPD_3GPMGR_SetFileName(MMP_USHORT usStreamType, MMP_BYTE bFileName[], MMP_USHORT ulFileNameLength)
{
    MMP_UBYTE null = 0;

    if (ulFileNameLength > m_ulTempFileNameBufSize) {
        return MMP_3GPMGR_ERR_UNSUPPORT;
    }
    
    MMPH_HIF_MemCopyHostToDev(m_ulTempFileNameBufAddr, (MMP_UBYTE *)bFileName, ulFileNameLength);
    MMPH_HIF_MemCopyHostToDev(m_ulTempFileNameBufAddr + ulFileNameLength, (MMP_UBYTE*)&null, 1);
    MMPH_HIF_MemCopyHostToDev(m_ulTempFileNameBufAddr + ulFileNameLength + 1, (MMP_UBYTE*)&null, 1);

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, m_ulTempFileNameBufAddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulFileNameLength);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, usStreamType);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | ENCODE_FILE_NAME);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetUserDataAtom
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Send user data atom to firmware for card mode.
 @param[in] UserDataBuf user data atom buffer.
 @param[in] UserDataLength Length of user data atom.
 @param[in] usStreamType file type.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetUserDataAtom(MMP_USHORT usStreamType, MMP_BYTE AtomName[], MMP_BYTE UserDataBuf[], MMP_USHORT UserDataLength)
{
    MMP_UBYTE	null = 0;
    MMP_UBYTE 	*ptr;
    MMP_ULONG   AtomLen;
    MMP_LONG    i;
    
    AtomLen = UserDataLength + 8;
    if (AtomLen > m_ulTempFileNameBufSize) {
        return MMP_3GPMGR_ERR_UNSUPPORT;
    }
    
    ptr = (MMP_UBYTE *)m_ulTempFileNameBufAddr;
    for (i = (4-1); i >= 0; i--) {
        *ptr++ = (MMP_UBYTE)((AtomLen >> (i << 3)) & 0xFF);
    }
    
    MMPH_HIF_MemCopyHostToDev(m_ulTempFileNameBufAddr + 4, (MMP_UBYTE *)AtomName, 4);
    MMPH_HIF_MemCopyHostToDev(m_ulTempFileNameBufAddr + 8, (MMP_UBYTE *)UserDataBuf, UserDataLength);
    
    UserDataLength += 8;
    MMPH_HIF_MemCopyHostToDev(m_ulTempFileNameBufAddr + UserDataLength, (MMP_UBYTE*)&null, 1);
    MMPH_HIF_MemCopyHostToDev(m_ulTempFileNameBufAddr + UserDataLength + 1, (MMP_UBYTE*)&null, 1);

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, m_ulTempFileNameBufAddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, UserDataLength);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, usStreamType);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | SET_USER_DATA_ATOM);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_EnableAVSyncEncode
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set video encode with audio or not.
 @param[in] bEnable Enable with audio or disable without audio.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_EnableAVSyncEncode(MMP_BOOL bEnable)
{
    m_bAVSyncEncode = bEnable;
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, (MMP_UBYTE)m_bAVSyncEncode);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AUDIO_ENCODE_CTL);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetAVSyncEncode
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Get video encode with audio or not.
 @retval Enable with audio or disable without audio.
*/
MMP_BOOL MMPD_3GPMGR_GetAVSyncEncode(void)
{
    return m_bAVSyncEncode;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetAudioParam
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set audio parameter.
 @param[in] param Audio parameter.
 @param[in] AudioMode Audio operation mode.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetAudioParam(  MMP_ULONG                   param,
                                    MMPD_3GPMGR_AUDIO_FORMAT    AudioMode)
{
    MMP_ULONG   ulParam, ret, mode;
    MMP_ERR	    err;

    ulParam         = param;
    m_3gpAudioFmt   = AudioMode;

    switch(AudioMode) {
    case MMPD_3GPMGR_AUDIO_FORMAT_AMR:
        mode = AUDIO_AMR_MODE;
        break;
    case MMPD_3GPMGR_AUDIO_FORMAT_AAC:
        mode = AUDIO_AAC_MODE;
        break;
    case MMPD_3GPMGR_AUDIO_FORMAT_ADPCM:
        mode = AUDIO_ADPCM_MODE;
        break;
    case MMPD_3GPMGR_AUDIO_FORMAT_MP3:
        mode = AUDIO_MP3_MODE;
        break;
    case MMPD_3GPMGR_AUDIO_FORMAT_PCM:
        mode = AUDIO_PCM_MODE;
        break;
    default:
        return MMP_3GPMGR_ERR_PARAMETER;
    }

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulParam);
    err = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | mode);
    ret = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    if (err)
        return err;
    else if (ret)
        return MMP_3GPMGR_ERR_PARAMETER;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetAudioFormat
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Return audio format.
 @retval MMPD_3GPMGR_AUDIO_FORMAT_AAC AAC.
 @retval MMPD_3GPMGR_AUDIO_FORMAT_AMR AMR.
 @retval MMPD_3GPMGR_AUDIO_FORMAT_ADPCM ADPCM.
 @retval MMPD_3GPMGR_AUDIO_FORMAT_MP3 MP3.
 @retval MMPD_3GPMGR_AUDIO_FORMAT_PCM raw PCM.
*/
MMPD_3GPMGR_AUDIO_FORMAT MMPD_3GPMGR_GetAudioFormat(void)
{
    return m_3gpAudioFmt;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetEncodeCompBuf
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set start address and size of firmware compressed buffer.
 @param[in] *BufInfo Pointer of encode buffer structure.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetEncodeCompBuf(VIDENC_STREAMTYPE              usStreamType,
                                     MMPD_3GPMGR_AV_COMPRESS_BUF    *BufInfo)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0,  BufInfo->ulVideoCompBufStart);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4,  BufInfo->ulVideoCompBufEnd -
                                            BufInfo->ulVideoCompBufStart);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8,  BufInfo->ulAudioCompBufStart);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 12, BufInfo->ulAudioCompBufEnd -
                                            BufInfo->ulAudioCompBufStart);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 16, usStreamType);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | COMPRESS_BUF_ADDR);  
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;   
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetEncodeCompBuf
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Get start address and size of firmware compressed buffer.
 @param[in]  usStreamType: file type
 @param[out] *bufaddr : compress buffer address
 @param[out] *bufsize : compress buffer size
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_GetEncodeCompBuf(VIDENC_STREAMTYPE usStreamType, MMP_ULONG *bufaddr, MMP_ULONG *bufsize)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0,  usStreamType);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | GET_COMPBUF_ADDR);  
    *bufaddr = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    *bufsize = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_Set3GPCreateModifyTimeInfo
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set creation time and modification time of 3GP file.
 @param[in] addr address of temp buffer.
 @param[in] size size of temp buffer.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_Set3GPCreateModifyTimeInfo(VIDENC_STREAMTYPE usStreamType, MMP_ULONG CreateTime, MMP_ULONG ModifyTime)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usStreamType);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, CreateTime);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, ModifyTime);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | SET_3GPMUX_TIMEATOM);  
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_ModifyAVIListAtom
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Modify LIST Atom for AVI format.

 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_ModifyAVIListAtom(MMP_BOOL bEnable, MMP_BYTE *pStr)
{
    MMP_ULONG  ultmp;

    ultmp = (MMP_ULONG)*pStr | 
            ((MMP_ULONG)*(pStr+1) << 8)  | 
            ((MMP_ULONG)*(pStr+2) << 16) |
            ((MMP_ULONG)*(pStr+3) << 24);

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, bEnable);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ultmp);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | MODIFY_AVI_LIST_ATOM);  
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetRepackMiscBuf
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set start address and size of firmware AV repack buffer.

 This buffer is for packing video and audio bitstream sequentially.
 @param[in] repackbuf information of AV repack buffer, such as start address/size,
 sync buffer address, buffer address/size for frame size, and buffer address/size for frame time.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetRepackMiscBuf(VIDENC_STREAMTYPE usStreamType, MMPD_3GPMGR_REPACKBUF *pRepackBuf)
{
    MMP_ERR	status;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, pRepackBuf->ulAvRepackStartAddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, pRepackBuf->ulAvRepackSize);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 8, usStreamType);
    
    status = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AV_REPACK_BUF);
    if (status != MMP_ERR_NONE) {
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
        return MMP_SYSTEM_ERR_CMDTIMEOUT;
    }

    if (usStreamType == VIDENC_STREAMTYPE_VIDRECD) {
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, pRepackBuf->ulVideoEncSyncAddr);
        status = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AV_SYNC_BUF_INFO);
    }

    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, pRepackBuf->ulVideoSizeTableAddr);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, pRepackBuf->ulVideoSizeTableSize);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 6, MMPD_3GPMGR_AUX_FRAME_TABLE);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 8, usStreamType);
    status = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AUX_TABLE_ADDR);

    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, pRepackBuf->ulVideoTimeTableAddr);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, pRepackBuf->ulVideoTimeTableSize);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 6, MMPD_3GPMGR_AUX_TIME_TABLE);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 8, usStreamType);
    status = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AUX_TABLE_ADDR);

    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetGOP
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set P frame count of one cycle.
 @param[in] ulFrameCnt Count of P frame.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetGOP(MMP_USHORT usStreamType, MMP_USHORT usPFrame, MMP_USHORT usBFrame)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 0, usPFrame);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 2, usBFrame);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, usStreamType);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | GOP_FRAME_TYPE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_Get3gpFileCurSize
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Get current size of encoded 3GP file.

 This size only counts file header and current AV bitstream.
 @param[out] ulCurSize Current file size.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_Get3gpFileCurSize(MMP_ULONG *ulCurSize)
{
    MMP_ULONG	status;

    *ulCurSize = 0;
    
    if (m_ub3gpMgrStoragePath) {
        MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
        status = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | GET_3GP_DATA_RECODE);

        if (status == MMP_ERR_NONE) {
            *ulCurSize = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
        }
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_Get3gpFileSize
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Get the final size of encoded 3GP file.

 This size should be counted after finishing 3GP file tail filling.
 @param[out] filesize Final 3GP file size.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_Get3gpFileSize(MMP_ULONG *filesize)
{
	MMP_ULONG ulFileSize = 0;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
	MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | GET_3GP_FILE_SIZE);
    ulFileSize = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

	*filesize = ulFileSize;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetRecordingTime
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Get audio recording time in unit of ms.
 @param[out] ulTime Recording time.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_GetRecordingTime(MMP_USHORT usStreamType, MMP_ULONG *ulTime)
{	
#if 0
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usStreamType);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | GET_RECORDING_TIME);

    *ulTime = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
#else
    MMPF_3GPMGR_GetRecordingTime(usStreamType, ulTime);
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetRecordingDuration
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Get current video duration in unit of ms.
 @param[out] ulTime Recording duration.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_GetRecordingDuration(MMP_USHORT usStreamType, MMP_ULONG *ulTime)
{	
#if 0
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usStreamType);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | GET_RECORD_DURATION);
    *ulTime = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);    
#else
    MMPF_3GPMGR_GetRecordingTimeOffset(ulTime, usStreamType);
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetRecordingOffset
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Get recording time offset in unit of ms.
 @param[out] ulTime Recording time offset.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_GetRecordingOffset(MMP_USHORT usStreamType, MMP_ULONG *ulTime)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usStreamType);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | GET_RECORDING_OFFSET);
    *ulTime = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);    
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetAudioParam
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set the maximum 3GP file size for video encoding.
 @param[in] ulFileMax Maximum file size.
 @param[in] ulFileMin Minimum file size.
 @param[out] ulSpace Available space size for recording.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_GetAudioParam(MMP_UBYTE ubEncIdx, MMP_ULONG *audsamplefre)
{
    MMPF_VIDMGR_GetAudioParam(ubEncIdx, audsamplefre);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetFileLimit
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set the maximum 3GP file size for video encoding.
 @param[in] ulFileMax Maximum file size.
 @param[in] ulFileMin Minimum file size.
 @param[out] ulSpace Available space size for recording.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetFileLimit(MMP_ULONG ulFileMax, MMP_ULONG ulReserved, MMP_ULONG *ulSpace)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulFileMax);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulReserved);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AV_FILE_LIMIT);

    if (m_ub3gpMgrStoragePath) {
        *ulSpace = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);
    }
    else {
        *ulSpace = ulFileMax;
    }

    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetTimeLimit
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set the maximum 3GP file time for video encoding.
 @param[in] ulTimeMax Maximum file time in unit of ms.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetTimeLimit(MMP_ULONG ulTimeMax)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);

    if (ulTimeMax > 0x7fffffff) {
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, 0x7fffffff);
    }
    else {
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulTimeMax);
    }
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AV_TIME_LIMIT);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetTimeDynamicLimit
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Change the maximum 3GP file time for current video encoding.
 @param[in] ulTimeMax Maximum file time.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetTimeDynamicLimit(MMP_ULONG ulTimeMax)
{
    MMP_BOOL bCheck;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    
    if (ulTimeMax > 0x7fffffff) {
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, 0x7fffffff);
    }
    else {
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulTimeMax);
    }
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | AV_TIME_DYNALIMIT);
        
    bCheck = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    if (bCheck == MMP_TRUE)
        return MMP_ERR_NONE;
    else
        return MMP_HIF_ERR_PARAMETER;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetSeamless
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set seamless recording mode enable or not
 @param[in] enable Enable or disable seamless recording.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetSeamless(MMP_BOOL bEnable)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, bEnable);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | SEAMLESS_MODE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetStatus
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Check the firmware merger engine status.
 @param[out] status Firmware merger engine status.
 @retval MMP_ERR_NONE Success.
 @note

 The parameter @a status can not be changed because it sync with the firmware merger engine
 status definitions. It can be
 - 0x0000 MMP_ERR_NONE
 - 0x0001 MMP_FS_ERR_OPEN_FAIL
 - 0x0002 MMP_FS_ERR_TARGET_NOT_FOUND
 - 0x0003 MMP_3GPMGR_ERR_HOST_CANCEL_SAVE
 - 0x0004 MMP_3GPMGR_ERR_MEDIA_FILE_FULL
*/
MMP_ERR MMPD_3GPMGR_GetStatus(MMP_ERR *status, MMP_ULONG *tx_status)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | GET_MERGER_STATUS);

    *status     = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    *tx_status  = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);

    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetRecordTailSpeed
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set video record with high speed to make tail mode.
 @param[in] ubHighSpeedEn Enable high speed mode or not.
 @param[in] ulTailInfoAddress Tail information address.
 @param[in] ulTailInfoSize Tail information size.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetRecordTailSpeed( VIDENC_STREAMTYPE   usStreamType,
                                        MMP_BOOL            ubHighSpeedEn,
                                        MMP_ULONG           ulTailInfoAddress,
                                        MMP_ULONG           ulTailInfoSize)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, ubHighSpeedEn);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulTailInfoAddress);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, ulTailInfoSize);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 12, usStreamType);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_TAILSPEEDMODE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_RegisterCallback
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set callback function pointer for the specified event
 @param[in] event the specified event
 @param[in] Callback the callback function pointer for the specified event
 @retval MMP_ERR_NONE Success.

 @warn The registered callback will not be auto deleted, please register a NULL
       to delete it.
*/
MMP_ERR MMPD_3GPMGR_RegisterCallback(VIDMGR_EVENT event, void *CallBack)
{
    return MMPF_VIDMGR_RegisterCallback(event, CallBack);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetRegisteredCallback
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Get callback function pointer for the specified event
 @param[in] event the specified event
 @param[out] Callback the callback function pointer for the specified event
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_GetRegisteredCallback(VIDMGR_EVENT event, void **CallBack)
{
    return MMPF_VIDMGR_GetRegisteredCallback(event, CallBack);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetSkipCntThreshold
//  Description : 
//------------------------------------------------------------------------------
/** @brief The function specified the threshold of skip frame counts 
           to define what calleda slow media.
@param[in] threshold number of skip frames happen in 1 sec represents a slow media
@retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetSkipCntThreshold(MMP_USHORT threshold)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 0, threshold);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | SKIP_THRESHOLD);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_MakeRoom
//  Description : 
//------------------------------------------------------------------------------
/** @brief The function make extra room from record storage space for other usage
@param[in] ulRequiredSize specify the size to ask recorder system make room for
@retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_MakeRoom(MMP_ULONG ulEncId, MMP_ULONG ulRequiredSize)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulRequiredSize);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | MAKE_EXTRA_ROOM);
    MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetVidRecdSkipModeParas
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Enalbe video record skip mode to set total skip count and continuous skip count.

 @param[in] ulTotalCount  - limitation of total skip count
 @param[in] ulContinCount - limitation of continuous skip count
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_3GPMGR_SetVidRecdSkipModeParas(MMP_ULONG ulTotalCount, MMP_ULONG ulContinCount)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);	
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulTotalCount);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulContinCount);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_SKIPMODE_ENABLE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetH264EnableEncMode
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Enable multi h264 encode and Set type for callback or record .

 @param[in] bEnable  - Enable dual h264 encode
 @param[in] type     - Set type for callback or record
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetH264EnableEncMode(MMP_USHORT usStreamType, MMP_ULONG type)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, type);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usStreamType);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_MULTISTREAM_USEMODE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetMuxer3gpConstantFps
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Enable muxer 3gp constant fps.

 @param[in] bEnable  - Enable muxer 3gp constant fps.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetMuxer3gpConstantFps(MMP_BOOL bEnable)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, bEnable);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_MUXER_3GP_CONSTANT_FPS);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetAVSyncMethod
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set AV-sync method .

 @param[in] usAVSyncMethod  - AV-sync method: 0:VIDMGR_AVSYNC_REF_AUD / 1:VIDMGR_AVSYNC_REF_VID.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetAVSyncMethod(VIDMGR_AVSYNC_METHOD usAVSyncMethod)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usAVSyncMethod);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_AVSYNC_METHOD);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetAVSyncMethod
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Get AV-sync method .

 @param[in] usAVSyncMethod  - AV-sync method: 0:VIDMGR_AVSYNC_REF_AUD / 1:VIDMGR_AVSYNC_REF_VID.
 @retval MMP_ERR_NONE Success.
*/
VIDMGR_AVSYNC_METHOD MMPD_3GPMGR_GetAVSyncMethod(void)
{    
    return MMPF_VIDMGR_GetAVSyncMethod();
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetSEIShutterMode
//  Description : 
//------------------------------------------------------------------------------
MMP_ERR MMPD_3GPMGR_SetSEIShutterMode(MMP_ULONG ulMode)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulMode);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_SEI_MODE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetFrameRate
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set video encode frame rate to container information.

 @param[in] usStreamType  - video file type
 @param[in] timeresol Time resolution.
 @param[in] timeincrement Time increment.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_3GPMGR_SetFrameRate(MMP_USHORT usStreamType, MMP_ULONG timeresol, MMP_ULONG timeincrement)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, usStreamType);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, timeresol);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, timeincrement);
	
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_ENCODE_FRAMERATE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetThumbnailInfo
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set thumb nail buffer address and size for video recording.

 @param[in] uladdr - buffer address.
 @param[in] ulsize - buffer size
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_3GPMGR_SetThumbnailInfo(MMP_ULONG ulAddr, MMP_ULONG ulSize)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulAddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulSize);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_SET_THUMB_INFO);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetThumbnailBuf
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Save thumb nail buffer for video recording.

 @param[in] uladdr - buffer address.
 @param[in] ulsize - buffer size
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_3GPMGR_SetThumbnailBuf(MMP_ULONG ulAddr, MMP_ULONG ulSize)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulAddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulSize);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_SET_THUMB_BUF);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_EnableDualRecd
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Enable emergent video recording.

 Enable emergent video recording.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_3GPMGR_EnableDualRecd(MMP_BOOL bEnabled)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, bEnabled);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_DUAL_ENABLE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

#if 0
void ____Emergent_Record_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_EnableEmergentRecd
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Enable emergent video recording.

 Enable emergent video recording.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_3GPMGR_EnableEmergentRecd(MMP_BOOL bEnabled)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, bEnabled);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_EMERGENABLE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_EnableDualEmergentRecd
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Enable Dual emergent video recording.

 Enable emergent video recording.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_3GPMGR_EnableDualEmergentRecd(MMP_BOOL bEnabled)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, bEnabled);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_DUALEMERG_ENABLE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_IsDualEmergentRecdEnable
//  Description : 
//------------------------------------------------------------------------------
MMP_BOOL MMPD_3GPMGR_IsDualEmergentRecdEnable(void)
{
    #if (DUAL_EMERGRECD_SUPPORT)
    return MMPF_VIDMGR_IsDualEmergentRecdEnable();
    #endif
    
    return MMP_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_StartEmergentRecd
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Start emergent video recording.

 Start to save the 3GP file.
 @param[in] bStopVidRecd: stop normal record, keep emergent record
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_3GPMGR_StartEmergentRecd(MMP_BOOL bStopVidRecd)
{
    MMP_ERR status;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, bStopVidRecd);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_EMERGSTART);
    status = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return status;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_StopEmergentRecd
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Stop emergent video recording.

 Stop to save the 3GP file.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
MMP_ERR MMPD_3GPMGR_StopEmergentRecd(void)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_EMERGSTOP);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetEmergentTimeLimit
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set the maximum 3GP file time for emergent video encoding.
 @param[in] ulTimeMax Maximum file time in unit of ms.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetEmergentTimeLimit(MMP_ULONG ulTimeMax)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);

    if (ulTimeMax > 0x7fffffff) {
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, 0x7fffffff);
    }
    else {
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulTimeMax);
    }
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | EMERGFILE_TIME_LIMIT);

    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetEmergentSizeLimit
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set the maximum 3GP file size for emergent video encoding.
 @param[in] ulSizeMax Maximum file size in unit of byte.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetEmergentSizeLimit(MMP_ULONG ulSizeMax)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);

    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulSizeMax);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | EMERGFILE_SIZE_LIMIT);

    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetEmergPreEncTimeLimit
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set the the pre-encoding time limit.
 @param[in] ulTimeMax Maximum file time.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetEmergPreEncTimeLimit(MMP_ULONG ulTimeMax)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulTimeMax);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | SET_EMERGENT_PREENCTIME);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_GetEmergentRecStatus
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Check the firmware merger engine emergent record status.
 @param[out] enable    If emergent record is enable
 @param[out] recording If emergent record is under recording
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_GetEmergentRecStatus(MMP_BOOL *bEnable, MMP_BOOL *bRecording)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | GET_EMERG_REC_STATUS);

    *bEnable    = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    *bRecording = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);

    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

#if 0
void ____ReFix_Record_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetTempBuf2FixedTailInfo
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set start address and size of temp buffer for fixed-tail-info record.
 @param[in] addr address of temp buffer.
 @param[in] size size of temp buffer.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetTempBuf2FixedTailInfo(MMP_ULONG tempAddr, MMP_ULONG tempSize, MMP_ULONG AVaddr, MMP_ULONG AVsize, MMP_ULONG ReserAddr, MMP_ULONG ReserSize)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, tempAddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, tempSize);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, AVaddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 12, AVsize);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 16, ReserAddr);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 20, ReserSize);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_PARAMETER | SET_REFIXRECD_BUFFER);  
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_SetTime2FlushFSCache
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set max time to flush FS cache buffer to SD.

 @param[in] time Set max time(ms).
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_SetTime2FlushFSCache(MMP_ULONG ulTime)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulTime);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_TIME_FLUSH_FSCACHE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_3GPMGR_CheckFile2Refix
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief check if there is a file need to be refixed.

 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_3GPMGR_CheckFile2Refix(void)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_MERGER_OPERATION | MERGER_REFIX_VIDRECD);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

/// @}

/// @end_ait_only
