
#include "Fm_i2c.h"
#include "mmp_i2c_inc.h"
#include "mmp_err.h"
#include "mmps_pio.h"
#include "AHC_OS.h"
#include "mmpf_i2cm.h"
#include "soft_i2c.h"
#include "mmpf_pio.h"

//#define LYJ;

#if 0
#define AM_MODE 	(1)

// STATUS bits - Used by all methods
#define STCINT  0x01

// GET_REV
#define GET_REV 0x10
#define RX_HARD_MUTE 0x4001

// RX_VOLUME
#define RX_VOLUME      0x4000
#define RX_VOLUME_MASK 0x003F

#define  FM_SEARCH_CHN_MIN_FREQ         87500
#define  FM_SEARCH_CHN_MAX_FREQ        108000


#define  AM_SEARCH_EUR_MIN_FREQ         5200
#define  AM_SEARCH_EUR_MAX_FREQ         16200

// FM_RDS_CONFIG
#define FM_RDS_CONFIG             0x1502

// FM_DEEMPHASIS
#define FM_DEEMPHASIS      0x1100

// FM_DEEMPH
#define FM_DEEMPH_75US 0x2
#define FM_DEEMPH_50US 0x1

// FM_SEEK_BAND_BOTTOM
#define FM_SEEK_BAND_BOTTOM 0x1400

// FM_SEEK_BAND_TOP
#define FM_SEEK_BAND_TOP 0x1401

// FM_SEEK_FREQ_SPACING
#define FM_SEEK_FREQ_SPACING      0x1402

// GET_INT_STATUS
#define GET_INT_STATUS 0x14

// FM_SEEK_START
#define FM_SEEK_START           0x21
#define FM_SEEK_START_IN_WRAP   0x04
#define FM_SEEK_START_IN_SEEKUP 0x08

// FM_TUNE_STATUS
#define FM_TUNE_STATUS           0x22
#define FM_TUNE_STATUS_IN_INTACK 0x01
#define FM_TUNE_STATUS_IN_CANCEL 0x02
#define FM_TUNE_STATUS_OUT_VALID 0x01
#define FM_TUNE_STATUS_OUT_AFCRL 0x02
#define FM_TUNE_STATUS_OUT_BTLF  0x80

// FM_SEEK_TUNE_SNR_THRESHOLD
#define FM_SEEK_TUNE_SNR_THRESHOLD      0x1403

// FM_SEEK_TUNE_RSSI_THRESHOLD
#define FM_SEEK_TUNE_RSSI_THRESHOLD      0x1404

// FM_TUNE_FREQ
#define FM_TUNE_FREQ 0x20



// AM_TUNE_FREQ
#define AM_TUNE_FREQ 0x40

// AM_SEEK_START
#define AM_SEEK_START           0x41
#define AM_SEEK_START_IN_WRAP   0x04
#define AM_SEEK_START_IN_SEEKUP 0x08

// AM_TUNE_STATUS
#define AM_TUNE_STATUS           0x42
#define AM_TUNE_STATUS_IN_INTACK 0x01
#define AM_TUNE_STATUS_IN_CANCEL 0x02
#define AM_TUNE_STATUS_OUT_VALID 0x01
#define AM_TUNE_STATUS_OUT_AFCRL 0x02
#define AM_TUNE_STATUS_OUT_BTLF  0x80

// AM_SEEK_BAND_BOTTOM
#define AM_SEEK_BAND_BOTTOM 0x3400

// AM_SEEK_BAND_TOP
#define AM_SEEK_BAND_TOP 0x3401

// AM_SEEK_FREQ_SPACING
#define AM_SEEK_FREQ_SPACING      0x3402
#define AM_SEEK_FREQ_SPACING_MASK 0x000F
#define AM_SEEK_FREQ_SPACING_SHFT 0

// AM_SEEK_TUNE_SNR_THRESHOLD
#define AM_SEEK_TUNE_SNR_THRESHOLD      0x3403
#define AM_SEEK_TUNE_SNR_THRESHOLD_MASK 0x003F
#define AM_SEEK_TUNE_SNR_THRESHOLD_SHFT 0

// AM_SEEK_TUNE_RSSI_THRESHOLD
#define AM_SEEK_TUNE_RSSI_THRESHOLD      0x3404
#define AM_SEEK_TUNE_RSSI_THRESHOLD_MASK 0x003F
#define AM_SEEK_TUNE_RSSI_THRESHOLD_SHFT 0

// GPO_IEN
#define GPO_IEN				 0x0001
#define GPO_IEN_STCIEN_MASK  0x0001


// POWER_DOWN
#define POWER_DOWN 0x11

#define GPIO_CTL 0x80     //zjh add GPIO输出控制器
#define GPIO_SET 0x81    //zjh add GPI0高低电平控制

MMP_UBYTE GpioSet = 0;


MMP_ULONG si47xx_min_freq = 8750;
MMP_ULONG si47xx_max_freq = 10800; //china

MMP_UBYTE cmd[8];
MMP_UBYTE rsp[15];

MMP_UBYTE STC1;
MMP_UBYTE   SMUTE;
MMP_UBYTE   BLTF;
MMP_UBYTE   AFCRL;
MMP_UBYTE   Valid;
MMP_UBYTE   Pilot;
MMP_UBYTE   Blend;
MMP_USHORT  Freq;
MMP_UBYTE   RSSI;
MMP_UBYTE   ASNR;
MMP_USHORT  AntCap;
MMP_UBYTE   FreqOff;

MMP_UBYTE   Radiocnt = 0;

MMP_UBYTE work_mode = 0;
MMP_UBYTE flagSelect = 1;

MMP_UBYTE PoweredUp = 0;

#endif 

#if 0

static MMP_I2CM_ATTR gI2cmAttribute_FM4730 = {
	MMP_I2CM1, 
	0xc6 >> 1, 
	8, 
	8, 
	0, 
	MMP_TRUE, 
	MMP_FALSE, 
	MMP_FALSE, 
	MMP_FALSE, 
	0, 
	0, 
	1, 
	50000/*KHZ*/, 
	MMP_TRUE, 
	MMP_GPIO0, 
	MMP_GPIO1, 
	MMP_FALSE, 
	MMP_FALSE,
	MMP_FALSE,
	0
	};

#endif

MMP_UBYTE flagSelect ;

 static   MMP_I2CM_ATTR gI2cmAttribute_BD3490 = {
	MMP_I2CM1, 
	0x80 >> 1, 
	8, 
	8, 
	0, 
	MMP_TRUE, 
	MMP_FALSE, 
	MMP_FALSE, 
	MMP_FALSE, 
	0, 
	0, 
	1, 
	50000/*KHZ*/, 
	MMP_TRUE, 
	MMP_GPIO0, 
	MMP_GPIO1, 
	MMP_FALSE, 
	MMP_FALSE,
	MMP_FALSE,
	0
	};

 void Set_flag(MMP_UBYTE FuncSelect)
 {
	flagSelect = FuncSelect;

 }

 MMP_UBYTE Get_flag(void)
 {
	return flagSelect;
 }

 void BD3490_reginit(void)
{
	//reset 

	//MMPS_PIO_EnableOutputMode(MMP_GPIO33,MMP_TRUE);// long 4-20
     //  MMPS_PIO_SetData(MMP_GPIO33, 1);  //long 4-20


	#if 0

	MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0xFE,0x81);//System Reset 

	AHC_OS_SleepMs(1);

	MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x04,0x07);
	MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x06,0x00);
	MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x21,0x00);
	MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x22,0x00);
	MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x51,0x80);
	MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x57,0x80);
	MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x78,0x00);

	#else

	AHC_OS_SleepMs(2);

	IIC_write_data( 0xFE,0x81,1);//System Reset 

	AHC_OS_SleepMs(1);

	IIC_write_data( 0x04,0x07,1);
	IIC_write_data( 0x06,0x00,1);// 0x10
	IIC_write_data( 0x21,0x00,1);
	IIC_write_data( 0x22,0x00,1);
	IIC_write_data( 0x51,0x80,1);//0x80
	IIC_write_data( 0x57,0x80,1);//0x80
	IIC_write_data( 0x78,0x00,1);

	



	#endif
	
	//printc("~~~~~~~~Init~~~finish~~~~\r\n");
	
}

AHC_BOOL work_mode1 = 99;// lyj 20190213
extern AHC_BOOL work_mode;
AHC_BOOL FM_AM_playback = 0;
void BD3490_init(void)
{
	//printc("~~~~~~~~555555longdebug~~~~~~\r\n");


	#if 0

	//MMPF_PIO_Initialize();
	MMPF_I2cm_Initialize(&gI2cmAttribute_BD3490);
	
	//MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0xFE,0x81);//System Reset 
	//MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0xF0,0x00);//Test Mode

	BD3490_reginit();

	AHC_OS_SleepMs(1);

	//MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x04,0x02);

	#if 1

	if(Get_flag() == 0)

		MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x04,0x00);//bluetooth
	else if (Get_flag() == 2)
		MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x04,0x02);//U盘
	else if(Get_flag() == 1)
		MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x04,0x01);
	else if(Get_flag() == 3)
		MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x04,0x03);

	#endif
	MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x06,0x14);


	#else

	//MMPF_PIO_Initialize();
	//MMPF_I2cm_Initialize(&gI2cmAttribute_BD3490);
	
	//MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0xFE,0x81);//System Reset 
	//MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0xF0,0x00);//Test Mode

	//BD3490_reginit();

	AHC_OS_SleepMs(1);

	//MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x04,0x02);

	#if 1

	if(Get_flag() == 0)

		IIC_write_data(0x04,0x00,1);//bluetooth
	else if (Get_flag() == 2)
		IIC_write_data(0x04,0x02,1);//U盘
	else if(Get_flag() == 1)
		IIC_write_data(0x04,0x01,1);
	else if(Get_flag() == 3)
	{
		//if(work_mode1 == 100)
			//work_mode1 = work_mode;
		if(work_mode==100 || work_mode  != work_mode1)
		{
			IIC_write_data(0x04,0x07,1); // 0x03
			work_mode1 = work_mode;
			FM_AM_playback = 0;
		}
		else
		{
			IIC_write_data(0x04,0x03,1);	
			FM_AM_playback = 1;
		}
	}// lyj 20190212

	#endif
	//IIC_write_data(0x06,0x14,1);


	

	#endif

	printc("~~~~~~long66xxx239888~~~~~~\r\n");

}

void Volume_set(MMP_USHORT volume)
{

	#if 0
	MMPF_I2cm_Initialize(&gI2cmAttribute_BD3490);

	MMPF_I2cm_WriteReg(&gI2cmAttribute_BD3490, 0x06,volume);
	printc("~~~~~~~volume set~~volume = %02x~~~\r\n",volume);
	#else
	

	IIC_write_data(0x06,volume,1);
	printc("~~~~~~~volume set~~volume = %02x~~~\r\n",volume);


	#endif

}

void Volume_set_EX(MMP_USHORT volume)
{
	IIC_write_data(0x21,volume,1);

	IIC_write_data(0x22,volume,1);
	printc("~~~~~~~volume set~~volume = %02x~~~\r\n",volume);
}

void Close_channal(void)
{
	IIC_write_data( 0x04,0x07,1);
	//printc("~~~~~off_channal~~~~\r\n");
	
}



#if (defined(LYJ))

static void fm4730_writereg(void)
{
	MMP_USHORT data =0xa3;

//	MMPF_I2cm_WriteReg(&gI2cmAttribute_FM4730, 0xff, data);

	
	printc("~~~~~~~~reg=~~~~%d~~~~~~~~\r\n",data);
	
}

static void si47xx_reset(void)
{
		MMPS_PIO_EnableOutputMode(FM_PATH_SELECT_GPIO, MMP_TRUE);
		MMPS_PIO_SetData(FM_PATH_SELECT_GPIO, FM_PATH_SELECT_SET);
		AHC_OS_SleepMs(10);

		MMPS_PIO_SetData(FM_PATH_SELECT_GPIO, FM_PATH_SELECT_RESET);
		AHC_OS_SleepMs(500);



}


void fm4730_init(void)
{

	//MMP_UBYTE test;

	//MMPS_PIO_EnableOutputMode(FM_PATH_SELECT_GPIO, MMP_TRUE);
	//MMPS_PIO_SetData(FM_PATH_SELECT_GPIO, FM_PATH_SELECT_SET);
	//AHC_OS_SleepMs(10);

	//MMPS_PIO_SetData(FM_PATH_SELECT_GPIO, FM_PATH_SELECT_RESET);
	//AHC_OS_SleepMs(10);

#if 0
           MMPS_PIO_PadConfig(MMP_GPIO1, 0x00);
		   AHC_GPIO_SetOutputMode(MMP_GPIO1,MMP_TRUE);
		   MMPS_PIO_SetData(MMP_GPIO1, 1);
			 
			
           MMPS_PIO_PadConfig(MMP_GPIO1, 0x00);
		   AHC_GPIO_SetOutputMode(MMP_GPIO1,MMP_FALSE);
		    MMPS_PIO_GetData(MMP_GPIO1, &test);
		   printc("~~MMP1_GPIO1~~=%d~~~\r\n",test);



		   MMPS_PIO_PadConfig(MMP_GPIO1, 0x00);
		   AHC_GPIO_SetOutputMode(MMP_GPIO1,MMP_TRUE);
		   MMPS_PIO_SetData(MMP_GPIO1, 0);
			 
			
           MMPS_PIO_PadConfig(MMP_GPIO1, 0x00);
		   AHC_GPIO_SetOutputMode(MMP_GPIO1,MMP_FALSE);
		    MMPS_PIO_GetData(MMP_GPIO1, &test);
		   printc("~~MMP2_GPIO1~~=%d~~~\r\n",test);

#endif

		   
			

	
	#if 0
	if (MMPF_I2cm_Initialize(&gI2cmAttribute_FM4730) == MMP_I2CM_ERR_SLAVE_NO_ACK)
	{
			printc("~~~~~~~init~~MMP_I2CM_ERR_SLAVE_NO_ACK~~~~fail\r\n");

	}

	printc("~~~~~~~init~~~~~~successful\r\n");
	//fm4730_writereg();

	#endif
}


void si47xx_lowWrite(MMP_UBYTE number_bytes, MMP_UBYTE  *data_out)
{  
 // MMPF_I2cm_WriteBurstData(&gI2cmAttribute_FM4730,reg,data_out, number_bytes,MMP_TRUE);

 	iic_write_bytes(data_out, number_bytes);
 
  //MMPF_I2cm_WriteBurstData(&gI2cmAttribute_FM4730,0x00,(MMP_USHORT*)si4730_powerup_reg,2,MMP_TRUE);
}



void si47xx_lowRead(MMP_UBYTE number_bytes, MMP_UBYTE  *data_in)
{ 
  // MMPF_I2cm_ReadBurstData(&gI2cmAttribute_FM4730,reg,data_in, number_bytes); 

   iic_read_bytes(data_in, number_bytes);
  
  // MMPF_I2cm_ReadBurstData(&gI2cmAttribute_FM4730,0x00, (MMP_USHORT*)rsp, 8);
}




void si47xx_waitForCTS()
{
    int i=500;

	MMP_UBYTE data;

	//MMPF_I2cm_ReadNoRegMode(&gI2cmAttribute_FM4730, &data, 1);
	//MMPF_I2cm_ReadReg(&gI2cmAttribute_FM4730, reg, &data);
	
	 iic_read_bytes(&data, 1);
	

    // Loop until CTS is found or stop due to the counter running out.
    while (--i && !(data & CTS))
    {
        AHC_OS_SleepMs(5);
    }

    // If the i is equal to 0 then something must have happened.
    // It is recommended that the controller do some type of error
    // handling in this case.
}

void si47xx_command(MMP_UBYTE cmd_size, MMP_UBYTE *cmd, MMP_UBYTE reply_size, MMP_UBYTE *reply)
{
    // It is always a good idea to check for cts prior to sending a command to
    // the part.
   // __here__;

     AHC_OS_SleepMs(3);
    si47xx_waitForCTS();
	//__here__;
    // Write the command to the part
    si47xx_lowWrite(cmd_size, cmd);
   
	//__here__;

	AHC_OS_SleepMs(1);

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




static void  Si4730PowerUp(void)
{
	//__u8 si4730_powerup_reg[3] = {0x01,0x90,0x05};

	#if (AM_MODE)
	if(PoweredUp == 0)
	{
	MMP_UBYTE si4730_powerup_reg[3] = {0x01,0xD0,0x05};

	si47xx_command(3,si4730_powerup_reg,1,rsp);

		printc("Si4730PowerUp-->%02x\r\n",rsp[0]);//STATUS --->0x80 Reply Status. Clear-to-send high.
	
	
	AHC_OS_SleepMs(12);

	PoweredUp = 1;
	}

	#else
	if(PoweredUp == 0)
	{

		MMP_UBYTE si4730_powerup_reg[3] = {0x01,0xD1,0x05};


	si47xx_command(3,si4730_powerup_reg,1,rsp);

		printc("Si4730PowerUp-lyj->%02x\r\n",rsp[0]);//STATUS --->0x80 Reply Status. Clear-to-send high.
	
	
	AHC_OS_SleepMs(12);
	PoweredUp = 1;

	}



	#endif



}

void si47xx_set_property(MMP_USHORT propNumber, MMP_USHORT propValue)
{
    // Put the ID for the command in the first byte.   0x1502
    int i;
    cmd[0] = SET_PROPERTY;

	// Initialize the reserved section to 0
    cmd[1] = 0;

	// Put the property number in the third and fourth bytes.
    cmd[2] = (MMP_UBYTE)(propNumber >> 8);
	cmd[3] = (MMP_UBYTE)(propNumber & 0x00FF);

	// Put the property value in the fifth and sixth bytes.
    cmd[4] = (MMP_UBYTE)(propValue >> 8);
    cmd[5] = (MMP_UBYTE)(propValue & 0x00FF);

    // Invoke the command
	//si47xx_command(6, cmd, 0, NULL);

	si47xx_command(6,cmd,0,NULL);
	//AHC_OS_SleepMs(100);
	

	
}


void si47xx_getPartInformation(void)
{
	MMP_UBYTE partNumber;
	char fwMajor;
	char fwMinor;
	MMP_USHORT patchID;
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
	patchID  = (MMP_USHORT)(rsp[4] << 8) | (MMP_USHORT)rsp[5];
	cmpMajor = (char)rsp[6];
	cmpMinor = (char)rsp[7];
	chipRev  = (char)rsp[8]; 

	printc("--%02x---%02x---\r\n",rsp[4],rsp[5]);
	printc("stauts = %02x，partNumber=%x,fwMajor=%x,fwMinor=%x,patchID=%x,cmpMajor=%x,cmpMinor=%x,chipRev=%x\r\n",
		rsp[0] ,partNumber,fwMajor,fwMinor,patchID,cmpMajor,cmpMinor,chipRev);
	

	    // Since we did not boot the part in query mode the result will not
        // contain the part information.
}


void si47xxAMRX_set_volume(MMP_UBYTE volume)
{
    // Turn off the mute
    si47xx_set_property(RX_HARD_MUTE, 0);

    // Set the volume to the passed value
    si47xx_set_property(RX_VOLUME, (MMP_USHORT)volume & RX_VOLUME_MASK);
}


void Si473xSetFrq(MMP_USHORT Frq)
{
	MMP_UBYTE si473x_tune_regs[6] = {0x40,0x00,0x00,0x00,0x00,0x01};
	if(work_mode  == 0)
	{
		si473x_tune_regs[2] = Frq>>8;
		si473x_tune_regs[3] = Frq&0xff;
		si473x_tune_regs[0] = 0x20;
		si47xx_command(5,si473x_tune_regs,0,NULL);
	}
	else
	{
		si473x_tune_regs[2] = Frq>>8;
		si473x_tune_regs[3] = Frq&0xff;
		si47xx_command(5,si473x_tune_regs, 1, rsp);
	}
	AHC_OS_SleepMs(100);

	printc("rsp=%x\r\n",rsp[0]);
}


static void fmTuneFreq(MMP_USHORT frequency)
{
    // Put the ID for the command in the first byte.
    printc("frequency=%d\r\n",frequency);
    cmd[0] = FM_TUNE_FREQ;

	// Initialize the reserved section to 0
    cmd[1] = 0;

	// Put the frequency in the second and third bytes.	
	cmd[2] = (MMP_UBYTE)((frequency >> 8)& 0x00FF);
	printc("cmd[2]=%x\r\n",cmd[2]);
	cmd[3] = (MMP_UBYTE)(frequency & 0x00FF);
	printc("cmd[3]=%x\r\n",cmd[3]);
	
	// Set the antenna calibration value.
    cmd[4] = (MMP_UBYTE)0;  // Auto


	si47xx_command(5, cmd, 1, rsp);
	printc("rsp=%x\r\n",rsp[0]);

	AHC_OS_SleepMs(100); 
}


static void si47xxFMRX_general_cfg(void)
{
    // Typically the settings used for stereo blend are determined by the 
    // designer and not exposed to the end user. They should be adjusted here.
    // If the user wishes to force mono set both of these values to 127.

    si47xx_set_property(FM_SEEK_TUNE_SNR_THRESHOLD, 3);
    si47xx_set_property(FM_SEEK_TUNE_RSSI_THRESHOLD, 12); //搜台阀值
	
}


static void si47xxFMRX_regional_cfg(country_enum country)
{
	switch(country)
	{
		case SI47XX_CHA:
		si47xx_set_property(FM_RDS_CONFIG, 0); 
        si47xx_set_property(FM_DEEMPHASIS, FM_DEEMPH_75US); // Deemphasis
      
        si47xx_set_property(FM_SEEK_BAND_BOTTOM, 8750);     // 87.5 MHz Bottom
        si47xx_set_property(FM_SEEK_BAND_TOP, 10790);        // 108 MHz Top
        si47xx_set_property(FM_SEEK_FREQ_SPACING, 10);      // 100 kHz Spacing

		case SI47XX_JAPAN:
		case SI47XX_SCHOOL:  //zjh add
        si47xx_set_property(FM_RDS_CONFIG, 0);              // Disable RDS
        si47xx_set_property(FM_DEEMPHASIS, FM_DEEMPH_50US); // Deemphasis
        si47xx_set_property(FM_SEEK_BAND_BOTTOM, 7600);     // 76 MHz Bottom
        si47xx_set_property(FM_SEEK_BAND_TOP, 9100);        // 91 MHz Top
        si47xx_set_property(FM_SEEK_FREQ_SPACING, 10);      // 100 kHz Spacing

	
	}
}


MMP_USHORT si47xxFMRX_get_frequency(void)
{
	// Get the tune status which contains the current frequency
    //fmTuneStatus(0,1);

    // Return the frequency
    return Freq;
}

MMP_UBYTE getIntStatus(void)
{
    MMP_UBYTE  cmd[1] = {0};
    MMP_UBYTE  rsp[1] = {0};
//	__msg("func=%s\n",__func__);
    // Put the ID for the command in the first byte.
    cmd[0] = GET_INT_STATUS;

    // Invoke the command
	si47xx_command(1, cmd, 1, rsp);
	printc("int--rsp[0]=%02x\r\n",rsp[0]);
	// Return the status
	return rsp[0];
}


static void fmSeekStart(MMP_UBYTE seekUp, MMP_UBYTE wrap)//(give 1 or 0) Begins searching for a valid frequency
{
    // Put the ID for the command in the first byte.
    cmd[0] = FM_SEEK_START;

	// Put the flags if the bit was set for the input parameters.
	cmd[1] = 0;
    if(seekUp) cmd[1] |= FM_SEEK_START_IN_SEEKUP;
	if(wrap)   cmd[1] |= FM_SEEK_START_IN_WRAP;

    // Invoke the command
	si47xx_command(2, cmd, 1, rsp);
}

static void fmTuneStatus(MMP_UBYTE cancel, MMP_UBYTE intack)
{
    // Put the ID for the command in the first byte.
    cmd[0] = FM_TUNE_STATUS;

	// Put the flags if the bit was set for the input parameters.
	cmd[1] = 0;
    if(cancel) cmd[1] |= FM_TUNE_STATUS_IN_CANCEL;
	if(intack) cmd[1] |= FM_TUNE_STATUS_IN_INTACK;

    // Invoke the command
	//si47xx_command(2, cmd, 8, rsp);
	si47xx_command(2, cmd, 8, rsp);
	/*{
		__s32 i =0;
		for(i=0;i<8;i++)
		{
			eLIBs_printf("rsp[%d]=%x\n",i,rsp[i]);
		}

	}*/
	
    // Parse the results
    STC1    = !!(rsp[0] & STCINT);
    BLTF   = !!(rsp[1] & FM_TUNE_STATUS_OUT_BTLF);
    AFCRL  = !!(rsp[1] & FM_TUNE_STATUS_OUT_AFCRL);
    Valid  = !!(rsp[1] & FM_TUNE_STATUS_OUT_VALID);
    Freq   = (rsp[2] << 8) | rsp[3];
    RSSI   = rsp[4];
    ASNR   = rsp[5];
    AntCap = rsp[7];

	printc(" zjh cur action Valid=%d,Freq=%d,rsp[1]=%d\r\n",Valid,Freq,rsp[1]);

	if(Valid)
	{
		printc(" zjh cur action Valid=%d,Freq=%d\r\n",Valid,Freq);
	}
}


void si47xxFMRX_gpio2_set_high(void)
{
	if(GpioSet){
		
	}else{
	
	    cmd[0] = GPIO_CTL;
	    cmd[1] = 0x04;
		si47xx_command(2, cmd, 0,NULL);
		AHC_OS_SleepMs(12);
	    cmd[0] = GPIO_SET;
		cmd[1] = 0x04;
		//si47xx_command(2, cmd, 0,NULL);
		si47xx_command(2, cmd, 0,NULL);
	    AHC_OS_SleepMs(12); 
		GpioSet = 1;
	}
}


//======================================AM================================================



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


#if 1
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
	    si47xx_set_property(AM_SEEK_FREQ_SPACING, 9); // Set spacing to 9kHz
	    si47xx_set_property(AM_SEEK_BAND_BOTTOM, 530); // Set the band bottom to 530kHz
	    si47xx_set_property(AM_SEEK_BAND_TOP, 1710);   // Set the band top to 1710kHz
	}
}


static void si47xxAMRX_general_cfg(void)
{

    si47xx_set_property(AM_SEEK_TUNE_SNR_THRESHOLD, 10);
    si47xx_set_property(AM_SEEK_TUNE_RSSI_THRESHOLD, 20);
}


//-----------------------------------------------------------------------------
// Configures the device for normal operation  SI47XX_AM_EUROPE
//-----------------------------------------------------------------------------
void si47xxAMRX_configure(am_country_enum val)
{
    // Configure all other registers
   // si47xxAMRX_hardware_cfg();
    si47xxAMRX_general_cfg();
    si47xxAMRX_regional_cfg(val);
	// Turn on the Headphone Amp and analog out.
//	M_INPUT_AD = 1;
//	M_OUTPUT_AD = 0;
//	GP1 = 1;
}


#endif


static void amTuneFreq(MMP_USHORT frequency)
{
    // Put the ID for the command in the first byte.

	printc("frequency=%d\r\n",frequency);
    cmd[0] = AM_TUNE_FREQ;

	// Initialize the reserved section to 0
    cmd[1] = 0;

	// Put the frequency in the second and third bytes.
    cmd[2] = (MMP_UBYTE)(frequency >> 8);
	cmd[3] = (MMP_UBYTE)(frequency & 0x00FF);

	// Set the antenna calibration value.
    cmd[4] = (MMP_UBYTE)0;  // Auto
	cmd[5] = (MMP_UBYTE)0;

    // Invoke the command
	si47xx_command(6, cmd, 1, rsp);
	printc("lyj~staut~~~~~%02x\r\n",rsp[0]);
	AHC_OS_SleepMs(400);
	
	
}





static void amSeekStart(MMP_UBYTE seekUp, MMP_UBYTE wrap)
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


static void amTuneStatus(MMP_UBYTE cancel, MMP_UBYTE intack)
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
    STC1    = !!(rsp[0] & STCINT);
    BLTF   = !!(rsp[1] & AM_TUNE_STATUS_OUT_BTLF);
    AFCRL  = !!(rsp[1] & AM_TUNE_STATUS_OUT_AFCRL);
    Valid  = !!(rsp[1] & AM_TUNE_STATUS_OUT_VALID);
    Freq   = ((MMP_USHORT)rsp[2] << 8) | (MMP_USHORT)rsp[3];
    RSSI   = rsp[4];
    ASNR   = rsp[5];
    AntCap = ((MMP_USHORT)rsp[6] << 8) | (MMP_USHORT)rsp[7]; 


	
	if(Valid)
	{
		printc(" lyj cur action Valid=%d,Freq=%d\r\n",Valid,Freq);
	}

	printc("lyj~staut~~am~~~%02x\r\n",rsp[0]);
	AHC_OS_SleepMs(1);

}



void si47xxAMRX_tune(MMP_USHORT frequency)
{
	// Enable the bit used for the interrupt of STC.
	MMP_USHORT err_count = 0;
	 MMP_UBYTE ret =0;
	


	// Call the tune command to start the tune.

	//amTuneStatus(0, 1);
	amTuneFreq(frequency);
	

	AHC_OS_SleepMs(1);  //5
   
		ret = getIntStatus();
    	//__msg("%d\n",ret);
		ret = !ret;
    	//__msg("%d\n",ret);
		while (ret & STCINT)
		{
			err_count++;
			if(err_count > 50000)
			{
				printc("have no STCINT\r\n");
			    err_count = 0;
				break;
			}
		}
 
	AHC_OS_SleepMs(20);
	// Clear the STC bit and get the results of the tune.
    amTuneStatus(0, 1);


	printc("freq =%d\r\n",Freq);

	// Disable the bit used for the interrupt of STC.
	AHC_OS_SleepMs(1);

    // Return the RSSI level
   // return RSSI;
}





void si47xxAMRX_initialize(void)
{
    // Zero status registers.
	PoweredUp = 0;

    // Perform a hardware reset, power up the device, and then perform the
    // initial configuration.
   // si47xx_reset();
    //si47xxAMRX_powerdown();
	//wait_10ms(6);
   // si47xxAMRX_powerup();


	
    //si47xxAMRX_hardware_cfg();
    //si47xxAMRX_general_cfg();


	
    si47xxAMRX_configure(SI47XX_AM_EUROPE);  //zjh mode  20171202
}



	













//======================================end=============================================

void si47xxFMRX_tune(MMP_USHORT frequency)
{
	// Enable the bit used for the interrupt of STC.
	MMP_ULONG err_count = 0;

	// Call the tune command to start the tune.
 	//WaitSTCInterrupt = 1;
	//__msg("frequency=%d\n",frequency);
	//fmTuneStatus(0, 1);

    fmTuneFreq(frequency);

	//Si473xSetFrq(frequency);

	
	AHC_OS_SleepMs(1);


    // wait for the interrupt before continuing
    // If you do not wish to use interrupts but wish to poll the part
    // then comment out this line.
//    __here__;
   // while (WaitSTCInterrupt); // Wait for interrupt to clear the bit
//	__here__;
    // Wait for stc bit to be set
   // while (!(getIntStatus() & STCINT));
    {
    	MMP_UBYTE ret =0;
		ret = getIntStatus();
    	//__msg("%d\n",ret);
		ret = !ret;
    	//__msg("%d\n",ret);
		while (ret & STCINT)
		{
			err_count++;
			if(err_count > 50000)
			{
				printc("STCINT~~~没有置1~\r\n");
			    err_count = 0;
				break;
			}
		}
   	}
   // __here__;
   // 
	
    

	// Clear the STC bit and get the results of the tune.

   
   fmTuneStatus(0, 1);

		//amTuneStatus(0,1);
   
	printc("freq =%d\r\n",Freq);
	// Disable the bit used for the interrupt of STC.
	//SeekTuneInProc = 0;
	AHC_OS_SleepMs(1);
    // Return the RSSI level

    //return RSSI;
}



MMP_UBYTE si47xxFMRX_seek(MMP_UBYTE seekup, MMP_UBYTE seekmode)
{
	// Enable the bit used for the interrupt of STC.
	//SeekTuneInProc = 1;

	// Call the tune command to start the seek.
 	//WaitSTCInterrupt = 1;
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
    	char  ret =0;
		ret = getIntStatus();
    	//__msg("%d\n",ret);
		ret = !ret;
    	//__msg("%d\n",ret);
		while (ret & STCINT);
   	}
	// Clear the STC bit and get the results of the tune.
    fmTuneStatus(0, 1);

	// Disable the bit used for the interrupt of STC.
	//SeekTuneInProc = 0;

    // The tuner is now set to the newly found channel if one was available
    // as indicated by the seek-fail bit.
    return BLTF; //return seek fail indicator
}


void Si473xGetTuneStatus(void)
{
	int i;
	MMP_UBYTE si473x_tune_command[2] = {0x23,0x01};

	si47xx_command(2, cmd, 8, rsp);

	for (i = 0; i < 8; i++)
		printc("---->rsp = %02x\r\n",rsp[i]);

	//return 0;
	
}







#if 1
void fm_auto_search(MMP_ULONG freQ)
{
	MMP_BOOL returnValue = 0;
	MMP_USHORT temp = 0;
	MMP_ULONG cur_freq=0;
	MMP_LONG valid = 0;
	
	if(work_mode == 0){

			temp = (MMP_USHORT)(freQ / 10);

			if((temp > si47xx_max_freq) || (temp < si47xx_min_freq))	{  //zjh mode
				//printc(" temp = %d\r\n", temp);
				return;
			
			}

			//returnValue = si47xxFMRX_seek(1, 1);
			//printc("-returnValue-->%d\r\n",returnValue);
			si47xxFMRX_tune(temp);
			
			//Si473xGetTuneStatus();

			//printc("---->freq = %d\r\n",Freq);

			#if 0
			
			if(Valid)
			{
				fmTuneFreq(Freq);
				break;
			}

			

			(temp)++;
			if(temp>10)
					break;
			#endif

}

	else{


			temp = (MMP_USHORT)(freQ / 10);

			if((temp > AM_SEARCH_EUR_MAX_FREQ/10) || (temp < AM_SEARCH_EUR_MIN_FREQ/10))	{ 
				//printc(" temp = %d\r\n", temp);
				return;
			
			}

			si47xxFMRX_tune(temp);
		





	}

}

void am_auto_search(MMP_ULONG freQ)
{

		MMP_USHORT temp = 0;

	if(work_mode == 0){

			temp = (MMP_USHORT)(freQ / 10);

			if((temp > AM_SEARCH_EUR_MAX_FREQ/10) || (temp < AM_SEARCH_EUR_MIN_FREQ/10))	{  //zjh mode
				//printc(" temp = %d\r\n", temp);
				return;
			
			}

			//returnValue = si47xxFMRX_seek(1, 1);
			//printc("-returnValue-->%d\r\n",returnValue);
			si47xxAMRX_tune(temp);
			
			//Si473xGetTuneStatus();

			//printc("---->freq = %d\r\n",Freq);

			#if 0
			
			if(Valid)
			{
				fmTuneFreq(Freq);
				break;
			}

			

			(temp)++;
			if(temp>10)
					break;
			#endif

}

		




}




MMP_ULONG Get_vailed_am_Freq(void)
{

	MMP_USHORT Tfreq = 522;
	
	MMP_USHORT radio[11] = {'\0'};

	MMP_UBYTE dex = 0;

	do{
		
		am_auto_search(Tfreq*10);

		AHC_OS_SleepMs(10);

			if(Valid)
			{
				//fmTuneFreq(Freq);
				//break;
				radio[RadiocntAm++] = Freq;
			}

		Tfreq += 9;
		//AHC_OS_SleepMs(500);
			
	} while(Tfreq < AM_SEARCH_EUR_MAX_FREQ/10);

	//printc("~~~total NUM = %d~~\r\n",Radiocnt);
	printc("lyj~~~~~~~~~~~~~~~\r\n");

	//for( dex = 0; dex < Radiocnt; dex++ )
			//printc("~~~Radio Freq = %d~~~~~~~~%d~\r\n",radio[dex],dex);
	AHC_OS_SleepMs(500);
	si47xxAMRX_tune(radio[dex-1]);
	return Freq*10;



}



#endif



MMP_ULONG Get_vailed_Freq(void)
{

	MMP_USHORT Tfreq = 8750;
	
	MMP_USHORT radio[11] = {'\0'};

	MMP_UBYTE dex = 0;

	do{
		
		fm_auto_search(Tfreq*10);

		AHC_OS_SleepMs(100);

			if(Valid)
			{
				//fmTuneFreq(Freq);
				//break;
				radio[Radiocnt++] = Freq;
			}

		Tfreq += 5;
		//AHC_OS_SleepMs(500);
			
	} while(Tfreq < FM_SEARCH_CHN_MAX_FREQ/10);

	//printc("~~~total NUM = %d~~\r\n",Radiocnt);

	f//or( dex = 0; dex < Radiocnt; dex++ )
			//printc("~~~Radio Freq = %d~~~~~~~~%d~\r\n",radio[dex],dex);
	AHC_OS_SleepMs(500);
	si47xxFMRX_tune(radio[dex-1]);
	return Freq*10;
}





void fm_Si4730_Init(void)
{
	si47xx_reset();
		
	Si4730PowerUp();

	//si47xxFMRX_gpio2_set_high();
	
	si47xx_set_property(GPO_IEN,GPO_IEN_STCIEN_MASK); //void si47xxFMRX_hardware_cfg(void) // Enable the  STC interrupt here

	//si47xx_getPartInformation();
	si47xxAMRX_set_volume(63);

	si47xxFMRX_general_cfg();

	si47xxFMRX_regional_cfg(SI47XX_CHA);

	si47xxFMRX_tune(9050);

	//si47xxAMRX_initialize();

	//si47xxAMRX_tune(1451);

	//Get_vailed_am_Freq();

	//si47xxAMRX_tune(747);
	//Get_vailed_Freq();

}

#endif






