//==============================================================================
//
//  File        : TvDecoder_RN6750.c
//  Description : Firmware Sensor Control File
//  Author      :
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#include "customer_config.h"
#include "snr_cfg.h"

#if (SENSOR_EN)
#if (BIND_SENSOR_RN6750)

#include "mmpf_sensor.h"
#include "Sensor_Mod_Remapping.h"
#include "isp_if.h"

#define TEST_BT601              (1)
#define TEST_BT656              (2)
#define TEST_MODE               (TEST_BT601)

#define EVM_ABOARD              (1)
#define EVM_MSTAR				(2)
#define EVM_BOARD               (EVM_MSTAR)

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================
MMPF_SENSOR_RESOLUTION m_RN6750_SensorRes_RawStore_Mode =
{
	2,			// ubSensorModeNum
	1,//0,			// ubDefPreviewMode
	1,//0,			// ubDefCaptureMode
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

MMPF_SENSOR_RESOLUTION m_RN6750_SensorRes_BypassISP_Mode =
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
MMPF_SENSOR_OPR_TABLE       m_RN6750_OprTable;
MMPF_SENSOR_VIF_SETTING     m_RN6750_VifSetting;

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
static MMP_I2CM_ATTR m_RN6750_I2cmAttr = 
{
    MMP_I2CM0,//MMP_I2CM0,  // i2cmID
    (0x2C),  	// ubSlaveAddr  
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
MMPF_SENSOR_AWBTIMIMG m_RN6750_AwbTime =
{
	6,	/* ubPeriod */
	1, 	/* ubDoAWBFrmCnt */
	3	/* ubDoCaliFrmCnt */
};

MMPF_SENSOR_AETIMIMG m_RN6750_AeTime =
{
	6, 	/* ubPeriod */
	0, 	/* ubFrmStSetShutFrmCnt */
	0	/* ubFrmStSetGainFrmCnt */
};

MMPF_SENSOR_AFTIMIMG m_RN6750_AfTime =
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

ISP_UINT16 SNR_RN6750_Reg_Init_Customer[] = 
{
     SENSOR_DELAY_REG, 0 // delay
};

#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} //dummy
#endif

#if (TEST_MODE == TEST_BT601)
#if (EVM_BOARD == EVM_ABOARD)
ISP_UINT16 SNR_RN6750_Reg_HD_30P[] = 
{
    // 720P@30, 72MHz, BT601 output
	// Slave address is 0x58
	// Register, data

	/* if clock source(Xin) of RN6750 is 26MHz, please add these procedures marked first
	//0xD2, 0x85, // disable auto clock detect
	//0xD6, 0x37, // 27MHz default
	//0xD8, 0x18, // switch to 26MHz clock
	//0x80, 0x36, // PLL reset
	//delay(100), // delay 100ms
	//0x80, 0x30, // reset complete
	//delay(100), // delay 100ms
	*/
 
    //SENSOR_DELAY_REG, 0, // delay
     
	0x81, 0x01, // turn on video decoder
	0xA3, 0x04, // enable 72MHz sampling
	0xDB, 0x8F, // internal use*
	0xFF, 0x00, // switch to ch0 (default; optional)
	0x2C, 0x30, // select sync slice points
	0x50, 0x02, // 720p resolution select for BT.601
	0x56, 0x04, // disable SAV & EAV for BT601; 0x00 enable SAV & EAV for BT656
	0x63, 0xBD, // filter control
	0x59, 0x00, // extended register access
	0x5A, 0x04, // data for extended register
	0x58, 0x01, // enable extended register write
	0x07, 0x23, // 720p format
	0x2F, 0x04, // internal use*
	0x5E, 0x0B, // enable H-scaling control
	0x51, 0x44, // scale factor1
	0x52, 0x86, // scale factor2
	0x53, 0x22, // scale factor3
	0x3A, 0x04, // no channel information insertion; invert VBLK for frame valid
	0x3E, 0x32, // AVID out AVID and VBLK out VBLK for BT.601

	0xDF, 0xFE, // enable 720p format
	0x8E, 0x00, // single channel output for VP
	0x8F, 0x80, // 720p mode for VP
	0x8D, 0x31, // enable VP out
	0x89, 0x09, // select 72MHz for SCLK
	0x88, 0xC1, // enable SCLK out
	0x81, 0x01, // turn on video decoder

	0x96, 0x08, // select AVID & VBLK as status indicator, VSYNC as GPO
	0x97, 0x03, // enable status indicator out on AVID & VBLK, GPO on VSYNC
	0x98, 0x00, // output 0 on VSYNC
	0x9A, 0x41, // select HSYNC as GPO
	0x9B, 0xE0, // enable GPO on HSYNC
	0x9C, 0x00, // output 0 on HSYNC
};
#endif

#if (EVM_BOARD == EVM_MSTAR)
ISP_UINT16 SNR_RN6750_Reg_HD_30P[] = 
{
    // 720P@30, 72MHz, BT601 output
	// Slave address is 0x58
	// Register, data

	// if clock source(Xin) of RN6752 is 26MHz, please add these procedures marked first
	0xD2, 0x85, // disable auto clock detect
	0xD6, 0x37, // 27MHz default
	0xD8, 0x18, // switch to 26MHz clock
	//delay(100), // delay 100ms
	SENSOR_DELAY_REG, 100,
	 
	0x81, 0x01, // turn on video decoder
	0xA3, 0x04, // enable 72MHz sampling
	0xDB, 0x8F, // internal use*
	0xFF, 0x00, // switch to ch0 (default; optional)
	0x2C, 0x30, // select sync slice points
	0x50, 0x02, // 720p resolution select for BT.601
	0x56, 0x04, // disable SAV & EAV for BT601; 0x00 enable SAV & EAV for BT656
	0x63, 0xBD, // filter control
	0x59, 0x00, // extended register access
	0x5A, 0x04, // data for extended register
	0x58, 0x01, // enable extended register write
	0x07, 0x23, // 720p format
	0x2F, 0x04, // internal use*
	0x5E, 0x0B, // enable H-scaling control
	0x51, 0x44, // scale factor1
	0x52, 0x86, // scale factor2
	0x53, 0x22, // scale factor3
	0x3A, 0x04, // no channel information insertion; invert VBLK for frame valid
	0x3E, 0x32, // AVID & VBLK out for BT.601
	0x40, 0x04, // no channel information insertion; invert VBLK for frame valid
	0x46, 0x23, // AVID & VBLK out for BT.601
	0x28, 0x92,
	0x00, 0x20,
	0x2D, 0xF2,
	0x05, 0x00,

	0xDF, 0xFE, // enable 720p format
	0x8E, 0x00, // single channel output for VP
	0x8F, 0x80, // 720p mode for VP
	0x8D, 0x31, // enable VP out
	0x89, 0x09, // select 72MHz for SCLK
	0x88, 0xC1, // enable SCLK out
	0x81, 0x01, // turn on video decoder

	0x96, 0x00, // select AVID & VBLK as status indicator
	0x97, 0x0B, // enable status indicator out on AVID,VBLK & VSYNC 
	0x98, 0x00, // 
	0x9A, 0x40, // select AVID & VBLK as status indicator 
	0x9B, 0xE1, // enable status indicator out on HSYNC
	0x9C, 0x00, //
};
#endif

#if (EVM_BOARD == EVM_ABOARD)
ISP_UINT16 SNR_RN6750_Reg_HD_25P[] = 
{
    // 720P@25, 72MHz, BT601 output
	// Slave address is 0x58
	// Register, data

	/* if clock source(Xin) of RN6750 is 26MHz, please add these procedures marked first
	//0xD2, 0x85, // disable auto clock detect
	//0xD6, 0x37, // 27MHz default
	//0xD8, 0x18, // switch to 26MHz clock
	//0x80, 0x36, // PLL reset
	//delay(100), // delay 100ms
	//0x80, 0x30, // reset complete
	//delay(100), // delay 100ms
	*/
 
    //SENSOR_DELAY_REG, 0, // delay
     
	0x81, 0x01, // turn on video decoder
	0xA3, 0x04, // enable 72MHz sampling
	0xDB, 0x8F, // internal use*
	0xFF, 0x00, // switch to ch0 (default; optional)
	0x2C, 0x30, // select sync slice points
	0x50, 0x02, // 720p resolution select for BT.601
	0x56, 0x04, // disable SAV & EAV for BT601; 0x00 enable SAV & EAV for BT656
	0x63, 0xBD, // filter control
	0x59, 0x00, // extended register access
	0x5A, 0x02, // data for extended register
	0x58, 0x01, // enable extended register write
	0x07, 0x23, // 720p format
	0x2F, 0x04, // internal use*
	0x5E, 0x0B, // enable H-scaling control
	0x51, 0x44, // scale factor1
	0x52, 0x86, // scale factor2
	0x53, 0x22, // scale factor3
	0x3A, 0x04, // no channel information insertion; invert VBLK for frame valid
	0x3E, 0x32, // AVID out AVID and VBLK out VBLK for BT.601

	0xDF, 0xFE, // enable 720p format
	0x8E, 0x00, // single channel output for VP
	0x8F, 0x80, // 720p mode for VP
	0x8D, 0x31, // enable VP out
	0x89, 0x09, // select 72MHz for SCLK
	0x88, 0xC1, // enable SCLK out
	0x81, 0x01, // turn on video decoder

	0x96, 0x08, // select AVID & VBLK as status indicator, VSYNC as GPO
	0x97, 0x03, // enable status indicator out on AVID & VBLK, GPO on VSYNC
	0x98, 0x00, // output 0 on VSYNC
	0x9A, 0x41, // select HSYNC as GPO
	0x9B, 0xE0, // enable GPO on HSYNC
	0x9C, 0x00, // output 0 on HSYNC
};
#endif

#if (EVM_BOARD == EVM_MSTAR)
ISP_UINT16 SNR_RN6750_Reg_HD_25P[] = 
{
	// 720P@25, 72MHz, BT601 output
	// Slave address is 0x58
	// Register, data

	// if clock source(Xin) of RN6752 is 26MHz, please add these procedures marked first
	0xD2, 0x85, // disable auto clock detect
	0xD6, 0x37, // 27MHz default
	0xD8, 0x18, // switch to 26MHz clock
	//delay(100), // delay 100ms
	SENSOR_DELAY_REG, 100,

	0x81, 0x01, // turn on video decoder
	0xA3, 0x04, // enable 72MHz sampling
	0xDB, 0x8F, // internal use*
	0xFF, 0x00, // switch to ch0 (default; optional)
	0x2C, 0x30, // select sync slice points
	0x50, 0x02, // 720p resolution select for BT.601
	0x56, 0x04, // disable SAV & EAV for BT601; 0x00 enable SAV & EAV for BT656
	0x63, 0xBD, // filter control
	0x59, 0x00, // extended register access
	0x5A, 0x02, // data for extended register
	0x58, 0x01, // enable extended register write
	0x07, 0x23, // 720p format
	0x2F, 0x04, // internal use*
	0x5E, 0x0B, // enable H-scaling control
	0x51, 0x44, // scale factor1
	0x52, 0x86, // scale factor2
	0x53, 0x22, // scale factor3
	0x3A, 0x04, // no channel information insertion; invert VBLK for frame valid
	0x3E, 0x32, // AVID & VBLK out for BT.601
	0x40, 0x04, // no channel information insertion; invert VBLK for frame valid
	0x46, 0x23, // AVID & VBLK out for BT.601
	0x28, 0x92,
	0x00, 0x20,
	0x2D, 0xF2,
	0x05, 0x00,

	0xDF, 0xFE, // enable 720p format
	0x8E, 0x00, // single channel output for VP
	0x8F, 0x80, // 720p mode for VP
	0x8D, 0x31, // enable VP out
	0x89, 0x09, // select 72MHz for SCLK
	0x88, 0xC1, // enable SCLK out
	0x81, 0x01, // turn on video decoder

	0x96, 0x00, // select AVID & VBLK as status indicator
	0x97, 0x0B, // enable status indicator out on AVID,VBLK & VSYNC 
	0x98, 0x00, // 
	0x9A, 0x40, // select AVID & VBLK as status indicator 
	0x9B, 0xE1, // enable status indicator out on HSYNC
	0x9C, 0x00, //
};
#endif

#endif //(TEST_MODE == TEST_BT601)

#if (TEST_MODE == TEST_BT656)
ISP_UINT16 SNR_RN6750_Reg_HD_30P[] = 
{
	// 720P@30, 72MHz, BT656 output
	// Slave address is 0x58
	// Register, data

	// if clock source(Xin) of RN6752 is 26MHz, please add these procedures marked first
	//0xD2, 0x85, // disable auto clock detect
	//0xD6, 0x37, // 27MHz default
	//0xD8, 0x18, // switch to 26MHz clock
	//delay(100), // delay 100ms

	 
	0x81, 0x01, // turn on video decoder
	0xA3, 0x04, // enable 72MHz sampling
	0xDB, 0x8F, // internal use*
	0xFF, 0x00, // switch to ch0 (default; optional)
	0x00, 0x20, // internal use
	0x2D, 0xF2, // cagc control
	0x04, 0x60, // hue
	0x05, 0x00, // sharpness
	0x2C, 0x30, // select sync slice points
	0x50, 0x02, // 720p resolution select for BT.601
	0x56, 0x00, // disable SAV & EAV for BT601; 0x00 enable SAV & EAV for BT656
	0x63, 0xBD, // filter control
	0x59, 0x00, // extended register access
	0x5A, 0x04, // data for extended register
	0x58, 0x01, // enable extended register write
	0x07, 0x23, // 720p format
	0x2F, 0x04, // internal use
	0x5E, 0x0B, // enable H-scaling control
	0x51, 0x44, // scale factor1
	0x52, 0x86, // scale factor2
	0x53, 0x22, // scale factor3
	0x3A, 0x04, // no channel information insertion; invert VBLK for frame valid
	0x3E, 0x32, // AVID out AVID and VBLK out VBLK for BT.601

	0xDF, 0xFE, // enable 720p format
	0x8E, 0x00, // single channel output for VP
	0x8F, 0x80, // 720p mode for VP
	0x8D, 0x31, // enable VP out
	0x89, 0x09, // select 72MHz for SCLK
	0x88, 0xC1, // enable SCLK out
	0x81, 0x01, // turn on video decoder

	0x96, 0x08, // select AVID & VBLK as status indicator, VSYNC as GPO
	0x97, 0x03, // enable status indicator out on AVID & VBLK, GPO on VSYNC
	0x98, 0x00, // output 0 on VSYNC
	0x9A, 0x41, // select HSYNC as GPO
	0x9B, 0xE0, // enable GPO on HSYNC
	0x9C, 0x00, // output 0 on HSYNC    
};

ISP_UINT16 SNR_RN6750_Reg_HD_25P[] = 
{
	// 720P@25, 72MHz, BT656 output
	// Slave address is 0x58
	// Register, data

	// if clock source(Xin) of RN6752 is 26MHz, please add these procedures marked first
	//0xD2, 0x85, // disable auto clock detect
	//0xD6, 0x37, // 27MHz default
	//0xD8, 0x18, // switch to 26MHz clock
	//delay(100), // delay 100ms

	 
	0x81, 0x01, // turn on video decoder
	0xA3, 0x04, // enable 72MHz sampling
	0xDB, 0x8F, // internal use*
	0xFF, 0x00, // switch to ch0 (default; optional)
	0x00, 0x20, // internal use
	0x2D, 0xF2, // cagc control
	0x04, 0x80, // hue
	0x05, 0x00, // sharpness
	0x2C, 0x30, // select sync slice points
	0x50, 0x02, // 720p resolution select for BT.601
	0x56, 0x00, // disable SAV & EAV for BT601; 0x00 enable SAV & EAV for BT656
	0x63, 0xBD, // filter control
	0x59, 0x00, // extended register access
	0x5A, 0x02, // data for extended register
	0x58, 0x01, // enable extended register write
	0x07, 0x23, // 720p format
	0x2F, 0x04, // internal use
	0x5E, 0x0B, // enable H-scaling control
	0x51, 0x44, // scale factor1
	0x52, 0x86, // scale factor2
	0x53, 0x22, // scale factor3
	0x3A, 0x04, // no channel information insertion; invert VBLK for frame valid
	0x3E, 0x32, // AVID out AVID and VBLK out VBLK for BT.601

	0xDF, 0xFE, // enable 720p format
	0x8E, 0x00, // single channel output for VP
	0x8F, 0x80, // 720p mode for VP
	0x8D, 0x31, // enable VP out
	0x89, 0x09, // select 72MHz for SCLK
	0x88, 0xC1, // enable SCLK out
	0x81, 0x01, // turn on video decoder

	0x96, 0x08, // select AVID & VBLK as status indicator, VSYNC as GPO
	0x97, 0x03, // enable status indicator out on AVID & VBLK, GPO on VSYNC
	0x98, 0x00, // output 0 on VSYNC
	0x9A, 0x41, // select HSYNC as GPO
	0x9B, 0xE0, // enable GPO on HSYNC
	0x9C, 0x00, // output 0 on HSYNC
};
#endif //(TEST_MODE == TEST_BT656)


#if 0
void ____Sensor_Customer_Func____(){ruturn;} // dummy
#endif

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_InitConfig
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_InitConfig(void)
{
    MMPF_SENSOR_CUSTOMER *pCust = NULL;

#if (TVDEC_SNR_PROI == PRM_SENSOR)
    pCust = &SensorCustFunc;
#else
    pCust = &SubSensorCustFunc;
#endif

    RTNA_DBG_Str(0, FG_PURPLE("SNR_Cust_InitConfig RN6750\r\n"));

    // Init Res Table
    if (MMP_GetTvDecSnrAttr()->bRawStorePathEnable)
        pCust->ResTable                                     = &m_RN6750_SensorRes_RawStore_Mode;
    else
        pCust->ResTable                                     = &m_RN6750_SensorRes_BypassISP_Mode;

    // Init OPR Table
    pCust->OprTable->usInitSize 						    = (sizeof(SNR_RN6750_Reg_Init_Customer)/sizeof(SNR_RN6750_Reg_Init_Customer[0]))/2;
    pCust->OprTable->uspInitTable 					        = &SNR_RN6750_Reg_Init_Customer[0];

    pCust->OprTable->bBinTableExist                         = MMP_FALSE;
    pCust->OprTable->bInitDoneTableExist                    = MMP_FALSE;

    // Share with NTSC format due to no initial setting.
    pCust->OprTable->usSize[RES_IDX_HD_30FPS]		        = (sizeof(SNR_RN6750_Reg_HD_30P)/sizeof(SNR_RN6750_Reg_HD_30P[0]))/2;
    pCust->OprTable->uspTable[RES_IDX_HD_30FPS] 	        = &SNR_RN6750_Reg_HD_30P[0];
    pCust->OprTable->usSize[RES_IDX_HD_25FPS]		        = (sizeof(SNR_RN6750_Reg_HD_25P)/sizeof(SNR_RN6750_Reg_HD_25P[0]))/2;
    pCust->OprTable->uspTable[RES_IDX_HD_25FPS] 	        = &SNR_RN6750_Reg_HD_25P[0];
   
    // Init Vif Setting : Common
    pCust->VifSetting->SnrType                      		= MMPF_VIF_SNR_TYPE_YUV;
    pCust->VifSetting->OutInterface 					    = MMPF_VIF_IF_PARALLEL;
    pCust->VifSetting->VifPadId 						    = MMPF_VIF_MDL_ID0;

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

    // Init Vif Setting : YUV Setting
    pCust->VifSetting->yuvAttr.bRawStoreEnable              = MMP_GetTvDecSnrAttr()->bRawStorePathEnable;
    pCust->VifSetting->yuvAttr.bYuv422ToYuv420              = MMP_FALSE;
    pCust->VifSetting->yuvAttr.bYuv422ToYuv422              = MMP_TRUE;
    pCust->VifSetting->yuvAttr.bYuv422ToBayer               = MMP_FALSE;
    pCust->VifSetting->yuvAttr.ubYuv422Order                = MMPF_VIF_YUV422_UYVY;
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_DoAWB_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_DoAWB_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_DoAWB_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_DoIQ
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_DoIQ(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_SetGain
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_SetGain(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_SetExposure
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_SetExposure(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain, MMP_ULONG shutter, MMP_ULONG vsync)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_SetFlip
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_SetFlip(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_SetRotate
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_SetRotate(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_CheckVersion
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_CheckVersion(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_GetIqCompressData
//  Description :
//------------------------------------------------------------------------------
static const MMP_UBYTE* SNR_RN6750_Cust_GetIqCompressData(MMP_UBYTE ubSnrSel)
{
    return NULL;
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_StreamEnable
//  Description : Enable/Disable streaming of sensor
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_StreamEnable(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_CheckSnrTVSrcType
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_CheckSnrTVSrcType(MMP_UBYTE ubSnrSel, MMP_SNR_TVDEC_SRC_TYPE *TVSrcType)
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
//  Function    : SNR_RN6750_Cust_GetSnrID
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_GetSnrID(MMP_UBYTE ubSnrSel, MMP_ULONG *SensorID)
{
    *SensorID = 0xBBBB6750; // The ID is customized.Pls make it different with other sensors.
}

//------------------------------------------------------------------------------
//  Function    : SNR_RN6750_Cust_GetSnrOddEvenState
//  Description :
//------------------------------------------------------------------------------
static void SNR_RN6750_Cust_GetSnrOddEvenState(MMP_UBYTE ubSnrSel, MMP_UBYTE *State)
{
    // TBD
}

#if 0
void ____Sensor_Cust_Func_Struc____(){ruturn;} // dummy
#endif

#if (TVDEC_SNR_PROI == PRM_SENSOR)
MMPF_SENSOR_CUSTOMER SensorCustFunc = 
{
    SNR_RN6750_Cust_InitConfig,
    SNR_RN6750_Cust_DoAE_FrmSt,
    SNR_RN6750_Cust_DoAE_FrmEnd,
    SNR_RN6750_Cust_DoAWB_FrmSt,
    SNR_RN6750_Cust_DoAWB_FrmEnd,
    SNR_RN6750_Cust_DoIQ,
    SNR_RN6750_Cust_SetGain,
    SNR_RN6750_Cust_SetShutter,
    SNR_RN6750_Cust_SetExposure,
    SNR_RN6750_Cust_SetFlip,
    SNR_RN6750_Cust_SetRotate,
    SNR_RN6750_Cust_CheckVersion,
    SNR_RN6750_Cust_GetIqCompressData,
    SNR_RN6750_Cust_StreamEnable,

	&m_RN6750_SensorRes_RawStore_Mode,
	&m_RN6750_OprTable,
	&m_RN6750_VifSetting,
	&m_RN6750_I2cmAttr,
	&m_RN6750_AwbTime,
	&m_RN6750_AeTime,
	&m_RN6750_AfTime,
    MMP_SNR_PRIO_PRM,
    
	SNR_RN6750_Cust_CheckSnrTVSrcType,
	SNR_RN6750_Cust_GetSnrID,
    SNR_RN6750_Cust_GetSnrOddEvenState
};

int SNR_Module_Init(void)
{
    MMPF_SensorDrv_Register(PRM_SENSOR, &SensorCustFunc);
    
    return 0;
}
#else
MMPF_SENSOR_CUSTOMER SubSensorCustFunc = 
{
    SNR_RN6750_Cust_InitConfig,
    SNR_RN6750_Cust_DoAE_FrmSt,
    SNR_RN6750_Cust_DoAE_FrmEnd,
    SNR_RN6750_Cust_DoAWB_FrmSt,
    SNR_RN6750_Cust_DoAWB_FrmEnd,
    SNR_RN6750_Cust_DoIQ,
    SNR_RN6750_Cust_SetGain,
    SNR_RN6750_Cust_SetShutter,
    SNR_RN6750_Cust_SetExposure,
    SNR_RN6750_Cust_SetFlip,
    SNR_RN6750_Cust_SetRotate,
    SNR_RN6750_Cust_CheckVersion,
    SNR_RN6750_Cust_GetIqCompressData,
    SNR_RN6750_Cust_StreamEnable,

	&m_RN6750_SensorRes_RawStore_Mode,
	&m_RN6750_OprTable,
	&m_RN6750_VifSetting,
	&m_RN6750_I2cmAttr,
	&m_RN6750_AwbTime,
	&m_RN6750_AeTime,
	&m_RN6750_AfTime,
    MMP_SNR_PRIO_SCD,
    
    SNR_RN6750_Cust_CheckSnrTVSrcType,
    SNR_RN6750_Cust_GetSnrID,
    SNR_RN6750_Cust_GetSnrOddEvenState
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

#endif // (BIND_SENSOR_RN6750)
#endif // (SENSOR_EN)
