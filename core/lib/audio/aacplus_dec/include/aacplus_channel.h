/*
   channel info
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#define AAC_DEC_CHANNELS 2

void MapMidSideMaskToPnsCorrelation(AacDecChannelInfo *pAacDecoderChannelInfo[AAC_DEC_CHANNELS]);

#endif /* #ifndef CHANNEL_H */
