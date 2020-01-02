/*
   current bitstream parameters
 */

#ifndef STREAMINFO_H
#define STREAMINFO_H


typedef struct
{
  Word16  Stream_FsIndex;
  Word32  Stream_Fs;
  Word16  Stream_DecProfile;
  Word16  Stream_ChannelConfig;
  Word16  Stream_Channels;
  Word32  Stream_BitRate;
  Word16  Stream_SamplesPerFrame;
}StreamInfo;



#endif /* #ifndef STREAMINFO_H */
