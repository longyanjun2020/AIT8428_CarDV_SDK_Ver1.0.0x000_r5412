#ifndef _DRAM_CFG_H_
#define _DRAM_CFG_H_

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================
#include "includes_fw.h"
#include "config_fw.h"
#include "mmpf_dram.h"

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================
//#define DRAM_DDR       (0)
//#define DRAM_DDR2      (1)
//#define DRAM_DDR3      (2)
//#define AUTO_DLL_LOCK   (1)
//Please make sure the mmpf layer library<core.a> use the same DRAM_ID when building 
#if defined (MCR_V2_32MB)      // defined in MCP
    #undef DRAM_ID
    #define DRAM_ID    (DRAM_DDR)
#elif defined (MCR_V2_Q_128MB) // defined in MCP
    #undef DRAM_ID
    #define DRAM_ID    (DRAM_DDR3)
    //if use 8x28Q 8x28G, change to COMMON_DRAM_SIZE_128MB
#else
    #undef DRAM_ID
    #define DRAM_ID    (DRAM_DDR)
#endif

//==============================================================================
//
//                              Definition
//
//==============================================================================
//#define DRAMID1      (0x00000000)
//#define DRAMID2      (0x00000001)
//#define DRAMID3      (0x00000010)
//#define DRAMIDNA     (0xFFFFFFFF)
#define INVALIDATEVALUE  (0xFFFFFFFF)
//Define initial values for DRR1
//#define SEARCH_DLY_UPBD_DDR1      (0x00000200)
//#define SEARCH_DLY_STEP_DDR1      (0x00000020) //32
//#define SEARCH_DLY_SUB_STEP_DDR1  (0x00000010) //16
//#define USE_LGT_DDR3_SETTING_DDR1 (0x00000000)
////Define initial values for DRR2
//#define SEARCH_DLY_UPBD_DDR2      (0x00000100)
//#define SEARCH_DLY_STEP_DDR2      (0x00000008)
//#define USE_LGT_DDR3_SETTING_DDR2 (0x00000000)
////Define initial values for DRR3
//#define SEARCH_DLY_UPBD_DDR3      (0x00000100)
//#define SEARCH_DLY_STEP_DDR3      (0x00000008)
//#define USE_LGT_DDR3_SETTING_DDR3 (0x00000001)


//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

//==============================================================================
//
//                              Extern Variable
//
//==============================================================================
//extern MMP_ULONG   DRAMID;
//extern MMP_ULONG   SEARCH_DLY_UPBD;
//extern MMP_ULONG   SEARCH_DLY_STEP;
//extern MMP_ULONG   SEARCH_DLY_SUB_STEP;
//extern MMP_ULONG   USE_LGT_DDR3_SETTING;

#if (AUTO_DRAM_LOCKCORE)
extern MMP_DRAM_CLK_DLY_SET m_delayTable[17];
//#if (DRAM_ID == DRAM_DDR)
extern MMPF_DRAM_DMA_PARAM m_DmaParam;
//#endif 
#endif // (AUTO_DRAM_LOCKCORE)

extern const MMPF_DRAM_SETTINGS m_DramSettings;
//==============================================================================
//
//                              API Declaration
//
//==============================================================================
void InitializeVariableForDram(void);

#endif //_DRAM_CFG_H_
