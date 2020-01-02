/*===========================================================================
 * Include file 
 *===========================================================================*/ 

#include "AHC_Common.h"
#include "AHC_GUI.h"
#include "AHC_Display.h"
#include "AHC_Utility.h"
#include "AHC_General_CarDV.h"
#include "MenuCommon.h"
#include "MenuDrawCommon.h"
#include "MenuDrawingFunc.h"
#include "ColorDefine.h"
#include "StateTVFunc.h"
#include "StateHdmiFunc.h"
#include "disp_drv.h"

/*===========================================================================
 * Global variable
 *===========================================================================*/ 

#define MAX_MENU_DRAW_OSD_NUM (2)
    
static UINT8  mLay1AvailableBuffer 	= 0;
static UINT8  mLay1CurrentBuffer 	= 0;
static UINT8  mLay1LastBuffer    	= 0 ;
#if 1
static UINT8  mLay0AvailableBuffer 	= 0;
static UINT8  mLay0CurrentBuffer 	= 0;
static UINT8  mLay0LastBuffer    	= 0 ;
#endif
static INT8   m_byEnterDrawing      = 0;

//Menu OSD
#if (HDMI_ENABLE)
UINT8 gMenuDrawOSDID_HDMI[MAX_MENU_DRAW_OSD_NUM] = {HDMI_MENU_OSD_ID, HDMI_MENU_OSD2_ID};
#endif
#if (TVOUT_ENABLE)
UINT8 gMenuDrawOSDID_TVOut[MAX_MENU_DRAW_OSD_NUM] = {TV_MENU_OSD_ID, TV_MENU_OSD2_ID};
#endif
UINT8 gMenuDrawOSDID_LCD[MAX_MENU_DRAW_OSD_NUM] = {OVL_BUFFER_INDEX2, OVL_BUFFER_INDEX3};

//Normal OSD
#if (HDMI_ENABLE)
#if HDMI_SINGLE_OSD_BUF
UINT8 gOvlDrawOSDID_HDMI[MAX_MENU_DRAW_OSD_NUM] = {HDMI_UI_OSD_ID, HDMI_UI_OSD_ID};
#else
UINT8 gOvlDrawOSDID_HDMI[MAX_MENU_DRAW_OSD_NUM] = {OVL_BUFFER_INDEX, OVL_BUFFER_INDEX1};
#endif
#endif
#if (TVOUT_ENABLE)
#if TV_SINGLE_OSD_BUF
UINT8 gOvlDrawOSDID_TVOut[MAX_MENU_DRAW_OSD_NUM] = {TV_UI_OSD_ID, TV_UI_OSD_ID};
#else
UINT8 gOvlDrawOSDID_TVOut[MAX_MENU_DRAW_OSD_NUM] = {OVL_BUFFER_INDEX, OVL_BUFFER_INDEX1};
#endif
#endif
UINT8 gOvlDrawOSDID_LCD[MAX_MENU_DRAW_OSD_NUM] = {OVL_BUFFER_INDEX, OVL_BUFFER_INDEX1};

UINT8 *gpMenuDrawOSDID = gMenuDrawOSDID_LCD;
UINT8 *gpOvlDrawOSDID = gOvlDrawOSDID_LCD;

/*===========================================================================
 * Extern variable
 *===========================================================================*/ 

extern AHC_BOOL gbAhcDbgBrk;
extern AHC_OS_SEMID gulDrawingSemID;
//extern UINT16 TVFunc_GetUImodeOsdID(void);
//extern UINT16 TVFunc_GetMenuOsdID(void);

/*===========================================================================
 * Main body
 *===========================================================================*/ 

#if (TVOUT_ENABLE)//(TVOUT_PREVIEW_EN)
UINT16 TVFunc_GetUImodeOsdID(void) //Andy Liu. TBD
{
    return TV_UI_OSD_ID;
}

UINT16 TVFunc_GetMenuOsdID(void) //Andy Liu. TBD
{
    return TV_MENU_OSD_ID;
}

UINT16 TVFunc_GetMenuOsd2ID(void) //Andy Liu. TBD
{
    return TV_MENU_OSD2_ID;
}
#endif

#if (HDMI_ENABLE)//(HDMI_PREVIEW_EN)
UINT16 HDMIFunc_GetUImodeOsdID(void) //Andy Liu. TBD
{
    return HDMI_UI_OSD_ID;
}

UINT16 HDMIFunc_GetUImodeOsd2ID(void) //Andy Liu. TBD
{
    return HDMI_UI_OSD2_ID;
}

UINT16 HDMIFunc_GetMenuOsdID(void) //Andy Liu. TBD
{
    return HDMI_MENU_OSD_ID;
}

UINT16 HDMIFunc_GetMenuOsd2ID(void) //Andy Liu. TBD
{
    return HDMI_MENU_OSD2_ID;
}
#endif

#if 0
void _____MainLayer_Function_________(){ruturn;} //dummy
#endif

#if 0
void OSDDraw_GetMainDrawBuffer(UINT8* ulID)
{
    if(0 == mLay0LastBuffer)
    {
        mLay0AvailableBuffer = MAIN_BUFFER_INDEX;
		mLay0LastBuffer      = MAIN_BUFFER_INDEX1;
    }
    else if (MAIN_BUFFER_INDEX == mLay0LastBuffer)
    {
        mLay0AvailableBuffer = MAIN_BUFFER_INDEX1;
    }
    else
    {
        mLay0AvailableBuffer = MAIN_BUFFER_INDEX;
    }
    *ulID = mLay0AvailableBuffer;
}

void OSDDraw_GetLastMainDrawBuffer(UINT8* ulID)
{
    *ulID = mLay0LastBuffer;
}

void OSDDraw_SetMainDrawBuffer(UINT8 ulID)
{
    mLay0CurrentBuffer = ulID;
    mLay0LastBuffer = mLay0CurrentBuffer;
}
#endif

void OSDDraw_GetDisplaySize(UINT8 bID, UINT16 *pw, UINT16* ph)
{
    UINT16 format;
    UINT32 addr;

#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
    AHC_DISPLAY_OUTPUTPANEL OutputDevice;

    AHC_GetDisplayOutputDev(&OutputDevice);

    if ((OutputDevice == AHC_DISPLAY_MAIN_LCD) || (OutputDevice == AHC_DISPLAY_SUB_LCD))
    {
        AHC_OSDGetBufferAttr(bID, ph, pw, &format, &addr);
    }
    else
#endif
    {
        AHC_OSDGetBufferAttr(bID, pw, ph, &format, &addr);
    }
}

#if 0
void OSDDraw_ClearMainDrawBuffer(void)
{
    UINT8 bID = 0;
    OSDDraw_GetLastMainDrawBuffer(&bID);
    OSDDraw_ClearPanel(bID, OSD_COLOR_TRANSPARENT);
}
#endif

void OSDDraw_ClearMainDrawBufferAll(void)
{
    OSDDraw_ClearPanel(MAIN_DISPLAY_BUFFER, OSD_COLOR_TRANSPARENT);

    AHC_OSDDuplicateBuffer(MAIN_DISPLAY_BUFFER, MAIN_BUFFER_INDEX);
    AHC_OSDDuplicateBuffer(MAIN_DISPLAY_BUFFER, MAIN_BUFFER_INDEX1);
}

#if 0
void _____MenuLayer_Function_________(){ruturn;} //dummy
#endif

AHC_BOOL SwitchDrawOSDIDFunc(void)
{
    AHC_BOOL    ahc_ret = AHC_TRUE;

#if (TVOUT_ENABLE)
    if (AHC_IsTVConnectEx()){
        gpMenuDrawOSDID = gMenuDrawOSDID_TVOut;
        gpOvlDrawOSDID = gOvlDrawOSDID_TVOut;
    }else
#endif
#if (HDMI_ENABLE)
	if (AHC_IsHdmiConnect()){
		gpMenuDrawOSDID = gMenuDrawOSDID_HDMI;
		gpOvlDrawOSDID = gOvlDrawOSDID_HDMI;
	}else
#endif
    {
        gpMenuDrawOSDID = gMenuDrawOSDID_LCD;
        gpOvlDrawOSDID = gOvlDrawOSDID_LCD;
    }  

    return ahc_ret;
}

    
void OSDDraw_GetMenuDrawBuffer(UINT8* ulID)
{
    if((*(gpMenuDrawOSDID + 0) != mLay0LastBuffer) && (*(gpMenuDrawOSDID + 1) != mLay0LastBuffer)){
        mLay0AvailableBuffer = *(gpMenuDrawOSDID + 0);
        mLay0LastBuffer      = *(gpMenuDrawOSDID + 1);
    }
    else if (*(gpMenuDrawOSDID + 0) == mLay0LastBuffer){
        mLay0AvailableBuffer = *(gpMenuDrawOSDID + 1);
    }
    else{
        mLay0AvailableBuffer = *(gpMenuDrawOSDID + 0);
    }

#if 0
#if HDMI_ENABLE
	if(AHC_IsHdmiConnect())
	{
        if(HDMI_MENU_OSD_ID != mLay1LastBuffer &&
           HDMI_MENU_OSD2_ID != mLay1LastBuffer)
        {
            mLay1AvailableBuffer = HDMI_MENU_OSD_ID;
    		mLay1LastBuffer      = HDMI_MENU_OSD2_ID;
        }
        else if (HDMI_MENU_OSD2_ID == mLay1LastBuffer)
        {
            mLay1AvailableBuffer = HDMI_MENU_OSD_ID;
        }
        else
        {
            mLay1AvailableBuffer = HDMI_MENU_OSD2_ID;
        }
	}	
	else
#endif
#if TVOUT_ENABLE
	if(AHC_IsTVConnectEx())
	{
        if(TV_MENU_OSD_ID != mLay1LastBuffer &&
           TV_MENU_OSD2_ID != mLay1LastBuffer)
        {
            mLay1AvailableBuffer = TV_MENU_OSD_ID;
    		mLay1LastBuffer      = TV_MENU_OSD2_ID;
        }
        else if (TV_MENU_OSD2_ID == mLay1LastBuffer)
        {
            mLay1AvailableBuffer = TV_MENU_OSD_ID;
        }
        else
        {
            mLay1AvailableBuffer = TV_MENU_OSD2_ID;
        }
	}
	else
#endif
    {
        if(OVL_BUFFER_INDEX2 != mLay1LastBuffer &&
           OVL_BUFFER_INDEX3 != mLay1LastBuffer)
        {
            mLay1AvailableBuffer = OVL_BUFFER_INDEX2;
    		mLay1LastBuffer      = OVL_BUFFER_INDEX3;
        }
        else if (OVL_BUFFER_INDEX2 == mLay1LastBuffer)
        {
            mLay1AvailableBuffer = OVL_BUFFER_INDEX3;
        }
        else
        {
            mLay1AvailableBuffer = OVL_BUFFER_INDEX2;
        }
    }
#endif

    *ulID = mLay0AvailableBuffer;
}

void OSDDraw_GetLastMenuDrawBuffer(UINT8* ulID)
{
    *ulID = mLay0LastBuffer;
}

void OSDDraw_SetMenuDrawBuffer(UINT8 ulID)
{
    mLay0CurrentBuffer = ulID;
    mLay0LastBuffer = mLay0CurrentBuffer;
}

MMP_ULONG 	startup = 0;
MMP_ULONG 	ending = 0; 

void OSDDraw_EnterMenuDrawing(UINT8* pbID0, UINT8* pbID1)
{
	MMPF_OS_GetTime(&ending);
	while((ending - startup) < 80){ //80 is for 480X854 Panel/Rotate 90
		MMPF_OS_Sleep(2);
		MMPF_OS_GetTime(&ending);	
		if (startup > ending) { //for overflow of ending
			ending = 0xFFFFFFFF - startup + ending;
			startup = 0;
			if((ending - startup) > 80)
				break;
		}
	}	
	startup = ending;

	OSDDraw_GetMenuDrawBuffer(pbID0);

	OSDDraw_GetLastMenuDrawBuffer(pbID1);

	AHC_OSDDuplicateBuffer(*pbID1, *pbID0);
}

void OSDDraw_ExitMenuDrawing(UINT8* pbID0, UINT8* pbID1)
{
	OSDDraw_SetMenuDrawBuffer(*pbID0);

	AHC_OSDSetCurrentDisplay(*pbID0);

	AHC_OSDRefresh();
	
	//AHC_OSDDuplicateBuffer(*pbID0, *pbID1);
} 

#if 0
void _____OverlayLayer_Function_________(){ruturn;} //dummy
#endif

void OSDDraw_GetOvlDrawBuffer(UINT8* ulID)
{
    if(*(gpOvlDrawOSDID + 0) == mLay1LastBuffer){
        mLay1AvailableBuffer = *(gpOvlDrawOSDID + 1);
    }
    else if (*(gpOvlDrawOSDID + 1) == mLay1LastBuffer){
        mLay1AvailableBuffer = *(gpOvlDrawOSDID + 0);
    }
    else{
        mLay1AvailableBuffer = *(gpOvlDrawOSDID + 0);
        mLay1LastBuffer      = *(gpOvlDrawOSDID + 1);
    }

    *ulID = mLay1AvailableBuffer;

#if 0
#if HDMI_SINGLE_OSD_BUF
	if(AHC_IsHdmiConnect())
	{
		*ulID = HDMIFunc_GetUImodeOsdID();
	}	
	else
#endif
#if TV_SINGLE_OSD_BUF
	if(AHC_IsTVConnectEx())
	{
		*ulID = TVFunc_GetUImodeOsdID();
	}
	else
#endif
    {
        if(OVL_BUFFER_INDEX  == mLay1LastBuffer)
        {
            mLay1AvailableBuffer = OVL_BUFFER_INDEX1;
        }
        else if (OVL_BUFFER_INDEX1 == mLay1LastBuffer)
        {
            mLay1AvailableBuffer = OVL_BUFFER_INDEX;
        }
        else
        {
            mLay1AvailableBuffer = OVL_BUFFER_INDEX;
            mLay1LastBuffer      = OVL_BUFFER_INDEX1;
        }

        *ulID = mLay1AvailableBuffer;
    }
#endif
}

void OSDDraw_GetLastOvlDrawBuffer(UINT8* ulID)
{
    *ulID = mLay1LastBuffer;

#if 0
#if HDMI_SINGLE_OSD_BUF
	if(AHC_IsHdmiConnect())
	{
		*ulID = HDMIFunc_GetUImodeOsdID();
	}	
	else
#endif
#if TV_SINGLE_OSD_BUF
	if(AHC_IsTVConnectEx())
	{
		*ulID = TVFunc_GetUImodeOsdID();
	}
	else
#endif
    {
        *ulID = mLay1LastBuffer;
    }
#endif

}

void OSDDraw_SetOvlDrawBuffer(UINT8 ulID)
{
    mLay1CurrentBuffer = ulID;
    mLay1LastBuffer = mLay1CurrentBuffer;

#if 0
#if HDMI_SINGLE_OSD_BUF
	if(AHC_IsHdmiConnect())
	{
		mLay1CurrentBuffer = HDMIFunc_GetUImodeOsdID();
		mLay1LastBuffer = HDMIFunc_GetUImodeOsdID();
	}	
	else
#endif
#if TV_SINGLE_OSD_BUF
	if(AHC_IsTVConnectEx())
	{
		mLay1CurrentBuffer = TVFunc_GetUImodeOsdID();
		mLay1LastBuffer = TVFunc_GetUImodeOsdID();
	}
	else
#endif
    {
        mLay1CurrentBuffer = ulID;
        mLay1LastBuffer = mLay1CurrentBuffer;
    }
#endif
}

void OSDDraw_ClearOvlDrawBuffer(UINT8 ulID)
{
    OSDDraw_ClearPanel(ulID, OSD_COLOR_TRANSPARENT);
}

void OSDDraw_ClearOvlDrawOvlBufferAll(void)
{
    OSDDraw_ClearPanel(OVL_DISPLAY_BUFFER, OSD_COLOR_TRANSPARENT);
    AHC_OSDDuplicateBuffer(OVL_DISPLAY_BUFFER, *(gpOvlDrawOSDID + 0) );
    AHC_OSDDuplicateBuffer(OVL_DISPLAY_BUFFER, *(gpOvlDrawOSDID + 1));

#if 0
#if HDMI_ENABLE
	if(AHC_IsHdmiConnect())
	{
#if (HDMI_SINGLE_OSD_BUF == 1)
        OSDDraw_ClearPanel(HDMIFunc_GetUImodeOsdID(), OSD_COLOR_TRANSPARENT);
#else
        OSDDraw_ClearPanel(HDMI_UI_OSD_ID, OSD_COLOR_TRANSPARENT);
        OSDDraw_ClearPanel(HDMI_UI_OSD2_ID, OSD_COLOR_TRANSPARENT);
#endif
	}	
	else
#endif
#if TVOUT_ENABLE
	if(AHC_IsTVConnectEx())
	{
#if (TV_SINGLE_OSD_BUF == 1)
        OSDDraw_ClearPanel(TVFunc_GetUImodeOsdID(), OSD_COLOR_TRANSPARENT);
#else
        OSDDraw_ClearPanel(TV_UI_OSD_ID, OSD_COLOR_TRANSPARENT);
//        OSDDraw_ClearPanel(HDMI_UI_OSD2_ID, OSD_COLOR_TRANSPARENT);
#endif
	}
	else
#endif
    {
        OSDDraw_ClearPanel(OVL_DISPLAY_BUFFER, OSD_COLOR_TRANSPARENT);
        AHC_OSDDuplicateBuffer(OVL_DISPLAY_BUFFER, OVL_BUFFER_INDEX );
        AHC_OSDDuplicateBuffer(OVL_DISPLAY_BUFFER, OVL_BUFFER_INDEX1);
    }
#endif
}

void OSDDraw_EnterDrawing(UINT8* pbID0, UINT8* pbID1)
{
    if (AHC_OS_AcquireSem(gulDrawingSemID, 0) != OS_NO_ERR) {
        printc("ASSERT:%s  gulDrawingSemID LOCKED\r\n", __func__);
    }

    //OSDDraw_EnterDrawing monitor functions, only one function can call it in the meantime
    m_byEnterDrawing++;
    if(m_byEnterDrawing != 1){
        printc(FG_YELLOW("m_byEnterDrawing %d\r\n"),m_byEnterDrawing);
        printc(FG_YELLOW("====Warning!!! Illegal access function %s====\r\n"),__func__);
    }

    OSDDraw_GetOvlDrawBuffer(pbID0);
    OSDDraw_GetLastOvlDrawBuffer(pbID1);

#if 0
#if HDMI_SINGLE_OSD_BUF
	if(AHC_IsHdmiConnect())
	{
		*pbID0 = HDMIFunc_GetUImodeOsdID();
		*pbID1 = HDMIFunc_GetUImodeOsdID();
	}	
	else
#endif
#if TV_SINGLE_OSD_BUF
	if(AHC_IsTVConnectEx())
	{
		*pbID0 = TVFunc_GetUImodeOsdID();
		*pbID1 = TVFunc_GetUImodeOsdID();
	}
	else
#endif
	{
   		OSDDraw_GetOvlDrawBuffer(pbID0);
    	OSDDraw_GetLastOvlDrawBuffer(pbID1);
	}
#endif

}

void OSDDraw_ExitDrawing(UINT8* pbID0, UINT8* pbID1)
{
    OSDDraw_SetOvlDrawBuffer(*pbID0);
    AHC_OSDSetCurrentDisplay(*pbID0);
    AHC_OSDRefresh();

#if 0//(HDMI_ENABLE) || (TVOUT_ENABLE) //???
	if( !AHC_IsTVConnectEx() || !AHC_IsHdmiConnect() )
#endif        
    {
        AHC_OSDDuplicateBuffer(*pbID0, *pbID1);
    }

    //OSDDraw_ExitDrawing monitor functions, only one function can call it in the meantime
    m_byEnterDrawing--;
    if(m_byEnterDrawing != 0){
        printc(FG_YELLOW("m_byEnterDrawing %d\r\n"),m_byEnterDrawing);
        printc(FG_YELLOW("====Warning!!! Illegal access function %s====\r\n"),__func__);
    }

    if (AHC_OS_ReleaseSem(gulDrawingSemID) != OS_NO_ERR) {
        printc("ASSERT:%s gulDrawingSemID UNLOCKED\r\n", __func__);
    }
}
 
#if 0
void _____Other_Function_________(){ruturn;} //dummy
#endif

void OSDDraw_ClearPanel(UINT16 ubID, GUI_COLOR color)
{
    UINT16 w, h;

    if (AHC_FALSE == AIHC_GetOSDInitStatus()) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); return; }

#if (defined(WIFI_PORT) && (WIFI_PORT == 1) && defined(CFG_DRAW_WIFI_STREAMING_MONO_PATTERN))
    if (AHC_Get_WiFi_Streaming_Status())
        color = CFG_DRAW_WIFI_STREAMING_MONO_PATTERN;
#endif

    BEGIN_LAYER(ubID);
    OSDDraw_GetDisplaySize(ubID, &w, &h);
    AHC_OSDSetColor(ubID, color);
    AHC_OSDDrawFillRect(ubID, 0, 0, w, h);
    END_LAYER(ubID);
}

AHC_BOOL OSDDraw_SetAlphaBlending(UINT8 ulID, AHC_BOOL bEnable)
{
    UINT32 	iVal[10];
    UINT8 	i;
    
    iVal[0] = ( AHC_FALSE == bEnable )?(0):(1);
    iVal[1] = AHC_OSD_ALPHA_ARGB;

    for(i=0; i<8; i++) 
    {
        iVal[2+i] = i * 256 / 8;
    }

    return AHC_OSDSetDisplayAttr( ulID, AHC_OSD_ATTR_ALPHA_BLENDING_ENABLE, iVal );
}   

AHC_BOOL OSDDraw_SetSemiTransparent(UINT8 ulID, UINT8 ulEnable, UINT8 ulOsdSemiType, UINT8 ulSemiWeight )
{
    UINT32 iVal[10];

    iVal[0] = (UINT32)ulEnable;
    iVal[1] = (UINT32)ulOsdSemiType;   //  AHC_OSD_SEMITP_AVG
                                       //  AHC_OSD_SEMITP_AND
                                       //  AHC_OSD_SEMITP_OR 
                                       //  AHC_OSD_SEMITP_INV

    iVal[2] = (UINT32) ulSemiWeight;   //  transparent weight 0~255

    return AHC_OSDSetDisplayAttr( ulID, AHC_OSD_ATTR_SEMI_TRANSPARENT_ENABLE, iVal );
}

/** @brief The general function to enable or disable the semi-transparent function.
 *
 * It uses the semi transparent feature based on the color depth of the buffer.
 *
 * @param id AHC buffer ID
 * @param enable enable or disable the buffer
 * @return The result from the semi-transparent function
 */
AHC_BOOL OSDDraw_EnableSemiTransparent(UINT8 id, AHC_BOOL enable)
{
    UINT16 format;
    UINT16 dummy;
    UINT32 addr;

    AHC_OSDGetBufferAttr(id, &dummy, &dummy, &format, &addr);
    if (format < OSD_COLOR_ALPHA_START) {//RGB without alpha
        return OSDDraw_SetSemiTransparent(id, enable, AHC_OSD_SEMITP_AVG, 170);//~0.66 alpha
    }
    return OSDDraw_SetAlphaBlending(id, enable);
}

void OSD_Draw_Icon(const GUI_BITMAP Bmp, RECT Draw, UINT16 ulID)
{   
    AHC_OSDDrawBitmap(ulID, &Bmp, Draw.uiLeft, Draw.uiTop);
}

void OSD_Draw_IconXY(UINT16 ulID, const GUI_BITMAP Bmp, UINT16 x, UINT16 y)
{   
    AHC_OSDDrawBitmap(ulID, &Bmp, x, y);
}

UINT8 NumOfDigit(UINT32 num)
{
    UINT8 k = 1;
    
	while((num/=10)>=1) k++;
    return k;
}

UINT8* GetFileNameAddr(char* filepath)
{
    UINT16 len = 0;
    char* addr;

    addr = filepath;
    while(*addr) {
    	len++;
    	addr++;
    }
    
    addr--;
    
	while((*addr != '\\') && (*addr != '/') && len) {
		len--;
		addr--;
	}
	
	if ((*addr == '\\') || (*addr == '/'))
	    addr++;

    return (UINT8*)addr;
}

static GUI_RECT m_BoundingRC;
/** @brief Reset the bounding rectangle structure.
 *
 *  @note The RC
 *
 *  @post Use OSDDraw_ScanBoundingRC or OSDDraw_ScanBoundingRCFromButton to scan the bounding rectangle.
 *        Typically for clearing the minimal rectangle to speed up menu drawing function.
 */
void OSDDraw_ResetBoundingRC(void)
{
    UINT16 format;
    UINT16 w, h;
    UINT32 addr;
    AHC_OSDGetBufferAttr(OVL_BUFFER_INDEX, &w, &h, &format, &addr);
    m_BoundingRC.x0 = w;
    m_BoundingRC.y0 = h;
    m_BoundingRC.x1 = 0;
    m_BoundingRC.y1 = 0;
}

GUI_RECT* OSDDraw_ScanBoundingRc(const RECT* rc)
{
    //printc("rc: %d, %d, (%dx%d)\r\n", rc->uiLeft, rc->uiTop, rc->uiWidth, rc->uiHeight);
    if (rc->uiLeft < m_BoundingRC.x0) {
        m_BoundingRC.x0 = rc->uiLeft;
    }

    if (rc->uiTop < m_BoundingRC.y0) {
        m_BoundingRC.y0 = rc->uiTop;
    }

    if (rc->uiLeft + rc->uiWidth > m_BoundingRC.x1) {
        m_BoundingRC.x1 = rc->uiLeft + rc->uiWidth;
    }

    if (rc->uiTop + rc->uiHeight > m_BoundingRC.y1) {
        m_BoundingRC.y1 = rc->uiTop + rc->uiHeight;
    }

    return &m_BoundingRC;
}

GUI_RECT* OSDDraw_ScanTouchBoundingRc(const SMENUTOUCHBUTTON* touch, UINT16 num)
{
    UINT16 i;

    for (i = 0; i < num; i++) {
        OSDDraw_ScanBoundingRc(&(touch[i].rcButton));
    }
    return &m_BoundingRC;
}

void OSDDraw_ClearTouchButtons(UINT16 uwDisplayID, SMENUTOUCHBUTTON* touchButtons, UINT16 num)
{
    OSDDraw_ResetBoundingRC();
    OSDDraw_ScanTouchBoundingRc(touchButtons, num);
    AHC_OSDSetColor(uwDisplayID, OSD_COLOR_TRANSPARENT);
    //printc("\r\n(%d,%d)~(%dx%d) has been scanned\r\n", m_BoundingRC.x0, m_BoundingRC.y0, m_BoundingRC.x1, m_BoundingRC.y1);
    AHC_OSDDrawFillRect(uwDisplayID, m_BoundingRC.x0, m_BoundingRC.y0, m_BoundingRC.x1, m_BoundingRC.y1);
}
