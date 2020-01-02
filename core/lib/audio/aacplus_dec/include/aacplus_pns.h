/*
   perceptual noise substitution tool
 */

#ifndef PNS_H
#define PNS_H

#include "aacplus_ffr.h"
#include "aacplus_channelinfo.h"



#define PNS_BAND_FLAGS_SIZE              16  /* (short-window) max. scalefactors * max. groups / sizeof(char) = 16 * 8 / 8 */

typedef struct {
  UWord16   sPns_Correlated[PNS_BAND_FLAGS_SIZE];
  Word16    sPns_RandomState[PNS_BAND_FLAGS_SIZE * 8];
} PnsInterChannelData;

typedef struct {
  Word16    Pns_current_seed;
  Word16    Pns_frame_number;
} PnsStaticInterChannelData;

typedef struct {
  UWord16 sPnsUsed[PNS_BAND_FLAGS_SIZE];
  Word16 sPns_CurrentEnergy;
  UWord8 bPns_Active;
} PnsData;


#endif /* #ifndef PNS_H */
