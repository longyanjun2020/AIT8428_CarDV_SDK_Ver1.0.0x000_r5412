//==============================================================================
//
//  File        : lcd_S8500L0_1280x320_MIPI.c
//  Description : 1280x320 Sync or DE mode RGB LCD Panel Control Function
//  Driver IC   : TC358778, EK79202
//  Author      : 
//  Revision    : 1.0
//
//==============================================================================

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "Customer_config.h" // CarDV
#include "lib_retina.h"
#include "lcd_common.h"
#include "mmpf_pio.h"
#include "Mipi_TC358778.h"


#define SPI_32bit_4wire_write	IIC_TC358778_WrData

#define regw	TC358778_Send_Short_Dsi_Packet


#if (USE_PWM_FOR_LCD_BACKLIGHT)
#include "mmpf_pwm.h"
#endif

#define PANEL_WIDTH                 (1280)
#define PANEL_HEIGHT                (340)

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================
#define Delayms(t) 					LCD_WAIT(t)

#define SECONDARY_DISPLAY 			(0)

#define LCD_RST_GPIO_PIN            (MMP_GPIO17)
#define LCD_STBY_GPIO_PIN           (MMP_GPIO16)


static MMPF_PANEL_ATTR m_PanelAttr = 
{
	// Panel basic setting
	PANEL_WIDTH, PANEL_HEIGHT,
	LCD_TYPE_RGBLCD,
	LCD_PRM_CONTROLER,
	0,

	// CarDV
#if ((16 == LCD_MODEL_RATIO_X) && (9 == LCD_MODEL_RATIO_Y))
	LCD_RATIO_16_9,
#else
	LCD_RATIO_4_3,
#endif

	// Panel initial sequence
	NULL,

	// Index/Cmd sequence
	NULL,

	// MCU interface
	LCD_BUS_WIDTH_8,
	LCD_PHASE0,
	LCD_POLARITY0,
	LCD_MCU_80SYS,
	0,
	0,
	0,
	LCD_RGB_ORDER_BGR,
 
	// RGB interface
	MMP_FALSE,
	LCD_SIG_POLARITY_H,     // DCLK Polarity
	LCD_SIG_POLARITY_L,     // H-Sync Polarity
	LCD_SIG_POLARITY_L,     // V-Sync Polarity
	RGB_D24BIT_BGR565,		//RGB_D24BIT_BGR565

	{0}
};

#define ENABLE_LCD_LOG       		(0)

#if defined(FAT_BOOT)
#define ENABLE_LCD_TEST_PATTERN 	(1)
#else
#define ENABLE_LCD_TEST_PATTERN 	(0)
#endif

#if (ENABLE_LCD_LOG)
#define LCD_DBG_LEVEL               (0)
#else
#define LCD_DBG_LEVEL               (3)
#endif

#define Delay_us(n)		MMPC_System_WaitUs(n*100/27)	//(lcm_util.udelay(n))
#define Delay_ms(n)   	MMPC_System_WaitMs(n*100/27)	//(lcm_util.mdelay(n))
#define LCD_delay_ms(n) MMPC_System_WaitMs(n*100/27)	//(lcm_util.mdelay(n))

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================
//#define LCD_DEGREE_180
#ifndef LCD_DEGREE_180
char Cmd52[] = {
	12, 0x52, 
		0x13,0x13,
		0x00,0x00,
		0x02,0x02,
		0x13,0x12,
		0x13,0x12,
		0x13,0x13,
		0x0A,0x04,
		0x08,0x06,
		0x06,0x08,
		0x04,0x0A,
		0x13,0x13,
		0x00//dummy 
};

char Cmd59[] ={          
	12, 0x59, 
		0x13,0x13,
		0x01,0x01,
		0x03,0x03,
		0x13,0x12,
		0x13,0x12,
		0x13,0x13,
		0x0B,0x05,
		0x09,0x07,
		0x07,0x09,
		0x05,0x0B,
		0x13,0x13,
		0x00///dummy 
};

char Cmd55[] = {    
	8,	0x55, 
		0x00,0x00,
		0x02,0x02,
		0x08,0x08,
		0x1E,0x1E,
		0x00,0x00,
		0x0F,0x0F, 
		0x00,///dummy
		0x00,///dummy
		0x00///dummy
};

char Cmd53[] = {    
	8, 	0x53, 
		0x3F,0x0A,
		0x0C,0x0B,
		0x12,0x13,
		0x0D,0x03,
		0x08,0x06,
		0x05,0x0B,
		0x14,
		0x00,///dummy
		0x00///dummy	
};
								
char Cmd54[] = {    
	8,	0x54, 
		0x3F,0x0A,
		0x0C,0x0B,
		0x12,0x13,
		0x0D,0x03,
		0x08,0x06,
		0x05,0x0B,
		0x14,
		0x00,///dummy
		0x00///dummy	
}; 

static void PowerOn_Panel(void) 
{
	regw(0xCD,0xAA);//Enable function table
	regw(0x0E,0x0D);//VCOM= -0.685V
	regw(0x18,0xFF);
	regw(0x19,0x3F);
	regw(0x1A,0xCC);
	regw(0x32,0x02);//R32[4]:horizontal flip;更改180度方向//12
	regw(0x3A,0x2D);
	regw(0x4E,0x2A);//VGMN= -5V//55，调整饱和度的代码
	regw(0x4F,0x2D);//VGMP voltage setting//55 调整饱和度的代码
	regw(0x51,0x80);

	TC358778_Send_Long_Dsi_Packet(Cmd52);//gamma
	TC358778_Send_Long_Dsi_Packet(Cmd53);//gamma
	TC358778_Send_Long_Dsi_Packet(Cmd54);//gamma
	TC358778_Send_Long_Dsi_Packet(Cmd55);
	regw(0x56,0x08);
	TC358778_Send_Long_Dsi_Packet(Cmd59);//GIP Control

	regw(0x67,0x12);
	regw(0x6B,0x48);
	regw(0x6C,0x88);
	regw(0x6D,0x00);
	regw(0x71,0xE3);
	regw(0x73,0xF0);
	regw(0x74,0x91);
	regw(0x75,0x03);
	regw(0x3C,0x48); // 4lane mip
	regw(0x5E,0x42);
	regw(0x4D,0x00);//disable function tabel

	Delay_us(500);
}

#else
char Cmd52[] = {
	12, 0x52, 
		0x13,0x13,
		0x00,0x00,
		0x02,0x02,
		0x12,0x13,
		0x13,0x13,
		0x12,0x13,
		0x04,0x0A,
		0x06,0x08,
		0x08,0x06,
		0x0A,0x04,
		0x13,0x13,
		0x00//dummy 
};

char Cmd59[] ={          
	12, 0x59, 
		0x13,0x13,
		0x01,0x01,
		0x03,0x03,
		0x12,0x13,
		0x13,0x13,
		0x12,0x13,
		0x05,0x0B,
		0x07,0x09,
		0x09,0x07,
		0x0B,0x05,
		0x13,0x13,
		0x00///dummy 
};

char Cmd55[] = {    
	8,	0x55, 
		0x00,0x00,
		0x02,0x02,
		0x08,0x08,
		0x1E,0x1E,
		0x00,0x00,
		0x0F,0x0F, 
		0x00,///dummy
		0x00,///dummy
		0x00///dummy
};

char Cmd53[] = {    
	8, 	0x53, 
		0x3F,0x0A,
		0x0C,0x0B,
		0x12,0x13,
		0x0D,0x03,
		0x08,0x06,
		0x05,0x0B,
		0x14,
		0x00,///dummy
		0x00///dummy	
};
								
char Cmd54[] = {    
	8,	0x54, 
		0x3F,0x0A,
		0x0C,0x0B,
		0x12,0x13,
		0x0D,0x03,
		0x08,0x06,
		0x05,0x0B,
		0x14,
		0x00,///dummy
		0x00///dummy	
}; 

static void PowerOn_Panel(void) 
{
	regw(0xCD,0xAA);//Enable function table
	regw(0x0E,0x0D);//VCOM= -0.685V
	regw(0x18,0xFF);
	regw(0x19,0x3F);
	regw(0x1A,0xCC);
	regw(0x32,0x12);//R32[4]:horizontal flip;更改180度方向//12
	regw(0x3A,0x2D);
	regw(0x4E,0x2A);//VGMN= -5V//55，调整饱和度的代码
	regw(0x4F,0x2D);//VGMP voltage setting//55 调整饱和度的代码
	regw(0x51,0x80);

	TC358778_Send_Long_Dsi_Packet(Cmd52);//gamma
	TC358778_Send_Long_Dsi_Packet(Cmd53);//gamma
	TC358778_Send_Long_Dsi_Packet(Cmd54);//gamma
	TC358778_Send_Long_Dsi_Packet(Cmd55);
	regw(0x56,0x08);
	TC358778_Send_Long_Dsi_Packet(Cmd59);//GIP Control

	regw(0x67,0x12);
	regw(0x6B,0x48);
	regw(0x6C,0x88);
	regw(0x6D,0x00);
	regw(0x71,0xE3);
	regw(0x73,0xF0);
	regw(0x74,0x91);
	regw(0x75,0x03);
	regw(0x3C,0x48); // 4lane mip
	regw(0x5E,0x42);
	regw(0x4D,0x00);//disable function tabel

	Delay_us(500);
}

#endif

void RTNA_Init_MIPI_DSI(void)
{
    MMP_USHORT reg_value;

    IIC_TC358778_InitGpio();

    IIC_TC358778_RdData(0x0000,&reg_value);
    RTNA_DBG_Str0("358778 ic id = 0");
    RTNA_DBG_Short0(reg_value);
    RTNA_DBG_Str0("\r\n");
    Delay_ms(100);

#if 0  //// ok version
	// TC358768XBG Software Reset
	// **************************************************
	SPI_32bit_4wire_write(0x00020001);  //SYSctl); S/W Reset
	Delay_us(10);  
	SPI_32bit_4wire_write(0x00020000);  //SYSctl); S/W Reset release
	// TC358768XBG PLL);Clock Setting
	// **************************************************
	SPI_32bit_4wire_write(0x0016207C);  //PLL Control Register 0 (PLL_PRD);PLL_FBD)
	SPI_32bit_4wire_write(0x00180603);  //PLL_FRS);PLL_LBWS); PLL oscillation enable
	LCD_delay_ms(1); 
	SPI_32bit_4wire_write(0x00180613);  //PLL_FRS);PLL_LBWS); PLL clock out enable
	// **************************************************
	// TC358768XBG DPI Input Control
	// **************************************************
	SPI_32bit_4wire_write(0x000601F4);  //FIFO Control Register
	// **************************************************
	// TC358768XBG D-PHY Setting
	// **************************************************
	SPI_32bit_4wire_write(0x01400000);  //D-PHY Clock lane enable
	SPI_32bit_4wire_write(0x01420000);  //
	SPI_32bit_4wire_write(0x01440000);  //D-PHY Data lane0 enable
	SPI_32bit_4wire_write(0x01460000);  //
	SPI_32bit_4wire_write(0x01480000);  //D-PHY Data lane1 enable
	SPI_32bit_4wire_write(0x014A0000);  //
	SPI_32bit_4wire_write(0x014C0000);  //D-PHY Data lane2 enable
	SPI_32bit_4wire_write(0x014E0000);  //
	SPI_32bit_4wire_write(0x01500000);  //D-PHY Data lane3 enable
	SPI_32bit_4wire_write(0x01520000);  //
	SPI_32bit_4wire_write(0x01000002);  //D-PHY Clock lane control
	SPI_32bit_4wire_write(0x01020000);  //
	SPI_32bit_4wire_write(0x01040002);  //D-PHY Data lane0 control
	SPI_32bit_4wire_write(0x01060000);  //
	SPI_32bit_4wire_write(0x01080002);  //D-PHY Data lane1 control
	SPI_32bit_4wire_write(0x010A0000);  //
	SPI_32bit_4wire_write(0x010C0002);  //D-PHY Data lane2 control
	SPI_32bit_4wire_write(0x010E0000);  //
	SPI_32bit_4wire_write(0x01100002);  //D-PHY Data lane3 control
	SPI_32bit_4wire_write(0x01120000);  //
	// **************************************************
	// TC358768XBG DSI-TX PPI Control
	// **************************************************
	SPI_32bit_4wire_write(0x02101644);  //LINEINITCNT
	SPI_32bit_4wire_write(0x02120000);  //
	SPI_32bit_4wire_write(0x02140002);  //LPTXTIMECNT
	SPI_32bit_4wire_write(0x02160000);  //
	SPI_32bit_4wire_write(0x02182002);  //TCLK_HEADERCNT
	SPI_32bit_4wire_write(0x021A0000);  //
	SPI_32bit_4wire_write(0x02200602);  //THS_HEADERCNT
	SPI_32bit_4wire_write(0x02220000);  //
	SPI_32bit_4wire_write(0x02244E20);  //TWAKEUPCNT
	SPI_32bit_4wire_write(0x02260000);  //
	SPI_32bit_4wire_write(0x022C0001);  //THS_TRAILCNT
	SPI_32bit_4wire_write(0x022E0000);  //
	SPI_32bit_4wire_write(0x02300005);  //HSTXVREGCNT
	SPI_32bit_4wire_write(0x02320000);  //
	SPI_32bit_4wire_write(0x0234001F);  //HSTXVREGEN enable
	SPI_32bit_4wire_write(0x02360000);  //
	SPI_32bit_4wire_write(0x02380001);  //DSI clock Enable/Disable during LP
	SPI_32bit_4wire_write(0x023A0000);  //
	SPI_32bit_4wire_write(0x023C0002);  //BTACNTRL1
	SPI_32bit_4wire_write(0x023E0002);  //
	SPI_32bit_4wire_write(0x02040001);  //STARTCNTRL
	SPI_32bit_4wire_write(0x02060000);  //
	// **************************************************
	// TC358768XBG DSI-TX Timing Control
	// **************************************************
	SPI_32bit_4wire_write(0x06200001);  //Sync Pulse/Sync Event mode setting
	SPI_32bit_4wire_write(0x0622001E);  //V Control Register1
	SPI_32bit_4wire_write(0x0624001C);  //V Control Register2
	SPI_32bit_4wire_write(0x06260140);  //V Control Register3
	SPI_32bit_4wire_write(0x06280121);  //H Control Register1
	SPI_32bit_4wire_write(0x062A00B5);  //H Control Register2
	SPI_32bit_4wire_write(0x062C0F00);  //H Control Register3
	SPI_32bit_4wire_write(0x05180001);  //DSI Start
	SPI_32bit_4wire_write(0x051A0000);  //
	// **************************************************
	// LCDD (Peripheral) Setting
	// **************************************************
	PowerOn_Panel();
	// **************************************************
	// Set to HS mode
	// **************************************************
	SPI_32bit_4wire_write(0x05000086);  //DSI lane setting); DSI mode=HS
	SPI_32bit_4wire_write(0x0502A300);  //bit set
	SPI_32bit_4wire_write(0x05008000);  //Switch to DSI mode
	SPI_32bit_4wire_write(0x0502C300);  //
	// **************************************************
	// Host: RGB(DPI) input start
	// **************************************************
	SPI_32bit_4wire_write(0x00080033);  //DSI-TX Format setting
	SPI_32bit_4wire_write(0x0050003E);  //DSI-TX Pixel stream packet Data Type setting
	SPI_32bit_4wire_write(0x00320000);  //HSYNC Polarity
	SPI_32bit_4wire_write(0x00040044);   //Configuration Control Register
	// **************************************************
	// LCDD (Peripheral) Setting
	// **************************************************
#endif

#if 1 /* 1280x340 */
// TC358768XBG Software Reset
// **************************************************
SPI_32bit_4wire_write(0x00020001);//SYSctl, S/W Reset
LCD_delay_ms(1);
SPI_32bit_4wire_write(0x00020000);//SYSctl, S/W Reset release
// TC358768XBG PLL,Clock Setting
// **************************************************
SPI_32bit_4wire_write(0x0016207D);//PLL Control Register 0 (PLL_PRD,PLL_FBD)
SPI_32bit_4wire_write(0x00180603);//PLL_FRS,PLL_LBWS, PLL oscillation enable
LCD_delay_ms(1);
SPI_32bit_4wire_write(0x00180613);//PLL_FRS,PLL_LBWS, PLL clock out enable
// **************************************************
// TC358768XBG DPI Input Control
// **************************************************
SPI_32bit_4wire_write(0x0006012C);//FIFO Control Register
// **************************************************
// TC358768XBG D-PHY Setting
// **************************************************
SPI_32bit_4wire_write(0x01400000);//D-PHY Clock lane enable
SPI_32bit_4wire_write(0x01420000);//
SPI_32bit_4wire_write(0x01440000);//D-PHY Data lane0 enable
SPI_32bit_4wire_write(0x01460000);//
SPI_32bit_4wire_write(0x01480000);//D-PHY Data lane1 enable
SPI_32bit_4wire_write(0x014A0000);//
SPI_32bit_4wire_write(0x014C0000);//D-PHY Data lane2 enable
SPI_32bit_4wire_write(0x014E0000);//
SPI_32bit_4wire_write(0x01500000);//D-PHY Data lane3 enable
SPI_32bit_4wire_write(0x01520000);//
SPI_32bit_4wire_write(0x01000002);//D-PHY Clock lane control
SPI_32bit_4wire_write(0x01020000);//
SPI_32bit_4wire_write(0x01040002);//D-PHY Data lane0 control
SPI_32bit_4wire_write(0x01060000);//
SPI_32bit_4wire_write(0x01080002);//D-PHY Data lane1 control
SPI_32bit_4wire_write(0x010A0000);//
SPI_32bit_4wire_write(0x010C0002);//D-PHY Data lane2 control
SPI_32bit_4wire_write(0x010E0000);//
SPI_32bit_4wire_write(0x01100002);//D-PHY Data lane3 control
SPI_32bit_4wire_write(0x01120000);//
// **************************************************
// TC358768XBG DSI-TX PPI Control
// **************************************************
SPI_32bit_4wire_write(0x02101644);//LINEINITCNT
SPI_32bit_4wire_write(0x02120000);//
SPI_32bit_4wire_write(0x02140003);//LPTXTIMECNT
SPI_32bit_4wire_write(0x02160000);//
SPI_32bit_4wire_write(0x02182003);//TCLK_HEADERCNT
SPI_32bit_4wire_write(0x021A0000);//
SPI_32bit_4wire_write(0x021C0001);//TCLK_TRAILCNT
SPI_32bit_4wire_write(0x021E0000);//
SPI_32bit_4wire_write(0x02200603);//THS_HEADERCNT
SPI_32bit_4wire_write(0x02220000);//
SPI_32bit_4wire_write(0x02244268);//TWAKEUPCNT
SPI_32bit_4wire_write(0x02260000);//
SPI_32bit_4wire_write(0x0228000B);//TCLK_POSTCNT
SPI_32bit_4wire_write(0x022A0000);//
SPI_32bit_4wire_write(0x022C0001);//THS_TRAILCNT
SPI_32bit_4wire_write(0x022E0000);//
SPI_32bit_4wire_write(0x02300005);//HSTXVREGCNT
SPI_32bit_4wire_write(0x02320000);//
SPI_32bit_4wire_write(0x0234001F);//HSTXVREGEN enable
SPI_32bit_4wire_write(0x02360000);//
SPI_32bit_4wire_write(0x02380000);//DSI clock Enable/Disable during LP
SPI_32bit_4wire_write(0x023A0000);//
SPI_32bit_4wire_write(0x023C0005);//BTACNTRL1
SPI_32bit_4wire_write(0x023E0003);//
SPI_32bit_4wire_write(0x02040001);//STARTCNTRL
SPI_32bit_4wire_write(0x02060000);//
// **************************************************
// TC358768XBG DSI-TX Timing Control
// **************************************************
SPI_32bit_4wire_write(0x06200001);//Sync Pulse/Sync Event mode setting
SPI_32bit_4wire_write(0x0622002A);//V Control Register1
SPI_32bit_4wire_write(0x06240028);//V Control Register2
SPI_32bit_4wire_write(0x06260154);//V Control Register3
SPI_32bit_4wire_write(0x062801A8);//H Control Register1
SPI_32bit_4wire_write(0x062A014D);//H Control Register2
SPI_32bit_4wire_write(0x062C0F00);//H Control Register3
SPI_32bit_4wire_write(0x05180001);//DSI Start
SPI_32bit_4wire_write(0x051A0000);//
// **************************************************
// LCDD (Peripheral) Setting
// **************************************************
PowerOn_Panel();
// **************************************************
// Set to HS mode
// **************************************************
SPI_32bit_4wire_write(0x05000086);//DSI lane setting, DSI mode=HS
SPI_32bit_4wire_write(0x0502A300);//bit set
SPI_32bit_4wire_write(0x05008000);//Switch to DSI mode
SPI_32bit_4wire_write(0x0502C300);//
// **************************************************
// Host: RGB(DPI) input start
// **************************************************
SPI_32bit_4wire_write(0x00080033);//DSI-TX Format setting
SPI_32bit_4wire_write(0x0050003E);//DSI-TX Pixel stream packet Data Type setting
SPI_32bit_4wire_write(0x00320000);//HSYNC Polarity
SPI_32bit_4wire_write(0x00040044);//Configuration Control Register
// **************************************************
// LCDD (Peripheral) Setting
// **************************************************
#endif

#if 0 /* 1280x320 */
// TC358768XBG Software Reset
// **************************************************
SPI_32bit_4wire_write(0x00020001);//SYSctl, S/W Reset
LCD_delay_ms(1);
SPI_32bit_4wire_write(0x00020000);//SYSctl, S/W Reset release
// TC358768XBG PLL,Clock Setting
// **************************************************
SPI_32bit_4wire_write(0x0016207D);//PLL Control Register 0 (PLL_PRD,PLL_FBD)
SPI_32bit_4wire_write(0x00180603);//PLL_FRS,PLL_LBWS, PLL oscillation enable
LCD_delay_ms(1);
SPI_32bit_4wire_write(0x00180613);//PLL_FRS,PLL_LBWS, PLL clock out enable
// **************************************************
// TC358768XBG DPI Input Control
// **************************************************
SPI_32bit_4wire_write(0x0006012C);//FIFO Control Register
// **************************************************
// TC358768XBG D-PHY Setting
// **************************************************
SPI_32bit_4wire_write(0x01400000);//D-PHY Clock lane enable
SPI_32bit_4wire_write(0x01420000);//
SPI_32bit_4wire_write(0x01440000);//D-PHY Data lane0 enable
SPI_32bit_4wire_write(0x01460000);//
SPI_32bit_4wire_write(0x01480000);//D-PHY Data lane1 enable
SPI_32bit_4wire_write(0x014A0000);//
SPI_32bit_4wire_write(0x014C0000);//D-PHY Data lane2 enable
SPI_32bit_4wire_write(0x014E0000);//
SPI_32bit_4wire_write(0x01500000);//D-PHY Data lane3 enable
SPI_32bit_4wire_write(0x01520000);//
SPI_32bit_4wire_write(0x01000002);//D-PHY Clock lane control
SPI_32bit_4wire_write(0x01020000);//
SPI_32bit_4wire_write(0x01040002);//D-PHY Data lane0 control
SPI_32bit_4wire_write(0x01060000);//
SPI_32bit_4wire_write(0x01080002);//D-PHY Data lane1 control
SPI_32bit_4wire_write(0x010A0000);//
SPI_32bit_4wire_write(0x010C0002);//D-PHY Data lane2 control
SPI_32bit_4wire_write(0x010E0000);//
SPI_32bit_4wire_write(0x01100002);//D-PHY Data lane3 control
SPI_32bit_4wire_write(0x01120000);//
// **************************************************
// TC358768XBG DSI-TX PPI Control
// **************************************************
SPI_32bit_4wire_write(0x02101644);//LINEINITCNT
SPI_32bit_4wire_write(0x02120000);//
SPI_32bit_4wire_write(0x02140003);//LPTXTIMECNT
SPI_32bit_4wire_write(0x02160000);//
SPI_32bit_4wire_write(0x02182003);//TCLK_HEADERCNT
SPI_32bit_4wire_write(0x021A0000);//
SPI_32bit_4wire_write(0x021C0001);//TCLK_TRAILCNT
SPI_32bit_4wire_write(0x021E0000);//
SPI_32bit_4wire_write(0x02200603);//THS_HEADERCNT
SPI_32bit_4wire_write(0x02220000);//
SPI_32bit_4wire_write(0x02244268);//TWAKEUPCNT
SPI_32bit_4wire_write(0x02260000);//
SPI_32bit_4wire_write(0x0228000B);//TCLK_POSTCNT
SPI_32bit_4wire_write(0x022A0000);//
SPI_32bit_4wire_write(0x022C0001);//THS_TRAILCNT
SPI_32bit_4wire_write(0x022E0000);//
SPI_32bit_4wire_write(0x02300005);//HSTXVREGCNT
SPI_32bit_4wire_write(0x02320000);//
SPI_32bit_4wire_write(0x0234001F);//HSTXVREGEN enable
SPI_32bit_4wire_write(0x02360000);//
SPI_32bit_4wire_write(0x02380000);//DSI clock Enable/Disable during LP
SPI_32bit_4wire_write(0x023A0000);//
SPI_32bit_4wire_write(0x023C0005);//BTACNTRL1
SPI_32bit_4wire_write(0x023E0003);//
SPI_32bit_4wire_write(0x02040001);//STARTCNTRL
SPI_32bit_4wire_write(0x02060000);//
// **************************************************
// TC358768XBG DSI-TX Timing Control
// **************************************************
SPI_32bit_4wire_write(0x06200001);//Sync Pulse/Sync Event mode setting
SPI_32bit_4wire_write(0x0622002A);//V Control Register1
SPI_32bit_4wire_write(0x06240028);//V Control Register2
SPI_32bit_4wire_write(0x06260140);//V Control Register3
SPI_32bit_4wire_write(0x062801A8);//H Control Register1
SPI_32bit_4wire_write(0x062A014D);//H Control Register2
SPI_32bit_4wire_write(0x062C0F00);//H Control Register3
SPI_32bit_4wire_write(0x05180001);//DSI Start
SPI_32bit_4wire_write(0x051A0000);//
// **************************************************
// LCDD (Peripheral) Setting
// **************************************************
PowerOn_Panel();
// **************************************************
// Set to HS mode
// **************************************************
SPI_32bit_4wire_write(0x05000086);//DSI lane setting, DSI mode=HS
SPI_32bit_4wire_write(0x0502A300);//bit set
SPI_32bit_4wire_write(0x05008000);//Switch to DSI mode
SPI_32bit_4wire_write(0x0502C300);//
// **************************************************
// Host: RGB(DPI) input start
// **************************************************
SPI_32bit_4wire_write(0x00080033);//DSI-TX Format setting
SPI_32bit_4wire_write(0x0050003E);//DSI-TX Pixel stream packet Data Type setting
SPI_32bit_4wire_write(0x00320000);//HSYNC Polarity
SPI_32bit_4wire_write(0x00040044);//Configuration Control Register
// **************************************************
// LCDD (Peripheral) Setting
// **************************************************

#endif
}

static MMP_ERR MMPF_LCD_InitRGBOder(MMPF_PANEL_ATTR* pAttr)
{
	DSPY_DECL;
	MMP_USHORT usTemp = 0;

	switch(pAttr->ubEvenLnOrder)
	{
		case LCD_SPI_PIX_ORDER_RGB:
			usTemp |= SPI_EVEN_LINE_RGB;
		break;
		case LCD_SPI_PIX_ORDER_RBG:
			usTemp |= SPI_EVEN_LINE_RBG;
		break;
		case LCD_SPI_PIX_ORDER_GRB:
			usTemp |= SPI_EVEN_LINE_GRB;
		break;
		case LCD_SPI_PIX_ORDER_GBR:
			usTemp |= SPI_EVEN_LINE_GBR;
		break;
		case LCD_SPI_PIX_ORDER_BRG:
			usTemp |= SPI_EVEN_LINE_BRG;
		break;
		case LCD_SPI_PIX_ORDER_BGR:
			usTemp |= SPI_EVEN_LINE_BGR;
		break;
	}

	switch(pAttr->ubOddLnOrder)
	{
		case LCD_SPI_PIX_ORDER_RGB:
			usTemp |= SPI_ODD_LINE_RGB;
		break;
		case LCD_SPI_PIX_ORDER_RBG:
			usTemp |= SPI_ODD_LINE_RBG;
		break;
		case LCD_SPI_PIX_ORDER_GRB:
			usTemp |= SPI_ODD_LINE_GRB;
		break;
		case LCD_SPI_PIX_ORDER_GBR:
			usTemp |= SPI_ODD_LINE_GBR;
		break;
		case LCD_SPI_PIX_ORDER_BRG:
			usTemp |= SPI_ODD_LINE_BRG;
		break;
		case LCD_SPI_PIX_ORDER_BGR:
			usTemp |= SPI_ODD_LINE_BGR;
		break;
	}

    DSPY_WR_B(DSPY_RGB2_DELTA_MODE, usTemp);

	return MMP_ERR_NONE;
}


void RTNA_LCD_InitMainLCD(void)
{
#if (SECONDARY_DISPLAY == 0)
	RTNA_DBG_Str(LCD_DBG_LEVEL, "### RTNA_LCD_InitMainLCD - S8500L0_1280x320\r\n");

	m_PanelAttr.usDotClkRatio 	= 3;

  #if 0
	m_PanelAttr.usHBPorch		= 20; 
	m_PanelAttr.usHBlanking		= 40;
	m_PanelAttr.usHSyncW		= 2;
	m_PanelAttr.usVBPorch		= 16;
	m_PanelAttr.usVBlanking 	= 22;
	m_PanelAttr.usVSyncW		= 2;
		
  #else	
	m_PanelAttr.usHBPorch		= 88; 
	m_PanelAttr.usHBlanking     = 160;
	m_PanelAttr.usHSyncW		= 24;
	m_PanelAttr.usVBPorch		= 34;
	m_PanelAttr.usVBlanking 	= 47;
	m_PanelAttr.usVSyncW		= 2; 	
  #endif	

	m_PanelAttr.usPRT2HdotClk 	= 2;

	m_PanelAttr.bDeltaRBG 		= MMP_FALSE;
	m_PanelAttr.bDummyRBG 		= MMP_FALSE;
	m_PanelAttr.ubEvenLnOrder   = LCD_SPI_PIX_ORDER_BGR;
	m_PanelAttr.ubOddLnOrder    = LCD_SPI_PIX_ORDER_BGR;

	// Window setting (For drawing test pattern)
	m_PanelAttr.ubDispWinId 	= LCD_DISP_WIN_PIP;
	m_PanelAttr.usWinStX 		= 0;
	m_PanelAttr.usWinStY 		= 0;
	m_PanelAttr.usWinW 			= PANEL_WIDTH;
	m_PanelAttr.usWinH 			= PANEL_HEIGHT;
	m_PanelAttr.usWinBPP 		= 2;
	m_PanelAttr.usWinFmt 		= LCD_WIN_FMT_16BPP;
	m_PanelAttr.ulWinAddr 		= 0x03000000;

	// CarDV
	#if ((16 == LCD_MODEL_RATIO_X) && (9 == LCD_MODEL_RATIO_Y))
	m_PanelAttr.ubRatio 		= LCD_RATIO_16_9;
	#else
	m_PanelAttr.ubRatio 		= LCD_RATIO_4_3;
	#endif

	MMPF_LCD_InitPanel(&m_PanelAttr);
	//MMPF_LCD_InitRGBOder(&m_PanelAttr);
#endif
}

void RTNA_LCD_Init2ndLCD(void)
{
#if (SECONDARY_DISPLAY == 1)
    // TBD
#endif
}

void RTNA_LCD_Init(void)
{
	RTNA_DBG_Str(0, "### RTNA_LCD_Init for EK79202_1280x320...\r\n");

////step 1: start rgb output
#if (SECONDARY_DISPLAY == 1)
	RTNA_LCD_Init2ndLCD();
#else
	RTNA_LCD_InitMainLCD();
#endif
////step 2: reset tc358778 ic
	#if 0
	MMPF_PIO_EnableOutputMode(LCD_RST_GPIO_PIN, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_SetData(LCD_RST_GPIO_PIN, GPIO_HIGH, MMP_TRUE);
	MMPF_OS_Sleep_MS(500);
	MMPF_PIO_SetData(LCD_RST_GPIO_PIN, GPIO_LOW, MMP_TRUE);
	MMPF_OS_Sleep_MS(50);
	MMPF_PIO_SetData(LCD_RST_GPIO_PIN, GPIO_HIGH, MMP_TRUE);
	MMPF_OS_Sleep_MS(10);
	#endif
	//PowerOn_Panel();
///step 3: init tc358778
    RTNA_Init_MIPI_DSI();

}

//============================================================================//
void RTNA_LCD_RGB_Test(void)
{
#if (ENABLE_LCD_TEST_PATTERN)
#if (USE_PWM_FOR_LCD_BACKLIGHT)
    //MMPF_PIO_Enable(LCD_GPIO_BACK_LIGHT, MMP_FALSE);
	MMPF_PWM_Initialize();
	MMPF_PWM_OutputPulse(LCD_BACKLIGHT_PWM_UNIT_PIN, MMP_TRUE, LCD_BACKLIGHT_PWM_FREQ,LCD_BACKLIGHT_PWM_DUTY, MMP_TRUE, MMP_FALSE, NULL, 0);
#else
  #if defined(LCD_GPIO_BACK_LIGHT)
	/* Force turn-on LCD backlight */
	if (LCD_GPIO_BACK_LIGHT != MMP_GPIO_MAX)
	{
	    MMPF_PIO_PadConfig(LCD_GPIO_BACK_LIGHT, PAD_OUT_DRIVING(0), MMP_TRUE);
		MMPF_PIO_EnableOutputMode(LCD_GPIO_BACK_LIGHT, MMP_TRUE, MMP_TRUE);
		RTNA_DBG_Str(LCD_DBG_LEVEL, "dbg-set LCD_BL to output mode\r\n");

		MMPF_PIO_SetData(LCD_GPIO_BACK_LIGHT, !LCD_GPIO_BACK_LIGHT_ACT_LEVEL, MMP_TRUE);
		Delay_ms(20);
		RTNA_DBG_Str(LCD_DBG_LEVEL, "dbg-set LCD_BL to low\r\n");

		MMPF_PIO_SetData(LCD_GPIO_BACK_LIGHT, LCD_GPIO_BACK_LIGHT_ACT_LEVEL, MMP_TRUE);
		Delay_ms(20);
		RTNA_DBG_Str(LCD_DBG_LEVEL, "dbg-set LCD_BL to high\r\n");
	}
  #endif
#endif
	MMPF_LCD_DrawTestPattern(&m_PanelAttr);
    Delay_ms(5000);
#endif
}

void RTNA_LCD_Direction(LCD_DIR dir)
{
	// TBD
}

void RTNA_LCD_SetReg(MMP_ULONG reg, MMP_UBYTE value)
{
	// TBD
}

void RTNA_LCD_GetReg(MMP_ULONG reg, MMP_ULONG *value)
{
	// TBD
}

void RTNA_LCD_AdjustBrightness(MMP_UBYTE level)
{
	// TBD
}

void RTNA_LCD_AdjustBrightness_R(MMP_UBYTE level)
{
	// TBD
}

void RTNA_LCD_AdjustBrightness_B(MMP_UBYTE level)
{
	// TBD
}

void RTNA_LCD_AdjustContrast(MMP_UBYTE level)
{
	// TBD
}

void RTNA_LCD_AdjustContrast_R(MMP_BYTE level)
{
	// TBD
}

void RTNA_LCD_AdjustContrast_B(MMP_BYTE level)
{
	// TBD
}

/*
 * Set Panel Properties
 * Width, Height, Color Depth, Display Type
 */
MMPF_PANEL_ATTR* RTNA_LCD_GetAttr(void)
{
	return &m_PanelAttr;
}

