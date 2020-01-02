//==============================================================================
//
//  File        : TvDecoder_TP2825.c
//  Description : Firmware Sensor Control File
//  Author      :
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#include "customer_config.h"
#include "snr_cfg.h"

#if (SENSOR_EN)
#if (BIND_SENSOR_TP2825)

#include "mmpf_sensor.h"
#include "Sensor_Mod_Remapping.h"
#include "isp_if.h"

#define TEST_BT601              (1)
#define TEST_BT656              (2)
#define TEST_MODE               (TEST_BT601)


//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================
MMPF_SENSOR_RESOLUTION m_TP2825_SensorRes_RawStore_Mode =
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

MMPF_SENSOR_RESOLUTION m_TP2825_SensorRes_BypassISP_Mode =
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
MMPF_SENSOR_OPR_TABLE       m_TP2825_OprTable;
MMPF_SENSOR_VIF_SETTING     m_TP2825_VifSetting;

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
static MMP_I2CM_ATTR m_TP2825_I2cmAttr = 
{
    MMP_I2CM0,//MMP_I2CM0,  // i2cmID
    (0x88 >> 1),  	// ubSlaveAddr  
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
MMPF_SENSOR_AWBTIMIMG m_TP2825_AwbTime =
{
	6,	/* ubPeriod */
	1, 	/* ubDoAWBFrmCnt */
	3	/* ubDoCaliFrmCnt */
};

MMPF_SENSOR_AETIMIMG m_TP2825_AeTime =
{
	6, 	/* ubPeriod */
	0, 	/* ubFrmStSetShutFrmCnt */
	0	/* ubFrmStSetGainFrmCnt */
};

MMPF_SENSOR_AFTIMIMG m_TP2825_AfTime =
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

ISP_UINT16 SNR_TP2825_Reg_Init_Customer[] = 
{
     SENSOR_DELAY_REG, 0 // delay
};

#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} //dummy
#endif

#if (TEST_MODE == TEST_BT601)
ISP_UINT16 SNR_TP2825_Reg_HD_30P[] = 
{
SENSOR_DELAY_REG, 0, // delay
};

ISP_UINT16 SNR_TP2825_Reg_HD_25P[] = 
{
	0x00, 0x11,
	0x01, 0x7E,
	0x02, 0xCA,
	0x03, 0x1D,
	0x04, 0x00,
	0x05, 0x00,
	0x06, 0x32,
	0x07, 0xC0,
	0x08, 0x00,
	0x09, 0x24,
	0x0A, 0x48,
	0x0B, 0xC0,
	0x0C, 0x53,
	0x0D, 0x10,
	0x0E, 0x00,
	0x0F, 0x00,
	0x10, 0x00,
	0x11, 0x40,
	0x12, 0x40,
	0x13, 0x00,
	0x14, 0x00,
	0x15, 0x13,
	0x16, 0x16,
	0x17, 0x00,
	0x18, 0x19,
	0x19, 0xD0,
	0x1A, 0x25,
	0x1B, 0x00,
	0x1C, 0x07,
	0x1D, 0xBC,
	0x1E, 0x80,
	0x1F, 0x80,
	0x20, 0x60,
	0x21, 0x86,
	0x22, 0x38,
	0x23, 0x3C,
	0x24, 0x56,
	0x25, 0xFF,
	0x26, 0x02,
	0x27, 0x2D,
	0x28, 0x00,
	0x29, 0x48,
	0x2A, 0x30,
	0x2B, 0x70,
	0x2C, 0x0A,
	0x2D, 0x30,
	0x2E, 0x70,
	0x2F, 0x00,
	0x30, 0x48,
	0x31, 0xBB,
	0x32, 0x2E,
	0x33, 0x90,
	0x34, 0x00,
	0x35, 0x25,
	0x36, 0xDC,
	0x37, 0x00,
	0x38, 0x40,
	0x39, 0x88,
	0x3A, 0x00,
	0x3B, 0x03,
	0x3C, 0x00,
	0x3D, 0x60,
	0x3E, 0x00,
	0x3F, 0x00,
	0x40, 0x00,
	0x41, 0x00,
	0x42, 0x00,
	0x43, 0x12,
	0x44, 0x07,
	0x45, 0x49,
	0x46, 0x00,
	0x47, 0x00,
	0x48, 0x00,
	0x49, 0x00,
	0x4A, 0x00,
	0x4B, 0x00,
	0x4C, 0x03,
	0x4D, 0x03,
	0x4E, 0x17,
	0x4F, 0x00,
	0x50, 0x00,
	0x51, 0x00,
	0x52, 0x00,
	0x53, 0x00,
	0x54, 0x00,
	0x55, 0x00,
	0x56, 0x00,
	0x57, 0x00,
	0x58, 0x00,
	0x59, 0x00,
	0x5A, 0x00,
	0x5B, 0x00,
	0x5C, 0x00,
	0x5D, 0x00,
	0x5E, 0x00,
	0x5F, 0x00,
	0x60, 0x00,
	0x61, 0x00,
	0x62, 0x00,
	0x63, 0x00,
	0x64, 0x00,
	0x65, 0x00,
	0x66, 0x00,
	0x67, 0x00,
	0x68, 0x00,
	0x69, 0x00,
	0x6A, 0x00,
	0x6B, 0x00,
	0x6C, 0x00,
	0x6D, 0x00,
	0x6E, 0x00,
	0x6F, 0x00,
	0x70, 0x00,
	0x71, 0x00,
	0x72, 0x00,
	0x73, 0x00,
	0x74, 0x00,
	0x75, 0x00,
	0x76, 0x00,
	0x77, 0x00,
	0x78, 0x00,
	0x79, 0x00,
	0x7A, 0x00,
	0x7B, 0x00,
	0x7C, 0x00,
	0x7D, 0x00,
	0x7E, 0x01,
	0x7F, 0x00,
	0x80, 0x00,
	0x81, 0x00,
	0x82, 0x00,
	0x83, 0x00,
	0x84, 0x00,
	0x85, 0x00,
	0x86, 0x00,
	0x87, 0x00,
	0x88, 0x00,
	0x89, 0x00,
	0x8A, 0x00,
	0x8B, 0xFF,
	0x8C, 0xFF,
	0x8D, 0xFF,
	0x8E, 0xFF,
	0x8F, 0xFF,
	0x90, 0xFF,
	0x91, 0xFF,
	0x92, 0xFF,
	0x93, 0xFF,
	0x94, 0xFF,
	0x95, 0x00,
	0x96, 0x00,
	0x97, 0x00,
	0x98, 0x00,
	0x99, 0x00,
	0x9A, 0x00,
	0x9B, 0x00,
	0x9C, 0x00,
	0x9D, 0x00,
	0x9E, 0x00,
	0x9F, 0x00,
	0xA0, 0x00,
	0xA1, 0x00,
	0xA2, 0x00,
	0xA3, 0x00,
	0xA4, 0x00,
	0xA5, 0x00,
	0xA6, 0x00,
	0xA7, 0x00,
	0xA8, 0x00,
	0xA9, 0x00,
	0xAA, 0x00,
	0xAB, 0x00,
	0xAC, 0x00,
	0xAD, 0x00,
	0xAE, 0x00,
	0xAF, 0x00,
	0xB0, 0x00,
	0xB1, 0x00,
	0xB2, 0x00,
	0xB3, 0xFA,
	0xB4, 0x00,
	0xB5, 0x00,
	0xB6, 0x00,
	0xB7, 0x00,
	0xB8, 0x00,
	0xB9, 0x01,
	0xBA, 0x00,
	0xBB, 0x00,
	0xBC, 0x03,
	0xBD, 0x00,
	0xBE, 0x00,
	0xBF, 0x00,
	0xC0, 0x00,
	0xC1, 0x00,
	0xC2, 0x0B,
	0xC3, 0x0C,
	0xC4, 0x00,
	0xC5, 0x00,
	0xC6, 0x1F,
	0xC7, 0x78,
	0xC8, 0x21,
	0xC9, 0x00,
	0xCA, 0x00,
	0xCB, 0x07,
	0xCC, 0x08,
	0xCD, 0x00,
	0xCE, 0x00,
	0xCF, 0x04,
	0xD0, 0x00,
	0xD1, 0x00,
	0xD2, 0x60,
	0xD3, 0x10,
	0xD4, 0x06,
	0xD5, 0xBE,
	0xD6, 0x39,
	0xD7, 0x27,
	0xD8, 0x00,
	0xD9, 0x00,
	0xDA, 0x00,
	0xDB, 0x00,
	0xDC, 0x00,
	0xDD, 0x00,
	0xDE, 0x00,
	0xDF, 0x00,
	0xE0, 0x00,
	0xE1, 0x00,
	0xE2, 0x00,
	0xE3, 0x00,
	0xE4, 0x00,
	0xE5, 0x00,
	0xE6, 0x00,
	0xE7, 0x00,
	0xE8, 0x00,
	0xE9, 0x00,
	0xEA, 0x00,
	0xEB, 0x00,
	0xEC, 0x00,
	0xED, 0x00,
	0xEE, 0x00,
	0xEF, 0x00,
	0xF0, 0x00,
	0xF1, 0x00,
	0xF2, 0x00,
	0xF3, 0x00,
	0xF4, 0x00,
	0xF5, 0x00,
	0xF6, 0x00,
	0xF7, 0x00,
	0xF8, 0x00,
	0xF9, 0x00,
	0xFA, 0x00,
	0xFB, 0x00,
	0xFC, 0xC0,
	0xFD, 0x00,
	0xFE, 0x28,
};

#endif //(TEST_MODE == TEST_BT601)

#if (TEST_MODE == TEST_BT656)
ISP_UINT16 SNR_TP2825_Reg_HD_30P[] = 
{
SENSOR_DELAY_REG, 0, // delay
};

ISP_UINT16 SNR_TP2825_Reg_HD_25P[] = 
{
SENSOR_DELAY_REG, 0, // delay
};

#endif //(TEST_MODE == TEST_BT656)


#if 0
void ____Sensor_Customer_Func____(){ruturn;} // dummy
#endif

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_InitConfig
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_InitConfig(void)
{
    MMPF_SENSOR_CUSTOMER *pCust = NULL;

#if (TVDEC_SNR_PROI == PRM_SENSOR)
    pCust = &SensorCustFunc;
#else
    pCust = &SubSensorCustFunc;
#endif

    RTNA_DBG_Str(0, FG_PURPLE("SNR_Cust_InitConfig TP2825\r\n"));

    // Init Res Table
    if (MMP_GetTvDecSnrAttr()->bRawStorePathEnable)
        pCust->ResTable                                     = &m_TP2825_SensorRes_RawStore_Mode;
    else
        pCust->ResTable                                     = &m_TP2825_SensorRes_BypassISP_Mode;

    // Init OPR Table
    pCust->OprTable->usInitSize 						    = (sizeof(SNR_TP2825_Reg_Init_Customer)/sizeof(SNR_TP2825_Reg_Init_Customer[0]))/2;
    pCust->OprTable->uspInitTable 					        = &SNR_TP2825_Reg_Init_Customer[0];

    pCust->OprTable->bBinTableExist                         = MMP_FALSE;
    pCust->OprTable->bInitDoneTableExist                    = MMP_FALSE;

    // Share with NTSC format due to no initial setting.
    pCust->OprTable->usSize[RES_IDX_HD_30FPS]		        = (sizeof(SNR_TP2825_Reg_HD_30P)/sizeof(SNR_TP2825_Reg_HD_30P[0]))/2;
    pCust->OprTable->uspTable[RES_IDX_HD_30FPS] 	        = &SNR_TP2825_Reg_HD_30P[0];
    pCust->OprTable->usSize[RES_IDX_HD_25FPS]		        = (sizeof(SNR_TP2825_Reg_HD_25P)/sizeof(SNR_TP2825_Reg_HD_25P[0]))/2;
    pCust->OprTable->uspTable[RES_IDX_HD_25FPS] 	        = &SNR_TP2825_Reg_HD_25P[0];
   
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
//  Function    : SNR_TP2825_Cust_DoAE_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_DoAE_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_DoAE_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_DoAE_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_DoAWB_FrmSt
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_DoAWB_FrmSt(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_DoAWB_FrmEnd
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_DoAWB_FrmEnd(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_DoIQ
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_DoIQ(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_SetGain
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_SetGain(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_SetShutter
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_SetShutter(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_SetExposure
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_SetExposure(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain, MMP_ULONG shutter, MMP_ULONG vsync)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_SetFlip
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_SetFlip(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_SetRotate
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_SetRotate(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_CheckVersion
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_CheckVersion(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion)
{
	// TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_GetIqCompressData
//  Description :
//------------------------------------------------------------------------------
static const MMP_UBYTE* SNR_TP2825_Cust_GetIqCompressData(MMP_UBYTE ubSnrSel)
{
    return NULL;
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_StreamEnable
//  Description : Enable/Disable streaming of sensor
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_StreamEnable(MMP_UBYTE ubSnrSel, MMP_BOOL bEnable)
{
    // TBD
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_CheckSnrTVSrcType
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_CheckSnrTVSrcType(MMP_UBYTE ubSnrSel, MMP_SNR_TVDEC_SRC_TYPE *TVSrcType)
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
//  Function    : SNR_TP2825_Cust_GetSnrID
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_GetSnrID(MMP_UBYTE ubSnrSel, MMP_ULONG *SensorID)
{
    *SensorID = 0xBBBB2825; // The ID is customized.Pls make it different with other sensors.
}

//------------------------------------------------------------------------------
//  Function    : SNR_TP2825_Cust_GetSnrOddEvenState
//  Description :
//------------------------------------------------------------------------------
static void SNR_TP2825_Cust_GetSnrOddEvenState(MMP_UBYTE ubSnrSel, MMP_UBYTE *State)
{
    // TBD
}

#if 0
void ____Sensor_Cust_Func_Struc____(){ruturn;} // dummy
#endif

#if (TVDEC_SNR_PROI == PRM_SENSOR)
MMPF_SENSOR_CUSTOMER SensorCustFunc = 
{
    SNR_TP2825_Cust_InitConfig,
    SNR_TP2825_Cust_DoAE_FrmSt,
    SNR_TP2825_Cust_DoAE_FrmEnd,
    SNR_TP2825_Cust_DoAWB_FrmSt,
    SNR_TP2825_Cust_DoAWB_FrmEnd,
    SNR_TP2825_Cust_DoIQ,
    SNR_TP2825_Cust_SetGain,
    SNR_TP2825_Cust_SetShutter,
    SNR_TP2825_Cust_SetExposure,
    SNR_TP2825_Cust_SetFlip,
    SNR_TP2825_Cust_SetRotate,
    SNR_TP2825_Cust_CheckVersion,
    SNR_TP2825_Cust_GetIqCompressData,
    SNR_TP2825_Cust_StreamEnable,

	&m_TP2825_SensorRes_RawStore_Mode,
	&m_TP2825_OprTable,
	&m_TP2825_VifSetting,
	&m_TP2825_I2cmAttr,
	&m_TP2825_AwbTime,
	&m_TP2825_AeTime,
	&m_TP2825_AfTime,
    MMP_SNR_PRIO_PRM,
    
	SNR_TP2825_Cust_CheckSnrTVSrcType,
	SNR_TP2825_Cust_GetSnrID,
    SNR_TP2825_Cust_GetSnrOddEvenState
};

int SNR_Module_Init(void)
{
    MMPF_SensorDrv_Register(PRM_SENSOR, &SensorCustFunc);
    
    return 0;
}
#else
MMPF_SENSOR_CUSTOMER SubSensorCustFunc = 
{
    SNR_TP2825_Cust_InitConfig,
    SNR_TP2825_Cust_DoAE_FrmSt,
    SNR_TP2825_Cust_DoAE_FrmEnd,
    SNR_TP2825_Cust_DoAWB_FrmSt,
    SNR_TP2825_Cust_DoAWB_FrmEnd,
    SNR_TP2825_Cust_DoIQ,
    SNR_TP2825_Cust_SetGain,
    SNR_TP2825_Cust_SetShutter,
    SNR_TP2825_Cust_SetExposure,
    SNR_TP2825_Cust_SetFlip,
    SNR_TP2825_Cust_SetRotate,
    SNR_TP2825_Cust_CheckVersion,
    SNR_TP2825_Cust_GetIqCompressData,
    SNR_TP2825_Cust_StreamEnable,

	&m_TP2825_SensorRes_RawStore_Mode,
	&m_TP2825_OprTable,
	&m_TP2825_VifSetting,
	&m_TP2825_I2cmAttr,
	&m_TP2825_AwbTime,
	&m_TP2825_AeTime,
	&m_TP2825_AfTime,
    MMP_SNR_PRIO_SCD,
    
    SNR_TP2825_Cust_CheckSnrTVSrcType,
    SNR_TP2825_Cust_GetSnrID,
    SNR_TP2825_Cust_GetSnrOddEvenState
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

#endif // (BIND_SENSOR_TP2825)
#endif // (SENSOR_EN)
