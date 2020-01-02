//-----------------------------------------------------------------------------
//
// si47xx_low.c
//
// Contains the low level, hardware functions
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "commanddefs.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define IO3W    0
#define IO2W    1

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
u8 io_mode = IO3W;
u8 idata RdsAvailable = 0;
u8 idata ProcessSame = 0;
bit WaitSTCInterrupt = 0;
bit PoweredUp = 0;
bit GpioSet = 0;
bit SeekTuneInProc = 0;
bit RdsTestInProc = 0;
bit SameTestInProc = 0;
u8   cmd[8];
u8   rsp[15];
u8  xdata Status;
u8  xdata RsqInts;
u8  xdata STC2;
u8  xdata SMUTE;
u8  xdata BLTF;
u8  xdata AFCRL;
u8  xdata Valid;
u8  xdata Pilot;
u8  xdata Blend;
u16 xdata Freq;
u8  xdata RSSI;
u8  xdata ASNR;
u16 xdata AntCap;
u8  xdata FreqOff;
u8  chipFunction;
u8  xdata AsqInts;
u8  xdata Alert;

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
void si47xx_Rest(void);
void wait_us(u16 us);
void wait_ms(u16 ms);
void si47xx_lowWrite(u8 number_bytes, u8 idata *data_out);
void si47xx_lowRead(u8 number_bytes, u8 idata *data_in);



void io2w_write(u8 number_bytes, u8 *data_out)
{  
   // si47xx_write_data(0xff,data_out,number_bytes);
    iic_write_bytes(data_out, number_bytes);
}

//-----------------------------------------------------------------------------
// Sends 2-wire start, reads an array of data, sends 2-wire stop.
//
// Inputs:
//		number_bytes: Number of bytes to be read
//		data_in:      Destination array for data read
//
//-----------------------------------------------------------------------------

void wait_us(u16 us)
{

	RTNA_WAIT_US(us);

}


void wait_ms(u16 ms)
{

 	AHC_OS_SleepMs(ms);

}


void io2w_read(u8 number_bytes, u8 *data_in)
{  
    //si47xx_read_data(0xff,data_in,number_bytes);
  iic_read_bytes(data_in, number_bytes);
}






//-----------------------------------------------------------------------------
// Externals
//-----------------------------------------------------------------------------
extern u8 xdata RdsDataAvailable;

//-----------------------------------------------------------------------------
// Reset the Si47xx and select the appropriate bus mode.
//-----------------------------------------------------------------------------
void si47xx_reset(void)
{
	si47xx_Rest();
	
}

//-----------------------------------------------------------------------------
// This command returns the status
//-----------------------------------------------------------------------------
u8 si47xx_readStatus()
{
    u8 status;
   
    si47xx_lowRead(1, &status);

    return status;
}

//-----------------------------------------------------------------------------
// Command that will wait for CTS before returning
//-----------------------------------------------------------------------------
void si47xx_waitForCTS()
{
    u16 i=1000;

    // Loop until CTS is found or stop due to the counter running out.
    while (--i && !(si47xx_readStatus() & CTS))
    {
        wait_us(500);
    }

    // If the i is equal to 0 then something must have happened.
    // It is recommended that the controller do some type of error
    // handling in this case.
}


void si47xx_fm_command(u8 cmd_size, u8  *cmd, u8 reply_size, u8  *reply)
{
    // It is always a good idea to check for cts prior to sending a command to
    // the part.
    
    si47xx_waitForCTS();
	//__here__;
    // Write the command to the part
    si47xx_lowWrite(cmd_size, cmd);
	//__here__;
	//esKRNL_TimeDly(1);  //zjh mode
	//wait_us(1000);
	//wait_us(5000);
    // Wait for CTS after sending the command
    si47xx_waitForCTS();
	//__here__;
    // If the calling function would like to have results then read them.
    if(reply_size)
    {
    	//__here__;
        si47xx_lowRead(reply_size, reply);
		
    }
	
}


//-----------------------------------------------------------------------------
// Sends a command to the part and returns the reply bytes
//-----------------------------------------------------------------------------
void si47xx_command(u8 cmd_size, u8 idata *cmd, u8 reply_size, u8 idata *reply)
{
    // It is always a good idea to check for cts prior to sending a command to
    // the part.
   // __here__;
    //esKRNL_TimeDly(1);
     wait_us(3000);
    si47xx_waitForCTS();
	//__here__;
    // Write the command to the part
    si47xx_lowWrite(cmd_size, cmd);
	//__here__;
	//esKRNL_TimeDly(1);
	wait_us(1000);
	//wait_us(5000);
    // Wait for CTS after sending the command
    si47xx_waitForCTS();
	//__here__;
    // If the calling function would like to have results then read them.
    if(reply_size)
    {
    	//__here__;
        si47xx_lowRead(reply_size, reply);
		
    }
	
}

//-----------------------------------------------------------------------------
// Helper function that is used to write to the part which abstracts what
// bus mode is currently being used.
//-----------------------------------------------------------------------------
void si47xx_lowWrite(u8 number_bytes, u8 idata *data_out)
{  
  io2w_write(number_bytes, data_out);  
}

//-----------------------------------------------------------------------------
// Helper function that is used to read from the part which abstracts what
// bus mode is currently being used.
//-----------------------------------------------------------------------------
void si47xx_lowRead(u8 number_bytes, u8 idata *data_in)
{ 
   io2w_read(number_bytes, data_in); 
}

//-----------------------------------------------------------------------------
// Helper function that sends the GET_INT_STATUS command to the part
//
// Returns:
//   The status byte from the part.
//-----------------------------------------------------------------------------
u8 getIntStatus(void)
{
    u8 idata cmd[1] = {0};
    u8 idata rsp[1] = {0};
//	__msg("func=%s\n",__func__);
    // Put the ID for the command in the first byte.
    cmd[0] = GET_INT_STATUS;

    // Invoke the command
	si47xx_command(1, cmd, 1, rsp);
	//printc("~~~int~~~rsp[0]=%x\n",rsp[0]);
	// Return the status
	return rsp[0];
}

//-----------------------------------------------------------------------------
// Set the passed property number to the passed value.
//
// Inputs:
//      propNumber:  The number identifying the property to set
//      propValue:   The value of the property.
//-----------------------------------------------------------------------------
void si47xx_set_property(u16 propNumber, u16 propValue)
{
    // Put the ID for the command in the first byte.
    cmd[0] = SET_PROPERTY;

	// Initialize the reserved section to 0
    cmd[1] = 0;

	// Put the property number in the third and fourth bytes.
    cmd[2] = (u8)(propNumber >> 8);
	cmd[3] = (u8)(propNumber & 0x00FF);

	// Put the property value in the fifth and sixth bytes.
    cmd[4] = (u8)(propValue >> 8);
    cmd[5] = (u8)(propValue & 0x00FF);

    // Invoke the command
	si47xx_command(6, cmd, 0, NULL);
}

//-----------------------------------------------------------------------------
// This function is strictly for debugging only.  Call this function to see
// how to get the part information.
//-----------------------------------------------------------------------------
void si47xx_getPartInformation(void)
{
	u8 partNumber;
	char fwMajor;
	char fwMinor;
	u16  patchID;
	char cmpMajor;
	char cmpMinor;
	char chipRev;

	// NOTE:  This routine should only be called when the part is powered up.
	// If you wish to retrieve some of the part information without fully
	// powering up the part call the POWER_UP command on the part with the
	// FUNC_DEBUG flag.

	// Put the ID for the command in the first byte.
	cmd[0] = GET_REV;

	// Invoke the command
	si47xx_command(1, cmd, 9, rsp);

	// Now take the result and put in the variables we have declared
	// Status is in the first element of the array so skip that.
	partNumber = rsp[1];
	fwMajor  = (char)rsp[2];
	fwMinor  = (char)rsp[3];
	patchID  = (u16)(rsp[4] << 8) | (u16)rsp[5];
	cmpMajor = (char)rsp[6];
	cmpMinor = (char)rsp[7];
	chipRev  = (char)rsp[8]; 
	printc("partNumber=%x,fwMajor=%x,fwMinor=%x,patchID=%x,cmpMajor=%x,cmpMinor=%x,chipRev=%x\n",
		partNumber,fwMajor,fwMinor,patchID,cmpMajor,cmpMinor,chipRev);
}


	
