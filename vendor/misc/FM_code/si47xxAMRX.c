//-----------------------------------------------------------------------------
//
// si47xxFAMRX.c
//
// Contains the AM radio functions with the exceptions of autoseek.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


#include  "si47xx.h"
#include "si47xxAMRX.h"


#define  AM_AGC_OVERRIDE 				0x48
#define  AM_AGC_STATUS					0x47



//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define POWERUP_TIME 110    // Powerup delay in milliseconds

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
extern bit WaitSTCInterrupt;
extern bit PoweredUp;
extern bit GpioSet;   //zjh add
extern bit SeekTuneInProc;
extern u8  idata cmd[8];
extern u8  idata rsp[15];
extern u8  chipFunction;



// This variables are used by the status commands.  Make sure to call those
// commands (amRsqStatus or amTuneStatus) prior to access.
extern u8  xdata Status;
extern u8  xdata RsqInts;
extern u8  xdata STC2;
extern u8  xdata SMUTE;
extern u8  xdata BLTF;
extern u8  xdata AFCRL;
extern u8  xdata Valid;
extern u8  xdata Pilot;
extern u8  xdata Blend;
extern u16 xdata Freq;
extern u8  xdata RSSI;
extern u8  xdata ASNR;
extern u16 xdata AntCap;

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
void wait_ms(u16 ms);
void si47xx_command(u8 cmd_size, u8 idata *cmd, u8 reply_size, u8 idata *reply);
void si47xx_reset(void);
u8 getIntStatus(void);
static void amTuneFreq(u16 frequency);
static void amSeekStart(u8 seekUp, u8 wrap);
static void amTuneStatus(u8 cancel, u8 intack);
static void amRsqStatus(u8 intack);

void AM_AGC_OVERRIDE_setting(void);
void AM_AGC_OVERRIDE_status(void);

//-----------------------------------------------------------------------------
// Take the Si47xx out of powerdown mode.
//-----------------------------------------------------------------------------
void si47xxAMRX_powerup(void)
{

    // Check if the device is already powered up.
    if (PoweredUp) {
    } else {
        // Put the ID for the command in the first byte.

        cmd[0] = POWER_UP;

		// Enable the GPO2OEN on the part because it will be used to determine
        // RDS Sync timing.
        //cmd[1] = POWER_UP_IN_GPO2OEN;
        //cmd[1] = POWER_UP_IN_OPMODE_TX_ANALOG;

#if 0  //zjh 2018514
        cmd[1] = 0X10;
        //cmd[1] = POWER_UP_IN_GPO2OEN;
        
       
		// The device is being powered up in FM RX mode.
        cmd[1] |= POWER_UP_IN_FUNC_AMRX;
#else
	    cmd[1] = 0xd1;   //使用时钟晶振，32.768khz ,AM模式
#endif


		// The opmode needs to be set to analog mode
        cmd[2] = POWER_UP_IN_OPMODE_RX_ANALOG;  //模拟L/R输出

        // Powerup the device
		si47xx_command(3, cmd, 8, rsp);
        //wait_ms(POWERUP_TIME);               // wait for si47xx to powerup
        wait_10ms(450);
		//wait_10ms(12);
        // Since we did not boot the part in query mode the result will not
        // contain the part information.
        //printc("~~~AM~lyj~~rsp[0] = %02x\r\n",rsp[0]);

		PoweredUp = 1;
    }
}

//-----------------------------------------------------------------------------
// Place the Si47xx into powerdown mode.
//-----------------------------------------------------------------------------
void si47xxAMRX_powerdown(void)
{

	// Check to see if the device is powered up.  If not do not do anything.
    if(PoweredUp)
    {   
        // Set the powered up variable to 0
        PoweredUp = 0;

	    // Put the ID for the command in the first byte.
	    cmd[0] = POWER_DOWN;

	    // Invoke the command
		si47xx_command(1, cmd, 1, rsp);
    }
}

//-----------------------------------------------------------------------------
// This function will set up some general items on the hardware like
// initializing the STC interrupts.
//-----------------------------------------------------------------------------
static void si47xxAMRX_hardware_cfg(void)
{
	// Enable the RDS and STC interrupt here
    si47xx_set_property(GPO_IEN, GPO_IEN_STCIEN_MASK);
}

//-----------------------------------------------------------------------------
// Set up general configuration properties:
//      Soft Mute Rate, Soft Mute Max Attenuation, Soft Mute SNR Threshold,
//      Seek Tune SNR Threshold, Seek Tune RSSI Threshold
//
// Note:
//     * RDS is only available on certain parts.  Please refer to the data
//       sheet for your part to determine if your part supports RDS.
//-----------------------------------------------------------------------------
static void si47xxAMRX_general_cfg(void)
{
    // The softmute feature can be disabled, but it is normally left on.
    // The softmute feature is disabled by setting the attenuation property
    // to zero.
      //si47xx_set_property(AM_SOFT_MUTE_RATE, 64);
      //si47xx_set_property(AM_SOFT_MUTE_SLOPE, 2);
      //si47xx_set_property(AM_SOFT_MUTE_MAX_ATTENUATION, 16);
      //si47xx_set_property(AM_SOFT_MUTE_SNR_THRESHOLD, 10);

    // The channel filter usually is defaulted to 2kHz.  Change this and
    // uncomment if a different channel filter is desired.
     //si47xx_set_property(AM_CHANNEL_FILTER, 3);
 
    // The deemphasis is usually disabled.  Change if desired.
     //si47xx_set_property(AM_DEEMPHASIS, 0);

    // Typically the settings used for seek are determined by the designer
    // and not exposed to the end user. They should be adjusted here.

    si47xx_set_property(AM_SEEK_TUNE_SNR_THRESHOLD, 10);
    si47xx_set_property(AM_SEEK_TUNE_RSSI_THRESHOLD, 20);
}

//AM (Medium Wave), SW (Short Wave), and LW (Long Wave) use the same AM_SW_LW 
//components, thus the commands and properties for these functions are the same.
//For simplicity reasons, the commands and properties only have a prefix AM 
//instead of AM_SW_LW. The main difference among AM, SW, and LW is only the
//frequency range.

//-----------------------------------------------------------------------------
// Set up regional configuration properties including:
//      Seek Band Bottom, Seek Band Top, Seek Freq Spacing
//-----------------------------------------------------------------------------
static void si47xxAMRX_regional_cfg(am_country_enum val)
{
	// Change the following properties if desired for the AM settings appropriate
    // for your region.
    if(val == SI47XX_AM_EUROPE)
    {
	    si47xx_set_property(AM_SEEK_FREQ_SPACING, 9); // Set spacing to 10kHz
	    si47xx_set_property(AM_SEEK_BAND_BOTTOM, 522); // Set the band bottom to 520kHz
	    si47xx_set_property(AM_SEEK_BAND_TOP, 1620);   // Set the band top to 1710kHz
    }
	else
	{
	    si47xx_set_property(AM_SEEK_FREQ_SPACING, 10); // Set spacing to 9kHz  9 // lyj 20190627  9
	    si47xx_set_property(AM_SEEK_BAND_BOTTOM, 530); // Set the band bottom to 530kHz
	    si47xx_set_property(AM_SEEK_BAND_TOP, 1710);   // Set the band top to 1710kHz
	}
}
//-----------------------------------------------------------------------------
// Set up regional configuration properties including:
//      Seek Band Bottom, Seek Band Top, Seek Freq Spacing
//-----------------------------------------------------------------------------
static void si47xxAMSWLWRX_regional_cfg(void)
{
	// Change the following properties if desired for the AM/SW/LW settings
	// appropriate for your region.

	//AM band
    ///si47xx_set_property(AM_SEEK_FREQ_SPACING, 10); // Set spacing to 10kHz
    //si47xx_set_property(AM_SEEK_BAND_BOTTOM, 520); // Set the band bottom to 520kHz
    //si47xx_set_property(AM_SEEK_BAND_TOP, 1710);   // Set the band top to 1710kHz

	//LW band
 //   si47xx_set_property(AM_SEEK_FREQ_SPACING, 9); // Set spacing to 9kHz
 //   si47xx_set_property(AM_SEEK_BAND_BOTTOM, 153); // Set the band bottom to 153kHz
 //   si47xx_set_property(AM_SEEK_BAND_TOP, 279);   // Set the band top to 279kHz

	//SW band (entire band)
 //   si47xx_set_property(AM_SEEK_FREQ_SPACING, 5); // Set spacing to 5kHz
 //   si47xx_set_property(AM_SEEK_BAND_BOTTOM, 2300); // Set the band bottom to 2300kHz
 //   si47xx_set_property(AM_SEEK_BAND_TOP, 23000);   // Set the band top to 23000kHz

	//SW band (120m band)
 //   si47xx_set_property(AM_SEEK_FREQ_SPACING, 5); // Set spacing to 5kHz
 //   si47xx_set_property(AM_SEEK_BAND_BOTTOM, 2300); // Set the band bottom to 2300kHz
 //   si47xx_set_property(AM_SEEK_BAND_TOP, 2495);   // Set the band top to 2495kHz

	//SW band (60m band)

    si47xx_set_property(AM_SEEK_FREQ_SPACING, 5); // Set spacing to 5kHz
    si47xx_set_property(AM_SEEK_BAND_BOTTOM, 4750); // Set the band bottom to 4750kHz
    si47xx_set_property(AM_SEEK_BAND_TOP, 5060);   // Set the band top to 5060kHz

}

//-----------------------------------------------------------------------------
// Configures the device for normal operation
//-----------------------------------------------------------------------------
void si47xxAMRX_configure(am_country_enum val)
{
    // Configure all other registers
    si47xxAMRX_hardware_cfg();
    si47xxAMRX_general_cfg();
    si47xxAMRX_regional_cfg(val);
	// Turn on the Headphone Amp and analog out.
//	M_INPUT_AD = 1;
//	M_OUTPUT_AD = 0;
//	GP1 = 1;
}

//-----------------------------------------------------------------------------
// Configures the device for normal operation
//-----------------------------------------------------------------------------
void si47xxAMSWLWRX_configure(void)
{
    // Configure all other registers
    si47xxAMRX_hardware_cfg();
    si47xxAMRX_general_cfg();
    si47xxAMSWLWRX_regional_cfg();

	// Turn on the Headphone Amp and analog out.
//	M_INPUT_AD = 1;
//	M_OUTPUT_AD = 0;
//	GP1 = 1;
}

//-----------------------------------------------------------------------------
// Resets the part and initializes registers to the point of being ready for
// the first tune or seek.
//-----------------------------------------------------------------------------
void si47xxAMRX_initialize(void)
{
    // Zero status registers.
	PoweredUp = 0;

    // Perform a hardware reset, power up the device, and then perform the
    // initial configuration.
    si47xx_reset();
    //si47xxAMRX_powerdown();
	//wait_10ms(6);
    si47xxAMRX_powerup();
   // si47xxAMRX_hardware_cfg();
    //si47xxAMRX_general_cfg();


	
    si47xxAMRX_configure(SI47XX_AM_OTHER/*SI47XX_AM_OTHER*/);  //lyj mode  20180514  SI47XX_AM_OTHER
	//AM_AGC_OVERRIDE_setting();
	//AM_AGC_OVERRIDE_status();

	
}

void si47xxAMSWLWRX_initialize(void)
{
    // Zero status registers.
	PoweredUp = 0;

    // Perform a hardware reset, power up the device, and then perform the
    // initial configuration.
    si47xx_reset();
    //si47xxAMRX_powerdown();
	//wait_10ms(6);
    si47xxAMRX_powerup();
    si47xxAMSWLWRX_configure();
}

//-----------------------------------------------------------------------------
// Set the volume and mute/unmute status
//
// Inputs:
//      volume:    a 6-bit volume value
//
// Note: It is assumed that if the volume is being adjusted, the device should
// not be muted.
//-----------------------------------------------------------------------------
void si47xxAMRX_set_volume(u8 volume)
{
    // Turn off the mute
    si47xx_set_property(RX_HARD_MUTE, 0);

    // Set the volume to the passed value
    si47xx_set_property(RX_VOLUME, (u16)volume & RX_VOLUME_MASK);
}

//-----------------------------------------------------------------------------
// Mute/unmute audio
//
// Inputs:
//      mute:  0 = output enabled (mute disabled)
//             1 = output muted
//-----------------------------------------------------------------------------
void si47xxAMRX_mute(u8 mute)
{
    if(mute)
    	si47xx_set_property(RX_HARD_MUTE, 
                                RX_HARD_MUTE_RMUTE_MASK | RX_HARD_MUTE_LMUTE_MASK);
    else
    	si47xx_set_property(RX_HARD_MUTE, 0);
}

//-----------------------------------------------------------------------------
// Tunes to a station number using the current band and spacing settings.
//
// Inputs:
//      frequency:  frequency in 1kHz steps
//
// Returns:
//      The RSSI level found during the tune.
//-----------------------------------------------------------------------------
u8 si47xxAMRX_tune(u16 frequency)
{
	// Enable the bit used for the interrupt of STC.
	__s32 err_count = 0;
	 u8 ret =0;
	
	SeekTuneInProc = 1;

	// Call the tune command to start the tune.
 	WaitSTCInterrupt = 1;
	//amTuneStatus(0, 1);
    amTuneFreq(frequency);
	wait_10ms(2);  //5
    // wait for the interrupt before continuing
    // If you do not wish to use interrupts but wish to poll the part
    // then comment out this line.
   // while (WaitSTCInterrupt); // Wait for interrupt to clear the bit

    // Wait for stc bit to be set
    //while (!(getIntStatus() & STCINT));
	{
    	u8 ret =0;
		ret = getIntStatus();
    	//__msg("%d\n",ret);
		ret = !ret;
    	//__msg("%d\n",ret);
		while (ret & STCINT)
		{
			err_count++;
			if(err_count > 50000)
			{
			    err_count = 0;
				break;
			}
		}
   	}

	// Clear the STC bit and get the results of the tune.
    //amTuneStatus(0, 0);
    amTuneStatus(0, 1);

	printc("freq =%d\r\n",Freq);

	// Disable the bit used for the interrupt of STC.
	SeekTuneInProc = 0;
	wait_10ms(4);

    // Return the RSSI level
    return RSSI;
}

//-----------------------------------------------------------------------------
// Inputs:
//      seekup:  0 = seek down
//               1 = seek up
//      seekmode: 0 = wrap at band limits
//                1 = stop at band limits
// Outputs:
//      zero = seek found a station
//      nonzero = seek did not find a station
//-----------------------------------------------------------------------------
u8 si47xxAMRX_seek(u8 seekup, u8 seekmode)
{
	// Enable the bit used for the interrupt of STC.
	SeekTuneInProc = 1;

	// Call the tune command to start the seek.
 	WaitSTCInterrupt = 1;
    amSeekStart(seekup, !seekmode);

    // wait for the interrupt before continuing
    // If you do not wish to use interrupts but wish to poll the part
    // then comment out these two lines.
   // while (WaitSTCInterrupt); // Wait for interrupt to clear the bit

    // Wait for stc bit to be set
    // If there is a display to update seek progress, then you could
    // call fmTuneStatus in this loop to get the current frequency.
    // When calling fmTuneStatus here make sure intack is zero.
   // while (!(getIntStatus() & STCINT));
   {
    	u8 ret =0;
		ret = getIntStatus();
    	//__msg("%d\n",ret);
		ret = !ret;
    	//__msg("%d\n",ret);
		while (ret & STCINT);
   	}

	// Clear the STC bit and get the results of the tune.
    amTuneStatus(0, 1);

	// Disable the bit used for the interrupt of STC.
	SeekTuneInProc = 0;

    // The tuner is now set to the newly found channel if one was available
    // as indicated by the seek-fail bit.
    return BLTF; //return seek fail indicator
}

//-----------------------------------------------------------------------------
// Returns the current tuned frequency of the part
//
// Returns:
//      frequency in 1kHz steps
//-----------------------------------------------------------------------------
u16 si47xxAMRX_get_frequency()
{
	// Get the tune status which contains the current frequency
    amTuneStatus(0, 0);

    // Return the frequency
    return Freq;
}

#if 0
u16 si47xxAMRX_get_valid()
{
    __s32 err_count = 0;
	u8 gLocVal=0;
	u8 ret =0;
	ret = getIntStatus();
	//__msg("%d\n",ret);
	ret = !ret;
	//__msg("%d\n",ret);
	while (ret & STCINT)
	{
	    err_count ++;
	    if(err_count > 50000)
	    {
	    	err_count = 0;
		    break;
	    }	
	}

	// Get the tune status which contains the current frequency
    amTuneStatus(0, 0);
	//__msg("STC=%d,BLTF=%d,AFCRL=%d,Valid=%d,Freq=%d,RSSI=%d,ASNR=%d,AntCap=%d\n",
	//		STC,BLTF,AFCRL,Valid,Freq,RSSI,ASNR,AntCap);
    // Return the Valid

	if(Valid == 1)
	{

		if((RSSI>=am_rssi)&&(ASNR >= am_snr))
		{
			return 1;
		}
		else
		{
		    return 0;
		}
	}
	else
	{
	   return 0;
	}
	
    //return Valid;
}

#endif



void AM_AGC_OVERRIDE_setting(void)
{
    cmd[0] = AM_AGC_OVERRIDE;


    cmd[1] = 0;


    cmd[2] = 0;


    // Invoke the command
	si47xx_command(3, cmd, 1, rsp);
	AHC_OS_SleepMs(100);
	
	printc("lyj~tune~~~staut~~~~~%02x\r\n",rsp[0]);

	

}


void AM_AGC_OVERRIDE_status(void)
{

	 cmd[0] = AM_AGC_STATUS;




	si47xx_command(1, cmd, 3, rsp);
	AHC_OS_SleepMs(100);
	
	printc("lyj~tune~~~staut~~~~~%02x~rsp[1] = %02x~rsp[2] = %02x\r\n",rsp[0],rsp[1],rsp[2]);

	


}

//-----------------------------------------------------------------------------
// Returns the current tuned frequency of the part
//
// Returns:
//      frequency in 10kHz steps
//-----------------------------------------------------------------------------
u8 si47xxAMRX_get_rssi()
{
	// Get the tune status which contains the current frequency
	//amTuneStatus(0,0);  // This should be changed to Rsq status when working.

    // Return the RSSI level
    return RSSI;
}


u8 si47xxAMRX_get_snr(void)
{
     return ASNR;
}


//-----------------------------------------------------------------------------
// Helper function that sends the AM_TUNE_FREQ command to the part
//
// Inputs:
// 	frequency in 1kHz steps
//-----------------------------------------------------------------------------
static void amTuneFreq(u16 frequency)
{
    // Put the ID for the command in the first byte.
    printc("frequency=%d\r\n",frequency);
    cmd[0] = AM_TUNE_FREQ;

	// Initialize the reserved section to 0
    cmd[1] = 0;

	// Put the frequency in the second and third bytes.
    cmd[2] = (u8)(frequency >> 8);
	cmd[3] = (u8)(frequency & 0x00FF);

	// Set the antenna calibration value.
    cmd[4] = (u8)0;  // Auto
	cmd[5] = (u8)0;

    // Invoke the command
	si47xx_command(6, cmd, 1, rsp);
	//printc("lyj~tune~~~staut~~~~~%02x\r\n",rsp[0]);
	AHC_OS_SleepMs(300);
}

//-----------------------------------------------------------------------------
// Helper function that sends the AM_SEEK_START command to the part
//
// Inputs:
// 	seekUp: If non-zero seek will increment otherwise decrement
//  wrap:   If non-zero seek will wrap around band limits when hitting the end
//          of the band limit.
//-----------------------------------------------------------------------------
static void amSeekStart(u8 seekUp, u8 wrap)
{
    // Put the ID for the command in the first byte.
    cmd[0] = AM_SEEK_START;

	// Put the flags if the bit was set for the input parameters.
	cmd[1] = 0;
    if(seekUp) cmd[1] |= AM_SEEK_START_IN_SEEKUP;
	if(wrap)   cmd[1] |= AM_SEEK_START_IN_WRAP;

    // Invoke the command
	si47xx_command(2, cmd, 1, rsp);
}

//-----------------------------------------------------------------------------
// Helper function that sends the AM_TUNE_STATUS command to the part
//
// Inputs:
// 	cancel: If non-zero the current seek will be cancelled.
//  intack: If non-zero the interrupt for STCINT will be cleared.
//
// Outputs:  // These are global variables and are set by this method
//  STC:    The seek/tune is complete
//  BLTF:   The seek reached the band limit or original start frequency
//  AFCRL:  The AFC is railed if this is non-zero
//  Valid:  The station is valid if this is non-zero
//  Freq:   The current frequency
//  RSSI:   The RSSI level read at tune.
//  ASNR:   The audio SNR level read at tune.
//  AntCap: The current level of the tuning capacitor.
//-----------------------------------------------------------------------------
static void amTuneStatus(u8 cancel, u8 intack)
{
    // Put the ID for the command in the first byte.
    cmd[0] = AM_TUNE_STATUS;

	// Put the flags if the bit was set for the input parameters.
	cmd[1] = 0;
    if(cancel) cmd[1] |= AM_TUNE_STATUS_IN_CANCEL;
	if(intack) cmd[1] |= AM_TUNE_STATUS_IN_INTACK;

    // Invoke the command
	si47xx_command(2, cmd, 8, rsp);

    // Parse the results
    STC2    = !!(rsp[0] & STCINT);
    BLTF   = !!(rsp[1] & AM_TUNE_STATUS_OUT_BTLF);
    AFCRL  = !!(rsp[1] & AM_TUNE_STATUS_OUT_AFCRL);
    Valid  = !!(rsp[1] & AM_TUNE_STATUS_OUT_VALID);
    Freq   = ((u16)rsp[2] << 8) | (u16)rsp[3];
    RSSI   = rsp[4];
    ASNR   = rsp[5];
    AntCap = ((u16)rsp[6] << 8) | (u16)rsp[7]; 

	if(Valid)
		printc("~~~~~~lyj~~~Valid = %d,Freq = %d\r\n",Valid,Freq);
	

}



