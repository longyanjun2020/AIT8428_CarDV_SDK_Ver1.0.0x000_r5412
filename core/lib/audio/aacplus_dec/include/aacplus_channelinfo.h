/*
   individual channel stream info
 */

#ifndef CHANNELINFO_H
#define CHANNELINFO_H

#include "aacplus_defines.h"
#include "aacplus_overlapadd.h"
#include "aacplus_pulsedata.h"
#include "aacplus_streaminfo.h"
#include "aacplus_pns.h"
#include "aacplus_FFR_bitbuffer.h"



#define MAX_WINDOWS   8
#define MAX_SFB_LONG  64
#define MAX_SFB_SHORT 16
#define MAX_QUANTIZED_VALUE 8191
// Eric add for MAIN profile support
#define MAX_SFB       51

enum
{
  OnlyLongSequence = 0,
  LongStartSequence,
  EightShortSequence,
  LongStopSequence,

  ProfileMain = 0,
  ProfileLowComplexity,
  ProfileSSR,
  ProfileReserved
};

typedef struct
{
  Word16 sCommonWindow;
  Word16 sElementInstanceTag;
  Word16 sGlobalGain;
} RawDataInfo;

// Eric add for MAIN profile support
typedef struct
{
    UWord8 blimit;
    UWord8 bpredictor_reset;
    UWord8 bpredictor_reset_group_number;
    UWord8 bprediction_used[MAX_SFB];
} PredInfo;

typedef struct
{
  Word16 sValid;

  Word16 sIcsReservedBit;
  Word16 sWindow_Shape;
  Word16 sWindow_Sequence;
  Word16 sMaxScalarFactorBands;
  Word16 sScaleFactorCluster;

  Word16 sTotalScalarFactorBands;
  Word16 sFsIndex;
  Word16 sProfileType;

  Word16 sWindowGroups;
  Word16 sWindowGroupLength[8];

  // Eric add for MAIN profile support
  //  Word16 swb_offset[52];
  Word16 sPredictorDataPresent;  
  UWord16 usScalarFactorWindBandIdx_offset_max;
  UWord8  ubSfb_cb[8][8*15];
  PredInfo PredInfo;
} IcsInfo;

#define JointStereoMaximumBands 64
#define JointStereoMaximumGroups 8

typedef struct
{
  Word16 sMsMaskPresent;
  Flag FlagMsUsed[JointStereoMaximumBands]; /*!< every byte contains flags for up to 8 groups */
} JointStereoData;

typedef struct
{
  Word16 sStartFreqBand;
  Word16 sStopFreqBand;
  Word16 sDirection;
  Word16 sFreqResolution;

  Word16 sFilterOrder;
  Word16 sFilterCoeff[MaximumOrder];
} FilterInfo;

typedef struct
{
  Flag          FlagTnsDataPresent;
  Word16        sNumberOfFilters[MaximumWindows];
  FilterInfo    FilterInfoMatrix[MaximumWindows][MaximumFilters];
} TnsData;

typedef struct
{
  const Word32 *pLongWindow[2];
  const Word32 *pShortWindow[2];
  OverlapAddData OverlapAddData;
} AacDecKeepChannelInfo;

typedef struct
{
  Word16 saSpecScaleByWindow[MAX_WINDOWS];
  Word16 saSfbScale[MAX_WINDOWS * MAX_SFB_SHORT];
  Word16 saScaleFactor[MAX_WINDOWS * MAX_SFB_SHORT];
  Word8  baHuffmanCodeBook[MAX_WINDOWS * MAX_SFB_SHORT];
} AacDecoderDynamicData;

typedef struct
{
  JointStereoData JointStereoData; /*! Common MS-mask for a channel-pair */
  PnsInterChannelData PnsInterChannelData;
} AacDecoderDynamicCommonData;

// Eric add for MAIN profile support 
/* used to save the prediction state */
typedef struct {
    Word16 r[2];
    Word16 COR[2];
    Word16 VAR[2];
} PredData;


typedef struct
{
  Word16 *pChanSpecScale;
  Word16 *pChanSfbScale;
  Word16 *pChanScaleFactor;
  Word8  *pChanCodeBook;
  Word32 *pChanSpectralCoefficient;

  IcsInfo ChanIcsInfo;
  TnsData ChanTnsData;
  PulseData ChanPulseData;
  RawDataInfo ChanRawDataInfo;
  JointStereoData * pChanJointStereoData;

  PnsData PnsData;
  PnsInterChannelData       *pChanPnsInterChannelData;
  PnsStaticInterChannelData *pChanPnsStaticInterChannelData;
  // Eric add for MAIN profile support 
  PredData                  *pChanIntraChannelPredictData;
} AacDecChannelInfo;

//Word32 GetSamplingFrequency(CIcsInfo *pIcsInfo);
Word16 GetMaximumTnsBands(IcsInfo *pIcsInfo);


#endif /* #ifndef CHANNELINFO_H */
