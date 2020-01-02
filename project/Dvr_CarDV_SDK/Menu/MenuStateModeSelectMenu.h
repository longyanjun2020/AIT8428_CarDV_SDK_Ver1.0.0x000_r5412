
#ifndef _MENU_STATE_MODE_SELECT_MENU_H_
#define _MENU_STATE_MODE_SELECT_MENU_H_

/*===========================================================================
 * Include files
 *===========================================================================*/ 

#include "AHC_Common.h"
#include "AHC_Gui.h"

/*===========================================================================
 * Enum define 
 *===========================================================================*/ 

#if (USB_MODE_SELECT_EN)
typedef enum {
    ITEMID_USB_MSDC_MODE = 1,
    ITEMID_USB_PCAM_MODE,
    ITEMID_USB_DSC_MODE,
    ITEMID_USB_DV_MODE
}USBMODEITEMID;

typedef enum {
    AHC_USB_MSDC_MODE = 1,
    AHC_USB_PCAM_MODE,
    AHC_USB_DSC_MODE,
    AHC_USB_DV_MODE,
    AHC_USB_MAX_MODE
}USBMODE;

#endif

/*===========================================================================
 * Function prototype 
 *===========================================================================*/ 
#if (USB_MODE_SELECT_EN)
UINT32 MenuStateUSBModeSelectModeHandler(UINT32 ulMsgId, UINT32 ulEvent, UINT32 ulParam );
AHC_BOOL MenuStateUSBModeSelectModeInitLCD(void* pData);
#if (TVOUT_ENABLE)
AHC_BOOL MenuStateUSBModeSelectModeInitTV(void* pData);
#endif
#if (HDMI_ENABLE)
AHC_BOOL MenuStateUSBModeSelectModeInitHDMI(void* pData);
#endif   
AHC_BOOL MenuStateUSBModeSelectModeShutDown(void* pData);
#endif

AHC_BOOL StateSelectFuncUSBSelectMenuMode(void);
#endif //_MENU_STATE_MODE_SELECT_MENU_H_


