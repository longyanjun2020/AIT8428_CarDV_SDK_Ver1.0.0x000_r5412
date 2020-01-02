//==============================================================================
//
//  File        : mmpf_h264enc.h
//  Description : Header function of video codec
//  Author      : Will Tseng
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_H264ENC_H_
#define _MMPF_H264ENC_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "includes_fw.h"
#include "mmpf_3gpmgr.h"
#include "mmp_vidrec_inc.h"

/** @addtogroup MMPF_VIDEO
@{
*/

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================

#define FPS_RC_CTL              (1) // Comment: Suggest to close it if no need to dynamic change FPS.

#define MIO_MOSAIC_TEST         (0)

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================

// QP Boundary Index
#define BD_LOW                  (0)
#define BD_HIGH                 (1)

// VUI Control
#if (SUPPORT_VUI_INFO)
#define VUI_HRD_MAX_CPB_CNT     (1)
#define SUPPORT_POC_TYPE_1      (0)
#define MAX_PARSET_BUF_SIZE     (256)
#define H264E_STARTCODE_LEN     (4)
#endif

// Frame Types
#define I_FRAME                 (VIDENC_I_FRAME)
#define P_FRAME                 (VIDENC_P_FRAME)
#define B_FRAME                 (VIDENC_B_FRAME)

// Image Compression
#define ICOMP_LSY_STATIC_REDUCTION_IDX  (8)
#define ICOMP_LSY_BLK_SIZE_IDX          (0)
#define ICOMP_LSY_YUVRNG_IDX            (1)
#define ICOMP_LSY_MAX_LVL_IDX           (2)
#define ICOMP_LSY_LUMA_BLK_SIZE_OFFSET  (3) ///< The offset to change the block size setting value to fit hardware opr value(0:4x4, 1: 8x8) 
#define ICOMP_LSY_CHR_BLK_SIZE_OFFSET   (2)
#define ICOMP_LSY_YUVRNG_OFFSET         (1)
#define ICOMP_LSY_MAX_LVL_OFFSET        (5)
#define ICOMP_LSY_MEAN_OFFSET           (0)
#define ICOMP_LSY_CONTRAST_OFFSET       (1)
#define ICOMP_LSY_DIFF_THR_OFFSET       (2)

// H264 Profile IDC
#define BASELINE_PROFILE        (66)
#define MAIN_PROFILE            (77)

// FREXT Profile IDC
#define FREXT_HP                (100)      ///< YUV 4:2:0/8 "High"
#define FREXT_Hi10P             (110)      ///< YUV 4:2:0/10 "High 10"
#define FREXT_Hi422             (122)      ///< YUV 4:2:2/10 "High 4:2:2"
#define FREXT_Hi444             (244)      ///< YUV 4:4:4/12 "High 4:4:4"

// H264 QP bound
#define H264E_MAX_MB_QP         (46)
#define H264E_MIN_MB_QP         (6)

// The default threshold of accumulate skip frames within one second.
// While the number of accumulate skip frames in 1 second is over the
// threshold, card_slow event will be triggered by callback.
#define CARD_SLOW_THRESHOLD     (6)

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MMPF_H264ENC_INTRA_REFRESH_MODE {
    MMPF_H264ENC_INTRA_REFRESH_DIS = 0,
    MMPF_H264ENC_INTRA_REFRESH_MB,
    MMPF_H264ENC_INTRA_REFRESH_ROW
} MMPF_H264ENC_INTRA_REFRESH_MODE;

typedef enum _MMPF_H264ENC_HBR_MODE {
    MMPF_H264ENC_HBR_MODE_60P,
    MMPF_H264ENC_HBR_MODE_30P,
    MMPF_H264ENC_HBR_MODE_MAX
} MMPF_H264ENC_HBR_MODE;

typedef enum _MMPF_H264ENC_HDRCODING_MODE {
    MMPF_H264ENC_HDRCODING_FULL = 0,
    MMPF_H264ENC_HDRCODING_COHDR,
    MMPF_H264ENC_HDRCODING_MAX
} MMPF_H264ENC_HDRCODING_MODE;

typedef enum _MMPF_H264ENC_COHDR_OPTION {
    MMPF_H264ENC_COHDR_NONE  = 0x0000,
    MMPF_H264ENC_COHDR_SEI_0 = 0x0001,
    MMPF_H264ENC_COHDR_SEI_1 = 0x0002,
    MMPF_H264ENC_COHDR_PNALU = 0x0004,
    MMPF_H264ENC_COHDR_SLICE = 0x0008
} MMPF_H264ENC_COHDR_OPTION;

#if (SUPPORT_VUI_INFO)
typedef enum _MMPF_H264ENC_SEI_TYPE {
    MMPF_H264ENC_SEI_TYPE_BUF_PERIOD      = 0x0001,
    MMPF_H264ENC_SEI_TYPE_PIC_TIMING      = 0x0002,
    MMPF_H264ENC_SEI_TYPE_USER_DATA_UNREG = 0x0020,
    MMPF_H264ENC_SEI_TYPE_MAX             = 0x0040
} MMPF_H264ENC_SEI_TYPE;

typedef enum _MMPF_H264ENC_BYTESTREAM_TYPE {
    MMPF_H264ENC_BYTESTREAM_ANNEXB= 0,
    MMPF_H264ENC_BYTESTREAM_NALU_EBSP,
    MMPF_H264ENC_BYTESTREAM_NALU_RBSP
} MMPF_H264ENC_BYTESTREAM_TYPE;

typedef enum _MMPF_H264ENC_NALU_TYPE {
    H264_NALU_TYPE_SLICE    = 1,
    H264_NALU_TYPE_DPA      = 2,
    H264_NALU_TYPE_DPB      = 3,
    H264_NALU_TYPE_DPC      = 4,
    H264_NALU_TYPE_IDR      = 5,
    H264_NALU_TYPE_SEI      = 6,
    H264_NALU_TYPE_SPS      = 7,
    H264_NALU_TYPE_PPS      = 8,
    H264_NALU_TYPE_AUD      = 9,
    H264_NALU_TYPE_EOSEQ    = 10,
    H264_NALU_TYPE_EOSTREAM = 11,
    H264_NALU_TYPE_FILL     = 12,
    H264_NALU_TYPE_SPSEXT   = 13,
    H264_NALU_TYPE_PREFIX   = 14,
    H264_NALU_TYPE_SUBSPS   = 15
} MMPF_H264ENC_NALU_TYPE;

typedef enum _MMPF_H264ENC_NAL_REF_IDC{
    H264_NALU_PRIORITY_HIGHEST     = 3,
    H264_NALU_PRIORITY_HIGH        = 2,
    H264_NALU_PRIORITY_LOW         = 1,
    H264_NALU_PRIORITY_DISPOSABLE  = 0
} MMPF_H264ENC_NAL_REF_IDC;
#endif

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

// VUI Information
#if (SUPPORT_VUI_INFO)
typedef struct _MMPF_H264ENC_SEI_PARAM {
    struct {
        MMP_ULONG init_cpb_removal_delay[2][VUI_HRD_MAX_CPB_CNT];
        MMP_ULONG init_cpb_removal_delay_offset[2][VUI_HRD_MAX_CPB_CNT];
    } BUF_PERIOD;
    struct {
        MMP_ULONG cpb_removal_delay;
        MMP_ULONG dpb_output_delay; 
    } PIC_TIMING;
} MMPF_H264ENC_SEI_PARAM;

typedef struct _MMPF_H264ENC_SYNTAX_ELEMENT {
    MMP_LONG    type;           //!< type of syntax element for data part.
    MMP_LONG    value1;         //!< numerical value of syntax element
    MMP_LONG    value2;         //!< for blocked symbols, e.g. run/level
    MMP_LONG    len;            //!< length of code
    MMP_LONG    inf;            //!< info part of UVLC code
    MMP_ULONG   bitpattern;     //!< UVLC bitpattern
    MMP_LONG    context;        //!< CABAC context
    //!< for mapping of syntaxElement to UVLC
    //void    (*mapping)(int value1, int value2, int* len_ptr, int* info_ptr);
} MMPF_H264ENC_SYNTAX_ELEMENT;

typedef struct _MMPF_H264ENC_NALU_INFO {
    MMPF_H264ENC_NALU_TYPE      nal_unit_type;
    MMPF_H264ENC_NAL_REF_IDC    nal_ref_idc;
    MMP_UBYTE                   temporal_id;    ///< SVC extension
} MMPF_H264ENC_NALU_INFO;

typedef struct _MMPF_H264ENC_BS_INFO {     
    MMP_LONG    byte_pos;           ///< current position in bitstream;
    MMP_LONG    bits_to_go;         ///< current bitcounter
    MMP_UBYTE   byte_buf;           ///< current buffer for last written byte
    MMP_UBYTE   *streamBuffer;      ///< actual buffer for written bytes
} MMPF_H264ENC_BS_INFO;

typedef struct _MMPF_H264ENC_HRD_INFO {
    MMP_ULONG   cpb_cnt_minus1;                                 // ue(v)
    MMP_ULONG   bit_rate_scale;                                 // u(4)
    MMP_ULONG   cpb_size_scale;                                 // u(4)
    MMP_ULONG   bit_rate_value_minus1 [VUI_HRD_MAX_CPB_CNT];  	// ue(v)
    MMP_ULONG   cpb_size_value_minus1 [VUI_HRD_MAX_CPB_CNT];  	// ue(v)
    MMP_ULONG   cbr_flag              [VUI_HRD_MAX_CPB_CNT];  	// u(1)
    MMP_ULONG   initial_cpb_removal_delay_length_minus1;        // u(5)
    MMP_ULONG   cpb_removal_delay_length_minus1;                // u(5)
    MMP_ULONG   dpb_output_delay_length_minus1;                 // u(5)
    MMP_ULONG   time_offset_length;                             // u(5)
} MMPF_H264ENC_HRD_INFO;

typedef struct _MMPF_H264ENC_VUI_INFO {
    MMP_BOOL    aspect_ratio_info_present_flag;               	// u(1)
    MMP_ULONG   aspect_ratio_idc;                             	// u(8)
    MMP_ULONG   sar_width;                                		// u(16)
    MMP_ULONG   sar_height;                               		// u(16)
    MMP_BOOL    overscan_info_present_flag;                   	// u(1)
    MMP_BOOL    overscan_appropriate_flag;                		// u(1)
    MMP_BOOL    video_signal_type_present_flag;               	// u(1)
   	MMP_ULONG   video_format;                                   // u(3) 0:component, 1:PAL, 2:NTSC, 3:SECAM, 4:MAC 5:Unspecified video format
    MMP_BOOL    video_full_range_flag;                        	// u(1)
    MMP_BOOL    colour_description_present_flag;              	// u(1)
    MMP_ULONG   colour_primaries;                             	// u(8)
    MMP_ULONG   transfer_characteristics;                     	// u(8)
    MMP_ULONG   matrix_coefficients;                          	// u(8)
    MMP_BOOL    chroma_location_info_present_flag;              // u(1)
    MMP_ULONG   chroma_sample_loc_type_top_field;               // ue(v)
    MMP_ULONG   chroma_sample_loc_type_bottom_field;            // ue(v)
    MMP_BOOL    timing_info_present_flag;                       // u(1)
    MMP_ULONG   num_units_in_tick;                              // u(32)
    MMP_ULONG   time_scale;                                     // u(32)
    MMP_BOOL    fixed_frame_rate_flag;                          // u(1)
    MMP_BOOL    nal_hrd_parameters_present_flag;                // u(1)
    MMPF_H264ENC_HRD_INFO nal_hrd_parameters;                   // hrd_paramters_t
    MMP_BOOL    vcl_hrd_parameters_present_flag;                // u(1)
    MMPF_H264ENC_HRD_INFO vcl_hrd_parameters;                   // hrd_paramters_t
    MMP_BOOL    low_delay_hrd_flag;                             // u(1)
    MMP_BOOL    pic_struct_present_flag;                        // u(1)
    MMP_BOOL    bitstream_restriction_flag;                     // u(1)
    MMP_BOOL    motion_vectors_over_pic_boundaries_flag;        // u(1)
    MMP_ULONG   max_bytes_per_pic_denom;                        // ue(v)
    MMP_ULONG   max_bits_per_mb_denom;                          // ue(v)
    MMP_ULONG   log2_max_mv_length_vertical;                    // ue(v)
    MMP_ULONG   log2_max_mv_length_horizontal;                  // ue(v)
    MMP_ULONG   num_reorder_frames;                             // ue(v)
    MMP_ULONG   max_dec_frame_buffering;                        // ue(v)
} MMPF_H264ENC_VUI_INFO;

typedef struct _MMPF_H264ENC_SPS_INFO {
    MMP_BOOL    Valid; // indicates the parameter set is valid
    MMP_ULONG   profile_idc;                                    // u(8)
    MMP_BOOL    constrained_set0_flag;                          // u(1)
    MMP_BOOL    constrained_set1_flag;                          // u(1)
    MMP_BOOL    constrained_set2_flag;                          // u(1)
    MMP_BOOL    constrained_set3_flag;                          // u(1)
    MMP_BOOL    constrained_set4_flag;                          // u(1)
    MMP_BOOL    constrained_set5_flag;                          // u(1)
    MMP_BOOL    constrained_set6_flag;                          // u(1)
    MMP_ULONG   level_idc;                                      // u(8)
    MMP_ULONG   seq_parameter_set_id;                           // ue(v)
    MMP_ULONG   chroma_format_idc;                              // ue(v)

    MMP_BOOL    seq_scaling_matrix_present_flag;                // u(1) => always 0
    MMP_ULONG   bit_depth_luma_minus8;                          // ue(v)
    MMP_ULONG   bit_depth_chroma_minus8;                        // ue(v)
    MMP_ULONG   log2_max_frame_num_minus4;                      // ue(v)
    MMP_ULONG   pic_order_cnt_type;
    MMP_ULONG   log2_max_pic_order_cnt_lsb_minus4;              // ue(v)
    #if (SUPPORT_POC_TYPE_1 == 1)
    MMP_BOOL delta_pic_order_always_zero_flag;                  // u(1)
    MMP_LONG    offset_for_non_ref_pic;                     	// se(v)
    MMP_LONG    offset_for_top_to_bottom_field;             	// se(v)
    MMP_ULONG   num_ref_frames_in_pic_order_cnt_cycle;      	// ue(v)
    MMP_LONG    offset_for_ref_frame[MAX_REF_FRAME_IN_POC_CYCLE];// se(v)
    #endif
    MMP_ULONG   num_ref_frames;                                 // ue(v)
    MMP_BOOL    gaps_in_frame_num_value_allowed_flag;           // u(1)
    MMP_ULONG   pic_width_in_mbs_minus1;                        // ue(v)
    MMP_ULONG   pic_height_in_map_units_minus1;                 // ue(v)
    MMP_BOOL    frame_mbs_only_flag;                            // u(1)
    MMP_BOOL    mb_adaptive_frame_field_flag;                   // u(1)
    MMP_BOOL    direct_8x8_inference_flag;                      // u(1)
    MMP_BOOL    frame_cropping_flag;                            // u(1)
    MMP_ULONG   frame_cropping_rect_left_offset;                // ue(v)
    MMP_ULONG   frame_cropping_rect_right_offset;               // ue(v)
    MMP_ULONG   frame_cropping_rect_top_offset;                 // ue(v)
    MMP_ULONG   frame_cropping_rect_bottom_offset;              // ue(v)
    MMP_BOOL    vui_parameters_present_flag;                    // u(1)
    MMPF_H264ENC_VUI_INFO vui_seq_parameters;
} MMPF_H264ENC_SPS_INFO;
#endif

// H264 Padding Setting
typedef struct _MMPF_H264ENC_PADDING_INFO {
    VIDENC_PADDING_TYPE     type;
    MMP_UBYTE               ubPaddingCnt;
} MMPF_H264ENC_PADDING_INFO;

// ME/MD Parameter
typedef struct _MMPF_H264ENC_MEMD_PARAM {
    MMP_USHORT  usMeStopThr[2];                     // 0: low, 1: high
    MMP_USHORT  usMeSkipThr[2];                     // 0: low, 1: high
} MMPF_H264ENC_MEMD_PARAM;

// H264 Module Status
typedef struct _MMPF_H264ENC_FUNC_STATES {
    MMP_USHORT                      ProfileIdc;     // Encoder profile
    VIDENC_ENTROPY                  EntropyMode;    // VLC mode
} MMPF_H264ENC_FUNC_STATES;

// H264 Module
typedef struct _MMPF_H264ENC_MODULE {
    MMP_BOOL                        bWorking;       // The module is working or not
    struct _MMPF_H264ENC_ENC_INFO   *pH264Inst;     // Pointer to instance
    MMPF_H264ENC_FUNC_STATES        HwState;        // HW status
} MMPF_H264ENC_MODULE;

// Image Compression
typedef struct _MMPF_H264ENC_ICOMP {
    MMP_BOOL    bICompEnable;
    MMP_BOOL    bICompCurMbLsyEn;
    MMP_BOOL    bICompLsyLvlCtlEn;
    MMP_UBYTE   ubICompRatio;
    MMP_UBYTE   ubICompRatioIndex;
    MMP_UBYTE   ubICompMinLsyLvlLum;
    MMP_UBYTE   ubICompMinLsyLvlChr;
    MMP_UBYTE   ubICompIniLsyLvlLum;
    MMP_UBYTE   ubICompIniLsyLvlChr;
    MMP_UBYTE   ubICompMaxLsyLvlLum;
    MMP_UBYTE   ubICompMaxLsyLvlChr;
    MMP_ULONG   ulICompFrmSize;
} MMPF_H264ENC_ICOMP;

// TNR Control
#if (H264ENC_TNR_EN)
typedef struct _MMPF_H264ENC_TNR_FILTER {
    MMP_UBYTE   luma_4x4;
    MMP_UBYTE   chroma_4x4;
    MMP_USHORT  thr_8x8;
} MMPF_H264ENC_TNR_FILTER;

typedef struct _MMPF_H264ENC_TNR {
    MMP_USHORT  low_mv_x_thr;
    MMP_USHORT  low_mv_y_thr;
    MMP_USHORT  zero_mv_x_thr;
    MMP_USHORT  zero_mv_y_thr;
    MMP_UBYTE   zero_luma_pxl_diff_thr;
    MMP_UBYTE   zero_chroma_pxl_diff_thr;
    MMP_UBYTE   zero_mv_4x4_cnt_thr; 
    MMPF_H264ENC_TNR_FILTER low_mv_filter;
    MMPF_H264ENC_TNR_FILTER zero_mv_filter;
    MMPF_H264ENC_TNR_FILTER high_mv_filter;
} MMPF_H264ENC_TNR;
#endif

// Rate Control
typedef struct _VBV_PARAM {
    MMP_ULONG   LayerBitRate[MAX_NUM_TMP_LAYERS];
    MMP_ULONG   BitRate[MAX_NUM_TMP_LAYERS];
    MMP_ULONG   LayerVBVSize[MAX_NUM_TMP_LAYERS];
    MMP_ULONG   VBVSizeInByte[MAX_NUM_TMP_LAYERS];
    MMP_LONG    VBVCurFullness[MAX_NUM_TMP_LAYERS];
    MMP_ULONG   TargetVBVInByte[MAX_NUM_TMP_LAYERS];
    MMP_ULONG   TargetVBVInMS[MAX_NUM_TMP_LAYERS];
    MMP_ULONG   VBVRatio[MAX_NUM_TMP_LAYERS];
} VBV_PARAM;

typedef struct _RC {
    // Global settings
    MMP_LONG    rc_mode;                    // CBR or VBR or Constant QP mode
    MMP_ULONG   Bitrate;                    // Stream avarage bitrate
    MMP_LONG    TargetFrmSize;              // Target frame size
    
    MMP_ULONG   nP;                         // Number of P frame within GOP
    MMP_ULONG   nB;                         // Number of B frame within GOP   
    MMP_LONG    IntraPeriod;                // The frame num of a GOP (I+P+B frames)
    
    MMP_ULONG	MaxWeight[3];		        // The max size ratio of I/P/B frame (Base:1024)
    MMP_ULONG	MinWeight[3];		        // The min size ratio of I/P/B frame (Base:1024)
    MMP_LONG    Alpha[3];                   // The current weighting of I/P/B frame for bit budget dispatch (Base:1024)	
    MMP_ULONG   MaxQPDelta[3];              // The QP delta max range within MB layer, Ex:-3 ~ +3
    MMP_ULONG   QPLowBound[3];              // QP lower bound
    MMP_ULONG   QPUpBound[3];               // QP upper bound
    MMP_ULONG   bUseInitQP;                 // Use initial QP for 1st frame

    MMP_ULONG   VideoFormat;                // Video format (H264/MJPEG)
    MMP_UBYTE   VidFormatIdx;               // Video format index
    MMP_ULONG   LastFrmType;                // The last frame type (I/P/B)
    MMP_ULONG   LastFrmQP;                  // The last frame QP value
    MMP_ULONG   LastQP[3];                  // The last OP value of I/P/B frame
    MMP_LONG    LastX[3];                   // The last frame complexity of I/P/B frame  
    unsigned long long Acc_X[3];            // The accumulation of frame complexity of I/P/B frame
    MMP_LONG    FrmCount[3];                // The frame count of I/P/B frame     
    MMP_BOOL    bResetRC;                   // Reset RC module or not
    
    MMP_ULONG   temporal_id;                // I:0, P:1(ref),2(non-ref)
    MMP_UBYTE   slice_mode;
    MMP_ULONG   slice_size;
    
    MMP_LONG    avg_QP[3];                  // The average QP of I/P/B frame (Base:1000)
    MMP_ULONG   avg_XP[3];                  // The average complexity of I/P/B frame
    MMP_LONG    avg_FrameSize[3];           // The average frame size of I/P/B frame
    
    // VBV settings
    MMP_LONG    VBVCurFullness;             // The current fullness of VBV (Unit:Bytes)
    MMP_LONG    VBVSizeInByte;              // The size of VBV (Unit:Bytes)
    MMP_ULONG   TargetVBVInMS;              // The target fullness of VBV (Unit:ms), Ex:250 ms 
    MMP_ULONG   TargetVBVInByte;            // The target fullness of VBV (Unit:Bytes)
    MMP_ULONG   VBVRatio;                   // The ratio of VBV size (Unit:bit) and BitRate, use for reset bitrate or reset buffer size (Base:1000)

    // CBR/LBR settings
    MMP_LONG    cbr_RemainBudgetInRcPeriod; // [CBR mode] Remain bit budget in the RC GOP (Unit:Bytes)
    MMP_LONG    cbr_RemainFrmsInRcPeriod;   // [CBR mode] Left frame within the RC GOP
    MMP_ULONG   cbr_TargetPSize;            // [CBR mode] Target P frame size

    // VBR settings
    MMP_ULONG   vbr_RcPeriodBudget;         // [VBR mode] Total bit budget of VBR_RC_PERIOD (300) frames
    MMP_LONG    vbr_RemainBudgetInRcPeriod; // [VBR mode] Remain bit budget in the RC period (Unit:Bytes)
    MMP_LONG    vbr_RemainBudgetInRcGOP;    // [VBR mode] The remain bit budget within the vbr_RcGOP
    MMP_LONG    vbr_RemainFrmsInRcPeriod;   // [VBR mode] Remain total frames in the RC period
    MMP_LONG    vbr_TotalFrmsInRcPeriod;    // [VBR mode] Total RC period frame num. Ex:300 frames
    MMP_ULONG   vbr_FrameCount;             // [VBR mode] The encoded frame count
    MMP_LONG    vbr_GOPNumPerIPeriod;       // [VBR mode] The frame num ratio of real GOP and vbr_RcGOP
    MMP_LONG    vbr_GOPCntWithinRealGOP;    // [VBR mode] The vbr_RcGOP counter between 0 ~ vbr_GOPNumPerIPeriod

    MMP_LONG    vbr_mode_start;             // [VBR mode] Start VBR mode
    MMP_LONG    vbr_TargetPSize;            // [VBR mode] The target P frame size 
    MMP_ULONG   vbr_RcGOP;                  // [VBR mode] Frame number within RC GOP
    MMP_LONG    vbr_RcGOP_QP[3];            // [VBR mode] The QP base of I/P/B frame within the vbr_RcGOP
    MMP_LONG    vbr_QPSumInRcGOP;           // [VBR mode] The QP summation
    MMP_LONG    vbr_FrameCntInRcGOP;        // [VBR mode] The frame count between (0 ~ vbr_RcGOP)
    MMP_LONG    vbr_LeftFrmInRcGOP[3];      // [VBR mode] The left frame num of I/P/B frame within the vbr_RcGOP
    MMP_LONG    vbr_RcGOPCount;             // [VBR mode] The GOP count (assume 15 frame as a vbr_RcGOP)    

    // Skip Frame settings
    MMP_BOOL    bSkipPrevFrame;             // Skip previous frame or not
    MMP_ULONG   SkipFrameThd;               // The skip frame threshold of the VBV (Base:1024)    
    MMP_BOOL    bPreSkipFrmEn;              // Enable skip frame before encode
    MMP_BOOL    bPostSkipFrmEn;             // Enable skip frame after encode

    // Multi-Layer settings
    void*       pGlobalVBV;                 // Pointer to global VBV structure
    MMP_ULONG   LayerRelated;               // Enable layer operation
    MMP_ULONG   Layer;                      // Layer ID
} RC;

typedef struct _RC_CONFIG_PARAM {
    MMP_ULONG	MaxIWeight;			        // The max size ratio of I/P frame (Base:1000), Ex:1.5 * 1000
    MMP_ULONG	MinIWeight;			        // The min size ratio of I/P frame (Base:1000), Ex:1.0 * 1000
    MMP_ULONG	MaxBWeight;			        // The max size ratio of B/P frame (Base:1000), Ex:1.0 * 1000
    MMP_ULONG	MinBWeight;			        // The min size ratio of B/P frame (Base:1000), Ex:0.5 * 1000
    MMP_ULONG	VBVSizeInBit;	            // The size of VBV (Unit:bits) 
    MMP_ULONG	TargetVBVInMS;              // The target fullness of VBV (Unit:ms), Ex:250 ms 
    MMP_ULONG	InitWeight[3];              // The weighting of I/P/B frame for bit budget dispatch (Use P = 1000 as base)
    MMP_ULONG	MaxQPDelta[3];              // The QP delta max range within MB layer, Ex:-3 ~ +3
    MMP_ULONG   SkipFrameThd;               // The skip frame threshold of the VBV (Base:1000), Ex:700
    MMP_ULONG   MBWidth;                    // Frame width (Unit:MB)
    MMP_ULONG   MBHeight;                   // Frame height (Unit:MB)
    MMP_ULONG   InitQP[3];                  // Initial QP value of I/P/B frame set by host
    MMP_ULONG   rc_mode;                    // CBR or VBR or CONSTANT_QP mode
    MMP_ULONG   bPreSkipFrmEn;              // Skip frame or not

    MMP_ULONG   LayerRelated;               // Enable layer operation
    MMP_ULONG   Layer;                      // Layer ID
    MMP_ULONG   EncID;                      // Encode instance ID
} RC_CONFIG_PARAM;

// Encode Instance
typedef struct _MMPF_H264ENC_ENC_INFO {

    /* General Configuration */
    MMP_ULONG                   enc_id;             // Instance ID
    MMPF_H264ENC_MODULE         *module;            // Pointer to connected module
    void                        *priv_data;         // Pointer to connected private data
    void                        *Container;         // Pointer to connected container
    VidEncEndCallBackFunc       *EncEndCallback;    // The callback function for encode end
        
    MMP_USHORT                  profile;            // Encoder profile ID
    MMP_USHORT                  level;              // Encoder level ID
    MMP_ULONG                   mb_num;             // The total MB num of one frame
    MMP_USHORT                  mb_w;               // The width of frame (Unit:MB)
    MMP_USHORT                  mb_h;               // The height of frame (Unit:MB)
    MMPF_H264ENC_PADDING_INFO   paddinginfo;        // Encoder padding information
    MMP_USHORT                  b_frame_num;        // The B frame num within GOP
    MMP_ULONG                   conus_i_frame_num;  // The contiguous I-frame count
    MMP_ULONG                   gop_frame_num;      // Set zero if sync frame is output
    MMP_USHORT                  gop_size;           // The total frame num within GOP
    MMP_BOOL                    co_mv_valid;        //
    MMP_BOOL                    insert_skip_frame;  // Insert skip frame to prevent overflow
    MMP_BOOL                    insert_sps_pps;     // Insert SPS/PPS or not
    MMP_ULONG                   stream_bitrate;	    // The stream bitrate
    VIDENC_DUMMY_DROP_INFO      dummydropinfo;      // The dummy/drop frame information
    VIDENC_ENTROPY              entropy_mode;       // The entropy VLC mode
    VIDENC_CROPPING             crop;               // The cropping configuration
    MMP_ULONG                   TotalEncodeSize;    // Total encoded size
    MMP_ULONG                   total_frames;	    // Total encoded frames
    MMP_ULONG                   prev_ref_num;       // The previous ref frame num, increase when cur frame can be ref
    VIDENC_CURBUF_MODE          CurBufMode;         // Frame mode or Real-Time mode
    MMP_ULONG64                 timestamp;          // The current frame timestamp 
    VIDENC_FRAME_TYPE           cur_frm_type;       // The current frame type (I/P/B)
    MMP_UBYTE                   SrcPipeId;          // The source pipe ID
    MMP_USHORT                  usMaxNumRefFrame;   // The max reference frame number
    MMP_UBYTE                   usMaxFrameNumAndPoc;// The max reference frame number and POC number
    MMP_ULONG                   sps_len;            // The current SPS length
    MMP_ULONG                   pps_len;            // The current PPS length
    
    MMP_USHORT                  Operation;          // The operation from host command
    MMP_USHORT                  Status;             // The status of video engine
    MMP_UBYTE                   VideoCaptureStart;  // The flag of execute encode function (MMPF_VIDENC_TriggerEncode)

    /* Real Time Mode Status */
    MMP_BOOL                    bRtTrigEncByFrmSrc; // Trigger RT encode by outter frame source.
    MMP_BOOL                    bRtEncDropFrameEn;  // Need to drop current frame or not in RT mode.
    MMP_BOOL                    bRtCurBufOvlp;      // The PingPong buffer is overlap in RT mode
    MMP_BOOL                    bRtScaleDFT;        // The scaler double frame start occured in RT mode
    
    /* Buffer Configuration */    
    #if (SUPPORT_VUI_INFO)
    MMP_UBYTE                   *glVidRecdTmpSPSAddr;// The temp SPS address
    MMP_ULONG                   glVidRecdTmpSPSSize;// The temp SPS size
    MMP_UBYTE                   sps_buf[MAX_PARSET_BUF_SIZE]; // The SPS buffer
    MMP_UBYTE                   pps_buf[MAX_PARSET_BUF_SIZE]; // The PPS buffer
    MMP_UBYTE                   sei_buf[MAX_PARSET_BUF_SIZE]; // The SEI buffer
    #endif
    
    MMP_UBYTE                   *VidRecdSPSAddr;    // The current SPS address
    MMP_ULONG                   VidRecdSPSSize;     // The current SPS size
    MMP_UBYTE                   *VidRecdPPSAddr;    // The current PPS address 
    MMP_ULONG                   VidRecdPPSSize;     // The current PPS size
    MMP_ULONG                   mv_addr;            // The address for store motion vector
    MMP_ULONG                   slice_addr;         // The slice length buffer address

    MMP_ULONG                   RefGenBufLowBound;  // The Ref/Gen (Share) buffer lower bound
    MMP_ULONG                   RefGenBufHighBound; // The Ref/Gen (Share) buffer upper bound
    VIDENC_FRAMEBUF_BD          RefBufBound;        // The Ref buffer bound
    VIDENC_FRAMEBUF_BD          GenBufBound;        // The Gen buffer bound

    VIDENC_FRAME_INFO           cur_frm[4];         // The current frame buffer addr and timestamp information
    VIDENC_FRAME_INFO           ref_frm;            // The current referece frame buffer addr and timestamp information
    VIDENC_FRAME_INFO           rec_frm;            // The current reconstruct frame buffer addr and timestamp information

    MMP_ULONG                   cur_frm_bs_addr;    // BitStream addr for current enc frame
    MMP_ULONG                   cur_frm_bs_high_bd; // BitStream addr high bound for current enc frame
    MMP_ULONG                   cur_frm_bs_low_bd;  // BitStream addr low bound for current enc frame

    /* Skip Frame Configuration */
    MMP_BOOL                    rc_skippable;       // Rc can skip frames or not
    MMP_BOOL                    rc_skip_smoothdown; // 0: direct skip, 1: smooth skip
    MMP_BOOL                    rc_skip_bypass;     // Bypass current frame RC skip

    /* RC Temporal Layers Configuration */
    MMP_ULONG                   layer_bitrate[MAX_NUM_TMP_LAYERS];      // Per-layer bitrate
    MMP_ULONG                   layer_lb_size[MAX_NUM_TMP_LAYERS];      // Leakybucket size per-layer in ms
    MMP_ULONG                   layer_tgt_frm_size[MAX_NUM_TMP_LAYERS]; // Target frame size for each layer
    MMP_ULONG                   layer_frm_thr[MAX_NUM_TMP_LAYERS];      // Threshold to skip encoding
    MMP_ULONG                   layer_fps_ratio[MAX_NUM_TMP_LAYERS];    // Fps ratio for each layer
    #if (H264ENC_LBR_EN)
    MMP_ULONG                   layer_fps_ratio_low_br[MAX_NUM_TMP_LAYERS_LBR];
    #endif
    RC_CONFIG_PARAM             rc_config[MAX_NUM_TMP_LAYERS];			// The RC temporal layers configuration
    void                        *layer_rc_hdl[MAX_NUM_TMP_LAYERS];		// The RC handle
    MMP_UBYTE                   priority_id[MAX_NUM_TMP_LAYERS];        // Priority_id config by temporal layer
    MMP_UBYTE                   total_layers;

    /* ME/MC Configuration */
    MMPF_H264ENC_MEMD_PARAM     Memd;               // ME/MD parameters
    MMP_USHORT                	inter_cost_th;		// Reset 1024
    MMP_USHORT                	intra_cost_adj;		// Reset 18
    MMP_USHORT                	inter_cost_bias;	// Reset 0
    MMP_USHORT                	me_itr_steps;
    
    /* VUI Configuration */
    #if (SUPPORT_VUI_INFO)
    MMPF_H264ENC_SPS_INFO       sps;                // The temp SPS information
    MMPF_H264ENC_SEI_CTL        SeiCtl;             // SEI control
    MMPF_H264ENC_SEI_PARAM      sei_param;          // Parameter for supported sei message
    MMPF_H264ENC_NALU_INFO      nalu;               // The NALU information
    #endif
    
    /* QP Tune Configuration */
    MMP_BOOL                    bRcEnable;          // Rate control QP tune enable
    MMP_UBYTE                   qp_tune_mode;       // The QP tune basic unit (MB, row, slice, frame)
    MMP_UBYTE                   qp_tune_size;       // The QP tune basic unit size
    MMP_LONG                    MbQpBound[MAX_NUM_TMP_LAYERS][VIDENC_FRAME_TYPE_NUM][2];
    MMP_LONG                    CurRcQP[MAX_NUM_TMP_LAYERS][VIDENC_FRAME_TYPE_NUM];
    
    /* RDO Configuration */
    #if (H264ENC_RDO_EN)
    MMP_BOOL                    bRdoEnable;         // Enable RDO function or not
    MMP_BOOL                  	bRdoMadValid;       // Indicate the RDO MAD is valid 
    MMP_ULONG                   ulRdoMadFrmPre;     // The MAD of previous frame
    MMP_ULONG                   ulRdoInitFac;       // The RDO initial factor
    MMP_ULONG                   ulRdoMadLastRowPre; // The MAD of last row of previous frame     
    MMP_UBYTE                   ubRdoMissThrLoAdj;  // The RDO miss tune threshold
    MMP_UBYTE                   ubRdoMissThrHiAdj;  // The RDO miss tune threshold
    MMP_UBYTE                   ubRdoMissTuneQp;    // The RDO miss tune QP
    MMP_UBYTE                   qstep3[10];         // RDO QSTEP3 parameters
    #endif

    /* RunTime Update Parameter */
    MMP_ULONG                   ulParamQueueRdWrapIdx;          // The paramter queue read index (MS3Byte wrap, LSByte idx)
    MMP_ULONG                   ulParamQueueWrWrapIdx;          // The paramter queue write index (MS3Byte wrap, LSByte idx)
    VIDENC_PARAM_CTL            ParamQueue[VIDENC_MAX_PARAM_SET_NUM];  // The paramter queue 
    
    /* Frame Rate Control */
    MMP_ULONG                   ulMaxFpsRes;        // Host controled max encode out fps resolution
    MMP_ULONG                   ulMaxFpsInc;        // Host controled max encode out fps increament
    MMP_ULONG                   ulMaxFps1000xInc;   // Host controled max encode out fps increament [NO USE]
    MMP_ULONG                   ulEncFpsRes;        // Host controled encode fps resolution [NO USE]
    MMP_ULONG                   ulEncFpsInc;        // Host controled encode fps increament [NO USE]
    MMP_ULONG                   ulEncFps1000xInc;   // Host controled encode fps increament [NO USE]
    MMP_ULONG                   ulSnrFpsRes;        // Host controled sensor fps resolution
    MMP_ULONG                   ulSnrFpsInc;        // Host controled sensor fps increament
    MMP_ULONG                   ulSnrFps1000xInc;   // Host controled sensor fps increament [NO USE]
    MMP_ULONG                   ulFpsInputRes;      // Input frame rate resolution
    MMP_ULONG                   ulFpsInputInc;      // Input frame rate increament
    MMP_ULONG                   ulFpsInputAcc;      // Input frame rate accumulation
    MMP_ULONG                   ulFpsOutputRes;     // Encode frame rate resolution
    MMP_ULONG                   ulFpsOutputInc;     // Encode frame rate increament
    MMP_ULONG                   ulFpsOutputAcc;     // Encode frame rate accumulation
    
    /* CoHeader Configuration */
    MMP_USHORT                  cohdr_sei_rbsp_bits;
    MMP_UBYTE                   cohdr_pnalu_bits;
    MMP_UBYTE                   cohdr_slicehdr_bits;
    MMPF_H264ENC_COHDR_OPTION   cohdr_option;
    
    /* Deblock Configuration */
    MMP_BOOL                    dblk_disable_idc;
    MMP_SHORT                   dblk_alpha_div2;
    MMP_SHORT                   dblk_beta_div2;
    
    /* Low Bitrate Configuration */
    #if (H264ENC_LBR_EN)
    MMP_UBYTE                   ubAccResetLowBrRatioTimes;
    MMP_UBYTE                   ubAccResetHighBrRatioTimes;
    MMP_BOOL                    bResetLowBrRatio;   // Reset low bitrate (P-frame) ratio
    MMP_BOOL                    bResetHighBrRatio;  // Reset high bitrate (I-frame) ratio
    MMP_ULONG                   intra_mb;           // Intra MB count
    MMP_UBYTE                   decision_mode;      // Decide the intra/inter MB mode
    #endif
    
    /* TNR Configuration */
    #if (H264ENC_TNR_EN)
    MMP_UBYTE                   tnr_en;             // TNR enable bitmap
    MMPF_H264ENC_TNR            tnr_ctl;            // TNR control parameters
    #endif
    
    /* Image Compression Configuration */
    #if (H264ENC_ICOMP_EN)
    MMPF_H264ENC_ICOMP          ICompConfig;        // Image compression configuration
    #endif
    
    /* I Too Many (I2MANY) Configuration */
    #if (H264ENC_I2MANY_EN)
    MMP_UBYTE                   bI2ManyEn;
    MMP_USHORT                  usI2ManyStartMB;
    MMP_UBYTE                   ubI2ManyQpStep1;
    MMP_UBYTE                   ubI2ManyQpStep2;
    MMP_UBYTE                   ubI2ManyQpStep3;
    #endif

} MMPF_H264ENC_ENC_INFO;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#if (SUPPORT_VUI_INFO)
MMP_UBYTE MMPF_H264ENC_GenerateNaluHeader (MMPF_H264ENC_NALU_INFO *nalu_inf);
void MMPF_H264ENC_GetUserDataUnreg(MMP_ULONG* ulAGC, MMP_ULONG* ulWBMode, MMP_ULONG* ulShutter, MMP_ULONG ShuMode, MMPF_CONTAINER_INFO *pContainer);
MMP_ULONG MMPF_H264ENC_GenerateSEI(MMP_UBYTE *nalu_buf, MMPF_H264ENC_SEI_PARAM *pSeiParam,
                                   MMPF_H264ENC_SEI_TYPE PayloadType, MMPF_H264ENC_BYTESTREAM_TYPE ByteStrType,
								   MMPF_H264ENC_ENC_INFO *pEnc, MMPF_CONTAINER_INFO *pContainer);
MMP_ULONG MMPF_H264ENC_GenerateAUD(MMP_UBYTE *nalu_buf, VIDENC_FRAME_TYPE fr_type,
				                   MMPF_H264ENC_BYTESTREAM_TYPE ByteStrType);
MMP_ULONG MMPF_H264ENC_GenerateSPS (MMP_UBYTE *nalu_buf, MMPF_H264ENC_SPS_INFO *sps);
#endif

MMPF_H264ENC_ENC_INFO *MMPF_H264ENC_GetHandle(MMP_UBYTE ubEncId);
MMP_ERR MMPF_H264ENC_SetSrcPipe(MMPF_H264ENC_ENC_INFO *pEnc, MMP_UBYTE ubPipe);
MMP_ERR MMPF_H264ENC_SetCropping(MMPF_H264ENC_ENC_INFO *pEnc, MMP_BOOL bEnable);
MMP_ERR MMPF_H264ENC_SetPadding(MMPF_H264ENC_ENC_INFO       *pEnc,
                                VIDENC_PADDING_TYPE   ubPaddingType,
                                MMP_UBYTE                   ubPaddingCnt);
MMP_ERR MMPF_H264ENC_InitImageComp( MMPF_H264ENC_ENC_INFO   *pEnc,
                                    MMPF_H264ENC_ICOMP      *pConfig);
MMP_ERR MMPF_H264ENC_SetH264ByteCnt(MMP_USHORT usByteCnt);
MMP_ERR MMPF_H264ENC_SetIntraRefresh(MMPF_H264ENC_INTRA_REFRESH_MODE ubMode, MMP_USHORT usPeriod, MMP_USHORT usOffset);

MMP_ERR MMPF_H264ENC_SetBSBuf(MMPF_H264ENC_ENC_INFO *pEnc,
                              MMP_ULONG             ulLowBound,
                              MMP_ULONG             ulHighBound);
MMP_ERR MMPF_H264ENC_SetMiscBuf(MMPF_H264ENC_ENC_INFO   *pEnc,
                                MMP_ULONG               ulMVBuf,
                                MMP_ULONG               ulSliceLenBuf);
MMP_ERR MMPF_H264ENC_SetRefListBound(MMPF_H264ENC_ENC_INFO  *pEnc,
                                     MMP_ULONG              ulLowBound, 
                                     MMP_ULONG              ulHighBound);
MMP_ERR MMPF_H264ENC_SetSetHeaderBuf(   MMPF_H264ENC_ENC_INFO   *pEnc, 
                                        MMP_ULONG               ulSPSStart, 
                                        MMP_ULONG               ulPPSStart, 
                                        MMP_ULONG               ulTmpSPSStart);
MMP_ERR MMPF_H264ENC_SetParameter(  MMPF_H264ENC_ENC_INFO   *pEnc,
                                    VIDENC_ATTRIBUTE        attrib,
                                    void                    *arg);
MMP_ERR MMPF_H264ENC_GetParameter(  MMPF_H264ENC_ENC_INFO   *pEnc,
                                    VIDENC_ATTRIBUTE        attrib,
                                    void                    *ulValue);
MMP_ERR MMPF_H264ENC_InitRCConfig (MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR MMPF_H264ENC_SetQPBound(MMPF_H264ENC_ENC_INFO  *pEnc, 
                                MMP_ULONG               layer,
                                VIDENC_FRAME_TYPE       type,
                                MMP_LONG                lMinQp, 
                                MMP_LONG                lMaxQp);
MMP_ERR MMPF_H264ENC_ResetVLDModule(void);
MMP_ERR MMPF_H264ENC_InitModule(MMPF_H264ENC_MODULE *pModule);
MMP_ERR MMPF_H264ENC_DeinitModule (MMPF_H264ENC_MODULE *pModule);
MMP_ERR MMPF_H264ENC_InitInstance(MMPF_H264ENC_ENC_INFO *pEnc, 
                                  MMPF_H264ENC_MODULE   *attached_mod, 
                                  void                  *priv, 
                                  MMP_USHORT            usStreamType,
                                  MMP_USHORT            usRcMode);
MMP_ERR MMPF_H264ENC_DeInitRCConfig(MMPF_H264ENC_ENC_INFO *pEnc, MMP_ULONG InstId);
MMP_ERR MMPF_H264ENC_InitRefListConfig(MMPF_H264ENC_ENC_INFO *pEnc);
#if (SUPPORT_VUI_INFO)
MMP_ERR MMPF_H264ENC_InitSpsConfig(MMPF_H264ENC_ENC_INFO *pEnc);
#endif
MMP_ERR MMPF_H264ENC_Open(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR MMPF_H264ENC_Resume(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR MMPF_H264ENC_TriggerEncode( MMPF_H264ENC_ENC_INFO   *pEnc,
                                    VIDENC_FRAME_TYPE       FrameType,
                                    MMP_ULONG               ulFrameTime,
                                    VIDENC_FRAME_INFO       *pFrameInfo);

#endif	// _MMPF_H264ENC_H_
