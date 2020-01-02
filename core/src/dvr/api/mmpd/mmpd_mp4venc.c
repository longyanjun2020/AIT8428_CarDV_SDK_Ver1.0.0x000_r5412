/// @ait_only
/**
 @file mmpd_mp4venc.c
 @brief Retina Video Encoder Control Driver Function
 @author Will Tseng
 @version 1.0
*/

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mmp_lib.h"
#include "lib_retina.h"
#include "mmpd_mp4venc.h"
#include "mmpd_system.h"
#include "mmph_hif.h"
#include "mmpf_mp4venc.h"
#include "mmpf_mci.h"

/** @addtogroup MMPD_MP4VENC
 *  @{
 */

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

static MMP_USHORT           m_usVidRecdWidth;       ///< encode width
static MMP_USHORT           m_usVidRecdHeight;      ///< encode height

static MMP_BOOL             m_bEncodeTimerEn = MMP_FALSE;

static MMP_ULONG            m_ulMaxBitsPer3600MB[VIDEO_INIT_QP_STEP_NUM] =
                                                    {66667, 133334, 266667};

static MMP_ULONG            m_ulH264QpStep[VIDEO_INIT_QP_STEP_NUM][3] = {
                                                    {36, 36, 36}, // I,P,B
                                                    {30, 30, 30},
                                                    {24, 24, 24}};

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#if (DUALENC_SUPPORT)
//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetSWStickerAttr
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set S/W Sticker attribute.
 @param[in] usStickerSrcW : Sticker width
 @param[in] usStickerSrcH : Sticker height
 @param[in] usDstStartX   : Sticker position X
 @param[in] usDstStartY   : Sticker position Y
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_VIDENC_SetSWStickerAttr(MMP_USHORT usStickerSrcW, 
                                     MMP_USHORT usStickerSrcH, 
                                     MMP_USHORT usDstStartX, 
                                     MMP_USHORT usDstStartY)
{
    return MMPF_VIDENC_SetSWStickerAttribute(usStickerSrcW, usStickerSrcH, usDstStartX, usDstStartY);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetSWStickerAddress
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set S/W Sticker address.
 @param[in] ulStickerSrcAddr : Sticker source image address
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_VIDENC_SetSWStickerAddress(MMP_ULONG ulStickerSrcAddr)
{
    return MMPF_VIDENC_SetSWStickerAddress(ulStickerSrcAddr);
}
#endif

#if 0
void _____Initialize_Functions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_IsModuleInit
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMPD_VIDENC_IsModuleInit(void)
{
    return MMPF_VIDENC_IsModuleInit();
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_InitModule
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_InitModule(void)
{
    MMP_ERR	mmpstatus = MMP_ERR_NONE;

    MMPD_System_EnableClock(MMPD_SYS_CLK_H264, MMP_TRUE);

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_INITMOD);
   	
   	if (mmpstatus != MMP_ERR_NONE) {
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
        MMPD_System_EnableClock(MMPD_SYS_CLK_H264, MMP_FALSE);
       	return mmpstatus;
   	}
   	
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    MMPD_System_EnableClock(MMPD_SYS_CLK_H264, MMP_FALSE);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_DeinitModule
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_DeinitModule(void)
{
    return MMPF_VIDENC_DeinitModule();
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_InitInstance
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_InitInstance(MMP_ULONG *InstId, MMP_USHORT usStreamType, MMP_USHORT usRcMode)
{
    return MMPF_VIDENC_InitInstance(InstId, usStreamType, usRcMode);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_DeInitInstance
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_DeInitInstance(MMP_ULONG InstId)
{
    return MMPF_VIDENC_DeInitInstance(InstId);
}

#if 0
void _____Encode_Functions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_IsTimerEnabled
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Start timer for recording
 @param[in] bEnable enable/disable timer
 @retval MMP_ERR_NONE Success
*/
MMP_ERR MMPD_VIDENC_IsTimerEnabled(MMP_BOOL* pbEnable)
{
    *pbEnable = m_bEncodeTimerEn;
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_EnableTimer
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Start timer for recording
 @param[in] bEnable enable/disable timer
 @retval MMP_ERR_NONE Success
*/
MMP_ERR MMPD_VIDENC_EnableTimer(MMP_BOOL bEnable)
{
    MMP_ERR	mmpstatus = MMP_ERR_NONE;
    
    m_bEncodeTimerEn = bEnable;
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterB(GRP_IDX_VID, 0, bEnable);
    
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_TIMER_EN);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetQuality
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetQuality(MMP_ULONG ulEncId, MMP_ULONG ulTargetSize, MMP_ULONG ulBitrate)
{
    MMP_ULONG   ulBaseTargetBits, ulMBNum, i;
    MMP_ULONG   InitQp[3];

    ulMBNum = (((MMP_ULONG)m_usVidRecdWidth)*((MMP_ULONG)m_usVidRecdHeight)) / 256;
    if (ulMBNum == 0) {
        return MMP_MP4VE_ERR_NOT_SUPPORTED_PARAMETERS;
    }

    ulBaseTargetBits = ((ulTargetSize * 3600) / ulMBNum) * 8;

    for (i = 0; i < (VIDEO_INIT_QP_STEP_NUM - 1); i++) {
        if (ulBaseTargetBits <= m_ulMaxBitsPer3600MB[i]) {
            break;
        }
    }

    InitQp[0] = m_ulH264QpStep[i][0];
    InitQp[1] = m_ulH264QpStep[i][1];
    InitQp[2] = m_ulH264QpStep[i][2];
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0,  ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4,  InitQp[0]);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8,  InitQp[1]);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 12, InitQp[2]);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 16, ulBitrate);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_QUALITY_CTL);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetSrcPipe
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetSrcPipe(MMP_ULONG ulEncId, MMP_UBYTE ubPipe)
{
    MMP_ERR mmpstatus;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, ubPipe);

    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | SET_SRC_PIPE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetCropping
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetCropping(MMP_ULONG ulEncId, MMP_USHORT usTop, MMP_USHORT usBottom, MMP_USHORT usLeft, MMP_USHORT usRight)
{
    if ((usTop >= 16) || (usBottom >= 16) || (usBottom >= 16) || (usBottom >= 16) ||
        (usTop & 0x01) || (usBottom & 0x01) || (usBottom & 0x01) || (usBottom & 0x01)) {
        return MMP_MP4VE_ERR_NOT_SUPPORTED_PARAMETERS;
    }
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usTop);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 6, usBottom);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 8, usLeft);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 10, usRight);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_CROPPING);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetCurBufMode
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Set encoding current buffer mode to H264
 @param[in] VideoFormat Video current buffer mode.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_VIDENC_SetCurBufMode(MMP_ULONG ulEncId, VIDENC_CURBUF_MODE VideoCurBufMode)
{
    MMP_ERR mmpstatus;
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, VideoCurBufMode);

    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_CURBUFMODE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    if (mmpstatus) {
        return mmpstatus;
    }
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetResolution
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetResolution(MMP_ULONG ulEncId, MMP_USHORT usWidth, MMP_USHORT usHeight)
{
    MMP_ERR mmpstatus = MMP_ERR_NONE;

    m_usVidRecdWidth = usWidth;
    m_usVidRecdHeight = usHeight;
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usWidth);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 6, usHeight);
    
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_RESOLUTION);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetProfile
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetProfile(MMP_ULONG ulEncId, VIDENC_PROFILE profile)
{
    MMP_ULONG   ulProfile;
    MMP_ERR	    mmpstatus = MMP_ERR_NONE;

    if ((profile <= H264ENC_PROFILE_NONE) || 
        (profile >= H264ENC_PROFILE_MAX)) {
        return MMP_MP4VE_ERR_NOT_SUPPORTED_PARAMETERS;
    }
    
    switch(profile) {
    case H264ENC_BASELINE_PROFILE:
        ulProfile = BASELINE_PROFILE;
        break;
    case H264ENC_MAIN_PROFILE:
        ulProfile = MAIN_PROFILE;
        break;
    case H264ENC_HIGH_PROFILE:
        ulProfile = FREXT_HP;
        break;
    default:
        break;
    }

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulProfile);
    
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_PROFILE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetLevel
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetLevel(MMP_ULONG ulEncId, MMP_ULONG ulLevel)
{
    MMP_ERR	mmpstatus = MMP_ERR_NONE;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulLevel);
    
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_LEVEL);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetEntropy
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetEntropy(MMP_ULONG ulEncId, VIDENC_ENTROPY entropy)
{
    MMP_ERR	mmpstatus = MMP_ERR_NONE;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, entropy);
    
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_ENTROPY);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetForceI
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetForceI(MMP_ULONG ulEncId, MMP_ULONG ulCnt)
{
    MMP_ERR mmpstatus;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, ulCnt);

    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_FORCE_I);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetRcMode
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetRcMode(MMP_ULONG ulEncId, VIDENC_RC_MODE mode)
{
    MMP_ERR mmpstatus;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, mode);

    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_RC_MODE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetRcSkipEn
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetRcSkipEn(MMP_ULONG ulEncId, MMP_BOOL bSkip)
{
    MMP_ERR mmpstatus;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, bSkip);

    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_RC_SKIP);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetRcSkipType
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetRcSkipType(MMP_ULONG ulEncId, VIDENC_RC_SKIPTYPE type)
{
    MMP_ERR mmpstatus;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, type);

    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_RC_SKIPTYPE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetRcVBVSize
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetRcVBVSize(MMP_ULONG ulEncId, MMP_ULONG lbs)
{
    MMP_ERR mmpstatus;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, lbs);

    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_RC_LBS);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetTNREnable
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetTNREnable(MMP_ULONG ulEncId, MMP_ULONG tnr)
{
    MMP_ERR mmpstatus;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, tnr);

    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_TNR_EN);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetInitQP
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetInitQP(MMP_ULONG ulEncId, MMP_UBYTE ubIQP, MMP_UBYTE ubPQP)
{
    MMP_ERR mmpstatus;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, ubIQP);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 8, ubPQP);

    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | SET_QP_INIT);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetQPBoundary
//  Description : 
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetQPBoundary(MMP_ULONG ulEncId, MMP_ULONG ulFrmType, MMP_ULONG ulLowerBound, MMP_ULONG ulUpperBound)
{
    MMP_ERR	mmpstatus = MMP_ERR_NONE;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulFrmType);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, ulLowerBound);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 12, ulUpperBound);
    
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | SET_QP_BOUNDARY);	
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_GetSkipThreshold
//  Description : 
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_GetSkipThreshold(MMP_ULONG ulEncId, MMP_ULONG *pThr)
{
    MMP_ERR	mmpstatus = MMP_ERR_NONE;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | GET_SKIP_THD);	
    *pThr = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetEncodeMode
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetEncodeMode(void)
{
    MMPD_System_EnableClock(MMPD_SYS_CLK_H264, MMP_TRUE);

    MMPF_VIDENC_ResetVLDModule();

    MMPD_System_EnableClock(MMPD_SYS_CLK_H264, MMP_FALSE);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetGOP
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetGOP(MMP_ULONG ulEncId, MMP_USHORT usPFrame, MMP_USHORT usBFrame)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usPFrame);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 6, usBFrame);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_GOP);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetBitrate
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetBitrate(MMP_ULONG ulEncId, MMP_ULONG ulBitrate)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulBitrate);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_BIT_RATE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetEncFrameRate
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetEncFrameRate(MMP_ULONG ulEncId, MMP_ULONG ulTimeIncrement, MMP_ULONG ulTimeResol)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulTimeResol);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, ulTimeIncrement);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_FRAME_RATE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_UpdateEncFrameRate
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_UpdateEncFrameRate(MMP_ULONG ulEncId, MMP_ULONG ulTimeIncrement, MMP_ULONG ulTimeResol)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ulTimeResol);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, ulTimeIncrement);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | UPD_ENC_FRAME_RATE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetSnrFrameRate
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_SetSnrFrameRate(MMP_ULONG ulEncId, MMP_USHORT usStreamType, MMP_ULONG ulTimeIncrement, MMP_ULONG ulTimeResol)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, usStreamType);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, ulTimeResol);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 12, ulTimeIncrement);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | SNR_FRAME_RATE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_EnableClock
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_EnableClock(MMP_BOOL bEnable)
{
    MMPD_System_EnableClock(MMPD_SYS_CLK_H264, bEnable);
    MMPD_System_EnableClock(MMPD_SYS_CLK_AUD, bEnable);
    MMPD_System_EnableClock(MMPD_SYS_CLK_ADC, bEnable);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_GetNumOpen
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Check the video encoding number.
 @param[out] ulNumOpening The number of the opening stream.
 @retval MMP_ERR_NONE Success.
 @note
*/
MMP_ERR MMPD_VIDENC_GetNumOpen(MMP_ULONG *ulNumOpening)
{
    MMP_ERR	mmpstatus = MMP_ERR_NONE;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | GET_VIDRECD_NUMOPEN);

    if (mmpstatus) {
        *ulNumOpening = 0;
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
        return mmpstatus;
    }

    *ulNumOpening = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);

    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_GetStatus
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Check the firmware video encoding engine status.
 @param[out] status Firmware video engine status.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_VIDENC_GetStatus(MMP_ULONG ulEncId, VIDENC_FW_STATUS *status)
{
    MMP_ERR	mmpstatus = MMP_ERR_NONE;

    if (ulEncId != INVALID_ENC_ID) {
        MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
        
        #if 1 //Enhance system performance since this function is called by multi-tasks
        *status = MMPF_MP4VENC_GetStatus(ulEncId);
        #else
        MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
        mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | GET_VIDRECD_STATUS);

        if (mmpstatus) {
            MMPH_HIF_ReleaseSem(GRP_IDX_VID);
            return mmpstatus;
        }
        
        *status = MMPH_HIF_GetParameterW(GRP_IDX_VID, 0);
        #endif
        
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    }
    else {
        *status = VIDENC_FW_STATUS_NONE;
    }

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_GetMergerStatus
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
MMP_ERR MMPD_VIDENC_GetMergerStatus(MMP_ERR *status, MMP_ULONG *tx_status)
{
    MMP_ERR	mmpstatus = MMP_ERR_NONE;
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | MERGER_STATUS);

    if (mmpstatus) {
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
        return mmpstatus;
    }
    
    *status    = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    *tx_status = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);
    
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_CheckCapability
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Check the capability of video encoding engine.
 @param[out] If the specified resolution and frame rate can be supported.
 @retval MMP_ERR_NONE for Supported.
 @note
*/
MMP_ERR MMPD_VIDENC_CheckCapability(MMP_ULONG w, MMP_ULONG h, MMP_ULONG fps)
{
    MMP_ERR	    ret = MMP_ERR_NONE;
    MMP_BOOL    supported = MMP_FALSE;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ((w + 15) >> 4) * ((h + 15) >> 4));
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, fps);
    
    ret = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_CAPABILITY);
    if (ret == MMP_ERR_NONE) {
        supported = MMPH_HIF_GetParameterB(GRP_IDX_VID, 0);
    }
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    if (!supported) { // unsupported resol & frame rate
        ret = MMP_MP4VE_ERR_CAPABILITY;
    }
    return ret;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_TuneMCIPriority
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Fine tune MCI priority to fit VGA size encoding. It's an access issue.
 @param[in] ubMode Mode.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_VIDENC_TuneMCIPriority(MMPD_VIDENC_MCI_MODE mciMode)
{
    return MMPF_MCI_TunePriority(mciMode);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_TunePipeMaxMCIPriority
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Fine tune MCI priority of encode pipe
 @param[in] ubPipe Encode pipe
 @retval MMP_ERR_NONE Success.
*/
void MMPD_VIDENC_TunePipeMaxMCIPriority(MMP_UBYTE ubPipe)
{
    MMPF_MCI_SetIBCMaxPriority(ubPipe);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_TunePipe2ndMCIPriority
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Fine tune MCI priority of second encode pipe (smaller resolution)
 @param[in] ubPipe Encode pipe
 @retval MMP_ERR_NONE Success.
*/
void MMPD_VIDENC_TunePipe2ndMCIPriority(MMP_UBYTE ubPipe)
{
    MMPF_MCI_SetIBCSecondPriority(ubPipe);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_EnableVrThumbnail
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set VR thumbnail feature enable or not
 @param[in] Enable VR thumbnail
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_VIDENC_EnableVrThumbnail(MMP_UBYTE ubEnable, MMP_UBYTE ubIsCreateJpg)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubEnable);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, ubIsCreateJpg);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | VR_THUMBNAIL_ENABLE);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_SetVrThumbRingBufNum
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set VR thumbnail Ring buffer numbers
 @param[in] Set Ring buffer numbers
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_VIDENC_SetVrThumbRingBufNum(MMP_UBYTE ubRingBufNum)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubRingBufNum);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | VR_THUMBNAIL_SET_RING_BUF_NUM);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

#if (SUPPORT_VR_THUMBNAIL)
//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_GetVrThumbnailSts
//  Description : 
//------------------------------------------------------------------------------
MMP_UBYTE MMPD_VIDENC_GetVrThumbnailSts(void)
{
    MMP_ERR     mmpstatus = MMP_ERR_NONE;
    MMP_UBYTE   bEnable;
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | VR_THUMBNAIL_GET_STS);

    if (mmpstatus) {
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
        return mmpstatus;
    }
    
    bEnable = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return bEnable;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_StartStreaming
//  Description : To quickly start WiFi H264 encoder
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_StartStreaming(MMP_ULONG ulEncId, MMP_USHORT usStreamType)
{
    MMP_ERR	status = MMP_ERR_NONE;
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usStreamType);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | STREAMING_ENC_START);
    status = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
  
    return status;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_VIDENC_StopStreaming
//  Description : To quickly stop WiFi H264 encoder
//------------------------------------------------------------------------------
MMP_ERR MMPD_VIDENC_StopStreaming(MMP_ULONG ulEncId, MMP_USHORT usStreamType)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usStreamType);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | STREAMING_ENC_STOP);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

#if 0
void _____H264E_Functions_____(){}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_H264ENC_SetBitstreamBuf
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPD_H264ENC_SetBitstreamBuf(MMP_ULONG ulEncId, MMPD_H264ENC_BITSTREAM_BUF *pBsBuf)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, pBsBuf->ulStart);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, pBsBuf->ulEnd);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_BSBUF);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_H264ENC_SetSourcePPBuf
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set video current buffer.

 This buffer is for sensor raw data and should be counted to the maximum encoding
 resolution. It can be pingpong or not, just depends on the memory requirement.
 @param[in] *inputbuf Pointer of encode buffer structure.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_SetSourcePPBuf(VIDENC_INPUT_BUF *inputbuf)
{
    extern MMP_ERR MMPF_H264ENC_SetSourcePPBuf(VIDENC_INPUT_BUF *inputbuf);

    MMPD_System_EnableClock(MMPD_SYS_CLK_H264, MMP_TRUE);

    MMPF_H264ENC_SetSourcePPBuf(inputbuf);

    MMPD_System_EnableClock(MMPD_SYS_CLK_H264, MMP_FALSE);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_H264ENC_SetRefGenBound
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set video Reference/Generate buffer bound
 @param[in] *refgenbd Pointer of encode buffer structure.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_SetRefGenBound(MMP_ULONG ubEncId, MMPD_H264ENC_REFGEN_BD *refgenbd)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, refgenbd->ulRefYStart);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, refgenbd->ulGenUVEnd);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_REFGENBD);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_H264ENC_SetMiscBuf
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set slice length, MV buffer.
 @param[in] *miscbuf Pointer of encode buffer structure.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_SetMiscBuf(MMP_ULONG ubEncId, MMPD_H264ENC_MISC_BUF *miscbuf)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, miscbuf->ulMVBuf);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, miscbuf->ulSliceLenBuf);

    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_MISCBUF);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_H264ENC_SetHeaderBuf
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set buffer for SPS, PPS
 @param[in] hdrbuf Buffer address
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_SetHeaderBuf(MMP_ULONG ubEncId, MMPD_H264ENC_HEADER_BUF *hdrbuf)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ubEncId);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, hdrbuf->ulSPSStart);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 8, hdrbuf->ulPPSStart);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 12, hdrbuf->ulTmpSPSStart);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | SET_HEADER_BUF);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_H264ENC_GetHeaderInfo
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Get SPS, PPS address and size
 @param[out] hdrbuf Info
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_GetHeaderInfo(MMPD_H264ENC_HEADER_BUF *hdrbuf)
{
    MMP_ERR	mmpstatus;

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    
    mmpstatus = MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | GET_HEADER_INFO);
    if (mmpstatus != MMP_ERR_NONE) {
        MMPH_HIF_ReleaseSem(GRP_IDX_VID);
       	return mmpstatus;
   	}

    hdrbuf->ulSPSStart = MMPH_HIF_GetParameterL(GRP_IDX_VID, 0);
    hdrbuf->ulSPSSize  = MMPH_HIF_GetParameterL(GRP_IDX_VID, 4);
    hdrbuf->ulPPSStart = MMPH_HIF_GetParameterL(GRP_IDX_VID, 8);
    hdrbuf->ulPPSSize  = MMPH_HIF_GetParameterL(GRP_IDX_VID, 12);
    
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_H264ENC_CalculateRefBuf
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Calculate and Generate the REF/GEN buffer for Video Encode Module.

 Depends on encoded resolution to generate the REF/GEN buffer
 @param[in] usWidth Encode width.
 @param[in] usHeight Encode height.
 @param[out] refgenbd Mp4 engine require REF/GEN buffer.
 @param[in,out] ulCurAddr Available start address for buffer start.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_CalculateRefBuf(MMP_USHORT             usWidth, 
                                     MMP_USHORT             usHeight, 
                                     MMPD_H264ENC_REFGEN_BD *refgenbd, 
                                     MMP_ULONG              *ulCurAddr)
{
    MMP_ULONG bufsize;
    
    *ulCurAddr = ALIGN32(*ulCurAddr);

    bufsize = usWidth * usHeight;
    refgenbd->ulRefYStart   = *ulCurAddr;
    *ulCurAddr += bufsize;
    refgenbd->ulRefYEnd     = *ulCurAddr;

    bufsize /= 2;
    refgenbd->ulRefUVStart  = *ulCurAddr;
    *ulCurAddr += bufsize;
    refgenbd->ulRefUVEnd    = *ulCurAddr;

    *ulCurAddr = ALIGN32(*ulCurAddr);

    #if (SHARE_REF_GEN_BUF == 1)
        refgenbd->ulGenYStart   = refgenbd->ulRefYStart;
        refgenbd->ulGenUVEnd    = refgenbd->ulRefUVEnd;
    #else
        #if (H264ENC_ICOMP_EN)
        bufsize = usWidth * usHeight;
        refgenbd->ulGenYStart   = *ulCurAddr;
        *ulCurAddr += bufsize;
        refgenbd->ulGenYEnd     = *ulCurAddr;
        
        refgenbd->ulGenUVStart  = *ulCurAddr;
        *ulCurAddr += bufsize / 2;
        refgenbd->ulGenUVEnd    = *ulCurAddr;
        #else
        bufsize = (usWidth * usHeight * 3) / 2;
        refgenbd->ulGenYStart   = *ulCurAddr;
        *ulCurAddr += bufsize;
        refgenbd->ulGenUVEnd    = *ulCurAddr;
        #endif
    #endif

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_H264ENC_SetPadding
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set video padding for video height
 @param[in] usType padding type 0: zero, 1: repeat
 @param[in] usCnt  the height line offset which need to pad
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_SetPadding(MMP_ULONG ulEncId, MMP_USHORT usType, MMP_USHORT usCnt)
{
    if (usCnt > 15) {
        return MMP_MP4VE_ERR_NOT_SUPPORTED_PARAMETERS;
    }
    
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, ulEncId);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 4, usType);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 6, usCnt);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_PADDING);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_H264ENC_SetEncByteCnt
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set encode mci byte count
 @param[in] usByteCnt byte count value (128 or 256)
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_SetEncByteCnt(MMP_USHORT usByteCnt)
{
    MMPD_System_EnableClock(MMPD_SYS_CLK_H264, MMP_TRUE);

    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterW(GRP_IDX_VID, 0, usByteCnt);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | ENCODE_BYTE_COUNT);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    MMPD_System_EnableClock(MMPD_SYS_CLK_H264, MMP_FALSE);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_H264ENC_SetUVCHdrBuf
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief Set buffer for SPS, PPS to UVC record
 @param[in] hdrbuf Buffer address
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_SetUVCHdrBuf(MMPD_H264ENC_HEADER_BUF *hdrbuf)
{
    MMPH_HIF_WaitSem(GRP_IDX_VID, 0);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 0, hdrbuf->ulSPSStart);
    MMPH_HIF_SetParameterL(GRP_IDX_VID, 4, hdrbuf->ulPPSStart);
    
    MMPH_HIF_SendCmd(GRP_IDX_VID, HIF_VID_CMD_RECD_PARAMETER | SET_UVC_HDRBUF);
    MMPH_HIF_ReleaseSem(GRP_IDX_VID);
    
    return MMP_ERR_NONE;
}

/// @}
/// @end_ait_only
