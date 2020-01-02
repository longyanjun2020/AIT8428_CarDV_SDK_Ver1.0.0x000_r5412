//==============================================================================
//
//  File        : sensor_SC2143.c
//  Description : Firmware Sensor Control File
//  Author      : Philip Lin
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#include "customer_config.h"

#if (SENSOR_EN)
#if (BIND_SENSOR_SC2143)

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmpf_sensor.h"
#include "mmpf_i2cm.h"
#include "Sensor_Mod_Remapping.h"
#include "isp_if.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define MAX_SENSOR_GAIN     (64)          // max sensor gain

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================

// Resolution Table
MMPF_SENSOR_RESOLUTION m_SensorRes = 
{
	2,				/* ubSensorModeNum */
	0,				/* ubDefPreviewMode */
	0,				/* ubDefCaptureMode */
	3000,			// usPixelSize
//  Mode0   Mode1
    {1,     1},	/* usVifGrabStX */
    {1,     1},	/* usVifGrabStY */
    {1928,  1928},	/* usVifGrabW */
    {1088,  1088},	/* usVifGrabH */
    #if (CHIP == MCR_V2)
    {1,     1}, 	// usBayerInGrabX
    {1,     1},    // usBayerInGrabY
    {8,     8},  /* usBayerDummyInX */
    {8,     8},  /* usBayerDummyInY */
    {1920,  1920},	/* usBayerOutW */
    {1080,  1080},	/* usBayerOutH */
    #endif
    {1920,  1920},	/* usScalInputW */
    {1080,  1080},	/* usScalInputH */
    {300,   600},	/* usTargetFpsx10 */
    {1130,  1134},	/* usVsyncLine */  //Reg 0x22 0x23
    {1,     1},  /* ubWBinningN */
    {1,     1},  /* ubWBinningN */
    {1,     1},  /* ubWBinningN */
    {1,     1},  /* ubWBinningN */
    {0,     0},    	// ubCustIQmode
    {0,     0}     	// ubCustAEmode  
};

// OPR Table and Vif Setting
MMPF_SENSOR_OPR_TABLE       m_OprTable;
MMPF_SENSOR_VIF_SETTING     m_VifSetting;

//lv1-7, 2	3	6	11	17	30	60
#if 1//	new extent node for18//LV1,		LV2,		LV3,		LV4,		LV5,		LV6,		LV7,		LV8,		LV9,		LV10,	LV11,	LV12,	LV13,	LV14,	LV15,	LV16 	LV17  	LV18
//abby curve iq 12
ISP_UINT32 AE_Bias_tbl[54] =
/*lux*/			//			{2,			3,			6,			10,			17, 		26, 		54, 		101, 		206,		407,	826,	1638,	3277,	6675,	13554,	27329,	54961, 111285/*930000=LV17*/  //with  1202
/*lux*/						{1,			2,			3,			6,			10,			17, 		26, 		54, 		101, 		206,	407,	826,	1638,	3277,	6675,	13554,	27329,	54961/*930000=LV17*/ //with  1203
/*ENG*/						,0x2FFFFFF, 4841472*2,	3058720,	1962240,	1095560,  	616000, 	334880, 	181720,     96600,	 	52685,	27499,	14560,	8060,	4176,	2216,	1144,	600,   300
/*Tar*/						,55,		60,		 	65,	        90,			110, 		122,	 	180,	 	200,	    205,	    210,	215,	220,	225,	230,	235,	240,	250,   250 //with max 15x 1202V1
/*Tar*/			//			,42,		48,		 	55,	        60,			75, 		90,	 		130,	 	180,	    220,	    240,	242,	244,	246,	247,	248,	248,	249,   250 //with max 13x 1202V2
/*Tar*/			//			,30,		40,			45,		 	52,	        60,			75, 		90,	 		150,	 	200,	    220,	240,	242,	244,	246,	247,	248,	248,	249 //with max 11x 1203V1
///*Tar*/						,30,		40,			45,		 	52,	        65,			80, 	   100,	 		150,	 	200,	    220,	240,	255,	265,	268,	270,	274,	277,	280 //with max 11x 1207

 };	
#define AE_tbl_size  (18)	//32  35  44  50
#endif
// IQ Table
const ISP_UINT8 Sensor_IQ_CompressedText[] = 
{
#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC)         
#include "isp_8428_iq_data_v3_SC2143_ezmode_20170412.xls.ciq.txt"
#else //Use old IQ table
#if(CHIP == MCR_V2)
	#include "isp_8428_iq_data_v3_SC2143_ezmode_20170412.xls.ciq.txt"	//Adjust Lux reference
#endif
#endif
};

#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC) 
//Replace it by custom IQ table.
const  __align(4) ISP_UINT8 Sensor_EZ_IQ_CompressedText[] = 
{
//#include "ez_isp_8428_ov4689.txt"
//#include "eziq_0413.txt"
//#include "eziq_0509.txt"
NULL
};

ISP_UINT32 eziqsize = sizeof(Sensor_EZ_IQ_CompressedText);
#endif

// I2cm Attribute
static MMP_I2CM_ATTR m_I2cmAttr = {
    MMP_I2CM0,  // i2cmID
    0x30,       // ubSlaveAddr
    16,         // ubRegLen
    8,          // ubDataLen
    0,          // ubDelayTime
    MMP_FALSE,  // bDelayWaitEn
    MMP_TRUE,   // bInputFilterEn
    MMP_FALSE,  // b10BitModeEn
    MMP_FALSE,  // bClkStretchEn
    0,          // ubSlaveAddr1
    0,          // ubDelayCycle
    0,          // ubPadNum
    150000, //150KHZ,,,,,,400000,     // ulI2cmSpeed 400KHZ
    MMP_TRUE,   // bOsProtectEn
    NULL,       // sw_clk_pin
    NULL,       // sw_data_pin
    MMP_FALSE,  // bRfclModeEn
    MMP_FALSE,  // bWfclModeEn
    MMP_FALSE,  // bRepeatModeEn
    0           // ubVifPioMdlId
};

// 3A Timing
MMPF_SENSOR_AWBTIMIMG   m_AwbTime    = 
{
	6,	/* ubPeriod */
	1, 	/* ubDoAWBFrmCnt */
	2	/* ubDoCaliFrmCnt */
};

MMPF_SENSOR_AETIMIMG m_AeTime = 
{
	4, 	// ubPeriod
	1, 	// ubFrmStSetShutFrmCnt
	2 	// ubFrmStSetGainFrmCnt
};

MMPF_SENSOR_AFTIMIMG    m_AfTime     = 
{
	1, 	/* ubPeriod */
	0	/* ubDoAFFrmCnt */
};

// IQ Data
#define ISP_IQ_DATA_NAME "isp_8428_iq_data_v3_SC2143_ezmode_20170412.xls.ciq.txt"

static const MMP_UBYTE s_IqCompressData[] =
{
	#include ISP_IQ_DATA_NAME
};

#if (ISP_EN)
static ISP_UINT32 s_gain;
#endif

//==============================================================================
//
//                              EXTERN VARIABLE
//
//==============================================================================

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void ____Sensor_OPR_Table____(){ruturn;} //dummy
#endif

MMP_USHORT SNR_SC2143_Reg_Init_Customer[] = 
{
    SENSOR_DELAY_REG, 10
};

// 1080p 30FPS
MMP_USHORT SNR_SC2143_Reg_1920x1080_30_Customer[] = 
{
    0x0100,0x00,
0x3c00,0x45, //FIFO RESET

0x3907,0x01, //RNC BLC
0x3908,0xc0,
0x3416,0x10,

0x3200,0x00,
0x3201,0x00,
0x3204,0x07,
0x3205,0x9f,
0x322c,0x07,
0x322d,0xa8,
0x322e,0x07,
0x322f,0xff,
0x3400,0x53,
0x3401,0x1e,
0x3402,0x04,
0x3403,0x30,

0x3637,0x87,//RAMP

0x3623,0x02,//analog
0x3620,0xc4,
0x3621,0x18,

0x3635,0x03,

//Timing

0x3300,0x10,//EQ
0x3306,0xc0,
0x330a,0x01,
0x330b,0xa0,

0x3333,0x00, 
0x3334,0x20,

0x3039,0x00, //74.25M pclk
0x303a,0x35,
0x303b,0x0c,
0x3035,0xca,
0x320c,0x08, //0x898 for 30fps
0x320d,0x98,
0x3211,0x10,
0x3213,0x10,

//0x301c,0xa4,//close mipi
//0x3018,0xff, 

0x3d08,0x00, //pclk inv

0x337f,0x03, //new auto precharge  330e in 3372
0x3368,0x04,
0x3369,0x00,
0x336a,0x00,
0x336b,0x00,
0x3367,0x08,
0x330e,0x40,

//0x3630/0x3635/0x3620 auto ctrl
0x3670,0x0b, //bit[3] for 3635 in 3687, bit[1] for 3630 in 3686,bit[0] for 3620 in 3685
0x3674,0xa0, //3630 value <gain0
0x3675,0x90, //3630 value between gain0 and gain1
0x3676,0x40, //3630 value > gain1
0x367c,0x07, //gain0
0x367d,0x0f, //gain1

0x3677,0x0a, //3635 value <gain0
0x3678,0x07, //3635 value between gain0 and gain1
0x3679,0x07, //3635 value > gain1
0x367e,0x07, //gain0
0x367f,0x1f, //gain1

0x3671,0xc2, //3620 value <gain0  11.23
0x3672,0xc2, //3620 value between gain0 and gain1  11.23
0x3673,0x63, //3620 value > gain1  11.23
0x367a,0x07, //gain0
0x367b,0x1f, //gain1



0x3e03,0x03, //AE
0x3e01,0x46,
0x3e08,0x00,


0x3401,0x1e,
0x3402,0x00, 
0x3403,0x48, //increase rnc col num to 72+16=88


0x5781,0x08, //dpc
0x5782,0x08,
0x5785,0x20,
0x57a2,0x01,
0x57a3,0xf1,


//fullwell
0x3637,0x86,
0x3635,0x03,
0x3622,0x0e,
0x3630,0x00, 
0x3630,0x00, 
0x3631,0x80,
0x3633,0x54,

//mipi
0x3c00,0x41, //FIFO RESET for mipi
0x3001,0xfe,
0x303f,0x01, //[7] 0:sel pll_pclk

0x3018,0x33, //lanenum=[7:5]+1
0x3031,0x0a, // 10bit

0x3039,0x00,
0x303a,0x31,
0x303b,0x06,
0x303c,0x08,

0x3306,0xf0,
0x330b,0xd0,

0x320c,0x09, //0x960 for 30fps
0x320d,0x60,

0x3650,0x46,
0x3651,0x0c,

0x3e09,0x10,

//fullwell adjust 0907
0x3637,0x87,
0x3674,0xd0,
0x3677,0x07,
0x3633,0x74,


0x3333,0x80, //col fpn
0x3334,0xa0,
0x3300,0x20, //shading
0x3632,0x40, //gain >8 0x42

//0908 update rnc num
0x3403,0x58,
0x3416,0x11,

0x3302,0x28, //rst go low point to cancel left column fpn from rst
0x3309,0x20, //ramp gap to cancel right column fpn from tx
0x331f,0x17,
0x3321,0x1a,

0x3677,0x0b, //high txvdd when gain<2
0x3678,0x08, //3635 value between gain0 and gain1
0x3679,0x06, //3635 value > gain1

0x3306,0xe0,


//ECO
0x322e,0x08, //rnc
0x322f,0x37,
0x3403,0x78,

0x3679,0x08,

//1118
0x3679,0x06,
0x3620,0xc4, //0x64

//1122
0x3637,0x84,
0x3638,0x84,

//170304
0x3039,0x50,
0x303a,0x53,
0x303b,0x06,
0x303c,0x08,
0x3035,0xba,

//0314
0x330e,0x20,
0x3308,0x20, //Lag
0x3632,0x70,
0x3676,0x38,

//20170314B
0x3676,0x0f,

//phy add
0x3200,0x00,
0x3201,0x00,
0x3202,0x00,
0x3203,0x00,
0x3204,0x07,
0x3205,0x9f, // xend = 1951
0x3206,0x04,
0x3207,0x57, // yend = 1111
0x3208,0x07,
0x3209,0x88, // 1928
0x320a,0x04,
0x320b,0x40, // 1088

//0418
0x3035,0xd2, // counterclk = 162M
0x3637,0x83, // ramp
0x3621,0x08,
0x330b,0xf8,
0x3306,0xd0,

0x3223,0x50, // first frame
0x3364,0x05,

0x0100,0x01,
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
	MMP_USHORT 	i;

#if (SENSOR_IF == SENSOR_IF_PARALLEL)
	RTNA_DBG_Str0(0, "SNR_Cust_InitConfig SC2143 Parallel\r\n");
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
	RTNA_DBG_Str(0, "SNR_Cust_InitConfig SC2143 MIPI 2-lane\r\n");
#endif
	
    // Init OPR Table
    SensorCustFunc.OprTable->usInitSize                   = (sizeof(SNR_SC2143_Reg_Init_Customer)/sizeof(SNR_SC2143_Reg_Init_Customer[0]))/2;
    SensorCustFunc.OprTable->uspInitTable                 = &SNR_SC2143_Reg_Init_Customer[0];    
    
    SensorCustFunc.OprTable->usSize[RES_IDX_1920x1080]    = (sizeof(SNR_SC2143_Reg_1920x1080_30_Customer)/sizeof(SNR_SC2143_Reg_1920x1080_30_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_1920x1080]  = &SNR_SC2143_Reg_1920x1080_30_Customer[0];

	// Init Vif Setting : Common
    //SensorCustFunc.VifSetting->SnrType                                = MMPF_VIF_SNR_TYPE_BAYER;
    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
    SensorCustFunc.VifSetting->OutInterface                           = MMPF_VIF_IF_PARALLEL;
    #elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
	SensorCustFunc.VifSetting->OutInterface 					      = MMPF_VIF_IF_MIPI_DUAL_01;
    #else
	SensorCustFunc.VifSetting->OutInterface 						  = MMPF_VIF_IF_MIPI_SINGLE_0;
    #endif

    #if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
    SensorCustFunc.VifSetting->VifPadId							      = MMPF_VIF_MDL_ID1;
    #else
    SensorCustFunc.VifSetting->VifPadId							      = MMPF_VIF_MDL_ID0;
    #endif
    
    // Init Vif Setting : PowerOn Setting
    SensorCustFunc.VifSetting->powerOnSetting.bTurnOnExtPower         = MMP_TRUE;
    #if (CHIP == MCR_V2)
    SensorCustFunc.VifSetting->powerOnSetting.usExtPowerPin           = MMP_GPIO_MAX;
    #endif   
    SensorCustFunc.VifSetting->powerOnSetting.bFirstEnPinHigh         = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.ubFirstEnPinDelay       = 10;
    SensorCustFunc.VifSetting->powerOnSetting.bNextEnPinHigh          = MMP_FALSE;
    SensorCustFunc.VifSetting->powerOnSetting.ubNextEnPinDelay        = 10;
    SensorCustFunc.VifSetting->powerOnSetting.bTurnOnClockBeforeRst   = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.bFirstRstPinHigh        = MMP_FALSE;
    SensorCustFunc.VifSetting->powerOnSetting.ubFirstRstPinDelay      = 10;
    SensorCustFunc.VifSetting->powerOnSetting.bNextRstPinHigh         = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.ubNextRstPinDelay       = 20;
    
    // Init Vif Setting : PowerOff Setting
    SensorCustFunc.VifSetting->powerOffSetting.bEnterStandByMode      = MMP_FALSE;
    SensorCustFunc.VifSetting->powerOffSetting.usStandByModeReg       = 0x100;
    SensorCustFunc.VifSetting->powerOffSetting.usStandByModeMask      = 0x0;    
    SensorCustFunc.VifSetting->powerOffSetting.bEnPinHigh             = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOffSetting.ubEnPinDelay           = 20;
    SensorCustFunc.VifSetting->powerOffSetting.bTurnOffMClock         = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOffSetting.bTurnOffExtPower       = MMP_FALSE;    
    SensorCustFunc.VifSetting->powerOffSetting.usExtPowerPin          = MMP_GPIO_MAX;
    
    // Init Vif Setting : Sensor paralAttr Setting
    SensorCustFunc.VifSetting->clockAttr.bClkOutEn                    = MMP_TRUE; 
    SensorCustFunc.VifSetting->clockAttr.ubClkFreqDiv                 = 0;
    SensorCustFunc.VifSetting->clockAttr.ulMClkFreq                   = 24000;
    SensorCustFunc.VifSetting->clockAttr.ulDesiredFreq                = 24000;
    SensorCustFunc.VifSetting->clockAttr.ubClkPhase                   = MMPF_VIF_SNR_PHASE_DELAY_NONE;
    SensorCustFunc.VifSetting->clockAttr.ubClkPolarity                = MMPF_VIF_SNR_CLK_POLARITY_POS;
    SensorCustFunc.VifSetting->clockAttr.ubClkSrc					  = MMPF_VIF_SNR_CLK_SRC_PMCLK;
        
    // Init Vif Setting : Parallel Sensor Setting
    SensorCustFunc.VifSetting->paralAttr.ubLatchTiming                = MMPF_VIF_SNR_LATCH_POS_EDGE;
    SensorCustFunc.VifSetting->paralAttr.ubHsyncPolarity              = MMPF_VIF_SNR_CLK_POLARITY_POS;
    SensorCustFunc.VifSetting->paralAttr.ubVsyncPolarity              = MMPF_VIF_SNR_CLK_POLARITY_NEG;

	// Init Vif Setting : MIPI Sensor Setting
    SensorCustFunc.VifSetting->mipiAttr.bClkDelayEn                   = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bClkLaneSwapEn                = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.usClkDelay                    = 0;
    SensorCustFunc.VifSetting->mipiAttr.ubBClkLatchTiming             = MMPF_VIF_SNR_LATCH_NEG_EDGE;
	#if (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[0]                = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[1]                = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[2]                = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[3]                = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[0]               = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[1]               = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[2]               = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[3]               = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[0]            = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[1]            = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[2]            = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[3]            = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0]              = (SensorCustFunc.VifSetting->VifPadId == MMPF_VIF_MDL_ID0)? MMPF_VIF_MIPI_DATA_SRC_PHY_0 : MMPF_VIF_MIPI_DATA_SRC_PHY_1;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[2]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[3]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[0]                = 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[1]                = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[2]                = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[3]                = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[0]               = 0x0F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[1]               = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[2]               = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[3]               = 0x08;
    #elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[0]                = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[1]                = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[2]                = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[3]                = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[0]               = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[1]               = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[2]               = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[3]               = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[0]            = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[1]            = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[2]            = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[3]            = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0]              = (SensorCustFunc.VifSetting->VifPadId == MMPF_VIF_MDL_ID0)? MMPF_VIF_MIPI_DATA_SRC_PHY_0 : MMPF_VIF_MIPI_DATA_SRC_PHY_1;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1]              = (SensorCustFunc.VifSetting->VifPadId == MMPF_VIF_MDL_ID0)? MMPF_VIF_MIPI_DATA_SRC_PHY_1 : MMPF_VIF_MIPI_DATA_SRC_PHY_2;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[2]              = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[3]              = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[0]                = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[1]                = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[2]                = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[3]                = 0x08;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[0]               = 0x18;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[1]               = 0x18;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[2]               = 0x18;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[3]               = 0x18;
	#endif
	
    // Init Vif Setting : Color ID Setting
    SensorCustFunc.VifSetting->colorId.VifColorId              		  = MMPF_VIF_COLORID_11; 
    
	SensorCustFunc.VifSetting->colorId.CustomColorId.bUseCustomId  	  = MMP_TRUE; //MMP_FALSE;

    for (i = 0; i < MAX_SENSOR_RES_MODE; i++)
    {
        SensorCustFunc.VifSetting->colorId.CustomColorId.Rot0d_Id[i]   = MMPF_VIF_COLORID_11;
        SensorCustFunc.VifSetting->colorId.CustomColorId.Rot90d_Id[i]  = MMPF_VIF_COLORID_UNDEF;
        SensorCustFunc.VifSetting->colorId.CustomColorId.Rot180d_Id[i] = MMPF_VIF_COLORID_00;
        SensorCustFunc.VifSetting->colorId.CustomColorId.Rot270d_Id[i] = MMPF_VIF_COLORID_UNDEF;
        SensorCustFunc.VifSetting->colorId.CustomColorId.H_Flip_Id[i]  = MMPF_VIF_COLORID_10;
        SensorCustFunc.VifSetting->colorId.CustomColorId.V_Flip_Id[i]  = MMPF_VIF_COLORID_10;
        SensorCustFunc.VifSetting->colorId.CustomColorId.HV_Flip_Id[i] = MMPF_VIF_COLORID_00;
    }
}

//------------------------------------------------------------------------------
//  Function    : SNR_SC2143_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
ISP_UINT32 isp_gain;

void SNR_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
	MMP_ULONG s_gain = 0;
	MMP_ULONG ulVsync = 0;
	MMP_ULONG ulShutter = 0;
	MMP_UBYTE ubPeriod 				= (SensorCustFunc.pAeTime)->ubPeriod;
	MMP_UBYTE ubFrmStSetShutFrmCnt 	= (SensorCustFunc.pAeTime)->ubFrmStSetShutFrmCnt;
	MMP_UBYTE ubFrmStSetGainFrmCnt 	= (SensorCustFunc.pAeTime)->ubFrmStSetGainFrmCnt;
	
		
    // For AE curve
    if ((ulFrameCnt % 100) == 10)
    {
        ISP_IF_F_SetWDREn(1);
        ISP_IF_CMD_SendCommandtoAE(0x51, AE_Bias_tbl,AE_tbl_size, 0); // <<AE table set once at preview start 
        ISP_IF_NaturalAE_Enable(2); // 0: no, 1: ENERGY, 2: Lux, 3: test mode
        //ISP_IF_CMD_SendCommandtoAE(0x52, 0,0, 1); // <<AE table set once at preview start 
    }

	if (ulFrameCnt % ubPeriod == ubFrmStSetShutFrmCnt)
	{
		ISP_IF_AE_Execute();
		s_gain = VR_MAX(ISP_IF_AE_GetGain(), ISP_IF_AE_GetGainBase());

		if (s_gain >= (ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN))
		{
			s_gain  = ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN;
		}

		ulVsync 	= (gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetVsync()) / ISP_IF_AE_GetVsyncBase();
		ulShutter 	= (gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetShutter()) / ISP_IF_AE_GetShutterBase();
		
		gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, ulShutter, ulVsync);
		gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, s_gain);		  		
	}
	else if ((ulFrameCnt % ubPeriod) == ubFrmStSetGainFrmCnt)
	{
		//gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, s_gain);
		ISP_IF_IQ_SetAEGain(isp_gain, ISP_IF_AE_GetGainBase());
	}
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{

}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAWB_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{

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
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetGain
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetGain(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain)
{

    ISP_UINT16 sensor_gain;
    sensor_gain = ISP_MAX(ulGain,ISP_IF_AE_GetGainBase()) * 0x10 / ISP_IF_AE_GetGainBase();
    
    if (sensor_gain > 0xF8)
    {
       sensor_gain = 0xF8;
	   isp_gain = ISP_IF_AE_GetGainBase() * ulGain / (sensor_gain * ISP_IF_AE_GetGainBase() / 0x10); //use Dgain to compensate rounding error
    }
   // gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3e08, sensor_gain>>8);
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3e09, sensor_gain); 


	RTNA_DBG_Str0("ulGain\r\n");
	RTNA_DBG_Dec0(ulGain);
	RTNA_DBG_Str0("\r\nsensor_gain\r\n");
	RTNA_DBG_Dec0(sensor_gain);
	RTNA_DBG_Str0("\r\nisp_gain\r\n");
	RTNA_DBG_Dec0(isp_gain);
	
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
	ISP_UINT32 new_vsync    = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetVsync() / ISP_IF_AE_GetVsyncBase();
	ISP_UINT32 new_shutter  = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetShutter() / ISP_IF_AE_GetShutterBase();

	new_vsync   = ISP_MIN(ISP_MAX(new_shutter , new_vsync), 0x0465);
	new_shutter = ISP_MIN(ISP_MAX(new_shutter, 1), new_vsync - 4);

	//gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x320e,(ISP_UINT8)((new_vsync >> 8) & 0xFF));
	//gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x320f,(ISP_UINT8)((new_vsync << 0) & 0xFF));
	
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3e01,(ISP_UINT8)((new_shutter >> 4) & 0xFF));
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3e02,(ISP_UINT8)((new_shutter << 4) & 0xF0));
	
	RTNA_DBG_Str0("\r\nnew_shutter\r\n");
	RTNA_DBG_Dec0(new_shutter);
	RTNA_DBG_Str0("\r\n");
	
}
//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetExposure
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_SetExposure(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain, MMP_ULONG ulShutter, MMP_ULONG ulVsync)
{
    //TBD
    printc(FG_RED("Warning!!! Please review SNR_Cust_SetExposure in sensor driver!!!\r\n"));
    gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, ulShutter, ulVsync);
    gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, ulGain);
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

}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_CheckVersion
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_CheckVersion(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion)
{
    // TBD
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
    // TBD
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
}

#pragma arm section code = "initcall6", rodata = "initcall6", rwdata = "initcall6", zidata = "initcall6"
#pragma O0
ait_module_init(SNR_Module_Init);
#pragma
#pragma arm section rodata, rwdata, zidata

#endif  //BIND_SENSOR_SC2143
#endif	//SENSOR_EN
