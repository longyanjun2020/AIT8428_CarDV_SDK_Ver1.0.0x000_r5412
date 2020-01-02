//==============================================================================
//
//  File        : sensor_imx326.c
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

#include "config_fw.h"
#include "includes_fw.h"
#include "customer_config.h"

#if (SENSOR_EN)
#if (BIND_SENSOR_IMX326)

#include "mmpf_sensor.h"
#include "isp_if.h"
#include "mmp_i2c_inc.h"
#include "mmpf_vif.h"
#include "Sensor_Mod_Remapping.h"

#define DATA_LANE_FREQUENCY_720MBPS  (0) //0: 576 Mbps, 1: 720 Mbps. We prefer slower frequency

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================

MMPF_SENSOR_RESOLUTION m_SensorRes = 
{
    3,                      // ubSensorModeNum
    1,                      // ubDefPreviewMode
    2,                      // ubDefCaptureMode
    1620,                   // usPixelSize
//  Mode0       Mode1      Mode2
//  1736@30P    2160@30P   1944@30P
    {8,         8,         1       },  // usVifGrabStX
    {223,       12,        1       },  // usVifGrabStY
    {3080,      3080,      2600    },  // usVifGrabW
    {1736,      2168,      1952    },  // usVifGrabH
    #if (CHIP == MCR_V2)
    {1,         1,         1       },  // usBayerInGrabX
    {1,         1,         1       },  // usBayerInGrabY
    {8,         8,         8       },  // usBayerInDummyX
    {8,         8,         8       },  // usBayerInDummyY
    {3040,      2560,      2560    },  // usBayerOutW
    {1648,      1440,      1440    },  // usBayerOutH
    #endif
    {3040,      2560,      2560    },  // usScalInputW
    {1648,      1440,      1440    },  // usScalInputH
    {300,       300,       300     },  // usTargetFpsx10
    #if (DATA_LANE_FREQUENCY_720MBPS == 1)
    {4464,      4464,      5460    },  // usVsyncLine
    #else
    {4464,      4620,      5460    },  // usVsyncLine
    #endif
    {1,         1,         1       },  // ubWBinningN
    {1,         1,         1       },  // ubWBinningM
    {1,         1,         1       },  // ubHBinningN
    {1,         1,         1       },  // ubHBinningM
    {0xFF,     	0xFF,      0xFF    },  // ubCustIQmode
    {0xFF,     	0xFF,      0xFF    },  // ubCustAEmode
};

// OPR Table and Vif Setting
MMPF_SENSOR_OPR_TABLE   m_OprTable;
MMPF_SENSOR_VIF_SETTING m_VifSetting;

// IQ Table
const ISP_UINT8 Sensor_IQ_CompressedText[] = 
{
    #if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC)
    //#include "isp_8428_iq_data_v3_IMX326_ezmode.xls.ciq.txt"
    #include "isp_8428_iq_data_v2_IMX326.xls.ciq.txt"
    #else // Use old IQ table
    #ifdef CUS_ISP_8428_IQ_DATA // maybe defined in project MCP or Config_SDK_xxx.h
    #include CUS_ISP_8428_IQ_DATA
    #else
    #include "isp_8428_iq_data_v2_IMX326.xls.ciq.txt"
    #endif
    #endif
};

#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC)
// Replace it by custom IQ table.
const __align(4) ISP_UINT8 Sensor_EZ_IQ_CompressedText[] = 
{
    //#include "eziq_8428_IMX326.txt"
    NULL
};

ISP_UINT32 eziqsize = sizeof(Sensor_EZ_IQ_CompressedText);
#endif

// I2cm Attribute
static MMP_I2CM_ATTR m_I2cmAttr = 
{
    MMP_I2CM0,                  // i2cmID
    SENSOR_I2C_ADDR_IMX326,     // ubSlaveAddr
    16,                         // ubRegLen
    8,                          // ubDataLen
    0,                          // ubDelayTime
    MMP_FALSE,                  // bDelayWaitEn
    MMP_TRUE,                   // bInputFilterEn
    MMP_FALSE,                  // b10BitModeEn
    MMP_FALSE,                  // bClkStretchEn
    0,                          // ubSlaveAddr1
    0,                          // ubDelayCycle
    0,                          // ubPadNum
    400000,                     // ulI2cmSpeed 400KHZ
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
    6,  /* ubPeriod */
    1,  /* ubDoAWBFrmCnt */
    3   /* ubDoCaliFrmCnt */
};

MMPF_SENSOR_AETIMIMG m_AeTime = 
{
    4,  /* ubPeriod */
    0,  /* ubFrmStSetShutFrmCnt */
    0   /* ubFrmStSetGainFrmCnt */
};

MMPF_SENSOR_AFTIMIMG m_AfTime = 
{
    1,  /* ubPeriod */
    0   /* ubDoAFFrmCnt */
};

// IQ Data
#define ISP_IQ_DATA_NAME "isp_8428_iq_data_v2_IMX326.xls.ciq.txt"

static const MMP_UBYTE s_IqCompressData[] = 
{
    #include ISP_IQ_DATA_NAME
};

#define MAX_SENSOR_GAIN     (22)

static ISP_UINT32 isp_dgain, dGainBase = 0x200, s_gain;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if 0
void ____Sensor_Init_OPR_Table____(){ruturn;} // dummy
#endif

#if (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
ISP_UINT16 SNR_IMX326_Reg_Init_Customer[] = 
{
    SENSOR_DELAY_REG, 100
};
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
ISP_UINT16 SNR_IMX326_Reg_Init_Customer[] = 
{
    #if (DATA_LANE_FREQUENCY_720MBPS == 1) // 720 Mbps Data frequency

    0x3000, 0x12,
    0x3120, 0xF0,
    0x3121, 0x00,
    0x3122, 0x02,
    0x3123, 0x01,
    0x3129, 0x9C,
    0x312A, 0x02,
    0x312D, 0x02,
    0x3AC4, 0x01,
    0x310B, 0x00,
    0x304C, 0x00,
    0x304D, 0x03,
    0x331C, 0x1A,
    0x331D, 0x00,
    0x3502, 0x02,
    0x3529, 0x0E,
    0x352A, 0x0E,
    0x352B, 0x0E,
    0x3538, 0x0E,
    0x3539, 0x0E,
    0x3553, 0x00,
    0x357D, 0x05,
    0x357F, 0x05,
    0x3581, 0x04,
    0x3583, 0x76,
    0x3587, 0x01,
    0x35BB, 0x0E,
    0x35BC, 0x0E,
    0x35BD, 0x0E,
    0x35BE, 0x0E,
    0x35BF, 0x0E,
    0x366E, 0x00,
    0x366F, 0x00,
    0x3670, 0x00,
    0x3671, 0x00,
    0x30EE, 0x01,
    0x3304, 0x32,
    0x3305, 0x00,
    0x3306, 0x32,
    0x3307, 0x00,
    0x3590, 0x32,
    0x3591, 0x00,
    0x3686, 0x32,
    0x3687, 0x00,
    0x3134, 0x77,
    0x3135, 0x00,
    0x3136, 0x67,
    0x3137, 0x00,
    0x3138, 0x37,
    0x3139, 0x00,
    0x313A, 0x37,
    0x313B, 0x00,
    0x313C, 0x37,
    0x313D, 0x00,
    0x313E, 0xDF,
    0x313F, 0x00,
    0x3140, 0x37,
    0x3141, 0x00,
    0x3142, 0x2F,
    0x3143, 0x00,
    0x3144, 0x0F,
    0x3145, 0x00,
    0x3A86, 0x47,
    0x3A87, 0x00,
    SENSOR_DELAY_REG,10,
    0x3000, 0x00,
    0x303E, 0x02,
    SENSOR_DELAY_REG,7,
    0x30F4, 0x00,
    0x3018, 0xA2

    #else  // 576 Mbps Data frequency

    0x3000, 0x12,
    0x3120, 0xC0, //0xF0,
    0x3121, 0x00,
    0x3122, 0x02,
    0x3123, 0x01,
    0x3129, 0x9C,
    0x312A, 0x02,
    0x312D, 0x02,
    0x3AC4, 0x01,
    0x310B, 0x00,
    0x304C, 0x00,
    0x304D, 0x03,
    0x331C, 0x1A,
    0x331D, 0x00,
    0x3502, 0x02,
    0x3529, 0x0E,
    0x352A, 0x0E,
    0x352B, 0x0E,
    0x3538, 0x0E,
    0x3539, 0x0E,
    0x3553, 0x00,
    0x357D, 0x05,
    0x357F, 0x05,
    0x3581, 0x04,
    0x3583, 0x76,
    0x3587, 0x01,
    0x35BB, 0x0E,
    0x35BC, 0x0E,
    0x35BD, 0x0E,
    0x35BE, 0x0E,
    0x35BF, 0x0E,
    0x366E, 0x00,
    0x366F, 0x00,
    0x3670, 0x00,
    0x3671, 0x00,
    0x30EE, 0x01,
    0x3304, 0x32,
    0x3305, 0x00,
    0x3306, 0x32,
    0x3307, 0x00,
    0x3590, 0x32,
    0x3591, 0x00,
    0x3686, 0x32,
    0x3687, 0x00,
    0x3134, 0x5F, //0x77,
    0x3135, 0x00,
    0x3136, 0x47, //0x67,
    0x3137, 0x00,
    0x3138, 0x27, //0x37,
    0x3139, 0x00,
    0x313A, 0x27, //0x37,
    0x313B, 0x00,
    0x313C, 0x27, //0x37,
    0x313D, 0x00,
    0x313E, 0x97, //0xDF,
    0x313F, 0x00,
    0x3140, 0x27, //0x37,
    0x3141, 0x00,
    0x3142, 0x1F, //0x2F,
    0x3143, 0x00,
    0x3144, 0x0F,
    0x3145, 0x00,
    0x3A86, 0x47,
    0x3A87, 0x00,
    SENSOR_DELAY_REG,10,
    0x3000, 0x00,
    0x303E, 0x03, //0x02,
    SENSOR_DELAY_REG,7,
    0x30F4, 0x00,
    0x3018, 0xA2

    #endif
};
#endif

#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} // dummy
#endif

ISP_UINT16 SNR_IMX326_Reg_Unsupport[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};

#if (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
ISP_UINT16 SNR_IMX326_Reg_3080x1736_30P_Customer[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
ISP_UINT16 SNR_IMX326_Reg_3080x1736_30P_Customer[] = 
{
    #if 1 // from codebase AIT8589 (KT/STB) @ 20161101

    /* mode 1 3072x2160 */
    0x3004, 0x01,
    0x3005, 0x01,
    0x3006, 0x00,
    0x3007, 0x02,

    #if 1
    0x3037, 0x01,
    0x3038, 0x80,
    0x3039, 0x01,
    0x303A, 0x98,
    0x303B, 0x0D,

    0x306B, 0x05,
    0x30DD, 0x00,
    0x30DE, 0x00,
    0x30DF, 0x00,

    0x30E0, 0x00,
    0x30E1, 0x00,
    #endif

    0x30E2, 0x01,
    0x30EE, 0x01,
    0x30F6, 0x1A,
    0x30F7, 0x02,
    0x30F8, 0x70,
    0x30F9, 0x11,
    0x30FA, 0x00,
    0x3130, 0x86,
    0x3131, 0x08,
    0x3132, 0x7E,
    0x3133, 0x08,

    0x3342, 0x0A,
    0x3343, 0x00,
    0x3344, 0x16,
    0x3345, 0x00,
    0x33A6, 0x01,
    0x3528, 0x0E,
    0x3554, 0x1F,
    0x3555, 0x01,
    0x3556, 0x01,
    0x3557, 0x01,
    0x3558, 0x01,
    0x3559, 0x00,
    0x355A, 0x00,
    0x35BA, 0x0E,
    0x366A, 0x1B,
    0x366B, 0x1A,
    0x366C, 0x19,
    0x366D, 0x17,

    0x3A41, 0x08

    #else

    SENSOR_DELAY_REG, 100 // delay

    #endif
};
#endif

#if (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
ISP_UINT16 SNR_IMX326_Reg_3080x2168_30P_Customer[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
ISP_UINT16 SNR_IMX326_Reg_3080x2168_30P_Customer[] = 
{
    #if (DATA_LANE_FREQUENCY_720MBPS == 1)   // 720 Mbps Data frequency

   /* mode 1 3072x2160 */
    0x3004, 0x01,
    0x3005, 0x01,
    0x3006, 0x00,
    0x3007, 0x02,

    #if 1
    0x3037, 0x01,
    0x3038, 0x80,
    0x3039, 0x01,
    0x303A, 0x98,
    0x303B, 0x0D,

    0x306B, 0x05,
    0x30DD, 0x00,
    0x30DE, 0x00,
    0x30DF, 0x00,

    0x30E0, 0x00,
    0x30E1, 0x00,
    #endif

    0x30E2, 0x01,
    0x30EE, 0x01,

    0x30F6, 0x1A,
    0x30F7, 0x02,
    0x30F8, 0x70,
    0x30F9, 0x11,
    0x30FA, 0x00,
    0x3130, 0x86,
    0x3131, 0x08,
    0x3132, 0x7E,
    0x3133, 0x08,

    0x3342, 0x0A,
    0x3343, 0x00,
    0x3344, 0x16,
    0x3345, 0x00,
    0x33A6, 0x01,
    0x3528, 0x0E,
    0x3554, 0x1F,
    0x3555, 0x01,
    0x3556, 0x01,
    0x3557, 0x01,
    0x3558, 0x01,
    0x3559, 0x00,
    0x355A, 0x00,
    0x35BA, 0x0E,
    0x366A, 0x1B,
    0x366B, 0x1A,
    0x366C, 0x19,
    0x366D, 0x17,

    0x3A41, 0x08

    #else   // 576 Mbps Data frequency

    0x3004, 0x01,
    0x3005, 0x01,
    0x3006, 0x00,
    0x3007, 0x02,

    #if 1
    0x3037, 0x01,
    0x3038, 0x80,
    0x3039, 0x01,
    0x303A, 0x98,
    0x303B, 0x0D,

    0x306B, 0x05,
    0x30DD, 0x00,
    0x30DE, 0x00,
    0x30DF, 0x00,

    0x30E0, 0x00,
    0x30E1, 0x00,
    #endif

    0x30E2, 0x01,
    0x30EE, 0x01,

    0x30F6, 0x08, //0x1A,
    0x30F7, 0x02,
    0x30F8, 0x0B, //0x70,
    0x30F9, 0x12, //0x11,
    0x30FA, 0x00,
    0x3130, 0x86,
    0x3131, 0x08,
    0x3132, 0x7E,
    0x3133, 0x08,

    0x3342, 0x0A,
    0x3343, 0x00,
    0x3344, 0x16,
    0x3345, 0x00,
    0x33A6, 0x01,
    0x3528, 0x0E,
    0x3554, 0x1F,
    0x3555, 0x01,
    0x3556, 0x01,
    0x3557, 0x01,
    0x3558, 0x01,
    0x3559, 0x00,
    0x355A, 0x00,
    0x35BA, 0x0E,
    0x366A, 0x1B,
    0x366B, 0x1A,
    0x366C, 0x19,
    0x366D, 0x17,

    0x3A41, 0x08

    #endif
};
#endif

#if (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
ISP_UINT16 SNR_IMX326_Reg_2600x1952_30P_Customer[] = 
{
    SENSOR_DELAY_REG, 100 // delay
};
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
ISP_UINT16 SNR_IMX326_Reg_2600x1952_30P_Customer[] = 
{
    #if 0 // from vendor @ 20170118
/*
 * Mstar_IMX326_SettingRegisters_720M__Mode3_170118.xlsx
 *
 * INCK Frequency [MHz]     : 24MHz
 * Lane number              : 4
 * Output Frequency [Mbps]  : 720
 *
 * Mode No.                 : mode3
 * Sensor Mode Name         : All-pixel crop scan 4:3 mode (10-bit)
 * View Angle Mode          : 2592x1944
 * Interface                : CSI-2 4-lane
 * HMAX [DEC]               : 350
 * VMAX [DEC]               : 6864
 * Frame rate               : 29.97
 */

    0x3004, 0x01,
    0x3005, 0x01,
    0x3006, 0x00,
    0x3007, 0xA2/*0x02*/,
    
    0x3037, 0x01,
    0x3038, 0x70/*0x80*/,
    0x3039, 0x02/*0x01*/,
    0x303A, 0xA8/*0x98*/,
    0x303B, 0x0C/*0x0D*/,

    0x306B, 0x05,

    0x30DD, 0x01/*0x00*/,
    0x30DE, 0x6C/*0x00*/,
    0x30DF, 0x00,

    0x30E0, 0x36/*0x00*/,
    0x30E1, 0x00,
    0x30E2, 0x01,

    0x30EE, 0x01,

    0x30F6, 0x5E,
    0x30F7, 0x01,
    0x30F8, 0xCF,
    0x30F9, 0x1A,
    0x30FA, 0x00,
    0x3130, 0xAE,
    0x3131, 0x07,
    0x3132, 0xA6,
    0x3133, 0x07,

    0x3342, 0x0A,
    0x3343, 0x00,
    0x3344, 0x16,
    0x3345, 0x00,
    0x33A6, 0x01,
    0x3528, 0x0E,
    0x3554, 0x1F,
    0x3555, 0x01,
    0x3556, 0x01,
    0x3557, 0x01,
    0x3558, 0x01,
    0x3559, 0x00,
    0x355A, 0x00,
    0x35BA, 0x0E,
    0x366A, 0x1B,
    0x366B, 0x1A,
    0x366C, 0x19,
    0x366D, 0x17,

    0x3A41, 0x08 // [5:0] MDSEL5
    
    #else // from vendor @ 20170117

/*
 * Mstar_IMX326_SettingRegisters_576M_Mode3_170719.xlsx
 *
 * INCK Frequency [MHz]     : 24MHz
 * Lane number              : 4
 * Output Frequency [Mbps]  : 576
 *
 * Mode No.                 : mode1
 * Sensor Mode Name         : All-pixel scan mode (10-bit)
 * View Angle Mode          : 2592x1944
 * Interface                : CSI-2 4-lane
 * HMAX [DEC]               : 440
 * VMAX [DEC]               : 5460
 * Frame rate               : 29.97
 */

    0x3004, 0x01,
    0x3005, 0x01,
    0x3006, 0x00,
    0x3007, 0xA2, //0x02,
    
    0x3037, 0x01,
    0x3038, 0x70, //0x80,
    0x3039, 0x02, //0x01,
    0x303A, 0xA8, //0x98,
    0x303B, 0x0C, //0x0D,

    0x306B, 0x05,

    0x30DD, 0x01, //0x00,
    0x30DE, 0x6C, //0x00,
    0x30DF, 0x00,

    0x30E0, 0x36, //0x00,
    0x30E1, 0x00,
    0x30E2, 0x01,

    0x30EE, 0x01,

    0x30F6, 0xB8, //0xA0/*0x5E*/,
    0x30F7, 0x01,
    0x30F8, 0x53, //0x8E/*0xCF*/,
    0x30F9, 0x15, //0x16/*0x1A*/,
    0x30FA, 0x00,
    0x3130, 0xAE, //0xD6/*0xAE*/,
    0x3131, 0x07, //0x06/*0x07*/,
    0x3132, 0xA6, //0xCE/*0xA6*/,
    0x3133, 0x07, //0x06/*0x07*/,

    0x3342, 0x0A,
    0x3343, 0x00,
    0x3344, 0x16,
    0x3345, 0x00,
    0x33A6, 0x01,
    0x3528, 0x0E,
    0x3554, 0x1F,
    0x3555, 0x01,
    0x3556, 0x01,
    0x3557, 0x01,
    0x3558, 0x01,
    0x3559, 0x00,
    0x355A, 0x00,
    0x35BA, 0x0E,
    0x366A, 0x1B,
    0x366B, 0x1A,
    0x366C, 0x19,
    0x366D, 0x17,

    0x3A41, 0x08 // [5:0] MDSEL5

    #endif
};
#endif

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

    #if (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    RTNA_DBG_Str(0, "SNR_Cust_InitConfig IMX326 MIPI 2-lane\r\n");
    #elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
    RTNA_DBG_Str(0, "SNR_Cust_InitConfig IMX326 MIPI 4-lane\r\n");
    #endif

    // Init OPR Table
    #if (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
    #error "SENSOR_IF_MIPI_2_LANE TBD"
    #elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
    SensorCustFunc.OprTable->usInitSize                             = (sizeof(SNR_IMX326_Reg_Init_Customer)/sizeof(SNR_IMX326_Reg_Init_Customer[0]))/2;
    SensorCustFunc.OprTable->uspInitTable                           = &SNR_IMX326_Reg_Init_Customer[0];

    SensorCustFunc.OprTable->bBinTableExist                         = MMP_FALSE;
    SensorCustFunc.OprTable->bInitDoneTableExist                    = MMP_FALSE;

    for (i = 0; i < MAX_SENSOR_RES_MODE; i++)
    {
        SensorCustFunc.OprTable->usSize[i]                          = (sizeof(SNR_IMX326_Reg_Unsupport)/sizeof(SNR_IMX326_Reg_Unsupport[0]))/2;
        SensorCustFunc.OprTable->uspTable[i]                        = &SNR_IMX326_Reg_Unsupport[0];
    }

    // 16:9
	SensorCustFunc.OprTable->usSize[RES_IDX_3080x1736_30P]          = (sizeof(SNR_IMX326_Reg_3080x1736_30P_Customer)/sizeof(SNR_IMX326_Reg_3080x1736_30P_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_3080x1736_30P]        = &SNR_IMX326_Reg_3080x1736_30P_Customer[0];
	SensorCustFunc.OprTable->usSize[RES_IDX_3072x2160_30P]         = (sizeof(SNR_IMX326_Reg_3080x2168_30P_Customer)/sizeof(SNR_IMX326_Reg_3080x2168_30P_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_3072x2160_30P]       = &SNR_IMX326_Reg_3080x2168_30P_Customer[0];

    // 4:3
    SensorCustFunc.OprTable->usSize[RES_IDX_2592x1944_30P]          = (sizeof(SNR_IMX326_Reg_2600x1952_30P_Customer)/sizeof(SNR_IMX326_Reg_2600x1952_30P_Customer[0]))/2;
    SensorCustFunc.OprTable->uspTable[RES_IDX_2592x1944_30P]        = &SNR_IMX326_Reg_2600x1952_30P_Customer[0];
    #else
    #error "SENSOR_IF_IMX326_ERROR"
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
    SensorCustFunc.VifSetting->powerOnSetting.usExtPowerPinDelay    = 30;
    SensorCustFunc.VifSetting->powerOnSetting.bFirstEnPinHigh       = MMP_FALSE;
    SensorCustFunc.VifSetting->powerOnSetting.ubFirstEnPinDelay     = 10;
    SensorCustFunc.VifSetting->powerOnSetting.bNextEnPinHigh        = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.ubNextEnPinDelay      = 10;
    SensorCustFunc.VifSetting->powerOnSetting.bTurnOnClockBeforeRst = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.bFirstRstPinHigh      = MMP_FALSE;
    SensorCustFunc.VifSetting->powerOnSetting.ubFirstRstPinDelay    = 10;
    SensorCustFunc.VifSetting->powerOnSetting.bNextRstPinHigh       = MMP_TRUE;
    SensorCustFunc.VifSetting->powerOnSetting.ubNextRstPinDelay     = 10;

    // Init Vif Setting : PowerOff Setting
    SensorCustFunc.VifSetting->powerOffSetting.bEnterStandByMode    = MMP_FALSE;
    SensorCustFunc.VifSetting->powerOffSetting.usStandByModeReg     = 0x3000;
    SensorCustFunc.VifSetting->powerOffSetting.usStandByModeMask    = 0x01;
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
    SensorCustFunc.VifSetting->clockAttr.ubClkPolarity              = MMPF_VIF_SNR_CLK_POLARITY_POS/*MMPF_VIF_SNR_CLK_POLARITY_NEG*/;
    SensorCustFunc.VifSetting->clockAttr.ubClkSrc                   = MMPF_VIF_SNR_CLK_SRC_PMCLK;

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
    #elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[0]              = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[1]              = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[2]              = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneEn[3]              = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[0]             = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[1]             = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[2]             = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataDelayEn[3]             = MMP_TRUE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[0]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[1]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[2]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.bDataLaneSwapEn[3]          = MMP_FALSE;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[0]            = MMPF_VIF_MIPI_DATA_SRC_PHY_0;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[1]            = MMPF_VIF_MIPI_DATA_SRC_PHY_1;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[2]            = MMPF_VIF_MIPI_DATA_SRC_PHY_2;
    SensorCustFunc.VifSetting->mipiAttr.ubDataLaneSrc[3]            = MMPF_VIF_MIPI_DATA_SRC_PHY_3;
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
    SensorCustFunc.VifSetting->colorId.VifColorId                   = MMPF_VIF_COLORID_10;
    SensorCustFunc.VifSetting->colorId.CustomColorId.bUseCustomId   = MMP_FALSE;
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    ISP_UINT32 Lux = ISP_IF_AE_GetLightCond();

    switch ((ulFrameCnt % m_AeTime.ubPeriod))
    {
        case 0:
            ISP_IF_AE_Execute();
            s_gain = ISP_MAX(ISP_IF_AE_GetGain(), ISP_IF_AE_GetGainBase());	
            if (s_gain >= ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN)
            {
                isp_dgain = dGainBase * s_gain / (ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN);
                s_gain = ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN;
            }
            else
            {
                isp_dgain = dGainBase;
            }

            gsSensorFunction->MMPF_Sensor_SetShutter(ubSnrSel, 0, 0);
            break;
        case 1:
            gsSensorFunction->MMPF_Sensor_SetGain(ubSnrSel, s_gain);
            break;
    }
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_DoAWB_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
#if (ISP_EN)
    // TBD
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
    ISP_UINT32 gain;

    gain = 0x800 - 0x800 * 0x100 / ulGain; // reg = 2048 - (2048 / Gain)
    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel,0x300A, (gain >> 0) & 0xFF);
    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel,0x300B, (gain >> 8) & 0x07);
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
void SNR_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
#if (ISP_EN)
    ISP_UINT32 new_vsync    = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetVsync() / ISP_IF_AE_GetVsyncBase();
    ISP_UINT32 new_shutter  = gSnrLineCntPerSec[ubSnrSel] * ISP_IF_AE_GetShutter() / ISP_IF_AE_GetShutterBase();

    // 1 ~ (frame length line - 6)
    if (vsync)
    {
        new_vsync = vsync ;
    }
    else
    {
        new_vsync = VR_MIN(VR_MAX(new_shutter , new_vsync), 0xFFFF);
    }
    if (shutter)
    {
        new_shutter = shutter ;
    }
    else
    {
        new_shutter = VR_MIN(VR_MAX(new_shutter, 1), new_vsync - 12);
    }

    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel,0x300C, ((new_vsync - new_shutter) >> 0) & 0xFF);
    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel,0x300D, ((new_vsync - new_shutter) >> 8) & 0xFF);

    // Vsync
    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel,0x30F8, (new_vsync >> 0 ) & 0xFF);
    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel,0x30F9, (new_vsync >> 8 ) & 0xFF);
    gsSensorFunction->MMPF_Sensor_SetReg(ubSnrSel,0x30FA, (new_vsync >> 16) & 0x0F);
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

#endif // (BIND_SENSOR_IMX326)
#endif // (SENSOR_EN)
