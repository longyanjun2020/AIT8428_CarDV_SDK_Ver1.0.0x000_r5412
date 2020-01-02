//==============================================================================
//
//  File        : sensor_ov2643.c
//  Description : Firmware Sensor Control File
//  Author      : 
//  Revision    : 1.0
//
//=============================================================================

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "config_fw.h"
#include "includes_fw.h"
#include "customer_config.h"

#if (SENSOR_EN)
#if (BIND_SENSOR_OV2643)

#include "mmpf_sensor.h"
#include "isp_if.h"
#include "mmp_i2c_inc.h"
#include "mmpf_vif.h"
#include "Sensor_Mod_Remapping.h"

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================

#ifdef SENSOR_IF
#undef SENSOR_IF
#define SENSOR_IF (SENSOR_IF_PARALLEL) // TBD
#endif

MMPF_SENSOR_RESOLUTION m_OV2643_SensorRes = 
{
    3,                                  // ubSensorModeNum
    2,                                  // ubDefPreviewMode
    0,                                  // ubDefCaptureMode
    2200,                               // usPixelSize
//  Mode0       Mode1		Mode2
//  1200@15P    600@30P     720@30P
    {1,         1,          1       },  // usVifGrabStX
    {1,         1,          1       },  // usVifGrabStY
    #if 1 // For YUV raw store mode grab range.
    {3200,      1600,       2560    },  // usVifGrabW
    {1200,      600,        720     },  // usVifGrabH
    #else // For YUV bypass ISP mode grab range.
    {1600,      800,        1280    },  // usVifGrabW
    {1200,      600,        720     },  // usVifGrabH
    #endif
    #if (CHIP == MCR_V2)
    {1,         1,          1       },  // usBayerInGrabX
    {1,         1,          1       },  // usBayerInGrabY
    {0,         0,          1       },  // usBayerInDummyX
    {0,         0,          1       },  // usBayerInDummyY
    {1600,      800,        1280    },  // usBayerOutW
    {1200,      600,        720     },  // usBayerOutH
    #endif
    {1600,      800,        1280    },  // usScalInputW
    {1200,      600,        720     },  // usScalInputH
    {150,       300,        300     },  // usTargetFpsx10
    {1230,      630,        750     },  // usVsyncLine
    {1,         1,          1       },  // ubWBinningN
    {1,         1,          1       },  // ubWBinningM
    {1,         1,          1       },  // ubHBinningN
    {1,         1,          1       },  // ubHBinningM
    {0xFF,     	0xFF,     	0xFF    },  // ubCustIQmode
    {0xFF,     	0xFF,     	0xFF    },  // ubCustAEmode
};

// VIF Setting
static MMPF_SENSOR_VIF_SETTING m_OV2643_VifSetting_Prm =
{
    // SnrType
    MMPF_VIF_SNR_TYPE_YUV,

    // OutInterface
    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
    MMPF_VIF_IF_PARALLEL,
    #elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
    MMPF_VIF_IF_MIPI_SINGLE_0;    
    #elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    MMPF_VIF_IF_MIPI_DUAL_01,
    #else
    MMPF_VIF_IF_MIPI_QUAD,
    #endif

    // VifPadId
    MMPF_VIF_MDL_ID0,

    // powerOnSetting
    {
        MMP_TRUE,       // bTurnOnExtPower
        SENSOR_GPIO_ENABLE,             // usExtPowerPin
        SENSOR_GPIO_ENABLE_ACT_LEVEL,   // bExtPowerPinHigh
        0,              // usExtPowerPinDelay
        MMP_TRUE,       // bFirstEnPinHigh
        10,             // ubFirstEnPinDelay
        MMP_FALSE,      // bNextEnPinHigh
        10,             // ubNextEnPinDelay
        MMP_TRUE,       // bTurnOnClockBeforeRst
        ((SENSOR_RESET_ACT_LEVEL == GPIO_HIGH) ? MMP_TRUE : MMP_FALSE), // bFirstRstPinHigh
        10,             // ubFirstRstPinDelay
        ((SENSOR_RESET_ACT_LEVEL == GPIO_HIGH) ? MMP_FALSE : MMP_TRUE), // bNextRstPinHigh
        10              // ubNextRstPinDelay
    },

    // powerOffSetting
    {
        MMP_FALSE,      // bEnterStandByMode
        0x3D,           // usStandByModeReg
        0x40,           // usStandByModeMask
        MMP_TRUE,       // bEnPinHigh
        20,             // ubEnPinDelay
        MMP_TRUE,       // bTurnOffMClock
        MMP_TRUE,       // bTurnOffExtPower
        MMP_GPIO_MAX    // usExtPowerPin
    },

    // clockAttr
    {
        MMP_TRUE,       // bClkOutEn
        0,              // ubClkFreqDiv
        24000,          // ulMClkFreq
        24000,			// ulDesiredFreq

        MMPF_VIF_SNR_PHASE_DELAY_NONE,  // ubClkPhase
        MMPF_VIF_SNR_CLK_POLARITY_POS,  // ubClkPolarity
        MMPF_VIF_SNR_CLK_SRC_PMCLK      // ubClkSrc
    },

    // paralAttr
    {
        MMPF_VIF_SNR_LATCH_NEG_EDGE,    // ubLatchTiming
        MMPF_VIF_SNR_CLK_POLARITY_POS,  // ubHsyncPolarity
        MMPF_VIF_SNR_CLK_POLARITY_NEG,  // ubVsyncPolarity
        MMPF_VIF_SNR_PARAL_BITMODE_10   // ubBusBitMode
    },

    // mipiAttr
    {
        MMP_FALSE,                      // bClkDelayEn
        MMP_FALSE,                      // bClkLaneSwapEn
        0,                              // usClkDelay
        MMPF_VIF_SNR_LATCH_NEG_EDGE,    // ubBClkLatchTiming
#if (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
        // Lane 0,                      Lane 1,                         Lane 2,                             Lane 3
        {MMP_TRUE,                      MMP_TRUE,                       MMP_TRUE,                           MMP_TRUE},                          // bDataLaneEn
        {MMP_TRUE,                      MMP_TRUE,                       MMP_TRUE,                           MMP_TRUE},                          // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_1,   MMPF_VIF_MIPI_DATA_SRC_PHY_2,       MMPF_VIF_MIPI_DATA_SRC_PHY_3},      // ubDataLaneSrc
        {0,                             0,                              0,                                  0},                                 // usDataDelay
        {0x1F,                          0x1F,                           0x1F,                               0x1F}                               // ubDataSotCnt
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
        // Lane 0,                      Lane 1,                         Lane 2,                             Lane 3
        {MMP_TRUE,                      MMP_TRUE,                       MMP_FALSE,                          MMP_FALSE},                         // bDataLaneEn
        {MMP_TRUE,                      MMP_TRUE,                       MMP_FALSE,                          MMP_FALSE},                         // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_1,   MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF,   MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF},  // ubDataLaneSrc
        {0,                             0,                              0,                                  0},                                 // usDataDelay
        {0x1F,                          0x1F,                           0x1F,                               0x1F}                               // ubDataSotCnt
#elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
        // Lane 0,                      Lane 1,                         Lane 2,                             Lane 3
        {MMP_TRUE,                      MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataLaneEn
        {MMP_TRUE,                      MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF, MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF, MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF},  // ubDataLaneSrc
        {0,                             0,                              0,                                  0},                                 // usDataDelay
        {0x1F,                          0x1F,                           0x1F,                               0x1F}                               // ubDataSotCnt
#endif
    },

    // colorId
    {
        MMPF_VIF_COLORID_00,            // VifColorId
        MMP_FALSE                       // bUseCustomId
    },

    // vcAttr
    {
        MMP_FALSE,                                      	// bEnable
        // MMP_FALSE,                                      	// bAllChannel2Isp
        // {MMP_FALSE, MMP_FALSE, MMP_FALSE, MMP_FALSE},   	// bVC2Isp
        // {MMP_FALSE, MMP_FALSE, MMP_FALSE, MMP_FALSE},  	// bVC2Raw
        // MMP_FALSE                                       	// bSlowFsForStagger
    },

    // yuvAttr
    {
        MMP_TRUE,                                      	    // bRawStoreEnable
        MMP_FALSE,                                      	// bYuv422ToYuv420
        MMP_TRUE,                                      	    // bYuv422ToYuv422
        MMP_FALSE,                                      	// bYuv422ToBayer
        MMPF_VIF_YUV422_UYVY,                           	// ubYuv422Order
    }
};

static MMPF_SENSOR_VIF_SETTING m_OV2643_VifSetting_Scd =
{
    // SnrType
    MMPF_VIF_SNR_TYPE_YUV,

    // OutInterface
    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
    MMPF_VIF_IF_PARALLEL,
    #elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
    MMPF_VIF_IF_MIPI_SINGLE_0;    
    #elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    MMPF_VIF_IF_MIPI_DUAL_01,
    #else
    MMPF_VIF_IF_MIPI_QUAD,
    #endif

    // VifPadId
    MMPF_VIF_MDL_ID0,

    // powerOnSetting
    {
        MMP_TRUE,       // bTurnOnExtPower
        SENSOR_GPIO_ENABLE,             // usExtPowerPin
        SENSOR_GPIO_ENABLE_ACT_LEVEL,   // bExtPowerPinHigh
        0,              // usExtPowerPinDelay
        MMP_TRUE,       // bFirstEnPinHigh
        10,             // ubFirstEnPinDelay
        MMP_FALSE,      // bNextEnPinHigh
        10,             // ubNextEnPinDelay
        MMP_TRUE,       // bTurnOnClockBeforeRst
        ((SENSOR_RESET_ACT_LEVEL == GPIO_HIGH) ? MMP_TRUE : MMP_FALSE), // bFirstRstPinHigh
        10,             // ubFirstRstPinDelay
        ((SENSOR_RESET_ACT_LEVEL == GPIO_HIGH) ? MMP_FALSE : MMP_TRUE), // bNextRstPinHigh
        10              // ubNextRstPinDelay
    },

    // powerOffSetting
    {
        MMP_FALSE,      // bEnterStandByMode
        0x3D,           // usStandByModeReg
        0x40,           // usStandByModeMask
        MMP_TRUE,       // bEnPinHigh
        20,             // ubEnPinDelay
        MMP_TRUE,       // bTurnOffMClock
        MMP_TRUE,       // bTurnOffExtPower
        MMP_GPIO_MAX    // usExtPowerPin
    },

    // clockAttr
    {
        MMP_TRUE,       // bClkOutEn
        0,              // ubClkFreqDiv
        24000,          // ulMClkFreq
        24000,			// ulDesiredFreq

        MMPF_VIF_SNR_PHASE_DELAY_NONE,  // ubClkPhase
        MMPF_VIF_SNR_CLK_POLARITY_POS,  // ubClkPolarity
        MMPF_VIF_SNR_CLK_SRC_PMCLK      // ubClkSrc
    },

    // paralAttr
    {
        MMPF_VIF_SNR_LATCH_NEG_EDGE,    // ubLatchTiming
        MMPF_VIF_SNR_CLK_POLARITY_POS,  // ubHsyncPolarity
        MMPF_VIF_SNR_CLK_POLARITY_NEG,  // ubVsyncPolarity
        MMPF_VIF_SNR_PARAL_BITMODE_10   // ubBusBitMode
    },

    // mipiAttr
    {
        MMP_FALSE,                      // bClkDelayEn
        MMP_FALSE,                      // bClkLaneSwapEn
        0,                              // usClkDelay
        MMPF_VIF_SNR_LATCH_NEG_EDGE,    // ubBClkLatchTiming
#if (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
        // Lane 0,                      Lane 1,                         Lane 2,                             Lane 3
        {MMP_TRUE,                      MMP_TRUE,                       MMP_TRUE,                           MMP_TRUE},                          // bDataLaneEn
        {MMP_TRUE,                      MMP_TRUE,                       MMP_TRUE,                           MMP_TRUE},                          // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_1,   MMPF_VIF_MIPI_DATA_SRC_PHY_2,       MMPF_VIF_MIPI_DATA_SRC_PHY_3},      // ubDataLaneSrc
        {0,                             0,                              0,                                  0},                                 // usDataDelay
        {0x1F,                          0x1F,                           0x1F,                               0x1F}                               // ubDataSotCnt
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
        // Lane 0,                      Lane 1,                         Lane 2,                             Lane 3
        {MMP_TRUE,                      MMP_TRUE,                       MMP_FALSE,                          MMP_FALSE},                         // bDataLaneEn
        {MMP_TRUE,                      MMP_TRUE,                       MMP_FALSE,                          MMP_FALSE},                         // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_1,   MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF,   MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF},  // ubDataLaneSrc
        {0,                             0,                              0,                                  0},                                 // usDataDelay
        {0x1F,                          0x1F,                           0x1F,                               0x1F}                               // ubDataSotCnt
#elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
        // Lane 0,                      Lane 1,                         Lane 2,                             Lane 3
        {MMP_TRUE,                      MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataLaneEn
        {MMP_TRUE,                      MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF, MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF, MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF},  // ubDataLaneSrc
        {0,                             0,                              0,                                  0},                                 // usDataDelay
        {0x1F,                          0x1F,                           0x1F,                               0x1F}                               // ubDataSotCnt
#endif
    },

    // colorId
    {
        MMPF_VIF_COLORID_00,            // VifColorId
        MMP_FALSE                       // bUseCustomId
    },

    // vcAttr
    {
        MMP_FALSE,                                      	// bEnable
        // MMP_FALSE,                                      	// bAllChannel2Isp
        // {MMP_FALSE, MMP_FALSE, MMP_FALSE, MMP_FALSE},   	// bVC2Isp
        // {MMP_FALSE, MMP_FALSE, MMP_FALSE, MMP_FALSE},  	// bVC2Raw
        // MMP_FALSE                                       	// bSlowFsForStagger
    },

    // yuvAttr
    {
        MMP_TRUE,                                      	    // bRawStoreEnable
        MMP_FALSE,                                      	// bYuv422ToYuv420
        MMP_TRUE,                                      	    // bYuv422ToYuv422
        MMP_FALSE,                                      	// bYuv422ToBayer
        MMPF_VIF_YUV422_UYVY,                           	// ubYuv422Order
    }
};

// IQ Table
#if (SENSOR_PROI == PRM_SENSOR)
const ISP_UINT8 Sensor_IQ_CompressedText[] = 
{
#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC)
    #include "isp_8428_iq_data_v3_OV2710_ezmode.xls.ciq.txt"
#else // Use old IQ table
    #include "isp_8428_iq_data_v2_OV2710.xls.ciq.txt"
#endif
};

#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC)     
//Replace it by custom IQ table.
const  __align(4) ISP_UINT8 Sensor_EZ_IQ_CompressedText[] = 
{
//#include "ez_isp_8428_ov4689.txt" //TBD. Wait for ISP release.
//#include "eziq_0413.txt"  //TBD. Wait for ISP release.
//#include "eziq_0509.txt" //customer
NULL 
};

ISP_UINT32 eziqsize = sizeof(Sensor_EZ_IQ_CompressedText);
#endif
#endif

// I2cm Attribute for primary sensor
static MMP_I2CM_ATTR m_OV2643_I2cmAttr_Prm = 
{
    MMP_I2CM0,                  // i2cmID
    SENSOR_I2C_ADDR_OV2643,     // ubSlaveAddr
    8,                          // ubRegLen
    8,                          // ubDataLen
    0,                          // ubDelayTime
    MMP_FALSE,                  // bDelayWaitEn
    MMP_TRUE,                   // bInputFilterEn
    MMP_FALSE,                  // b10BitModeEn
    MMP_FALSE,                  // bClkStretchEn
    0,                          // ubSlaveAddr1
    0,                          // ubDelayCycle
    0,                          // ubPadNum
    250000,                     // ulI2cmSpeed 250KHZ
    MMP_TRUE,                   // bOsProtectEn
    NULL,                       // sw_clk_pin
    NULL,                       // sw_data_pin
    MMP_FALSE,                  // bRfclModeEn
    MMP_FALSE,                  // bWfclModeEn
    MMP_FALSE,                  // bRepeatModeEn
    0                           // ubVifPioMdlId
};

// I2cm Attribute for secondary sensor
static MMP_I2CM_ATTR m_OV2643_I2cmAttr_Scd = 
{
    MMP_I2CM0,                  // i2cmID
    SENSOR_I2C_ADDR_OV2643,     // ubSlaveAddr
    8,                          // ubRegLen
    8,                          // ubDataLen
    0,                          // ubDelayTime
    MMP_FALSE,                  // bDelayWaitEn
    MMP_TRUE,                   // bInputFilterEn
    MMP_FALSE,                  // b10BitModeEn
    MMP_FALSE,                  // bClkStretchEn
    0,                          // ubSlaveAddr1
    0,                          // ubDelayCycle
    0,                          // ubPadNum
    250000,                     // ulI2cmSpeed 250KHZ
    MMP_TRUE,                   // bOsProtectEn
    NULL,                       // sw_clk_pin
    NULL,                       // sw_data_pin
    MMP_FALSE,                  // bRfclModeEn
    MMP_FALSE,                  // bWfclModeEn
    MMP_FALSE,                  // bRepeatModeEn
    0                           // ubVifPioMdlId
};

// 3A Timing
static MMPF_SENSOR_AWBTIMIMG m_OV2643_AwbTime = 
{
    6,  /* ubPeriod */
    1,  /* ubDoAWBFrmCnt */
    3   /* ubDoCaliFrmCnt */
};

static MMPF_SENSOR_AETIMIMG m_OV2643_AeTime = 
{	
    5,  /* ubPeriod */
    0,  /* ubFrmStSetShutFrmCnt */
    0   /* ubFrmStSetGainFrmCnt */
};

static MMPF_SENSOR_AFTIMIMG m_OV2643_AfTime = 
{
    1,  /* ubPeriod */
    0   /* ubDoAFFrmCnt */
};

// IQ Data
#define ISP_IQ_DATA_NAME "isp_8428_iq_data_v2_OV2710.xls.ciq.txt"

static const MMP_UBYTE s_IqCompressData[] = 
{
    #include ISP_IQ_DATA_NAME
};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void ____Sensor_Init_OPR_Table____(){ruturn;} // dummy
#endif

ISP_UINT16 SNR_OV2643_Reg_Init_Customer[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};

#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} // dummy
#endif

ISP_UINT16 SNR_OV2643_Reg_Unsupport[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};

ISP_UINT16 SNR_OV2643_Reg_1600x1200_15P[] = 
{
    /*
     * @@ UXGA(YUV) 15fps
     * 100 99 1600 1200
     * 100 98 0 0
     * 102 3601 5DC
     * ;
     * ;OV2643 setting Version History
     * ;
     * ;
     * ;date 01/12/2011
     * ;--R1B 4th release of OV2643 Rev1B(AA) setting based AB_V02. 
     * ;--Lower internal reference voltage
     * ;
     * ;date 03/31/2009
     * ;--R1B 3rd release of OV2643 Rev1B(AA) setting based AB_V01.
     * ;--Lower the PCLK for SVGA 30fps and CIF 30fps settings
     */

    0x12, 0x80,
    0xc3, 0x1f,
    0xc4, 0xff,
    0x3d, 0x48,
    0xdd, 0x95,
    #if 0
    // windows setup
    0x20, 0x01,
    0x21, 0x91/*0x25*/,
    0x22, 0x00,
    0x23, 0x0c,
    0x24, 0x40/*0x50*//*0x28*//*0x50*/, // 0x500 = 1280
    0x25, 0x08,
    0x26, 0x24/*0x2D*//*0x1E*//*0x2d*/, // 0x2d0 = 720
    0x27, 0x04,
    0x28, 0x42,
    #endif
    0x0e, 0xb4,
    0x10, 0x0a,
    0x11, 0x00,
    0x0f, 0x14,
    0x21, 0x25,
    0x23, 0x0c,

    #if 0 // Rogers: test H Brank
    0x29, 0x10,
    0x2A, 0x00,
    #endif

    0x12, 0x08,
    0x39, 0x10,
    0xcd, 0x12,
    #if 1 // AEC use Banding step
    0x13, 0xEF,
    #else
    0x13, 0xff,
    #endif
    #if 1
    0x14, 0x27,
    #else
    0x14, 0xa7,
    #endif
    0x15, 0x42,
    0x3c, 0xa4,
    #if 1 // Target lum.
    0x18, 0x20,
    0x19, 0x18,
    #else
    0x18, 0x60,
    0x19, 0x50,
    #endif
    0x37, 0xe8,
    0x16, 0x90,
    0x43, 0x00,
    0x40, 0xfb,
    0xa9, 0x44,
    0x2f, 0xec,
    #if 1 // BLC
    0x35, 0x02,
    0x36, 0x02,
    #else
    0x35, 0x10,
    0x36, 0x10,
    #endif
    0x0c, 0x00,
    0x0d, 0x00,
    0xd0, 0x93,
    0xdc, 0x2b,
    0xd9, 0x81,
    0xd3, 0x04,
    0xcc, 0x04,
    0xc7, 0x08,
    0x3d, 0x08,
    0x0c, 0x00,
    0x18, 0x2c,
    0x19, 0x24,
    #if 1 // simple AWB On & stable speed setting
    0x1a, 0x51,
    0x75, 0x6b,
    #else	
    0x1a, 0x71,
    0x75, 0x6a,
    #endif

    #if 1 // weight table
    0x9b, 0x00,
    0x9c, 0x3B,
    0x9d, 0x3B,
    0x9e, 0x00,
    #else
    0x9b, 0x69,
    0x9c, 0x7d,
    0x9d, 0x7d,
    0x9e, 0x69,
    #endif

    0x65, 0x12,
    0x66, 0x20,
    0x67, 0x39,
    0x68, 0x4e,
    0x69, 0x62,
    0x6a, 0x74,
    0x6b, 0x85,
    0x6c, 0x92,
    0x6d, 0x9e,
    0x6e, 0xb2,
    0x6f, 0xc0,
    0x70, 0xcc,
    0x71, 0xe0,
    0x72, 0xee,
    0x73, 0xf6,
    0x74, 0x11,
    0xab, 0x20,
    0xac, 0x5b,
    0xad, 0x05,
    0xae, 0x1b,
    0xaf, 0x76,
    0xb0, 0x90,
    0xb1, 0x90,
    0xb2, 0x8c,
    0xb3, 0x04,
    0xb4, 0x98,
    0x4c, 0x03,
    0x4d, 0x30,
    0x4e, 0x02,
    0x4f, 0x5c,
    0x50, 0x56,
    0x51, 0x00,
    0x52, 0x66,
    0x53, 0x03,
    0x54, 0x30,
    0x55, 0x02,
    0x56, 0x5c,
    0x57, 0x40,
    0x58, 0x00,
    0x59, 0x66,
    0x5a, 0x03,
    0x5b, 0x20,
    0x5c, 0x02,
    0x5d, 0x5c,
    0x5e, 0x3a,
    0x5f, 0x00,
    0x60, 0x66,
    0x41, 0x1f,
    0xb5, 0x01,
    #if 1 // HSB
    0xb6, 0x03,
    0xb7, 0x80,
    0xb8, 0x00,
    0xb9, 0x60,
    0xba, 0x40,
    #else
    0xb9, 0x40,
    0xba, 0x28,
    #endif
    0xbf, 0x0c,
    0xc0, 0x3e,
    0xa3, 0x0a,
    0xa4, 0x0f,
    0xa5, 0x09,
    0xa6, 0x16,
    0x9f, 0x0a,
    0xa0, 0x0f,
    0xa7, 0x0a,
    0xa8, 0x0f,
    0xa1, 0x10,
    0xa2, 0x04,
    0xa9, 0x04,
    0xaa, 0xa6,
    0x76, 0x11,
    0x77, 0x92,
    0x78, 0x21,
    0x79, 0xe1,
    0x7a, 0x02,
    0x7c, 0x05,
    0x7d, 0x08,
    0x7e, 0x08,
    0x7f, 0x7c,
    0x80, 0x58,
    0x81, 0x2a,
    0x82, 0xc5,
    0x83, 0x46,
    0x84, 0x3a,
    0x85, 0x54,
    0x86, 0x44,
    0x87, 0xf8,
    0x88, 0x08,
    0x89, 0x70,
    0x8a, 0xf0,
    0x8b, 0xf0,
    0x90, 0xe3,
    0x93, 0x10,
    0x94, 0x20,
    0x95, 0x10,
    0x96, 0x18
};

ISP_UINT16 SNR_OV2643_Reg_800x600_30P[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};

ISP_UINT16 SNR_OV2643_Reg_1280x720_30P[] = 
{
    /*
     * MCLK = 22MHz, Sensor output:YUV 1280x720@30fps
     */
    0x12, 0x80,
    0xc3, 0x1f,
    0xc4, 0xff,
    0x3d, 0x48,
    0xdd, 0x95,
    0x0e, 0xb4,
    0x10, 0x0a,
    0x11, 0x00,
    0x0f, 0x14,
    0x20, 0x01,
    0x21, 0x25,
    0x22, 0x00,
    0x23, 0x0c,
    0x24, 0x50,
    0x26, 0x2d,
    0x27, 0x04,
    0x29, 0x06,
    0x2a, 0x40,
    0x2b, 0x02,
    0x2c, 0xee,
    0x1d, 0x04,
    0x25, 0x04,
    0x27, 0x04,
    0x28, 0x40,
    0x12, 0x48,
    0x39, 0x10,
    0xcd, 0x12,
    0x13, 0xff,
    #if 1 // set max gain & dummy frame
    0x14, 0xaf,
    0x15, 0x40,
    0x16, 0x80,
    #else // original
    0x14, 0xa7,
    0x15, 0x42,
    0x16, 0x90,
    #endif
    0x3c, 0xa4,
    0x18, 0x60,
    0x19, 0x50,
    0x1a, 0xe2,
    0x37, 0xe8,
    0x43, 0x00,
    #if 1
    0x40, 0xf8, // kris
    #else
    0x40, 0xf8, // kris
    #endif
    0xa9, 0x44,

    0x0c, 0x00,
    0x0d, 0x60,//0x00, YUV Order
    0xd0, 0x93,
    0xdc, 0x2b,
    0xd9, 0x81,
    0xd3, 0x04,
    0xcc, 0x04,
    0xc7, 0x08,
    0x3d, 0x08,
    0x0c, 0x00,
    #if 1
    0x18, 0x1c, // kris
    0x19, 0x16,
    #else // original AEC stable
    0x18, 0x2c,
    0x19, 0x24,
    #endif
    0x1a, 0x71,
    #if 1 // weighting table
    0x9b, 0x00,
    0x9c, 0x3C,
    0x9d, 0xFF,
    0x9e, 0xFF,
    #else
    0x9b, 0x69,
    0x9c, 0x7d,
    0x9d, 0x7d,
    0x9e, 0x69,
    #endif

    #if 1 // kris
    0x65, 0x0A,
    0x66, 0x18,
    0x67, 0x32,
    0x68, 0x49,
    0x69, 0x5F,
    0x6a, 0x72,
    0x6b, 0x84,
    0x6c, 0x91,
    0x6d, 0x9e,
    0x6e, 0xb2,
    0x6f, 0xc0,
    0x70, 0xcc,
    0x71, 0xe0,
    0x72, 0xee,
    0x73, 0xf6,
    0x74, 0x0D,
    #else
    0x65, 0x12,
    0x66, 0x20,
    0x67, 0x39,
    0x68, 0x4e,
    0x69, 0x62,
    0x6a, 0x74,
    0x6b, 0x85,
    0x6c, 0x92,
    0x6d, 0x9e,
    0x6e, 0xb2,
    0x6f, 0xc0,
    0x70, 0xcc,
    0x71, 0xe0,
    0x72, 0xee,
    0x73, 0xf6,
    0x74, 0x11,
    #endif
    0xab, 0x20,
    0xac, 0x5b,
    0xad, 0x05,
    0xae, 0x1b,
    0xaf, 0x76,
    0xb0, 0x90,
    0xb1, 0x90,
    0xb2, 0x8c,
    0xb3, 0x04,
    0xb4, 0x98,
    #if 1 // kris
    0x4c, 0x03,
    0x4d, 0x30,
    0x4e, 0x02,
    0x4f, 0x5c,
    0x50, 0x40,
    0x51, 0x00,
    0x52, 0x66,
    0x53, 0x03,
    0x54, 0x30,
    0x55, 0x02,
    0x56, 0x5c,
    0x57, 0x40,
    0x58, 0x00,
    0x59, 0x66,
    0x5a, 0x03,
    0x5b, 0x20,
    0x5c, 0x02,
    0x5d, 0x5c,
    0x5e, 0x30,
    0x5f, 0x00,
    0x60, 0x66,
    #else
    0x4c, 0x03,
    0x4d, 0x30,
    0x4e, 0x02,
    0x4f, 0x5c,
    0x50, 0x56,
    0x51, 0x00,
    0x52, 0x66,
    0x53, 0x03,
    0x54, 0x30,
    0x55, 0x02,
    0x56, 0x5c,
    0x57, 0x40,
    0x58, 0x00,
    0x59, 0x66,
    0x5a, 0x03,
    0x5b, 0x20,
    0x5c, 0x02,
    0x5d, 0x5c,
    0x5e, 0x3a,
    0x5f, 0x00,
    0x60, 0x66,
    #endif
    0x41, 0x1f, // SDE_EN
    0xb5, 0x01,
    #if 1
    0xb6, 0x02, // saturation enable
    0xb5, 0x01,
    #else
    0xb6, 0x02,
    #endif
    0xb9, 0x40,
    0xba, 0x28,
    0xbf, 0x0c,
    0xc0, 0x3e,
    #if 1
    0xa3, 0x0a,
    0xa4, 0x0f,
    0xa5, 0x0c,
    0xa6, 0x20,
    #else
    0xa3, 0x0a,
    0xa4, 0x0f,
    0xa5, 0x09,
    0xa6, 0x16,
    #endif
    0x9f, 0x0a,
    0xa0, 0x0f,
    0xa7, 0x0a,
    0xa8, 0x0f,
    0xa1, 0x10,
    0xa2, 0x04,
    0xa9, 0x04,
    0xaa, 0xa6,
    #if 1 // kris
    0x75, 0x68,
    0x76, 0x11,
    0x77, 0x92,
    0x78, 0x21,
    0x79, 0xe1,
    0x7a, 0x02,
    0x7c, 0x0F,
    0x7d, 0x10,
    0x7e, 0x17,
    0x7f, 0x7c,
    0x80, 0x58,
    0x81, 0x57,
    0x82, 0xBB,
    0x83, 0x35,
    0x84, 0x36,
    0x85, 0x6B,
    0x86, 0x3C,
    0x87, 0xf8,
    0x88, 0x08,
    0x89, 0x70,
    0x8a, 0xf0,
    0x8b, 0xf0,
    #else
    0x75, 0x6a,
    0x76, 0x11,
    0x77, 0x92,
    0x78, 0x21,
    0x79, 0xe1,
    0x7a, 0x02,
    0x7c, 0x05,
    0x7d, 0x08,
    0x7e, 0x08,
    0x7f, 0x7c,
    0x80, 0x58,
    0x81, 0x2a,
    0x82, 0xc5,
    0x83, 0x46,
    0x84, 0x3a,
    0x85, 0x54,
    0x86, 0x44,
    0x87, 0xf8,
    0x88, 0x08,
    0x89, 0x70,
    0x8a, 0xf0,
    0x8b, 0xf0,
    #endif
    0x90, 0xe3,
    0x93, 0x10,
    0x94, 0x20,
    0x95, 0x10,
    0x96, 0x18,
    0x0f, 0x14,
           
    // PLL modif
    0x0e, 0xb6,
    0x0f, 0x14,
    0x10, 0x09,
    0x11, 0x00,
    0x12, 0x48,
    0x29, 0x06,
    0x2a, 0x5e,
    #if 1 // Bend steps 60Hz (Taiwan)
    0x1e, 0x01, 
    0x1f, 0x77,
    #else // 50Hz 
    0x1e, 0x00,
    0x1f, 0xE1,
    #endif

    0x2f, 0xec,
    0x35, 0x10,
    0x36, 0x10
};

// OPR Table Setting
static MMPF_SENSOR_OPR_TABLE m_OV2643_OprTable = 
{
    // usInitSize
    (sizeof(SNR_OV2643_Reg_Init_Customer)/sizeof(SNR_OV2643_Reg_Init_Customer[0]))/2,

    // uspInitTable
    &SNR_OV2643_Reg_Init_Customer[0],

    MMP_FALSE,  // bBinTableExist
    0,          // usBinTableNum
    {0},        // usBinRegAddr
    {0},        // usBinSize
    {NULL},     // ubBinTable

    MMP_FALSE,  // bInitDoneTableExist
    0,          // usInitDoneSize
    NULL,       // uspInitDoneTable

    // usSize
    {
        (sizeof(SNR_OV2643_Reg_1600x1200_15P)/sizeof(SNR_OV2643_Reg_1600x1200_15P[0]))/2,
        (sizeof(SNR_OV2643_Reg_800x600_30P)/sizeof(SNR_OV2643_Reg_800x600_30P[0]))/2,
        (sizeof(SNR_OV2643_Reg_1280x720_30P)/sizeof(SNR_OV2643_Reg_1280x720_30P[0]))/2
    },

    // uspTable
    {
        &SNR_OV2643_Reg_1600x1200_15P[0],
        &SNR_OV2643_Reg_800x600_30P[0],
        &SNR_OV2643_Reg_1280x720_30P[0],
    }
};

#if 0
void ____Sensor_Customer_Func____(){ruturn;} // dummy
#endif

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_InitConfig
//  Description :
//------------------------------------------------------------------------------
static void SNR_OV2643_Cust_InitConfig(void)
{
    RTNA_DBG_Str(0, "SNR_Cust_InitConfig OV2643\r\n");
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
void SNR_OV2643_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
void SNR_OV2643_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_DoAWB_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_OV2643_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_DoAWB_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_OV2643_Cust_DoAWB_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_DoIQ
//  Description :
//------------------------------------------------------------------------------
void SNR_OV2643_Cust_DoIQ(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_SetGain
//  Description :
//------------------------------------------------------------------------------
void SNR_OV2643_Cust_SetGain(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
void SNR_OV2643_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_SetExposure
//  Description :
//------------------------------------------------------------------------------
static void SNR_OV2643_Cust_SetExposure(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain, MMP_ULONG ulShutter, MMP_ULONG ulVsync)
{
    //TBD
    printc(FG_RED("Warning!!! Please review SNR_Cust_SetExposure in sensor driver!!!\r\n"));
    gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, ulShutter, ulVsync);
    gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, ulGain);
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_SetFlip
//  Description :
//------------------------------------------------------------------------------
void SNR_OV2643_Cust_SetFlip(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_SetRotate
//  Description :
//------------------------------------------------------------------------------
void SNR_OV2643_Cust_SetRotate(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_CheckVersion
//  Description :
//------------------------------------------------------------------------------
void SNR_OV2643_Cust_CheckVersion(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_GetIqCompressData
//  Description :
//------------------------------------------------------------------------------
const MMP_UBYTE* SNR_OV2643_Cust_GetIqCompressData(MMP_UBYTE ubSnrSel)
{
    return s_IqCompressData;
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_StreamEnable
//  Description : Enable/Disable streaming of sensor
//------------------------------------------------------------------------------
static void SNR_OV2643_Cust_StreamEnable(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_CheckSnrTVSrcType
//  Description :
//------------------------------------------------------------------------------
static void SNR_OV2643_Cust_CheckSnrTVSrcType(MMP_UBYTE ubSnrSel, MMP_SNR_TVDEC_SRC_TYPE *TVSrcType)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_OV2643_Cust_GetSnrID
//  Description :
//------------------------------------------------------------------------------
static void SNR_OV2643_Cust_GetSnrID(MMP_UBYTE ubSnrSel, MMP_ULONG *SensorID)
{
    *SensorID = 0xFFFF2643;
}

static MMPF_SENSOR_CUSTOMER SensorCustFunc_Prm = 
{
    SNR_OV2643_Cust_InitConfig,
    SNR_OV2643_Cust_DoAE_FrmSt,
    SNR_OV2643_Cust_DoAE_FrmEnd,
    SNR_OV2643_Cust_DoAWB_FrmSt,
    SNR_OV2643_Cust_DoAWB_FrmEnd,
    SNR_OV2643_Cust_DoIQ,
    SNR_OV2643_Cust_SetGain,
    SNR_OV2643_Cust_SetShutter,
    SNR_OV2643_Cust_SetExposure,
    SNR_OV2643_Cust_SetFlip,
    SNR_OV2643_Cust_SetRotate,
    SNR_OV2643_Cust_CheckVersion,
    SNR_OV2643_Cust_GetIqCompressData,
    SNR_OV2643_Cust_StreamEnable,

    &m_OV2643_SensorRes,
    &m_OV2643_OprTable,
    &m_OV2643_VifSetting_Prm,
    &m_OV2643_I2cmAttr_Prm,
    &m_OV2643_AwbTime,
    &m_OV2643_AeTime,
    &m_OV2643_AfTime,
    MMP_SNR_PRIO_PRM,
    
    SNR_OV2643_Cust_CheckSnrTVSrcType,
    SNR_OV2643_Cust_GetSnrID    
};

static MMPF_SENSOR_CUSTOMER SensorCustFunc_Scd = 
{
    SNR_OV2643_Cust_InitConfig,
    SNR_OV2643_Cust_DoAE_FrmSt,
    SNR_OV2643_Cust_DoAE_FrmEnd,
    SNR_OV2643_Cust_DoAWB_FrmSt,
    SNR_OV2643_Cust_DoAWB_FrmEnd,
    SNR_OV2643_Cust_DoIQ,
    SNR_OV2643_Cust_SetGain,
    SNR_OV2643_Cust_SetShutter,
    SNR_OV2643_Cust_SetExposure,
    SNR_OV2643_Cust_SetFlip,
    SNR_OV2643_Cust_SetRotate,
    SNR_OV2643_Cust_CheckVersion,
    SNR_OV2643_Cust_GetIqCompressData,
    SNR_OV2643_Cust_StreamEnable,

    &m_OV2643_SensorRes,
    &m_OV2643_OprTable,
    &m_OV2643_VifSetting_Scd,
    &m_OV2643_I2cmAttr_Scd,
    &m_OV2643_AwbTime,
    &m_OV2643_AeTime,
    &m_OV2643_AfTime,
    MMP_SNR_PRIO_SCD,
    
    SNR_OV2643_Cust_CheckSnrTVSrcType,
    SNR_OV2643_Cust_GetSnrID    
};

#if (SENSOR_PROI == PRM_SENSOR)
int SNR_Module_Init(void)
{
    if (SensorCustFunc_Prm.sPriority == MMP_SNR_PRIO_PRM)
        MMPF_SensorDrv_Register(PRM_SENSOR, &SensorCustFunc_Prm);
    else
        MMPF_SensorDrv_Register(SCD_SENSOR, &SensorCustFunc_Prm);

#if defined(BIND_SENSOR_OV2643_2ND) && (BIND_SENSOR_OV2643_2ND)
    MMPF_SensorDrv_Register(SCD_SENSOR, &SensorCustFunc_Scd);
#endif

    return 0;
}
#else
int SubSNR_Module_Init(void)
{
#if !defined(MBOOT_FW)
    MMPF_SensorDrv_Register(SCD_SENSOR, &SensorCustFunc_Scd);
#endif        
    return 0;
}
#endif

#pragma arm section code = "initcall6", rodata = "initcall6", rwdata = "initcall6", zidata = "initcall6"
#pragma O0
#if (SENSOR_PROI == PRM_SENSOR)
ait_module_init(SNR_Module_Init);
#else
ait_module_init(SubSNR_Module_Init);
#endif
#pragma
#pragma arm section rodata, rwdata, zidata

#endif // (BIND_SENSOR_OV2643)
#endif // (SENSOR_EN)
