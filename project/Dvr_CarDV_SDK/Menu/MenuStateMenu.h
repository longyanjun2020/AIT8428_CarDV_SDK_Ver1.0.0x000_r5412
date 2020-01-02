
#ifndef _MENU_STATE_MENU_H_
#define _MENU_STATE_MENU_H_

/*===========================================================================
 * Include file 
 *===========================================================================*/ 
#include "Customer_Config.h"

#include "AHC_Common.h"
#include "AHC_Gui.h"
#include "MenuCommon.h"

/*===========================================================================
 * Enum define 
 *===========================================================================*/ 

typedef enum {
    ITEMID_MENU= 1,
    ITEMID_SLIDESHOW
} ITEMID;

//===========lyj==============
#define FRONT1 		0xc1
#define FRONT2		0xc2
#define REAR1  		0xc3
#define REAR2		0xc4
#define OUTPUT1		0xc5
#define OUTPUT2 	0xc6
#define MAINVOICE 	0xc0
#define SUBVOICE1	0xc7
#define SUBVOICE2	0xc8

#define MUTE_ON		0x11
#define MUTE_OFF	0x00



//===========end==============

/*===========================================================================
 * Function prototype
 *===========================================================================*/ 

int 	 GetCatagoryMenuID(PSMENUSTRUCT pMenu);
UINT32   QuickMenuMenuEventHandler(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam );
UINT32   QuickMenuMenuEventHandler_Playback(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam );
UINT32   MainMenuEventHandler(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam );
UINT32 MenuEditConfirmEventHandler(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32   SubMenuEventHandler(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam );
UINT32 SubMenuEventHandler1(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);//lyj 20180517
UINT32   QuickMenuSubMenuEventHandler(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam );
UINT32   MenuGetDefault(PSMENUSTRUCT pMenu );
AHC_BOOL CommonMenuOK( PSMENUSTRUCT pMenu, AHC_BOOL bHover );
AHC_BOOL MenuModePreCheck(UINT32 ulEvent);

UINT32  TopMenuEventHandler(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);

UINT32 	SubMenuEventHandler_EditAllFile(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 	SubMenuEventHandler_ClockSetting(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 	SubMenuEventHandler_ResetSetup(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 	SubMenuEventHandler_FwVersionInfo(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 	SubMenuEventHandler_GPSInfoChart(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 	SubMenuEventHandler_EV(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 	SubMenuEventHandler_FormatSDCard(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 	SubMenuEventHandler_StorageInfo(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 	SubMenuEventHandler_Volume(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);

#if MENU_GENERAL_LDWS_CALIBRATION_EN
UINT32  SubMenuEventHandler_LdwsCalibration(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32  SubMenuEventHandler_UserLdwsCalibration(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
#endif

UINT32 SubMenuEventHandler_Contrast(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 SubMenuEventHandler_Saturation(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 SubMenuEventHandler_Sharpness(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
UINT32 SubMenuEventHandler_Gamma(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam);
#if(SUPPORT_TOUCH_PANEL)
UINT32 MainMenuItem_Touch(UINT16 pt_x,UINT16 pt_y);
UINT32 SubMenuItem_Touch(UINT16 pt_x,UINT16 pt_y);
UINT32  SubMenuComfirm_Touch(UINT16 pt_x,UINT16 pt_y);
#endif

#endif //_MENU_STATE_MENU_H_

