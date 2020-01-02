//==============================================================================
//
//  File        : mmp_reg_h264enc.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Hsin-Wei Yang
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMP_REG_H264ENC_H_
#define _MMP_REG_H264ENC_H_

///@ait_only

#include "mmp_register.h"

/** @addtogroup MMPH_reg
@{
*/

//--------------------------------------------
// H264 Encoder Register Address (0x80000400)
//--------------------------------------------
typedef struct _AITS_H264ENC {
    
    // GLOBAL
    AIT_REG_B   H264ENC_FRAME_CTL;                      // 0x0000
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_OVWR_STRICT_CHK               0x01
        #define H264_SPEED_UP                       0x02
        #define H264_DVS_EN                         0x04
        #define H264_CUR_PP_TNR_EN                  0x08
        #define H264_CUR_PP_MODE_EN                 0x10
        #define H264E_128B_CNT_MODE                 0x20
        #define H264E_256B_CNT_MODE					0x00
        #define H264E_URGENT_BY_PP                  0x40
        #define H264E_URGENT_BY_TB                  0x80
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x0001;
    AIT_REG_B   H264ENC_SW_RST;                         // 0x0002
        /*-DEFINE-----------------------------------------------------*/
        #define H264_VLC_RESET                      0x01
        #define H264_TRANSFORM_RESET                0x02
        #define H264_INTRA_PRED_RESET               0x04
        #define H264_INTER_PRED_RESET               0x08
        #define H264_FRAME_RECONSTRUCT_RESET        0x10
        #define H264_QP_FINETUNE_RESET              0x20
        #define H264_FLOW_CTRL_RESET                0x40
        #define H264_BUS_IF_RESET                   0x80
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x0003;
    AIT_REG_W   H264ENC_INT_CPU_EN;                     // 0x0004
    AIT_REG_W   H264ENC_INT_CPU_SR;                     // 0x0006
    AIT_REG_W   H264ENC_INT_HOST_EN;                    // 0x0008
    AIT_REG_W   H264ENC_INT_HOST_SR;                    // 0x000A
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_INT_SPS_PPS_DONE              0x0001
        #define H264E_INT_SLICE_DONE                0x0002
        #define H264E_INT_SKIP_FRAME_DONE           0x0004
        #define H264E_INT_EP_BYTE                   0x0008
        #define H264E_INT_TRANS_MB_DONE             0x0020
        #define H264E_INT_TRANS_BLK_DONE            0x0040
        #define H264E_INT_TRANS_CHROMA_DONE         0x0080
        
        #define H264E_INT_FRAME_ENC_DONE            0x0100
        #define H264E_INT_INTRA_MB_DONE             0x0200
        #define H264E_INT_DVS_MODE_FRM_DONE         0x0400
		#define H264E_INT_CUR_LINEBUF_OVWR          0x0800
		#define H264E_INT_ICOMP_LUMA_OVLP           0x1000
		#define H264E_INT_ICOMP_CHROMA_OVLP         0x2000
        /*------------------------------------------------------------*/
    AIT_REG_B                       _0x000C[0x4];

    AIT_REG_W   H264ENC_SLICE_ARG;                      // 0x0010
    AIT_REG_B   H264ENC_SLICE_MODE;                     // 0x0012
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_SLICE_MODE_MASK               0x03
        #define H264E_SLICE_MODE_FRM                0x00
        #define H264E_SLICE_MODE_MB                 0x01
        #define H264E_SLICE_MODE_BYTE               0x02
        /*------------------------------------------------------------*/
    AIT_REG_B                       _0x0013;
    AIT_REG_D   H264ENC_QP_SUM;                         // 0x0014 [RO]
    AIT_REG_D   H264ENC_ROI_QP_SUM;                     // 0x0018 [RO]
    AIT_REG_D   H264ENC_MAD;                            // 0x001C
    
    AIT_REG_W   H264ENC_SLICE_STUFF_BYTES;              // 0x0020
    AIT_REG_B                       _x0022[0x2];
    AIT_REG_W   H264ENC_CODED_SLICE_NUM;                // 0x0024 [RO]
    AIT_REG_W   H264ENC_ROI_MB_NUM;                     // 0x0026 [RO]
    AIT_REG_B                       _x0028[0x8];

    // VLC
    AIT_REG_B   H264ENC_VLC_CTRL1;                      // 0x0030
        /*-DEFINE-----------------------------------------------------*/
        #define ENCODE_CURR_FRAME_AS_SKIP          	0x01
        #define ENCODE_SPS_PPS_ONLY                 0x02
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_VLC_CTRL2;                      // 0x0031
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_INSERT_EP3_EN                 0x01
        #define INSERT_SPS_PPS_BEFORE_I_SLICE       0x02
        #define H264E_INSERT_SEI_BEFORE_SPSPPS      0x04
        #define H264E_INSERT_SEI_AFTER_SPSPPS       0x08
        #define H264E_INSERT_PREFIX_NALU            0x10
        #define H264E_SEI_COHDR_EN                  0x20
        #define H264E_PREFIX_NALU_COHDR_EN          0x40
        #define H264E_SLICE_COHDR_EN                0x80
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_VLC_CTRL3;                      // 0x0032
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_HDR_INSERT_EP3_EN             0x01
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x0033;
    AIT_REG_D   H264ENC_BS_START_ADDR;                  // 0x0034
    AIT_REG_D   H264ENC_BS_LOWER_BOUND_ADDR;            // 0x0038
    AIT_REG_D   H264ENC_BS_UPPER_BOUND_ADDR;            // 0x003C
    
    AIT_REG_D   H264ENC_SLICE_LEN_BUF_ADDR;             // 0x0040
    AIT_REG_D   H264ENC_INTRAMB_BITS_COUNT;             // 0x0044 [RO]
    AIT_REG_D   H264ENC_MV_BITS_COUNT;                  // 0x0048 [RO]
    AIT_REG_D   H264ENC_HEADER_BITS_COUNT;              // 0x004C [RO]
    
    AIT_REG_D   H264ENC_FRAME_LENGTH;                   // 0x0050 [RO]
    AIT_REG_W   H264ENC_ROI_SIZE_LSB;                   // 0x0054
    AIT_REG_B   H264ENC_ROI_SIZE_MSB;                   // 0x0056
    AIT_REG_B   H264ENC_VLC_SR;                         // 0x0057 [RO]
    AIT_REG_D   H264ENC_BS_TURNAROUND_ADDR;             // 0x0058
    AIT_REG_B                       _x005C[0x4];
    
    AIT_REG_B                       _x0060[0x34];
    
    // CUR/REF BUFFER
    AIT_REG_D   H264ENC_CURR_Y_ADDR;                    // 0x0094
    AIT_REG_D   H264ENC_CURR_U_ADDR;                    // 0x0098
    AIT_REG_D   H264ENC_CURR_V_ADDR;                    // 0x009C
    
    AIT_REG_D   H264ENC_REFBD_Y_LOW;                    // 0x00A0
    AIT_REG_D   H264ENC_REFBD_Y_HIGH;                   // 0x00A4
    AIT_REG_D   H264ENC_REFBD_UV_LOW;                   // 0x00A8
    AIT_REG_D   H264ENC_REFBD_UV_HIGH;                  // 0x00AC
    
    AIT_REG_B                       _x00B0[0x8];
    AIT_REG_D   H264ENC_REF_Y_ADDR;                     // 0x00B8
    AIT_REG_D   H264ENC_REF_UV_ADDR;                    // 0x00BC
    
    AIT_REG_B                       _x00C0[0x18];
    
    // INTRA/INTER DECISION
    AIT_REG_W   H264ENC_ME_REFINE_COUNT;                // 0x00D8
    AIT_REG_W   H264ENC_ME_PART_LIMIT_COUNT;            // 0x00DA
    AIT_REG_W   H264ENC_ME_PART_COST_THRES;             // 0x00DC
    AIT_REG_W   H264ENC_ME_INTER_COST_THRES;            // 0x00DE
    
    AIT_REG_B   H264ENC_INTRA_COST_BIAS;                // 0x00E0
    AIT_REG_B   H264ENC_INTRA_COST_THRES;               // 0x00E1
    AIT_REG_W   H264ENC_INTER_COST_BIAS;                // 0x00E2
	AIT_REG_W	H264ENC_INTER_COST_SKIP;	            // 0x00E4
	AIT_REG_B                       _x00E6[0xA];
    
    AIT_REG_D   H264ENC_FIRST_P_AFTER_I_IDX;            // 0x00F0
    	/*-DEFINE-----------------------------------------------------*/
        #define H264E_COLOCATE_MV_REF_DIS           0x01
   		/*------------------------------------------------------------*/
    AIT_REG_D   H264ENC_MV_BUFF_START_ADDR;             // 0x00F4
    AIT_REG_B                       _x00F8[0x2];
    AIT_REG_B   H264ENC_INTER_CTL;                      // 0x00FA
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_MB_16X8_DIS                   0x04
        #define H264E_MB_8X16_DIS                   0x08
        #define H264E_MB_8X8_DIS                    0x10
        #define H264E_MB_4X4_DIS                    0x20
        #define H264E_MB_TYPE_MASK                  0x3C
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x00FB[0x5];

    // TRANSFORM
    AIT_REG_B   H264ENC_TRANS_CTL;                      // 0x0100
        /*-DEFINE-----------------------------------------------------*/
        #define CBP_CTL_EN                          0x01
        #define TRANS_8x8_FLAG                      0x02
        #define H264E_NS_MODE_EN                    0x04
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_NS_MAX_QP;                      // 0x0101
    AIT_REG_B   H264ENC_LUMA_COEFF_COST;                // 0x0102
    AIT_REG_B   H264ENC_CHROMA_COEFF_COST;              // 0x0103
    AIT_REG_W   H264ENC_COST_LARGER_THAN_ONE;           // 0x0104
    AIT_REG_B                       _x0106[0xA];

    // SLICE HEADER
    AIT_REG_W   H264ENC_FRAME_NUM;                      // 0x0110
    AIT_REG_W   H264ENC_IDR_FRAME;                      // 0x0112
    AIT_REG_B   H264ENC_REF_LIST0_REORDER_CTL0;         // 0x0114
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_ABS_DIFF_PIC_NUM_MINUS1(_a)   ((_a & 0xF) << 4)
        #define H264E_REORDER_OF_PIC_NUMS_IDC(_a)   (_a & 0xF)
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x0115;
    AIT_REG_W   H264ENC_POC;                            // 0x0116
    AIT_REG_B                       _x0118[0x8];
    
    // SPS
    AIT_REG_B   H264ENC_LEVEL_IDC;                      // 0x0120
    AIT_REG_B   H264ENC_MAX_FRAMENUM_AND_POC;           // 0x0121
        /*-DEFINE-----------------------------------------------------*/
        #define SET_LOG2_MAX_FRAMENUM_MINUS_4(X,Y)  X = (X | (Y & 0xF))
        #define SET_LOG2_MAX_POC_MINUS_4(X,Y)       X = (X | ((Y & 0xF) << 4))
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_GAPS_IN_FRAME_NUM_ALLOWED;      // 0x0122
        /*-DEFINE-----------------------------------------------------*/
        #define GAPS_IN_FRAME_NUM_DIS               0x00
        #define GAPS_IN_FRAME_NUM_EN                0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_PROFILE_IDC;                    // 0x0123
    AIT_REG_W   H264ENC_FRAME_CROPPING_FLAG;            // 0x0124
        /*-DEFINE-----------------------------------------------------*/
        #define FRAME_CROPPING_DIS                  0x00
        #define FRAME_CROPPING_EN                   0x01
        /*------------------------------------------------------------*/    
    AIT_REG_B   H264ENC_FRAME_CROPPING_LEFT;            // 0x0126
    AIT_REG_B   H264ENC_FRAME_CROPPING_RIGHT;           // 0x0127
    AIT_REG_B   H264ENC_FRAME_CROPPING_TOP;             // 0x0128
    AIT_REG_B   H264ENC_FRAME_CROPPING_BOTTOM;          // 0x0129
    AIT_REG_B                       _x012A[0x6];
    
    // PPS
    AIT_REG_B   H264ENC_DBLK_CTRL_PRESENT;              // 0x0130
        /*-DEFINE-----------------------------------------------------*/
        #define H264ENC_DBLK_DIS                    0x00
        #define H264ENC_DBLK_EN                     0x01
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x0131[0xF];

    // INTRA
    AIT_REG_B   H264ENC_INTRA_PRED_MODE;                // 0x0140
        /*-DEFINE-----------------------------------------------------*/
        #define INTRA_PRED_IN_INTER_SLICE_EN        0x01
        #define ONLY_16X16_INTRA_PRED_EN            0x02
        #define INTRA_8x8_EN                        0x04
        #define INTRA_6_MODE_EN                     0x08
        #define FAST_INTRA_6_MODE_EN                0x10
        #define INTRA_ONLY_8x8_16X16_EN             0x20
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_INTRA_16x16_COST_ADJ;           // 0x0141
    AIT_REG_W   H264ENC_INTRA_REFRESH_PERIOD;           // 0x0142
    AIT_REG_W   H264ENC_INTRA_REFRESH_OFFSET;           // 0x0144
    AIT_REG_B   H264ENC_INTRA_REFRESH_MODE;             // 0x0146
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_INTRA_REFRESH_DIS             0x00
        #define H264E_INTRA_REFRESH_MB_MODE         0x01
        #define H264E_INTRA_REFRESH_ROW_MODE        0x02
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x0147[0x9];

    // QP FINE TUNE
    AIT_REG_B   H264ENC_QP_FINE_TUNE_EN;                // 0x0150
        /*-DEFINE-----------------------------------------------------*/
        #define QP_FINETUNE_DIS                     0x00
        #define QP_FINETUNE_ROW_MODE_EN             0x01
        #define QP_FINETUNE_SLICE_MODE_EN           0x03
        #define QP_FINETUNE_MB_MODE_EN              0x05
        #define H264E_FRM_MAD_EN                    0x08
        #define H264E_RDO_EN                        0x10
        #define H264E_HBR_QP_EN                     0x20
        #define H264E_ROI_MODE_CQP                  0x40
        #define H264E_ROI_MODE_RC                   0x80
        #define H264E_ROI_MODE_MASK                 0xC0
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x0151;
    AIT_REG_B   H264ENC_QP_BASICUNIT_SIZE;              // 0x0152
    AIT_REG_B   H264ENC_ROI_BASIC_FIX_QP;               // 0x0153
    AIT_REG_W   H264ENC_TGT_MB_SIZE_X256_LSB;           // 0x0154
    AIT_REG_B   H264ENC_TGT_MB_SIZE_X256_MSB;           // 0x0156
    AIT_REG_B   H264ENC_ROI_TGT_MB_SIZE_X256_MSB;       // 0x0157
    AIT_REG_B   H264ENC_BASIC_QP;                       // 0x0158
    AIT_REG_B   H264ENC_QP_TUNE_STEP;                   // 0x0159
    AIT_REG_B   H264ENC_QP_UP_BOUND;                    // 0x015A
    AIT_REG_B   H264ENC_QP_LOW_BOUND;                   // 0x015B
    AIT_REG_W   H264ENC_ROI_TGT_MB_SIZE_X256_LSB;       // 0x015C
    AIT_REG_B   H264ENC_ROI_QP_UP_BOUND;                // 0x015E
    AIT_REG_B   H264ENC_ROI_QP_LOW_BOUND;               // 0x015F

    // ME THRESHOLD
    AIT_REG_W   H264ENC_ME_EARLY_STOP_THRES;            // 0x0160
    AIT_REG_W   H264ENC_ME_STOP_THRES_UPPER_BOUND;      // 0x0162
    AIT_REG_W   H264ENC_ME_STOP_THRES_LOWER_BOUND;      // 0x0164
    AIT_REG_W   H264ENC_ME_SKIP_THRES_UPPER_BOUND;      // 0x0166
    AIT_REG_W   H264ENC_ME_SKIP_THRES_LOWER_BOUND;      // 0x0168
    AIT_REG_W   H264ENC_ME_NO_SUBBLOCK;                 // 0x016A
        /*-DEFINE-----------------------------------------------------*/
        #define REFINE_PART_NO_SUBBLK               0x03
        /*------------------------------------------------------------*/
    AIT_REG_W   H264ENC_ME_COMPLEXITY;                  // 0x016C
    AIT_REG_W   H264ENC_ME_INTER_CTL;                   // 0x016E
        /*-DEFINE-----------------------------------------------------*/
        #define INTER_8_PIXL_ONLY                   0x01
        #define INTER_Y_LINE_SPLIT                  0x02
        #define SKIP_CAND_INCR_ME_STEP              0x04
        #define INTER_PIPE_MODE_EN                  0x08
        #define SKIP_CAND_INCR_ME_STEP_IME          0x10
        #define DIS_2PIX_SEARCH_FME                 0x20
        /*------------------------------------------------------------*/
    
    AIT_REG_B   H264ENC_ME16X16_MAX_MINUS_1;            // 0x0170
    AIT_REG_B   H264ENC_IME16X16_MAX_MINUS_1;           // 0x0171
    AIT_REG_B   H264ENC_IME_COST_WEIGHT0;               // 0x0172
    AIT_REG_B   H264ENC_IME_COST_WEIGHT1;               // 0x0173
    AIT_REG_B   H264ENC_PMV_CTL;                        // 0x0174
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_COLOCATE_MV_DIS               0x08
        #define H264E_PMV_SH_CNT_MASK               0x07
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_BIPRED_COST_ADJUST;             // 0x0175
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_BIPRED_COST_ADJ_MASK          0x1F
        /*------------------------------------------------------------*/
    AIT_REG_W   H264ENC_BIPRED_MB_COUNT;                // 0x0176
    AIT_REG_B   H264ENC_DIRECT_COST_ADJUST;             // 0x0178
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_DIS_DIRECT_MODE               0x20
        #define H264E_DIRECT_COST_ADJ_MASK          0x1F
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_MV_RANGE_OPT;                   // 0x0179
	    /*-DEFINE-----------------------------------------------------*/
		#define H264E_MV_RANGE_Y_46                 0x04 // +-46
		#define H264E_MV_RANGE_Y_46_40              0x09 // -46 ~ +40
		#define H264E_MV_RANGE_Y_14                 0x0E
		#define H264E_MV_RANGE_Y_14_8               0x0F // -14 ~ +8
		#define H264E_MV_RANGE_X_2046               0x00
		#define H264E_MV_RANGE_X_1024               0x10
		#define H264E_MV_RANGE_X_512                0x20
		#define H264E_MV_RANGE_X_256                0x30
		/*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_PADDING_CNT;                    // 0x017A
	AIT_REG_B   H264ENC_PADDING_MODE_SWT;               // 0x017B
        /*-DEFINE-----------------------------------------------------*/
        #define ZERO_PADDING_MODE               	0x01
        #define REPEAT_PADDING_MODE               	0x02
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x017C[0x18];
    
    AIT_REG_D   H264ENC_CURR_Y_ADDR1;                   // 0x0194
    AIT_REG_D   H264ENC_CURR_U_ADDR1;                   // 0x0198
    AIT_REG_D   H264ENC_CURR_V_ADDR1;                   // 0x019C
    
    AIT_REG_B                       _x01A0[0x40];       // Skip Data Group and VBR Information 
    AIT_REG_W   H264ENC_INTRAMB_NUM;                    // 0x01E0
    AIT_REG_B                       _x01E2[0x1E];
    
    // ROI WINDOW
    AIT_REG_B   H264ENC_ROIWIN_ST_X[8];                 // 0x0200
    AIT_REG_B   H264ENC_ROIWIN_ED_X[8];                 // 0x0208
    
    AIT_REG_B   H264ENC_ROIWIN_ST_Y[8];                 // 0x0210
    AIT_REG_B   H264ENC_ROIWIN_ED_Y[8];                 // 0x0218
    
    // CO-HEADER
    AIT_REG_W   H264ENC_COHDR_SEI_LEN;                  // 0x0220
    AIT_REG_B   H264ENC_COHDR_PNALU_LEN;                // 0x0222
    AIT_REG_B   H264ENC_COHDR_SLICE_LEN;                // 0x0223
    AIT_REG_B   H264ENC_COHDR_SEI_DAT_40B_ST;           // 0x0224
    AIT_REG_B                       _x0225[39];
    
    AIT_REG_B   H264ENC_COHDR_PNALU_DAT_10B_ST;         // 0x024C
    AIT_REG_B                       _x024D[0x9];
    
    AIT_REG_B   H264ENC_COHDR_SLICE_DAT_10B_ST;         // 0x0256
    AIT_REG_B                       _x0257[0x9];
    
    // TIME BUDGET
    AIT_REG_W   H264ENC_TB_ROW_BUDGET;                  // 0x0260
    AIT_REG_W   H264ENC_TB_FRM_BUDGET;                  // 0x0262
    AIT_REG_B   H264ENC_TB_ROI_QP_UP;                   // 0x0264
    AIT_REG_B   H264ENC_TB_ROI_QP_LOW;                  // 0x0265
    AIT_REG_B   H264ENC_TB_QP_UP;                       // 0x0266
    AIT_REG_B   H264ENC_TB_QP_LOW;                      // 0x0267
    AIT_REG_B   H264ENC_TB_EN;                          // 0x0268
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_TIMING_BUDGET_EN              0x01
        /*------------------------------------------------------------*/
    AIT_REG_B                       _x0269[0x3];
    
    // RDO, COLOR NOISE
    AIT_REG_D   H264ENC_PREV_FRM_MAD;                   // 0x026C
    AIT_REG_W   H264ENC_PREV_AVG_MB_MAD;                // 0x0270
    AIT_REG_W   H264ENC_RDO_FRM_INIT_FAC;               // 0x0272
    AIT_REG_B   H264ENC_QSTEP_2[6];                     // 0x0274
    AIT_REG_B   H264ENC_QSTEP_3[5];                     // 0x027A
    AIT_REG_B                       _x027F;
    
    AIT_REG_D   H264ENC_LAST_MB_ROW_MAD;                // 0x0280 [RO]
        /*-DEFINE-----------------------------------------------------*/
        #define H264ENC_LAST_MB_ROW_MAD_MASK        (0x00FFFFFF)
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_COLOR_NOISE_YCOMPTH;            // 0x0284
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_COLOR_NOISE_EN                0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_COLOR_NOISE_CBCRCOMPTH;         // 0x0285
    AIT_REG_B                       _x0286[0x2];
    
    // I2MANY FUNCTION, RDO MISS
    AIT_REG_B	H264ENC_I2MANY_START_MB_LOW;		    // 0x0288
    AIT_REG_B	H264ENC_I2MANY_START_MB_HIGH;		    // 0x0289
        /*------------------------------------------------------------*/
        #define H264E_I2MANY_LO_BYTE_MASK           0xFF
        #define H264E_I2MANY_HI_BYTE_MASK           0xF
        /*------------------------------------------------------------*/
    AIT_REG_B	H264ENC_I2MANY_QPSTEP_TABLE;		    // 0x028A
    AIT_REG_B	H264ENC_RDO_MISS_TUNE_QP;  			    // 0x028B
    AIT_REG_W	H264ENC_RDO_MISS_THR_LO;			    // 0x028C
    AIT_REG_B	H264ENC_RDO_MISS_THR_HI;			    // 0x028E
        /*-DEFINE-----------------------------------------------------*/
        #define H264ENC_RDO_MISS_THR_LO_MASK        0xFFF
        #define H264ENC_RDO_MISS_THR_HI_MASK        0xF
        /*------------------------------------------------------------*/
    AIT_REG_B                       _0x028F;
	
	// TNR FUNCTION
    AIT_REG_B	TNR_ENABLE;             	            // 0x0290
        /*-DEFINE-----------------------------------------------------*/
        #define TNR_MCTF_ENABLE                     0x01
        #define TNR_ZERO_MV_FUNC_ENABLE             0x02
        #define TNR_INTRA_MB_FUNC_ENABLE            0x04
        /*------------------------------------------------------------*/
    AIT_REG_B   TNR_MCTF_MVX_THR_LOW;                   // 0x0291
    AIT_REG_B   TNR_MCTF_MVX_THR_HIGH;                  // 0x0292
    AIT_REG_B   TNR_MCTF_MVY_THR_LOW;                   // 0x0293
    AIT_REG_B   TNR_MCTF_MVY_THR_HIGH;                  // 0x0294
    AIT_REG_B   TNR_MCTF_SATD_THR_LOW;                  // 0x0295
    AIT_REG_B   TNR_MCTF_SATD_THR_HIGH;                 // 0x0296
    AIT_REG_B   TNR_ZERO_MV_LUMA_PIXEL_DIFF_THR;        // 0x0297
    AIT_REG_B   TNR_ZERO_MV_CHRO_PIXEL_DIFF_THR;        // 0x0298
    AIT_REG_B   TNR_ZERO_MV_THR_4X4;                    // 0x0299
        /*-DEFINE-----------------------------------------------------*/
        #define TNR_ZERO_MV_CHRO_THR_4X4_MASK       0xF0
        #define TNR_ZERO_MV_LUMA_THR_4X4_MASK       0x0F
        /*------------------------------------------------------------*/
    AIT_REG_B   TNR_ZERO_MVX_THR_LOW;                   // 0x029A
    AIT_REG_B   TNR_ZERO_MVX_THR_HIGH;                  // 0x029B
    AIT_REG_B   TNR_ZERO_MVY_THR_LOW;                   // 0x029C
    AIT_REG_B   TNR_ZERO_MVY_THR_HIGH;                  // 0x029D
    AIT_REG_B   TNR_MCTF_FIL_PARA_LUMA_4X4;             // 0x029E
    AIT_REG_B   TNR_MCTF_FIL_PARA_CHRO_4X4;             // 0x029F
    
    AIT_REG_B   TNR_MCTF_FIL_PARA_LUMA_8X8_LOW;         // 0x02A0
    AIT_REG_B   TNR_MCTF_FIL_PARA_LUMA_8X8_HIGH;        // 0x02A1
    AIT_REG_B   TNR_ZERO_MV_FIL_PARA_LUMA_4X4;          // 0x02A2
    AIT_REG_B   TNR_ZERO_MV_FIL_PARA_CHRO_4X4;          // 0x02A3
    AIT_REG_B   TNR_ZERO_MV_FIL_PARA_LUMA_8X8_LOW;      // 0x02A4
    AIT_REG_B   TNR_ZERO_MV_FIL_PARA_LUMA_8X8_HIGH;     // 0x02A5
    AIT_REG_B   TNR_INTRA_MB_FIL_PARA_LUMA_4X4;         // 0x02A6
    AIT_REG_B   TNR_INTRA_MB_FIL_PARA_CHRO_4X4;         // 0x02A7
    AIT_REG_B   TNR_INTRA_MB_FIL_PARA_CHRO_8X8_LOW;     // 0x02A8
    AIT_REG_B   TNR_INTRA_MB_FIL_PARA_CHRO_8X8_HIGH;    // 0x02A9

    // HIGH BITRATE
    AIT_REG_W	H264ENC_HBR_TABLE_0;    			    // 0x02AA
    AIT_REG_W	H264ENC_HBR_TABLE_1;    			    // 0x02AC
    AIT_REG_W	H264ENC_HBR_TABLE_2;    			    // 0x02AE
    
    AIT_REG_W	H264ENC_HBR_TABLE_3;    			    // 0x02B0
    AIT_REG_W	H264ENC_HBR_TABLE_4;    			    // 0x02B2
    AIT_REG_W	H264ENC_HBR_TABLE_5;    			    // 0x02B4
    AIT_REG_W	H264ENC_HBR_TABLE_6;    			    // 0x02B6
    AIT_REG_W	H264ENC_HBR_TABLE_7;    			    // 0x02B8
    AIT_REG_B                       _0x02BA[0x6];

    // IMAGE COMPRESSION
    AIT_REG_B   H264ENC_ICOMP_CTL;                      // 0x02C0
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_ICOMP_ENABLE                  0x01
        #define H264E_ICOMP_LSY_LVL_CTL_EN          0x02
        #define H264E_ICOMP_CUR_MB_LSY_EN           0x04
        /*------------------------------------------------------------*/
    AIT_REG_B                       _0x02C1;
    AIT_REG_W   H264ENC_ICOMP_LSY_LVL;                  // 0x02C2
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_ICOMP_Y_MIN_LSY_LVL_MSK       0x0007
        #define H264E_ICOMP_CBCR_MIN_LSY_LVL_MSK    0x0038
        #define H264E_ICOMP_Y_INI_LSY_LVL_MSK       0x01C0
        #define H264E_ICOMP_CBCR_INI_LSY_LVL_MSK    0x0E00
        /*------------------------------------------------------------*/
    AIT_REG_D   H264ENC_ICOMP_LSY_Y_MB_ROW_BGT;         // 0x02C4
    AIT_REG_D   H264ENC_ICOMP_LSY_CBCR_MB_ROW_BGT;      // 0x02C8
    AIT_REG_B                       _0x02CC[0x8];

    AIT_REG_D   H264ENC_ICOMP_Y_OVFLOW_BGT;             // 0x02D4
    AIT_REG_D   H264ENC_ICOMP_CBCR_OVFLOW_BGT;          // 0x02D8
    AIT_REG_D   H264ENC_ICOMP_Y_LSY_LVL_INCR_THR;       // 0x02DC
    
    AIT_REG_D   H264ENC_ICOMP_CBCR_LSY_LVL_INCR_THR;    // 0x02E0
    AIT_REG_D   H264ENC_ICOMP_Y_LSY_LVL_DECR_THR;       // 0x02E4
    AIT_REG_D   H264ENC_ICOMP_CBCR_LSY_LVL_DECR_THR;    // 0x02E8
    AIT_REG_B   H264ENC_ICOMP_Y_LSY_LVL_TABLE;          // 0x02EC
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_ICOMP_Y_LSY_BLK_SIZE_SEL      0x01
        #define H264E_ICOMP_Y_LSY_YUVRNG_MSK        0x1E
        #define H264E_ICOMP_Y_LSY_LVL_MAX_MSK       0xE0
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_ICOMP_CBCR_LSY_LVL_TABLE;       // 0x02ED
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_ICOMP_CBCR_LSY_BLK_SIZE_SEL   0x01
        #define H264E_ICOMP_CBCR_LSY_YUVRNG_MSK     0x1E
        #define H264E_ICOMP_CBCR_LSY_LVL_MAX_MSK    0xE0
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_ICOMP_Y_MEAN;                   // 0x02EE
    AIT_REG_B   H264ENC_ICOMP_CBCR_MEAN;                // 0x02EF
    
    AIT_REG_B   H264ENC_ICOMP_Y_CONTRAST[4];            // 0x02F0
    AIT_REG_B   H264ENC_ICOMP_CBCR_CONTRAST[4];         // 0x02F4
    AIT_REG_B   H264ENC_ICOMP_Y_DIFF_THR[4];            // 0x02F8
    AIT_REG_B   H264ENC_ICOMP_CBCR_DIFF_THR[4];         // 0x02FC
    
    AIT_REG_D   H264ENC_ICOMP_DECOMP_Y_LENGTH;          // 0x0300
    AIT_REG_D   H264ENC_ICOMP_DECOMP_CBCR_LENGTH;       // 0x0304
    AIT_REG_B   H264ENC_ICOMP_DYNAMIC_LSY_MAX_LVL;      // 0x0308
    AIT_REG_B                       _0x0309[0x7];

    AIT_REG_B   H264ENC_ICOMP_Y_MB_X_OVERFLOW;          // 0x0310
    AIT_REG_B   H264ENC_ICOMP_Y_MB_Y_OVERFLOW;          // 0x0311
    AIT_REG_B   H264ENC_ICOMP_CBCR_MB_X_OVERFLOW;       // 0x0312
    AIT_REG_B   H264ENC_ICOMP_CBCR_MB_Y_OVERFLOW;       // 0x0313
    AIT_REG_B   H264ENC_ICOMP_Y_MB_X;                   // 0x0314
    AIT_REG_B   H264ENC_ICOMP_Y_MB_Y;                   // 0x0315
    AIT_REG_B   H264ENC_ICOMP_CBCR_MB_X;                // 0x0316
    AIT_REG_B   H264ENC_ICOMP_CBCR_MB_Y;                // 0x0317
    AIT_REG_W   H264ENC_ICOMP_Y_VLC_BYTE_NUM_LSB;       // 0x0318
    AIT_REG_B   H264ENC_ICOMP_Y_VLC_BYTE_NUM_MSB;       // 0x031A
    AIT_REG_B   H264ENC_ICOMP_OVERFLOW_FLAG;            // 0x031B
        /*-DEFINE-----------------------------------------------------*/
        #define H264ENC_ICOMP_Y_OVWR                0x01
        #define H264ENC_ICOMP_CBCR_OVWR             0x02
        /*------------------------------------------------------------*/
    AIT_REG_W   H264ENC_ICOMP_CBCR_VLC_BYTE_NUM_LSB;    // 0x031C
    AIT_REG_B   H264ENC_ICOMP_CBCR_VLC_BYTE_NUM_MSB;    // 0x031E
    AIT_REG_B   H264ENC_ICOMP_MSB_MB_LOC;               // 0x031F
        /*-DEFINE-----------------------------------------------------*/
        #define H264ENC_ICOMP_MSB_Y_MB_X_OVFLOW     0x01
        #define H264ENC_ICOMP_MSB_Y_MB_Y_OVFLOW     0x02
        #define H264ENC_ICOMP_MSB_CBCR_MB_X_OVFLOW  0x04
        #define H264ENC_ICOMP_MSB_CBCR_MB_Y_OVFLOW  0x08
        #define H264ENC_ICOMP_MSB_Y_MB_X            0x10
        #define H264ENC_ICOMP_MSB_Y_MB_Y            0x20
        #define H264ENC_ICOMP_MSB_CBCR_MB_X         0x40
        #define H264ENC_ICOMP_MSB_CBCR_MB_Y         0x80
        /*------------------------------------------------------------*/
        
    AIT_REG_B   H264ENC_ICOMP_Y_MB_X_DECOMP_LEN_HIT;        // 0x0320
    AIT_REG_B   H264ENC_ICOMP_T_MB_Y_DECOMP_LEN_HIT;        // 0x0321
    AIT_REG_B   H264ENC_ICOMP_CBCR_MB_X_DECOMP_LEN_HIT;     // 0x0322
    AIT_REG_B   H264ENC_ICOMP_CBCR_MB_Y_DECOMP_LEN_HIT;     // 0x0323
    AIT_REG_B   H264ENC_ICOMP_Y_MB_X_DECOMP;                // 0x0324
    AIT_REG_B   H264ENC_ICOMP_Y_MB_Y_DECOMP;                // 0x0325
    AIT_REG_B   H264ENC_ICOMP_CBCR_MB_X_DECOMP;             // 0x0326
    AIT_REG_B   H264ENC_ICOMP_CBCR_MB_Y_DECOMP;             // 0x0327
    AIT_REG_W   H264ENC_ICOMP_DECOMP_Y_VLD_BYTE_NUM_LSB;    // 0x0328
    AIT_REG_B   H264ENC_ICOMP_DECOMP_Y_VLD_BYTE_NUM_MSB;    // 0x032A
    AIT_REG_B   H264ENC_ICOMP_DECOMP_FRM_LEN_HIT_FLAG;      // 0x032B
    AIT_REG_W   H264ENC_ICOMP_DECOMP_CBCR_VLD_BYTE_NUM_LSB; // 0x032C
    AIT_REG_B   H264ENC_ICOMP_DECOMP_CBCR_VLD_BYTE_NUM_MSB; // 0x032E
    AIT_REG_B   H264ENC_ICOMP_MSB_MB_LOC_DECOMP;            // 0x032F
        /*-DEFINE-----------------------------------------------------*/
        #define H264ENC_ICOMP_MSB_Y_MB_X_DECOMP_FRM_LEN_HIT     0x01
        #define H264ENC_ICOMP_MSB_Y_MB_Y_DECOMP_FRM_LEN_HIT     0x02
        #define H264ENC_ICOMP_MSB_CBCR_MB_X_DECOMP_FRM_LEN_HIT  0x04
        #define H264ENC_ICOMP_MSB_CBCR_MB_Y_DECOMP_FRM_LEN_HIT  0x08
        #define H264ENC_ICOMP_MSB_Y_MB_X_DECOMP                 0x10
        #define H264ENC_ICOMP_MSB_Y_MB_Y_DECOMP                 0x20
        #define H264ENC_ICOMP_MSB_CBCR_MB_X_DECOMP              0x40
        #define H264ENC_ICOMP_MSB_CBCR_MB_Y_DECOMP              0x80
        /*------------------------------------------------------------*/

} AITS_H264ENC, *AITPS_H264ENC;

/// @}

#endif// _MMPH_REG_H264ENC_H_

///@end_ait_only