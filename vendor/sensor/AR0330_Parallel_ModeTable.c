#if (SENSOR_IF == SENSOR_IF_PARALLEL)

MMPF_SENSOR_RESOLUTION m_SensorRes = 
{
	4,				// ubSensorModeNum
	0,				// ubDefPreviewMode
	1,				// ubDefCaptureMode
	2200,           // usPixelSize
//  Mode0   Mode1   Mode2   Mode3
    {1,     1,      1,      1},     // usVifGrabStX
    {4,     4,      4,       4},	/* usVifGrabStY */
    {2308,  2056,   1928,   1284},	// usVifGrabW
    {1300,  1544,   1096,   724},	// usVifGrabH
#if (CHIP == MCR_V2)   
    {1,     1,      1,      1},     // usBayerInGrabX
    {1,     1,      1,      1},     // usBayerInGrabY
    {4,     8,      8,      4},     // usBayerInDummyX
    {4,     8,      16,     4},	    // usBayerInDummyY
    {2304,  2048,   1920,   1280},	// usBayerOutW
    {1296,  1536,   1080,   720},	// usBayerOutH
#endif
    {2304,  2048,   1920,   1280},	// usScalInputW
    {1296,  1536,   1080,   720},	// usScalInputH 
#if SUPPORT_27_5FPS
    {275,    250,    275,     600}, // usTargetFpsx10    
    {1451,  1578,   1430,     811},	// usVsyncLine
#else
    {300,    250,    300,     600}, // usTargetFpsx10
    {1329,  1578,   1314,     811},	// usVsyncLine
#endif
    {1,     1,      1,      1},     // ubWBinningN
    {1,     1,      1,      1},     // ubWBinningN
    {1,     1,      1,      1},     // ubWBinningN
    {1,     1,      1,      1},     // ubWBinningN
    {0xFF,	0xFF,	0xFF,   0xFF},  // ubCustIQmode
    {0xFF,  0xFF,	0xFF,   0xFF}   // ubCustAEmode
};

#if 0
void ____Sensor_Init_OPR_Table____(){ruturn;} //dummy
#endif

MMP_USHORT SNR_AR0330_PARALLEL_Reg_Init_V1_Customer[] = 
{
	//Table 2. Recommend default register and Sequencer
	0x30BA, 0x2C  , 								   
	0x30FE, 0x0080, 								   
	0x31E0, 0x0003, 								   
	0x3ECE, 0xFF  , 								   
	0x3ED0, 0xE4F6, 								   
	0x3ED2, 0x0146, 								   
	0x3ED4, 0x8F6C, 								   
	0x3ED6, 0x66CC, 								   
	0x3ED8, 0x8C42, 								   
	0x3EDA, 0x8822, 								   
	0x3EDC, 0x2222, 								   
	0x3EDE, 0x22C0, 								   
	0x3EE0, 0x1500, 								   
	0x3EE6, 0x0080, 								   
	0x3EE8, 0x2027, 								   
	0x3EEA, 0x001D, 								   
	0x3F06, 0x046A, 								   
	0x305E, 0x00A0, 								   
													   
	//Sequencer A	
	0x301A, 0x0058,     // Disable streaming
	SNR_REG_DELAY, 10,
	
	0x3088, 0x8000,
	0x3086, 0x4540,
	0x3086, 0x6134,
	0x3086, 0x4A31,
	0x3086, 0x4342,
	0x3086, 0x4560,
	0x3086, 0x2714,
	0x3086, 0x3DFF,
	0x3086, 0x3DFF,
	0x3086, 0x3DEA,
	0x3086, 0x2704,
	0x3086, 0x3D10,
	0x3086, 0x2705,
	0x3086, 0x3D10,
	0x3086, 0x2715,
	0x3086, 0x3527,
	0x3086, 0x053D,
	0x3086, 0x1045,
	0x3086, 0x4027,
	0x3086, 0x0427,
	0x3086, 0x143D,
	0x3086, 0xFF3D,
	0x3086, 0xFF3D,
	0x3086, 0xEA62,
	0x3086, 0x2728,
	0x3086, 0x3627,
	0x3086, 0x083D,
	0x3086, 0x6444,
	0x3086, 0x2C2C,
	0x3086, 0x2C2C,
	0x3086, 0x4B01,
	0x3086, 0x432D,
	0x3086, 0x4643,
	0x3086, 0x1647,
	0x3086, 0x435F,
	0x3086, 0x4F50,
	0x3086, 0x2604,
	0x3086, 0x2684,
	0x3086, 0x2027,
	0x3086, 0xFC53,
	0x3086, 0x0D5C,
	0x3086, 0x0D60,
	0x3086, 0x5754,
	0x3086, 0x1709,
	0x3086, 0x5556,
	0x3086, 0x4917,
	0x3086, 0x145C,
	0x3086, 0x0945,
	0x3086, 0x0045,
	0x3086, 0x8026,
	0x3086, 0xA627,
	0x3086, 0xF817,
	0x3086, 0x0227,
	0x3086, 0xFA5C,
	0x3086, 0x0B5F,
	0x3086, 0x5307,
	0x3086, 0x5302,
	0x3086, 0x4D28,
	0x3086, 0x6C4C,
	0x3086, 0x0928,
	0x3086, 0x2C28,
	0x3086, 0x294E,
	0x3086, 0x1718,
	0x3086, 0x26A2,
	0x3086, 0x5C03,
	0x3086, 0x1744,
	0x3086, 0x2809,
	0x3086, 0x27F2,
	0x3086, 0x1714,
	0x3086, 0x2808,
	0x3086, 0x164D,
	0x3086, 0x1A26,
	0x3086, 0x8317,
	0x3086, 0x0145,
	0x3086, 0xA017,
	0x3086, 0x0727,
	0x3086, 0xF317,
	0x3086, 0x2945,
	0x3086, 0x8017,
	0x3086, 0x0827,
	0x3086, 0xF217,
	0x3086, 0x285D,
	0x3086, 0x27FA,
	0x3086, 0x170E,
	0x3086, 0x2681,
	0x3086, 0x5300,
	0x3086, 0x17E6,
	0x3086, 0x5302,
	0x3086, 0x1710,
	0x3086, 0x2683,
	0x3086, 0x2682,
	0x3086, 0x4827,
	0x3086, 0xF24D,
	0x3086, 0x4E28,
	0x3086, 0x094C,
	0x3086, 0x0B17,
	0x3086, 0x6D28,
	0x3086, 0x0817,
	0x3086, 0x014D,
	0x3086, 0x1A17,
	0x3086, 0x0126,
	0x3086, 0x035C,
	0x3086, 0x0045,
	0x3086, 0x4027,
	0x3086, 0x9017,
	0x3086, 0x2A4A,
	0x3086, 0x0A43,
	0x3086, 0x160B,
	0x3086, 0x4327,
	0x3086, 0x9445,
	0x3086, 0x6017,
	0x3086, 0x0727,
	0x3086, 0x9517,
	0x3086, 0x2545,
	0x3086, 0x4017,
	0x3086, 0x0827,
	0x3086, 0x905D,
	0x3086, 0x2808,
	0x3086, 0x530D,
	0x3086, 0x2645,
	0x3086, 0x5C01,
	0x3086, 0x2798,
	0x3086, 0x4B12,
	0x3086, 0x4452,
	0x3086, 0x5117,
	0x3086, 0x0260,
	0x3086, 0x184A,
	0x3086, 0x0343,
	0x3086, 0x1604,
	0x3086, 0x4316,
	0x3086, 0x5843,
	0x3086, 0x1659,
	0x3086, 0x4316,
	0x3086, 0x5A43,
	0x3086, 0x165B,
	0x3086, 0x4327,
	0x3086, 0x9C45,
	0x3086, 0x6017,
	0x3086, 0x0727,
	0x3086, 0x9D17,
	0x3086, 0x2545,
	0x3086, 0x4017,
	0x3086, 0x1027,
	0x3086, 0x9817,
	0x3086, 0x2022,
	0x3086, 0x4B12,
	0x3086, 0x442C,
	0x3086, 0x2C2C,
	0x3086, 0x2C00,
									   
	0x301A, 0x0004,
	
	0x301A, 0x0059, 	// RESET_REGISTER
	SNR_REG_DELAY,    10,  // delay = 100

	0x3052,0xA114,  	// Modified for OTPM cannot be access at low temperature
	0X304A,0x0070,  	// Modified for OTPM cannot be access at low temperature

	SNR_REG_DELAY,    10,   // delay = 50
	0x301A, 0x10D8, 	// RESET_REGISTER = 4312

	0x302A,      6,		// VT_PIX_CLK_DIV = 6
	0x302C,      1,		// VT_SYS_CLK_DIV = 1
	0x302E,      2,		// PRE_PLL_CLK_DIV = 2
	0x3030,     49,		// PLL_MULTIPLIER = 49
	0x3036,     12,		// OP_PIX_CLK_DIV = 12
	0x3038,      1, 	// OP_SYS_CLK_DIV = 1
	0x31AC,   3084, 	// DATA_FORMAT_BITS = 3084
	0x301A,   4316,		// RESET_REGISTER = 4316

	0x31AE, 0x0301,		// SERIAL_FORMAT = 769

	0x3002,    234,		// Y_ADDR_START = 234
	0x3004,    198,		// X_ADDR_START = 198
	0x3006,   1313+16,	// Y_ADDR_END = 1313
	0x3008,   2117+8,	// X_ADDR_END = 2117
	0x300A,   1315,		// FRAME_LENGTH_LINES = 1315
	0x300C,   1242,		// LINE_LENGTH_PCK = 1242
	0x3012,    657,		// COARSE_INTEGRATION_TIME = 657
	0x3014,      0,		// FINE_INTEGRATION_TIME = 0
	0x30A2,      1,		// X_ODD_INC = 1
	0x30A6,      1,		// Y_ODD_INC = 1
	0x308C,      6,		// Y_ADDR_START_CB = 6
	0x308A,      6,		// X_ADDR_START_CB = 6
	0x3090,   1541,		// Y_ADDR_END_CB = 1541
	0x308E,   2309,		// X_ADDR_END_CB = 2309
	0x30AA,   1570,		// FRAME_LENGTH_LINES_CB = 1570
	0x303E,   1248,		// LINE_LENGTH_PCK_CB = 1248
	0x3016,   1569,		// COARSE_INTEGRATION_TIME_CB = 1569
	0x3018,      0,		// FINE_INTEGRATION_TIME_CB = 0
	0x30AE,      1,		// X_ODD_INC_CB = 1
	0x30A8,      1,		// Y_ODD_INC_CB = 1
    #if SENSOR_ROTATE_180
	0x3040, 0xC000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
    #else
	0x3040, 0x0000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	#endif
	0x3042,    103,		// EXTRA_DELAY = 103

	//0x30BA, 0x002C,  	// DIGITAL_CTRL = 44
	0x3088, 0x80BA,  	// SEQ_CTRL_PORT = 32954
	0x3086, 0xE653,  	// SEQ_DATA_PORT = 58963
	0x306E, 0xFC10,  

	0xFFFF,    10   	// delay = 100
};
MMP_USHORT SNR_AR0330_PARALLEL_Reg_Init_V2_Customer[] =
{
	
	//Table 2. Recommend default register and Sequencer
	0x30BA, 0x2C  , 								   
	0x30FE, 0x0080, 								   
	0x31E0, 0x0003, 								   
	0x3ECE, 0xFF  , 								   
	0x3ED0, 0xE4F6, 								   
	0x3ED2, 0x0146, 								   
	0x3ED4, 0x8F6C, 								   
	0x3ED6, 0x66CC, 								   
	0x3ED8, 0x8C42, 								   
	0x3EDA, 0x889B, 								   
	0x3EDC, 0x8863, 								   
	0x3EDE, 0xAA04, 								   
	0x3EE0, 0x15F0, 								   
	0x3EE6, 0x008C, 								   
	0x3EE8, 0x2024, 								   
	0x3EEA, 0xFF1F, 								   
	0x3F06, 0x046A, 								   
	0x305E, 0x00A0, 								   
													   
	//Sequencer B	
	0x301A, 0x0058,     // Disable streaming
	SNR_REG_DELAY, 10,
	0x3088, 0x8000, 								   
	0x3086, 0x4A03, 								   
	0x3086, 0x4316, 								   
	0x3086, 0x0443, 								   
	0x3086, 0x1645, 								   
	0x3086, 0x4045, 								   
	0x3086, 0x6017, 								   
	0x3086, 0x2045, 								   
	0x3086, 0x404B, 								   
	0x3086, 0x1244, 								   
	0x3086, 0x6134, 								   
	0x3086, 0x4A31, 								   
	0x3086, 0x4342, 								   
	0x3086, 0x4560, 								   
	0x3086, 0x2714, 								   
	0x3086, 0x3DFF, 								   
	0x3086, 0x3DFF, 								   
	0x3086, 0x3DEA, 								   
	0x3086, 0x2704, 								   
	0x3086, 0x3D10, 								   
	0x3086, 0x2705, 								   
	0x3086, 0x3D10, 								   
	0x3086, 0x2715, 								   
	0x3086, 0x3527, 								   
	0x3086, 0x053D, 								   
	0x3086, 0x1045, 								   
	0x3086, 0x4027, 								   
	0x3086, 0x0427, 								   
	0x3086, 0x143D, 								   
	0x3086, 0xFF3D, 								   
	0x3086, 0xFF3D, 								   
	0x3086, 0xEA62, 								   
	0x3086, 0x2728, 								   
	0x3086, 0x3627, 								   
	0x3086, 0x083D, 								   
	0x3086, 0x6444, 								   
	0x3086, 0x2C2C, 								   
	0x3086, 0x2C2C, 								   
	0x3086, 0x4B01, 								   
	0x3086, 0x432D, 								   
	0x3086, 0x4643, 								   
	0x3086, 0x1647, 								   
	0x3086, 0x435F, 								   
	0x3086, 0x4F50, 								   
	0x3086, 0x2604, 								   
	0x3086, 0x2684, 								   
	0x3086, 0x2027, 								   
	0x3086, 0xFC53, 								   
	0x3086, 0x0D5C, 								   
	0x3086, 0x0D57, 								   
	0x3086, 0x5417, 								   
	0x3086, 0x0955, 								   
	0x3086, 0x5649, 								   
	0x3086, 0x5307, 								   
	0x3086, 0x5302, 								   
	0x3086, 0x4D28, 								   
	0x3086, 0x6C4C, 								   
	0x3086, 0x0928, 								   
	0x3086, 0x2C28, 								   
	0x3086, 0x294E, 								   
	0x3086, 0x5C09, 								   
	0x3086, 0x6045, 								   
	0x3086, 0x0045, 								   
	0x3086, 0x8026, 								   
	0x3086, 0xA627, 								   
	0x3086, 0xF817, 								   
	0x3086, 0x0227, 								   
	0x3086, 0xFA5C, 								   
	0x3086, 0x0B17, 								   
	0x3086, 0x1826, 								   
	0x3086, 0xA25C, 								   
	0x3086, 0x0317, 								   
	0x3086, 0x4427, 								   
	0x3086, 0xF25F, 								   
	0x3086, 0x2809, 								   
	0x3086, 0x1714, 								   
	0x3086, 0x2808, 								   
	0x3086, 0x1701, 								   
	0x3086, 0x4D1A, 								   
	0x3086, 0x2683, 								   
	0x3086, 0x1701, 								   
	0x3086, 0x27FA, 								   
	0x3086, 0x45A0, 								   
	0x3086, 0x1707, 								   
	0x3086, 0x27FB, 								   
	0x3086, 0x1729, 								   
	0x3086, 0x4580, 								   
	0x3086, 0x1708, 								   
	0x3086, 0x27FA, 								   
	0x3086, 0x1728, 								   
	0x3086, 0x5D17, 								   
	0x3086, 0x0E26, 								   
	0x3086, 0x8153, 								   
	0x3086, 0x0117, 								   
	0x3086, 0xE653, 								   
	0x3086, 0x0217, 								   
	0x3086, 0x1026, 								   
	0x3086, 0x8326, 								   
	0x3086, 0x8248, 								   
	0x3086, 0x4D4E, 								   
	0x3086, 0x2809, 								   
	0x3086, 0x4C0B, 								   
	0x3086, 0x6017, 								   
	0x3086, 0x2027, 								   
	0x3086, 0xF217, 								   
	0x3086, 0x535F, 								   
	0x3086, 0x2808, 								   
	0x3086, 0x164D, 								   
	0x3086, 0x1A17, 								   
	0x3086, 0x0127, 								   
	0x3086, 0xFA26, 								   
	0x3086, 0x035C, 								   
	0x3086, 0x0145, 								   
	0x3086, 0x4027, 								   
	0x3086, 0x9817, 								   
	0x3086, 0x2A4A, 								   
	0x3086, 0x0A43, 								   
	0x3086, 0x160B, 								   
	0x3086, 0x4327, 								   
	0x3086, 0x9C45, 								   
	0x3086, 0x6017, 								   
	0x3086, 0x0727, 								   
	0x3086, 0x9D17, 								   
	0x3086, 0x2545, 								   
	0x3086, 0x4017, 								   
	0x3086, 0x0827, 								   
	0x3086, 0x985D, 								   
	0x3086, 0x2645, 								   
	0x3086, 0x5C01, 								   
	0x3086, 0x4B17, 								   
	0x3086, 0x0A28, 								   
	0x3086, 0x0853, 								   
	0x3086, 0x0D52, 								   
	0x3086, 0x5112, 								   
	0x3086, 0x4460, 								   
	0x3086, 0x184A, 								   
	0x3086, 0x0343, 								   
	0x3086, 0x1604, 								   
	0x3086, 0x4316, 								   
	0x3086, 0x5843, 								   
	0x3086, 0x1659, 								   
	0x3086, 0x4316, 								   
	0x3086, 0x5A43, 								   
	0x3086, 0x165B, 								   
	0x3086, 0x4345, 								   
	0x3086, 0x4027, 								   
	0x3086, 0x9C45, 								   
	0x3086, 0x6017, 								   
	0x3086, 0x0727, 								   
	0x3086, 0x9D17, 								   
	0x3086, 0x2545, 								   
	0x3086, 0x4017, 								   
	0x3086, 0x1027, 								   
	0x3086, 0x9817, 								   
	0x3086, 0x2022, 								   
	0x3086, 0x4B12, 								   
	0x3086, 0x442C, 								   
	0x3086, 0x2C2C, 								   
	0x3086, 0x2C00, 								   
	0x3086, 0x0000, 								   
	0x301A, 0x0004,
	
	0x301A, 0x0059, 	// RESET_REGISTER
	SNR_REG_DELAY,    10,  // delay = 100

	0x3052,0xA114,  	// Modified for OTPM cannot be access at low temperature
	0X304A,0x0070,  	// Modified for OTPM cannot be access at low temperature

	SNR_REG_DELAY,    10,   // delay = 50
	0x301A, 0x10D8, 	// RESET_REGISTER = 4312

	0x302A,      6,		// VT_PIX_CLK_DIV = 6
	0x302C,      1,		// VT_SYS_CLK_DIV = 1
	0x302E,      2,		// PRE_PLL_CLK_DIV = 2
	0x3030,     49,		// PLL_MULTIPLIER = 49
	0x3036,     12,		// OP_PIX_CLK_DIV = 12
	0x3038,      1, 	// OP_SYS_CLK_DIV = 1
	0x31AC,   3084, 	// DATA_FORMAT_BITS = 3084
	0x301A,   4316,		// RESET_REGISTER = 4316

	0x31AE, 0x0301,		// SERIAL_FORMAT = 769

	0x3002,    234,		// Y_ADDR_START = 234
	0x3004,    198,		// X_ADDR_START = 198
	0x3006,   1313+16,	// Y_ADDR_END = 1313
	0x3008,   2117+8,	// X_ADDR_END = 2117
	0x300A,   1315,		// FRAME_LENGTH_LINES = 1315
	0x300C,   1242,		// LINE_LENGTH_PCK = 1242
	0x3012,    657,		// COARSE_INTEGRATION_TIME = 657
	0x3014,      0,		// FINE_INTEGRATION_TIME = 0
	0x30A2,      1,		// X_ODD_INC = 1
	0x30A6,      1,		// Y_ODD_INC = 1
	0x308C,      6,		// Y_ADDR_START_CB = 6
	0x308A,      6,		// X_ADDR_START_CB = 6
	0x3090,   1541,		// Y_ADDR_END_CB = 1541
	0x308E,   2309,		// X_ADDR_END_CB = 2309
	0x30AA,   1570,		// FRAME_LENGTH_LINES_CB = 1570
	0x303E,   1248,		// LINE_LENGTH_PCK_CB = 1248
	0x3016,   1569,		// COARSE_INTEGRATION_TIME_CB = 1569
	0x3018,      0,		// FINE_INTEGRATION_TIME_CB = 0
	0x30AE,      1,		// X_ODD_INC_CB = 1
	0x30A8,      1,		// Y_ODD_INC_CB = 1
    #if SENSOR_ROTATE_180
	0x3040, 0xC000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
    #else
	0x3040, 0x0000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	#endif
	0x3042,    103,		// EXTRA_DELAY = 103

	//0x30BA, 0x002C,  	// DIGITAL_CTRL = 44
	0x3088, 0x80BA,  	// SEQ_CTRL_PORT = 32954
	0x3086, 0xE653,  	// SEQ_DATA_PORT = 58963
	0x306E, 0xFC10,  

	0xFFFF,    10   	// delay = 100

};

MMP_USHORT SNR_AR0330_PARALLEL_Reg_Init_V3_Customer[] =
{
	
	//Table 2. Recommend default register and Sequencer
	0x31E0, 0x0003,
	0x3ED2, 0x0146,
	0x3ED4, 0x8F6C,
	0x3ED6, 0x66CC,
    0x3ED8, 0x8C42,
	0x3EDA, 0x88BC,
	0x3EDC, 0xAA63,
	0x305E, 0x00A0,
	0x3046, 0x4038,
	0x3048, 0x8480,
	
	//Sequencer Patch 1
	0x301A, 0x0058,     // Disable streaming
	
	//0x301A, 0x10DD, 	// RESET_REGISTER
	SNR_REG_DELAY,    10,  // delay = 100

	0x3052,0xA114,  	// Modified for OTPM cannot be access at low temperature
	0X304A,0x0070,  	// Modified for OTPM cannot be access at low temperature

	SNR_REG_DELAY,    10,   // delay = 50
	0x301A, 0x10D8, 	// RESET_REGISTER = 4312

	0x302A,      6,		// VT_PIX_CLK_DIV = 6
	0x302C,      1,		// VT_SYS_CLK_DIV = 1
	0x302E,      2,		// PRE_PLL_CLK_DIV = 2
	0x3030,     49,		// PLL_MULTIPLIER = 49
	0x3036,     12,		// OP_PIX_CLK_DIV = 12
	0x3038,      1, 	// OP_SYS_CLK_DIV = 1
	0x31AC,   3084, 	// DATA_FORMAT_BITS = 3084
	0x301A,   4316,		// RESET_REGISTER = 4316

	0x31AE, 0x0301,		// SERIAL_FORMAT = 769

	0x3002,    234,		// Y_ADDR_START = 234
	0x3004,    198,		// X_ADDR_START = 198
	0x3006,   1313+16,	// Y_ADDR_END = 1313
	0x3008,   2117+8,	// X_ADDR_END = 2117
	0x300A,   1315,		// FRAME_LENGTH_LINES = 1315
	0x300C,   1242,		// LINE_LENGTH_PCK = 1242
	0x3012,    657,		// COARSE_INTEGRATION_TIME = 657
	0x3014,      0,		// FINE_INTEGRATION_TIME = 0
	0x30A2,      1,		// X_ODD_INC = 1
	0x30A6,      1,		// Y_ODD_INC = 1
	0x308C,      6,		// Y_ADDR_START_CB = 6
	0x308A,      6,		// X_ADDR_START_CB = 6
	0x3090,   1541,		// Y_ADDR_END_CB = 1541
	0x308E,   2309,		// X_ADDR_END_CB = 2309
	0x30AA,   1570,		// FRAME_LENGTH_LINES_CB = 1570
	0x303E,   1248,		// LINE_LENGTH_PCK_CB = 1248
	0x3016,   1569,		// COARSE_INTEGRATION_TIME_CB = 1569
	0x3018,      0,		// FINE_INTEGRATION_TIME_CB = 0
	0x30AE,      1,		// X_ODD_INC_CB = 1
	0x30A8,      1,		// Y_ODD_INC_CB = 1
    #if SENSOR_ROTATE_180
	0x3040, 0xC000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
    #else
	0x3040, 0x0000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	#endif
	0x3042,    103,		// EXTRA_DELAY = 103

	//0x30BA, 0x002C,  	// DIGITAL_CTRL = 44
	0x3088, 0x80BA,  	// SEQ_CTRL_PORT = 32954
	0x3086, 0xE653,  	// SEQ_DATA_PORT = 58963
	0x306E, 0xFC10,  

	0xFFFF,    10   	// delay = 100

};

MMP_USHORT SNR_AR0330_PARALLEL_Reg_Init_V4_Customer[] =
{
	//Table 2. Recommend default register and Sequencer
	0x31E0, 0x0003,
	0x3ED2, 0x0146,
	0x3ED6, 0x66CC,
    0x3ED8, 0x8C42,
	0x3EDA, 0x88BC,
	0x3EDC, 0xAA63,
	0x305E, 0x00A0,
	0x3046, 0x4038,
	0x3048, 0x8480,
	
	0x301A, 0x0059, 	// RESET_REGISTER
	SNR_REG_DELAY,    10,  // delay = 100

	0x3052,0xA114,  	// Modified for OTPM cannot be access at low temperature
	0X304A,0x0070,  	// Modified for OTPM cannot be access at low temperature

	SNR_REG_DELAY,    10,   // delay = 50
	0x301A, 0x10D8, 	// RESET_REGISTER = 4312

	0x302A,      6,		// VT_PIX_CLK_DIV = 6
	0x302C,      1,		// VT_SYS_CLK_DIV = 1
	0x302E,      2,		// PRE_PLL_CLK_DIV = 2
	0x3030,     49,		// PLL_MULTIPLIER = 49
	0x3036,     12,		// OP_PIX_CLK_DIV = 12
	0x3038,      1, 	// OP_SYS_CLK_DIV = 1
	0x31AC,   3084, 	// DATA_FORMAT_BITS = 3084
	0x301A,   4316,		// RESET_REGISTER = 4316

	0x31AE, 0x0301,		// SERIAL_FORMAT = 769

	0x3002,    234,		// Y_ADDR_START = 234
	0x3004,    198,		// X_ADDR_START = 198
	0x3006,   1313+16,	// Y_ADDR_END = 1313
	0x3008,   2117+8,	// X_ADDR_END = 2117
	0x300A,   1315,		// FRAME_LENGTH_LINES = 1315
	0x300C,   1242,		// LINE_LENGTH_PCK = 1242
	0x3012,    657,		// COARSE_INTEGRATION_TIME = 657
	0x3014,      0,		// FINE_INTEGRATION_TIME = 0
	0x30A2,      1,		// X_ODD_INC = 1
	0x30A6,      1,		// Y_ODD_INC = 1
	0x308C,      6,		// Y_ADDR_START_CB = 6
	0x308A,      6,		// X_ADDR_START_CB = 6
	0x3090,   1541,		// Y_ADDR_END_CB = 1541
	0x308E,   2309,		// X_ADDR_END_CB = 2309
	0x30AA,   1570,		// FRAME_LENGTH_LINES_CB = 1570
	0x303E,   1248,		// LINE_LENGTH_PCK_CB = 1248
	0x3016,   1569,		// COARSE_INTEGRATION_TIME_CB = 1569
	0x3018,      0,		// FINE_INTEGRATION_TIME_CB = 0
	0x30AE,      1,		// X_ODD_INC_CB = 1
	0x30A8,      1,		// Y_ODD_INC_CB = 1
    #if SENSOR_ROTATE_180
	0x3040, 0xC000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
    #else
	0x3040, 0x0000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	#endif
	0x3042,    103,		// EXTRA_DELAY = 103

	//0x30BA, 0x002C,  	// DIGITAL_CTRL = 44
	0x3088, 0x80BA,  	// SEQ_CTRL_PORT = 32954
	0x3086, 0xE653,  	// SEQ_DATA_PORT = 58963
	0x306E, 0xFC10,  

	0xFFFF,    10   	// delay = 100

};

MMP_USHORT SNR_AR0330_PARALLEL_Reg_Init_V5_Customer[] =
{
	//Table 2. Recommend default register and Sequencer
	0x3ED2, 0x0146,
	0x3EDA, 0x88BC,
	0x3EDC, 0xAA63,
	0x305E, 0x00A0,
	0x3046, 0x4038,
	0x3048, 0x8480,
	
	0x301A, 0x0059, 	// RESET_REGISTER
	SNR_REG_DELAY,    10,  // delay = 100

	0x3052,0xA114,  	// Modified for OTPM cannot be access at low temperature
	0X304A,0x0070,  	// Modified for OTPM cannot be access at low temperature

	SNR_REG_DELAY,    10,   // delay = 50
	0x301A, 0x10D8, 	// RESET_REGISTER = 4312

	0x302A,      6,		// VT_PIX_CLK_DIV = 6
	0x302C,      1,		// VT_SYS_CLK_DIV = 1
	0x302E,      2,		// PRE_PLL_CLK_DIV = 2
	0x3030,     49,		// PLL_MULTIPLIER = 49
	0x3036,     12,		// OP_PIX_CLK_DIV = 12
	0x3038,      1, 	// OP_SYS_CLK_DIV = 1
	0x31AC,   3084, 	// DATA_FORMAT_BITS = 3084
	0x301A,   4316,		// RESET_REGISTER = 4316

	0x31AE, 0x0301,		// SERIAL_FORMAT = 769

	0x3002,    234,		// Y_ADDR_START = 234
	0x3004,    198,		// X_ADDR_START = 198
	0x3006,   1313+16,	// Y_ADDR_END = 1313
	0x3008,   2117+8,	// X_ADDR_END = 2117
	0x300A,   1315,		// FRAME_LENGTH_LINES = 1315
	0x300C,   1242,		// LINE_LENGTH_PCK = 1242
	0x3012,    657,		// COARSE_INTEGRATION_TIME = 657
	0x3014,      0,		// FINE_INTEGRATION_TIME = 0
	0x30A2,      1,		// X_ODD_INC = 1
	0x30A6,      1,		// Y_ODD_INC = 1
	0x308C,      6,		// Y_ADDR_START_CB = 6
	0x308A,      6,		// X_ADDR_START_CB = 6
	0x3090,   1541,		// Y_ADDR_END_CB = 1541
	0x308E,   2309,		// X_ADDR_END_CB = 2309
	0x30AA,   1570,		// FRAME_LENGTH_LINES_CB = 1570
	0x303E,   1248,		// LINE_LENGTH_PCK_CB = 1248
	0x3016,   1569,		// COARSE_INTEGRATION_TIME_CB = 1569
	0x3018,      0,		// FINE_INTEGRATION_TIME_CB = 0
	0x30AE,      1,		// X_ODD_INC_CB = 1
	0x30A8,      1,		// Y_ODD_INC_CB = 1
    #if SENSOR_ROTATE_180
	0x3040, 0xC000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
    #else
	0x3040, 0x0000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	#endif
	0x3042,    103,		// EXTRA_DELAY = 103

	//0x30BA, 0x002C,  	// DIGITAL_CTRL = 44
	0x3088, 0x80BA,  	// SEQ_CTRL_PORT = 32954
	0x3086, 0xE653,  	// SEQ_DATA_PORT = 58963
	0x306E, 0xFC10,  

	0xFFFF,    10   	// delay = 100

};


#if 0
void ____Sensor_Res_OPR_Table____(){ruturn;} //dummy
#endif

MMP_USHORT SNR_AR0330CS_Reg_1280x720_60fps_Customer[] = 
{
	0x301A, 0x10DC,  	// RESET_REGISTER
	0xFFFF, 100,
	0x301A, 0x10D8,  	// RESET_REGISTER
	0xFFFF, 100,
	0x31AE, 0x0301,  	// SERIAL_FORMAT
	0x3004,   384,	 	// X_ADDR_START
	0x3008,   384+1283,	// X_ADDR_END
	0x3002,   408,		// Y_ADDR_START
	0x3006,   408+723,	// Y_ADDR_END
	0x30A2,      1,		// X_ODD_INC
	0x30A6,      1,		// Y_ODD_INC
    #if SENSOR_ROTATE_180
	0x3040, 0xC000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	0x3040, 0xC000,  	// READ_MODE
	0x3040, 0xC000,  	// READ_MODE
    #else
	0x3040, 0x0000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	0x3040, 0x0000,  	// READ_MODE
	0x3040, 0x0000,  	// READ_MODE
	#endif
	0x300C,   1014,		// LINE_LENGTH_PCK
	0x300A,    788,		// FRAME_LENGTH_LINES
	0x3014,      0,		// FINE_INTEGRATION_TIME
	0x3012,    785,		// COARSE_INTEGRATION_TIME
	0x3042,    968,		// EXTRA_DELAY
	0x30BA, 0x006C,  	// DIGITAL_CTRL
	0x301A, 0x10D8,  	// RESET_REGISTER
	0xFFFF, 100,
	0x3088, 0x80BA,  	// SEQ_CTRL_PORT
	0x3086, 0x0253,  	// SEQ_DATA_PORT
	0x301A, 0x10DC,  	// RESET_REGISTER
	0x301A, 0x10DC,  	// RESET_REGISTER
	0xFFFF,    100   	// delay = 100
};

MMP_USHORT SNR_AR0330CS_Reg_2048x1536_25fps_Customer[] = 
{
	0x301A, 0x10DC, 	// RESET_REGISTER
    0xFFFF,    100,  	// delay = 100
	0x301A, 0x10D8,  	// RESET_REGISTER = 4312
	0xFFFF,    100,

	0x302A,      6,		// VT_PIX_CLK_DIV = 6
	0x302C,      1,		// VT_SYS_CLK_DIV = 1
	0x302E,      2,		// PRE_PLL_CLK_DIV = 2
	0x3030,     49,		// PLL_MULTIPLIER = 49
	0x3036,     12,		// OP_PIX_CLK_DIV = 12
	0x3038,      1,		// OP_SYS_CLK_DIV = 1
	0x31AC,   3084,		// DATA_FORMAT_BITS = 3084
	0x301A,   4316,	 	// RESET_REGISTER = 4316
	0xFFFF, 100,

	0x31AE, 0x0301,  	// SERIAL_FORMAT = 769

	0x3002,      6,		// Y_ADDR_START = 6
	0x3004,    134,		// X_ADDR_START = 134
	0x3006,   1541+8,	// Y_ADDR_END = 1541
	0x3008,   2181+8,	// X_ADDR_END = 2181
	0x300A,   1578,		// FRAME_LENGTH_LINES = 1578
	0x300C,   1242,		// LINE_LENGTH_PCK = 1242
	0x3012,    657,		// COARSE_INTEGRATION_TIME = 657
	0x3014,      0,		// FINE_INTEGRATION_TIME = 0
	0x30A2,      1,		// X_ODD_INC = 1
	0x30A6,      1,		// Y_ODD_INC = 1
	0x308C,      6,		// Y_ADDR_START_CB = 6
	0x308A,      6,		// X_ADDR_START_CB = 6
	0x3090,   1541+8,	// Y_ADDR_END_CB = 1541
	0x308E,   2309,		// X_ADDR_END_CB = 2309
	0x30AA,   1570,		// FRAME_LENGTH_LINES_CB = 1570
	0x303E,   1248,		// LINE_LENGTH_PCK_CB = 1248
	0x3016,   1569,		// COARSE_INTEGRATION_TIME_CB = 1569
	0x3018,      0,		// FINE_INTEGRATION_TIME_CB = 0
	0x30AE,      1,		// X_ODD_INC_CB = 1
	0x30A8,      1,		// Y_ODD_INC_CB = 1
    #if SENSOR_ROTATE_180
	0x3040, 0xC000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
    #else
	0x3040, 0x0000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	#endif
	0x3042,    124,		// EXTRA_DELAY = 124

	0x30BA, 0x002C,  	// DIGITAL_CTRL = 44
	0x3088, 0x80BA,  	// SEQ_CTRL_PORT = 32954
	0x3086, 0xE653, 	// SEQ_DATA_PORT = 58963

	0xFFFF,    100   	// delay = 100
};

MMP_USHORT SNR_AR0330CS_Reg_2304x1296_Customer[] = 
{
	0x301A, 0x10DC,  	// RESET_REGISTER
	0xFFFF, 10,
	0x301A, 0x10D8,  	// RESET_REGISTER = 4312
	0xFFFF, 10,

	0x302A,      6,		// VT_PIX_CLK_DIV = 6
	0x302C,      1,		// VT_SYS_CLK_DIV = 1
	0x302E,      2,		// PRE_PLL_CLK_DIV = 2
	0x3030,     51,		// PLL_MULTIPLIER = 49=>OK
	0x3036,     12,		// OP_PIX_CLK_DIV = 12
	0x3038,      1,		// OP_SYS_CLK_DIV = 1
	0x31AC,   3084,		// DATA_FORMAT_BITS = 3084
	0x301A,   4316,		// RESET_REGISTER = 4316
	0xFFFF, 10,

	0x31AE, 0x0301,  	// SERIAL_FORMAT = 769

	0x3002,    120,		// Y_ADDR_START = 234
	0x3004,    0,		// X_ADDR_START = 198
	0x3006,   1422,		// Y_ADDR_END = 1313
	0x3008,   2308,		// X_ADDR_END = 2117
	#if (SUPPORT_27_5FPS)
	0x300A,   1451,		// FRAME_LENGTH_LINES = 1315
	#else
	0x300A,   1330,		// FRAME_LENGTH_LINES = 1315
	#endif
	0x300C,   2308-1030,
	0x3012,    657,		// COARSE_INTEGRATION_TIME = 657
	0x3014,      0,		// FINE_INTEGRATION_TIME = 0
	0x30A2,      1,		// X_ODD_INC = 1
	0x30A6,      1,		// Y_ODD_INC = 1
	0x308C,      6,		// Y_ADDR_START_CB = 6
	0x308A,      6,		// X_ADDR_START_CB = 6
	0x3090,   1541,		// Y_ADDR_END_CB = 1541
	0x308E,   2309,		// X_ADDR_END_CB = 2309
	0x30AA,   1570,		// FRAME_LENGTH_LINES_CB = 1570
	0x303E,   1248,		// LINE_LENGTH_PCK_CB = 1248
	0x3016,   1569,		// COARSE_INTEGRATION_TIME_CB = 1569
	0x3018,      0,		// FINE_INTEGRATION_TIME_CB = 0
	0x30AE,      1,		// X_ODD_INC_CB = 1
	0x30A8,      1,	 	// Y_ODD_INC_CB = 1
    #if SENSOR_ROTATE_180
	0x3040, 0xC000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
    #else
	0x3040, 0x0000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	#endif
	0x3042,      0,		// EXTRA_DELAY

	0x30BA, 0x002C,  	// DIGITAL_CTRL = 44
	0x3088, 0x80BA,  	// SEQ_CTRL_PORT = 32954
	0x3086, 0xE653,  	// SEQ_DATA_PORT = 58963
	0x306E, 0xFC10,  
	///AR0330 defect correction
	0x31E0, 0x0741,
	0xFFFF,    10   	// delay = 100
};

// 1080p 30FPS
MMP_USHORT SNR_AR0330CS_Reg_1920x1080_Customer[] = 
{
	0x301A, 0x10DC,  	// RESET_REGISTER
	0x301A, 0x10D8,  	// RESET_REGISTER = 4312

	0x302A,      6,		// VT_PIX_CLK_DIV = 6
	0x302C,      1,		// VT_SYS_CLK_DIV = 1
	0x302E,      2,		// PRE_PLL_CLK_DIV = 2
	0x3030,     49,		// PLL_MULTIPLIER = 49
	0x3036,     12,		// OP_PIX_CLK_DIV = 12
	0x3038,      1,		// OP_SYS_CLK_DIV = 1
	0x31AC,   3084,		// DATA_FORMAT_BITS = 3084
	0x301A,   4316,		// RESET_REGISTER = 4316

	0x31AE, 0x0301,  	// SERIAL_FORMAT = 769
#if 0
	0x3002,    234,		// Y_ADDR_START = 234
	0x3004,    198,		// X_ADDR_START = 198
	0x3006,   1095+234,	// Y_ADDR_END = 1313
	0x3008,   1927+198,	// X_ADDR_END = 2117
#else
	0x3002,    220,		// Y_ADDR_START = 234
	0x3004,    184,		// X_ADDR_START = 198
	0x3006,   1096+220,	// Y_ADDR_END = 1313
	0x3008,   1936+184,	// X_ADDR_END = 2117
#endif

	#if (SUPPORT_27_5FPS)
	0x300A,   1430,		// FRAME_LENGTH_LINES = 1315
	#else
	0x300A,   1310,		// FRAME_LENGTH_LINES = 1315
	#endif
	0x300C,   1248,		// LINE_LENGTH_PCK = 1242

	0x3012,   1000,		// COARSE_INTEGRATION_TIME = 657
	0x3014,      0,		// FINE_INTEGRATION_TIME = 0
	0x30A2,      1,		// X_ODD_INC = 1
	0x30A6,      1,		// Y_ODD_INC = 1
	0x308C,      6,		// Y_ADDR_START_CB = 6
	0x308A,      6,		// X_ADDR_START_CB = 6
	0x3090,   1541,		// Y_ADDR_END_CB = 1541
	0x308E,   2309,		// X_ADDR_END_CB = 2309
	0x30AA,   1570,		// FRAME_LENGTH_LINES_CB = 1570
	0x303E,   1248,		// LINE_LENGTH_PCK_CB = 1248
	0x3016,   1569,		// COARSE_INTEGRATION_TIME_CB = 1569
	0x3018,      0,		// FINE_INTEGRATION_TIME_CB = 0
	0x30AE,      1,		// X_ODD_INC_CB = 1
	0x30A8,      1,		// Y_ODD_INC_CB = 1
    #if SENSOR_ROTATE_180
	0x3040, 0xC000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
    #else
	0x3040, 0x0000,	    // [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	#endif
	0x3042,    103,		// EXTRA_DELAY = 103

	0x30BA, 0x002C,  	// DIGITAL_CTRL = 44
	0x3088, 0x80BA,  	// SEQ_CTRL_PORT = 32954
	0x3086, 0xE653,  	// SEQ_DATA_PORT = 58963
	0x306E, 0xFC10,
	// AR0330 defect correction
	0x31E0, 0x0741,
	0xFFFF,    10   // delay = 100
};

#endif  // SENSOR_IF
