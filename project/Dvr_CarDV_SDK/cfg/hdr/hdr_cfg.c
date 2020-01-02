//------------------------------------------------------------------------------
//
//  File        : hdr_cfg.c
//  Description : Source file of HDR configuration
//  Author      : Eroy
//  Revision    : 0.1
//
//------------------------------------------------------------------------------

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "customer_config.h"
#include "AHC_Config_SDK.h"
#include "hdr_cfg.h"
#include "snr_cfg.h"
#include "mmps_3gprecd.h"

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

/*
 * Configure of HDR
 */
HDR_CFG gsHdrCfg = {
    MMP_FALSE,              // bVidEnable
    MMP_FALSE,              // bDscEnable
#if 0//(HDR_FULL_FOV_EN)
    HDR_MODE_SEQUENTIAL,    // ubMode
    MMP_FALSE,              // bRawGrabEnable
#else
    HDR_MODE_STAGGER,   	// ubMode
    MMP_TRUE,     			// bRawGrabEnable
#endif
    HDR_BITMODE_8BIT,   	// ubRawStoreBitMode
    #if 1
    HDR_VC_STORE_2PLANE,	// ubVcStoreMethod
    #else
    HDR_VC_STORE_2ENGINE,	// ubVcStoreMethod
	#endif
	2						// ubBktFrameNum
};

void MMP_EnableVidHDR(MMP_BOOL bEnable)
{
#if (SNR_CLK_POWER_SAVING_SETTING || (AHC_DRAM_SIZE == COMMON_DRAM_SIZE_32MB))
    RTNA_DBG_Str(0, FG_BLUE("Force turn-off HDR\r\n"));
	gsHdrCfg.bVidEnable = MMP_FALSE;
	MMPS_3GPRECD_GetConfig()->bRawPreviewEnable[0] = MMP_FALSE;
#else
	gsHdrCfg.bVidEnable = bEnable;
	MMPS_3GPRECD_GetConfig()->bRawPreviewEnable[0] = bEnable;
#endif

#if (SUPPORT_EIS)	//reset setting from MMPC_3gpRecd_InitConfig()
	MMPS_3GPRECD_GetConfig()->bRawPreviewEnable[0] = MMP_TRUE;
#endif

    if (CAM_CHECK_SCD(SCD_CAM_BAYER_SENSOR)) {
        gsHdrCfg.bVidEnable = MMP_FALSE;
        gsHdrCfg.bDscEnable = MMP_FALSE;
        MMPS_3GPRECD_GetConfig()->bRawPreviewEnable[0] = MMP_TRUE;
        MMPS_3GPRECD_GetConfig()->bRawPreviewEnable[1] = MMP_TRUE;
    }
}

void MMP_EnableDscHDR(MMP_BOOL bEnable)
{
#if (SNR_CLK_POWER_SAVING_SETTING || (AHC_DRAM_SIZE == COMMON_DRAM_SIZE_32MB) || (!SUPPORT_DSC_HDR_MODE))
    RTNA_DBG_Str(0, FG_BLUE("Force turn-off HDR\r\n"));
    gsHdrCfg.bDscEnable = MMP_FALSE;
#else
    gsHdrCfg.bDscEnable = bEnable;
#endif
}

MMP_BOOL MMP_IsVidHDREnable(void)
{
    return gsHdrCfg.bVidEnable;
}

MMP_BOOL MMP_IsDscHDREnable(void)
{
    return gsHdrCfg.bDscEnable;
}
