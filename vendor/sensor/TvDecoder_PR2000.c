//==============================================================================
//
//  File        : TvDecoder_PR2000.c
//  Description : Firmware Sensor Control File
//  Author      :
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#include "customer_config.h"
#include "snr_cfg.h"

#if (SENSOR_EN)
#if (BIND_SENSOR_PR2000)

#include "mmpf_sensor.h"
#include "Sensor_Mod_Remapping.h"
#include "isp_if.h"

#define TEST_BT601              (1)
#define TEST_BT656              (2)

#define TEST_MODE               (TEST_BT601)

#define TEST_PATTERN            (0)

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================
MMPF_SENSOR_RESOLUTION m_PR2000_SensorRes_RawStore_Mode =
{
	2,			// ubSensorModeNum
	0,			// ubDefPreviewMode
	0,			// ubDefCaptureMode
	3000,       // usPixelSize (TBD)
//  Mode0
    {1  	,1		},  // usVifGrabStX
    {1      ,1		},  // usVifGrabStY
    {2560   ,2560	},  // usVifGrabW
    {720    ,720	},  // usVifGrabH
    #if (CHIP == MCR_V2)
    {1      ,1		}, 	// usBayerInGrabX
    {1   	,1		}, 	// usBayerInGrabY
    {1     	,1		},  // usBayerInDummyX
    {1     	,1		},  // usBayerInDummyY
    {1280   ,1280	},  // usBayerOutW
    {720  	,720	},	// usBayerOutH
    #endif
    
    {1280   ,1280	},  // usScalInputW
    {720    ,720	},	// usScalInputH
   
    {300    ,250	},  // usTargetFpsx10
    {250    ,250	},  // usVsyncLine
    {1      ,1		}, 	// ubWBinningN
    {1      ,1		}, 	// ubWBinningN
    {1      ,1		}, 	// ubWBinningN
    {1      ,1		}, 	// ubWBinningN
    {0xFF   ,0xFF	},  // ubCustIQmode
    {0xFF   ,0xFF	}   // ubCustAEmode
};

MMPF_SENSOR_RESOLUTION m_PR2000_SensorRes_BypassISP_Mode =
{
	2,			// ubSensorModeNum
	1,//0,			// ubDefPreviewMode
	1,//0,			// ubDefCaptureMode
	3000,       // usPixelSize (TBD)
//  Mode0
    {1  	,1		},  // usVifGrabStX
    {1      ,1		},  // usVifGrabStY
    {1280   ,1280	},  // usVifGrabW
    {720    ,720	},  // usVifGrabH
    #if (CHIP == MCR_V2)
    {1      ,1		}, 	// usBayerInGrabX
    {1   	,1		}, 	// usBayerInGrabY
    {1     	,1		},  // usBayerInDummyX
    {1     	,1		},  // usBayerInDummyY
    {1280   ,1280	},  // usBayerOutW
    {720  	,720	},	// usBayerOutH
    #endif
    
    {1280   ,1280	},  // usScalInputW
    {720    ,720	},	// usScalInputH
   
    {300    ,250	},  // usTargetFpsx10
    {250    ,250	},  // usVsyncLine
    {1      ,1		}, 	// ubWBinningN
    {1      ,1		}, 	// ubWBinningN
    {1      ,1		}, 	// ubWBinningN
    {1      ,1		}, 	// ubWBinningN
    {0xFF   ,0xFF	},  // ubCustIQmode
    {0xFF   ,0xFF	}   // ubCustAEmode
};

// OPR Table and Vif Setting
MMPF_SENSOR_OPR_TABLE       m_PR2000_OprTable;
MMPF_SENSOR_VIF_SETTING     m_PR2000_VifSetting;

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

// I2cm Attribute
static MMP_I2CM_ATTR m_PR2000_I2cmAttr = 
{
    MMP_I2CM0,//MMP_I2CM0,  // i2cmID
    (0xB8>>1),  // ubSlaveAddr   //0xBA
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
MMPF_SENSOR_AWBTIMIMG m_PR2000_AwbTime =
{
	6,	/* ubPeriod */
	1, 	/* ubDoAWBFrmCnt */
	3	/* ubDoCaliFrmCnt */
};

MMPF_SENSOR_AETIMIMG m_PR2000_AeTime =
{
	6, 	/* ubPeriod */
	0, 	/* ubFrmStSetShutFrmCnt */
	0	/* ubFrmStSetGainFrmCnt */
};

MMPF_SENSOR_AFTIMIMG m_PR2000_AfTime =
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

ISP_UINT16 SNR_PR2000_Reg_Init_Customer[] = 
{
     SENSOR_DELAY_REG, 0 // delay
};

#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} //dummy
#endif

#if (TEST_MODE == TEST_BT601)
#if (TVDEC_SNR_IF == SENSOR_IF_MIPI_4_LANE)
ISP_UINT16 SNR_PR2000_Reg_HD_30P[] = 
{
//Camera:5(HDT_NEW), cameraResolution:4(camera_1280x720p30), videoResolution:6(video_1280x720p30), MIPI
//   [CH/IP1]:I2C SlvAddr(0x5d)
//    vinMode(0:[Differential|VinPN], 1:VinP, 3:VinN): 1
//    vidOutMode(0:pararllel, 2:mipi_2lane(~HD), 4:mipi_4lane(~FHD): 4
//    cascade(0:no, 1:cascade): 0
//    cascadeMux(If cascade, 0:single(slave), 1:mux(master)): 0
//    chid_num(If cascade, 0:master, 1:slave): 0
//    bt656(If parallel, 0:bt1120(8bit), 1:bt656(8bit)): 0
//    datarate(If parallel, 0:148.5Mhz, 1:74.25Mhz, 2:36Mhz, 3:27Mhz): 0
//    clkphase_Mux(If parallel & cascadeMux, clkphase:0~15): 0
//    clkphase_148Mhz(If parallel & datarate=0, clkphase:0~15): 0
//    clkphase_74Mhz(If parallel & datarate=1, clkphase:0~15): 0
//    clkphase_37Mhz(If parallel & datarate>1, clkphase:0~15): 0

/// slv, page, addr, data/////////////////////
//  > 0xb8, 0, 0xa1, 0xff]
//  > 0xba, 0, 0xa1, 0xff]
0xa1, 0xff,
//  > 0xbc, 0, 0xa1, 0xff]
//  > 0xbe, 0, 0xa1, 0xff]
//  > 0xb8, 0, 0xff, 0x80]
//  > 0xba, 0, 0xff, 0x80]
0xff, 0x80,
//  > 0xbc, 0, 0xff, 0x80]
//  > 0xbe, 0, 0xff, 0x80]

//Page0 sys
0xff, 0x00,

0x10, 0xda,
0x11, 0x00,
0x12, 0x00,
0x13, 0x00,
0x14, 0x21,
0x15, 0x44,
0x16, 0x0d,
0x40, 0x40,
0x41, 0x08,
0x42, 0x00,
0x43, 0x00,
0x47, 0xba,
0x48, 0x10,
0x49, 0x0a,
0x4e, 0x40,
0x80, 0x56,
0x81, 0x0e,
0x82, 0x0d,
0x84, 0x30,
0x86, 0x30,
0x87, 0x00,
0x8a, 0x00,
0x90, 0x00,
0x91, 0x00,
0x92, 0x00,
0x94, 0x00,
0x95, 0x80,
0x96, 0x00,
0xa0, 0x01,
0xa1, 0xe3,
0xa4, 0x01,
0xa5, 0xe3,
0xa6, 0x00,
0xa7, 0x12,
0xa8, 0x00,
0xd0, 0x30,
0xd1, 0x08,
0xd2, 0x21,
0xd3, 0x00,
0xd8, 0x37,
0xd9, 0x08,
0xda, 0x21,
0xe0, 0x19,
0xe1, 0x00,
0xe2, 0x18,
0xe3, 0x00,
0xe4, 0x00,
0xe5, 0x0c,
0xe6, 0x00,
0xea, 0x00,
0xeb, 0x00,
0xf1, 0x44,
0xf2, 0x01,

//Page1 vdec
0xff, 0x01,

0x00, 0xe4,
0x01, 0x61,
0x02, 0x00,
0x03, 0x56,
0x04, 0x0c,
0x05, 0x88,
0x06, 0x04,
0x07, 0xb2,
0x08, 0x44,
0x09, 0x34,
0x0a, 0x02,
0x0b, 0x14,
0x0c, 0x04,
0x0d, 0x08,
0x0e, 0x5e,
0x0f, 0x5e,
0x10, 0x26,
0x11, 0x01,
0x12, 0x45,
0x13, 0x68,
0x14, 0x00,
0x15, 0x18,
0x16, 0xd0,
0x17, 0x00,
0x18, 0x41,
0x19, 0x46,
0x1a, 0x22,
0x1b, 0x07,
0x1c, 0xe9,
0x1d, 0x45,
0x1e, 0x40,
0x1f, 0x00,
0x20, 0x80,
0x21, 0x80,
0x22, 0x80,
0x23, 0x80,
0x24, 0xc0,
0x25, 0xc0,
0x26, 0x80,
0x27, 0x80,
0x28, 0x00,
0x29, 0x7f,
0x2a, 0xff,
0x2b, 0x00,
0x2c, 0x00,
0x2d, 0x00,
0x2e, 0x02,
0x2f, 0xc0,
0x30, 0x00,
0x31, 0x00,
0x32, 0xc0,
0x33, 0x14,
0x34, 0x14,
0x35, 0x80,
0x36, 0x80,
0x37, 0x00,
0x38, 0x40,
0x39, 0x04,
0x3a, 0x07,
0x3b, 0x02,
0x3c, 0x01,
0x3d, 0x22,
0x3e, 0x01,
0x3f, 0xc4,
0x40, 0x05,
0x41, 0x11,
0x42, 0x01,
0x43, 0x34,
0x44, 0x6a,
0x45, 0x00,
0x46, 0x12,
0x47, 0x2f,
0x48, 0x24,
0x49, 0x08,
0x4a, 0x6c,
0x4b, 0x60,
0x4c, 0x00,
0x4d, 0x22,
0x4e, 0x00,
#if (TEST_PATTERN)
0x4f, 0x10,
#else
0x4f, 0x00,
#endif
0x50, 0xc6,
0x51, 0x28,
0x52, 0x40,
0x53, 0x0c,
0x54, 0x0f,
0x55, 0x8d,
0x70, 0x06,
0x71, 0x08,
0x72, 0x0a,
0x73, 0x0c,
0x74, 0x0e,
0x75, 0x10,
0x76, 0x12,
0x77, 0x14,
0x78, 0x06,
0x79, 0x08,
0x7a, 0x0a,
0x7b, 0x0c,
0x7c, 0x0e,
0x7d, 0x10,
0x7e, 0x12,
0x7f, 0x14,
0x80, 0x00,
0x81, 0x09,
0x82, 0x00,
0x83, 0x07,
0x84, 0x00,
0x85, 0x14,
0x86, 0x03,
0x87, 0x36,
0x88, 0x0a,
0x89, 0x62,
0x8a, 0x0a,
0x8b, 0x62,
0x8c, 0x0b,
0x8d, 0xe0,
0x8e, 0x06,
0x8f, 0x66,
0x90, 0x06,
0x91, 0xd1,
0x92, 0x73,
0x93, 0x39,
0x94, 0x0f,
0x95, 0x5e,
0x96, 0x09,
0x97, 0x26,
0x98, 0x1c,
0x99, 0x20,
0x9a, 0x17,
0x9b, 0x70,
0x9c, 0x0e,
0x9d, 0x10,
0x9e, 0x0b,
0x9f, 0xb8,
0xa0, 0x01,
0xa1, 0xc2,
0xa2, 0x01,
0xa3, 0xb8,
0xa4, 0x00,
0xa5, 0xe1,
0xa6, 0x00,
0xa7, 0xc6,
0xa8, 0x01,
0xa9, 0x7c,
0xaa, 0x01,
0xab, 0x7c,
0xac, 0x00,
0xad, 0xea,
0xae, 0x00,
0xaf, 0xea,
0xb0, 0x09,
0xb1, 0xaa,
0xb2, 0x0f,
0xb3, 0xae,
0xb4, 0x00,
0xb5, 0x17,
0xb6, 0x08,
0xb7, 0xe8,
0xb8, 0xb0,
0xb9, 0xce,
0xba, 0x90,
0xbb, 0x03,
0xbc, 0x00,
0xbd, 0x00,
0xbe, 0x05,
0xbf, 0x08,
0xc0, 0x00,
0xc1, 0x19,
0xc2, 0x02,
0xc3, 0xd0,
          
0x54, 0x0e,
0x54, 0x0f,

#if (TEST_PATTERN)
//Page2 for Test Pattern
0xff, 0x02,

0x80, 0xB8,
#endif
                
//mipi 4lane    
//Page0 sys
0xff, 0x00,
    
0x40, 0x00,
0x4e, 0x40,
0x40, 0x40,
0x4e, 0x60,
0x4e, 0x40,
0x47, 0xba,
};
#endif

#if (TVDEC_SNR_IF == SENSOR_IF_MIPI_2_LANE)
ISP_UINT16 SNR_PR2000_Reg_HD_30P[] = 
{
//Camera:5(HDT_NEW), cameraResolution:4(camera_1280x720p30), videoResolution:6(video_1280x720p30), MIPI
//   [CHIP1]:I2C SlvAddr(0x5d)
//    vinMode(0:[Differential|VinPN], 1:VinP, 3:VinN): 1
//    vidOutMode(0:pararllel, 2:mipi_2lane(~HD), 4:mipi_4lane(~FHD): 2
//    cascade(0:no, 1:cascade): 0
//    cascadeMux(If cascade, 0:single(slave), 1:mux(master)): 0
//    chid_num(If cascade, 0:master, 1:slave): 0
//    bt656(If parallel, 0:bt1120(8bit), 1:bt656(8bit)): 0
//    datarate(If parallel, 0:148.5Mhz, 1:74.25Mhz, 2:36Mhz, 3:27Mhz): 0
//    clkphase_Mux(If parallel & cascadeMux, clkphase:0~15): 0
//    clkphase_148Mhz(If parallel & datarate=0, clkphase:0~15): 0
//    clkphase_74Mhz(If parallel & datarate=1, clkphase:0~15): 0
//    clkphase_36Mhz(If parallel & datarate=2, clkphase:0~3): 0
//    clkphase_27Mhz(If parallel & datarate=3, clkphase:0~3): 0
///////////////////////////////////////////////
/// slv, addr, data/////////////////////

//                      If CHIP_ID(0x2000) is invalid, reload i2c slave address of all pr2000 chips like below.
//> 0xb8, 0xff, 0x00]
//> 0xb8, 0xa1, 0xff]
//> 0xba, 0xff, 0x00]
//> 0xba, 0xa1, 0xff]
//> 0xbc, 0xff, 0x00]
//> 0xbc, 0xa1, 0xff]
//> 0xbe, 0xff, 0x00]
//> 0xbe, 0xa1, 0xff]
//> 0xb8, 0xff, 0x80]
//> 0xba, 0xff, 0x80]
//> 0xbc, 0xff, 0x80]
//> 0xbe, 0xff, 0x80]

//Page0 sys
  0xff, 0x00,
  0x10, 0xda,
  0x11, 0x07,
  0x12, 0x00,
  0x13, 0x00,
  0x14, 0x21,    //b[1:0] => Select Camera Input. VinP(1), VinN(3), Differ(0).
  0x15, 0x44,
  0x16, 0x0d,
  0x40, 0x00,
  0x41, 0x08,
  0x42, 0x00,
  0x43, 0x00,
  0x47, 0x9a,
  0x48, 0x00,
  0x49, 0x0a,
  0x4e, 0x4c,
  0x80, 0x56,
  0x81, 0x0e,
  0x82, 0x0d,
  0x84, 0x30,
  0x86, 0x30,
  0x87, 0x00,
  0x8a, 0x00,
  0x90, 0x00,
  0x91, 0x00,
  0x92, 0x00,
  0x94, 0xff,
  0x95, 0xff,
  0x96, 0xff,
  
  #if 0
  0xa0, 0x01,
  0xa1, 0xe8,
  0xa4, 0x00,
  0xa5, 0x11,
  0xa6, 0x00,
  0xa7, 0x81,
  0xa8, 0x00,
  #else 
  //18V
  0xa0, 0x00,
  0xa1, 0x20,
  0xa4, 0x01,
  0xa5, 0xE3,
  0xa6, 0x00,
  0xa7, 0x12,
  0xa8, 0x00,
  #endif
  
  0xd0, 0x30,
  0xd1, 0x08,
  0xd2, 0x21,
  0xd3, 0x00,
  0xd8, 0x37,
  0xd9, 0x08,
  0xda, 0x21,
  0xe0, 0x19,
  0xe1, 0x10,
  0xe2, 0x18,
  0xe3, 0x00,
  0xe4, 0x00,
  0xe5, 0x0c,
  0xe6, 0x00,
  0xea, 0x00,
  0xeb, 0x00,
  0xf1, 0x44,
  0xf2, 0x01,

//Page1 vdec
  0xff, 0x01,
  0x00, 0xe4,
  0x01, 0x61,
  0x02, 0x00,
  0x03, 0x57,
  0x04, 0x0c,
  0x05, 0x88,
  0x06, 0x04,
  0x07, 0xb2,
  0x08, 0x44,
  0x09, 0x34,
  0x0a, 0x02,
  0x0b, 0x14,
  0x0c, 0x04,
  0x0d, 0x08,
  0x0e, 0x5e,
  0x0f, 0x5e,
  0x10, 0x26,
  0x11, 0x01,
  0x12, 0x45,
  0x13, 0x68,
  0x14, 0x00,
  0x15, 0x18,
  0x16, 0xd0,
  0x17, 0x00,
  0x18, 0x41,
  0x19, 0x46,
  0x1a, 0x22,
  0x1b, 0x07,
  0x1c, 0xe9,
  0x1d, 0x45,
  0x1e, 0x40,
  0x1f, 0x00,
  0x20, 0x80,
  0x21, 0x80,
  0x22, 0x80,
  0x23, 0x80,
  0x24, 0xc0,
  0x25, 0xc0,
  0x26, 0x80,
  0x27, 0x80,
  0x28, 0x00,
  0x29, 0x7f,
  0x2a, 0xff,
  0x2b, 0x00,
  0x2c, 0x00,
  0x2d, 0x00,
  0x2e, 0x02,
  0x2f, 0xc0,
  0x30, 0x00,
  0x31, 0x00,
  0x32, 0xc0,
  0x33, 0x14,
  0x34, 0x14,
  0x35, 0x80,
  0x36, 0x80,
  0x37, 0x00,
  0x38, 0x40,
  0x39, 0x04,
  0x3a, 0x07,
  0x3b, 0x02,
  0x3c, 0x01,
  0x3d, 0x22,
  0x3e, 0x01,
  0x3f, 0xc4,
  0x40, 0x05,
  0x41, 0x11,
  0x42, 0x01,
  0x43, 0x34,
  0x44, 0x6a,
  0x45, 0x00,
  0x46, 0x12,
  0x47, 0x2f,
  0x48, 0x24,
  0x49, 0x08,
  0x4a, 0x6c,
  0x4b, 0x60,
  0x4c, 0x00,
  0x4d, 0x22,
  0x4e, 0x00,
  #if (TEST_PATTERN)
  0x4f, 0x10,
  #else
  0x4f, 0x00,
  #endif
  0x50, 0xc6,
  0x51, 0x28,
  0x52, 0x40,
  0x53, 0x0c,
  0x54, 0x0f,
  0x55, 0x8d,
  0x70, 0x06,
  0x71, 0x08,
  0x72, 0x0a,
  0x73, 0x0c,
  0x74, 0x0e,
  0x75, 0x10,
  0x76, 0x12,
  0x77, 0x14,
  0x78, 0x06,
  0x79, 0x08,
  0x7a, 0x0a,
  0x7b, 0x0c,
  0x7c, 0x0e,
  0x7d, 0x10,
  0x7e, 0x12,
  0x7f, 0x14,
  0x80, 0x00,
  0x81, 0x09,
  0x82, 0x00,
  0x83, 0x07,
  0x84, 0x00,
  0x85, 0x14,
  0x86, 0x03,
  0x87, 0x36,
  0x88, 0x0a,
  0x89, 0x62,
  0x8a, 0x0a,
  0x8b, 0x62,
  0x8c, 0x0b,
  0x8d, 0xe0,
  0x8e, 0x06,
  0x8f, 0x66,
  0x90, 0x06,
  0x91, 0xd1,
  0x92, 0x73,
  0x93, 0x39,
  0x94, 0x0f,
  0x95, 0x5e,
  0x96, 0x09,
  0x97, 0x26,
  0x98, 0x1c,
  0x99, 0x20,
  0x9a, 0x17,
  0x9b, 0x70,
  0x9c, 0x0e,
  0x9d, 0x10,
  0x9e, 0x0b,
  0x9f, 0xb8,
  0xa0, 0x01,
  0xa1, 0xc2,
  0xa2, 0x01,
  0xa3, 0xb8,
  0xa4, 0x00,
  0xa5, 0xe1,
  0xa6, 0x00,
  0xa7, 0xc6,
  0xa8, 0x01,
  0xa9, 0x7c,
  0xaa, 0x01,
  0xab, 0x7c,
  0xac, 0x00,
  0xad, 0xea,
  0xae, 0x00,
  0xaf, 0xea,
  0xb0, 0x09,
  0xb1, 0xaa,
  0xb2, 0x0f,
  0xb3, 0xae,
  0xb4, 0x00,
  0xb5, 0x17,
  0xb6, 0x08,
  0xb7, 0xe8,
  0xb8, 0xb0,
  0xb9, 0xce,
  0xba, 0x90,
  0xbb, 0x03,
  0xbc, 0x00,
  0xbd, 0x04,
  0xbe, 0x05,
  0xbf, 0x00,
  0xc0, 0x00,
  0xc1, 0x18,
  0xc2, 0x02,
  0xc3, 0xd0,
            
  0xff, 0x01,
  0x54, 0x0e,
  0xff, 0x01,
  0x54, 0x0f,
  
  
  #if (TEST_PATTERN)
  //Page2 for Test Pattern
  0xff, 0x02,

  0x80, 0xB8,
  #endif

//Stop mipi 2lane
  0xff, 0x00,
  0x47, 0x1a,
  0xff, 0x00,
  0x40, 0x00,
  0xff, 0x00,
  0x4e, 0x7f,

//Start mipi 2lane
  0xff, 0x00,
  0x40, 0x00,
  0xff, 0x00,
  0x4e, 0x4c,
  0xff, 0x00,
  0x40, 0x40,
  0xff, 0x00,
  0x4e, 0x6c,
  0xff, 0x00,
  0x4e, 0x4c,
  0xff, 0x00,
  0x47, 0x9a,
};
#endif

ISP_UINT16 SNR_PR2000_Reg_HD_25P[] = 
{
    SENSOR_DELAY_REG, 0, // delay
};
#endif //(TEST_MODE == TEST_BT601)

#if (TEST_MODE == TEST_BT656)
ISP_UINT16 SNR_PR2000_Reg_HD_30P[] = 
{
SENSOR_DELAY_REG, 0, // delay
};

ISP_UINT16 SNR_PR2000_Reg_HD_25P[] = 
{
SENSOR_DELAY_REG, 0, // delay
};
#endif //(TEST_MODE == TEST_BT656)


#if 0
void ____Sensor_Customer_Func____(){ruturn;} // dummy
#endif

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_InitConfig
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_InitConfig(void)
{
    MMPF_SENSOR_CUSTOMER *pCust = NULL;

#if (TVDEC_SNR_PROI == PRM_SENSOR)
    pCust = &SensorCustFunc;
#else
    pCust = &SubSensorCustFunc;
#endif
    
#if (TVDEC_SNR_IF == SENSOR_IF_PARALLEL)
	RTNA_DBG_Str(0, "SNR_Cust_InitConfig PR2000 Parallel\r\n");
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_1_LANE)
	RTNA_DBG_Str(0, "SNR_Cust_InitConfig PR2000 MIPI 1-lane\r\n");
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_2_LANE)
	RTNA_DBG_Str(0, "SNR_Cust_InitConfig PR2000 MIPI 2-lane\r\n");	
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_4_LANE)
	RTNA_DBG_Str(0, "SNR_Cust_InitConfig PR2000 MIPI 4-lane\r\n");
#endif


    // Init Res Table
    if (MMP_GetTvDecSnrAttr()->bRawStorePathEnable)
        pCust->ResTable                                     = &m_PR2000_SensorRes_RawStore_Mode;
    else
        pCust->ResTable                                     = &m_PR2000_SensorRes_BypassISP_Mode;

    // Init OPR Table
    pCust->OprTable->usInitSize 						    = (sizeof(SNR_PR2000_Reg_Init_Customer)/sizeof(SNR_PR2000_Reg_Init_Customer[0]))/2;
    pCust->OprTable->uspInitTable 					        = &SNR_PR2000_Reg_Init_Customer[0];

    pCust->OprTable->bBinTableExist                         = MMP_FALSE;
    pCust->OprTable->bInitDoneTableExist                    = MMP_FALSE;

    // Share with NTSC format due to no initial setting.
    pCust->OprTable->usSize[RES_IDX_HD_30FPS]		        = (sizeof(SNR_PR2000_Reg_HD_30P)/sizeof(SNR_PR2000_Reg_HD_30P[0]))/2;
    pCust->OprTable->uspTable[RES_IDX_HD_30FPS] 	        = &SNR_PR2000_Reg_HD_30P[0];
    pCust->OprTable->usSize[RES_IDX_HD_25FPS]		        = (sizeof(SNR_PR2000_Reg_HD_25P)/sizeof(SNR_PR2000_Reg_HD_25P[0]))/2;
    pCust->OprTable->uspTable[RES_IDX_HD_25FPS] 	        = &SNR_PR2000_Reg_HD_25P[0];
   
    // Init Vif Setting : Common
    pCust->VifSetting->SnrType                      		= MMPF_VIF_SNR_TYPE_YUV;
    
#if (TVDEC_SNR_IF == SENSOR_IF_PARALLEL)
	pCust->VifSetting->OutInterface                         = MMPF_VIF_IF_PARALLEL;
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_1_LANE)
	pCust->VifSetting->OutInterface 					    = MMPF_VIF_IF_MIPI_SINGLE_0;
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_2_LANE)
#if (SCD_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1) //MIPI 2 lane
	pCust->VifSetting->OutInterface 					    = MMPF_VIF_IF_MIPI_DUAL_12;
#else
	pCust->VifSetting->OutInterface 					    = MMPF_VIF_IF_MIPI_DUAL_01;
#endif
#elif (TVDEC_SNR_IF == SENSOR_IF_MIPI_4_LANE)
	pCust->VifSetting->OutInterface 					    = MMPF_VIF_IF_MIPI_QUAD;
#endif

#if (SCD_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1)
	pCust->VifSetting->VifPadId 						    = MMPF_VIF_MDL_ID1;
#else
    pCust->VifSetting->VifPadId 						    = MMPF_VIF_MDL_ID0;
#endif

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
    pCust->VifSetting->clockAttr.ubClkPolarity 		        = MMPF_VIF_SNR_CLK_POLARITY_POS; 
    pCust->VifSetting->clockAttr.ubClkSrc 			        = MMPF_VIF_SNR_CLK_SRC_PMCLK;    

    // Init Vif Setting : Parallel Sensor Setting
    pCust->VifSetting->paralAttr.ubLatchTiming 		        = MMPF_VIF_SNR_LATCH_POS_EDGE;   ///MMPF_VIF_SNR_LATCH_NEG_EDGE;
    pCust->VifSetting->paralAttr.ubHsyncPolarity 	        = MMPF_VIF_SNR_CLK_POLARITY_POS; 
    pCust->VifSetting->paralAttr.ubVsyncPolarity 	        = MMPF_VIF_SNR_CLK_POLARITY_POS; 
    pCust->VifSetting->paralAttr.ubBusBitMode               = MMPF_VIF_SNR_PARAL_BITMODE_10;//caution:In dual sensor case,use as rear cam,must be MMPF_VIF_SNR_PARAL_BITMODE_10,
    																						//Otherwise VIF1(front cam) will be abnormal

    // Init Vif Setting : Color ID Setting
    pCust->VifSetting->colorId.VifColorId 				    = MMPF_VIF_COLORID_00;
    pCust->VifSetting->colorId.CustomColorId.bUseCustomId 	= MMP_FALSE;

    // Init Vif Setting : MIPI Sensor Setting
    pCust->VifSetting->mipiAttr.bClkDelayEn                 = MMP_FALSE;
    pCust->VifSetting->mipiAttr.bClkLaneSwapEn              = MMP_FALSE;
    pCust->VifSetting->mipiAttr.usClkDelay                  = 0;
    pCust->VifSetting->mipiAttr.ubBClkLatchTiming           = MMPF_VIF_SNR_LATCH_NEG_EDGE;
#if (TVDEC_SNR_IF == SENSOR_IF_MIPI_4_LANE)
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
#if (SCD_SENSOR_VIF_ID == MMPF_VIF_MDL_ID1) //MIPI 2 lane
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
#endif

    // Init Vif Setting : YUV Setting
    pCust->VifSetting->yuvAttr.bRawStoreEnable              = MMP_GetTvDecSnrAttr()->bRawStorePathEnable;
    pCust->VifSetting->yuvAttr.bYuv422ToYuv420              = MMP_FALSE;
    pCust->VifSetting->yuvAttr.bYuv422ToYuv422              = MMP_TRUE;
    pCust->VifSetting->yuvAttr.bYuv422ToBayer               = MMP_FALSE;
    pCust->VifSetting->yuvAttr.ubYuv422Order                = MMPF_VIF_YUV422_UYVY;
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_DoAWB_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_DoAWB_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_DoAWB_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_DoIQ
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_DoIQ(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_SetGain
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_SetGain(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_SetExposure
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_SetExposure(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain, MMP_ULONG shutter, MMP_ULONG vsync)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_SetFlip
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_SetFlip(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_SetRotate
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_SetRotate(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_CheckVersion
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_CheckVersion(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_GetIqCompressData
//  Description :
//------------------------------------------------------------------------------
static const MMP_UBYTE* SNR_PR2000_Cust_GetIqCompressData(MMP_UBYTE ubSnrSel)
{
    return NULL;
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_StreamEnable
//  Description : Enable/Disable streaming of sensor
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_StreamEnable(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_CheckSnrTVSrcType
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_CheckSnrTVSrcType(MMP_UBYTE ubSnrSel, MMP_SNR_TVDEC_SRC_TYPE *TVSrcType)
{
#if !defined(MBOOT_FW)
    #if 0 //TODO
    MMP_USHORT usData;

	gsSensorFunction->MMPF_Sensor_GetReg(ubSnrSel, 0x1C, &usData);
	
    usData = (usData & 0x70) >> 4;
    
    printc("TVSrc = %d\r\n", usData);
    
	if ((usData == 7) || (usData == 2)) {
		*TVSrcType =  MMP_SNR_TVDEC_SRC_NO_READY;
	}
	else if ((usData == 0) || (usData == 3)) {
		*TVSrcType =  MMP_SNR_TVDEC_SRC_NTSC;
	}
	else {
		*TVSrcType =  MMP_SNR_TVDEC_SRC_PAL;
	}
    
    #endif
	*TVSrcType =  MMP_SNR_TVDEC_SRC_HD;//for debug
#endif
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_GetSnrID
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_GetSnrID(MMP_UBYTE ubSnrSel, MMP_ULONG *SensorID)
{
    *SensorID = 0xBBBB6750; // The ID is customized.Pls make it different with other sensors.
}

//------------------------------------------------------------------------------
//  Function    : SNR_PR2000_Cust_GetSnrOddEvenState
//  Description :
//------------------------------------------------------------------------------
static void SNR_PR2000_Cust_GetSnrOddEvenState(MMP_UBYTE ubSnrSel, MMP_UBYTE *State)
{
    // TBD
}

#if 0
void ____Sensor_Cust_Func_Struc____(){ruturn;} // dummy
#endif

#if (TVDEC_SNR_PROI == PRM_SENSOR)
MMPF_SENSOR_CUSTOMER SensorCustFunc = 
{
    SNR_PR2000_Cust_InitConfig,
    SNR_PR2000_Cust_DoAE_FrmSt,
    SNR_PR2000_Cust_DoAE_FrmEnd,
    SNR_PR2000_Cust_DoAWB_FrmSt,
    SNR_PR2000_Cust_DoAWB_FrmEnd,
    SNR_PR2000_Cust_DoIQ,
    SNR_PR2000_Cust_SetGain,
    SNR_PR2000_Cust_SetShutter,
    SNR_PR2000_Cust_SetExposure,
    SNR_PR2000_Cust_SetFlip,
    SNR_PR2000_Cust_SetRotate,
    SNR_PR2000_Cust_CheckVersion,
    SNR_PR2000_Cust_GetIqCompressData,
    SNR_PR2000_Cust_StreamEnable,

	&m_PR2000_SensorRes_RawStore_Mode,
	&m_PR2000_OprTable,
	&m_PR2000_VifSetting,
	&m_PR2000_I2cmAttr,
	&m_PR2000_AwbTime,
	&m_PR2000_AeTime,
	&m_PR2000_AfTime,
    MMP_SNR_PRIO_PRM,
    
	SNR_PR2000_Cust_CheckSnrTVSrcType,
	SNR_PR2000_Cust_GetSnrID,
    SNR_PR2000_Cust_GetSnrOddEvenState
};

int SNR_Module_Init(void)
{
    MMPF_SensorDrv_Register(PRM_SENSOR, &SensorCustFunc);
    
    return 0;
}
#else
MMPF_SENSOR_CUSTOMER SubSensorCustFunc = 
{
    SNR_PR2000_Cust_InitConfig,
    SNR_PR2000_Cust_DoAE_FrmSt,
    SNR_PR2000_Cust_DoAE_FrmEnd,
    SNR_PR2000_Cust_DoAWB_FrmSt,
    SNR_PR2000_Cust_DoAWB_FrmEnd,
    SNR_PR2000_Cust_DoIQ,
    SNR_PR2000_Cust_SetGain,
    SNR_PR2000_Cust_SetShutter,
    SNR_PR2000_Cust_SetExposure,
    SNR_PR2000_Cust_SetFlip,
    SNR_PR2000_Cust_SetRotate,
    SNR_PR2000_Cust_CheckVersion,
    SNR_PR2000_Cust_GetIqCompressData,
    SNR_PR2000_Cust_StreamEnable,

	&m_PR2000_SensorRes_RawStore_Mode,
	&m_PR2000_OprTable,
	&m_PR2000_VifSetting,
	&m_PR2000_I2cmAttr,
	&m_PR2000_AwbTime,
	&m_PR2000_AeTime,
	&m_PR2000_AfTime,
    MMP_SNR_PRIO_SCD,
    
    SNR_PR2000_Cust_CheckSnrTVSrcType,
    SNR_PR2000_Cust_GetSnrID,
    SNR_PR2000_Cust_GetSnrOddEvenState
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

#endif // (BIND_SENSOR_PR2000)
#endif // (SENSOR_EN)
