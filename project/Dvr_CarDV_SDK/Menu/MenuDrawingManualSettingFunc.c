/*===========================================================================
 * Include file
 *===========================================================================*/

#include "AHC_Common.h"
#include "AHC_MACRO.h"
#include "AHC_Utility.h"
#include "AHC_General_CarDV.h"
#include "MenuCommon.h"
#include "IconDefine.h"
#include "IconPosition.h"
#include "ColorDefine.h"
#include "OsdStrings.h"
#include "MenuDrawingFunc.h"
#include "MenuDrawCommon.h"
#include "ShowOSDFunc.h"
#include "DrawStateMenuSetting.h"
#include "MenuStateManualSettingMenu.h"
#include "StateHDMIFunc.h"
#include "AHC_Display.h"

/*===========================================================================
 * Extern varible
 *===========================================================================*/ 
extern AHC_BOOL                 gbAhcDbgBrk;
/*===========================================================================
 * Main body
 *===========================================================================*/
 extern UINT32 ulTimess;
#if(MENU_BLUETOOTH_PROGRESS_BAR)
extern UINT16 wDispID;
#endif
static void MenuDrawProgress(UINT8 bID0, int current, int min_val, int max_val, int fclr, int bclr)
{
    RECT            rc = RECT_MENU_ADJUST_BAR_ITEM;
    UINT16 x1 = 46;
    UINT16 y1 = 270;
    UINT16 T1 = 8;
    UINT16          BarPos;
	UINT32 ulFrquency;
    char            curValStr[8] = {0};
	printc("----min_val=%d-----current=%d------max_val=%d-------\r\n",min_val,current,max_val);
	#if(MENU_BLUETOOTH_PROGRESS_BAR)
	wDispID=bID0;
	#endif

    //Draw Current Value
/*    AHC_OSDSetFont(bID0, &GUI_Font24B_1);// ÏÔÊ¾Êý×Ö
    AHC_OSDSetColor(bID0, OSD_COLOR_WHITE);
    AHC_OSDSetBkColor(bID0, OSD_COLOR_DARKGRAY2);
    AHC_OSDDrawText(bID0, (void *) "       ", rc.uiLeft + (rc.uiWidth >> 1), rc.uiTop - 24, 0);
    sprintf((void *) &curValStr, "%4d", current);
    AHC_OSDDrawText(bID0, (void *) curValStr, rc.uiLeft + (rc.uiWidth >> 1), rc.uiTop - 24, 0);
*/
    //Draw Rectangle
	
/*
	OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
	OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
	OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
	OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35);
	OSD_Draw_IconXY(bID0, bmIcon_B5, x1+64+T1*4, y1-49);
	OSD_Draw_IconXY(bID0, bmIcon_B6, x1+81+T1*5, y1-63);
	OSD_Draw_IconXY(bID0, bmIcon_B7, x1+97+T1*6, y1-74);
	OSD_Draw_IconXY(bID0, bmIcon_B8, x1+116+T1*7, y1-88);
	OSD_Draw_IconXY(bID0, bmIcon_B9, x1+131+T1*8, y1-101);
	OSD_Draw_IconXY(bID0, bmIcon_B10, x1+148+T1*9, y1-114);
	OSD_Draw_IconXY(bID0, bmIcon_B11, x1+165+T1*10, y1-125);
	OSD_Draw_IconXY(bID0, bmIcon_B12, x1+182+T1*11, y1-138);
	OSD_Draw_IconXY(bID0, bmIcon_B13, x1+199+T1*12, y1-151);*/

	
	switch(current)
		{
		case 1 :   OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1); ulTimess=90;  break;// 74
		case 2 :   OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
		               OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11); ulTimess=80; break;
		case 3 :   OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);ulTimess=72; break;
		case 4 :   OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
				OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35); ulTimess=68; break;
		case 5 :    OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
				OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35);
				OSD_Draw_IconXY(bID0, bmIcon_B5, x1+64+T1*4, y1-49); ulTimess=66; break;
		case 6 :   OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
				OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35);
				OSD_Draw_IconXY(bID0, bmIcon_B5, x1+64+T1*4, y1-49);
				OSD_Draw_IconXY(bID0, bmIcon_B6, x1+81+T1*5, y1-63);  ulTimess=64; break;
		case 7 :   OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
				OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35);
				OSD_Draw_IconXY(bID0, bmIcon_B5, x1+64+T1*4, y1-49);
				OSD_Draw_IconXY(bID0, bmIcon_B6, x1+81+T1*5, y1-63);
				OSD_Draw_IconXY(bID0, bmIcon_B7, x1+97+T1*6, y1-74);ulTimess=62; break ;
		case 8 :   OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
				OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35);
				OSD_Draw_IconXY(bID0, bmIcon_B5, x1+64+T1*4, y1-49);
				OSD_Draw_IconXY(bID0, bmIcon_B6, x1+81+T1*5, y1-63);
				OSD_Draw_IconXY(bID0, bmIcon_B7, x1+97+T1*6, y1-74);
				OSD_Draw_IconXY(bID0, bmIcon_B8, x1+114+T1*7, y1-88);ulTimess=60; break ;
		case 9 :   OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
				OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35);
				OSD_Draw_IconXY(bID0, bmIcon_B5, x1+64+T1*4, y1-49);
				OSD_Draw_IconXY(bID0, bmIcon_B6, x1+81+T1*5, y1-63);
				OSD_Draw_IconXY(bID0, bmIcon_B7, x1+97+T1*6, y1-74);
				OSD_Draw_IconXY(bID0, bmIcon_B8, x1+114+T1*7, y1-88);
				OSD_Draw_IconXY(bID0, bmIcon_B9, x1+131+T1*8, y1-101);ulTimess=58; break ;
		case 10 : OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
				OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35);
				OSD_Draw_IconXY(bID0, bmIcon_B5, x1+64+T1*4, y1-49);
				OSD_Draw_IconXY(bID0, bmIcon_B6, x1+81+T1*5, y1-63);
				OSD_Draw_IconXY(bID0, bmIcon_B7, x1+97+T1*6, y1-74);
				OSD_Draw_IconXY(bID0, bmIcon_B8, x1+114+T1*7, y1-88);
				OSD_Draw_IconXY(bID0, bmIcon_B9, x1+131+T1*8, y1-101);
				OSD_Draw_IconXY(bID0, bmIcon_B10, x1+148+T1*9, y1-114);ulTimess=30; break ;
		case 11 : OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
				OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35);
				OSD_Draw_IconXY(bID0, bmIcon_B5, x1+64+T1*4, y1-49);
				OSD_Draw_IconXY(bID0, bmIcon_B6, x1+81+T1*5, y1-63);
				OSD_Draw_IconXY(bID0, bmIcon_B7, x1+97+T1*6, y1-74);
				OSD_Draw_IconXY(bID0, bmIcon_B8, x1+114+T1*7, y1-88);
				OSD_Draw_IconXY(bID0, bmIcon_B9, x1+131+T1*8, y1-101);
				OSD_Draw_IconXY(bID0, bmIcon_B10, x1+148+T1*9, y1-114);
				OSD_Draw_IconXY(bID0, bmIcon_B11, x1+165+T1*10, y1-125);ulTimess=15; break;
				
		case 12 : OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
				OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35);
				OSD_Draw_IconXY(bID0, bmIcon_B5, x1+64+T1*4, y1-49);
				OSD_Draw_IconXY(bID0, bmIcon_B6, x1+81+T1*5, y1-63);
				OSD_Draw_IconXY(bID0, bmIcon_B7, x1+97+T1*6, y1-74);
				OSD_Draw_IconXY(bID0, bmIcon_B8, x1+114+T1*7, y1-88);
				OSD_Draw_IconXY(bID0, bmIcon_B9, x1+131+T1*8, y1-101);
				OSD_Draw_IconXY(bID0, bmIcon_B10, x1+148+T1*9, y1-114);
				OSD_Draw_IconXY(bID0, bmIcon_B11, x1+165+T1*10, y1-125);
				OSD_Draw_IconXY(bID0, bmIcon_B12, x1+182+T1*11, y1-138); ulTimess=8; break ;
		case 13 : OSD_Draw_IconXY(bID0, bmIcon_B1, x1, y1);
				OSD_Draw_IconXY(bID0, bmIcon_B2, x1+15+T1,y1-11);
				OSD_Draw_IconXY(bID0, bmIcon_B3, x1+31+T1*2, y1-24);
				OSD_Draw_IconXY(bID0, bmIcon_B4, x1+48+T1*3, y1-35);
				OSD_Draw_IconXY(bID0, bmIcon_B5, x1+64+T1*4, y1-49);
				OSD_Draw_IconXY(bID0, bmIcon_B6, x1+81+T1*5, y1-63);
				OSD_Draw_IconXY(bID0, bmIcon_B7, x1+97+T1*6, y1-74);
				OSD_Draw_IconXY(bID0, bmIcon_B8, x1+114+T1*7, y1-88);
				OSD_Draw_IconXY(bID0, bmIcon_B9, x1+131+T1*8, y1-101);
				OSD_Draw_IconXY(bID0, bmIcon_B10, x1+148+T1*9, y1-114);
				OSD_Draw_IconXY(bID0, bmIcon_B11, x1+165+T1*10, y1-125);
				OSD_Draw_IconXY(bID0, bmIcon_B12, x1+182+T1*11, y1-138);
				OSD_Draw_IconXY(bID0, bmIcon_B13, x1+199+T1*12, y1-151); ulTimess=2; break; // 2

	}
	MMPF_PWM_Initialize();
	MMPF_PWM_OutputPulse(LCD_BACKLIGHT_PWM_UNIT_PIN, MMP_TRUE, 50000,ulTimess, MMP_TRUE, MMP_FALSE, NULL, 0);
	//MMPF_OS_Sleep_MS(100);
    
 //   AHC_OSDSetColor(bID0, fclr);
 //   AHC_OSDSetPenSize(bID0, 1);

//    AHC_OSDDrawLine(bID0, rc.uiLeft, rc.uiLeft+rc.uiWidth, rc.uiTop, rc.uiTop);
//    AHC_OSDDrawLine(bID0, rc.uiLeft, rc.uiLeft+rc.uiWidth, rc.uiTop+rc.uiHeight, rc.uiTop+rc.uiHeight);
//    AHC_OSDDrawLine(bID0, rc.uiLeft, rc.uiLeft, rc.uiTop, rc.uiTop+rc.uiHeight);
//    AHC_OSDDrawLine(bID0, rc.uiLeft+rc.uiWidth, rc.uiLeft+rc.uiWidth, rc.uiTop, rc.uiTop+rc.uiHeight);

    //Draw Progress Bar
 //   AHC_OSDSetColor(bID0, bclr);

    

  //  AHC_OSDDrawFillRect(bID0, rc.uiLeft+1, rc.uiTop+1, (rc.uiLeft+1)+BarPos, rc.uiTop+rc.uiHeight-1);

    //Draw Progress Bar
//    AHC_OSDSetColor(bID0, OSD_COLOR_TRANSPARENT);

//    AHC_OSDDrawFillRect(bID0, (rc.uiLeft+1)+BarPos, rc.uiTop+1, rc.uiLeft+rc.uiWidth-1, rc.uiTop+rc.uiHeight-1);
	
}

////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
void MenuDrawSubButtons_EV(UINT16 uwDispID)
{
	UINT16 x1 = POS_MENU_EV_DEC_X;
	UINT16 x2 = POS_MENU_EV_INC_X;
	
	if(AHC_IsHdmiConnect())
	{
		if(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080P)
		{
			x1 += 50;
			x2 += 50;
		}
		else
		{
			x1 += 50;
			x2 += 50;
		}	
	}	
    OSD_Draw_IconXY(uwDispID, bmIcon_Volume_Decrease, x1, POS_MENU_EV_DEC_Y);
    OSD_Draw_IconXY(uwDispID, bmIcon_Volume_Increase, x2, POS_MENU_EV_INC_Y);
    #if (SUPPORT_TOUCH_PANEL)
    OSD_Draw_IconXY(uwDispID, bmIcon_OK,              POS_MENU_EV_OK_X,  POS_MENU_EV_OK_Y);
    #endif
}

GUI_COLOR GetSubItemColor_EV(int iItems)
{
    switch(iItems)
    {
        case 0: {return 0xFF111111; break;}
        case 1: {return 0xFF1C1C1C; break;}
        case 2: {return 0xFF292929; break;}
        case 3: {return 0xFF333333; break;}
        case 4: {return 0xFF4D4D4D; break;}
        case 5: {return 0xFF5F5F5F; break;}
        case 6: {return 0xFF777777; break;}
        case 7: {return 0xFF808080; break;}
        case 8: {return 0xFF969696; break;}
        case 9: {return 0xFFB2B2B2; break;}
        case 10:{return 0xFFC0C0C0; break;}
        case 11:{return 0xFFDDDDDD; break;}
        case 12:{return 0xFFEAEAEA; break;}
        default:{return 0xFFEAEAEA; break;}
    }
}

void DrawSubItem_ShowEV(UINT16 uwDispID, PSMENUSTRUCT pMenu, UINT32 iItem)
{

}

void DrawSubItem_EV(UINT16 uwDispID, int iItem, int iTotalItems, UINT32 iStrID, const GUI_BITMAP* IconID, UINT32 barId, GUI_COLOR clrFont, GUI_COLOR clrBack, GUI_COLOR clrSelect)
{
    RECT        rc = RECT_MENU_EV_ITEM;
    GUI_COLOR   FillColor;

	if(AHC_IsHdmiConnect())
	{
		if(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080P)
		{
			rc.uiLeft += 50;
		}
		else
		{
			rc.uiLeft += 50;
		}	
	}

    rc.uiLeft += (rc.uiWidth + 3) * iItem;

    FillColor  = GetSubItemColor_EV(iItem);

    ///////////////////////////////////////////////////////
    if(iItem==0 || iItem==3 || iItem==6 || iItem==9 || iItem==12)
    {
        AHC_OSDSetFont(uwDispID, &GUI_Font16B_1);
        AHC_OSDSetColor(uwDispID, OSD_COLOR_BLACK);
        AHC_OSDSetBkColor(uwDispID, MENU_BACKGROUND_COLOR);
    }

    switch ( iItem )
    {
        case 0:
            AHC_OSDDrawText(uwDispID, (UINT8*)"-2", rc.uiLeft,      rc.uiTop -20, strlen("-2"));
        break;
        case 3:
            AHC_OSDDrawText(uwDispID, (UINT8*)"-1", rc.uiLeft,      rc.uiTop - 20, strlen("-1"));
        break;
        case 6:
            AHC_OSDDrawText(uwDispID, (UINT8*)"0",  rc.uiLeft + 4,  rc.uiTop - 20, strlen("0"));
        break;
        case 9:
            AHC_OSDDrawText(uwDispID, (UINT8*)"+1", rc.uiLeft,      rc.uiTop - 20, strlen("+1"));
        break;
        case 12:
            AHC_OSDDrawText(uwDispID, (UINT8*)"+2", rc.uiLeft,      rc.uiTop - 20, strlen("+2"));
        break;
    }

    if( clrSelect != 0 )
        FillColor = clrSelect;

    AHC_OSDSetColor(uwDispID, OSD_COLOR_BLACK);
    AHC_OSDDrawRect2(uwDispID, &rc);

    AHC_OSDSetColor(uwDispID, FillColor);
    AHC_OSDDrawFillRect(uwDispID, rc.uiLeft+1, rc.uiTop+1, rc.uiLeft+rc.uiWidth-1, rc.uiTop+rc.uiHeight-1);
}

void MenuDrawSubPage_EV(PSMENUSTRUCT pMenu)
{
    UINT8   bID0 = 0, bID1 = 0;
    UINT32  i;

#ifdef SLIDE_STRING
    StopSlideString();
#endif

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    OSDDraw_ClearOvlDrawBuffer(bID0);

    MenuDrawSubBackCurtain(bID0, MENU_BACKGROUND_COLOR);

    MenuDrawSubButtons_EV(bID0);

    for( i=0; i< pMenu->iNumOfItems; i++ )
    {
        GUI_COLOR clrSelect   = 0x0;

        if( i == pMenu->pfMenuGetDefaultVal(pMenu) )
        {
            clrSelect = OSD_COLOR_GREEN;
        }

        DrawSubItem_EV( bID0, i, pMenu->iNumOfItems,
                        pMenu->pItemsList[i]->uiStringId,
                        pMenu->pItemsList[i]->bmpIcon,
                        pMenu->pItemsList[i]->iItemId,
                        MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, clrSelect);
    }

    if( pMenu->uiStringId != -1 )
        MenuDrawSubTitle(bID0, pMenu->uiStringId);

    if(pMenu->uiStringDescription != NULL)
        MenuDrawSubMenuDescription(bID0, pMenu->uiStringDescription);

    MenuDraw_BatteryStatus(bID0);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}

void MenuDrawChangeSubItem_EV(PSMENUSTRUCT pMenu, UINT32 uiCurrItem, UINT32 uiPrevItem, UINT32 uiPreSelected)
{
    UINT8 bID0 = 0, bID1 = 0;

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    DrawSubItem_EV( bID0, uiPrevItem, pMenu->iNumOfItems,
                    pMenu->pItemsList[uiPrevItem]->uiStringId,
                    pMenu->pItemsList[uiPrevItem]->bmpIcon,
                    NULL,
                    MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, 0x0);

    DrawSubItem_EV( bID0, uiCurrItem, pMenu->iNumOfItems,
                    pMenu->pItemsList[uiCurrItem]->uiStringId,
                    pMenu->pItemsList[uiCurrItem]->bmpIcon,
                    NULL,
                    MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, OSD_COLOR_GREEN);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////
#if (MENU_MANUAL_CONTRAST_EN || MENU_STILL_CONTRAST_EN || MENU_MOVIE_CONTRAST_EN)

void MenuDrawSubButtons_Contrast(UINT16 uwDispID)
{
	#if 0 // lyj 20190624
	UINT16 x1 = POS_MENU_EV_DEC_X;
	UINT16 x2 = POS_MENU_EV_INC_X;
	UINT16 y1 = POS_MENU_EV_DEC_Y;
	UINT16 y2 = POS_MENU_EV_INC_Y;
	
	if(AHC_IsHdmiConnect()|| AHC_IsTVConnectEx())
	{
		if(DISP_WIDTH == 720 && DISP_HEIGHT == 1280)
		{
			x1 = 30;
			x2 = 330;
			y1 = 90;
			y2 = 90;
		}
		
		if(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080P)
		{
			x1 += 50;
			x2 += 50;
		}
		else
		{
			x1 += 50;
			x2 += 50;
		}	
	}		
	#endif
	 Main_Set_Page(BRIGHTNESS_FLAG);// liao
    OSD_Draw_IconXY(uwDispID, bmSub_Brightness,              0,  0);
	SelfAllFlag.SaveTheBrightness=1;
	SelfAllFlag.SaveTheRGB=0;
	SelfAllFlag.SaveTheLightBar =0;
	//    OSD_Draw_IconXY(uwDispID, bmIcon_Volume_Decrease, x1, y1);
   // OSD_Draw_IconXY(uwDispID, bmIcon_Volume_Increase, x2, y2);

	{
		 char	szver[20];
		   RECT 	tmpRECT = {10, 295, 100,  20};
		    AHC_OSDSetFont(uwDispID, &GUI_Font10_1);
		//  sprintf(szver, ": %04d.%04d.%04d", );
		    sprintf(szver, "MTX67B-19.12.04");// 11.04
		  
		  OSD_ShowString(uwDispID, szver, NULL, tmpRECT, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT, GUI_TA_HCENTER|GUI_TA_VCENTER);


	}
    #if (SUPPORT_TOUCH_PANEL)
    OSD_Draw_IconXY(uwDispID, bmIcon_OK,              POS_MENU_EV_OK_X,  POS_MENU_EV_OK_Y);
    #endif
}


void MenuDrawSubPage_Contrast(PSMENUSTRUCT pMenu)
{
    UINT8   bID0 = 0, bID1 = 0;
    INT32   s32Contrast = 0;

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }

#ifdef SLIDE_STRING
    StopSlideString();
#endif

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    OSDDraw_ClearOvlDrawBuffer(bID0);

    BEGIN_LAYER(bID0);

   // MenuDrawSubBackCurtain(bID0, OSD_COLOR_DARKGRAY2); // lyj 20190624

    MenuDrawSubButtons_Contrast(bID0);
   
    AHC_Menu_SettingGetCB( (char *) COMMON_KEY_CONTRAST, &s32Contrast );

    MenuDrawProgress( bID0,
                (int) s32Contrast, (int) MENU_MANUAL_CONTRAST_MIN, (int) MENU_MANUAL_CONTRAST_MAX,
                OSD_COLOR_WHITE, OSD_COLOR_ORANGE);
	
#if 0
    if (pMenu->uiStringId != -1)
        MenuDrawSubTitle(bID0, pMenu->uiStringId);

    if (pMenu->uiStringDescription != NULL)
        MenuDrawSubMenuDescription(bID0, pMenu->uiStringDescription);

    END_LAYER(bID0);

    MenuDraw_BatteryStatus(bID0);

    BEGIN_LAYER(bID0);
#endif

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    END_LAYER(bID0);
}

void MenuDrawChangeSubItem_Contrast(void)
{
    INT32 s32Contrast = 0;
    UINT8 bID0 = 0, bID1 = 0;

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    BEGIN_LAYER(bID0);

    pf_General_EnGet(COMMON_KEY_CONTRAST, &s32Contrast);
	OSD_Draw_IconXY(bID0, bmBrightness_nor,              26,  121);// lyj
//	pf_General_EnSet(COMMON_KEY_CONTRAST, s32Contrast);
    MenuDrawProgress( bID0,
                  (int) s32Contrast, (int) MENU_MANUAL_CONTRAST_MIN, (int) MENU_MANUAL_CONTRAST_MAX,
                  OSD_COLOR_WHITE,
                  OSD_COLOR_ORANGE);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    END_LAYER(bID0);
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////
#if (MENU_MANUAL_SATURATION_EN || MENU_STILL_SATURATION_EN || MENU_MOVIE_SATURATION_EN)

void MenuDrawSubButtons_Saturation(UINT16 uwDispID)
{
	UINT16 x1 = POS_MENU_EV_DEC_X;
	UINT16 x2 = POS_MENU_EV_INC_X;
	UINT16 y1 = POS_MENU_EV_DEC_Y;
	UINT16 y2 = POS_MENU_EV_INC_Y;
	
	if(AHC_IsHdmiConnect()|| AHC_IsTVConnectEx())
	{
		if(DISP_WIDTH == 720 && DISP_HEIGHT == 1280)
		{
			x1 = 30;
			x2 = 330;
			y1 = 90;
			y2 = 90;
		}
		
		if(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080P)
		{
			x1 += 50;
			x2 += 50;
		}
		else
		{
			x1 += 50;
			x2 += 50;
		}	
	}	

    OSD_Draw_IconXY(uwDispID, bmIcon_Volume_Decrease, x1, y1);
    OSD_Draw_IconXY(uwDispID, bmIcon_Volume_Increase, x2, y2);
    #if (SUPPORT_TOUCH_PANEL)
    OSD_Draw_IconXY(uwDispID, bmIcon_OK,              POS_MENU_EV_OK_X,  POS_MENU_EV_OK_Y);
    #endif
}

void MenuDrawSubPage_Saturation(PSMENUSTRUCT pMenu)
{
    UINT8   bID0 = 0, bID1 = 0;
    INT32   s32Saturation = 0;

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }

#ifdef SLIDE_STRING
    StopSlideString();
#endif

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    OSDDraw_ClearOvlDrawBuffer(bID0);

    BEGIN_LAYER(bID0);

    MenuDrawSubBackCurtain(bID0, OSD_COLOR_DARKGRAY2);

    MenuDrawSubButtons_Saturation(bID0);

    AHC_Menu_SettingGetCB( (char *) COMMON_KEY_SATURATION, &s32Saturation );

    MenuDrawProgress( bID0,
                      (int) s32Saturation, (int) MENU_MANUAL_SATURATION_MIN, (int) MENU_MANUAL_SATURATION_MAX,
                      OSD_COLOR_WHITE, OSD_COLOR_ORANGE);

#if 0
    if (pMenu->uiStringId != -1)
        MenuDrawSubTitle(bID0, pMenu->uiStringId);

    if (pMenu->uiStringDescription != NULL)
        MenuDrawSubMenuDescription(bID0, pMenu->uiStringDescription);

    END_LAYER(bID0);

    MenuDraw_BatteryStatus(bID0);

    BEGIN_LAYER(bID0);
#endif

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    END_LAYER(bID0);
}

void MenuDrawChangeSubItem_Saturation(void)
{
    INT32 s32Saturation = 0;
    UINT8 bID0 = 0, bID1 = 0;

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    BEGIN_LAYER(bID0);

    pf_General_EnGet(COMMON_KEY_SATURATION, &s32Saturation);
    MenuDrawProgress( bID0,
                      (int) s32Saturation, (int) MENU_MANUAL_SATURATION_MIN, (int) MENU_MANUAL_SATURATION_MAX,
                      OSD_COLOR_WHITE,
                      OSD_COLOR_ORANGE);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    END_LAYER(bID0);
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////
#if (MENU_MANUAL_SHARPNESS_EN || MENU_STILL_SHARPNESS_EN || MENU_MOVIE_SHARPNESS_EN)

void MenuDrawSubButtons_Sharpness(UINT16 uwDispID)
{
	UINT16 x1 = POS_MENU_EV_DEC_X;
	UINT16 x2 = POS_MENU_EV_INC_X;
	UINT16 y1 = POS_MENU_EV_DEC_Y;
	UINT16 y2 = POS_MENU_EV_INC_Y;
	
	if(AHC_IsHdmiConnect()|| AHC_IsTVConnectEx())
	{
		if(DISP_WIDTH == 720 && DISP_HEIGHT == 1280)
		{
			x1 = 30;
			x2 = 330;
			y1 = 90;
			y2 = 90;
		}
		
		if(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080P)
		{
			x1 += 50;
			x2 += 50;
		}
		else
		{
			x1 += 50;
			x2 += 50;
		}	
	}	

    OSD_Draw_IconXY(uwDispID, bmIcon_Volume_Decrease, x1, y1);
    OSD_Draw_IconXY(uwDispID, bmIcon_Volume_Increase, x2, y2);
    #if (SUPPORT_TOUCH_PANEL)
    OSD_Draw_IconXY(uwDispID, bmIcon_OK,              POS_MENU_EV_OK_X,  POS_MENU_EV_OK_Y);
    #endif
}

void MenuDrawSubPage_Sharpness(PSMENUSTRUCT pMenu)
{
    UINT8   bID0 = 0, bID1 = 0;
    INT32   s32Sharpness = 0;

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }

#ifdef SLIDE_STRING
    StopSlideString();
#endif

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    OSDDraw_ClearOvlDrawBuffer(bID0);

    BEGIN_LAYER(bID0);

    MenuDrawSubBackCurtain(bID0, OSD_COLOR_DARKGRAY2);

    MenuDrawSubButtons_Sharpness(bID0);

    AHC_Menu_SettingGetCB( (char *) COMMON_KEY_SHARPNESS, &s32Sharpness );

    MenuDrawProgress( bID0,
                      (int) s32Sharpness, (int) MENU_MANUAL_SHARPNESS_MIN, (int) MENU_MANUAL_SHARPNESS_MAX,
                      OSD_COLOR_WHITE, OSD_COLOR_ORANGE);

#if 0
    if (pMenu->uiStringId != -1)
        MenuDrawSubTitle(bID0, pMenu->uiStringId);

    if (pMenu->uiStringDescription != NULL)
        MenuDrawSubMenuDescription(bID0, pMenu->uiStringDescription);

    END_LAYER(bID0);

    MenuDraw_BatteryStatus(bID0);

    BEGIN_LAYER(bID0);
#endif
    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    END_LAYER(bID0);
}

void MenuDrawChangeSubItem_Sharpness(void)
{
    INT32 s32Sharpness = 0;
    UINT8 bID0 = 0, bID1 = 0;

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    BEGIN_LAYER(bID0);

    pf_General_EnGet(COMMON_KEY_SHARPNESS, &s32Sharpness);
    MenuDrawProgress( bID0,
                      (int) s32Sharpness, (int) MENU_MANUAL_SHARPNESS_MIN, (int) MENU_MANUAL_SHARPNESS_MAX,
                      OSD_COLOR_WHITE,
                      OSD_COLOR_ORANGE);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    END_LAYER(bID0);
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////
#if (MENU_MANUAL_GAMMA_EN || MENU_STILL_GAMMA_EN || MENU_MOVIE_GAMMA_EN)

void MenuDrawSubButtons_Gamma(UINT16 uwDispID)
{
	UINT16 x1 = POS_MENU_EV_DEC_X;
	UINT16 x2 = POS_MENU_EV_INC_X;
	UINT16 y1 = POS_MENU_EV_DEC_Y;
	UINT16 y2 = POS_MENU_EV_INC_Y;
	
	if(AHC_IsHdmiConnect()|| AHC_IsTVConnectEx())
	{
		if(DISP_WIDTH == 720 && DISP_HEIGHT == 1280)
		{
			x1 = 30;
			x2 = 330;
			y1 = 90;
			y2 = 90;
		}
		
		if(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080P)
		{
			x1 += 50;
			x2 += 50;
		}
		else
		{
			x1 += 50;
			x2 += 50;
		}	
	}		

    OSD_Draw_IconXY(uwDispID, bmIcon_Volume_Decrease, x1, y1);
    OSD_Draw_IconXY(uwDispID, bmIcon_Volume_Increase, x2, y2);
    #if (SUPPORT_TOUCH_PANEL)
    OSD_Draw_IconXY(uwDispID, bmIcon_OK,              POS_MENU_EV_OK_X,  POS_MENU_EV_OK_Y);
    #endif
}

void MenuDrawSubPage_Gamma(PSMENUSTRUCT pMenu)
{
    UINT8   bID0 = 0, bID1 = 0;
    INT32   s32Gamma = 0;

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }

#ifdef SLIDE_STRING
    StopSlideString();
#endif

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    OSDDraw_ClearOvlDrawBuffer(bID0);

    BEGIN_LAYER(bID0);

    MenuDrawSubBackCurtain(bID0, OSD_COLOR_DARKGRAY2);

    MenuDrawSubButtons_Saturation(bID0);

    AHC_Menu_SettingGetCB( (char *) COMMON_KEY_GAMMA, &s32Gamma );

    MenuDrawProgress( bID0,
                      (int) s32Gamma, (int) MENU_MANUAL_GAMMA_MIN, (int) MENU_MANUAL_GAMMA_MAX,
                      OSD_COLOR_WHITE, OSD_COLOR_ORANGE);

#if 0
    if (pMenu->uiStringId != -1)
        MenuDrawSubTitle(bID0, pMenu->uiStringId);

    if (pMenu->uiStringDescription != NULL)
        MenuDrawSubMenuDescription(bID0, pMenu->uiStringDescription);

    END_LAYER(bID0);

    MenuDraw_BatteryStatus(bID0);

    BEGIN_LAYER(bID0);
#endif
    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    END_LAYER(bID0);
}

void MenuDrawChangeSubItem_Gamma(void)
{
    INT32 s32Gamma = 0;
    UINT8 bID0 = 0, bID1 = 0;

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    BEGIN_LAYER(bID0);

    pf_General_EnGet(COMMON_KEY_GAMMA, &s32Gamma);
    MenuDrawProgress( bID0,
                      (int) s32Gamma, (int) MENU_MANUAL_GAMMA_MIN, (int) MENU_MANUAL_GAMMA_MAX,
                      OSD_COLOR_WHITE,
                      OSD_COLOR_ORANGE);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    END_LAYER(bID0);
}

#endif
