/*
 * PCBA config file
 */
#ifndef	__CONFIG_PANEL__
#define	__CONFIG_PANEL__

#include "dsc_key.h"
#include "all_fw.h"
#include "mmps_3gprecd.h"
#include "mmps_pio.h"
#include "mmpf_sensor.h"
#include "AHC_common.h"
#include "AHC_Macro.h"

/*===========================================================================
 *  Panel Config
 *===========================================================================*/
#define VERTICAL_LCD_DEGREE_0       	(0)     // No rotate
#define VERTICAL_LCD_DEGREE_90         	(1)     // 90 degree rotate
#define VERTICAL_LCD_DEGREE_270        	(2)     // 270 degree rotate


//PANEL,Model No,Driver IC,Resolution
#define	PANEL_V920TW01A_EK79202_1280x320_MIPI		(0)//DriverIC:EK79202, PanelSize:9.2"
#define	PANEL_S8500L0_EK79202_1280x320_MIPI			(1)//PanelSize:8.5"
#define	PANEL_TG078UW019A0_EK79030_400x1280_MIPI	(2)//PanelSize:7.84"
#define	PANEL_WTL098801G01_OTA7290B_400x1600_MIPI	(3)//PanelSize:9.88"
#define	PANEL_854x480_MIPI							(4)
#define	PANEL_1280x720_MIPI							(5)
#define	PANEL_320x240_RGB							(6)

#define PANEL_USE									PANEL_320x240_RGB

#if (PANEL_USE == PANEL_V920TW01A_EK79202_1280x320_MIPI)
	#undef 	LCD_GPIO_BACK_LIGHT
	#define LCD_GPIO_BACK_LIGHT             (MMP_GPIO_MAX)  
	#define LCD_GPIO_BACK_LIGHT_ACT_LEVEL 	(GPIO_HIGH)

	#define LCD_GPIO_RESET                  (MMP_GPIO63) 
	#define LCD_GPIO_RESET_ACT_LEVEL        (GPIO_LOW)

	#define LCD_GPIO_ENABLE                 (MMP_GPIO24)
	#define LCD_GPIO_ENABLE_ACT_LEVEL 		(GPIO_HIGH)

	//Panel size of different project
	#define DISP_WIDTH						1280
	#define DISP_HEIGHT						320
	#define ORIGINAL_OSD_W					(320)
	#define ORIGINAL_OSD_H					(240)

	//#define VERTICAL_PANEL			//DISP_HEIGHT > DISP_WIDTH
    #define VERTICAL_LCD                (VERTICAL_LCD_DEGREE_0)     // 0: Use Horizontal LCD

	#if !defined(MINIBOOT_FW)
	//Auto calulate OSD Multiples. if user want to use their own UI, please force OSD_DISPLAY_SCALE_LCD to 1.
	#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
		#define OSD_DISPLAY_SCALE_LCD           (MIN((DISP_WIDTH/ORIGINAL_OSD_W),(DISP_HEIGHT/ORIGINAL_OSD_H)))
	#else
		#define OSD_DISPLAY_SCALE_LCD          	(MIN((DISP_HEIGHT/ORIGINAL_OSD_W),(DISP_WIDTH/ORIGINAL_OSD_H)))
	#endif
	#endif

#elif (PANEL_USE == PANEL_S8500L0_EK79202_1280x320_MIPI)
	/*confirm MCP:
				:Mipi_TC358778.C 
				:lcd_S85000L0_1280x320_MIPI.C
	*/
	#undef 	LCD_GPIO_BACK_LIGHT
	#define LCD_GPIO_BACK_LIGHT             (MMP_GPIO_MAX)    
	#define LCD_GPIO_BACK_LIGHT_ACT_LEVEL 	(GPIO_HIGH)

	#define LCD_GPIO_RESET                  (MMP_GPIO63)  
	#define LCD_GPIO_RESET_ACT_LEVEL        (GPIO_LOW)

	#define LCD_GPIO_ENABLE                 (MMP_GPIO24)
	#define LCD_GPIO_ENABLE_ACT_LEVEL 		(GPIO_HIGH)

	//Panel size of different project
	#define DISP_WIDTH						1280
	#define DISP_HEIGHT						340
	#define ORIGINAL_OSD_W					(320)
	#define ORIGINAL_OSD_H					(240)

	//#define VERTICAL_PANEL			//DISP_HEIGHT > DISP_WIDTH
    #define VERTICAL_LCD                (VERTICAL_LCD_DEGREE_0)     // 0: Use Horizontal LCD

	#if !defined(MINIBOOT_FW)
	//Auto calulate OSD Multiples. if user want to use their own UI, please force OSD_DISPLAY_SCALE_LCD to 1.
	#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
		#define OSD_DISPLAY_SCALE_LCD           (MIN((DISP_WIDTH/ORIGINAL_OSD_W),(DISP_HEIGHT/ORIGINAL_OSD_H)))
	#else
		#define OSD_DISPLAY_SCALE_LCD          	(MIN((DISP_HEIGHT/ORIGINAL_OSD_W),(DISP_WIDTH/ORIGINAL_OSD_H)))
	#endif
	#endif

#elif (PANEL_USE == PANEL_TG078UW019A0_EK79030_400x1280_MIPI)

	#undef 	LCD_GPIO_BACK_LIGHT
	#define LCD_GPIO_BACK_LIGHT             (MMP_GPIO_MAX)   
	#define LCD_GPIO_BACK_LIGHT_ACT_LEVEL 	(GPIO_HIGH)

	#define LCD_GPIO_RESET                  (MMP_GPIO63) 
	#define LCD_GPIO_RESET_ACT_LEVEL        (GPIO_LOW)

	#define LCD_GPIO_ENABLE                 (MMP_GPIO24)
	#define LCD_GPIO_ENABLE_ACT_LEVEL 		(GPIO_HIGH)

	//Panel size of different project
	#define DISP_WIDTH						400
	#define DISP_HEIGHT						1280
	#define ORIGINAL_OSD_W					(320)
	#define ORIGINAL_OSD_H					(240)

	#define VERTICAL_PANEL					//DISP_HEIGHT > DISP_WIDTH

    #undef 	FLM_GPIO_NUM
    //#define FLM_GPIO_NUM 				(MMP_GPIO123) // PLCD_FLM -->> LCD_FLM
    #define VERTICAL_LCD                (VERTICAL_LCD_DEGREE_270)     // 0: Use Horizontal LCD

	#if !defined(MINIBOOT_FW)
	//Auto calulate OSD Multiples. if user want to use their own UI, please force OSD_DISPLAY_SCALE_LCD to 1.
	#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
		#define OSD_DISPLAY_SCALE_LCD           (MIN((DISP_WIDTH/ORIGINAL_OSD_W),(DISP_HEIGHT/ORIGINAL_OSD_H)))
	#else
		#define OSD_DISPLAY_SCALE_LCD          	(MIN((DISP_HEIGHT/ORIGINAL_OSD_W),(DISP_WIDTH/ORIGINAL_OSD_H)))
	#endif
	#endif

#elif (PANEL_USE == PANEL_WTL098801G01_OTA7290B_400x1600_MIPI)

	#undef 	LCD_GPIO_BACK_LIGHT
	#define LCD_GPIO_BACK_LIGHT             (MMP_GPIO_MAX)   
	#define LCD_GPIO_BACK_LIGHT_ACT_LEVEL 	(GPIO_HIGH)

	#define LCD_GPIO_RESET                  (MMP_GPIO63) 
	#define LCD_GPIO_RESET_ACT_LEVEL        (GPIO_LOW)

	#define LCD_GPIO_ENABLE                 (MMP_GPIO24)
	#define LCD_GPIO_ENABLE_ACT_LEVEL 		(GPIO_HIGH)

	//Panel size of different project
	#define DISP_WIDTH						400
	#define DISP_HEIGHT						1600
	#define ORIGINAL_OSD_W					(320)
	#define ORIGINAL_OSD_H					(240)

	#define VERTICAL_PANEL					//DISP_HEIGHT > DISP_WIDTH
    #undef 	FLM_GPIO_NUM
    //#define FLM_GPIO_NUM 				(MMP_GPIO123) // PLCD_FLM -->> LCD_FLM
    #define VERTICAL_LCD                (VERTICAL_LCD_DEGREE_270)     // 0: Use Horizontal LCD

	#if !defined(MINIBOOT_FW)
	//Auto calulate OSD Multiples. if user want to use their own UI, please force OSD_DISPLAY_SCALE_LCD to 1.
	#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
		#define OSD_DISPLAY_SCALE_LCD           (MIN((DISP_WIDTH/ORIGINAL_OSD_W),(DISP_HEIGHT/ORIGINAL_OSD_H)))
	#else
		#define OSD_DISPLAY_SCALE_LCD          	(MIN((DISP_HEIGHT/ORIGINAL_OSD_W),(DISP_WIDTH/ORIGINAL_OSD_H)))
	#endif
	#endif

#elif (PANEL_USE == PANEL_854x480_MIPI)

	#undef 	LCD_GPIO_BACK_LIGHT
	#define LCD_GPIO_BACK_LIGHT             (MMP_GPIO_MAX)    // AGPIO3 -->> LCD_BL
	#define LCD_GPIO_BACK_LIGHT_ACT_LEVEL 	(GPIO_HIGH)

	//#define LCD_GPIO_ENABLE                 (MMP_GPIO_MAX)
	#define LCD_GPIO_ENABLE_ACT_LEVEL 		(GPIO_HIGH)

	//Panel size of different project
	#define DISP_WIDTH					(480)//(854)
	#define DISP_HEIGHT					(854)//(480)
	#define ORIGINAL_OSD_W					(320)
	#define ORIGINAL_OSD_H					(240)

	#define VERTICAL_PANEL			//DISP_HEIGHT > DISP_WIDTH
    #undef 	FLM_GPIO_NUM
    //#define FLM_GPIO_NUM 				(MMP_GPIO123) // PLCD_FLM -->> LCD_FLM
    #define VERTICAL_LCD                (VERTICAL_LCD_DEGREE_90)     // 0: Use Horizontal LCD

	#if !defined(MINIBOOT_FW)
	//Auto calulate OSD Multiples. if user want to use their own UI, please force OSD_DISPLAY_SCALE_LCD to 1.
	#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
		#define OSD_DISPLAY_SCALE_LCD           (MIN((DISP_WIDTH/ORIGINAL_OSD_W),(DISP_HEIGHT/ORIGINAL_OSD_H)))
	#else
		#define OSD_DISPLAY_SCALE_LCD          	(MIN((DISP_HEIGHT/ORIGINAL_OSD_W),(DISP_WIDTH/ORIGINAL_OSD_H)))
	#endif
	#endif

#elif (PANEL_USE == PANEL_1280x720_MIPI)

	#undef 	LCD_GPIO_BACK_LIGHT
	#define LCD_GPIO_BACK_LIGHT             (MMP_GPIO_MAX)    // AGPIO3 -->> LCD_BL
	#define LCD_GPIO_BACK_LIGHT_ACT_LEVEL 	(GPIO_HIGH)

	#define LCD_GPIO_RESET                  (MMP_GPIO63)  // PCGPIO31
	#define LCD_GPIO_RESET_ACT_LEVEL        (GPIO_LOW)

	#define LCD_GPIO_ENABLE                 (MMP_GPIO24)
	#define LCD_GPIO_ENABLE_ACT_LEVEL 		(GPIO_HIGH)

	//Panel size of different project
	#define DISP_WIDTH						1280//320
	#define DISP_HEIGHT						720//240

	#define VERTICAL_PANEL			//DISP_HEIGHT > DISP_WIDTH
	#define VERTICAL_LCD                (VERTICAL_LCD_DEGREE_90)     // 0: Use Horizontal LCD
	#define ORIGINAL_OSD_W					(1280)//(320)
	#define ORIGINAL_OSD_H					(720)//(240)

	#if !defined(MINIBOOT_FW)
	//Auto calulate OSD Multiples. if user want to use their own UI, please force OSD_DISPLAY_SCALE_LCD to 1.
	#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
		#define OSD_DISPLAY_SCALE_LCD           1//(MIN((DISP_WIDTH/ORIGINAL_OSD_W),(DISP_HEIGHT/ORIGINAL_OSD_H)))
	#else
		#define OSD_DISPLAY_SCALE_LCD          	1//(MIN((DISP_HEIGHT/ORIGINAL_OSD_W),(DISP_WIDTH/ORIGINAL_OSD_H)))
	#endif
	#endif

#else

	#undef 	LCD_GPIO_BACK_LIGHT
	#define LCD_GPIO_BACK_LIGHT             (MMP_GPIO_MAX)    // AGPIO3 -->> LCD_BL
	#define LCD_GPIO_BACK_LIGHT_ACT_LEVEL 	(GPIO_HIGH)

	#define LCD_GPIO_RESET                  (MMP_GPIO_MAX)  // PCGPIO31   gpio 63   liao
	#define LCD_GPIO_RESET_ACT_LEVEL        (GPIO_LOW)

	#define LCD_GPIO_ENABLE                 (MMP_GPIO24)
	#define LCD_GPIO_ENABLE_ACT_LEVEL 		(GPIO_HIGH)

	//Panel size of different project
	#define DISP_WIDTH						320
	#define DISP_HEIGHT						480

	//#define VERTICAL_PANEL			//DISP_HEIGHT > DISP_WIDTH
	#define ORIGINAL_OSD_W					(480)
	#define ORIGINAL_OSD_H					(320)
	#define VERTICAL_LCD                (VERTICAL_LCD_DEGREE_270)     // 0: Use Horizontal LCD

	#if !defined(MINIBOOT_FW)
	//Auto calulate OSD Multiples. if user want to use their own UI, please force OSD_DISPLAY_SCALE_LCD to 1.
	#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
		#define OSD_DISPLAY_SCALE_LCD           (MIN((DISP_WIDTH/ORIGINAL_OSD_W),(DISP_HEIGHT/ORIGINAL_OSD_H)))
	#else
		#define OSD_DISPLAY_SCALE_LCD          	(MIN((DISP_HEIGHT/ORIGINAL_OSD_W),(DISP_WIDTH/ORIGINAL_OSD_H)))
	#endif
	#endif

#endif


// Temp. define
#define LCD_IF_NONE 					(0)
#define LCD_IF_SERIAL 					(1)
#define LCD_IF_PARALLEL 				(2)
#define LCD_IF_RGB 						(3)
#define LCD_IF 							(LCD_IF_RGB)

// Set up LCD display Width/Height ratio. The default is 4/3 if No these definition.
#define LCD_MODEL_RATIO_X               (16)
#define LCD_MODEL_RATIO_Y               (9)

#define CUS_LCD_BRIGHTNESS              80
#define CUS_LCD_CONTRAST                75
//Original OSD W&H in AIT SDK 
//#define ORIGINAL_OSD_W					(320)
//#define ORIGINAL_OSD_H					(240)

#endif // __CONFIG_PANEL__