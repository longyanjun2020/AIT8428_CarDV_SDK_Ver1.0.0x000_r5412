/// @ait_only
/**
 @file mmpd_3gpmgr.h
 @brief Header File for the Host 3GP MERGER Driver.
 @author Will Tseng
 @version 1.0
*/

#ifndef _MMPD_3GPMGR_H_
#define _MMPD_3GPMGR_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "includes_fw.h"
#include "mmp_vidrec_inc.h"
#include "mmp_graphics_inc.h"

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

// AV operation mode. This is for recorder only, used in Initialization
typedef enum _MMPD_3GPMGR_AUDIO_FORMAT {
    MMPD_3GPMGR_AUDIO_FORMAT_AAC = 0x00,    ///< Video encode with AAC audio
    MMPD_3GPMGR_AUDIO_FORMAT_AMR,           ///< Video encode with AMR audio
    MMPD_3GPMGR_AUDIO_FORMAT_ADPCM,         ///< Video encode with ADPCM audio
    MMPD_3GPMGR_AUDIO_FORMAT_MP3,           ///< Video encode with MP3 audio
    MMPD_3GPMGR_AUDIO_FORMAT_PCM            ///< Video encode with raw PCM audio
} MMPD_3GPMGR_AUDIO_FORMAT;

// Aux table index
typedef enum _MMPD_3GPMGR_AUX_TABLE {
    MMPD_3GPMGR_AUX_FRAME_TABLE = 0,        ///< Video encode aux frame table
    MMPD_3GPMGR_AUX_TIME_TABLE = 1	        ///< Video encode aux time table
} MMPD_3GPMGR_AUX_TABLE;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct _MMPD_3GPMGR_REPACKBUF {
    MMP_ULONG ulAvRepackStartAddr;   			///< AV repack buffer start address for 3gp encoder (can be calculated)
    MMP_ULONG ulAvRepackSize;     				///< AV repack buffer size for 3gp encoder (can be calculated)
    MMP_ULONG ulVideoEncSyncAddr;   	      	///< Parameter sync buffer for 3gp encoder (can be calculated)
    MMP_ULONG ulVideoSizeTableAddr;		   		///< Video encode frame table buffer start address (can be calculated)
    MMP_ULONG ulVideoSizeTableSize;     		///< Video encode frame table buffer size (can be calculated)
    MMP_ULONG ulVideoTimeTableAddr;   			///< Video encode time table buffer start address (can be calculated)
    MMP_ULONG ulVideoTimeTableSize;    			///< Video encode time table buffer size (can be calculated)
} MMPD_3GPMGR_REPACKBUF;

typedef struct _MMPD_3GPMGR_AV_COMPRESS_BUF {
	MMP_ULONG ulVideoCompBufStart;
	MMP_ULONG ulVideoCompBufEnd;
	MMP_ULONG ulAudioCompBufStart;
	MMP_ULONG ulAudioCompBufEnd;
} MMPD_3GPMGR_AV_COMPRESS_BUF;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

/** @addtogroup MMPD_3GPMGR
 *  @{
 */

MMP_ERR MMPD_3GPMGR_PreCapture(MMP_USHORT usStreamType, MMP_ULONG ulPreCaptureMs);
MMP_ERR MMPD_3GPMGR_StartCapture(MMP_ULONG ubEncId, MMP_USHORT usStreamType);
MMP_ERR MMPD_3GPMGR_StopCapture(MMP_ULONG ubEncId, MMP_USHORT usStreamType);
MMP_ERR MMPD_3GPMGR_PauseCapture(void);
MMP_ERR MMPD_3GPMGR_ResumeCapture(void);
MMP_ERR MMPD_3GPMGR_SetStoragePath(MMP_UBYTE ubEnable);
MMP_ERR MMPD_3GPMGR_SetFileName(MMP_USHORT usStreamType, MMP_BYTE bFileName[], MMP_USHORT usLength);
MMP_ERR MMPD_3GPMGR_SetUserDataAtom(MMP_USHORT usStreamType, MMP_BYTE AtomName[], MMP_BYTE UserDataBuf[], MMP_USHORT UserDataLength);
MMP_ERR MMPD_3GPMGR_EnableAVSyncEncode(MMP_UBYTE ubEnable);
MMP_BOOL MMPD_3GPMGR_GetAVSyncEncode(void);
MMP_ERR MMPD_3GPMGR_GetEncodeCompBuf(VIDENC_STREAMTYPE usStreamType, MMP_ULONG *bufaddr, MMP_ULONG *bufsize);
MMP_ERR MMPD_3GPMGR_SetEncodeCompBuf(VIDENC_STREAMTYPE           usStreamType,
                                     MMPD_3GPMGR_AV_COMPRESS_BUF *BufInfo);
MMP_ERR MMPD_3GPMGR_SetTempBuf2FixedTailInfo(MMP_ULONG tempaddr, MMP_ULONG tempsize, MMP_ULONG AVaddr, MMP_ULONG AVsize, MMP_ULONG Reseraddr, MMP_ULONG Resersize);
MMP_ERR MMPD_3GPMGR_Set3GPCreateModifyTimeInfo(VIDENC_STREAMTYPE usStreamType, MMP_ULONG CreateTime, MMP_ULONG ModifyTime);
MMP_ERR MMPD_3GPMGR_ModifyAVIListAtom(MMP_BOOL bEnable, MMP_BYTE *pStr);
MMP_ERR MMPD_3GPMGR_Get3gpFileCurSize(MMP_ULONG *ulCurSize);
MMP_ERR MMPD_3GPMGR_Get3gpFileSize(MMP_ULONG *filesize);
MMP_ERR MMPD_3GPMGR_SetFileLimit(MMP_ULONG ulFileMax, MMP_ULONG ulReserved, MMP_ULONG *ulSpace);

MMP_ERR MMPD_3GPMGR_SetAudioParam(  MMP_ULONG                   param,
                                    MMPD_3GPMGR_AUDIO_FORMAT    AudioMode);
MMP_ERR MMPD_3GPMGR_GetAudioParam(MMP_UBYTE ubEncIdx, MMP_ULONG *audsamplefre);
MMP_ERR MMPD_3GPMGR_GetRecordingTime(MMP_USHORT usStreamType, MMP_ULONG *ulTime);
MMP_ERR MMPD_3GPMGR_GetRecordingDuration(MMP_USHORT usStreamType, MMP_ULONG *ulTime);
MMP_ERR MMPD_3GPMGR_GetRecordingOffset(MMP_USHORT usStreamType, MMP_ULONG *ulTime);
MMP_ERR MMPD_3GPMGR_SetTimeLimit(MMP_ULONG ulTimeMax);
MMP_ERR MMPD_3GPMGR_SetTimeDynamicLimit(MMP_ULONG ulTimeMax);
MMP_ERR MMPD_3GPMGR_GetStatus(MMP_ERR *status, MMP_ULONG *tx_status);
MMP_UBYTE MMPD_3GPMGR_GetStoragePath(void);
MMPD_3GPMGR_AUDIO_FORMAT MMPD_3GPMGR_GetAudioFormat(void);
MMP_ERR MMPD_3GPMGR_SetGOP(MMP_USHORT usStreamType, MMP_USHORT usPFrame, MMP_USHORT usBFrame);
MMP_ERR MMPD_3GPMGR_SetRepackMiscBuf(VIDENC_STREAMTYPE usStreamType, MMPD_3GPMGR_REPACKBUF *repackbuf);
MMP_ERR MMPD_3GPMGR_SetSeamless(MMP_BOOL enable);
MMP_ERR MMPD_3GPMGR_SetRecordSpeed(VIDENC_SPEED_MODE SpeedMode, VIDENC_SPEED_RATIO SpeedRatio);
MMP_ERR MMPD_3GPMGR_SetRecordTailSpeed( VIDENC_STREAMTYPE       usStreamType,
                                        MMP_BOOL                ubHighSpeedEn,
                                        MMP_ULONG               ulTailInfoAddress,
                                        MMP_ULONG               ulTailInfoSize);
MMP_ERR MMPD_3GPMGR_SetContainerType(VIDMGR_CONTAINER_TYPE type);
MMP_ERR MMPD_3GPMGR_RegisterCallback(VIDMGR_EVENT event, void *CallBack);
MMP_ERR MMPD_3GPMGR_GetRegisteredCallback(VIDMGR_EVENT event, void **CallBack);
MMP_ERR MMPD_3GPMGR_SetSkipCntThreshold(MMP_USHORT threshold);
MMP_ERR MMPD_3GPMGR_MakeRoom(MMP_ULONG ulEncId, MMP_ULONG ulRequiredSize);
MMP_ERR MMPD_3GPMGR_SetFrameRate(MMP_USHORT usStreamType, MMP_ULONG timeresol, MMP_ULONG timeincrement);
MMP_ERR MMPD_3GPMGR_GetEmergentRecStatus(MMP_BOOL *bEnable, MMP_BOOL *bRecording);
MMP_ERR MMPD_3GPMGR_EnableEmergentRecd(MMP_BOOL bEnabled);
MMP_ERR MMPD_3GPMGR_StartEmergentRecd(MMP_BOOL bStopVidRecd);
MMP_ERR MMPD_3GPMGR_StopEmergentRecd(void);
MMP_ERR MMPD_3GPMGR_SetEmergentTimeLimit(MMP_ULONG ulTimeMax);
MMP_ERR MMPD_3GPMGR_SetEmergentSizeLimit(MMP_ULONG ulSizeMax);
MMP_ERR MMPD_3GPMGR_SetEmergPreEncTimeLimit(MMP_ULONG ulTimeMax);
MMP_ERR MMPD_3GPMGR_GetTempFileNameAddr(MMP_ULONG* pulAddr, MMP_ULONG* pulSize);
MMP_ERR MMPD_3GPMGR_SetTempFileNameAddr(MMP_ULONG addr, MMP_ULONG size);
MMP_ERR MMPD_3GPMGR_SetVidRecdSkipModeParas(MMP_ULONG ulTotalCount, MMP_ULONG ulContinCount);
MMP_ERR MMPD_3GPMGR_SetH264EnableEncMode(MMP_USHORT usStreamType, MMP_ULONG type);
MMP_ERR MMPD_3GPMGR_SetMuxer3gpConstantFps(MMP_BOOL bEnable);
MMP_ERR MMPD_3GPMGR_SetAVSyncMethod(VIDMGR_AVSYNC_METHOD usAVSyncMethod);
MMP_ERR MMPD_3GPMGR_SetSEIShutterMode(MMP_ULONG ulMode);
MMP_ERR MMPD_3GPMGR_SetThumbnailInfo(MMP_ULONG ulAddr, MMP_ULONG ulSize);
MMP_ERR MMPD_3GPMGR_SetThumbnailBuf(MMP_ULONG ulAddr, MMP_ULONG ulSize);
VIDMGR_AVSYNC_METHOD MMPD_3GPMGR_GetAVSyncMethod(void);

MMP_ERR MMPD_3GPMGR_EnableDualRecd(MMP_BOOL bEnabled);
MMP_ERR MMPD_3GPMGR_StartAllCapture(MMP_UBYTE ubTotalEncCnt, MMP_ULONG *pEncID);
MMP_ERR MMPD_3GPMGR_EnableDualEmergentRecd(MMP_BOOL bEnabled);
MMP_BOOL MMPD_3GPMGR_IsDualEmergentRecdEnable(void);

// Refix Tail Function
MMP_ERR MMPD_3GPMGR_SetTime2FlushFSCache(MMP_ULONG time);
MMP_ERR MMPD_3GPMGR_CheckFile2Refix(void);

#endif // _MMPD_3GPMGR_H_

/// @end_ait_only
