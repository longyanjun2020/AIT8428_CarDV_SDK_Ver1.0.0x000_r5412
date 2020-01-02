/*
   independent channel concealment
 */

#ifndef CONCEAL_H
#define CONCEAL_H
#include "aacplus_channel.h"
#include "aacplus_channelinfo.h"
#include "aacplus_aacdecoder.h"

typedef enum{
  ErrConcealUninitialized = -1,
  ErrConcealOk         = 0x0000,
  ErrConcealInterpolate= 0x0001,
  ErrConcealFadeOut    = 0x0002,
  ErrConcealMute       = 0x0004,
  ErrConcealSuppressed = 0x0008,
  ErrConcealFadeIn     = 0x0010
} ErrConcealState;

typedef struct
{
  Word32            SpectralCoeff[AAC_DEC_CHANNELS][FRAME_SIZE];        /*!< Spectral data of previous frame   */
  Word16            PreFrameSpecScale[AAC_DEC_CHANNELS][MAX_WINDOWS];   /*!< Scaling factors of previous frame */
  Word16            PreFrameWindowShape[AAC_DEC_CHANNELS];              /*!< Window shape of previous frame */
  Word16            PreFrameBlockType[AAC_DEC_CHANNELS];                /*!< Block type of previous frame */
  Flag              PrevTwoFrameFlag[AAC_DEC_CHANNELS][2];              /*!< Stores frameOK flag of two previous frames */
  Word16            ConcealFrameAcc[AAC_DEC_CHANNELS];                  /*!< total of concealed frames */
  ErrConcealState   ConcealState[AAC_DEC_CHANNELS];                     /*!< Status of concealment state machine */

  Word16   iRandomPhase;                              /*!< Index into random phase table */
} AacErrConcealmentInfo;



#endif /* #ifndef CONCEAL_H */
