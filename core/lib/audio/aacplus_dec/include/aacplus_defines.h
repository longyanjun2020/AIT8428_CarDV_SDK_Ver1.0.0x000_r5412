/*
   global defines
 */

#ifndef __DEFINES_H
#define __DEFINES_H

#include "mmpf_audio_typedef.h"

/* TNS */

#define MaximumWindows 8
#define MaximumBands 49
#define MaximumOrder 31
#define MaximumFilters 3
/* code books related */
#define HuffmanBits 2
#define HuffmanEntries (1 << HuffmanBits)
typedef struct tagCodeBookDescription
{
  Word16 Dimension;
  Word16 numBits;
  Word16 Offset;
  const Word16 (*HuffmanCodeBook)[HuffmanEntries];
} CodeBookDescription;


#endif
