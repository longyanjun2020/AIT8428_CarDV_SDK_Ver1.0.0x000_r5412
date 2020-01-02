//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "si47xxAMRX.h"
#include "AMRXtest.h"
#include "propertydefs.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
void wait_ms(u16 ms);

//-----------------------------------------------------------------------------
// Externals
//-----------------------------------------------------------------------------
extern u16 xdata am_seek_preset[];

//-----------------------------------------------------------------------------
// This exercises the minimum number of function calls to initiate a tune when
// the Si47xx device is first initialized.
//-----------------------------------------------------------------------------
void test_AMRXtune(void)
{
    si47xxAMRX_initialize();
    si47xxAMRX_set_volume(63);     // full volume, turn off mute
    si47xxAMRX_tune(1200);        // tune to a station
    wait_ms(DEMO_DELAY);
}

//AM (Medium Wave), SW (Short Wave), and LW (Long Wave) use the same AM_SW_LW 
//components, thus the commands and properties for these functions are the same.
//For simplicity reasons, the commands and properties only have a prefix AM 
//instead of AM_SW_LW. The main difference among AM, SW, and LW is only the
//frequency range.

void test_AMSWLWRXtune(void)
{
    si47xxAMSWLWRX_initialize();
    si47xxAMRX_set_volume(63);     // full volume, turn off mute
    si47xxAMRX_tune(4750);        // tune to a station
    wait_ms(DEMO_DELAY);
}

//-----------------------------------------------------------------------------
// Simple routine that tests the si47xx driver autoseek function.
//
// This particular test:
//  - Resets/Initializes the Si47xx
//  - Fills the preset array with the strongest stations found
//  - Loops through each preset pausing on each one
//-----------------------------------------------------------------------------
void test_AMRXautoseek(void)
{
    u16 preset_freq;
    u8 preset_number;
    u8 num_found;

    si47xxAMRX_initialize();
    si47xxAMRX_set_volume(63); // full volume, turn off mute

    num_found = si47xxAMRX_autoseek(); // populate the preset array with strongest stations

  //  _nop_();           // break here to inspect preset array
  	wait_ms(1000);

    preset_number = 0;
	#if 0
    while(preset_number < num_found)
    {
        // tune to the next station in the preset array
        preset_freq = am_seek_preset[preset_number];

        if(preset_freq != 0)
        {
            si47xxAMRX_tune(preset_freq);
            wait_ms(DEMO_DELAY);
        }

        preset_number++;
    }

	#endif
}


//-----------------------------------------------------------------------------
// Test the volume.  Starts at mute, moves up through all 63 steps.
//-----------------------------------------------------------------------------
void test_AMRXvolume(void)
{
    u8 vol;
    for (vol=0; vol<63; vol++) {
        si47xxAMRX_set_volume(vol);
        wait_ms(500);
    }
}

//-----------------------------------------------------------------------------
// Test the power down function of the part.  Unlike the Si470x parts the Si47xx
// parts need to be fully initialized after power down.
//-----------------------------------------------------------------------------
void test_AMRXpowerCycle(void)
{
    si47xxAMRX_powerdown(); // Test powerdown with audio hi-z
    wait_ms(DEMO_DELAY);    // Wait 5s before starting next test
    si47xxAMRX_powerup();   // Powerup without reset, the part will need to be tuned again.
    si47xxAMRX_configure(SI47XX_AM_EUROPE);
    si47xxAMRX_set_volume(63);     // full volume, turn off mute
	si47xxAMRX_tune(1200);
	wait_ms(DEMO_DELAY);    // Wait 5s before starting next test
}

