#ifndef _FM_I2C_H_
#define _FM_I2C_H_

#include "mmp_gpio_inc.h"


#define PMP_HWI2C_CLK_PIN			MMP_GPIO0//fm_clk
#define PMP_HWI2C_DAT_PIN			MMP_GPIO1//fm_sdk

//For FM
#define FM_PATH_SELECT_GPIO              (MMP_GPIO61)//long 4-14  RSTN
#define FM_PATH_SELECT_SET          	(GPIO_LOW)//long 4-14
#define FM_PATH_SELECT_RESET		(GPIO_HIGH)//long 4


#define FM_I2C_SLAVE_ADDR                  0xc6//0xc2 machine addr

#define FM_I2C_SLAVE_ADDR_R              1//machine read addr
#define FM_I2C_SLAVE_ADDR_W		0//machine write addr

#define AHC_I2C_HANDLE		void*

//#define SET_PROPERTY 0x12

//#define CTS			 0x80



/*lyj 20190605 define*/

typedef struct _PowerUpFlag
{
	unsigned char Fm;
	unsigned char Am;
	unsigned char AUX;
	unsigned char USB;
	unsigned char blueTooth;
}PowerUpFlag;
typedef struct _linePowerOn
{
	unsigned short aFreq;
	unsigned char ReturnFlag;
	unsigned char uiStatus;

}PowerOnSaveTheStatus;


/*lyj end*/
#if 0
typedef enum 
{
	SI47XX_JAPAN,
	SI47XX_SCHOOL,	
	SI47XX_EUROPE,	
	SI47XX_USA1, 
	SI47XX_CHA,
	SI47XX_RUSSIAN,
	SI47XX_RUSSIAN1,
	
}country_enum; 

typedef enum 
{
	SI47XX_AM_EUROPE,	
	SI47XX_AM_OTHER, 

}am_country_enum; // Could be expanded

#endif

//void fm4730_init(void);
void BD3490_init(void);
void Volume_set(MMP_USHORT volume);
void Volume_set_EX(MMP_USHORT volume);
void Close_channal(void);
void Set_flag(MMP_UBYTE FuncSelect);
MMP_UBYTE Get_flag(void);
//void fm_Si4730_Init(void);

#endif




