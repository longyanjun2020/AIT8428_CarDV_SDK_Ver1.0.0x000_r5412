//-----------------------------------------------------------------------------
//
// si47xxFMRX.c
//
// Contains the FM radio functions with the exceptions of autoseek and rds.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include  "si47xx.h"
#include "si47xxFMRX.h"



//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define POWERUP_TIME 110    // Powerup delay in milliseconds

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
extern char WaitSTCInterrupt;
extern char PoweredUp;
extern char GpioSet;   //zjh add
extern char SeekTuneInProc;
extern u8   cmd[8];
extern u8   rsp[15];
extern u8  chipFunction;

// This variables are used by the status commands.  Make sure to call those
// commands (fmRsqStatus, fmTuneStatus, or fmRdsStatus) prior to access.
extern u8   Status;
extern u8   RsqInts;
extern u8   STC2;
extern u8   SMUTE;
extern u8   BLTF;
extern u8   AFCRL;
extern u8   Valid;
extern u8   Pilot;
extern u8   Blend;
extern u16  Freq;
extern u8   RSSI;
extern u8   ASNR;
extern u16  AntCap;
extern u8   FreqOff;




//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
void wait_ms(u16 ms);
void si47xx_command(u8 cmd_size, u8  *cmd, u8 reply_size, u8  *reply);
void si47xx_fm_command(u8 cmd_size, u8  *cmd, u8 reply_size, u8  *reply);

void si47xx_reset(void);
u8 getIntStatus(void);
static void fmTuneFreq(u16 frequency);
static void fmSeekStart(u8 seekUp, u8 wrap);
static void fmTuneStatus(u8 cancel, u8 intack);
static void fmRsqStatus(u8 intack);


//-----------------------------------------------------------------------------
// Take the Si47xx out of powerdown mode.
//-----------------------------------------------------------------------------
void si47xxFMRX_powerup(void)
{

    // Check if the device is already powered up.
    if (PoweredUp) {
    } else {
        // Put the ID for the command in the first byte.
        cmd[0] = POWER_UP;

#if 0  //20180514
		// Enable the GPO2OEN on the part because it will be used to determine
        // RDS Sync timing.
        //cmd[1] = POWER_UP_IN_GPO2OEN;
        //cmd[1] = POWER_UP_IN_OPMODE_TX_ANALOG;//POWER_UP_IN_OPMODE_TX_ANALOG;
        cmd[1] = 0x10; //0X10;//POWER_UP_IN_OPMODE_TX_ANALOG;
        //cmd[1] = POWER_UP_IN_GPO2OEN;

		// The device is being powered up in FM RX mode.
        cmd[1] |= POWER_UP_IN_FUNC_FMRX;
#else
		cmd[1] = 0xd0;
#endif
 
		// The opmode needs to be set to analog mode
        cmd[2] = POWER_UP_IN_OPMODE_RX_ANALOG;
       
		//__here__;
        // Powerup the device
		si47xx_command(3, cmd, 8, rsp);
		
		//__here__;
       // wait_ms(POWERUP_TIME);               // wait for si47xx to powerup
       wait_10ms(450);
		//{
		//	__s32 i =0;
		//	for(i=0;i<8;i++){
		//		printc("rsp[%d]=%x\n",i,rsp[i]);
		//	}
		//}
        // Since we did not boot the part in query mode the result will not
        // contain the part information.
        printc("~~~~lyj~~rsp[0] = %02x\r\n",rsp[0]);

		PoweredUp = 1;
    }
}

//-----------------------------------------------------------------------------
// Place the Si47xx into powerdown mode.
//-----------------------------------------------------------------------------
void si47xxFMRX_powerdown(void)
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

void si47xxFMRX_gpio2_set_high(void)
{
	if(GpioSet){
		
	}else{
	
	    cmd[0] = GPIO_CTL;
	    cmd[1] = 0x04;
	    cmd[2] = GPIO_SET;
		cmd[3] = 0x04;
		si47xx_command(4, cmd, 0,NULL);
	    wait_10ms(12); 
		GpioSet = 1;
	}
}

//-----------------------------------------------------------------------------
// This function will set up some general items on the hardware like
// initializing the RDS and STC interrupts.
//
// Note:
//     * RDS is only available on certain parts.  Please refer to the data
//       sheet for your part to determine if your part supports RDS.
//-----------------------------------------------------------------------------
static void si47xxFMRX_hardware_cfg(void)
{
	// Enable the RDS and STC interrupt here
    //si47xx_set_property(GPO_IEN, GPO_IEN_STCIEN_MASK | GPO_IEN_RDSIEN_MASK);
    si47xx_set_property(GPO_IEN, GPO_IEN_STCIEN_MASK);
	//printc("~~~~lyj~~rsp[0] = %02x\r\n",rsp[0]);
}

//-----------------------------------------------------------------------------
// Set up general configuration properties:
//      Soft Mute Rate, Soft Mute Max Attenuation, Soft Mute SNR Threshold,
//      Blend Mono Threshold, Blend Stereo Threshold, Max Tune Error,
//      Seek Tune SNR Threshold, Seek Tune RSSI Threshold
//
// Note:
//     * RDS is only available on certain parts.  Please refer to the data
//       sheet for your part to determine if your part supports RDS.
//-----------------------------------------------------------------------------
static void si47xxFMRX_general_cfg(void)
{
    // Typically the settings used for stereo blend are determined by the 
    // designer and not exposed to the end user. They should be adjusted here.
    // If the user wishes to force mono set both of these values to 127.
    // si47xx_set_property(FM_BLEND_MONO_THRESHOLD, 30);
    // si47xx_set_property(FM_BLEND_STEREO_THRESHOLD, 49);

    // The softmute feature can be disabled, but it is normally left on.
    // The softmute feature is disabled by setting the attenuation property
    // to zero.
    //  si47xx_set_property(FM_SOFT_MUTE_RATE, 64);
    //  si47xx_set_property(FM_SOFT_MUTE_MAX_ATTENUATION, 16);
    //  si47xx_set_property(FM_SOFT_MUTE_SNR_THRESHOLD, 4);

    // The max tune error is normally left in its default state.  The designer
    // can change if desired.
    //  si47xx_set_property(FM_MAX_TUNE_ERROR, 30);
 
    // Typically the settings used for seek are determined by the designer
    // and not exposed to the end user. They should be adjusted here.
    //eLIBs_printf("zjh fm_snr ==%d,fm_rssi==%d\n",fm_snr,fm_rssi);
    si47xx_set_property(FM_SEEK_TUNE_SNR_THRESHOLD, 3);
    si47xx_set_property(FM_SEEK_TUNE_RSSI_THRESHOLD, 12); //ËÑÌ¨·§Öµ
	
}

//-----------------------------------------------------------------------------
// Set up regional configuration properties including:
//      Seek Band Bottom, Seek Band Top, Seek Freq Spacing, Deemphasis
//
// Inputs:
//     country
//
// Note:
//     * RDS is only available on certain parts.  Please see the part's
//       datasheet for more information.
//-----------------------------------------------------------------------------
static void si47xxFMRX_regional_cfg(country_enum country)
{
    // Typically the settings used for stereo blend are determined by the 
    // designer and not exposed to the end user. They should be adjusted here.
    // If the user wishes to force mono set both of these values to 127.
    // si47xx_set_property(FM_BLEND_MONO_THRESHOLD, 30);
    // si47xx_set_property(FM_BLEND_STEREO_THRESHOLD, 49);

    // Depending on the country, set the de-emphasis, band, and space settings
    // Also optionally enable RDS for countries that support it
    switch (country) {
    case SI47XX_USA1:
	case SI47XX_USA2:
	case SI47XX_RUSSIAN:


		#if 0
        // This interrupt will be used to determine when RDS is available.
        si47xx_set_property(FM_RDS_INTERRUPT_SOURCE, 
					FM_RDS_INTERRUPT_SOURCE_SYNCFOUND_MASK); // RDS Interrupt

		// Enable the RDS and allow all blocks so we can compute the error
        // rate later.
        si47xx_set_property(FM_RDS_CONFIG, FM_RDS_CONFIG_RDSEN_MASK |
			(3 << FM_RDS_CONFIG_BLETHA_SHFT) |
			(3 << FM_RDS_CONFIG_BLETHB_SHFT) |
			(3 << FM_RDS_CONFIG_BLETHC_SHFT) |
			(3 << FM_RDS_CONFIG_BLETHD_SHFT));


		#endif
		si47xx_set_property(FM_RDS_CONFIG, 0); 
        si47xx_set_property(FM_DEEMPHASIS, FM_DEEMPH_75US); // Deemphasis
        ////////////////////////
        si47xx_set_property(FM_SEEK_BAND_BOTTOM, 8750);     // 87.5 MHz Bottom
        si47xx_set_property(FM_SEEK_BAND_TOP, 10790);        // 108 MHz Top
        si47xx_set_property(FM_SEEK_FREQ_SPACING, 20);      // 100 kHz Spacing 200j\kHz lyj 20190627
        ////////////////////////
        // Band is already set to 87.5-107.9MHz (US)
        // Space is already set to 200kHz (US)
        break;
    case SI47XX_JAPAN:
	case SI47XX_SCHOOL:  //zjh add
        si47xx_set_property(FM_RDS_CONFIG, 0);              // Disable RDS
        si47xx_set_property(FM_DEEMPHASIS, FM_DEEMPH_50US); // Deemphasis
        si47xx_set_property(FM_SEEK_BAND_BOTTOM, 7600);     // 76 MHz Bottom
        si47xx_set_property(FM_SEEK_BAND_TOP, 9100);        // 91 MHz Top
        si47xx_set_property(FM_SEEK_FREQ_SPACING, 10);      // 100 kHz Spacing
        break;
    case SI47XX_EUROPE:

		#if 0
        // This interrupt will be used to determine when RDS is available.
        si47xx_set_property(FM_RDS_INTERRUPT_SOURCE, 
			FM_RDS_INTERRUPT_SOURCE_SYNCFOUND_MASK); // RDS Interrupt

	    // Enable the RDS and allow all blocks so we can compute the error
        // rate later.
        si47xx_set_property(FM_RDS_CONFIG, FM_RDS_CONFIG_RDSEN_MASK |
		    (3 << FM_RDS_CONFIG_BLETHA_SHFT) |
			(3 << FM_RDS_CONFIG_BLETHB_SHFT) |
			(3 << FM_RDS_CONFIG_BLETHC_SHFT) |
			(3 << FM_RDS_CONFIG_BLETHD_SHFT));
		#endif
		si47xx_set_property(FM_RDS_CONFIG, 0);              // Disable RDS
        si47xx_set_property(FM_DEEMPHASIS, FM_DEEMPH_50US); // Deemphasis
        si47xx_set_property(FM_SEEK_BAND_BOTTOM, 8750);     // 87.5 MHz Bottom
        si47xx_set_property(FM_SEEK_BAND_TOP, 10800);        // 108 MHz Top
        // Band is already set to 87.5-107.9MHz (Europe)
        si47xx_set_property(FM_SEEK_FREQ_SPACING, 5);      // 100 kHz Spacing
        break;
		case SI47XX_RUSSIAN1:
		{
			si47xx_set_property(FM_RDS_CONFIG, 0);              // Disable RDS
	        si47xx_set_property(FM_DEEMPHASIS, FM_DEEMPH_50US); // Deemphasis
	        si47xx_set_property(FM_SEEK_BAND_BOTTOM, 8750);     // 87.5 MHz Bottom
	        si47xx_set_property(FM_SEEK_BAND_TOP, 10800);        // 108 MHz Top
	        // Band is already set to 87.5-107.9MHz (Europe)
	        si47xx_set_property(FM_SEEK_FREQ_SPACING, 5);      // 50 kHz Spacing	
		}
		break;
		default:
		break;
    }
}

//-----------------------------------------------------------------------------
// Configures the device for normal operation
//-----------------------------------------------------------------------------
void si47xxFMRX_configure(void)
{
    // Configure all other registers
   // __here__;
    si47xxFMRX_hardware_cfg();
	//__here__;
    si47xxFMRX_general_cfg();
	//__here__;
    si47xxFMRX_regional_cfg(SI47XX_USA1/*SI47XX_EUROPE*/); //USA  JAPAN


	// Turn on the Headphone Amp and analog out.
}

//-----------------------------------------------------------------------------
// Resets the part and initializes registers to the point of being ready for
// the first tune or seek.
//-----------------------------------------------------------------------------
void si47xxFMRX_initialize(void)
{
    // Zero status registers.
	PoweredUp = 0;
	GpioSet = 0;
	//printc("func=%s\n",__func__);  
	// Perform a hardware reset, power up the device, and then perform the
    // initial configuration.

    si47xx_reset();

    si47xxFMRX_powerup();

    //si47xxFMRX_hardware_cfg();
    //si47xxFMRX_general_cfg();

	

	//si47xxFMRX_gpio2_set_high();  //lyj add

    si47xxFMRX_configure();  //lyj mode

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
void si47xxFMRX_set_volume(u8 volume)
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
void si47xxFMRX_mute(u8 mute)
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
//      frequency:  frequency in 10kHz steps
//
// Returns:
//      The RSSI level found during the tune.
//-----------------------------------------------------------------------------
u8 si47xxFMRX_tune(u16 frequency)
{
	// Enable the bit used for the interrupt of STC.
	__s32 err_count = 0;
	u8 ret =0;
	SeekTuneInProc = 1;

	// Call the tune command to start the tune.
 	WaitSTCInterrupt = 1;
	printc("frequency=%d\r\n",frequency);
	//fmTuneStatus(0, 1);

    fmTuneFreq(frequency);
	wait_10ms(1); // 3


    // wait for the interrupt before continuing
    // If you do not wish to use interrupts but wish to poll the part
    // then comment out this line.
//    __here__;
   // while (WaitSTCInterrupt); // Wait for interrupt to clear the bit
//	__here__;
    // Wait for stc bit to be set
   // while (!(getIntStatus() & STCINT));
    {
    	//u8 ret =0;
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
   // __here__;
   // 
	
    

	// Clear the STC bit and get the results of the tune.
    fmTuneStatus(0, 1);
	//__msg("freq =%d\n",Freq);
	// Disable the bit used for the interrupt of STC.
	SeekTuneInProc = 0;
	wait_10ms(1);
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
u8 si47xxFMRX_seek(u8 seekup, u8 seekmode)
{

    fmSeekStart(seekup, !seekmode);

    // wait for the interrupt before continuing
    // If you do not wish to use interrupts but wish to poll the part
    // then comment out these two lines.
    //while (WaitSTCInterrupt); // Wait for interrupt to clear the bit

    // Wait for stc bit to be set
    // If there is a display to update seek progress, then you could
    // call fmTuneStatus in this loop to get the current frequency.
    // When calling fmTuneStatus here make sure intack is zero.
    //while (!(getIntStatus() & STCINT));
	{
    	__s32 ret =0;
		ret = getIntStatus();
    	//__msg("%d\n",ret);
		ret = !ret;
    	//__msg("%d\n",ret);
		while (ret & STCINT);
   	}
	// Clear the STC bit and get the results of the tune.
    fmTuneStatus(0, 1);

	// Disable the bit used for the interrupt of STC.

    // The tuner is now set to the newly found channel if one was available
    // as indicated by the seek-fail bit.
    return BLTF; //return seek fail indicator
}

//-----------------------------------------------------------------------------
// Returns the current tuned frequency of the part
//
// Returns:
//      frequency in 10kHz steps
//-----------------------------------------------------------------------------
u16 si47xxFMRX_get_frequency()
{
	// Get the tune status which contains the current frequency
    //fmTuneStatus(0,1);

    // Return the frequency
    return Freq;
}



#if 0
u16 si47xxFMRX_get_valid()
{
	// Get the tune status which contains the current frequency
   // fmTuneStatus(0, 1);
    __s32 err_count = 0;
    u8 gLocVal=0;
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

	fmTuneStatus(0, 1);


	if(fm_si47xx_get_radio_loc())
	{
		gLocVal = g_loc_setval;
	}
	else
	{
		gLocVal = g_loc_default;
	}


	if(Valid == 1)
	{
		/*if((AntCap>0x1A)&&(AntCap<0xe5))
		{
			return 0;
		}*/
		if((RSSI>=gLocVal)&&(ASNR >=g_fm_db))
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
}

#endif

//-----------------------------------------------------------------------------
// Returns the current tuned frequency of the part
//
// Returns:
//      frequency in 10kHz steps
//-----------------------------------------------------------------------------

__s32 fm_antcap = 0;
u32 si47xxFMRX_get_rssi(void)
{
	// Get the tune status which contains the current frequency
	//fmRsqStatus(0);

    // Return the RSSI level
    return RSSI;
    //return fm_antcap;
}

u32 si47xxFMRX_get_snr(void)
{
	return ASNR;
}


//-----------------------------------------------------------------------------
// Quickly tunes to the passed frequency, checks the power level and snr, 
// and returns
//
// Inputs:
//  Channel number in 10kHz steps
//
// Output:
//  The RSSI level after tune
//-----------------------------------------------------------------------------
u8 quickAFTune(u16 freq)
{
	u16 current_freq = 0;
	u8  current_rssi = 0;

	// Get the current frequency from the part
    fmTuneStatus(0, 0);
	current_freq = Freq;

    // Tune to the AF frequency, check the RSSI, tune back
    current_rssi = si47xxFMRX_tune(freq);

    // Return to the original channel
    si47xxFMRX_tune(current_freq);
    return current_rssi;
}


//-----------------------------------------------------------------------------
// Helper function that sends the FM_TUNE_FREQ command to the part
//
// Inputs:
// 	frequency in 10kHz steps
//-----------------------------------------------------------------------------
static void fmTuneFreq(u16 frequency)
{
    // Put the ID for the command in the first byte.
    //__msg("frequency=%d\n",frequency);
    cmd[0] = FM_TUNE_FREQ;

	// Initialize the reserved section to 0
    cmd[1] = 0;

	// Put the frequency in the second and third bytes.	
	cmd[2] = (u8)((frequency >> 8)& 0x00FF);
	//printc("~~~cmd[2]=%x\r\n",cmd[2]);
	cmd[3] = (u8)(frequency & 0x00FF);
	//printc("~~~cmd[3]=%x\r\n",cmd[3]);
	
	// Set the antenna calibration value.
    cmd[4] = (u8)0;  // Auto

    // Invoke the command
	//si47xx_command(5, cmd, 1, rsp);
	si47xx_fm_command(5, cmd, 1, rsp);
	//printc("~~lyj~~~rsp=%x\n",rsp[0]);

	//si47xx_lowWrite(5,cmd);
	wait_10ms(100); 
}

//-----------------------------------------------------------------------------
// Helper function that sends the FM_SEEK_START command to the part
//
// Inputs:
// 	seekUp: If non-zero seek will increment otherwise decrement
//  wrap:   If non-zero seek will wrap around band limits when hitting the end
//          of the band limit.
//-----------------------------------------------------------------------------
static void fmSeekStart(u8 seekUp, u8 wrap)
{
    // Put the ID for the command in the first byte.
    cmd[0] = FM_SEEK_START;

	// Put the flags if the bit was set for the input parameters.
	cmd[1] = 0;
    if(seekUp) cmd[1] |= FM_SEEK_START_IN_SEEKUP;
	if(wrap)   cmd[1] |= FM_SEEK_START_IN_WRAP;

    // Invoke the command
	si47xx_command(2, cmd, 1, rsp);
	//wait_10ms(100);
	//printc("~~~~~~~rsp = %d\r\n",rsp[0]);
}

//-----------------------------------------------------------------------------
// Helper function that sends the FM_TUNE_STATUS command to the part
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
static void fmTuneStatus(u8 cancel, u8 intack)
{
    // Put the ID for the command in the first byte.
    cmd[0] = FM_TUNE_STATUS;

	// Put the flags if the bit was set for the input parameters.
	cmd[1] = 0;
    if(cancel) cmd[1] |= FM_TUNE_STATUS_IN_CANCEL;
	if(intack) cmd[1] |= FM_TUNE_STATUS_IN_INTACK;

    // Invoke the command
	//si47xx_command(2, cmd, 8, rsp);
	si47xx_fm_command(2, cmd, 8, rsp);
	/*{
		__s32 i =0;
		for(i=0;i<8;i++)
		{
			eLIBs_printf("rsp[%d]=%x\n",i,rsp[i]);
		}

	}*/
	
    // Parse the results
    STC2    = !!(rsp[0] & STCINT);
    BLTF   = !!(rsp[1] & FM_TUNE_STATUS_OUT_BTLF);
    AFCRL  = !!(rsp[1] & FM_TUNE_STATUS_OUT_AFCRL);
    Valid  = !!(rsp[1] & FM_TUNE_STATUS_OUT_VALID);
    Freq   = ((u16)rsp[2] << 8) | (u16)rsp[3];
    RSSI   = rsp[4];
    ASNR   = rsp[5];
    AntCap = rsp[7];  
	fm_antcap = rsp[7];
	if(Valid)
	{
		printc(" lyj cur action Valid=%d,Freq=%d\r\n",Valid,Freq);
	}

	//printc("=================\r\n");
}

//-----------------------------------------------------------------------------
// Helper function that sends the FM_RSQ_STATUS command to the part
//
// Inputs:
//  intack: If non-zero the interrupt for STCINT will be cleared.
//
// Outputs:
//  Status:  Contains bits about the status returned from the part.
//  RsqInts: Contains bits about the interrupts that have fired related to RSQ.
//  SMUTE:   The soft mute function is currently enabled
//  AFCRL:   The AFC is railed if this is non-zero
//  Valid:   The station is valid if this is non-zero
//  Pilot:   A pilot tone is currently present
//  Blend:   Percentage of blend for stereo. (100 = full stereo)
//  RSSI:    The RSSI level read at tune.
//  ASNR:    The audio SNR level read at tune.
//  FreqOff: The frequency offset in kHz of the current station from the tuned 
//           frequency.
//-----------------------------------------------------------------------------
static void fmRsqStatus(u8 intack)
{
    // Put the ID for the command in the first byte.
    cmd[0] = FM_RSQ_STATUS;

	// Put the flags if the bit was set for the input parameters.
	cmd[1] = 0;
	if(intack) cmd[1] |= FM_RSQ_STATUS_IN_INTACK;

    // Invoke the command
	si47xx_command(2, cmd, 8, rsp);

    // Parse the results
	Status  = rsp[0];
    RsqInts = rsp[1];
    SMUTE   = !!(rsp[2] & FM_RSQ_STATUS_OUT_SMUTE);
    AFCRL   = !!(rsp[2] & FM_RSQ_STATUS_OUT_AFCRL);
    Valid   = !!(rsp[2] & FM_RSQ_STATUS_OUT_VALID);
    Pilot   = !!(rsp[3] & FM_RSQ_STATUS_OUT_PILOT);
    Blend   = rsp[3] & FM_RSQ_STATUS_OUT_STBLEND;
    RSSI    = rsp[4];
    ASNR    = rsp[5];
    FreqOff = rsp[7];   
}




