/// @ait_only
//==============================================================================
//
//  File        : mmpd_iva.c
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
#include "mmpd_system.h"
#include "mmpd_sensor.h"
#include "mmpd_iva.h"
#include "mmph_hif.h"
#include "mmpf_vmd.h"

/** @addtogroup MMPD_IVA
 *  @{
 */

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void _____MDTC_Functions_____(){}
#endif

#if (defined(SUPPORT_SPEECH_RECOG)&&(SUPPORT_SPEECH_RECOG))
/*
m_AUDRECOG.workbuf = (MMP_UBYTE *)ulParameter[0];
m_AUDRECOG.workbuf_len = ulParameter[1];
m_AUDRECOG.freq = ulParameter[2];
m_AUDRECOG.param1 = ulParameter[3];
m_AUDRECOG.param2 = ulParameter[4];
*/
MMP_ERR MMPD_InitializeSpeechRecog(MMP_ULONG workbuf, MMP_ULONG workbuf_len, MMP_ULONG freq, MMP_ULONG param1, MMP_UBYTE param2)//, MMP_ULONG GapTimeMs)
{
    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);

    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, workbuf);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 4, (MMP_ULONG)workbuf_len);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 8, (MMP_ULONG)freq);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 12, (MMP_ULONG)param1);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 16, (MMP_ULONG)param2);
	
    MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_AUDRECOG | INIT_AUDRECOG | AUDRECOG_RUN_ON_CPUB);
    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);
    
    return MMP_ERR_NONE;
}

MMP_ERR MMPD_StartSpeechRecog(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, (MMP_ULONG)bEnable);
    //MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 4, (MMP_ULONG)bEnable);

    MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_AUDRECOG | START_AUDRECOG | AUDRECOG_RUN_ON_CPUB);
 
    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);
    
    return MMP_ERR_NONE;
}

MMP_ERR MMPD_SetSpeechCB(MMP_SpeechRecogCbFunc  CB)
{
	MMPF_Speech_SetCB(CB);
    return MMP_ERR_NONE;
}


#endif

#if (SUPPORT_MDTC)
 extern MMPF_VMD_INSTANCE	m_VMD[];
//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_InitializeVMD
//  Description :
//------------------------------------------------------------------------------
/** @brief Function to initialize VMD

The function initailizes the VMD configuration
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_InitializeVMD(MMP_UBYTE ubSnrSel, MMP_UBYTE RunOnCPUX, MMP_ULONG ResolW, MMP_ULONG ResolH, MMP_UBYTE FrameGap)//, MMP_ULONG GapTimeMs)
{
    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);

    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, ubSnrSel);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 4, (MMP_ULONG)RunOnCPUX);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 8, (MMP_ULONG)ResolW);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 12, (MMP_ULONG)ResolH);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 16, (MMP_ULONG)FrameGap);
	
    if (RunOnCPUX == CPU_B)
    	MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_VMD | INIT_VMD | VMD_RUN_ON_CPUB);
    else
    	MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_VMD | INIT_VMD | VMD_RUN_ON_CPUA);
    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetVMDBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief Set working buffer address for motion detection operation.

The function set video motion detection buffer for operation
@param[in] buf_addr the address of motion detection working buffer
@param[out] buf_size the total size of working buffer
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetVMDBuf(MMP_UBYTE ubSnrSel, MMP_ULONG buf_addr, MMP_ULONG *buf_size)
{
    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);

    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, ubSnrSel);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 4, buf_addr);
//    RTNA_DBG_Str(0, "MMPD_Sensor_SetVMDBuf  ubSnrSel   ");RTNA_DBG_Long(0, ubSnrSel);
//    RTNA_DBG_Str(0, "      buf_addr   ");					RTNA_DBG_Long(0, buf_addr);
//    RTNA_DBG_Str(0, "\r\n");
	
    if (m_VMD[ubSnrSel].RunOnCPUX == CPU_B)
    	MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_VMD | SET_VMD_BUF | VMD_RUN_ON_CPUB);
    else
    	MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_VMD | SET_VMD_BUF | VMD_RUN_ON_CPUA);
    *buf_size = MMPH_HIF_GetParameterL(GRP_IDX_FLOWCTL, 0);

//    RTNA_DBG_Str(0, "MMPD_Sensor_SetVMDBuf  buf_size   ");
//    RTNA_DBG_Long(0, *buf_size);
//    RTNA_DBG_Str(0, "\r\n");

    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_GetVMDResolution
//  Description :
//------------------------------------------------------------------------------
/** @brief the function gets the configured resolution of motion detection frame.

The function gets the configured resolution of motion detection frame.
@param[out] width   The width of motion detection source frame
@param[out] height  The height of motion detection source frame
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_GetVMDResolution(MMP_UBYTE ubSnrSel, MMP_ULONG *width, MMP_ULONG *height)
{
    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);

    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, ubSnrSel);
    //    RTNA_DBG_Str(0, "MMPD_Sensor_GetVMDResolution  ubSnrSel   ");	RTNA_DBG_Long(0, ubSnrSel);
    //    RTNA_DBG_Str(0, "\r\n");
	

    if (m_VMD[ubSnrSel].RunOnCPUX == CPU_B)
    	MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_VMD | GET_VMD_RESOL | VMD_RUN_ON_CPUB);
    else
    	MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_VMD | GET_VMD_RESOL | VMD_RUN_ON_CPUA);

    *width  = MMPH_HIF_GetParameterL(GRP_IDX_FLOWCTL, 0);
    *height = MMPH_HIF_GetParameterL(GRP_IDX_FLOWCTL, 4);
    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_RegisterVMDCallback
//  Description :
//------------------------------------------------------------------------------
/** @brief the function configures the sensitivity of video motion detection.

The function set command to firmware to configures the sensitivity of motion detection.
@param[in] ulDiffThreshold The threshold of movement to be regarded as motion
@param[in] ulCntThreshold  The threshold of MB counts with difference larger than
                           ulDiffThreshold
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_RegisterVMDCallback(MMP_UBYTE ubSnrSel, void *Callback)
{
    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, (MMP_ULONG)ubSnrSel);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 4, (MMP_ULONG)Callback);

//    RTNA_DBG_Str(0, "MMPD_Sensor_RegisterVMDCallback  ubSnrSel   "); RTNA_DBG_Long(0, ubSnrSel);
//    RTNA_DBG_Str(0, "\r\n");
	
    if (m_VMD[ubSnrSel].RunOnCPUX == CPU_B)
    	MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_VMD | REG_VMD_CALLBACK | VMD_RUN_ON_CPUB);
    else
    	MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_VMD | REG_VMD_CALLBACK | VMD_RUN_ON_CPUA);

    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_StartVMD
//  Description :
//------------------------------------------------------------------------------
/** @brief the function enable the video motion detection.

The function set command to firmware to start the video motion detection.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_StartVMD(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, (MMP_ULONG)ubSnrSel);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 4, (MMP_ULONG)bEnable);

//    RTNA_DBG_Str(0, "MMPD_Sensor_StartVMD  ubSnrSel   ");  RTNA_DBG_Long(0, ubSnrSel);
//    RTNA_DBG_Str(0, "\r\n");
	

    if (m_VMD[ubSnrSel].RunOnCPUX == CPU_B)
    	MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_VMD | START_VMD | VMD_RUN_ON_CPUB);
    else
    	MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_VMD | START_VMD | VMD_RUN_ON_CPUA);

    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);
    
    return MMP_ERR_NONE;
}
#endif

#if 0
void _____ADAS_Functions_____(){}
#endif

#if (SUPPORT_ADAS)
extern MMPF_ADAS_INSTANCE	m_ADAS[];
//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_InitializeADAS
//  Description :
//------------------------------------------------------------------------------
/** @brief Function to initialize ADAS

The function initailizes the ADAS configuration
@param[in] usWidth the width of input frame for ADAS detection
@param[in] usHeight the height of input frame for ADAS detection
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_InitializeADAS(MMP_UBYTE ubSnrSel, MMP_UBYTE RunOnCPUX, MMP_ULONG ResolW, MMP_ULONG ResolH, MMP_UBYTE FrameGap)
{
    MMP_ERR status;

    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);

    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, ubSnrSel);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 4, (MMP_ULONG)RunOnCPUX);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 8, (MMP_ULONG)ResolW);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 12, (MMP_ULONG)ResolH);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 16, (MMP_ULONG)FrameGap);

    if (RunOnCPUX == CPU_B)
    	status = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_ADAS | INIT_ADAS | ADAS_RUN_ON_CPUB);
    else
    	status = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_ADAS | INIT_ADAS | ADAS_RUN_ON_CPUA);
    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);

    return status;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_GetADASResolution
//  Description :
//------------------------------------------------------------------------------
/** @brief the function gets the configured resolution of ADAS frame.

The function gets the configured resolution of ADAS frame.
@param[out] width   The width of ADAS source frame
@param[out] height  The height of ADAS source frame
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_GetADASResolution(MMP_UBYTE ubSnrSel, MMP_ULONG *width, MMP_ULONG *height)
{
    MMP_ERR status;

    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);

    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, ubSnrSel);
//    RTNA_DBG_Str(0, "MMPD_Sensor_GetADASResolution  ubSnrSel   ");	RTNA_DBG_Long(0, ubSnrSel);
//    RTNA_DBG_Str(0, "\r\n");

    if (m_ADAS[ubSnrSel].RunOnCPUX == CPU_B)
    	status = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_ADAS | GET_ADAS_RESOL | ADAS_RUN_ON_CPUB);
    else
    	status = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_ADAS | GET_ADAS_RESOL | ADAS_RUN_ON_CPUA);

    *width  = MMPH_HIF_GetParameterL(GRP_IDX_FLOWCTL, 0);
    *height = MMPH_HIF_GetParameterL(GRP_IDX_FLOWCTL, 4);
//    RTNA_DBG_Str(0, "\r\n *width   ");	RTNA_DBG_Long(0, *width);
//    RTNA_DBG_Str(0, "\r\n  *height ");	RTNA_DBG_Long(0, *height);
//    RTNA_DBG_Str(0, "\r\n");
    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);
    
    return status;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetADASFeature
//  Description :
//------------------------------------------------------------------------------
/** @brief Enable or disable each feature in ADAS.

The function control each feature within ADAS ON/OFF.
@param[in] ldws_on Enable LDWS feature
@param[in] fcws_on Enable FCWS feature
@param[in] sng_on Enable SnG feature
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetADASFeature(MMP_UBYTE ubSnrSel, MMP_BOOL ldws_on, MMP_BOOL fcws_on, MMP_BOOL sag_on)
{
    MMP_ERR status;

    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_FLOWCTL, 0, ubSnrSel);
    MMPH_HIF_SetParameterB(GRP_IDX_FLOWCTL, 1, ldws_on);
    MMPH_HIF_SetParameterB(GRP_IDX_FLOWCTL, 2, fcws_on);
    MMPH_HIF_SetParameterB(GRP_IDX_FLOWCTL, 3, sag_on);

    if (m_ADAS[ubSnrSel].RunOnCPUX == CPU_B)
    	status = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_ADAS | CTL_ADAS_MODE | ADAS_RUN_ON_CPUB);
    else
    	status = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_ADAS | CTL_ADAS_MODE | ADAS_RUN_ON_CPUA);

    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);

    return status;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetADASBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief Set working buffer address for ADAS operation.

The function set video ADAS buffer for operation
@param[in] buf_addr the address of ADAS working buffer
@param[in] buf_addr the address of ADAS DMA destination buffer
@param[out] buf_size the total size of working buffer
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetADASBuf(MMP_UBYTE ubSnrSel, MMP_ULONG buf_addr, MMP_ULONG *buf_size, MMP_ULONG dma_buf_addr)
{
    MMP_ERR status;

    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, ubSnrSel);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 4, buf_addr);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 8, dma_buf_addr);

//    RTNA_DBG_Str(0, "MMPD_Sensor_SetADASBuf  ubSnrSel   ");	RTNA_DBG_Long(0, ubSnrSel);
//    RTNA_DBG_Str(0, "\r\n");

    if (m_ADAS[ubSnrSel].RunOnCPUX == CPU_B)
    	status = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_ADAS | SET_ADAS_BUF | ADAS_RUN_ON_CPUB);
    else
    	status = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_ADAS | SET_ADAS_BUF | ADAS_RUN_ON_CPUA);

    *buf_size = MMPH_HIF_GetParameterL(GRP_IDX_FLOWCTL, 0);
    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);
    
    return status;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_StartADAS
//  Description :
//------------------------------------------------------------------------------
/** @brief the function enable ADAS feature.

The function set command to firmware to start ADAS feature.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_StartADAS(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    MMP_ERR status;

    MMPH_HIF_WaitSem(GRP_IDX_FLOWCTL, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 0, (MMP_ULONG)ubSnrSel);
    MMPH_HIF_SetParameterL(GRP_IDX_FLOWCTL, 4, (MMP_ULONG)bEnable);

    if (m_ADAS[ubSnrSel].RunOnCPUX == CPU_B)
    	status = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_ADAS | START_ADAS | ADAS_RUN_ON_CPUB);
    else
    	status = MMPH_HIF_SendCmd(GRP_IDX_FLOWCTL, HIF_FCTL_CMD_ADAS | START_ADAS | ADAS_RUN_ON_CPUA);

    MMPH_HIF_ReleaseSem(GRP_IDX_FLOWCTL);

    return status;
}
#endif

