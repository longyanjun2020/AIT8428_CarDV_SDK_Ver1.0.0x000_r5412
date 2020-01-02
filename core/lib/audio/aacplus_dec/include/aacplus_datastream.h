/*
   data stream element
 */

#ifndef DATASTREAM_H
#define DATASTREAM_H

#include "aacplus_bitstream.h"
#include "aacplus_FFR_bitbuffer.h"

enum
{
  MaximumElementLength = 512
};

typedef struct
{
  Word16 ElementInstanceTag;
  Flag DataByteAlignFlag;
  Word16 Count;
  Word8 DataStreamByte[MaximumElementLength];
} DataStreamElement;

void CDataStreamElementOpen(DataStreamElement **pDataStreamElement);
void CDataStreamElement_Read(HANDLE_BIT_BUF bs,DataStreamElement *pDataStreamElement,Word32 *pByteAlignBits);

#endif /* #ifndef DATASTREAM_H */


