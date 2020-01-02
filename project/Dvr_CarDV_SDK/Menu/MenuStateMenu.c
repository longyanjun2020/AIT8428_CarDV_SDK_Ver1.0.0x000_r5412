/*===========================================================================
 * Include file 
 *===========================================================================*/ 

#include "AHC_Utility.h"
#include "AHC_MACRO.h"
#include "AHC_Menu.h"
#include "AHC_Message.h"
#include "AHC_GUI.h"
#include "AHC_General.h"
#include "AHC_Parameter.h"
#include "AHC_Display.h"
#include "AHC_Browser.h"
#include "AHC_Warningmsg.h"
#include "AHC_General_CarDV.h"
#include "MenuCommon.h"
#include "MenuTouchButton.h"
#include "MenuStateMenu.h"
#include "MenuDrawingFunc.h"
#include "MenuDrawCommon.h"
#include "MenuStateMenu.h"
#include "MenuSetting.h"
#include "IconPosition.h"
#include "ColorDefine.h"
#include "OsdStrings.h"
#include "IconDefine.h"
#include "StateBrowserFunc.h"
#ifdef _OEM_MENU_
#include "Oem_Menu.h"
#endif
#include "StateTVFunc.h"

#include "si47xx.h"
#include "si47xxFMRX.h"
#include "si47xxAMRX.h"
#include "mmp_gpio_inc.h"
#include "FMRXtest.h" 
/*===========================================================================
 * Global variable
 *===========================================================================*/ 
#define BlueTOOthMACO
PSMENUSTRUCT MenuPageList[] = 
{
#if (MENU_MOVIE_PAGE_EN)
	&sMainMenuVideo,
#endif
#if (MENU_STILL_PAGE_EN)	
	&sMainMenuStill,
#endif
#if (MENU_PLAYBACK_PAGE_EN)	
	&sMainMenuPlayback,
#endif
#if (MENU_MEDIA_PAGE_EN)		
	&sMainMenuMedia,
#endif
#if (MENU_GENERAL_PAGE_EN)	
	&sMainMenuGeneral,
#endif
#if (MENU_WIFI_PAGE_EN)
	&sMainMenuWifi,
#endif
#if (EXIT_MENU_PAGE_EN)	
	&sMainMenuExit,
#endif	
};

//AHC_BOOL    bluetooth_key = AHC_FALSE; // lyj 20180522

AHC_BOOL	m_ubAtMenuTab = AHC_FALSE;
extern AHC_BOOL Menu_To_Video_One;
/*===========================================================================
 * Extern variable
 *===========================================================================*/ 
 
extern AHC_BOOL 	Deleting_InBrowser;
extern AHC_BOOL 	Protecting_InBrowser;
extern AHC_BOOL   	bShowHdmiWMSG;
extern AHC_BOOL   	bShowTvWMSG;
//extern AHC_BOOL 	bForceSwitchBrowser;
extern AHC_BOOL 	sub_menu_page;

AHC_BOOL 			return_flag = 0;
AHC_BOOL			lightBarFlag_1 = 0;
AHC_BOOL			lightBarFlag_2 = 0;
AHC_BOOL			lightBarFlag_3 = 0;
AHC_BOOL			lightBarFlag_4 = 0;
AHC_BOOL			lightBarFlag_5 = 0;
AHC_BOOL			lightBarFlag_6 = 0;
int 			volume_num = -1;
int 			volume_num1 = -1;
int 			volume_num2 = -1;
int 			mainVol = 0;
int 			subVol  = 0;
//MMP_UBYTE 		volume_conver = 0;
AHC_BOOL switch_search = 0;

AHC_BOOL 			icon_flag = 0;
AHC_BOOL 			select_flag ;

AHC_BOOL 			FM_select = 0;

AHC_BOOL 			AM_select = 0;

//AHC_BOOL 			display_radio = 0;

short Radio_i = 0;
short Radio_y =10;
short Radio_z = 20; // lyj 20190226

short Radio_am_i = 0;
short Radio_am_y = 10;

AHC_BOOL Mute_ON_OFF = 0;


MMP_USHORT Mannal_Freq = 8750;
MMP_USHORT AM_Freq 	   = 522;

//AHC_BOOL   mainVflag = 0;

extern MMP_USHORT radio[31]; // lyj
extern MMP_USHORT radio_am[21];

extern MMP_USHORT Tfreq;

extern __u8    Radiocnt;// lyj 
extern __u8      RadiocntAm;

//extern AHC_BOOL volume_M;
//extern AHC_BOOL volume_B ;
extern AHC_BOOL sub_flag_ex ;

//extern  UINT8 Uwdraw;
extern UINT16 wDispID;
//extern UINT8 Uwdrawx;
 
void Reserved_radio(MMP_USHORT availedFreq[],MMP_UBYTE z0,AHC_BOOL bicIcon);
AHC_BOOL Menu_Get_Page(void);

//void Draw_FM_AM_icon(MMP_USHORT Dfreq);

//void Draw_AM_icon(MMP_USHORT Dfreq);

//MMP_ULONG Get_vailed_Freq(void);
//MMP_ULONG Get_vailed_am_Freq(void);
void DrwawSubItem_lightBarEX(int dexLight,AHC_BOOL lihgtFlag);

MMP_ULONG Get_vailed_Freq_manul(MMP_USHORT Tfreq);
MMP_ULONG Get_vailed_am_Freq_manul(MMP_USHORT Tfreq);
void Close_channal(void);
void Bluetooth_LEdoff(void);

void  MenuStateExecutionCommon1(UINT32 ulPosition);
void Draw_Volume_icon(int vol);

void uart_to_mcu_voice(MMP_BYTE typev,MMP_BYTE val,MMP_BYTE mutef);
void light_Bar_fun(MMP_GPIO_PIN pin,AHC_BOOL vel);

void volume_conver_size(MMP_UBYTE ch ,int vol);



#if (VIRTUAL_KEY_BOARD)
extern UINT32	SubMenuEventHandler_Keyboard(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam );
#endif

/*===========================================================================
 * Main body
 *===========================================================================*/

#if 0
void ________Touch_Function_______(){ruturn;} //dummy
#endif

#if 0
void ________MainSubMenu_Function_______(){ruturn;} //dummy
#endif

#define MENU_PAGE_NUM 	sizeof(MenuPageList)/sizeof(PSMENUSTRUCT)

PSMENUSTRUCT GetPrevCatagoryMenu(PSMENUSTRUCT pMenu)
{
	int i, Prev;
	
	for(i=0; i<MENU_PAGE_NUM; i++)
	{
		if(pMenu->iMenuId==MenuPageList[i]->iMenuId)
		{
			Prev = (i==0)?(MENU_PAGE_NUM-1):(i-1);
			return (MenuPageList[Prev]);
		}
	}

	return &sMainMenuVideo;
}

PSMENUSTRUCT GetNextCatagoryMenu(PSMENUSTRUCT pMenu)
{
	int i, Next;
	
	for(i=0; i<MENU_PAGE_NUM; i++)
	{
		if(pMenu->iMenuId==MenuPageList[i]->iMenuId)
		{
			Next = ((i+1)==MENU_PAGE_NUM)?(0):(i+1);
			return (MenuPageList[Next]);
		}
	}

	return &sMainMenuVideo;
}

int GetCatagoryMenuID(PSMENUSTRUCT pMenu)
{
    if     ( pMenu == &sMainMenuManual   ) { return 0;  }
    else if( pMenu == &sMainMenuVideo    ) { return 1;  }
    else if( pMenu == &sMainMenuStill    ) { return 2;  }
    else if( pMenu == &sMainMenuPlayback ) { return 3;  }
    else if( pMenu == &sMainMenuEdit     ) { return 4;  }
    else if( pMenu == &sMainMenuMedia    ) { return 5;  }
    else if( pMenu == &sMainMenuGeneral  ) { return 6;  }
    else if( pMenu == &sMainMenuExit	 ) { return 7;  }
#if (MENU_WIFI_PAGE_EN)
	else if( pMenu == &sMainMenuWifi	 ) { return 8;	}
#endif
    else                                   { return 0;  }
}

AHC_BOOL CommonMenuOK(PSMENUSTRUCT pMenu, AHC_BOOL bHover)
{
    INT32		 	i;
    PSMENUITEM 		pCurItem;
    PSMENUSTRUCT 	pSubMenu;

	printc("~pItemsList = %d~iCurrentPos = %d~\r\n",pMenu->pItemsList[pMenu->iCurrentPos],pMenu->iCurrentPos);

    if( pMenu == NULL )
    {
    	return AHC_FALSE;
    }

    if( bHover == AHC_FALSE )
	    i = pMenu->iSelected;
    else
	    i = pMenu->iCurrentPos;

    pCurItem = pMenu->pItemsList[i];
    pSubMenu = pCurItem->pSubMenu;

    if( pSubMenu != NULL )
    {
   
        pSubMenu->pParentMenu = pMenu;
        SetCurrentMenu(pSubMenu);
printc("~~~~~~~~~%s~~~~~~~~\r\n",__func__);

        pSubMenu->pfEventHandler( pSubMenu, MENU_INIT, 0 );

	printc("lyj~~~~~~~~~~~~~~\r\n");
		Deleting_InBrowser = 0;
		Protecting_InBrowser = 0;
  
        return AHC_FALSE;
    }
    else if( pCurItem->pfItemSelect != NULL )
    {
        pCurItem->pfItemSelect( pCurItem, bHover );
        
        return AHC_TRUE;
    }
    else
    {
        return AHC_FALSE;
    }
}


//=====================================

AHC_BOOL SSUB_CommonMenuOK1(PSMENUSTRUCT pMenu, AHC_BOOL bHover)
{
    INT32		 	i;
    PSMENUITEM 		pCurItem;
    PSMENUSTRUCT 	pSubMenu;

	printc("~pItemsList = %d~iCurrentPos = %d~\r\n",pMenu->pItemsList[pMenu->iCurrentPos],pMenu->iCurrentPos);

    if( pMenu == NULL )
    {
    	return AHC_FALSE;
    }

    if( bHover == AHC_FALSE )
	    i = pMenu->iSelected;
    else
	    i = pMenu->iCurrentPos;

	sub_menu_page = pMenu->iCurrentPos;

    pCurItem = pMenu->pItemsList[i];
    pSubMenu = pCurItem->pSubMenu;

    if( pSubMenu != NULL )
    {
   
        pSubMenu->pParentMenu = pMenu;
        SetCurrentMenu(pSubMenu);
printc("~~~~~~~~~%s~~~~~~~~\r\n",__func__);

        pSubMenu->pfEventHandler( pSubMenu, MENU_INIT, 0 );

	printc("lyj~~~~~~~~~~~~~~\r\n");
		Deleting_InBrowser = 0;
		Protecting_InBrowser = 0;
  
        return AHC_FALSE;
    }
    else if( pCurItem->pfItemSelect != NULL )
    {
        pCurItem->pfItemSelect( pCurItem, bHover );
        
        return AHC_TRUE;
    }
    else
    {
        return AHC_FALSE;
    }
}


UINT32 MenuGetDefault(PSMENUSTRUCT pMenu)
{
	return 0;
}

AHC_BOOL MenuModePreCheck(UINT32 ulEvent)
{
#if (TVOUT_PREVIEW_EN==0 && HDMI_PREVIEW_EN==0)

    if(BUTTON_HDMI_DETECTED == ulEvent)
    {
        if(AHC_IsHdmiConnect() && bShowHdmiWMSG)
        {
            bShowHdmiWMSG = AHC_FALSE;
            AHC_WMSG_Draw(AHC_TRUE, WMSG_HDMI_TV, 3);
        }         
    }
    
    if( BUTTON_TV_DETECTED == ulEvent)
    {
        if(AHC_IsTVConnectEx() && bShowTvWMSG) 
        {
            bShowTvWMSG = AHC_FALSE;
            AHC_WMSG_Draw(AHC_TRUE, WMSG_HDMI_TV, 3);
        }    
    }

    if(BUTTON_HDMI_REMOVED == ulEvent)
    {
        bShowHdmiWMSG = AHC_TRUE;
    }
    
    if(BUTTON_TV_REMOVED == ulEvent)
    {
        bShowTvWMSG = AHC_TRUE;
    }
#endif

    if(BUTTON_CLEAR_WMSG == ulEvent)
    {
        AHC_WMSG_Draw(AHC_FALSE, WMSG_NONE, 0);
    }
    
    return AHC_TRUE;
}

UINT32 MenuEditConfirmEventHandler(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam)
{
    INT32 	iPrevPos;
    
#if(SUPPORT_TOUCH_PANEL)    
    POINT	TouchPoint;

    if( MENU_TOUCH == ulEvent || MENU_TOUCH_MOVE == ulEvent ){
        TouchPoint.PointX = ulParam & 0xFFFF;
        TouchPoint.PointY = (ulParam >> 16) & 0xFFFF;
    }
#endif

    switch(ulEvent)
    {
        case MENU_EXIT:
            return MENU_ERR_EXIT;
            break;

        case MENU_INIT:
            pMenu->uiStatus = MENU_STATUS_NONE;
            pMenu->bSetOne  = 0;
            pMenu->iCurrentPos = pMenu->iSelected;

#if (SUPPORT_TOUCH_PANEL)
            KeyParser_TouchItemRegister(&MainMenu_TouchButton[0], ITEMNUM(MainMenu_TouchButton));
#endif

#if (EXIT_MENU_PAGE_EN)  
            if(pMenu == (&sMainMenuExit))
                MenuDrawExitMainPage( pMenu );
            else
#endif
                MenuDrawMainPage( pMenu );
            break;

        case MENU_UP            :
            iPrevPos = pMenu->iCurrentPos;
            pMenu->iCurrentPos = OffsetItemNumber( pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, -1, AHC_TRUE );

#if (EXIT_MENU_PAGE_EN)
            if(pMenu == (&sMainMenuExit))
                MenuDrawChangeExitItem( pMenu, pMenu->iCurrentPos, iPrevPos);
            else
#endif
                MenuDrawChangeItem( pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected );
            break;

        case MENU_DOWN          :
            iPrevPos = pMenu->iCurrentPos;
            pMenu->iCurrentPos = OffsetItemNumber( pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, 1, AHC_TRUE);

#if (EXIT_MENU_PAGE_EN)
            if(pMenu == (&sMainMenuExit)) 
                MenuDrawChangeExitItem( pMenu, pMenu->iCurrentPos, iPrevPos);
            else
#endif
                MenuDrawChangeItem( pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected ); 
            break;

        case MENU_LEFT          :
            {
#ifdef CFG_MENU_TOP_PAGE_SD //may be defined in config_xxx.h
                break;
#else
                PSMENUSTRUCT pNewMenu;

                pNewMenu = GetPrevCatagoryMenu( pMenu );
                SetCurrentMenu( pNewMenu );
                pNewMenu->pfEventHandler ( pNewMenu, MENU_INIT, 0 );
#endif
            }
            break;

        case MENU_RIGHT         :
            {	
#ifdef CFG_MENU_TOP_PAGE_SD //may be defined in config_xxx.h
                    break;
#else
                PSMENUSTRUCT pNewMenu;

                pNewMenu = GetNextCatagoryMenu( pMenu );


                SetCurrentMenu( pNewMenu );
                pNewMenu->pfEventHandler ( pNewMenu, MENU_INIT, 0 );
#endif
            }
            break;

        case MENU_OK            :
        case MENU_SET_ONE		:

            if(ulEvent==MENU_SET_ONE){	
                pMenu->iCurrentPos = ulParam;
                pMenu->iSelected   = ulParam;
                pMenu->bSetOne   = 1;
                pMenu->uiStatus |= MENU_STATUS_PRESSED;
                pMenu->uiStatus |= MENU_STATUS_ITEM_TOUCHED;
            }
            else{
                pMenu->bSetOne   = 1;
                pMenu->iSelected = pMenu->iCurrentPos;
                pMenu->uiStatus |= MENU_STATUS_PRESSED;
            }

            if( CommonMenuOK( pMenu, AHC_TRUE ) == AHC_TRUE ){

				#if (TOP_MENU_PAGE_EN)
                PSMENUSTRUCT pCurrMenu = NULL;

                SetCurrentMenu(&sTopMenu);
                pCurrMenu = GetCurrentMenu();
                if( pCurrMenu == NULL ){
                    return MENU_ERR_EXIT;
                }

                pCurrMenu->pfEventHandler(pCurrMenu, MENU_INIT, 0);
				#endif
				
                return MENU_ERR_EXIT;
            }
            break;

        case MENU_MENU          :
            {
                PSMENUSTRUCT pParentMenu;

                pMenu->bSetOne     = 0;
                pParentMenu        = pMenu->pParentMenu;
                pMenu->pParentMenu = NULL;

                BrowserFunc_ThumbnailEditFilelComplete();
                if( pParentMenu == NULL ){
                    return MENU_ERR_EXIT;
                }

                SetCurrentMenu(pParentMenu);
                pParentMenu->pfEventHandler(pParentMenu, MENU_INIT, 0);
            }
            break;

        default:
            return MENU_ERR_OK;//MENU_ERR_NOT_MENU_EVENT;
            break;
    }

    return MENU_ERR_OK;
}



//==============lyj========


extern void Draw_sub_vicon(int vicon);
extern AHC_BOOL flagJump;
UINT32 MainMenuEventHandler(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam)
{
    INT32 	iPrevPos;
    
#if(SUPPORT_TOUCH_PANEL)    
    POINT	TouchPoint;

    if( MENU_TOUCH == ulEvent || MENU_TOUCH_MOVE == ulEvent ){
        TouchPoint.PointX = ulParam & 0xFFFF;
        TouchPoint.PointY = (ulParam >> 16) & 0xFFFF;
    }
#endif

    switch(ulEvent)
    {
        case MENU_EXIT:
            return MENU_ERR_EXIT;
            break;
		case EVENT_USB_DETECT:
			return MENU_EXIT_DO_PARENT_EVENT;
			break;
			
        case MENU_INIT:
            pMenu->uiStatus = MENU_STATUS_NONE;
            pMenu->bSetOne  = 0;
            pMenu->iCurrentPos = pMenu->iSelected;

#if (SUPPORT_TOUCH_PANEL)
            KeyParser_TouchItemRegister(&MainMenu_TouchButton[0], ITEMNUM(MainMenu_TouchButton));
#endif

#if (EXIT_MENU_PAGE_EN)  
            if(pMenu == (&sMainMenuExit))
                MenuDrawExitMainPage( pMenu );
            else
#endif
                MenuDrawMainPage( pMenu );//绘制主页面(菜单界面)long MenuDrawTitle();
                printc("~~~~~mainpage~~~~~~\r\n");//long 
            break;

        case MENU_UP            :
            iPrevPos = pMenu->iCurrentPos;
            pMenu->iCurrentPos = OffsetItemNumber( pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, -1, AHC_TRUE );

#if (EXIT_MENU_PAGE_EN)
            if(pMenu == (&sMainMenuExit))
                MenuDrawChangeExitItem( pMenu, pMenu->iCurrentPos, iPrevPos);
            else
#endif
                MenuDrawChangeItem( pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected );
		   printc("~~~~~mainpage~~~~~~~~~up~~\r\n");//long
		   //volume_M = 0;
		   //volume_B = 0;
		   sub_flag_ex = 0;
            break;

        case MENU_DOWN          :
            iPrevPos = pMenu->iCurrentPos;
            pMenu->iCurrentPos = OffsetItemNumber( pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, 1, AHC_TRUE);

#if (EXIT_MENU_PAGE_EN)
            if(pMenu == (&sMainMenuExit)) 
                MenuDrawChangeExitItem( pMenu, pMenu->iCurrentPos, iPrevPos);
            else
#endif
                MenuDrawChangeItem( pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected );//item选择跳动改变，重新绘制 
                  printc("~~~~~mainpage~~~~~~~~~down~~\r\n");//long
                 // volume_M = 0;
			 //volume_B = 0;
		   sub_flag_ex = 0;
            break;

        case MENU_LEFT          :
            {
#ifdef CFG_MENU_TOP_PAGE_SD //may be defined in config_xxx.h
                break;
#elif 0
		#if 0

                PSMENUSTRUCT pNewMenu;

                pNewMenu = GetPrevCatagoryMenu( pMenu );//改变菜单模式
                SetCurrentMenu( pNewMenu );
                pNewMenu->pfEventHandler ( pNewMenu, MENU_INIT, 0 );//重新绘制菜单模式，并Init

		#endif
		if(sub_flag == 0)
		{	if(mainVflag == 0)
			{
				mainVflag = 1;
			}

			if(mainVol >= 0 && mainVol <= 10 )
			{
				mainVol--;
				Draw_Volume_icon(mainVol);
				if(mainVol == 0)
				{
					uart_to_mcu_voice(MAINVOICE,(MMP_BYTE)mainVol,MUTE_ON);
				}
				else if(mainVol > 0)
					//uart_to_mcu_voice(MAINVOICE,(MMP_BYTE)mainVol,MUTE_OFF);// add voice
					volume_conver_size(MAINVOICE,mainVol);
			}
			if(mainVol < 0)
			{
				mainVol = 0;
				Draw_Volume_icon(mainVol);
			}
				
		
		}
		else if(sub_flag == 1)
		{
			if(subVol == 0)
			{}
			if(subVol >= 0 && subVol <= 10)
			{
				subVol--;
				Draw_sub_vicon(subVol);
				if(subVol == 0)
				{
					uart_to_mcu_voice(SUBVOICE1,(MMP_BYTE)subVol,MUTE_ON);
					uart_to_mcu_voice(SUBVOICE2,(MMP_BYTE)subVol,MUTE_ON);
				}
				else
				{
					if(subVol > 0)
					{
						//uart_to_mcu_voice(SUBVOICE1,(MMP_BYTE)subVol,MUTE_OFF);
						//uart_to_mcu_voice(SUBVOICE2,(MMP_BYTE)subVol,MUTE_OFF);
						volume_conver_size(SUBVOICE1,subVol);
						volume_conver_size(SUBVOICE2,subVol);
						
					}
				}
				
			}
			if(subVol < 0)
			{
				subVol = 0;
				Draw_sub_vicon(subVol);
			}
			

		}
		printc("~~uuuuuuuuuuuuuuu~MENU_LEFT run~~~\r\n");
#endif
            }
            break;

        case MENU_RIGHT         :
            {	
#ifdef CFG_MENU_TOP_PAGE_SD //may be defined in config_xxx.h
                break;
#elif 0
	#if 0
                PSMENUSTRUCT pNewMenu;

                pNewMenu = GetNextCatagoryMenu( pMenu );


                SetCurrentMenu( pNewMenu );
                pNewMenu->pfEventHandler ( pNewMenu, MENU_INIT, 0 );

	#endif
		if(sub_flag == 0)
		{	if(mainVflag == 0)
			{
				mainVflag = 1;
			}

			if(mainVol >= 0 && mainVol < 10 )
			{
				mainVol++;
				Draw_Volume_icon(mainVol);
				//uart_to_mcu_voice(MAINVOICE,(MMP_BYTE)mainVol,MUTE_OFF);// add voice
				volume_conver_size(MAINVOICE,mainVol);
			}
			if(mainVol >= 10)
			{
				mainVol = 10;
				Draw_Volume_icon(mainVol);
			}
				

		}
		else if(sub_flag == 1)
		{
			if(subVol == 0)
			{}
			if(subVol >= 0 && subVol < 10)
			{
				subVol++;
				Draw_sub_vicon(subVol);
				//uart_to_mcu_voice(SUBVOICE1,(MMP_BYTE)subVol,MUTE_OFF);
				//uart_to_mcu_voice(SUBVOICE2,(MMP_BYTE)subVol,MUTE_OFF);
				volume_conver_size(SUBVOICE1,subVol);
				volume_conver_size(SUBVOICE2,subVol);
			}
			if(subVol >=10)
			{
				subVol = 10;
				Draw_sub_vicon(subVol);
			}
			
			
		}
			printc("~~uuuuuuuuuuuuuuu~MENU_RIGHT run~~~\r\n");
#endif
            }
            break;

        case MENU_OK            :
        case MENU_SET_ONE		:

			  printc("~~~~~mainpage~~~~~~~MENU_OK~~%d~ulParam = %d~\r\n",ulEvent,ulParam);//long
		
            if(ulEvent==MENU_SET_ONE){	
                pMenu->iCurrentPos = ulParam;
                pMenu->iSelected   = ulParam;
                pMenu->bSetOne   = 1;
                pMenu->uiStatus |= MENU_STATUS_PRESSED;
                pMenu->uiStatus |= MENU_STATUS_ITEM_TOUCHED;

		 printc("~~~~a~~~~~~~~mainpage~~~~~~~MENU_OK~~%d~ulParam = %d~\r\n",ulEvent,ulParam);//long
            }
            else{
                pMenu->bSetOne   = 1;

				//if(bluetooth_key == AHC_FALSE)
                	flagJump = pMenu->iSelected = pMenu->iCurrentPos;
			//	else
				//	pMenu->iCurrentPos = ulParam; // lyj 20181027
                pMenu->uiStatus |= MENU_STATUS_PRESSED;

		 printc("~~~b~~~~~~~~~~mainpage~~~~~~~MENU_OK~~%d~ulParam = %d~\r\n",ulEvent,ulParam);//long
            }

            if( CommonMenuOK( pMenu, AHC_TRUE ) == AHC_TRUE ){
#if (TOP_MENU_PAGE_EN)
                PSMENUSTRUCT pCurrMenu = NULL;

                SetCurrentMenu(&sTopMenu);
                pCurrMenu = GetCurrentMenu();
                if( pCurrMenu == NULL ){
                    return MENU_ERR_EXIT;
                }

                pCurrMenu->pfEventHandler(pCurrMenu, MENU_INIT, 0);
#endif
				//bluetooth_key = AHC_FALSE; // lyj
                return MENU_ERR_OK;
            }
			//bluetooth_key = AHC_FALSE; // lyj
            break;


	//===============================lyj20180604====================================



	//===============================end===================================

        case MENU_MENU          :
        
        	if(Deleting_InBrowser || Protecting_InBrowser){
            	printc("Delete/Protect/UnProtect Change to Browser Mode2\r\n");
            	// To check is there file in card, it no any file
                // to reset Delete/(Un)Protect file in broswer flags.
                // Otherwise those flag will make key/device events to
                // be NULL!! UI will be stuck
                BrowserFunc_ThumbnailEditCheckFileObj();
                return MENU_ERR_FORCE_BROWSER_EXIT;
            }
            else
            {
		
             printc("~~~~~menu~~~~~~~~mainpage~~~~~~~~%d~ulParam = %d~\r\n",ulEvent,ulParam);//long
            	Main_Set_Page(PREVIEW_FLAG);// liao
           	 if(Menu_To_Video_One)// liao 20180316 按下menu_menu.执行模式跳转
            		{
            		 printc("~~~1111~~~~~~~~~mainpage~~~~~~~~ =~\r\n");//long
            		Menu_To_Video_One=AHC_FALSE;
			//OpenThePreviwVideoPower();// lyj 20190226
                	StateSwitchMode(UI_VIDEO_STATE);//匹配什么模式,录像模式
            	}else
            		{
	                PSMENUSTRUCT pParentMenu;

					//Menu_To_Video_One=AHC_TRUE; // lyj 20180605

	                pMenu->bSetOne     = 0;
	                pParentMenu        = pMenu->pParentMenu;
	                pMenu->pParentMenu = NULL;

	                if( pParentMenu == NULL ){
				OpenThePreviwVideoPower();// lyj 20190226
	                    return MENU_ERR_EXIT;
	                }

	                SetCurrentMenu(pParentMenu);//回到bluetooth主界面
	                pParentMenu->pfEventHandler(pParentMenu, MENU_INIT, 0);//界面初始化
	                 printc("~~~1111~~~~~~~~~mainpage~~~~~~~~ =~\r\n");//long
            	}

		
	
            }
            break;
	#if 1
        case MENU_POWER_OFF     :
		//	StateSwitchMode(UI_VIDEO_STATE);// liao 20180316
		//ReserveTheRadio_AM_FM(); // lyj 20190114 bao cun dian tai
            AHC_PowerOff_NormalPath();
            break; 
	#endif

        default:
            return MENU_ERR_OK;//MENU_ERR_NOT_MENU_EVENT;
            break;
    }

    return MENU_ERR_OK;
}

#if (SUPPORT_TOUCH_PANEL)
UINT32 MainMenuItem_Touch(UINT16 pt_x,UINT16 pt_y)
{
	UINT32  uiNextEvent = MENU_NONE;
	UINT32  TouchEvent;
	UINT16  i = 0;
	PSMENUSTRUCT pMenu;
	int iPrevPos;
	
	pMenu=GetCurrentMenu();

  	i = (pt_y - OSD_MENU_ITEM_Y )/( OSD_MENU_ITEM_H + OSD_MENU_ITEM_INTERVAL );
    i = (pMenu->iCurrentPos/MAIN_MENU_PAGE_ITEM)*MAIN_MENU_PAGE_ITEM + i;
	printc("MainMenuItem_Touch %d,ofItem %d\r\n",i,pMenu->iNumOfItems);
	if(i < pMenu->iNumOfItems)
	{
		if(i ==  pMenu->iCurrentPos)
			MainMenuEventHandler(pMenu,MENU_SET_ONE,i);
		else
		{
			iPrevPos = pMenu->iCurrentPos;
			pMenu->iCurrentPos = i;
			MenuDrawChangeItem( pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected );
		}
	}
	return MENU_ERR_OK;
}
#endif

extern MMP_USHORT FMCurruntFreq;
extern MMP_USHORT AMCurruntFreq;
AHC_BOOL displayFreqflag = 0;// lyj 20190215
AHC_BOOL displayFreqflagAM=0;

extern AHC_BOOL bl_flag;
//Common SubMenuEventHandler.子菜单处理
UINT32 SubMenuEventHandler(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam)
{
    INT32 	iPrevPos;
#if(SUPPORT_TOUCH_PANEL)       
    POINT	TouchPoint;
#endif

#if(SUPPORT_TOUCH_PANEL) 
    if( MENU_TOUCH == ulEvent || MENU_TOUCH_MOVE == ulEvent )
    {
        TouchPoint.PointX = ulParam & 0xFFFF;
        TouchPoint.PointY = (ulParam >> 16) & 0xFFFF;
    }
#endif

	//volume_num = MenuSettingConfig()->uiVolume;
	//printc("~~~~uiVolume = %d~~~~~~\r\n",MenuSettingConfig()->uiVolume);
	//printc("~~~~uiVolume1 = %d~~~~~~\r\n",MenuSettingConfig()->uiMOVPowerOffTime);
	//printc("~~~~uiVolume2 = %d~~~~~~\r\n",MenuSettingConfig()->uiBatteryVoltage);

    switch(ulEvent)
    {
        case MENU_EXIT:
            return MENU_ERR_EXIT;
            break;	

        case MENU_INIT          :
            pMenu->uiStatus = MENU_STATUS_NONE;

			printc("~~~~~pfMenuGetDefaultVal~~~%d~~~~~\r\n",pMenu->pfMenuGetDefaultVal(pMenu)); // lyj

            if( pMenu->pfMenuGetDefaultVal ){
                pMenu->iSelected = pMenu->pfMenuGetDefaultVal(pMenu);
            }
            else{
                pMenu->iSelected = 0 ;
            }
            pMenu->iCurrentPos = pMenu->iSelected;
            pMenu->bSetOne     = 0;
			//printc("~~~2019~~pMenu->iCurrentPos == %d~~~~pMenu->iSelected == %d~~~~~\n",pMenu->iCurrentPos,pMenu->iSelected);

		#if (SUPPORT_TOUCH_PANEL)
            if( pMenu->iNumOfItems <= 2 )
                KeyParser_TouchItemRegister(&SubMenu2_TouchButton[0], ITEMNUM(SubMenu2_TouchButton));
            else if( pMenu->iNumOfItems <= 4 )
                KeyParser_TouchItemRegister(&SubMenu4_TouchButton[0], ITEMNUM(SubMenu4_TouchButton));
            else
                KeyParser_TouchItemRegister(&SubMenu6_TouchButton[0], ITEMNUM(SubMenu6_TouchButton));
		#endif			

            MenuDrawSubPage(pMenu);
            break;

        case MENU_UP            :

		#ifdef SLIDE_MENU
            if(IsSlidingMenu())
                break;
		#endif

            iPrevPos = pMenu->iCurrentPos;

		#if (DIR_KEY_TYPE==KEY_TYPE_2KEY)
		if(Menu_Get_Page() == 1 || Menu_Get_Page() == 2/* || Menu_Get_Page() == 4*/)
		{
			if(return_flag)
			{
				if(bl_flag)
				{
					iPrevPos = pMenu->iCurrentPos = pMenu->pfMenuGetDefaultVal(pMenu);
					if(Menu_Get_Page() == 1)
					 pMenu->iCurrentPos = 0;
					else
						 pMenu->iCurrentPos = 1;
					 return_flag = 0;
					// bl_flag = 0; // lyj 20190711
				}
			}
			else
			{
				iPrevPos = pMenu->iCurrentPos = pMenu->pfMenuGetDefaultVal(pMenu);// lyj 20190214
				 pMenu->iCurrentPos = OffsetItemNumber(pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, -1, AHC_TRUE);
				 //FMCurruntFreq = 0;
				//AMCurruntFreq =0;// lyj 20190214
			}

		}
		else
		{
			#ifdef BlueTOOthMACO
			if(Menu_Get_Page() == 0)
			{
				BlueMusicPrev();	
			}
			else if(Menu_Get_Page() == 6)// lyj 20181026
			{
				USBMusicPrev();
			}
			#endif
			else
            		pMenu->iCurrentPos = OffsetItemNumber(pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, -1, AHC_TRUE);
		}
		#elif defined(FONT_LARGE)
            pMenu->iCurrentPos = OffsetItemNumber(pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, -1, AHC_FALSE);
		#else
            pMenu->iCurrentPos = OffsetItemNumber(pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, -2, AHC_FALSE);
		#endif
		if(return_flag == 0){
			
				if(Menu_Get_Page() != 0&& Menu_Get_Page() != 6)
				MenuDrawChangeSubItem(pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected);//上下移动item,并重新绘制
				//printc("~~up~~pMenu->iCurrentPos == %d~~~~~iPrevPos == %d~~~~~~~\n",pMenu->iCurrentPos,iPrevPos);
				if(bl_flag)
				{
					return_flag = 1; // lyj 20190712
					bl_flag = 0;
				}
		}
		else
			{

//===================================================================================

				if(Menu_Get_Page() == 3)
				{
					switch(select_flag)

					{
					case 0:

						
				
					if(volume_num > 15 )
						volume_num = 15;
				
						if(volume_num <= 15 && volume_num > 0) // 取消等于lyj 20190427
						{

						--volume_num;
						
						MenuDrawChangeSubItem3(pMenu, volume_num, iPrevPos, pMenu->iSelected);
						MenuSettingConfig()->uiVolume = volume_num;
						if(volume_num == 0)
						{
							uart_to_mcu_voice(FRONT1,(MMP_BYTE)volume_num,MUTE_ON);
							uart_to_mcu_voice(FRONT2,(MMP_BYTE)volume_num,MUTE_ON);

							//volume_conver_size(FRONT1,volume_num);
							//volume_conver_size(FRONT2,volume_num);
						}
						else if(volume_num > 0)
						{
							//uart_to_mcu_voice(FRONT1,(MMP_BYTE)volume_num,MUTE_OFF);
							//uart_to_mcu_voice(FRONT2,(MMP_BYTE)volume_num,MUTE_OFF);

							volume_conver_size(FRONT1,volume_num);
							volume_conver_size(FRONT2,volume_num);

							
						}
						//printc("~~~~~~~~~~aaaaaaaaaaaa~~~pMenu->iCurrentPos = %d~~~\r\n",pMenu->iCurrentPos);
					
						}

						if(volume_num < 0)
							volume_num = 0;
						//printc("~~b~~~sub=%s~~%d~\r\n",__func__,__LINE__);
						break;

					case 1:

					
					if(volume_num1 > 15 )
						volume_num1 = 15;
				
						if(volume_num1 <= 15 && volume_num1 > 0)
						{
						--volume_num1;
						MenuDrawChangeSubItem3(pMenu, volume_num1, iPrevPos, pMenu->iSelected);
						MenuSettingConfig()->uiMOVPowerOffTime= volume_num1;
						if(volume_num1 == 0)
						{
							uart_to_mcu_voice(REAR1,(MMP_BYTE)volume_num1,MUTE_ON);
							uart_to_mcu_voice(REAR2,(MMP_BYTE)volume_num1,MUTE_ON);
						}
						else if(volume_num1 > 0)
						{
							//uart_to_mcu_voice(REAR1,(MMP_BYTE)volume_num1,MUTE_OFF);
							//uart_to_mcu_voice(REAR2,(MMP_BYTE)volume_num1,MUTE_OFF);

							volume_conver_size(REAR1,volume_num1);
							volume_conver_size(REAR2,volume_num1);
						}
						//printc("~~~~~~~~~~bbbbbbb~~~pMenu->iCurrentPos = %d~~~\r\n",pMenu->iCurrentPos);
					
						}

						if(volume_num1 < 0)
							volume_num1 = 0;

							break;


					case 2:


					if(volume_num2 > 15 )
						volume_num2= 15;
				
						if(volume_num2 <= 15 && volume_num2 > 0)
						{
						--volume_num2;
						
						MenuDrawChangeSubItem3(pMenu, volume_num2, iPrevPos, pMenu->iSelected);
						MenuSettingConfig()->uiEffect= volume_num2;
						if(volume_num2 == 0)
						{
							uart_to_mcu_voice(OUTPUT1,(MMP_BYTE)volume_num2,MUTE_ON);
							uart_to_mcu_voice(OUTPUT2,(MMP_BYTE)volume_num2,MUTE_ON);
						}
						else if(volume_num2 > 0)
						{
							//uart_to_mcu_voice(OUTPUT1,(MMP_BYTE)volume_num2,MUTE_OFF);
							//uart_to_mcu_voice(OUTPUT2,(MMP_BYTE)volume_num2,MUTE_OFF);

							volume_conver_size(OUTPUT1,volume_num2);
							volume_conver_size(OUTPUT2,volume_num2);

							
						}
						//printc("~~~~~~~~~~cccccccc~~~pMenu->iCurrentPos = %d~~~\r\n",pMenu->iCurrentPos);
					
						}

						if(volume_num2 < 0)
							volume_num2 = 0;

							break;

							default:
								
								//printc("~~~~~~~~~~ddddd~~~pMenu->iCurrentPos = %d~~~\r\n",pMenu->iCurrentPos);
								break;
						

					}

				}

				if(Menu_Get_Page() == 1)
				{
					if(/*FM_select == 1&& */MenuSettingConfig()->uiMOVQuality == 2)
					{
						#if 0
							//if(Mannal_Freq < 8750)
									//Mannal_Freq = 8750 // lyj 20181124
							if(Mannal_Freq >= 8750 && Mannal_Freq <= 10800)
							{
								Mannal_Freq -= 10;
								if(Mannal_Freq >= 8750)
								{
									FMCurruntFreq = Mannal_Freq;
									Get_vailed_Freq_manul(Mannal_Freq);
								}

								
							}
							else if(Mannal_Freq < 8750)
							{
								Mannal_Freq = 8750;
							}
							else if(Mannal_Freq > 10800)
							{
								Mannal_Freq = 10800;
							}
						
						#else
							
						Radio_z--;
						if(Radio_z < 20)
							 Radio_z = 30;
						printc("^_^~~~~~~x--~~~%s~~\n",__func__); // lyj 20190423
						if(radio[Radio_z]) // lyj 201090423
						{
							FMCurruntFreq = radio[Radio_z];// lyj 20190216
							
							Draw_FM_AM_icon_Ex(radio[Radio_z],wDispID);
							si47xxFMRX_tune(radio[Radio_z]);
							//if(Radio_i == 0)
								//Radio_i = 10; // lyj 20181124
						}
						else
						{
							while(1)
							{
								Radio_z--;
								if(Radio_z < 20)
							 		Radio_z = 20; 
								if(radio[Radio_z] || Radio_z== 20)
								{

									if( Radio_z == 20)
									{
										if(radio[20])
										{
											FMCurruntFreq = radio[20];// lyj 20190216
											Draw_FM_AM_icon_Ex(radio[20],wDispID);
											si47xxFMRX_tune(radio[Radio_z]);
											//if(Radio_i == 0)
												//Radio_i = 10; // lyj 20181124
											break;
										}
										else
											break;
									}
										
									FMCurruntFreq = radio[Radio_z];// lyj 20190216
									Draw_FM_AM_icon_Ex(radio[Radio_z],wDispID);
									si47xxFMRX_tune(radio[Radio_z]);
									break;
								}
							}

						}

				

						#endif
						//printc("~~~~~~~~~search down !~Mannal_Freq = %d~~~\r\n",Mannal_Freq);

					}

					if(MenuSettingConfig()->uiMOVQuality == 0)
					{
						Radio_i--;
						if(Radio_i < 0)
							 Radio_i = 10;

						if(radio[Radio_i])
						{
							FMCurruntFreq = radio[Radio_i];// lyj 20190216
							
							Draw_FM_AM_icon_Ex(radio[Radio_i],wDispID);
							si47xxFMRX_tune(radio[Radio_i]);
							//if(Radio_i == 0)
								//Radio_i = 10; // lyj 20181124
						}
						else
						{
							while(1)
							{
								Radio_i--;
								if(Radio_i < 0)
							 		Radio_i = 0;
								if(radio[Radio_i] || Radio_i == 0)
								{

									if( Radio_i == 0)
									{
										if(radio[0])
										{
											FMCurruntFreq = radio[0];// lyj 20190216
											Draw_FM_AM_icon_Ex(radio[0],wDispID);
											si47xxFMRX_tune(radio[Radio_i]);
											//if(Radio_i == 0)
												//Radio_i = 10; // lyj 20181124
											break;
										}
										else
											break;
									}
										
									FMCurruntFreq = radio[Radio_i];// lyj 20190216
									Draw_FM_AM_icon_Ex(radio[Radio_i],wDispID);
									si47xxFMRX_tune(radio[Radio_i]);
									break;
								}
							}

						}

					}

					if(MenuSettingConfig()->uiMOVQuality == 1)
					{
						Radio_y--;
						if(Radio_y < 10)
							 Radio_y = 20;

						if(radio[Radio_y])
						{
							FMCurruntFreq = radio[Radio_y];// lyj 20190216
							
							Draw_FM_AM_icon_Ex(radio[Radio_y],wDispID);
							si47xxFMRX_tune(radio[Radio_y]);
							//if(Radio_y == 10)
								//Radio_y = 20; // lyj 20181124
						}
						else
						{
							while(1)
							{
								Radio_y--;
								if(Radio_y< 10)
							 		Radio_y = 10;
								if(radio[Radio_y] || Radio_y == 10)
								{

									if( Radio_y == 10)
									{
										if(radio[10])
										{
											FMCurruntFreq = radio[10];// lyj 20190216
											Draw_FM_AM_icon_Ex(radio[10],wDispID);
											si47xxFMRX_tune(radio[Radio_y]);
											//if(Radio_y == 10)
												//Radio_y = 20; // lyj 20181124
											break;
										}
										else
											break;
									}
										
									FMCurruntFreq = radio[Radio_y];// lyj 20190216
									Draw_FM_AM_icon_Ex(radio[Radio_y],wDispID);
									si47xxFMRX_tune(radio[Radio_y]);
									break;
								}
							}

						}
						



					}



				}

				if(Menu_Get_Page() == 2)
				{
					if(AM_select == 1&& MenuSettingConfig()->uiBeep == 0)
					{
					
							if(AM_Freq >= 530 && AM_Freq <= 1710)
							{
								//AM_Freq -= 9;
								AM_Freq -= 10;//lyj 20190216
								if(AM_Freq >= 530)
								{
									AMCurruntFreq = AM_Freq;
									Get_vailed_am_Freq_manul(AM_Freq);
								}

							}
							else if(AM_Freq < 530)
							{
								AM_Freq = 530;
							}
							else if(AM_Freq > 1710)
							{
								AM_Freq = 1710;
							}
					

						//printc("~~~~~~am~~~search down!~Mannal_Freq = %d~~~\r\n",AM_Freq );

					}

					if(MenuSettingConfig()->uiBeep == 1)
					{
						Radio_am_i--;
						if(Radio_am_i < 0)
							 Radio_am_i = 10;

						if(radio_am[Radio_am_i])
						{
							AMCurruntFreq = radio_am[Radio_am_i];
							Draw_AM_icon_Ex(radio_am[Radio_am_i],wDispID);
							si47xxAMRX_tune(radio_am[Radio_am_i]);
							//if(Radio_am_i == 0)
								//Radio_am_i = 10; // lyj 20181124
						}
						else
						{
							while(1)
							{
								Radio_am_i--;
								if(Radio_am_i < 0)
							 		Radio_am_i = 0;
								if(radio_am[Radio_am_i] || Radio_am_i == 0)
								{

									if( Radio_am_i == 0)
									{
										if(radio_am[0])
										{
											AMCurruntFreq = radio_am[0];
											Draw_AM_icon_Ex(radio_am[0],wDispID);
											si47xxAMRX_tune(radio_am[Radio_am_i]);
											//if(Radio_am_i == 0)
												//Radio_am_i = 10; // lyj 20181124
											break;
										}
										else
											break;
									}
										
									AMCurruntFreq = radio_am[Radio_am_i];
									Draw_AM_icon_Ex(radio_am[Radio_am_i],wDispID);
									si47xxAMRX_tune(radio_am[Radio_am_i]);
									break;
								}
							}

						}

					}

					if(MenuSettingConfig()->uiBeep == 2)
					{
						Radio_am_y--;
						if(Radio_am_y < 10)
							 Radio_am_y = 20;

						if(radio_am[Radio_am_y])
						{
							AMCurruntFreq = radio_am[Radio_am_y];
							Draw_AM_icon_Ex(radio_am[Radio_am_y],wDispID);
							si47xxAMRX_tune(radio_am[Radio_am_y]);
								//if(Radio_am_y == 10)
								//Radio_am_y = 20; // lyj 20181124
						}
						else
						{
							while(1)
							{
								Radio_am_y--;
								if(Radio_am_y< 10)
							 		Radio_am_y = 10;
								if(radio_am[Radio_am_y] || Radio_am_y == 10)
								{

									if( Radio_am_y == 10)
									{
										if(radio_am[10])
										{
											AMCurruntFreq = radio_am[10];
											Draw_AM_icon_Ex(radio_am[10],wDispID);
											si47xxAMRX_tune(radio_am[Radio_am_y]);
											//if(Radio_am_y == 10)
												//Radio_am_y = 20; // lyj 20181124
											break;
										}
										else
											break;
									}
										
									AMCurruntFreq = radio_am[Radio_am_y];
									Draw_AM_icon_Ex(radio_am[Radio_am_y],wDispID);
									si47xxAMRX_tune(radio_am[Radio_am_y]);
									break;
								}
							}

						}
						



					}


				}


			}
            break;

        case MENU_DOWN          :

		#ifdef SLIDE_MENU
            if(IsSlidingMenu())
                break;
		#endif

            iPrevPos = pMenu->iCurrentPos;

		#if (DIR_KEY_TYPE==KEY_TYPE_2KEY)

		if(Menu_Get_Page() == 1 || Menu_Get_Page() == 2 /*|| Menu_Get_Page() == 4*/)
		{
			if(return_flag)
			{}
			else
			{
				iPrevPos = pMenu->iCurrentPos = pMenu->pfMenuGetDefaultVal(pMenu);
				//FMCurruntFreq = 0;
				//AMCurruntFreq =0;// lyj 20190214
				//printc("~1111~down~~pMenu->iCurrentPos == %d~~~~~iPrevPos == %d~~~~~~~\n",pMenu->iCurrentPos,iPrevPos);
				pMenu->iCurrentPos = OffsetItemNumber(pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, 1, AHC_TRUE);
				//printc("~222~down~~pMenu->iCurrentPos == %d~~~~~iPrevPos == %d~~~~~~~\n",pMenu->iCurrentPos,iPrevPos);
			}

		}
		else
		{	
		#ifdef BlueTOOthMACO
		if(Menu_Get_Page() == 0)
				{
					 BlueMusicNext();	
				}
			
			else if(Menu_Get_Page() == 6)// lyj 20181026
			{
				USBMusicNext();
			}
			else
			#endif
            		pMenu->iCurrentPos = OffsetItemNumber(pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, 1, AHC_TRUE);
		}
		#elif defined(FONT_LARGE)
            pMenu->iCurrentPos = OffsetItemNumber(pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, 1, AHC_FALSE);
		#else
            pMenu->iCurrentPos = OffsetItemNumber(pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, 2, AHC_FALSE);
		#endif
		if(return_flag ==  0)
		{
			if(Menu_Get_Page() != 0&&Menu_Get_Page() != 6)
	            MenuDrawChangeSubItem(pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected);

			
		}
		else
			{
			#if 1
			
			if(Menu_Get_Page() == 3)
				
			{		switch(select_flag)

					{
					case 0:

						//volume_num = MenuSettingConfig()->uiVolume;
				
						if(volume_num < 0)//MenuSettingConfig()->uiVolume < 0
								volume_num = 0;
				
						if(volume_num < 15 && volume_num >= 0)
						{

						++volume_num;
						MenuDrawChangeSubItem3(pMenu, volume_num, iPrevPos, pMenu->iSelected);
						MenuSettingConfig()->uiVolume = volume_num;
						//uart_to_mcu_voice(FRONT1,(MMP_BYTE)volume_num,MUTE_OFF);
						//uart_to_mcu_voice(FRONT2,(MMP_BYTE)volume_num,MUTE_OFF);
						volume_conver_size(FRONT1,volume_num);
						volume_conver_size(FRONT2,volume_num);
						//printc("~~~~~~~~~~aaaaaaaaaaaa~~~pMenu->iCurrentPos = %d~~~\r\n",pMenu->iCurrentPos);
					
						}

						if(volume_num > 15)
							volume_num = 10;
						printc("~~b~~~sub=%s~~%d~\r\n",__func__,__LINE__); break;

					case 1:

					
							if(volume_num1 < 0)
								volume_num1 = 0;
				
						if(volume_num1 < 15 && volume_num1 >= 0)
						{

						++volume_num1;
						MenuDrawChangeSubItem3(pMenu, volume_num1, iPrevPos, pMenu->iSelected);
						MenuSettingConfig()->uiMOVPowerOffTime= volume_num1;
						//uart_to_mcu_voice(REAR1,(MMP_BYTE)volume_num1,MUTE_OFF);
						//uart_to_mcu_voice(REAR2,(MMP_BYTE)volume_num1,MUTE_OFF);
						volume_conver_size(REAR1,volume_num1);
						volume_conver_size(REAR2,volume_num1);
						//printc("~~~~~~~~~~bbbbbbb~~~pMenu->iCurrentPos = %d~~~\r\n",pMenu->iCurrentPos);
					
						}

						if(volume_num1 > 15)
							volume_num1 = 10;

							break;


					case 2:


							if(volume_num2 < 0)
								volume_num2 = 0;
				
						if(volume_num2 < 15 && volume_num2 >= 0)
						{

						++volume_num2;
						MenuDrawChangeSubItem3(pMenu, volume_num2, iPrevPos, pMenu->iSelected);
						MenuSettingConfig()->uiEffect= volume_num2;
						//uart_to_mcu_voice(OUTPUT1,(MMP_BYTE)volume_num2,MUTE_OFF);
						//uart_to_mcu_voice(OUTPUT2,(MMP_BYTE)volume_num2,MUTE_OFF);
						volume_conver_size(OUTPUT1,volume_num2);
						volume_conver_size(OUTPUT2,volume_num2);
						//printc("~~~~~~~~~~cccccccc~~~pMenu->iCurrentPos = %d~~~\r\n",pMenu->iCurrentPos);
					
						}

						if(volume_num2 >15)
							volume_num2 = 10;

							break;

							default:
								
								printc("~~~~~~~~~~ddddd~~~pMenu->iCurrentPos = %d~~~\r\n",pMenu->iCurrentPos);
								break;
						

					}

				}

				if(Menu_Get_Page() == 1)
				{
					if(/*FM_select == 1&&*/MenuSettingConfig()->uiMOVQuality == 2)
					{
						#if 0
							//if(Mannal_Freq > 10800)
									//Mannal_Freq = 10800; // lyj 20181124
							if(Mannal_Freq >= 8750 && Mannal_Freq <= 10800)
							{


								Mannal_Freq += 10;
								FMCurruntFreq = Mannal_Freq;//lyj 20190216
						
								Get_vailed_Freq_manul(Mannal_Freq);

								
							}
							else if(Mannal_Freq < 8750)
							{
								Mannal_Freq = 8750;
							}
							else if(Mannal_Freq > 10800)
							{
								Mannal_Freq = 10800;
							}
				
						#else

					
						Radio_z++;
						if(Radio_z >= 30)
							 Radio_z = 20;
						printc("^_^~~~~~~x++~~~%s~~\n",__func__); // lyj 20190423
						if(radio[Radio_z])
						{
							FMCurruntFreq = radio[Radio_z];// lyj 20190213
							Draw_FM_AM_icon_Ex(radio[Radio_z],wDispID);
							si47xxFMRX_tune(radio[Radio_z]);
						}
						else
						{
							while(1)
							{
								Radio_z++;
								if(Radio_z >= 30)
							 		Radio_z = 20;
								if(radio[Radio_z] || Radio_z == 20)
								{

									if( Radio_z == 20)
									{
										if(radio[20])
										{
											FMCurruntFreq = radio[20];// lyj 20190213
											Draw_FM_AM_icon_Ex(radio[20],wDispID);
											si47xxFMRX_tune(radio[20]);
											break;
										}
										else
											break;
									}
										
									FMCurruntFreq = radio[Radio_z];// lyj 20190213
									Draw_FM_AM_icon_Ex(radio[Radio_z],wDispID);
									si47xxFMRX_tune(radio[Radio_z]);
									break;
								}
							}

						}

					

						#endif
						//printc("~~~~~~~~~search up !~Mannal_Freq = %d~~~\r\n",(Mannal_Freq - 10));
					}

					if(MenuSettingConfig()->uiMOVQuality == 0)
					{
						Radio_i++;
						if(Radio_i >= 10)
							 Radio_i = 0;

						if(radio[Radio_i])
						{
							FMCurruntFreq = radio[Radio_i];// lyj 20190213
							Draw_FM_AM_icon_Ex(radio[Radio_i],wDispID);
							si47xxFMRX_tune(radio[Radio_i]);
						}
						else
						{
							while(1)
							{
								Radio_i++;
								if(Radio_i >= 10)
							 		Radio_i = 0;
								if(radio[Radio_i] || Radio_i == 0)
								{

									if( Radio_i == 0)
									{
										if(radio[0])
										{
											FMCurruntFreq = radio[0];// lyj 20190213
											Draw_FM_AM_icon_Ex(radio[0],wDispID);
											si47xxFMRX_tune(radio[0]);
											break;
										}
										else
											break;
									}
										
									FMCurruntFreq = radio[Radio_i];// lyj 20190213
									Draw_FM_AM_icon_Ex(radio[Radio_i],wDispID);
									si47xxFMRX_tune(radio[Radio_i]);
									break;
								}
							}

						}

					}

					if(MenuSettingConfig()->uiMOVQuality == 1)
					{
						Radio_y++;
						if(Radio_y >= 20)
							 Radio_y = 10;

						if(radio[Radio_y])
						{
							FMCurruntFreq = radio[Radio_y];// lyj 20190213
							Draw_FM_AM_icon_Ex(radio[Radio_y],wDispID);
							si47xxFMRX_tune(radio[Radio_y]);
						}
						else
						{
							while(1)
							{
								Radio_y++;
								if(Radio_y >= 20)
							 		Radio_y = 10;
								if(radio[Radio_y] || Radio_y == 10)
								{

									if( Radio_y == 10)
									{
										if(radio[10])
										{
											FMCurruntFreq = radio[10];// lyj 20190213
											Draw_FM_AM_icon_Ex(radio[10],wDispID);
											si47xxFMRX_tune(radio[10]);
											break;
										}
										else
											break;
									}
										
									FMCurruntFreq = radio[Radio_y];// lyj 20190213
									Draw_FM_AM_icon_Ex(radio[Radio_y],wDispID);
									si47xxFMRX_tune(radio[Radio_y]);
									break;
								}
							}

						}

					}

				}

				if(Menu_Get_Page() == 2)
				{
					if(AM_select == 1&&MenuSettingConfig()->uiBeep == 0)
					{
	
							//if(AM_Freq >= 522 && AM_Freq <= 1620)
							if(AM_Freq >= 530 && AM_Freq <= 1710)
							{
								//AM_Freq += 9;
								AM_Freq += 10;//lyj 20190216
								AMCurruntFreq = AM_Freq;

								Get_vailed_am_Freq_manul(AM_Freq);

							}
							else if(AM_Freq < 530)
							{
								AM_Freq = 530;
							}
							else if(AM_Freq > 1710)
							{
								AM_Freq = 1710;
							}
						

						//printc("~~~~~~am~~~search up !~Mannal_Freq = %d~~~\r\n",(AM_Freq - 10));

					}


					if(MenuSettingConfig()->uiBeep == 1)
					{
						Radio_am_i++;
						if(Radio_am_i >= 10)
							 Radio_am_i = 0;

						if(radio_am[Radio_am_i])
						{
							AMCurruntFreq = radio_am[Radio_am_i];
							Draw_AM_icon_Ex(radio_am[Radio_am_i],wDispID);
							si47xxAMRX_tune(radio_am[Radio_am_i]);
						}
						else
						{
							while(1)
							{
								Radio_am_i++;
								if(Radio_am_i >= 10)
							 		Radio_am_i = 0;
								if(radio_am[Radio_am_i] || Radio_am_i == 0)
								{

									if( Radio_am_i == 0)
									{
										if(radio_am[0])
										{
											AMCurruntFreq = radio_am[0];
											Draw_AM_icon_Ex(radio_am[0],wDispID);
											si47xxAMRX_tune(radio_am[Radio_am_i]);
											break;
										}
										else
											break;
									}
										
									AMCurruntFreq = radio_am[Radio_am_i];
									Draw_AM_icon_Ex(radio_am[Radio_am_i],wDispID);
									si47xxAMRX_tune(radio_am[Radio_am_i]);
									break;
								}
							}

						}

					}

					if(MenuSettingConfig()->uiBeep == 2)
					{
						Radio_am_y++;
						if(Radio_am_y >= 20)
							 Radio_am_y = 10;

						if(radio_am[Radio_am_y])
						{
							AMCurruntFreq = radio_am[Radio_am_y];
							Draw_AM_icon_Ex(radio_am[Radio_am_y],wDispID);
							si47xxAMRX_tune(radio_am[Radio_am_y]);
						}
						else
						{
							while(1)
							{
								Radio_am_y++;
								if(Radio_am_y >= 20)
							 		Radio_am_y = 0;
								if(radio_am[Radio_am_y] || Radio_am_y == 10)
								{

									if( Radio_am_y == 10)
									{
										if(radio_am[10])
										{
											AMCurruntFreq = radio_am[10];
											Draw_AM_icon_Ex(radio_am[10],wDispID);
											si47xxAMRX_tune(radio_am[Radio_am_y]);
											break;
										}
										else
											break;
									}
										
									AMCurruntFreq = radio_am[Radio_am_y];
									Draw_AM_icon_Ex(radio_am[Radio_am_y],wDispID);
									si47xxAMRX_tune(radio_am[Radio_am_y]);
									break;
								}
							}

						}

					}


				}
			
				#endif
			}
			
            break;


        case MENU_OK            :

		#if 1   
			if(Menu_Get_Page() == 3)
				{
			

				if(return_flag == 0)
				{
					
				    sub_menu_page = pMenu->iCurrentPos;
					//printc("~~~sub_menu_page = %d~~~\r\n",sub_menu_page);
				
					if(sub_menu_page == 0)
					{
						
						select_flag = 0;
						if(volume_num == -1)
						{
							volume_num = MenuSettingConfig()->uiVolume;
							if(volume_num == 0)
							{
								uart_to_mcu_voice(FRONT1,(MMP_BYTE)volume_num,MUTE_ON); // add voice
								uart_to_mcu_voice(FRONT2,(MMP_BYTE)volume_num,MUTE_ON); // add voice
							}
							else if(volume_num > 0)
							{
								//uart_to_mcu_voice(FRONT1,(MMP_BYTE)volume_num,MUTE_OFF); // add voice
								//uart_to_mcu_voice(FRONT2,(MMP_BYTE)volume_num,MUTE_OFF);

								volume_conver_size(FRONT1,volume_num);
								volume_conver_size(FRONT2,volume_num);

							}
							
						}
						MenuDrawChangeSubItem3(pMenu, volume_num, iPrevPos, pMenu->iSelected);
						MenuSettingConfig()->uiVolume = volume_num;

						

						return_flag = 1;
					}
					else if(sub_menu_page ==1)
					{
						select_flag = 1;
						
						if(volume_num1 == -1)
						{
							volume_num1 = MenuSettingConfig()->uiMOVPowerOffTime;
							if(volume_num1 == 0)
							{
								uart_to_mcu_voice(REAR1,(MMP_BYTE)volume_num1,MUTE_ON); // add voice
								uart_to_mcu_voice(REAR2,(MMP_BYTE)volume_num1,MUTE_ON); // add voice
							}
							else if(volume_num1 > 0)
							{
								//uart_to_mcu_voice(FRONT1,(MMP_BYTE)volume_num1,MUTE_OFF); // add voice
								//uart_to_mcu_voice(FRONT2,(MMP_BYTE)volume_num1,MUTE_OFF);
								volume_conver_size(REAR1,volume_num1);
								volume_conver_size(REAR2,volume_num1);

							}
						}
						
						MenuDrawChangeSubItem3(pMenu, volume_num1, iPrevPos, pMenu->iSelected);
						MenuSettingConfig()->uiMOVPowerOffTime= volume_num1;
						
						
						return_flag = 1;
					}
					else if(sub_menu_page ==2)
					{
						
						select_flag = 2;

						if(volume_num2 == -1)
						{
							volume_num2 = MenuSettingConfig()->uiEffect;
							if(volume_num2 == 0)
							{
								uart_to_mcu_voice(OUTPUT1,(MMP_BYTE)volume_num2,MUTE_ON); // add voice
								uart_to_mcu_voice(OUTPUT2,(MMP_BYTE)volume_num2,MUTE_ON); // add voice
							}
							else if(volume_num2 > 0)
							{
								//uart_to_mcu_voice(FRONT1,(MMP_BYTE)volume_num2,MUTE_OFF); // add voice
								//uart_to_mcu_voice(FRONT2,(MMP_BYTE)volume_num2,MUTE_OFF);

								volume_conver_size(OUTPUT1,volume_num2);
								volume_conver_size(OUTPUT2,volume_num2);

							}
						}
						MenuDrawChangeSubItem3(pMenu, volume_num2, iPrevPos, pMenu->iSelected);
						MenuSettingConfig()->uiEffect= volume_num2;

						
						return_flag = 1;

					}

				}
				else{
						pMenu->uiStatus = MENU_STATUS_NONE;
						pMenu->iSelected = pMenu->pfMenuGetDefaultVal(pMenu);
						pMenu->iCurrentPos = pMenu->iSelected;
           				pMenu->bSetOne     = 0;
						
						MenuDrawSubPage(pMenu);
						 return_flag = 0;
						// volume_num = 0;
					}
			}

			else if(Menu_Get_Page() == 1)
			{
			
				if(return_flag == 0)
				{
					pMenu->iCurrentPos = pMenu->pfMenuGetDefaultVal(pMenu);//lyj 20190215
					switch(pMenu->iCurrentPos)
					{
						case 3:

							{ 
								MMP_UBYTE i;
								for(i = 0; i < 31; i++)
		
								 radio[i] = 0;
							}
							//fm_si47xx_init();
							//Draw_FM_AM_icon(8750);
							return_flag = 1;
							switch_search = 1;
							Tfreq = 8750;
							Radiocnt = 0;
							FMCurruntFreq = 0;
							//Get_vailed_Freq();
							//test_FMRXautoseek();
							
	
							break;
						case 2:
							#if 0
							if(!FMCurruntFreq)
								Mannal_Freq = 8750;
							else
								Mannal_Freq = FMCurruntFreq;// lyj 20190216

							FM_select = 1;
							
							return_flag = 1;
							//FMCurruntFreq = 0;
							Get_vailed_Freq_manul(Mannal_Freq);
							#endif

							
								#if 0
								 radio[20] = 8880;
								 radio[21] = 8890;
								 radio[22] = 9880;
								  radio[23] = 10060;
								 radio[24] = 10020;
								 radio[25] = 10120;
								  radio[26] = 10190;
								 radio[27] = 10280;
								 radio[28] = 10320;
								  radio[29] = 10610;
								  #endif
								  // Reserved_radio(radio);
							
							if(radio[20] != 0)
							{
								si47xxFMRX_tune(radio[20]);
								Draw_FM_AM_icon_Ex(radio[20],wDispID);
								FMCurruntFreq = radio[20];
								displayFreqflag = 1;
							}
							else
							{
								si47xxFMRX_tune(10430);
								Draw_FM_AM_icon_Ex(10430,wDispID);
								FMCurruntFreq = 10430;
								displayFreqflag = 1;
							}

							  //Reserved_radio(radio,wDispID,0);// lyj 20190606
							return_flag = 1; break;
							
						case 0: 
						
								#if 0
								 radio[0] = 8860;
								 radio[1] = 8890;
								 radio[2] = 9860;
								  radio[3] = 10060;
								 radio[4] = 10070;
								 radio[5] = 10120;
								  radio[6] = 10190;
								 radio[7] = 10280;
								 radio[8] = 10390;
								  radio[9] = 10610;
								  #endif
								  // Reserved_radio(radio);
							

							
							if(radio[0] != 0)
							{
								si47xxFMRX_tune(radio[0]);
								Draw_FM_AM_icon_Ex(radio[0],wDispID);
								FMCurruntFreq = radio[0];
								displayFreqflag = 1;
							}
							else
							{
								si47xxFMRX_tune(9050);
								Draw_FM_AM_icon_Ex(9050,wDispID);
								FMCurruntFreq = 9050;
								displayFreqflag = 1;
							}
							return_flag = 1;

						
								// Reserved_radio(radio,wDispID,0);// lyj 20190606
								 //display_radio =1;


						
							break;
						case 1: 

					
								#if 0
								 radio[10] = 8880;
								 radio[11] = 8890;
								 radio[12] = 9880;
								  radio[13] = 10060;
								 radio[14] = 10020;
								 radio[15] = 10120;
								  radio[16] = 10190;
								 radio[17] = 10280;
								 radio[18] = 10320;
								  radio[19] = 10610;
								  #endif
								  // Reserved_radio(radio);
						
							if(radio[10] != 0)
							{
								si47xxFMRX_tune(radio[10]);
								Draw_FM_AM_icon_Ex(radio[10],wDispID);
								FMCurruntFreq = radio[10];
								displayFreqflag = 1;
							}
							else
							{
								si47xxFMRX_tune(10870);
								Draw_FM_AM_icon_Ex(10870,wDispID);
								FMCurruntFreq = 10870;
								displayFreqflag = 1;
							}

							 // Reserved_radio(radio,wDispID,0);// lyj 20190606
							return_flag = 1; break;
					}

					if( MenuSettingConfig()->uiMOVQuality == 4)
						{
								if(!FMCurruntFreq)
								Mannal_Freq = 8750;
							else
								Mannal_Freq = FMCurruntFreq;// lyj 20190216
							
								//if(Mannal_Freq < 8750)
										//Mannal_Freq = 8750 // lyj 20181124
								if(Mannal_Freq >= 8750 && Mannal_Freq <= 10790)
								{
									Mannal_Freq -= 20;
									if(Mannal_Freq >= 8750)
									{
										FMCurruntFreq = Mannal_Freq;
										Get_vailed_Freq_manul(Mannal_Freq);
									}

									
								}
								else if(Mannal_Freq < 8750)
								{
									Mannal_Freq = 8750;
								}
								else if(Mannal_Freq > 10790)
								{
									Mannal_Freq = 10790;
								}
							

							//printc("~~~~~~~~~search down !~Mannal_Freq = %d~~~\r\n",Mannal_Freq);

						}

						else if( MenuSettingConfig()->uiMOVQuality == 5)
						{

							if(!FMCurruntFreq)
								Mannal_Freq = 8750;
							else
								Mannal_Freq = FMCurruntFreq;// lyj 20190216
							//if(Mannal_Freq > 10800)
									//Mannal_Freq = 10800; // lyj 20181124
							if(Mannal_Freq >= 8750 && Mannal_Freq <= 10800)
							{


								Mannal_Freq += 20;// 10
								FMCurruntFreq = Mannal_Freq;//lyj 20190216
						
								Get_vailed_Freq_manul(Mannal_Freq);

								
							}
							else if(Mannal_Freq < 8750)
							{
								Mannal_Freq = 8750;
							}
							else if(Mannal_Freq > 10790)
							{
								Mannal_Freq = 10790;
							}
				

						//printc("~~~~~~~~~search up !~Mannal_Freq = %d~~~\r\n",(Mannal_Freq - 10));
						}//lyj 20190226

				}
				else
				{
						//pMenu->uiStatus = MENU_STATUS_NONE;
						//pMenu->iSelected = pMenu->pfMenuGetDefaultVal(pMenu);
						//pMenu->iCurrentPos = pMenu->iSelected;
           				//pMenu->bSetOne     = 0;
						
						//MenuDrawSubPage(pMenu);
						//fm_si47xx_exit();
						FM_select = 0;
						 return_flag = 0;
						//  display_radio =0;

						 if(pMenu->iCurrentPos == 3)
						 {
						 
							if(radio[0] != 0 && switch_search !=0)
							{
								si47xxFMRX_tune(radio[0]);
								Draw_FM_AM_icon_Ex(radio[0],wDispID);
							}

								switch_search =0;
								//Radiocnt = 0;
						 }
						 else if(pMenu->iCurrentPos == 1)
						 {
						 	displayFreqflag = 0;
						 	//Reserved_radio(radio,wDispID,1);// lyj 20190606

						 }
						 else if(pMenu->iCurrentPos == 2)
						 {
						 	displayFreqflag=0;
							//Reserved_radio(radio,wDispID,1);// lyj 20190606
						 }
						  else if(pMenu->iCurrentPos == 0)
						 {
						 	displayFreqflag=0;
							//Reserved_radio(radio,wDispID,1);// lyj 20190606
						 }

						 
						 	
				}
			}

			else if(Menu_Get_Page() == 2)
			{
				 
				if(return_flag == 0)
				{
					pMenu->iCurrentPos = pMenu->pfMenuGetDefaultVal(pMenu);//lyj 20190215
					switch(pMenu->iCurrentPos)
					{
						case 3:	
							//am_si47xx_init();
							{ 
								MMP_UBYTE i;
								for(i = 0; i < 21; i++)
		
								 radio_am[i] = 0;
							}
							return_flag = 1;
							//Get_vailed_am_Freq();

							switch_search = 1;
							Tfreq = 530;
							//Radiocnt = 0;
							RadiocntAm = 0;
							AMCurruntFreq = 0;
							 break;
						case 0:

							#if 0
							if(radio[0] != 0)
								si47xxAMRX_tune(radio[0]);
							else
							{
								si47xxAMRX_tune(747);
								Draw_AM_icon(747);
							}
							
							#endif
							if(!AMCurruntFreq)
							AM_Freq = 530;
							else
								AM_Freq = AMCurruntFreq;
							AM_select = 1;
							return_flag = 1;
							//AMCurruntFreq = 0;

							Get_vailed_am_Freq_manul(AM_Freq);
							
							 break;
						case 1: 

							{
								#if 0
								radio_am[0]	= 747;	
								radio_am[1]	= 549;
								radio_am[2]	= 594;
								radio_am[3]	= 603;
								radio_am[4]	= 630;
								radio_am[5]	= 657;
								radio_am[6]	= 936;
								radio_am[7]	= 1044;
								radio_am[8]	= 1053;
								radio_am[9]	= 1071;
								#endif
								// Reserved_radio(radio_am);
							}
							if(radio_am[0] != 0)
							{
								displayFreqflagAM = 1;
								AMCurruntFreq = radio_am[0];
								si47xxAMRX_tune(radio_am[0]);
								Draw_AM_icon_Ex(radio_am[0],wDispID);
							}
							else
							{
								displayFreqflagAM = 1;
								AMCurruntFreq = 750;
								si47xxAMRX_tune(750);
								Draw_AM_icon_Ex(750,wDispID);
							}
							return_flag = 1; 

							//Reserved_radio(radio_am,wDispID,0);// lyj 20190606
							break;
						case 2: 

							{
									#if 0
								radio_am[10]	= 738;	
								radio_am[11]	= 549;
								radio_am[12]	= 594;
								radio_am[13]	= 603;
								radio_am[14]	= 630;
								radio_am[15]	= 666;
								radio_am[16]	= 936;
								radio_am[17]	= 1044;
								radio_am[18]	= 1053;
								radio_am[19]	= 1080;

								// Reserved_radio(radio_am);
								 #endif
							}
							if(radio_am[10] != 0)
							{
								displayFreqflagAM = 1;
								AMCurruntFreq = radio_am[10];
								si47xxAMRX_tune(radio_am[10]);
								Draw_AM_icon_Ex(radio_am[10],wDispID);
							}
							else
							{
								displayFreqflagAM = 1;
								AMCurruntFreq = 900;
								si47xxAMRX_tune(900);
								Draw_AM_icon_Ex(900,wDispID);
							}
							return_flag = 1; 
							//Reserved_radio(radio_am,wDispID,0);// lyj 20190606
							break;
					}				
					

				}
				else
				{
						//pMenu->uiStatus = MENU_STATUS_NONE;
						//pMenu->iSelected = pMenu->pfMenuGetDefaultVal(pMenu);
						//pMenu->iCurrentPos = pMenu->iSelected;
           				//pMenu->bSetOne     = 0;
						
						//MenuDrawSubPage(pMenu);
						//am_si47xx_exit();

						

						AM_select = 0;
						 return_flag = 0;

						  if(pMenu->iCurrentPos == 3)
						 {
						 
							if(radio_am[0] != 0 && switch_search !=0)
							{
								si47xxAMRX_tune(radio_am[0]);
								Draw_AM_icon_Ex(radio_am[0],wDispID);
							}

								switch_search =0;
								//Radiocnt = 0;
						 }
						  else if(pMenu->iCurrentPos == 2)
						  {
						  	displayFreqflagAM = 0;
								//	Reserved_radio(radio_am,wDispID,1); // lyj 20190606
						  }
						  else if(pMenu->iCurrentPos == 1)
						  {
						  	displayFreqflagAM = 0;
						  	//Reserved_radio(radio_am,wDispID,1);// lyj 20190606

						  }//lyj 20190214
						  else if(pMenu->iCurrentPos == 0)
						  {

						  }
				}
				

			}

			#if 1
			else if (Menu_Get_Page() == 4)
			{
				//if(return_flag == 0)
				//{

					//MMP_ULONG light_bar;void light_Bar_fun(MMP_GPIO_PIN pin,AHC_BOOL vel)
					switch(pMenu->iCurrentPos)
					{
						case 0:
							//return_flag = 1;
							
							if(lightBarFlag_1)
							{
								
								light_Bar_fun(MMP_GPIO43,1);
								
								//DrwawSubItem_lightBarEX(0,lightBarFlag_1);
								lightBarFlag_1 = 0;
								MenuSettingConfig()->uiBiglangStauts &=0xfe;
								//DrwawSubItem_lightBarEX(wDispID,0,lightBarFlag_1)
							}
							else
							{
							
								light_Bar_fun(MMP_GPIO43,0);
									
									//DrwawSubItem_lightBarEX(0,lightBarFlag_1);
									lightBarFlag_1 = 1;
									MenuSettingConfig()->uiBiglangStauts |=0x01;
							}
							DrwawSubItem_lightBarEX(0,lightBarFlag_1);
							printc("~~~~~~MMP_GPIO43~~~~~~~~~~\r\n");
							break;
						case 1:
							//return_flag = 1;
							if(lightBarFlag_2)
							{
								
								light_Bar_fun(MMP_GPIO44,1);
								
								//DrwawSubItem_lightBarEX(1,lightBarFlag_2);
								lightBarFlag_2 = 0;
								//MenuSettingConfig()->uiBiglangStauts =0;
								MenuSettingConfig()->uiBiglangStauts &=0xfd;
							}
							else
							{
								
								light_Bar_fun(MMP_GPIO44,0);
								
								//DrwawSubItem_lightBarEX(1,lightBarFlag_2);
								lightBarFlag_2 = 1;
								MenuSettingConfig()->uiBiglangStauts |=0x02;
							}
							DrwawSubItem_lightBarEX(1,lightBarFlag_2);
							//light_Bar_fun(MMP_GPIO44,1);
							printc("~~~~~~MMP_GPIO44~~~~~~~~~~\r\n");
							break;

						case 2:
							//return_flag = 1;
							if(lightBarFlag_3)
							{
								
								light_Bar_fun(MMP_GPIO45,1);
								
								//DrwawSubItem_lightBarEX(2,lightBarFlag_3);
								lightBarFlag_3 = 0;
								MenuSettingConfig()->uiBiglangStauts &=0xfb;
							}
							else
							{
								
								light_Bar_fun(MMP_GPIO45,0);
								
								//DrwawSubItem_lightBarEX(2,lightBarFlag_3);
								lightBarFlag_3 = 1;
								MenuSettingConfig()->uiBiglangStauts |=0x04;
							}
							
							//light_Bar_fun(MMP_GPIO45,1);
							DrwawSubItem_lightBarEX(2,lightBarFlag_3);
							printc("~~~~~~MMP_GPIO45~~~~~~~~~~\r\n");
							break;
						case 3:
							//return_flag = 1;
							if(lightBarFlag_4)
							{
								
								light_Bar_fun(MMP_GPIO46,1);
								
								//DrwawSubItem_lightBarEX(3,lightBarFlag_4);
								lightBarFlag_4 = 0;
								MenuSettingConfig()->uiBiglangStauts &=0xf7;
							}
							else
							{
								
								light_Bar_fun(MMP_GPIO46,0);
								
								//DrwawSubItem_lightBarEX(3,lightBarFlag_4);
								lightBarFlag_4 = 1;
								MenuSettingConfig()->uiBiglangStauts |=0x08;
							}
							DrwawSubItem_lightBarEX(3,lightBarFlag_4);
							//light_Bar_fun(MMP_GPIO46,1);
							printc("~~~~~~MMP_GPIO46~~~~~~~~~~\r\n");
							break;
						case 4:
							//return_flag = 1;
							if(lightBarFlag_5)
							{
								light_Bar_fun(MMP_GPIO47,1);
								//DrwawSubItem_lightBarEX(4,lightBarFlag_5);
								
								
								lightBarFlag_5 = 0;
								MenuSettingConfig()->uiBiglangStauts &=0xef;
								
							}
							else
							{
								
								light_Bar_fun(MMP_GPIO47,0);
								//DrwawSubItem_lightBarEX(4,lightBarFlag_5);
								lightBarFlag_5 = 1;
								MenuSettingConfig()->uiBiglangStauts |=0x10;
							}
							DrwawSubItem_lightBarEX(4,lightBarFlag_5);
							//light_Bar_fun(MMP_GPIO47,1);
							printc("~~~~~~MMP_GPIO47~~~~~~~~~~\r\n");
							break;
						case 5:
							//return_flag = 1;
							if(lightBarFlag_6)
							{
								
								light_Bar_fun(MMP_GPIO48,1);
								//DrwawSubItem_lightBarEX(5,lightBarFlag_6);
								lightBarFlag_6 = 0;
								MenuSettingConfig()->uiBiglangStauts &=0xdf;
							}
							else
							{
								
								light_Bar_fun(MMP_GPIO48,0);
								//DrwawSubItem_lightBarEX(5,lightBarFlag_6);
								lightBarFlag_6 = 1;
								MenuSettingConfig()->uiBiglangStauts |=0x20;
							}
							DrwawSubItem_lightBarEX(5,lightBarFlag_6);
							//light_Bar_fun(MMP_GPIO48,1);
							printc("~~~~~~MMP_GPIO48~~~~~~~~~~\r\n");
							break;
						

					}
					
							printc("~~~~~~uiBiglangStauts==%d~~~~~~~~~~\r\n",MenuSettingConfig()->uiBiglangStauts);
				//}
				#if 0
				else
				{
						#if 0
						pMenu->uiStatus = MENU_STATUS_NONE;
						pMenu->iSelected = pMenu->pfMenuGetDefaultVal(pMenu);
						pMenu->iCurrentPos = pMenu->iSelected;
           				pMenu->bSetOne     = 0;
						
						MenuDrawSubPage(pMenu);

						#endif

						switch(pMenu->iCurrentPos)
					{
						case 0:
							return_flag = 0;
							light_Bar_fun(MMP_GPIO43,0);
							printc("~~~~~~MMP_GPIO43~~~~~~~~~~\r\n");
							break;
						case 1:
							return_flag = 0;
							light_Bar_fun(MMP_GPIO44,0);
							printc("~MMP_GPIO44\r\n");
							break;

						case 2:
							return_flag = 0;
							light_Bar_fun(MMP_GPIO45,0);
							printc("~~~~~~MMP_GPIO45~~~~~~~~~~\r\n");
							break;
						case 3:
							return_flag = 0;
							light_Bar_fun(MMP_GPIO46,0);
							printc("~~~~~~MMP_GPIO46~~~~~~~~~~\r\n");
							break;
						case 4:
							return_flag = 0;
							light_Bar_fun(MMP_GPIO47,0);
							printc("~~~~~~MMP_GPIO47~~~~~~~~~~\r\n");
							break;
						case 5:
							return_flag = 0;
							light_Bar_fun(MMP_GPIO48,0);
							printc("~~~~~~MMP_GPIO48~~~~~~~~~~\r\n");
							break;
						

					}
				

				}
				#endif

			}
			#ifdef  BlueTOOthMACO
			else if(Menu_Get_Page() == 0)
			{
				BlueMusicPlay_Pause();
			}
			else if(Menu_Get_Page() == 6)// lyj 20181026
			{
				USBMusicPlay_Pause();// lyj 20181115
			}
			#endif
			else if(Menu_Get_Page() == 7)
			{

				if(pMenu->iCurrentPos == 2)
				{
					if(!SelfAllFlag.Speedmode) 
						SelfAllFlag.Speedmode =1;
					else if(SelfAllFlag.Speedmode == 1)
						SelfAllFlag.Speedmode = 2;
					else 
						SelfAllFlag.Speedmode = 0;

					MenuSettingConfig()->uiSpeedRGB= SelfAllFlag.Speedmode;// lyj 20191021
				}
				else if(pMenu->iCurrentPos == 3)
				{
					if(SelfAllFlag.onceFlag < 3)
						SelfAllFlag.onceFlag = 3;
					else if(SelfAllFlag.onceFlag == 3)
						SelfAllFlag.onceFlag = 4;
					else if(SelfAllFlag.onceFlag == 4)
						SelfAllFlag.onceFlag = 5;
					else if(SelfAllFlag.onceFlag == 5)
						SelfAllFlag.onceFlag = 6;
					else if(SelfAllFlag.onceFlag == 6)
						SelfAllFlag.onceFlag = 7;
					else if(SelfAllFlag.onceFlag == 7)
						SelfAllFlag.onceFlag = 8;
					else if(SelfAllFlag.onceFlag == 8)
						SelfAllFlag.onceFlag = 9;
					else if(SelfAllFlag.onceFlag == 9)
						SelfAllFlag.onceFlag = 0;

					MenuSettingConfig()->uiRGBmode = SelfAllFlag.onceFlag;
				}
					

			} // lyj 20191018

			#endif
			

	#endif
				break;
        case MENU_SET_ONE		:
 
            pMenu->bSetOne   = 1;
            iPrevPos      	 = pMenu->iCurrentPos;

            if(ulEvent==MENU_SET_ONE){         
                pMenu->iCurrentPos = ulParam;
                pMenu->iSelected   = ulParam;
                pMenu->uiStatus |= MENU_STATUS_PRESSED;
                pMenu->uiStatus |= MENU_STATUS_ITEM_TOUCHED;     
            }
            else{
                pMenu->iSelected = pMenu->iCurrentPos;
                pMenu->uiStatus |= MENU_STATUS_PRESSED;
            }
			printc("~鍉~~~www~~~~~~~%d~~~~~~~\r\n",ulEvent);

            if( CommonMenuOK( pMenu, AHC_TRUE ) == AHC_TRUE )
            {

		printc("~鍉~~qq~~~~~~~~%d~~~~~~~\r\n",ulEvent);
                if(Deleting_InBrowser || Protecting_InBrowser){
                    printc("Delete/Protect/UnProtect Change to Browser Mode2\r\n");
                    // To check is there file in card, it no any file
                    // to reset Delete/(Un)Protect file in broswer flags.
                    // Otherwise those flag will make key/device events to
                    // be NULL!! UI will be stuck
                    BrowserFunc_ThumbnailEditCheckFileObj();
					if(AHC_CheckCurSysMode(AHC_MODE_PLAYBACK))
					{
						Deleting_InBrowser = 0;
						Protecting_InBrowser = 0;
					}
                    return MENU_ERR_FORCE_BROWSER_EXIT;
                }
                else
                {
	
                    PSMENUSTRUCT pParentMenu;

                    pParentMenu        = pMenu->pParentMenu;
                    pMenu->pParentMenu = NULL;

			printc("~鍉~~~~~~~~~~%d~~~~~~~\r\n",ulEvent);
			
                    if( pParentMenu == NULL ){
						printc("~鍉~~hhh~~~~~~~~%d~~~~~~~\r\n",ulEvent);
                        return MENU_ERR_EXIT;
                    }
                    SetCurrentMenu(pParentMenu);
                    pParentMenu->pfEventHandler(pParentMenu, MENU_INIT, 0);	
                }
		// -----------------------------------
		 printc("--------------------666666-----------\r\n");
	//	ResetCurrentMenu();
       		 Menu_WriteSetting();
 //------------------------------------
                return MENU_ERR_OK;
            }
            break;

        case MENU_MENU          :
            { 
			//extern PowerUpFlag PowerOnFlag;// lyj 20190605
			//extern AHC_BOOL AUX_flag ;
			//extern AHC_BOOL FM_AM_falg ;
			//extern AHC_BOOL brgiht_RGB_flag ;
	
                    PSMENUSTRUCT pParentMenu;

					//==========================lyj=========

					pMenu->iSelected = pMenu->iCurrentPos;
              		pMenu->uiStatus |= MENU_STATUS_PRESSED;

					//=============================

                    pMenu->bSetOne     = 0;
                    pParentMenu        = pMenu->pParentMenu;
                    pMenu->pParentMenu = NULL;

                    if( pParentMenu == NULL ){
                        return MENU_ERR_EXIT;
                    }

                  //  SetCurrentMenu(pParentMenu);
                  //  pParentMenu->pfEventHandler(pParentMenu, MENU_INIT, 0);
					return_flag = 0;

			#if 1
					if(Menu_Get_Page() == 2)
					{
						FM_select = 0;
						AM_select = 0;
						switch_search =0;
						printc("20190215~~~~~FM_select == %d\n",FM_select);
						//fm_si47xx_exit(); // lyj 20190213
						//AHC_OS_SleepMs(300);//lyj
					}
					else if(Menu_Get_Page() == 1)
					{
						AM_select = 0;
						FM_select = 0;
						switch_search =0;
					//	PowerOnFlag.Fm = 100; // lyj 20190605
						MenuSettingConfig()->uiPowerUpFlag = 0;
						printc("20190215~~~~~AM_select == %d\n",AM_select);
					//	  display_radio =0;
						//am_si47xx_exit();// lyj 20190213
						//AHC_OS_SleepMs(100);

						
					}
					else if(Menu_Get_Page() == 4)
					{
						//White_light_bar_off(); // lyj
					}
					else if(Menu_Get_Page() == 0)
					{
						// Close_channal();
						// Bluetooth_LEdoff(); // lyj 20190423
					}
					#if 0
					else if(Menu_Get_Page() == 5)
					{
						Close_channal();
					}
					else if(Menu_Get_Page() == 6)
					{
						Close_channal();
					}
					#endif

			#else
				switch(Menu_Get_Page())
				{
					case 0:
							if(SelfAllFlag.SaveTheBluetooth)
								AUX_flag = 0,SelfAllFlag.SaveTheBluetooth = 0;

						break;
					case 1:

						AM_select = 0;
						FM_select = 0;
						switch_search =0;
					//	PowerOnFlag.Fm = 100; // lyj 20190605
						//MenuSettingConfig()->uiPowerUpFlag = 0;

						if(SelfAllFlag.SaveTheFM)
							FM_AM_falg = 0,SelfAllFlag.SaveTheFM = 0;
						printc("20190215~~~~~AM_select == %d\n",AM_select);

						break;
					case 2:

						FM_select = 0;
						AM_select = 0;
						switch_search =0;
						if(SelfAllFlag.SaveTheAM)
							FM_AM_falg = 1,SelfAllFlag.SaveTheAM = 0;
						
						printc("20190215~~~~~FM_select == %d\n",FM_select);

						break;

					case 3:

						break;

					case 4:

						if(SelfAllFlag.SaveTheAUX)
								brgiht_RGB_flag = 2,SelfAllFlag.SaveTheAUX= 0;

						break;

					case 5:

						if(SelfAllFlag.SaveTheAUX)
								AUX_flag = 1,SelfAllFlag.SaveTheAUX = 0;

						break;

					case 6:
						if(SelfAllFlag.SaveTheUSB)
								AUX_flag = 2,SelfAllFlag.SaveTheUSB= 0;
						
						break;

					case 7:
						if(SelfAllFlag.SaveTheRGB)
								brgiht_RGB_flag = 1,SelfAllFlag.SaveTheRGB= 0;

						break;

					case 8:
							if(SelfAllFlag.SaveTheBrightness)
								brgiht_RGB_flag = 0,SelfAllFlag.SaveTheBrightness= 0;
						

						break;




					default:

						break;


				}


			#endif
					MenuSettingConfig()->uiPowerUpFlag = 0; // lyj 20190606
					 Main_Set_Page(MENU_MAIN_FLAG);// lyj 20181108
					SetCurrentMenu(pParentMenu);
                  			pParentMenu->pfEventHandler(pParentMenu, MENU_INIT, 0);
                		
            }
        break;

		
	#if 1
        case MENU_POWER_OFF     :
			//ReserveTheRadio_AM_FM(); //关机保存电台
            AHC_PowerOff_NormalPath();
            break; 
	#endif

        default:
            return MENU_ERR_OK;//MENU_ERR_NOT_MENU_EVENT;        
            break;
    }

    return MENU_ERR_OK;
}



//========================lyj===========

UINT32 SubMenuEventHandler1(PSMENUSTRUCT pMenu, UINT32 ulEvent, UINT32 ulParam)
{
    INT32 	iPrevPos;




    switch(ulEvent)
    {
        case MENU_EXIT:
            return MENU_ERR_EXIT;
            break;	

        case MENU_INIT          :
            pMenu->uiStatus = MENU_STATUS_NONE;

            if( pMenu->pfMenuGetDefaultVal ){
                pMenu->iSelected = pMenu->pfMenuGetDefaultVal(pMenu);
            }
            else{
                pMenu->iSelected = 0 ;
            }
            pMenu->iCurrentPos = pMenu->iSelected;
            pMenu->bSetOne     = 0;	

			printc("~88888888~~ulEvent = %d~~ulParam = %d~\r\n",ulEvent,ulParam);

            MenuDrawSubPage1(pMenu);
            break;

        case MENU_UP            :


            iPrevPos = pMenu->iCurrentPos;

            MenuDrawChangeSubItem1(pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected);//上下移动item,并重新绘制

		printc("~~~~a~sub=%s~%d~~\r\n",__func__,__LINE__);
            break;

        case MENU_DOWN          :

		#ifdef SLIDE_MENU
            if(IsSlidingMenu())
                break;
		#endif

            iPrevPos = pMenu->iCurrentPos;

            MenuDrawChangeSubItem1(pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected);

		printc("~~b~~~sub=%s~~%d~\r\n",__func__,__LINE__);
            break;

	#if (DIR_KEY_TYPE==KEY_TYPE_4KEY)
        case MENU_LEFT          :

            iPrevPos = pMenu->iCurrentPos;
            pMenu->iCurrentPos = OffsetItemNumber(pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, -1, AHC_FALSE);

            MenuDrawChangeSubItem(pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected);
			printc("~~c~~~sub=%s~~%d~\r\n",__func__,__LINE__);
            break;

        case MENU_RIGHT          :


            iPrevPos = pMenu->iCurrentPos;
            pMenu->iCurrentPos = OffsetItemNumber(pMenu->iCurrentPos, 0, pMenu->iNumOfItems-1, 1, AHC_FALSE);

            MenuDrawChangeSubItem(pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected);
			printc("~~d~~~sub=%s~~%d~\r\n",__func__,__LINE__);
            break;
	#endif

        case MENU_OK            :

			#if 1



                    pMenu->bSetOne     = 2;
                     pMenu->iSelected = pMenu->iCurrentPos;
                   pMenu->uiStatus |= MENU_STATUS_PRESSED;

					 SSUB_CommonMenuOK1( pMenu, AHC_TRUE );

								
			break;

			#endif
        case MENU_SET_ONE		:
 
            pMenu->bSetOne   = 1;
            iPrevPos      	 = pMenu->iCurrentPos;

            if(ulEvent==MENU_SET_ONE){         
                pMenu->iCurrentPos = ulParam;
                pMenu->iSelected   = ulParam;
                pMenu->uiStatus |= MENU_STATUS_PRESSED;
                pMenu->uiStatus |= MENU_STATUS_ITEM_TOUCHED;     
            }
            else{
                pMenu->iSelected = pMenu->iCurrentPos;
                pMenu->uiStatus |= MENU_STATUS_PRESSED;
            }
			printc("~鍉~~~www~~~~~~~%d~~~~~~~\r\n",ulEvent);

            if( CommonMenuOK( pMenu, AHC_TRUE ) == AHC_TRUE )
            {

		printc("~鍉~~qq~~~~~~~~%d~~~~~~~\r\n",ulEvent);
                if(Deleting_InBrowser || Protecting_InBrowser){
                    printc("Delete/Protect/UnProtect Change to Browser Mode2\r\n");
                    // To check is there file in card, it no any file
                    // to reset Delete/(Un)Protect file in broswer flags.
                    // Otherwise those flag will make key/device events to
                    // be NULL!! UI will be stuck
                    BrowserFunc_ThumbnailEditCheckFileObj();
					if(AHC_CheckCurSysMode(AHC_MODE_PLAYBACK))
					{
						Deleting_InBrowser = 0;
						Protecting_InBrowser = 0;
					}
                    return MENU_ERR_FORCE_BROWSER_EXIT;
                }
                else
                {
	
                    PSMENUSTRUCT pParentMenu;

                    pParentMenu        = pMenu->pParentMenu;
                    pMenu->pParentMenu = NULL;

			printc("~鍉~~~~~~~~~~%d~~~~~~~\r\n",ulEvent);
			
                    if( pParentMenu == NULL ){
						printc("~鍉~~hhh~~~~~~~~%d~~~~~~~\r\n",ulEvent);
                        return MENU_ERR_EXIT;
                    }
                    SetCurrentMenu(pParentMenu);
                    pParentMenu->pfEventHandler(pParentMenu, MENU_INIT, 0);	
                }
		// -----------------------------------
		 printc("--------------------666666-----------\r\n");
	//	ResetCurrentMenu();
       		 Menu_WriteSetting();
 //------------------------------------
                return MENU_ERR_OK;
            }
            break;

        case MENU_MENU          :
            {     
                {
                    PSMENUSTRUCT pParentMenu;

                    pMenu->bSetOne     = 0;
                    pParentMenu        = pMenu->pParentMenu;
                    pMenu->pParentMenu = NULL;

                    if( pParentMenu == NULL ){
                        return MENU_ERR_EXIT;
                    }

                    SetCurrentMenu(pParentMenu);
                    pParentMenu->pfEventHandler(pParentMenu, MENU_INIT, 0);
                }
            }
        break;

		
	#if 1
        case MENU_POWER_OFF     :
            AHC_PowerOff_NormalPath();
            break; 
	#endif

        default:
            return MENU_ERR_OK;//MENU_ERR_NOT_MENU_EVENT;        
            break;
    }

    return MENU_ERR_OK;
}



#if (SUPPORT_TOUCH_PANEL)
UINT32 SubMenuItem_Touch(UINT16 pt_x,UINT16 pt_y)
{
	UINT16  i = 0;
	PSMENUSTRUCT pMenu;
	int iPrevPos;
	UINT32 ret = MENU_ERR_OK;
	
	pMenu=GetCurrentMenu();

    if( pMenu->iNumOfItems <= 2 )
    {
		i =  ( pt_x- OSD_SUBMENU20_X )/( OSD_SUBMENU20_W + OSD_MENU_SUBITEM2X_INTERVAL );
    }
    else if( pMenu->iNumOfItems <= 4 )
    {
		i =  (( pt_x- OSD_SUBMENU40_X )/( OSD_SUBMENU40_W + OSD_MENU_SUBITEM4X_INTERVAL )
			+(( pt_y - OSD_SUBMENU40_Y )/( OSD_SUBMENU40_H+ OSD_MENU_SUBITEM4Y_INTERVAL ))*2 );
    }
    else
    {
		i =  (( pt_x- OSD_SUBMENU60_X )/( OSD_SUBMENU60_W + OSD_MENU_SUBITEM6X_INTERVAL )
            +(( pt_y - OSD_SUBMENU60_Y )/( OSD_SUBMENU60_H + OSD_MENU_ITEM_INTERVAL ))*2 );

		i = (pMenu->iCurrentPos/SUB_MENU_PAGE_ITEM)*SUB_MENU_PAGE_ITEM + i;
    }
	printc("SubMenuItem_Touch %d,ofItem %d\r\n",i,pMenu->iNumOfItems);

	if(i < pMenu->iNumOfItems)
	{
		if(i ==  pMenu->iCurrentPos)
			ret = SubMenuEventHandler(pMenu,MENU_SET_ONE,i);
		else
		{
			iPrevPos = pMenu->iCurrentPos;
			pMenu->iCurrentPos = i;
			MenuDrawChangeSubItem( pMenu, pMenu->iCurrentPos, iPrevPos, pMenu->iSelected );
		}
	}
	printc(FG_RED("SubMenuItem_Touch ret %d\r\n"),ret);
	if(ret == MENU_ERR_EXIT)
	{
		#ifdef SLIDE_MENU
		if(IsSlidingMenu())
			StopSlideMenu();
		#endif    
    
        ResetCurrentMenu();

		Menu_WriteSetting();
        
        if(Deleting_InBrowser || Protecting_InBrowser)
        {
        	//bForceSwitchBrowser = AHC_TRUE;
        	StateSwitchMode(UI_BROWSER_STATE);
        }
       
    }	
	return MENU_ERR_OK;	
}

UINT32  SubMenuComfirm_Touch(UINT16 pt_x,UINT16 pt_y)
{
	UINT16  i = 0;
	UINT32 uParam;
	UINT32 ret = MENU_ERR_OK;
	PSMENUSTRUCT pMenu;
	RECT rc = RECT_TOUCH_BUTTON_MENU_YES;
	#define STR_GAP		(30)
	
	pMenu = GetCurrentMenu();
	i = (pt_x -rc.uiLeft)/(rc.uiWidth+ STR_GAP);
	if(i==0)
		uParam = 0;
	else
		uParam = 1;
	ret = pMenu->pfEventHandler(pMenu,MENU_SET_ONE,uParam);
	
	return MENU_ERR_OK;
}

#endif
#if 0
extern UINT16 wDispID; // lyj 20180605
void Draw_Volume_icon(int vol)
{	
			UINT8   bID0 = 0, bID1 = 0;
			RECT RECTExit = {0,   0,   44, 36};
			 OSDDraw_EnterMenuDrawing(&bID0, &bID1);
			
		switch(vol)
		{
		
				case 0:		
							OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
							break;
	            		
				case 1:
						
							OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
							RECTExit.uiLeft = 182;//286
							RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
							  //Volume_set_EX(0x25);
							  break;
				case 2:
						
						OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);   

						break;

				case 3:
						
						OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);  

						RECTExit.uiLeft = 205;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						break;

				case 4:
						
						OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);  

						RECTExit.uiLeft = 205;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 216;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);


					break;


				case 5:
						
						OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);  

						RECTExit.uiLeft = 205;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 216;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 227;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

					break;

				case 6:

						

						OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);  

						RECTExit.uiLeft = 205;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 216;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 227;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 238;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

					break;

				case 7:

						

						OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);  

						RECTExit.uiLeft = 205;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 216;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 227;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 238;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 249;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);


					break;

				case 8:

				

						OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);  

						RECTExit.uiLeft = 205;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 216;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 227;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 238;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 249;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 260;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

					break;

				case 9:

					

						OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);  

						RECTExit.uiLeft = 205;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 216;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 227;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 238;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 249;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 260;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 271;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

					break;

				case 10:

					

						OSD_Draw_Icon(bmmainVolume, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);  

						RECTExit.uiLeft = 205;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 216;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 227;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 238;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 249;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 260;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 271;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						RECTExit.uiLeft = 282;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

					break;


		}

		OSDDraw_ExitMenuDrawing(&bID0, &bID1);

			

}
#endif

MMP_UBYTE CoversizeThevol(MMP_UBYTE Pvol)
{
	switch(Pvol)
	{
			case 0:

				return 200;
				break;
			case 1:
				return 70;
				break;
			case 2:
				return 60;
				break;
			case 3:
				return 50;
				break;
			case 4:
				return 40;
				break;
			case 5:
				return 30;
				break;
			case 6:
				return 27;
				break;
			case 7:
				return 24;
				break;
			case 8:
				return 21;
				break;
			case 9:
				return 18;
				break;
			case 10:
				return 15;
				break;
			case 11:
				return 12;
				break;
			case 12:
				return 9;
				break;
			case 13:
				return 6;
				break;
			case 14:
				return 3;
				break;

			case 15:
				return 0;
				break;

			default :
				return 200;
				break;




	}


}

void Draw_Mute_icon(RECT tmpRECT6,AHC_BOOL iconFlag)
{
	if(iconFlag)
	{
		OSD_Draw_Icon(bmMUTE, tmpRECT6, wDispID);
		if(Mute_ON_OFF == 1)
		{
			Mute_ON_OFF = 0;
			volume_conver_size(MAINVOICE,(int)MenuSettingConfig()->uiEV);
		}
	}
	else
	{
		OSD_Draw_Icon(bmMUTEicon, tmpRECT6, wDispID);
		if(Mute_ON_OFF == 2)
		{
			Mute_ON_OFF = 0;
			//uart_to_mcu_voice(MAINVOICE,0x00,MUTE_ON);
			uart_to_mcu_voice(MAINVOICE,CoversizeThevol(MenuSettingConfig()->uiEV),MUTE_ON); // lyj 20190831
		}
	}

}
#if 0
void APP_Draw_Mute_icon(AHC_BOOL iconFlag)
{
	RECT tmpRECT6 = {17,   15,   44, 36};
	if(iconFlag)
	{
		OSD_Draw_Icon(bmMUTE, tmpRECT6, wDispID);
	
			volume_conver_size(MAINVOICE,(int)MenuSettingConfig()->uiEV);

	}
	else
	{
		OSD_Draw_Icon(bmMUTEicon, tmpRECT6, wDispID);

			uart_to_mcu_voice(MAINVOICE,0x00,MUTE_ON);
	}

}
#endif

void DrawBluetoothLinkIcon(AHC_BOOL BlueFlag)
{
	if(BlueFlag)
		AHC_OSDDrawBitmap(wDispID, &bmIcon_Num_6, 382, 14);
	else
		AHC_OSDDrawBitmap(wDispID, &bmIcon_Num_7, 382, 14);
}

void volume_conver_size(MMP_UBYTE ch ,int vol)
{
	switch(vol)
	{
		case 0:
			
			uart_to_mcu_voice(ch,200,MUTE_OFF);// add voice 0x32
			break;
		case 1:
			uart_to_mcu_voice(ch,70,MUTE_OFF);// 0x2d
			break;

		case 2:

			uart_to_mcu_voice(ch,60,MUTE_OFF);// 0x28
			break;
		case 3:
			uart_to_mcu_voice(ch,50,MUTE_OFF);// 0x23
			break;

		case 4:

			uart_to_mcu_voice(ch,40,MUTE_OFF);
			break;
		case 5:
			uart_to_mcu_voice(ch,30,MUTE_OFF);
			break;

		case 6:

			uart_to_mcu_voice(ch,27,MUTE_OFF);
			break;
		case 7:
			uart_to_mcu_voice(ch,24,MUTE_OFF);
			break;

		case 8:

			uart_to_mcu_voice(ch,21,MUTE_OFF);
			break;
		case 9:
			uart_to_mcu_voice(ch,18,MUTE_OFF);
			break;
		case 10:
			uart_to_mcu_voice(ch,15,MUTE_OFF);
			break;

		case 11:

			uart_to_mcu_voice(ch,12,MUTE_OFF);
			break;
		case 12:
			uart_to_mcu_voice(ch,9,MUTE_OFF);
			break;

		case 13:

			uart_to_mcu_voice(ch,6,MUTE_OFF);
			break;
		case 14:
			uart_to_mcu_voice(ch,3,MUTE_OFF);

		
			break;

				case 15:
			uart_to_mcu_voice(ch,0,MUTE_OFF);

		
			break;
	}
	

}


void Reserved_radio(MMP_USHORT availedFreq[],MMP_UBYTE z0,AHC_BOOL bicIcon)// lyj 20190214
{
	//UINT8  	bID0 = 0;
	//UINT8  	bID1 = 0;
	UINT8 i ;
	//UINT8 j;

	RECT tmpRECT[11]= {{73,230,60,20},{138,230,60,20},{203,230,60,20},{268,230,60,20},{333,230,60,20},{73,255,60,20},{138,255,60,20},{203,255,60,20},{268,255,60,20},{333,255,60,20}};

	 char    szv[16];

	   //CHARGE_ICON_ENABLE(AHC_FALSE);

	//OSDDraw_EnterDrawing(&bID0, &bID1);
	// OSDDraw_EnableSemiTransparent(bID0, AHC_TRUE);

	//BEGIN_LAYER(Uwdraw);
    //	BEGIN_LAYER(Uwdrawx);//wDispID
   	//BEGIN_LAYER(bID0);
	//OSDDraw_EnableSemiTransparent(Uwdraw, AHC_TRUE);
	//OSDDraw_EnableSemiTransparent(Uwdrawx, AHC_TRUE);
	//AHC_OSDSetFont(Uwdraw, &GUI_Font16_1);
	   AHC_OSDSetFont(z0, &GUI_Font16_1);
	//AHC_OSDSetColor(Uwdraw, OSD_COLOR_FM); 
	//AHC_OSDSetColor(bID0, OSD_COLOR_WHITE);
	//AHC_OSDSetBkColor(bID0, OSD_COLOR_FM);
	if(Menu_Get_Page() == 1)
	if(MenuSettingConfig()->uiMOVQuality == 0)
	{
		for(i = 0; i<10; i++)
		{
			if(availedFreq[i] != 0)
			{
			

					//sprintf(szv, "%d.%dMHz", availedFreq[i]/100,(availedFreq[i] % 100)/10);
					//printc("~~s~~~~~i = %d~~~~~~~~\r\n",i);
					//for(j = 0; j<3;j++)
					AHC_OSDSetColor(z0, OSD_COLOR_FM);// z0 wDispID
					AHC_OSDDrawFillRect(z0, tmpRECT[i].uiLeft, tmpRECT[i].uiTop, tmpRECT[i].uiLeft + tmpRECT[i].uiWidth, tmpRECT[i].uiTop+tmpRECT[i].uiHeight);// lyj 20181124

					if(bicIcon == 0)
					{
					sprintf(szv, "%d.%dMHz", availedFreq[i]/100,(availedFreq[i] % 100)/10);
					OSD_ShowString( z0,szv, NULL, tmpRECT[i], OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
					}

				#if 0
				AHC_OSDDrawDec(bID0, (INT32)availedFreq[0],     100,    200, uwLength);
				AHC_OSDDrawText(bID0, (UINT8*)".",  100+20, 200, strlen("."));
				AHC_OSDDrawDec(bID0, Last_wei,     125,    200, 1);
				AHC_OSDDrawText(bID0, (UINT8*)"MHz",  135, 200, strlen("MHz"));
				#endif
			}
			else
			{

					AHC_OSDSetColor(z0, OSD_COLOR_FM);// z0 wDispID
					AHC_OSDDrawFillRect(z0, tmpRECT[i].uiLeft, tmpRECT[i].uiTop, tmpRECT[i].uiLeft + tmpRECT[i].uiWidth, tmpRECT[i].uiTop+tmpRECT[i].uiHeight);// lyj 20181124

			}
		}
	}
	else if(MenuSettingConfig()->uiMOVQuality == 1)
	{
		for(i = 10; i < 20; i++)
		{
			if(availedFreq[i] != 0)
			{
			

					//sprintf(szv, "%d.%dMHz", availedFreq[i]/100,(availedFreq[i] % 100)/10);
					AHC_OSDSetColor(z0, OSD_COLOR_FM);
					AHC_OSDDrawFillRect(z0, tmpRECT[i-10].uiLeft, tmpRECT[i-10].uiTop, tmpRECT[i-10].uiLeft + tmpRECT[i-10].uiWidth, tmpRECT[i-10].uiTop+tmpRECT[i-10].uiHeight);// lyj 20181124

					if(bicIcon == 0)
					{
					sprintf(szv, "%d.%dMHz", availedFreq[i]/100,(availedFreq[i] % 100)/10);
					OSD_ShowString( z0,szv, NULL, tmpRECT[i-10], OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
					}

			
			}
			else
			{

					AHC_OSDSetColor(z0, OSD_COLOR_FM);// z0 wDispID
					AHC_OSDDrawFillRect(z0, tmpRECT[i-10].uiLeft, tmpRECT[i-10].uiTop, tmpRECT[i-10].uiLeft + tmpRECT[i-10].uiWidth, tmpRECT[i-10].uiTop+tmpRECT[i-10].uiHeight);// lyj 20181124

			}
		}
	}
	else if(MenuSettingConfig()->uiMOVQuality == 2)
	{
		for(i = 20; i < 30; i++)
		{
			if(availedFreq[i] != 0)
			{
			

					//sprintf(szv, "%d.%dMHz", availedFreq[i]/100,(availedFreq[i] % 100)/10);
					AHC_OSDSetColor(z0, OSD_COLOR_FM);
					AHC_OSDDrawFillRect(z0, tmpRECT[i-20].uiLeft, tmpRECT[i-20].uiTop, tmpRECT[i-20].uiLeft + tmpRECT[i-20].uiWidth, tmpRECT[i-20].uiTop+tmpRECT[i-20].uiHeight);// lyj 20181124

					if(bicIcon == 0)
					{
					sprintf(szv, "%d.%dMHz", availedFreq[i]/100,(availedFreq[i] % 100)/10);
					OSD_ShowString( z0,szv, NULL, tmpRECT[i-20], OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
					}

			
			}
			else
			{

					AHC_OSDSetColor(z0, OSD_COLOR_FM);// z0 wDispID
					AHC_OSDDrawFillRect(z0, tmpRECT[i-20].uiLeft, tmpRECT[i-20].uiTop, tmpRECT[i-20].uiLeft + tmpRECT[i-20].uiWidth, tmpRECT[i-20].uiTop+tmpRECT[i-20].uiHeight);// lyj 20181124

			}
		}



		};

	if(Menu_Get_Page() == 2)
	 if(MenuSettingConfig()->uiBeep == 1)
	{
		for(i = 0; i < 10; i++)
		{
			if(availedFreq[i] != 0)
			{

					//sprintf(szv, "%dKHz", availedFreq[i]);
					AHC_OSDSetColor(z0, OSD_COLOR_AM);
					AHC_OSDDrawFillRect(z0, tmpRECT[i].uiLeft, tmpRECT[i].uiTop, tmpRECT[i].uiLeft + tmpRECT[i].uiWidth, tmpRECT[i].uiTop+tmpRECT[i].uiHeight);// lyj 20181124


					if(bicIcon == 0)
					{
						sprintf(szv, "%dKHz", availedFreq[i]);
					OSD_ShowString( z0,szv, NULL, tmpRECT[i], OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
					}

			
			}
			else
			{	
				AHC_OSDSetColor(z0, OSD_COLOR_AM);
					AHC_OSDDrawFillRect(z0, tmpRECT[i].uiLeft, tmpRECT[i].uiTop, tmpRECT[i].uiLeft + tmpRECT[i].uiWidth, tmpRECT[i].uiTop+tmpRECT[i].uiHeight);// lyj 20181124

			}
		}



	}
	else if(MenuSettingConfig()->uiBeep == 2)
	{
		for(i = 10; i < 20; i++)
		{
			if(availedFreq[i] != 0)
			{

					//sprintf(szv, "%dKHz", availedFreq[i]);
					AHC_OSDSetColor(z0, OSD_COLOR_AM);
					AHC_OSDDrawFillRect(z0, tmpRECT[i-10].uiLeft, tmpRECT[i-10].uiTop, tmpRECT[i-10].uiLeft + tmpRECT[i-10].uiWidth, tmpRECT[i-10].uiTop+tmpRECT[i-10].uiHeight);// lyj 20181124

					if(bicIcon == 0)
					{
					sprintf(szv, "%dKHz", availedFreq[i]);
					OSD_ShowString( z0,szv, NULL, tmpRECT[i-10], OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
					}

			
			}
			else
			{	
				AHC_OSDSetColor(z0, OSD_COLOR_AM);
					AHC_OSDDrawFillRect(z0, tmpRECT[i-10].uiLeft, tmpRECT[i-10].uiTop, tmpRECT[i-10].uiLeft + tmpRECT[i-10].uiWidth, tmpRECT[i-10].uiTop+tmpRECT[i-10].uiHeight);// lyj 20181124

			}
			
		}


	};

		

	
	//END_LAYER(Uwdraw);
	//END_LAYER(Uwdrawx);

	//OSDDraw_ExitDrawing(&bID0, &bID1);
	 // CHARGE_ICON_ENABLE(AHC_TRUE);

}

void AutoDisplayTheFreq(MMP_USHORT SavailedFreq,MMP_UBYTE z0,int i)
{
	RECT tmpRECT1[11]= {{73,230,60,20},{138,230,60,20},{203,230,60,20},{268,230,60,20},{333,230,60,20},{73,255,60,20},{138,255,60,20},{203,255,60,20},{268,255,60,20},{333,255,60,20}};
	 char    szv1[12];
	 AHC_OSDSetFont(z0, &GUI_Font16_1); // lyj 20190622
	 
	if(Menu_Get_Page() == 1)
	{

		AHC_OSDSetColor(z0, OSD_COLOR_FM);
		AHC_OSDDrawFillRect(z0, tmpRECT1[i].uiLeft, tmpRECT1[i].uiTop, tmpRECT1[i].uiLeft + tmpRECT1[i].uiWidth, tmpRECT1[i].uiTop+tmpRECT1[i].uiHeight);// lyj 20181124


		sprintf(szv1, "%d.%dMHz", SavailedFreq/100,(SavailedFreq % 100)/10);
		OSD_ShowString( z0,szv1, NULL, tmpRECT1[i], OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
	}
	else if(Menu_Get_Page() == 2)
	{
					AHC_OSDSetColor(z0, OSD_COLOR_AM);
					AHC_OSDDrawFillRect(z0, tmpRECT1[i].uiLeft, tmpRECT1[i].uiTop, tmpRECT1[i].uiLeft + tmpRECT1[i].uiWidth, tmpRECT1[i].uiTop+tmpRECT1[i].uiHeight);// lyj 20181124

					sprintf(szv1, "%dKHz", SavailedFreq);
					OSD_ShowString( z0,szv1, NULL, tmpRECT1[i], OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);


	}

}

