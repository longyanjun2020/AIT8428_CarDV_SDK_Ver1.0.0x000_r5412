//==============================================================================
//
//  File        : JobDispatch.c
//  Description : ce job dispatch function
//  Author      : Robin Feng
//  Revision    : 1.0
//
//==============================================================================
#include "Customer_Config.h"

#include "includes_fw.h"
#include "mmpf_typedef.h"

#include "bootloader.h"		// CarDV

#include "mmp_reg_gpio.h"
#include "mmp_reg_gbl.h"
#include "mmp_reg_dma.h"
#include "mmp_reg_sd.h"

#include "os_wrap.h"
#include "mmph_hif.h"
#include "mmpd_display.h"
#include "mmps_dsc.h"
#include "mmps_fs.h"
#include "mmps_system.h"
#include "mmpd_system.h"
#include "mmpf_dram.h"
#include "mmpf_fs_api.h"
#include "mmps_sensor.h"
#include "mmps_usb.h"
#include "mmps_dsc.h"
#include "mmps_audio.h"
#include "mmps_vidplay.h"
#include "mmpf_uart.h"
#include "mmpf_pll.h"
#include "mmpf_storage_api.h"
#include "mmpf_sd.h"
#include "mmpf_hif.h"
#include "mmpf_mci.h"
#include "mmpf_system.h"
#include "mmpf_display.h"
#include "ait_config.h"
#include "mmpf_pio.h"
#include "mmpf_adc.h"
#include "mmpf_i2cm.h"
#include "mmpf_dma.h"
#include "Clk_cfg.h"
#if (USING_SM_IF == 1)
#include "mmpf_nand.h"
#endif
#include "PMUCtrl.h"        // CarDV
#include "fs_cfg.h" // CarDV

//==============================================================================
//
//                              COMPILING OPTIONS
//
//==============================================================================
/* 
#if defined(SD_BOOT)
#define BOOT_FROM_SD        (1)
#define BOOT_FROM_SF        (0)
#else
#define BOOT_FROM_SD        (0)
#define BOOT_FROM_SF        (1)
#endif
*/
#define BOOT_FROM_NAND      (0)

#if (BOOT_FROM_SF)
#include "mmpf_sf.h"
#endif

#ifndef JTAG_EN             // maybe defined in ADS
//#define JTAG_EN             (1) //Truman 161110: This option is no longer needed due to CZ patch 20160204
#endif
// When JTAG_EN is 1
// JTAG_TIME_OUT is 0 - go to JTAG immediately
//               is S - pause, and wait any key from UART in S seconds into JTAG.
//                      resume, if no any after S seconds.
//#define	JTAG_TIME_OUT		(1) //Truman 161110: This option is no longer needed due to CZ patch 20160204

/* FIXME To be reviewed. The target is to remove this define in case (1).
 * CZ wanted to use common code section instead of function for testing if Return key is pressed. (20161110)
 * But the code is not 100% equivalent (MMU initialization on SIF path) so I still use another function.*/
#define CR_SD_AND_SIF_FAST_JTAG (1)
#define DDRIIIEPFILENAME			"DDRIIIDelayRange.txt"

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
extern void Customer_Init_CarDV(void);
extern int SF_Module_Init(void);
#if (EN_WP_AREA_CONFIG == 1)
extern MMP_ERR MMPF_SF_SetProtWpArea(void);
extern MMP_ERR MMPF_SF_ClrProtArea(void);
extern MMP_BOOL MMPF_SF_IsProtect(void);
#endif
#if defined(MBOOT_SRAM_FW)
//extern FCTABLE_LIST FCTableContent[]; 
extern void MMPF_MMU_InvalidateDCacheMVA(MMP_ULONG ulRegion, MMP_ULONG ulSize); 
#endif

//#define DRAM_TEST

#ifdef DRAM_TEST
volatile MMP_UBYTE DMADone = 0;
void ReleaseDma(MMP_ULONG argu) 
{
	RTNA_DBG_Str(0, "ReleaseDma\r\n");
	DMADone = 1;
}

MMP_ERR DramTest1 (MMP_ULONG size, MMP_ULONG offset)     
{	
	MMP_ULONG *ulTempAddr = (MMP_ULONG *)0x1000000;
    MMP_ULONG i;
    for (i = 0 ; i < size; i+=offset) {
    	*(ulTempAddr +i)= i;
    }
    for (i = 0 ; i < size; i+=offset) {
    	if (*(ulTempAddr +i) != i) {
    		RTNA_DBG_Str(0, "Dram test fail 1:");
    		RTNA_DBG_Long(0, *(ulTempAddr +i));
    		RTNA_DBG_Str(0, ":");
    		RTNA_DBG_Long(0, i);
    		RTNA_DBG_Str(0, "\r\n");
    		//return MMP_SYSTEM_ERR_HW;
    	}
    }
    for (i = 0 ; i < size; i+=offset) {
    	*(ulTempAddr +i)= 0xFFFFFFFF - i;
    }
    for (i = 0 ; i < size; i+=offset) {
    	if (*(ulTempAddr +i) != 0xFFFFFFFF - i) {
    		RTNA_DBG_Str(0, "Dram test fail 2:");
    		RTNA_DBG_Long(0, *(ulTempAddr +i));
    		RTNA_DBG_Str(0, ":");
    		RTNA_DBG_Long(0, 0xFFFFFFFF-i);
    		RTNA_DBG_Str(0, "\r\n");
    		//return MMP_SYSTEM_ERR_HW;
    	}
    }
    return MMP_ERR_NONE;
}

MMP_ERR DramDMATest(MMP_ULONG src, MMP_ULONG dst, MMP_ULONG wordSize, MMP_USHORT usLoop)     
{
    MMP_ULONG   max_transfer;
    MMP_USHORT  *dev_src = (MMP_USHORT *)src;
    MMP_USHORT  *dev_dst = (MMP_USHORT *)dst;
    MMP_LONG	i;
    MMP_ULONG   loop = 0;
    MMP_USHORT	ret = 0;
    MMP_ULONG   fail_cnt = 0;
    
    MMP_GRAPHICS_BUF_ATTR 	srcbufattribute;
    MMP_GRAPHICS_RECT 		srcrect;
    MMP_GRAPHICS_BUF_ATTR 	dstbufattribute;

    if (wordSize > 0x20000) //(height - 1) can't over 12-bit
        wordSize = 0x20000;
    max_transfer = wordSize;

    srcbufattribute.usWidth = 16;
    srcbufattribute.usHeight = max_transfer >> 4;
    srcbufattribute.usLineOffset = 32;
    srcbufattribute.colordepth = MMP_GRAPHICS_COLORDEPTH_16;
    srcbufattribute.ulBaseAddr = (MMP_ULONG)dev_src;
    srcbufattribute.ulBaseUAddr = 0;
    srcbufattribute.ulBaseVAddr = 0;

    srcrect.usLeft = 0;
    srcrect.usTop = 0;
    srcrect.usWidth = srcbufattribute.usWidth;
    srcrect.usHeight = srcbufattribute.usHeight;

    dstbufattribute.usWidth = srcbufattribute.usWidth;
    dstbufattribute.usHeight = srcbufattribute.usHeight;
    dstbufattribute.usLineOffset = 32;
    dstbufattribute.colordepth = MMP_GRAPHICS_COLORDEPTH_16;
    dstbufattribute.ulBaseAddr = (MMP_ULONG)dev_dst;
    dstbufattribute.ulBaseUAddr = 0;
    dstbufattribute.ulBaseVAddr = 0;

    MMPF_DMA_Initialize();
    while (1) {
        if (loop == usLoop)
            break;

        loop++;

        RTNA_DBG_Str(0, "Loop test:");
        RTNA_DBG_Long(0, loop);
        RTNA_DBG_Str(0, "\r\n");
        fail_cnt = 0;

        srand(loop);
        for (i = 0 ; i < max_transfer; i++) {
            *(dev_src +i) = RAND() % 0xFFFF;
        }

        RTNA_DBG_Str(0, "Test:");
        RTNA_DBG_Long(0, (MMP_ULONG)dev_src);
        RTNA_DBG_Str(0, ":");
        RTNA_DBG_Long(0, (MMP_ULONG)dev_dst);
        RTNA_DBG_Str(0, ":");
        RTNA_DBG_Long(0, (MMP_ULONG)max_transfer);
        RTNA_DBG_Str(0, "\r\n");   
        DMADone = 0;
        MMPF_DMA_RotateImageBuftoBuf(&srcbufattribute, &srcrect, 
                                     &dstbufattribute, 0, 0, 
                                     MMP_GRAPHICS_ROTATE_NO_ROTATE, 
    	                             ReleaseDma, 0, MMP_FALSE, 2);
        while(DMADone == 0);

        for (i = 0; i < max_transfer; i++) {
            if (*(dev_src+i) != *(dev_dst+i)) {
                RTNA_DBG_Str(0, "Compare fail:");
                RTNA_DBG_Str(0, " ~");
                RTNA_DBG_Long(0, (MMP_ULONG)dev_src+i);
                RTNA_DBG_Str(0, ":");
                RTNA_DBG_Long(0, (MMP_ULONG)dev_dst+i);
                RTNA_DBG_Str(0, " = ");
                RTNA_DBG_Short(0, *(dev_src+i));
                RTNA_DBG_Str(0, ":");
                RTNA_DBG_Short(0, *(dev_dst+i));
                RTNA_DBG_Str(0, "\r\n");
                ret = 1;
                fail_cnt++;
                if (fail_cnt >= 0x20) {
                    break;
                }
            }
        }    
    }

    return  ret;
}
#endif

void Customer_Init(void)
{
	//PMUCtrl_Power_Gpio_En(MMP_TRUE);

#if (SNR_CLK_POWER_SAVING_SETTING != 0)
    MMPF_PLL_ChangeVIFClock(DPLL1, 1);      // CLK_GRP_SNR, 380MHz
    MMPF_PLL_WaitCount(50);
    MMPF_PLL_ChangeBayerClock(DPLL1, 1);    // CLK_GRP_BAYER, 380MHz
    MMPF_PLL_WaitCount(50);
    MMPF_PLL_ChangeISPClock(DPLL5, 1);      // CLK_GRP_COLOR, 480MHz
    MMPF_PLL_WaitCount(50);
#elif (MENU_MOVIE_SIZE_1080_60P_EN)
    //MMPF_PLL_ChangeISPClock(DPLL2, 1);      // CLK_GRP_COLOR  528MHz
    //MMPF_PLL_WaitCount(50);
#elif (CUSTOMIZED_DPLL1_CLOCK == 0)
    MMPF_PLL_ChangeVIFClock(DPLL1, 1);      // CLK_GRP_SNR, 380MHz
    MMPF_PLL_WaitCount(50);
#endif

#ifdef DEVICE_GPIO_SD_POWER
	// Turn-On SD Power
    if (DEVICE_GPIO_SD_POWER != MMP_GPIO_MAX) {
        MMPF_PIO_PadConfig(DEVICE_GPIO_SD_POWER, PAD_OUT_DRIVING(0), MMP_TRUE);
        MMPF_PIO_EnableOutputMode(DEVICE_GPIO_SD_POWER, MMP_TRUE, MMP_TRUE);
        MMPF_PIO_SetData(DEVICE_GPIO_SD_POWER, DEVICE_GPIO_SD_POWER_LEVEL, MMP_TRUE);
    }
#endif

		#if 0 // lyj 20190917
	MMPF_PIO_PadConfig(MMP_GPIO62, PAD_OUT_DRIVING(0), MMP_TRUE);// lyj 20180730
	MMPF_PIO_EnableOutputMode(MMP_GPIO62, MMP_TRUE, MMP_FALSE);
	MMPF_PIO_SetData(MMP_GPIO62, 1, MMP_TRUE);
	#endif


	Customer_Init_CarDV();
}

#if CR_SD_AND_SIF_FAST_JTAG
__inline CheckReturnForJtag(void)
{//CZ patch...20160204
	AITPS_GBL pGBL = AITC_BASE_GBL;
	extern unsigned char RTNA_DBG_GetChar_NoWait(void);

	if (RTNA_DBG_GetChar_NoWait() != 0) {
		MMPF_Uart_DisableRx(DEBUG_UART_NUM);
		pGBL->GBL_BOOT_STRAP_CTL = MODIFY_BOOT_STRAP_EN |
				ARM_JTAG_MODE_EN | ROM_BOOT_MODE |
				JTAG_CHAIN_MODE_DIS | JTAG_CHAIN_CPUB_SET0;
		pGBL->GBL_BOOT_STRAP_CTL &= ~(MODIFY_BOOT_STRAP_EN);
		RTNA_DBG_Str(0,"Please use JTAG download ALL FW...\r\n");
		while(1);
	}
}
#endif

#if defined(SD_FW_MAKE_MP_BIN)&&(SD_FW_MAKE_MP_BIN)
MMP_UBYTE BootRemoveDramDataWP(MMP_ULONG ulHasWpCfg)
{ 
	MMP_ULONG   ulFileID;
	MMP_ERR		ErrResult = MMP_FALSE;	
	char		FileName[256];
	MMP_ULONG	ulDmaAddr,i;
	AIT_STORAGE_INDEX_TABLE2 *pIndexTable;// = (AIT_STORAGE_INDEX_TABLE2 *)AIT_BOOT_HEADER_ADDR;
	//MMP_ULONG ulHasWpCfg = 0;

    MMPF_SF_GetTmpAddr(&ulDmaAddr);
    pIndexTable = (AIT_STORAGE_INDEX_TABLE2 *)ulDmaAddr;
    for (i = 10; i<MAX_PARTI_NUM; i++) {
		if (pIndexTable->it[i].ulFlag0 & PART_FLAG_WP) {
            ulHasWpCfg = MMP_TRUE;
            RTNA_DBG_Str0(FG_YELLOW("Has WpConfig!\r\n"));
            break;
        }
        else if (pIndexTable->it[i].ulFlag0 & 0xFFFFFFF0) {
            RTNA_DBG_Str0(FG_YELLOW("Bad Flag config!\r\n"));
        }   
    }

    STRCPY(FileName, "SF:1:\\DDRIIIDelayRange.txt");
    if (ulHasWpCfg) {
    	MEMSET(FileName, 0, sizeof(FileName));
    	STRCPY(FileName, (char *)FS_GetLastPartitionName());
		STRCAT(FileName, DDRIIIEPFILENAME);
    	MMPF_SF_ClrProtArea(); 
	    ErrResult = MMPF_FS_FOpen(FileName, "wb", &ulFileID);
    }
    else {
	    ErrResult = MMPF_FS_FOpen(FileName, "wb", &ulFileID);
    }
	
	if(ErrResult){
		RTNA_DBG_Str0(FG_YELLOW("MMPF_FS_FOpen DDRIIIDelayRange.txt fail\r\n"));
	}
	else{
		ErrResult = MMPF_FS_FClose(ulFileID);
		if(ErrResult)
			RTNA_DBG_Str0(FG_YELLOW("fail@ MMPF_FS_FClose DDRIIIDelayRange.txt\r\n"));

		ErrResult = MMPF_FS_Remove(FileName);
		if (ErrResult){
			RTNA_DBG_Str0(FG_YELLOW("fail@ MMPF_FS_FClose DDRIIIDelayRange.txt\r\n"));
		}
		else{
			RTNA_DBG_Str0(FG_YELLOW("success@ MMPF_FS_FClose DDRIIIDelayRange.txt\r\n"));
		}
	}
	return 0;
}
#endif

#if defined(MBOOT_SRAM_FW)
//In order to use FS in bootloader add defined(MBOOT_SRAM_FW) 

MMP_UBYTE InitialDelayRangeInfo(void){ 
	extern DDRIII_SCANRANGE_TABLE gNewScanRangeTable;

	memset(&gNewScanRangeTable,0,sizeof(gNewScanRangeTable));
	//gNewScanRangeTable.ulHeader      = DDRINFO_HEADER;
	//gNewScanRangeTable.ulTail        = DDRINFO_TAIL;
	return 0;
}

MMP_UBYTE FillDelayRangeInfoToSF(MMP_ULONG ulHasWpCfg){ 
	#if(BOOT_MEDIA_ACCESS) //Need if mesia raw access
	MMPF_SIF_INFO *info;	
	extern DDRIII_SCANRANGE_TABLE gNewScanRangeTable;
	#else // Need if mesia FS access
	MMP_ULONG   ulFileID;
	MMP_ULONG	ulWriteSize = 0;
	#endif	
	MMP_ERR		ErrResult = MMP_FALSE;	
	extern DDRIII_SCANRANGE_TABLE gNewScanRangeTable;
	char		FileName[256];
	//Manual fill DDRinfo
	/*
	MMPF_MMU_FlushDCacheMVA((MMP_ULONG) &gNewScanRangeTable, sizeof(gNewScanRangeTable));
	memset(&gNewScanRangeTable,0,sizeof(gNewScanRangeTable));		
	gNewScanRangeTable.ulHeader      = 0x4D435232;
	gNewScanRangeTable.ubTriangle    = 1;
	gNewScanRangeTable.ubReserved0   = 0xFF;
	gNewScanRangeTable.ubUpperBD     = 0x04;
	gNewScanRangeTable.ubUpperMIN    = 0x50;
	gNewScanRangeTable.ubUpperMAX    = 0xA8;
	gNewScanRangeTable.ubReserved1   = 0xFF;
	gNewScanRangeTable.ubDownBD      = 0x1A;
	gNewScanRangeTable.ubDownMIN     = 0x68;
	gNewScanRangeTable.ubDownMAX     = 0xB0;
	gNewScanRangeTable.usBestAsyncRd = 0x98;
	gNewScanRangeTable.ubBestDLY     = 0x70;
	gNewScanRangeTable.ulSkipScan    = 0x19283465;
	gNewScanRangeTable.ulTail        = 0x4D435232;	
	*/
	
	#if(BOOT_MEDIA_ACCESS) //Need if mesia raw access
	info = MMPF_SF_GetSFInfo();
	ErrResult = MMPF_SF_EraseSector( (MMP_ULONG)( (info->ulSFTotalSize) - (info->ulSFBlockSize) ) );	
	if(ErrResult)
		RTNA_DBG_Str0("fail@ MMPF_SF_EraseSector \r\n");
	
	//RTNA_DBG_Str(0, "SIF WriteAddress ulSFAddr = ");
	//RTNA_DBG_Long(0, (info->ulSFTotalSize) - (info->ulSFBlockSize) );
	//RTNA_DBG_Str(0, "\r\n");
	ErrResult = MMPF_SF_WriteData((MMP_ULONG)((info->ulSFTotalSize)-(info->ulSFBlockSize)), (MMP_ULONG) &gNewScanRangeTable, 256);
	if(ErrResult)
		RTNA_DBG_Str0("fail@ MMPF_SF_WriteData \r\n");	
	
	#else // Need if mesia FS access
    gNewScanRangeTable.ulHeader      = DDRINFO_HEADER;
	gNewScanRangeTable.ulTail        = DDRINFO_TAIL;
    
    STRCPY(FileName, "SF:1:\\DDRIIIDelayRange.txt");
    if (ulHasWpCfg) {
    	MEMSET(FileName, 0, sizeof(FileName));
    	STRCPY(FileName, (char *)FS_GetLastPartitionName());
		STRCAT(FileName, DDRIIIEPFILENAME);
    	MMPF_SF_ClrProtArea(); // Rom code will clear WP setting in MV2. However, add this to make sure the WP status is clear.
	    ErrResult = MMPF_FS_FOpen(FileName, "wb", &ulFileID);
    }
    else {
	    ErrResult = MMPF_FS_FOpen(FileName, "wb", &ulFileID);
    }
	if(ErrResult)
		RTNA_DBG_Str0(FG_YELLOW("fail@ MMPF_FS_FOpen DDRIIIDelayRange.txt\r\n"));
	ErrResult = MMPF_FS_FWrite(ulFileID,(void *)&(gNewScanRangeTable.ulHeader),0x0200,&ulWriteSize);	
	if(ErrResult)
		RTNA_DBG_Str0(FG_YELLOW("fail@ MMPF_FS_FWrite DDRIIIDelayRange.txt\r\n"));
	ErrResult = MMPF_FS_FClose(ulFileID);
	if(ErrResult)
		RTNA_DBG_Str0(FG_YELLOW("fail@ MMPF_FS_FClose DDRIIIDelayRange.txt\r\n"));
    
    RTNA_DBG_Str(0, "Write DDRIII data into SF & show detail:\r\n");
    RTNA_DBG_Str(0, "ulHeader:");
    RTNA_DBG_Long(0, gNewScanRangeTable.ulHeader);
    RTNA_DBG_Str(0, "\r\n");
    //AsyncRd
    RTNA_DBG_Str(0, "ulSearchDirection:");
    RTNA_DBG_Long(0, gNewScanRangeTable.ulSearchDirection);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "ubUpperBD:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubUpperBD);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "ubUpperMIN:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubUpperMIN);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "ubUpperMAX:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubUpperMAX);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "ubDownBD:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubDownBD);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "ubDownMIN:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubDownMIN);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "ubDownMAX:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubDownMAX);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "usBestAsyncRd:");
    RTNA_DBG_Short(0, gNewScanRangeTable.usBestAsyncRd);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "ubBestDLY:");
    RTNA_DBG_Short(0, gNewScanRangeTable.ubBestDLY);
    RTNA_DBG_Str(0, "\r\n");
    //DataEyeDly
    RTNA_DBG_Str(0, "ubDataEyeDlyBlk0:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubDataEyeDlyBlk0);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "ubDataEyeDlyBlk1:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubDataEyeDlyBlk1);
    RTNA_DBG_Str(0, "\r\n");
    //WrLeveling
        RTNA_DBG_Str(0, "ubAfeWRDQSDHL0:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubAfeWRDQSDHL0);
    RTNA_DBG_Str(0, "\r\n");
        RTNA_DBG_Str(0, "ubAfeWRDQSDHL1:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubAfeWRDQSDHL1);
    RTNA_DBG_Str(0, "\r\n");
        RTNA_DBG_Str(0, "ubAfeRWDlyUpdateEN:");
    RTNA_DBG_Byte(0, gNewScanRangeTable.ubAfeRWDlyUpdateEN);
    RTNA_DBG_Str(0, "\r\n");
    
    RTNA_DBG_Str(0, "ulSkipScan:");
    RTNA_DBG_Long(0, gNewScanRangeTable.ulSkipScan);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "ulTail:");
    RTNA_DBG_Long(0, gNewScanRangeTable.ulTail);
    RTNA_DBG_Str(0, "\r\n");
    
	#endif	
	return 0;
}

MMP_UBYTE ExtractDelayRangeInfo(DDRIII_SCANRANGE_TABLE * pScanRangeTable, MMP_ULONG ulHasWpCfg){ //return=> 0:no need to scan; 1:need scan
	#if(BOOT_MEDIA_ACCESS) //Need if mesia raw access
	MMPF_SIF_INFO *info;	
	#else // Need if mesia FS access
	MMP_ULONG   ulFileID;
	MMP_ULONG	ulReadSize = 0;
	#endif		
	MMP_ERR		ErrResult = MMP_FALSE;
	char		FileName[256];	
	extern DDRIII_SCANRANGE_TABLE gNewScanRangeTable;

	MMPF_MMU_InvalidateDCacheMVA(AIT_BOOT_HEADER_ADDR, 512);
	memset((void *)AIT_BOOT_HEADER_ADDR,0,512);
	#if(BOOT_MEDIA_ACCESS) //Need if mesia raw access
	//SF initial process
	//SF_Module_Init();
	MMPF_SF_SetIdBufAddr((MMP_ULONG)(STORAGE_TEMP_BUFFER - 0x0200));//0x00106000-0x0200
	MMPF_SF_SetTmpAddr((MMP_ULONG)(STORAGE_TEMP_BUFFER - 0x1000));//0x00106000-0x1000
  	MMPF_SF_InitialInterface();  	
	ErrResult = MMPF_SF_Reset();
	if (ErrResult) {
		RTNA_DBG_Str(0, "SIF Reset error !!\r\n");
		return MMP_TRUE;
	}
	info = MMPF_SF_GetSFInfo();

	//FillDelayRangeInfoToSF();
	//RTNA_DBG_Str(0, "SIF ReadAddress ulSFAddr = ");
	//RTNA_DBG_Long(0, (info->ulSFTotalSize) - (info->ulSFBlockSize) );
	//RTNA_DBG_Str(0, "\r\n");	
	ErrResult = MMPF_SF_FastReadData( (MMP_ULONG)((info->ulSFTotalSize) - (info->ulSFBlockSize)), (MMP_ULONG)pScanRangeTable, 512);
	if(ErrResult) {
		RTNA_DBG_Str0("fail@ MMPF_SF_FastReadData \r\n");	
		return MMP_TRUE; 
	}
	#else // Need if mesia FS access
	/* //verify usage
	//FillDelayRangeInfoToSF();	
	ErrResult = MMPF_FS_Remove("SF:1:\\DDRIIIDelayRange.txt");
	if (ErrResult)
		RTNA_DBG_Str0("fail@ MMPF_FS_Remove \r\n");	
    while(1);
	*/
	STRCPY(FileName, "SF:1:\\DDRIIIDelayRange.txt");
	
	if (ulHasWpCfg) {
		MEMSET(FileName, 0, sizeof(FileName));
		STRCPY(FileName, (char *)FS_GetLastPartitionName());
		STRCAT(FileName, DDRIIIEPFILENAME);
		ErrResult = MMPF_FS_FOpen(FileName, "rb", &ulFileID);
    }
    else {
    	ErrResult = MMPF_FS_FOpen(FileName, "rb", &ulFileID);
        RTNA_DBG_Str0(FG_YELLOW("SF:1:\\"));
    }
    
	if (ErrResult) {
        RTNA_DBG_Str0(FG_YELLOW("DDRIIIDelayRange.txt Miss!!\r\n"));        
		return MMP_TRUE;
    }
	else {
        RTNA_DBG_Str0(FG_YELLOW("DDRIIIDelayRange.txt Hit!!\r\n"));        
		MMPF_FS_FRead(ulFileID, (void *)AIT_BOOT_HEADER_ADDR, 0x200, &ulReadSize);			
		MMPF_FS_FClose(ulFileID);
	}
	#endif
	
	if ((pScanRangeTable->ulHeader == DDRINFO_HEADER) && 
		(pScanRangeTable->ulTail == DDRINFO_TAIL) ){
		gNewScanRangeTable.ulHeader      = DDRINFO_HEADER;
		gNewScanRangeTable.ulSearchDirection    = pScanRangeTable->ulSearchDirection;		
		gNewScanRangeTable.ubReserved0   = 0xFF;
		gNewScanRangeTable.ubUpperBD     = pScanRangeTable->ubUpperBD;
		gNewScanRangeTable.ubUpperMIN    = pScanRangeTable->ubUpperMIN;
		gNewScanRangeTable.ubUpperMAX    = pScanRangeTable->ubUpperMAX;		
		gNewScanRangeTable.ubReserved1   = 0xFF;
		gNewScanRangeTable.ubDownBD 	 = pScanRangeTable->ubDownBD;
		gNewScanRangeTable.ubDownMIN 	 = pScanRangeTable->ubDownMIN;
		gNewScanRangeTable.ubDownMAX 	 = pScanRangeTable->ubDownMAX;
		gNewScanRangeTable.usBestAsyncRd = pScanRangeTable->usBestAsyncRd;        
		gNewScanRangeTable.ubBestDLY 	 = pScanRangeTable->ubBestDLY;	

        gNewScanRangeTable.ubDataEyeDlyBlk0 	 = pScanRangeTable->ubDataEyeDlyBlk0;        
        gNewScanRangeTable.ubDataEyeDlyBlk1 	 = pScanRangeTable->ubDataEyeDlyBlk1;
        
        gNewScanRangeTable.ubAfeWRDQSDHL0 	 = pScanRangeTable->ubAfeWRDQSDHL0;
        gNewScanRangeTable.ubAfeWRDQSDHL1 	 = pScanRangeTable->ubAfeWRDQSDHL1;
        gNewScanRangeTable.ubAfeRWDlyUpdateEN 	 = pScanRangeTable->ubAfeRWDlyUpdateEN;
        
		gNewScanRangeTable.ulSkipScan 	 = 0xEFCD3412;        	
	    gNewScanRangeTable.ulTail        = DDRINFO_TAIL;
		return MMP_FALSE;		
	}
	else {
		gNewScanRangeTable.ulSkipScan = 0;
		return MMP_TRUE;	
	}
}

MMP_UBYTE CheckTXTForScanLockCore(MMP_ULONG *ulHasWpCfg){ //return=> 0:no need to scan; 1:need scan	
	MMP_ERR		ErrResult = MMP_FALSE;	
	DDRIII_SCANRANGE_TABLE *pScanRangeTable = (DDRIII_SCANRANGE_TABLE *)AIT_BOOT_HEADER_ADDR;
	#if(BOOT_MEDIA_ACCESS) //Need if mesia raw access
	stSDMMCHandler  *pSDHandle;
	extern stSDMMCHandler m_SDMMCArg[MMPF_SD_DEV_NUM];
	pSDHandle = m_SDMMCArg+STOREFIRMWARE_SD_ID;
	#else // Need if mesia FS access
	MMP_ULONG   ulLoop;
	MMP_ULONG   ulFileID;
	extern void _SD_DevSetPad(MMP_ULONG ulPad);
	#endif
	AIT_STORAGE_INDEX_TABLE2 *pIndexTable;
	MMP_ULONG   ulDmaAddr, i;

	#if(BOOT_MEDIA_ACCESS) //Need if mesia raw access
	MMPF_MMU_InvalidateDCacheMVA(AIT_BOOT_HEADER_ADDR, 512);
	memset((void *)AIT_BOOT_HEADER_ADDR,0,512);
	MMPF_SD_ReadSector(pSDHandle, AIT_BOOT_HEADER_ADDR, SECTOR_DDRINFO, 1); 
	RTNA_DBG_Str0("Header: ");
	RTNA_DBG_Long(0,pScanRangeTable->ulHeader);
	RTNA_DBG_Str0("   Tail: ");
	RTNA_DBG_Long(0,pScanRangeTable->ulTail);
	RTNA_DBG_Str0("\r\n");
	if ((pScanRangeTable->ulHeader == DDRINFO_HEADER) && 
		(pScanRangeTable->ulTail == DDRINFO_TAIL) ){
		RTNA_DBG_Str0("DDR INFO header&tail is in SD physical sector 8195\r\n");
		return MMP_TRUE;	
	}
	#else // Need if mesia FS access
	//FileSystem initialize
	SF_Module_Init();	
	_SD_DevSetPad(SD_SOCKET0_MAP_ID);            
    for(ulLoop = 0; ulLoop < FS_MAX_UNITS; ++ulLoop){
        MMPF_STORAGE_RegisterFSDevie(&gpFS_DeviceCustom[ulLoop], (MMP_ULONG)&acCacheMemory[ulLoop], FS_CACHE_LENGTH, 
            NULL, NULL  ); //Set custom FS device.  CarDV...                    
    }     //(MMP_ULONG)(m_ubSDFreeClusterTable[ulLoop]) //FCTableContent[ulLoop].ulTableLength                                                    
	for(ulLoop = 0; ulLoop < MSDC_MAX_UNITS; ++ulLoop){    
        MMPF_STORAGE_RegisterMSDCDevie(&gpMSDC_DeviceCustom[ulLoop]);
    }          
    MMPS_System_AllocateFSBuffer(MMPS_SYSTEM_APMODE_DSC);	
	
	ErrResult = MMPF_FS_FOpen("SD:0:\\scanlockcore.txt", "rb", &ulFileID);
	if (ErrResult == MMP_ERR_NONE){	
		MMPF_FS_FClose(ulFileID);
		return MMP_TRUE;
	}	
	#endif

    MMPF_SF_GetTmpAddr(&ulDmaAddr);
    pIndexTable = (AIT_STORAGE_INDEX_TABLE2 *)ulDmaAddr;
    for (i = 10; i<MAX_PARTI_NUM; i++) {
		if (pIndexTable->it[i].ulFlag0 & PART_FLAG_WP) {
            *ulHasWpCfg = MMP_TRUE;
            RTNA_DBG_Str0(FG_YELLOW("Has WpConfig!\r\n"));
            break;
        }
        else if (pIndexTable->it[i].ulFlag0 & 0xFFFFFFF0) {
            RTNA_DBG_Str0(FG_YELLOW("Bad Flag config!\r\n"));
        }   
    }
	ErrResult = ExtractDelayRangeInfo(pScanRangeTable, *ulHasWpCfg);	
	
	return ErrResult;
}

#endif

void CE_JOB_DISPATCH_Task(void *p_arg)
{
    void (*FW_Entry)(void) = NULL;

#if (BOOT_FROM_SD)
	#if !defined(FAT_BOOT)
    extern stSDMMCHandler m_SDMMCArg[MMPF_SD_DEV_NUM];
    #endif
    #if (CHIP == MCR_V2)
    AIT_STORAGE_INDEX_TABLE2 *pIndexTable = (AIT_STORAGE_INDEX_TABLE2 *)AIT_BOOT_HEADER_ADDR;
    MMP_ULONG   ulTableAddr;
    #endif
    stSDMMCHandler  *pSDHandle;
#endif	// BOOT_FROM_SD

	#if (BOOT_FROM_SF == 1)
	AIT_STORAGE_INDEX_TABLE2 *pIndexTable = (AIT_STORAGE_INDEX_TABLE2 *)STORAGE_TEMP_BUFFER;
    MMP_ULONG   ulSifAddr = 0x0;
	MMP_USHORT  usCodeCrcValue = 0x0;
	MMP_ERR     err;
	#endif

    MMP_ULONG   ulDramFreq, ulHasWpCfg = 0;

    RTNA_DBG_Str0("MBoot Job Task!\r\n");

    PMUCtrl_Power_Gpio_En(MMP_TRUE);
    MMPF_PLL_GetGroupFreq(CLK_GRP_DRAM, &ulDramFreq);
    ulDramFreq = ulDramFreq/1000;
    RTNA_DBG_PrintLong(0, ulDramFreq);

	#if defined(MBOOT_SRAM_FW)
	if ( CheckTXTForScanLockCore(&ulHasWpCfg) ) {
		InitialDelayRangeInfo();
	#endif	
		/*DRAM Init*/
		#if (AUTO_DRAM_LOCKCORE)
    	MMPF_DRAM_ScanNewLockCore(MMPS_System_GetConfig()->stackMemoryType,
                            &(MMPS_System_GetConfig()->ulStackMemorySize),
                            ulDramFreq,
                            MMPS_System_GetConfig()->stackMemoryMode);
		#endif	
	#if defined(MBOOT_SRAM_FW)	
		FillDelayRangeInfoToSF(ulHasWpCfg);
	}
	else{
		
        #if (AUTO_DRAM_LOCKCORE)
    	MMPF_DRAM_ScanNewLockCore(MMPS_System_GetConfig()->stackMemoryType,
                            &(MMPS_System_GetConfig()->ulStackMemorySize),
                            ulDramFreq,
                            MMPS_System_GetConfig()->stackMemoryMode);
		#endif
	}	
	#endif
	
	#if (EN_WP_AREA_CONFIG == 1)
    if (ulHasWpCfg) {
        MMPF_SF_SetProtWpArea();
    }
	#endif

    /*Bandwidth Configuration*/
    #if (MCI_READ_QUEUE_4 == 1)
    MMPF_MCI_SetQueueDepth(4);
    #endif

    #if defined(MBOOT_FW)
    /*Display Init*/
    RTNA_DBG_Str(0,"Display a moment\r\n");
    MMPC_Display_InitConfig(); 
    MMPC_Display_InitLCD();
    #endif

    /*Platfrom Init*/
    Customer_Init();

	MMPF_MMU_Deinitialize();

    #if 1//(JTAG_EN)
    {
	    extern void RTNA_DBG_RxCallback(MMP_UBYTE size, volatile MMP_UBYTE *fifo);

	    MMPF_Uart_EnableRx(DEBUG_UART_NUM, 1, (UartCallBackFunc *)&RTNA_DBG_RxCallback);
		
		#if 1 //CZ patch...20160204
		RTNA_DBG_Str(0, "Press any key to enter JTAG before . running\r\n");
		#else		
	    if (cnt != 0) {
			RTNA_DBG_Str(0, "Press any key to enter JTAG before . running\r\n");
	    }
		while (cnt != 0) {
			if (RTNA_DBG_GetChar_NoWait() != 0) {
				break;
			}
			RTNA_DBG_Str(0, ".");
			MMPF_OS_Sleep(100);
			cnt--;
		}
	    MMPF_Uart_DisableRx(DEBUG_UART_NUM);
		if (cnt || cnt == (JTAG_TIME_OUT * 10)) {
			AITPS_GBL pGBL = AITC_BASE_GBL; 
		    pGBL->GBL_BOOT_STRAP_CTL = MODIFY_BOOT_STRAP_EN |
		                        ARM_JTAG_MODE_EN | ROM_BOOT_MODE |
		                        JTAG_CHAIN_MODE_DIS | JTAG_CHAIN_CPUB_SET0;
		    pGBL->GBL_BOOT_STRAP_CTL &= ~(MODIFY_BOOT_STRAP_EN);
		    RTNA_DBG_Str(0,"Please use JTAG download ALL FW...\r\n");
		    while(1);
		}
		#endif		
    }
    #endif

	/*Download firmware from moviNAND*/
	#if (BOOT_FROM_SD)
	/*moviNAND reset*/
	#if !defined(FAT_BOOT)
    pSDHandle = m_SDMMCArg+STOREFIRMWARE_SD_ID;
	MMPF_SD_InitHandler();
	MMPF_SD_InitialInterface(pSDHandle);
	MMPF_SD_SetTmpAddr(AIT_FW_TEMP_BUFFER_ADDR, 0);
	
	if(MMPF_SD_Reset(pSDHandle)) {
		MMPF_MMC_Reset(pSDHandle);
	}
	#endif
	
	#if defined(FAT_BOOT)
	RTNA_DBG_Str(0,"Start load moviNAND code\r\n");
	{
		#define		SECTOR_SIZE	(512)
		MMP_ERR		err;
		MMP_ULONG   ulfileHandle;
		MMP_ULONG   ulReadCount = 0;
		MMP_ULONG   FileSystemReadBuffer = AIT_BOOT_HEADER_ADDR;
		AIT_STORAGE_INDEX_TABLE2 *pIndex;
        MMP_ULONG ulLoop;
		#if 1
		extern void _SD_DevSetPad(MMP_ULONG ulPad);
            _SD_DevSetPad(SD_SOCKET0_MAP_ID);
            
            for(ulLoop = 0; ulLoop < FS_MAX_UNITS; ++ulLoop){
                MMPF_STORAGE_RegisterFSDevie(&gpFS_DeviceCustom[ulLoop], (MMP_ULONG)&acCacheMemory[ulLoop], FS_CACHE_LENGTH, 
                    (MMP_ULONG)(m_ubSDFreeClusterTable[ulLoop]), sizeof(*m_ubSDFreeClusterTable[ulLoop])); //Set custom FS device.  CarDV...                    
            }

            for(ulLoop = 0; ulLoop < MSDC_MAX_UNITS; ++ulLoop){    
                MMPF_STORAGE_RegisterMSDCDevie(&gpMSDC_DeviceCustom[ulLoop]);
            }  
        
		MMPS_System_AllocateFSBuffer(MMPS_SYSTEM_APMODE_DSC);
		#endif
		
		err = MMPF_FS_FOpen("SD:\\SdFwCode.bin", "rb", &ulfileHandle);
		if (err == MMP_ERR_NONE)
		{						
			MMPF_FS_FRead(ulfileHandle, (void *)FileSystemReadBuffer, SECTOR_SIZE, &ulReadCount);	
			if (ulReadCount == SECTOR_SIZE) {
            	pIndex = (AIT_STORAGE_INDEX_TABLE2 *)FileSystemReadBuffer;
            	if((pIndex->ulHeader == INDEX_TABLE_HEADER) &&
                	(pIndex->ulTail == INDEX_TABLE_HEADER) &&
                	(pIndex->ulFlag & 0x1))
            	{ 
					MMPF_FS_FSeek(ulfileHandle,SECTOR_SIZE*(1+pIndex->ulTotalSectorsInLayer), MMP_FS_FILE_BEGIN);
					MMPF_FS_FRead(ulfileHandle, (void *)FileSystemReadBuffer, SECTOR_SIZE, &ulReadCount);
					
	                RTNA_DBG_PrintLong(0, pIndex->bt.ulStartBlock);
	                RTNA_DBG_PrintLong(0, pIndex->ulTotalSectorsInLayer);
        	        RTNA_DBG_PrintLong(0, pIndex->bt.ulDownloadDstAddress);
            	    
            		MMPF_FS_FRead(ulfileHandle, (void *)pIndex->bt.ulDownloadDstAddress, pIndex->bt.ulCodeSize, &ulReadCount);
            	}
            }
		}
	}
	#endif

	/*read boot area partition table*/
	MMPF_SD_ReadSector(pSDHandle, AIT_BOOT_HEADER_ADDR, 0, 1);
    //RTNA_DBG_PrintLong(0, pIndexTable->it[10].ulStartSec);
    RTNA_DBG_PrintLong(0, pIndexTable->bt.ulDownloadDstAddress);


    ulTableAddr = pIndexTable->ulTotalSectorsInLayer;
    MMPF_SD_ReadSector(pSDHandle, AIT_BOOT_HEADER_ADDR, ulTableAddr, 1);

    /*read boot code to  AIT_FW_START_ADDR (0x1000000 DRAM start address)*/
    if ((pIndexTable->ulHeader == INDEX_TABLE_HEADER) && 
		(pIndexTable->ulTail == INDEX_TABLE_TAIL) &&
		(pIndexTable->ulFlag & 0x1)) {
        MMPF_SD_ReadSector(pSDHandle, pIndexTable->bt.ulDownloadDstAddress, 
                            pIndexTable->bt.ulStartBlock,
                            (pIndexTable->bt.ulCodeSize + 511) >>
                            pIndexTable->ulAlignedBlockSizeShift);
        FW_Entry = (void (*)(void))(pIndexTable->bt.ulDownloadDstAddress);
        RTNA_DBG_PrintLong(0, pIndexTable->bt.ulDownloadDstAddress);
        RTNA_DBG_Str(0,"End load moviNAND code\r\n");
	#if CR_SD_AND_SIF_FAST_JTAG
		CheckReturnForJtag();
	#endif
    }
    else {
		RTNA_DBG_PrintLong(0, pIndexTable->ulHeader);
		RTNA_DBG_PrintLong(0, pIndexTable->ulTail);
		RTNA_DBG_PrintLong(0, pIndexTable->ulFlag);
		RTNA_DBG_Str(0, "Invalid Index Table!!\r\n");
		while(1);
	}
	#endif //(BOOT_FROM_SD)

	#if (BOOT_FROM_NAND)&&(USING_SM_IF)
    RTNA_DBG_Str(0,"Start load NAND code\r\n");
    /*switch SD to access boot area*/
    //MMPF_MMC_SwitchBootPartition(STOREFIRMWARE_SD_ID, 1);			
    /*read boot area partition table*/
    MMPF_NAND_SetMemory(AIT_FW_TEMP_BUFFER_ADDR);
    MMPF_NAND_InitialInterface();
    MMPF_NAND_Reset();
    MMPF_NAND_ReadSector(AIT_BOOT_HEADER_ADDR, 0,1);

    /*read boot code to  AIT_FW_START_ADDR (0x1000000 DRAM start address)*/
    MMPF_NAND_ReadSector(AIT_FW_START_ADDR, pAit_boot_header->it[1].ulStartSec, pAit_boot_header->it[1].ulSecSize);
    RTNA_DBG_Str(0,"End load NAND code\r\n");	
    #endif //(BOOT_FROM_NAND)

	#if (BOOT_FROM_SF)
	SF_Module_Init();
	MMPF_SF_SetIdBufAddr((MMP_ULONG)(STORAGE_TEMP_BUFFER - 0x1000));
	MMPF_SF_SetTmpAddr((MMP_ULONG)(STORAGE_TEMP_BUFFER - 0x1000));
  	MMPF_SF_InitialInterface();
	
	err = MMPF_SF_Reset();
	if (err) {
		RTNA_DBG_Str(0, "SIF Reset error !!\r\n");
		return;
	}
	
	MMPF_SF_FastReadData(ulSifAddr, STORAGE_TEMP_BUFFER, 512);  //AitMiniBootloader Header Table
	ulSifAddr = ulSifAddr + (0x1 << 12)*(pIndexTable->ulTotalSectorsInLayer);
	MMPF_SF_ReadData(ulSifAddr, (STORAGE_TEMP_BUFFER), 512);  //AitBootloader Header Table
	ulSifAddr = ulSifAddr + (0x1 << 12)*(pIndexTable->ulTotalSectorsInLayer);
	#if 0//CZ patch...20160204
	while(1) {
		MMPF_SF_ReadData(ulSifAddr, (STORAGE_TEMP_BUFFER), 512);  //Firmware Header Table
		if ((pIndexTable->ulHeader == INDEX_TABLE_HEADER) && 
			(pIndexTable->ulTail == INDEX_TABLE_TAIL) &&
			(pIndexTable->ulFlag & 0x1)) {
				RTNA_DBG_Str(0, "PASS\r\n");
			}
	    else {
	    		RTNA_DBG_Str(0, "FAIL\r\n");
	    }
	}
	#else
	MMPF_SF_ReadData(ulSifAddr, (STORAGE_TEMP_BUFFER), 512);  //Firmware Header Table
	#endif
	if ((pIndexTable->ulHeader == INDEX_TABLE_HEADER) && 
		(pIndexTable->ulTail == INDEX_TABLE_TAIL) &&
		(pIndexTable->ulFlag & 0x1)) {
			RTNA_DBG_PrintLong(0, pIndexTable->bt.ulDownloadDstAddress);
			FW_Entry = (void (*)(void))(pIndexTable->bt.ulDownloadDstAddress);

		#if (CHIP == MCR_V2)
			if(pIndexTable->ulFlag & 0x40000000) { //check bit 30
				MMPF_SF_EnableCrcCheck(MMP_TRUE);
			}
		#endif 
			RTNA_DBG_Str(0, "SIF downlod start\r\n");
			
			RTNA_DBG_PrintLong(0, pIndexTable->bt.ulStartBlock);

			MMPF_SF_FastReadData(pIndexTable->bt.ulStartBlock << pIndexTable->ulAlignedBlockSizeShift, 
								 pIndexTable->bt.ulDownloadDstAddress, 
								 pIndexTable->bt.ulCodeSize);

		#if (CHIP == MCR_V2)
			if(pIndexTable->ulFlag & 0x40000000) { //check bit 30
				usCodeCrcValue = MMPF_SF_GetCRC();
				MMPF_SF_FastReadData(ulSifAddr + 0x1000, STORAGE_TEMP_BUFFER, 2); //Read CRC block value
				if(*((MMP_USHORT*)STORAGE_TEMP_BUFFER) == usCodeCrcValue) {
					RTNA_DBG_Str(0, "CRC pass \r\n");
				}
				else {
					RTNA_DBG_Str(0, "CRC check fail !!! \r\n");
					RTNA_DBG_PrintShort(0, usCodeCrcValue);
					RTNA_DBG_PrintShort(0, *((MMP_USHORT*)STORAGE_TEMP_BUFFER));
					while(1);
				}
			}	
		#endif
			RTNA_DBG_Str(0, "SIF download End\r\n");
			RTNA_DBG_Str(0, "\r\n");
		#if CR_SD_AND_SIF_FAST_JTAG
			CheckReturnForJtag();
		#else
			{//CZ patch...20160204
				AITPS_GBL pGBL = AITC_BASE_GBL; 
			    //extern void RTNA_DBG_RxCallback(MMP_UBYTE size, volatile MMP_UBYTE *fifo);
				extern unsigned char RTNA_DBG_GetChar_NoWait(void);

				if (RTNA_DBG_GetChar_NoWait() != 0) {
				    MMPF_Uart_DisableRx(DEBUG_UART_NUM);
					pGBL->GBL_BOOT_STRAP_CTL = MODIFY_BOOT_STRAP_EN |
				                        ARM_JTAG_MODE_EN | ROM_BOOT_MODE |
				                        JTAG_CHAIN_MODE_DIS | JTAG_CHAIN_CPUB_SET0;
				    pGBL->GBL_BOOT_STRAP_CTL &= ~(MODIFY_BOOT_STRAP_EN);
				    RTNA_DBG_Str(0,"Please use JTAG download ALL FW...\r\n");	
				    while(1);

				}
    		}
		#endif

            //Test only
            MMPF_MMU_Initialize((MMP_ULONG *)MMU_TRANSLATION_TABLE_ADDR,(MMP_ULONG *)MMU_COARSEPAGE_TABLE_ADDR);
            RTNA_DBG_Str(0, "\r\nGoto ALL_FW...\r\n");

    		FW_Entry();	//enter the firmware entry
    		while (1);
	}
	else {
		RTNA_DBG_PrintLong(0, pIndexTable->ulHeader);
		RTNA_DBG_PrintLong(0, pIndexTable->ulTail);
		RTNA_DBG_PrintLong(0, pIndexTable->ulFlag);
		RTNA_DBG_Str(0, "Invalid Index Table!!\r\n");
		while(1);
	}
	#endif //(BOOT_FROM_SF)
	
    /* Jump PC to AIT_FW_START_ADDR */
    FW_Entry = (void (*)(void))(AIT_FW_START_ADDR);
    FW_Entry();

    while (1) ;
}

