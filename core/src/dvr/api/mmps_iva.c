//==============================================================================
//
//  File        : mmps_iva.c
//  Description : Intelligent video analysis function
//  Author      :
//  Revision    : 1.0
//
//==============================================================================

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_lib.h"
#include "ait_utility.h"
#include "mmps_iva.h"
#include "mmpd_system.h"
#include "mmpd_iva.h"
#include "mmpd_sensor.h"
#include "lib_retina.h"
#if (CPU_ID == CPU_A)
#include "mdtc_cfg.h"
#include "ldws_cfg.h"
#include "mmpf_vmd.h"
#endif
//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

#if (SUPPORT_MDTC)
/** @brief The video motion detection mode.

   Use @ref MMPS_Sensor_IsVMDStarted to get it,
   and use @ref MMPS_Sensor_StartVMD to set it.
*/
static MMP_BOOL                 m_bCurVMDStarted[MAX_ALGO_PIPE_NUM] = {MMP_FALSE};

static MMP_BOOL                 m_bVMDEnable[MAX_ALGO_PIPE_NUM] = {MMP_FALSE};

/** @brief The pipe to output luma for motion detection.

   Use @ref MMPS_Sensor_SetVMDPipe to set it,
*/
static MMP_IBC_PIPEID           m_VMDPipe[MAX_ALGO_PIPE_NUM] = {MMP_IBC_PIPE_MAX};
#endif

#if (SUPPORT_ADAS)
/** @brief The ADAS mode.

   Use @ref MMPS_Sensor_IsADASStarted to get it,
   and use @ref MMPS_Sensor_StartADAS to set it.
*/
static MMP_BOOL                 m_bCurADASStarted[MAX_ALGO_PIPE_NUM] = {MMP_FALSE};

static MMP_BOOL                 m_bCurADASState[MAX_ALGO_PIPE_NUM][MMPS_ADAS_FEATURE_NUM] = {0};
static MMP_BOOL                 m_bADASEnable[MAX_ALGO_PIPE_NUM] = {MMP_FALSE};
#endif

extern MMP_BOOL                 gubMmpDbgBk;
//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0		 
void _____VMD_Functions_____(){}
#endif


#if (defined(SUPPORT_SPEECH_RECOG)&&(SUPPORT_SPEECH_RECOG))
MMP_ERR MMPDS_InitializeSpeechRecog(MMP_ULONG workbuf, MMP_ULONG workbuf_len, MMP_ULONG freq, MMP_ULONG param1, MMP_UBYTE param2)//, MMP_ULONG GapTimeMs)
{
    MMPD_InitializeSpeechRecog(workbuf, workbuf_len, freq, param1, param2);
    
    return MMP_ERR_NONE;
}

MMP_ERR MMPS_Sensor_StartSpeechRecog(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    MMPD_StartSpeechRecog(ubSnrSel, bEnable);
    
    return MMP_ERR_NONE;
}

MMP_ERR MMPS_StartSpeechRecog(MMP_SpeechRecogCbFunc  CB)
{
    MMPD_SetSpeechCB(CB);
    
    return MMP_ERR_NONE;
}

#endif



#if (SUPPORT_MDTC)
//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_EnableVMD
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will enable/disable the VMD operation.

The function set command to firmware to enable/disable the VMD.
@param[in] bEnable Enable or disable VMD.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_Sensor_EnableVMD(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    //MMP_ERR sRet = MMP_ERR_NONE;

    m_bVMDEnable[ubSnrSel] = bEnable;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_IsVMDEnable
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will check if VMD is enabled.

The function get the current status of VMD.
@return It reports the status of the operation.
*/
MMP_BOOL MMPS_Sensor_IsVMDEnable(MMP_UBYTE ubSnrSel)
{
    return m_bVMDEnable[ubSnrSel];
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_SetVMDPipe
//  Description :
//------------------------------------------------------------------------------
/** @brief the function set which pipe outputs luma for motion detection.

The function should be called before MMPS_Sensor_StartVMD().
@param[in] pipe     The pipe to output Luma frame.
@return None.
*/
void MMPS_Sensor_SetVMDPipe(MMP_UBYTE ubSnrSel, MMP_IBC_PIPEID pipe)
{
	extern MMP_UBYTE gbVMDPipeLinkedSnr[];
    m_VMDPipe[ubSnrSel] = pipe;
    gbVMDPipeLinkedSnr[pipe] = ubSnrSel;
}


MMP_ERR MMPS_Sensor_InitializeVMD(MMP_UBYTE ubSnrSel, MDTC_CFG *pMdtcCfg)
{
	return MMPD_Sensor_InitializeVMD(ubSnrSel,  pMdtcCfg->CpuX,  pMdtcCfg->width,  pMdtcCfg->height,  pMdtcCfg->FrameGap);
}
//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_AllocateVMDBuffer
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will allocate buffers for video motion detection.

The function should be called in MMPS_3GPRECD_SetPreviewMemory().
@param[in] ulStartAddr  The start address to allocate buffers.
@param[in] usWidth      The width of VMD input buffer.
@param[in] usHeight     The height of VMD input buffer.
@param[in] bAllocate    Allocate buffers from memory pool.
@return It reports the status of buffer allocation.
*/
MMP_ERR MMPS_Sensor_AllocateVMDBuffer(MMP_UBYTE ubSnrSel, MMP_ULONG *ulStartAddr, MMP_BOOL bAllocate)
{
    #if (SUPPORT_MDTC)
    MMP_ERR     err 		= MMP_ERR_NONE;
    MMP_ULONG   ulCurBufPos = 0, ulBufSize = 0;


    ulCurBufPos = *ulStartAddr;
    ulCurBufPos = ALIGN4K(ulCurBufPos); ///< 4K alignment for dynamic adjustment cache mechanism


	err = MMPD_Sensor_SetVMDBuf(ubSnrSel, ulCurBufPos, &ulBufSize);
	if (err != MMP_ERR_NONE) {
		RTNA_DBG_Str(0, "Set video motion detection buffers failed\r\n");
		return err;
	}

    ulCurBufPos += ulBufSize;
	
    RTNA_DBG_Str(0,"End of VMD buffer = 0x");
    RTNA_DBG_Long(0, ulCurBufPos);
    RTNA_DBG_Str(0,"\r\n");
	
    *ulStartAddr = ulCurBufPos;

    return MMP_ERR_NONE;
    #else
    return MMP_SENSOR_ERR_VMD;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_RegisterVMDCallback
//  Description :
//------------------------------------------------------------------------------
/** @brief the function registers the callback function to inform motion detected.

The function registers the callback which will be called when motion detected.
@param[in] Callback The Callback function
@return It reports the status of the operation.
*/
MMP_ERR MMPS_Sensor_RegisterCallback(MMP_UBYTE ubSnrSel, MMPS_IVA_EVENT event, void *callback)
{
    MMPD_SENSOR_EVENT e;

    switch(event) {
    case MMPS_IVA_EVENT_MDTC:
        #if (SUPPORT_MDTC)
        return MMPD_Sensor_RegisterVMDCallback(ubSnrSel, callback);
        #endif
    case MMPS_IVA_EVENT_TV_SRC_TYPE:
        e = MMPD_SENSOR_EVENT_TV_SRC_TYPE;
        break;
    default:
        return MMP_SENSOR_ERR_NOT_SUPPORT;
    }

    return MMPD_Sensor_RegisterCallback(e, callback);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_GetVMDResolution
//  Description :
//------------------------------------------------------------------------------
/** @brief the function gets the configured resolution of motion detection frame.

The function gets the configured resolution of motion detection frame.
@param[out] width   The width of motion detection source frame
@param[out] height  The height of motion detection source frame
@return It reports the status of the operation.
*/
MMP_ERR MMPS_Sensor_GetVMDResolution(MMP_UBYTE ubSnrSel, MMP_ULONG *ulWidth, MMP_ULONG *ulHeight)
{
    #if (SUPPORT_MDTC)
    return MMPD_Sensor_GetVMDResolution(ubSnrSel, ulWidth, ulHeight);
    #else
    return MMP_SENSOR_ERR_VMD;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_SetVMDEnable
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will enable/disable the video motion detection.

The function set command to firmware to enable/disable the motion detection.
@param[in] bStart Enable or disable motion detection.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_Sensor_StartVMD(MMP_UBYTE ubSnrSel, MMP_BOOL bStart)
{
    #if (SUPPORT_MDTC)
    if (bStart != m_bCurVMDStarted[ubSnrSel]) {

        MMPD_Sensor_StartVMD(ubSnrSel, bStart);

        if (bStart)
        {
            MMPD_Fctl_LinkPipeToMdtc(m_VMDPipe[ubSnrSel]);
        }
        else
        {
            MMPD_Fctl_UnLinkPipeToMdtc(m_VMDPipe[ubSnrSel]);
        }
        m_bCurVMDStarted[ubSnrSel] = bStart;
    }
    return MMP_ERR_NONE;
    #else
    return MMP_SENSOR_ERR_VMD;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_IsVMDStarted
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will check if motion detection is enabled.

The function get the current status of motion detection.
@return It reports the status of the operation.
*/
MMP_BOOL MMPS_Sensor_IsVMDStarted(MMP_UBYTE ubSnrSel)
{
    #if (SUPPORT_MDTC)
    return m_bCurVMDStarted[ubSnrSel];
    #else
    return MMP_FALSE;
    #endif
}
#endif

#if 0
void _____ADAS_Functions_____(){}
#endif

#if (SUPPORT_ADAS)
//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_EnableADAS
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will enable/disable the ADAS operation.

The function set command to firmware to enable/disable the ADAS.
Note: Use VMD flow/pipe for ADAS
@param[in] bEnable Enable or disable ADAS.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_Sensor_EnableADAS(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    //MMP_ERR sRet = MMP_ERR_NONE;

    m_bADASEnable[ubSnrSel] = bEnable;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_IsADASEnable
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will check if ADAS is enabled.

The function get the current status of ADAS.
@return It reports the status of the operation.
*/
MMP_BOOL MMPS_Sensor_IsADASEnable(MMP_UBYTE ubSnrSel)
{
    return m_bADASEnable[ubSnrSel];
}

MMP_ERR MMPS_Sensor_InitializeADAS(MMP_UBYTE ubSnrSel, LDWS_CFG *pLdwsCfg)
{
	return MMPD_Sensor_InitializeADAS(ubSnrSel, pLdwsCfg[ubSnrSel].CpuX, pLdwsCfg[ubSnrSel].width, pLdwsCfg[ubSnrSel].height, pLdwsCfg[ubSnrSel].FrameGap);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_AllocateADASBuffer
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will allocate buffers for video ADAS.

The function should be called in MMPS_3GPRECD_SetPreviewMemory().
@param[in] ulStartAddr  The start address to allocate buffers.
@param[in] bAllocate    Allocate buffers from memory pool.
@param[in] ulDMABufSize The dma destination buffer size of ADAS.
@return It reports the status of buffer allocation.
*/
MMP_ERR MMPS_Sensor_AllocateADASBuffer(MMP_UBYTE ubSnrSel, MMP_ULONG *ulStartAddr, MMP_BOOL bAllocate, MMP_ULONG ulDMABufSize)
{
    MMP_ERR     err 		= MMP_ERR_NONE;
    MMP_ULONG   ulCurBufPos = 0, ulBufSize = 0;
    MMP_ULONG   ulDMAAddr = 0;


    ulCurBufPos = *ulStartAddr;
    ulCurBufPos = ALIGN4K(ulCurBufPos); ///< 4K alignment for dynamic adjustment cache mechanism
    // Allocate DMA buffer
    ulDMAAddr = ulCurBufPos;
    ulCurBufPos += ulDMABufSize;
    ulCurBufPos = ALIGN4K(ulCurBufPos); ///< 4K alignment for dynamic adjustment cache mechanism

    if (err != MMP_ERR_NONE) {
        PRINTF("Initialize video ADAS config failed\r\n");
        return err;
    }

    err = MMPD_Sensor_SetADASFeature(ubSnrSel, m_bCurADASState[ubSnrSel][MMPS_ADAS_LDWS],
                                     m_bCurADASState[ubSnrSel][MMPS_ADAS_FCWS],
                                     m_bCurADASState[ubSnrSel][MMPS_ADAS_SAG]);
    DBG_AutoTestPrint(ATEST_ACT_REC_0x0001, 
                        ATEST_STS_ADAS_0x0009,  
                        m_bCurADASState[ubSnrSel][MMPS_ADAS_SAG],  
                        m_bCurADASState[ubSnrSel][MMPS_ADAS_FCWS]<<8 + m_bCurADASState[ubSnrSel][MMPS_ADAS_LDWS], 
                        gubMmpDbgBk);

    if (err != MMP_ERR_NONE) {
        PRINTF("Set video ADAS features failed\r\n");
        return err;
    }

    err = MMPD_Sensor_SetADASBuf(ubSnrSel, ulCurBufPos, &ulBufSize, ulDMAAddr);

    if (err != MMP_ERR_NONE) {
        PRINTF("Set video ADAS buffers failed\r\n");
        return err;
    }

    ulCurBufPos += ulBufSize;
    PRINTF("End of ADAS buffer = 0x%X\r\n", ulCurBufPos);
    *ulStartAddr = ulCurBufPos;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_GetADASResolution
//  Description :
//------------------------------------------------------------------------------
/** @brief the function gets the configured resolution of ADAS frame.

The function gets the configured resolution of ADAS frame.
@param[out] width   The width of ADAS source frame
@param[out] height  The height of ADAS source frame
@return It reports the status of the operation.
*/
MMP_ERR MMPS_Sensor_GetADASResolution(MMP_UBYTE ubSnrSel, MMP_ULONG *ulWidth, MMP_ULONG *ulHeight)
{
    return MMPD_Sensor_GetADASResolution(ubSnrSel, ulWidth, ulHeight);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_SetADASFeatureEn
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will enable/disable sub-feature of ADAS.

The function set command to firmware to enable/disable the ADAS.
@param[in] feature Sub-feature of ADAS
@param[in] bEnable Enable or disable ADAS.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_Sensor_SetADASFeatureEn(MMP_UBYTE ubSnrSel, MMPS_IVA_ADAS_FEATURE feature, MMP_BOOL bEnable)
{
    if (feature < MMPS_ADAS_FEATURE_NUM) {
        m_bCurADASState[ubSnrSel][feature] = bEnable;
        return MMP_ERR_NONE;
    }

    return MMP_SENSOR_ERR_PARAMETER;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_GetADASFeatureEn
//  Description :
//------------------------------------------------------------------------------
/** @brief the function gets the current state of ADAS sub-feature.

The function gets the current state of the specified ADAS sub-feature.
@param[in] feature Sub-feature of ADAS
@return It reports the status of the specified ADAS sub-feature.
*/
MMP_BOOL MMPS_Sensor_GetADASFeatureEn(MMP_UBYTE ubSnrSel, MMPS_IVA_ADAS_FEATURE feature)
{
    if (feature < MMPS_ADAS_FEATURE_NUM)
        return m_bCurADASState[ubSnrSel][feature] & m_bCurADASStarted[ubSnrSel];

    return MMP_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_StartADAS
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will start/stop the ADAS operation.

The function set command to firmware to enable/disable the ADAS.
Note: Use VMD flow/pipe for ADAS
@param[in] bStart Enable or disable ADAS.
@return It reports the status of the operation.
*/
MMP_ERR MMPS_Sensor_StartADAS(MMP_UBYTE ubSnrSel, MMP_BOOL bStart)
{
    MMP_ERR sRet = MMP_ERR_NONE;
    
    if (bStart != m_bCurADASStarted[ubSnrSel]) {

        sRet = MMPD_Sensor_StartADAS(ubSnrSel, bStart);
        if(sRet != MMP_ERR_NONE) { MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}
        
        if (bStart){
            sRet = MMPD_Fctl_LinkPipeToMdtc(m_VMDPipe[PRM_SENSOR]);
            if(sRet != MMP_ERR_NONE) { MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}
        }
        else{
            sRet = MMPD_Fctl_UnLinkPipeToMdtc(m_VMDPipe[PRM_SENSOR]);
            if(sRet != MMP_ERR_NONE) { MMP_PRINT_RET_ERROR(0, sRet, "",gubMmpDbgBk); return sRet;}
        }

        m_bCurADASStarted[ubSnrSel] = bStart;
    }
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_IsADASStarted
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will check if ADAS is start/stop.

The function get the current status of ADAS.
@return It reports the status of the operation.
*/
MMP_BOOL MMPS_Sensor_IsADASStarted(MMP_UBYTE ubSnrSel)
{
    return m_bCurADASStarted[ubSnrSel];
}
#endif
