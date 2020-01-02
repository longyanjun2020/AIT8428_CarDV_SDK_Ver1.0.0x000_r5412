//==============================================================================
//
//  File        : AHC_ISP.c
//  Description : AHC ISP control function
//  Author      :
//  Revision    : 1.0
//
//==============================================================================

/*===========================================================================
 * Include files
 *===========================================================================*/

#include "AHC_ISP.h"
#include "isp_if.h"
#include "MenuSetting.h"
#include "mmps_sensor.h"
#include "mmps_fs.h"
#include "ISP_cfg.h"
#include "AHC_Config_Dummy.h"
#include "mmp_usb_inc.h"
#include "ait_utility.h"
/*===========================================================================
 * Macro Define
 *===========================================================================*/

#define ISP_DEBUG_OFF 	(0)
#define ISP_DEBUG_ON 	(1)
#define ISP_DEBUG 		ISP_DEBUG_OFF

/*===========================================================================
 * Global variable
 *===========================================================================*/

MMP_SHORT gsSharpnessCenter = SHARPNESS_CENTER;
MMP_SHORT gsGammaCenter    	= GAMMA_CENTER;
MMP_SHORT gsContrastCenter 	= CONTRAST_CENTER;
MMP_SHORT gsSaturateCenter 	= SATURATE_CENTER;

static AHC_BOOL     m_bEnableFastAeAwb      = AHC_FALSE;

/*===========================================================================
 * Main body
 *===========================================================================*/
#if  (defined(SUPPORT_ISP_CALIBRATION) && (SUPPORT_ISP_CALIBRATION == 1))			

#define ISP_CALI_PARTITION          "1"
#define ISP_CALI_FILENAME_PREFIX    "ISP_Calibration_"
#define ISP_CALI_FILENAME_EXT       ".bin"
#define ISP_CALI_FILE_NUM           2

#define ISP_CALI_HEADER 1
//#define	COLOR_TEMP 5100

#define	LUX 800 //don't care
#define AWB_CALI_TABLE_SIZE 8
#define LS_CALI_TABLE_SIZE ((2*8)+(256*2*3))
#define CS_CALI_TABLE_SIZE ((2*7)+(41*2)+(31*2)+(42*32*2*3))

#define CALI_DATA_SIZE (28 + AWB_CALI_TABLE_SIZE+LS_CALI_TABLE_SIZE+CS_CALI_TABLE_SIZE)
//#define CALI_DATA_SIZE 0x2660 //0x265C used, 9824, 9 byte:migic word, 8byte:AWB, Luma table: 1552 byte, Chroma:8222 byte,....
    
ISP_UINT8 CaliDataBuf[CALI_DATA_SIZE+1]; //LAB
ISP_UINT8* CaliData; //LAB        
MMP_USHORT temp_cali[10]; //LAB
MMP_USHORT m_usISPTempDataSize=CALI_DATA_SIZE; //calibration data size for one temperature
//------------------------------------------------------------------------------
//  Function    : AHC_WriteISPCalibData2SIF_CB
//  Description :
//------------------------------------------------------------------------------

extern MMP_ULONG glUvcIqTblBinSz;
extern MMP_UBYTE *gpUvcIqTblBinPtr;
extern MMP_UBYTE gubISPCaliIndex;

MMP_BYTE bISPFilePath[MAX_FILE_NAME_SIZE]={0};
void AHC_WriteISPCalibData2SIF_CB(MMP_USB_EVENT event)
{
#if defined(SUPPORT_UVC_ISP_EZMODE_FUNC) && (SUPPORT_UVC_ISP_EZMODE_FUNC) 
	MMP_ULONG   ulFileId;
	MMP_ULONG   ulActualSize;
	INT8       StrIdx[4];

	printc("AHC_WriteISPCalibData2SIF_CB\r\n");

    MEMSET(bISPFilePath, 0, sizeof(bISPFilePath));
    
	STRCPY(bISPFilePath, "SF:");
	STRCAT(bISPFilePath, ISP_CALI_PARTITION);
    STRCAT(bISPFilePath,":\\");
    STRCAT(bISPFilePath, ISP_CALI_FILENAME_PREFIX);
    AHC_UTILITY_Int2Str(gubISPCaliIndex, StrIdx);
    STRCAT(bISPFilePath, StrIdx);
    STRCAT(bISPFilePath, ISP_CALI_FILENAME_EXT);
         
    printc("Write File name:%s  into SIF\r\n", bISPFilePath);
    
    if(MMP_ERR_NONE != MMPS_FS_FileOpen(bISPFilePath, strlen(bISPFilePath), "w+b", strlen("w+b"), &ulFileId)) {
        printc("AHC_WriteISPCalibData2SIF_CB  MMPS_FS_FileOpen err\r\n");
        MMPS_FS_FileClose(ulFileId);
        return;
    }
    
    printc("WR index %d size 0x%x  from DRAM addr 0x%x to SIF \r\n", gubISPCaliIndex, glUvcIqTblBinSz, gpUvcIqTblBinPtr);
	MMPS_FS_FileWrite(ulFileId, gpUvcIqTblBinPtr, glUvcIqTblBinSz, &ulActualSize);
	MMPS_FS_FileClose(ulFileId);
    #endif	   
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetISPTmpDataSize
//  Description :
//------------------------------------------------------------------------------
MMP_USHORT AHC_GetISPTmpDataSize(void)
{
    return m_usISPTempDataSize;
}

//------------------------------------------------------------------------------
//  Function    : AHC_ReadISPTmpData
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_ReadISPTmpData(void)
{
	MMP_ERR 	err = MMP_ERR_NONE;
    MMP_ULONG   ulFileId;
    MMP_USHORT  usCalibISPDataSize;
    MMP_ULONG64 datasize;
    MMP_ULONG   read_out_bytes;
    MMP_UBYTE   isp_cali_header[10] ="ISP_CALI";
    MMP_ULONG   HeaderSize, ISPdataSize, ColorTemp, lux, sensorid, Offset=0;
    MMP_USHORT  index;
    INT8        StrIdx[4];
    INT8        i=0;

    for(i=0 ; i < ISP_CALI_FILE_NUM ;i++) {
        
        MEMSET(bISPFilePath, 0, sizeof(bISPFilePath));
    	STRCPY(bISPFilePath, "SF:");
    	STRCAT(bISPFilePath, ISP_CALI_PARTITION);
        STRCAT(bISPFilePath,":\\");
        STRCAT(bISPFilePath, ISP_CALI_FILENAME_PREFIX);
        AHC_UTILITY_Int2Str(i, StrIdx);
        STRCAT(bISPFilePath, StrIdx);
        STRCAT(bISPFilePath, ISP_CALI_FILENAME_EXT);

        usCalibISPDataSize= AHC_GetISPTmpDataSize();
        //load temp calibration data into ISP lib
        err = MMPS_FS_FileOpen(bISPFilePath, strlen(bISPFilePath), "rb", strlen("rb"), &ulFileId);
        if (err != MMP_ERR_NONE) {
            RTNA_DBG_Str(0, "ISP Calibration data open failed!!\r\n");
            MMPS_FS_FileClose(ulFileId);
            continue;
        }
        else
        {
        	// file size check
			MMPS_FS_FileGetSize(ulFileId, &datasize);
			if(datasize != usCalibISPDataSize){
				MMPS_FS_FileClose(ulFileId);
				RTNA_DBG_Str(0, "ISP Calibration data size mismatch!!\r\n");
				continue;
			}
			memset(CaliDataBuf, 0, usCalibISPDataSize);
			MMPS_FS_FileRead(ulFileId, CaliDataBuf, datasize, &read_out_bytes);
			MMPS_FS_FileClose(ulFileId);
        }


        CaliData = CaliDataBuf;
        
        if(memcmp(CaliData, isp_cali_header, 8) != 0){
           RTNA_DBG_Str(0, "ISP Calibration Data Header error!!!\r\n");           
        } 
        else {
            Offset=8;
            HeaderSize      =   *(MMP_ULONG *)(CaliData + Offset);
            Offset += 4;
            ISPdataSize     =   *(MMP_ULONG *)(CaliData + Offset);
            Offset += 4;
            ColorTemp      =   *(MMP_ULONG *)(CaliData + Offset);
            Offset += 4;
            lux             =   *(MMP_ULONG *)(CaliData + Offset);
            Offset += 4;
            sensorid        =   *(MMP_ULONG *)(CaliData + Offset);
            Offset += 4;
 
            if(ColorTemp == 3000) {
                index=1;
                AHC_GetISPCalibData(CaliDataBuf, datasize, ColorTemp, index);
            }
            else if (ColorTemp == 5000) {
                index=0;
                AHC_GetISPCalibData(CaliDataBuf, datasize, ColorTemp, index);
                index=2;
                AHC_GetISPCalibData(CaliDataBuf, datasize, ColorTemp, index);
            }   
			printc("Load ISP Calibration file %s done!!\r\n",bISPFilePath);    
        } 
    }

    #if 0
    //load temp calibration data into ISP lib
    if(MMP_ERR_NONE != MMPS_FS_FileOpen(ISP_CALI_1_FILE_NAME, strlen(ISP_CALI_1_FILE_NAME), "rb", strlen("rb"), &ulFileId))
        return AHC_FALSE; 
    // file size check 
    MMPS_FS_FileGetSize(ulFileId, &datasize);    
    if(datasize != usCalibISPDataSize){
        RTNA_DBG_Str(0, "ISP Calibration data size mismatch!!\r\n");
        return AHC_FALSE; 
    }
    memset(CaliDataBuf, 0, usCalibISPDataSize);
    MMPS_FS_FileRead(ulFileId, CaliDataBuf, datasize, &read_out_bytes);
    #if 1
    CaliData = CaliDataBuf;
    if(memcmp(CaliData, isp_cali_header, 8) != 0){
       RTNA_DBG_Str(0, "ISP Calibration Data Header error!!!\r\n");
       return AHC_FALSE;
    } 
    Offset=8;
    HeaderSize      =   *(MMP_ULONG *)(CaliData + Offset);
    Offset += 4;
    ISPdataSize     =   *(MMP_ULONG *)(CaliData + Offset);
    Offset += 4;
    ColorTemp      =   *(MMP_ULONG *)(CaliData + Offset);
    Offset += 4;
    lux             =   *(MMP_ULONG *)(CaliData + Offset);
    Offset += 4;
    sensorid        =   *(MMP_ULONG *)(CaliData + Offset);
    Offset += 4;
    #endif
    
    if(ColorTemp == 3000) {
        index=1;
        AHC_GetISPCalibData(CaliDataBuf, datasize, ColorTemp, index);
    }
    else if (ColorTemp == 5000) {
        index=0;
        AHC_GetISPCalibData(CaliDataBuf, datasize, ColorTemp, index);
        index=2;
        AHC_GetISPCalibData(CaliDataBuf, datasize, ColorTemp, index);
    }

    MMPS_FS_FileClose(ulFileId);
    #endif
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_ReadISPAWBData
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_ReadISP_AWBData(void)
{
    MMP_ULONG   ulFileId;
    MMP_USHORT  usCalibISPDataSize;
    MMP_ULONG64 datasize;
    MMP_ULONG   read_out_bytes;
    MMP_UBYTE   isp_cali_header[10] ="ISP_CALI";
    MMP_ULONG   ColorTemp, Offset = 0;
    MMP_ULONG   sensorid = 0;
    INT8        StrIdx[4];
    INT8        i = 0;

    for (i = 0; i < ISP_CALI_FILE_NUM; i++) {
    
        MEMSET(bISPFilePath, 0, sizeof(bISPFilePath));
        STRCPY(bISPFilePath, "SF:");
        STRCAT(bISPFilePath, ISP_CALI_PARTITION);
        STRCAT(bISPFilePath,":\\");
        STRCAT(bISPFilePath, ISP_CALI_FILENAME_PREFIX);
        AHC_UTILITY_Int2Str(i, StrIdx);
        STRCAT(bISPFilePath, StrIdx);
        STRCAT(bISPFilePath, ISP_CALI_FILENAME_EXT);

        usCalibISPDataSize= AHC_GetISPTmpDataSize();
        
        // load temp calibration data into ISP lib
        if (MMP_ERR_NONE != MMPS_FS_FileOpen(bISPFilePath, strlen(bISPFilePath), "rb", strlen("rb"), &ulFileId)) {
            RTNA_DBG_Str(0, "ISP Calibration data open failed!!\r\n");
            MMPS_FS_FileClose(ulFileId);
            continue;
        }
        else
        {
			// file size check
			MMPS_FS_FileGetSize(ulFileId, &datasize);
			if (datasize != usCalibISPDataSize){
				MMPS_FS_FileClose(ulFileId);
				RTNA_DBG_Str(0, "ISP Calibration data size mismatch!!\r\n");
				continue;
			}

			memset(CaliDataBuf, 0, usCalibISPDataSize);
			MMPS_FS_FileRead(ulFileId, CaliDataBuf, datasize, &read_out_bytes);
			MMPS_FS_FileClose(ulFileId);
        }

        CaliData = CaliDataBuf;
        
        if (memcmp(CaliData, isp_cali_header, 8) != 0){
           RTNA_DBG_Str(0, "ISP Calibration Data Header error!!!\r\n");
        } 
        else {
        	Offset = 16;
        	ColorTemp      =   *(MMP_ULONG *)(CaliData + Offset);
            Offset = 24;
            sensorid  =   *(MMP_ULONG *)(CaliData + Offset);

            if (ColorTemp == 5000)
            {
            	AHC_GetISP_AWB_CalibData(CaliDataBuf, (MMP_UBYTE)sensorid);
            	printc("Load ISP Calibration AWB file %s done!!\r\n",bISPFilePath);
            }
        }
    }

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_GetISPCalibData
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_GetISPCalibData(UINT8 *buf, UINT16 bufsize, UINT16 color_temp, UINT16 index)
{  	
        MMP_UBYTE isp_cali_header[10] ="ISP_CALI";
        MMP_USHORT scale;
        MMP_USHORT gain_r, gain_gr, gain_gb, gain_b;
        MMP_USHORT base_x, base_y, offset_x, offset_y, center_x, center_y, rate_x, rate_y;
        MMP_USHORT *table_r, *table_g, *table_b, *pos_x, *pos_y;
        MMP_USHORT Offset = 0;
            
        CaliData = buf;
    
        //AWB ============================================   
        if(memcmp(CaliData, isp_cali_header, 8) != 0){
            RTNA_DBG_Str(3, "Not Support ISP Calibration Function!!!\r\n");
            return AHC_FALSE;
        } 
        Offset = 28;
        gain_r  = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        gain_gr = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        gain_gb = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        gain_b  = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
    
        temp_cali[0] = gain_r;  
        temp_cali[1] = gain_gr; 
        temp_cali[2] = gain_b;          
        ISP_IF_CALI_SetAWB( index,
                            gain_r, gain_gr, gain_gr, gain_b,
                            color_temp, LUX);    
        //LS =============================================
        base_x  = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        base_y  = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        offset_x= *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        offset_y= *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        center_x= *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        center_y= *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        rate_x  = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        rate_y  = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        table_r = (MMP_USHORT *)(CaliData + Offset);
        Offset += 256 * 2;
        table_g = (MMP_USHORT *)(CaliData + Offset);
        Offset += 256 * 2;
        table_b = (MMP_USHORT *)(CaliData + Offset);
        Offset += 256 * 2;
                
        ISP_IF_CALI_SetLS(  index,
                          base_x, base_y,
                          center_x, center_y,
                          offset_x, offset_y,
                         rate_x, rate_y,
                          (MMP_USHORT *)table_r, (MMP_USHORT *)table_g, (MMP_USHORT *)table_b,
                          color_temp, LUX);                
        //CS =============================================
        base_x  = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        base_y  = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        offset_x= *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        offset_y= *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        center_x= *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        center_y= *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        scale   = *(MMP_USHORT *)(CaliData + Offset);
        Offset += 2;
        pos_x   = (MMP_USHORT *)(CaliData + Offset);
        Offset += 41 * 2;
        pos_y   = (MMP_USHORT *)(CaliData + Offset);
        Offset += 31 * 2;
        table_r = (MMP_USHORT *)(CaliData + Offset);
        Offset += 42 * 32 * 2;
        table_g = (MMP_USHORT *)(CaliData + Offset);
        Offset += 42 * 32 * 2;
        table_b = (MMP_USHORT *)(CaliData + Offset);
        Offset += 42 * 32 * 2;
    
        temp_cali[3] = pos_x[38];
        temp_cali[4] = base_y;          
        temp_cali[5] = scale;   
        temp_cali[6] = offset_y;    
          
        ISP_IF_CALI_SetCS(  index,
                            base_x, base_y,
                            center_x, center_y,
                            offset_x, offset_y,
                            scale,
                            (MMP_USHORT *)pos_x, (MMP_USHORT *)pos_y,
                            (MMP_USHORT *)table_r, (MMP_USHORT *)table_g, (MMP_USHORT *)table_b,
                            color_temp, LUX);
     return AHC_TRUE; 
}

#if 1//(INSTA360_DEMO_EN)
//------------------------------------------------------------------------------
//  Function    : AHC_GetISP_AWB_CalibData
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_GetISP_AWB_CalibData(UINT8 *buf, UINT8 sensorid)
{
    MMP_UBYTE isp_cali_header[10] ="ISP_CALI";
    MMP_USHORT gain_r, gain_gr, gain_gb, gain_b;
    MMP_USHORT Offset = 0;

    CaliData = buf;
    //AWB ============================================
    if (memcmp(CaliData, isp_cali_header, 8) != 0){
        RTNA_DBG_Str(3, "Not Support ISP Calibration Function!!!\r\n");
        return AHC_FALSE;
    }

    Offset = 28;
    gain_r  = *(MMP_USHORT *)(CaliData + Offset);
    Offset += 2;
    gain_gr = *(MMP_USHORT *)(CaliData + Offset);
    Offset += 2;
    gain_gb = *(MMP_USHORT *)(CaliData + Offset);
    Offset += 2;
    gain_b  = *(MMP_USHORT *)(CaliData + Offset);

    temp_cali[0] = gain_r << 2;
    temp_cali[1] = gain_gr << 2;
    temp_cali[2] = gain_b << 2;
    printc("sensorid %d R %x G %x B %x\r\n", sensorid, temp_cali[0], temp_cali[1], temp_cali[2]);

//    printc("Snr %d R 0x%x  0x%x\r\n", sensorid, temp_cali[0] >> 8, temp_cali[0] & 0xFF);
    gsSensorFunction->MMPF_Sensor_SetReg((MMP_UBYTE)sensorid, 0x500C, temp_cali[0] >> 8);
    gsSensorFunction->MMPF_Sensor_SetReg((MMP_UBYTE)sensorid, 0x500D, temp_cali[0] & 0xFF);
//    printc("Snr %d G 0x%x  0x%x\r\n", sensorid, temp_cali[1] >> 8, temp_cali[1] & 0xFF);
    gsSensorFunction->MMPF_Sensor_SetReg((MMP_UBYTE)sensorid, 0x500E, temp_cali[1] >> 8);
    gsSensorFunction->MMPF_Sensor_SetReg((MMP_UBYTE)sensorid, 0x500F, temp_cali[1] & 0xFF);
//    printc("Snr %d B 0x%x  0x%x\r\n", sensorid, temp_cali[2] >> 8, temp_cali[2] & 0xFF);
    gsSensorFunction->MMPF_Sensor_SetReg((MMP_UBYTE)sensorid, 0x5010, temp_cali[2] >> 8);
    gsSensorFunction->MMPF_Sensor_SetReg((MMP_UBYTE)sensorid, 0x5011, temp_cali[2] & 0xFF);

    return AHC_TRUE;
}
#endif
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_TransAWBMenu2ISPSetting
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_TransAWBMenu2ISPSetting(UINT8 ubMenuItem, UINT8 *ISPItem)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_TransAWBMenu2ISPSetting ubMenuItem=%d\r\n", ubMenuItem);
    #endif

	switch (ubMenuItem)
	{
	#if (MENU_MANUAL_WB_AUTO_EN)
		case WB_AUTO: 			*ISPItem = AHC_AWB_MODE_AUTO; 					break;
	#endif
	#if (MENU_MANUAL_WB_DAYLIGHT_EN)
		case WB_DAYLIGHT: 		*ISPItem = AHC_AWB_MODE_DAYLIGHT; 				break;
	#endif
	#if (MENU_MANUAL_WB_CLOUDY_EN)
		case WB_CLOUDY: 		*ISPItem = AHC_AWB_MODE_CLOUDY; 				break;
	#endif
	#if (MENU_MANUAL_WB_FLUORESCENT1_EN)
		case WB_FLUORESCENT1: 	*ISPItem = AHC_AWB_MODE_FLUORESCENT_WHITE; 		break;
	#endif
	#if (MENU_MANUAL_WB_FLUORESCENT2_EN)
		case WB_FLUORESCENT2: 	*ISPItem = AHC_AWB_MODE_FLUORESCENT_NATURAL; 	break;
	#endif
	#if (MENU_MANUAL_WB_FLUORESCENT3_EN)
		case WB_FLUORESCENT3: 	*ISPItem = AHC_AWB_MODE_FLUORESCENT_DAYLIGHT; 	break;
	#endif
	#if (MENU_MANUAL_WB_INCANDESCENT_EN)
		case WB_INCANDESCENT: 	*ISPItem = AHC_AWB_MODE_INCANDESCENT; 			break;
	#endif
	#if (MENU_MANUAL_WB_FLASH_EN)
		case WB_FLASH: 			*ISPItem = AHC_AWB_MODE_FLASH; 					break;
	#endif
	#if (MENU_MANUAL_WB_ONEPUSH_EN)
		case WB_ONEPUSH: 		*ISPItem = AHC_AWB_MODE_ONEPUSH; 				break;
	#endif
	#if (MENU_MANUAL_WB_ONE_PUSH_SET_EN)
		case WB_ONE_PUSH_SET: 	*ISPItem = AHC_AWB_MODE_ONEPUSH; 				break;
	#endif
		default: 				*ISPItem = AHC_AWB_MODE_AUTO; 					break;
	}

	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAwbFastMode
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetAwbFastMode(UINT8 byEnable)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetAwbFastMode byEnable=%d\r\n", byEnable);
    #endif

	MMPS_Sensor_Set3AFunction(MMP_ISP_3A_FUNC_SET_AWB_FAST_MODE, byEnable);
	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAeFastMode
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetAeFastMode(UINT8 byEnable)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetAeFastMode byEnable=%d\r\n", byEnable);
    #endif

	MMPS_Sensor_Set3AFunction(MMP_ISP_3A_FUNC_SET_AE_FAST_MODE, byEnable);
	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAeIsoMode
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetAeIsoMode(AHC_AE_ISO byMode)
{
	UINT32 ulIso = 0;

    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetAeIsoMode byMode=%d\r\n", byMode);
	#endif

	switch(byMode) {
		case AHC_AE_ISO_AUTO: 	ulIso = 0; 		break;
		case AHC_AE_ISO_100: 	ulIso = 100; 	break;
		case AHC_AE_ISO_200: 	ulIso = 200; 	break;
		case AHC_AE_ISO_400: 	ulIso = 400; 	break;
		case AHC_AE_ISO_800: 	ulIso = 800; 	break;
		case AHC_AE_ISO_1600: 	ulIso = 1600; 	break;
		case AHC_AE_ISO_3200: 	ulIso = 3200; 	break;
		default:				ulIso = 0; 		break;
	}

	MMPS_Sensor_Set3AFunction(MMP_ISP_3A_FUNC_SET_ISO, ulIso);
	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAeFlickerMode
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetAeFlickerMode(AHC_AE_FLICKER byMode)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetAeFlickerMode byMode=%d\r\n", byMode);
    #endif
    
	MMPS_Sensor_Set3AFunction(MMP_ISP_3A_FUNC_SET_AE_FLICKER_MODE, byMode);
	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAeMeteringMode
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetAeMeteringMode(AHC_AE_METERING byMode)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetAeMeteringMode byMode=%d\r\n", byMode);
    #endif

    MMPS_Sensor_Set3AFunction(MMP_ISP_3A_FUNC_SET_AE_METER_MODE, byMode);
	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAeEvBiasMode
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetAeEvBiasMode(AHC_AE_EV_BIAS byMode)
{
    int lEv = 0;

    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetAeEvBiasMode byMode=%d\r\n", byMode);
    #endif

	switch (byMode)
	{
		case AHC_AE_EV_BIAS_M200:
			lEv = -200;
			break;
		case AHC_AE_EV_BIAS_M166:
			lEv = -166;
			break;
		case AHC_AE_EV_BIAS_M133:
			lEv = -133;
			break;
		case AHC_AE_EV_BIAS_M100:
			lEv = -100;
			break; 
		case AHC_AE_EV_BIAS_M066:
			lEv = -66;
			break;
		case AHC_AE_EV_BIAS_M033:
			lEv = -33;
			break;
		case AHC_AE_EV_BIAS_0000:
			lEv = 0;
			break;
		case AHC_AE_EV_BIAS_P033:
			lEv = 33;
			break;
		case AHC_AE_EV_BIAS_P066:
			lEv = 66;
			break;
		case AHC_AE_EV_BIAS_P100:
			lEv = 100;
			break;
		case AHC_AE_EV_BIAS_P133:
			lEv = 133;
			break;
		case AHC_AE_EV_BIAS_P166:
			lEv = 166;
			break;
		case AHC_AE_EV_BIAS_P200:
			lEv = 200;
			break;
	}
	
	MMPS_Sensor_Set3AFunction(MMP_ISP_3A_FUNC_SET_EV, lEv);
	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAeSceneMode
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetAeSceneMode(AHC_SCENE byMode)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetAeSceneMode byMode=%d\r\n", byMode);
	#endif

#if 0
	ISP_IF_F_SetScene(byMode);
#endif
	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAwbMode
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetAwbMode(AHC_AWB_MODE byMode)
{
    MMP_USHORT usMode       = 0;
    MMP_USHORT usColorTemp  = 5500;

    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetAwbMode byMode=%d\r\n", byMode);
	#endif

	switch (byMode)
	{
	case AHC_AWB_MODE_BYPASS:
	    usMode = ISP_AWB_MODE_BYPASS;
	    break;
	case AHC_AWB_MODE_AUTO:
	    usMode = ISP_AWB_MODE_AUTO;
	    break;
	case AHC_AWB_MODE_MANUAL:
	    usMode = ISP_AWB_MODE_MANUAL;
	    usColorTemp = 5500;
	    break;
    case AHC_AWB_MODE_DAYLIGHT:
        usMode = ISP_AWB_MODE_MANUAL;
        usColorTemp = 6500;
        break;
	case AHC_AWB_MODE_CLOUDY:
	    usMode = ISP_AWB_MODE_MANUAL;
	    usColorTemp = 7500;
	    break;
	case AHC_AWB_MODE_FLUORESCENT_WHITE:
	    usMode = ISP_AWB_MODE_MANUAL;
	    usColorTemp = 4100;
	    break;
	case AHC_AWB_MODE_FLUORESCENT_DAYLIGHT:
	    usMode = ISP_AWB_MODE_MANUAL;
	    usColorTemp = 6000;
	    break;
	case AHC_AWB_MODE_INCANDESCENT:
	    usMode = ISP_AWB_MODE_MANUAL;
	    usColorTemp = 3000;
	    break;
	default :
	    usMode = ISP_AWB_MODE_AUTO;
	}

    MMPS_Sensor_Set3AFunction(MMP_ISP_3A_FUNC_SET_AWB_MODE, usMode);

    if (AHC_AWB_MODE_AUTO != usMode) {
        // Run MMP_ISP_3A_FUNC_SET_AWB_COLOR_TEMP, AWB will be change to manual mode by ISP LIB
        MMPS_Sensor_Set3AFunction(MMP_ISP_3A_FUNC_SET_AWB_COLOR_TEMP, usColorTemp);
    }

	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetFastAeAwbMode
//  Description :
//------------------------------------------------------------------------------
void AHC_SetFastAeAwbMode(AHC_BOOL bEnable)
{
    if (bEnable == AHC_TRUE)
    {
        AHC_SetAeFastMode(1);
        AHC_SetAwbFastMode(1);
        
        m_bEnableFastAeAwb = AHC_TRUE;
    }
    else
    {
        m_bEnableFastAeAwb = AHC_FALSE;
        
        AHC_SetAeFastMode(0);
        AHC_SetAwbFastMode(0);
    }
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetImageEffect
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetImageEffect(AHC_IMAGE_EFFECT byMode)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetImageEffect byMode=%d\r\n", byMode);
    #endif

	MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_EFFECT, byMode);
	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetColorSaturateLevel
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetColorSaturateLevel(INT16 byLev)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetColorSaturateLevel gsSaturateCenter %d byLev=%d\r\n",gsSaturateCenter,  byLev);
    #endif
    
    if (gsSaturateCenter + byLev > 128)
        MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_SATURATION, 128);
    else if (gsSaturateCenter + byLev < -128)
        MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_SATURATION, -128);
    else
        MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_SATURATION, gsSaturateCenter + byLev);

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetColorSaturateCenter
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetColorSaturateCenter(INT16 byLev)
{
    gsSaturateCenter = byLev;
    #if (ISP_DEBUG == ISP_DEBUG_ON)
    printc("ISP:AHC_SetColorSaturateCenter gsSaturateCenter new %d ", gsSaturateCenter);
    #endif

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetColorSharpnessLevel
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetColorSharpnessLevel(INT16 byLev)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
    printc("ISP:AHC_SetColorSharpnessLevel gsSharpnessCenter %d  byLev=%d\r\n", gsSharpnessCenter, byLev);
    #endif
    
    if (gsSharpnessCenter + byLev > 127)
        MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_SHARPNESS, 127);
    else if (gsSharpnessCenter + byLev < -128)
        MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_SHARPNESS, -128);
    else
        MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_SHARPNESS, gsSharpnessCenter + byLev);

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetColorSharpnessCenter
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetColorSharpnessCenter(INT16 byLev)
{
    gsSharpnessCenter = byLev;
    #if (ISP_DEBUG == ISP_DEBUG_ON)
    printc("ISP:AHC_SetColorSharpnessCenter gsSharpnessCenter new %d ", gsSharpnessCenter);
    #endif
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetColorGammaLevel
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetColorGammaLevel(INT16 byLev)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
    printc("ISP:AHC_SetColorGammaLevel gsGammaCenter %d  byLev=%d\r\n", gsGammaCenter, byLev);
    #endif
    
    if (gsGammaCenter + byLev > 127)
        MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_GAMMA, 127);
    else if (gsGammaCenter + byLev < -128)
        MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_GAMMA, -128);
    else
        MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_GAMMA, gsGammaCenter + byLev);

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetColorGammaCenter
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetColorGammaCenter(INT16 byLev)
{
    gsGammaCenter = byLev;
    #if (ISP_DEBUG == ISP_DEBUG_ON)
    printc("ISP:AHC_SetColorGammaCenter gsGammaCenter new %d ", gsGammaCenter);
    #endif

    return AHC_TRUE;
}
//------------------------------------------------------------------------------
//  Function    : AHC_SetColorContrastLevel
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetColorContrastLevel(INT16 byLev)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
    printc("ISP:AHC_SetColorContrastLevel gusContrastCenter %d  byLev=%d\r\n", gsContrastCenter, byLev);
    #endif

    if (gsContrastCenter + byLev > 128)
            MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_CONTRAST, 128);
        else if (gsContrastCenter + byLev < -128)
            MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_CONTRAST, -128);
        else
            MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_CONTRAST, gsContrastCenter + byLev);

        return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetColorContrastCenter
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetColorContrastCenter(INT16 byLev)
{
    gsContrastCenter = byLev;
    #if (ISP_DEBUG == ISP_DEBUG_ON)
    printc("ISP:AHC_SetColorContrastCenter gsContrastCenter new %d ", gsContrastCenter);
    #endif

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_SetAeWDRMode
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_SetAeWDRMode(AHC_AE_WDR byMode)
{
    #if (ISP_DEBUG == ISP_DEBUG_ON)
	printc("ISP:AHC_SetAeWDRMode byMode=%d\r\n", byMode);
    #endif
    
    MMPS_Sensor_SetIQFunction(MMP_ISP_IQ_FUNC_SET_WDR, byMode);
	return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : MMP_SensorDrv_PostInitISP
//  Description : This function will be called in MMPS_Sensor_Initialize()
//                which will reset ISP settings earlier.
//                It's used to reset ISP settings after sensor init.
//------------------------------------------------------------------------------
void MMP_SensorDrv_PostInitISP(void)
{
	// Re-set ISP settings in AHC_PostInitISP()
	if (AHC_PostInitISP != NULL)
	{
		AHC_PostInitISP();
	}
}


//------------------------------------------------------------------------------
//  Function    : MMP_SensorDrv_FirstInitISPCB
//  Description : 
//------------------------------------------------------------------------------
void MMP_SensorDrv_FirstInitISPCB(void)
{
    #if  (defined(SUPPORT_ISP_CALIBRATION) && (SUPPORT_ISP_CALIBRATION == 1))			
	if (AHC_FirstInitISPCB != NULL)
	{
		AHC_FirstInitISPCB();
	}
    #endif
}
//------------------------------------------------------------------------------
//  Function    : 
//  Description : 
//------------------------------------------------------------------------------
//ISP_INT8 MultiShading;
//ISP_INT16 CCM_PRT[9];

void VR_PrintStringOnPreview(void)
{
#if (ISP_EN)
    extern ISP_UINT8 gDrawTextEn;

	if(gDrawTextEn){
		
	#if 0
		_sprintf(gDrawTextBuf, (ISP_INT8*)"AE  : Shutter=%x (%d)FPS, Gain=%x, ShutterBase = %x", (ISP_UINT32)ISP_IF_AE_GetShutter(), (ISP_UINT32)ISP_IF_AE_GetRealFPS(), (ISP_UINT32)ISP_IF_AE_GetGain(), ISP_IF_AE_GetShutterBase());
		VR_PrintString(gDrawTextBuf,  10, 0, 0x0000, 0x0000);
		_sprintf(gDrawTextBuf, (ISP_INT8*)"EV:%x, AE  : AvgLum=%x %x %x",(ISP_UINT32)ISP_IF_AE_GetDbgData(0), (ISP_UINT32)ISP_IF_AE_GetDbgData(1), (ISP_UINT32)ISP_IF_AE_GetDbgData(2), (ISP_UINT32)ISP_IF_AE_GetDbgData(3));
		VR_PrintString(gDrawTextBuf,  20, 0, 0x0000, 0x0000);
		_sprintf(gDrawTextBuf, (ISP_INT8*)"DBG8:%x, %x, %x, %x, %x",(ISP_UINT32)ISP_IF_IQ_GetID(ISP_IQ_CHECK_CLASS_COLORTEMP),  ISP_IF_IQ_GetID(ISP_IQ_CHECK_CLASS_GAIN),  ISP_IF_IQ_GetID(ISP_IQ_CHECK_CLASS_ENERGY),ISP_IF_AE_GetMetering(),(ISP_UINT32)ISP_IF_AE_GetLightCond());
		VR_PrintString(gDrawTextBuf,  80, 0, 0x0000, 0x0000);
		_sprintf(gDrawTextBuf, (ISP_INT8*)"Avglum:%x, EVTarget%x",(ISP_UINT32)ISP_IF_AE_GetCurrentLum(),  ISP_IF_AE_GetTargetLum());
		VR_PrintString(gDrawTextBuf,  90, 0, 0x0000, 0x0000);
	#endif

	#if 0
		_sprintf(gDrawTextBuf, (ISP_INT8*)"AWB : Mode=%x, GainR=%x, GainGr=%x, GainB=%x, CT = %x,%x", (ISP_UINT32)ISP_IF_AWB_GetMode(), (ISP_UINT32)ISP_IF_AWB_GetGainR(), (ISP_UINT32)ISP_IF_AWB_GetGainG(), (ISP_UINT32)ISP_IF_AWB_GetGainB(),(ISP_UINT32)ISP_IF_AWB_GetColorTemp(),(ISP_UINT32)ISP_IF_AWB_GetMode());
		VR_PrintString(gDrawTextBuf,  30, 0, 0x0000, 0x0000);
	#endif

	#if 0
		{
			ISP_UINT32 *dbgPtr = (ISP_UINT32 *)ISP_IF_AF_GetDbgDataPtr();
			_sprintf(gDrawTextBuf, (ISP_INT8*)"AF  : AFPos=%x, dbg=%x %x %x", (ISP_UINT32)ISP_IF_AF_GetPos(10), (ISP_UINT32)ISP_IF_AF_GetPos(10), dbgPtr[1], dbgPtr[2]);
			VR_PrintString(gDrawTextBuf,  40, 0, 0xFC00, 0x0000);
		}
	#endif
	 }
#endif
}

