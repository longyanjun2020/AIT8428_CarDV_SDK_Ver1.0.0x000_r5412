//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmpf_typedef.h"
#include "config_fw.h"
#include "mmpf_pio.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define STR_VALUE(arg)  	#arg
#define _BSP_PAD_FILE(bsp) 	STR_VALUE(pad##bsp##.h)
#define BSP_PAD_FILE(bsp) 	_BSP_PAD_FILE(bsp)

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================

MMPF_PAD_INIT_DATA m_PioInitTable[] =
{
#include BSP_PAD_FILE(BSP_NAME)
    {MMP_PAD_MAX,    PAD_SCHMITT_TRIG | PAD_PULL_UP   | PAD_OUT_DRIVING(0)} // end of init table
};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void BSP_PIO_Init(void)
{
#if 0
    int i;

    for (i = 0; m_PioInitTable[i].ulPadPin != MMP_PAD_MAX; i++) {
        // Config pad
        MMPF_PIO_PadConfigEx(m_PioInitTable[i].ulPadPin, m_PioInitTable[i].ubPadConfig, MMP_FALSE);
    }
#endif    
}

