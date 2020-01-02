//==============================================================================
//
//  File        : sensor_imx291.c
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
#if (BIND_SENSOR_IMX291)

#include "mmpf_pll.h"
#include "mmpf_sensor.h"
#include "Sensor_Mod_Remapping.h"
#include "isp_if.h"

#define IMX291_MCLK_74250KHZ (0)

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================

// Resolution Table
MMPF_SENSOR_RESOLUTION m_SensorRes = 
{
	13, 																				                	// ubSensorModeNum
	2, 																				                		// ubDefPreviewMode
	0, 																					                	// ubDefCaptureMode
	3000,                                                                                                   // usPixelSize (TBD)
// Mode0 	Mode1 	Mode2 	Mode3 	Mode4 	Mode5 	Mode6 	Mode7 	Mode8 	Mode9 	Mode10  Mode11  Mode12
	{1+372, 1+452,      1,  1+140,  1+140,  1+140,  1+452,  1+189,  1+189,  1+189,  1+772,  1+140,	1+452}, // usVifGrabStX
	{1+12,   1+66,   1+12,   1+22,   1+22,   1+22,  1+186,   1+19,   1+19,   1+19,  1+306,   1+22,  1+186}, // usVifGrabStY
	{1448,   1288,   1928,   1928,   1928,   1928,   1288,   1288,   1288,   1288,    648,   1928,   1288}, // usVifGrabW
	{1084,    976,   1084,   1096,   1096,   1096,    724,    724,    724,    724,    496,   1096,    724}, // usVifGrabH
#if (CHIP == MCR_V2)
	{1,	        1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1}, // usBayerInGrabX
	{1,	        1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1}, // usBayerInGrabY
	{8,         8,      8,      8,      8,      8,      8,      8,      8,      8,      8,      8,      8}, // usBayerInDummyX
	{4,        16,      4,     16,     16,     16,      4,      4,      4,      4,     16,     16,      4}, // usBayerInDummyY
	{1440,   1280,   1920,   1920,   1920,   1920,   1280,   1280,   1280,   1280,    640,   1920,   1280}, // usBayerOutW
	{1080,    960,   1080,   1080,   1080,   1080,    720,    720,    720,    720,    480,   1080,    720}, // usBayerOutH
#endif
	{1440,   1280,   1920,   1920,   1920,   1920,   1280,   1280,   1280,   1280,    640,   1920,   1280}, // usScalInputW
	{1080,    960,   1080,   1080,   1080,   1080,    720,    720,    720,    720,    480,   1080,    720}, // usScalInputH
	{300,     300,    300,    500,    600,    150,    300,    600,   1000,   1200,    300,    240,    240}, // usTargetFpsx10
	{1125,   1141,   1126,   1141,   1141,   1141,   1141,   1141,   1141,   1141,   1141,   1141,   1141}, // usVsyncLine
	{1,         1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1}, // ubWBinningN
	{1,	        1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1}, // ubWBinningN
	{1,	        1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1}, // ubWBinningN
	{1,	        1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1}, // ubWBinningN
	{0xFF,	 0xFF,	 0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF}, // ubCustIQmode
	{0xFF,   0xFF,	 0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF}  // ubCustAEmode
};

// OPR Table and Vif Setting
MMPF_SENSOR_OPR_TABLE 	m_OprTable;
MMPF_SENSOR_VIF_SETTING m_VifSetting;

// IQ Table
const ISP_UINT8 Sensor_IQ_CompressedText[] = 
{
#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC) //TBD          
//#include "isp_8428_iq_data_v2_OV4689_ezmode_v1.xls.ciq.txt"
#include "isp_8428_iq_data_v3_IMX291_ezmode.xls.ciq.txt"
#else //Use old IQ table
#ifdef CUS_ISP_8428_IQ_DATA     // maybe defined in project MCP or Config_SDK.h
---#include CUS_ISP_8428_IQ_DATA
#else
//#include "isp_8428_iq_data_v2_IMX322.xls.ciq.txt"//"isp_8428_iq_data_v2_IMX322_MIO_IQ14B.xls.ciq.txt"
#include "isp_8428_iq_data_v3_IMX291_ezmode.xls.ciq.txt"
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
static MMP_I2CM_ATTR m_I2cmAttr = 
{
	MMP_I2CM0,      // i2cmID
	(0x34 >> 1), 	// ubSlaveAddr
	16, 			// ubRegLen
	8, 				// ubDataLen
	0, 				// ubDelayTime
	MMP_FALSE, 		// bDelayWaitEn
	MMP_TRUE, 		// bInputFilterEn
	MMP_FALSE, 		// b10BitModeEn
	MMP_FALSE, 		// bClkStretchEn
	0, 				// ubSlaveAddr1
	0, 				// ubDelayCycle
	0, 				// ubPadNum
	150000, 		// ulI2cmSpeed 150KHZ
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

// IQ Data
//#define ISP_IQ_DATA_NAME "isp_8428_iq_data_v2_IMX291.xls.ciq.txt"

static const MMP_UBYTE s_IqCompressData[] =
{
    NULL//#include ISP_IQ_DATA_NAME
};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void ____Sensor_Init_OPR_Table____(){ruturn;} //dummy
#endif

ISP_UINT16 SNR_IMX291_Reg_Unsupport[] = 
{
    SENSOR_DELAY_REG, 10 // delay
};

ISP_UINT16 SNR_IMX291_Reg_Init_Customer[] = 
{
	// TBD
	SENSOR_DELAY_REG, 10 // delay
};

#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} //dummy
#endif

ISP_UINT16 SNR_IMX291_Reg_1440x1080_30P[] = 
{
	// TBD
	SENSOR_DELAY_REG, 10 // delay
};

ISP_UINT16 SNR_IMX291_Reg_1280x960_30P[] = 
{
	// TBD
	SENSOR_DELAY_REG, 10 // delay
};

ISP_UINT16 SNR_IMX291_Reg_1920x1080_24P[] =
{
//        1080@24fps-12bit,Parallel CMOS SDR
//        Input 37.125MHz to INCK
//
//      address data
        0x3000,  0x31,
        0x0100,  0x00,

        0x302C,  0x01,
        0x0008,  0x00,
        0x0009,  0xF0, //Change it from 0x3C to 0xF0 because of green image
#if (SENSOR_ROTATE_180)
    	0x0101,	 0x03, 	// (IMG_ORIENTATION, address 0101h[1]), Vertical (V) scanning direction control (I2C)
					    // 0: Normal, 1: Inverted
					    // (IMG_ORIENTATION, address 0101h[2]), Horizontal (H) scanning direction control (I2C)
					    // 0: Normal, 1: Inverted
#else
        0x0101,  0x00,
#endif
        0x0104,  0x00,
        0x0112,  0x0C,
        0x0113,  0x0C,
        0x0202,  0x00,
        0x0203,  0x00,
        0x0340,  0x04,
        0x0341,  0x65,
        0x0342,  0x05,
        0x0343,  0x5F,

        0x3001,  0x00,
        0x3002,  0x0F,
        0x3003,  0x5F,
        0x3004,  0x05,
        0x3005,  0x65,
        0x3006,  0x04,
        0x3007,  0x00,
        0x3011,  0x00,
        0x3012,  0x82,
        0x3016,  0x3C,
        0x301F,  0x73,
        0x3020,  0x3C,
        0x3021,  0x00,
        0x3022,  0x00,
        0x3027,  0x20,
        0x307A,  0x00,
        0x307B,  0x00,
        0x3098,  0x26,
        0x3099,  0x02,
        0x309A,  0x26,
        0x309B,  0x02,
        0x30CE,  0x16,
        0x30CF,  0x82,
        0x30D0,  0x00,

//      address data
        0x3117,  0x0D,

        // wait 100ms
        SENSOR_DELAY_REG, 10,

//      address data
        0x302C,  0x00,

//      wait 100ms
        SENSOR_DELAY_REG, 10,

//      address data
        0x3000,  0x30,

        0x0100,  0x01,
};

ISP_UINT16 SNR_IMX291_Reg_1920x1080_30P[] = 
{
#if (SENSOR_IF == SENSOR_IF_PARALLEL)
    0x3002, 0x00,
    
    //0x3005,	0x00,
    0x3005, 0x00, //ADbit 12
	0x3007, 0x00, //ADbit 12
    //0x3009,	0x12,
    0x3009, 0x02, //0225
    0x300A, 0x3C, //0225
    0x300F, 0x00,
    0x3010, 0x21,
    0x3012, 0x64,
    0x3013, 0x00,
    0x3016, 0x09,
    0x3018, 0x65,
    0x3019, 0x04,
    0x301C, 0x30,
    0x301D, 0x11,
    0x3046, 0x00, //0225	
    0x304B, 0x0A, //0225
    0x305C, 0x18,
    0x305D, 0x00, //0225
    0x305E, 0x20,
    0x305F, 0x01,
    
    0x3070, 0x02,
    0x3071, 0x11,
    0x309B, 0x10,
    0x309C, 0x22,
    0x30A2, 0x02,
    0x30A6, 0x20,
    0x30A8, 0x20,
    0x30AA, 0x20,
    0x30AC, 0x20,
    0x30B0, 0x43,
    0x3119, 0x9E,
    0x311C, 0x1E,
    0x311E, 0x08,
    0x3128, 0x05,
    
    //0x3129,	0x1D,
    0x3129, 0x1D, //ADbit 12
    0x313D, 0x83,
    0x3150, 0x03,
    0x315E, 0x1A,
    0x3164, 0x1A,
    //0x317C,	0x12,
    0x317c, 0x12, //ADbit 12
    0x317E, 0x00,
    //0x31EC,	0x37,
    0x31EC, 0x37, //ADbit 12
    
    0x32B8, 0x50,
    0x32B9, 0x10,
    0x32BA, 0x00,
    0x32BB, 0x04,
    0x32C8, 0x50,
    0x32C9, 0x10,
    0x32CA, 0x00,
    0x32CB, 0x04,
    
    
    0x332C, 0xD3,
    0x332D, 0x10,
    0x332E, 0x0D,
    0x3358, 0x06,
    0x3359, 0xE1,
    0x335A, 0x11,
    0x3360, 0x1E,
    0x3361, 0x61,
    0x3362, 0x10,
    0x33B0, 0x50,
    0x33B2, 0x1A,
    0x33B3, 0x04,
    0x3480, 0x49,
    
    SENSOR_DELAY_REG, 100, // wait 100ms
    
    0x3001, 0x00,
    0x3002, 0x00,
    
	SENSOR_DELAY_REG, 10, // wait 100ms

	0x3000, 0x00,

	SENSOR_DELAY_REG, 100, // wait 100ms

	0x3021, 0x02

#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    #if (IMX291_MCLK_74250KHZ)
    0x3002, 0x00, // 
    0x3005, 0x01, // ADBIT X00: 10-BIT, X01: 12-BIT
    0x3007, 0x00, // [O]:V DIRECTION, [1]:H DIRECTION [6:4]0:FHD, 1:HD, 4:CROOPING
    0x3009, 0x02, // [4] 0:LCG, 1:HCG
    0x300A, 0xF0, // Black level offset [7:0]
    0x300F, 0x00, // Fixed
    0x3010, 0x21, // Fixed
    0x3012, 0x64, // Fixed
    0x3016, 0x09, // Fixed
    0x3018, 0x65, // VMAX LOW
    0x3019, 0x04, // VMAX MIDDLE
    0x301C, 0x30, // HMAX LOW   // Horizontal clock count 0x301D~0x301C: 0x1130 (30 FPS), 0x11C8 (29 FPS)
    0x301D, 0x11, // HMAX HIGH
    0x3046, 0x01, // OD BIT [1:0] 0:10-BIT, 1:12-BIT / [7:4] INTERFACE SELECTION 0:PARALLEL, D E F : LVDS 2 4 8, MIPI:don't care
    0x304B, 0x0A, // Sync out enable
    0x305C, 0x0C, // INCLK1: 74.25:0Ch, 37.125: 18h
    0x305D, 0x03, // INCLK2: 74.25:03h, 37.125: 03h
    0x305E, 0x10, // INCLK3: 74.25:10h, 37.125: 20h
    0x305F, 0x01, // INCLK4: 74.25:01h, 37.125: 01h
    0x3070, 0x02, // Fixed
    0x3071, 0x11, // Fixed
    0x309B, 0x10, // Fixed
    0x309C, 0x22, // Fixed
    0x30A2, 0x02, // Fixed
    0x30A6, 0x20, // Fixed
    0x30A8, 0x20, // Fixed
    0x30AA, 0x20, // Fixed
    0x30AC, 0x20, // Fixed
    0x30B0, 0x43, // Fixed
    0x3119, 0x9E, // Fixed
    0x311C, 0x1E, // Fixed
    0x311E, 0x08, // Fixed
    0x3128, 0x05, // Fixed
    0x3129, 0x00, // ADBIT1 1D:10-BIT, 00:12-BIT
    0x313D, 0x83, // Fixed
    0x3150, 0x03, // Fixed
    0x315E, 0x1B, // INCLK5: 74.25:1Bh, 37.125: 1Ah
    0x3164, 0x1B, // INCLK6: 74.25:1Bh, 37.125: 1Ah
    0x317C, 0x00, // ADBIT2 12:10-BIT, 00:12-BIT
    0x317E, 0x00, // Fixed
    0x31EC, 0x0E, // ADBIT3 37:10-BIT, 0E:12-BIT
    0x32B8, 0x50, // Fixed
    0x32B9, 0x10, // Fixed
    0x32BA, 0x00, // Fixed
    0x32BB, 0x04, // Fixed
    0x32C8, 0x50, // Fixed
    0x32C9, 0x10, // Fixed
    0x32CA, 0x00, // Fixed
    0x32CB, 0x04, // Fixed
    0x332C, 0xD3, // Fixed
    0x332D, 0x10, // Fixed
    0x332E, 0x0D, // Fixed
    0x3358, 0x06, // Fixed
    0x3359, 0xE1, // Fixed
    0x335A, 0x11, // Fixed
    0x3360, 0x1E, // Fixed
    0x3361, 0x61, // Fixed
    0x3362, 0x10, // Fixed
    0x33B0, 0x50, // Fixed
    0x33B2, 0x1A, // Fixed
    0x33B3, 0x04, // Fixed
    0x3405, 0x10, // Fixed
    0x3407, 0x01, // Fixed
    0x3414, 0x0A, // Fixed
    0x3418, 0x49, // Fixed
    0x3419, 0x04, // Fixed
    0x3441, 0x0C, // Fixed
    0x3442, 0x0C, // Fixed
    0x3443, 0x01, // Fixed
    0x3444, 0x40, // EXT CLK 37.125:X2520, 74.25:X4A40
    0x3445, 0x4A, // 
    0x3446, 0x57, // 
    0x3447, 0x00, // 
    0x3448, 0x37, // 
    0x3449, 0x00, // 
    0x344A, 0x1F, // 
    0x344B, 0x00, // 
    0x344C, 0x1F, // 
    0x344D, 0x00, // 
    0x344E, 0x1F, // 
    0x344F, 0x00, // 
    0x3450, 0x77, // 
    0x3451, 0x00, // 
    0x3452, 0x1F, // 
    0x3453, 0x00, // 
    0x3454, 0x17, // 
    0x3455, 0x00, // 
    0x3472, 0x9C, // 
    0x3473, 0x07, // 
    0x3480, 0x92, // INCK7: 92:74.25MHz, 49:37.125MHz
    /////////////////////////////////////////////////////
	SENSOR_DELAY_REG, 0x64, /* delay */
    0x3001, 0x00 / 
    0x3002, 0x00 / 
	SENSOR_DELAY_REG, 0x0A, /* delay */
    0x3000, 0x00 / 
	SENSOR_DELAY_REG, 0x64, /* delay */
    ///////////////////////////////////////////////////// 

    #else
	0x3003, 0x01,
	SENSOR_DELAY_REG, 0x10, /* delay */
	0x3000, 0x01,
	0x3002, 0x00,
	0x3005, 0x01,
	0x3007, 0x00,
	0x3009, 0x02,
	0x300a, 0xf0,
	0x300f, 0x00,
	0x3010, 0x21,
	0x3012, 0x64,
	0x3016, 0x09,
	0x3018, 0x65,
	0x3019, 0x04,
	0x301c, 0x30,  // Horizontal clock count 0x301D~0x301C: 0x1130 (30 FPS), 0x11C8 (29 FPS)
	0x301d, 0x11,
	0x3046, 0x01,
	0x304b, 0x0a,
	0x305c, 0x18,
	0x305d, 0x03,
	0x305e, 0x20,
	0x305f, 0x01,
	0x3070, 0x02,
	0x3071, 0x11,
	0x309b, 0x10,
	0x309c, 0x22,
	0x30a2, 0x02,
	0x30a6, 0x20,
	0x30a8, 0x20,
	0x30aa, 0x20,
	0x30ac, 0x20,
	0x30b0, 0x43,
	0x3119, 0x9e,
	0x311c, 0x1e,
	0x311e, 0x08,
	0x3128, 0x05,
	0x3129, 0x00,
	0x313d, 0x83,
	0x3150, 0x03,
	0x315e, 0x1a,
	0x3164, 0x1a,
	0x317c, 0x00,
	0x317e, 0x00,
	0x31ec, 0x0e,
	0x32b8, 0x50,
	0x32b9, 0x10,
	0x32ba, 0x00,
	0x32bb, 0x04,
	0x32c8, 0x50,
	0x32c9, 0x10,
	0x32ca, 0x00,
	0x32cb, 0x04,
//	0x3304, 0x22,
	0x332c, 0xd3,
	0x332d, 0x10,
	0x332e, 0x0d,
	0x3358, 0x06,
	0x3359, 0xe1,
	0x335a, 0x11,
	0x3360, 0x1e,
	0x3361, 0x61,
	0x3362, 0x10,
	0x33b0, 0x50,
	0x33b2, 0x1a,
	0x33b3, 0x04,
	0x3405, 0x10,
	0x3407, 0x01,//03,  //1 for 2lane; 3 for 4lane
	0x3414, 0x0a,
	0x3418, 0x49,
	0x3419, 0x04,
	0x3441, 0x0c,
	0x3442, 0x0c,
	0x3443, 0x01,//03,  //1 for 2lane; 3 for 4lane
	0x3444, 0x20,
	0x3445, 0x25,
	0x3446, 0x57,
	0x3447, 0x00,
	0x3448, 0x37,
	0x3449, 0x00,
	0x344a, 0x1f,
	0x344b, 0x00,
	0x344c, 0x1f,
	0x344d, 0x00,
	0x344e, 0x1f,
	0x344f, 0x00,
	0x3450, 0x77,
	0x3451, 0x00,
	0x3452, 0x1f,
	0x3453, 0x00,
	0x3454, 0x17,
	0x3455, 0x00,
	0x3472, 0x9c,
	0x3473, 0x07,
	0x3480, 0x49,

	0x3000, 0x00,
	0x0100,	0x01
    #endif
#endif

};

ISP_UINT16 SNR_IMX291_Reg_1280x720_30P[] = 
{
/*
 * 720@30fps-10bit,Parallel CMOS SDR
 * Input 37.125MHz to INCK
 */

	// address, data,
	0x3000, 0x31,
	0x0100, 0x00,

	0x302C, 0x01,


	0x0008, 0x00,
	0x0009, 0x3C,
#if (SENSOR_ROTATE_180)
	0x0101,	0x03, 	// (IMG_ORIENTATION, address 0101h[1]), Vertical (V) scanning direction control (I2C)
					// 0: Normal, 1: Inverted
					// (IMG_ORIENTATION, address 0101h[2]), Horizontal (H) scanning direction control (I2C)
					// 0: Normal, 1: Inverted
#else
	0x0101, 0x00,
#endif
	0x0104, 0x00,
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0202, 0x00,
	0x0203, 0x00,
	0x0340, 0x06,
	0x0341, 0x72,
	0x0342, 0x02,
	0x0343, 0xEE,

	0x3001, 0x00,
	0x3002, 0x01,
	0x3003, 0x72,
	0x3004, 0x06,
	0x3005, 0xEE,
	0x3006, 0x02,
	0x3007, 0x00,
	0x3011, 0x01,
	0x3012, 0x80,
	0x3016, 0xF0,
	0x301F, 0x73,
	0x3020, 0x3C,
	0x3021, 0x80,
	0x3022, 0xC0,
	0x3027, 0x20,
	0x307A, 0x40,
	0x307B, 0x02,
	0x3098, 0x26,
	0x3099, 0x02,
	0x309A, 0x4C,
	0x309B, 0x04,
	0x30CE, 0x00,
	0x30CF, 0x00,
	0x30D0, 0x00,

	// address, data,
	0x3117, 0x0D,

	SENSOR_DELAY_REG, 10, // wait 100ms
	// address, data,
	0x302C, 0x00,

	SENSOR_DELAY_REG, 10, // wait 100ms
	// address, data,
	0x3000, 0x30,

	0x0100, 0x01
};

ISP_UINT16 SNR_IMX291_Reg_1280x720_24P[] =
{
//720@24fps-12bit,Parallel CMOS SDR
//Input 37.125MHz to INCK

        0x3000,  0x31,
        0x0100,  0x00,

        0x302C,  0x01,

        0x0008,  0x00,
        0x0009,  0x3C,
#if (SENSOR_ROTATE_180)
        0x0101,  0x03,   // (IMG_ORIENTATION, address 0101h[1]), Vertical (V) scanning direction control (I2C)
                         // 0: Normal, 1: Inverted
                         // (IMG_ORIENTATION, address 0101h[2]), Horizontal (H) scanning direction control (I2C)
                         // 0: Normal, 1: Inverted
#else
        0x0101,  0x00,
#endif
        0x0104,  0x00,
        0x0112,  0x0A,//0x0A,//0x0C,
        0x0113,  0x0A,//0x0A,//0x0C,
        0x0202,  0x00,
        0x0203,  0x00,
        0x0340,  0x08,//0x02,
        0x0341,  0x0E,//0xEE,
        0x0342,  0x02,//0x08,
        0x0343,  0xEE,//0x0E,

        0x3001,  0x00,
        0x3002,  0x01,
        0x3003,  0x0E,
        0x3004,  0x08,
        0x3005,  0xEE,
        0x3006,  0x02,
        0x3007,  0x00,
        0x3011,  0x01,
        0x3012,  0x80,//0x80,//0x82
        0x3016,  0xF0,
        0x301F,  0x73,
        0x3020,  0x3C,//0x3C,//0xF0
        0x3021,  0x80,//0x80,//0x00
        0x3022,  0xC0,
        0x3027,  0x20,
        0x307A,  0x40,
        0x307B,  0x02,
        0x3098,  0x26,
        0x3099,  0x02,
        0x309A,  0x4C,
        0x309B,  0x04,
        0x30CE,  0x00,//0x00,//0x40
        0x30CF,  0x00,//0x00,//0x81
        0x30D0,  0x00,//0x00,//0x01

        0x3117,  0x0D,
        SENSOR_DELAY_REG, 10,
        0x302C,  0x00,
        SENSOR_DELAY_REG, 10,
        0x3000,  0x30,
        0x0100,  0x01,
};

ISP_UINT16 SNR_IMX291_Reg_1280x720_60P[] = 
{
/*
 * 720@60fps-10bit,Parallel CMOS SDR
 * Input 37.125MHz to INCK
 */

	// address, data,
	0x3000, 0x31,
	0x0100, 0x00,

	0x302C, 0x01,


	0x0008, 0x00,
	0x0009, 0x3C,
#if (SENSOR_ROTATE_180)
	0x0101,	0x03, 	// (IMG_ORIENTATION, address 0101h[1]), Vertical (V) scanning direction control (I2C)
					// 0: Normal, 1: Inverted
					// (IMG_ORIENTATION, address 0101h[2]), Horizontal (H) scanning direction control (I2C)
					// 0: Normal, 1: Inverted
#else
	0x0101, 0x00,
#endif
	0x0104, 0x00,
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0202, 0x00,
	0x0203, 0x00,
	0x0340, 0x03,
	0x0341, 0x39,
	0x0342, 0x02,
	0x0343, 0xEE,

	0x3001, 0x00,
	0x3002, 0x01,
	0x3003, 0x39,
	0x3004, 0x03,
	0x3005, 0xEE,
	0x3006, 0x02,
	0x3007, 0x00,
	0x3011, 0x00,
	0x3012, 0x80,
	0x3016, 0xF0,
	0x301F, 0x73,
	0x3020, 0x3C,
	0x3021, 0x80,
	0x3022, 0xC0,
	0x3027, 0x20,
	0x307A, 0x00,
	0x307B, 0x00,
	0x3098, 0x26,
	0x3099, 0x02,
	0x309A, 0x4C,
	0x309B, 0x04,
	0x30CE, 0x00,
	0x30CF, 0x00,
	0x30D0, 0x00,

	// address, data,
	0x3117, 0x0D,

	SENSOR_DELAY_REG, 10, // wait 100ms
	// address, data,
	0x302C, 0x00,

	SENSOR_DELAY_REG, 10, // wait 100ms
	// address, data,
	0x3000, 0x30,

	0x0100, 0x01
};

ISP_UINT16 SNR_IMX291_Reg_640x480_30P[] = 
{
	// TBD
	SENSOR_DELAY_REG, 10 // delay
};

#if 0
void ____Sensor_Customer_Func____(){ruturn;} // dummy
#endif

const ISP_UINT16 imx322_GainTable[141] = {
	256,
	265,
	274,
	284,
	294,
	304,
	315,
	326,
	338,
	350,
	362,
	375,
	388,
	402,
	416,
	431,
	446,
	461,
	478,
	495,
	512,
	530,
	549,
	568,
	588,
	609,
	630,
	653,
	676,
	699,
	724,
	750,
	776,
	803,
	832,
	861,
	891,
	923,
	955,
	989,
	1024,
	1060,
	1097,
	1136,
	1176,
	1218,
	1261,
	1305,
	1351,
	1399,
	1448,
	1499,
	1552,
	1607,
	1663,
	1722,
	1783,
	1846,
	1911,
	1978,
	2048,
	2120,
	2195,
	2272,
	2353,
	2435,
	2521,
	2610,
	2702,
	2798,
	2896,
	2998,
	3104,
	3214,
	3327,
	3444,
	3566,
	3692,
	3822,
	3956,
	4096,
	4240,
	4390,
	4545,
	4705,
	4871,
	5043,
	5221,
	5405,
	5595,
	5793,
	5997,
	6208,
	6427,
	6654,
	6889,
	7132,
	7383,
	7643,
	7913,
	8192,
	8481,
	8780,
	9090,
	9410,
	9742,
	10086,
	10441,
	10809,
	11191,
	11585,
	11994,
	12417,
	12855,
	13308,
	13777,
	14263,
	14766,
	15287,
	15826,
	16384,
	16962,
	17560,
	18179,
	18820,
	19484,
	20171,
	20882,
	21619,
	22381,
	23170,
	23988,
	24834,
	25709,
	26616,
	27554,
	28526,
	29532,
	30574,
	31652,
	32768,
};
//------------------------------------------------------------------------------
//  Function    : SNR_Cust_InitConfig
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_InitConfig(void)
{
	MMP_USHORT 	i;
	MMP_ULONG 	ulSensorClkSrc;
    
    #if (IMX291_MCLK_74250KHZ)
	MMP_ULONG 	ulSensorMCLK = 74250; // 74.250 M
    #else
	MMP_ULONG 	ulSensorMCLK = 37125; // 37.125 M
	#endif
#if (SENSOR_IF == SENSOR_IF_PARALLEL)
	RTNA_DBG_Str0(FG_PURPLE("SNR_Cust_InitConfig IMX291 Parallel\r\n"));
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    RTNA_DBG_Str0(FG_PURPLE("SNR_Cust_InitConfig IMX291 MIPI 2-lane\r\n"));
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
	RTNA_DBG_Str0(FG_PURPLE("SNR_Cust_InitConfig IMX291 MIPI 4-lane\r\n"));
#endif
	// Init OPR Table
	SensorCustFunc.OprTable->usInitSize 							= (sizeof(SNR_IMX291_Reg_Init_Customer)/sizeof(SNR_IMX291_Reg_Init_Customer[0]))/2;
	SensorCustFunc.OprTable->uspInitTable 							= &SNR_IMX291_Reg_Init_Customer[0];

	for (i = 0; i < MAX_SENSOR_RES_MODE; i++)
	{
		SensorCustFunc.OprTable->usSize[i] 							= (sizeof(SNR_IMX291_Reg_Unsupport)/sizeof(SNR_IMX291_Reg_Unsupport[0]))/2;
		SensorCustFunc.OprTable->uspTable[i] 						= &SNR_IMX291_Reg_Unsupport[0];
	}

	// 16:9
	SensorCustFunc.OprTable->usSize[RES_IDX_1920x1080_30P]			= (sizeof(SNR_IMX291_Reg_1920x1080_30P)/sizeof(SNR_IMX291_Reg_1920x1080_30P[0]))/2;
	SensorCustFunc.OprTable->uspTable[RES_IDX_1920x1080_30P] 		= &SNR_IMX291_Reg_1920x1080_30P[0];
#if 1 // cropped from FHD30P, TBC??
	SensorCustFunc.OprTable->usSize[RES_IDX_1280x720_30P] 			= (sizeof(SNR_IMX291_Reg_1920x1080_30P)/sizeof(SNR_IMX291_Reg_1920x1080_30P[0]))/2;
	SensorCustFunc.OprTable->uspTable[RES_IDX_1280x720_30P] 		= &SNR_IMX291_Reg_1920x1080_30P[0];
#else
	SensorCustFunc.OprTable->usSize[RES_IDX_1280x720_30P] 			= (sizeof(SNR_IMX291_Reg_1280x720_30P)/sizeof(SNR_IMX291_Reg_1280x720_30P[0]))/2; // TBD
	SensorCustFunc.OprTable->uspTable[RES_IDX_1280x720_30P] 		= &SNR_IMX291_Reg_1280x720_30P[0]; // TBD
#endif
	SensorCustFunc.OprTable->usSize[RES_IDX_1920x1080_24P]           = (sizeof(SNR_IMX291_Reg_1920x1080_24P)/sizeof(SNR_IMX291_Reg_1920x1080_24P[0]))/2;
	SensorCustFunc.OprTable->uspTable[RES_IDX_1920x1080_24P]         = &SNR_IMX291_Reg_1920x1080_24P[0];
#if 1
	SensorCustFunc.OprTable->usSize[RES_IDX_1280x720_24P]           = (sizeof(SNR_IMX291_Reg_1920x1080_24P)/sizeof(SNR_IMX291_Reg_1920x1080_24P[0]))/2;
	SensorCustFunc.OprTable->uspTable[RES_IDX_1280x720_24P]         = &SNR_IMX291_Reg_1920x1080_24P[0];
#else
	SensorCustFunc.OprTable->usSize[RES_IDX_1280x720_24P]           = (sizeof(SNR_IMX291_Reg_1280x720_24P)/sizeof(SNR_IMX291_Reg_1280x720_24P[0]))/2;
	SensorCustFunc.OprTable->uspTable[RES_IDX_1280x720_24P]         = &SNR_IMX291_Reg_1280x720_24P[0];
#endif
	SensorCustFunc.OprTable->usSize[RES_IDX_1280x720_60P] 			= (sizeof(SNR_IMX291_Reg_1280x720_60P)/sizeof(SNR_IMX291_Reg_1280x720_60P[0]))/2;
	SensorCustFunc.OprTable->uspTable[RES_IDX_1280x720_60P] 		= &SNR_IMX291_Reg_1280x720_60P[0];

	// 4:3
#if 1 // cropped from FHD30P, TBC??
	SensorCustFunc.OprTable->usSize[RES_IDX_1440x1080_30P] 			= (sizeof(SNR_IMX291_Reg_1920x1080_30P)/sizeof(SNR_IMX291_Reg_1920x1080_30P[0]))/2;
	SensorCustFunc.OprTable->uspTable[RES_IDX_1440x1080_30P] 		= &SNR_IMX291_Reg_1920x1080_30P[0];
	SensorCustFunc.OprTable->usSize[RES_IDX_1280x960_30P] 			= (sizeof(SNR_IMX291_Reg_1920x1080_30P)/sizeof(SNR_IMX291_Reg_1920x1080_30P[0]))/2;
	SensorCustFunc.OprTable->uspTable[RES_IDX_1280x960_30P] 		= &SNR_IMX291_Reg_1920x1080_30P[0];
	SensorCustFunc.OprTable->usSize[RES_IDX_640x480_30P] 			= (sizeof(SNR_IMX291_Reg_1920x1080_30P)/sizeof(SNR_IMX291_Reg_1920x1080_30P[0]))/2;
	SensorCustFunc.OprTable->uspTable[RES_IDX_640x480_30P] 			= &SNR_IMX291_Reg_1920x1080_30P[0];
#else
	SensorCustFunc.OprTable->usSize[RES_IDX_1440x1080_30P] 			= (sizeof(SNR_IMX291_Reg_1440x1080_30P)/sizeof(SNR_IMX291_Reg_1440x1080_30P[0]))/2; // TBD
	SensorCustFunc.OprTable->uspTable[RES_IDX_1440x1080_30P] 		= &SNR_IMX291_Reg_1440x1080_30P[0];
	SensorCustFunc.OprTable->usSize[RES_IDX_1280x960_30P] 			= (sizeof(SNR_IMX291_Reg_1280x960_30P)/sizeof(SNR_IMX291_Reg_1280x960_30P[0]))/2; // TBD
	SensorCustFunc.OprTable->uspTable[RES_IDX_1280x960_30P] 		= &SNR_IMX291_Reg_1280x960_30P[0]; // TBD
	SensorCustFunc.OprTable->usSize[RES_IDX_640x480_30P] 			= (sizeof(SNR_IMX291_Reg_640x480_30P)/sizeof(SNR_IMX291_Reg_640x480_30P[0]))/2; // TBD
	SensorCustFunc.OprTable->uspTable[RES_IDX_640x480_30P] 			= &SNR_IMX291_Reg_640x480_30P[0]; // TBD
#endif

	// Init Vif Setting : Common
#if (SENSOR_IF == SENSOR_IF_PARALLEL)
	SensorCustFunc.VifSetting->OutInterface 						= MMPF_VIF_IF_PARALLEL;
#elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
#if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
	SensorCustFunc.VifSetting->OutInterface 						= MMPF_VIF_IF_MIPI_SINGLE_1;
#else 	
	SensorCustFunc.VifSetting->OutInterface 						= MMPF_VIF_IF_MIPI_SINGLE_0;
#endif
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
#if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
	SensorCustFunc.VifSetting->OutInterface 						= MMPF_VIF_IF_MIPI_DUAL_12;
#else
	SensorCustFunc.VifSetting->OutInterface 						= MMPF_VIF_IF_MIPI_DUAL_01; //TBD
#endif
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
	SensorCustFunc.VifSetting->OutInterface							= MMPF_VIF_IF_MIPI_QUAD;
#endif

#if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
    SensorCustFunc.VifSetting->VifPadId							    = MMPF_VIF_MDL_ID1;
#else
	SensorCustFunc.VifSetting->VifPadId 							= MMPF_VIF_MDL_ID0;
#endif

	// Init Vif Setting : PowerOn Setting
	SensorCustFunc.VifSetting->powerOnSetting.bTurnOnExtPower 		= MMP_TRUE;
	SensorCustFunc.VifSetting->powerOnSetting.usExtPowerPin 		= SENSOR_GPIO_ENABLE; // it might be defined in Config_SDK.h
	SensorCustFunc.VifSetting->powerOnSetting.bExtPowerPinHigh 		= SENSOR_GPIO_ENABLE_ACT_LEVEL;
	SensorCustFunc.VifSetting->powerOnSetting.usExtPowerPinDelay 	= 0;//50;
	SensorCustFunc.VifSetting->powerOnSetting.bFirstEnPinHigh 		= MMP_FALSE;
	SensorCustFunc.VifSetting->powerOnSetting.ubFirstEnPinDelay 	= 10;
	SensorCustFunc.VifSetting->powerOnSetting.bNextEnPinHigh 		= MMP_TRUE;
	SensorCustFunc.VifSetting->powerOnSetting.ubNextEnPinDelay 		= 10;//100;
	SensorCustFunc.VifSetting->powerOnSetting.bTurnOnClockBeforeRst = MMP_TRUE;
	SensorCustFunc.VifSetting->powerOnSetting.bFirstRstPinHigh 		= MMP_FALSE;
	SensorCustFunc.VifSetting->powerOnSetting.ubFirstRstPinDelay 	= 30;//100;
	SensorCustFunc.VifSetting->powerOnSetting.bNextRstPinHigh 		= MMP_TRUE;
	SensorCustFunc.VifSetting->powerOnSetting.ubNextRstPinDelay 	= 10;//100;

	// Init Vif Setting : PowerOff Setting
	SensorCustFunc.VifSetting->powerOffSetting.bEnterStandByMode 	= MMP_FALSE;
	SensorCustFunc.VifSetting->powerOffSetting.usStandByModeReg 	= 0x0100;
	SensorCustFunc.VifSetting->powerOffSetting.usStandByModeMask 	= 0x01;
	SensorCustFunc.VifSetting->powerOffSetting.bEnPinHigh 			= MMP_TRUE;
	SensorCustFunc.VifSetting->powerOffSetting.ubEnPinDelay 		= 20;
	SensorCustFunc.VifSetting->powerOffSetting.bTurnOffMClock 		= MMP_TRUE;
	SensorCustFunc.VifSetting->powerOffSetting.bTurnOffExtPower 	= MMP_TRUE;
	SensorCustFunc.VifSetting->powerOffSetting.usExtPowerPin 		= SENSOR_GPIO_ENABLE; // it might be defined in Config_SDK.h

	// Init Vif Setting : Sensor MClock Setting
	MMPF_PLL_GetGroupFreq(CLK_GRP_SNR, &ulSensorClkSrc);

	SensorCustFunc.VifSetting->clockAttr.bClkOutEn 					= MMP_TRUE;
	SensorCustFunc.VifSetting->clockAttr.ubClkFreqDiv 				= ulSensorClkSrc / ulSensorMCLK; // (528MHz / 37.125MHz)
	SensorCustFunc.VifSetting->clockAttr.ulMClkFreq 				= 24000;
	SensorCustFunc.VifSetting->clockAttr.ulDesiredFreq 				= 24000;
	SensorCustFunc.VifSetting->clockAttr.ubClkPhase 				= MMPF_VIF_SNR_PHASE_DELAY_NONE;
	SensorCustFunc.VifSetting->clockAttr.ubClkPolarity 				= MMPF_VIF_SNR_CLK_POLARITY_NEG;
	SensorCustFunc.VifSetting->clockAttr.ubClkSrc 					= MMPF_VIF_SNR_CLK_SRC_VIFCLK;

	// Init Vif Setting : Parallel Sensor Setting
	SensorCustFunc.VifSetting->paralAttr.ubLatchTiming 				= MMPF_VIF_SNR_LATCH_POS_EDGE;
	SensorCustFunc.VifSetting->paralAttr.ubHsyncPolarity 			= MMPF_VIF_SNR_CLK_POLARITY_POS;
	SensorCustFunc.VifSetting->paralAttr.ubVsyncPolarity 			= MMPF_VIF_SNR_CLK_POLARITY_POS;
    SensorCustFunc.VifSetting->paralAttr.ubBusBitMode               = MMPF_VIF_SNR_PARAL_BITMODE_16;
    
	// Init Vif Setting : MIPI Sensor Setting
	SensorCustFunc.VifSetting->mipiAttr.bClkDelayEn 				= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bClkLaneSwapEn 				= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.usClkDelay 					= 0;
	SensorCustFunc.VifSetting->mipiAttr.ubBClkLatchTiming 			= MMPF_VIF_SNR_LATCH_POS_EDGE;
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
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_1; // TBD
	#else
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_0;
	#endif
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[2] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[3] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[0] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[1] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[2] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[3] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[0] 			= 0x1F;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[1] 			= 0x1F;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[2] 			= 0x1F;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[3] 			= 0x1F;
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[0] 				= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[1] 				= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[2] 				= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[3] 				= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[0] 			= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[1] 			= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[2] 			= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[3] 			= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[0] 			= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[1] 			= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[2] 			= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[3] 			= MMP_FALSE;
	#if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_1;
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_2;
	#else
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_0; // TBD
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_1;	
	#endif
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[2] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[3] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[0] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[1] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[2] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[3] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[0] 			= 0x1F;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[1] 			= 0x1F;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[2] 			= 0x1F;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[3] 			= 0x1F;
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[0] 				= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[1] 				= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[2] 				= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[3] 				= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[0] 			= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[1] 			= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[2] 			= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[3] 			= MMP_TRUE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[0] 			= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[1] 			= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[2] 			= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[3] 			= MMP_FALSE;
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_0;
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_1;
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[2] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_2;
	SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[3] 			= MMPF_VIF_MIPI_DATA_SRC_PHY_3;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[0] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[1] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[2] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.usDataDelay[3] 				= 0x08;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[0] 			= 0x1F;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[1] 			= 0x1F;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[2] 			= 0x1F;
	SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[3] 			= 0x1F;
#endif

	// Init Vif Setting : Color ID Setting
	SensorCustFunc.VifSetting->colorId.VifColorId 					= MMPF_VIF_COLORID_00;
	SensorCustFunc.VifSetting->colorId.CustomColorId.bUseCustomId 	= MMP_FALSE;
}

//20150526
//lv1-7, 2	3	6	11	17	30	60
#if 1//	new extent node for18//LV1,		LV2,		LV3,		LV4,		LV5,		LV6,		LV7,		LV8,		LV9,		LV10,	LV11,	LV12,	LV13,	LV14,	LV15,	LV16 	LV17  	LV18
//abby curve iq 12
ISP_UINT32 AE_Bias_tbl[54] =
/*lux*/						{2,			3,			5,			11,			21, 		44, 		88, 		179, 		350,		686,	1356,	2694,	5304,	10200,	20400,	40800,	81600, 163200/*930000=LV17*/
/*ENG*/						,0x2FFFFFF, 4841472*2,	3058720,	1962240,	1095560,  	616000, 	334880, 	181720,     96600,	 	52685,	27499,	14560,	8060,	4176,	2216,	1144,	600,   300
/*Tar*/						,86,		86,		    86,	        126,		     167,	 	178,	 196,	 	230,	    280,	    280,	280,	280,	280,	280,	280,	280,	280,   280
 };	
#define AE_tbl_size  (18)	//32  35  44  50
#endif

#define AGAIN_1X  0x100
#define DGAIN_1X  0x100
#define IMX122_MaxGain 128

static ISP_UINT32	/*again, */dgain, s_gain/*, tmp_shutter*/;
ISP_UINT32 isp_gain;

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	static ISP_UINT16 ae_gain;
	static ISP_UINT16 ae_shutter;
	static ISP_UINT16 ae_vsync;
	
	MMP_ULONG ulVsync = 0;
	MMP_ULONG ulShutter = 0;
	
	if(ulFrameCnt<2)
		return ;
	
	if (ulFrameCnt % 100 == 10) {
		ISP_IF_F_SetWDREn(1);
		ISP_IF_CMD_SendCommandtoAE(0x51,AE_Bias_tbl,AE_tbl_size,0);         // <<AE table set once at preview start
		ISP_IF_NaturalAE_Enable(2);	//0: no , 1:ENERGY 2: Lux 3: test mode
    	ISP_IF_CMD_SendCommandtoAE(0x52,0,0,1);
	}

	if ((ulFrameCnt % m_AeTime.ubPeriod) == 0) 
	{
		ISP_IF_AE_Execute();

		ae_gain 	= ISP_IF_AE_GetGain();
		ae_shutter 	= ISP_IF_AE_GetShutter();
		ae_vsync 	= ISP_IF_AE_GetVsync();
	}
	
	ulVsync 	= (gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetVsync()) / ISP_IF_AE_GetVsyncBase();
	ulShutter 	= (gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetShutter()) / ISP_IF_AE_GetShutterBase();
	
	dgain = DGAIN_1X;
	dgain = dgain * 4 * ulShutter / (4*ulShutter -1);
	s_gain= ae_gain * dgain/DGAIN_1X;
	
	if( s_gain > AGAIN_1X * IMX122_MaxGain )
	{
		s_gain = AGAIN_1X * IMX122_MaxGain;
		dgain  = DGAIN_1X * s_gain / (AGAIN_1X*IMX122_MaxGain);
	}
    else
	{
	    dgain = DGAIN_1X;
	}

	if ((ulFrameCnt % m_AeTime.ubPeriod) == m_AeTime.ubFrmStSetShutFrmCnt)
	{
        gsSensorFunction->MMPF_Sensor_SetReg(PRM_SENSOR, 0x3001, 0x01); // Enable hold
        gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, ulShutter, ulVsync);
        gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, s_gain);
        gsSensorFunction->MMPF_Sensor_SetReg(PRM_SENSOR, 0x3001, 0x00); // off hold
	}


/*
	if ((ulFrameCnt % m_AeTime.ubPeriod) == m_AeTime.ubFrmStSetShutFrmCnt)
		gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, ulShutter, ulVsync);
	
	if ((ulFrameCnt % m_AeTime.ubPeriod) == m_AeTime.ubFrmStSetGainFrmCnt)
		gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, s_gain);
*/	
	//casio
	if ((ulFrameCnt % m_AeTime.ubPeriod) == 2)
		ISP_IF_IQ_SetAEGain(DGAIN_1X, DGAIN_1X);

         //RTNA_DBG_Str(0,"\r\n colortemp");
	     //RTNA_DBG_Long(0, ISP_IF_AWB_GetColorTemp());  

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
void SNR_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAWB_FrmEnd
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_DoAWB_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
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
	ISP_UINT8 i, sensor_gain;
	
	//ulGain = 1024;
	
	for( i = 0; i< 141; i++)
	{
		if (ulGain >= imx322_GainTable[i])
			sensor_gain = i;
		else
			break;
	}

	//isp_gain = isp_gain*ulGain/imx322_GainTable[sensor_gain];
	
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3014, sensor_gain);
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
	ISP_UINT32 new_vsync 	= vsync;  
	ISP_UINT32 new_shutter 	= shutter;

	new_vsync 	= ISP_MIN(ISP_MAX((new_shutter + 3), new_vsync), 0xFFFF);	
	new_shutter = ISP_MIN(ISP_MAX(new_shutter, 1), (new_vsync - 3));
	
	
//	printc("===>new_vsync=%d new_shutter=%d\n",new_vsync,new_shutter);
	// VSYNC
	
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3018, ((( new_vsync ) >> 0 ) & 0xFF));
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3019, ((( new_vsync ) >> 8 ) & 0xFF));
	
	
	//gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3018, 0x46);
	//gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3019, 0x05);
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x301A, ((( new_vsync ) >> 16 ) & 0xFF));

	// SHUTTER
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3020,  ((( new_vsync - new_shutter) >> 0  ) & 0xFF));
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3021,  ((( new_vsync - new_shutter) >> 8  ) & 0xFF));
	gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x3022,  ((( new_vsync - new_shutter) >> 16 ) & 0xFF));
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
	// TBD
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
    if (SensorCustFunc.sPriority == MMP_SNR_PRIO_PRM)
    MMPF_SensorDrv_Register(PRM_SENSOR, &SensorCustFunc);
    else
        MMPF_SensorDrv_Register(SCD_SENSOR, &SensorCustFunc); 
    
    return 0;
}

#pragma arm section code = "initcall6", rodata = "initcall6", rwdata = "initcall6", zidata = "initcall6"
#pragma O0
ait_module_init(SNR_Module_Init);
#pragma
#pragma arm section rodata, rwdata, zidata

#endif // (BIND_SENSOR_IMX291)
#endif // (SENSOR_EN)
