/*===========================================================================
 * Include file 
 *===========================================================================*/ 
#include "AHC_Common.h"
#include "ParkingModeCtrl.h"
#include <string.h>

/*===========================================================================
 * Macro define
 *===========================================================================*/ 



/*===========================================================================
 * Global varible
 *===========================================================================*/ 

static PARKING_STATE_INFO   ParkingModeInfo;

/*===========================================================================
 * Extern function
 *===========================================================================*/ 



/*===========================================================================
 * Main body
 *===========================================================================*/ 
#if 0
void _____ParkingMode_Function_________(){ruturn;} //dummy
#endif

void uiSetParkingModeStateInit(void)
{
    ParkingModeInfo.bParkingModeConfirmMenu = AHC_FALSE;
    ParkingModeInfo.bParkingMode = AHC_FALSE;
    ParkingModeInfo.bRecordVideo = AHC_FALSE;
    ParkingModeInfo.bParkingIdling = AHC_FALSE;
    ParkingModeInfo.bParkingExitState = AHC_FALSE;
    ParkingModeInfo.bAutoStopParkingMode = AHC_FALSE;
    ParkingModeInfo.bParkingMoveHintMenu = AHC_FALSE;
    ParkingModeInfo.bParkingSpaceState = AHC_FALSE;
    ParkingModeInfo.bParkingStartDrawed = AHC_FALSE;
}

void uiSetParkingModeEnable(UINT8 enable)
{
    ParkingModeInfo.bParkingIdling = AHC_FALSE;
    ParkingModeInfo.bParkingMode   = enable;
    ParkingModeInfo.bParkingModeConfirmMenu = AHC_FALSE;
    ParkingModeInfo.bParkingExitState = AHC_FALSE;
    ParkingModeInfo.bParkingSpaceState = AHC_FALSE;
    ParkingModeInfo.bParkingMoveHintMenu = AHC_FALSE;
    ParkingModeInfo.bParkingStartDrawed = AHC_FALSE;
}

UINT8 uiGetParkingModeEnable(void)
{
    return ParkingModeInfo.bParkingMode;
}

#if 0
void _____ParkingMode_Config_________(){ruturn;} //dummy
#endif

PARKLING_MODE_CFG sPatkingCfg = {AHC_TRUE, 
								 PARKING_MODE_TRIGGER_ENCODE_MOTION,
								 3000,
								 60000};

void uiSetParkingCfg(PARKLING_MODE_CFG *pConfig)
{
	memcpy(&sPatkingCfg, pConfig, sizeof(PARKLING_MODE_CFG));
}

PARKLING_MODE_CFG* uiGetParkingCfg(void)
{
	return (&sPatkingCfg);
}

