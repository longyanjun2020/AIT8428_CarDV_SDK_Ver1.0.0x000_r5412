//==============================================================================
//
//  File        : TvDecoder_TW9992.c
//  Description : Firmware Sensor Control File
//  Author      :
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#include "customer_config.h"
#include "snr_cfg.h"

#if (SENSOR_EN)
#if (BIND_SENSOR_TW9992)

#include "mmpf_sensor.h"
#include "Sensor_Mod_Remapping.h"
#include "isp_if.h"

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================

MMPF_SENSOR_RESOLUTION m_TW9992_SensorRes_RawStore_Mode =
{
	2,				// ubSensorModeNum
	0,				// ubDefPreviewMode
	0,				// ubDefCaptureMode
	3000,           // usPixelSize (TBD)
//  Mode0   Mode1
    {1,        1},  // usVifGrabStX
    {1,        1},  // usVifGrabStY
    {1440,  1440},  // usVifGrabW
    {240,    240},  // usVifGrabH
    #if (CHIP == MCR_V2)
    {1,   	   1}, 	// usBayerInGrabX
    {1,   	   1}, 	// usBayerInGrabY
    {2,        2},  // usBayerInDummyX
    {2,        2},  // usBayerInDummyY
    {720,    720},  // usBayerOutW
    {240,    240},	// usBayerOutH
    #endif
    #if 1//(TVDEC_SNR_USE_DMA_DEINTERLACE)
    {688, 	 688},  // usScalInputW
    {480,    480},	// usScalInputH
    #else
    {688, 	 688},  // usScalInputW
    {240,    240},	// usScalInputH
    #endif
    {250,    300},  // usTargetFpsx10
    {250,    250},  // usVsyncLine
    {1,   	   1}, 	// ubWBinningN
    {1,   	   1}, 	// ubWBinningN
    {1,   	   1}, 	// ubWBinningN
    {1,   	   1}, 	// ubWBinningN
    {0xFF,  0xFF},  // ubCustIQmode
    {0xFF,  0xFF}   // ubCustAEmode
};

MMPF_SENSOR_RESOLUTION m_TW9992_SensorRes_BypassISP_Mode =
{
	2,				// ubSensorModeNum
	0,				// ubDefPreviewMode
	0,				// ubDefCaptureMode
	3000,           // usPixelSize (TBD)
//  Mode0   Mode1
    {1,       1},   // usVifGrabStX
    {1,       1},   // usVifGrabStY
    {720,   720},   // usVifGrabW
    {240,   240},   // usVifGrabH
    #if (CHIP == MCR_V2)
    {1,   	  1}, 	// usBayerInGrabX
    {1,   	  1}, 	// usBayerInGrabY
    {2,       2},   // usBayerInDummyX
    {2,       2},   // usBayerInDummyY
    {720,   720},   // usBayerOutW
    {240,   240},	// usBayerOutH
    #endif
    #if 0//(TVDEC_SNR_USE_DMA_DEINTERLACE)
    {688, 	688},   // usScalInputW
    {480,   480},	// usScalInputH
    #else
    {688, 	688},   // usScalInputW
    {240,   240},	// usScalInputH
    #endif
    {250,   300},   // usTargetFpsx10
    {250,   250},   // usVsyncLine
    {1,   	  1}, 	// ubWBinningN
    {1,   	  1}, 	// ubWBinningN
    {1,   	  1}, 	// ubWBinningN
    {1,   	  1}, 	// ubWBinningN
    {0xFF, 0xFF},   // ubCustIQmode
    {0xFF, 0xFF}    // ubCustAEmode
};

// OPR Table and Vif Setting
MMPF_SENSOR_OPR_TABLE       m_TW9992_OprTable;
MMPF_SENSOR_VIF_SETTING     m_TW9992_VifSetting;

// IQ Table
#if (TVDEC_SNR_PROI == PRM_SENSOR)
const ISP_UINT8 Sensor_IQ_CompressedText[] = 
{
#ifdef CUS_ISP_8428_IQ_DATA     // maybe defined in project MCP or Config_SDK.h
#include CUS_ISP_8428_IQ_DATA
#else
#include "isp_8428_iq_data_v2_AR0331.xls.ciq.txt"
#endif
};
#endif

// I2cm Attribute
static MMP_I2CM_ATTR m_TW9992_I2cmAttr = 
{
    MMP_I2CM0,  // i2cmID
    (0x7A>>1),  // ubSlaveAddr  //D1=H, D0=H ' add=7Ah (7bit Type)
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
    150000,     // ulI2cmSpeed 400KHZ
    MMP_TRUE,   // bOsProtectEn
    NULL,       // sw_clk_pin
    NULL,       // sw_data_pin
    MMP_FALSE,  // bRfclModeEn
    MMP_FALSE,  // bWfclModeEn
    MMP_FALSE,	// bRepeatModeEn
    0           // ubVifPioMdlId
};

// 3A Timing
MMPF_SENSOR_AWBTIMIMG m_TW9992_AwbTime =
{
	6,	/* ubPeriod */
	1, 	/* ubDoAWBFrmCnt */
	3	/* ubDoCaliFrmCnt */
};

MMPF_SENSOR_AETIMIMG m_TW9992_AeTime =
{
	6, 	/* ubPeriod */
	0, 	/* ubFrmStSetShutFrmCnt */
	0	/* ubFrmStSetGainFrmCnt */
};

MMPF_SENSOR_AFTIMIMG m_TW9992_AfTime =
{
	1, 	/* ubPeriod */
	0	/* ubDoAFFrmCnt */
};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void ____Sensor_Init_OPR_Table____(){ruturn;} //dummy
#endif

ISP_UINT16 SNR_TW9992_Reg_Init_Customer[] = 
{
    // NTSC	
    0x00, 0x92,
    0x01, 0x00,
    0x02, 0x40,
    0x03, 0x78,
    0x04, 0x00,
    0x05, 0x09,
    0x06, 0x00,
    0x07, 0x02,
    0x08, 0x15,
    0x09, 0xF0,
    0x0A, 0x14,
    0x0B, 0xD0,
    0x0C, 0xCC,
    0x0D, 0x00,
    0x10, 0x00,
    0x11, 0x64,
    0x12, 0x11,
    0x13, 0x80,
    0x14, 0x80,
    0x15, 0x00,
    0x17, 0x80,
    0x18, 0x44,
    0x19, 0x06,
    0x1A, 0x10,
    0x1B, 0x00,
    0x1C, 0x0F,
    0x1D, 0x7F,
    0x1F, 0x00,
    0x20, 0x50,
    0x21, 0x22,
    0x22, 0xF0,
    0x23, 0xD8,
    0x24, 0xBC,
    0x25, 0xB8,
    0x26, 0x44,
    0x27, 0x38,
    0x28, 0x00,
    0x29, 0x00,
    0x2A, 0x78,
    0x2B, 0x44,
    0x2C, 0x30,
    0x2D, 0x14,
    0x2E, 0xA5,
    0x2F, 0xE0,
    0x30, 0x00,
    0x31, 0x10,
    0x32, 0xFF,
    0x33, 0x05,
    0x34, 0x1A,
    0x35, 0x00,
    0x36, 0x7A,
    0x37, 0x18,
    0x38, 0xDD,
    0x39, 0x00,
    0x3A, 0x30,
    0x3B, 0x00,
    0x3C, 0x00,
    0x3D, 0x00,
    0x3F, 0x1A,
    0x40, 0x80,
    0x41, 0x00,
    0x42, 0x00,
    0x48, 0x02,
    0x49, 0x00,
    0x4A, 0x81,
    0x4B, 0x0A,
    0x4C, 0x00,
    0x4D, 0x01,
    0x4E, 0x01,
    0x50, 0x00,
    0x51, 0x57,
    0x52, 0x00,
    0x53, 0x00,
    0x54, 0x06,
    0x55, 0x00,
    0x56, 0x00,
    0x57, 0x00,
    0x58, 0x00,
    0x60, 0x00,
    0x61, 0x00,
    0x62, 0x00,
    0x63, 0x00,
    0x70, 0x81, // b7[1]->MIPI ouput disable, b7[0]->MIPI output enable.
    0x71, 0x85,
    0x72, 0xA0,
    0x73, 0x00,
    0x74, 0xF0,
    0x75, 0x00,
    0x76, 0x17,
    0x77, 0x05,
    0x78, 0x88,
    0x79, 0x06,
    0x7A, 0x28,
    0x7B, 0x46,
    0x7C, 0xB3,
    0x7D, 0x06,
    0x7E, 0x13,
    0x7F, 0x00,
    0x80, 0x05,
    0x81, 0xA0,
    0x82, 0x12,
    0x83, 0x05,
    0x84, 0x02,
    0x85, 0x0E,
    0x86, 0x08,
    0x87, 0x37,
    0x88, 0x00,
    0x89, 0x00,
    0x8A, 0x02,
    0x8B, 0x11,
    0x8C, 0x22,
    0x8D, 0x03,
    0x8E, 0x22,
    0x8F, 0x01,
    0x90, 0x00,
    0x91, 0x0C,
    0x92, 0x00,
    0x93, 0x0E,
    0x94, 0x07,
    0x95, 0xFF,
    0x96, 0x1A,
    0x9B, 0x02,
    0xA0, 0x00,
    0xA1, 0x00,
    0xA2, 0x30,
    0xA3, 0xC0,
    0xA4, 0x00,
    0xC0, 0x06,
    0xC1, 0x20,
    0xC2, 0x20,
    0xC3, 0x08,
    0xC4, 0x09,
    0xC5, 0x10,
    0xC6, 0x06,
    0xC7, 0x00,
    0xC8, 0x38,
    0xC9, 0x10,
    0xCA, 0x1C,
    0xCB, 0x14,
    0xCC, 0x0A,
    0xCD, 0x1F,
    0xCE, 0x1A,
    0xCF, 0x0C,
    0xD0, 0x0C,
    0xD1, 0x0C,
    0xD2, 0x08,
    0xD3, 0x04,
    0xD4, 0x0C,
    0xD5, 0x04,
    0xD6, 0x00,
    0xD7, 0x00,
    0xD8, 0x01,
    0xD9, 0x68,
    0xDA, 0x00,
    0xDB, 0x01,
    0xDC, 0x20,
    0xE0, 0x78,
    0xE1, 0x77,
    0xE2, 0x4F,
    0xE3, 0xAC,
    0xE4, 0x2A,
    0xE5, 0xE6,
    0xE6, 0x05,
    0xE7, 0xEA,
    0xE8, 0x03,
    0xE9, 0x37,
    0xEA, 0x4E,
    0xEB, 0x3E,
    0xEC, 0x3C,
    0xED, 0x43,
    0xEE, 0x77,
    0xEF, 0xB1,
    0xF0, 0x3C,
    0xF1, 0x7C,
    0xF2, 0xB4,
    0xF3, 0x02,
    0xF4, 0x00,
    0xF5, 0x00,

	SENSOR_DELAY_REG, 20,
	0x70, 0x01, // b7[1]->MIPI ouput disable, b7[0]->MIPI output enable
};

#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} //dummy
#endif

ISP_UINT16 SNR_TW9992_Reg_720x240_30P[] = 
{
    SENSOR_DELAY_REG, 0 // delay
};

#if 0
void ____Sensor_Customer_Func____(){ruturn;} // dummy
#endif

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_InitConfig
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_InitConfig(void)
{
    MMPF_SENSOR_CUSTOMER *pCust = NULL;

#if (TVDEC_SNR_PROI == PRM_SENSOR)
    pCust = &SensorCustFunc;
#else
    pCust = &SubSensorCustFunc;
#endif

    RTNA_DBG_Str(0, FG_PURPLE("SNR_Cust_InitConfig TW9992\r\n"));

    // Init Res Table
    if (MMP_GetTvDecSnrAttr()->bRawStorePathEnable)
        pCust->ResTable                                     = &m_TW9992_SensorRes_RawStore_Mode;
    else
        pCust->ResTable                                     = &m_TW9992_SensorRes_BypassISP_Mode;

    // Init OPR Table
    pCust->OprTable->usInitSize 						    = (sizeof(SNR_TW9992_Reg_Init_Customer)/sizeof(SNR_TW9992_Reg_Init_Customer[0]))/2;
    pCust->OprTable->uspInitTable 					        = &SNR_TW9992_Reg_Init_Customer[0];

    pCust->OprTable->bBinTableExist                         = MMP_FALSE;
    pCust->OprTable->bInitDoneTableExist                    = MMP_FALSE;

    // Share with NTSC format due to no initial setting.
    pCust->OprTable->usSize[RES_IDX_PAL_25FPS]		        = (sizeof(SNR_TW9992_Reg_720x240_30P)/sizeof(SNR_TW9992_Reg_720x240_30P[0]))/2;
    pCust->OprTable->uspTable[RES_IDX_PAL_25FPS] 	        = &SNR_TW9992_Reg_720x240_30P[0];
    pCust->OprTable->usSize[RES_IDX_NTSC_30FPS]		        = (sizeof(SNR_TW9992_Reg_720x240_30P)/sizeof(SNR_TW9992_Reg_720x240_30P[0]))/2;
    pCust->OprTable->uspTable[RES_IDX_NTSC_30FPS] 	        = &SNR_TW9992_Reg_720x240_30P[0];

    // Init Vif Setting : Common
    pCust->VifSetting->SnrType                      		= MMPF_VIF_SNR_TYPE_YUV_TV_DEC;
#if (TVDEC_SNR_IF == SENSOR_IF_PARALLEL)
    pCust->VifSetting->OutInterface 					    = MMPF_VIF_IF_PARALLEL;
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_1_LANE)
    pCust->VifSetting->OutInterface 				        = MMPF_VIF_IF_MIPI_SINGLE_0;
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_2_LANE)
    pCust->VifSetting->OutInterface 					    = MMPF_VIF_IF_MIPI_DUAL_12;
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_4_LANE)
    pCust->VifSetting->OutInterface				            = MMPF_VIF_IF_MIPI_QUAD;
#endif
    pCust->VifSetting->VifPadId 						    = MMPF_VIF_MDL_ID1;

    // Init Vif Setting : PowerOn Setting
    pCust->VifSetting->powerOnSetting.bTurnOnExtPower       = MMP_TRUE;
    pCust->VifSetting->powerOnSetting.usExtPowerPin 	    = SENSOR_GPIO_ENABLE; // it might be defined in Config_SDK.h
    pCust->VifSetting->powerOnSetting.bExtPowerPinHigh      = SENSOR_GPIO_ENABLE_ACT_LEVEL;
    pCust->VifSetting->powerOnSetting.usExtPowerPinDelay    = 1;
    pCust->VifSetting->powerOnSetting.bFirstEnPinHigh       = MMP_TRUE;
    pCust->VifSetting->powerOnSetting.ubFirstEnPinDelay     = 5;
    pCust->VifSetting->powerOnSetting.bNextEnPinHigh        = MMP_FALSE;
    pCust->VifSetting->powerOnSetting.ubNextEnPinDelay      = 5;
    pCust->VifSetting->powerOnSetting.bTurnOnClockBeforeRst = MMP_TRUE;
    pCust->VifSetting->powerOnSetting.bFirstRstPinHigh      = MMP_FALSE;
    pCust->VifSetting->powerOnSetting.ubFirstRstPinDelay    = 5;
    pCust->VifSetting->powerOnSetting.bNextRstPinHigh       = MMP_TRUE;
    pCust->VifSetting->powerOnSetting.ubNextRstPinDelay     = 5;

    // Init Vif Setting : PowerOff Setting
    pCust->VifSetting->powerOffSetting.bEnterStandByMode    = MMP_FALSE;
    pCust->VifSetting->powerOffSetting.usStandByModeReg     = 0x0100;
    pCust->VifSetting->powerOffSetting.usStandByModeMask    = 0x01;
    pCust->VifSetting->powerOffSetting.bEnPinHigh 	        = MMP_TRUE;
    pCust->VifSetting->powerOffSetting.ubEnPinDelay 	    = 20;
    pCust->VifSetting->powerOffSetting.bTurnOffMClock       = MMP_TRUE;
    pCust->VifSetting->powerOffSetting.bTurnOffExtPower     = MMP_FALSE;
    pCust->VifSetting->powerOffSetting.usExtPowerPin        = SENSOR_GPIO_ENABLE; // it might be defined in Config_SDK.h

    // Init Vif Setting : Sensor MClock Setting
    pCust->VifSetting->clockAttr.bClkOutEn 			        = MMP_TRUE;
    pCust->VifSetting->clockAttr.ubClkFreqDiv 		        = 0;
    pCust->VifSetting->clockAttr.ulMClkFreq 			    = 24000;
    pCust->VifSetting->clockAttr.ulDesiredFreq 		        = 24000;
    pCust->VifSetting->clockAttr.ubClkPhase 		        = MMPF_VIF_SNR_PHASE_DELAY_NONE;
    pCust->VifSetting->clockAttr.ubClkPolarity 		        = MMPF_VIF_SNR_CLK_POLARITY_NEG;
    pCust->VifSetting->clockAttr.ubClkSrc 			        = MMPF_VIF_SNR_CLK_SRC_VIFCLK;

    // Init Vif Setting : Parallel Sensor Setting
    pCust->VifSetting->paralAttr.ubLatchTiming 		        = MMPF_VIF_SNR_LATCH_NEG_EDGE;
    pCust->VifSetting->paralAttr.ubHsyncPolarity 	        = MMPF_VIF_SNR_CLK_POLARITY_NEG;
    pCust->VifSetting->paralAttr.ubVsyncPolarity 	        = MMPF_VIF_SNR_CLK_POLARITY_POS;
    pCust->VifSetting->paralAttr.ubBusBitMode               = MMPF_VIF_SNR_PARAL_BITMODE_10;

    // Init Vif Setting : Color ID Setting
    pCust->VifSetting->colorId.VifColorId 				    = MMPF_VIF_COLORID_00;
    pCust->VifSetting->colorId.CustomColorId.bUseCustomId 	= MMP_FALSE;

    // Init Vif Setting : MIPI Sensor Setting
    pCust->VifSetting->mipiAttr.bClkDelayEn                 = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bClkLaneSwapEn              = MMP_FALSE;
    pCust->VifSetting->mipiAttr.usClkDelay                  = 0;
    pCust->VifSetting->mipiAttr.ubBClkLatchTiming           = MMPF_VIF_SNR_LATCH_NEG_EDGE;
   
#if (TVDEC_SNR_IF == SENSOR_IF_MIPI_1_LANE)
    pCust->VifSetting->mipiAttr.bDataLaneEn[0]              = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataLaneEn[1]              = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneEn[2]              = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneEn[3]              = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[0]             = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[1]             = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[2]             = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[3]             = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[0]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[1]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[2]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[3]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[0]            = MMPF_VIF_MIPI_DATA_SRC_PHY_1;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[1]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[2]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[3]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    pCust->VifSetting->mipiAttr.usDataDelay[0]              = 0;
    pCust->VifSetting->mipiAttr.usDataDelay[1]              = 0;
    pCust->VifSetting->mipiAttr.usDataDelay[2]              = 0;
    pCust->VifSetting->mipiAttr.usDataDelay[3]              = 0;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[0]             = 0x1F;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[1]             = 0x1F;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[2]             = 0x1F;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[3]             = 0x1F;
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_2_LANE)
    pCust->VifSetting->mipiAttr.bDataLaneEn[0]              = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataLaneEn[1]              = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataLaneEn[2]              = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneEn[3]              = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[0]             = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[1]             = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[2]             = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[3]             = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[0]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[1]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[2]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[3]          = MMP_FALSE;
    #if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[0]            = MMPF_VIF_MIPI_DATA_SRC_PHY_1;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[1]            = MMPF_VIF_MIPI_DATA_SRC_PHY_2;
    #else
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[0]            = MMPF_VIF_MIPI_DATA_SRC_PHY_0;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[1]            = MMPF_VIF_MIPI_DATA_SRC_PHY_1;
    #endif
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[2]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[3]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    pCust->VifSetting->mipiAttr.usDataDelay[0]              = 0;
    pCust->VifSetting->mipiAttr.usDataDelay[1]              = 0;
    pCust->VifSetting->mipiAttr.usDataDelay[2]              = 0;
    pCust->VifSetting->mipiAttr.usDataDelay[3]              = 0;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[0]             = 0x1F;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[1]             = 0x1F;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[2]             = 0x1F;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[3]             = 0x1F;
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_4_LANE)
    pCust->VifSetting->mipiAttr.bDataLaneEn[0]              = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataLaneEn[1]              = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataLaneEn[2]              = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataLaneEn[3]              = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[0]             = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[1]             = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[2]             = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataDelayEn[3]             = MMP_TRUE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[0]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[1]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[2]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bDataLaneSwapEn[3]          = MMP_FALSE;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[0]            = MMPF_VIF_MIPI_DATA_SRC_PHY_0;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[1]            = MMPF_VIF_MIPI_DATA_SRC_PHY_1;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[2]            = MMPF_VIF_MIPI_DATA_SRC_PHY_2;
    pCust->VifSetting->mipiAttr.ubDataLaneSrc[3]            = MMPF_VIF_MIPI_DATA_SRC_PHY_3;
    pCust->VifSetting->mipiAttr.usDataDelay[0]              = 0;
    pCust->VifSetting->mipiAttr.usDataDelay[1]              = 0;
    pCust->VifSetting->mipiAttr.usDataDelay[2]              = 0;
    pCust->VifSetting->mipiAttr.usDataDelay[3]              = 0;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[0]             = 0x1F;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[1]             = 0x1F;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[2]             = 0x1F;
    pCust->VifSetting->mipiAttr.ubDataSotCnt[3]             = 0x1F;
#endif

    // Init Vif Setting : YUV Setting
    pCust->VifSetting->yuvAttr.bRawStoreEnable              = MMP_TRUE;
    pCust->VifSetting->yuvAttr.bYuv422ToYuv420              = MMP_FALSE;
    pCust->VifSetting->yuvAttr.bYuv422ToYuv422              = MMP_TRUE;
    pCust->VifSetting->yuvAttr.bYuv422ToBayer               = MMP_FALSE;
    pCust->VifSetting->yuvAttr.ubYuv422Order                = MMPF_VIF_YUV422_VYUY;
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_DoAWB_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_DoAWB_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_DoAWB_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_DoIQ
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_DoIQ(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_SetGain
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_SetGain(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_SetExposure
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_SetExposure(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain, MMP_ULONG ulShutter, MMP_ULONG ulVsync)
{
    //TBD
    printc(FG_RED("Warning!!! Please review SNR_Cust_SetExposure in sensor driver!!!\r\n"));
    gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, ulShutter, ulVsync);
    gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, ulGain);
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_SetFlip
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_SetFlip(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_SetRotate
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_SetRotate(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_CheckVersion
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_CheckVersion(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_GetIqCompressData
//  Description :
//------------------------------------------------------------------------------
static const MMP_UBYTE* SNR_TW9992_Cust_GetIqCompressData(MMP_UBYTE ubSnrSel)
{
    return NULL;
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_StreamEnable
//  Description : Enable/Disable streaming of sensor
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_StreamEnable(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_CheckSnrTVSrcType
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_CheckSnrTVSrcType(MMP_UBYTE ubSnrSel, MMP_SNR_TVDEC_SRC_TYPE *TVSrcType)
{
#if !defined(MBOOT_FW)
    MMP_USHORT usSrcType = 0;
    MMP_USHORT usPID = 0;
    MMP_USHORT usVideoDetect = 0;

	gsSensorFunction->MMPF_Sensor_GetReg(ubSnrSel, 0x00, &usPID);
	
	if (usPID != 0x92)
	{
		printc("TVSrc err\r\n");
		*TVSrcType =  MMP_SNR_TVDEC_SRC_NO_READY;
		return;
	}
	
	gsSensorFunction->MMPF_Sensor_GetReg(ubSnrSel, 0x54, &usVideoDetect);
	
	if (usVideoDetect & 0x20)
	{
		printc("TVSrc err\r\n");
		*TVSrcType =  MMP_SNR_TVDEC_SRC_NO_READY;
		return;
	}
	
	gsSensorFunction->MMPF_Sensor_GetReg(ubSnrSel, 0x1C, &usSrcType);
    usSrcType = (usSrcType & 0x70) >> 4;
    
	if ((usSrcType == 7) || (usSrcType == 2)) {
		*TVSrcType =  MMP_SNR_TVDEC_SRC_NO_READY;
	}
	else if ((usSrcType == 0) || (usSrcType == 3)) {
		*TVSrcType =  MMP_SNR_TVDEC_SRC_NTSC;
	}
	else {
		*TVSrcType =  MMP_SNR_TVDEC_SRC_PAL;
	}
	printc("TVSrc = %d\r\n", *TVSrcType);
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_TW9992_Cust_GetSnrID
//  Description :
//------------------------------------------------------------------------------
static void SNR_TW9992_Cust_GetSnrID(MMP_UBYTE ubSnrSel, MMP_ULONG *SensorID)
{
    *SensorID = 0xbbbb9992; // The ID is customized.Pls make it different with other sensors.
}

#if 0
void ____Sensor_Cust_Func_Struc____(){ruturn;} // dummy
#endif

#if (TVDEC_SNR_PROI == PRM_SENSOR)
MMPF_SENSOR_CUSTOMER SensorCustFunc = 
{
    SNR_TW9992_Cust_InitConfig,
    SNR_TW9992_Cust_DoAE_FrmSt,
    SNR_TW9992_Cust_DoAE_FrmEnd,
    SNR_TW9992_Cust_DoAWB_FrmSt,
    SNR_TW9992_Cust_DoAWB_FrmEnd,
    SNR_TW9992_Cust_DoIQ,
    SNR_TW9992_Cust_SetGain,
    SNR_TW9992_Cust_SetShutter,
    SNR_TW9992_Cust_SetExposure,
    SNR_TW9992_Cust_SetFlip,
    SNR_TW9992_Cust_SetRotate,
    SNR_TW9992_Cust_CheckVersion,
    SNR_TW9992_Cust_GetIqCompressData,
    SNR_TW9992_Cust_StreamEnable,

	&m_TW9992_SensorRes_RawStore_Mode,
	&m_TW9992_OprTable,
	&m_TW9992_VifSetting,
	&m_TW9992_I2cmAttr,
	&m_TW9992_AwbTime,
	&m_TW9992_AeTime,
	&m_TW9992_AfTime,
    MMP_SNR_PRIO_PRM,
    
	SNR_TW9992_Cust_CheckSnrTVSrcType,
	SNR_TW9992_Cust_GetSnrID
};

int SNR_Module_Init(void)
{
    MMPF_SensorDrv_Register(PRM_SENSOR, &SensorCustFunc);
    
    return 0;
}
#else
MMPF_SENSOR_CUSTOMER SubSensorCustFunc = 
{
    SNR_TW9992_Cust_InitConfig,
    SNR_TW9992_Cust_DoAE_FrmSt,
    SNR_TW9992_Cust_DoAE_FrmEnd,
    SNR_TW9992_Cust_DoAWB_FrmSt,
    SNR_TW9992_Cust_DoAWB_FrmEnd,
    SNR_TW9992_Cust_DoIQ,
    SNR_TW9992_Cust_SetGain,
    SNR_TW9992_Cust_SetShutter,
    SNR_TW9992_Cust_SetExposure,
    SNR_TW9992_Cust_SetFlip,
    SNR_TW9992_Cust_SetRotate,
    SNR_TW9992_Cust_CheckVersion,
    SNR_TW9992_Cust_GetIqCompressData,
    SNR_TW9992_Cust_StreamEnable,

	&m_TW9992_SensorRes_RawStore_Mode,
	&m_TW9992_OprTable,
	&m_TW9992_VifSetting,
	&m_TW9992_I2cmAttr,
	&m_TW9992_AwbTime,
	&m_TW9992_AeTime,
	&m_TW9992_AfTime,
    MMP_SNR_PRIO_SCD,
    
    SNR_TW9992_Cust_CheckSnrTVSrcType,
    SNR_TW9992_Cust_GetSnrID
};

int SubSNR_Module_Init(void)
{
#if !defined(MBOOT_FW)
    MMPF_SensorDrv_Register(SCD_SENSOR, &SubSensorCustFunc); 
#endif
    return 0;
}
#endif

#pragma arm section code = "initcall6", rodata = "initcall6", rwdata = "initcall6", zidata = "initcall6"
#pragma O0
#if (TVDEC_SNR_PROI == PRM_SENSOR)
ait_module_init(SNR_Module_Init);
#else
ait_module_init(SubSNR_Module_Init);
#endif
#pragma
#pragma arm section rodata, rwdata, zidata

#endif // (BIND_SENSOR_TW9992)
#endif // (SENSOR_EN)
