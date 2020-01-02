//==============================================================================
//
//  File        : JobDispatch.c
//  Description : ce job dispatch function
//  Author      : Robin Feng
//  Revision    : 1.0
//
//==============================================================================

#include "Customer_Config.h"

#include "mmp_err.h"
#include "mmpf_typedef.h"
#include "lib_retina.h"
#include "mmpf_storage_api.h"
#include "mmpf_pio.h"

//==============================================================================
//
//                              COMPILING OPTIONS
//
//==============================================================================

#define BOOT_FROM_SD        (0)
#define BOOT_FROM_NAND      (0)
#define BOOT_FROM_SF        (1)

#if (BOOT_FROM_SF)
#include "mmpf_sf.h"
#endif

#define JTAG_EN             (0)

//==============================================================================
//
//                              CONSTANT
//
//==============================================================================

#define AIT_FW_TEMP_BUFFER_ADDR  0x106000
#define AIT_BOOT_HEADER_ADDR     0x106200
#define AIT_FW_START_ADDR        0x1000000
#define AIT_BOOT_START_ADDR      0x122000

#define STOREFIRMWARE_SD_ID     (0)

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
void Customer_Init(void)
{
#ifdef POWER_EN_GPIO
	if (POWER_EN_GPIO != MMP_GPIO_MAX) {
		MMPF_PIO_EnableOutputMode(POWER_EN_GPIO, MMP_TRUE, MMP_FALSE);
		#ifdef POWER_EN_GPIO_LEVEL
		MMPF_PIO_SetData(POWER_EN_GPIO, POWER_EN_GPIO_LEVEL, MMP_FALSE);
		#else
		MMPF_PIO_SetData(POWER_EN_GPIO, GPIO_HIGH, MMP_FALSE);
		#endif
	}
#endif
}

#define STORAGE_TEMP_BUFFER 0x106000
extern int SF_Module_Init(void);

void CE_JOB_DISPATCH_Task(void *p_arg)
{
    void (*FW_Entry)(void) = NULL;
    #if (BOOT_FROM_SD)
    extern stSDMMCHandler m_SDMMCArg[MMPF_SD_DEV_NUM];
    #if (CHIP == MCR_V2)
    AIT_STORAGE_INDEX_TABLE2 *pIndexTable = (AIT_STORAGE_INDEX_TABLE2 *)AIT_BOOT_HEADER_ADDR;
    MMP_ULONG   ulTableAddr;
    #endif
    stSDMMCHandler  *pSDHandle;
    #endif // (BOOT_FROM_SD)
	#if (BOOT_FROM_SF == 1)
	AIT_STORAGE_INDEX_TABLE2 *pIndexTable = (AIT_STORAGE_INDEX_TABLE2 *)STORAGE_TEMP_BUFFER;
    MMP_ULONG   ulSifAddr = 0x0;
	MMP_USHORT  usCodeCrcValue = 0x0;
	MMP_ERR     err;
	#endif// (BOOT_FROM_SF == 1)

    RTNA_DBG_Str0("MiniBoot Job Task!\r\n");

	#if (JTAG_EN)
    while(1);		// For ICE Debug
    #endif
    
	/*Download firmware from moviNAND*/
	#if (BOOT_FROM_SD)
	/*moviNAND reset*/
    pSDHandle = m_SDMMCArg+STOREFIRMWARE_SD_ID;
	MMPF_SD_InitHandler();
	MMPF_SD_InitialInterface(pSDHandle);
	MMPF_SD_SetTmpAddr(AIT_FW_TEMP_BUFFER_ADDR, 0);
	
	if(MMPF_SD_Reset(pSDHandle)) {
		MMPF_MMC_Reset(pSDHandle);
	}
	RTNA_DBG_Str(0,"Start load moviNAND code\r\n");

	/*read boot area partition table*/
	MMPF_SD_ReadSector(pSDHandle, AIT_BOOT_HEADER_ADDR, 0, 1);

    #if (CHIP == MCR_V2)
    ulTableAddr = pIndexTable->ulTotalSectorsInLayer;
    MMPF_SD_ReadSector(pSDHandle, AIT_BOOT_HEADER_ADDR, ulTableAddr, 1);

    /* read boot code */
    if ((pIndexTable->ulHeader == INDEX_TABLE_HEADER) && 
		(pIndexTable->ulTail == INDEX_TABLE_TAIL) &&
		(pIndexTable->ulFlag & 0x1))
    {
        MMPF_SD_ReadSector(pSDHandle, pIndexTable->bt.ulDownloadDstAddress, 
                            pIndexTable->bt.ulStartBlock,
                            (pIndexTable->bt.ulCodeSize + 511) >>
                            pIndexTable->ulAlignedBlockSizeShift);
        FW_Entry = (void (*)(void))(pIndexTable->bt.ulDownloadDstAddress);
        RTNA_DBG_PrintLong(0, pIndexTable->bt.ulDownloadDstAddress);
        RTNA_DBG_Str(0,"End load moviNAND code\r\n");
    }
    else {
		RTNA_DBG_PrintLong(0, pIndexTable->ulHeader);
		RTNA_DBG_PrintLong(0, pIndexTable->ulTail);
		RTNA_DBG_PrintLong(0, pIndexTable->ulFlag);
		RTNA_DBG_Str(0, "Invalid Index Table!!\r\n");
		while(1);
	}
    #endif //(CHIP == MCR_V2)
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
	#if 0//CZ patch...20160204
	while(1) {
		MMPF_SF_ReadData(ulSifAddr, (STORAGE_TEMP_BUFFER), 512);  //AitBootloader Header Table
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
	MMPF_SF_ReadData(ulSifAddr, (STORAGE_TEMP_BUFFER), 512);  //AitBootloader Header Table
	#endif
	if ((pIndexTable->ulHeader == INDEX_TABLE_HEADER) && 
		(pIndexTable->ulTail == INDEX_TABLE_TAIL) &&
		(pIndexTable->ulFlag & 0x1))
    {
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
			RTNA_DBG_Str(0, "SIF downlod End\r\n");
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
#if 0 //For debug
    {
        MMP_ULONG ulLoop=0;
        RTNA_DBG_Str(0, "CE_JOB_DISPATCH_Task mini_boot.\r\n");
        RTNA_DBG_PrintLong(0, AIT_FW_START_ADDR);
        for(ulLoop = 0; ulLoop < 16; ++ulLoop){
            RTNA_DBG_Long(0, *(MMP_UBYTE *)(AIT_FW_START_ADDR+ulLoop));    
            RTNA_DBG_Str(0, ",");            
        }
        RTNA_DBG_Str(0, "\r\n");
    }
#endif	
    /* Jump PC to AIT_FW_START_ADDR */
    FW_Entry = (void (*)(void))(AIT_FW_START_ADDR);
    FW_Entry();

    while (1) ;
}

