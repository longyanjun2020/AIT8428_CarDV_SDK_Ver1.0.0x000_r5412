//==============================================================================
//
//  File        : sensor_ar0832e.c
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
#if (BIND_SENSOR_AR0832E)

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

MMPF_SENSOR_RESOLUTION m_SensorRes = 
{
    7,                                      // ubSensorModeNum
    0,                                      // ubDefPreviewMode
    2,                                      // ubDefCaptureMode
    1400,                                   // usPixelSize
//  Mode0       Mode1       Mode2
//  1296@30P    720@30P     1944@18P
    {1,         1,          1,          },  // usVifGrabStX
    {1,         1,          1,          },  // usVifGrabStY
    {2312,      1296,       2600,       },  // usVifGrabW
    {1304,      732,   	    1952,       },  // usVifGrabH
    #if (CHIP == MCR_V2)
    {1,         1,          1,          },  // usBayerInGrabX
    {1,         1,          1,          },  // usBayerInGrabY
    {8,         16,         8,          },  // usBayerInDummyX
    {8,         12,         8,          },  // usBayerInDummyY
    {2304,  	1280,       2592,       },  // usBayerOutW
    {1296,  	720,   	    1944,       },  // usBayerOutH
    #endif 
    {2304,      1280,       2592,       },  // usScalInputW
    {1296,      720,   	    1944,       },  // usScalInputH
    {300,       300,        180,        },  // usTargetFpsx10
    {1571,      1450,       2119,       },  // usVsyncLine
    {1,         1,          1,          },  // ubWBinningN
    {1,         1,          1,          },  // ubWBinningM
    {1,         1,          1,          },  // ubHBinningN
    {1,         1,          1,          },  // ubHBinningM
    {0xFF,     	0xFF,      	0xFF,      	},  // ubCustIQmode
    {0xFF,     	0xFF,      	0xFF,      	},  // ubCustAEmode
};

// OPR Table and Vif Setting
MMPF_SENSOR_OPR_TABLE   m_OprTable;
MMPF_SENSOR_VIF_SETTING m_VifSetting;

// IQ Table
const ISP_UINT8 Sensor_IQ_CompressedText[] = 
{
    #if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC)
    #include "isp_8428_iq_data_v3_AR0832E_ezmode.xls.ciq.txt"
    #else // Use old IQ table
    #ifdef CUS_ISP_8428_IQ_DATA // maybe defined in project MCP or Config_SDK_xxx.h
    #include CUS_ISP_8428_IQ_DATA
    #else
    #include "isp_8428_iq_data_v2_AR0832E.xls.ciq.txt"
    #endif
    #endif
};

#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC)
// Replace it by custom IQ table.
const __align(4) ISP_UINT8 Sensor_EZ_IQ_CompressedText[] = 
{
    //#include "ez_iq_8428_AR0832E.txt"
    NULL
};

ISP_UINT32 eziqsize = sizeof(Sensor_EZ_IQ_CompressedText);
#endif

// I2cm Attribute
static MMP_I2CM_ATTR m_I2cmAttr = 
{
    MMP_I2CM0,                  // i2cmID
    SENSOR_I2C_ADDR_AR0832E,    // ubSlaveAddr
    16,                         // ubRegLen
    16,                         // ubDataLen
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
MMPF_SENSOR_AWBTIMIMG m_AwbTime = 
{
	6,	/* ubPeriod */
	1, 	/* ubDoAWBFrmCnt */
	3	/* ubDoCaliFrmCnt */
};

MMPF_SENSOR_AETIMIMG m_AeTime = 
{	
	5, 	/* ubPeriod */
	0, 	/* ubFrmStSetShutFrmCnt */
	0	/* ubFrmStSetGainFrmCnt */
};

MMPF_SENSOR_AFTIMIMG m_AfTime = 
{
	1, 	/* ubPeriod */
	0	/* ubDoAFFrmCnt */
};

// IQ Data
#define ISP_IQ_DATA_NAME "isp_8428_iq_data_v2_AR0832E.xls.ciq.txt"

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

ISP_UINT16 SNR_AR0832E_Reg_Init_Customer[] = 
{
    0x301A, 0x0658, // RESET_REGISTER
    0x301A, 0x0650, // RESET_REGISTER
    0x0104, 0x0001, // GROUPED_PARAMETER_HOLD
    0x301D, 0x0001, // IMAGE_ORIENTATION
    0x3044, 0x0590, // RESERVED_MFR_3044
    0x306E, 0xFC80, // DATAPATH_SELECT
    0x30B2, 0xC000, // RESERVED_MFR_30B2
    0x30D6, 0x0800, // RESERVED_MFR_30D6
    0x316C, 0xB214, // RESERVED_MFR_316C
    0x316E, 0x828A, // RESERVED_MFR_316E
    0x3170, 0x210E, // RESERVED_MFR_3170
    0x3174, 0x8000, // RESERVED_MFR_3174
    0x317A, 0x010E, // RESERVED_MFR_317A
    0x31E0, 0x1FB9, // RESERVED_MFR_31E0
    0x31E6, 0x07FC, // RESERVED_MFR_31E6
    0x37C0, 0x0000, // P_GR_Q5
    0x37C2, 0x0000, // P_RD_Q5
    0x37C4, 0x0000, // P_BL_Q5
    0x37C6, 0x0000, // P_GB_Q5
    0x3E00, 0x0011, // RESERVED_MFR_3E00
    0x3E02, 0x8801, // RESERVED_MFR_3E02
    0x3E04, 0x2801, // RESERVED_MFR_3E04
    0x3E06, 0x8449, // RESERVED_MFR_3E06
    0x3E08, 0x6B41, // RESERVED_MFR_3E08
    0x3E0A, 0x400C, // RESERVED_MFR_3E0A
    0x3E0C, 0x1001, // RESERVED_MFR_3E0C
    0x3E0E, 0x2603, // RESERVED_MFR_3E0E
    0x3E10, 0x4B41, // RESERVED_MFR_3E10
    0x3E12, 0x4B24, // RESERVED_MFR_3E12
    0x3E14, 0xA3CF, // RESERVED_MFR_3E14
    0x3E16, 0x8802, // RESERVED_MFR_3E16
    0x3E18, 0x8401, // RESERVED_MFR_3E18
    0x3E1A, 0x8601, // RESERVED_MFR_3E1A
    0x3E1C, 0x8401, // RESERVED_MFR_3E1C
    0x3E1E, 0x840A, // RESERVED_MFR_3E1E
    0x3E20, 0xFF00, // RESERVED_MFR_3E20
    0x3E22, 0x8401, // RESERVED_MFR_3E22
    0x3E24, 0x00FF, // RESERVED_MFR_3E24
    0x3E26, 0x0088, // RESERVED_MFR_3E26
    0x3E28, 0x2E8A, // RESERVED_MFR_3E28
    0x3E30, 0x0000, // RESERVED_MFR_3E30
    0x3E32, 0x8801, // RESERVED_MFR_3E32
    0x3E34, 0x4029, // RESERVED_MFR_3E34
    0x3E36, 0x00FF, // RESERVED_MFR_3E36
    0x3E38, 0x846C, // RESERVED_MFR_3E38
    0x3E3A, 0x00FF, // RESERVED_MFR_3E3A
    0x3E3C, 0x2801, // RESERVED_MFR_3E3C
    0x3E3E, 0x3E2A, // RESERVED_MFR_3E3E
    0x3E40, 0x1C01, // RESERVED_MFR_3E40
    0x3E42, 0x8486, // RESERVED_MFR_3E42
    0x3E44, 0x8401, // RESERVED_MFR_3E44
    0x3E46, 0x0C01, // RESERVED_MFR_3E46
    0x3E48, 0x8401, // RESERVED_MFR_3E48
    0x3E4A, 0x00FF, // RESERVED_MFR_3E4A
    0x3E4C, 0x8402, // RESERVED_MFR_3E4C
    0x3E4E, 0x8984, // RESERVED_MFR_3E4E
    0x3E50, 0x6928, // RESERVED_MFR_3E50
    0x3E52, 0x8340, // RESERVED_MFR_3E52
    0x3E54, 0x00FF, // RESERVED_MFR_3E54
    0x3E56, 0x4A42, // RESERVED_MFR_3E56
    0x3E58, 0x2703, // RESERVED_MFR_3E58
    0x3E5A, 0x6752, // RESERVED_MFR_3E5A
    0x3E5C, 0x3F2A, // RESERVED_MFR_3E5C
    0x3E5E, 0x846D, // RESERVED_MFR_3E5E
    0x3E60, 0x4C01, // RESERVED_MFR_3E60
    0x3E62, 0x8401, // RESERVED_MFR_3E62
    0x3E66, 0x3901, // RESERVED_MFR_3E66
    0x3E90, 0x2C01, // RESERVED_MFR_3E90
    0x3E92, 0x2A04, // RESERVED_MFR_3E92
    0x3E94, 0x2509, // RESERVED_MFR_3E94
    0x3E96, 0xF000, // RESERVED_MFR_3E96
    0x3E98, 0x2B02, // RESERVED_MFR_3E98
    0x3E9A, 0x2905, // RESERVED_MFR_3E9A
    0x3E9C, 0x00FF, // RESERVED_MFR_3E9C
    0x3ECC, 0x00E2, // RESERVED_MFR_3ECC
    0x3ED0, 0x2024, // RESERVED_MFR_3ED0
    0x3ED4, 0xF4B2, // RESERVED_MFR_3ED4
    0x3ED6, 0x909B, // RESERVED_MFR_3ED6
    0x3EDE, 0x2429, // RESERVED_MFR_3EDE
    0x3EE0, 0x2929, // RESERVED_MFR_3EE0
    0x3EE4, 0xC100, // RESERVED_MFR_3EE4
    0x3EE6, 0x0540, // RESERVED_MFR_3EE6
    0x3EDA, 0xD005, // RESERVED_MFR_3EDA
    0x31B0, 0x0083, // FRAME_PREAMBLE
    0x31B2, 0x004D, // LINE_PREAMBLE
    0x31B4, 0x0E67, // MIPI_TIMING_0
    0x31B6, 0x0D24, // MIPI_TIMING_1
    0x31B8, 0x020E, // MIPI_TIMING_2
    0x31BA, 0x0710, // MIPI_TIMING_3
    0x31BC, 0x2A0D, // MIPI_TIMING_4
    0x31BE, 0xC007, // RESERVED_MFR_31BE
    0x3064, 0x7800, // RESERVED_MFR_3064
    0x31AE, 0x0202, // SERIAL_FORMAT
    0x31B8, 0x0E3F, // MIPI_TIMING_2
    0x0112, 0x0A0A, // CCP_DATA_FORMAT

    0x0300, 0x0004, // VT_PIX_CLK_DIV       0x0005 (24fps)
    0x0302, 0x0001, // VT_SYS_CLK_DIV
    0x0304, 0x0003, // PRE_PLL_CLK_DIV
    0x0306, 0x0060,//0x0064, // PLL_MULTIPLIER
    0x0308, 0x000A, // OP_PIX_CLK_DIV
    0x030A, 0x0001, // OP_SYS_CLK_DIV

    /*
    0x0344, 0x0008, // X_ADDR_START
    0x0348, 0x0CC9, // X_ADDR_END           diff: 0x0cc1 = 3264
    0x0346, 0x013A, // Y_ADDR_START
    0x034A, 0x0867, // Y_ADDR_END           diff: 0x072d = 1836

    0x034C, 0x0510, // X_OUTPUT_SIZE        0x0510 = 1296 (1280+8x2)
    0x034E, 0x02DC, // Y_OUTPUT_SIZE        0x02DC = 732 (720+6x2)

    0x3040, 0x40C1, // READ_MODE
    0x3040, 0x40C3, // READ_MODE
    0x306E, 0xFCB0, // DATAPATH_SELECT
    0x3040, 0x50C3, // READ_MODE
    0x3040, 0x50C3, // READ_MODE
    0x3040, 0x54C3, // READ_MODE
    0x3040, 0x54C3, // READ_MODE
    0x3178, 0x0000, // RESERVED_MFR_3178
    0x3ED0, 0x2024, // RESERVED_MFR_3ED0
    0x0400, 0x0002, // SCALING_MODE
    0x0404, 0x0014, // SCALE_M
    0x0340, 0x05AA, // FRAME_LENGTH_LINES
    0x0342, 0x1124,//0x11F4, // LINE_LENGTH_PCK
    0x0202, 0x05A9, // COARSE_INTEGRATION_TIME
    0x3014, 0x0AEE, // FINE_INTEGRATION_TIME
    0x3010, 0x0284, // FINE_CORRECTION
    */

    0x301E, 0x002A, //data pedestal

    0x301A, 0x0258, // RESET_REGISTER
    0x301A, 0x0658, // RESET_REGISTER
    0x301A, 0x8658, // RESET_REGISTER

    0x0104, 0x0000, // GROUPED_PARAMETER_HOLD
    0x301A, 0x065C, // RESET_REGISTER
};

#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} // dummy
#endif

ISP_UINT16 SNR_AR0832E_Reg_Unsupport[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};

ISP_UINT16 SNR_AR0832E_Reg_2312x1304_30P_Customer[] = 
{
    0x0344, 0x0175, // X_ADDR_START
    0x0348, 0x0B90,//0x0C80, // X_ADDR_END
    0x0346, 0x0250,//0x0206, // Y_ADDR_START
    0x034A, 0x0799, // Y_ADDR_END
    0x034C, 0x0908,//0x0788, // X_OUTPUT_SIZE
    0x034E, 0x0518,//0x0440, // Y_OUTPUT_SIZE

    0x306E, 0xFC80, // DATAPATH_SELECT
    0x3040, 0x4041, // READ_MODE
    0x3178, 0x0000, // ANALOG_CONTROL5
    0x0400, 0x0001,//0x0002, // SCALING_MODE
    0x0404, 0x0011,//0x0015, // SCALE_M
    0x0342, 0x0FE8, // LINE_LENGTH_PCK
    0x0340, 0x0623, // FRAME_LENGTH_LINES
    0x0202, 0x0623, // COARSE_INTEGRATION_TIME
    0x3014, 0x0B48, // FINE_INTEGRATION_TIME
    0x3010, 0x0078, // FINE_CORRECTION
};

ISP_UINT16 SNR_AR0832E_Reg_1296x732_30P_Customer[] = 
{
#if 1 // 1280x720
    0x0344, 0x0008, // X_ADDR_START
    0x0348, 0x0CC9, // X_ADDR_END           diff: 0x0cc1 = 3264
    0x0346, 0x013A, // Y_ADDR_START
    0x034A, 0x0867, // Y_ADDR_END           diff: 0x072d = 1836

    0x034C, 0x0510, // X_OUTPUT_SIZE        0x0510 = 1296 (1280+8x2)
    0x034E, 0x02DC, // Y_OUTPUT_SIZE        0x02DC = 732 (720+6x2)

    0x3040, 0x40C1, // READ_MODE
    0x3040, 0x40C3, // READ_MODE
    0x306E, 0xFCB0, // DATAPATH_SELECT
    0x3040, 0x50C3, // READ_MODE
    0x3040, 0x50C3, // READ_MODE
    0x3040, 0x54C3, // READ_MODE
    0x3040, 0x54C3, // READ_MODE
    0x3178, 0x0000, // RESERVED_MFR_3178
    0x3ED0, 0x2024, // RESERVED_MFR_3ED0
    0x0400, 0x0002, // SCALING_MODE
    0x0404, 0x0014, // SCALE_M
    0x3010, 0x0284, // FINE_CORRECTION
    0x0202, 0x05A9, // COARSE_INTEGRATION_TIME
    0x3014, 0x0AEE, // FINE_INTEGRATION_TIME
    0x0340, 0x05AA, // FRAME_LENGTH_LINES
    0x0342, 0x1124,//0x11F4, // LINE_LENGTH_PCK
#else // 1280x960
    0x0344, 0x0008, // X_ADDR_START
    0x0348, 0x0CC5, // X_ADDR_END
    0x0346, 0x0008, // Y_ADDR_START
    0x034A, 0x0995, // Y_ADDR_END
    0x034C, 0x0510, // X_OUTPUT_SIZE
    0x034E, 0x03C8, // Y_OUTPUT_SIZE

    0x306E, 0xFCB0, // DATAPATH_SELECT
    0x3040, 0x54C3, // READ_MODE
    0x3178, 0x0000, // ANALOG_CONTROL5
    0x3ED0, 0x2024, // DAC_LD_4_5
    0x0400, 0x0002, // SCALING_MODE
    0x0404, 0x0014, // SCALE_M
    0x0340, 0x0557, // FRAME_LENGTH_LINES -- modified for 30FPS max
    0x0342, 0x124C, // LINE_LENGTH_PCK
    0x0202, 0x0557, // COARSE_INTEGRATION_TIME -- modified for 33.3ms of exposure time
    0x3014, 0x0998, // FINE_INTEGRATION_TIME
    0x3010, 0x0130, // FINE_CORRECTION
#endif
};

ISP_UINT16 SNR_AR0832E_Reg_2600x1952_18P_Customer[] =  // TBD, frame rate should be adjusted to 30fps later
{
    0x0344, 0x0150,//0x0008, // X_ADDR_START
    0x0348, 0x0B77,//0x0CC7, // X_ADDR_END
    0x0346, 0x00FC,//0x0008, // Y_ADDR_START
    0x034A, 0x089B,//0x0997, // Y_ADDR_END
    0x034C, 0x0A28,//0x0CC0, // X_OUTPUT_SIZE
    0x034E, 0x07A0,//0x0990, // Y_OUTPUT_SIZE

    0x306E, 0xFC80, // DATAPATH_SELECT
    0x3040, 0x4041, // READ_MODE
    0x3178, 0x0000, // ANALOG_CONTROL5
    0x0400, 0x0000, // SCALING_MODE
    0x0404, 0x0010, // SCALE_M
    0x0342, 0x134C, // LINE_LENGTH_PCK
    0x0340, 0x0847,//0x0A1F, // FRAME_LENGTH_LINES
    0x0202, 0x0847,//0x0A1F, // COARSE_INTEGRATION_TIME
    0x3014, 0x03F6, // FINE_INTEGRATION_TIME
    0x3010, 0x0078, // FINE_CORRECTION
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

    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
    RTNA_DBG_Str(0, "SNR_Cust_InitConfig AR0832E Parallel\r\n");
    #elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
    RTNA_DBG_Str(0, "SNR_Cust_InitConfig AR0832E MIPI 1-lane\r\n");
    #elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    RTNA_DBG_Str(0, "SNR_Cust_InitConfig AR0832E MIPI 2-lane\r\n");
    #endif

    // Init OPR Table
    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
    #error "SENSOR_IF_AR0832E_PARALLEL_TBD"
    #elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
    #error "SENSOR_IF_AR0832E_MIPI_1_LANE_TBD"
    #elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    SensorCustFunc.OprTable->usInitSize                             = (sizeof(SNR_AR0832E_Reg_Init_Customer)/sizeof(SNR_AR0832E_Reg_Init_Customer[0]))/2;
    SensorCustFunc.OprTable->uspInitTable                           = &SNR_AR0832E_Reg_Init_Customer[0];

    SensorCustFunc.OprTable->bBinTableExist                         = MMP_FALSE;
    SensorCustFunc.OprTable->bInitDoneTableExist                    = MMP_FALSE;

    for (i = 0; i < MAX_SENSOR_RES_MODE; i++)
    {
        SensorCustFunc.OprTable->usSize[i]                          = (sizeof(SNR_AR0832E_Reg_Unsupport)/sizeof(SNR_AR0832E_Reg_Unsupport[0]))/2;
        SensorCustFunc.OprTable->uspTable[i]                        = &SNR_AR0832E_Reg_Unsupport[0];
    }

    // 16:9
	SensorCustFunc.OprTable->usSize[RES_IDX_2304x1296_30P]          = (sizeof(SNR_AR0832E_Reg_2312x1304_30P_Customer)/sizeof(SNR_AR0832E_Reg_2312x1304_30P_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_2304x1296_30P]        = &SNR_AR0832E_Reg_2312x1304_30P_Customer[0];
	SensorCustFunc.OprTable->usSize[RES_IDX_1280x720_30P]           = (sizeof(SNR_AR0832E_Reg_1296x732_30P_Customer)/sizeof(SNR_AR0832E_Reg_1296x732_30P_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_1280x720_30P]         = &SNR_AR0832E_Reg_1296x732_30P_Customer[0];

    // 4:3
	SensorCustFunc.OprTable->usSize[RES_IDX_2592x1944_18P]          = (sizeof(SNR_AR0832E_Reg_2600x1952_18P_Customer)/sizeof(SNR_AR0832E_Reg_2600x1952_18P_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_2592x1944_18P]        = &SNR_AR0832E_Reg_2600x1952_18P_Customer[0];
    #else
    #error "SENSOR_IF_AR0832E_ERROR"
    #endif

    // Init Vif Setting : Common
    SensorCustFunc.VifSetting->SnrType                              = MMPF_VIF_SNR_TYPE_BAYER;
    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
    SensorCustFunc.VifSetting->OutInterface                         = MMPF_VIF_IF_PARALLEL;
    #elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
    SensorCustFunc.VifSetting->OutInterface                         = MMPF_VIF_IF_MIPI_SINGLE_0;
    #elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    #if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1) // MIPI 2-lane
    SensorCustFunc.VifSetting->OutInterface                         = MMPF_VIF_IF_MIPI_DUAL_12;
    #else
    SensorCustFunc.VifSetting->OutInterface                         = MMPF_VIF_IF_MIPI_DUAL_01;
    #endif
    #elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
    SensorCustFunc.VifSetting->OutInterface                         = MMPF_VIF_IF_MIPI_QUAD;
    #endif
    #if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1) // MIPI 2-lane
    SensorCustFunc.VifSetting->VifPadId                             = MMPF_VIF_MDL_ID1;
    #else
	SensorCustFunc.VifSetting->VifPadId                             = MMPF_VIF_MDL_ID0;
    #endif

    // Init Vif Setting : PowerOn Setting
    SensorCustFunc.VifSetting->powerOnSetting.bTurnOnExtPower       = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.usExtPowerPin         = SENSOR_GPIO_ENABLE; // maybe defined in Config_SDK_xxx.h
    SensorCustFunc.VifSetting->powerOnSetting.bExtPowerPinHigh      = SENSOR_GPIO_ENABLE_ACT_LEVEL; // maybe defined in Config_SDK_xxx.h
    SensorCustFunc.VifSetting->powerOnSetting.usExtPowerPinDelay    = 50;
    SensorCustFunc.VifSetting->powerOnSetting.bFirstEnPinHigh       = (SENSOR_GPIO_ENABLE_ACT_LEVEL == GPIO_HIGH) ? MMP_TRUE : MMP_FALSE;
    SensorCustFunc.VifSetting->powerOnSetting.ubFirstEnPinDelay     = 1;
    SensorCustFunc.VifSetting->powerOnSetting.bNextEnPinHigh        = (SENSOR_GPIO_ENABLE_ACT_LEVEL == GPIO_HIGH) ? MMP_FALSE : MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.ubNextEnPinDelay      = 10;
    SensorCustFunc.VifSetting->powerOnSetting.bTurnOnClockBeforeRst = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.bFirstRstPinHigh      = (SENSOR_RESET_ACT_LEVEL == GPIO_HIGH) ? MMP_TRUE : MMP_FALSE;
    SensorCustFunc.VifSetting->powerOnSetting.ubFirstRstPinDelay    = 10;
    SensorCustFunc.VifSetting->powerOnSetting.bNextRstPinHigh       = (SENSOR_RESET_ACT_LEVEL == GPIO_HIGH) ? MMP_FALSE : MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.ubNextRstPinDelay     = 10;
    
    // Init Vif Setting : PowerOff Setting
    SensorCustFunc.VifSetting->powerOffSetting.bEnterStandByMode    = MMP_FALSE;
    SensorCustFunc.VifSetting->powerOffSetting.usStandByModeReg     = 0x301A;
    SensorCustFunc.VifSetting->powerOffSetting.usStandByModeMask    = 0x04;
    SensorCustFunc.VifSetting->powerOffSetting.bEnPinHigh           = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOffSetting.ubEnPinDelay         = 20;
    SensorCustFunc.VifSetting->powerOffSetting.bTurnOffMClock       = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOffSetting.bTurnOffExtPower     = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOffSetting.usExtPowerPin        = SENSOR_GPIO_ENABLE; // maybe defined in Config_SDK_xxx.h
    
    // Init Vif Setting : Sensor MClock Setting
    SensorCustFunc.VifSetting->clockAttr.bClkOutEn                  = MMP_TRUE;
    SensorCustFunc.VifSetting->clockAttr.ubClkFreqDiv               = 0;
    SensorCustFunc.VifSetting->clockAttr.ulMClkFreq                 = 24000;
    SensorCustFunc.VifSetting->clockAttr.ulDesiredFreq              = 24000;
    SensorCustFunc.VifSetting->clockAttr.ubClkPhase                 = MMPF_VIF_SNR_PHASE_DELAY_NONE;
    SensorCustFunc.VifSetting->clockAttr.ubClkPolarity              = MMPF_VIF_SNR_CLK_POLARITY_POS;
    SensorCustFunc.VifSetting->clockAttr.ubClkSrc					= MMPF_VIF_SNR_CLK_SRC_PMCLK; 
    
    // Init Vif Setting : Parallel Sensor Setting
    SensorCustFunc.VifSetting->paralAttr.ubLatchTiming              = MMPF_VIF_SNR_LATCH_POS_EDGE;
    SensorCustFunc.VifSetting->paralAttr.ubHsyncPolarity            = MMPF_VIF_SNR_CLK_POLARITY_POS;
    SensorCustFunc.VifSetting->paralAttr.ubVsyncPolarity            = MMPF_VIF_SNR_CLK_POLARITY_POS;
    SensorCustFunc.VifSetting->paralAttr.ubBusBitMode               = MMPF_VIF_SNR_PARAL_BITMODE_16;
    
    // Init Vif Setting : MIPI Sensor Setting
    SensorCustFunc.VifSetting->mipiAttr.bClkDelayEn                 = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bClkLaneSwapEn              = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.usClkDelay                  = 0;
    SensorCustFunc.VifSetting->mipiAttr.ubBClkLatchTiming           = MMPF_VIF_SNR_LATCH_NEG_EDGE;
    #if (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[0]              = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[1]              = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[2]              = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[3]              = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[0]             = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[1]             = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[2]             = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[3]             = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[0]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[1]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[2]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[3]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0]            = MMPF_VIF_MIPI_DATA_SRC_PHY_0;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[2]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[3]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[0]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[1]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[2]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[3]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[0]             = 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[1]             = 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[2]             = 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[3]             = 0x1F;
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
    #if (PRM_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1) // MIPI 2-lane
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0]            = MMPF_VIF_MIPI_DATA_SRC_PHY_1;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1]            = MMPF_VIF_MIPI_DATA_SRC_PHY_2;
    #else
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0]            = MMPF_VIF_MIPI_DATA_SRC_PHY_0;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1]            = MMPF_VIF_MIPI_DATA_SRC_PHY_1;
    #endif
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[2]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[3]            = MMPF_VIF_MIPI_DATA_SRC_PHY_UNDEF;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[0]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[1]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[2]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.usDataDelay[3]              = 0;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[0]             = 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[1]             = 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[2]             = 0x1F;
    SensorCustFunc.VifSetting->mipiAttr.ubDataSotCnt[3]             = 0x1F;
    #endif

    // Init Vif Setting : Color ID Setting
    SensorCustFunc.VifSetting->colorId.VifColorId              	    = MMPF_VIF_COLORID_00;
	SensorCustFunc.VifSetting->colorId.CustomColorId.bUseCustomId   = MMP_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
#if 1 // TBD, modified from other project codebase and should be confirmed with ISP Team later
    ISP_UINT32 again;
    ISP_UINT32 s_gain;

    switch (ulFrameCnt % SensorCustFunc.pAeTime->ubPeriod)
    {
        case 0:
            ISP_IF_AE_Execute();
            again = ISP_MAX(ISP_IF_AE_GetGain(), ISP_IF_AE_GetGainBase());
            //s_gain = again * dgain / ISP_IF_AE_GetDGainBase();
            s_gain = again;// * dgain / ISP_IF_AE_GetDGainBase();
            gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, ISP_IF_AE_GetShutter(), ISP_IF_AE_GetVsync());
            gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, s_gain);
            break;
    }
#endif
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
#if 1 // TBD, modified from other project codebase and should be confirmed with ISP Team later
    ISP_IF_AE_GetHWAcc(1);

    switch (ulFrameCnt++ % SensorCustFunc.pAeTime->ubPeriod)
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            //ISP_IF_AE_GetHWAcc(1);
            break;
    }
#endif
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAWB_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
#if 1 // TBD, modified from other project codebase and should be confirmed with ISP Team later
    switch (ulFrameCnt % SensorCustFunc.pAeTime->ubPeriod)
    {
        case 3:
            //ISP_IF_R_DoAWB();
            ISP_IF_AWB_Execute();
            ISP_IF_IQ_SetAWBGains(ISP_IF_AWB_GetGainR(), ISP_IF_AWB_GetGainG(), ISP_IF_AWB_GetGainB(), ISP_IF_AWB_GetGainBase());
            ISP_IF_CALI_Execute();
        break;
    }
#endif
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAWB_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_DoAWB_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
	// TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoIQ
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_DoIQ(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetGain
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetGain(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain)
{
#if (ISP_EN)
#if 1 // TBD, modified from other project codebase and should be confirmed with ISP Team later
    ISP_UINT16 sensor_again;
    ISP_UINT16 sensor_dgain;

    ulGain = ISP_MIN(ulGain * 0x40 / ISP_IF_AE_GetGainBase(), 0x3FF); // 0x40 = 1x, for sensor min_gain limit 1.23x 79/64 = 1.235

    // Sensor Gain Mapping
    if (ulGain < 0x80)
    {
        // < 2X
        sensor_dgain = ulGain;
        sensor_again = 0x0000;
    }
    else if (ulGain < 0x100)
    {
        // < 4X
        sensor_dgain = ulGain >> 1;
        sensor_again = 0x0080;
    }
    else if (ulGain < 0x200)
    {
        // 4X ~ 8X (AG = 4x)
        sensor_dgain = ulGain >> 2;
        sensor_again = 0x0480;
    }
    else
    {
        // 8X ~16X (AG = 8x)
        sensor_dgain = ulGain >> 3;
        sensor_again = 0x0680;
    }

    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x305E, 0x1000 | sensor_again | sensor_dgain);
#endif
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
#if (ISP_EN)
#if 1 // TBD, modified from other project codebase and should be confirmed with ISP Team later
    ISP_UINT32 new_vsync;
    ISP_UINT32 new_shutter;

    vsync   = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetVsync() / ISP_IF_AE_GetVsyncBase();
    shutter = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetShutter() / ISP_IF_AE_GetShutterBase();
    //new_vsync   = ISP_MAX(vsync, shutter + 8);
    new_vsync   = ISP_MAX(vsync, shutter + 5);// + 8);
    new_shutter = ISP_MIN(ISP_MAX(shutter, 1), new_vsync - 5);

    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x0340, new_vsync);
    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel, 0x0202, new_shutter);
#endif
#endif
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

#endif // (BIND_SENSOR_AR0832E)
#endif // (SENSOR_EN)
