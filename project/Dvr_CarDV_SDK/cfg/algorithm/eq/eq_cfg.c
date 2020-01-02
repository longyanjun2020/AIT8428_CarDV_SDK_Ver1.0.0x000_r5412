//------------------------------------------------------------------------------
//
//  File        : NR_cfg.c
//  Description : Source file of NR configuration
//  Author      : Pohan Chen
//  Revision    : 0.0
//
//------------------------------------------------------------------------------

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "lib_retina.h"  
//==============================================================================
//
//                              LOCAL VARIABLES
//
//==============================================================================
MMP_SHORT    m_bWeightx256[257] = {
    256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 258,
    310, 400, 512, 512, 512, 512, 512, 512,
    512, 512, 512, 512, 512, 512, 512, 512,
    512, 512, 512, 512, 512, 512, 512, 512,
    512, 512, 512, 512, 512, 512, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840, 840, 840, 840, 840, 840, 840, 840,
    840
};

/*
//6k notch filter(Fs=16000)
short m_bweight_x256_16K[257] ={
256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 1	, 1	, 1	, 1	, 1	, 1	, 1	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	
};
*/
/*
//6k notch filter and 220~8000 bandpass filter (Fs=16000)
short m_bweight_x256_16K[257] ={
 1	, 1	, 1	, 1	, 1	, 1	, 1	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 1	, 1	, 1	, 1	, 1	, 1	, 1	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	, 256	,
 256	, 256	, 256	, 256	, 256	
};
*/

MMP_SHORT m_bweight_x256_16K[257] = {0, 0, 0, 0, 3, 16, 67, 174, 240, 
253, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256};


MMP_SHORT m_bweight_x256_48K[257] = {0, 0, 21, 309, 237, 248, 
255, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
256, 256, 256, 256, 256, 256, 256, 256, 256, 255, 
255, 255, 255, 254, 254, 253, 252, 251, 249, 248, 
245, 242, 239, 234, 229, 223, 215, 207, 198, 188, 
177, 166, 155, 143, 132, 121, 111, 101, 92, 83, 
75, 68, 61, 55, 50, 45, 41, 37, 33, 30, 27, 24, 
22, 20, 18, 16, 14, 13, 12, 11, 10, 9, 8, 7, 6, 6, 
5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};