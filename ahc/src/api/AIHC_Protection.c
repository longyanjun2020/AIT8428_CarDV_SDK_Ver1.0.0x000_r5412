/*===========================================================================
 * Include files
 *===========================================================================*/ 
 
#include "AHC_FS.h"
#include "AHC_General.h"
#include "AHC_Parameter.h"
#include "AHC_Message.h"
#include "AHC_Menu.h"
#include "AHC_Video.h"
#include "AHC_Parameter.h"

#include "AHC_Config_SDK.h"
#include "AHC_Version.h"
#include "AHC_Media.h"
#include "AHC_Warningmsg.h"
#include "AHC_Utility.h"
#include "AHC_UF.h"
#include "AIHC_DCF.h"
#include "AHC_DCF.h"
#include "includes_fw.h"
#include "ait_utility.h"

/*===========================================================================
 * Global varible
 *===========================================================================*/ 
static AHC_PROTECT_TYPE m_VRProtectType  = AHC_PROTECT_NONE;

/*===========================================================================
 * Main body
 *===========================================================================*/ 

void AHC_Protect_SetType(AHC_PROTECT_TYPE Type) 
{
    m_VRProtectType = Type;
}

AHC_PROTECT_TYPE AHC_Protect_GetType(void) 
{
    return m_VRProtectType;    
}

AHC_BOOL AHC_Protect_SpellFileName(char* FilePathName, INT8* pchDirName, INT8* pchFileName)
{
    UINT32  uiFileNameLength;
    INT8    tmpDirName[MAX_FILE_NAME_SIZE];
    GetPathDirStr(tmpDirName,sizeof(tmpDirName),FilePathName);
    memcpy(pchDirName,tmpDirName,AHC_StrLen(tmpDirName));
    uiFileNameLength = AHC_StrLen(FilePathName) - AHC_StrLen(pchDirName) - 1;
    memcpy(pchFileName,FilePathName + AHC_StrLen(pchDirName) + 1,uiFileNameLength);
    
    return AHC_TRUE;
}

AHC_BOOL AHC_Protect_SetVRFile(AHC_PROTECTION_OP Op, AHC_PROTECT_TYPE Type)
{
    #define MAX_NAME_LENGTH (32)
    
    UINT32          	uiMaxObjNumber;    
    char     			FilePathName[MAX_FILE_NAME_SIZE];
	UINT16              DirKey;
	
    INT8 				OldDirName[MAX_NAME_LENGTH];
    INT8 				OldFileName[MAX_NAME_LENGTH];
    
	AHC_UF_SetFreeChar(0, DCF_SET_ALLOWED, (UINT8*)SEARCH_MOVIE);
 		
	AHC_UF_GetTotalFileCount(&uiMaxObjNumber);
   
	if (uiMaxObjNumber == 0) {
		return AHC_FALSE;
	}
	else if (uiMaxObjNumber == 1) {
	
		goto FINAL;
    }
    
 	if ((Op & AHC_PROTECTION_PRE_FILE) != 0)
 	{
 	    MEMSET(FilePathName, 0, sizeof(FilePathName));
        MEMCPY(FilePathName, AHC_VIDEO_GetPrevRecFullName(), MAX_FILE_NAME_SIZE);
        if(STRLEN(FilePathName) == 0)
        {
            printc("PreFileName == NULL,No continuous event!\r\n");
            goto FINAL;
        }
    	printc("PreFileName: %s \n", FilePathName);

        MEMSET(OldDirName, 0, sizeof(OldDirName));
        MEMSET(OldFileName, 0, sizeof(OldFileName));
        
        AHC_Protect_SpellFileName(FilePathName, OldDirName, OldFileName);
        
        #if (PROTECT_FILE_TYPE == PROTECT_FILE_MOVE_SUBDB)///< #if (PROTECT_FILE_TYPE)

        if((Type & AHC_PROTECT_MENU) != 0){
		    AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_READ_ONLY_ALL_CAM, NULL, NULL);
		}    
		
    	if(((Type & AHC_PROTECT_G) != 0) || ((Type & AHC_PROTECT_MOVE) != 0)){

            AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_MOVE_SUBDB, NULL, NULL);            
        }    

		#elif (PROTECT_FILE_TYPE == PROTECT_FILE_RENAME) ///< #if (PROTECT_FILE_TYPE)
        {
        INT8    NewFileName[MAX_NAME_LENGTH];

        MEMSET(NewFileName, 0, sizeof(NewFileName));

    	STRCPY(NewFileName, OldFileName);

        if((Type & AHC_PROTECT_MENU) != 0){
		    AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_READ_ONLY_ALL_CAM, NULL,NULL);
		}    
		
    	if((Type & AHC_PROTECT_G) != 0){
            MEMCPY(NewFileName, DCF_CHARLOCK_PATTERN, STRLEN(DCF_CHARLOCK_PATTERN));

            AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_CHAR_LOCK_ALL_CAM, NULL, NULL);
        }    
        //#if (LIMIT_MAX_LOCK_FILE_NUM && MAX_LOCK_FILE_ACT!=LOCK_FILE_STOP)
        //AHC_UF_UpdateLockFileTable(uiObjIndex,NULL); this act should be in AHC_DCF.h
        //#endif
		printc("NewFileName: %s \n", NewFileName);
		}
		
		#elif (PROTECT_FILE_TYPE == PROTECT_FILE_MOVE_DB) ///< #if (PROTECT_FILE_TYPE)
		#if(FS_FORMAT_FREE_ENABLE == 0)
        if(!(AHC_UF_MoveFile(AHC_UF_GetDB(), DCF_DB_TYPE_3RD_DB, DirKey, OldFileName, AHC_FALSE)))
        {
            printc("MoveFile to DB %d failed\r\n",DCF_DB_TYPE_3RD_DB);
            printc("Replace it by ReadOnly Operation\r\n");
            AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_READ_ONLY_ALL_CAM, NULL,NULL);
        }
		#else
		printc(FG_RED("Warning : Format Free does not support PROTECT_FILE_TYPE = PROTECT_FILE_MOVE_DB"));
		#endif
        
		#else///< #if (PROTECT_FILE_TYPE)
        
        AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_READ_ONLY_ALL_CAM, NULL,NULL);
        
		#endif///< #if (PROTECT_FILE_TYPE)
	}

FINAL:

	if((Op & AHC_PROTECTION_CUR_FILE) != 0)  
	{
		MEMSET(FilePathName, 0, sizeof(FilePathName));
        MEMCPY(FilePathName , (UINT8*)AHC_VIDEO_GetCurRecFileName(AHC_TRUE), sizeof(FilePathName));
        if(STRLEN(FilePathName) == 0)
        {
            printc("Cur FilePathName == NULL, Lock Current File failed\r\n");
            return AHC_FALSE;
        }
		printc("CurFileName: %s \n", FilePathName);

		MEMSET(OldDirName, 0, sizeof(OldDirName));
		MEMSET(OldFileName, 0, sizeof(OldFileName));
		
        AHC_Protect_SpellFileName(FilePathName, OldDirName, OldFileName);
        
        #if (PROTECT_FILE_TYPE == PROTECT_FILE_MOVE_SUBDB)///< #if (PROTECT_FILE_TYPE)

        if((Type & AHC_PROTECT_MENU) != 0) {
		    AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_READ_ONLY_ALL_CAM, NULL,NULL);
		}
		
        if(((Type & AHC_PROTECT_G) != 0) || ((Type & AHC_PROTECT_MOVE) != 0)) {
            AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_MOVE_SUBDB, NULL, NULL);
        }

		#elif (PROTECT_FILE_TYPE == PROTECT_FILE_RENAME)///< #if (PROTECT_FILE_TYPE)
		{
        INT8    NewFileName[MAX_NAME_LENGTH];

        MEMSET(NewFileName, 0, sizeof(NewFileName));
        
        STRCPY(NewFileName, OldFileName);
        
        if((Type & AHC_PROTECT_MENU) != 0) {
		    AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_READ_ONLY_ALL_CAM, NULL,NULL);
		}
		
        if((Type & AHC_PROTECT_G) != 0) {
        
            MEMCPY(NewFileName, DCF_CHARLOCK_PATTERN, STRLEN(DCF_CHARLOCK_PATTERN));

			#if (USE_INFO_FILE)
            STRCPY(AHC_InfoLog()->DCFCurVideoFileName, NewFileName);
			#endif            

            AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_CHAR_LOCK_ALL_CAM, NULL, NULL);
        }
		//#if (LIMIT_MAX_LOCK_FILE_NUM && MAX_LOCK_FILE_ACT!=LOCK_FILE_STOP)
        //AHC_UF_UpdateLockFileTable(uiObjIndex,NULL); this act should be in AHC_DCF.h
        //#endif
		printc("NewFileName: %s \n", NewFileName);
		}
		
		#elif (PROTECT_FILE_TYPE == PROTECT_FILE_MOVE_DB) ///< #if (PROTECT_FILE_TYPE)
		#if(FS_FORMAT_FREE_ENABLE == 0)
		if(!(AHC_UF_MoveFile(AHC_UF_GetDB(), DCF_DB_TYPE_3RD_DB, DirKey, OldFileName, AHC_FALSE)))
        {
            printc("MoveFile to DB %d failed\r\n",DCF_DB_TYPE_3RD_DB);
            printc("Replace it by ReadOnly Operation\r\n");
            AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_READ_ONLY_ALL_CAM, NULL,NULL);
        }        
        #else 
		printc(FG_RED("Warning : Format Free does not support PROTECT_FILE_TYPE = PROTECT_FILE_MOVE_DB"));
		#endif
		#else///< #if (PROTECT_FILE_TYPE)
        
        AHC_UF_FileOperation((UINT8*)OldDirName,(UINT8*)OldFileName, DCF_FILE_READ_ONLY_ALL_CAM, NULL,NULL);
        
		#endif///< #if (PROTECT_FILE_TYPE)
	}
	return AHC_TRUE;	
}
