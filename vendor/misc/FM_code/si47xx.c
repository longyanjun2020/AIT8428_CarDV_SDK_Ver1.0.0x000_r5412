/*
*********************************************************************************************************
*											        ePDK
*						            the Easy Portable/Player Develop Kits
*									           hello world sample
*
*						        (c) Copyright 2006-2007, Steven.ZGJ China
*											All	Rights Reserved
*
* File    : rda5820.c
* By      : xueli
* Version : V1.00
*********************************************************************************************************
*/



#include "MenuSetting.h"// lyj 20190114
#include  "si47xx.h"
#include "si47xxFMRX.h"
#include "si47xxAMRX.h"

#define READ    1
#define WRITE   0
#define  FM_SEARCH_CHN_MIN_FREQ         87500
#define  FM_SEARCH_CHN_MAX_FREQ        108000

#define  FM_SEARCH_JAP_MIN_FREQ         76000
#define  FM_SEARCH_JAP_MAX_FREQ         91000   //108000

#define  AM_SEARCH_EUR_MIN_FREQ         5220
#define  AM_SEARCH_EUR_MAX_FREQ         16200


#define  AM_SEARCH_OTHER_MIN_FREQ         5300
#define  AM_SEARCH_OTHER_MAX_FREQ         17100


#define  SW_SEARCH_CHN_MIN_FREQ         4750
#define  SW_SEARCH_CHN_MAX_FREQ         5060


//#define  FM_SEARCH_CHN_MIN_FREQ         87500
//#define  FM_SEARCH_CHN_MAX_FREQ        108000

//#define  FM_SEARCH_JAP_MIN_FREQ         76000
//#define  FM_SEARCH_JAP_MAX_FREQ         91000

#define  FM_SEARCH_USA_MIN_FREQ         87500
#define  FM_SEARCH_USA_MAX_FREQ         107900


#define  FM_SEARCH_RUSSIAN_MIN_FREQ     65000
#define  FM_SEARCH_RUSSIAN_MAX_FREQ     74000




//send 脚为LOW
#define SI47xx_WR_ADDRESS  0x22       /// 写地址
#define SI47xx_RD_ADDRESS  0x23       /// 读地址


__u8    Radiocnt = 0;

__u8   RadiocntAm = 0;


__u32 si47xx_max_freq = 0;
__u32 si47xx_min_freq = 0;


//__u32 si47xx_freq_step = 0;
__u32 si47xx_area =0;
__s32 loc_flag = 0;
//__s32 st_flag = 0;


__s32 test_am_rssi = 0;
__s32 test_am_snr = 0;


//__u32 fm_snr=0;
//__u32 fm_rssi=0;
__u32 am_snr=0;
__u32 am_rssi=0;
__u32  g_fm_db = 4;
__u32  g_loc_default = 28;
__u32  g_loc_setval = 36;
static __s32  fm_get_data_flag = 0;

MMP_USHORT radio[31] = {'\0'};/*{8750,8760,8770,8760,8780,8790,8800,8810,8820,8830,\
								  9050,9060,9070,9080,9090,10010,10020,10030,10040,10050,\
								   10060,10070,10080,10090,10100,10110,10120,10130,10620,10720};*///{'\0'};
MMP_USHORT radio_am[21] = {'\0'};/*{522,529,536,543,550,557,558,559,660,770,559,689,782,1024,1056,1420};*///{'\0'};
MMP_USHORT FMCurruntFreq =0;
MMP_USHORT AMCurruntFreq =0;

extern u8  xdata Valid;
extern u16 xdata Freq;

extern AHC_BOOL 			return_flag;

extern AHC_BOOL displayFreqflag ;// lyj 20190215
extern AHC_BOOL displayFreqflagAM;



//MMP_ULONG Get_vailed_Freq(void);
//MMP_ULONG Get_vailed_am_Freq(void);

//extern void Draw_FM_AM_icon(MMP_USHORT Dfreq);

//extern void Draw_AM_icon(MMP_USHORT Dfreq);



//send 脚为HIGH
//#define SI47xx_WR_ADDRESS 0xC6
//#define SI47xx_RD_ADDRESS 0xC7

 void si47xx_Rest(void)
{
		MMPS_PIO_EnableOutputMode(FM_PATH_SELECT_GPIO, MMP_TRUE);
		MMPS_PIO_SetData(FM_PATH_SELECT_GPIO, FM_PATH_SELECT_SET);
		AHC_OS_SleepMs(10);

		MMPS_PIO_SetData(FM_PATH_SELECT_GPIO, FM_PATH_SELECT_RESET);
		AHC_OS_SleepMs(450);

		printc("~~~~~~~~reset finish!~~~~\r\n");

}




#if 0
static void si_reset_on(void)    
{                        
	
}

static void si_reset_off(void)  
{


}

void si47xx_Rest(void)
{

   si_reset_on();
  // esKRNL_TimeDly(10);     
   si_reset_off();
  // esKRNL_TimeDly(5);
   //__here__;
}

#endif

void wait_10ms(u16 ms)
{
	//esKRNL_TimeDly(ms);
	AHC_OS_SleepMs(ms);
}

unsigned char  work_mode =100;// lyj 20190213

extern AHC_BOOL work_mode1;
void  fm_si47xx_init(void)
{
 #if 1
	work_mode = 0;
 	//if(work_mode1 == 100)
		work_mode1 = work_mode;
	//if(MenuSettingConfig()->uiBeep !=1 && MenuSettingConfig()->uiBeep !=2) //AM
	//displayFreqflagAM=0;// lyj 20190215
	//FMCurruntFreq = 0;
    si47xxFMRX_initialize();
	 si47xx_min_freq = FM_SEARCH_USA_MIN_FREQ/10;
	 si47xx_max_freq = FM_SEARCH_USA_MAX_FREQ/10;
	


    si47xxAMRX_set_volume(63);  // lyj 20190213
	AHC_OS_SleepMs(1);
	IIC_write_data(0x04,0x03,1);
  // si47xxAMRX_mute(1);// lyj 20190213
    //fm_mute(DRV_FM_VOICE_OFF);
	if(FMCurruntFreq != 0)
	si47xxFMRX_tune(FMCurruntFreq); // lyj 20190610
	

	 //si47xxFMRX_tune(9050);//8980 

	 //Draw_FM_AM_icon(9050);

	//Get_vailed_Freq();

 
 	//si47xxFMRX_autoseek();*/
	

	#endif

}
void  am_si47xx_init(void)//-----------ok
{

	work_mode =1;
		//if(work_mode1 == 100)
		work_mode1 = work_mode;
		//FMCurruntFreq = 0;
		//if(MenuSettingConfig()->uiMOVQuality !=0 && MenuSettingConfig()->uiMOVQuality !=1 &&\
		//	MenuSettingConfig()->uiMOVQuality !=2)// FM
		//displayFreqflag=0;// lyj 20190215

		
	si47xxAMRX_initialize();
	 si47xx_min_freq = AM_SEARCH_OTHER_MIN_FREQ/10;//AM_SEARCH_EUR_MIN_FREQ/10;
	 si47xx_max_freq = AM_SEARCH_OTHER_MAX_FREQ/10;//AM_SEARCH_EUR_MAX_FREQ/10;	
	
    si47xxAMRX_set_volume(63);      // full volume, turn off mute
    AHC_OS_SleepMs(10);
	IIC_write_data(0x04,0x03,1);
	//  si47xxAMRX_mute(1);// lyj 20190213
	//Get_vailed_am_Freq();
	if(FMCurruntFreq != 0)
		si47xxAMRX_tune(FMCurruntFreq);// lyj 20190610


}

void  sw_si47xx_init(void)//-----------ok
{

	work_mode =2;
    si47xxAMSWLWRX_initialize();
	//freq_range->fm_area_min_freq = SW_SEARCH_CHN_MIN_FREQ;
    //freq_range->fm_area_max_freq = SW_SEARCH_CHN_MAX_FREQ;		
    si47xxAMRX_set_volume(63);     // full volume, turn off mute
}


__s32  fm_si47xx_exit(void)
{
	//work_mode = 0;
	work_mode1 = 0; // lyj 20190213
	si47xxFMRX_powerdown();
	//si_reset_on();

    return 0;
}

__s32  am_si47xx_exit(void)
{
	//work_mode = 1;
	work_mode1 = 1;// lyj 20190213
	si47xxAMRX_powerdown();
	//si_reset_on();

    return 0;
}

/**********************************************
* function:      fm_play
*
* description:   根据输入的频率，播放此频率的
*                电台节目，即使没有节目只有
*                噪音，照样播出
*
* notes:         只返回成功
*
**********************************************/
 __s32 fm_si47xx_play(__s32 freq)
 {
	if(0 == work_mode){
		si47xxFMRX_tune(freq/10); 	
	}else if(1 == work_mode){
		si47xxAMRX_tune(freq/10); 	
	}else if(2 == work_mode){
		si47xxAMRX_tune(freq); 	
	}	
 	//fm_si47xx_mute(DRV_FM_VOICE_ON);	
	
 	return 0;
 }



__s32  fm_si47xx_get_rssi_value(void)
{
   return  test_am_rssi;
}
 __s32  fm_si47xx_get_snr_valule(void)
{
   return  test_am_snr;
}



/**********************************************
* function:      fm_auto_search
*
* description:   自动搜索，支持向上搜索和向下搜索
*
* notes:         搜索到一个频点后退出，返回值是一个XX.X MHz单位
*
**********************************************/
extern  UINT16 wDispID;
__s32  fm_si47xx_auto_search(__s32 freq)
{
		MMP_USHORT temp = 0;
	
		if(work_mode == 0){

			temp = (MMP_USHORT)(freq / 10);

			if((temp > si47xx_max_freq) || (temp < si47xx_min_freq))	{  //zjh mode
				//printc(" temp = %d\r\n", temp);
				return 1;
			
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

			//printc("~~~~~~~~~lyj~~???????~~\r\n");
			temp = (MMP_USHORT)(freq / 10);

			if((temp > /*AM_SEARCH_EUR_MAX_FREQ*/AM_SEARCH_OTHER_MAX_FREQ/10) || (temp < /*AM_SEARCH_EUR_MIN_FREQ*/AM_SEARCH_OTHER_MIN_FREQ/10))	{ 
				//printc(" temp = %d\r\n", temp);
				return 1;
			
			}

			si47xxAMRX_tune(temp);
		





	}
	return 0;
}




MMP_ULONG Get_vailed_Freq(void)
{
#if 0
	MMP_USHORT Tfreq = 8750;
	
	//MMP_USHORT radio[11] = {'\0'};

	MMP_UBYTE dex = 0;

	do{
		
		fm_si47xx_auto_search(Tfreq*10);

		AHC_OS_SleepMs(10);

			if(Valid)
			{
				//fmTuneFreq(Freq);
				//break;
				radio[Radiocnt++] = Freq;
			}

		Draw_FM_AM_icon(Tfreq);

		Tfreq += 10;
		//AHC_OS_SleepMs(500);

			
	} while(Tfreq < FM_SEARCH_CHN_MAX_FREQ/10);

	//printc("~~~total NUM = %d~~\r\n",Radiocnt);

	for( dex = 0; dex < Radiocnt; dex++ )
			printc("~~~Radio Freq = %d~~~~~~~~%d~\r\n",radio[dex],dex);
	AHC_OS_SleepMs(300);
	if(radio[dex-1])
	{
		si47xxFMRX_tune(radio[dex-1]);
		Draw_FM_AM_icon(radio[dex-1]);
	}	
	else
	{
		si47xxFMRX_tune(9050);
		Draw_FM_AM_icon(9050);		
	}
		
	return Freq*10;

	#endif
}


MMP_ULONG Get_vailed_Freq_manul(MMP_USHORT Tfreq)
{



	MMP_UBYTE dex = 0;

	
	if(Tfreq < FM_SEARCH_CHN_MAX_FREQ/10)
	{
		
		fm_si47xx_auto_search(Tfreq*10);

		AHC_OS_SleepMs(100);

			if(Valid)
			{
				//fmTuneFreq(Freq);
				//break;
				radio[Radiocnt++] = Freq;
				//MenuSettingConfig()->uiLCDBrightness = radio;
				//MenuSettingConfig()->uiTimeZone= Radiocnt;// lyj 20190216
			}

		Draw_FM_AM_icon_Ex(Tfreq,wDispID);


			
	} 

	//printc("~~~total NUM = %d~~\r\n",Radiocnt);

	/*for( dex = 0; dex < Radiocnt; dex++ )
			printc("~~~Radio Freq = %d~~~~~~~~%d~\r\n",radio[dex],dex);*/

		
	return Freq*10;
}




MMP_ULONG Get_vailed_am_Freq(void)
{
	#if 0
	MMP_USHORT Tfreq = 522;
	MMP_UBYTE i;
	MMP_UBYTE dex = 0;

	for(i = 0; i < 11; i++)
	
	 radio[i] = 0;

	

	do{
		
		fm_si47xx_auto_search(Tfreq*10);

		AHC_OS_SleepMs(10);

			if(Valid)
			{
				//fmTuneFreq(Freq);
				//break;
				radio[Radiocnt++] = Freq;
			}

		Draw_AM_icon(Tfreq);

		Tfreq += 9;

		//AHC_OS_SleepMs(500);
			
	} while(Tfreq < AM_SEARCH_EUR_MAX_FREQ/10);

	//printc("~~~total NUM = %d~~\r\n",Radiocnt);
	printc("lyj~~~~~~~~~~~~~~~\r\n");

	for( dex = 0; dex < Radiocnt; dex++ )
			printc("~~~Radio Freq = %d~~~~~~~~%d~\r\n",radio[dex],dex);
	AHC_OS_SleepMs(500);
	if(radio[0] != 0)
		si47xxAMRX_tune(radio[dex-1]);
	else
		si47xxAMRX_tune(747);
	return Freq*10;

#endif

}


MMP_ULONG Get_vailed_am_Freq_manul(MMP_USHORT Tfreq)
{


	MMP_UBYTE i;
	MMP_UBYTE dex = 0;

	/*for(i = 0; i < 21; i++)
	
	 radio_am[i] = 0;*/

	

	if(Tfreq < AM_SEARCH_OTHER_MAX_FREQ/*AM_SEARCH_EUR_MAX_FREQ*//10) // lyj 20190627
	{
		
		fm_si47xx_auto_search(Tfreq*10);

		AHC_OS_SleepMs(10);

			if(Valid)
			{
				//fmTuneFreq(Freq);
				//break;
				radio_am[RadiocntAm++] = Freq;
				//MenuSettingConfig()->uiTimeZone = radio_am;
				//MenuSettingConfig()->uiLCDBrightness= RadiocntAm;// lyj 20190216
			}

		Draw_AM_icon_Ex(Tfreq,wDispID);
			
	} 



	/*for( dex = 0; dex < Radiocnt; dex++ )
			printc("~~~Radio Freq = %d~~~~~~~~%d~\r\n",radio[dex],dex);*/

	return Freq*10;



}





/**********************************************
* function:      fm_manual_search
*
* description:   手动搜索，使用fm_play
*                返回值是下一个搜索频率点
*
* notes:
*
**********************************************/
__s32 fm_si47xx_manual_search(__s32 freq)
{
	__bool returnValue = 0;
	__u16 temp = 0;
	__u32 cur_freq=0;
	__s32 valid = 0;
	printc(" freq = %d, temp = %d,work_mode=%d\n", freq, temp,work_mode);	
	if(work_mode == 0){

		temp = (__u16)(freq / 10);
		while(1)	{
			
			if((temp > si47xx_max_freq) || (temp < si47xx_min_freq))	{  //zjh mode
				printc(" temp = %d\n", temp);
				return 1;
			}
			//returnValue = si47xxFMRX_seek(1, 1);
			si47xxFMRX_tune(temp);
			//__msg("returnValue=%d\n",returnValue);


			if(temp == 9600 || temp == 9900)
			{
				valid = 0;
			}
			else
			{
			   //valid = si47xxFMRX_get_valid();  
			}

#if 0
			if(temp == 8750)
			{
				test_am_rssi = si47xxFMRX_get_rssi();
				test_am_snr = si47xxFMRX_get_snr();
			}
			else if(temp == 8980)
			{
				test_am_rssi = si47xxFMRX_get_rssi();
				test_am_snr = si47xxFMRX_get_snr();
			}
#endif

				
			printc("valid=%d\n",valid);
			if(valid){
				//fm_si47xx_mute(DRV_FM_VOICE_ON);
				printc("lyj cur_feq =%d\n",temp*10);
				return temp*10;
			}else {
				//fm_si47xx_mute(DRV_FM_VOICE_OFF);
				return (temp * 10)|0xff000000;
			}


			
		}
	}else if(work_mode == 1){
		temp = (__u16)(freq/10);
		//eLIBs_printf(" temp = %d\n", temp);
		while(1)	{
			if(work_mode == 1){
				if((temp > si47xx_max_freq) || (temp < si47xx_min_freq))	{
					printc(" temp = %d\n", temp);
					return 1;
				}	
			}else if(work_mode == 2){
				if((temp > SW_SEARCH_CHN_MAX_FREQ) || (temp < SW_SEARCH_CHN_MIN_FREQ))	{
					printc(" temp = %d\n", temp);
					return 1;
				}
			}
			
			//returnValue = si47xxAMRX_seek(1, 1);
			si47xxAMRX_tune(temp);
			//__msg("returnValue=%d\n",returnValue);
			//valid = si47xxAMRX_get_valid();
			//eLIBs_printf("valid=%d\n",valid);	


			//valid = si47xxAMRX_get_valid();



			if(valid){				
				printc("cur_feq =%d\n",freq);
				//fm_si47xx_mute(DRV_FM_VOICE_ON);
				return freq;
			}else {
			    //fm_si47xx_mute(DRV_FM_VOICE_ON);
				return (temp*10)|0xff000000;
			}	
		}
	}	
    return 0;
}
/**********************************************
* function:      fm_area_choose
*
* description:   地区选择，区别不同的起始和终止
*                频率
*
* notes:         输入正确地区返回成功，否则失败
*
**********************************************/                    //OK

#if 0
__s32 fm_si47xx_area_choose(__s32 area)
{



	if(work_mode == 0)
	{
		if((area == DVR_FM_AREA_JAPAN) || (area == DRV_FM_AREA_CHN_SCHOOL))   {

			si47xx_max_freq = FM_SEARCH_JAP_MAX_FREQ / 10;
			si47xx_min_freq= FM_SEARCH_JAP_MIN_FREQ / 10;
			//si47xx_area = SI47XX_JAPAN;
			freq_range->fm_area_min_freq = FM_SEARCH_JAP_MIN_FREQ;
			freq_range->fm_area_max_freq = FM_SEARCH_JAP_MAX_FREQ;	

			//si47xxFMRX_configure(SI47XX_JAPAN);
		}
		else if(area == DRV_FM_AREA_CHN_US_EUP)
		{
			si47xx_max_freq = FM_SEARCH_CHN_MAX_FREQ / 10;
			si47xx_min_freq = FM_SEARCH_CHN_MIN_FREQ / 10;
			freq_range->fm_area_min_freq = FM_SEARCH_CHN_MIN_FREQ;
			freq_range->fm_area_max_freq = FM_SEARCH_CHN_MAX_FREQ;	
			//si47xx_area = SI47XX_EUROPE;
			//si47xxFMRX_configure(SI47XX_EUROPE);
		}
		else if(area == DRV_FM_AREA_CHN_USA1)
		{
			si47xx_max_freq = FM_SEARCH_USA_MAX_FREQ / 10;
			si47xx_min_freq = FM_SEARCH_USA_MIN_FREQ / 10;
			freq_range->fm_area_min_freq = FM_SEARCH_USA_MIN_FREQ;
			freq_range->fm_area_max_freq = FM_SEARCH_USA_MAX_FREQ;

			//si47xxFMRX_configure(SI47XX_USA1);
			//si47xx_area = SI47XX_USA1;
		}
		else if(area == DRV_FM_AREA_CHN_USA2)
		{
			si47xx_max_freq = FM_SEARCH_USA_MAX_FREQ / 10;
			si47xx_min_freq = FM_SEARCH_USA_MIN_FREQ / 10;
			freq_range->fm_area_min_freq = FM_SEARCH_USA_MIN_FREQ;
			freq_range->fm_area_max_freq = FM_SEARCH_USA_MAX_FREQ;
			//si47xxFMRX_configure(SI47XX_USA2);
			//si47xx_area = SI47XX_USA2;
		}
		else if(area == DRV_FM_AREA_CHN_RUSSIAN)
		{
			si47xx_max_freq = FM_SEARCH_USA_MAX_FREQ / 10;
			si47xx_min_freq = FM_SEARCH_USA_MIN_FREQ / 10;
			freq_range->fm_area_min_freq = FM_SEARCH_USA_MIN_FREQ;
			freq_range->fm_area_max_freq = FM_SEARCH_USA_MAX_FREQ;
			//si47xxFMRX_configure(SI47XX_RUSSIAN);
			//si47xx_area = SI47XX_RUSSIAN;
		}
		else 
		{
			return EPDK_FAIL;
		}
	}
	else
	{
		if(area == DRV_FM_AREA_CHN_US_EUP)
		{
			si47xx_max_freq = AM_SEARCH_EUR_MAX_FREQ / 10;
			si47xx_min_freq = AM_SEARCH_EUR_MIN_FREQ / 10;
			freq_range->fm_area_min_freq = AM_SEARCH_EUR_MIN_FREQ;
			freq_range->fm_area_max_freq = AM_SEARCH_EUR_MAX_FREQ;	
			//si47xx_area = SI47XX_AM_EUROPE;
		}
		else
		{
			si47xx_max_freq = AM_SEARCH_OTHER_MAX_FREQ / 10;
			si47xx_min_freq = AM_SEARCH_OTHER_MIN_FREQ / 10;
			freq_range->fm_area_min_freq = AM_SEARCH_OTHER_MIN_FREQ;
			freq_range->fm_area_max_freq = AM_SEARCH_OTHER_MAX_FREQ;	
			//si47xx_area = SI47XX_AM_OTHER;
		}
	}
	
	return EPDK_OK;

}

#endif
unsigned char  fm_si47xx_get_mode(void)
{
 	//__msg("work_mode=%d\n",work_mode);
    return work_mode;
}


__s32 fm_si47xx_get_status(void)
{
 
    return 0;
}


/**********************************************
* function:      fm_russian_fm2_sel
**********************************************/
#if  0
__s32 fm_si47xx_russian_fm3_choose(__s32 area, void *pbuffer)
{
    __drv_fm_area_freq_t   *freq_range = (__drv_fm_area_freq_t *)pbuffer;
    if(area == DRV_FM_AREA_CHN_RUSSIAN)
	{
        si47xx_max_freq = FM_SEARCH_RUSSIAN_MAX_FREQ / 10;
        si47xx_min_freq = FM_SEARCH_RUSSIAN_MIN_FREQ / 10;
	 	freq_range->fm_area_min_freq = FM_SEARCH_RUSSIAN_MIN_FREQ;
        freq_range->fm_area_max_freq = FM_SEARCH_RUSSIAN_MAX_FREQ;	
		//si47xxFMRX_configure(SI47XX_RUSSIAN1);
		//freq_step = QND_FSTEP_50KHZ;
	}
    return EPDK_OK;
}

#endif


__s32 fm_si47xx_get_radio_loc(void)
{
	return  loc_flag;
}

__s32  fm_si47xx_set_radio_loc(__s32 flag)
{

	loc_flag = flag;
}



__s32 fm_si47xx_get_st_flag(void)
{

	__u8 r_reg[8] ={0};
	__u8 stcommand[2] = {0x23,0x01};
	//si47xx_write_data(0xff,stcommand,2);
   
	//si47xx_read_data(0xff,r_reg,8);
	if((r_reg[3]&0x80) == 0x80)
	{
		return 0;
	}
	return 1;	
}


const __u16 SI_MONO 	=0x2000;


void Si473xSetStereo(void)
{
    __u16 propNumber1 = 0x1800;
	__u16 propValue =  0x14;
	__u16 propNumber2 = 0x1801;
	si47xx_set_property(propNumber1,propValue);
	si47xx_set_property(propNumber2,propValue);
}

void Si473xSetMono(void)
{

    __u16 propNumber1 = 0x1800;
	__u16 propValue =  0x7f;
	__u16 propNumber2 = 0x1801;

	si47xx_set_property(propNumber1,propValue);
	si47xx_set_property(propNumber2,propValue);

}



/**********************************************
* function:      fm_stereo_choose
*
* description:   音质选择，立体声和普通声音
*
* notes:
*
**********************************************/                 //OK
__s32 fm_si47xx_stereo_choose(__s32 audio_method)
{
	if(audio_method == 0)
	{
		//st_flag = 0;
		Si473xSetMono();
	}
	else
	{
		//st_flag = 1;
		Si473xSetStereo();
	}
    return 0;

}

__s32 fm_si47xx_pa_gain(__u8 pagain)
{
   
	return 0;
}

__s32 fm_si47xx_signal_gain(__u8 sigain)
{
   
	return 0;
}

/**********************************************
* function:      fm_send
*
* description:   发射功能
*
* notes:
*
**********************************************/
__s32 fm_si47xx_send_on(void)
{

    return 0;
}

__s32 fm_si47xx_send(__s32 freq)//2======================================
{
 
    return 0;
}

__s32 fm_si47xx_send_off(void)
{
   
    return 0;
}
/**********************************************
* function:      fm_mute
*
* description:   静音功能
*
* notes:
*
**********************************************/     //OK
__s32 fm_si47xx_mute(__s32 voice_onoff)//2=================================
{
	if (voice_onoff == 0){
		//si47xxFMRX_set_volume(0);
		si47xxFMRX_mute(1);
	}else{
		//si47xxFMRX_set_volume(63);
		si47xxFMRX_mute(0);
	}
    return 0;
}


/**********************************************
* function:      fm_signal_level
*
* description:   信号强度选择，要求信号强大越高，收到的电台越少
*                   要求信号强大越高，收到的电台越多，但无效电台也多
*
* notes:
*
**********************************************/             //OK
__s32 fm_si47xx_signal_level(__s32 signal_level)
{
    return 0;
}


// lyj 20190114
//extern PowerUpFlag PowerOnFlag;
void ReserveTheStatus(void)
{
	//if(FMCurruntFreq!=0)
	//{
		MMP_UBYTE writeData[] ={0,0,0,0};
	writeData[0] = FMCurruntFreq>>8; 
	writeData[1] = FMCurruntFreq&0xff; 
	writeData[2] = AMCurruntFreq>>8;
	writeData[3] = AMCurruntFreq&0xff; 
		IIC_write_data_e2prom(0xe0,writeData,4);
	//}

}

void ReadTheStatus(void)
{

	MMP_UBYTE ReadData[] = {0,0,0,0};
	iic_read_bytes_e2prom(0xe0,ReadData,4);
	FMCurruntFreq = (ReadData[0]<<8) + ReadData[1];
	AMCurruntFreq = (ReadData[2]<<8) + ReadData[3];
	if(FMCurruntFreq<8750 || FMCurruntFreq>10800)
		FMCurruntFreq = 0;
	if(AMCurruntFreq<522 || AMCurruntFreq>1620)
		AMCurruntFreq = 0;
	printc("~~~@_@~~~status == %d~~~~AMCurruntFreq = %d~~~~\r\n",FMCurruntFreq,AMCurruntFreq);

}





void ReserveTheRadio_AM_FM(void)
{
	MMP_UBYTE i ,WriteData[40] = {'\0'};
	if(radio[0] !=0)
	{
		for(i = 0; i< Radiocnt; i++)
		{
			if(radio[i])
			{
				WriteData[2*i] = 	(radio[i] >> 8);
				WriteData[2*i +1] = 	radio[i] & 0xff;
				//printc("radio -Freq == %d,H == %d,L== %d,-----i== %d\n",radio[i],WriteData[2*i],WriteData[2*i+1],i);
			}
		}
		for(i = 0; i< (2*Radiocnt)/8;i++)
		{
			if(i > 0)
				MMPF_OS_Sleep_MS(100);
			IIC_write_data_e2prom(0x00+i*8,WriteData+i*8,8);
		}

			MMPF_OS_Sleep_MS(100);

			IIC_write_data_e2prom(0x00+(2*Radiocnt/8)*8,WriteData+(2*Radiocnt/8)*8,(2*Radiocnt)%8);
	}

	
	if(radio_am[0] !=0)
	{
		for(i = 0; i< RadiocntAm; i++)
		{
			if(radio_am[i])
			{
				WriteData[2*i] = 	radio_am[i] >> 8;
				WriteData[2*i +1] = 	radio_am[i] & 0xff;
			
			}
		}
		for(i = 0; i<(2*RadiocntAm)/8;i++)
		{
			if(i > 0)
				MMPF_OS_Sleep_MS(100);
			IIC_write_data_e2prom(0x80+i*8,WriteData+i*8,2*RadiocntAm);
		}
		MMPF_OS_Sleep_MS(100);

		IIC_write_data_e2prom(0x80 + (2*RadiocntAm/8)*8,WriteData+(2*RadiocntAm/8)*8,(2*RadiocntAm)%8);
	}

		//printc(" write finish!!!!!!!!!!!!!!!!!!!\n");
		//if(PowerOnFlag.Fm == 1)
		MMPF_OS_Sleep_MS(100); // lyj 20190625
		ReserveTheStatus(); // lyj 20190605


	return ;
}


void PowerOnReadTheE2promToAM_FM(void)
{
		MMP_UBYTE i ,temp,ReadData[40] = {'\0'};
	
		if(MenuSettingConfig()->uiTimeZone !=0)
		{
			temp = 2*MenuSettingConfig()->uiTimeZone;
			/*for(i =0; i<temp/8;i++)	
				iic_read_bytes_e2prom(0x00+i*8,ReadData,8);
			MMPF_OS_Sleep_MS(50);
				iic_read_bytes_e2prom(0x00+(temp/8)*8,ReadData,temp%8);*/

				iic_read_bytes_e2prom(0x00,ReadData,temp);

			for(i =0; i< temp;i++)
			{
				if(i%2 == 1)
				{
					radio[Radiocnt++] = ReadData[i] + (ReadData[i-1]<<8);
					//printc("~~~~~~~~~~~FMfreq == %04d~L = %d,~H-%d,~i == %d~~~~~\n",radio[Radiocnt-1],ReadData[i],ReadData[i-1],i-1);
				}
			}
			
			//printc("~~~fm~~~~~temp == %d~~~~~~~~~~\n",temp);
		}

		if(MenuSettingConfig()->uiLCDBrightness !=0)
		{

			temp = 2*MenuSettingConfig()->uiLCDBrightness;
				
			iic_read_bytes_e2prom(0x80,ReadData,temp);

			for(i =0; i< temp;i++)
			{
				if(i%2 == 1)
				radio_am[RadiocntAm++] = ReadData[i] + (ReadData[i-1]<<8);

			}

			//printc("~~~am~~~~~temp == %d~~~~~~~~~~\n",temp);
		}



}





