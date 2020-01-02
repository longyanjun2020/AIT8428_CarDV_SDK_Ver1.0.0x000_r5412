#ifndef _MMPF_USBPCCAM_H_
#define _MMPF_USBPCCAM_H_

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define PIPE0_EN            (1<<0)
#define PIPE1_EN            (1<<1)
#define PIPE_EN_MASK        (0x3)
#define PIPE_EN_SYNC        (1<<2)

#define PIPE_CFG_YUY2       (1<<0)
#define PIPE_CFG_MJPEG      (1<<1)
#define PIPE_CFG_H264       (1<<2)
#define PIPE_CFG_NV12       (1<<3)
#define PIPE_CFG_MASK       (0x7)

#define PIPE_CFG(pipe, cfg) ((cfg & PIPE_CFG_MASK) << (pipe << 2))

#define PIPE_PH_TYPE_NA     (0)
#define PIPE_PH_TYPE_1      (1)
#define PIPE_PH_TYPE_2      (2)

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _H264_FORMAT_TYPE {
    INVALID_H264 = 0,
    FRAMEBASE_H264,
    UVC_H264,
    SKYPE_H264
} H264_FORMAT_TYPE;

typedef enum _RES_TYPE_LIST {
    PCCAM_640_360 = 0,
    PCCAM_640_480,
    PCCAM_720_480,
    PCCAM_800_600,
    PCCAM_848_480,
    PCCAM_960_720,
    PCCAM_1024_576,
    PCCAM_1024_600,
    PCCAM_1024_768,
    PCCAM_1280_720,
    PCCAM_1280_960,
    PCCAM_1600_1200,
    PCCAM_1920_1080,
    PCCAM_2048_1536,
    PCCAM_2176_1224,
    PCCAM_2560_1440,
    PCCAM_2560_1920,
    PCCAM_DUALSNR_1472_736,
    PCCAM_DUALSNR_1920_960,
    PCCAM_DUALSNR_2176_1088,
    PCCAM_DUALSNR_2560_1280,
    PCCAM_DUALSNR_3008_1504,
    PCCAM_RES_NUM
} RES_TYPE_LIST;

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct _STREAM_BUF_CTL
{
    MMP_ULONG   rd_index;
    MMP_ULONG   wr_index;
    MMP_ULONG   total_rd;
    MMP_ULONG   total_wr;
    MMP_ULONG   buf_addr;
    MMP_ULONG   slot_size;
    MMP_USHORT  slot_num;
} STREAM_BUF_CTL;

typedef struct _STREAM_CFG
{
    MMP_UBYTE   pipe_en;
    MMP_UBYTE   pipe_cfg;
    MMP_USHORT  pipe0_w;
    MMP_USHORT  pipe0_h;
    MMP_USHORT  pipe1_w;
    MMP_USHORT  pipe1_h;
    STREAM_BUF_CTL pipe0_b;
    STREAM_BUF_CTL pipe1_b;
} STREAM_CFG;

typedef struct _RES_TYPE_CFG
{
    RES_TYPE_LIST res_type;
    MMP_USHORT  res_w;
    MMP_USHORT  res_h;
} RES_TYPE_CFG;

typedef struct _STREAM_DMA_BLK
{
    MMP_USHORT  max_dsize;
    MMP_USHORT  header_len;
    MMP_ULONG   blk_addr;
    MMP_ULONG   blk_size;
    MMP_ULONG   next_blk;
    MMP_ULONG   cur_addr;
    MMP_ULONG   tx_len;
    MMP_ULONG   tx_packets;
    MMP_ULONG   dma_buf[2];
    MMP_ULONG   dummy_flag;
} STREAM_DMA_BLK;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

RES_TYPE_CFG *GetResCfg(MMP_UBYTE resolution);
MMP_ULONG GetYUY2FrameSize(MMP_UBYTE resolution);
MMP_ULONG GetMJPEGFrameSize(MMP_UBYTE resolution);
MMP_ULONG GetYUV420FrameSize(MMP_UBYTE resolution);
MMP_ULONG GetH264FrameSize(MMP_UBYTE resolution);
MMP_USHORT GetMaxFrameRate(MMP_UBYTE resolution) ;

void MMPF_PCAM_InitMJPGByQueue(STREAM_CFG *stream_cfg, MMP_ULONG res);
void MMPF_PCAM_InitMJPEG(STREAM_CFG *stream_cfg,MMP_ULONG res);
void MMPF_PCAM_InitStream(STREAM_CFG *stream_cfg,MMP_ULONG res);
void MMPF_PCAM_UnInitStream(void);
STREAM_CFG *usb_get_cur_image_pipe(void);
void usb_set_cur_image_pipe(STREAM_CFG *cur_pipe);
MMP_USHORT MMPF_PCAM_GetJfifTag(MMP_USHORT *pTagID, MMP_USHORT *pTagLength, MMP_UBYTE **ppImgAddr, MMP_ULONG length);

MMP_BOOL MMPF_Video_IsEmpty(MMP_UBYTE pipe);
MMP_BOOL MMPF_Video_IsFull(MMP_UBYTE pipe);
MMP_UBYTE MMPF_Video_FreeSlot(MMP_UBYTE pipe);
void MMPF_Video_UpdateWrPtr(MMP_UBYTE pipe);
void MMPF_Video_UpdateRdPtr(MMP_UBYTE pipe);
MMP_UBYTE *MMPF_Video_CurRdPtr(MMP_UBYTE pipe);
MMP_UBYTE *MMPF_Video_CurWrPtr(MMP_UBYTE pipe);
void MMPF_Video_Init_Buffer(void);
STREAM_BUF_CTL *MMPF_Video_GetStreamCtlInfo(MMP_UBYTE pipe);
void MMPF_Video_UpdateRdPtrByPayloadLength(void);

void MMPF_Video_InitDMABlk(MMP_USHORT uvc_payload_size,MMP_ULONG dmabuf1,MMP_ULONG dmabuf2) ;
void MMPF_Video_AddDMABlk(MMP_ULONG header_len,MMP_ULONG blk_addr,MMP_ULONG blk_size,MMP_USHORT dummy_flag);
STREAM_DMA_BLK *MMPF_Video_CurBlk(void);
MMP_BOOL MMPF_Video_NextBlk(void);
MMP_UBYTE *MMPF_Video_GetBlkApp3Header(STREAM_DMA_BLK *dma_blk);

void MMPF_USB_ReleaseDm(void);

#endif //_MMPF_USBPCCAM_H_
