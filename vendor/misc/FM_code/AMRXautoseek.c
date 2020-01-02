//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "si47xxAMRX.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define NUM_SEEK_PRESETS 15

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
u16 xdata am_seek_preset[NUM_SEEK_PRESETS];

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Inserts a channel in the preset array. First it finds the channel with the
// lowest rssi.  If the current channel has a higher rssi, the channel with the
// lowest rssi is replaced. Channels are kept in order of increasing frequency.
//
// Inputs:
//      frequency:          Frequency of station found
//      rssi:               Rssi of station found
//      seek_prset_rssi:    Array of rsssi values for each preset
//-----------------------------------------------------------------------------
static void am_insert_preset(u16 frequency, u8 rssi, u8 *seek_preset_rssi)
{
    u8 i;
    u8 min_rssi = 0xff;
    u8 min_rssi_preset = 0;

    // first find the minimum rssi and its location
    // this will always stop at the first location with a zero rssi
    for ( i = 0; i < NUM_SEEK_PRESETS; i++ )
    {
        if(seek_preset_rssi[i] < min_rssi)
        {
            min_rssi = seek_preset_rssi[i];
            min_rssi_preset = i;
        }
    }

    // return if current preset rssi is less than minimum preset in array
    if(rssi < min_rssi)
        return;

    // delete the preset with the minimum rssi, and clear the last preset
    // since it would only be a copy of the second to last preset after
    // the deletion
    for ( i = min_rssi_preset; i < NUM_SEEK_PRESETS - 1; i++ )
    {
        am_seek_preset[i] = am_seek_preset[i + 1];
        seek_preset_rssi[i] = seek_preset_rssi[i + 1];
    }

    am_seek_preset[i] = 0;
    seek_preset_rssi[i] = 0;

    // fill the first preset with a zero for the frequency.  This will
    // always overwrite the last preset once all presets have been filled.
    for ( i = min_rssi_preset; i < NUM_SEEK_PRESETS; i++ )
    {
        if(am_seek_preset[i] == 0)
        {
            am_seek_preset[i] = frequency;
            seek_preset_rssi[i] = rssi;
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// This routine scans the band and populates the seek_presets array with the
// strongest signals in the band.  If the band is searched without filling all
// presets, the lowest preset numbers are left set to zero.  Do not tune to
// presets set to zero.
//
// Note that during autoseek, the seek threshold can be lowered to increase
// the chances of filling all of the presets.  A lower seek threshold will
// only increase the seek time and fill empty preset slots with weak stations
// instead of leaving them set to zero.  It should not change the outcome in
// the preset array.
//
// Note that this algorithm will not locate valid stations at the band limits.
// To check the band limits for a valid station, tune to 530khz or 531khz,
// set SEEKUP=0 and WRAP=1 and then perform a seek. If a station is not found
// by the time the current frequency is 1 frequency spacing below the upper limit
// (i.e. 1710MHz), then the seek can be aborted and it can be
// assumed that no stations exist at the band limits.
//
// Outputs:
//      Number of stations found
//-----------------------------------------------------------------------------
u8 si47xxAMRX_autoseek(void) {
    u8 seek_preset_rssi[NUM_SEEK_PRESETS]; // local array storing rssi of each preset.
    u8 i;
    u8 seek_fail;
    u16 current_frequency;
	u16 last_frequency = 0;
    u8 num_found = 0;

    // initialize preset array and rssi array
    for ( i = 0; i < NUM_SEEK_PRESETS; i++ ) {
        seek_preset_rssi[i] = 0;
        am_seek_preset[i] = 0;
    }

    // Mute the audio
    si47xxAMRX_mute(1);

    // tune to the bottom of the band, then seek up once
    si47xxAMRX_tune(522);
    seek_fail = si47xxAMRX_seek(1, 1);  // seek up, stop at limits
    current_frequency = si47xxAMRX_get_frequency();

	// Note:  This routine should stop at limits but a errata in
    // the AM firmware prevents this from working so wrap and look
    // for a smaller value that the previous value to break the loop.
    while((seek_fail == 0)) {
        am_insert_preset(current_frequency, si47xxAMRX_get_rssi(), seek_preset_rssi);
        seek_fail = si47xxAMRX_seek(1, 0); // seek up, wrap at limits
        current_frequency = si47xxAMRX_get_frequency();
		if(current_frequency <= last_frequency)
			break;
		last_frequency = current_frequency;
        num_found++;
    }

    // Tune to a good station. Either the one that was already tuned when the
    // seek started, or the first station in the array.
    si47xxAMRX_tune(am_seek_preset[0]);

    // Unmute
    si47xxAMRX_mute(0);

    // Limit num_found to the size of the array if it has been filled
    num_found = (num_found > NUM_SEEK_PRESETS) ? NUM_SEEK_PRESETS : num_found;

	for ( i = 0; i < num_found; i++ ) {

		printc("~~rssi[%d] = %d~~seek_preset[%d] = %d~\r\n",i,seek_preset_rssi[i],i,am_seek_preset[i]);
    }

    return num_found;
}
