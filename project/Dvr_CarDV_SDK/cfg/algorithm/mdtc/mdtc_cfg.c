//------------------------------------------------------------------------------
//
//  File        : mdtc_cfg.c
//  Description : Source file of motion detection configuration
//  Author      : Alterman
//  Revision    : 0.0
//
//------------------------------------------------------------------------------

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mdtc_cfg.h"

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
/*
 * Configure of (Resolution of source image, detect windows , Gap of frames to do motion detection)
 */
MDTC_CFG    gstMdtcCfg[2] = {
	//src width		//src heigth	//Gap			//CpuX 		// x_lt		y_lt	x_rb			y_rb				x_div				y_div
	//PRM_SENSOR
	{MDTC_WIDTH, 	MDTC_HEIGHT,	MDTC_FRAME_GAP,		1,		0,          0,	   	MDTC_WIDTH - 1, MDTC_HEIGHT - 1,    MDTC_WIN_W_DIV,     MDTC_WIN_H_DIV},

	//SCD_SENSOR
	{REAR_MDTC_WIDTH, 	REAR_MDTC_HEIGHT,		3,	 	0,		0,          0,	REAR_MDTC_WIDTH - 1, REAR_MDTC_HEIGHT - 1,  MDTC_WIN_W_DIV,  MDTC_WIN_H_DIV},
};

/*
 * Input parameters of detect windows
 */
MD_params_in_t  gstMdtcWinParam[MDTC_WIN_W_DIV][MDTC_WIN_H_DIV] = {
    {
        1,      // enable
        4,      // size_perct_thd_min
        100,    // size_perct_thd_max
        17,     // sensitivity
        500,    // learn_rate
    },
};

/*
 * Output results of detect windows
 */
MD_params_out_t gstMdtcWinResult[MDTC_WIN_W_DIV][MDTC_WIN_H_DIV] = {
    {
        0,      // md_result
        0       // obj_cnt
    },
};

