//==============================================================================
//
//  File        : sensor_OV2735.c
//  Description : Firmware Sensor Control File
//  Author      : Philip Lin
//  Revision    : 1.0
//
//==============================================================================

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "includes_fw.h"
#include "customer_config.h"

#if (SENSOR_EN)
#if (BIND_SENSOR_OV2735)

#include "mmpf_sensor.h"
#include "Sensor_Mod_Remapping.h"
#include "isp_if.h"
//#include "mmpf_i2cm.h"
//#include "hdr_cfg.h"
// Customer Defined Index (Large to Small)
#define RES_IDX_1920x1080   (0)
#define RES_IDX_1280x720    (1)

#define SENSOR_ROTATE_180   (0)

#define MAX_SENSOR_GAIN     (16)          // 16x

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================

// Resolution Table
MMPF_SENSOR_RESOLUTION m_SensorRes =
{
    2,				// ubSensorModeNum
	0,				// ubDefPreviewMode
    0,				// ubDefCaptureMode
	2200,           // usPixelSize
//  Mode0	Mode1   Mode2	Mode3	Mode4
    {1,        1},	// usVifGrabStX
    {1,        1},	// usVifGrabStY
    #if (PARALLEL_SENSOR_CROP_2M == 1)
    {1928,  1288},	// usVifGrabW
    {1088,   724},	// usVifGrabH
    #else
    {1928,  1288},	// usVifGrabW
    {1088,   724},	// usVifGrabH
    #endif
    
    #if (CHIP == MCR_V2)
    {1,   	   1},  // usBayerInGrabX
    {1,  	   1},  // usBayerInGrabY
    #if (PARALLEL_SENSOR_CROP_2M == 1)
    {8,        8},  // usBayerDummyInX
    {8,        4},  // usBayerDummyInY
    {1920,  1280},	// usBayerOutW
    {1080,   720},	// usBayerOutH
    #else
    {8,        8},  // usBayerDummyInX
    {8,        4},  // usBayerDummyInY
    {1920,  1280},	// usBayerOutW
    {1080,   720},	// usBayerOutH
    #endif
    #endif

    #if (PARALLEL_SENSOR_CROP_2M == 1)
    {1920,  1280},	// usScalInputW
    {1080,   720},	// usScalInputH
    #else
    {1920,  1280},	// usScalInputW
    {1080,   720},	// usScalInputH
    #endif
    
	#if (SENSOR_IF == SENSOR_IF_PARALLEL) || (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    {300,    600},	// usTargetFpsx10
	#else
    {600, 	 250},	// usTargetFpsx10
	#endif
    #if (PARALLEL_SENSOR_CROP_2M == 1)
    {1218,   768},	// usVsyncLine
    #else
    {1218,   768},	// usVsyncLine
    #endif
    {1,		   1},  // ubWBinningN
    {1,		   1},  // ubWBinningN
    {1,		   1},  // ubWBinningN
    {1,		   1},  /* ubWBinningN */
    {0xFF, 0xFF},   // ubCustIQmode
    {0xFF, 0xFF},   // ubCustAEmode
};

// VIF Setting
//MMPF_SENSOR_OPR_TABLE       m_OprTable;
static MMPF_SENSOR_VIF_SETTING m_VifSetting_Prm =
{
	// SnrType
    MMPF_VIF_SNR_TYPE_BAYER,

    // OutInterface
    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
    MMPF_VIF_IF_PARALLEL,
    #elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    MMPF_VIF_IF_MIPI_DUAL_01,
    #else
    MMPF_VIF_IF_MIPI_QUAD,
    #endif

    // VifPadId
    #if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
    MMPF_VIF_MDL_ID1,
    #else
    MMPF_VIF_MDL_ID0,
    #endif

    // powerOnSetting
    {
        MMP_TRUE,       // bTurnOnExtPower
        MMP_GPIO_MAX,   // usExtPowerPin
        MMP_FALSE,      // bExtPowerPinHigh
        0,              // usExtPowerPinDelay
        MMP_TRUE,       // bFirstEnPinHigh
        1,              // ubFirstEnPinDelay
        MMP_FALSE,      // bNextEnPinHigh
        10,             // ubNextEnPinDelay
        MMP_TRUE,       // bTurnOnClockBeforeRst
        MMP_FALSE,      // bFirstRstPinHigh
        10,             // ubFirstRstPinDelay
        MMP_TRUE,       // bNextRstPinHigh
        20              // ubNextRstPinDelay
    },

    // powerOffSetting
    {
        MMP_FALSE,      // bEnterStandByMode
        0x0,            // usStandByModeReg
        0x0,            // usStandByModeMask
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
        MMPF_VIF_SNR_LATCH_POS_EDGE,    // ubLatchTiming
        MMPF_VIF_SNR_CLK_POLARITY_POS,  // ubHsyncPolarity
        MMPF_VIF_SNR_CLK_POLARITY_NEG,  // ubVsyncPolarity
        MMPF_VIF_SNR_PARAL_BITMODE_16   // ubBusBitMode
    },

    // mipiAttr
    {
        MMP_FALSE,                      // bClkDelayEn
        MMP_FALSE,                      // bClkLaneSwapEn
        0,                              // usClkDelay
        MMPF_VIF_SNR_LATCH_NEG_EDGE,    // ubBClkLatchTiming
#if (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
        // Lane 0,                      Lane 1,                             Lane 2,                             Lane 3
        {MMP_TRUE,                      MMP_TRUE,                           MMP_TRUE,                           MMP_TRUE},                          // bDataLaneEn
        {MMP_TRUE,                      MMP_TRUE,                           MMP_TRUE,                           MMP_TRUE},                          // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                          MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_1,       MMPF_VIF_MIPI_DATA_SRC_PHY_2,       MMPF_VIF_MIPI_DATA_SRC_PHY_3},      // ubDataLaneSrc
        {0,                             0,                                  0,                                  0},                                 // usDataDelay
        {0x1F,                          0x1F,                               0x1F,                               0x1F}                               // ubDataSotCnt

#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
        // Lane 0,                      Lane 1,                             Lane 2,                             Lane 3
        {MMP_TRUE,                      MMP_TRUE,                           MMP_FALSE,                          MMP_FALSE},                         // bDataLaneEn
        {MMP_TRUE,                      MMP_TRUE,                           MMP_FALSE,                          MMP_FALSE},                         // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                          MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_1,       MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF,   MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF},  // ubDataLaneSrc
        {0x08,                          0x08,                               0x08,                               0x08},                              // usDataDelay
        {0x2F,                          0x2F,                               0x2F,                               0x2F}                               // ubDataSotCnt
#elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
        // Lane 0,                      Lane 1,                             Lane 2,                             Lane 3
        {MMP_TRUE,                      MMP_FALSE,                          MMP_FALSE,                          MMP_FALSE},                         // bDataLaneEn
        {MMP_TRUE,                      MMP_FALSE,                          MMP_FALSE,                          MMP_FALSE},                         // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                          MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF,   MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF,   MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF},  // ubDataLaneSrc
        {0x08,                          0x08,                               0x08,                               0x08},                              // usDataDelay
        {0x20,                          0x20,                               0x20,                               0x20}                               // ubDataSotCnt
#endif
    },

    // colorId
    {
        MMPF_VIF_COLORID_11,            // VifColorId
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
        MMP_FALSE,                                      	// bRawStoreEnable
        // MMP_FALSE,                                      	// bYuv422ToYuv420
        // MMP_FALSE,                                      	// bYuv422ToYuv422
        // MMP_FALSE,                                      	// bYuv422ToBayer
        // MMPF_VIF_YUV422_YUYV,                           	// ubYuv422Order
    }
};

#if defined(BIND_SENSOR_OV2735_2ND) && (BIND_SENSOR_OV2735_2ND)
static MMPF_SENSOR_VIF_SETTING m_VifSetting_Scd =
{
};
#endif

// IQ Table
const ISP_UINT8 Sensor_IQ_CompressedText[] = 
{
#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC)
#include "isp_8428_iq_data_OV2735_Mina_20161118.xls.ciq.txt"
#else //Use old IQ table
#include "isp_8428_iq_data_OV2735_Mina_20161118.xls.ciq.txt"
#endif
};

#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC)
//Replace it by custom IQ table.
const  __align(4) ISP_UINT8 Sensor_EZ_IQ_CompressedText[] = 
{
NULL
};

ISP_UINT32 eziqsize = sizeof(Sensor_EZ_IQ_CompressedText);
#endif

// I2cm Attribute for primary sensor
static MMP_I2CM_ATTR m_I2cmAttr_Prm =
{
    MMP_I2CM0, 	// i2cmID
#if (SENSOR_IF == SENSOR_IF_PARALLEL)
    (0x78 >> 1),       // ubSlaveAddr (0x7A >> 1)
#else
    (0x78 >> 1),       // ubSlaveAddr (0x7A >> 1)
#endif
    8,          // ubRegLen
    8,          // ubDataLen
    0,          // ubDelayTime
    MMP_FALSE,  // bDelayWaitEn
    MMP_TRUE,   // bInputFilterEn
    MMP_FALSE,  // b10BitModeEn
    MMP_FALSE,  // bClkStretchEn
    0,          // ubSlaveAddr1
    0,          // ubDelayCycle
    0,          // ubPadNum
    400000,     // ulI2cmSpeed 400KHZ
    MMP_FALSE,  // bOsProtectEn
    NULL,       // sw_clk_pin
    NULL,       // sw_data_pin
    MMP_FALSE,  // bRfclModeEn
	MMP_FALSE,  // bWfclModeEn
	MMP_FALSE,	// bRepeatModeEn
    0           // ubVifPioMdlId
};

// I2cm Attribute for secondary sensor
static MMP_I2CM_ATTR m_I2cmAttr_Scd =
{
};

// 3A Timing
static MMPF_SENSOR_AWBTIMIMG	m_AwbTime =
{
	6,	// ubPeriod
	1, 	// ubDoAWBFrmCnt
	3	// ubDoCaliFrmCnt
};

static MMPF_SENSOR_AETIMIMG	m_AeTime =
{
    4, 	// ubPeriod
    //0, 	// ubFrmEndGetAccFrmCnt
	0, 	// ubFrmStSetShutFrmCnt
	0	// ubFrmStSetGainFrmCnt
};

static MMPF_SENSOR_AFTIMIMG	m_AfTime =
{
	1, 	// ubPeriod
	0	// ubDoAFFrmCnt
};


#if (ISP_EN)
    static ISP_UINT32 s_gain;
#endif

// IQ Data
#define ISP_IQ_DATA_NAME "isp_8428_iq_data_OV2735_Mina_20161118.xls.ciq.txt"

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
void ____Sensor_Init_OPR_Table____(){ruturn;} //dummy
#endif

#if 1//(SENSOR_IF == SENSOR_IF_PARALLEL)

static ISP_UINT16 SNR_OV2735_Reg_Init_Customer[] =
{
    SENSOR_DELAY_REG, 10, // delay
    };

    // 720p 60FPS
    MMP_USHORT SNR_OV2735_Reg_1280x720_Customer[] =
    {
    0xfd, 0x00,  // page 0
    0x20, 0x00,  // [0]: reset
    SENSOR_DELAY_REG, 20, // delay
    0x20, 0x01,  // [0]: reset

    // PLL output = [24MHz/(pll_mc+1)] * [(pll_nc+3)/(pll_outdiv+1)]
    0xfd, 0x00,  // page 0
    0x2f, 0x10,  // PLL_CTRL_BUF, [1:0]: pll_mc, [6:2]: pll_nc, [7]: pll_clk_sel
    0x34, 0x00,  // BUF_PLL_OUTDIV
    0x30, 0x1f,  // CLK_MODE_BUF, [6:4]: 001:Pclk = pll_clk/2
    0x33, 0x01,  
    0x35, 0x20,  

    0xfd, 0x01,  // page 1
    0x31, 0x01,  
    0x0d, 0x00,  // [4]: Frame_exp_seperate_en  
    0x30, 0x00,  
    0x03, 0x01,  // BUF_EXP[15:8]  
    0x04, 0xd5,  // BUF_EXP[7:0]  
    0x09, 0x00,  // HBLANK[11:8]  
    0x0a, 0x80,  // HBLANK[7:0]
    0x06, 0x0a,  // VBLANK[7:0]
    0x24, 0x10,  
    0x01, 0x01,  // [0]: Enable of Frame Sync Signal
    0xfb, 0x73,  // [0]: BLC enable, [2:1]: BLC mode

    // Timing control
    0xfd, 0x01,  // page 1
    0x1a, 0x6b,  
    0x1c, 0xea,  
    0x16, 0x0c,  
    0x21, 0x00,  
    0x25, 0xe0,  
    0x11, 0x56,  
    0x19, 0xc3,  
    0x29, 0x01,  
    0x33, 0x5f,  
    0x2a, 0xea,  
    0x2c, 0x40,  
    0xd0, 0x02,  
    0xd1, 0x01,  
    0xd2, 0x20,  
    0xd3, 0x04,  
    0xd4, 0x2a,  
    0x50, 0x00,  
    0x51, 0x2c,  
    0x52, 0x29,  
    0x53, 0x00,  
    0x55, 0x46,  
    0x58, 0x29,  
    0x5a, 0x00,  
    0x5b, 0x00,  
    0x5d, 0x00,  
    0x64, 0x2f,  
    0x66, 0x62,  
    0x68, 0x5b,  
    0x75, 0x46,  
    0x76, 0x36,  
    0x77, 0x4f,  
    0x78, 0xef,  
    0x72, 0xbf,  
    0x73, 0x36,  
    0x7d, 0x0d,  
    0x7e, 0x0d,  
    0x8a, 0x77,  
    0x8b, 0x77,  

    0xfd, 0x01,  // page 1
    0xb1, 0x83,  //DPHY enable
    0xb4, 0x15,  //MIPI PLL enable
    0xb5, 0x30,  
    0x9d, 0x40,  
    0xa1, 0x03,  //mipi tx speed
    0x96, 0x1a,  
    0x95, 0x44,  
    0x94, 0x44,  
    0xa0, 0x01,  //mipi enable
    0x8e, 0x05,  
    0x8f, 0x08,  
    0x90, 0x02,  
    0x91, 0xd8,  

    0xfd, 0x01,  // page 1  
    0xf0, 0x40,  // black level offset, Gb[7:0]
    0xf1, 0x40,  // black level offset, B[7:0]
    0xf2, 0x40,  // black level offset, R[7:0]
    0xf3, 0x40,  // black level offset, Gr[7:0]

    //crop to 1280x720
    0xfd, 0x02,  // page 2
    0xa0, 0x00,  // VStart[10:8], 
    0xa1, 0x04,  // VStart[7:0]
    0xa2, 0x02,  // VSize[10:8], 724
    0xa3, 0xd4,  // VSize[7:0]
    0xa4, 0x00,  // HStart[10:8]
    0xa5, 0x02,  // HStart[7:0]
    0xa6, 0x02,  // Half HSize[10:8], 644
    0xa7, 0x84,  // Half HSize[7:0]

    0xfd, 0x01,  // page 1 
    0x8e, 0x05,  // H_SIZE_MIPI[11:8], 1288
    0x8f, 0x08,  // H_SIZE_MIPI[7:0]
    0x90, 0x02,  // V_SIZE_MIPI[11:8], 724
    0x91, 0xd4,  // V_SIZE_MIPI[7:0]

    0xfd, 0x01,  // page 1
    0x0d, 0x10,	 // manual modify the VTS
    0x0e, 0x03,  // frame_length[15:8]
    0x0f, 0x00,  // frame_length[7:0], Vblank, VTS:0x2e8, 60.037fps
    0x01, 0x01,  // [0]: Enable of Frame Sync Signal

    };

    // 1080p 30FPS
    MMP_USHORT SNR_OV2735_Reg_1920x1080_Customer[] =
    {
    0xfd, 0x00,  // page 0
    0x20, 0x00,  // [0]: reset
    SENSOR_DELAY_REG, 20, // delay
    0x20, 0x01,  // [0]: reset

    // PLL output = [24MHz/(pll_mc+1)] * [(pll_nc+3)/(pll_outdiv+1)]
    0xfd, 0x00,  // page 0
    0x2f, 0x10,  // PLL_CTRL_BUF, [1:0]: pll_mc, [6:2]: pll_nc, [7]: pll_clk_sel
    0x34, 0x00,  // BUF_PLL_OUTDIV
    0x30, 0x1f,  // CLK_MODE_BUF, [6:4]: 001:Pclk = pll_clk/2
    0x33, 0x01,  
    0x35, 0x20,  

    0xfd, 0x01,  // page 1
    0x0d, 0x00,  // [4]: Frame_exp_seperate_en
    0x30, 0x00,  
    0x03, 0x01,  // BUF_EXP[15:8]
    0x04, 0x6d,  // BUF_EXP[7:0]
    0x09, 0x00,  // HBLANK[11:8]
    0x0a, 0x80,  // HBLANK[7:0]
    0x06, 0x0a,  // VBLANK[7:0]
    0x24, 0x10,  
    0x01, 0x01,  // [0]: Enable of Frame Sync Signal
    0xfb, 0x73,  // [0]: BLC enable, [2:1]: BLC mode

    // Timing control
    0xfd, 0x01,  // page 1
    0x1a, 0x6b,  
    0x1c, 0xea,  
    0x16, 0x0c,  
    0x21, 0x00,  
    0x11, 0x63,  
    0x19, 0xc3,  
    0x29, 0x01,  
    0x33, 0x6f,  
    0x2a, 0xea,  
    0x2c, 0x40,  
    0xd0, 0x02,  
    0xd1, 0x01,  
    0xd2, 0x20,  
    0xd3, 0x04,  
    0xd4, 0x2a,  
    0x50, 0x00,  
    0x51, 0x2c,  
    0x52, 0x29,  
    0x53, 0x00,  
    0x55, 0x44,  
    0x58, 0x29,  
    0x5a, 0x00,  
    0x5b, 0x00,  
    0x5d, 0x00,  
    0x64, 0x2f,  
    0x66, 0x62,  
    0x68, 0x5b,  
    0x75, 0x46,  
    0x76, 0x36,  
    0x77, 0x4f,  
    0x78, 0xef,  
    0x72, 0xcf,  
    0x73, 0x36,  
    0x7d, 0x0d,  
    0x7e, 0x0d,  
    0x8a, 0x77,  
    0x8b, 0x77,  

    0xfd, 0x01,  // page 1
    0xb1, 0x83,  //DPHY enable
    0xb3, 0x0b,  //0x0b
    0xb4, 0x14,  //MIPI PLL enable;14
    0x9d, 0x40,  
    0xa1, 0x03,  //mipi tx speed
    0xb5, 0x50,  
    0xa0, 0x01,  //mipi enable
    0x25, 0xe0,  
    0x20, 0x7b,  

    0xfd, 0x01,  // page 1
    0xf0, 0x40,  // black level offset, Gb[7:0]
    0xf1, 0x40,  // black level offset, B[7:0]
    0xf2, 0x40,  // black level offset, R[7:0]
    0xf3, 0x40,  // black level offset, Gr[7:0]

    //crop to 1928x1092
    0xfd, 0x02,	 // page 2
    0xa0, 0x00,	 // VStart[10:8], 
    0xa1, 0x04, //0x08,	 // VStart[7:0]
    0xa2, 0x04,	 // VSize[10:8], 1092
    0xa3, 0x44, //0x38,	 // VSize[7:0]
    0xa4, 0x00,  // HStart[10:8]
    0xa5, 0x04, //0x08,	 // HStart[7:0]
    0xa6, 0x03,  // Half HSize[10:8], 964 * 2
    0xa7, 0xc4,	 // Half HSize[7:0]

    0xfd, 0x01,  // page 1
    0x8e, 0x07,  // H_SIZE_MIPI[11:8], 1928
    0x8f, 0x88,	 // H_SIZE_MIPI[7:0]
    0x90, 0x04,	 // V_SIZE_MIPI[11:8], 1092
    0x91, 0x44,  // V_SIZE_MIPI[7:0]

    0xfd, 0x01,  // page 1
    0x0d, 0x10,	 // manual modify the VTS
    0x0e, 0x04,  // frame_length[15:8]
    0x0f, 0xc2, //0xc1,	 // frame_length[7:0], Vblank, VTS:0x4c1, 30.037fps
    0x01, 0x01,  // [0]: Enable of Frame Sync Signal

};
#endif

// OPR Table Setting
static MMPF_SENSOR_OPR_TABLE m_OprTable = 
{
    // usInitSize
    (sizeof(SNR_OV2735_Reg_Init_Customer) / sizeof(SNR_OV2735_Reg_Init_Customer[0]))/2,

    //uspInitTable
    &SNR_OV2735_Reg_Init_Customer[0],

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
        (sizeof(SNR_OV2735_Reg_1920x1080_Customer)/sizeof(SNR_OV2735_Reg_1920x1080_Customer[0]))/2,
        (sizeof(SNR_OV2735_Reg_1280x720_Customer)/sizeof(SNR_OV2735_Reg_1280x720_Customer[0]))/2,
    },

    // uspTable
    {
        &SNR_OV2735_Reg_1920x1080_Customer[0],
        &SNR_OV2735_Reg_1280x720_Customer[0],
    }
};

#if 0
void ____Sensor_Customer_Func____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_InitConfig
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_InitConfig(void)
{
	// TBD
}

static ISP_UINT32 dgain;
static ISP_UINT32 dgainbase = 0x200;

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    MMP_ULONG   ulVsync = 0;
    MMP_ULONG   ulShutter = 0;
    MMP_UBYTE   ubPeriod              = (SensorCustFunc.pAeTime)->ubPeriod;
    MMP_UBYTE   ubFrmStSetShutFrmCnt  = (SensorCustFunc.pAeTime)->ubFrmStSetShutFrmCnt;
    MMP_UBYTE   ubFrmStSetGainFrmCnt  = (SensorCustFunc.pAeTime)->ubFrmStSetGainFrmCnt;

	if (ulFrameCnt < 1)	
    {        
	    return;
	}

    if(ulFrameCnt % ubPeriod == ubFrmStSetShutFrmCnt)
    {

        ISP_IF_AE_Execute();

        s_gain = VR_MAX(ISP_IF_AE_GetGain(), ISP_IF_AE_GetGainBase());

        if (s_gain >= ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN)
        {
            dgain = s_gain * dgainbase / (ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN);
            s_gain  = ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN;
            //  RTNA_DBG_Str(0,"ssssssssssssssssssssssssssss\r\n");
        }
        else
        {
            dgain  = dgainbase;
            //    RTNA_DBG_Str(0,"aaaaaaaaaaaaaaaaaaaaaaaaa\r\n");
        }

        ulVsync     = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetVsync() / ISP_IF_AE_GetVsyncBase();
        ulShutter   = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetShutter() / ISP_IF_AE_GetShutterBase();

        gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, ulShutter, ulVsync);
        gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, s_gain);

    }
    else if(ulFrameCnt % ubPeriod == ubFrmStSetShutFrmCnt + 1)
    {
        ISP_IF_IQ_SetAEGain(dgain, dgainbase);
    }
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAWB_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAWB_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_DoAWB_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoIQ
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_DoIQ(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetGain
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_SetGain(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain)
{
#if (ISP_EN)

	ISP_UINT32 sensor_gain;

	sensor_gain = VR_MIN(32 * VR_MAX(ulGain, ISP_IF_AE_GetGainBase()) / ISP_IF_AE_GetGainBase() , 511);
		
   	//dbg_printf(0, "G:0x%x\r\n", sensor_gain);

    //gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0xfd,  0x01);  // page 1
    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x38, (ISP_UINT8)((sensor_gain >> 8) & 0x01));
    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x24, (ISP_UINT8)(sensor_gain & 0xff));

    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x01,  0x01);  // [0]: Enable of Frame Sync Signal

#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
    ISP_UINT32 new_vsync    = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetVsync() / ISP_IF_AE_GetVsyncBase();
    ISP_UINT32 new_shutter  = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetShutter() / ISP_IF_AE_GetShutterBase();

    new_vsync   = ISP_MIN(ISP_MAX(new_shutter + 5, new_vsync), 0xFFFF);
    new_shutter = ISP_MIN(ISP_MAX(new_shutter, 1), new_vsync - 5);

   	//dbg_printf(0, "S:0x%x,V:0x%x\r\n", new_shutter, new_vsync);

    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0xfd,  0x01);  // page 1
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x0e, (new_vsync >> 8));
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x0f, new_vsync);                
	
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x03, (ISP_UINT8)(new_shutter >> 8));
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x04, (ISP_UINT8)(new_shutter));

}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetExposure
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_SetExposure(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain, MMP_ULONG shutter, MMP_ULONG vsync)
{
#if (ISP_EN)
    ISP_UINT16 array[6];
	ISP_UINT32 new_vsync, new_shutter;

    // Gain Setting
    array[0] = 0x305E;
    array[2] = 0x3060;

    ulGain = ulGain * 0x40 / ISP_IF_AE_GetGainBase();

    // Sensor Gain Mapping
    if (ulGain < 0x80) {
        array[1] = ulGain << 1;
        array[3] = 0x0;     // 1X ~ 2X
    }
    else if (ulGain < 0x100) {
        array[1] = ulGain;
        array[3] = 0x10;    // 2X ~ 4X
    }
    else if (ulGain < 0x200) {
        array[1] = ulGain >> 1;
        array[3] = 0x20;    // 4X ~ 8X
    }
    else {
        array[1] = ulGain >> 2;
        array[3] = 0x30;    // 8X ~16X
    }

    // Shutter Setting
	new_vsync   = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetVsync() / ISP_IF_AE_GetVsyncBase();
	new_shutter = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetShutter() / ISP_IF_AE_GetShutterBase();

    array[4] = 0x3012;
	array[5] = ISP_MIN(ISP_MAX(new_shutter, 1), new_vsync  - 3);
    #if 0 // Do not change the frame_length_lines for fixed frame rate
    array[6] = 0x300A;
	array[7] = ISP_MIN(ISP_MAX(new_shutter + 3, new_vsync ), 0xFFFF);
    #endif

    // Set data via I2C
    gsSensorFunction->MMPF_Sensor_SetRegArray(ubSnrSel, array, ARRAY_SIZE(array)/2);
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetFlip
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_SetFlip(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
    ISP_IF_3A_Control(ISP_3A_PAUSE);

    switch(ubMode)
    {
        case MMPF_SENSOR_NO_FLIP:
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0xfd,  0x01);  // page 1
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3f,  0x00);  // [0]: mirror, [1]: Updown
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x01,  0x01);  // [0]: Enable of Frame Sync Signal
            ISP_IF_IQ_SetColorID(0);
            ISP_IF_IQ_SetDirection(ISP_IQ_DIRECTION_ORIGINAL);
            break;
        case MMPF_SENSOR_COLUMN_FLIP:
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0xfd,  0x01);  // page 1
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3f,  0x02);  // [0]: mirror, [1]: Updown
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x01,  0x01);  // [0]: Enable of Frame Sync Signal
            ISP_IF_IQ_SetColorID(0);
            ISP_IF_IQ_SetDirection(ISP_IQ_DIRECTION_V_MIRROR);
            break;
        case MMPF_SENSOR_ROW_FLIP:
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0xfd,  0x01);  // page 1
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3f,  0x01);  // [0]: mirror, [1]: Updown
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x01,  0x01);  // [0]: Enable of Frame Sync Signal
            ISP_IF_IQ_SetColorID(0);
            ISP_IF_IQ_SetDirection(ISP_IQ_DIRECTION_H_MIRROR);
            break;
        case MMPF_SENSOR_COLROW_FLIP:
            break;
        default:
            break;
    }

    ISP_IF_3A_Control(ISP_3A_RECOVER);

}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetRotate
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_SetRotate(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
    ISP_IF_3A_Control(ISP_3A_PAUSE);

    switch(ubMode)
    {
        case MMPF_SENSOR_ROTATE_NO_ROTATE:
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0xfd,  0x01);  // page 1
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3f,  0x00);  // [0]: mirror, [1]: Updown
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x01,  0x01);  // [0]: Enable of Frame Sync Signal
            ISP_IF_IQ_SetColorID(0);
            ISP_IF_IQ_SetDirection(ISP_IQ_DIRECTION_ORIGINAL);
            break;
        case MMPF_SENSOR_ROTATE_RIGHT_90:
            break;
        case MMPF_SENSOR_ROTATE_RIGHT_180:
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0xfd,  0x01);  // page 1
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3f,  0x03);  // [0]: mirror, [1]: Updown
            gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x01,  0x01);  // [0]: Enable of Frame Sync Signal
            ISP_IF_IQ_SetColorID(0);
            ISP_IF_IQ_SetDirection(ISP_IQ_DIRECTION_180_DEGREE);
            break;
        case MMPF_SENSOR_ROTATE_RIGHT_270:
            break;
        default:
            break;
    }

    ISP_IF_3A_Control(ISP_3A_RECOVER);

}
//------------------------------------------------------------------------------
//  Function    : SNR_Cust_CheckVersion
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_CheckVersion(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion)
{
	MMP_UBYTE 	ubSnrOTPMVersion = 0xFF;
	MMP_USHORT 	usRdVal300E = 0, usRdVal30F0 = 0, usRdVal3072 = 0;
    const MMP_I2CM_ATTR *pI2cAttr = ubSnrSel == PRM_SENSOR ? &m_I2cmAttr_Prm : &m_I2cmAttr_Scd;

    /* Initial I2cm */
    MMPF_I2cm_Initialize(pI2cAttr);
    MMPF_OS_SleepMs(10);

	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x301A, 0x0001);
	MMPF_OS_SleepMs(20);

    gsSensorFunction->MMPF_Sensor_GetReg(ubSnrSel, 0x300E, &usRdVal300E);
	gsSensorFunction->MMPF_Sensor_GetReg(ubSnrSel, 0x30F0, &usRdVal30F0);
	gsSensorFunction->MMPF_Sensor_GetReg(ubSnrSel, 0x3072, &usRdVal3072);

    if (usRdVal3072 != 0x0) {
		if (usRdVal3072 == 0x0008)
			ubSnrOTPMVersion = 5;
		else if (usRdVal3072 == 0x0007)
			ubSnrOTPMVersion = 4;
		else
			ubSnrOTPMVersion = 3;
    }
	else {
		if (usRdVal300E == 0x10)
			ubSnrOTPMVersion = 1;
		else
			ubSnrOTPMVersion = 2;
	}

    printc(FG_BLUE("SNR[%d] AR0330-Ver%d")"\r\n", ubSnrSel, ubSnrOTPMVersion);
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_GetIqCompressData
//  Description :
//------------------------------------------------------------------------------
const MMP_UBYTE* SNR_Cust_GetIqCompressData(MMP_UBYTE ubSnrSel)
{
    return s_IqCompressData;
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_StreamEnable
//  Description : Enable/Disable streaming of sensor
//------------------------------------------------------------------------------
static void SNR_Cust_StreamEnable(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
#if 0
#if (SENSOR_IF == SENSOR_IF_PARALLEL)
    #error stream enable/disable fro serial mode not implemented yet!!
#else
    MMP_USHORT status = 0;

    if (bEnable) {
        gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x301A, 0x000C);
    }
    else {

        gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x301A, 0x045A);

        // Polling & sleep-wait for standby status
        while (status == 0) {

            gsSensorFunction->MMPF_Sensor_GetReg(ubSnrSel, 0x303C, &status);
            status &= 0x0002; // Standby bit

            // if(status == 0) // to save some time if status is standby already
            MMPF_OS_SleepMs(1);
        }
    }
#endif
#endif
}
//------------------------------------------------------------------------------
//  Function    : SNR_Cust_CheckVersion
//  Description :
//------------------------------------------------------------------------------
//void SNR_Cust_CheckVersion(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion)
//{
//}
//------------------------------------------------------------------------------
//  Function    : SNR_Cust_GetIqCompressData
//  Description :
//------------------------------------------------------------------------------
//const MMP_UBYTE* SNR_Cust_GetIqCompressData(MMP_UBYTE ubSnrSel)
//{
//    return s_IqCompressData;
//}
//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetIRLED
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetIRLED(MMP_UBYTE ubSnrSel, MMP_UBYTE ubOn)
{
#if SUPPORT_IR_LED
    //MMPF_PIO_SetData(IR_LED, on);
    dbg_printf(3, "IR LED = %d\r\n", ubOn);
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetICR
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetICR(MMP_UBYTE ubSnrSel, MMP_UBYTE ubOn)
{
#if SUPPORT_IR_LED
#if 1//(SUPPORT_IR_CUT)
    //MMPF_PIO_SetData(ICR_ENB, 0);  // Set ENB to low

    if(ubOn)
    {
        //MMPF_PIO_SetData(NMODE, 0);  // Set FBC to low, Night mode
        dbg_printf(3, "ICR Night mode\r\n");
    }
    else
    {
        //MMPF_PIO_SetData(NMODE, 1);  // Set FBC to high, Normal mode
        dbg_printf(3, "ICR Normal mode\r\n");
    }

    //MMPF_PIO_SetData(ICR_ENB, 1);  // Set ENB to high

#endif
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_NightVision_Control
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_NightVision_Control(MMP_UBYTE ubSnrSel, MMP_UBYTE NightMode)
{
#if SUPPORT_IR_LED
    if(NightMode)
    {
        ISP_IF_IQ_SetSysMode(0);
        //ISP_IF_F_SetSaturation(-128);
        ISP_IF_F_SetImageEffect(ISP_IMAGE_EFFECT_GREY);  // Grey
        SNR_Cust_SetIRLED(ubSnrSel, 1);  // turn on IR LED
        SNR_Cust_SetICR(ubSnrSel, 1);  // turn on ICR
        MMPF_Sensor_SetNightVisionMode(ubSnrSel, NV_CTRL_LIGHT_MODE, MMP_TRUE);
        dbg_printf(3, "Enter Night Mode\r\n");
    }
    else
    {
        ISP_IF_IQ_SetSysMode(1);
        //ISP_IF_F_SetSaturation(0);
        ISP_IF_F_SetImageEffect(ISP_IMAGE_EFFECT_NORMAL);  // Normal
        SNR_Cust_SetIRLED(ubSnrSel, 0);  // turn off IR LED
        SNR_Cust_SetICR(ubSnrSel, 0);  // turn off ICR
        MMPF_Sensor_SetNightVisionMode(ubSnrSel, NV_CTRL_LIGHT_MODE, MMP_FALSE);
        dbg_printf(3, "Exit Night Mode\r\n");
    }
#endif
}

MMPF_SENSOR_CUSTOMER  SensorCustFunc =
{
    SNR_Cust_InitConfig,
    SNR_Cust_DoAE_FrmSt,
    SNR_Cust_DoAE_FrmEnd,
    SNR_Cust_DoAWB_FrmSt,
    SNR_Cust_DoAWB_FrmEnd,
    SNR_Cust_DoIQ,
    SNR_Cust_SetGain,
    SNR_Cust_SetShutter,
    SNR_Cust_SetExposure,
    SNR_Cust_SetFlip,
    SNR_Cust_SetRotate,
    SNR_Cust_CheckVersion,
    SNR_Cust_GetIqCompressData,
    SNR_Cust_StreamEnable,
    //SNR_Cust_SetIRLED,
    //SNR_Cust_SetICR,
    //SNR_Cust_NightVision_Control,
    
    &m_SensorRes,
    &m_OprTable,
    &m_VifSetting_Prm,
    &m_I2cmAttr_Prm,
    &m_AwbTime,
    &m_AeTime,
    &m_AfTime,
    MMP_SNR_PRIO_PRM
};

#if defined(BIND_SENSOR_AR0330_2ND) && (BIND_SENSOR_AR0330_2ND)
static MMPF_SENSOR_CUSTOMER SensorCustFunc_Scd =
{
#endif

int SNR_Module_Init(void)
{
    if (SensorCustFunc.sPriority == MMP_SNR_PRIO_PRM)
        MMPF_SensorDrv_Register(PRM_SENSOR, &SensorCustFunc);
    else
        MMPF_SensorDrv_Register(SCD_SENSOR, &SensorCustFunc);

#if defined(BIND_SENSOR_AR0330_2ND) && (BIND_SENSOR_AR0330_2ND)
    MMPF_SensorDrv_Register(SCD_SENSOR, &SensorCustFunc_Scd);
#endif

    return 0;
}

#pragma arm section code = "initcall6", rodata = "initcall6", rwdata = "initcall6",  zidata = "initcall6" 
#pragma O0
ait_module_init(SNR_Module_Init);
#pragma
#pragma arm section rodata, rwdata, zidata

#endif  //BIND_SENSOR_OV2735
#endif	//SENSOR_EN
