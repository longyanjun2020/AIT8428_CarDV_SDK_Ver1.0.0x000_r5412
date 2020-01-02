//==============================================================================
//
//  File        : mmps_system.c
//  Description : Ritian System Control function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
@file mmps_system.c
@brief The System Control functions
@author Penguin Torng
@version 1.0
*/

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_lib.h"
#include "mmps_system.h"
#include "mmph_hif.h"
#include "mmp_reg_gbl.h"
#include "mmpd_audio.h"
#include "ait_utility.h"
#include "mmps_pio.h"
#include "fs_cfg.h"    //FS_NEWBUFFER 
/** @addtogroup MMPS_System
@{
*/

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

/**@brief The system configuration

Use @ref MMPS_System_GetConfig to assign the field value of it.
You should read this functions for more information.
*/
static MMPS_SYSTEM_CONFIG	m_systemConfig;

/**@brief The system operation mode.
         
Use @ref MMPS_System_SetOperationMode to set it.
And use @ref MMPS_System_GetOperationMode to get the current mode.
You should read these functions for more information.
*/
static MMPS_SYSTEM_OPMODE	m_systemOpMode = MMPS_SYSTEM_OPMODE_ACTIVE;

/**@brief The system application mode.
         
Use @ref MMPS_System_SetApplicationMode to set it.
And use @ref MMPS_System_GetApplicationMode to get the current mode.
You should read these functions for more information.
*/
static MMPS_SYSTEM_APMODE	m_systemApMode = MMPS_SYSTEM_APMODE_NULL;

/**@brief Keep the audio frame start address, and the preview frame start address
         
Use @ref MMPS_System_SetAudioFrameStart to set it, and use 
@ref MMPS_System_GetAudioFrameStart to retrieve it
*/
static MMP_ULONG	m_ulAudioFrameStart;
static MMP_ULONG	m_ulPreviewFrameStart;

/**@brief Firmware bootup status, set to TRUE when lower priority tasks is ready.
*/
MMP_BOOL            gbSysBootComplete = MMP_FALSE;

/**@brief Firmware memory heap end address
*/
static MMP_ULONG    m_ulSysHeapEnd = 0xFFFFFFFF;

#if (AHC_FS_SDNEWCACHE)//#if 1//(FS_NEWBUFFER)
#if defined (ALL_FW)
extern FS_CACHE_INFO	SDCacheInfo;
#endif
#endif
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetConfig
//  Description :
//------------------------------------------------------------------------------
/** @brief The function gets the current system configuration for the host application

The function gets the current system configuration for reference by the host application. The current
configuration can be accessed from output parameter pointer. The function calls
MMPD_System_GetConfiguration to get the current settings from Host Device Driver Interface.

@return It return the pointer of the system configuration data structure.
*/
MMPS_SYSTEM_CONFIG *MMPS_System_GetConfig(void)
{
    return &m_systemConfig;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_ReadCoreID
//  Description :
//------------------------------------------------------------------------------
/** @brief Check the chip code ID

This function Get the code ID of the chip
@return It reports the status of the operation.
*/
MMP_UBYTE MMPS_System_ReadCoreID(void)
{
	return MMPD_System_ReadCoreID();
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetOperationMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function sets possible system operation mode

The function sets possible system operation mode defined in MMPS_System_OpMode by calling
MMPD_System_SetOperationMode. Three operation modes can be supported Bypass, Active,
Power Down. (Bypass <--> Active <--> Power Down)

@param[in] systemopmode the system operation mode
@return It reports the status of the operation.
*/
MMP_ERR MMPS_System_SetOperationMode(MMPS_SYSTEM_OPMODE opMode)
{
	if (m_systemOpMode == MMPS_SYSTEM_OPMODE_ACTIVE) {
		 if (opMode == MMPS_SYSTEM_OPMODE_SELFSLEEP) {
			 MMPD_System_SetPSMode(MMPD_SYSTEM_ENTER_PS);
			 MMPD_System_SetSelfSleepMode();
			 MMPD_System_SetPSMode(MMPD_SYSTEM_EXIT_PS);
		 }
	}
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetWakeUpEvent
//  Description :
//------------------------------------------------------------------------------
/** @brief The function is used to set wake-up event for self sleep mode.

The function is used to set wake-up event for self sleep mode.

@param[bEnable] bEnable : enable/disable the wake up event
@param[bEnable] event : wake up event that AIT suppot
@param[bEnable] piopin : used in GPIO wake up event, to select GPIO
@param[bEnable] polarity : used in GPIO wake up event, to set high level/low level wake up. 0 is for high level
@return It reports the status of the operation.
*/
MMP_ERR MMPS_System_SetWakeUpEvent(MMP_BOOL bEnable, MMPS_SYSTEM_WAKEUP_EVNET event, MMP_GPIO_PIN piopin, MMP_UBYTE polarity)
{
	return MMPD_System_SetWakeUpEvent(bEnable, event, piopin, polarity);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_ResetSystem
//  Description :
//------------------------------------------------------------------------------
/** @brief The function is used for system reset

The function is used for system reset
@return It reports the status of the operation.
*/
MMP_ERR MMPS_System_ResetSystem(void)
{
	return MMPD_System_ResetSystem();
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_AllocateFSBuffer
//  Description :
//------------------------------------------------------------------------------
/** 
@brief  Application-mode Memory Allocation
@param[in] apMode : application mode
@return MMPS_SYSTEM_SUCCESS
*/
MMP_ERR MMPS_System_AllocateFSBuffer(const MMPS_SYSTEM_APMODE apMode)
{
#if ((!defined(MBOOT_FW))||defined(FAT_BOOT)||defined(MBOOT_SRAM_FW))
//In order to use FS in bootloader add defined(MBOOT_SRAM_FW)

    #if (FS_INPUT_ENCODE == UCS2)
    MMP_BYTE	volume[32];
    #endif

    MMP_ULONG	buf_start, buf_size, cur_buf;
    MMP_ULONG   nameBufSize, paramBufSize, nandBufRequire, sfBufRequire, sfnandBufRequire;
    MMP_ULONG	maxFileCount, maxDirCount;

    #if defined(MBOOT_SRAM_FW)
    /*
    Total FS buffer in <MMPS_System_AllocateFSBuffer> is 247K,if dismiss (MMPS_System_SetAudioFrameStart)it becomes 55K
    reserved as 256K<0x00040000> now.
    */
    buf_start = 0x001D0000; //Bottom address of SRAM location
    buf_size  = 0x00040000; //20161116
    #else
    
    buf_start = MMPS_System_GetConfig()->ulStackMemoryStart + 
                MMPS_System_GetConfig()->ulStackMemorySize;
    buf_size  = MMPS_System_GetConfig()->ulStackMemorySize;
    #endif

    #if (FS_INPUT_ENCODE == UCS2)
    nameBufSize = 0x300;
    #elif (FS_INPUT_ENCODE == UTF8)
    nameBufSize = 0x200;
    #endif

    paramBufSize = 8*1024;

    cur_buf = buf_start;

    /* Allocate for Filename buffer */
   	cur_buf -= nameBufSize;  
   	cur_buf -= paramBufSize;
    MMPD_FS_SetFileNameDmaAddr(cur_buf, nameBufSize, cur_buf + nameBufSize, paramBufSize);

    /* Allocate for SD working buffer */
    cur_buf -= 512;
    MMPD_FS_SetSDTmpMemory(cur_buf, 512);

    #if (OMA_DRM_EN == 1)
    /* Allocate for DRM working buffer */
    cur_buf -= 16*12;
    MMPD_FS_SetDRMIVMemory(cur_buf, 16*12);
    #endif
    
    /* Allocate for SM working buffer */
    MMPD_FS_GetSMMEMAlloc(&nandBufRequire);
    cur_buf -= nandBufRequire;
    MMPD_FS_SetSMMemory(cur_buf, nandBufRequire);
    
    /* Allocate for SN<SPI Nand Flash> working buffer */
    MMPD_FS_GetSNMEMAlloc(&sfnandBufRequire);
   	cur_buf -= sfnandBufRequire;
    MMPD_FS_SetSNMemory(cur_buf, sfnandBufRequire);	

    /* Allocate for SF working buffer */
    cur_buf -= 32; // reserve 32-byte for read device ID
    MMPD_FS_GetSFMEMAlloc(cur_buf, &sfBufRequire);
    #if (DEBUG_UART_TO_FILE) && (DEBUG_UART_TO_SF)
    // Double SF Address for debug info
    sfBufRequire *= 2;
    #endif
    
	if (sfBufRequire != 0) {
		cur_buf -= sfBufRequire;
		MMPD_FS_SetSFMemory(cur_buf, sfBufRequire);
	}

	#if (AHC_FS_SDNEWCACHE)//(FS_NEWBUFFER) //allocate working buffer for NewBuffer as 2nd layer cache	
    #if defined (ALL_FW)
    //SDBuff register & initial
	/*
	bResult = MMPD_FS_RegCacheType((MMP_ULONG)&FS_CacheType,(FS_CACHE_INFO *)ulCacheType);
	if(bResult == MMP_ERR_NONE){
		MMPD_FS_InitCacheNum(8);
		MMPD_FS_InitCacheSize(32 * 1024);
		MMPD_FS_InitCacheSectorSize(FS_MAX_SECTOR_SIZE);
		MMPD_FS_RegCacheMedia((MMP_ULONG)&FS_CacheMedia);
		MMPD_FS_RegCacheData((MMP_ULONG)&FS_CacheData);
		
		if( (pNewCacheInfo->CacheNumber) && (pNewCacheInfo->CacheSize) )	{
			for(bNumber = 0;bNumber < pNewCacheInfo->CacheNumber;bNumber++) {	
				cur_buf = cur_buf - pNewCacheInfo->CacheSize;	
				MMPD_FS_SetCacheAddr(cur_buf,bNumber,(MMP_ULONG)pNewCacheInfo);
			}	
			NewCache_Initialize(); 
		}	
	}
	*/
	
	//cur_buf = cur_buf - (8*32*1024);	
	cur_buf = cur_buf - ((SDCacheInfo.CacheNumber)*(SDCacheInfo.CacheSize));	
	SDCacheInfo.pf_SetCacheStartAddrSize(&SDCacheInfo,cur_buf,SDCacheInfo.CacheTotalSize);
	SDCacheInfo.pf_SetCacheNumSize(&SDCacheInfo,SDCacheInfo.CacheNumber,SDCacheInfo.CacheSize);		
	SDCacheInfo.pf_SetCacheSectorSize(&SDCacheInfo,FS_MAX_SECTOR_SIZE);	
	SDCacheInfo.pf_RegCacheMedia(&SDCacheInfo,&FS_MMC_CardMode_Driver);	
	SDCacheInfo.pf_RegCacheSelf(&SDCacheInfo,&FS_MMC_CardMode_SDCacheDriver);	
	SDCacheInfo.pf_RegCacheObject(&SDCacheInfo,&SDCacheInfo);	
	//SDCacheInfo.pf_InitCacheSectorSize(&SDCacheInfo,FS_MAX_SECTOR_SIZE);
	//NewCache_Initialize(SDCacheInfo); 


	
	
	/*
    SDBufferInfo.BuffNumber = 8;
    SDBufferInfo.BuffSize = 32 * 1024;
    SDBufferInfo.pSelf = &FS_MMC_CardMode_SDBufDriver;
	SDBufferInfo.pBufData = &SD_Buffer;	
	if( (SDBufferInfo.BuffNumber) && (SDBufferInfo.BuffSize) )	{
		for(bNumber = 0;bNumber < SDBufferInfo.BuffNumber;bNumber++) {	
			cur_buf = cur_buf - SDBufferInfo.BuffSize;	
			MMPD_FS_SetBufAddr(cur_buf,bNumber,(MMP_ULONG)&SDBufferInfo);
		}	
	}
	*/	
	#endif
	#endif
	
	/* Allocate for File System/Audio buffer */
	MMPD_FS_GetMaxFileOpenSupport(&maxFileCount);
	MMPD_FS_GetMaxDirOpenSupport(&maxDirCount);	
	MMPD_FS_SetFATGlobalBufAddr(cur_buf - 8*1024 - (maxFileCount + maxDirCount)*1024, 8*1024);
    #if defined(ALL_FW)
    printc("file system memory alocate start: 0x%08x, End : 0x%08x\r\n", buf_start, (cur_buf - 
                                                   8*1024 - (maxFileCount + maxDirCount)*1024));
    #endif
    MMPS_FS_Initialize();

    cur_buf = cur_buf - 8*1024 - (maxFileCount + maxDirCount)*1024;
   	MMPS_System_SetAudioFrameStart(&cur_buf);
   	// Caution : Don`t set memory heap end at here, the value of cur_buf is not that what we want and may be larger than GUI base address
    //MMPS_System_SetMemHeapEnd(cur_buf); 

	gbSysBootComplete = MMP_TRUE;

	#if (FS_INPUT_ENCODE == UCS2)
	uniStrcpy(volume, (const char *)L"SD:\\");
	MMPS_FS_MountVolume(volume, uniStrlen((const short *)volume));
	uniStrcpy(volume, (const char *)L"SM:\\");
	MMPS_FS_MountVolume(volume, uniStrlen((const short *)volume));
	#elif (FS_INPUT_ENCODE == UTF8)
	#if USING_SD_IF
	MMPS_FS_MountVolume_Ex("SD:\\", STRLEN("SD:\\"));
	#endif
    #if defined(ALL_FW)
    #if USING_SD1_IF 
    MMPS_FS_MountVolume_Ex("SD1:\\", STRLEN("SD1:\\"));
    #endif
    #endif
	#if USING_SF_IF
	MMPS_FS_MountVolume("SF:\\", STRLEN("SF:\\"));
	#endif
	#if USING_SM_IF
	MMPS_FS_MountVolume("SM:\\", STRLEN("SM:\\"));
	#endif
	#endif
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_SendEchoCommand
//  Description :
//------------------------------------------------------------------------------
/** @brief The function will check current host interface workable

@return The result of the current host command interface is work or not.
*/
MMP_ERR MMPS_System_SendEchoCommand(void)
{
	if (m_systemApMode != MMPS_SYSTEM_APMODE_NULL) {
		return MMPD_System_SendEchoCommand();
	}
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetPreviewFrameStart
//  Description :
//------------------------------------------------------------------------------
/** 
@brief  Return the frame end address
@return MMPS_SYSTEM_SUCCESS
*/
MMP_ERR MMPS_System_GetPreviewFrameStart(MMP_ULONG *ulFBAddr)
{
	*ulFBAddr = m_ulPreviewFrameStart;
	
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetAudioFrameStart
//  Description :
//------------------------------------------------------------------------------
/** 
@brief  Keep the frame end address
@return MMPS_SYSTEM_SUCCESS
*/
MMP_ERR	MMPS_System_SetAudioFrameStart(MMP_ULONG *ulFBEndAddr)
{
	m_ulAudioFrameStart = *ulFBEndAddr;
    *ulFBEndAddr        = *ulFBEndAddr -
                            (MMPS_System_GetConfig()->ulMaxReserveBufferSize *
                            MMPS_System_GetConfig()->ulMaxReserveChannel);
	m_ulPreviewFrameStart = *ulFBEndAddr;

    #if defined(ALL_FW)
    printc("Audio memory alocate start: 0x%X, End: 0x%X\r\n",
            *ulFBEndAddr, m_ulAudioFrameStart);
    #endif

	return	MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetAudioFrameStart
//  Description :
//------------------------------------------------------------------------------
/** 
@brief  Return the frame end address
@return MMPS_SYSTEM_SUCCESS
*/
MMP_ERR MMPS_System_GetAudioFrameStart(MMP_ULONG audio_id, MMP_ULONG *ulFBEndAddr)
{
	 *ulFBEndAddr = m_ulAudioFrameStart - (audio_id * MMPS_System_GetConfig()->ulMaxReserveBufferSize);
	
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetGroupSrcAndDiv
//  Description :
//------------------------------------------------------------------------------
/** @brief Get the group PLL Source.
@param[in]  ubGroup Select the group.
@param[out] ulGroupFreq The current group PLL Source.
@return Always return success.
*/
MMP_ERR MMPS_System_GetGroupSrcAndDiv(CLK_GRP ubGroup, MMP_ULONG* ulGroupSrc, MMP_UBYTE* ubPllDiv)
{
    return MMPD_System_GetGroupSrcAndDiv(ubGroup, ulGroupSrc, ubPllDiv);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetGroupFreq
//  Description :
//------------------------------------------------------------------------------
/** @brief Get the group freq.
@param[in]  ubGroup Select the group.
@param[out] ulGroupFreq The current group freq.
@return Always return success.
*/
MMP_ERR MMPS_System_GetGroupFreq(CLK_GRP ubGroup, MMP_ULONG *ulGroupFreq)
{
	return MMPD_System_GetGroupFreq(ubGroup, ulGroupFreq);
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetAudioGroupFract
//  Description :
//------------------------------------------------------------------------------
/** @brief Set the fraction part of audio PLL. Each project and each
           sensor mode may have different frame rate inaccuracy.
           To achieve a native A/V sync, we allow application to specify the
           fraction part of audio PLL by this function.
@param[in]  ulFract Fraction part of audio PLL. Max: 0x3FFFF for MCR_V2
@return Always return success.
*/
MMP_ERR MMPS_System_SetAudioGroupFract(MMP_LONG ulFract)
{
	return MMPD_System_SetAudioGroupFract(ulFract);
}

#if 0
void ____MemPool_Function____(){ruturn;} //dummy
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetMemHeapEnd
//  Description :
//------------------------------------------------------------------------------
/** @brief Set the end address of memory heap, kernel feature should not
           allocate memory with address over than this end address.
@param[in]  ulHeapEnd The end address of heap.
@return It reports the status of the operation.
*/
void MMPS_System_SetMemHeapEnd(MMP_ULONG ulHeapEnd)
{
    m_ulSysHeapEnd = ulHeapEnd;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetMemHeapEnd
//  Description :
//------------------------------------------------------------------------------
/** @brief Get the end address of memory heap, kernel feature must check if the
           address of allocated memory is larger than this end address.
@return The end address of memory heap.
*/
MMP_ULONG MMPS_System_GetMemHeapEnd(void)
{
    return m_ulSysHeapEnd;
}

#if (EN_AUTO_TEST_LOG == 1)
//------------------------------------------------------------------------------
//  Function    : MMPS_System_ClrAutoTestLog
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_System_ClrAutoTestLog(void) 
{
    return MMPD_System_ClrAutoTestLog();
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_DumpAutoTestLog
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_System_DumpAutoTestLog(void) 
{
    return MMPD_System_DumpAutoTestLog();
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetUserDefCmdNum
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_System_SetUserDefCmdNum(MMP_LONG ulUserDefNum) 
{
    return MMPD_System_SetUserDefCmdNum(ulUserDefNum);
}
#endif

/// @}