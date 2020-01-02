//------------------------------------------------------------------------------
//
//  File        : clk_cfg.h
//  Description : Header file of clock frequency configuration
//  Author      : Alterman
//  Revision    : 1.0
//
//------------------------------------------------------------------------------
#ifndef _CLK_CFG_H_
#define _CLK_CFG_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "includes_fw.h"
#include "mmpf_pll.h"
#include "mmp_reg_gbl.h"

#if (CPU_ID == CPU_A)
#include "Customer_config.h"
#endif

#if (BIND_SENSOR_IMX291 || BIND_SENSOR_IMX322)
#define CUSTOMIZED_DPLL1_CLOCK      (1)
#else
#define CUSTOMIZED_DPLL1_CLOCK      (0)
#endif

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct _GRP_CLK_CFG {
    GRP_CLK_SRC src;    ///< clock source
    MMP_UBYTE   div;    ///< clock divider
} GRP_CLK_CFG;

//==============================================================================
//
//                              EXTERN VARIABLES
//
//==============================================================================

extern const MMPF_PLL_SETTINGS m_PllSettings[GRP_CLK_SRC_MAX];
extern const GRP_CLK_CFG gGrpClkCfg[CLK_GRP_NUM];

#endif // _CLK_CFG_H_

