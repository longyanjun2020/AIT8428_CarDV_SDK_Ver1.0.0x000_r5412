//==============================================================================
//
//  File        : sensor_GC2023.c
//  Description : Firmware Sensor Control File
//  Author      :
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
#if (BIND_SENSOR_GC2023)

#include "mmpf_sensor.h"
#include "Sensor_Mod_Remapping.h"
#include "isp_if.h"

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================

// Resolution Table
MMPF_SENSOR_RESOLUTION m_SensorRes = 
{
	4, 												// ubSensorModeNum
	0, 												// ubDefPreviewMode
	2, 												// ubDefCaptureMode
	3000,                                           // usPixelSize
// Mode0 	Mode1	Mode2 	Mode3
	{1,  	321,  	241,   	641}, 	// usVifGrabStX
	{1,  	181,	1,     	301}, 	// usVifGrabStY
	{1928, 	1288,   1448,  	648}, 	// usVifGrabW
	{1088, 	724,    1088,  	488}, 	// usVifGrabH
#if (CHIP == MCR_V2)
	{1,   	1,      1,      1}, 	// usBayerInGrabX
	{1,   	1,      1,      1}, 	// usBayerInGrabY
	{8,   	8,      8,      8}, 	// usBayerInDummyX
	{8,   	4,      8,      8}, 	// usBayerInDummyY
	{1920, 	1280,   1440,	640}, 	// usBayerOutW
	{1080, 	720,    1080, 	480}, 	// usBayerOutH
#endif
	{1920, 	1280,   1440, 	640}, 	// usScalInputW
	{1080, 	720,    1080, 	480}, 	// usScalInputH
	{300,   300,    300,    300}, 	// usTargetFpsx10
	{1104, 	1104,	1104,   1104}, 	// usVsyncLine
	{1,   	1,      1,      1}, 	// ubWBinningN
	{1,   	1,      1,      1}, 	// ubWBinningM
	{1,   	1,      1,      1}, 	// ubHBinningN
	{1,   	1,      1,      1}, 	// ubHBinningM
    {1,		0,      0,      0},    	// ubCustIQmode
    {0,		0,      0,      0}     	// ubCustAEmode
};

// OPR Table and Vif Setting
MMPF_SENSOR_OPR_TABLE 	m_OprTable;
MMPF_SENSOR_VIF_SETTING m_VifSetting;

// IQ Table
const ISP_UINT8 Sensor_IQ_CompressedText[] = 
{
#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC) //TBD          
//#include "isp_8428_iq_data_v3_OV2710_ezmode.xls.ciq.txt"
#include "isp_8428_iq_data_v3_GC2023_ezmode.xls.ciq.txt"
#else //Use old IQ table
#ifdef CUS_ISP_8428_IQ_DATA
#include CUS_ISP_8428_IQ_DATA
#else
#include "isp_8428_iq_data_v3_GC2023_ezmode.xls.ciq.txt"
#endif
#endif
};

//#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC) //TBD          
const  __align(4) ISP_UINT8 Sensor_EZ_IQ_CompressedText[] = 
{
//#include "ez_isp_8428_ov4689.txt"
//#include "eziq_0413.txt"
//#include "eziq_0509.txt"
0
};

ISP_UINT32 eziqsize = sizeof(Sensor_EZ_IQ_CompressedText);
//#endif

// I2cm Attribute
static MMP_I2CM_ATTR m_I2cmAttr = 
{
	MMP_I2CM0,      // i2cmID
    0x37,       	// ubSlaveAddr
	8, 				// ubRegLen
	8, 				// ubDataLen
	0, 				// ubDelayTime
	MMP_FALSE, 		// bDelayWaitEn
	MMP_TRUE, 		// bInputFilterEn
	MMP_FALSE, 		// b10BitModeEn
	MMP_FALSE, 		// bClkStretchEn
	0, 				// ubSlaveAddr1
	0, 				// ubDelayCycle
	0, 				// ubPadNum
	200000, 		// ulI2cmSpeed 250KHZ
	MMP_TRUE, 		// bOsProtectEn
	NULL, 			// sw_clk_pin
	NULL, 			// sw_data_pin
	MMP_FALSE, 		// bRfclModeEn
	MMP_FALSE,      // bWfclModeEn
	MMP_FALSE,		// bRepeatModeEn
    0               // ubVifPioMdlId
};

// 3A Timing
MMPF_SENSOR_AWBTIMIMG m_AwbTime = 
{
	6, 	// ubPeriod
	1, 	// ubDoAWBFrmCnt
	3 	// ubDoCaliFrmCnt
};

MMPF_SENSOR_AETIMIMG m_AeTime = 
{
	4, 	// ubPeriod
	0, 	// ubFrmStSetShutFrmCnt
	0 	// ubFrmStSetGainFrmCnt
};

MMPF_SENSOR_AFTIMIMG m_AfTime = 
{
	1, 	// ubPeriod
	0 	// ubDoAFFrmCnt
};


//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void ____Sensor_Init_OPR_Table____(){ruturn;} //dummy
#endif

ISP_UINT16 SNR_GC2023_Reg_Unsupport[] = 
{
	SENSOR_DELAY_REG, 100 // delay
};

ISP_UINT16 SNR_GC2023_Reg_Init_Customer[] = 
{	
	// TBD
	SENSOR_DELAY_REG, 10 // delay
};

#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} //dummy
#endif

// 1928x1088 30fps
ISP_UINT16 SNR_GC2023_Reg_1920x1080_30P_Customer[] = 
{
/////////////////////////////////////////////////////
//////////////////////   SYS   //////////////////////
/////////////////////////////////////////////////////
0xf2,0x00,
0xf6,0x00,
0xfc,0x06,
0xf7,0x01,
0xf8,0x0f,
0xf9,0x0e,
0xfa,0x00,
0xfc,0x1e,
/////////////////////////////////////////////////////
////////////////   ANALOG & CISCTL   ////////////////
/////////////////////////////////////////////////////
0xfe,0x00,
0x03,0x03, 
0x04,0xf6, 
0x05,0x02, //HB
0x06,0xc6,
0x07,0x00, //VB
0x08,0x10, 
0x09,0x00,
0x0a,0x00, //row start
0x0b,0x00,
0x0c,0x00, //col start
0x0d,0x04, 
0x0e,0x40, //height 1088
0x0f,0x07, 
0x10,0x88, //width 1928
0x17,0x54,
0x18,0x02,
0x19,0x0d,
0x1a,0x18,

0x20,0x54,
0x23,0xf0,
0x24,0xc1,
0x25,0x18,
0x26,0x64,
0x28,0xf4,
0x29,0x08,
0x2a,0x08,
0x2b,0x4c,
0x2f,0x40,
0x30,0x99,
0x34,0x00,
0x38,0x80,
0x3b,0x12,
0x3d,0xb0,
0xcc,0x8a,
0xcd,0x99,
0xcf,0x70,
0xd0,0x9c,
0xd2,0xc1,
0xd8,0x80,
0xda,0x28,
0xdc,0x24,
0xe1,0x14,
0xe3,0xf0,
0xe4,0xfa,
0xe6,0x1f,
0xe8,0x02,
0xe9,0x02,
0xea,0x03,
0xeb,0x03,
/////////////////////////////////////////////////////
//////////////////////   ISP   //////////////////////
/////////////////////////////////////////////////////
0xfe,0x00,
0x80,0x5c,
0x88,0x73,
0x89,0x03,
0x90,0x01, 
0x92,0x00, //crop win y
0x94,0x00, //crop win x
0x95,0x04, //crop win height
0x96,0x40,
0x97,0x07, //crop win width
0x98,0x88,
/////////////////////////////////////////////////////
//////////////////////   BLK   //////////////////////
/////////////////////////////////////////////////////
0xfe,0x00,
0x40,0x22,
0x4e,0x3c,
0x4f,0x00,
0x60,0x00,
0x61,0x80,
/////////////////////////////////////////////////////
//////////////////////   GAIN   /////////////////////
/////////////////////////////////////////////////////
0xfe,0x00,
0xb0,0x58,
0xb1,0x01, 
0xb2,0x00, 
0xb6,0x00, 
0xfe,0x01,
0x01,0x00,
0x02,0x01,
0x03,0x02,
0x04,0x03,
0x05,0x04,
0x06,0x05,
0x07,0x06,
0x08,0x0e,
0x09,0x16,
0x0a,0x1e,
0x0b,0x36,
0x0c,0x3e,
0x0d,0x56,
0x0e,0x5e,
/////////////////////////////////////////////////////
//////////////////////   DNDD   /////////////////////
/////////////////////////////////////////////////////
0xfe,0x02,
0x81,0x05,
/////////////////////////////////////////////////////
//////////////////////   dark sun   /////////////////
/////////////////////////////////////////////////////
0xfe,0x01,
0x54,0x77,
0x58,0x00,
0x5a,0x05,
/////////////////////////////////////////////////////
//////////////////////	 MIPI	/////////////////////
/////////////////////////////////////////////////////
0xfe,0x03,
0x01,0x5b,
0x02,0x10,
0x03,0x9a,
0x10,0x90,
0x11,0x2b,
0x12,0x6a, //lwc 1928*5/4
0x13,0x09,
0x15,0x06,
0x36,0x88,///
0x21,0x0b,
0x22,0x05,
0x23,0x30,
0x24,0x02,
0x25,0x12,
0x26,0x08,
0x29,0x06,
0x2a,0x08,
0x2b,0x08,
0xfe,0x00,
};

// 1928x1088 25fps
ISP_UINT16 SNR_GC2023_Reg_1920x1080_25P_Customer[] = 
{
	SENSOR_DELAY_REG, 100 // delay
};

// 1288x728 30fps
MMP_USHORT SNR_GC2023_Reg_1280x720_30P_Customer[] = 
{
	SENSOR_DELAY_REG, 100 // delay
};

// 1288x728 60fps
MMP_USHORT SNR_GC2023_Reg_1280x720_60P_Customer[] = 
{
	SENSOR_DELAY_REG, 100 // delay
};

// 1448x1088 30fps
ISP_UINT16 SNR_GC2023_Reg_1440x1080_30P_Customer[] = 
{
	SENSOR_DELAY_REG, 100 // delay
};

// 648x488 30fps
ISP_UINT16 SNR_GC2023_Reg_640x480_30P_Customer[] = 
{
	SENSOR_DELAY_REG, 100 // delay
};

#if 0
void ____Sensor_Customer_Func____(){ruturn;} // dummy
#endif

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_InitConfig
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_InitConfig(void)
{
    MMP_USHORT i;

    RTNA_DBG_Str(0, "!!SNR_Cust_InitConfig GC2023\r\n");

    // Init OPR Table
    SensorCustFunc.OprTable->usInitSize 							= (sizeof(SNR_GC2023_Reg_Init_Customer)/sizeof(SNR_GC2023_Reg_Init_Customer[0]))/2;
    SensorCustFunc.OprTable->uspInitTable 							= &SNR_GC2023_Reg_Init_Customer[0];

    SensorCustFunc.OprTable->bBinTableExist                         = MMP_FALSE;
    SensorCustFunc.OprTable->bInitDoneTableExist                    = MMP_FALSE;

    for (i = 0; i < MAX_SENSOR_RES_MODE; i++)
    {
        SensorCustFunc.OprTable->usSize[i] 							= (sizeof(SNR_GC2023_Reg_Unsupport)/sizeof(SNR_GC2023_Reg_Unsupport[0]))/2;
        SensorCustFunc.OprTable->uspTable[i] 						= &SNR_GC2023_Reg_Unsupport[0];
    }

    // 16:9
    SensorCustFunc.OprTable->usSize[RES_IDX_1920x1080_30P] 			= (sizeof(SNR_GC2023_Reg_1920x1080_30P_Customer)/sizeof(SNR_GC2023_Reg_1920x1080_30P_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_1920x1080_30P] 		= &SNR_GC2023_Reg_1920x1080_30P_Customer[0];
    //SensorCustFunc.OprTable->usSize[RES_IDX_1920x1080_25P] 			= (sizeof(SNR_GC2023_Reg_1920x1080_25P_Customer)/sizeof(SNR_GC2023_Reg_1920x1080_25P_Customer[0]))/2;
    //SensorCustFunc.OprTable->uspTable[RES_IDX_1920x1080_25P] 		= &SNR_GC2023_Reg_1920x1080_25P_Customer[0];
#if 1 // cropped from FHD30P, TBC??
    SensorCustFunc.OprTable->usSize[RES_IDX_1280x720_30P] 			= (sizeof(SNR_GC2023_Reg_1920x1080_30P_Customer)/sizeof(SNR_GC2023_Reg_1920x1080_30P_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_1280x720_30P] 		= &SNR_GC2023_Reg_1920x1080_30P_Customer[0];
#else
    //SensorCustFunc.OprTable->usSize[RES_IDX_1280x720_30P] 			= (sizeof(SNR_GC2023_Reg_1280x720_30P_Customer)/sizeof(SNR_GC2023_Reg_1280x720_30P_Customer[0]))/2; // TBD
    //SensorCustFunc.OprTable->uspTable[RES_IDX_1280x720_30P] 		= &SNR_GC2023_Reg_1280x720_30P_Customer[0];
#endif
    //SensorCustFunc.OprTable->usSize[RES_IDX_1280x720_60P] 			= (sizeof(SNR_GC2023_Reg_1280x720_60P_Customer)/sizeof(SNR_GC2023_Reg_1280x720_60P_Customer[0]))/2; // TBD
    //SensorCustFunc.OprTable->uspTable[RES_IDX_1280x720_60P] 		= &SNR_GC2023_Reg_1280x720_60P_Customer[0];

    // 4:3
#if 1 // cropped from FHD30P, TBC??
    SensorCustFunc.OprTable->usSize[RES_IDX_1440x1080_30P] 			= (sizeof(SNR_GC2023_Reg_1920x1080_30P_Customer)/sizeof(SNR_GC2023_Reg_1920x1080_30P_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_1440x1080_30P] 		= &SNR_GC2023_Reg_1920x1080_30P_Customer[0];
    SensorCustFunc.OprTable->usSize[RES_IDX_640x480_30P] 			= (sizeof(SNR_GC2023_Reg_1920x1080_30P_Customer)/sizeof(SNR_GC2023_Reg_1920x1080_30P_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_640x480_30P] 			= &SNR_GC2023_Reg_1920x1080_30P_Customer[0];
#else
    //SensorCustFunc.OprTable->usSize[RES_IDX_1440x1080_30P] 			= (sizeof(SNR_GC2023_Reg_1440x1080_30P_Customer)/sizeof(SNR_GC2023_Reg_1440x1080_30P_Customer[0]))/2;
    //SensorCustFunc.OprTable->uspTable[RES_IDX_1440x1080_30P] 		= &SNR_GC2023_Reg_1440x1080_30P_Customer[0];
    //SensorCustFunc.OprTable->usSize[RES_IDX_640x480_30P] 			= (sizeof(SNR_GC2023_Reg_640x480_30P_Customer)/sizeof(SNR_GC2023_Reg_640x480_30P_Customer[0]))/2;
    //SensorCustFunc.OprTable->uspTable[RES_IDX_640x480_30P] 			= &SNR_GC2023_Reg_640x480_30P_Customer[0];
#endif

    // Init Vif Setting : Common
    SensorCustFunc.VifSetting->SnrType                              = MMPF_VIF_SNR_TYPE_BAYER;
#if (SENSOR_IF == SENSOR_IF_PARALLEL)
    SensorCustFunc.VifSetting->OutInterface 						= MMPF_VIF_IF_PARALLEL;
#elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
#if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
    SensorCustFunc.VifSetting->OutInterface 						= MMPF_VIF_IF_MIPI_SINGLE_1;//use for 2531project
#else 	
    SensorCustFunc.VifSetting->OutInterface 						= MMPF_VIF_IF_MIPI_SINGLE_0;//use for HSG
#endif
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
#if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
    SensorCustFunc.VifSetting->OutInterface                         = MMPF_VIF_IF_MIPI_DUAL_01;
#else
    SensorCustFunc.VifSetting->OutInterface                         = MMPF_VIF_IF_MIPI_DUAL_01; //TBD
#endif
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
    SensorCustFunc.VifSetting->OutInterface                         = MMPF_VIF_IF_MIPI_QUAD;
#endif

#if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1) //MIPI 2 lane or 1 lane
    SensorCustFunc.VifSetting->VifPadId  = MMPF_VIF_MDL_ID1; 
#else 
    SensorCustFunc.VifSetting->VifPadId = MMPF_VIF_MDL_ID0; 
#endif     
    // Init Vif Setting : PowerOn Setting
    SensorCustFunc.VifSetting->powerOnSetting.bTurnOnExtPower 		= MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.usExtPowerPin 		= SENSOR_GPIO_ENABLE; // it might be defined in Config_SDK.h
    SensorCustFunc.VifSetting->powerOnSetting.bExtPowerPinHigh 		= SENSOR_GPIO_ENABLE_ACT_LEVEL;
    SensorCustFunc.VifSetting->powerOnSetting.usExtPowerPinDelay 	= 0; // 0 ms which is suggested by Kenny Shih @ 20150327
    SensorCustFunc.VifSetting->powerOnSetting.bFirstEnPinHigh 		= MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.ubFirstEnPinDelay 	= 10;
    SensorCustFunc.VifSetting->powerOnSetting.bNextEnPinHigh 		= MMP_FALSE;
    SensorCustFunc.VifSetting->powerOnSetting.ubNextEnPinDelay 		= 10;
    SensorCustFunc.VifSetting->powerOnSetting.bTurnOnClockBeforeRst = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.bFirstRstPinHigh 		= MMP_FALSE;
    SensorCustFunc.VifSetting->powerOnSetting.ubFirstRstPinDelay 	= 10;
    SensorCustFunc.VifSetting->powerOnSetting.bNextRstPinHigh 		= MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.ubNextRstPinDelay 	= 10;

    // Init Vif Setting : PowerOff Setting
    SensorCustFunc.VifSetting->powerOffSetting.bEnterStandByMode 	= MMP_FALSE;
    SensorCustFunc.VifSetting->powerOffSetting.usStandByModeReg 	= 0x0;
    SensorCustFunc.VifSetting->powerOffSetting.usStandByModeMask 	= 0x0;
    SensorCustFunc.VifSetting->powerOffSetting.bEnPinHigh 			= MMP_TRUE;
    SensorCustFunc.VifSetting->powerOffSetting.ubEnPinDelay 		= 10;
    SensorCustFunc.VifSetting->powerOffSetting.bTurnOffMClock 		= MMP_TRUE;
    SensorCustFunc.VifSetting->powerOffSetting.bTurnOffExtPower 	= MMP_TRUE;
    SensorCustFunc.VifSetting->powerOffSetting.usExtPowerPin 		= SENSOR_GPIO_ENABLE; // it might be defined in Config_SDK.h

    // Init Vif Setting : Sensor MClock Setting
    SensorCustFunc.VifSetting->clockAttr.bClkOutEn 					= MMP_TRUE;
    SensorCustFunc.VifSetting->clockAttr.ubClkFreqDiv 				= 0;
    SensorCustFunc.VifSetting->clockAttr.ulMClkFreq 				= 24000;
    SensorCustFunc.VifSetting->clockAttr.ulDesiredFreq 				= 24000;
    SensorCustFunc.VifSetting->clockAttr.ubClkPhase 				= MMPF_VIF_SNR_PHASE_DELAY_NONE;
    SensorCustFunc.VifSetting->clockAttr.ubClkPolarity 				= MMPF_VIF_SNR_CLK_POLARITY_POS;
    SensorCustFunc.VifSetting->clockAttr.ubClkSrc 					= MMPF_VIF_SNR_CLK_SRC_PMCLK;

    // Init Vif Setting : Parallel Sensor Setting
    SensorCustFunc.VifSetting->paralAttr.ubLatchTiming 				= MMPF_VIF_SNR_LATCH_POS_EDGE;
    SensorCustFunc.VifSetting->paralAttr.ubHsyncPolarity 			= MMPF_VIF_SNR_CLK_POLARITY_POS;
    SensorCustFunc.VifSetting->paralAttr.ubVsyncPolarity 			= MMPF_VIF_SNR_CLK_POLARITY_NEG;
    SensorCustFunc.VifSetting->paralAttr.ubBusBitMode               = MMPF_VIF_SNR_PARAL_BITMODE_16;

    // Init Vif Setting : MIPI Sensor Setting
    SensorCustFunc.VifSetting->mipiAttr.bClkDelayEn 				= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bClkLaneSwapEn 				= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.usClkDelay 					= 0;
    SensorCustFunc.VifSetting->mipiAttr.ubBClkLatchTiming 			= MMPF_VIF_SNR_LATCH_NEG_EDGE;
#if (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[0] 				= MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[1] 				= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[2] 				= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[3] 				= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[0] 			= MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[1] 			= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[2] 			= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[3] 			= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[0] 			= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[1] 			= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[2] 			= MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[3] 			= MMP_FALSE;
#if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_1;//use for 2531project	
#else
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_0;//use for HSG project
#endif
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[2] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[3] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[0] 				= 0x0E;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[1] 				= 0x08;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[2] 				= 0x08;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[3] 				= 0x08;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[0] 			= 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[1] 			= 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[2] 			= 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[3] 			= 0x1F;
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[0]              = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[1]              = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[2]              = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[3]              = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[0]             = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[1]             = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[2]             = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[3]             = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[0]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[1]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[2]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[3]          = MMP_FALSE;
#if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)    
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0]            = MMPF_VIF_MIPI_DATA_SRC_PHY_1;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1]            = MMPF_VIF_MIPI_DATA_SRC_PHY_2;
#else
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0]            = MMPF_VIF_MIPI_DATA_SRC_PHY_0;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1]            = MMPF_VIF_MIPI_DATA_SRC_PHY_1;
#endif    
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[2]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[3]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[0]              = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[1]              = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[2]              = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[3]              = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[0]             = 0x0F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[1]             = 0x0F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[2]             = 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[3]             = 0x1F;
#endif

    // Init Vif Setting : Color ID Setting
    SensorCustFunc.VifSetting->colorId.VifColorId 					= MMPF_VIF_COLORID_00;
    SensorCustFunc.VifSetting->colorId.CustomColorId.bUseCustomId 	= MMP_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------

#if (ISP_EN)
static ISP_UINT32 s_gain;
#endif
#define ISP_DGAIN_BASE		(0x200)
ISP_UINT32	Dgain = ISP_DGAIN_BASE;


void SNR_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	#if (ISP_EN)
	MMP_UBYTE ubPeriod 				= (SensorCustFunc.pAeTime)->ubPeriod;
	MMP_UBYTE ubFrmStSetShutFrmCnt 	= (SensorCustFunc.pAeTime)->ubFrmStSetShutFrmCnt;
	MMP_UBYTE ubFrmStSetGainFrmCnt 	= (SensorCustFunc.pAeTime)->ubFrmStSetGainFrmCnt;

//Daniel - 20160622
	
	//stop AE
	if((*(MMP_UBYTE*) 0x800070C8 ) == 0) return;

	//WDR Enable
	if ((ulFrameCnt % 1000) == 10)	ISP_IF_F_SetWDREn(1);
	
	//set shutter
	if (ulFrameCnt % ubPeriod == ubFrmStSetShutFrmCnt)
	{
		ISP_IF_AE_Execute();		
		gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, 0, 0);			
	}	

	//set gain
	if (ulFrameCnt % ubPeriod == ubFrmStSetGainFrmCnt)
	{
		s_gain = ISP_MAX(ISP_IF_AE_GetGain(), ISP_IF_AE_GetGainBase());
		if (s_gain >= (ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN))
		{
			s_gain  = ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN;
		}			
	    gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, s_gain);
	}
	#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
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
void SNR_Cust_DoIQ(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{

}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetGain
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetGain(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain)
{
	//Daniel
    ISP_UINT16 s_gain, gainbase;
    ISP_UINT32 H, L;
	ISP_UINT32 set_gain = 16;
	ISP_UINT32 A_Gain, P_Gain;

    gainbase = ISP_IF_AE_GetGainBase();

	if (ulGain >= (ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN))
	{
		Dgain 	= ISP_DGAIN_BASE * ulGain / (ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN);
		ulGain  = ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN;
	}
	else
	{
		Dgain	= ISP_DGAIN_BASE;
	}

    s_gain = ISP_MIN(ISP_MAX(ulGain, gainbase), gainbase * MAX_SENSOR_GAIN - 1); // API input gain range : 64~511, 64=1X

	//set gain
	if (s_gain >= gainbase * 8)
	{
        A_Gain 	= 6;
        P_Gain	= s_gain / 8;
    } 		
	else if (s_gain >= gainbase * 4)
	{
        A_Gain 	= 4;
        P_Gain	= s_gain / 4;
    } 
    else if (s_gain >= gainbase * 2)
    {
        A_Gain 	= 2;
        P_Gain	= s_gain / 2;             
    }     
    else
    {
        A_Gain 	= 1;
        P_Gain	= s_gain / 1;                    
    }
    
    P_Gain	= P_Gain * 0x40 / gainbase;
    	
	// Total sensor gain = (A_gain * G_gain * P_gain), G-gain = 0x40 (Defalut) reg:0xB0
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0xB6, A_Gain); 				// A_gain
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0xB1, ((P_Gain>>6)&0x07)); 	// P_gain integer
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0xB2, ((P_Gain<<2)&0xFC));	// P_gain decimal

}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
#if (ISP_EN)
    ISP_UINT32 new_vsync;
    ISP_UINT32 new_shutter;

    #if 1
		if(shutter == 0 | vsync == 0)
		{
			new_vsync 	= gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetVsync() / ISP_IF_AE_GetVsyncBase();
	   		new_shutter = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetShutter() / ISP_IF_AE_GetShutterBase();
		}
		else
		{
			new_vsync 	= gSnrLineCntPerSec[ubSnrSel] * vsync / ISP_IF_AE_GetVsyncBase();
			new_shutter = gSnrLineCntPerSec[ubSnrSel] * shutter / ISP_IF_AE_GetShutterBase();
		}
	#endif
	
    //new_vsync   = VR_MAX(new_vsync, new_shutter + 5);
    new_shutter = ISP_MIN(ISP_MAX(new_shutter, 1), new_vsync - 5);

    if	(new_shutter < 1)		new_shutter = 1;
    if	(new_shutter > 8191)	new_shutter = 8191;//2^13

//	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x08, (new_vsync) & 0xFF); //blanking
//	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x07, (new_vsync >> 8) & 0x1F); 
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x04, (new_shutter) & 0xFF); 
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x03, (new_shutter >> 8) & 0x1F); 

#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetExposure
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_SetExposure(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain, MMP_ULONG shutter, MMP_ULONG vsync)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetFlip
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetFlip(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{

}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetRotate
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetRotate(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_CheckVersion
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_CheckVersion(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion)
{
	// TBD
	MMP_USHORT usRdValF0, usRdValF1 = 0;
    
    /* Initial I2cm */
    MMPF_I2cm_Initialize(&m_I2cmAttr);
    MMPF_OS_SleepMs(10);

	gsSensorFunction->MMPF_Sensor_GetReg(ubSnrSel, 0xf0, &usRdValF0);
	gsSensorFunction->MMPF_Sensor_GetReg(ubSnrSel, 0xf1, &usRdValF1);

	printc(FG_YELLOW("Get GC2023 sensor ID %#x %#x\n"), usRdValF0, usRdValF1);
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_StreamEnable
//  Description : Enable/Disable streaming of sensor
//------------------------------------------------------------------------------
static void SNR_Cust_StreamEnable(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    // TBD
}

MMPF_SENSOR_CUSTOMER SensorCustFunc = 
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
	NULL,
    SNR_Cust_StreamEnable,

	&m_SensorRes,
	&m_OprTable,
	&m_VifSetting,
	&m_I2cmAttr,
	&m_AwbTime,
	&m_AeTime,
	&m_AfTime,
    MMP_SNR_PRIO_PRM
};

int SNR_Module_Init(void)
{
    MMPF_SensorDrv_Register(PRM_SENSOR, &SensorCustFunc);
       return 0;
    if (SensorCustFunc.sPriority == MMP_SNR_PRIO_PRM)
        MMPF_SensorDrv_Register(PRM_SENSOR, &SensorCustFunc);
    else
        MMPF_SensorDrv_Register(SCD_SENSOR, &SensorCustFunc);

    return 0;
}

#pragma arm section code = "initcall6", rodata = "initcall6", rwdata = "initcall6",  zidata = "initcall6" 
#pragma O0
ait_module_init(SNR_Module_Init);
#pragma
#pragma arm section rodata, rwdata, zidata

#endif // (BIND_SENSOR_GC2023)
#endif // (SENSOR_EN)
