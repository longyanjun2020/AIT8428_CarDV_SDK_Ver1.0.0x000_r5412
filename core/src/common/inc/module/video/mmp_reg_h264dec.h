//==============================================================================
//
//  File        : mmp_reg_h264dec.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Will Chen
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMP_REG_H264DEC_H_
#define _MMP_REG_H264DEC_H_

#include "mmp_register.h"

#define H264_BASE_ADR               0x8000C800
#define H264_REF_BASE_ADDR          0x8000C810
#define H264_REC_FRAME_ADDR         0x8000C860
#define H264_DBLK_BASE_ADDR         0x8000C870
#define H264_VLD_BASE_ADDR          0x8000C880
#define H264_VLD_DATA0_BASE_ADDR    0x8000C900
#define H264_VLD_DATA1_BASE_ADDR    0x8000C980
#define H264_MB_TO_SLICE_MAP_ADDR   0x80010000
#define H264_VLD_BUF0_ADDR          0x8000E800
#define H264_VLD_BUF1_ADDR          0x8000EC00

#define HW_VLD_MODE                 0x00
#define SW_VLD_MODE                 0x40
#define FMO_MODE                    0x02
#define NON_FMO_MODE                0x00

/** @addtogroup MMPH_reg
@{
*/
//--------------------------------------------
// H264 Decoder structure (0x80000000)
//--------------------------------------------

//--------------------------------------------
// Global Register Address (0x80000000)
//--------------------------------------------
typedef struct _AITS_H264DEC_CTL {
    AIT_REG_B   FRAME_CTL_REG;                              // 0x0000
        /*-DEFINE-----------------------------------------------------*/
        #define AVC_DEC_DISABLE                     0x00
        #define AVC_DEC_ENABLE                      0x01
        #define AVC_FMO_DISABLE                     0x00
        #define AVC_FMO_ENABLE                      0x02
        #define AVC_NEW_FRAME_START                 0x04
        #define AVC_PARSE_EP3_SLICE_MODE_ENABLE     0x08
        #define AVC_ENC_ENABLE                      0x10
        #define AVC_ENC_DISABLE                     0x00
        #define AVC_PARSE_EP3_SEG_MODE_ENABLE       0x20
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x0001;
    AIT_REG_B   SW_RESET_CTL;                               // 0x0002
        /*-DEFINE-----------------------------------------------------*/
        #define VLD_RESET                           0x01
        #define IDCT_RESET                          0x02
        #define INTRA_PRED_RESET                    0x04
        #define INTER_PRED_RESET                    0x08
        #define FRAME_RECONSTRUCT_RESET             0x10
        #define DBLK_RESET                          0x20
        #define FLOW_CONTROL_RESET                  0x40
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x0003;
    AIT_REG_W   CPU_INT_ENABLE;					            // 0x0004
    AIT_REG_W   CPU_INT_STATUS;					            // 0x0006
    AIT_REG_W   HOST_INT_ENABLE;				            // 0x0008
    AIT_REG_W   HOST_INT_STATUS;				            // 0x000A
        /*-DEFINE-----------------------------------------------------*/
        #define H264D_INT_DECODE_SEGMENT_DONE       0x0001
        #define H264D_INT_DECODE_SLICE_DONE         0x0002
        #define H264D_INT_DECODE_SLICE_ERROR        0x0004
        #define H264D_INT_DECODE_TIMEOUT_ERROR      0x0020
        #define H264D_INT_DBLK_FRAME_DONE           0x0040
        #define H264D_INT_PARSE_EP3_DONE            0x0080
        #define H264D_INT_INTRA_PRED_MB_DONE        0x0100
        #define H264D_INT_INTER_PRED_REFFRAME_FETCH_DONE    0x0200
        #define H264D_INT_INTER_PRED_INTERPOLATION_DONE     0x0400
        #define H264D_INT_INVERT_TRANS_MB_DONE      0x0800
        #define H264D_INT_RECONSTRUCT_SLICE_DONE    0x1000
        #define H264D_INT_RECONSTRUCT_MB_DONE       0x2000
        #define H264D_INT_RECONSTRUCT_BUF_EMPTY     0x4000
        #define H264D_INT_DEBLOCK_DONE              0x8000
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x000C[0x4];
} AITS_H264DEC_CTL, *AITPS_H264DEC_CTL;

//--------------------------------------------
// Reference Frame Start Address (0x80000010)
//--------------------------------------------
typedef struct _AITS_H264DEC_REF {
    AIT_REG_B   LIST0_MAP[8];           /* frame N+1 | frame N */
    AIT_REG_D   REFERENCE_ADDR[16];     /* 4th byte | 3rd byte | 2nd byte | 1st byte */
    AIT_REG_D   Y_SIZE;
    AIT_REG_B                       _x005C[0x4];
} AITS_H264DEC_REF, *AITPS_H264DEC_REF;

//--------------------------------------------
// Frame Reconstruct Register Address (0x80000060)
//--------------------------------------------
typedef struct _AITS_H264DEC_REC {
    AIT_REG_D   REC_FRAME_ADDR;                         // 0x0060
    AIT_REG_W   RING_BUF_SIZE;			                // 0x0064
    AIT_REG_B                       _x0066[0x2];
    AIT_REG_D   RING_BUF_LOWERBOUND_ADDR;	            // 0x0068
    AIT_REG_D   RING_BUF_HIGHERBOUND_ADDR;	            // 0x006C
} AITS_H264DEC_REC, *AITPS_H264DEC_REC;

//--------------------------------------------
// Deblocking Register Address (0x80000070)
//--------------------------------------------
typedef struct _AITS_H264DEC_DBLK {
    AIT_REG_D   SOURCE_ADDR;			                // 0x0070
    AIT_REG_D   DEST_ADDR;			                    // 0x0074
    AIT_REG_B   SOURCE_SIZE_IN_MBROWS;                  // 0x0078
    AIT_REG_B                       _x0079;
    AIT_REG_B   DEBLOCKING_CTRL;		                // 0x007A
        /*-DEFINE-----------------------------------------------------*/
        #define DBLK_START                          0x01
        #define DBLK_TRIGGER_BY_HW                  0x00
        #define DBLK_TRIGGER_BY_SW                  0x02
        #define DBLK_OUT_ROT_MODE                   0x04
        #define DBLK_UP_ROW_IN_FB                   0x08
        #define DBLK_ONE_MB_MODE                    0x10
        #define DBLK_OUT_EN_I                       0x20
        #define DBLK_OUT_EN_P                       0x40
        #define DBLK_OUT_EN_B                       0x80
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x007B;
    AIT_REG_D   DBLK_PARAMS_BUF_ADDR;		            // 0x007C
} AITS_H264DEC_DBLK, *AITPS_H264DEC_DBLK;

//--------------------------------------------
// VLD Register Address (0x80000080)
//--------------------------------------------
typedef struct _AITS_H264DEC_VLD {
    AIT_REG_W   PICTURE_SIZE_IN_MBS;		            // 0x0080
    AIT_REG_W   MB_WIDTH;			                    // 0x0082
    AIT_REG_W   MB_HEIGHT;			                    // 0x0084
    AIT_REG_B   ALPHA_OFFSET_DIV2;		                // 0x0086
    AIT_REG_B   BETA_OFFSET_DIV2;		                // 0x0087
    AIT_REG_B   QP;				                        // 0x0088
    AIT_REG_B   CHROMA_QP_INDEX_OFFSET;		            // 0x0089
        /*-DEFINE-----------------------------------------------------*/
        #define H264_CB_QP_IDX_OFST_MASK            0x1F
        #define H264_CB_QP_IDX_OFST(_a)             (_a & H264_CB_QP_IDX_OFST_MASK)
        /*------------------------------------------------------------*/
    AIT_REG_B   DATA_INFO_REG;			                // 0x008A
        /*-DEFINE-----------------------------------------------------*/
        #define H264_I_SLICE_FLAG                   0x01
        #define H264_DIS_DBLK_IDC_MASK              0x06
        #define H264_DIS_DBLK_IDC(_a)               ((_a << 1) & H264_DIS_DBLK_IDC_MASK)
        #define H264_REC_NV12                       0x08
        #define H264_NUM_REF_IDX_L0_MINUS1_MASK     0xF0
        #define H264_NUM_REF_IDX_L0_MINUS1(_a)      (_a << 4)
        /*------------------------------------------------------------*/
    AIT_REG_B   FRAME_DATA_INFO_REG;		            // 0x008B
        /*-DEFINE-----------------------------------------------------*/
        #define H264_CONSTRAINED_INTRA_PRED_FLAG    0x01
        #define H264_CURR_NV12                      0x02
        #define H264_P_SLICE_FLAG                   0x04
        #define H264_B_SLICE_FLAG                   0x08
        /*------------------------------------------------------------*/
    AIT_REG_W   START_MB_NR;			                // 0x008C
    AIT_REG_B   START_MB_XPOS;			                // 0x008E
    AIT_REG_B   START_MB_YPOS;                          // 0x008F
    
    AIT_REG_W   ERR_CONCEALMENT;                        // 0x0090
    AIT_REG_B   VLD_CTL_REG;			                // 0x0092
        /*-DEFINE-----------------------------------------------------*/
        #define START_TO_DECODE_SLICE_DATA          0x01
        #define SET_BS_START_BIT(reg, offset)       { reg = (reg | ((offset & 0x7) << 1)); }
        #define ENABLE_SW_VLD_BUF0                  0x10
        #define ENABLE_SW_VLD_BUF1                  0x20
        #define DISABLE_SW_VLD                      0x00
        #define ENABLE_SW_VLD                       0x40
        #define DISABLE_VLD_RINGBUF                 0x00
        #define ENABLE_VLD_RINGBUF                  0x80
        /*------------------------------------------------------------*/                
    AIT_REG_B   VLD_CTL2_REG;			                // 0x0093
        /*-DEFINE-----------------------------------------------------*/
        #define ENABLE_CHECK_REF_NUM                0x01
        #define ENABLE_LATCH_SEGMENT_ADDR           0x02
        #define SET_LIST_SIZE(reg, list_size)       { reg = (AIT_REG_B)((reg & 0xF) | (list_size << 4)); } //CHECK
        /*------------------------------------------------------------*/
    AIT_REG_D   BS_START_ADDR;			                // 0x0094
    AIT_REG_D   BS_LENGTH_MINUS1;		                // 0x0098
    AIT_REG_D   VLD_RINGBUF_LOWERBOUND_ADDR;	        // 0x009C
    
    AIT_REG_D   VLD_RINGBUF_HIGHERBOUND_ADDR;	        // 0x00A0
    AIT_REG_W   CURRENT_MB_NUM;			                // 0x00A4 [RO]
    AIT_REG_B   SLICE_STATUS;       		            // 0x00A6 [RO]
    AIT_REG_B                       _x00A7;
    AIT_REG_D   BS_SHIFT_BITCOUNT;  		            // 0x00A8 [RO]	/* RESERVED | 3rd byte | 2nd byte | 1st byte */
    AIT_REG_D   SEGMENT_LENGTH;			                // 0x00AC
    
    AIT_REG_B   MB_STATUS[50/*396 / 8*/];               // 0x00B0 ~ 0x00E1
        /*-DEFINE-----------------------------------------------------*/
        #define GET_MB_VALID(VLD, i)                (((VLD->MB_STATUS[i/8]) >> (i%8)) & 0x1)
        /*------------------------------------------------------------*/
    
    AIT_REG_B                       _x00E2[0x6];
    AIT_REG_B   HIGH_PROF_REG_1;                        // 0x00E8
        /*-DEFINE-----------------------------------------------------*/
        #define H264_NUM_REF_IDX_L1_MINUS1_MASK     0xF0
        #define H264_NUM_REF_IDX_L1_MINUS1(_a)      (_a << 4)
        #define H264_NUM_REF_IDX_OVERRIDE_FLAG      0x08
        #define H264_TRANS_8X8_FLAG                 0x04
        #define H264_DIRECT_8X8_FLAG                0x02
        #define H264_CABAC_MODE_FLAG                0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   HIGH_PROF_REG_2;                        // 0x00E9
        /*-DEFINE-----------------------------------------------------*/
        #define H264_NUM_REF_FRAME_MASK             0x0F
        #define H264_FRM_MBS_ONLY_FLAG              0x10
        #define H264_MAIN_PROFILE_FLAG              0x20
        #define H264_CABAC_INIT_IDC_MASK            0xC0
        #define H264_CABAC_INIT_IDC(_a)             (_a << 6)
        /*------------------------------------------------------------*/
    AIT_REG_B   HIGH_PROF_REG_3;                        // 0x00EA
    AIT_REG_B   HIGH_PROF_REG_4;                        // 0x00EB
        /*-DEFINE-----------------------------------------------------*/
        #define H264_POC_TYPE_MASK                  0xC0
        #define H264_POC_TYPE(_a)                   (_a << 6)
        #define H264_SEQ_SCAL_LIST_PRESENT_FLAG     0x20
        #define H264_SEQ_SCAL_MATRIX_PRESENT_FLAG   0x10
        #define H264_SPS_ID_MASK                    0x0F
        /*------------------------------------------------------------*/
    AIT_REG_B   HIGH_PROF_REG_5;                        // 0x00EC
        /*-DEFINE-----------------------------------------------------*/
        #define H264_CR_QP_IDX_OFST_MASK            (0xF1)
        #define H264_CR_QP_IDX_OFST(_a)             ((_a << 3) & H264_CR_QP_IDX_OFST_MASK)
        #define H264_PIC_SCAL_LIST_PRESENT_FLAG     0x04
        #define H264_PIC_SCAL_MATRIX_PRESENT_FLAG   0x02
        #define H264_PIC_ORDER_PRESENT_FLAG         0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   HIGH_PROF_REG_6;                        // 0x00ED
        /*-DEFINE-----------------------------------------------------*/
        #define H264_PPS_ID_MASK                    0x0F
        #define H264_HIGH_PROFILE_FLAG              0x10
        /*------------------------------------------------------------*/
    AIT_REG_B   HIGH_PROF_REG_7;                        // 0x00EE
        /*-DEFINE-----------------------------------------------------*/
        #define H264_LONG_TERM_REF_FLAG             0x08
        #define H264_REF_LIST_REORDER_FLAG          0x04
        #define H264_NAL_REF_IDC_MASK               0x03
        /*------------------------------------------------------------*/                
    AIT_REG_B   HIGH_PROF_REG_8;                        // 0x00EF
        /*-DEFINE-----------------------------------------------------*/
        #define H264_REF_LIST_IDX(_l0, _l1)         ((_l1 << 4) | (_l0 & 0x0F))
        /*------------------------------------------------------------*/

} AITS_H264DEC_VLD, *AITPS_H264DEC_VLD;

//--------------------------------------------
// Parse EP3 Register Address (0x800000F0)
//--------------------------------------------
typedef struct _AITS_H264DEC_PARSE_EP3 {
    AIT_REG_D   BS_SRC_ADDR;			                // 0x00F0
    AIT_REG_D	BS_SRC_BITCOUNT;		                // 0x00F4
    AIT_REG_D   BS_DST_ADDR;			                // 0x00F8
    AIT_REG_D	BS_DST_BITCOUNT;		                // 0x00FC  [RO]
    AIT_REG_B	                    _xC900[0xF0];
    AIT_REG_D   BS_SRC_LOWBD_ADDR;		                // 0x01F0
    AIT_REG_D   BS_SRC_HIGHBD_ADDR;		                // 0x01F4
    AIT_REG_D   BS_DST_LOWBD_ADDR;		                // 0x01F8
    AIT_REG_D   BS_DST_HIGHBD_ADDR;		                // 0x01FC
} AITS_H264DEC_PARSE_EP3, *AITPS_H264DEC_PARSE_EP3;

typedef struct {
    AIT_REG_W   MVD_X;
    AIT_REG_W   MVD_Y;
} AVC_MVD;

//--------------------------------------------
// SW VLD MB Register Buffer 0 Address (0x80000100)
// SW VLD MB Register Buffer 1 Address (0x80000180)
//--------------------------------------------
typedef struct _AITS_H264DEC_MB_DATA {
    AIT_REG_B   MB_TYPE;			                    // 0x0000
        /*-DEFINE-----------------------------------------------------*/
        #define SET_MB_TYPE(reg, mb_type, is_intra) {reg = ((mb_type << 1) | is_intra); }
        /*------------------------------------------------------------*/
    AIT_REG_B   CBP;				                    // 0x0001
        /*-DEFINE-----------------------------------------------------*/
        #define SET_CBP(reg, luma_cbp, chroma_cbp) {reg = ((chroma_cbp << 4) | (luma_cbp)); }
        /*------------------------------------------------------------*/
    AIT_REG_B   QP_Y;				                    // 0x0002
    AIT_REG_B   ADJ_MB_INFO;			                // 0x0003
        /*-DEFINE-----------------------------------------------------*/
        #define SET_LEFT_MB_AVAIL(reg, is_avail)        {reg = ((reg & 0xFE) | ((uint8_t)is_avail));}
        #define SET_TOP_MB_AVAIL(reg, is_avail)         {reg = ((reg & 0xFD) | ((uint8_t)(is_avail << 1)));}
        #define SET_TOPRIGHT_MB_AVAIL(reg, is_avail)    {reg = ((reg & 0xFB) | ((uint8_t)(is_avail << 2)));}
        #define SET_TOPLEFT_MB_AVAIL(reg, is_avail)     {reg = ((reg & 0xF7) | ((uint8_t)(is_avail << 3)));}
        #define SET_LEFT_MB_INTRA(reg, is_intra)        {reg = ((reg & 0xEF) | ((uint8_t)(is_intra << 4)));}
        #define SET_TOP_MB_INTRA(reg, is_intra)         {reg = ((reg & 0xDF) | ((uint8_t)(is_intra << 5)));}
        #define SET_TOPRIGHT_MB_INTRA(reg, is_intra)    {reg = ((reg & 0xBF) | ((uint8_t)(is_intra << 6)));}
        #define SET_TOPLEFT_MB_INTRA(reg, is_intra)     {reg = ((reg & 0x7F) | ((uint8_t)(is_intra << 7)));}
        /*------------------------------------------------------------*/
    AIT_REG_W   MB_SLICE_NR;			                // 0x0004
    AIT_REG_W   MB_NR;				                    // 0x0006
    AIT_REG_W   MB_XPOS;			                    // 0x0008
    AIT_REG_W   MB_YPOS;			                    // 0x000A
    AIT_REG_B   INTRA_MB_PRED_MODE;		                // 0x000C
        /*-DEFINE-----------------------------------------------------*/
        #define SET_INTRA_16X16_PRED_MODE(reg, mode)    { reg = ((reg & 0xFC) | mode); }
        #define SET_INTRA_CHROMA_PRED_MODE(reg, mode)   { reg = ((reg & 0xF3) | ((mode & 3) << 2)); }
        /*------------------------------------------------------------*/
    AIT_REG_B	                    _x000D;
    AIT_REG_B   INTRA_4X4_PRED_MODE[16 / 2];	        // 0x000E
        /*-DEFINE-----------------------------------------------------*/
        #define SET_INTRA_4X4_PREV_PRED(reg, blk_idx, prev_mode_flag) {\
                    reg = ( reg | \
                           (prev_mode_flag << ((blk_idx & 1) * 4))    \
                          );\
                    }
        #define SET_INTRA_4X4_REM_PRED(reg, blk_idx, rem_pred_mode) {\
                    reg = ( reg | \
                           (rem_pred_mode << (1+ (blk_idx & 1)*4))    \
                          );\
                    }
        /*------------------------------------------------------------*/
    AIT_REG_W   REF_IDX;			                    // 0x0016
        /*-DEFINE-----------------------------------------------------*/
        #define SET_REF_IDX(reg,blk_idx,ref_idx) {\
                    reg = (reg) | (ref_idx << (4*blk_idx));\
                    }
        /*------------------------------------------------------------*/
    AVC_MVD     MVD[16];			                    // 0x0018
    AIT_REG_B   SUB_MB_TYPE;			                // 0x0058
    AIT_REG_B	                    _x0059;
    AIT_REG_W   CBP_Y;				                    // 0x005A
        /*-DEFINE-----------------------------------------------------*/
        #define SET_NOZERO_BLK(reg, idx, nonzero) { reg = (reg | (nonzero << idx));}
        /*------------------------------------------------------------*/
    AIT_REG_W   REF_FRM_IDX_L1;				            // 0x005C
    AIT_REG_B	                    _x005E[0x2];   
} AITS_H264DEC_MB_DATA, *AITPS_H264DEC_MB_DATA;

//--------------------------------------------
// Deblocking Rotate Mode Register Address (0x800002D0)
//--------------------------------------------
typedef struct _AITS_H264DEC_DBLK_ROT {
    AIT_REG_D   Y_LOWBD_ADDR;                           // 0x02D8
    AIT_REG_D   Y_HIGHBD_ADDR;                          // 0x02DC
    AIT_REG_D   UV_LOWBD_ADDR;                          // 0x02E0
    AIT_REG_D   UV_HIGHBD_ADDR;                         // 0x02E4
    AIT_REG_D   V_LOWBD_ADDR;                           // 0x02E8
    AIT_REG_D   V_HIGHBD_ADDR;                          // 0x02EC
    AIT_REG_D   UV_ST_ADDR;                             // 0x02F0
    AIT_REG_D   V_ST_ADDR;                              // 0x02F4
    AIT_REG_D   UP_ROW_ST_ADDR;                         // 0x02F8
    AIT_REG_W   H264_TIMEOUT_CNT;                       // 0x02FC
} AITS_H264DEC_DBLK_ROT, *AITPS_H264DEC_DBLK_ROT;

/* VLD Buffer0 / Buffer1 */
typedef struct {
    AIT_REG_W   DCT_16_DC[16];
    AIT_REG_W   DCT_4X4[16][16];
    AIT_REG_W	RESERVED[8];
    AIT_REG_W	DCT_CHROMA_DC[2*4];    
    AIT_REG_W	DCT_CHROMA_AC[8][16];
} AVC_VLD_BUF;

#define MAX_MB_MAP_ELEMENT  (396 / 4)

/* MB To Slice Map */
typedef struct {
    AIT_REG_W   MAP[MAX_MB_MAP_ELEMENT]; /* mb n+3 | mb n+2 | mb n+1 | mb n   */
                              /* 3 bits | 3 bits | 3 bits | 3 bits */
    /* NOTICE! Before use this macro, RESET REG TO ZERO */
    #define SET_MB_MAP(reg, mb_num, slice_group) { reg = ( reg | (slice_group << (3*(mb_num&3))) ); }
    /* CHECK THIS!!*/
    #define GET_MB_MAP(reg, mb_num, value) { value = ( (reg & (0x7 << (3*(mb_num&3)))) >> (3*(mb_num&3)) ); }
} AVC_MB_MAP;

#if !defined(BUILD_FW)
#define H264DEC_VLD_RINGBUF_LOWERBOUND_ADDR  (H264VLD_BASE + (MMP_ULONG)(&(((AITPS_H264DEC_VLD)0)->VLD_RINGBUF_LOWERBOUND_ADDR   )))
#define H264DEC_VLD_RINGBUF_HIGHERBOUND_ADDR (H264VLD_BASE + (MMP_ULONG)(&(((AITPS_H264DEC_VLD)0)->VLD_RINGBUF_HIGHERBOUND_ADDR  )))
#define H264_HP_REG_3                        (H264VLD_BASE + (MMP_ULONG)(&(((AITPS_H264DEC_VLD)0)->HIGH_PROF_REG_3               )))
#define H264_HP_REG_4                        (H264VLD_BASE + (MMP_ULONG)(&(((AITPS_H264DEC_VLD)0)->HIGH_PROF_REG_4               )))
#define H264_HP_REG_5                        (H264VLD_BASE + (MMP_ULONG)(&(((AITPS_H264DEC_VLD)0)->HIGH_PROF_REG_5               )))
#define H264_HP_REG_6                        (H264VLD_BASE + (MMP_ULONG)(&(((AITPS_H264DEC_VLD)0)->HIGH_PROF_REG_6               )))
#define H264_HP_REG_7                        (H264VLD_BASE + (MMP_ULONG)(&(((AITPS_H264DEC_VLD)0)->HIGH_PROF_REG_7               )))
#endif

#endif  /* _MMP_REG_H264DEC_H_ */