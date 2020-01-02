//==============================================================================
//
//  File        : sensor_ap1302.c
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
#if (BIND_SENSOR_AP1302)

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
#define SENSOR_IF (SENSOR_IF_MIPI_4_LANE) // TBD
#endif

MMPF_SENSOR_RESOLUTION m_AP1302_SensorRes_Prm = 
{
    3,                                  // ubSensorModeNum
    0,                                  // ubDefPreviewMode
    0,                                  // ubDefCaptureMode
    2200,                               // usPixelSize  //Sunny
//  Mode0       Mode1		Mode2
//  1080P30     600@30P     720@30P
    {1,         1,          1       },  // usVifGrabStX
    {1,         1,          1       },  // usVifGrabStY
    {1920,      800,        1280    },  // usVifGrabW
    {1080,      600,        720     },  // usVifGrabH
    #if (CHIP == MCR_V2)
    {1,         1,          1       },  // usBayerInGrabX
    {1,         1,          1       },  // usBayerInGrabY
    {0,         0,          0       },  // usBayerInDummyX
    {0,         0,          0       },  // usBayerInDummyY
    {1920,      800,        1280    },  // usBayerOutW
    {1080,      600,        720     },  // usBayerOutH
    #endif
    {1920,      800,        1280    },  // usScalInputW
    {1080,      600,        720     },  // usScalInputH
    {300,       300,        300     },  // usTargetFpsx10
    {1230,      630,        750     },  // usVsyncLine   //Sunny
    {1,         1,          1       },  // ubWBinningN
    {1,         1,          1       },  // ubWBinningM
    {1,         1,          1       },  // ubHBinningN
    {1,         1,          1       },  // ubHBinningM
    {0xFF,     	0xFF,     	0xFF    },  // ubCustIQmode
    {0xFF,     	0xFF,     	0xFF    },  // ubCustAEmode
};

MMPF_SENSOR_RESOLUTION m_AP1302_SensorRes_Scd = 
{
    3,                                  // ubSensorModeNum
    0,                                  // ubDefPreviewMode
    0,                                  // ubDefCaptureMode
    2200,                               // usPixelSize  //Sunny
//  Mode0       Mode1		Mode2
//  1080P30     600@30P     720@30P
    {1,         1,          1       },  // usVifGrabStX
    {1,         1,          1       },  // usVifGrabStY
    {3840,      1600,       2560    },  // usVifGrabW
    {1080,      600,        720     },  // usVifGrabH
    #if (CHIP == MCR_V2)
    {1,         1,          1       },  // usBayerInGrabX
    {1,         1,          1       },  // usBayerInGrabY
    {0,         0,          0       },  // usBayerInDummyX
    {0,         0,          0       },  // usBayerInDummyY
    {1920,      800,        1280    },  // usBayerOutW
    {1080,      600,        720     },  // usBayerOutH
    #endif
    {1920,      800,        1280    },  // usScalInputW
    {1080,      600,        720     },  // usScalInputH
    {300,       300,        300     },  // usTargetFpsx10
    {1230,      630,        750     },  // usVsyncLine   //Sunny
    {1,         1,          1       },  // ubWBinningN
    {1,         1,          1       },  // ubWBinningM
    {1,         1,          1       },  // ubHBinningN
    {1,         1,          1       },  // ubHBinningM
    {0xFF,     	0xFF,     	0xFF    },  // ubCustIQmode
    {0xFF,     	0xFF,     	0xFF    },  // ubCustAEmode
};

// VIF Setting
static MMPF_SENSOR_VIF_SETTING m_AP1302_VifSetting_Prm =
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
        MMP_FALSE, //MMP_TRUE,       // bFirstEnPinHigh
        10,             // ubFirstEnPinDelay
        MMP_TRUE, //MMP_FALSE,      // bNextEnPinHigh
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
        {MMP_FALSE,                      MMP_FALSE,                       MMP_FALSE,                           MMP_FALSE},                          // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_1,   MMPF_VIF_MIPI_DATA_SRC_PHY_2,       MMPF_VIF_MIPI_DATA_SRC_PHY_3},      // ubDataLaneSrc
        {8,                             8,                              8,                                  8},                                 // usDataDelay
        {0x19,                          0x19,                           0x19,                               0x19}                               // ubDataSotCnt
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
        MMP_FALSE,                                      	    // bRawStoreEnable
        MMP_FALSE,                                      	// bYuv422ToYuv420
        MMP_TRUE,                                      	    // bYuv422ToYuv422
        MMP_FALSE,                                      	// bYuv422ToBayer
        MMPF_VIF_YUV422_UYVY,                           	// ubYuv422Order
    }
};

static MMPF_SENSOR_VIF_SETTING m_AP1302_VifSetting_Scd =
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
        MMP_FALSE, //MMP_TRUE,       // bFirstEnPinHigh
        10,             // ubFirstEnPinDelay
        MMP_TRUE, //MMP_FALSE,      // bNextEnPinHigh
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
        {MMP_FALSE,                      MMP_FALSE,                       MMP_FALSE,                           MMP_FALSE},                          // bDataDelayEn
//        {MMP_TRUE,                      MMP_TRUE,                       MMP_TRUE,                           MMP_TRUE},                          // bDataDelayEn
        {MMP_FALSE,                     MMP_FALSE,                      MMP_FALSE,                          MMP_FALSE},                         // bDataLaneSwapEn
        {MMPF_VIF_MIPI_DATA_SRC_PHY_0,  MMPF_VIF_MIPI_DATA_SRC_PHY_1,   MMPF_VIF_MIPI_DATA_SRC_PHY_2,       MMPF_VIF_MIPI_DATA_SRC_PHY_3},      // ubDataLaneSrc
        {0,                             0,                              0,                                  0},                                 // usDataDelay
        {0x0F,                          0x0F,                           0x0F,                               0x0F}                               // ubDataSotCnt
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
static MMP_I2CM_ATTR m_AP1302_I2cmAttr_Prm = 
{
    MMP_I2CM0,                  // i2cmID
    SENSOR_I2C_ADDR_AP1302,     // ubSlaveAddr
    16,                         // ubRegLen    //Sunny
    8,                          // ubDataLen
    0,                          // ubDelayTime
    MMP_FALSE,                  // bDelayWaitEn
    MMP_FALSE,                  // bInputFilterEn
    MMP_FALSE,                  // b10BitModeEn
    MMP_TRUE,                   // bClkStretchEn
    0,                          // ubSlaveAddr1
    0,                          // ubDelayCycle
    0,                          // ubPadNum
    400000,                     // ulI2cmSpeed 250KHZ
    MMP_TRUE,                   // bOsProtectEn
    NULL,                       // sw_clk_pin
    NULL,                       // sw_data_pin
    MMP_FALSE,                  // bRfclModeEn
    MMP_FALSE,                  // bWfclModeEn
    MMP_FALSE,                  // bRepeatModeEn
    0                           // ubVifPioMdlId
};

// I2cm Attribute for secondary sensor
static MMP_I2CM_ATTR m_AP1302_I2cmAttr_Scd = 
{
    MMP_I2CM0,                  // i2cmID
    SENSOR_I2C_ADDR_AP1302,     // ubSlaveAddr
    16,                          // ubRegLen
    8,                          // ubDataLen
    0,                          // ubDelayTime
    MMP_FALSE,                  // bDelayWaitEn
    MMP_FALSE,                  // bInputFilterEn
    MMP_FALSE,                  // b10BitModeEn
    MMP_TRUE,                   // bClkStretchEn
    0,                          // ubSlaveAddr1
    0,                          // ubDelayCycle
    0,                          // ubPadNum
    400000,                     // ulI2cmSpeed 250KHZ
    MMP_TRUE,                   // bOsProtectEn
    NULL,                       // sw_clk_pin
    NULL,                       // sw_data_pin
    MMP_FALSE,                  // bRfclModeEn
    MMP_FALSE,                  // bWfclModeEn
    MMP_FALSE,                  // bRepeatModeEn
    0                           // ubVifPioMdlId
};

// 3A Timing
static MMPF_SENSOR_AWBTIMIMG m_AP1302_AwbTime = 
{
    6,  /* ubPeriod */
    1,  /* ubDoAWBFrmCnt */
    3   /* ubDoCaliFrmCnt */
};

static MMPF_SENSOR_AETIMIMG m_AP1302_AeTime = 
{	
    5,  /* ubPeriod */
    0,  /* ubFrmStSetShutFrmCnt */
    0   /* ubFrmStSetGainFrmCnt */
};

static MMPF_SENSOR_AFTIMIMG m_AP1302_AfTime = 
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

MMP_I2CM_ATTR m_snr_I2cmAttribute;
#if 0
MMP_BOOL AP1302_I2CBurstWrite(MMP_UBYTE* WrBuf, MMP_LONG NumOfWriteBytes)
{
    MMP_USHORT usAdress;

	usAdress = (WrBuf[0]<<8) + WrBuf[1];
	//RTNA_DBG_Str(0, "##########################%x, %x\r\n", NumOfWriteBytes, usAdress);

	//if(NumOfWriteBytes>=3)
	if(1)
	{
		//MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute,usAdress, (MMP_UBYTE*)&WrBuf[2],(MMP_UBYTE)NumOfWriteBytes-2);
		MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute,usAdress, (MMP_UBYTE*)&WrBuf[2],(MMP_UBYTE)NumOfWriteBytes-2);
	}
	else
	{
		MMPF_I2cm_WriteReg(&m_snr_I2cmAttribute,usAdress, (MMP_USHORT)WrBuf[2]);
	}
	
	//MMPF_I2cm_WriteBurstData(&uAudioI2cmAttribute,usAdress, (MMP_UBYTE*)&WrBuf[2],NumOfWriteBytes-2);
	
    return MMP_TRUE;
}

/* AP1302_I2CWriteBurstData() use to replace MMPF_I2cm_WriteBurstData() */
MMP_ERR AP1302_I2CWriteBurstData(MMP_USHORT usReg, MMP_UBYTE *usData, MMP_LONG usDataCnt)
{
    //MMP_ULONG   InpValue;
    MMP_ERR     status = MMP_ERR_NONE;

    //InpValue = *(MMP_ULONG*)usData;

    //RTNA_DBG_Str(0, "AP1302_I2CWriteBurstData: %x, %x,\r\n", usReg, *(MMP_ULONG*)usData);

    #if (USE_SPI_IF == 1)

    AP1302_SPIWriteReg(usReg, usData, usDataCnt);

    #else

    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, usReg, usData, usDataCnt);

    #endif

    return status;
}

#endif

MMP_BOOL AP1302_I2cWriteThenRead(MMP_UBYTE* WriteBuf, MMP_LONG NumOfWriteBytes, MMP_UBYTE* pReadBuf, MMP_LONG NumOfReadBytes)
{
#if 0
    MMP_USHORT  usAdress;

        //if(NumOfWriteBytes >= 3)//NumOfWriteBytes is size of register 
    if(1)
    {
		usAdress = ((MMP_USHORT)(WriteBuf[0]<<8) +(MMP_USHORT)(WriteBuf[1]));
        if( AP1302_I2CBurstWrite(WriteBuf, NumOfWriteBytes) )
		{ 
			
			//MMPF_I2cm_ReadBurstData(&uAudioI2cmAttribute, usAdress, (MMP_UBYTE*)pReadBuf ,NumOfReadBytes);
			MMPF_I2cm_CCI_ReadData(&m_snr_I2cmAttribute, (MMP_UBYTE)NumOfReadBytes, (MMP_UBYTE*)pReadBuf);
			//MMPF_I2cm_ReadNoRegMode(&m_snr_I2cmAttribute, (MMP_UBYTE)NumOfReadBytes, (MMP_UBYTE*)pReadBuf);
			
	    }
    }
    else
    {
        usAdress = (MMP_USHORT)WriteBuf[0]/*((MMP_USHORT)(WriteBuf[0]<<8) +(MMP_USHORT)(WriteBuf[1]))*/;
		MMPF_I2cm_ReadBurstData(&m_snr_I2cmAttribute, usAdress, (MMP_UBYTE*)pReadBuf ,NumOfReadBytes);
		
    }    
#endif

    return MMP_TRUE;
}

MMP_USHORT SNR_AP1302_Reg_JPEG_Customer[] = {
//#include "AP1302_3840x2160EXIF.h"// 48 MHz MCLK
#include "AP1302_3840x2160EXIF_24IN_D110416.h" //24 MHz MCLK
};

void SNR_AP1302_Reg_Init() 
{
    #define PREVIEW_WIDTH  1920
    #define PREVIEW_HEIGHT 1080
    #define PREVIEW_FPS    30
    
    #define CONST_NUM_100  100
    #define CONST_NUM_1000 1000

    #define SENSOR_SIPS_CRC 0xF052 
    #define SENSOR__WRAP_SIZE 0x2000
    
    #define FW_CHKSUM_VAL           (0x9A14) 
    #define FW_CHKSUM_RDY_TIME      (276)
    #define FW_CHKSUM_WAIT_TIMEOUT  (30000)
    #define CPU_SLEEP_DO_BUSY_WAIT  (1)
    #define EN_MEAS_FW_CHKSUM_TIME  (0)
    #define EN_CHKSUM_ERR_RETRY     (0)

    MMP_USHORT *reg_setting = 0;
	MMP_USHORT ulI2C_Reg[6] = {0x00, 0x26, 0x00, 0x54};
	MMP_ULONG Sensor_ChipVersionReg = 0;
	MMP_USHORT  usSensor_ID_Reg = 0x0000; 
	MMP_USHORT  I2C_data;
	MMP_USHORT reg_init_length = 0;      
	MMP_USHORT reg_length = 0;   
	MMP_USHORT reg_crc = 0;   
	MMP_ULONG  addr;
	ISP_UINT32  i, l;
    //MMP_ULONG  ulTmpWaitCnt = CONST_NUM_100, ulTotalWaitCnt = 0;
    //MMP_ULONG   ulKickWdCnt = CONST_NUM_1000;
    //MMP_ULONG  ulFwChkSumVal = 0;

    MMP_ULONG slSensorOutFps;

    MMP_USHORT usWriteBurstData[2];

    RTNA_DBG_Str(0, "<Note> Sensor AP1302 requests CLK_GRP_SNR, CLK_GRP_BAYER & CLK_GRP_COLOR up to 528 MHz\r\n");
    RTNA_DBG_Str(0, "<Note> Sensor AP1302 requests HANDLE_H264E_EVENT_BY_QUEUE if SCD_CAM_YUV_SENSOR\r\n");

#if (SENSOR_PROI == PRM_SENSOR)
    memcpy( &m_snr_I2cmAttribute, &m_AP1302_I2cmAttr_Prm, sizeof(MMP_I2CM_ATTR));
#else
    memcpy( &m_snr_I2cmAttribute, &m_AP1302_I2cmAttr_Scd, sizeof(MMP_I2CM_ATTR));
#endif

	/* Use I2C I/F */
    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, 0xF038, ulI2C_Reg, 4, MMP_TRUE);
    usSensor_ID_Reg = 0x00E0;
    AP1302_I2cWriteThenRead((MMP_UBYTE*)&usSensor_ID_Reg, 2, (MMP_UBYTE*)&Sensor_ChipVersionReg, 4);

    Sensor_ChipVersionReg = 0;
    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, 0xF038, ulI2C_Reg, 4, MMP_TRUE);
    usSensor_ID_Reg = 0xE000;
    ulI2C_Reg[0] = 0;
    ulI2C_Reg[1] = 0;
    ulI2C_Reg[2] = 0;
    ulI2C_Reg[3] = 1;
    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, usSensor_ID_Reg, ulI2C_Reg, 4, MMP_TRUE);
    //RTNA_DBG_Str(0, "WriteBurstData 0xE000 -\r\n");
    usSensor_ID_Reg = 0x0000;
        
		//gsBayer169W60Fps[MAIN_SENSOR] = gsBayer169W[MAIN_SENSOR] = gsSensorLCModeWidth = gsSensorMCModeWidth  = cur_pipe->pipe_w[PIPE_0];
        //gsBayer169H60Fps[MAIN_SENSOR] = gsBayerInH[MAIN_SENSOR] = gsBayer169H[MAIN_SENSOR] = gsSensorLCModeHeight = gsSensorMCModeHeight  = cur_pipe->pipe_h[PIPE_0];
#if 0
    //Read Chip ID
	 do{
        AP1302_I2cWriteThenRead((MMP_UBYTE*)&usSensor_ID_Reg, 2, (MMP_UBYTE*)&Sensor_ChipVersionReg, 2);
        printc("1 sensor chip version:%x\r\n", Sensor_ChipVersionReg);
        if(Sensor_ChipVersionReg)
            break;
        else
            RTNA_DBG_Str(0, "sensor ID get fail\r\n");

        MMPF_OS_Sleep(3);
    }while(1);
    printc("2 sensor chip version:%x\r\n", Sensor_ChipVersionReg);

#else
    MMPF_OS_Sleep(500);
     
#endif

    usWriteBurstData[0] = 0xFF; 
    usWriteBurstData[1] = 0xFF;

    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, SENSOR_SIPS_CRC, &usWriteBurstData, 2, MMP_TRUE);

    reg_setting = SNR_AP1302_Reg_JPEG_Customer;
    reg_length = 872;   
    reg_crc = 0x06a6;   // 0xa606
    reg_init_length = sizeof(SNR_AP1302_Reg_JPEG_Customer) / sizeof(MMP_USHORT);
    
	i = 0;
	addr = 0x8000;
    while( i < (reg_init_length))
    {
	    if(i < reg_length)
	      l = reg_length;
	    else
	      l = (reg_init_length);
	      
	    l -= i;
	    if(l > 16)
	        l = 16;
	    if ((i % SENSOR__WRAP_SIZE) + l > SENSOR__WRAP_SIZE)
	        l = SENSOR__WRAP_SIZE - i % SENSOR__WRAP_SIZE;

	    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, addr + i % SENSOR__WRAP_SIZE, reg_setting + i, l, MMP_TRUE);

	    //RTNA_DBG_Str(0, "Written %x %dB (%d/%d)\r\n",  addr + i % SENSOR__WRAP_SIZE, l, i, sizeof(SNR_AP1302_Reg_Init_Customer));
	    i += l;
	    if(i == reg_length)
	    {
	        //I2C_data = 0x0200;
            usWriteBurstData[0] = 0x00;
            usWriteBurstData[1] = 0x02; 
	        MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, 0x6002, &usWriteBurstData, 2, MMP_TRUE);
	        RTNA_DBG_Str(0, "Delay for PLL start.\r\n");
	        MMPF_OS_Sleep(100);
	        //break;
	    }
    }
    
	/* Read SIPS_CRC R0xF052 */
	usSensor_ID_Reg = 0x52F0;
	AP1302_I2cWriteThenRead((MMP_UBYTE*)&usSensor_ID_Reg, 2, (MMP_UBYTE*)&Sensor_ChipVersionReg, 2);
	//printc("CRC:%x\r\n", Sensor_ChipVersionReg);
	/* Read BOOTDATA_STAGE R0x6002 */
	I2C_data= 0xFFFF;
	usSensor_ID_Reg = 0x0260;
	AP1302_I2cWriteThenRead((MMP_UBYTE*)&usSensor_ID_Reg, 2, (MMP_UBYTE*)&Sensor_ChipVersionReg, 2);

	/* Write BOOTDATA_STAGE R0x6002 */
    usWriteBurstData[0] = 0xFF; 
    usWriteBurstData[1] = 0xFF;
    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, 0x6002, &usWriteBurstData, 2);    
    //AP1302_I2CWriteBurstData(0x6002, (MMP_UBYTE *)&I2C_data, 2);
    
    #if (CPU_SLEEP_DO_BUSY_WAIT == 1)
    RTNA_WAIT_MS(80);
    #else
	MMPF_OS_Sleep(80); //100
    #endif
	//printc("boot status:%x\r\n", Sensor_ChipVersionReg);
#if 0
label_retry:

	/* Read BOOTDATA_CHECKSUM R0x6134 =  */
	usSensor_ID_Reg = 0x3461;
    
    ulFwChkSumVal = FW_CHKSUM_VAL;
    
    #if (EN_MEAS_FW_CHKSUM_TIME == 1)
    ulTotalWaitCnt = 0;
    #else
    #if (CPU_SLEEP_DO_BUSY_WAIT == 1)
    RTNA_WAIT_MS(FW_CHKSUM_RDY_TIME);
    #else
	MMPF_OS_Sleep(FW_CHKSUM_RDY_TIME);
    #endif
    ulTotalWaitCnt = FW_CHKSUM_RDY_TIME;
	#endif
    RTNA_DBG_Str(0, "Wait");
	ulTmpWaitCnt = CONST_NUM_100;

#if 1    
    do {
        #if (CPU_SLEEP_DO_BUSY_WAIT == 1)
        RTNA_WAIT_MS(1);
        #else
    	MMPF_OS_Sleep(1);
        #endif
    	AP1302_I2cWriteThenRead((MMP_UBYTE*)&usSensor_ID_Reg, 2, (MMP_UBYTE*)&Sensor_ChipVersionReg, 2);
		ulTotalWaitCnt++;
		if (ulTmpWaitCnt-- == 0) {
            RTNA_DBG_Str(0, ".");
			ulTmpWaitCnt = CONST_NUM_100;
		}
        if (ulKickWdCnt-- == 0) {
            /* Kick WD every (1000)ms */
			MMPF_WD_Kick();
			ulKickWdCnt = CONST_NUM_1000;
    	    //RTNA_DBG_Str(3, "%#x,", Sensor_ChipVersionReg);
        }
    } while((Sensor_ChipVersionReg != ulFwChkSumVal) && (ulTotalWaitCnt < FW_CHKSUM_WAIT_TIMEOUT));

    if (ulTotalWaitCnt >= FW_CHKSUM_WAIT_TIMEOUT) {
	    printc("\r\nCHKSUM: %#x ERROR,\r\n", Sensor_ChipVersionReg);
        while(1);
    }
#else
    MMPF_OS_Sleep(500);
#endif
	printc("\r\nCHKSUM: %#x ready, take %d ms,\r\n", Sensor_ChipVersionReg, ulTotalWaitCnt);

#if (EN_CHKSUM_ERR_RETRY == 1)
    if (Sensor_ChipVersionReg != ulFwChkSumVal) {
	    RTNA_DBG_Str(0, "Not ready, retry,\r\n");
        goto label_refine;
    }
#endif
#endif
	// Output YUV422 format
	//usSensor_ID_Reg = 0x5000;
    //AP1302_I2CWriteBurstData(0x2012, (MMP_UBYTE *)&usSensor_ID_Reg, 2);
    usWriteBurstData[0] = 0x00;
    usWriteBurstData[1] = 0x50; 
    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, 0x2012, &usWriteBurstData, 2);    

    slSensorOutFps = 30;
	I2C_data= 0xFFFF;
	usSensor_ID_Reg = 0x1220;
	// PREVIEW_AE_MAX_ET
	AP1302_I2cWriteThenRead((MMP_UBYTE*)&usSensor_ID_Reg, 2, (MMP_UBYTE*)&Sensor_ChipVersionReg, 2);

	// preview width
    usWriteBurstData[0] = ( PREVIEW_WIDTH & 0xFF00 ) >> 8;
    usWriteBurstData[1] = PREVIEW_WIDTH & 0xFF; 
    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, 0x2000, &usWriteBurstData, 2);    
	// preview height
    usWriteBurstData[0] = ( PREVIEW_HEIGHT & 0xFF00 ) >> 8;
    usWriteBurstData[1] = PREVIEW_HEIGHT & 0xFF; 
    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, 0x2002, &usWriteBurstData, 2);    
	// PREVIEW_MAX_FPS
    usWriteBurstData[0] = slSensorOutFps & 0xFF; 
    usWriteBurstData[1] = ( slSensorOutFps & 0xFF00 ) >> 8;
    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, 0x2020, &usWriteBurstData, 2);    
    
    usWriteBurstData[0] = 0x00; 
    usWriteBurstData[1] = 0x00;
    MMPF_I2cm_WriteBurstData(&m_snr_I2cmAttribute, 0x100C, &usWriteBurstData, 2);    
    
	#if 0
	Sensor_ChipVersionReg = 0x03;
    AP1302_I2CWriteBurstData(0x600C, (MMP_UBYTE *)&Sensor_ChipVersionReg, 2);
    #endif
    I2C_data= 0xFFFF;
	usSensor_ID_Reg = 0x1810;
	// PREVIEW_AE_MAX_ET
	AP1302_I2cWriteThenRead((MMP_UBYTE*)&usSensor_ID_Reg, 2, (MMP_UBYTE*)&Sensor_ChipVersionReg, 2);
	//printc("0x1018:%x\r\n", Sensor_ChipVersionReg);

}

#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} // dummy
#endif

ISP_UINT16 SNR_AP1302_Reg_Init_Customer[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};

ISP_UINT16 SNR_AP1302_Reg_Unsupport[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};

ISP_UINT16 SNR_AP1302_Reg_1920x1080_30P[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};

// OPR Table Setting
static MMPF_SENSOR_OPR_TABLE m_AP1302_OprTable = 
{
    // usInitSize
    (sizeof(SNR_AP1302_Reg_Init_Customer)/sizeof(SNR_AP1302_Reg_Init_Customer[0]))/2,

    // uspInitTable
    &SNR_AP1302_Reg_Init_Customer[0],

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
        (sizeof(SNR_AP1302_Reg_1920x1080_30P)/sizeof(SNR_AP1302_Reg_1920x1080_30P[0]))/2,
    },

    // uspTable
    {
        &SNR_AP1302_Reg_1920x1080_30P[0],
    }
};

#if 0
void ____Sensor_Customer_Func____(){ruturn;} // dummy
#endif

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_InitConfig
//  Description :
//------------------------------------------------------------------------------
static void SNR_AP1302_Cust_InitConfig(void)
{
    RTNA_DBG_Str(0, "SNR_Cust_InitConfig AP1302\r\n");
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
void SNR_AP1302_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
void SNR_AP1302_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_DoAWB_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_AP1302_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_DoAWB_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_AP1302_Cust_DoAWB_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_DoIQ
//  Description :
//------------------------------------------------------------------------------
void SNR_AP1302_Cust_DoIQ(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_SetGain
//  Description :
//------------------------------------------------------------------------------
void SNR_AP1302_Cust_SetGain(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
void SNR_AP1302_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_SetExposure
//  Description :
//------------------------------------------------------------------------------
static void SNR_AP1302_Cust_SetExposure(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain, MMP_ULONG ulShutter, MMP_ULONG ulVsync)
{
    //TBD
    printc(FG_RED("Warning!!! Please review SNR_Cust_SetExposure in sensor driver!!!\r\n"));
    gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, ulShutter, ulVsync);
    gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, ulGain);
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_SetFlip
//  Description :
//------------------------------------------------------------------------------
void SNR_AP1302_Cust_SetFlip(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_SetRotate
//  Description :
//------------------------------------------------------------------------------
void SNR_AP1302_Cust_SetRotate(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_CheckVersion
//  Description :
//------------------------------------------------------------------------------
void SNR_AP1302_Cust_CheckVersion(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_GetIqCompressData
//  Description :
//------------------------------------------------------------------------------
const MMP_UBYTE* SNR_AP1302_Cust_GetIqCompressData(MMP_UBYTE ubSnrSel)
{
    return s_IqCompressData;
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_StreamEnable
//  Description : Enable/Disable streaming of sensor
//------------------------------------------------------------------------------
static void SNR_AP1302_Cust_StreamEnable(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    static bInitialized = MMP_FALSE;
    if( bEnable == MMP_TRUE )
    {
        if( bInitialized == MMP_FALSE )
        {
            bInitialized = MMP_TRUE;
            SNR_AP1302_Reg_Init();            
        }
    }
    else
    {
        bInitialized = MMP_FALSE;
    }
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_CheckSnrTVSrcType
//  Description :
//------------------------------------------------------------------------------
static void SNR_AP1302_Cust_CheckSnrTVSrcType(MMP_UBYTE ubSnrSel, MMP_SNR_TVDEC_SRC_TYPE *TVSrcType)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_AP1302_Cust_GetSnrID
//  Description :
//------------------------------------------------------------------------------
static void SNR_AP1302_Cust_GetSnrID(MMP_UBYTE ubSnrSel, MMP_ULONG *SensorID)
{
    *SensorID = 0xFFFF1302;
}

static MMPF_SENSOR_CUSTOMER SensorCustFunc_Prm = 
{
    SNR_AP1302_Cust_InitConfig,
    SNR_AP1302_Cust_DoAE_FrmSt,
    SNR_AP1302_Cust_DoAE_FrmEnd,
    SNR_AP1302_Cust_DoAWB_FrmSt,
    SNR_AP1302_Cust_DoAWB_FrmEnd,
    SNR_AP1302_Cust_DoIQ,
    SNR_AP1302_Cust_SetGain,
    SNR_AP1302_Cust_SetShutter,
    SNR_AP1302_Cust_SetExposure,
    SNR_AP1302_Cust_SetFlip,
    SNR_AP1302_Cust_SetRotate,
    SNR_AP1302_Cust_CheckVersion,
    SNR_AP1302_Cust_GetIqCompressData,
    SNR_AP1302_Cust_StreamEnable,

    &m_AP1302_SensorRes_Prm,
    &m_AP1302_OprTable,
    &m_AP1302_VifSetting_Prm,
    &m_AP1302_I2cmAttr_Prm,
    &m_AP1302_AwbTime,
    &m_AP1302_AeTime,
    &m_AP1302_AfTime,
    MMP_SNR_PRIO_PRM,
    
    SNR_AP1302_Cust_CheckSnrTVSrcType,
    SNR_AP1302_Cust_GetSnrID    
};

static MMPF_SENSOR_CUSTOMER SensorCustFunc_Scd = 
{
    SNR_AP1302_Cust_InitConfig,
    SNR_AP1302_Cust_DoAE_FrmSt,
    SNR_AP1302_Cust_DoAE_FrmEnd,
    SNR_AP1302_Cust_DoAWB_FrmSt,
    SNR_AP1302_Cust_DoAWB_FrmEnd,
    SNR_AP1302_Cust_DoIQ,
    SNR_AP1302_Cust_SetGain,
    SNR_AP1302_Cust_SetShutter,
    SNR_AP1302_Cust_SetExposure,
    SNR_AP1302_Cust_SetFlip,
    SNR_AP1302_Cust_SetRotate,
    SNR_AP1302_Cust_CheckVersion,
    SNR_AP1302_Cust_GetIqCompressData,
    SNR_AP1302_Cust_StreamEnable,

    &m_AP1302_SensorRes_Scd,
    &m_AP1302_OprTable,
    &m_AP1302_VifSetting_Scd,
    &m_AP1302_I2cmAttr_Scd,
    &m_AP1302_AwbTime,
    &m_AP1302_AeTime,
    &m_AP1302_AfTime,
    MMP_SNR_PRIO_SCD,
    
    SNR_AP1302_Cust_CheckSnrTVSrcType,
    SNR_AP1302_Cust_GetSnrID    
};

#if (SENSOR_PROI == PRM_SENSOR)
int SNR_Module_Init(void)
{
    if (SensorCustFunc_Prm.sPriority == MMP_SNR_PRIO_PRM)
        MMPF_SensorDrv_Register(PRM_SENSOR, &SensorCustFunc_Prm);
    else
        MMPF_SensorDrv_Register(SCD_SENSOR, &SensorCustFunc_Prm);

#if defined(BIND_SENSOR_AP1302_2ND) && (BIND_SENSOR_AP1302_2ND)
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

#endif // (BIND_SENSOR_AP1302)
#endif // (SENSOR_EN)
