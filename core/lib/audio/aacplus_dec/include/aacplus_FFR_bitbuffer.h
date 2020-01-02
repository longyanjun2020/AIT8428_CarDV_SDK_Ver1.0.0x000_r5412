/*
   Bit Buffer Management
 */

#ifndef FFR_BITBUFFER_H
#define FFR_BITBUFFER_H

#include "aacplus_intrinsics.h"

struct BIT_BUF
{
  UWord8 *pBitBufStartAddr;          /*!< pointer points to Start address in bitstream buffer tartAddr */
  UWord8 *pBitBufEndAddr;           /*!< pointer points to end address in bitstream buffer */

  UWord8 *pReadNext;                /*!< pointer points to next available word in bitstream buffer to read */
  UWord8 *pWriteNext;               /*!< pointer points to next available word in bitstream buffer to write */

  Word32  ReadBitPos;               /*!< rBitPos, range 0~7*/
  Word32  WriteBitPos;              /*!< wBitPos, range 0~7*/
  Word32  cntBits;                  /*!< number of available bits in the bitstream buffer
                                     write bits to bitstream buffer  => increment cntBits
                                     read bits from bitstream buffer => decrement cntBits */
  Word32  TotalBits;                 /*!< size of bitbuffer in bits */
}; /* size Word16: 8 */

/*! Define pointer to bit buffer structure */
typedef struct BIT_BUF *HANDLE_BIT_BUF;


/*---------------------------------------------------------------------------

functionname:AACPLUS_CreateBitBuffer
description: initialize bit buffer
returns:     bit buffer handler

---------------------------------------------------------------------------*/
HANDLE_BIT_BUF AACPLUS_CreateBitBuffer(HANDLE_BIT_BUF hBitBuf,
                               UWord8 *pBitBufBase,
                               Word16  bitBufSize);

/*---------------------------------------------------------------------------

functionname:AACPLUS_interleaveSamples
description: Interleave output samples. 
             In case of mono input, copy left channel to right channel.

---------------------------------------------------------------------------*/
void AACPLUS_interleaveSamples(Word16 *pTimeCh0, 
                       Word16 *pTimeCh1, 
                       Word16 *pTimeOut, 
                       Word32  frameSize, 
                       Word16 *channels);


#endif /* FFR_BITBUFFER_H */
