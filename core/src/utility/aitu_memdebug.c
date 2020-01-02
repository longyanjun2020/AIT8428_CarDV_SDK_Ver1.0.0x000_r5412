//==============================================================================
//
//  File        : aitu_memdebug.c
//  Description : Generic Memory Debug Utility
//  Author      : Eroy
//  Revision    : 1.0
//
//==============================================================================

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "includes_fw.h"
#include "aitu_memdebug.h"
#include "UartShell.h"

//==============================================================================
//
//                              PUBLIC FUNCTIONS
//
//==============================================================================
//------------------------------------------------------------------------------
//  Function    : AUTL_MemDbg_Init
//------------------------------------------------------------------------------
/** 
    @brief  Initialize the memory debug block.
    @param[in] pBlock : pointer to memory debug block.
    @return 0 for success, others for failed.
*/
int AUTL_MemDbg_Init(AUTL_MEMDBG_BLOCK *pBlock, MMP_UBYTE ubUsageId)
{
#if (MEM_MAP_DBG_EN)
	int i = 0;

    if (!pBlock) {
        return MEM_DBG_ERR_NULL_PTR;
	}

	if (ubUsageId >= AUTL_MEMDBG_USAGE_ID_UNDEF) {
		return MEM_DBG_ERR_PARAM;
	}

    pBlock->sUsageId = ubUsageId;
    
    MEMSET(&(pBlock->sItem[0]), 0, MEM_DBG_MAX_ITEM_NUM * sizeof(AUTL_MEMDBG_ITEM));

	for (i = 0; i < MEM_DBG_MAX_ITEM_NUM; i++) {
		pBlock->sItem[i].ulItemIdx = 0xFF;
	}
#endif
    return MEM_DBG_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : AUTL_MemDbg_PushItem
//------------------------------------------------------------------------------
/** 
    @brief  Push the item into debug block
    @param[in] pBlock 		: pointer to memory debug block.
    @param[in] ulItemIdx 	: item index.
    @param[in] ulMemSt 		: buffer start address.
    @param[in] ulMemEd 		: buffer end address.
    @param[in] ulMemSz 		: buffer size.
    @param[in] ubMemDesc 	: buffer description.     
    @return 0 for success, others for failed.
*/
int AUTL_MemDbg_PushItem(AUTL_MEMDBG_BLOCK 	*pBlock, 
						 MMP_ULONG 			ulItemIdx, 
						 MMP_ULONG   		ulMemSt,
						 MMP_ULONG   		ulMemEd,
						 MMP_ULONG   		ulMemSz,
						 MMP_UBYTE*   		ubMemDesc)
{
#if (MEM_MAP_DBG_EN)
    if (!pBlock) {
        return MEM_DBG_ERR_NULL_PTR;
	}
	
	if (ulItemIdx >= MEM_DBG_MAX_ITEM_NUM) {
		return MEM_DBG_ERR_PARAM;
	}

	pBlock->sItem[ulItemIdx].ulItemIdx 	= ulItemIdx;
	pBlock->sItem[ulItemIdx].ulMemSt	= ulMemSt;
	pBlock->sItem[ulItemIdx].ulMemEd	= ulMemEd;
	pBlock->sItem[ulItemIdx].ulMemSz	= ulMemSz;
	
	memcpy(pBlock->sItem[ulItemIdx].ubMemDesc, ubMemDesc, MEM_DBG_MAX_STRING_LEN);
#endif	
    return MEM_DBG_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : AUTL_MemDbg_ShowItem
//------------------------------------------------------------------------------
/** 
    @brief  Show the item.
    @param[in] pBlock 		: pointer to memory debug block.
    @param[in] ulItemIdx 	: item index.
    @return 0 for success, others for failed.
*/
int AUTL_MemDbg_ShowItem(AUTL_MEMDBG_BLOCK *pBlock, MMP_ULONG ulItemIdx)
{
#if (MEM_MAP_DBG_EN)
    if (!pBlock) {
        return MEM_DBG_ERR_NULL_PTR;
	}
	
	if (ulItemIdx >= MEM_DBG_MAX_ITEM_NUM) {
		return MEM_DBG_ERR_PARAM;
	}
	
	if (pBlock->sItem[ulItemIdx].ulItemIdx != 0xFF) {
	    printc("Item[%2d] Start 0x%x, End 0x%x, Size %10d (Byte) [%s]\r\n",
	    		pBlock->sItem[ulItemIdx].ulItemIdx,
	    		pBlock->sItem[ulItemIdx].ulMemSt,
	    		pBlock->sItem[ulItemIdx].ulMemEd,
	    		pBlock->sItem[ulItemIdx].ulMemSz,
	    		pBlock->sItem[ulItemIdx].ubMemDesc);
    }		
#endif
    return MEM_DBG_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : AUTL_MemDbg_ShowAllItems
//------------------------------------------------------------------------------
/** 
    @brief  Show all items.
    @param[in] pBlock : pointer to memory debug block.
    @return 0 for success, others for failed.
*/
int AUTL_MemDbg_ShowAllItems(AUTL_MEMDBG_BLOCK *pBlock, MMP_BOOL bShow)
{
#if (MEM_MAP_DBG_EN)
	int i = 0;

    if (!pBlock) {
        return MEM_DBG_ERR_NULL_PTR;
	}
	
	if (!bShow) {
		return MEM_DBG_ERR_NONE;
	}
	
	printc("=== Memory Block ID[%d] ===\r\n", pBlock->sUsageId);
	
	for (i = 0; i < MEM_DBG_MAX_ITEM_NUM; i++) {
	    AUTL_MemDbg_ShowItem(pBlock, i);
	}
	
	printc("===========================\r\n");
#endif	
    return MEM_DBG_ERR_NONE;
}

