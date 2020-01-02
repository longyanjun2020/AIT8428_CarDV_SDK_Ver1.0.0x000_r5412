//==============================================================================
//
//  File        : AHC_Sensor.c
//  Description : AHC Sensor control function
//  Author      : 
//  Revision    : 1.0
//
//==============================================================================

/*===========================================================================
 * Include files
 *===========================================================================*/ 

#include "ait_config.h"
#include "mmps_dsc.h"
#include "mmps_3gprecd.h"
#include "mmps_display.h"
#include "mmps_sensor.h"
#include "mmpf_sensor.h"
#include "mmpf_pll.h"
#include "AHC_general.h"
#include "AHC_Sensor.h"
#include "isp_if.h"
#include "hdr_cfg.h"
#include "MenuSetting.h"
#include "Sensor_Mod_Remapping.h"

/*===========================================================================
 * Local varible
 *===========================================================================*/ 

static INT32                    m_wSensorMode           = AHC_SENSOR_MODE_AUTO;
static INT32                    m_wCaptureSensorMode    = AHC_SENSOR_MODE_AUTO;
static INT32                    m_wTVDecSrcTypeMode     = AHC_SENSOR_MODE_AUTO;

MMP_UBYTE                       m_ubSnrFlipType[VIF_SENSOR_MAX_NUM] = {0};

/*===========================================================================
 * Extern varible
 *===========================================================================*/ 

extern MMP_USHORT               gsAhcPrmSensor;

/*===========================================================================
 * Main body
 *===========================================================================*/

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_IsSnrInitialized
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SNR_IsSnrInitialized(UINT8 ubSnrSel)
{
    return MMPS_Sensor_IsInitialized(ubSnrSel);
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_SetFlipDir
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SNR_SetFlipDir(UINT8 ubSnrSel, AHC_SENSOR_FLIP_TYPE ubFlipType)
{
    MMPS_Sensor_SetFlip(ubSnrSel, ubFlipType);
    m_ubSnrFlipType[ubSnrSel] = (MMP_UBYTE)ubFlipType;
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_SetLightFreq
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SNR_SetLightFreq(AHC_SENSOR_DEBAND_MODE smode)
{
    MMPS_Sensor_Set3AFunction(MMP_ISP_3A_FUNC_SET_AE_FLICKER_MODE, (MMP_SNR_DEBAND_MODE)smode);
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_GetScalInputWH
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SNR_GetScalInputWH(UINT8 ubSnrSel, UINT16 wMode, UINT16* puwWidth, UINT16* puwHeight)
{
    MMP_ULONG ulWidth, ulHeight;

    if (wMode != SENSOR_DRIVER_MODE_NOT_SUUPORT) {
        return AHC_FALSE;
    }

    MMPS_Sensor_GetScalInputRes(ubSnrSel, wMode, &ulWidth, &ulHeight);
    
    *puwWidth = ulWidth;
    *puwHeight = ulHeight;

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_GetCurSensorFPS
//  Description :
//------------------------------------------------------------------------------
UINT32 AHC_SNR_GetCurSensorFPS(UINT8 ubSnrSel)
{
    extern MMPF_SENSOR_FUNCTION *gsSensorFunction;

    if (gsSensorFunction->MMPF_Sensor_GetRealFPS) {
        return (UINT32) gsSensorFunction->MMPF_Sensor_GetRealFPS(ubSnrSel);
    }
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_PowerCtrl
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SNR_PowerCtrl(AHC_BOOL bOn)
{
#ifdef SENSOR_EN_GPIO
    if (SENSOR_EN_GPIO != AHC_PIO_REG_UNKNOWN)
    {
        AHC_GPIO_ConfigPad(SENSOR_EN_GPIO, PAD_OUT_DRIVING(0));
        AHC_GPIO_SetOutputMode(SENSOR_EN_GPIO, AHC_TRUE);
        AHC_GPIO_SetData(SENSOR_EN_GPIO, (bOn ? SENSOR_EN_GPIO_ACT_MODE : !SENSOR_EN_GPIO_ACT_MODE));
    }
#endif

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_PresetSnrMode
//  Description : 
//------------------------------------------------------------------------------
/**
 @brief This API preset sensor mode for AHC usage.
 
 This function preset sensor mode for AHC usage. This APS is 
 for select sensor mode before enter video preview or still 
 capture preview.
 
 Parameters:
 
 @param[in] wMode          The sensor mode id. This is the 
                           definition in sensor driver or in
                           ait_config.c .
  
 @retval AHC_TRUE Success.
*/
AHC_BOOL AHC_SNR_PresetSnrMode(INT16 wMode)
{
    UINT32 ulHdrEn = 0;

    switch(wMode) {
        case AHC_SENSOR_MODE_VGA_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_VGA_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_VGA_50P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_VGA_50P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_VGA_60P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_VGA_60P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_VGA_100P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_VGA_100P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_VGA_120P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_VGA_120P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_24P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_HD_24P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_HD_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_50P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_HD_50P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_60P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_HD_60P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_100P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_HD_100P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_120P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_HD_120P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_1600x900_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_1600x900_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_FULL_HD_15P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_FULL_HD_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_FULL_HD_24P_RESOLUTION:
             m_wSensorMode = SENSOR_DRIVER_MODE_FULL_HD_24P_RESOLUTION;
             break;
        case AHC_SENSOR_MODE_FULL_HD_25P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_FULL_HD_25P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_FULL_HD_30P_RESOLUTION:

            if (AHC_Menu_SettingGetCB((char *)COMMON_KEY_HDR_EN, &ulHdrEn) == AHC_TRUE) {
                if (COMMON_HDR_EN_ON == ulHdrEn) {
                    m_wSensorMode = SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION_HDR;
                    if (m_wSensorMode == SENSOR_DRIVER_MODE_NOT_SUUPORT)
                    {
                        pf_HDR_EnSet(COMMON_HDR_EN_OFF);
                        printc("Sensor not support HDR, set sensor mode back to FHD\r\n");
                        m_wSensorMode = SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION;
                    }
                }
                else {
                    m_wSensorMode = SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION;
                }
            }
            else {
                m_wSensorMode = SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION;
            }
            break;
        case AHC_SENSOR_MODE_FULL_HD_30P_RESOLUTION_HDR:
            m_wSensorMode = SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION_HDR;
            break;
        case AHC_SENSOR_MODE_FULL_HD_50P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_FULL_HD_50P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_FULL_HD_60P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_FULL_HD_60P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_1440_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_1440_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_SUPER_HD_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_SUPER_HD_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_SUPER_HD_25P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_SUPER_HD_25P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_SUPER_HD_24P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_SUPER_HD_24P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_2D7K_15P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_2D7K_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_2D7K_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_2D7K_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4K2K_15P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4K2K_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4K2K_24P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4K2K_24P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4K2K_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4K2K_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_VGA_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_VGA_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_1D2M_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_1D2M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_1D5M_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_1D5M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_3M_15P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_3M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_3M_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_3M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_5M_15P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_5M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_5M_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_5M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_8M_15P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_8M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_8M_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_8M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_10M_15P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_10M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_10M_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_10M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_12M_15P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_12M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_12M_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_12M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_14M_15P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_14M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_14M_30P_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_4TO3_14M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_PAL_25FPS:
            m_wSensorMode = SENSOR_DRIVER_MODE_PAL_25P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_NTSC_30FPS:
            m_wSensorMode = SENSOR_DRIVER_MODE_NTSC_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_BEST_CAMERA_PREVIEW_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_BEST_CAMERA_PREVIEW_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_DEFAULT_PCCAM_RESOLUTION:
            m_wSensorMode = SENSOR_DRIVER_MODE_PCCAM_DEFAULT_RESOLUTION;
            break;
        default:
            m_wSensorMode = SENSOR_DRIVER_MODE_NOT_SUUPORT;
            break;
    }

    if (m_wSensorMode == SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION_HDR &&
        m_wSensorMode != SENSOR_DRIVER_MODE_NOT_SUUPORT) {
        MMP_EnableVidHDR(MMP_TRUE); 
    }
    else {
        MMP_EnableVidHDR(MMP_FALSE);
    }

    if (m_wSensorMode == SENSOR_DRIVER_MODE_NOT_SUUPORT) {
        printc(">>>>> AHC_SNR_PresetSnrMode Error!!!:%d\r\n",wMode);
        while(1);
    }
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_GetPresetSnrMode
//  Description :
//------------------------------------------------------------------------------
INT16 AHC_SNR_GetPresetSnrMode(void)
{
    return m_wSensorMode;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_PresetCaptureSnrMode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_SNR_PresetCaptureSnrMode(INT16 wMode)
{
    switch(wMode){
        case AHC_SENSOR_MODE_VGA_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_VGA_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_VGA_50P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_VGA_50P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_VGA_60P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_VGA_60P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_VGA_100P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_VGA_100P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_VGA_120P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_VGA_120P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_HD_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_50P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_HD_50P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_60P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_HD_60P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_100P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_HD_100P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_HD_120P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_HD_120P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_1600x900_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_1600x900_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_FULL_HD_15P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_FULL_HD_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_FULL_HD_25P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_FULL_HD_25P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_FULL_HD_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_FULL_HD_50P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_FULL_HD_50P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_1440_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_1440_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_SUPER_HD_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_SUPER_HD_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_FULL_HD_60P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_FULL_HD_60P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_2D7K_15P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_2D7K_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_2D7K_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_2D7K_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4K2K_15P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4K2K_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4K2K_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4K2K_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_VGA_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_VGA_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_1D2M_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_1D2M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_1D5M_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_1D5M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_3M_15P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_3M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_3M_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_3M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_5M_15P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_5M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_5M_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_5M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_8M_15P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_8M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_8M_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_8M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_10M_15P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_10M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_10M_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_10M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_12M_15P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_12M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_12M_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_12M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_14M_15P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_14M_15P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_4TO3_14M_30P_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_4TO3_14M_30P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_BEST_CAMERA_CAPTURE_4TO3_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_BEST_CAMERA_CAPTURE_4TO3_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_BEST_CAMERA_CAPTURE_16TO9_RESOLUTION:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_BEST_CAMERA_CAPTURE_16TO9_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_PAL_25FPS:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_PAL_25P_RESOLUTION;
            break;
        case AHC_SENSOR_MODE_NTSC_30FPS:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_NTSC_30P_RESOLUTION;
            break;
        default:
            m_wCaptureSensorMode = SENSOR_DRIVER_MODE_NOT_SUUPORT;
            break;
    }

    if (m_wCaptureSensorMode == SENSOR_DRIVER_MODE_NOT_SUUPORT) {
        printc(">>>>> AHC_SNR_PresetCaptureSnrMode Error!!!:%d\r\n",wMode);
        while(1);
    }

    MMPS_Sensor_SetCaptureMode(gsAhcPrmSensor, m_wCaptureSensorMode);

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_GetPresetCaptureSnrMode
//  Description :
//------------------------------------------------------------------------------
INT16 AHC_SNR_GetPresetCaptureSnrMode(void)
{
    return m_wCaptureSensorMode;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_GetTvDecSnrCnntStatus
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_SNR_GetTvDecSnrCnntStatus(void)
{
    MMP_SNR_TVDEC_SRC_TYPE sSnrTVSrc;
    
    MMPS_Sensor_GetTVDecSrcType(&sSnrTVSrc);

    if (sSnrTVSrc == MMP_SNR_TVDEC_SRC_NO_READY) {
        return AHC_FALSE;
    }
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_PresetTVDecSrcType
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SNR_PresetTVDecSrcType(INT32 wTVDecSrcType)
{
    switch (wTVDecSrcType) {
	case AHC_SENSOR_MODE_AHDHD_30FPS:
        m_wTVDecSrcTypeMode = SENSOR_DRIVER_MODE_AHDHD_30P_RESOLUTION;
        break;
    case AHC_SENSOR_MODE_AHDHD_25FPS:
        m_wTVDecSrcTypeMode = SENSOR_DRIVER_MODE_AHDHD_25P_RESOLUTION;
        break;
	case AHC_SENSOR_MODE_AHDFHD_30FPS:
        m_wTVDecSrcTypeMode = SENSOR_DRIVER_MODE_AHDFHD_25P_RESOLUTION;
        break;
	case AHC_SENSOR_MODE_AHDFHD_25FPS:
        m_wTVDecSrcTypeMode = SENSOR_DRIVER_MODE_AHDFHD_25P_RESOLUTION;
        break;
    case AHC_SENSOR_MODE_NTSC_30FPS:
    default:
        m_wTVDecSrcTypeMode = SENSOR_DRIVER_MODE_NTSC_30P_RESOLUTION;
        break;
    }
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SNR_GetPresetTVDecSrcType
//  Description :
//------------------------------------------------------------------------------
INT32 AHC_SNR_GetPresetTVDecSrcType(void)
{
    return m_wTVDecSrcTypeMode;
}