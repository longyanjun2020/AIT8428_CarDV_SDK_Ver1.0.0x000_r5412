//------------------------------------------------------------------------------
//
//  File        : clk_cfg.c
//  Description : Source file of clock frequency configuration
//  Author      : Alterman
//  Revision    : 1.0
//
//------------------------------------------------------------------------------

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "clk_cfg.h"
//#include "mmpf_dram.h"
#include "dram_cfg.h"
#if (CPU_ID == CPU_A)
#include "Customer_config.h"
#endif

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
/*
 * The following table decides the clock frequency of each clock domain.
 * The settings could seriously impact the system reliability & performance,
 * you MUST modifiy it carefully.
 *
 * The clock frequency of each group depends on the frequency of its source
 * (PLL or PMCLK), and its clock divider.
 */

//Notes: MCV_V2 EXT_CLK is 24Mhz
const MMPF_PLL_SETTINGS m_PllSettings[GRP_CLK_SRC_MAX] = 
{
    // M,      			N,      		DIV,        		Frac,	VCO,          		Freq}
    //DPLL0 400Hz
    #if ((DRAM_ID == DRAM_DDR)||(DRAM_ID == DRAM_DDR2)) && defined(ULTRA_LOW_POWER) // Ultra low power
    {DPLL0_M_DIV_1,	    DPLL0_N(25), 	DPLL0_P_DIV_1,	    0, 		DPLL012_VCO_TYPE2, 	600000},
    #else
    {DPLL0_M_DIV_3,	    DPLL0_N(50), 	DPLL0_P_DIV_2,	    0, 		DPLL012_VCO_TYPE3, 	400000},
	#endif
	
    //8428D use this seting for 540/3 for Dram.
    //{DPLL0_M_DIV_2,   DPLL0_N(45), 	DPLL0_P_DIV_1, 	    0, 		DPLL012_VCO_TYPE1, 	540000},
    #if (CUSTOMIZED_DPLL1_CLOCK)
    //DPLL1 594MHz (For SONY IMX322 sensor MCLK, it requests 297MHz / 8 = 37.125MHz)
    {DPLL0_M_DIV_4, 	DPLL2_N(99),	DPLL0_P_DIV_1, 	    0, 		DPLL012_VCO_TYPE4, 	594000},
    #else
    //DPLL1 360MHz
    {DPLL0_M_DIV_2, 	DPLL1_N(30), 	DPLL0_P_DIV_2, 	    0, 		DPLL012_VCO_TYPE2, 	360000},
    #endif
	
    //DPLL2 528MHz
	#if defined(ULTRA_LOW_POWER)// Ultra low power
	{DPLL0_M_DIV_3, 	DPLL2_N(50),	DPLL0_P_DIV_2, 	    0, 		DPLL012_VCO_TYPE3, 	400000},
	#else
	#if (CCIR656_OUTPUT_ENABLE)
    {DPLL0_M_DIV_2,  	DPLL2_N(45),	DPLL0_P_DIV_2, 		0, 		DPLL012_VCO_TYPE3, 	540000},
	#else
    {DPLL0_M_DIV_2, 	DPLL2_N(44),	DPLL0_P_DIV_2, 	    0, 		DPLL012_VCO_TYPE3, 	528000},
	#endif
	#endif
	
    //PMCLK
    {0, 				0, 				0, 					0, 		0, 					EXT_CLK},
    //DPLL3 24.576MHz for Audio
    {DPLL3_M(15), 		DPLL3_N(15), 	0, 					0, 		0, 					24576},
    //DPLL4 378MHz for TV
    {DPLL4_M(23), 		DPLL4_N(188), 	DPLL4_P_DIV_2, 		0, 		DPLL4_400MHZ, 		378000},
    //DPLL5 400MHz, for DRAM
    #if (CUSTOMIZED_DPLL1_CLOCK)
    {DPLL5_OUTPUT_DIV_2, DPLL5_N(20), 	DPLL5_P_DIV_1, 		0, 		DPLL5_800MHZ, 		480000}
    #else
	{DPLL5_OUTPUT_DIV_2, DPLL5_N(16), 	DPLL5_P_DIV_1, 		0, 		DPLL5_1200MHZ, 		400000}
    #endif
};


const GRP_CLK_CFG gGrpClkCfg[CLK_GRP_NUM] = {
/*    src    div  */
    { DPLL2, 2 }, // CLK_GRP_GBL    		264MHz
    { DPLL2, 1 }, // CLK_GRP_CPUA   		528MHz
    { DPLL2, 1 }, // CLK_GRP_CPUB   		528MHz
#if (DRAM_ID == DRAM_DDR) || (DRAM_ID == DRAM_DDR2)
    #if  defined(ULTRA_LOW_POWER) // Ultra low power
    { DPLL0, 3 }, // CLK_GRP_DRAM   		200MHz/180MHz
    #else
    { DPLL0, 2 }, // CLK_GRP_DRAM   		200MHz/180MHz
    #endif
#else //(DRAM_ID == DRAM_DDR3)
    { DPLL0, 1 }, // CLK_GRP_DRAM   		400MHz
#endif
    { DPLL3, 1 }, // CLK_GRP_AUD 			24.576MHz (may be changed in driver)
#if (CUSTOMIZED_DPLL1_CLOCK == 1)
    { DPLL1, 2 }, // CLK_GRP_SNR    		297MHz (For SONY IMX322 sensor MCLK, it requests 297MHz / 8 = 37.125MHz)
#else
	#if ((DRAM_ID == DRAM_DDR)||(DRAM_ID == DRAM_DDR2)) && defined(ULTRA_LOW_POWER) // Ultra low power
    { DPLL0, 2 }, // CLK_GRP_SNR    		300MHz
    #else
	{ DPLL0, 1 }, // CLK_GRP_SNR    		400MHz
    #endif
#endif
#if (HDR_FOV_ENLARGE || MENU_MOVIE_SIZE_1080_60P_EN)
    { DPLL2, 1 }, // CLK_GRP_BAYER  		528MHz
    { DPLL0, 1 }, // CLK_GRP_COLOR(SCALE)  	400MHz 
#else
	#if ((DRAM_ID == DRAM_DDR)||(DRAM_ID == DRAM_DDR2)) && defined(ULTRA_LOW_POWER)// Ultra low power
	{ DPLL0, 2 }, // CLK_GRP_BAYER  		300MHz
    { DPLL0, 2 }, // CLK_GRP_COLOR(SCALE)  	300MHz
    #else
    { DPLL0, 1 }, // CLK_GRP_BAYER  		400MHz
    { DPLL0, 1 }, // CLK_GRP_COLOR(SCALE)  	400MHz
    #endif
#endif    
    { PMCLK, 1 }, // CLK_GRP_USB     		24MHz
    { DPLL4, 7 }, // CLK_GRP_TV      		54MHz
    { DPLL4, 2 }, // CLK_GRP_HDMI    		24MHz (may be changed in driver)
    { PMCLK, 1 }, // CLK_GRP_MIPITX  		24MHz
    { PMCLK, 1 }, // CLK_GRP_RXBIST  		24MHz
};

