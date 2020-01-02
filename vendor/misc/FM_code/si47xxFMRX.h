//-----------------------------------------------------------------------------
//
// si47xxFMRX.h
//
// Contains the function prototypes for the functions contained in si47xxFMRX.c
//
//-----------------------------------------------------------------------------
#ifndef _SI47XXFMRX_H_
#define _SI47XXFMRX_H_
#include "commanddefs.h"
#include "propertydefs.h"

//typedef  MMP_UBYTE u8;
//typedef MMP_USHORT u16;

//#define idata
//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
extern char WaitSTCInterrupt;
extern char PoweredUp;
extern char GpioSet;   //zjh add





typedef enum 
{
	SI47XX_JAPAN,
	SI47XX_SCHOOL,	
	SI47XX_EUROPE,	
	SI47XX_USA1, 
	SI47XX_USA2,
	SI47XX_RUSSIAN,
	SI47XX_RUSSIAN1,
	
}country_enum; // Could be expanded


//-----------------------------------------------------------------------------
// Function prototypes for si47xxFMRX.c
//-----------------------------------------------------------------------------
void si47xxFMRX_powerup(void);
void si47xxFMRX_powerdown(void);
void si47xxFMRX_initialize(void);
void si47xxFMRX_configure(void);
void si47xxFMRX_set_volume(u8 volume);
void si47xxFMRX_mute(u8 mute);
u8 si47xxFMRX_tune(u16 frequency);
u8 si47xxFMRX_seek(u8 seekup, u8 seekmode);
u16 si47xxFMRX_get_frequency(void);
u32 si47xxFMRX_get_rssi(void);
u32 si47xxFMRX_get_snr(void);

u8 quickAFTune(u16 freq);


//-----------------------------------------------------------------------------
// Function prototypes for autoseek.c
//-----------------------------------------------------------------------------
u8 si47xxFMRX_autoseek(void);

//-----------------------------------------------------------------------------
// Function prototypes for si47xx_low.c
//-----------------------------------------------------------------------------
void si47xx_set_property(u16 propNumber, u16 propValue);
void si47xx_getPartInformation(void);

#endif
