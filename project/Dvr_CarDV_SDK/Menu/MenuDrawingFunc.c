/*===========================================================================
 * Include file
 *===========================================================================*/

#include "customer_config.h"
#include "AHC_Common.h"
#include "AHC_Utility.h"
#include "AHC_MACRO.h"
#include "AHC_Parameter.h"
#include "AHC_General.h"
#include "AHC_OS.h"
#include "AHC_Menu.h"
#include "AHC_Display.h"
#include "AHC_Version.h"
#include "AHC_General_CarDV.h"
#include "MenuCommon.h"
#include "IconDefine.h"
#include "IconPosition.h"
#include "ColorDefine.h"
#include "OsdStrings.h"
#include "MenuSetting.h"
#include "MenuDrawingFunc.h"
#include "MenuStateMenu.h"
#include "MenuDrawCommon.h"
#include "DrawStateMenuSetting.h"
#include "DrawStateTVFunc.h"
#include "DrawStateHDMIFunc.h"
#include "ShowOSDFunc.h"
#include "StateHDMIFunc.h"
#include "StateTVFunc.h"
#include "DrawStateHDMIFunc.h"
#include "DrawStateTVFunc.h"
#include "UI_DrawGeneral.h"
#include "Stdio.h"
#include "lib_retina.h"
#include "MenuTouchButton.h"

#include "fm_i2c.h" //long 2-27

/*===========================================================================
 * Macro define
 *===========================================================================*/

#define ICON_UP_WIDTH               (44)
#define TV_HDMI_DISP_BUF_WIDTH       320
#define TV_HDMI_DISP_BUF_HEIGHT      240

static UINT16   DispBufWidth = 0;
static UINT16   DispBufHeight = 0;
static UINT16   usShiftStringRight = 20;

extern AHC_BOOL 			select_flag ;

extern AHC_BOOL flag_twinkle;
extern AHC_BOOL app_flag;
extern MMP_USHORT radio[31];
extern MMP_USHORT radio_am[21];

AHC_BOOL flag_color = 0;

extern AHC_BOOL volume_M;
//extern AHC_BOOL volume_B;
extern AHC_BOOL sub_flag_ex;
extern AHC_BOOL 			return_flag;

extern  AHC_BOOL			lightBarFlag_1;
extern AHC_BOOL			lightBarFlag_2;
extern AHC_BOOL			lightBarFlag_3;
extern AHC_BOOL			lightBarFlag_4;
extern AHC_BOOL			lightBarFlag_5;
extern AHC_BOOL			lightBarFlag_6;

//UINT8 	Uwdraw = 0;
//UINT8 	Uwdrawx = 0;

/*===========================================================================
 * Extern function
 *===========================================================================*/

extern AHC_BOOL AHC_Charger_GetStatus(void);
extern void Color_chang(MMP_ULONG ulFreq,MMP_ULONG rled,MMP_ULONG gled,MMP_ULONG bled);

extern void White_light_bar_off(void);
extern void twinkle_led(MMP_ULONG ulFreq,UINT32 uiMiliSecond);

extern  void Volume_set(MMP_USHORT volume);

extern void Volume_set_EX(MMP_USHORT volume);

void  fm_si47xx_init(void);
//void Draw_FM_AM_icon(MMP_USHORT Dfreq);
//void Draw_AM_icon(MMP_USHORT Dfreq);
void Draw_FM_AM_icon_Ex(MMP_USHORT Dfreq,UINT8   bID0);
void Draw_AM_icon_Ex(MMP_USHORT Dfreq,UINT8   bID0);
void  am_si47xx_init(void);
extern void Bluetooth_LEd(void);
extern void SetPrevNextPin(MMP_UBYTE level);
//extern  void Bluetooth_LEdoff(void);

/*===========================================================================
 * Global variable
 *===========================================================================*/

static GUI_POINT    m_PointU[]          = { {  0, 0}, {-7,  10}, {7,  10} };
static GUI_POINT    m_PointD[]          = { {  0, 0}, {-7, -10}, {7, -10} };
static GUI_POINT    m_PointL[]          = { {  0, 0}, {10,  -7}, {10,  7} };
static GUI_POINT    m_PointR[]          = { {  0, 0}, {-10, -7}, {-10, 7} };
GUI_POINT*          m_PointDir[]        = {&m_PointU[0], &m_PointD[0], &m_PointL[0], &m_PointR[0]};


MENU_ATTR           m_MainMenuAttr;
MENU_ATTR           m_SubPageMenuAttr;
SIDEMENU_ATTR       m_SubIconTextMenuAttr;

UINT8               m_ubSubPageType     = SUBMENU_SUBPAGE ;
UINT32              m_ulSubItemNum      = SUB_MENU_PAGE_ITEM;
GUI_COLOR           m_SubTextClr        = MENU_TEXT_COLOR;
GUI_COLOR           m_SubLineClr        = MENU_LINE_COLOR;
GUI_COLOR           m_SubFocusClr       = MENU_FOCUS_COLOR;
UINT32              m_ulR2SepX          = 47;
UINT32              m_ulPenSize         = 2;


#if defined(FONT_LARGE)
PSMENUSTRUCT LongStringMenu[] = {
                NULL
            };

BAR_SET vBarSet[] = {
            {   &BMICON_SUBBAR_WHITE,
                &BMICON_SUBBAR_WHITE_DEFAULT,
                &BMICON_SUBBAR_YELLOW,
                &BMICON_SUBBAR_YELLOW_DEFAULT },
            {   &BMICON_SUBBAR_WHITE,
                &BMICON_SUBBAR_WHITE_DEFAULT,
                &BMICON_SUBBAR_YELLOW,
                &BMICON_SUBBAR_YELLOW_DEFAULT },
            {   &BMICON_SMALLBAR_WHITE,
                NULL,
                &BMICON_SMALLBAR_YELLOW,
                NULL }
};

#else   // FONT_LARGE

extern SMENUSTRUCT sSubProtect;
extern SMENUSTRUCT sSubDelete;
extern SMENUSTRUCT sSubProtectAll;
extern SMENUSTRUCT sSubUnProtectAll;
extern SMENUSTRUCT sSubDateTimeFormat;
extern SMENUSTRUCT sSubMovieMode;

extern AHC_BOOL icon_flag; //lyj

PSMENUSTRUCT LongStringMenu[] = {
                &sSubProtect,
                &sSubDelete,
                &sSubProtectAll,
                &sSubUnProtectAll,
                &sSubDateTimeFormat,
                &sSubMovieMode,
                NULL
            };

BAR_SET vBarSet[] = {
            {   &BMICON_SUBBAR_WHITE,
                &BMICON_SUBBAR_WHITE_DEFAULT,
                &BMICON_SUBBAR_YELLOW,
                &BMICON_SUBBAR_YELLOW_DEFAULT },
            {   &BMICON_MENUBAR_WHITE,
                &BMICON_MENUBAR_WHITE_DEFAULT,
                &BMICON_MENUBAR_YELLOW,
                &BMICON_MENUBAR_YELLOW_DEFAULT },
            {   &BMICON_SMALLBAR_WHITE,
                NULL,
                &BMICON_SMALLBAR_YELLOW,
                NULL }
};

#endif  // FONT_LARGE

/*===========================================================================
 * Extern varible
 *===========================================================================*/

extern UINT16           m_ulVMDShotNum[];
extern UINT16           m_ulTimeLapseTime[];
extern UINT16           m_ulVolume[];
extern AHC_OS_SEMID     m_WMSGSemID;
extern AHC_BOOL         m_ubAtMenuTab;
extern UINT8            ubModeDrawStyle;
extern AHC_OSD_INSTANCE *m_OSD[];

/*===========================================================================
 * Main body
 *===========================================================================*/

//extern MMP_UBYTE flagSelect;

#if 0
void ________Common_Function_______(){ruturn;} //dummy
#endif

//--------------------------------------
AHC_BOOL Menu_Page=21;
AHC_BOOL Menu_Page_Flag=MENU_MAIN_FLAG;

AHC_BOOL sub_menu_page = 15;

UINT16 wDispID;

void White_light_bar(MMP_ULONG lightBar); // lyj 
void Menu_Set_Page(AHC_BOOL Flag)
{
	Menu_Page=Flag;
}
AHC_BOOL Menu_Get_Page(void)
{
	return Menu_Page;
}


void sub_Menu_Set_Page(AHC_BOOL Flag)
{
	sub_menu_page=Flag;
}
AHC_BOOL sub_Menu_Get_Page(void)
{
	return sub_menu_page;
}





void Main_Set_Page(AHC_BOOL Flag)
{
	Menu_Page_Flag=Flag;
}
AHC_BOOL Main_Get_Page(void)
{
	return Menu_Page_Flag;
}
//--------------------------------------


AHC_BOOL MenuDraw_GetMainMenuPageItem(UINT16 uwDispID, GUI_BITMAP barID, UINT32 *PageItemNum, UINT32 *ItemInterval)
{
    *PageItemNum = (AHC_GET_ATTR_OSD_H(uwDispID) + OSD_MENU_ITEM_INTERVAL -40-40)/(OSD_MENU_ITEM_INTERVAL+barID.YSize);
    *ItemInterval = (AHC_GET_ATTR_OSD_H(uwDispID) - 40 - 40)/(*PageItemNum);

    return 0;
}

void MenuDrawSetMenuAttribute( UINT32    MenuType,  UINT32    PageItemNum,
                               GUI_COLOR R1BkClr,   GUI_COLOR R2BkClr,
                               GUI_COLOR R3BkClr,   GUI_COLOR TextClr,      GUI_COLOR LineClr,
                               GUI_COLOR FocusClr,  UINT32    PenSize,      UINT32    R2SepX)
{
}

void MenuDrawSetIconTextSubAttribute(   UINT32    MenuType,    UINT32    PageType, UINT32    PageItemNum,
                                        GUI_COLOR L1BkClr,     GUI_COLOR L2BkClr,  GUI_COLOR TextClr,
                                        GUI_COLOR LineClr,     GUI_COLOR FocusClr, UINT32    PenSize)
{
}

void MenuDrawDirection(UINT16 uwDispID, UINT8 dir, UINT16 x, UINT16 y, GUI_COLOR FillClr)
{
}

void MenuDrawTab(UINT16 uwDispID, UINT16 x, UINT16 y, AHC_BOOL bFill, GUI_COLOR FillClr, GUI_COLOR LineClr)
{
}

void MenuDrawGridLine(UINT16 uwDispID, UINT16 x0, UINT16 x1, UINT16 y0, UINT16 iOffset,
                      UINT8 ubLineNum, UINT32 ulLineSize, GUI_COLOR LineClr)
{
}

void MenuDrawClearUnusedGrid(UINT16 uwDispID, PSMENUSTRUCT pMenu)
{
}

void MenuDrawClearRegion(UINT16 uwDispID, MENU_ATTR attr, UINT32 ulRegionNum)
{
}

void MenuDrawResetRegionSize(void)
{
}

void MenuDrawItemPosBar(UINT16 uwDispID, UINT32 item, MENU_ATTR attr, AHC_BOOL bClear)
{
}

void MenuDrawNaviOK(UINT16 uwDispID)
{
    RECT tempRECT = POS_MENU_EXIT;

    OSD_Draw_Icon(bmIcon_OK, tempRECT, uwDispID);
}

void MenuDrawPageInfo(UINT16 uwDispID, int uiCurPage, int uiTotalPage)
{
    #if defined(FONT_LARGE)
    AHC_OSDSetFont(uwDispID, &GUI_Font16B_1);
    AHC_OSDSetPenSize(uwDispID, 6);
    AHC_OSDSetColor(uwDispID, MENU_TEXT_COLOR);
    AHC_OSDSetBkColor(uwDispID, OSD_COLOR_DARKGRAY);

    AHC_OSDDrawLine(uwDispID,
                    0, 0,
                    OSD_MENU_UP_Y, OSD_MENU_DOWN_Y + OSD_MENU_DOWN_W);
    AHC_OSDSetColor(uwDispID, OSD_COLOR_RED);
    AHC_OSDDrawLine(uwDispID,
                    0, 0,
                    OSD_MENU_UP_Y + ((OSD_MENU_DOWN_Y + OSD_MENU_DOWN_W) - OSD_MENU_UP_Y) * (uiCurPage - 1) / uiTotalPage,
                    OSD_MENU_UP_Y + ((OSD_MENU_DOWN_Y + OSD_MENU_DOWN_W) - OSD_MENU_UP_Y) * uiCurPage / uiTotalPage);
    #else   // FONT_LARGE
    UINT32 x, y;

    x = POS_MENU_PAGEINFO_X;
    y = POS_MENU_PAGEINFO_Y;

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {
        x += usShiftStringRight/2;
    }

    AHC_OSDSetFont(uwDispID, &GUI_Font16B_1);
    AHC_OSDSetColor(uwDispID, MENU_TEXT_COLOR);
    AHC_OSDSetBkColor(uwDispID, OSD_COLOR_BACKGROUND);

    AHC_OSDDrawDec(uwDispID, uiCurPage,     x,    y, 1);
    AHC_OSDDrawText(uwDispID, (UINT8*)"/",  x+10, y, strlen("/"));
    AHC_OSDDrawDec(uwDispID, uiTotalPage,   x+18, y, 1 );
    #endif  // FONT_LARGE
}

void MenuDraw_UIMode(UINT16 uwDispID)
{
//    if(AHC_IsHdmiConnect())
//    {
//        UIDrawEx(uwDispID, VIDEO_GUI_UI_MODE, AHC_FALSE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
//    }
//    else if(AHC_IsTVConnectEx())
//    {
//        UIDrawEx(uwDispID, VIDEO_GUI_UI_MODE, AHC_TRUE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
//    }
//    else
    {
        UIDrawEx(uwDispID, VIDEO_GUI_UI_MODE, AHC_TRUE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
    }   
}

void MenuDraw_ClearMediaType(UINT16 uwDispID)
{
//    if(HDMIFunc_IsInMenu())
//    {
//        UIDrawEx(uwDispID, VIDEO_GUI_SD_STATE, AHC_TRUE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
//    }
//    else if(TVFunc_IsInMenu())
//    {
//        UIDrawEx(uwDispID, VIDEO_GUI_SD_STATE, AHC_TRUE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
//    }
//    else
    {
        UIDrawEx(uwDispID, VIDEO_GUI_SD_STATE, AHC_TRUE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
    }   
}

void MenuDraw_MediaType(UINT16 uwDispID)
{
//    if(AHC_IsHdmiConnect())
//    {
//        UIDrawEx(uwDispID, VIDEO_GUI_SD_STATE, AHC_FALSE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
//    }
//    else if(AHC_IsTVConnectEx())
//    {
//        UIDrawEx(uwDispID, VIDEO_GUI_SD_STATE, AHC_TRUE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
//    }
//    else
    {
        UIDrawEx(uwDispID, VIDEO_GUI_SD_STATE, AHC_TRUE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
    }   
}

void MenuDraw_SD_Status(void)
{
#ifndef CFG_DRAW_IGNORE_SD_STAUS //may be defined in config_xxx.h

    UINT8  bID0 = 0, bID1 = 0;

    CHARGE_ICON_ENABLE(AHC_FALSE);

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);
    MenuDraw_ClearMediaType(bID0);
    MenuDraw_MediaType(bID0);
    OSDDraw_ExitMenuDrawing(&bID0, &bID1);

    CHARGE_ICON_ENABLE(AHC_TRUE);
#endif
}

void MenuDraw_BatteryStatus(UINT16 uwDispID)
{
#ifndef CFG_DRAW_IGNORE_BATTERY  //may be defined in config_xxx.h
//    if(AHC_IsHdmiConnect())
//    {
//        UIDrawEx(uwDispID, VIDEO_GUI_BATTERY, AHC_TRUE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
//    }
//    else if(AHC_IsTVConnectEx())
//    {
//        UIDrawEx(uwDispID, VIDEO_GUI_BATTERY, AHC_TRUE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
//    }
//    else
    {
        UIDrawEx(uwDispID, VIDEO_GUI_BATTERY, AHC_TRUE, AHC_TRUE, OSD_COLOR_TITLE, NULL_PARAM);
    }
#endif
}

void MenuDraw_GetPageInfo(UINT32 *pCurPage, UINT32 *pTotalPage, UINT32 *pCurPos, UINT32 ulPageItemNum)
{
    PSMENUSTRUCT    pMenu = GetCurrentMenu();
    UINT32          iCurPage, iTotalPage;


    if((pMenu->iNumOfItems/ulPageItemNum != 0) && (pMenu->iNumOfItems%ulPageItemNum ==0))
    {
        iTotalPage = pMenu->iNumOfItems/ulPageItemNum;
    }
    else
    {
        iTotalPage = pMenu->iNumOfItems/ulPageItemNum + 1;
    }

    iCurPage = (pMenu->iCurrentPos/ulPageItemNum) + 1;

    if(pCurPage!=NULL)
        *pCurPage = iCurPage;

    if(pTotalPage!=NULL)
        *pTotalPage = iTotalPage;

    if(pCurPos!=NULL)
        *pCurPos = pMenu->iCurrentPos;
}

void MenuDraw_CatagoryIcon(UINT16 uwDispID, UINT32 iCata)
{
}

/*
 * Always display Select language page by English, because width of character is too wide
 * on some language (SimChinses and Tradition Chinese)
 */
UINT8 MenuDraw_GetDefaultLanguage(PSMENUSTRUCT pMenu)
{
    if (pMenu->iMenuId == MENUID_SUB_MENU_LANGUAGE)
        return SHOWOSD_LANG_ENG;

    return MenuSettingConfig()->uiLanguage;
}

#if 0
void ________PowerOff_Function_TV________(){ruturn;} //dummy
#endif

extern PSMENUITEM   sMenuListDeleteOne[];
extern AHC_BOOL     PowerOff_Option;
//extern UINT8        m_DelFile;

#if (POWER_OFF_CONFIRM_PAGE_EN)

#if (TVOUT_ENABLE)

void DrawSubItem_PowerOff_TV(UINT16 uwDispID, int iItem, int iTotalItems, UINT32 iStrID, const GUI_BITMAP* IconID, GUI_BITMAP barID, GUI_COLOR clrFont, GUI_COLOR clrBack, GUI_COLOR clrSelect)
{
    RECT    rc, tmpRECT;
    RECT    StrYesRECT      = RECT_TV_PWROFF_PLAY_STR_YES;
    RECT    StrNoRECT       = RECT_TV_PWROFF_PLAY_STR_NO;
    RECT    StrBrwYesRECT   = RECT_TV_PWROFF_BROW_STR_YES;
    RECT    StrBrwNoRECT    = RECT_TV_PWROFF_BROW_STR_NO;

#if 1 //Andy Liu TBD. 
    printc(FG_RED("\r\nPlease modify here!\r\n"));
    AHC_PRINT_RET_ERROR(0, 0);
#else      
    switch(TVFunc_Status())
    {
        case AHC_TV_VIDEO_PREVIEW_STATUS:
        case AHC_TV_DSC_PREVIEW_STATUS:
            rc.uiWidth  = barID.XSize;
            rc.uiHeight = barID.YSize;
            GetSubItemRect(uwDispID, iItem%CONFIRM_MENU_PAGE_ITEM, iTotalItems, &rc );
            rc.uiTop = rc.uiTop + 50;
        break;

        case AHC_TV_BROWSER_STATUS:
            rc = (iItem==CONFIRM_OPT_YES)?(StrBrwYesRECT):(StrBrwNoRECT);
        break;

        case AHC_TV_MOVIE_PB_STATUS:
        case AHC_TV_PHOTO_PB_STATUS:
        case AHC_TV_SLIDE_SHOW_STATUS:
            rc = (iItem==CONFIRM_OPT_YES)?(StrYesRECT):(StrNoRECT);
        break;
    }
#endif

    OSD_Draw_Icon(barID, rc, uwDispID);

    tmpRECT.uiLeft   = rc.uiLeft    + STR_RECT_OFFSET_X;
    tmpRECT.uiTop    = rc.uiTop     + STR_RECT_OFFSET_Y;
    tmpRECT.uiWidth  = rc.uiWidth   + STR_RECT_OFFSET_W;
    tmpRECT.uiHeight = rc.uiHeight  - 9;

    OSD_ShowStringPool(uwDispID, iStrID, tmpRECT, clrFont, clrBack, GUI_TA_HCENTER|GUI_TA_VCENTER);
}

void MenuDrawSubPage_PowerOffCancel_TV(void)
{
    UINT8   uwDispID;
    RECT    PlayOsdRect = RECT_TV_PWROFF_PLAY_REGION;
    RECT    BrwOsdRect  = POS_TV_PAL_BROWSER_OSD;

#if 1 //Andy Liu TBD. 
    printc(FG_RED("\r\nPlease modify here!\r\n"));
    AHC_PRINT_RET_ERROR(0, 0);
#else      
    switch(TVFunc_Status())
    {
        case AHC_TV_VIDEO_PREVIEW_STATUS:
        case AHC_TV_DSC_PREVIEW_STATUS:
            AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
            AHC_OSDDrawFillRect(MAIN_DISPLAY_BUFFER, 0, 0, TV_HDMI_DISP_BUF_WIDTH, TV_HDMI_DISP_BUF_HEIGHT);
            AHC_OSDSetActive(MAIN_DISPLAY_BUFFER, 0);
            AHC_OSDSetActive(TVFunc_GetUImodeOsdID(), 1);
        break;

        case AHC_TV_BROWSER_STATUS:
            OSDDraw_GetLastOvlDrawBuffer(&uwDispID);
            AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
            AHC_OSDDrawFillRect2(uwDispID, &BrwOsdRect);
            DrawStateBrowserInit();
        break;

        case AHC_TV_MOVIE_PB_STATUS:
            if(!m_DelFile)
            {
                OSDDraw_GetLastOvlDrawBuffer(&uwDispID);
                AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
                AHC_OSDDrawFillRect2(uwDispID, &PlayOsdRect);
                DrawMovPb_Status(uwDispID, POS_TV_MOV_STATE_X, POS_TV_MOV_STATE_Y, LARGE_SIZE, MAG_2X, AHC_TRUE, OSD_COLOR_TRANSPARENT, NULL_PARAM);
            }
            else
                DrawStateHdmiMoviePBDel(m_DelFile);
        break;

        case AHC_TV_PHOTO_PB_STATUS:
            if(!m_DelFile)
            {
                OSDDraw_GetLastOvlDrawBuffer(&uwDispID);
                AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
                AHC_OSDDrawFillRect2(uwDispID, &PlayOsdRect);
                DrawStateTVPhotoPBChangeFile();
            }
            else
                DrawStateHdmiMoviePBDel(m_DelFile);
        break;

        case AHC_TV_SLIDE_SHOW_STATUS:
            if(!m_DelFile)
            {
                OSDDraw_GetLastOvlDrawBuffer(&uwDispID);
                AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
                AHC_OSDDrawFillRect2(uwDispID, &PlayOsdRect);
                DrawStateTVSlideShowChangeFile();
            }
            else
                DrawStateHdmiMoviePBDel(m_DelFile);
        break;
    }
#endif    
}

void MenuDrawSubPage_PowerOff_TV_Preview(void)
{
    UINT32      StartX   = 145;
    UINT32      StartY   = 50;
    UINT16      uwDispID  = MAIN_DISPLAY_BUFFER;
    UINT32      i;
    GUI_RECT    Rect;
    UINT32      iVal[2];
    UINT16      Width, Height;

    AHC_OSDSetActive(uwDispID, 0);

    switch(MenuSettingConfig()->uiTVSystem)
    {
        case TV_SYSTEM_NTSC:
            AHC_GetNtscTvDisplayWidthHeight(&Width, &Height);
        break;
        case TV_SYSTEM_PAL:
            AHC_GetPalTvDisplayWidthHeight(&Width, &Height);
        break;
    }

    iVal[0] = (Width - TV_HDMI_DISP_BUF_WIDTH)/2;
    iVal[1] = (Height - TV_HDMI_DISP_BUF_HEIGHT)/2;
    AHC_OSDSetDisplayAttr(uwDispID, AHC_OSD_ATTR_DISPLAY_OFFSET, iVal);

    OSDDraw_SetAlphaBlending(uwDispID, AHC_FALSE);

    AHC_OSDSetPenSize(uwDispID, 5);
    AHC_OSDSetColor(uwDispID, TV_MENU_BK_COLOR);
    AHC_OSDDrawFillRect(uwDispID, 0, 0, TV_HDMI_DISP_BUF_WIDTH, TV_HDMI_DISP_BUF_HEIGHT);

    AHC_OSDSetColor(uwDispID, MENU_TEXT_COLOR);
    AHC_OSDSetBkColor(uwDispID, TV_MENU_BK_COLOR);

    Rect.x0 = 0;
    Rect.y0 = 110;
    Rect.x1 = 320;
    Rect.y1 = 140;

    ShowOSD_SetLanguage(uwDispID);
    AHC_OSDDrawTextInRect(uwDispID, (UINT8*)OSD_GetStringViaID(IDS_DS_MSG_SURE_TO_POWER_OFF), &Rect, GUI_TA_CENTER|GUI_TA_VCENTER);

    AHC_OSDDrawBitmap(uwDispID, &bmIcon_WMSG, StartX, StartY);

    for( i=0; i<2; i++ )
    {
        GUI_COLOR   colorBk = OSD_COLOR_TV_BACKGROUND;
        GUI_BITMAP  barID   = BMICON_SMALLBAR_WHITE;

        if(i == CONFIRM_OPT_YES)//Default
            barID = BMICON_SMALLBAR_YELLOW;

        DrawSubItem_PowerOff_TV(uwDispID, i, CONFIRM_MENU_PAGE_ITEM, sMenuListDeleteOne[i]->uiStringId, sMenuListDeleteOne[i]->bmpIcon, barID, MENU_TEXT_COLOR, colorBk, 0x0);
    }

    AHC_OSDSetCurrentDisplay(uwDispID);
    AHC_OSDSetActive(uwDispID, 1);

    PowerOff_Option  = CONFIRM_OPT_YES;
}

void MenuDrawSubPage_PowerOff_TV_Browser(UINT16 uwDispID)
{
    UINT8   i;
    RECT    OsdRect;
    RECT    StrDecideRECT = RECT_TV_PWROFF_BROW_STR_DECIDE;

    switch(MenuSettingConfig()->uiTVSystem)
    {
        case TV_SYSTEM_NTSC:
            OsdRect.uiLeft      = POS_TV_NTSC_BROWSER_OSD_X0;
            OsdRect.uiTop       = POS_TV_NTSC_BROWSER_OSD_Y0;
            OsdRect.uiWidth     = POS_TV_NTSC_BROWSER_OSD_W;
            OsdRect.uiHeight    = POS_TV_NTSC_BROWSER_OSD_H;
        break;

        case TV_SYSTEM_PAL:
            OsdRect.uiLeft      = POS_TV_PAL_BROWSER_OSD_X0;
            OsdRect.uiTop       = POS_TV_PAL_BROWSER_OSD_Y0;
            OsdRect.uiWidth     = POS_TV_PAL_BROWSER_OSD_W;
            OsdRect.uiHeight    = POS_TV_PAL_BROWSER_OSD_H;
        break;
    }

    AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
    AHC_OSDDrawFillRect2(uwDispID, &OsdRect);

    ShowOSD_SetLanguage(uwDispID);
    OSD_ShowStringPool(uwDispID, IDS_DS_MSG_SURE_TO_POWER_OFF, StrDecideRECT, MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, GUI_TA_CENTER|GUI_TA_VCENTER);

    for( i=0; i<2; i++ )
    {
        GUI_COLOR   colorBackground = OSD_COLOR_TV_BACKGROUND;
        GUI_BITMAP  barID = BMICON_SUBBAR_WHITE;

        if(i == CONFIRM_OPT_YES)//Default
            barID = BMICON_SUBBAR_YELLOW;

        DrawSubItem_PowerOff_TV(uwDispID, i, 2, sMenuListDeleteOne[i]->uiStringId, sMenuListDeleteOne[i]->bmpIcon, barID, MENU_TEXT_COLOR, colorBackground, 0x0);
    }
}

void MenuDrawSubPage_PowerOff_TV_Play(UINT16 uwDispID)
{
    UINT8   i;
    RECT    PlayOsdRect     = RECT_TV_PWROFF_PLAY_REGION;
    RECT    StrDecideRECT   = RECT_TV_PWROFF_PLAY_STR_DECIDE;

    AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
    AHC_OSDDrawFillRect2(uwDispID, &PlayOsdRect);

    ShowOSD_SetLanguage(uwDispID);
    OSD_ShowStringPool(uwDispID, IDS_DS_MSG_SURE_TO_POWER_OFF, StrDecideRECT, MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, GUI_TA_CENTER|GUI_TA_VCENTER);

    for( i=0; i<2; i++ )
    {
        GUI_COLOR   colorBk = OSD_COLOR_TV_BACKGROUND;
        GUI_BITMAP  barID   = BMICON_SUBBAR_WHITE;

        if(i == CONFIRM_OPT_YES)//Default
            barID = BMICON_SUBBAR_YELLOW;

        DrawSubItem_PowerOff_TV(uwDispID, i, 2, sMenuListDeleteOne[i]->uiStringId, sMenuListDeleteOne[i]->bmpIcon, barID, MENU_TEXT_COLOR, colorBk, 0x0);
    }
}

void MenuDrawSubPage_PowerOff_TV(void)
{
    UINT8   bID0 = 0, bID1 = 0;

#if 1 //Andy Liu TBD. 
    printc(FG_RED("\r\nPlease modify here!\r\n"));
    AHC_PRINT_RET_ERROR(0, 0);
#else      
    switch(TVFunc_Status())
    {
        case AHC_TV_VIDEO_PREVIEW_STATUS:
        case AHC_TV_DSC_PREVIEW_STATUS:
            MenuDrawSubPage_PowerOff_TV_Preview();
        break;

        case AHC_TV_BROWSER_STATUS:
            OSDDraw_EnterDrawing(&bID0, &bID1);
            MenuDrawSubPage_PowerOff_TV_Browser(bID0);
            OSDDraw_ExitDrawing(&bID0,&bID1);
        break;

        case AHC_TV_MOVIE_PB_STATUS:
        case AHC_TV_PHOTO_PB_STATUS:
        case AHC_TV_SLIDE_SHOW_STATUS:
            OSDDraw_GetLastOvlDrawBuffer(&bID0);
            MenuDrawSubPage_PowerOff_TV_Play(bID0);
        break;
    }
#endif

    PowerOff_Option  = CONFIRM_OPT_YES;
}

void MenuDrawChangeSubItem_PowerOff_TV(void)
{
    GUI_COLOR   colorFill       = OSD_COLOR_TV_BACKGROUND;
    UINT8       bID0            = 0;
    UINT8       bID1            = 0;
    UINT32      uiPrevItem      = CONFIRM_OPT_NO;
    UINT32      uiCurrItem      = CONFIRM_OPT_YES;

#if 1 //Andy Liu TBD. 
    printc(FG_RED("\r\nPlease modify here!\r\n"));
    AHC_PRINT_RET_ERROR(0, 0);
#else      
    switch(TVFunc_Status())
    {
        case AHC_TV_VIDEO_PREVIEW_STATUS:
        case AHC_TV_DSC_PREVIEW_STATUS:
            bID0 = MAIN_DISPLAY_BUFFER;
        break;

        case AHC_TV_BROWSER_STATUS:
            OSDDraw_EnterDrawing(&bID0, &bID1);
            OSDDraw_ClearOvlDrawOvlBufferAll();
        break;

        case AHC_TV_MOVIE_PB_STATUS:
        case AHC_TV_PHOTO_PB_STATUS:
        case AHC_TV_SLIDE_SHOW_STATUS:
            OSDDraw_GetLastOvlDrawBuffer(&bID0);
        break;
    }
#endif

    if(PowerOff_Option==CONFIRM_OPT_YES)
    {
        uiPrevItem = CONFIRM_OPT_NO;
        uiCurrItem = CONFIRM_OPT_YES;
    }
    else if(PowerOff_Option==CONFIRM_OPT_NO)
    {
        uiPrevItem = CONFIRM_OPT_YES;
        uiCurrItem = CONFIRM_OPT_NO;
    }

    DrawSubItem_PowerOff_TV(bID0, uiPrevItem%2, 2, sMenuListDeleteOne[uiPrevItem]->uiStringId, sMenuListDeleteOne[uiPrevItem]->bmpIcon, BMICON_SUBBAR_WHITE,  MENU_TEXT_COLOR, colorFill, 0x0);
    DrawSubItem_PowerOff_TV(bID0, uiCurrItem,   2, sMenuListDeleteOne[uiCurrItem]->uiStringId, sMenuListDeleteOne[uiCurrItem]->bmpIcon, BMICON_SUBBAR_YELLOW, MENU_TEXT_COLOR, colorFill, 0x0);

#if 1 //Andy Liu TBD. 
    printc(FG_RED("\r\nPlease modify here!\r\n"));
    AHC_PRINT_RET_ERROR(0, 0);
#else      
    if(TVFunc_Status()==AHC_TV_BROWSER_STATUS)
    {
        OSDDraw_ExitDrawing(&bID0, &bID1);
    }
#endif    
}

#endif

#if 0
void ________PowerOff_Function_HDMI________(){ruturn;} //dummy
#endif

#if (HDMI_ENABLE)

void DrawSubItem_PowerOff_HDMI(UINT16 uwDispID, int iItem, int iTotalItems, UINT32 iStrID, const GUI_BITMAP* IconID,  GUI_BITMAP barID, GUI_COLOR clrFont, GUI_COLOR clrBack, GUI_COLOR clrSelect)
{
    RECT    rc, tmpRECT;
    RECT    StrYesRECT          = RECT_HDMI_PWOFF_PLAY_STR_YES;
    RECT    StrNoRECT           = RECT_HDMI_PWOFF_PLAY_STR_NO;
    RECT    StrBrwYesRECT_1080P = RECT_HDMI_1080P_PWOFF_BROW_YES;
    RECT    StrBrwNoRECT_1080P  = RECT_HDMI_1080P_PWOFF_BROW_NO;
    RECT    StrBrwYesRECT_1080I = RECT_HDMI_1080I_PWOFF_BROW_YES;
    RECT    StrBrwNoRECT_1080I  = RECT_HDMI_1080I_PWOFF_BROW_NO;
    RECT    StrBrwYesRECT_720P  = RECT_HDMI_720P_PWOFF_BROW_YES;
    RECT    StrBrwNoRECT_720P   = RECT_HDMI_720P_PWOFF_BROW_NO;
    RECT    StrBrwYesRECT_480P  = RECT_HDMI_480P_PWOFF_BROW_YES;    // TBD
    RECT    StrBrwNoRECT_480P   = RECT_HDMI_480P_PWOFF_BROW_NO;     // TBD

#if 1 //Andy Liu TBD. 
    printc(FG_RED("\r\nPlease modify here!\r\n"));
    AHC_PRINT_RET_ERROR(0, 0);
#else
    switch(HDMIFunc_Status())
    {
        case AHC_HDMI_VIDEO_PREVIEW_STATUS:
        case AHC_HDMI_DSC_PREVIEW_STATUS:
            rc.uiWidth  = barID.XSize;
            rc.uiHeight = barID.YSize;
            GetSubItemRect(uwDispID, iItem%CONFIRM_MENU_PAGE_ITEM, iTotalItems, &rc );
            rc.uiTop = rc.uiTop + 50;
        break;

        case AHC_HDMI_BROWSER_STATUS:
            if(MenuSettingConfig()->uiHDMIOutput==HDMI_OUTPUT_1080I)
                rc = (iItem==CONFIRM_OPT_YES)?(StrBrwYesRECT_1080I):(StrBrwNoRECT_1080I);
            else if(MenuSettingConfig()->uiHDMIOutput==HDMI_OUTPUT_720P)
                rc = (iItem==CONFIRM_OPT_YES)?(StrBrwYesRECT_720P):(StrBrwNoRECT_720P);
            else if(MenuSettingConfig()->uiHDMIOutput==HDMI_OUTPUT_480P)    // TBD
                rc = (iItem==CONFIRM_OPT_YES)?(StrBrwYesRECT_480P):(StrBrwNoRECT_480P);
            else //if(MenuSettingConfig()->uiHDMIOutput==HDMI_OUTPUT_1080P)     // TBD
                rc = (iItem==CONFIRM_OPT_YES)?(StrBrwYesRECT_1080P):(StrBrwNoRECT_1080P);
        break;

        case AHC_HDMI_MOVIE_PB_STATUS:
        case AHC_HDMI_PHOTO_PB_STATUS:
        case AHC_HDMI_SLIDE_SHOW_STATUS:
            rc = (iItem==CONFIRM_OPT_YES)?(StrYesRECT):(StrNoRECT);
        break;
    }
#endif

    OSD_Draw_Icon(barID, rc, uwDispID);

    tmpRECT.uiLeft   = rc.uiLeft    + STR_RECT_OFFSET_X;
    tmpRECT.uiTop    = rc.uiTop     + STR_RECT_OFFSET_Y;
    tmpRECT.uiWidth  = rc.uiWidth   + STR_RECT_OFFSET_W;
    tmpRECT.uiHeight = rc.uiHeight  - 9;
    OSD_ShowStringPool(uwDispID, iStrID, tmpRECT, clrFont, clrBack, GUI_TA_HCENTER|GUI_TA_VCENTER);
}

void MenuDrawSubPage_PowerOffCancel_HDMI(void)
{
    UINT8   uwDispID            = 0;
    RECT    PlayOsdRect         = RECT_HDMI_PWROFF_PLAY_REGION;
    RECT    BrwOsdRect          = {0, 0, 0 , 0};
    RECT    BrwOsdRect_1080P    = POS_HDMI_1080P_BROWSER_OSD;
    RECT    BrwOsdRect_1080I    = POS_HDMI_1080I_BROWSER_OSD;
    RECT    BrwOsdRect_720P     = POS_HDMI_720P_BROWSER_OSD;

#if 1 //Andy Liu TBD. 
    printc(FG_RED("\r\nPlease modify here!\r\n"));
    AHC_PRINT_RET_ERROR(0, 0);
#else
    switch(HDMIFunc_Status())
    {
        case AHC_HDMI_VIDEO_PREVIEW_STATUS:
        case AHC_HDMI_DSC_PREVIEW_STATUS:
            AHC_OSDSetColor(MAIN_DISPLAY_BUFFER, OSD_COLOR_TRANSPARENT);
            AHC_OSDDrawFillRect(MAIN_DISPLAY_BUFFER, 0, 0, TV_HDMI_DISP_BUF_WIDTH, TV_HDMI_DISP_BUF_HEIGHT);
            AHC_OSDSetActive(MAIN_DISPLAY_BUFFER, 0);
            AHC_OSDSetActive(HDMIFunc_GetUImodeOsdID(), 1);
        break;

        case AHC_HDMI_BROWSER_STATUS:

            OSDDraw_GetLastOvlDrawBuffer(&uwDispID);
            AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);

            if(MenuSettingConfig()->uiHDMIOutput==HDMI_OUTPUT_1080I)
                BrwOsdRect = BrwOsdRect_1080I;
            else if(MenuSettingConfig()->uiHDMIOutput==HDMI_OUTPUT_720P)
                BrwOsdRect = BrwOsdRect_720P;
            else if(MenuSettingConfig()->uiHDMIOutput==HDMI_OUTPUT_480P)        // TBD
                BrwOsdRect = BrwOsdRect_480P;
            else    //if(MenuSettingConfig()->uiHDMIOutput==HDMI_OUTPUT_1080P)
                BrwOsdRect = BrwOsdRect_1080P;

            AHC_OSDDrawFillRect2(uwDispID, &BrwOsdRect);
            DrawStateBrowserInit();
        break;

        case AHC_HDMI_MOVIE_PB_STATUS:
            if(!m_DelFile)
            {
                OSDDraw_GetLastOvlDrawBuffer(&uwDispID);
                AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
                AHC_OSDDrawFillRect2(uwDispID, &PlayOsdRect);
                DrawHdmiMoviePB_Status(uwDispID, POS_HDMI_MOV_STATE_X, POS_HDMI_MOV_STATE_Y, LARGE_SIZE, MAG_2X, AHC_TRUE, OSD_COLOR_TRANSPARENT, NULL_PARAM);
            }
            else
                DrawStateHdmiMoviePBDel(m_DelFile);
        break;

        case AHC_HDMI_PHOTO_PB_STATUS:
            if(!m_DelFile)
            {
                OSDDraw_GetLastOvlDrawBuffer(&uwDispID);
                AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
                AHC_OSDDrawFillRect2(uwDispID, &PlayOsdRect);
                DrawStateHdmiPhotoPBChangeFile();
            }
            else
                DrawStateHdmiMoviePBDel(m_DelFile);
        break;

        case AHC_HDMI_SLIDE_SHOW_STATUS:
            if(!m_DelFile)
            {
                OSDDraw_GetLastOvlDrawBuffer(&uwDispID);
                AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
                AHC_OSDDrawFillRect2(uwDispID, &PlayOsdRect);
                DrawStateHdmiSlideShowChangeFile();
            }
            else
                DrawStateHdmiMoviePBDel(m_DelFile);
        break;
    }
#endif    
}

void MenuDrawSubPage_PowerOff_HDMI_Preview(void)
{
    UINT32      StartX   = 145;
    UINT32      StartY   = 50;
    UINT16      uwDispID  = MAIN_DISPLAY_BUFFER;
    UINT32      i;
    GUI_RECT    Rect;
    UINT32      iVal[2];
    UINT16      Width, Height;

    AHC_OSDSetActive(uwDispID, 0);

    AHC_GetHdmiDisplayWidthHeight(&Width, &Height);

    iVal[0] = (Width - TV_HDMI_DISP_BUF_WIDTH)/2;
    iVal[1] = (Height - TV_HDMI_DISP_BUF_HEIGHT)/2;
    AHC_OSDSetDisplayAttr(uwDispID, AHC_OSD_ATTR_DISPLAY_OFFSET, iVal);

    OSDDraw_SetAlphaBlending(uwDispID, AHC_FALSE);

    AHC_OSDSetPenSize(uwDispID, 5);
    AHC_OSDSetColor(uwDispID, TV_MENU_BK_COLOR);
    AHC_OSDDrawFillRect(uwDispID, 0, 0, TV_HDMI_DISP_BUF_WIDTH, TV_HDMI_DISP_BUF_HEIGHT);

    AHC_OSDSetColor(uwDispID, MENU_TEXT_COLOR);
    AHC_OSDSetBkColor(uwDispID, TV_MENU_BK_COLOR);

    Rect.x0 = 0;
    Rect.y0 = 110;
    Rect.x1 = 320;
    Rect.y1 = 140;

    ShowOSD_SetLanguage(uwDispID);
    AHC_OSDDrawTextInRect(uwDispID, (UINT8*)OSD_GetStringViaID(IDS_DS_MSG_SURE_TO_POWER_OFF), &Rect, GUI_TA_CENTER|GUI_TA_VCENTER);

    AHC_OSDDrawBitmap(uwDispID, &bmIcon_WMSG, StartX, StartY);

    for( i=0; i<2; i++ )
    {
        GUI_COLOR   colorBk = OSD_COLOR_TRANSPARENT;
        GUI_BITMAP  barID   = BMICON_SMALLBAR_WHITE;

        if(i == CONFIRM_OPT_YES)//Default
            barID = BMICON_SMALLBAR_YELLOW;

        DrawSubItem_PowerOff_HDMI(uwDispID, i, CONFIRM_MENU_PAGE_ITEM, sMenuListDeleteOne[i]->uiStringId, sMenuListDeleteOne[i]->bmpIcon, barID, MENU_TEXT_COLOR, colorBk, 0x0);
    }

    AHC_OSDSetCurrentDisplay(uwDispID);
    AHC_OSDSetActive(uwDispID, 1);

    PowerOff_Option  = CONFIRM_OPT_YES;
}

void MenuDrawSubPage_PowerOff_HDMI_Browser(UINT16 uwDispID)
{
    UINT8   i;
    RECT    OsdRect;
    RECT    StrDecideRECT;
    RECT    StrDecideRECT1080P = RECT_HDMI_1080P_PWOFF_BROW_DECIDE;
    RECT    StrDecideRECT1080I = RECT_HDMI_1080I_PWOFF_BROW_DECIDE;
    RECT    StrDecideRECT720P  = RECT_HDMI_720P_PWOFF_BROW_DECIDE;

    switch(MenuSettingConfig()->uiHDMIOutput)
    {
        case HDMI_OUTPUT_1080I:
            OsdRect.uiLeft          = POS_HDMI_1080I_BROWSER_OSD_X0;
            OsdRect.uiTop           = POS_HDMI_1080I_BROWSER_OSD_Y0;
            OsdRect.uiWidth         = POS_HDMI_1080I_BROWSER_OSD_W;
            OsdRect.uiHeight        = POS_HDMI_1080I_BROWSER_OSD_H;

            StrDecideRECT           = StrDecideRECT1080I;
            break;
        case HDMI_OUTPUT_720P:
            OsdRect.uiLeft          = POS_HDMI_720P_BROWSER_OSD_X0;
            OsdRect.uiTop           = POS_HDMI_720P_BROWSER_OSD_Y0;
            OsdRect.uiWidth         = POS_HDMI_720P_BROWSER_OSD_W;
            OsdRect.uiHeight        = POS_HDMI_720P_BROWSER_OSD_H;

            StrDecideRECT           = StrDecideRECT720P;
            break;
        case HDMI_OUTPUT_480P:      // TBD
            OsdRect.uiLeft          = POS_HDMI_480P_BROWSER_OSD_X0;
            OsdRect.uiTop           = POS_HDMI_480P_BROWSER_OSD_Y0;
            OsdRect.uiWidth         = POS_HDMI_480P_BROWSER_OSD_W;
            OsdRect.uiHeight        = POS_HDMI_480P_BROWSER_OSD_H;

            StrDecideRECT           = StrDecideRECT480P;
            break;
        default:
        case HDMI_OUTPUT_1080P:
            OsdRect.uiLeft          = POS_HDMI_1080P_BROWSER_OSD_X0;
            OsdRect.uiTop           = POS_HDMI_1080P_BROWSER_OSD_Y0;
            OsdRect.uiWidth         = POS_HDMI_1080P_BROWSER_OSD_W;
            OsdRect.uiHeight        = POS_HDMI_1080P_BROWSER_OSD_H;

            StrDecideRECT           = StrDecideRECT1080P;
            break;
    }

    AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
    AHC_OSDDrawFillRect2(uwDispID, &OsdRect);

    ShowOSD_SetLanguage(uwDispID);
    OSD_ShowStringPool(uwDispID, IDS_DS_MSG_SURE_TO_POWER_OFF, StrDecideRECT, MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, GUI_TA_CENTER|GUI_TA_VCENTER);

    for( i=0; i<2; i++ )
    {
        GUI_COLOR   colorBk = OSD_COLOR_TV_BACKGROUND;
        GUI_BITMAP  barID   = BMICON_SUBBAR_WHITE;

        if(i == CONFIRM_OPT_YES)//Default
            barID = BMICON_SUBBAR_YELLOW;

        DrawSubItem_PowerOff_HDMI(uwDispID, i, 2, sMenuListDeleteOne[i]->uiStringId, sMenuListDeleteOne[i]->bmpIcon, barID, MENU_TEXT_COLOR, colorBk, 0x0);
    }
}

void MenuDrawSubPage_PowerOff_HDMI_Play(UINT16 uwDispID)
{
    UINT8   i;
    RECT    PlayOsdRect     = RECT_HDMI_PWROFF_PLAY_REGION;
    RECT    StrDecideRECT   = RECT_HDMI_PWROFF_PLAY_STR_DECIDE;

    AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);
    AHC_OSDDrawFillRect2(uwDispID, &PlayOsdRect);

    ShowOSD_SetLanguage(uwDispID);
    OSD_ShowStringPool(uwDispID, IDS_DS_MSG_SURE_TO_POWER_OFF, StrDecideRECT, MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, GUI_TA_CENTER|GUI_TA_VCENTER);

    for( i=0; i<2; i++ )
    {
        GUI_COLOR   colorBk = OSD_COLOR_TV_BACKGROUND;
        GUI_BITMAP  barID   = BMICON_SUBBAR_WHITE;

        if(i == CONFIRM_OPT_YES)//Default
            barID = BMICON_SUBBAR_YELLOW;

        DrawSubItem_PowerOff_HDMI(uwDispID, i, 2, sMenuListDeleteOne[i]->uiStringId, sMenuListDeleteOne[i]->bmpIcon, barID, MENU_TEXT_COLOR, colorBk, 0x0);
    }
}

void MenuDrawSubPage_PowerOff_HDMI(void)
{
    UINT8   bID0 = 0, bID1 = 0;

#if 1 //Andy Liu TBD. 
    printc(FG_RED("\r\nPlease modify here!\r\n"));
    AHC_PRINT_RET_ERROR(0, 0);
#else
    switch(HDMIFunc_Status())
    {
        case AHC_HDMI_VIDEO_PREVIEW_STATUS:
        case AHC_HDMI_DSC_PREVIEW_STATUS:
            MenuDrawSubPage_PowerOff_HDMI_Preview();
        break;

        case AHC_HDMI_BROWSER_STATUS:
            OSDDraw_EnterDrawing(&bID0, &bID1);
            MenuDrawSubPage_PowerOff_HDMI_Browser(bID0);
            OSDDraw_ExitDrawing(&bID0, &bID1);
        break;

        case AHC_HDMI_MOVIE_PB_STATUS:
        case AHC_HDMI_PHOTO_PB_STATUS:
        case AHC_HDMI_SLIDE_SHOW_STATUS:
            OSDDraw_GetLastOvlDrawBuffer(&bID0);
            MenuDrawSubPage_PowerOff_HDMI_Play(bID0);
        break;
    }
#endif

    PowerOff_Option  = CONFIRM_OPT_YES;//Default
}

void MenuDrawChangeSubItem_PowerOff_HDMI(void)
{
    GUI_COLOR   colorFill       = OSD_COLOR_TV_BACKGROUND;
    UINT8       bID0            = 0;
    UINT8       bID1            = 0;
    UINT32      uiPrevItem      = CONFIRM_OPT_NO;
    UINT32      uiCurrItem      = CONFIRM_OPT_YES;

#if 1 //Andy Liu TBD. 
    printc(FG_RED("\r\nPlease modify here!\r\n"));
    AHC_PRINT_RET_ERROR(0, 0);
#else
    switch(HDMIFunc_Status())
    {
        case AHC_HDMI_VIDEO_PREVIEW_STATUS:
        case AHC_HDMI_DSC_PREVIEW_STATUS:
            bID0 = MAIN_DISPLAY_BUFFER;
        break;

        case AHC_HDMI_BROWSER_STATUS:
            OSDDraw_EnterDrawing(&bID0, &bID1);
            OSDDraw_ClearOvlDrawOvlBufferAll();
        break;

        case AHC_HDMI_MOVIE_PB_STATUS:
        case AHC_HDMI_PHOTO_PB_STATUS:
        case AHC_HDMI_SLIDE_SHOW_STATUS:
            OSDDraw_GetLastOvlDrawBuffer(&bID0);
        break;
    }
#endif

    if(PowerOff_Option==CONFIRM_OPT_YES)
    {
        uiPrevItem = CONFIRM_OPT_NO;
        uiCurrItem = CONFIRM_OPT_YES;
    }
    else if(PowerOff_Option==CONFIRM_OPT_NO)
    {
        uiPrevItem = CONFIRM_OPT_YES;
        uiCurrItem = CONFIRM_OPT_NO;
    }

    DrawSubItem_PowerOff_HDMI(bID0, uiPrevItem%2, 2, sMenuListDeleteOne[uiPrevItem]->uiStringId, sMenuListDeleteOne[uiPrevItem]->bmpIcon, BMICON_SUBBAR_WHITE,  MENU_TEXT_COLOR, colorFill, 0x0);
    DrawSubItem_PowerOff_HDMI(bID0, uiCurrItem,   2, sMenuListDeleteOne[uiCurrItem]->uiStringId, sMenuListDeleteOne[uiCurrItem]->bmpIcon, BMICON_SUBBAR_YELLOW, MENU_TEXT_COLOR, colorFill, 0x0);

#if 1 //Andy Liu TBD. 
    printc(FG_RED("\r\nPlease modify here!\r\n"));
    AHC_PRINT_RET_ERROR(0, 0);
#else
    if(HDMIFunc_Status()==AHC_HDMI_BROWSER_STATUS)
    {
        OSDDraw_ExitDrawing(&bID0, &bID1);
    }
#endif
}

#endif

#if 0
void ________PowerOff_Function_Normal________(){ruturn;} //dummy
#endif

void DrawSubItem_PowerOff(UINT16 uwDispID, int iItem, int iTotalItems, UINT32 iStrID, const GUI_BITMAP* IconID, GUI_BITMAP barID, GUI_COLOR clrFont, GUI_COLOR clrBack, GUI_COLOR clrSelect)
{
    RECT rc, tmpRECT;

    rc.uiWidth  = barID.XSize;
    rc.uiHeight = barID.YSize;
    GetSubItemRect(uwDispID, iItem%CONFIRM_MENU_PAGE_ITEM, iTotalItems, &rc );

    rc.uiTop = rc.uiTop + 50;

    OSD_Draw_Icon(barID, rc, uwDispID);

    tmpRECT.uiLeft   = rc.uiLeft    + STR_RECT_OFFSET_X;
    tmpRECT.uiTop    = rc.uiTop     + STR_RECT_OFFSET_Y;
    tmpRECT.uiWidth  = rc.uiWidth   + STR_RECT_OFFSET_W;
    tmpRECT.uiHeight = rc.uiHeight  + STR_RECT_OFFSET_H;
    OSD_ShowStringPool3(uwDispID, iStrID, tmpRECT, clrFont, clrBack, GUI_TA_HCENTER|GUI_TA_VCENTER);

    #ifdef FLM_GPIO_NUM
    AHC_OSDRefresh_PLCD();
    #endif
}

void MenuDrawSubPage_PowerOff(PSMENUSTRUCT pMenu)
{
    UINT32 StartX    = 145;
    UINT32 StartY    = 50;
    UINT16 uwDispID  = MAIN_DISPLAY_BUFFER;
    UINT32 i;
    GUI_RECT Rect;

#if (TVOUT_ENABLE)
    if(uiGetCurrentState()==UI_TVOUT_STATE)
    {
        MenuDrawSubPage_PowerOff_TV();
        return;
    }
#endif
#if (HDMI_ENABLE)
    if(uiGetCurrentState()==UI_HDMI_STATE)
    {
        MenuDrawSubPage_PowerOff_HDMI();
        return;
    }
#endif

    if(AHC_OS_AcquireSem(m_WMSGSemID, 0) != OS_NO_ERR)
        return;

    AHC_OSDSetActive(uwDispID, 0);

    OSDDraw_SetAlphaBlending(uwDispID, AHC_FALSE);
    OSDDraw_SetSemiTransparent(uwDispID, AHC_FALSE, AHC_OSD_SEMITP_AVG, 0);

    AHC_OSDSetPenSize(uwDispID, 5);
    AHC_OSDSetColor(uwDispID, MENU_BACKGROUND_COLOR);
    AHC_OSDSetBkColor(uwDispID, MENU_BACKGROUND_COLOR);
    AHC_OSDDrawFillRect(uwDispID, 0, 0, AHC_GET_ATTR_OSD_W(uwDispID), AHC_GET_ATTR_OSD_H(uwDispID));

    AHC_OSDSetColor(uwDispID, MENU_TEXT_COLOR);
    AHC_OSDSetBkColor(uwDispID, MENU_BACKGROUND_COLOR);

    Rect.x0 = 0;
    Rect.y0 = 110;
    Rect.x1 = 320;
    Rect.y1 = 140;

    ShowOSD_SetLanguage(uwDispID);
    AHC_OSDDrawTextInRect(uwDispID, (UINT8*)OSD_GetStringViaID(IDS_DS_MSG_SURE_TO_POWER_OFF), &Rect, GUI_TA_CENTER|GUI_TA_VCENTER);

    AHC_OSDDrawBitmap(uwDispID, &bmIcon_WMSG, StartX, StartY);

    for( i=0; i<2; i++ )
    {
        GUI_COLOR   colorBk = MENU_BACKGROUND_COLOR;
        GUI_BITMAP  barID   = BMICON_SMALLBAR_WHITE;

        if(i == CONFIRM_OPT_YES)//Default
            barID = BMICON_SMALLBAR_YELLOW;

        DrawSubItem_PowerOff(uwDispID, i, CONFIRM_MENU_PAGE_ITEM, sMenuListDeleteOne[i]->uiStringId, sMenuListDeleteOne[i]->bmpIcon, barID, MENU_TEXT_COLOR, colorBk, 0x0);
    }

    AHC_OSDSetCurrentDisplay(uwDispID);
    AHC_OSDSetActive(uwDispID, 1);

    PowerOff_Option  = CONFIRM_OPT_YES;

    #ifdef FLM_GPIO_NUM
    AHC_OSDRefresh_PLCD();
    #endif

    AHC_OS_ReleaseSem(m_WMSGSemID);
}

void MenuDrawChangeSubItem_PowerOff(void)
{
    GUI_COLOR   colorFill       = MENU_BACKGROUND_COLOR;
    UINT16      uwDispID        = MAIN_DISPLAY_BUFFER;
    UINT32      uiPrevItem      = CONFIRM_OPT_NO;
    UINT32      uiCurrItem      = CONFIRM_OPT_YES;

#if (TVOUT_ENABLE)
    if(uiGetCurrentState()==UI_TVOUT_STATE)
    {
        MenuDrawChangeSubItem_PowerOff_TV();
        return;
    }
#endif
#if (HDMI_ENABLE)
    if(uiGetCurrentState()==UI_HDMI_STATE)
    {
        MenuDrawChangeSubItem_PowerOff_HDMI();
        return;
    }
#endif

    if(AHC_OS_AcquireSem(m_WMSGSemID, 0) != OS_NO_ERR)
        return;

    if(PowerOff_Option==CONFIRM_OPT_YES)
    {
        uiPrevItem = CONFIRM_OPT_NO;
        uiCurrItem = CONFIRM_OPT_YES;
    }
    else if(PowerOff_Option==CONFIRM_OPT_NO)
    {
        uiPrevItem = CONFIRM_OPT_YES;
        uiCurrItem = CONFIRM_OPT_NO;
    }

    DrawSubItem_PowerOff(uwDispID, uiPrevItem%SUB_MENU_PAGE_ITEM, 2, sMenuListDeleteOne[uiPrevItem]->uiStringId, sMenuListDeleteOne[uiPrevItem]->bmpIcon, BMICON_SMALLBAR_WHITE,  MENU_TEXT_COLOR, colorFill, 0x0);
    DrawSubItem_PowerOff(uwDispID, uiCurrItem,                    2, sMenuListDeleteOne[uiCurrItem]->uiStringId, sMenuListDeleteOne[uiCurrItem]->bmpIcon, BMICON_SMALLBAR_YELLOW, MENU_TEXT_COLOR, colorFill, 0x0);

    #ifdef FLM_GPIO_NUM
    AHC_OSDRefresh_PLCD();
    #endif

    AHC_OS_ReleaseSem(m_WMSGSemID);
}

#endif

#if 0
void ________ExitMenu_Function_________(){ruturn;} //dummy
#endif

#if (POWER_ON_MENU_SET_EN) || (EXIT_MENU_PAGE_EN)

void MenuDrawExitItem(UINT16 uwDispID, int iItem, UINT32 iStrID, GUI_BITMAP barID, GUI_COLOR clrFont, GUI_COLOR clrBack)
{
    RECT rc, tmpRECT;
    RECT tmpRECT1 = RECT_EXIT_MENU_STR;
    RECT tmpRECT2 = RECT_EXIT_MENU_ICON;

    rc.uiWidth  = barID.XSize;
    rc.uiHeight = barID.YSize;
    GetSubItemRect(uwDispID, iItem%CONFIRM_MENU_PAGE_ITEM, CONFIRM_MENU_PAGE_ITEM, &rc );

    AHC_OSDSetBkColor(uwDispID, clrBack);
    OSD_Draw_Icon(bmIcon_Exit, tmpRECT2, uwDispID);

    OSD_ShowStringPool(uwDispID, IDS_DS_EXITMENU, tmpRECT1, clrFont, clrBack, GUI_TA_HCENTER|GUI_TA_VCENTER);

    rc.uiTop += 45;

    OSD_Draw_Icon(barID, rc, uwDispID);

    AHC_OSDSetColor(uwDispID, clrBack);
    tmpRECT.uiLeft   = rc.uiLeft    + STR_RECT_OFFSET_X;
    tmpRECT.uiTop    = rc.uiTop     + STR_RECT_OFFSET_Y;
    tmpRECT.uiWidth  = rc.uiWidth   + STR_RECT_OFFSET_W;
    tmpRECT.uiHeight = rc.uiHeight  + STR_RECT_OFFSET_H;
    OSD_ShowStringPool(uwDispID, iStrID, tmpRECT, clrFont, clrBack, GUI_TA_HCENTER|GUI_TA_VCENTER);
}

void MenuDrawExitMainPage(PSMENUSTRUCT pMenu)
{
    UINT8  bID0 = 0;
    UINT8  bID1 = 0;
    UINT32 i;

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    OSDDraw_SetAlphaBlending(bID0, AHC_TRUE);
    OSDDraw_SetAlphaBlending(bID1, AHC_TRUE);

    OSDDraw_ClearOvlDrawBuffer(bID0);

    MenuMainDrawBackCurtain(bID0, OSD_COLOR_BLACK);

    MenuMainDrawCatagory(bID0, GetCatagoryMenuID(pMenu));

    if( pMenu->uiStringId != -1 )
    {
        MenuDrawTitle(bID0, pMenu->uiStringId);
    }

    for( i=0; i< pMenu->iNumOfItems; i++ )
    {
        GUI_BITMAP barID = BMICON_SMALLBAR_WHITE;

        if( i == pMenu->iCurrentPos )
        {
            barID = BMICON_SMALLBAR_YELLOW;
        }

        MenuDrawExitItem(bID0, i%CONFIRM_MENU_PAGE_ITEM, pMenu->pItemsList[i]->uiStringId, barID, MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT);
    }

    MenuDraw_BatteryStatus(bID0);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}

void MenuDrawChangeExitItem(PSMENUSTRUCT pMenu, UINT32 uiCurrItem, UINT32 uiPrevItem)
{
    UINT8     bID0 = 0;
    UINT8     bID1 = 0;

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    MenuDrawExitItem(bID0, uiPrevItem%CONFIRM_MENU_PAGE_ITEM, pMenu->pItemsList[uiPrevItem]->uiStringId, BMICON_SMALLBAR_WHITE,  MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT);
    MenuDrawExitItem(bID0, uiCurrItem%CONFIRM_MENU_PAGE_ITEM, pMenu->pItemsList[uiCurrItem]->uiStringId, BMICON_SMALLBAR_YELLOW, MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}

#endif

#if 0
void ________MainMenu_Function_________(){ruturn;} //dummy
#endif


#if (MENU_WIFI_PAGE_EN)
void DrawCatagoryMenuIcon(UINT16 uwDispID, AHC_BOOL ManualHL, AHC_BOOL MovieHL, AHC_BOOL StillHL, AHC_BOOL PlaybackHL, AHC_BOOL EditHL, AHC_BOOL MediaHL, AHC_BOOL GeneralHL, AHC_BOOL WifiHL)
#else
void DrawCatagoryMenuIcon(UINT16 uwDispID, AHC_BOOL ManualHL, AHC_BOOL MovieHL, AHC_BOOL StillHL, AHC_BOOL PlaybackHL, AHC_BOOL EditHL, AHC_BOOL MediaHL, AHC_BOOL GeneralHL)
#endif
{
    RECT        tmpRECT  = RECT_2NDMENU_CATAGORY;
    RECT        tmpRECT1 = RECT_2NDMENU_CATAGORY;
    GUI_BITMAP  IconBmp;
    int         g;

    #if (DISP_WIDTH == 480) && defined(STRETCH_X) && defined(STRETCH_Y)
    #define ICON_GAP(i)     (STRETCH_X(35) * i)
    #else
    #define ICON_GAP(i)     (35*i)
    #endif

    tmpRECT.uiTop = AHC_GET_ATTR_OSD_W(uwDispID)-34;
    tmpRECT1.uiTop = AHC_GET_ATTR_OSD_H(uwDispID)-34;

    g = 0;
    IconBmp = (MovieHL)?(BMICON_CATALOG_MOVIE_MENU_HL):(BMICON_CATALOG_MOVIE_MENU);
    OSD_Draw_Icon(IconBmp, tmpRECT1, uwDispID);

    tmpRECT1.uiLeft = tmpRECT.uiLeft-1 + ICON_GAP(++g);
#if (DSC_MODE_ENABLE)
    IconBmp = (StillHL)?(BMICON_CATALOG_CAMERA_MENU_HL):(BMICON_CATALOG_CAMERA_MENU);
    OSD_Draw_Icon(IconBmp, tmpRECT1, uwDispID);

    tmpRECT1.uiLeft = tmpRECT.uiLeft-1 + ICON_GAP(++g);
#endif
    IconBmp = (PlaybackHL)?(BMICON_CATALOG_PLAYBACK_MENU_HL):(BMICON_CATALOG_PLAYBACK_MENU);
    OSD_Draw_Icon(IconBmp, tmpRECT1, uwDispID);

    tmpRECT1.uiLeft = tmpRECT.uiLeft-1 + ICON_GAP(++g);

    IconBmp = (MediaHL)?(BMICON_CATALOG_MEDIA_MENU_HL):(BMICON_CATALOG_MEDIA_MENU);
    OSD_Draw_Icon(IconBmp, tmpRECT1, uwDispID);

    tmpRECT1.uiLeft = tmpRECT.uiLeft-1 + ICON_GAP(++g);

    IconBmp = (GeneralHL)?(BMICON_CATALOG_GENERAL_MENU_HL):(BMICON_CATALOG_GENERAL_MENU);
    OSD_Draw_Icon(IconBmp, tmpRECT1, uwDispID);

#if (MENU_WIFI_PAGE_EN)
    tmpRECT1.uiLeft = tmpRECT.uiLeft-1 + ICON_GAP(++g);

    IconBmp = (WifiHL)?(BMICON_CATALOG_GENERAL_MENU_HL):(BMICON_CATALOG_GENERAL_MENU);
    OSD_Draw_Icon(IconBmp, tmpRECT1, uwDispID);
#endif

}

void MenuMainDrawCatagory(UINT16 uwDispID, int  iCata)
{
    switch(iCata)
    {
#if (MENU_WIFI_PAGE_EN)
        case 0:
            DrawCatagoryMenuIcon(uwDispID, 1 , 0 , 0 , 0 , 0 , 0 , 0 , 0);
        break;

        case 1:
            DrawCatagoryMenuIcon(uwDispID, 0 , 1 , 0 , 0 , 0 , 0 , 0 , 0);
        break;

        case 2:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 1 , 0 , 0 , 0 , 0 , 0);
        break;

        case 3:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 1 , 0 , 0 , 0 , 0);
        break;

        case 4:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 0 , 1 , 0 , 0 , 0);
        break;

        case 5:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 0 , 0 , 1 , 0 , 0);
        break;

        case 6:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0);
        break;

        case 7:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0);
        break;

        case 8:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1);
        break;
#else
        case 0:
            DrawCatagoryMenuIcon(uwDispID, 1 , 0 , 0 , 0 , 0 , 0 , 0);
        break;

        case 1:
            DrawCatagoryMenuIcon(uwDispID, 0 , 1 , 0 , 0 , 0 , 0 , 0);
        break;

        case 2:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 1 , 0 , 0 , 0 , 0);
        break;

        case 3:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 1 , 0 , 0 , 0);
        break;

        case 4:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 0 , 1 , 0 , 0);
        break;

        case 5:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 0 , 0 , 1 , 0);
        break;

        case 6:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 0 , 0 , 0 , 1);
        break;

        case 7:
            DrawCatagoryMenuIcon(uwDispID, 0 , 0 , 0 , 0 , 0 , 0 , 0);
        break;
#endif
    }
}
void DrawAudPb_ProgressBar_Menu(UINT8 ubID, UINT32 CurTime, UINT32 MaxTime, AHC_BOOL bClear)
{ 
    UINT16 BarStartX 	= 40;
    UINT16 BarStartY 	= 140;
    UINT32 MaxBarLine 	= m_OSD[ubID]->width - BarStartX*2;
    UINT32 BarPos;
    static UINT32  tempBarPos = 0;

    BarPos = (MaxBarLine * CurTime) / MaxTime;

    if(BarPos == tempBarPos)
        return;

    tempBarPos = BarPos;    
    
	if(bClear)
	{
		AHC_OSDSetColor(ubID, OSD_COLOR_BLACK);
		AHC_OSDDrawFillRect(ubID, BarStartX-5, BarStartY-5, BarStartX+MaxBarLine+5, BarStartY+5);
	}
	else
	{
	    AHC_OSDSetColor(ubID, OSD_COLOR_RED);
	    AHC_OSDSetPenSize(ubID, 3);
	      
	    AHC_OSDDrawLine(ubID, BarStartX, 				BarStartX+BarPos, 		BarStartY+2, BarStartY+2);
	    
	    AHC_OSDSetColor(ubID, OSD_COLOR_WHITE);
	    AHC_OSDSetPenSize(ubID, 1);
	    
	    AHC_OSDDrawLine(ubID, BarStartX, 				BarStartX+MaxBarLine, 	BarStartY, 	 BarStartY);
	    AHC_OSDDrawLine(ubID, BarStartX, 				BarStartX+MaxBarLine, 	BarStartY+5, BarStartY+5);
	    AHC_OSDDrawLine(ubID, BarStartX-1, 				BarStartX-1, 			BarStartY-2, BarStartY+5);
	    AHC_OSDDrawLine(ubID, BarStartX+MaxBarLine+1, 	BarStartX+MaxBarLine+1, BarStartY-2, BarStartY+5);
	}
}
void MenuMainDrawBackCurtain(UINT16 uwDispID, GUI_COLOR bkColor)
{
    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
        AHC_OSDSetColor(uwDispID, TV_MENU_BK_COLOR);
    else
        AHC_OSDSetColor(uwDispID, OSD_COLOR_TITLE);

#if (DISP_WIDTH == 480) && defined(STRETCH_X) && defined(STRETCH_Y)
    AHC_OSDDrawFillRect(uwDispID, 0, STRETCH_Y(  0), STRETCH_X(320), STRETCH_Y( 36));
    AHC_OSDDrawFillRect(uwDispID, 0, STRETCH_Y(204), STRETCH_X(320), STRETCH_Y(240));

    AHC_OSDSetColor(uwDispID, bkColor);
    AHC_OSDDrawFillRect(uwDispID, 0, STRETCH_Y(40),  STRETCH_X(320), STRETCH_Y(200));

    AHC_OSDSetPenSize(uwDispID, STRETCH_Y(5));
    AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);
    AHC_OSDDrawFillRect(uwDispID, 0, STRETCH_Y(36),  STRETCH_X(320), STRETCH_Y(40));
    AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);
    AHC_OSDDrawFillRect(uwDispID, 0, STRETCH_Y(200), STRETCH_X(320), STRETCH_Y(204));
#else
    DispBufWidth = AHC_GET_ATTR_OSD_W(uwDispID);
    DispBufHeight = AHC_GET_ATTR_OSD_H(uwDispID);
        
    AHC_OSDDrawFillRect(uwDispID, 0, 0  , DispBufWidth, 36 );
    AHC_OSDDrawFillRect(uwDispID, 0, DispBufHeight-36, DispBufWidth, DispBufHeight);

    AHC_OSDSetColor(uwDispID, bkColor);

    AHC_OSDDrawFillRect(uwDispID, 0, 40,  DispBufWidth, DispBufHeight-40);

    AHC_OSDSetPenSize(uwDispID, 5);
//	printc("DispBufWidth=%d\r\n",DispBufWidth);// 480
//	printc("DispBufHeight=%d\r\n",DispBufHeight);//320

	//上横条
//    AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);// 菜单主界面上下横条颜色
//    AHC_OSDDrawFillRect(uwDispID, 0, 97,  DispBufWidth, 101);
	// 竖条
//      AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);
//    AHC_OSDDrawFillRect(uwDispID, DispBufWidth-76, 0, DispBufWidth-72, DispBufHeight);

//	AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);// 菜单主界面上下横条颜色
 //   AHC_OSDDrawFillRect(uwDispID, 0, 0,  480, 320);

//    AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);
 //   AHC_OSDDrawFillRect(uwDispID, 0, DispBufHeight-40, DispBufWidth, DispBufHeight-36);
#endif  
}

void MenuMainDrawButtons(UINT16 uwDispID)
{
    #if defined(FONT_LARGE)
    // NOP
    #else   // FONT_LARGE   
    GUI_BITMAP      barID   = bmIcon_D_Up;
    UINT32          PageItemNum, ItemInterval;
    
    RECT tempRECT2 = POS_MENU_UP;
    RECT tempRECT3 = POS_MENU_DOWN;

    MenuDraw_GetMainMenuPageItem(uwDispID, barID, &PageItemNum, &ItemInterval);

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {
        tempRECT2.uiLeft += usShiftStringRight;
        tempRECT3.uiLeft += usShiftStringRight;
    }   

    tempRECT2.uiTop     = 40 + (ItemInterval-barID.YSize)/2;
    OSD_Draw_Icon(barID,     tempRECT2, uwDispID);

    barID = bmIcon_D_Down;
    tempRECT3.uiTop     = 40 + ItemInterval*(PageItemNum-1) + (ItemInterval-barID.YSize)/2;
    OSD_Draw_Icon(barID, tempRECT3, uwDispID);
    #endif  // FONT_LARGE
}
#if 0//(MENU_BLUETOOTH_PROGRESS_BAR)
UINT16 wDispID=0;
#endif
extern AHC_BOOL work_mode1;
void MenuDrawItem(UINT16 uwDispID, PSMENUITEM pCurItem, int iItem, UINT32 iStrID, GUI_BITMAP barID, GUI_COLOR clrFont, GUI_COLOR clrBack, AHC_BOOL bCustom)
{
    UINT32          iDefValue = 0;
    PSMENUSTRUCT    pSubMenu;
	AHC_RTC_TIME 	CurRtctime;
    UINT32          PageItemNum, ItemInterval;
	extern UINT16 wSecond;
    UINT32  iDefStrID;
    INT16   wTextAlign  = GUI_TA_HCENTER|GUI_TA_VCENTER;
    RECT    tmpRECT     = RECT_MENU_MAIN_ITEM;
    RECT    tmpRECT1=RECT_MENU_MAIN_ITEM;
    pSubMenu = pCurItem->pSubMenu;

    if(pSubMenu)
        iDefValue = pSubMenu->pfMenuGetDefaultVal(pSubMenu);

    if(pSubMenu && iDefValue < pSubMenu->iNumOfItems)
        iDefStrID = pSubMenu->pItemsList[iDefValue]->uiStringId;

    if (bCustom == 2 /* display at center of screen */)
    {
        tmpRECT.uiLeft = (AHC_GET_ATTR_OSD_W(uwDispID) - barID.XSize) / 2;
    }
    else
    {
        #if defined(FONT_LARGE)
        tmpRECT.uiLeft -= ICON_UP_WIDTH;
        #endif
    }

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {
        tmpRECT.uiLeft += usShiftStringRight;
    }   

    tmpRECT.uiWidth     = barID.XSize;
    
    //MenuDraw_GetMainMenuPageItem(uwDispID, barID, &PageItemNum, &ItemInterval);
	printc("---111--iItem=%d-----\r\n",iItem);
	Menu_Set_Page(iItem);// 设置所选择的界面
	if(iItem == 2 && work_mode1!=99)
		work_mode1 = 1;
	else if(iItem == 1 && work_mode1!=99)
		work_mode1 = 0;//记忆收音机的状态lyj 20190214		
    switch(iItem)
		#if 0
	{
		case 0:
			tmpRECT.uiTop       =170 ;//y
			tmpRECT.uiLeft       =12 ;//x
			break;
		
		case 1:
			tmpRECT.uiTop       =170 ;
			tmpRECT.uiLeft       =205 ;// 
			break;
		case 2:
			tmpRECT.uiTop       =170 ;
			tmpRECT.uiLeft       =301 ;
			break;
		case 3 :
			tmpRECT.uiTop       =232 ;
			tmpRECT.uiLeft       =12 ;
			break;
		case 4 :
			tmpRECT.uiTop       =232 ;
			tmpRECT.uiLeft       =205 ;
			break;
		case 5 :
			tmpRECT.uiTop       =232 ;
			tmpRECT.uiLeft       =301 ;
			break;
		case 6 :
			tmpRECT.uiTop       =294 ;
			tmpRECT.uiLeft       =12 ;
			break;
		case 7 :
			tmpRECT.uiTop       =294 ;
			tmpRECT.uiLeft       =141 ;//? long
			break;
		case 8 :
			tmpRECT.uiTop       =294 ;
			tmpRECT.uiLeft       =269 ;
			break;

		case 20:    
			tmpRECT.uiTop       =-1 ;
			tmpRECT.uiLeft       =0 ;
			break;
		default:
			break;
		
	}
	#else
	{
				case 0:
					//  barID = bmBluetooth_nor;   
					tmpRECT.uiTop       =116 ;//y 117
					tmpRECT.uiLeft       =11 ;//x 12 11 
					break;
				
				case 1:
					//  barID = bmFm_nor ;   
					tmpRECT.uiTop       =116 ;
					tmpRECT.uiLeft       =205 ;// 205 206 
					break;
				case 2:
					//  barID = bmAm_nor;   
					tmpRECT.uiTop       =116 ;
					tmpRECT.uiLeft       =300 ;// 301 302
					break;
				case 3 :
					 // barID = bmVolume_nor;   
					tmpRECT.uiTop       =179 ;
					tmpRECT.uiLeft       =12 ;// 12 13
					break;
				case 4 :
					//  barID = bmLight_nor;   
					tmpRECT.uiTop       =179 ;
					tmpRECT.uiLeft       =205 ;
					break;
				case 5 :
					//  barID = bmAux_nor;   
					tmpRECT.uiTop       =179 ;
					tmpRECT.uiLeft       =300 ;
					break;
				case 6 :
					//  barID = bmUsb_nor;   
					tmpRECT.uiTop       =241 ;
					tmpRECT.uiLeft       =12 ;
					break;
				case 7 :
					 // barID = bmRgb_nor;   
					tmpRECT.uiTop       =240 ;
					tmpRECT.uiLeft       =141 ;//? long
					break;
				case 8 :
					//  barID = bmBright_nor;   
					tmpRECT.uiTop       =241 ;// 294 293
					tmpRECT.uiLeft       =267 ;
					break;

				case 20:    
					tmpRECT.uiTop       =-1 ;
					tmpRECT.uiLeft       =0 ;
					break;
				default:
					break;
		
			}

	#endif
    
//    tmpRECT.uiTop       = 40 + ItemInterval*iItem + (ItemInterval-barID.YSize)/2;
/*
    // Draw Current Value String
    switch(pSubMenu->iMenuId)
    {
        case MENUID_SUB_MENU_VOLUME:
        case MENUID_SUB_MENU_FW_VERSION_INFO:
        case MENUID_SUB_MENU_VMD_SHOT_NUM:
        case MENUID_SUB_MENU_TIMELAPSE_TIME:
        case MENUID_SUB_MENU_CONTRAST:
        case MENUID_SUB_MENU_SATURATION:
        case MENUID_SUB_MENU_SHARPNESS:
        case MENUID_SUB_MENU_GAMMA:
            {
                char    szv[16];

                tmpRECT1.uiLeft     = tmpRECT.uiLeft + barID.XSize;
                tmpRECT1.uiTop      = tmpRECT.uiTop;
                tmpRECT1.uiWidth    = AHC_GET_ATTR_OSD_W(uwDispID) - tmpRECT1.uiLeft;
                tmpRECT1.uiHeight   = tmpRECT.uiHeight - 2;

                if((pSubMenu->iMenuId==MENUID_SUB_MENU_VOLUME)||(pSubMenu->iMenuId==MENUID_SUB_MENU_CONTRAST)||(pSubMenu->iMenuId==MENUID_SUB_MENU_SHARPNESS)
                        ||(pSubMenu->iMenuId==MENUID_SUB_MENU_SATURATION)||(pSubMenu->iMenuId==MENUID_SUB_MENU_GAMMA))
                    sprintf(szv, "%d", (INT32)iDefValue);
                else if(pSubMenu->iMenuId==MENUID_SUB_MENU_VMD_SHOT_NUM)
                    sprintf(szv, "%d %s", m_ulVMDShotNum[iDefValue],  (char*)OSD_GetStringViaID(iDefStrID));
                else if(pSubMenu->iMenuId==MENUID_SUB_MENU_TIMELAPSE_TIME)
                    sprintf(szv, "%d %s", m_ulTimeLapseTime[iDefValue],  (char*)OSD_GetStringViaID(iDefStrID));
                else if(pSubMenu->iMenuId==MENUID_SUB_MENU_FW_VERSION_INFO)
                {
                    sprintf(szv, "%04d", AHC_GetMinorVersion());
                }
           //     ShowOSD_SetLanguage(uwDispID);
          //      OSD_ShowString(uwDispID, szv, NULL, tmpRECT1, clrFont, MENU_BACKGROUND_COLOR, wTextAlign);
            }
        break;

        default:
            if(bCustom == AHC_FALSE)
            {
                tmpRECT1.uiLeft   = tmpRECT.uiLeft +  barID.XSize - 8;
                tmpRECT1.uiTop    = tmpRECT.uiTop;
                tmpRECT1.uiWidth  = AHC_GET_ATTR_OSD_W(uwDispID) - tmpRECT1.uiLeft;
                tmpRECT1.uiHeight = tmpRECT.uiHeight - 2;
         //       OSD_ShowStringPool(uwDispID, iDefStrID, tmpRECT1, clrFont, MENU_BACKGROUND_COLOR, wTextAlign);
            }
        break;
    }
*/
    tmpRECT1.uiLeft   = tmpRECT.uiLeft;
    tmpRECT1.uiTop    = tmpRECT.uiTop + 1 ;
    tmpRECT1.uiWidth  = tmpRECT.uiWidth ;
    tmpRECT1.uiHeight = tmpRECT.uiHeight;
    OSD_Draw_Icon(barID, tmpRECT1, uwDispID);//绘制选择条
//-------------------------------------------------------------

//	printc("--------------uwDispID=%d----------\r\n",uwDispID);
#if(MENU_BLUETOOTH_PROGRESS_BAR)
	wDispID=uwDispID;
#endif
//	while(1)
		
//	AHC_OSDSetFont(uwDispID, &GUI_Font20B_1);
//	AHC_OSDDrawDec( uwDispID,wSecond, 240,40, 2); 
/*
    AHC_OSDSetColor(uwDispID, clrBack);

    tmpRECT1.uiLeft   = tmpRECT.uiLeft   + STR_RECT_OFFSET_X;
    tmpRECT1.uiTop    = tmpRECT.uiTop    + STR_RECT_OFFSET_Y;
    tmpRECT1.uiWidth  = tmpRECT.uiWidth  + STR_RECT_OFFSET_W;
    tmpRECT1.uiHeight = tmpRECT.uiHeight + STR_RECT_OFFSET_H;

    #ifdef SLIDE_STRING
    if(!AHC_IsTVConnectEx() && !AHC_IsHdmiConnect())
    {
        #define SLIDE_FOCUSED_STRING (0)

        int     slide;

        #if (SLIDE_FOCUSED_STRING==1)
        if(memcmp(&barID , &BMICON_MENUBAR_YELLOW,sizeof(GUI_BITMAP)) ==0)
        #else
        if(1)
        #endif
        {
            slide = IsSlideSting(iStrID);

            OSD_ShowStringPoolSlide(uwDispID, iStrID, tmpRECT1, clrFont, clrBack, wTextAlign, &slide);

            if (slide < 0) //[slide] will less than 0 when string length is more then drawing rectangle
            {
                RECT    brc;

                brc.uiLeft   = tmpRECT.uiLeft;
                brc.uiTop    = tmpRECT.uiTop + 1 ;
                brc.uiWidth  = tmpRECT.uiWidth ;
                brc.uiHeight = tmpRECT.uiHeight;
                StartSlideString(uwDispID, iStrID, tmpRECT1, barID, brc, clrFont, clrBack, -slide);
            }
        }
        else
        {
            int idx = GetSlideStringIdx(iStrID);

            if(idx!=0xFF)
                StopSlideStringIdx(idx);

            slide = 0;

            OSD_ShowStringPoolSlide(uwDispID, iStrID, tmpRECT1, clrFont, clrBack, wTextAlign, &slide);

            if(slide < 0)
                OSD_ShowStringPool(uwDispID, iStrID, tmpRECT1, clrFont, clrBack, GUI_TA_LEFT|GUI_TA_VCENTER);
        }
    }
    else
    #endif  // SLIDE_STRING
    {
     //   OSD_ShowStringPool(uwDispID, iStrID, tmpRECT1, clrFont, clrBack, wTextAlign);
    }*/
}

static UINT32 ulCurMenuPageItem;
UINT32 ulGetCurMenuPageItem(void)
{
    return ulCurMenuPageItem;
}

void ulSetCurMenuPageItem(UINT32 value)
{
    ulCurMenuPageItem = value;
}

void MenuDrawItemEx(UINT16 uwDispID, PSMENUSTRUCT pMenu)
{
    UINT32          i, iBegin, iEnd;
    PSMENUITEM      pCurItem;
    PSMENUSTRUCT    pSubMenu;
    UINT32          PageItemNum, ItemInterval;
    GUI_COLOR       TextClr;
    GUI_BITMAP      barID   = BMICON_MENUBAR_WHITE;

    MenuDraw_GetMainMenuPageItem(uwDispID, barID, &PageItemNum, &ItemInterval);//MAIN_MENU_PAGE_ITEM;
    ulSetCurMenuPageItem(PageItemNum);
    
    TextClr     = MENU_TEXT_COLOR;// 字体颜色

    iBegin = ALIGN_DOWN( pMenu->iCurrentPos, PageItemNum );
    iEnd   = MIN( iBegin+PageItemNum, pMenu->iNumOfItems );

    for( i=iBegin; i<iEnd; i++ )
    {
        GUI_COLOR  colorBk  = OSD_COLOR_TRANSPARENT;
        
        barID   = bmLucency;//BMICON_MENUBAR_WHITE;
    	switch(i)// 扫描显示
			{
		//	case 0 :   //   barID = bmBluetooth_UI;     break;
      	    	//	case 1 :    //  barID = bmRADIO_FM;         break;
		//	case 2 :   //   barID = bmRADIO_AM;         break;
		//		barID = bmMENU_MAIN_PAGE;      break;
		}

        if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
        {
            colorBk = OSD_COLOR_TV_BACKGROUND;
        }

#if defined(FONT_LARGE)
        #ifndef CFG_DRAW_FORCE_SMALL_MENU_BAR //may be defined in config_xxx.h
        if( pMenu->iMenuId==MENUID_MAIN_MENU_MEDIA)
        #else
        if (0)
        #endif
        {
            barID = BMICON_MENUBAR_WHITE_XL;

            if( i == pMenu->iCurrentPos )
            {
                barID = BMICON_MENUBAR_YELLOW_XL;
            }
        }
        else
        {
            barID = BMICON_MENUBAR_WHITE;

            if( i == pMenu->iCurrentPos )
            {
                barID = BMICON_MENUBAR_YELLOW;
            }
        }
#else   // FONT_LARGE
        if( i == pMenu->iCurrentPos )
        {// 选定的项目
         //   barID = BMICON_MENUBAR_YELLOW;
         printc("1111111   i=%d\r\n",i);
		switch(i)
			#if 0
			{
			case 0 :      barID =bmBig;     break;
			case 3 :	  barID =bmBig;     break;
      	  //  case 1 :      barID = bmshort;         break;
		//	case 2 :      barID = bmshort;         break;
			case 6: 	 barID = bmmidleBar; break;
			case 7: 	 barID = bmmidleBar; break;
			case 8: 	 barID = bmmidleBar; break;
		  
			default:	  barID = bmshort;         break;
		}
			#else

			{
				case 0 : 	barID = bmBluetooth_nor;  break;
				case 1 :      barID = bmFm_sel;         break;
				case 2: 	 barID = bmAm_sel;  break;
				case 3 : 	 barID = bmVolume_sel;  break;
				case 4:	 barID =bmLight_nor;     break;
	      	   		case 5 :      barID = bmAux_sel;         break;
				case 6: 	 barID = bmUsb_sel; break;
				case 7: 	 barID = bmRgb_sel; break;
				case 8: 	 barID = bmBright_sel; break;
			  
				//default:	  barID = bmshort;         break;
			}

			#endif
	    
        }
#endif  // FONT_LARGE

        pCurItem = pMenu->pItemsList[i];
        pSubMenu = pCurItem->pSubMenu;

        MenuDrawItem(uwDispID, pCurItem,pMenu->iCurrentPos /*i%PageItemNum*/,
                     pCurItem->uiStringId, barID,
                     TextClr, colorBk, pSubMenu->bCustom);
    }
}

void MenuDrawTitle(UINT16 uwDispID, UINT32 uiStrID)
{

    RECT        tmpRECT  = POS_MENU_TITLE;
    GUI_COLOR   bkColor;
	RECT RECTBlue = RECT_Blue_BUTTON_MENU;
    #if (SUPPORT_TOUCH_PANEL)
    RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;

    OSD_Draw_Icon(bmIcon_Return, RECTExit, uwDispID);// 打开触摸时的返回按钮
    #else
    #if defined(_OEM_DRAW_DEV_DV178_)
    tmpRECT.uiLeft = 10;
    #else
    tmpRECT.uiLeft = 5;
    #endif
    #endif
	// bmBuletooth_Title  标题栏图片
    // 换上整张主菜单
    OSD_Draw_Icon(bmMENU_MAIN_PAGE/*bmmainpage3*/, RECTBlue, uwDispID);// 标题栏图片
    Main_Set_Page(MENU_MAIN_FLAG);// liao
    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {
        tmpRECT.uiLeft += usShiftStringRight;       
        bkColor = TV_MENU_BK_COLOR;
    }   
    else
    {
        bkColor = OSD_COLOR_TITLE;// 标题背景颜色
    }   

#ifdef SLIDE_STRING
    {
        int slide = IsSlideSting(uiStrID);

        OSD_ShowStringPoolSlide(uwDispID, uiStrID, tmpRECT, MENU_TEXT_COLOR, bkColor, GUI_TA_LEFT|GUI_TA_VCENTER, &slide);

        if (slide < 0) //[slide] will less than 0 when string length is more then drawing rectangle
        {
            RECT    brc;
            GUI_BITMAP barID;

            memset(&barID, 0, sizeof(GUI_BITMAP));
            brc.uiLeft   = tmpRECT.uiLeft;
            brc.uiTop    = tmpRECT.uiTop + 1 ;
            brc.uiWidth  = tmpRECT.uiWidth ;
            brc.uiHeight = tmpRECT.uiHeight;
            StartSlideString(uwDispID, uiStrID, tmpRECT, barID, brc, MENU_TEXT_COLOR, bkColor, -slide);
        }
    }
#else  //显示标题文字
 //   OSD_ShowStringPool(uwDispID, uiStrID, tmpRECT, MENU_TEXT_COLOR, bkColor, GUI_TA_LEFT|GUI_TA_VCENTER);
#endif

}

void MenuDrawMainPageR1(UINT8 uwDispID, PSMENUSTRUCT pMenu)
{
    if( pMenu->uiStringId != -1 )
    {
        MenuDrawTitle(uwDispID, pMenu->uiStringId);// 标题栏
    }

  //  MenuDraw_UIMode(uwDispID);

 //   MenuDraw_MediaType(uwDispID);

  //  MenuDraw_BatteryStatus(uwDispID);
}

void MenuDrawMainPageR2L(UINT8 uwDispID, PSMENUSTRUCT pMenu)
{
    UINT32          PageItemNum, ItemInterval;
    UINT32  iCurPage, iTotalPage;
    GUI_BITMAP      barID   = BMICON_MENUBAR_WHITE;

    MenuDraw_GetMainMenuPageItem(uwDispID, barID, &PageItemNum, &ItemInterval);//MAIN_MENU_PAGE_ITEM;
    ulSetCurMenuPageItem(PageItemNum);

    // Up/Down Icons
    MenuMainDrawButtons(uwDispID);

    MenuDraw_GetPageInfo(&iCurPage, &iTotalPage, NULL, ulGetCurMenuPageItem());

    MenuDrawPageInfo(uwDispID, iCurPage, iTotalPage);

}

void MenuDrawMainPageR3(UINT8 uwDispID, PSMENUSTRUCT pMenu)
{
    #if (SUPPORT_TOUCH_PANEL)
    RECT RECTLeft   = RECT_TOUCH_BUTTON_MENU_LEFT;
    RECT RECTRight  = RECT_TOUCH_BUTTON_MENU_RIGHT;

    OSD_Draw_Icon(bmIcon_D_Left,  RECTLeft  ,uwDispID);
    OSD_Draw_Icon(bmIcon_D_Right, RECTRight ,uwDispID);
    #endif

    MenuMainDrawCatagory(uwDispID, GetCatagoryMenuID(pMenu));
}

void MenuDrawMainPageEx(UINT16 uwDispID, PSMENUSTRUCT pMenu)
{

#ifdef SLIDE_STRING
    StopSlideString();
#endif
//	RECT RECTBlue = RECT_Blue_BUTTON_MENU;
//	 OSD_Draw_Icon(bmGrounding, RECTBlue, uwDispID);// neijin
    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {
        MenuMainDrawBackCurtain(uwDispID, TV_MENU_BK_COLOR);
    }
    else
    {
        OSDDraw_ClearOvlDrawBuffer(uwDispID);
     //   MenuMainDrawBackCurtain(uwDispID, OSD_COLOR_BLUE);// 背景
    }
   
    // Draw title and Icons (UI Mode, Card State, Battery State)
   MenuDrawMainPageR1(uwDispID, pMenu);
    // Vertical scoll bar
//    MenuDrawMainPageR2L(uwDispID, pMenu);// 滚动条

    MenuDrawItemEx(uwDispID, pMenu);

//    MenuDrawMainPageR3(uwDispID, pMenu);
}

void MenuDrawMainPage(PSMENUSTRUCT pMenu)
{
    UINT8 bID0 = 0, bID1 = 0;
    UINT16 dummy;
    UINT32 addr;
    UINT16 color;

#ifdef FLM_GPIO_NUM
    AHC_OSDSetActive(0, 0);
    AHC_OS_SleepMs(100);
#endif

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    AHC_OSDGetBufferAttr(bID0, &dummy, &dummy, &color, &addr);
 

        OSDDraw_EnableSemiTransparent(bID0, AHC_FALSE);// liao 20180316    AHC_TRUE
        OSDDraw_EnableSemiTransparent(bID1, AHC_FALSE);


    MenuDrawMainPageEx(bID0, pMenu);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}

void MenuDrawChangeItem(PSMENUSTRUCT pMenu, UINT32 uiCurrItem, UINT32 uiPrevItem, UINT32 uiPreSelected)
{
    PSMENUITEM      pPrevItem;
    PSMENUITEM      pCurItem;
    UINT32          PageItemNum;
    GUI_COLOR       TextClr;
	UINT32     i;
	GUI_BITMAP      barID   = BMICON_MENUBAR_WHITE;
    PageItemNum = ulGetCurMenuPageItem();// 页数
    TextClr     = MENU_TEXT_COLOR;

	printc("~~~MenuDrawChangeItem~~uiCurrItem = %d~~~~uiPrevItem = %d~~~PageItemNum = %d~~~\r\n",uiCurrItem,uiPrevItem,PageItemNum);
	#if 0
    if( !IS_SAME_PAGE( uiCurrItem, uiPrevItem, PageItemNum ) )
    {
    	printc("~~~IS_SAME_PAGE~~uiCurrItem = %d~~~~uiPrevItem = %d~~~PageItemNum = %d~~~\r\n",uiCurrItem,uiPrevItem,PageItemNum);
        MenuDrawMainPage(pMenu);
    }
    else
		#endif
    {
        UINT8           bID0    = 0;
        UINT8           bID1    = 0;
        PSMENUSTRUCT    pSubMenu;
        GUI_COLOR       clrFill = OSD_COLOR_TRANSPARENT;

        UINT32  iCurPage, iTotalPage;
		RECT    tmpRECT = {0,0,0,0};

        if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
        {
            clrFill = OSD_COLOR_TV_BACKGROUND;
        }

        OSDDraw_EnterMenuDrawing(&bID0, &bID1);

        //Region R2-L

        MenuDraw_GetPageInfo(&iCurPage, &iTotalPage, NULL, PageItemNum);
      //  MenuDrawPageInfo(bID0, iCurPage, iTotalPage);// 左右转时显示页数

        //Region R2-R
        pPrevItem = pMenu->pItemsList[uiPrevItem];
        pSubMenu  = pPrevItem->pSubMenu;
		printc("---------uiCurrItem%PageItemNum=%d-------\r\n",uiCurrItem%PageItemNum);
        //Draw Previous
        #if defined(FONT_LARGE)
        {
            const GUI_BITMAP    *pBar;

            #ifndef CFG_DRAW_FORCE_SMALL_MENU_BAR //may be defined in config_xxx.h
            if( pMenu->iMenuId==MENUID_MAIN_MENU_MEDIA)
            #else
            if (0)
            #endif
            {
                pBar = &BMICON_MENUBAR_WHITE_XL;
            }
            else
            {
                pBar = &BMICON_MENUBAR_WHITE;
            }
            MenuDrawItem(   bID0, pPrevItem, uiPrevItem%PageItemNum,
                            pPrevItem->uiStringId,
                            *pBar,
                            TextClr, clrFill, pSubMenu->bCustom);
        }
        #else   // FONT_LARGE
		 
       // MenuDrawItem(bID0, pPrevItem, 20/*uiPrevItem%PageItemNum*/, pPrevItem->uiStringId,
        //             /*bmMENU_MAIN_PAGE*/bmmainpage3, TextClr, clrFill, pSubMenu->bCustom); 

		   	switch(uiPrevItem)
				#if 0
			{
				case 0:
					  barID = bmBluetooth_nor;   
					tmpRECT.uiTop       =170 ;//y
					tmpRECT.uiLeft       =12 ;//x 12 11 
					break;
				
				case 1:
					  barID = bmFm_nor ;   
					tmpRECT.uiTop       =170 ;
					tmpRECT.uiLeft       =205 ;// 205 206 
					break;
				case 2:
					  barID = bmAm_nor;   
					tmpRECT.uiTop       =170 ;
					tmpRECT.uiLeft       =301 ;// 301 302
					break;
				case 3 :
					  barID = bmVolume_nor;   
					tmpRECT.uiTop       =232 ;
					tmpRECT.uiLeft       =12 ;// 12 13
					break;
				case 4 :
					  barID = bmLight_nor;   
					tmpRECT.uiTop       =232 ;
					tmpRECT.uiLeft       =206 ;
					break;
				case 5 :
					  barID = bmAux_nor;   
					tmpRECT.uiTop       =232 ;
					tmpRECT.uiLeft       =302 ;
					break;
				case 6 :
					  barID = bmUsb_nor;   
					tmpRECT.uiTop       =294 ;
					tmpRECT.uiLeft       =12 ;
					break;
				case 7 :
					  barID = bmRgb_nor;   
					tmpRECT.uiTop       =294 ;
					tmpRECT.uiLeft       =141 ;//? long
					break;
				case 8 :
					  barID = bmBright_nor;   
					tmpRECT.uiTop       =294 ;// 294 293
					tmpRECT.uiLeft       =269 ;
					break;

				case 20:    
					tmpRECT.uiTop       =-1 ;
					tmpRECT.uiLeft       =0 ;
					break;
				default:
					break;
		
			}
				#else
				{
				case 0:
					  barID = bmBluetooth_sel;   
					tmpRECT.uiTop       =116 ;//y 117
					tmpRECT.uiLeft       =11 ;//x 12 11 
					break;
				
				case 1:
					  barID = bmFm_nor ;   
					tmpRECT.uiTop       =116 ;
					tmpRECT.uiLeft       =205 ;// 205 206 
					break;
				case 2:
					  barID = bmAm_nor;   
					tmpRECT.uiTop       =116 ;
					tmpRECT.uiLeft       =300 ;// 301 302
					break;
				case 3 :
					  barID = bmVolume_nor;   
					tmpRECT.uiTop       =179 ;
					tmpRECT.uiLeft       =12 ;// 12 13
					break;
				case 4 :
					  barID = bmLight_sel;   
					tmpRECT.uiTop       =179 ;
					tmpRECT.uiLeft       =205 ;
					break;
				case 5 :
					  barID = bmAux_nor;   
					tmpRECT.uiTop       =179 ;
					tmpRECT.uiLeft       =300 ;
					break;
				case 6 :
					  barID = bmUsb_nor;   
					tmpRECT.uiTop       =241 ;
					tmpRECT.uiLeft       =12 ;
					break;
				case 7 :
					  barID = bmRgb_nor;   
					tmpRECT.uiTop       =240 ;
					tmpRECT.uiLeft       =141 ;//? long
					break;
				case 8 :
					  barID = bmBright_nor;   
					tmpRECT.uiTop       =241 ;// 294 293
					tmpRECT.uiLeft       =267 ;
					break;

				case 20:    
					tmpRECT.uiTop       =-1 ;
					tmpRECT.uiLeft       =0 ;
					break;
				default:
					break;
		
			}

				#endif
			tmpRECT.uiTop = tmpRECT.uiTop + 1 ;
		OSD_Draw_Icon(barID, tmpRECT, bID0);

        #endif  // FONT_LARGE

        pCurItem = pMenu->pItemsList[uiCurrItem];
        pSubMenu = pCurItem->pSubMenu;

		pMenu->iSelected = uiCurrItem; // lyj

        //Draw current
        #if defined(FONT_LARGE)
        {
            const GUI_BITMAP    *pBar;

            #ifndef CFG_DRAW_FORCE_SMALL_MENU_BAR //may be defined in config_xxx.h
            if( pMenu->iMenuId==MENUID_MAIN_MENU_MEDIA)
            #else
            if (0)
            #endif
            {
                pBar = &BMICON_MENUBAR_YELLOW_XL;
            }
            else
            {
                pBar = &BMICON_MENUBAR_YELLOW;
            }
            MenuDrawItem(   bID0, pCurItem, uiCurrItem%PageItemNum, pCurItem->uiStringId,
                            *pBar, TextClr, clrFill, pSubMenu->bCustom);
        }
        #else   // FONT_LARGE
		//if(pMenu->iCurrentPos )
        {// 选定的项目
         //   barID = BMICON_MENUBAR_YELLOW;
         printc("22222222   i=%d\r\n",pMenu->iCurrentPos);

			switch(pMenu->iCurrentPos)
			#if 0
			{
				case 0 :
				case 3 :
							  barID =bmBig;     break;
	      	  //  case 1 :      barID = bmshort;         break;
			//	case 2 :      barID = bmshort;         break;
			case 6: 	 barID = bmmidleBar; break;
			case 7: 	 barID = bmmidleBar; break;
			case 8: 	 barID = bmmidleBar; break;
			  
				default:	  barID = bmshort;         break;
			}
			#else
			{
				case 0 : 	barID = bmBluetooth_nor;  break;
				case 1 :      barID = bmFm_sel;         break;
				case 2: 	 barID = bmAm_sel;  break;
				case 3 : 	 barID = bmVolume_sel;  break;
				case 4:	 barID =bmLight_nor;     break;
	      	   		case 5 :      barID = bmAux_sel;         break;
				case 6: 	 barID = bmUsb_sel; break;
				case 7: 	 barID = bmRgb_sel; break;
				case 8: 	 barID = bmBright_sel; break;
			  
				//default:	  barID = bmshort;         break;
			}

			#endif
	    
        }
        MenuDrawItem(bID0, pCurItem, /*uiCurrItem%PageItemNum*/pMenu->iCurrentPos, pCurItem->uiStringId,
                     barID, TextClr, clrFill, pSubMenu->bCustom);
        #endif

        OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    }
}

#if 0
void ________SubMenu_Function_________(){ruturn;} //dummy
#endif

UINT8 GetSubMenuType(void)
{
    return m_ubSubPageType;
}

void SetSubMenuType(UINT8 ubType)
{
    m_ubSubPageType = ubType;
}

void GetSubItemRect(UINT16 uwDispID, int i, int iItems, RECT* pRc)
{
    #define BAR_GAP     (24)    // Change for SELF TIMER ICON, it paints over left-side button
    UINT16  SubItemTotalWidth = 0;
    
    SubItemTotalWidth = AHC_GET_ATTR_OSD_W(uwDispID);

    if( iItems <= 2 )
    {
        switch(i)
        {
            case 0: {SET_SUBRECT(pRc, (SubItemTotalWidth - pRc->uiWidth * 2 - BAR_GAP) / 2, 105, pRc->uiWidth, pRc->uiHeight); break;}
            case 1: {SET_SUBRECT(pRc, (SubItemTotalWidth + BAR_GAP) / 2,                     105, pRc->uiWidth, pRc->uiHeight); break;}
            default:{SET_SUBRECT(pRc, (SubItemTotalWidth - pRc->uiWidth * 2 - BAR_GAP) / 2, 105, pRc->uiWidth, pRc->uiHeight); break;}
        }
    }
    else if( iItems <= 4 )
    {
        switch(i)
        {
            case 0: {SET_SUBRECT(pRc, (SubItemTotalWidth - pRc->uiWidth * 2 - BAR_GAP) / 2,  80,  pRc->uiWidth, pRc->uiHeight); break;}
            case 1: {SET_SUBRECT(pRc, (SubItemTotalWidth + BAR_GAP) / 2,                      80,  pRc->uiWidth, pRc->uiHeight); break;}
            case 2: {SET_SUBRECT(pRc, (SubItemTotalWidth - pRc->uiWidth * 2 - BAR_GAP) / 2,  140, pRc->uiWidth, pRc->uiHeight); break;}
            case 3: {SET_SUBRECT(pRc, (SubItemTotalWidth + BAR_GAP) / 2,                      140, pRc->uiWidth, pRc->uiHeight); break;}
            default:{SET_SUBRECT(pRc, (SubItemTotalWidth - pRc->uiWidth * 2 - BAR_GAP) / 2,  80,  pRc->uiWidth, pRc->uiHeight); break;}
        }
    }
    else
    {
        if(AHC_IsHdmiConnect())
        {
            if(MenuSettingConfig()->uiHDMIOutput == HDMI_OUTPUT_1080P)
            {
                switch(i)
                {
                    case 0: {SET_SUBRECT(pRc, 90,  52 , pRc->uiWidth, pRc->uiHeight);break;}
                    case 1: {SET_SUBRECT(pRc, 260, 52 , pRc->uiWidth, pRc->uiHeight);break;}
                    case 2: {SET_SUBRECT(pRc, 90,  115, pRc->uiWidth, pRc->uiHeight);break;}
                    case 3: {SET_SUBRECT(pRc, 260, 115, pRc->uiWidth, pRc->uiHeight);break;}
                    case 4: {SET_SUBRECT(pRc, 90,  180, pRc->uiWidth, pRc->uiHeight);break;}
                    case 5: {SET_SUBRECT(pRc, 260, 180, pRc->uiWidth, pRc->uiHeight);break;}
                    default:{SET_SUBRECT(pRc, 260, 170, pRc->uiWidth, pRc->uiHeight);break;}
                }                   
            }
            else
            {
                switch(i)
                {
                    case 0: {SET_SUBRECT(pRc, 90,  52 , pRc->uiWidth, pRc->uiHeight);break;}
                    case 1: {SET_SUBRECT(pRc, 260, 52 , pRc->uiWidth, pRc->uiHeight);break;}
                    case 2: {SET_SUBRECT(pRc, 90,  115, pRc->uiWidth, pRc->uiHeight);break;}
                    case 3: {SET_SUBRECT(pRc, 260, 115, pRc->uiWidth, pRc->uiHeight);break;}
                    case 4: {SET_SUBRECT(pRc, 90,  180, pRc->uiWidth, pRc->uiHeight);break;}
                    case 5: {SET_SUBRECT(pRc, 260, 180, pRc->uiWidth, pRc->uiHeight);break;}
                    default:{SET_SUBRECT(pRc, 260, 170, pRc->uiWidth, pRc->uiHeight);break;}
                }   
            }   
        }   
        else if(AHC_IsTVConnectEx())
        {
            int x0, x1;
            
            x0 =  (SubItemTotalWidth - pRc->uiWidth * 2 - BAR_GAP) / 2 + (28); //STRETCH_X(70);
            x1 =  (SubItemTotalWidth + BAR_GAP) / 2 + (28);//   STRETCH_X(x0 + 28 /* Icon Width */ + pRc->uiWidth);
            
            switch(i)
            {
                case 0: {SET_SUBRECT(pRc, x0, (50) , pRc->uiWidth, pRc->uiHeight);break;}
                case 1: {SET_SUBRECT(pRc, x1, (50) , pRc->uiWidth, pRc->uiHeight);break;}
                case 2: {SET_SUBRECT(pRc, x0, (103), pRc->uiWidth, pRc->uiHeight);break;}
                case 3: {SET_SUBRECT(pRc, x1, (103), pRc->uiWidth, pRc->uiHeight);break;}
                case 4: {SET_SUBRECT(pRc, x0, (156), pRc->uiWidth, pRc->uiHeight);break;}
                case 5: {SET_SUBRECT(pRc, x1, (156), pRc->uiWidth, pRc->uiHeight);break;}
                case 6: {SET_SUBRECT(pRc, x0, (208), pRc->uiWidth, pRc->uiHeight);break;}
                case 7: {SET_SUBRECT(pRc, x1, (208), pRc->uiWidth, pRc->uiHeight);break;}
                default:{SET_SUBRECT(pRc, x0, (210), pRc->uiWidth, pRc->uiHeight);break;}
            }

        }
        else
        {           
            int x0, x1;
            
            x0 =  (SubItemTotalWidth - pRc->uiWidth * 2 - BAR_GAP) / 2 + (28); //STRETCH_X(70);
            x1 =  (SubItemTotalWidth + BAR_GAP) / 2 + (28);//   STRETCH_X(x0 + 28 /* Icon Width */ + pRc->uiWidth);
            
            switch(i)
            {
                case 0: {SET_SUBRECT(pRc, x0, (50) , pRc->uiWidth, pRc->uiHeight);break;}
                case 1: {SET_SUBRECT(pRc, x1, (50) , pRc->uiWidth, pRc->uiHeight);break;}
                case 2: {SET_SUBRECT(pRc, x0, (103), pRc->uiWidth, pRc->uiHeight);break;}
                case 3: {SET_SUBRECT(pRc, x1, (103), pRc->uiWidth, pRc->uiHeight);break;}
                case 4: {SET_SUBRECT(pRc, x0, (156), pRc->uiWidth, pRc->uiHeight);break;}
                case 5: {SET_SUBRECT(pRc, x1, (156), pRc->uiWidth, pRc->uiHeight);break;}
                case 6: {SET_SUBRECT(pRc, x0, (208), pRc->uiWidth, pRc->uiHeight);break;}
                case 7: {SET_SUBRECT(pRc, x1, (208), pRc->uiWidth, pRc->uiHeight);break;}
                default:{SET_SUBRECT(pRc, x0, (210), pRc->uiWidth, pRc->uiHeight);break;}
            }
        }   
    }
}
#if 0 //(MENU_BLUETOOTH_PROGRESS_BAR)
extern UINT16 wDispID;
//extern UINT16 wSecond;
#endif
void DrawSubItem(UINT16 uwDispID, int iItem, int iTotalItems, UINT8 uLang, UINT32 iStrID, GUI_BITMAP IconID, UINT32 barId, GUI_COLOR clrFont, GUI_COLOR clrBack, GUI_COLOR clrSelect)
{
    #if defined(FONT_LARGE)

    RECT    rc = POS_MENU_ITEM;
    RECT    tmpRECT;

    if(AHC_TRUE == AHC_Charger_GetStatus())
    {
        AHC_OSDSetColor(uwDispID, MENU_TEXT_COLOR);

        if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
            AHC_OSDSetBkColor(uwDispID, OSD_COLOR_TV_BACKGROUND);
        else
            AHC_OSDSetBkColor(uwDispID, MENU_BACKGROUND_COLOR);
    }

    MenuDrawPageInfo(uwDispID, iItem / SUB_MENU_PAGE_ITEM + 1,
                     (iTotalItems + SUB_MENU_PAGE_ITEM - 1) / SUB_MENU_PAGE_ITEM);

    rc.uiWidth   = vBarSet[0].bar[BAR_STATE_WHT]->XSize;
    rc.uiLeft    = (AHC_GET_ATTR_OSD_W(uwDispID) - rc.uiWidth) >> 1;
    rc.uiTop    += ((rc.uiHeight+OSD_MENU_ITEM_INTERVAL) * (iItem % SUB_MENU_PAGE_ITEM));

    if (IconID != NULL && IconID != &bmIcon_Dummy)
    {
        int     offset;

        if((iTotalItems == SUB_MENU_PAGE_ITEM-2) && ( IconID->XSize >= 50 ))
            rc.uiLeft = rc.uiLeft + 10;

        offset = (IconID->XSize >= 50)?(50):(32);

        AHC_OSDDrawBitmap(uwDispID, IconID, rc.uiLeft - offset, rc .uiTop);
        rc.uiLeft += (IconID->XSize - offset);
    }

    if( clrSelect != 0 )
    {
        AHC_OSDSetColor(uwDispID, clrSelect);
        AHC_OSDDrawFillRoundedRect(uwDispID, rc.uiLeft+2, rc.uiTop+rc.uiHeight-7, rc.uiLeft+rc.uiWidth-2, rc.uiTop+rc.uiHeight-2, RECT_ROUND_RADIUS );
    }

    OSD_Draw_Icon(BAR(barId), rc, uwDispID);

    tmpRECT.uiLeft   = rc.uiLeft + 2;
    tmpRECT.uiTop    = rc.uiTop;
    tmpRECT.uiWidth  = PBAR(barId)->XSize;
    tmpRECT.uiHeight = PBAR(barId)->YSize;

    ShowOSD_SetLanguageEx(uwDispID, uLang);
    OSD_ShowString( uwDispID, (char*)OSD_GetStringViaID(iStrID), NULL,
                    tmpRECT, clrFont, clrBack,
                    GUI_TA_HCENTER|GUI_TA_VCENTER);
//===================================================================
    #else // defined(FONT_LARGE)
	RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;
	RECT tmpRECT1 = {415,   55,   60, 35};
	RECT RECTExit1;
	char    szv[16];
	GUI_COLOR clrFont1;
	
	#if 0// (MENU_BLUETOOTH_PROGRESS_BAR)// 进度条与读秒
	//------------------------------------------
	UINT16          BarPos=20;
	
	 RECT            rc = RECT_MENU_ADJUST_BAR_ITEM;
	
    AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);
    AHC_OSDSetPenSize(uwDispID, 1);

    AHC_OSDDrawLine(uwDispID, rc.uiLeft, rc.uiLeft+rc.uiWidth, rc.uiTop, rc.uiTop);
    AHC_OSDDrawLine(uwDispID, rc.uiLeft, rc.uiLeft+rc.uiWidth, rc.uiTop+rc.uiHeight, rc.uiTop+rc.uiHeight);
    AHC_OSDDrawLine(uwDispID, rc.uiLeft, rc.uiLeft, rc.uiTop, rc.uiTop+rc.uiHeight);
    AHC_OSDDrawLine(uwDispID, rc.uiLeft+rc.uiWidth, rc.uiLeft+rc.uiWidth, rc.uiTop, rc.uiTop+rc.uiHeight);

    //Draw Progress Bar
    AHC_OSDSetColor(uwDispID, OSD_COLOR_ORANGE);//选定
    AHC_OSDDrawFillRect(uwDispID, rc.uiLeft+1, rc.uiTop+1, (rc.uiLeft+1)+BarPos, rc.uiTop+rc.uiHeight-1);

    //Draw Progress Bar
    AHC_OSDSetColor(uwDispID, OSD_COLOR_TRANSPARENT);//未选定

    AHC_OSDDrawFillRect(uwDispID, (rc.uiLeft+1)+BarPos, rc.uiTop+1, rc.uiLeft+rc.uiWidth-1, rc.uiTop+rc.uiHeight-1);
	OSD_Draw_IconXY(uwDispID, bmUsb_Yuandian, 70,  rc.uiTop+3);
	//------------------------------------------

#endif
	#if(MENU_BLUETOOTH_PROGRESS_BAR)
	wDispID=uwDispID;
	#endif
        if(Menu_Get_Page()==7) // long
        {
        	//printc("-------iItem=%d--------------\r\n",iItem);
		if(iItem==7)
        	{
        		switch(uLang)
        		{
 				#if 0
				case 0:    RECTExit.uiLeft = 20;
						RECTExit.uiTop = 125;  
						break;
				case 1:    RECTExit.uiLeft = 271;
						RECTExit.uiTop = 125;  
						 break;
				case 2:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 217;    
						 break;
				case 3:    RECTExit.uiLeft = 146;
						RECTExit.uiTop = 217;  
						  break;
				case 4:    RECTExit.uiLeft = 271;
						RECTExit.uiTop = 217;   
						break;
				#else

					case 0:    RECTExit.uiLeft = 82;
						RECTExit.uiTop = 127;  
							break;
					case 1:    RECTExit.uiLeft = 22;
							RECTExit.uiTop = 221;  
							 break;
					case 2:    RECTExit.uiLeft = 150;
							RECTExit.uiTop = 220;    
							 break;
					case 3:    RECTExit.uiLeft = 280;
							RECTExit.uiTop = 220;  
							  break;
				

				#endif

			}
			OSD_Draw_Icon(IconID, RECTExit, uwDispID);

        	}else
        		{
			 switch(iStrID)
	            	{
	            		//case 5:
				case 0:    RECTExit.uiLeft = 82;
						RECTExit.uiTop = 127;  
						//printc("~~~~~~~~2222222333~~~\r\n");
						app_flag = 0;
						MenuSettingConfig()->uiRGBstatus = 0;
						White_light_bar_off(); break;
				case 1:    RECTExit.uiLeft = 22;
						RECTExit.uiTop = 221;  
						app_flag =2;
						MenuSettingConfig()->uiRGBstatus = 1;
						//printc("~~~~~~~~222222211~~~\r\n"); 

						 break;
				case 2:    RECTExit.uiLeft = 150;
						RECTExit.uiTop = 220;    
						// printc("~~~~~~~~2222222555~~~\r\n");
						// Color_chang(1,12,35,22);

						break;
				case 3:    RECTExit.uiLeft = 280;
						RECTExit.uiTop = 220;  
						app_flag = 2;
						MenuSettingConfig()->uiRGBstatus = 2;
						 //printc("~~~~~~~~2222222666~~~\r\n"); 
						 break;

			}
			RECTExit1.uiLeft   = RECTExit.uiLeft;
			 RECTExit1.uiTop    = RECTExit.uiTop;
			 RECTExit1.uiWidth  = RECTExit.uiWidth ;
			 RECTExit1.uiHeight = RECTExit.uiHeight;
		        OSD_Draw_Icon(IconID, RECTExit1, uwDispID);
	// 显示电压
	//sprintf(szv, "%dV", (INT32)wSecond);
	//AHC_OSDSetFont(uwDispID, &GUI_Font20_1);
	//OSD_ShowString( uwDispID,szv, NULL, tmpRECT1, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);


#if 0
		if(iStrID == 2) // lyj
		{
			flag_twinkle = 1;
		}
		else
			flag_twinkle = 0;
#endif

		if(iStrID == 1)
			flag_color = 1;
		else {
			if(iStrID !=2)
			flag_color = 0;
			}

		if(iStrID == 3)
			SelfAllFlag.SlowBecome =1;
		else{
			if(iStrID !=2)
			SelfAllFlag.SlowBecome =0;
			}
        	}


		
        }
	else if(Menu_Get_Page()==4) //5
		{
			//MMP_ULONG light_bar = 101;
        	printc("-------iItem=%d--------------\r\n",iItem);
		if(iItem==7)// 7
        	{
			switch(uLang)
        		{
				case 0:    RECTExit.uiLeft = 33;
						RECTExit.uiTop = 124;
						if(lightBarFlag_1)
							OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						else
							OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						
						break;
				case 1:    RECTExit.uiLeft = 160;
						RECTExit.uiTop = 124; 
						if(lightBarFlag_2)
							OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						else
						OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;
				case 2:    RECTExit.uiLeft = 286;
						RECTExit.uiTop = 124; 
						if(lightBarFlag_3)
							OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						else
						OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;
						
				case 3:    RECTExit.uiLeft = 33;
						RECTExit.uiTop = 217;  
						if(lightBarFlag_4)
							OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						else
						OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;
				case 4:    RECTExit.uiLeft = 160;
						RECTExit.uiTop = 217;  
						if(lightBarFlag_5)
							OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						else
						OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;
				case 5:    RECTExit.uiLeft = 286;
						RECTExit.uiTop = 217;  
						if(lightBarFlag_6)
							OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						else
						OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;

			}
			
			//OSD_Draw_Icon(IconID, RECTExit, uwDispID);
        	}else
        		{
			 switch(iStrID)
	            	{
	            		
				case 0:    RECTExit.uiLeft = 33;
						RECTExit.uiTop = 124;    
						//light_bar = 95;
						//OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;
				case 1:    RECTExit.uiLeft = 160;
						RECTExit.uiTop = 124;    
						//light_bar = 76;
						//OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;
				case 2:    RECTExit.uiLeft = 286;
						RECTExit.uiTop = 124;   //light_bar = 57;
						//OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;
						
				case 3:    RECTExit.uiLeft = 33;
						RECTExit.uiTop = 217;   //light_bar = 38;
						//OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;
				case 4:    RECTExit.uiLeft = 160;
						RECTExit.uiTop = 217;  //light_bar = 19;
						//OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;
				case 5:    RECTExit.uiLeft = 286;
						RECTExit.uiTop = 217;   //light_bar = 0; 
						//OSD_Draw_Icon(IconID, RECTExit, uwDispID);
						break;
			}
		  // RECTExit1.uiLeft   = RECTExit.uiLeft;
			// RECTExit1.uiTop    = RECTExit.uiTop;
			// RECTExit1.uiWidth  = RECTExit.uiWidth ;
			// RECTExit1.uiHeight = RECTExit.uiHeight;
		
		       OSD_Draw_Icon(IconID, RECTExit, uwDispID);
        	}
		
        }

	#if 1
	else if(/*Menu_Get_Page()==1 || */Menu_Get_Page()==2) // lyj 20180521
		{
		//printc("~~2019~~lyj666666~iStrID = %d~~iItem = %d~~~~~uLang == %d~~~\r\n",iStrID,iItem,uLang);

		if(iItem==7)// 7
        	{
			 switch(uLang)
			 {
				case 0:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 202;     break;
				case 1:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 240;     break;
				case 2:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 272;     break;
				case 3:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 170;     break;
			}
			OSD_Draw_Icon(IconID, RECTExit, uwDispID);
        	}else
        	{
        		//printc("~~2019~~lyj77777~iStrID = %d~~iItem = %d~\r\n",iStrID,iItem);
			 switch(iStrID)
	            	{
	            		
				case 0:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 202;     break;
				case 1:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 240;     break;
				case 2:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 272;     break;
				case 3:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 170;     break;
			}
		   	 RECTExit1.uiLeft   = RECTExit.uiLeft;
			 RECTExit1.uiTop    = RECTExit.uiTop;
			 RECTExit1.uiWidth  = RECTExit.uiWidth ;
			 RECTExit1.uiHeight = RECTExit.uiHeight;
		        OSD_Draw_Icon(IconID, RECTExit1, uwDispID);
        	}


	}
	else if(Menu_Get_Page()==1)
	{
		
		if(iItem==7)// 7
        	{
			 switch(uLang)
			 {
				case 0:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 202;     break;
				case 1:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 240;     break;
				case 2:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 272;     break;
				case 3:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 170;     break;
						#if 1
				case 4:    RECTExit.uiLeft = 100;
						RECTExit.uiTop = 130;     break;
				case 5:    RECTExit.uiLeft = 360;
						RECTExit.uiTop = 130;     break;

						#endif
			}
			OSD_Draw_Icon(IconID, RECTExit, uwDispID);
        	}else
        	{
        		//printc("~~2019~~lyj77777~iStrID = %d~~iItem = %d~\r\n",iStrID,iItem);
			 switch(iStrID)
	            	{
	            		
				case 0:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 202;     break;
				case 1:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 240;     break;
				case 2:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 272;     break;
				case 3:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 170;     break;

							#if 1
				case 4:    RECTExit.uiLeft = 100;
						RECTExit.uiTop = 130;     break;
				case 5:    RECTExit.uiLeft = 360;
						RECTExit.uiTop = 130;     break;

						#endif
			}
		   	 RECTExit1.uiLeft   = RECTExit.uiLeft;
			 RECTExit1.uiTop    = RECTExit.uiTop;
			 RECTExit1.uiWidth  = RECTExit.uiWidth ;
			 RECTExit1.uiHeight = RECTExit.uiHeight;
		        OSD_Draw_Icon(IconID, RECTExit1, uwDispID);

        		}



	}


	

	#endif

	
	else if(Menu_Get_Page()==3)
		{
		printc("-------iItem=%d---clrSelect=%d-----------\r\n",iItem,clrSelect);
		printc("----------------iStrID=%d---------------\r\n",iStrID);
		switch(iItem)//(iStrID)
	            	{

					//printc("~~~~~lyj~~~~iItem = %d~~~~~\r\n",iItem);
	            		
				case 0:  
						printc("~~~~~lyj~~~111~iItem = %d~~~~~\r\n",iItem);
						sprintf(szv, "FRONT     ");
						RECTExit.uiLeft = 33;
						RECTExit.uiTop = 140;  
						if(iStrID==0)
							clrFont1= OSD_COLOR_WHITE;
						else
						 clrFont1= OSD_COLOR_GRAY; 
						 break;
				case 1:    sprintf(szv, "REAR       ");
						RECTExit.uiLeft = 33;
						RECTExit.uiTop = 175;
						if(iStrID==1)
							clrFont1= OSD_COLOR_WHITE;
						else
						 clrFont1= OSD_COLOR_GRAY; 
						 break;
				case 2:    sprintf(szv, "OUTPUT1");
						RECTExit.uiLeft = 33;
						RECTExit.uiTop = 215; 
						if(iStrID==2)
							clrFont1= OSD_COLOR_WHITE;
						else
						 clrFont1= OSD_COLOR_GRAY; 
						
			}
			
		  
	    RECTExit1.uiLeft     =RECTExit.uiLeft;
	    RECTExit1.uiTop     = RECTExit.uiTop; 
	    RECTExit1.uiWidth  = 100;
	    RECTExit1.uiHeight = 50;
		AHC_OSDSetFont(uwDispID, &GUI_Font24B_1); 
	OSD_ShowString( uwDispID,szv, NULL, RECTExit1, clrFont1, clrBack,GUI_TA_HCENTER|GUI_TA_VCENTER);
        	
		
        }
	
#if 0
    RECT    rc, tmpRECT;
    UINT32  iMenuId = GetCurrentMenu()->iMenuId;

    if(AHC_TRUE == AHC_Charger_GetStatus())
    {
        AHC_OSDSetColor(uwDispID, MENU_TEXT_COLOR);

        if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
            AHC_OSDSetBkColor(uwDispID, OSD_COLOR_TV_BACKGROUND);
        else
            AHC_OSDSetBkColor(uwDispID, MENU_BACKGROUND_COLOR);
    }

    rc.uiWidth  = BAR(barId).XSize;
    rc.uiHeight = BAR(barId).YSize;
    GetSubItemRect(uwDispID, iItem%SUB_MENU_PAGE_ITEM, iTotalItems, &rc );

    OSD_Draw_Icon(BAR(barId), rc, uwDispID);

    if(0/*AHC_IsTVConnectEx() || AHC_IsHdmiConnect()*/)
    {
        tmpRECT.uiLeft   = rc.uiLeft            + STR_RECT_OFFSET_X;
        tmpRECT.uiTop    = rc.uiTop             + 3;
        tmpRECT.uiWidth  = PBAR(barId)->XSize   + STR_RECT_OFFSET_W;
        tmpRECT.uiHeight = PBAR(barId)->YSize   + (-13);
    }
    else
    {
        tmpRECT.uiLeft   = rc.uiLeft;
        tmpRECT.uiTop    = rc.uiTop;
        tmpRECT.uiWidth  = PBAR(barId)->XSize;
        tmpRECT.uiHeight = PBAR(barId)->YSize;
    }

    ShowOSD_SetLanguageEx(uwDispID, uLang);

    switch(iMenuId)
    {
        case MENUID_SUB_MENU_VMD_SHOT_NUM:
        case MENUID_SUB_MENU_TIMELAPSE_TIME:
        {
            char sz[20]= {0};

            if(iMenuId==MENUID_SUB_MENU_TIMELAPSE_TIME)
                sprintf(sz, "%d %s", m_ulTimeLapseTime[iItem], (char*)OSD_GetStringViaID(iStrID));
            else if(iMenuId==MENUID_SUB_MENU_VMD_SHOT_NUM)
                sprintf(sz, "%d %s", m_ulVMDShotNum[iItem], (char*)OSD_GetStringViaID(iStrID));

            OSD_ShowString( uwDispID, sz, NULL,
                            tmpRECT, clrFont, clrBack,
                            GUI_TA_HCENTER|GUI_TA_VCENTER);
        }
        break;

        default:
        #ifdef __SLIDE_STRING__ // Under Coding
        if(!AHC_IsTVConnectEx() && !AHC_IsHdmiConnect())
        {
            #define SLIDE_FOCUSED_STRING (0)

            int     slide;
            RECT    tmpRECT1;
            INT16   wTextAlign  = GUI_TA_HCENTER|GUI_TA_VCENTER;

            tmpRECT1.uiLeft   = tmpRECT.uiLeft   + STR_RECT_OFFSET_X;
            tmpRECT1.uiTop    = tmpRECT.uiTop    + STR_RECT_OFFSET_Y;
            tmpRECT1.uiWidth  = tmpRECT.uiWidth  + STR_RECT_OFFSET_W;
            tmpRECT1.uiHeight = tmpRECT.uiHeight + STR_RECT_OFFSET_H;
            if(1)
            {
                slide = IsSlideSting(iStrID);

                OSD_ShowStringPoolSlide(uwDispID, iStrID, tmpRECT1, clrFont, clrBack, wTextAlign, &slide);
                slide = -20;
                if (slide < 0) //[slide] will less than 0 when string length is more then drawing rectangle
                {
                    StartSlideString(uwDispID, iStrID, tmpRECT1, BAR(barId), rc, clrFont, clrBack, -slide);
                }
            }
            else
            {
                int idx = GetSlideStringIdx(iStrID);

                if(idx!=0xFF)
                    StopSlideStringIdx(idx);

                slide = 0;

                OSD_ShowStringPoolSlide(uwDispID, iStrID, tmpRECT1, clrFont, clrBack, wTextAlign, &slide);

                if(slide < 0)
                    OSD_ShowStringPool(uwDispID, iStrID, tmpRECT1, clrFont, clrBack, GUI_TA_LEFT|GUI_TA_VCENTER);
            }
        }
        else
        #endif
            OSD_ShowString( uwDispID, (char*)OSD_GetStringViaID(iStrID), NULL,
                            tmpRECT, clrFont, clrBack,
                            GUI_TA_HCENTER|GUI_TA_VCENTER);
        break;
    }
#endif
    #endif  // FONT_LARGE
}

//=======================lyj===

void DrwawSubItem_lightBar(UINT16 uwDispID,int dexLight)
{
	RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;


		switch(dexLight)
        		{
				case 0:    RECTExit.uiLeft = 33;
						RECTExit.uiTop = 124;
						if(lightBarFlag_1)
						OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						
						break;
				case 1:    RECTExit.uiLeft = 160;
						RECTExit.uiTop = 124;  
						if(lightBarFlag_2)
						OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						break;
				case 2:    RECTExit.uiLeft = 286;
						RECTExit.uiTop = 124;
						if(lightBarFlag_3)
						OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						break;
						
				case 3:    RECTExit.uiLeft = 33;
						RECTExit.uiTop = 217;  
						if(lightBarFlag_4)
						OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						break;
				case 4:    RECTExit.uiLeft = 160;
						RECTExit.uiTop = 217;  
						if(lightBarFlag_5)
						OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						break;
				case 5:    RECTExit.uiLeft = 286;
						RECTExit.uiTop = 217;  
						if(lightBarFlag_6)
						OSD_Draw_Icon(bmIcon_Num_5, RECTExit, uwDispID);
						break;

			}

	
}

void DrwawSubItem_lightBarEX(int dexLight,AHC_BOOL lihgtFlag)
{
	RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;


		switch(dexLight)
        		{
				case 0:    RECTExit.uiLeft = 33;
						RECTExit.uiTop = 124;
						
						break;
				case 1:    RECTExit.uiLeft = 160;
						RECTExit.uiTop = 124;    
						
						break;
				case 2:    RECTExit.uiLeft = 286;
						RECTExit.uiTop = 124; 
						
						break;
						
				case 3:    RECTExit.uiLeft = 33;
						RECTExit.uiTop = 217;   
						
						break;
				case 4:    RECTExit.uiLeft = 160;
						RECTExit.uiTop = 217;  
					
						break;
				case 5:    RECTExit.uiLeft = 286;
						RECTExit.uiTop = 217;  
						
						break;
						

			}

		printc("~~~~~~DrwawSubItem_lightBarEX  ~~lihgtFlag =%d~~~~~~~~\r\n",lihgtFlag);
						if(lihgtFlag)
							OSD_Draw_Icon(bmIcon_Num_5, RECTExit, wDispID);
						else
							OSD_Draw_Icon(bmSub_bar, RECTExit, wDispID);

}

void DrawSubItem1(UINT16 uwDispID, int iItem, int iTotalItems, UINT8 uLang, UINT32 iStrID, GUI_BITMAP IconID, UINT32 barId, GUI_COLOR clrFont, GUI_COLOR clrBack, GUI_COLOR clrSelect)
{


	RECT RECTExit = {154,236,0,0};
	RECT tmpRECT1 = {153,   235,   180, 8};
	RECT RECTExit1 = {340 ,234,20,10};// lyj 20190216
	char    szv[5];
	//GUI_COLOR clrFont1;
	MMP_UBYTE _index;
	

	#if(MENU_BLUETOOTH_PROGRESS_BAR)
	wDispID=uwDispID;
	#endif

#if 0
        if(sub_Menu_Get_Page()==0) // long
        	{
        	printc("-------iItem=%d--------------\r\n",iItem);
		if(iItem==7)
        		{
			OSD_Draw_Icon(IconID, RECTExit, uwDispID);
        	}else
        		{
			 switch(iStrID)
	            	{
	            		//case 5:
				case 0:    RECTExit.uiLeft = 20;
						RECTExit.uiTop = 125;     break;
				case 1:    RECTExit.uiLeft = 271;
						RECTExit.uiTop = 125;     break;
				case 2:    RECTExit.uiLeft = 19;
						RECTExit.uiTop = 217;     break;
				case 3:    RECTExit.uiLeft = 146;
						RECTExit.uiTop = 217;     break;
				case 4:    RECTExit.uiLeft = 271;
						RECTExit.uiTop = 217;     break;
			}
			RECTExit1.uiLeft   = RECTExit.uiLeft;
			 RECTExit1.uiTop    = RECTExit.uiTop;
			 RECTExit1.uiWidth  = RECTExit.uiWidth ;
			 RECTExit1.uiHeight = RECTExit.uiHeight;
		        OSD_Draw_Icon(IconID, RECTExit1, uwDispID);
		// 显示电压
	sprintf(szv, "%dV", (INT32)wSecond);
	AHC_OSDSetFont(uwDispID, &GUI_Font20_1);
	OSD_ShowString( uwDispID,szv, NULL, tmpRECT1, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
			
        	}
		
        }
	else if(sub_Menu_Get_Page()==1) //5
		{
        	printc("-------iItem=%d--------------\r\n",iItem);
		if(iItem==6)// 7
        		{
			OSD_Draw_Icon(IconID, RECTExit, uwDispID);
        	}else
        		{
			 switch(iStrID)
	            	{
	            		
				case 0:    RECTExit.uiLeft = 33;
						RECTExit.uiTop = 124;     break;
				case 1:    RECTExit.uiLeft = 160;
						RECTExit.uiTop = 124;     break;
				case 2:    RECTExit.uiLeft = 286;
						RECTExit.uiTop = 124;     break;
						
				case 3:    RECTExit.uiLeft = 33;
						RECTExit.uiTop = 217;     break;
				case 4:    RECTExit.uiLeft = 160;
						RECTExit.uiTop = 217;     break;
				case 5:    RECTExit.uiLeft = 286;
						RECTExit.uiTop = 217;     break;
			}
		   RECTExit1.uiLeft   = RECTExit.uiLeft;
			 RECTExit1.uiTop    = RECTExit.uiTop;
			 RECTExit1.uiWidth  = RECTExit.uiWidth ;
			 RECTExit1.uiHeight = RECTExit.uiHeight;
		        OSD_Draw_Icon(IconID, RECTExit1, uwDispID);
        	}
		
        }
	else

#endif

#if 0
		//if(sub_Menu_Get_Page()==2)
		{
		printc("-------iItem=%d---clrSelect=%d-----------\r\n",iItem,clrSelect);
		printc("----------------iStrID=%d---------------\r\n",iStrID);
		switch(sub_Menu_Get_Page())//(iStrID)
	            	{

					printc("~~~~~lyj~~~~iItem = %d~~~~~\r\n",iItem);
	            		
				case 0:  
						printc("~~~~~lyj~~~111~iItem = %d~~~~~\r\n",iItem);
						sprintf(szv, "FRONT     ");
						RECTExit.uiLeft = 33;
						RECTExit.uiTop = 140;  
						if(iStrID==0)
							clrFont1= OSD_COLOR_WHITE;
						else
						 clrFont1= OSD_COLOR_GRAY; 
						 break;
				case 1:    sprintf(szv, "REAR       ");
						RECTExit.uiLeft = 33;
						RECTExit.uiTop = 175;
						if(iStrID==1)
							clrFont1= OSD_COLOR_WHITE;
						else
						 clrFont1= OSD_COLOR_GRAY; 
						 break;
				case 2:    sprintf(szv, "OUTPUT 1");
						RECTExit.uiLeft = 33;
						RECTExit.uiTop = 215; 
						if(iStrID==2)
							clrFont1= OSD_COLOR_WHITE;
						else
						 clrFont1= OSD_COLOR_GRAY; 
						
			}
			
		  
	    RECTExit1.uiLeft     =RECTExit.uiLeft;
	    RECTExit1.uiTop     = RECTExit.uiTop; 
	    RECTExit1.uiWidth  = 100;
	    RECTExit1.uiHeight = 50;
		AHC_OSDSetFont(uwDispID, &GUI_Font24B_1); 
	OSD_ShowString( uwDispID,szv, NULL, RECTExit1, clrFont1, clrBack,GUI_TA_HCENTER|GUI_TA_VCENTER);

#endif
			//volume_M = 0;
			//volume_B  = 0;
			sub_flag_ex = 0;

			sprintf(szv, "%d",iItem);
			AHC_OSDSetFont(uwDispID, &GUI_Font16B_1);
			AHC_OSDSetColor(uwDispID, 0x00cc0099);//0x00cc0099 0xFF323232
			AHC_OSDDrawFillRect(uwDispID, RECTExit1.uiLeft, RECTExit1.uiTop, 360, 244);
			OSD_ShowString( uwDispID,szv, NULL, RECTExit1, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);// lyj 20190216

			AHC_OSDSetColor(uwDispID, 0xFFFFFFFF); 
			AHC_OSDDrawFillRect(uwDispID, tmpRECT1.uiLeft, tmpRECT1.uiTop, tmpRECT1.uiLeft + tmpRECT1.uiWidth, tmpRECT1.uiTop+tmpRECT1.uiHeight);
			#if 0
			switch(select_flag)
			{
				case 0: 
					//OSD_Draw_Icon(bmFront,RECTExit,uwDispID);
					AHC_OSDDrawFillRect(uwDispID, tmpRECT1.uiLeft, tmpRECT1.uiTop, tmpRECT1.uiLeft + tmpRECT1.uiWidth, tmpRECT1.uiTop+tmpRECT1.uiHeight);
					printc("~~11111111111111111~\r\n",iStrID); break;
				case 1:
					OSD_Draw_Icon(bmRear,RECTExit,uwDispID); 
					printc("~~22222222222222222~\r\n",iStrID);break;
				case 2:
					OSD_Draw_Icon(bmoutput,RECTExit,uwDispID); 
					printc("~~133333333333333333~\r\n",iStrID); break;
			}
			#endif
	
#if 0
		
				4514541564489

				#elif 1
				for(_index = 0; _index< iItem;_index++)
				{
					AHC_OSDDrawBitmap(uwDispID, &IconID, RECTExit.uiLeft + 12*_index, RECTExit.uiTop);
				}

				#else


				#endif


		
   }
	





//======================end====


void DrawSubItemEx(UINT16 uwDispID, PSMENUSTRUCT pMenu)
{
    UINT32      i, iBegin, iEnd;
    UINT32      barType = BAR_NORMAL_TYPE;
    UINT32      uPageItemNum;
    GUI_COLOR   TextClr;
    UINT8       uLang;
    GUI_BITMAP Bicon;
    // To find some Submenu text is longer the width of default button
    // to use wide button
    #if !defined(FONT_LARGE)
    BAR_TYPE(barType, pMenu);
    #endif

    uPageItemNum = SUB_MENU_PAGE_ITEM;
    TextClr      = MENU_TEXT_COLOR;

    uLang  = MenuDraw_GetDefaultLanguage(pMenu);

    iBegin = ALIGN_DOWN( pMenu->iCurrentPos, uPageItemNum );
    iEnd   = MIN( iBegin + uPageItemNum, pMenu->iNumOfItems );
printc("--------------uLang=%d---------\r\n",uLang);
printc("--------------pMenu->iCurrentPos=%d---------\r\n",pMenu->iCurrentPos);
printc("--------------pMenu->iNumOfItems=%d---------\r\n",pMenu->iNumOfItems);
printc("--------------iBegin=%d----iEnd =%d-----\r\n",iBegin,iEnd);
printc("--------------pMenu->pfMenuGetDefaultVal(pMenu) =%d---------\r\n",pMenu->pfMenuGetDefaultVal(pMenu) );
	
	 if(Menu_Get_Page()==7) // long
	 {
	 	switch(pMenu->iCurrentPos)
            	{
			case 0:   Bicon=bmON_APP ;    break;
			case 1:   Bicon=bmColor ;    break;
			case 2:   Bicon=bmSpeed ;    break;
			case 3:   Bicon=bmFlash ;    break;
			//case 4:   Bicon=bmLight_Show ;    break;
			//case 5:   Bicon=bmON_APP ;    break;
		}

		#if 0 // lyj 20191018
		{
			RECT tmpRECT1 = {415,   55,   60, 35};
			char szvd[8];
			INT32		level=0;
			// INT32 	OldLevel = 0;
			 UINT16		level1=0;
			
			 pf_General_EnGet(COMMON_KEY_B_VOL, &level);
			if(level<820)
				{
					level1=(level*100/791)%10;//791
					level=level*10/791;	
				}
				else if(level<1300)
				{
					//level=level+30;
					level1=(level*100/758)%10;// 758
					level=level*10/758;

				}

				sprintf(szvd, "%d.%dV", level,level1);
				AHC_OSDSetFont(uwDispID, &GUI_Font20_1);
				//AHC_OSDDrawBitmap(uwDispID, &bmIcon_Num_4,415, 55);
				OSD_ShowString( uwDispID,szvd, NULL, tmpRECT1, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
					
		}
		#endif
	 }
	 else if(Menu_Get_Page()==4)// long
	 {
		Bicon=bmSub_bar ; //bmIcon_Num_5
	 }
	 else if (Menu_Get_Page()==1)
	 {
		//===============lyj20180231=======
		 //fm_si47xx_init();
	 
		switch(pMenu->iCurrentPos)
		{

				case 0: Bicon = bmFM1;

				// Draw_FM_AM_icon(9050);
				 break;
				case 1: Bicon = bmFM2;
				//Draw_FM_AM_icon(9050);
				break;
				case 2: Bicon = bmFM3; 
			  // Draw_FM_AM_icon(9050);
			   break;

			    case 3: Bicon = bmSCAN;
				break;
				// Draw_FM_AM_icon(8750);
				#if 1
				case 4: Bicon = bmvol3;
				 break;

				 case 5: Bicon = bmvol4;
				 break;// lyj 20190226
				#endif
		}

	 }

	 else if (Menu_Get_Page()==2)
	 {
	 	//am_si47xx_init();
		switch(pMenu->iCurrentPos)
			{

				case 0: Bicon = bmAM1; break;
				case 1: Bicon = bmAM2; break;
				case 2: Bicon = bmAM3;  break;

			    case 3: Bicon = bmSCAN1; break;

			}
	 }
	
	 
    for( i=iBegin; i<iEnd; i++ )
    {
        UINT32      barState;
        UINT8       idx = i;
        GUI_COLOR   bkColor = OSD_COLOR_TRANSPARENT;

	
 /*
        if(pMenu->iNumOfItems > 1)
        {
       
            if(( i == pMenu->iCurrentPos ) && ( i != pMenu->pfMenuGetDefaultVal(pMenu) ))
            {
                barState = BAR_STATE_YEL;
            }
            else if(( i == pMenu->iCurrentPos ) && ( i == pMenu->pfMenuGetDefaultVal(pMenu) ))
            {
                barState = BAR_STATE_YEL_DEF;
            }
            else if(( i != pMenu->iCurrentPos ) && ( i == pMenu->pfMenuGetDefaultVal(pMenu) ))
            {
                barState = BAR_STATE_WHT_DEF;
            }
            else
            {
                barState = BAR_STATE_WHT;
            }
           
        }
        else
        {
            barState = BAR_STATE_YEL;
        }
*/
	if(Menu_Get_Page() ==4)
		DrwawSubItem_lightBar(uwDispID,i); // lyj 20181024

        DrawSubItem(uwDispID, idx, pMenu->iNumOfItems,
                    uLang, pMenu->iCurrentPos/*pMenu->pItemsList[i]->uiStringId*/,
                    Bicon/*pMenu->pItemsList[i]->bmpIcon*/,
                    MAKE_BAR_ID(barType, barState),
                    TextClr, bkColor, 0x0);
    }
}

void MenuDrawSubBackCurtain(UINT16 uwDispID, GUI_COLOR bkColor)
{
    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
        AHC_OSDSetColor(uwDispID, TV_MENU_BK_COLOR);
    else
        AHC_OSDSetColor(uwDispID, OSD_COLOR_TITLE);

    DispBufWidth = AHC_GET_ATTR_OSD_W(uwDispID);
    DispBufHeight = AHC_GET_ATTR_OSD_H(uwDispID);
        
    AHC_OSDDrawFillRect(uwDispID, 0, 0  , DispBufWidth, 36 );
    AHC_OSDDrawFillRect(uwDispID, 0, DispBufHeight-36, DispBufWidth, DispBufHeight);

    AHC_OSDSetColor(uwDispID, bkColor);

    AHC_OSDDrawFillRect(uwDispID, 0, 40,  DispBufWidth, DispBufHeight-40);

    AHC_OSDSetPenSize(uwDispID, 5);
    AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);

    AHC_OSDDrawFillRect(uwDispID, 0, 36,  DispBufWidth, 40);
    AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);
    AHC_OSDDrawFillRect(uwDispID, 0, DispBufHeight-40, DispBufWidth, DispBufHeight-36);

}

void MenuDrawSubBackCurtainGrid(UINT16 uwDispID)
{
}

void MenuDrawSubBackCurtainConfirm(UINT16 uwDispID)
{
}

void MenuDrawSubPageInfo(UINT16 uwDispID, int uiCurPage, int uiTotalPage)
{
}

void MenuDrawSubMenuDescription(UINT16 uwDispID, UINT32 iStrID)
{
    RECT    tempRECT   = RECT_MENU_SUB_DESC;
    INT16   wTextAlign = GUI_TA_LEFT|GUI_TA_VCENTER;

    GUI_COLOR bkColor;

    tempRECT.uiTop = AHC_GET_ATTR_OSD_W(uwDispID)-34;

    bkColor = (AHC_IsTVConnectEx() || AHC_IsHdmiConnect())?(TV_MENU_BK_COLOR):(OSD_COLOR_DARKGRAY2);
	
    #ifdef SLIDE_STRING
    {
        int slide = IsSlideSting(iStrID);

        OSD_ShowStringPoolSlide(uwDispID, iStrID, tempRECT, MENU_TEXT_COLOR, bkColor, wTextAlign, &slide);

        if (slide < 0) //[slide] will less than 0 when string length is more then drawing rectangle
        {
            RECT    brc;
            GUI_BITMAP barID;

            memset(&barID, 0, sizeof(GUI_BITMAP));
            brc.uiLeft   = tempRECT.uiLeft;
            brc.uiTop    = tempRECT.uiTop + 1 ;
            brc.uiWidth  = tempRECT.uiWidth ;
            brc.uiHeight = tempRECT.uiHeight;
            StartSlideString(uwDispID, iStrID, tempRECT, barID, brc, MENU_TEXT_COLOR, bkColor, -slide);
        }
    }
    #else
    OSD_ShowStringPool(uwDispID, iStrID, tempRECT, MENU_TEXT_COLOR, bkColor, wTextAlign);
    #endif
}

void MenuDrawSubButtons(UINT16 uwDispID)
{
    #if defined(FONT_LARGE)
    // NOP
    #else   // FONT_LARGE   
    GUI_BITMAP      barID   = bmIcon_D_Up;
    UINT32          PageItemNum, ItemInterval;
    
    RECT tempRECT2 = POS_MENU_UP;
    RECT tempRECT3 = POS_MENU_DOWN;

    MenuDraw_GetMainMenuPageItem(uwDispID, barID, &PageItemNum, &ItemInterval);

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {
        tempRECT2.uiLeft += usShiftStringRight;
        tempRECT3.uiLeft += usShiftStringRight;
    }   

    tempRECT2.uiTop     = 40 + (ItemInterval-barID.YSize)/2;
    OSD_Draw_Icon(barID,     tempRECT2, uwDispID);

    barID = bmIcon_D_Down;
    tempRECT3.uiTop     = 40 + ItemInterval*(PageItemNum-1) + (ItemInterval-barID.YSize)/2;
    OSD_Draw_Icon(barID, tempRECT3, uwDispID);
    #endif  // FONT_LARGE
}
extern MMP_USHORT FMCurruntFreq;
extern MMP_USHORT AMCurruntFreq;
extern AHC_BOOL FM_AM_playback;
extern AHC_BOOL displayFreqflag ;// lyj 20190215
extern AHC_BOOL displayFreqflagAM;
void Reserved_radio(MMP_USHORT availedFreq[],MMP_UBYTE z0,AHC_BOOL bicIcon);
extern AHC_BOOL FM_AM_falg; // lyj 20190626
extern AHC_BOOL PowerOnVoiceMode;
void MenuDrawSubTitle(UINT16 uwDispID, UINT32 iStrID)
{
    RECT      tmpRECT = POS_MENU_TITLE;
	RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;
    GUI_COLOR bkColor;
	AHC_BOOL Flag;
    #if (SUPPORT_TOUCH_PANEL)
    RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;

    OSD_Draw_Icon(bmIcon_Return, RECTExit, uwDispID);
    #else       // #if (SUPPORT_TOUCH_PANEL)
    tmpRECT.uiLeft = 5;

	
    switch(Menu_Get_Page())
    	{
		case 0:	//printc("OSD1~~~~~~~~~~~%d~~~~~~~~~\r\n",Menu_Get_Page());
				OSD_Draw_Icon(bmSub_Bluetooth, RECTExit, uwDispID);// 显示背景图片   蓝牙
				Flag = BLUETOOH_FLAG;
				SelfAllFlag.SaveTheBluetooth=1;
				SelfAllFlag.SaveTheUSB=0;
				SelfAllFlag.SaveTheAUX=0;
				//printc("~~~~~~~~select~~bluetooth~~\r\n");
				Set_flag(0);
				BD3490_init();// long 4-27
						break;
		case 1:  
				//printc("OSD2~~~~~~~~~~~%d~~~~~~~~~\r\n",Menu_Get_Page());
				OSD_Draw_Icon(/*bmSub_FM*/bmFM_item, RECTExit, uwDispID);
				//FM_AMFLagSet(0);
				FM_AM_falg = 1;
				Set_flag(3);
				BD3490_init();// long 4-27

				if(/*FM_AM_playback ==0*/0)// lyj 20190606
				{
					
					Draw_FM_AM_icon_Ex(10880,uwDispID);
				}
				else
				{
					if(FMCurruntFreq !=0 )
					{
							Draw_FM_AM_icon_Ex(FMCurruntFreq,uwDispID);
							Reserved_radio(radio,uwDispID,0);
					
						/*if(displayFreqflag ==1)
						{
							//Draw_FM_AM_icon_Ex(FMCurruntFreq,uwDispID);
							//Reserved_radio(radio,uwDispID,0);
							return_flag = 1;
						}
						else
						{
							//Draw_FM_AM_icon_Ex(FMCurruntFreq,uwDispID);
							return_flag = 0;
						}*/ // lyj 20190712
						if(MenuSettingConfig()->uiMOVQuality < 3)
							return_flag = 1;
					}
					else
					{
						Draw_FM_AM_icon_Ex(10880,uwDispID);
					}
				}
				//fm_si47xx_init();
				SelfAllFlag.SaveTheFM=1;
				SelfAllFlag.SaveTheAM=0;
				Flag = RADIO_FM_FLAG;
			break;
		case 2:
				//printc("OSD3~~~~~~~~~~~%d~~~~~~~~~\r\n",Menu_Get_Page());
				OSD_Draw_Icon(/*bmSub_AM*/bmAM_item, RECTExit, uwDispID);
				Flag = RADIO_AM_FLAG;
				SelfAllFlag.SaveTheFM=1;
				SelfAllFlag.SaveTheAM=1;
				//FM_AMFLagSet(1);
				FM_AM_falg = 0;
				
				Set_flag(3);
				BD3490_init();// long 4-27

				if(/*FM_AM_playback ==0*/0) // lyj 20190606
				{
					Draw_AM_icon_Ex(1620,uwDispID);
				}
				else
				{
					if(AMCurruntFreq != 0)
					{
						Draw_AM_icon_Ex(AMCurruntFreq,uwDispID);
						Reserved_radio(radio_am,uwDispID,0);
					
						if(displayFreqflagAM == 1)
						{
							//Draw_AM_icon_Ex(AMCurruntFreq,uwDispID);
							//Reserved_radio(radio_am,uwDispID,0);
							return_flag = 1;
						}
						else
						{
							//Draw_AM_icon_Ex(AMCurruntFreq,uwDispID);
							return_flag = 0;
						}
					}
					else
					{
						Draw_AM_icon_Ex(1620,uwDispID);
					}

				}
				//am_si47xx_init();
			break;
		case 3: 
				//printc("OSD4~~~~~~~~~~~%d~~~~~~~~~\r\n",Menu_Get_Page());
				OSD_Draw_Icon(bmSub_Vol, RECTExit, uwDispID);
				Flag = VOLUME_FLAG;
			break;

			#if 0 //WB long
		case 4:
				
				printc("OSD5~~~~~~~~~~~%d~~~~~~~~~\r\n",Menu_Get_Page());
				Flag = RADIO_WB_FLAG; 
				break;

			#endif
		case 4:  
				//printc("OSD6~~~~~~~~~~~%d~~~~~~~~~\r\n",Menu_Get_Page());
				OSD_Draw_Icon(bmSub_Light_Bar, RECTExit, uwDispID);
				SelfAllFlag.SaveTheLightBar =1;
				SelfAllFlag.SaveTheRGB=0;
				SelfAllFlag.SaveTheBrightness = 0;
				Flag = LIGHT_BAR_FLAG; 
			break;
		case 5: 	//printc("OSD7~~~~~~~~~~~%d~~~~~~~~~\r\n",Menu_Get_Page());
				OSD_Draw_Icon(bmSub_AUX, RECTExit, uwDispID);// 显示背景图片 AUX
				SelfAllFlag.SaveTheBluetooth=0;
				SelfAllFlag.SaveTheAUX=1;
				SelfAllFlag.SaveTheUSB=0;
				Flag = AUX_FLAG;
				//FM_AM_falg = 0;
				Set_flag(1);
				BD3490_init();// long 4-27
			break;
		case 6: 	//printc("OSD8~~~~~~~~~~~%d~~~~~~~~~\r\n",Menu_Get_Page());
				OSD_Draw_Icon(bmSub_USB, RECTExit, uwDispID);// 显示背景图片
				SelfAllFlag.SaveTheBluetooth=0;
				SelfAllFlag.SaveTheAUX=0;
				SelfAllFlag.SaveTheUSB=1;
				Flag = USB_FLAG;
				//FM_AM_falg = 1;
				//printc("~~~~~~~~select~~usb~~\r\n");
				//if(MenuSettingConfig()->uiPowerUpFlag ==0)
				//{
				//	uart_to_mcu_voice(MAINVOICE,0x00,MUTE_ON);
				//	AHC_OS_SleepMs(5);
			//	}
				SetPrevNextPin(1); // lyj 20190530
				AHC_OS_SleepMs(10);

				if(PowerOnVoiceMode == 7)
				{
					Set_flag(2);
					BD3490_init();// long 4-27
				}
			break;
		case 7:  	//printc("OSD9~~~~~~~~~~~%d~~~~~~~~~\r\n",Menu_Get_Page());
				OSD_Draw_Icon(bmSub_RGB, RECTExit, uwDispID);
				SelfAllFlag.SaveTheRGB=1;
				SelfAllFlag.SaveTheLightBar =0;
				SelfAllFlag.SaveTheBrightness = 0;
				Flag = RGB_FLAG;
			break;
		case 8:  
			//	OSD_Draw_Icon(bmSub_Brightness, RECTExit, uwDispID);// 显示背景图片 
			break;
		default:
			break;

	}
//	printc("----------iStrID=%d------------\r\n",iStrID);
    Main_Set_Page(Flag);
    #endif      // #if (SUPPORT_TOUCH_PANEL)

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {
        tmpRECT.uiLeft += usShiftStringRight;       
        bkColor = TV_MENU_BK_COLOR;
    }   
    else
    {
        bkColor = OSD_COLOR_TITLE;
    }   

    #ifdef SLIDE_STRING
    {
        int slide = IsSlideSting(iStrID);

        OSD_ShowStringPoolSlide(uwDispID, iStrID, tmpRECT, MENU_TEXT_COLOR, bkColor, GUI_TA_LEFT|GUI_TA_VCENTER, &slide);

        if (slide < 0) //[slide] will less than 0 when string length is more then drawing rectangle
        {
            RECT    brc;
            GUI_BITMAP barID;

            memset(&barID, 0, sizeof(GUI_BITMAP));
            brc.uiLeft   = tmpRECT.uiLeft;
            brc.uiTop    = tmpRECT.uiTop + 1 ;
            brc.uiWidth  = tmpRECT.uiWidth ;
            brc.uiHeight = tmpRECT.uiHeight;
            StartSlideString(uwDispID, iStrID, tmpRECT, barID, brc, MENU_TEXT_COLOR, bkColor, -slide);
        }
    }
    #else// 显示标题文字
//    OSD_ShowStringPool(uwDispID, iStrID, tmpRECT, MENU_TEXT_COLOR, bkColor, GUI_TA_LEFT|GUI_TA_VCENTER);
    #endif
}


//==========================lyj==
void MenuDrawSubTitle1(UINT16 uwDispID, UINT32 iStrID)
{
    RECT      tmpRECT = POS_MENU_TITLE;
	RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;
    GUI_COLOR bkColor;
	AHC_BOOL Flag;
    #if (SUPPORT_TOUCH_PANEL)
    RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;

    OSD_Draw_Icon(bmIcon_Return, RECTExit, uwDispID);
    #else       // #if (SUPPORT_TOUCH_PANEL)
    tmpRECT.uiLeft = 5;

	//OSD_Draw_Icon(bmvolume, RECTExit, uwDispID);
	printc("~~~99999999999999999~~Menu_Get_Page() = %d~~~~~~\r\n",Menu_Get_Page());
    switch(sub_Menu_Get_Page())
    	{
		case 0:	printc("OSD1~~~~~~~~~~~%d~~~~~~~~~\r\n",sub_Menu_Get_Page());
				OSD_Draw_Icon(bmSub_Vol, RECTExit, uwDispID);
				
						break;
		case 1:  
				printc("OSD2~~~~~~~~~~~%d~~~~~~~~~\r\n",sub_Menu_Get_Page());
				OSD_Draw_Icon(bmSub_Vol, RECTExit, uwDispID);
				//Flag = RADIO_FM_FLAG;
			break;
		case 2:
				printc("OSD3~~~~~~~~~~~%d~~~~~~~~~\r\n",sub_Menu_Get_Page());
				OSD_Draw_Icon(bmSub_Vol, RECTExit, uwDispID);
			break;
		default:
			break;

	}
//	printc("----------iStrID=%d------------\r\n",iStrID);
   // Main_Set_Page(Flag);
    #endif      // #if (SUPPORT_TOUCH_PANEL)

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {
        tmpRECT.uiLeft += usShiftStringRight;       
        bkColor = TV_MENU_BK_COLOR;
    }   
    else
    {
        bkColor = OSD_COLOR_TITLE;
    }   


}

//===========================lyj===20180530=============

static void Freq_arim(MMP_USHORT aFreq,MMP_UBYTE *a_Freq,MMP_UBYTE *b_Freq,MMP_UBYTE *c_Freq, MMP_UBYTE *d_Freq)
{

		if(aFreq)
		{

			*a_Freq = aFreq / 10000;

			if(*a_Freq)
			{
				*b_Freq = 10;

				*c_Freq = (aFreq % 10000) / 100;

				*d_Freq = ((aFreq % 10000) % 100) /10;
				
				printc("~~~~~~111~~~a_Freq = %d~~~~~~~~~~~~\r\n",*a_Freq);

			}else{

					*b_Freq = aFreq / 1000;

					*c_Freq = (aFreq % 1000) / 100;

					*d_Freq = ((aFreq % 1000) % 100) / 10;

					printc("~~~~222~~~~~a_Freq = %d~~~~~~~~~~~~\r\n",*a_Freq);

			}




		}


}

static void Am_Freq_arim(MMP_USHORT aFreq,MMP_UBYTE *a_Freq,MMP_UBYTE *b_Freq,MMP_UBYTE *c_Freq, MMP_UBYTE *d_Freq)
{

		if(aFreq)
		{

			*a_Freq = aFreq / 1000;

			if(*a_Freq)
			{

			
				*b_Freq = (aFreq % 1000)/100;

				*c_Freq = (aFreq % 100) / 10;

				*d_Freq = (aFreq % 100) % 10;
				
				//printc("~~~~am~~111~~~a_Freq = %d~~~~~~~~~~~~\r\n",*a_Freq);

			}else{

					*b_Freq = aFreq / 100;

					*c_Freq = (aFreq % 100) / 10;

					*d_Freq = (aFreq % 100) % 10;

					//printc("~~am~~222~~~~~a_Freq = %d~~~~~~~~~~~~\r\n",*a_Freq);

			}




		}


}





static void Select_icon(MMP_UBYTE SFreq,UINT8   bID0,RECT RECTExit)
{
	if(SFreq == 0)
		{

			
			RECTExit.uiTop = 129;
			OSD_Draw_Icon(bm0, RECTExit, bID0);
			
		}
		else if (SFreq == 1)
		{

			OSD_Draw_Icon(bm1, RECTExit, bID0);
		}
		else if(SFreq == 2)
		{

			OSD_Draw_Icon(bm2, RECTExit, bID0);

		}
		else if(SFreq == 3)
		{
			
			RECTExit.uiTop = 129;
			OSD_Draw_Icon(bm3, RECTExit, bID0);
		}
		else if (SFreq == 4)
		{

			OSD_Draw_Icon(bm4, RECTExit, bID0);
		}
		else if(SFreq == 5)
		{

			OSD_Draw_Icon(bm5, RECTExit, bID0);
		}
		else if(SFreq == 6)
		{

			OSD_Draw_Icon(bm6, RECTExit, bID0);
		}
		else if (SFreq == 7)
		{

			OSD_Draw_Icon(bm7, RECTExit, bID0);
		}
		else if(SFreq == 8)
		{

			OSD_Draw_Icon(bm8, RECTExit, bID0);

		}
		else if(SFreq == 9)
		{

			OSD_Draw_Icon(bm9, RECTExit, bID0);

		}



}


static void Am_Select_icon(MMP_UBYTE SFreq,UINT8   bID0,RECT RECTExit)
{
	if(SFreq == 0)
		{

			OSD_Draw_Icon(bmam0, RECTExit, bID0);
			
		}
		else if (SFreq == 1)
		{

			OSD_Draw_Icon(bmama, RECTExit, bID0);
		}
		else if(SFreq == 2)
		{

			OSD_Draw_Icon(bmamb, RECTExit, bID0);

		}
		else if(SFreq == 3)
		{
			

			OSD_Draw_Icon(bmamc, RECTExit, bID0);
		}
		else if (SFreq == 4)
		{

			OSD_Draw_Icon(bmam4, RECTExit, bID0);
		}
		else if(SFreq == 5)
		{

			OSD_Draw_Icon(bmam5, RECTExit, bID0);
		}
		else if(SFreq == 6)
		{

			OSD_Draw_Icon(bmam6, RECTExit, bID0);
		}
		else if (SFreq == 7)
		{

			OSD_Draw_Icon(bmam7, RECTExit, bID0);
		}
		else if(SFreq == 8)
		{

			OSD_Draw_Icon(bmam8, RECTExit, bID0);

		}
		else if(SFreq == 9)
		{

			OSD_Draw_Icon(bmam9, RECTExit, bID0);

		}



}

#if 0
void Draw_FM_AM_icon(MMP_USHORT Dfreq)
{

	UINT8   bID0 = 0, bID1 = 0;
    RECT      tmpRECT = POS_MENU_TITLE;
	RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;

	//RECT tmpRECT1 = {425,   55,   40, 35};
	RECT RECTExit1;

	MMP_UBYTE a_Freq,b_Freq,c_Freq,d_Freq;


    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

	Freq_arim(Dfreq,&a_Freq,&b_Freq,&c_Freq,&d_Freq);

	printc("~~mmmmmm~~~Dfreq = %d ~~~a_Freq = %d~~~~b_Freq = %d ~~c_Freq = %d ~~d_Freq = %d~~\r\n",Dfreq,a_Freq,b_Freq,c_Freq,d_Freq);

	
	if(Dfreq)
	{
		if(a_Freq)
		{
			if(b_Freq == 10)
			{
				if(1)
				{
						RECTExit.uiLeft = 127;
						RECTExit.uiTop =  132;
						OSD_Draw_Icon(bm1, RECTExit, bID0);
					
				}
				if(1)
				{
						RECTExit.uiLeft = 154;
						RECTExit.uiTop =  129;
						OSD_Draw_Icon(bm0, RECTExit, bID0);
				}
			

			}

		}
		else
		{

			RECTExit.uiLeft = 154;
			RECTExit.uiTop =  132;
			if(b_Freq == 8)
			{
			 OSD_Draw_Icon(bm8, RECTExit, bID0);			
			}
			else if(b_Freq == 9)
			{
	
			OSD_Draw_Icon(bm9, RECTExit, bID0);
			}

			if(b_Freq < 10)
			{
				RECTExit.uiLeft = 127;
				RECTExit.uiTop =  132;
				OSD_Draw_Icon(bmFMblue, RECTExit, bID0);

			}
		}

		if(c_Freq < 100)
		{

			RECTExit.uiLeft = 181;
			RECTExit.uiTop =  132;
			
			Select_icon(c_Freq,bID0,RECTExit);
		}
		if(1)
		{	
			
			RECTExit.uiLeft = 210;
			RECTExit.uiTop =  140;
			
			OSD_Draw_Icon(bmdot, RECTExit, bID0);

		}


		if(d_Freq < 100)
		{
			RECTExit.uiLeft = 225;
			RECTExit.uiTop =  132;
			
			Select_icon(d_Freq,bID0,RECTExit);

		}

		
	}	



    OSDDraw_ExitMenuDrawing(&bID0, &bID1);



}
#endif

void Draw_FM_AM_icon_Ex(MMP_USHORT Dfreq,UINT8   bID0)
{
	 RECT      tmpRECT = POS_MENU_TITLE;
	RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;

	//RECT tmpRECT1 = {425,   55,   40, 35};
	RECT RECTExit1;

	MMP_UBYTE a_Freq,b_Freq,c_Freq,d_Freq;

	Freq_arim(Dfreq,&a_Freq,&b_Freq,&c_Freq,&d_Freq);

	//printc("~~mmmmmm~~~Dfreq = %d ~~~a_Freq = %d~~~~b_Freq = %d ~~c_Freq = %d ~~d_Freq = %d~~\r\n",Dfreq,a_Freq,b_Freq,c_Freq,d_Freq);

	
	if(Dfreq)
	{
		if(a_Freq)
		{
			if(b_Freq == 10)
			{
				if(1)
				{
						RECTExit.uiLeft = 127;
						RECTExit.uiTop =  132;
						OSD_Draw_Icon(bm1, RECTExit, bID0);
					
				}
				if(1)
				{
						RECTExit.uiLeft = 154;
						RECTExit.uiTop =  129;
						OSD_Draw_Icon(bm0, RECTExit, bID0);
				}
			

			}

		}
		else
		{

			RECTExit.uiLeft = 154;
			RECTExit.uiTop =  132;
			if(b_Freq == 8)
			{
			 OSD_Draw_Icon(bm8, RECTExit, bID0);			
			}
			else if(b_Freq == 9)
			{
	
			OSD_Draw_Icon(bm9, RECTExit, bID0);
			}

			if(b_Freq < 10)
			{
				RECTExit.uiLeft = 127;
				RECTExit.uiTop =  132;
				OSD_Draw_Icon(bmFMblue, RECTExit, bID0);

			}
		}

		if(c_Freq < 100)
		{

			RECTExit.uiLeft = 181;
			RECTExit.uiTop =  132;
			
			Select_icon(c_Freq,bID0,RECTExit);
		}
		if(1)
		{	
			
			RECTExit.uiLeft = 210;
			RECTExit.uiTop =  140;
			
			OSD_Draw_Icon(bmdot, RECTExit, bID0);

		}


		if(d_Freq < 100)
		{
			RECTExit.uiLeft = 225;
			RECTExit.uiTop =  132;
			
			Select_icon(d_Freq,bID0,RECTExit);

		}

		
	}	


}

#if 0
void Draw_AM_icon(MMP_USHORT Dfreq)
{

	UINT8   bID0 = 0, bID1 = 0;
    RECT      tmpRECT = POS_MENU_TITLE;
	RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;

	//RECT tmpRECT1 = {425,   55,   40, 35};
	RECT RECTExit1;

	MMP_UBYTE a_Freq,b_Freq,c_Freq,d_Freq;


    OSDDraw_EnterMenuDrawing(&bID0, &bID1);


	Am_Freq_arim(Dfreq,&a_Freq,&b_Freq,&c_Freq,&d_Freq);

	//printc("~~mmmmmm~~~Dfreq = %d ~~~a_Freq = %d~~~~b_Freq = %d ~~c_Freq = %d ~~d_Freq = %d~~\r\n",Dfreq,a_Freq,b_Freq,c_Freq,d_Freq);

	
	if(Dfreq)
	{
		if(a_Freq)
		{
			
			RECTExit.uiLeft = 141;
			RECTExit.uiTop =  124;
			OSD_Draw_Icon(bmama, RECTExit, bID0);

		}
		else
		{
			RECTExit.uiLeft = 141;
			RECTExit.uiTop =  124;
			OSD_Draw_Icon(bmAMgreen, RECTExit, bID0);			

		}
		if(b_Freq < 100)
		{

			RECTExit.uiLeft = 168;
			RECTExit.uiTop =  124;

			Am_Select_icon(b_Freq,bID0,RECTExit);
		}

		if(c_Freq < 100)
		{

			RECTExit.uiLeft = 195;
			RECTExit.uiTop =  124;
			
			Am_Select_icon(c_Freq,bID0,RECTExit);
		}


		if(d_Freq < 100)
		{
			RECTExit.uiLeft = 222;
			RECTExit.uiTop =  124;
			
			Am_Select_icon(d_Freq,bID0,RECTExit);

		}

		
	}	


    OSDDraw_ExitMenuDrawing(&bID0, &bID1);



}
#endif

void Draw_AM_icon_Ex(MMP_USHORT Dfreq,UINT8   bID0)
{
	  RECT      tmpRECT = POS_MENU_TITLE;
	RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;

	RECT RECTExit1;

	MMP_UBYTE a_Freq,b_Freq,c_Freq,d_Freq;

	Am_Freq_arim(Dfreq,&a_Freq,&b_Freq,&c_Freq,&d_Freq);

	if(Dfreq)
	{
		if(a_Freq)
		{
			
			RECTExit.uiLeft = 141;
			RECTExit.uiTop =  124;
			OSD_Draw_Icon(bmama, RECTExit, bID0);

		}
		else
		{
			RECTExit.uiLeft = 141;
			RECTExit.uiTop =  124;
			OSD_Draw_Icon(bmAMgreen, RECTExit, bID0);			

		}
		if(b_Freq < 100)
		{

			RECTExit.uiLeft = 168;
			RECTExit.uiTop =  124;

			Am_Select_icon(b_Freq,bID0,RECTExit);
		}

		if(c_Freq < 100)
		{

			RECTExit.uiLeft = 195;
			RECTExit.uiTop =  124;
			
			Am_Select_icon(c_Freq,bID0,RECTExit);
		}


		if(d_Freq < 100)
		{
			RECTExit.uiLeft = 222;
			RECTExit.uiTop =  124;
			
			Am_Select_icon(d_Freq,bID0,RECTExit);

		}

		
	}


}


//=========================end==============================


void MenuDraw2ndSubPageInit(UINT16 uwDispID, PSMENUSTRUCT pMenu, UINT32 CurtainType)
{
}

void MenuDrawSubPageR1(UINT8 uwDispID, PSMENUSTRUCT pMenu)
{
    if (GetSubMenuType()==SUBMENU_SUBPAGE)
    {
        if( pMenu->uiStringId != -1 )
        {
     		
            MenuDrawSubTitle(uwDispID, pMenu->uiStringId);
        }

   //     MenuDraw_MediaType(uwDispID);// 显示媒体

   //    MenuDraw_BatteryStatus(uwDispID);// 显示电池
    }
}

//===================lyj=================
void MenuDrawSubPageR4(UINT8 uwDispID, PSMENUSTRUCT pMenu)
{
    if (GetSubMenuType()==SUBMENU_SUBPAGE)
    {
        if( pMenu->uiStringId != -1 )
        {
     		
            MenuDrawSubTitle1(uwDispID, pMenu->uiStringId);
        }

   //     MenuDraw_MediaType(uwDispID);// 显示媒体

   //    MenuDraw_BatteryStatus(uwDispID);// 显示电池
    }
}



void MenuDrawSubPageR2L(UINT8 uwDispID, PSMENUSTRUCT pMenu)
{
    UINT32  iCurPage, iTotalPage;

    if( pMenu->iNumOfItems > SUB_MENU_PAGE_ITEM - 2 )
    {
        // Up/Down Icons
        MenuDrawSubButtons(uwDispID);

        MenuDraw_GetPageInfo(&iCurPage, &iTotalPage, NULL, SUB_MENU_PAGE_ITEM);

        MenuDrawPageInfo(uwDispID, iCurPage, iTotalPage);
    }
}

void MenuDrawSubPageR3(UINT8 uwDispID, PSMENUSTRUCT pMenu)
{
    if (GetSubMenuType()==SUBMENU_SUBPAGE)
    {
        if(pMenu->uiStringDescription != NULL)
        {
            MenuDrawSubMenuDescription(uwDispID, pMenu->uiStringDescription);
        }
    }
}
// unsigned char  fm_si47xx_get_mode(void);
 extern unsigned char work_mode;

//extern PowerUpFlag PowerOnFlag;// lyj 20190605
void MenuDrawSubPage(PSMENUSTRUCT pMenu)
{
    UINT8   bID0 = 0, bID1 = 0;
	extern AHC_BOOL     BlueStause; // lyj 20190427
#ifdef FLM_GPIO_NUM
    AHC_OSDSetActive(0, 0);
    AHC_OS_SleepMs(100);
#endif

#ifdef SLIDE_STRING
    StopSlideString();
#endif

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    OSDDraw_ClearOvlDrawBuffer(bID0);

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
        MenuDrawSubBackCurtain(bID0, TV_MENU_BK_COLOR);
    else
        MenuDrawSubBackCurtain(bID0, MENU_BACKGROUND_COLOR);

    // Title, Battery, Media Type
    //OSDDraw_EnableSemiTransparent(bID0, AHC_TRUE);
	//OSDDraw_EnableSemiTransparent(bID1, AHC_TRUE);
    
    //Uwdraw = bID0;// lyj
   // Uwdrawx = bID1;
    MenuDrawSubPageR1(bID0, pMenu);
    // Up/Down Icons, Page Info.
//    MenuDrawSubPageR2L(bID0, pMenu);

    DrawSubItemEx(bID0, pMenu);
    // Draw Sub-Menu Description
    MenuDrawSubPageR3(bID0, pMenu);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);

	if(Menu_Get_Page() == 1)
	{
		//if(volume_M == 0)
		CloseTheUSBPower();// lyj 20190226
		if(work_mode == 1 || work_mode == 100)// lyj 20190213
		fm_si47xx_init();
		if(MenuSettingConfig()->uiPowerUpFlag ==0)
		converseChannel(0xF4);
		if(BlueStause == 1)
		Bluetooth_LEdoff(); // lyj 20190427
		//PowerOnFlag.Fm = 1; // lyj 20190605
		MenuSettingConfig()->uiPowerUpFlag = 1;
	}
	else if(Menu_Get_Page() == 2)
	{
		CloseTheUSBPower();// lyj 20190226
		//if(volume_M == 0)
		if(work_mode == 0 || work_mode == 100)// lyj 20190213
		am_si47xx_init();
		if(MenuSettingConfig()->uiPowerUpFlag ==0)
		converseChannel(0xF5);
		if(BlueStause == 1)
		Bluetooth_LEdoff(); // lyj 20190427
		MenuSettingConfig()->uiPowerUpFlag = 2;// lyj 20190606
	}
	else if(Menu_Get_Page() == 6)
	{
		//SetPrevNextPin(1); // lyj 20190530
		if(MenuSettingConfig()->uiPowerUpFlag ==0)
		//{
			converseChannel(0xF3);
			//AHC_OS_SleepMs(5);
			//volume_conver_size(MAINVOICE,(int)MenuSettingConfig()->uiEV);
		//}
		if(BlueStause == 1)
		Bluetooth_LEdoff(); // lyj 20190427
		MenuSettingConfig()->uiPowerUpFlag = 4;// lyj 20190606
		if(PowerOnVoiceMode == 0) // lyj 20190711
			PowerOnVoiceMode = 6;
		//Set_flag(2);
				//BD3490_init();// long 4-27
		//MMPF_OS_SleepMs(100); volume_conver_size(MAINVOICE,(int)MenuSettingConfig()->uiEV);
		//SetPrevNextPin(1); // lyj 20190626 uart_to_mcu_voice(MAINVOICE,0x00,MUTE_ON);
		//volume_conver_size(MAINVOICE,(int)MenuSettingConfig()->uiEV);
	}
	else if(Menu_Get_Page() == 0)
	{
		SetPrevNextPin(0);
		if(MenuSettingConfig()->uiPowerUpFlag ==0)
		converseChannel(0xF2);
		Bluetooth_LEd(); // lyj 20190423
		MenuSettingConfig()->uiPowerUpFlag = 5;// lyj 20190606
	}
	else if(Menu_Get_Page() == 5)
	{
		CloseTheUSBPower();// lyj 20190226
		if(MenuSettingConfig()->uiPowerUpFlag ==0)
		converseChannel(0xF1);
		if(BlueStause == 1)
		Bluetooth_LEdoff(); // lyj 20190427
		MenuSettingConfig()->uiPowerUpFlag = 3;// lyj 20190606
	}
}


//=====================lyj==========

void MenuDrawSubPage1(PSMENUSTRUCT pMenu)
{
    UINT8   bID0 = 0, bID1 = 0;


    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    OSDDraw_ClearOvlDrawBuffer(bID0);

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
        MenuDrawSubBackCurtain(bID0, TV_MENU_BK_COLOR);
    else
        MenuDrawSubBackCurtain(bID0, MENU_BACKGROUND_COLOR);

    // Title, Battery, Media Type
    MenuDrawSubPageR4(bID0, pMenu);
    // Up/Down Icons, Page Info.
//    MenuDrawSubPageR2L(bID0, pMenu);

    //DrawSubItemEx(bID0, pMenu);
    // Draw Sub-Menu Description
    //MenuDrawSubPageR3(bID0, pMenu);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}



void MenuDrawChangeSubItem(PSMENUSTRUCT pMenu, UINT32 uiCurrItem, UINT32 uiPrevItem, UINT32 uiPreSelected)
{
    UINT8     bID0 = 0;
    UINT8     bID1 = 0;
    UINT8     SubPageItem = SUB_MENU_PAGE_ITEM;
    UINT8     uLang;
	GUI_BITMAP Bicon;
    #if defined(FONT_LARGE)
    SubPageItem = SUB_MENU_PAGE_ITEM;
    #else   // FONT_LARGE
    if ( pMenu->iNumOfItems <= 2 )
        SubPageItem = 2;
    else if ( pMenu->iNumOfItems <= 4 )
		//=======================lyj
        SubPageItem = 4;
    else
        SubPageItem = SUB_MENU_PAGE_ITEM;
    #endif  // FONT_LARGE

    uLang = MenuDraw_GetDefaultLanguage(pMenu);
	printc("~~uiCurrItem = %d ~uiPrevItem = %d~ uiPreSelected = %d~\r\n",uiCurrItem,uiPrevItem,uiPreSelected);

    if( !IS_SAME_PAGE( uiCurrItem, uiPrevItem, SubPageItem ) )
    {
        MenuDrawSubPage(pMenu);
    }
    else
    {
        GUI_COLOR   clrFill     = OSD_COLOR_TRANSPARENT;
        GUI_COLOR   clrSelect   = 0x0;
        GUI_COLOR   clrText     = MENU_TEXT_COLOR;
        UINT32      idx;
        UINT32      barState;
        UINT32      barType     = BAR_NORMAL_TYPE;

        OSDDraw_EnterMenuDrawing(&bID0, &bID1);


        BAR_TYPE(barType, pMenu); // Set BAR Type for Normal, Large or Small to show, the BAR TYPE is for different string length on it.

        //Draw Previous
        {
     printc("----11---------pMenu->iSelected=%d------uiCurrItem=%d------pMenu->pfMenuGetDefaultVal(pMenu)=%d----\r\n",pMenu->iSelected,uiCurrItem,pMenu->pfMenuGetDefaultVal(pMenu));
            if( pMenu->iSelected == uiPrevItem )
                clrSelect = OSD_COLOR_SELECTED;
            else
                clrSelect = 0;

            barState = BAR_STATE_WHT;

            if(( pMenu->uiStatus == MENU_STATUS_PRESSED ) || ( uiPrevItem != pMenu->pfMenuGetDefaultVal(pMenu)))
                barState = BAR_STATE_WHT;
            else if ( uiPrevItem == pMenu->pfMenuGetDefaultVal(pMenu))
                barState = BAR_STATE_WHT_DEF;

            idx = uiPrevItem;
		//--------------------
		//printc("~~~2019~~~~Menu_Get_Page = %d~~FM==~~%d~~~\n",Menu_Get_Page(),MenuSettingConfig()->uiMOVQuality);
		 switch(Menu_Get_Page())
		 	{
		 	case 1: 
				switch(uiPrevItem)
				 {
					case 0:   Bicon=bmFM1_nor;    break;
					case 1:   Bicon=bmFM2_nor ;    break;
					case 2:   Bicon=bmFM3_nor ;    break;
					case 3:   Bicon=bmSCAN_FM_nor;    break;

					#if 1 // lyj 20190226
					case 4:   Bicon=bmvol1 ;    break;
					case 5:   Bicon=bmvol2;    break;
					#endif
				}  

				idx = 7; 			
				MenuSettingConfig()->uiMOVQuality = uiCurrItem;
			break;

			case 2: 
				switch(uiPrevItem)
				 {
					case 0:   Bicon=bmAM1_nor ;    break;
					case 1:   Bicon=bmAM2_nor ;    break;
					case 2:   Bicon=bmAM3_nor ;    break;
					case 3:   Bicon=bmSCAN_nor;    break;
				}  
				idx = 7;		
				MenuSettingConfig()->uiBeep = uiCurrItem;
			break;
		 	//case 5 :  Bicon = bmSub_Light_Bar;  idx = 7;   break;
			case 4 :  Bicon = bmSub_bar_nor;  idx = 7;   

						MenuSettingConfig()->uiIMGSize = uiCurrItem;
			break;
			
			//case 8 :  Bicon = bmSub_RGB;  idx = 7;   break;
			case 7 :  
				switch(uiPrevItem)
				 {
					case 0:   Bicon=bmON_APP_nor ;    break;
					case 1:   Bicon=bmColor_nor ;    break;
					case 2:   Bicon=bmSpeed_nor ;    break;
					case 3:   Bicon=bmFlash_nor ;    break;
					//case 4:   Bicon=bmLight_Show_nor ;    break;
					//case 5:   Bicon=bmON_APP_nor ;    break;
				}  
					idx = 7;   
					MenuSettingConfig()->uiMOVClipTime = uiCurrItem;
					break;
			
			case 3 :  MenuSettingConfig()->uiVMDRecTime = uiCurrItem;   break;

		 }

		  switch(Menu_Get_Page())
		  {
		  	case 1:
			case 2:
			case 3:
			case 4:
			case 7:
				//volume_M = 0;
				//volume_B  = 0;
				sub_flag_ex = 0;
				break;

		  }
		 //------------------------
            DrawSubItem(bID0, idx, pMenu->iNumOfItems,
                        (UINT8)uiPrevItem/*uLang*/, pMenu->pItemsList[uiPrevItem]->uiStringId,
                        Bicon/*pMenu->pItemsList[uiPrevItem]->bmpIcon*/,
                        MAKE_BAR_ID(barType, barState),
                        clrText, clrFill, clrSelect);
        }
#if 0
        //Draw Previous Selected
        if(uiPreSelected / SubPageItem == uiCurrItem / SubPageItem)
        {

		printc("~uiPreSelected = %d~ uiCurrItem = %d~ SubPageItem = %d\r\n",uiPreSelected,uiCurrItem,SubPageItem);
            if( pMenu->iSelected == uiPreSelected )
                clrSelect = OSD_COLOR_SELECTED;
            else
                clrSelect = 0;

            idx = uiPreSelected;

            if(( pMenu->uiStatus == MENU_STATUS_PRESSED ) || ( uiPreSelected != pMenu->pfMenuGetDefaultVal(pMenu)))
                barState = BAR_STATE_WHT;
            else if( uiPreSelected == pMenu->pfMenuGetDefaultVal(pMenu))
                barState = BAR_STATE_WHT_DEF;

            DrawSubItem(bID0, idx, pMenu->iNumOfItems,
                        uLang, pMenu->pItemsList[uiPreSelected]->uiStringId,
                        Bicon/*pMenu->pItemsList[uiPrevItem]->bmpIcon*/,
                       MAKE_BAR_ID(barType, barState),
                        clrText, clrFill, clrSelect);

			printc("~~~~~lyjaaaaaaaa~aa~~~~~\r\n");
        }
#endif
        //Draw current
        {
        printc("---22----------pMenu->iSelected=%d------uiCurrItem=%d------pMenu->pfMenuGetDefaultVal(pMenu)=%d----\r\n",pMenu->iSelected,uiCurrItem,pMenu->pfMenuGetDefaultVal(pMenu));
	if(Menu_Get_Page()==7)// lyj
	{
			 idx = uiPreSelected;
		 switch(pMenu->iCurrentPos)
            	{
			case 0:   Bicon=bmON_APP ;    break;
			case 1:   Bicon=bmColor ;    break;
			case 2:   Bicon=bmSpeed ;    break;
			case 3:   Bicon=bmFlash ;    break;
		//	case 4:   Bicon=bmLight_Show ;    break;
			//case 5:   Bicon=bmON_APP ;    break;
		}
	}
	else if(Menu_Get_Page()==4)// lyj
	{
		 idx = uiPreSelected;
		Bicon=bmSub_bar ; 

	}
	else if(Menu_Get_Page()==3)
		 idx = uiCurrItem;

	#if 1
	else if(Menu_Get_Page()==1)
		{
		 idx = uiPreSelected;
		switch(pMenu->iCurrentPos)
		{
			case 0: Bicon = bmFM1; 

			//Draw_FM_AM_icon(9050);
			break;

			case 1: Bicon = bmFM2;
			 //Draw_FM_AM_icon(9050);
			 break;

			case 2: Bicon = bmFM3; 
			//Draw_FM_AM_icon(9050);
			break;

			case 3: Bicon = bmSCAN;
			//Draw_FM_AM_icon(8750);
			break;

			#if 1
			case 4: Bicon = bmvol3; 
			//Draw_FM_AM_icon(9050);
			break;

			case 5: Bicon = bmvol4;
			//Draw_FM_AM_icon(8750);
			break;

			#endif

			//case 0: Bicon = ;	break;
			
		}
		

	}

	 else if (Menu_Get_Page()==2)
	 {
	 	 idx = uiPreSelected;
		switch(pMenu->iCurrentPos)
			{

				case 0: Bicon = bmAM1; break;
				case 1: Bicon = bmAM2; break;
				case 2: Bicon = bmAM3;  break;

			    case 3: Bicon = bmSCAN1; break;

		}
	 }
	
	#endif

	printc("~~lyj~~666666666666666~~\r\n");

/*
            if( pMenu->iSelected == uiCurrItem )
                clrSelect = OSD_COLOR_SELECTED;// 选定
            else
                clrSelect = 0;

            idx = uiCurrItem;

            if( uiCurrItem == pMenu->pfMenuGetDefaultVal(pMenu))
                barState = BAR_STATE_YEL_DEF;
            else
                barState = BAR_STATE_YEL;
		*/
            DrawSubItem(bID0, idx, pMenu->iNumOfItems,
                        uLang, pMenu->iCurrentPos/*pMenu->pItemsList[i]->uiStringId*/,
                        Bicon/*pMenu->pItemsList[uiPrevItem]->bmpIcon*/,
                        MAKE_BAR_ID(barType, barState),
                        clrText, clrFill, clrSelect);
        }

		if(Menu_Get_Page() ==1)
		{
			switch(MenuSettingConfig()->uiMOVQuality)
			{
				case 1:
				case 2:	
				case 0:
					Reserved_radio(radio,bID0,0);// lyj 20190606
					break;
				default:
					break;

			}

			

		}
		else if(Menu_Get_Page() ==2)
		{
			switch(MenuSettingConfig()->uiBeep)
			{

				case 1:
				case 2:	
				
					Reserved_radio(radio_am,bID0,0);// lyj 20190606
					break;
				default:
					break;

			}

		}

        OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    }
}



//===============lyj===========

void MenuDrawChangeSubItem1(PSMENUSTRUCT pMenu, UINT32 uiCurrItem, UINT32 uiPrevItem, UINT32 uiPreSelected)
{
    UINT8     bID0 = 0;
    UINT8     bID1 = 0;
    UINT8     SubPageItem = SUB_MENU_PAGE_ITEM;
    UINT8     uLang;
	GUI_BITMAP Bicon;
    #if defined(FONT_LARGE)
    SubPageItem = SUB_MENU_PAGE_ITEM;
    #else   // FONT_LARGE
    if ( pMenu->iNumOfItems <= 2 )
        SubPageItem = 2;
    else if ( pMenu->iNumOfItems <= 4 )
        SubPageItem = 4;
    else
        SubPageItem = SUB_MENU_PAGE_ITEM;
    #endif  // FONT_LARGE

    uLang = MenuDraw_GetDefaultLanguage(pMenu);
	printc("~~uiCurrItem = %d ~uiPrevItem = %d~ uiPreSelected = %d~\r\n",uiCurrItem,uiPrevItem,uiPreSelected);



    {
        GUI_COLOR   clrFill     = OSD_COLOR_TRANSPARENT;
        GUI_COLOR   clrSelect   = 0x0;
        GUI_COLOR   clrText     = MENU_TEXT_COLOR;
        UINT32      idx;
        UINT32      barState;
        UINT32      barType     = BAR_NORMAL_TYPE;

        OSDDraw_EnterMenuDrawing(&bID0, &bID1);


        BAR_TYPE(barType, pMenu); // Set BAR Type for Normal, Large or Small to show, the BAR TYPE is for different string length on it.

        //Draw Previous
        {
     printc("----11---------pMenu->iSelected=%d------uiCurrItem=%d------pMenu->pfMenuGetDefaultVal(pMenu)=%d----\r\n",pMenu->iSelected,uiCurrItem);
            if( pMenu->iSelected == uiPrevItem )
                clrSelect = OSD_COLOR_SELECTED;
            else
                clrSelect = 0;

            barState = BAR_STATE_WHT;

            if(( pMenu->uiStatus == MENU_STATUS_PRESSED ) || ( uiPrevItem != pMenu->pfMenuGetDefaultVal(pMenu)))
                barState = BAR_STATE_WHT;
            else if ( uiPrevItem == pMenu->pfMenuGetDefaultVal(pMenu))
                barState = BAR_STATE_WHT_DEF;

            idx = uiPrevItem;
		//--------------------
	
		 //------------------------
		 #if 0
            DrawSubItem(bID0, idx, pMenu->iNumOfItems,
                        uLang, pMenu->pItemsList[uiPrevItem]->uiStringId,
                        Bicon/*pMenu->pItemsList[uiPrevItem]->bmpIcon*/,
                        MAKE_BAR_ID(barType, barState),
                        clrText, clrFill, clrSelect);

		 #endif
        }
#if 1
        //Draw Previous Selected
   
#endif
        //Draw current
        {
        printc("---22----------pMenu->iSelected=%d------uiCurrItem=%d------pMenu->pfMenuGetDefaultVal(pMenu)=%d----\r\n",pMenu->iSelected,uiCurrItem,pMenu->pfMenuGetDefaultVal(pMenu));

		 idx = uiCurrItem;

	printc("~~lyj~~666666666666666~~\r\n");

/*
            if( pMenu->iSelected == uiCurrItem )
                clrSelect = OSD_COLOR_SELECTED;// 选定
            else
                clrSelect = 0;

            idx = uiCurrItem;

            if( uiCurrItem == pMenu->pfMenuGetDefaultVal(pMenu))
                barState = BAR_STATE_YEL_DEF;
            else
                barState = BAR_STATE_YEL;  
		*/
            DrawSubItem(bID0, idx, pMenu->iNumOfItems,
                        uLang, pMenu->iCurrentPos/*pMenu->pItemsList[i]->uiStringId*/,
                        Bicon/*pMenu->pItemsList[uiPrevItem]->bmpIcon*/,
                        MAKE_BAR_ID(barType, barState),
                        clrText, clrFill, clrSelect);
        }

        OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    }
}

void MenuDrawChangeSubItem3(PSMENUSTRUCT pMenu, UINT32 uiCurrItem, UINT32 uiPrevItem, UINT32 uiPreSelected)
{
    UINT8     bID0 = 0;
    UINT8     bID1 = 0;
    UINT8     SubPageItem = SUB_MENU_PAGE_ITEM;
    UINT8     uLang;
	GUI_BITMAP Bicon;
    #if defined(FONT_LARGE)
    SubPageItem = SUB_MENU_PAGE_ITEM;
    #else   // FONT_LARGE
    if ( pMenu->iNumOfItems <= 2 )
        SubPageItem = 2;
    else if ( pMenu->iNumOfItems <= 4 )
		//=======================lyj
        SubPageItem = 4;
    else
        SubPageItem = SUB_MENU_PAGE_ITEM;
    #endif  // FONT_LARGE

    uLang = 4;//MenuDraw_GetDefaultLanguage(pMenu);
	printc("~~uiCurrItem = %d ~uiPrevItem = %d~ uiPreSelected = %d~\r\n",uiCurrItem,uiPrevItem,uiPreSelected);


    {
        GUI_COLOR   clrFill     = OSD_COLOR_TRANSPARENT;
        GUI_COLOR   clrSelect   = 0x0;
        GUI_COLOR   clrText     = MENU_TEXT_COLOR;
        UINT32      idx;
        UINT32      barState;
        UINT32      barType     = BAR_NORMAL_TYPE;

        OSDDraw_EnterMenuDrawing(&bID0, &bID1);


        BAR_TYPE(barType, pMenu); // Set BAR Type for Normal, Large or Small to show, the BAR TYPE is for different string length on it.

        //Draw Previous
        #if 0 // lyj 20190216
        {
     printc("----11---------pMenu->iSelected=%d------uiCurrItem=%d------pMenu->pfMenuGetDefaultVal(pMenu)=%d----\r\n",pMenu->iSelected,uiCurrItem);
            if( pMenu->iSelected == uiPrevItem )
                clrSelect = OSD_COLOR_SELECTED;
            else
                clrSelect = 0;

            barState = BAR_STATE_WHT;

            if(( pMenu->uiStatus == MENU_STATUS_PRESSED ) || ( uiPrevItem != pMenu->pfMenuGetDefaultVal(pMenu)))
                barState = BAR_STATE_WHT;
            else if ( uiPrevItem == pMenu->pfMenuGetDefaultVal(pMenu))
                barState = BAR_STATE_WHT_DEF;

            idx = uiPrevItem;
		//--------------------


        }
		#endif

        //Draw current
        {
      //  printc("---22----------pMenu->iSelected=%d------uiCurrItem=%d------pMenu->pfMenuGetDefaultVal(pMenu)=%d----\r\n",pMenu->iSelected,uiCurrItem,pMenu->pfMenuGetDefaultVal(pMenu));
 if(Menu_Get_Page()==3)
		 idx = uiCurrItem;

	//printc("~~lyj~~666666666666666~~\r\n");

/*
            if( pMenu->iSelected == uiCurrItem )
                clrSelect = OSD_COLOR_SELECTED;// 选定
            else
                clrSelect = 0;

            idx = uiCurrItem;

            if( uiCurrItem == pMenu->pfMenuGetDefaultVal(pMenu))
                barState = BAR_STATE_YEL_DEF;
            else
                barState = BAR_STATE_YEL;
		*/
				Bicon = bmvolform;



			

			//printc("~~~~pMenu->iNumOfItems = %d~~~pMenu->iCurrentPos = %d ~~\r\n",pMenu->iNumOfItems,pMenu->iCurrentPos);
            DrawSubItem1(bID0, idx, pMenu->iNumOfItems,
                        uLang, pMenu->iCurrentPos/*pMenu->pItemsList[i]->uiStringId*/,
                        Bicon/*pMenu->pItemsList[uiPrevItem]->bmpIcon*/,
                        MAKE_BAR_ID(barType, barState),
                        clrText, clrFill, clrSelect);
        }

        OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    }
}






//================end===========

#if 0
void ________ConfirmMenu_Function_________(){ruturn;} //dummy
#endif

void DrawSubItem_ConfirmPage(UINT16 uwDispID, UINT16 uwMenuID, int iItem, int iTotalItems, UINT32 iStrID, const GUI_BITMAP* IconID, int bid, GUI_COLOR clrFont, GUI_COLOR clrBack, GUI_COLOR clrSelect)
{
    RECT        rc, tmpRECT;
    RECT        RECTStr     = POS_MENU_CONFIRM;
    INT16       wTextAlign  = GUI_TA_LEFT|GUI_TA_VCENTER;
    GUI_COLOR   TextBkColor;

    #define STR_GAP     (30)

    TextBkColor = OSD_COLOR_DARKGRAY2;

    rc.uiWidth  = BAR(bid).XSize;
    rc.uiHeight = BAR(bid).YSize;
    GetSubItemRect(uwDispID, iItem%CONFIRM_MENU_PAGE_ITEM, CONFIRM_MENU_PAGE_ITEM, &rc );

    RECTStr.uiWidth = AHC_GET_ATTR_OSD_W(uwDispID) - RECTStr.uiLeft;

    switch(uwMenuID)
    {
        case MENUID_SUB_MENU_RESET_SETUP:
            OSD_ShowStringPool(uwDispID,IDS_DS_RESET_SETUP_CONFIRM, RECTStr, clrFont, TextBkColor, wTextAlign);
            RECTStr.uiTop += STR_GAP;
            OSD_ShowStringPool(uwDispID,IDS_DS_RESET_INFO,          RECTStr, clrFont, TextBkColor, wTextAlign);
        break;

        case MENUID_SUB_MENU_FORMAT_SD_CARD:
            OSD_ShowStringPool(uwDispID,IDS_DS_FORMAT_CARD_CONFIRM, RECTStr, clrFont, TextBkColor, wTextAlign);
            RECTStr.uiTop += STR_GAP;
            OSD_ShowStringPool(uwDispID,IDS_DS_DATA_DELETED,        RECTStr, clrFont, TextBkColor, wTextAlign);
        break;

        #if (MENU_MEDIA_FORMAT_INT_MEM_EN)
        case MENUID_SUB_MENU_FORMAT_INTMEM:
            OSD_ShowStringPool(uwDispID,IDS_DS_FORMAT_INTMEM_CONFIRM, RECTStr, clrFont, TextBkColor, wTextAlign);
            RECTStr.uiTop += STR_GAP;
            OSD_ShowStringPool(uwDispID,IDS_DS_DATA_DELETED,          RECTStr, clrFont, TextBkColor, wTextAlign);
        break;
        #endif

        case MENUID_SUB_MENU_DELETE_ALL_VIDEO:
        case MENUID_SUB_MENU_DELETE_ALL_IMAGE:
            OSD_ShowStringPool(uwDispID,IDS_DS_DELETE_ALL_CONFIRM, RECTStr, clrFont, TextBkColor, wTextAlign);
        break;

        case MENUID_SUB_MENU_PROTECT_ALL_VIDEO:
        case MENUID_SUB_MENU_PROTECT_ALL_IMAGE:
            OSD_ShowStringPool(uwDispID,IDS_DS_PROTECT_ALL_CONFIRM, RECTStr, clrFont, TextBkColor, wTextAlign);
        break;

        case MENUID_SUB_MENU_UNPROTECT_ALL_VIDEO:
        case MENUID_SUB_MENU_UNPROTECT_ALL_IMAGE:
            OSD_ShowStringPool(uwDispID,IDS_DS_UNPROTECT_ALL, RECTStr, clrFont, TextBkColor, wTextAlign);
        break;
    }

    rc.uiTop = RECTStr.uiTop + 50;

    if( clrSelect != 0 )
    {
        AHC_OSDSetColor(uwDispID, clrSelect);
        AHC_OSDDrawFillRoundedRect(uwDispID, rc.uiLeft+2, rc.uiTop+rc.uiHeight-7, rc.uiLeft+rc.uiWidth-2, rc.uiTop+rc.uiHeight-2, RECT_ROUND_RADIUS);
    }

    OSD_Draw_Icon(BAR(bid), rc, uwDispID);

    tmpRECT.uiLeft   = rc.uiLeft    + STR_RECT_OFFSET_X;
    tmpRECT.uiTop    = rc.uiTop     + STR_RECT_OFFSET_Y;
    tmpRECT.uiWidth  = rc.uiWidth   + STR_RECT_OFFSET_W;
    tmpRECT.uiHeight = rc.uiHeight  + STR_RECT_OFFSET_H;
    OSD_ShowStringPool(uwDispID, iStrID, tmpRECT, clrFont, clrBack, GUI_TA_HCENTER|GUI_TA_VCENTER);
}

void MenuDrawSubPage_ConfirmPage(PSMENUSTRUCT pMenu)
{
    UINT8  bID0 = 0;
    UINT8  bID1 = 0;
    UINT32 i, iBegin, iEnd;

#ifdef SLIDE_STRING
    StopSlideString();
#endif

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    iBegin = ALIGN_DOWN( pMenu->iCurrentPos, CONFIRM_MENU_PAGE_ITEM );
    iEnd   = MIN( iBegin+CONFIRM_MENU_PAGE_ITEM, pMenu->iNumOfItems);

    OSDDraw_ClearOvlDrawBuffer(bID0);
    MenuDrawSubBackCurtain(bID0, OSD_COLOR_DARKGRAY2);

    MenuDrawSubPageR1(bID0, pMenu);

    for( i=iBegin; i<iEnd; i++ )
    {
        GUI_COLOR   colorBk = OSD_COLOR_TRANSPARENT;
        int         bid     = MAKE_BAR_ID(BAR_NARROW_TYPE, BAR_STATE_WHT);

        if( i == pMenu->iCurrentPos )
        {
            bid = MAKE_BAR_ID(BAR_NARROW_TYPE, BAR_STATE_YEL);
        }
        DrawSubItem_ConfirmPage(bID0, pMenu->iMenuId, i, pMenu->iNumOfItems, pMenu->pItemsList[i]->uiStringId, pMenu->pItemsList[i]->bmpIcon, bid, MENU_TEXT_COLOR, colorBk, 0x0);
    }

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}

void MenuDrawChangeSubItem_ConfirmPage(PSMENUSTRUCT pMenu, UINT32 uiCurrItem, UINT32 uiPrevItem, UINT32 uiPreSelected)
{
    UINT8     bID0 = 0;
    UINT8     bID1 = 0;
    GUI_COLOR clrFill   = OSD_COLOR_TRANSPARENT;
    GUI_COLOR clrSelect = 0x0;

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    if( pMenu->iSelected == uiPrevItem )
        clrSelect = OSD_COLOR_SELECTED;
    else
        clrSelect = 0;

    //Draw Previous
    DrawSubItem_ConfirmPage(bID0, pMenu->iMenuId,
                            uiPrevItem, pMenu->iNumOfItems,
                            pMenu->pItemsList[uiPrevItem]->uiStringId,
                            pMenu->pItemsList[uiPrevItem]->bmpIcon,
                            MAKE_BAR_ID(BAR_NARROW_TYPE, BAR_STATE_WHT),
                            MENU_TEXT_COLOR, clrFill, clrSelect);

    if( pMenu->iSelected == uiCurrItem )
        clrSelect = OSD_COLOR_SELECTED;
    else
        clrSelect = 0;

    //Draw current
    DrawSubItem_ConfirmPage(bID0, pMenu->iMenuId,
                            uiCurrItem, pMenu->iNumOfItems,
                            pMenu->pItemsList[uiCurrItem]->uiStringId,
                            pMenu->pItemsList[uiCurrItem]->bmpIcon,
                            MAKE_BAR_ID(BAR_NARROW_TYPE, BAR_STATE_YEL),
                            MENU_TEXT_COLOR, clrFill, clrSelect);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}

#if 0
void ________ModeMenu_Function_________(){ruturn;} //dummy
#endif

#if (USB_MODE_SELECT_EN)  

void GetSubItemRect_ModeSelect(int i, int iItems, RECT* pRc, int drawStyle)
{
    if(drawStyle==0 /* Text */)
    {
        int x0 = 40;
        int w  = DISP_WIDTH - (x0*2);
        int h  = 30;
        int offset;

        if( iItems == 2 )
        {
            offset = 70;

            SET_SUBRECT(pRc, x0, 70+i*offset, w, h);
        }
        else if( iItems == 3 )
        {
            offset = 50;

            SET_SUBRECT(pRc, x0, 55+i*offset, w, h);
        }
        else if( iItems == 4 )
        {
            offset = 40;

            SET_SUBRECT(pRc, x0, 45+i*offset, w, h);
        }
        else
        {
            offset = 30;

            SET_SUBRECT(pRc, x0, 35+i*offset, w, h);
        }
    }
    else if(drawStyle==1 /* Icon */)
    {
        UINT32 IconGap = (DISP_WIDTH - (pRc->uiWidth * iItems))/(iItems+1);

        if( iItems <= 2 )
        {
            switch(i)
            {
                case 0: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 2 - IconGap * 2), 85, pRc->uiWidth, pRc->uiHeight); break;}
                case 1: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 1 - IconGap * 1), 85, pRc->uiWidth, pRc->uiHeight); break;}
                default:{SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 2 - IconGap * 2), 85, pRc->uiWidth, pRc->uiHeight); break;}
            }
        }
        else if( iItems <= 3 )
        {
            switch(i)
            {
                case 0: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 3 - IconGap * 3), 100,  pRc->uiWidth, pRc->uiHeight); break;}
                case 1: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 2 - IconGap * 2), 100,  pRc->uiWidth, pRc->uiHeight); break;}
                case 2: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 1 - IconGap * 1), 100,  pRc->uiWidth, pRc->uiHeight); break;}
                default:{SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 2 - IconGap * 3), 100,  pRc->uiWidth, pRc->uiHeight); break;}
            }
        }
        else if( iItems <= 4 )
        {
            switch(i)
            {
                case 0: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 2 - IconGap * 2), 80,  pRc->uiWidth, pRc->uiHeight); break;}
                case 1: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 1 - IconGap * 2), 80,  pRc->uiWidth, pRc->uiHeight); break;}
                case 2: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 2 - IconGap * 2), 140, pRc->uiWidth, pRc->uiHeight); break;}
                case 3: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 1 - IconGap * 2), 140, pRc->uiWidth, pRc->uiHeight); break;}
                default:{SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 2 - IconGap * 2), 80,  pRc->uiWidth, pRc->uiHeight); break;}
            }
        }
        else
        {
            switch(i)
            {
                case 0: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 3 - IconGap * 3), 80,  pRc->uiWidth, pRc->uiHeight); break;}
                case 1: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 2 - IconGap * 2), 80,  pRc->uiWidth, pRc->uiHeight); break;}
                case 2: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 1 - IconGap * 1), 80,  pRc->uiWidth, pRc->uiHeight); break;}
                case 3: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 3 - IconGap * 3), 140, pRc->uiWidth, pRc->uiHeight); break;}
                case 4: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 2 - IconGap * 2), 140, pRc->uiWidth, pRc->uiHeight); break;}
                case 5: {SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 1 - IconGap * 1), 140, pRc->uiWidth, pRc->uiHeight); break;}
                default:{SET_SUBRECT(pRc, (DISP_WIDTH - pRc->uiWidth * 2 - IconGap * 3), 80,  pRc->uiWidth, pRc->uiHeight); break;}
            }
        }
    }
}

void DrawSubItem_ModeSelect(UINT16 uwDispID, int iItem, int iTotalItems, int iCurPos, UINT32 iStrID, const GUI_BITMAP* IconID, GUI_BITMAP barID, GUI_COLOR clrFont, GUI_COLOR clrBack, int drawStyle)
{
    RECT        rc;
    GUI_COLOR   bkClr;

    if(drawStyle==0 /* Text */)
    {
        GetSubItemRect_ModeSelect( iItem%SUB_MENU_PAGE_ITEM, iTotalItems, &rc, drawStyle);

        bkClr = (iItem == iCurPos)?(OSD_COLOR_DARKYELLOW):(OSD_COLOR_DARKGRAY2);

        AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);
        AHC_OSDSetPenSize(uwDispID, 2);
        AHC_OSDDrawRoundedRect(uwDispID, rc.uiLeft-1, rc.uiTop-1, rc.uiLeft+rc.uiWidth+1, rc.uiTop+rc.uiHeight+1, RECT_ROUND_RADIUS);

        AHC_OSDSetColor(uwDispID, bkClr);
        AHC_OSDDrawFillRoundedRect(uwDispID, rc.uiLeft, rc.uiTop+2, rc.uiLeft+rc.uiWidth, rc.uiTop+rc.uiHeight-2, RECT_ROUND_RADIUS);

        OSD_ShowStringPool(uwDispID, iStrID, rc, clrFont, bkClr, GUI_TA_HCENTER|GUI_TA_VCENTER);
    }
    else if(drawStyle==1 /* Icon */)
    {
        if(IconID != NULL)
        {
            rc.uiWidth  = IconID->XSize;
            rc.uiHeight = IconID->YSize;
            GetSubItemRect_ModeSelect( iItem%SUB_MENU_PAGE_ITEM, iTotalItems, &rc, drawStyle);

            if(iItem == iCurPos)
                AHC_OSDSetColor(uwDispID, OSD_COLOR_YELLOW);
            else
                AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);

            AHC_OSDDrawFillRoundedRect(uwDispID, rc.uiLeft-2, rc.uiTop-2, rc.uiLeft+IconID->XSize+2, rc.uiTop+IconID->YSize+2, 3);

            AHC_OSDSetColor(uwDispID, OSD_COLOR_WHITE);
            AHC_OSDSetBkColor(uwDispID, clrBack);
            AHC_OSDDrawBitmap(uwDispID, IconID, rc.uiLeft, rc.uiTop);

            rc.uiLeft   -= 25;
            rc.uiTop    += IconID->YSize;
            rc.uiWidth   = 25*2 + IconID->XSize;
            rc.uiHeight  = 30;
            OSD_ShowStringPool(uwDispID, iStrID, rc, clrFont, OSD_COLOR_TRANSPARENT, GUI_TA_HCENTER|GUI_TA_VCENTER);
        }
    }
}

void MenuDrawMainPage_ModeSelect(PSMENUSTRUCT pMenu)
{
    UINT8  bID0 = 0 ,bID1 = 0;
    UINT32 i, iBegin, iEnd;

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    #if (USB_MODE_SELECT_EN) && (USB_MODE_SELECT_STYLE==MENU_ORIGINAL)
    if (pMenu->iMenuId==MENUID_USB_MODE_SELECT)
    {
    	
        //OSDDraw_SetAlphaBlending(bID0, AHC_TRUE);//m_OSD[uwDisplayID]->ColorFormat==RGB_D24BIT_RGB565 will not show.
        //OSDDraw_SetAlphaBlending(bID1, AHC_TRUE);

		OSDDraw_EnableSemiTransparent(bID0, AHC_TRUE);//
		OSDDraw_EnableSemiTransparent(bID1, AHC_TRUE);

        OSDDraw_ClearOvlDrawBuffer(bID0);

        MenuMainDrawBackCurtain(bID0, OSD_COLOR_BACKGROUND);

        iBegin = ALIGN_DOWN( pMenu->iCurrentPos, MAIN_MENU_PAGE_ITEM );
        iEnd   = MIN( iBegin+MAIN_MENU_PAGE_ITEM, pMenu->iNumOfItems);

        if (pMenu->iNumOfItems > MAIN_MENU_PAGE_ITEM)
        {
            MenuMainDrawButtons(bID0);
            MenuDrawPageInfo(bID0, 1, 1);
        }

        if( pMenu->uiStringId != -1 )
        {
            MenuDrawTitle(bID0, pMenu->uiStringId);
        }

        for( i=iBegin; i< iEnd; i++ )
        {
            GUI_BITMAP barID = BMICON_MENUBAR_WHITE;

            if( i == pMenu->iCurrentPos )
            {
                barID = BMICON_MENUBAR_YELLOW;
            }

            MenuDrawItem(bID0, pMenu->pItemsList[i],
                         i%MAIN_MENU_PAGE_ITEM,
                         pMenu->pItemsList[i]->uiStringId,
                         barID,
                         MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, pMenu->bCustom);
        }
    }
    else
    #endif
    {
        OSDDraw_SetAlphaBlending(bID0, AHC_FALSE);
        OSDDraw_SetAlphaBlending(bID1, AHC_FALSE);

        OSDDraw_ClearOvlDrawBuffer(bID0);


        iBegin = ALIGN_DOWN( pMenu->iCurrentPos, 6 );
        iEnd   = MIN( iBegin+6, pMenu->iNumOfItems);

        for( i=iBegin; i< iEnd; i++ )
        {
            GUI_COLOR   colorBk = OSD_COLOR_TRANSPARENT;
            GUI_BITMAP  barID   = BMICON_MENUBAR_WHITE;

            DrawSubItem_ModeSelect( bID0, i, pMenu->iNumOfItems, pMenu->iCurrentPos,
                                    pMenu->pItemsList[i]->uiStringId,
                                    pMenu->pItemsList[i]->bmpIcon,
                                    barID, MENU_TEXT_COLOR, colorBk,
                                    ubModeDrawStyle);
        }
    }

    MenuDraw_BatteryStatus(bID0);

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
	AHC_OSDSetActive(bID0, MMP_TRUE);
}

void MenuDrawChangeItem_ModeSelect(PSMENUSTRUCT pMenu, UINT32 uiCurrItem, UINT32 uiPrevItem, UINT32 uiPreSelected)
{
    UINT8   bID0 = 0 , bID1 = 0;

    #if (USB_MODE_SELECT_STYLE==MENU_ORIGINAL) && (USB_MODE_SELECT_EN)
    if (pMenu->iMenuId==MENUID_USB_MODE_SELECT)
    {
        PSMENUITEM      pPrevItem;
        PSMENUITEM      pCurItem;

        OSDDraw_EnterMenuDrawing(&bID0, &bID1);

        pPrevItem    = pMenu->pItemsList[uiPrevItem];
        pCurItem     = pMenu->pItemsList[uiCurrItem];

        MenuDrawItem(bID0, pPrevItem, uiPrevItem%MAIN_MENU_PAGE_ITEM, pPrevItem->uiStringId,
                    BMICON_MENUBAR_WHITE,  MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, pMenu->bCustom);

        MenuDrawItem(bID0, pCurItem,  uiCurrItem%MAIN_MENU_PAGE_ITEM, pCurItem->uiStringId,
                    BMICON_MENUBAR_YELLOW, MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, pMenu->bCustom);

        OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    }
    else
    #endif
    {
        OSDDraw_EnterMenuDrawing(&bID0, &bID1);

        DrawSubItem_ModeSelect( bID0, uiPrevItem%SUB_MENU_PAGE_ITEM, pMenu->iNumOfItems, pMenu->iCurrentPos,
                                pMenu->pItemsList[uiPrevItem]->uiStringId,
                                pMenu->pItemsList[uiPrevItem]->bmpIcon,     BMICON_SUBBAR_WHITE_DEFAULT,
                                MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, ubModeDrawStyle);

        DrawSubItem_ModeSelect( bID0, uiPreSelected,                pMenu->iNumOfItems, pMenu->iCurrentPos,
                                pMenu->pItemsList[uiPreSelected]->uiStringId,
                                pMenu->pItemsList[uiPreSelected]->bmpIcon,  BMICON_SUBBAR_WHITE,
                                MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, ubModeDrawStyle);

        DrawSubItem_ModeSelect( bID0, uiCurrItem,                   pMenu->iNumOfItems, pMenu->iCurrentPos,
                                pMenu->pItemsList[uiCurrItem]->uiStringId,
                                pMenu->pItemsList[uiCurrItem]->bmpIcon,     BMICON_SUBBAR_YELLOW,
                                MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT, ubModeDrawStyle);

        OSDDraw_ExitMenuDrawing(&bID0, &bID1);
    }
}

#endif

#if 0
void ________TopMenu_Function_________(){ruturn;} //dummy
#endif

#if (TOP_MENU_PAGE_EN)

#define XSPACE          (POS_TOP_MENU_W/pMenu->iNumOfItems)
#define XOFFEST(IconW)  (XSPACE-IconW)/2

void DrawTopCatagoryMenuIcon(UINT16 uwDispID, PSMENUSTRUCT pMenu, AHC_BOOL DVHL, AHC_BOOL GeneralHL, AHC_BOOL LangHL)
{
}

void MenuTopDrawCatagory(UINT16 uwDispID, PSMENUSTRUCT pMenu, int  iCata)
{
}

void DrawTopMenuItem(UINT16 uwDispID, PSMENUSTRUCT pMenu, int iItem, UINT32 iStrID, const GUI_BITMAP* IconID, GUI_COLOR clrFont, GUI_COLOR clrBack, GUI_COLOR clrSelect)
{

    UINT32  x, y;

    x = POS_TOP_MENU_X0 + iItem*XSPACE + XOFFEST(IconID->XSize);
    y = POS_TOP_MENU_Y0 + 80;

    if(clrSelect!=0)
    {
        AHC_OSDSetColor(uwDispID, clrSelect);
        AHC_OSDDrawFillRoundedRect(uwDispID, x-3 , y-3, x+IconID->XSize+3, y+IconID->YSize+3, RECT_ROUND_RADIUS);
    }

    if(IconID != NULL)
    {
        AHC_OSDDrawBitmap(uwDispID, IconID, x, y);
    }

    if(iStrID != -1)
    {
        RECT    StrRect = {POS_TOP_MENU_X0, 80, POS_TOP_MENU_W, 30};

        if(iItem == GetCurrentMenu()->iCurrentPos)
        {
            AHC_OSDSetColor(uwDispID, clrBack);
            AHC_OSDDrawFillRect2(uwDispID, &StrRect);

            OSD_ShowStringPool3(uwDispID, iStrID, StrRect,
                                OSD_COLOR_WHITE,  clrBack,
                                GUI_TA_HCENTER|GUI_TA_HCENTER);
        }
    }
}

void MenuDrawTopMenuPage(PSMENUSTRUCT pMenu)
{
    UINT8       bID0 = 0 ,bID1 = 0;
    UINT32      i ,iBegin, iEnd;
    GUI_COLOR   bkColor = TOP_MENU_BK_COLOR;

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {
        OSDDraw_SetAlphaBlending(bID0, AHC_FALSE);
        OSDDraw_SetAlphaBlending(bID1, AHC_FALSE);

        bkColor = TV_MENU_BK_COLOR;
    }
    else
    {
        //OSDDraw_SetAlphaBlending(bID0, AHC_TRUE);
        //OSDDraw_SetAlphaBlending(bID1, AHC_TRUE);
        OSDDraw_EnableSemiTransparent(bID0, AHC_TRUE);
        OSDDraw_EnableSemiTransparent(bID1, AHC_TRUE);
        bkColor = TOP_MENU_BK_COLOR;
    }

    OSDDraw_ClearOvlDrawBuffer(bID0);

    iBegin = ALIGN_DOWN( pMenu->iCurrentPos, pMenu->iNumOfItems/*m_ulTopMenuPageItem*/ );
    iEnd   = MIN( iBegin+pMenu->iNumOfItems/*m_ulTopMenuPageItem*/, pMenu->iNumOfItems);

    AHC_OSDSetColor(bID0, bkColor);
    AHC_OSDDrawFillRect(bID0, POS_TOP_MENU_X0, POS_TOP_MENU_Y0, POS_TOP_MENU_X1, POS_TOP_MENU_Y1);

    for( i=iBegin; i< iEnd; i++ )
    {
        GUI_COLOR clrSelect = TOP_MENU_BK_COLOR;

        if( i == pMenu->iCurrentPos )
        {
            clrSelect = OSD_COLOR_YELLOW;
        }

        DrawTopMenuItem(bID0,
                        pMenu,
                        i%pMenu->iNumOfItems,
                        pMenu->pItemsList[i]->uiStringId,
                        pMenu->pItemsList[i]->bmpIcon,
                        MENU_TEXT_COLOR, bkColor, clrSelect);

    }

    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}

void MenuDrawChangeTopMenuItem(PSMENUSTRUCT pMenu, UINT32 uiCurrItem, UINT32 uiPrevItem, UINT32 uiPreSelected)
{
    UINT8           bID0 = 0 , bID1 = 0;
    PSMENUITEM      pPrevItem;
    PSMENUITEM      pCurItem;
    GUI_COLOR       bkColor = TOP_MENU_BK_COLOR;

    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {
        bkColor = TV_MENU_BK_COLOR;
    }

    pPrevItem = pMenu->pItemsList[uiPrevItem];

    DrawTopMenuItem(bID0, pMenu,
                    uiPrevItem%pMenu->iNumOfItems,
                    pPrevItem->uiStringId,
                    pPrevItem->bmpIcon,
                    MENU_TEXT_COLOR, bkColor, bkColor);

    pCurItem  = pMenu->pItemsList[uiCurrItem];

    DrawTopMenuItem(bID0, pMenu,
                    uiCurrItem%pMenu->iNumOfItems,
                    pCurItem->uiStringId,
                    pCurItem->bmpIcon,
                    MENU_TEXT_COLOR, bkColor, OSD_COLOR_DARKYELLOW);


    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}

/*
 * A gereral drawing touch menu page
 */
void MenuDrawMenuPage(PSMENUSTRUCT pMenu)
{
    UINT8       bID0 = 0 ,bID1 = 0;
    GUI_COLOR   bkColor = TOP_MENU_BK_COLOR;
    int         i;
    SMENUITEM           **pit;

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
    {   // TODO: need review TV/HDMI mode
        return;
    }
    printc(">>> Gereral draw touch menu page %d\r\n", pMenu->iMenuId);
    OSDDraw_EnterMenuDrawing(&bID0, &bID1);
    // Draw Icons on the Menu Page
    OSDDraw_SetAlphaBlending(bID0, AHC_FALSE);
    OSDDraw_SetAlphaBlending(bID1, AHC_FALSE);
    AHC_OSDClearBuffer(bID0);
    AHC_OSDSetColor(bID0, bkColor);//yining
    AHC_OSDDrawFillRect(bID0, 0, 0, AHC_GET_ATTR_OSD_W(bID0), AHC_GET_ATTR_OSD_H(bID0));
    pit = pMenu->pItemsList;
    for (i = 0; pit[i] != NULL && i < pMenu->iNumOfItems; i++) {
        SMENUTOUCHBUTTON    *pmb;

        pmb = pit[i]->ptp;
        /*
        printc("RECT %d %d %d %d\r\n", pmb->rcButton.uiLeft, pmb->rcButton.uiTop,
                                       pmb->rcButton.uiWidth,pmb->rcButton.uiHeight);
        printc("ItemID %d %x\r\n", pit[i]->iItemId, pit[i]->bmpIcon);
        */
        if (pit[i]->bmpIcon) {
            RECT    rc;
            OSD_Draw_IconXY(bID0, *(pit[i]->bmpIcon),
                                    pmb->rcButton.uiLeft,
                                    pmb->rcButton.uiTop);
            // Set Touch button size
            pmb->rcButton.uiWidth  = pit[i]->bmpIcon->XSize;
            pmb->rcButton.uiHeight = pit[i]->bmpIcon->YSize;
            if (pit[i]->uiStringId != (0 - 1)) {
                rc.uiLeft = pmb->rcButton.uiLeft - 10;
                rc.uiTop  = pmb->rcButton.uiTop + pit[i]->bmpIcon->YSize - 5;
                rc.uiWidth  = pmb->rcButton.uiWidth + 30;
                rc.uiHeight = 20;
                OSD_ShowStringPool(bID0, pit[i]->uiStringId,
                                    rc,
                                    MENU_TEXT_COLOR, OSD_COLOR_TRANSPARENT,
                                    GUI_TA_HCENTER|GUI_TA_VCENTER);
            }
        }
    }
    OSDDraw_ExitMenuDrawing(&bID0, &bID1);
}
#endif

//======================lyj=======================
#if 0
void Draw_sub_vicon(int vicon)
{

	UINT8   bID0 = 0, bID1 = 0;
    RECT      tmpRECT = {0,0,0,0};
	RECT RECTExit = RECT_TOUCH_BUTTON_MENU_EXIT;





    OSDDraw_EnterMenuDrawing(&bID0, &bID1);

	#if 0

	AHC_OSDSetColor(wDispID, 0x06FFFFFF); 

   	AHC_OSDDrawFillRect(wDispID, tmpRECT.uiLeft, tmpRECT.uiTop, 480, 320);

	#endif
	#if 0

    //OSDDraw_ClearOvlDrawBuffer(bID0);

    if(AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
        MenuDrawSubBackCurtain(bID0, TV_MENU_BK_COLOR);
    else
        MenuDrawSubBackCurtain(bID0, /*MENU_BACKGROUND_COLOR*/0x90808080);

	#endif

	#if 0
	if(vicon == 0)
		OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
	else if(vicon == 1)
		OSD_Draw_Icon(bmsubvol1, RECTExit, bID0);
	else if(vicon == 2)
		OSD_Draw_Icon(bmsubvol2, RECTExit, bID0);
	else if(vicon == 3)
		OSD_Draw_Icon(bmsubvol3, RECTExit, bID0);
	else if(vicon == 4)
		OSD_Draw_Icon(bmsubvol4, RECTExit, bID0);
	else if(vicon == 5)
		OSD_Draw_Icon(bmsubvol5, RECTExit, bID0);
	else if(vicon == 6)
		OSD_Draw_Icon(bmsubvol6, RECTExit, bID0);
	else if(vicon == 7)
		OSD_Draw_Icon(bmsubvol7, RECTExit, bID0);
	else if(vicon == 8)
		OSD_Draw_Icon(bmsubvol8, RECTExit, bID0);
	else if(vicon == 9)
		OSD_Draw_Icon(bmsubvol9, RECTExit, bID0);
	else if(vicon == 10)
		OSD_Draw_Icon(bmsubvol10, RECTExit, bID0);
	else if(vicon == 11)
		OSD_Draw_Icon(bmsubvol11, RECTExit, bID0);
	#endif 
		//if(vicon == 0)
		//OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
		switch(vicon)
		{
		
				case 0:		RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;
							
							OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
							break;
	            		
				case 1:
						RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;
							OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
							RECTExit.uiLeft = 182;//286
							RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
							  break;
				case 2:
						RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;
						OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);   

						break;

				case 3:
						RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;
						OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
						RECTExit.uiLeft = 182;
						RECTExit.uiTop = 222;  OSD_Draw_Icon(bmkedu4, RECTExit, bID0); 
						
						RECTExit.uiLeft = 194;
						RECTExit.uiTop = 222; OSD_Draw_Icon(bmkedu3, RECTExit, bID0);  

						RECTExit.uiLeft = 205;
						RECTExit.uiTop =  222;   OSD_Draw_Icon(bmkedu3, RECTExit, bID0);

						break;

				case 4:
						RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;
						OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
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
						RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;
						OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
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

						RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;

						OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
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

						RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;

						OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
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

					RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;

						OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
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

					RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;

						OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
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

					RECTExit.uiLeft = 170;//286
							RECTExit.uiTop = 107;

						OSD_Draw_Icon(bmsubvol, RECTExit, bID0);
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

//extern UINT16 wDispID;
extern AHC_BOOL exit_flag;
void Draw_Main_volum_icon(int *vicon, AHC_BOOL clear_flag,UINT16 bID0)
{
	 UINT8   /*bID0 = 0, bID1 = 0,*/i;
   	 RECT      tmpRECT1 = {120,   150,   200, 100};//40,   200,   320, 30
       //RECT RECTExit1 = {415,   270,  60 ,35};//40,   200,  110 ,30  {120,   150,  200 ,50};/
          RECT RECTExit1 = {430,   293,  30 ,15};
	   RECT RECTExit5 = {420, 105, 50, 15};
	   RECT RECTExit6 = {425, 120, 40, 15};

	   // RECT      tmpRECT3= {440,   118,   10, 147}; //  {147,   210,   147, 10};
		// RECT      tmpRECT4 = {440,   118,   10, 12}; // {147,   210,   12, 10};
		RECT      tmpRECT3= {442,   140,   7, 148}; //  {147,   210,   147, 10};
		 RECT      tmpRECT4 = {442,   140,   7, 8}; // {147,   210,   12, 10};
	 char    szvf[5];
	  char    szvf1[5];
	   char    szvf2[5];
	 UINT16          BarPos;
	 GUI_COLOR Color_1;
	//  RECT            rc = {160,200, 200, 30};
	if(*vicon > 15)
	{
		*vicon = 15;
		if(exit_flag)
			return ;
	}
	else if(*vicon < 0)
	{
		*vicon = 0;
		if(exit_flag)
			return ;
	}
	
	if(clear_flag == 0)
	{
	 //sprintf(szvf, "M:%02d",*vicon);
		//sprintf(szvf, "%d",*vicon);
		 sprintf(szvf1,"MSTR");
	}
	else
	{
	// sprintf(szvf, "S:%02d",*vicon);
		// sprintf(szvf, "%d",*vicon);
		 sprintf(szvf1,"SUB");
	}
	 sprintf(szvf, "%d",*vicon);
	 sprintf(szvf2,"VOL");
	 //if(vicon < 0)
	 		//vicon = 0;
	 //vicon = 5;
	//BarPos = (UINT16)vicon*10; 




    // OSDDraw_EnterMenuDrawing(&bID0, &bID1);

	 
	#if 0
	 if(volume_B == 0)
	{
		AHC_OSDSetColor(bID0, 0xFFEBEBEB); 
		 AHC_OSDSetFont(bID0, &GUI_Font24_1);
		AHC_OSDDrawFillRect(bID0, tmpRECT1.uiLeft, tmpRECT1.uiTop, tmpRECT1.uiLeft + tmpRECT1.uiWidth, tmpRECT1.uiTop+tmpRECT1.uiHeight);
		//OSD_ShowString( bID0,szvf, NULL, RECTExit1, OSD_COLOR_BLACK, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);

		volume_B = 1;
	}
	//AHC_OSDSetColor(bID0, 0xFF808080); 
	//AHC_OSDDrawFillRect(bID0, 270, 160, 290, 180);
	AHC_OSDSetColor(bID0, 0xFFEBEBEB); 
	 AHC_OSDSetFont(bID0, &GUI_Font24_1);
	AHC_OSDDrawFillRect(bID0, RECTExit1.uiLeft, RECTExit1.uiTop, RECTExit1.uiLeft + RECTExit1.uiWidth, RECTExit1.uiTop+RECTExit1.uiHeight);
	OSD_ShowString( bID0,szvf, NULL, RECTExit1, OSD_COLOR_BLACK, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);

	#endif

		switch(Main_Get_Page())
		{
		case MENU_MAIN_FLAG :        Color_1 = OSD_COLOR_MAIN_PAGE;  break;
		case BLUETOOH_FLAG  :         Color_1 = OSD_COLOR_BLUETOOTH;  break;
		case RADIO_FM_FLAG :          Color_1 = OSD_COLOR_FM;  break;
		case RADIO_AM_FLAG :         Color_1 = OSD_COLOR_AM;  break;
		case VOLUME_FLAG :             Color_1 = OSD_COLOR_VOL;  break;
		case LIGHT_BAR_FLAG :       Color_1 = OSD_COLOR_BAR;  break;
		case AUX_FLAG :                 Color_1 = OSD_COLOR_AUX;  break;
		case USB_FLAG : 			 Color_1 = OSD_COLOR_USB;  break;
		case RGB_FLAG :  			Color_1 = OSD_COLOR_RGB;  break; // lyj 20191018
		case BRIGHTNESS_FLAG :     Color_1 = OSD_COLOR_BRIGHTNESS;  break;
	}
		#if 0
	if(Main_Get_Page() == RGB_FLAG)
	{
		
		AHC_OSDDrawBitmap(bID0, &bmIcon_Num_4,420, 105);
		AHC_OSDSetFont(bID0, &GUI_Font20_1);
		OSD_ShowString( bID0,szvf1, NULL, RECTExit5, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
		OSD_ShowString( bID0,szvf2, NULL, RECTExit6, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
	}
	else
		#endif
	{
		AHC_OSDSetColor(bID0, Color_1); 
		AHC_OSDSetFont(bID0, &GUI_Font20_1);
	   	AHC_OSDDrawFillRect(bID0, RECTExit5.uiLeft, RECTExit5.uiTop, 470, 120);
		OSD_ShowString( bID0,szvf1, NULL, RECTExit5, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
		//AHC_OSDDrawFillRect(bID0, RECTExit6.uiLeft, RECTExit6.uiTop, 475, 305);
		OSD_ShowString( bID0,szvf2, NULL, RECTExit6, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
	}
	AHC_OSDSetColor(bID0, 0xFF323232);
	AHC_OSDDrawFillRect(bID0, RECTExit1.uiLeft, RECTExit1.uiTop, 460, 308);
	OSD_ShowString( bID0,szvf, NULL, RECTExit1, OSD_COLOR_WHITE, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);
	AHC_OSDSetColor(bID0, 0xFF323232);
	AHC_OSDDrawFillRect(bID0, tmpRECT3.uiLeft, tmpRECT3.uiTop, tmpRECT3.uiLeft + tmpRECT3.uiWidth, tmpRECT3.uiTop+tmpRECT3.uiHeight);

	AHC_OSDSetColor(bID0, 0xFFFFFFFF); //0xFF818181
	for(i = 0; i< *vicon;i++)
	{
		//AHC_OSDDrawFillRect(bID0, tmpRECT4.uiLeft + 15*i, tmpRECT4.uiTop, tmpRECT4.uiLeft+ 15*i + tmpRECT4.uiWidth, tmpRECT4.uiTop+tmpRECT4.uiHeight);	
		//AHC_OSDDrawFillRect(bID0, tmpRECT4.uiLeft, tmpRECT4.uiTop+135 - 15*i, tmpRECT4.uiLeft + tmpRECT4.uiWidth, tmpRECT4.uiTop+tmpRECT4.uiHeight + 135 -15*i);
		AHC_OSDDrawFillRect(bID0, tmpRECT4.uiLeft, tmpRECT4.uiTop+140 - 10*i, tmpRECT4.uiLeft + tmpRECT4.uiWidth, tmpRECT4.uiTop+tmpRECT4.uiHeight + 140 -10*i);
	}

	 #if 0

	AHC_OSDSetColor(bID0, 0xFFFFFFFF); 
	AHC_OSDSetFont(bID0, &GUI_Font16_1);


   	AHC_OSDDrawFillRect(bID0, tmpRECT1.uiLeft, tmpRECT1.uiTop, tmpRECT1.uiLeft + tmpRECT1.uiWidth, tmpRECT1.uiTop+tmpRECT1.uiHeight);
	OSD_ShowString( bID0,szvf, NULL, RECTExit1, OSD_COLOR_BLACK, OSD_COLOR_TRANSPARENT,GUI_TA_HCENTER|GUI_TA_VCENTER);

	AHC_OSDSetColor(bID0, 0xFF000000);//选定
   	 AHC_OSDDrawFillRect(bID0, tmpRECT1.uiLeft+121, tmpRECT1.uiTop+1, (tmpRECT1.uiLeft+121)+BarPos, tmpRECT1.uiTop+tmpRECT1.uiHeight-1);
	#endif
	//AHC_OSDSetColor(wDispID, OSD_COLOR_DARKGRAY);//未选定

  // AHC_OSDDrawFillRect(wDispID, (rc.uiLeft+1)+vicon, rc.uiTop+1, rc.uiLeft+rc.uiWidth-1, rc.uiTop+rc.uiHeight-1);
	 


	//OSDDraw_ExitMenuDrawing(&bID0, &bID1);

	if(clear_flag == 0)
	{
		MenuSettingConfig()->uiEV = *vicon;
		if(*vicon == 0)
			//uart_to_mcu_voice(MAINVOICE,(MMP_BYTE)(*vicon),MUTE_ON);
			uart_to_mcu_voice(MAINVOICE,/*(MMP_BYTE)(*vicon)*/200,MUTE_ON); // lyj 20190831
		else
			volume_conver_size(MAINVOICE,*vicon);
	}
	else
	{
		MenuSettingConfig()->uiSpeedStamp = *vicon;
		if(*vicon == 0)
		{
			uart_to_mcu_voice(SUBVOICE1,(MMP_BYTE)(*vicon),MUTE_ON);
			uart_to_mcu_voice(SUBVOICE2,(MMP_BYTE)(*vicon),MUTE_ON);
		}
		else
		{
			volume_conver_size(SUBVOICE1,*vicon);
			volume_conver_size(SUBVOICE2,*vicon);

		}

	}

	
		volume_M = 1;
		
		


}





//======================end======================
