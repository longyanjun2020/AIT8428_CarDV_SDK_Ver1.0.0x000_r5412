//==============================================================================
//
//  File        : AHC_USBHost.c
//  Description : AHC USB Host function
//  Author      : 
//  Revision    : 1.0
//
//==============================================================================

/*===========================================================================
 * Include files
 *===========================================================================*/ 

#include "AHC_Common.h"
#include "AHC_USBHost.h"
#include "AHC_General.h"
#include "AHC_USB.h"
#include "AHC_Display.h"
#include "mmpf_usbuvc.h"
#include "mmpf_usbh_uvc.h"
#include "lib_retina.h"
#include "mmp_rtc_inc.h"
#include "mmps_usb.h"
#include "mmps_3gprecd.h"
#include "mmps_uvcrecd.h"
#include "mmpd_scaler.h"
#include "usb_cfg.h"
#include "vidrec_cfg.h"
#include "menusetting.h"
#include "Mdtc_cfg.h"
#include "snr_cfg.h"
#include "mmpf_usbh_ctl.h"
#include "ParkingModeCtrl.h"

#if (UVC_HOST_VIDEO_ENABLE)
/*===========================================================================
 * Global varible
 *===========================================================================*/ 

static UVC_NALU_CFG glUvcNaluTbl[MMPS_3GPRECD_UVC_CFG_MAX_NUM] = {0};

static UVC_NALU_CFG glUvcNaluTblBulk[MMPS_3GPRECD_UVC_CFG_MAX_NUM] = 
{
{{0x4D,0x11,0x55,0x84,0x0,0x0,0x0,0x0}, //{{"8437"},
 {{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3F,0x0B,0x04,0,0,0x1B,0x04}}},
{{0x4D,0x11,0x51,0x84,0x0,0x0,0x0,0x0}, //{{"8451"},
 {{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04}}},
{{0x17,0x1B,0x10,0x02,0x0,0x0,0x0,0x0}, //Zz1
 {{0x30,0x3D,0x09,0x04,0,0,0x19,0x04},{0x30,0x3D,0x09,0x04,0,0,0x19,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3F,0x0B,0x04,0,0,0x1B,0x04}}},
{{0x45,0x0C,0xAB,0x64,0x0,0x0,0x0,0x0}, //SN9C5256
 {{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04}}},
{{0xA3,0x05,0x30,0x92,0x0,0x0,0x0,0x0}, //SNx158y
 {{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04}}},
{{0xCF,0x1B,0x95,0x2B,0x0,0x0,0x0,0x0}, //x198 H42
 {{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04}}},
{{0x3F,0x1B,0x01,0x83,0x0,0x0,0x0,0x0}, //JX-V01
 {{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04}}},
{{0x4D,0x11,0x28,0x83,0x0,0x0,0x0,0x0}, //{{"8328"},
 {{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3F,0x0B,0x04,0,0,0x1B,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04}}}	
};

static UVC_NALU_CFG glUvcNaluTblISO[MMPS_3GPRECD_UVC_CFG_MAX_NUM] = 
{
{{0x4D,0x11,0x55,0x84,0x0,0x0,0x0,0x0}, //{{"8437"},
 {{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3F,0x0B,0x04,0,0,0x1B,0x04}}},
{{0x45,0x0C,0x00,0x63,0x0,0x0,0x0,0x0}, //SN9C216
 {{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04}}},
{{0x17,0x1B,0x10,0x02,0x0,0x0,0x0,0x0}, //Zz1
 {{0x30,0x3D,0x09,0x04,0,0,0x19,0x04},{0x30,0x3D,0x09,0x04,0,0,0x19,0x04},{0x30,0x3E,0x0A,0x04,0,0,0x1A,0x04},{0x30,0x3F,0x0B,0x04,0,0,0x1B,0x04}}},
{{0x45,0x0C,0xAB,0x64,0x0,0x0,0x0,0x0}, //SN9C5256
 {{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04}}},
{{0xA3,0x05,0x30,0x92,0x0,0x0,0x0,0x0}, //SNx158y
 {{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04}}},
{{0xCF,0x1B,0x95,0x2B,0x0,0x0,0x0,0x0}, //x198 H42
 {{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04}}},
{{0x3F,0x1B,0x01,0x83,0x0,0x0,0x0,0x0}, //JX-V01
 {{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04},{0x30,0x58,0x24,0x04,0x17,0x0C,0x4F,0x04}}}	
};

/* guidExtensionCode */
static MMP_ULONG glUvcGuidEUTblBulk[MMPS_3GPRECD_UVC_CFG_MAX_NUM][(CS_INTERFACE_DESCR_GUID_SZ/(sizeof(MMP_ULONG)))] = 
{
{GUID_EC_AIT_0,  GUID_EC_AIT_1,  GUID_EC_AIT_2,  GUID_EC_AIT_3},    // {{"8437"},
{GUID_EC_AIT_0,  GUID_EC_AIT_1,  GUID_EC_AIT_2,  GUID_EC_AIT_3},    // {{"8451"},
{GUID_EC_SONIX_0,GUID_EC_SONIX_1,GUID_EC_SONIX_2,GUID_EC_SONIX_3},  // Zz1
{GUID_EC_SONIX_0,GUID_EC_SONIX_1,GUID_EC_SONIX_2,GUID_EC_SONIX_3},  // SN9C5256
{GUID_EC_SONIX_0,GUID_EC_SONIX_1,GUID_EC_SONIX_2,GUID_EC_SONIX_3},  // SNx158y
{GUID_EC_SONIX_0,GUID_EC_SONIX_1,GUID_EC_SONIX_2,GUID_EC_SONIX_3},  // x198 H42
{GUID_JX_SONIX_0,GUID_JX_SONIX_1,GUID_JX_SONIX_2,GUID_JX_SONIX_3},  // JX-V01
{GUID_EC_AIT_0,  GUID_EC_AIT_1,  GUID_EC_AIT_2,  GUID_EC_AIT_3}    // {{"8328"},
};

static MMP_ULONG glUvcGuidEUTblISO[MMPS_3GPRECD_UVC_CFG_MAX_NUM][(CS_INTERFACE_DESCR_GUID_SZ/(sizeof(MMP_ULONG)))] = 
{
{GUID_EC_AIT_0,  GUID_EC_AIT_1,  GUID_EC_AIT_2,  GUID_EC_AIT_3},    // {{"8437"},
{GUID_EC_SONIX_0,GUID_EC_SONIX_1,GUID_EC_SONIX_2,GUID_EC_SONIX_3},  // SN9C216
{GUID_EC_SONIX_0,GUID_EC_SONIX_1,GUID_EC_SONIX_2,GUID_EC_SONIX_3},  // SN9C216
{GUID_EC_SONIX_0,GUID_EC_SONIX_1,GUID_EC_SONIX_2,GUID_EC_SONIX_3},  // Zz1
{GUID_EC_SONIX_0,GUID_EC_SONIX_1,GUID_EC_SONIX_2,GUID_EC_SONIX_3},  // SNx158y
{GUID_EC_SONIX_0,GUID_EC_SONIX_1,GUID_EC_SONIX_2,GUID_EC_SONIX_3},  // x198 H42
{GUID_JX_SONIX_0,GUID_JX_SONIX_1,GUID_JX_SONIX_2,GUID_JX_SONIX_3}   // JX-V01
};

// H264 (1920x1080)
static MMP_UBYTE m_VsProbeCtl_8437[] = 
{
#if 1 //TBD???
    0x01, 0x00, 0x03, 0x05, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x60, 0x09, 0x00, 0x00, 0x00, 
    0x00, 0x00
#else
    0x01, 0x00, 0x03, 0x11, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00
#endif	
};

// H264 (1280x720)
static MMP_UBYTE m_VsProbeCtl_8451[] = 
{
    0x01, 0x00, 0x03, 0x01, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x60, 0x09, 0x00, 0x00, 0x00, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_64AB[] = 
{
    0x00, 0x00, 0x01, 0x01, 0x80, 0x1A, 0x06, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x20, 0x1C, 0x00, 0x00, 0x00, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_64AB_1[] = 
{
    0x01, 0x00, 0x02, 0x02, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_64AB_2[] = 
{
    0x01, 0x00, 0x02, 0x01, 0x80, 0x1A, 0x06, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_64AB_3[] = 
{
    0x00, 0x00, 0x01, 0x01, 0x40, 0x42, 0x0F, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x20, 0x1C, 0x00, 0x00, 0x0C, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_64AB_4[] = 
{
    0x01, 0x00, 0x02, 0x01, 0x80, 0x1A, 0x06, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE)// Pcam Mix Mode ... UVC function work under Video/DSC mode
    0x00, 0x00, UVC_DWMAXVIDEOFRAMESIZE_DESC(ISO_MJPEG_MAX_VIDEO_FRM_SIZE - ISO_MJPEG_MAX_VIDEO_FRM_TOLERANCE_SIZE), 0x00, 0x0C, 
#else
    0x00, 0x00, UVC_DWMAXVIDEOFRAMESIZE_DESC(ISO_MJPEG_MAX_VIDEO_FRM_SIZE), 0x00, 0x0C, 
#endif
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_8301_1[] =
{
	0x00, 0x00, 0x01, 0x01, 0x80, 0x1A, 0x06, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x60, 0x09, 0x00, 0x00, 0x00,
	0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_8301_2[] =
{
	0x00, 0x00, 0x01, 0x01, 0x80, 0x1A, 0x06, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x60, 0x09, 0x00, 0xB0, 0x03,
	0x00, 0x00
};


static MMP_UBYTE m_VsProbeCtl_PID_6300[] = 
{
    0x79, 0xD2, 0x01, 0x01, 0x80, 0x1A, 0x06, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x59, 0x07, 0x00, 0x60, 0x09, 0x00, 0xBF, 0x2A, 
    0x40, 0xD4
};

static MMP_UBYTE m_VsProbeCtl_PID_6300_1[] = 
{
    0x01, 0x00, 0x02, 0x01, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_6300_2[] = 
{
    0x79, 0xD2, 0x01, 0x01, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x59, 0x07, 0x00, 0x60, 0x09, 0x00, 0x00, 0x0C, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_6300_3[] = 
{
    0x01, 0x00, 0x02, 0x01, 0x15, 0x16, 0x05, 0x00, 
    //0x01, 0x00, 0x02, 0x02, 0x15, 0x16, 0x05, 0x00,  //352x288
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#if defined(PCAM_UVC_MIX_MODE_ENABLE) && (PCAM_UVC_MIX_MODE_ENABLE)// Pcam Mix Mode ... UVC function work under Video/DSC mode
    0x59, 0x07, UVC_DWMAXVIDEOFRAMESIZE_DESC(ISO_MJPEG_MAX_VIDEO_FRM_SIZE - ISO_MJPEG_MAX_VIDEO_FRM_TOLERANCE_SIZE), 0x00, 0x0C, 
#else
    0x59, 0x07, UVC_DWMAXVIDEOFRAMESIZE_DESC(ISO_MJPEG_MAX_VIDEO_FRM_SIZE), 0x00, 0x0C, 
#endif
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_9230_1[] = 
{
    0x00, 0x00, 0x01, 0x01, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_9230_2[] = 
{
    0x00, 0x00, 0x01, 0x01, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 
    0x00, 0x00, 0x4D, 0x22, 0x1C, 0x00, 0x00, 0x00, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_9230_3[] = 
{
    0x00, 0x00, 0x01, 0x01, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 
    0x00, 0x00, 0x4D, 0x22, 0x1C, 0x00, 0x40, 0x06, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_2B95_1[] = 
{
    0x00, 0x00, 0x01, 0x01, 0x80, 0x1A, 0x06, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x20, 0x1C, 0x00, 0x00, 0x00, 
    0x00, 0x00
};

static MMP_UBYTE m_VsProbeCtl_PID_2B95_2[] = 
{
    0x00, 0x00, 0x01, 0x01, 0x80, 0x1A, 0x06, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x20, 0x1C, 0x00, 0xF4, 0x0B, 
    0x00, 0x00
};

static MMP_UBYTE m_VsCommitCtl_8437[] = 
{
#if 1 //TBD???
    0x01, 0x00, 0x03, 0x05, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 
    0x00, 0x00
#else
    0x01, 0x00, 0x03, 0x11, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x26, 0x20, 0x00, 0x00, 0x20, 
    0x00, 0x00
#endif	
};

static MMP_UBYTE m_VsCommitCtl_8451[] = 
{
    0x01, 0x00, 0x03, 0x01, 0x15, 0x16, 0x05, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 
    0x00, 0x00
};

static MMP_UBYTE m_XuCmdIspx02[] = 
{
    0x02, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static MMP_UBYTE m_XuCmdIspx04[] = 
{
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static MMP_UBYTE m_XuCmdIspx08x80_8437[] = 
{
    0x08, 0x80, UVC_DWMAXVIDEOFRAMESIZE_DESC(AIT_MJPEG_MAX_VIDEO_FRM_SIZE), 0x00, 0x00
};

static MMP_UBYTE m_XuCmdIspx08xFF_8437[] = 
{
    0x08, 0xFF, UVC_DWMAXVIDEOFRAMESIZE_DESC(INIT_DEV_MJPEG_BITRATE), 0x00, 0x00
};

static MMP_UBYTE m_XuCmdIspx08_8451[] = 
{
    0x08, 0xFF, 0x40, 0x1F, 0x00, 0x00, 0x00, 0x00
};


static MMP_UBYTE m_XuCmdIspx09[] = 
{
    0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 //default as 60Hz.
};

static MMP_UBYTE m_XuCmdIspx0B[] = 
{
    0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static MMP_UBYTE m_XuCmdIspx11[] = 
{
    0xFF, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static MMP_UBYTE m_XuCmdIspx14[] =
{
    0x14, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};

static MMP_UBYTE m_XuCmdIspx29[] = //NV12 mirror only
{
    0xFF, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static MMP_UBYTE m_XuCmdMmpx09_h264[] = 
{
    0x09, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdMmpx0B[] = 
{
    0x0B, 0x13, 0x40, 0x01, 0xB4, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdMmpx0E[] = 
{
    0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdMmpx0C_8437[] = 
{
    0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdMmpx0C_8451[] = 
{
    0x0C, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

/*
static MMP_UBYTE m_XuCmdMmpx16[] = 
{
    0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};
*/

static MMP_UBYTE m_XuCmdMmpx21[] = 
{
    0x21, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdMmpx22[] = 
{
    0x22, 0xDF, 0x07, 0x01, 0x01, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdMmpx23_h264[] = 
{
    0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdMmpx23_yuv[] = 
{
    0x23, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdMmpx24[] = 
{
    0x24, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 //toggle on (current selected stream)
    //0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 //toggle off (current selected stream)
};

static MMP_UBYTE m_XuCmdMmpx25_h264[] = 
{
    0x25,  0x01,  0x80,  0x07,  0x38,  0x04,  0x1E, 0xC8 //16Mbps, 1920x1080
    //0x25,  0x01,  0x80,  0x07,  0x38,  0x04,  0x1E, 0x64 //8Mbps, 1920x1080
    //0x25,  0x01,  0x80,  0x07,  0x38,  0x04,  0x1E, 0x32 //4Mbps, 1920x1080
    //0x25,  0x01,  0x00,  0x05,  0xD0,  0x02,  0x1E, 0x64 //8Mbps, 1280x720
    //0x25,  0x01,  0x00,  0x05,  0xD0,  0x02,  0x1E, 0x32 //4Mbps, 1280x720
    //0x25,  0x01,  0x00,  0x05,  0xD0,  0x02,  0x1E, 0x19 //2Mbps, 1280x720
    //0x25,  0x01,  0x80,  0x02,  0x68,  0x01,  0x1E, 0x30 //3840Kbps, 640x360
    //0x25,  0x01,  0x80,  0x02,  0x68,  0x01,  0x1E, 0x24 //2880Kbps, 640x360
    //0x25,  0x01,  0x80,  0x02,  0x68,  0x01,  0x1E, 0x18 //1920Kbps, 640x360
};

static MMP_UBYTE m_XuCmdMmpx25_yuv[] = 
{
    //320x180
#if (DEVICE_YUV_TYPE==ST_YUY2)
     0x25,  0x03,  0x40,  0x01,  0xB4,  0x00,  0x7E,  0x0A 
#elif (DEVICE_YUV_TYPE==ST_NV12)
     0x25,  0x04,  0x40,  0x01,  0xB4,  0x00,  0x7E,  0x0A 
#endif
};

static MMP_UBYTE m_XuCmdMmpx29[] = 
{
    0x29, 0xFF, UVC_DEV_MD_DIV_W, UVC_DEV_MD_DIV_H, 0x00, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdMmpx2A[] = 
{
    0x2A, 0x02, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdMmpx2B[] = 
{
    0x2B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};
static MMP_UBYTE m_XuCmdIspx38[] =
{
    0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#if 0
static MMP_UBYTE m_XuCmdMmpx30[] = 
{
    0x30, 0x02, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00 
};

static MMP_UBYTE m_XuCmdSetData32[] = 
{
     'T',  'h',  'i',  's',  ' ',  'i',  's',  ' ', 
     'a',  ' ',  'T',  'e',  'x',  't',  ' ',  'S',
     't',  'r',  'i',  'n',  'g',  '.', 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static MMP_UBYTE m_XuCmdMmp16x01[] = 
{
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static MMP_UBYTE m_XuCmdMmp16x02_x01[] = 
{
    0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static MMP_UBYTE m_XuCmdMmp16x02_x02[] = 
{
    0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

static MMP_UBYTE m_XuCmdx01x1F[] = 
{
    0x1F, 0x10, 0x00, 0xFF
};

static MMP_ULONG                    glUVCDataLength;
static MMP_UBYTE                    gbUVCDataBuf[2048];
UINT32 gulUVCHostError = 0;

MMP_USHORT                          gsAhcUsbhSensor = USBH_SENSOR;

UVC_HOST_CFG gsUVCHostCfg = {0};

#if (MOTION_DETECTION_EN)
static MMP_UBYTE                    gubUVCMDSensitivity = UVC_DEV_MD_SENSITIVITY;
#endif

#if (DUALENC_SUPPORT)
MMP_BOOL		                    gbUVCDeviceEnabled = MMP_FALSE;
#endif

/*===========================================================================
 * Extern function
 *===========================================================================*/ 

extern MMP_USHORT gsAhcPrmSensor;
extern AHC_BOOL gbAhcDbgBrk;
#if (MOTION_DETECTION_EN)
void VRMotionDetectCB(MMP_UBYTE ubSnrSel);
#endif

void MMPF_USBH_CheckJpegEOIEveryPacketEnable(MMP_BOOL ubCheckJPGEOIEnable);
extern void ncam_set_rear_cam_ready(int state);

/*===========================================================================
 * Function prototype
 *===========================================================================*/ 

AHC_BOOL AHC_HostUVCGetRTCTime(UINT8 *pstream)
{
    AHC_RTC_TIME ahc_rtc_time;

    if(pstream == NULL){
        return AHC_FALSE;
    }
    AHC_RTC_GetTime(&ahc_rtc_time);

    *(pstream + 1) = (UINT8)(ahc_rtc_time.uwYear & 0x00FF);
    *(pstream + 2) = (UINT8)((ahc_rtc_time.uwYear >> 8) & 0x00FF);
    *(pstream + 3) = (UINT8)(ahc_rtc_time.uwMonth & 0x00FF);
    *(pstream + 4) = (UINT8)(ahc_rtc_time.uwDay & 0x00FF);
    *(pstream + 5) = (UINT8)(ahc_rtc_time.uwHour & 0x00FF); //Get always be 24 format
    *(pstream + 6) = (UINT8)(ahc_rtc_time.uwMinute & 0x00FF);
    *(pstream + 7) = (UINT8)(ahc_rtc_time.uwSecond & 0x00FF);
    
    return AHC_TRUE;    
}

void AHC_HostUVCVideoOpenUVCCb8437(void)
{    
    MMP_ERR sRet = MMP_ERR_NONE;    
    
    RTNA_DBG_Str0("Open UVC 8437/51 CB\r\n");
    //EU1_SET_ISP
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_ISP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_ISP, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_ISP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_ISP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_ISP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_ISP, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    
    
    // EU1_GET_ISP_RESULT
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_ISP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_ISP_RESULT, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_SET_FW_DATA
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_FW_DATA, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_FW_DATA, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_FW_DATA, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_FW_DATA, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_FW_DATA, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_FW_DATA, 32, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_SET_MMP
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_MMP, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_MMP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_MMP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_MMP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_MMP, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_GET_MMP_RESULT
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_MMP_RESULT, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    //EU1_SET_ISP_EX
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_ISP_EX, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_ISP_EX, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_ISP_EX, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_ISP_EX, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_ISP_EX, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_ISP_EX, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_GET_ISP_EX_RESULT
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_ISP_EX_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_ISP_EX_RESULT, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_ISP_EX_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_ISP_EX_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_ISP_EX_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_ISP_EX_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_READ_MMP_MEM
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_READ_MMP_MEM, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_READ_MMP_MEM, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_READ_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_READ_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_READ_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_READ_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_WRITE_MMP_MEM
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_WRITE_MMP_MEM, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_WRITE_MMP_MEM, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_WRITE_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_WRITE_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_WRITE_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_WRITE_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_GET_CHIP_INFO
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_CHIP_INFO, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_CHIP_INFO, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_CHIP_INFO, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_CHIP_INFO, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_CHIP_INFO, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_CHIP_INFO, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_GET_DATA_32
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_DATA_32, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_DATA_32, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_SET_DATA_32
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_DATA_32, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_DATA_32, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_SET_MMP_CMD16
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP_CMD16, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_MMP_CMD16, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_MMP_CMD16, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_MMP_CMD16, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_MMP_CMD16, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_MMP_CMD16, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_GET_MMP_CMD16_RESULT
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_CMD16_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_MMP_CMD16_RESULT, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_MMP_CMD16_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_MMP_CMD16_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_MMP_CMD16_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_MMP_CMD16_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // CT_EXPOSURE_TIME_ABSOLUTE_CONTROL
    sRet = VideoRecordGetCTCmd(GET_INFO_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MIN_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MAX_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_RES_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_DEF_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MIN_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MAX_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_RES_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // CT_AE_PRIORITY_CONTROL
    sRet = VideoRecordGetCTCmd(GET_INFO_CMD, CT_AE_PRIORITY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MIN_CMD, CT_AE_PRIORITY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVCCmd(GET_CUR_CMD, VC_REQUEST_ERROR_CODE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MAX_CMD, CT_AE_PRIORITY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVCCmd(GET_CUR_CMD, VC_REQUEST_ERROR_CODE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_RES_CMD, CT_AE_PRIORITY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVCCmd(GET_CUR_CMD, VC_REQUEST_ERROR_CODE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_DEF_CMD, CT_AE_PRIORITY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVCCmd(GET_CUR_CMD, VC_REQUEST_ERROR_CODE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // CT_IRIS_ABSOLUTE_CONTROL
    sRet = VideoRecordGetCTCmd(GET_INFO_CMD, CT_IRIS_ABSOLUTE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVCCmd(GET_CUR_CMD, VC_REQUEST_ERROR_CODE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // CT_ZOOM_ABSOLUTE_CONTROL
    sRet = VideoRecordGetCTCmd(GET_INFO_CMD, CT_ZOOM_ABSOLUTE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MIN_CMD, CT_ZOOM_ABSOLUTE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MAX_CMD, CT_ZOOM_ABSOLUTE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_RES_CMD, CT_ZOOM_ABSOLUTE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_DEF_CMD, CT_ZOOM_ABSOLUTE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // CT_PANTILT_ABSOLUTE_CONTROL
    sRet = VideoRecordGetCTCmd(GET_INFO_CMD, CT_PANTILT_ABSOLUTE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MIN_CMD, CT_PANTILT_ABSOLUTE_CONTROL, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MAX_CMD, CT_PANTILT_ABSOLUTE_CONTROL, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_RES_CMD, CT_PANTILT_ABSOLUTE_CONTROL, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_DEF_CMD, CT_PANTILT_ABSOLUTE_CONTROL, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_BRIGHTNESS_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_BRIGHTNESS_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_BRIGHTNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_BRIGHTNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_BRIGHTNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_BRIGHTNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_CONTRAST_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_CONTRAST_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_CONTRAST_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_CONTRAST_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_CONTRAST_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_CONTRAST_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    #if (0)
    // PU_HUE_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_HUE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_HUE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_HUE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_HUE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_HUE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    #endif
    // PU_SATURATION_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_SATURATION_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_SATURATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_SATURATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_SATURATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_SATURATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_SHARPNESS_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_SHARPNESS_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_SHARPNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_SHARPNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_SHARPNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_SHARPNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    #if (0)
    // PU_GAMMA_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_GAMMA_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_GAMMA_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_GAMMA_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_GAMMA_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_GAMMA_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    #endif
    // PU_WHITE_BALANCE_TEMPERATURE_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD,  PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD,  PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD,  PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD,  PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_BACKLIGHT_COMPENSATION_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_GAIN_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_GAIN_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_GAIN_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_GAIN_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_GAIN_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_GAIN_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_POWER_LINE_FREQUENCY_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    
}

void AHC_HostUVCVideoStartUVCCb8437(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    
    AHC_BOOL ahcRet = AHC_TRUE;
    
    RTNA_DBG_Str0("Start UVC 8437 CB\r\n");
    
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx0C_8437); //Select Stream Layout

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx23_h264); //Select Multicast Stream ID
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx25_h264); //Multicast Commit
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
	
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx09_h264); //GOP Commit
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
	
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx24); //Toggle Layer Control

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx23_yuv); //Select Multicast Stream ID
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx25_yuv); //Multicast Commit
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx24); //Toggle Layer Control

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    ahcRet = AHC_HostUVCResetTime();
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    
    
#if (MOTION_DETECTION_EN)    
    m_XuCmdMmpx29[1] = 0xFF;
    m_XuCmdMmpx29[2] = UVC_DEV_MD_DIV_W;
    m_XuCmdMmpx29[3] = UVC_DEV_MD_DIV_H;
    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx29); // MD reset
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
    ahcRet = AHC_HostUVCSetMDSensitivity(gubUVCMDSensitivity);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    
#endif

    sRet = VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf); //VS_PROBE_CONTROL => UVC_DMA_SIZE => transaction
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_8437); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf); //VS_PROBE_CONTROL => UVC_DMA_SIZE => transaction
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_COMMIT_CONTROL, 26, m_VsCommitCtl_8437); //VS_COMMIT_CONTROL
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
    //VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx04);
}

void AHC_HostUVCVideoOpenUVCCb8328(void)
{    
    MMP_ERR sRet = MMP_ERR_NONE;    
    
    RTNA_DBG_Str0("Open UVC 8328 CB\r\n");
    //EU1_SET_ISP
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_ISP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_ISP, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_ISP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_ISP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_ISP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_ISP, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    
    
    // EU1_GET_ISP_RESULT
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_ISP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_ISP_RESULT, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_SET_FW_DATA
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_FW_DATA, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_FW_DATA, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_FW_DATA, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_FW_DATA, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_FW_DATA, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_FW_DATA, 32, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_SET_MMP
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_MMP, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_MMP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_MMP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_MMP, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_MMP, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_GET_MMP_RESULT
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_MMP_RESULT, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    //EU1_SET_ISP_EX
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_ISP_EX, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_ISP_EX, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_ISP_EX, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_ISP_EX, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_ISP_EX, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_ISP_EX, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_GET_ISP_EX_RESULT
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_ISP_EX_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_ISP_EX_RESULT, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_ISP_EX_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_ISP_EX_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_ISP_EX_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_ISP_EX_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_READ_MMP_MEM
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_READ_MMP_MEM, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_READ_MMP_MEM, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_READ_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_READ_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_READ_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_READ_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_WRITE_MMP_MEM
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_WRITE_MMP_MEM, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_WRITE_MMP_MEM, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_WRITE_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_WRITE_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_WRITE_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_WRITE_MMP_MEM, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_GET_CHIP_INFO
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_CHIP_INFO, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_CHIP_INFO, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_CHIP_INFO, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_CHIP_INFO, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_CHIP_INFO, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_CHIP_INFO, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_GET_DATA_32
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_DATA_32, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_DATA_32, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_SET_DATA_32
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_DATA_32, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_DATA_32, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_DATA_32, 32, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_SET_MMP_CMD16
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP_CMD16, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_SET_MMP_CMD16, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_SET_MMP_CMD16, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_SET_MMP_CMD16, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_SET_MMP_CMD16, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_SET_MMP_CMD16, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // EU1_GET_MMP_CMD16_RESULT
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_CMD16_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_INFO_CMD, EU1_GET_MMP_CMD16_RESULT, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MIN_CMD, EU1_GET_MMP_CMD16_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_MAX_CMD, EU1_GET_MMP_CMD16_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_RES_CMD, EU1_GET_MMP_CMD16_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_DEF_CMD, EU1_GET_MMP_CMD16_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // CT_EXPOSURE_TIME_ABSOLUTE_CONTROL
    sRet = VideoRecordGetCTCmd(GET_INFO_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MIN_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MAX_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_RES_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_DEF_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MIN_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MAX_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_RES_CMD, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 4, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // CT_AE_PRIORITY_CONTROL
    sRet = VideoRecordGetCTCmd(GET_INFO_CMD, CT_AE_PRIORITY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MIN_CMD, CT_AE_PRIORITY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVCCmd(GET_CUR_CMD, VC_REQUEST_ERROR_CODE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MAX_CMD, CT_AE_PRIORITY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVCCmd(GET_CUR_CMD, VC_REQUEST_ERROR_CODE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_RES_CMD, CT_AE_PRIORITY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVCCmd(GET_CUR_CMD, VC_REQUEST_ERROR_CODE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_DEF_CMD, CT_AE_PRIORITY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVCCmd(GET_CUR_CMD, VC_REQUEST_ERROR_CODE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // CT_IRIS_ABSOLUTE_CONTROL
    sRet = VideoRecordGetCTCmd(GET_INFO_CMD, CT_IRIS_ABSOLUTE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVCCmd(GET_CUR_CMD, VC_REQUEST_ERROR_CODE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // CT_ZOOM_ABSOLUTE_CONTROL
    sRet = VideoRecordGetCTCmd(GET_INFO_CMD, CT_ZOOM_ABSOLUTE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MIN_CMD, CT_ZOOM_ABSOLUTE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MAX_CMD, CT_ZOOM_ABSOLUTE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_RES_CMD, CT_ZOOM_ABSOLUTE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_DEF_CMD, CT_ZOOM_ABSOLUTE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // CT_PANTILT_ABSOLUTE_CONTROL
    sRet = VideoRecordGetCTCmd(GET_INFO_CMD, CT_PANTILT_ABSOLUTE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MIN_CMD, CT_PANTILT_ABSOLUTE_CONTROL, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_MAX_CMD, CT_PANTILT_ABSOLUTE_CONTROL, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_RES_CMD, CT_PANTILT_ABSOLUTE_CONTROL, 8, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetCTCmd(GET_DEF_CMD, CT_PANTILT_ABSOLUTE_CONTROL, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_BRIGHTNESS_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_BRIGHTNESS_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_BRIGHTNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_BRIGHTNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_BRIGHTNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_BRIGHTNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_CONTRAST_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_CONTRAST_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_CONTRAST_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_CONTRAST_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_CONTRAST_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_CONTRAST_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    #if (0)
    // PU_HUE_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_HUE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_HUE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_HUE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_HUE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_HUE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    #endif
    // PU_SATURATION_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_SATURATION_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_SATURATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_SATURATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_SATURATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_SATURATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_SHARPNESS_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_SHARPNESS_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_SHARPNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_SHARPNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_SHARPNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_SHARPNESS_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    #if (0)
    // PU_GAMMA_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_GAMMA_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_GAMMA_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_GAMMA_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_GAMMA_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_GAMMA_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    #endif
    // PU_WHITE_BALANCE_TEMPERATURE_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD,  PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD,  PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD,  PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD,  PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_BACKLIGHT_COMPENSATION_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_GAIN_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_GAIN_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_GAIN_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_GAIN_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_GAIN_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_GAIN_CONTROL, 2, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    

    // PU_POWER_LINE_FREQUENCY_CONTROL	
    sRet = VideoRecordGetPUCmd(GET_INFO_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MIN_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_MAX_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_RES_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetPUCmd(GET_DEF_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    
}

void AHC_HostUVCVideoStartUVCCb8328(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    
    AHC_BOOL ahcRet = AHC_TRUE;
    
    RTNA_DBG_Str0("Start UVC 8328 CB\r\n");
    
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx0C_8437); //Select Stream Layout

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

#if(1)//H264 stream
	m_XuCmdMmpx23_h264[1] = 0;
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx23_h264); //Select Multicast Stream ID
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx25_h264); //Multicast Commit
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
	
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx09_h264); //GOP Commit
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
	
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx24); //Toggle Layer Control

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
#endif

#if(1)//MJPEG stream
	m_XuCmdMmpx23_yuv[1] = 3;
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx23_yuv); //Select Multicast Stream ID
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx25_yuv); //Multicast Commit
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx24); //Toggle Layer Control
#endif

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    ahcRet = AHC_HostUVCResetTime();
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    
    
#if (MOTION_DETECTION_EN)    
    m_XuCmdMmpx29[1] = 0xFF;
    m_XuCmdMmpx29[2] = UVC_DEV_MD_DIV_W;
    m_XuCmdMmpx29[3] = UVC_DEV_MD_DIV_H;
    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx29); // MD reset
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
    ahcRet = AHC_HostUVCSetMDSensitivity(gubUVCMDSensitivity);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }    
#endif

    sRet = VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf); //VS_PROBE_CONTROL => UVC_DMA_SIZE => transaction
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_8437); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf); //VS_PROBE_CONTROL => UVC_DMA_SIZE => transaction
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_COMMIT_CONTROL, 26, m_VsCommitCtl_8437); //VS_COMMIT_CONTROL
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
    //VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx04);
}

void AHC_HostUVCVideoStartUVCCb8451(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    
    AHC_BOOL ahcRet = AHC_TRUE;
    
    RTNA_DBG_Str0("Start UVC 8451 CB\r\n");
    
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx0C_8451); //set signal type
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx0B); //set Encoding Resolution (NV12)
    
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx25_h264); //Multicast Commit
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
	
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx09_h264); //GOP Commit
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
	
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx24); //Toggle Layer Control
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_ISP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx02); //set framerate
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_ISP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_ISP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx14); //set framerate
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_ISP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx25_yuv); //Multicast Commit
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx24); //Toggle Layer Control

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    ahcRet = AHC_HostUVCResetTime();
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }        

#if (MOTION_DETECTION_EN)    
    m_XuCmdMmpx29[1] = 0xFF;
    m_XuCmdMmpx29[2] = UVC_DEV_MD_DIV_W;
    m_XuCmdMmpx29[3] = UVC_DEV_MD_DIV_H;
    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx29); // MD reset
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
    ahcRet = AHC_HostUVCSetMDSensitivity(gubUVCMDSensitivity);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }        
#endif

    sRet = VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_8451); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf); //VS_PROBE_CONTROL => UVC_DMA_SIZE => transaction
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_COMMIT_CONTROL, 26, m_VsCommitCtl_8451); //VS_COMMIT_CONTROL
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
    RTNA_DBG_Str(0, "= GET_LEN_CMD =\r\n");
    
    sRet = VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_ISP, 2, &glUVCDataLength, gbUVCDataBuf);
    //RTNA_DBG_Str(0, "= SET_CUR_CMD =\r\n");
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx08_8451);
    
    //RTNA_DBG_Str(0, "= GET_LEN_CMD =\r\n");
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_ISP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    //RTNA_DBG_Str(0, "= GET_CUR_CMD =\r\n");
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
    //VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx04);
}

void AHC_HostUVCVideoOpenUVCCb_PID_0210(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    

    RTNA_DBG_Str0("Open UVC PID 0210 CB\r\n");

    // VS_PROBE_CONTROL
    sRet = VideoRecordGetVSCmd(GET_DEF_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    // EU3 x01
    sRet = VideoRecordGetEU3Cmd(GET_LEN_CMD, 0x01, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU3Cmd(GET_INFO_CMD, 0x01, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU3Cmd(SET_CUR_CMD, 0x01, 4, m_XuCmdx01x1F);
    sRet |= VideoRecordGetEU3Cmd(GET_CUR_CMD, 0x01, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU3Cmd(SET_CUR_CMD, 0x01, 4, m_XuCmdx01x1F);
    sRet |= VideoRecordGetEU3Cmd(GET_CUR_CMD, 0x01, 4, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    // GET_DESCRIPTOR, STRING_DESC
    //VideoRecordGetStdDevCmd(GET_DESCRIPTOR,((STRING_DESCR<<8)|0x05),0x0409,0xFF,&glUVCDataLength,gbUVCDataBuf);

}

void AHC_HostUVCVideoStartUVCCb_PID_0210(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    

    RTNA_DBG_Str0("Start UVC PID 0210 CB\r\n");
    
    //VS_PROBE_CONTROL
    sRet = VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
    //
    sRet = VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_3); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_3); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_COMMIT_CONTROL, 26, m_VsProbeCtl_PID_64AB_4); //VS_COMMIT_CONTROL

	// SET_INTERFACE, 1.6
    sRet |= VideoRecordSetStdIfCmd(SET_INTERFACE,0x06,0x01,0,NULL);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoOpenUVCCb_PID_64AB
//  Description :
//------------------------------------------------------------------------------
void AHC_HostUVCVideoOpenUVCCb_PID_64AB(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    

    RTNA_DBG_Str0("Open UVC PID 64AB CB\r\n");
    // VS_PROBE_CONTROL
    sRet = VideoRecordGetVSCmd(GET_DEF_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    // EU x02
    sRet = VideoRecordGetEU3Cmd(GET_LEN_CMD, 0x01, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU3Cmd(GET_INFO_CMD, 0x01, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU3Cmd(SET_CUR_CMD, 0x01, 4, m_XuCmdx01x1F);
    sRet |= VideoRecordGetEU3Cmd(GET_CUR_CMD, 0x01, 4, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU3Cmd(SET_CUR_CMD, 0x01, 4, m_XuCmdx01x1F);
    sRet |= VideoRecordGetEU3Cmd(GET_CUR_CMD, 0x01, 4, &glUVCDataLength, gbUVCDataBuf);

    // GET_DESCRIPTOR, STRING_DESC
    sRet |= VideoRecordGetStdDevCmd(GET_DESCRIPTOR,((STRING_DESCR<<8)|0x05),0x0409,0xFF,&glUVCDataLength,gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoStartUVCCb_PID_64AB
//  Description :
//------------------------------------------------------------------------------
void AHC_HostUVCVideoStartUVCCb_PID_64AB(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    

    RTNA_DBG_Str0("Start UVC PID 64AB CB\r\n");

    //VS_PROBE_CONTROL
    sRet = VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
    //
    sRet = VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_3); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_3); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_64AB_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_COMMIT_CONTROL, 26, m_VsProbeCtl_PID_64AB_4); //VS_COMMIT_CONTROL

    // SET_INTERFACE, 1.3
    //VideoRecordSetStdIfCmd(SET_INTERFACE,0x06,0x01,0,NULL);
    sRet |= VideoRecordSetStdIfCmd(SET_INTERFACE,0x03,0x01,0,NULL); //use IF 3, due to not support ISO high-bandwidth
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
}
void AHC_HostUVCVideoOpenUVCCb_PID_8301(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    

    RTNA_DBG_Str0("Open UVC PID 8301 CB\r\n");
	if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
	
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoStartUVCCb_PID_64AB
//  Description :
//------------------------------------------------------------------------------
void AHC_HostUVCVideoStartUVCCb_PID_8301(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    

    RTNA_DBG_Str0("Start UVC PID 8301 CB\r\n");
#if 0
	sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_8301_1); //VS_PROBE_CONTROL
#endif    
    
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_8301_1); //VS_PROBE_CONTROL
    
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_COMMIT_CONTROL, 26, m_VsProbeCtl_PID_8301_2); //VS_PROBE_CONTROL
    
    //This hardcoding is from Sunplus different FW with the same PID/VID.
    if (gUsbh_UvcDevInfo.wSzConfigDesc == (0x00F9)) {
        sRet |= VideoRecordSetStdIfCmd(SET_INTERFACE,0x02,0x01,0,NULL); 
    }
    else if (gUsbh_UvcDevInfo.wSzConfigDesc == (0x0141)) {
        sRet |= VideoRecordSetStdIfCmd(SET_INTERFACE,0x06,0x01,0,NULL); 
    }
    else {
        printc("UVC device descriptor length error!\r\n");
        AHC_PRINT_RET_ERROR(0, 0);
    }
    
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(sRet,0); } 
}

void AHC_HostUVCVideoOpenUVCCb_PID_9230(void)
{
    RTNA_DBG_Str0("Open UVC PID 9230 CB\r\n");
}
void AHC_HostUVCVideoStartUVCCb_PID_9230(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    
    RTNA_DBG_Str0("Start UVC PID 9230 CB\r\n");
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_9230_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_9230_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_COMMIT_CONTROL, 26, m_VsProbeCtl_PID_9230_3); //VS_PROBE_CONTROL
    sRet |= VideoRecordSetStdIfCmd(SET_INTERFACE,0x02,0x01,0,NULL); //use IF 3, due to not support ISO high-bandwidth
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
}

void AHC_HostUVCVideoOpenUVCCb_PID_2B95(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    
    RTNA_DBG_Str0("Open UVC PID 2B95 CB\r\n");
	if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
}

void AHC_HostUVCVideoStartUVCCb_PID_2B95(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    
    RTNA_DBG_Str0("Start UVC PID 2B95 CB\r\n");
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_2B95_1); //VS_PROBE_CONTROL
    
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_2B95_1); //VS_PROBE_CONTROL
    
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_COMMIT_CONTROL, 26, m_VsProbeCtl_PID_2B95_2); //VS_PROBE_CONTROL
    
    sRet |= VideoRecordSetStdIfCmd(SET_INTERFACE,0x02,0x01,0,NULL); //use IF 3, due to not support ISO high-bandwidth
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
}


//------------------------------------------------------------------------------
//  Function    : OpenUVCCb_PID_6300
//  Description : 
//------------------------------------------------------------------------------
void AHC_HostUVCVideoOpenUVCCb_PID_6300(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    

    RTNA_DBG_Str0("Open UVC PID 6300 CB\r\n");
    // VS_PROBE_CONTROL
    sRet = VideoRecordGetVSCmd(GET_DEF_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);

    // EU x03
    sRet |= VideoRecordGetEU3Cmd(GET_LEN_CMD, 0x01, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU3Cmd(GET_INFO_CMD, 0x01, 1, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU3Cmd(SET_CUR_CMD, 0x01, 4, m_XuCmdx01x1F);
    sRet |= VideoRecordSetEU3Cmd(SET_CUR_CMD, 0x01, 4, m_XuCmdx01x1F);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 

    // GET_DESCRIPTOR, STRING_DESC
    //VideoRecordGetStdDevCmd(GET_DESCRIPTOR,((STRING_DESCR<<8)|0x05),0x0409,0xFF,&glUVCDataLength,gbUVCDataBuf);

}

//------------------------------------------------------------------------------
//  Function    : StartUVCCb_PID_6300
//  Description : 
//------------------------------------------------------------------------------
void AHC_HostUVCVideoStartUVCCb_PID_6300(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;    

    RTNA_DBG_Str0("Start UVC PID 6300 CB\r\n");

    //VS_PROBE_CONTROL
    sRet = VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
    //
    sRet = VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300_2); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    //
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl_PID_6300_1); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_COMMIT_CONTROL, 26, m_VsProbeCtl_PID_6300_3); //VS_COMMIT_CONTROL

    // SET_INTERFACE, 1.3
    //VideoRecordSetStdIfCmd(SET_INTERFACE,0x06,0x01,0,NULL);
    sRet |= VideoRecordSetStdIfCmd(SET_INTERFACE,0x03,0x01,0,NULL); //use IF 3, due to 8428 not support ISO high-bandwidth
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
    
}

void AHC_HostUVCVideoStartCb8589(void)
{
    //MMP_ERR sRet = MMP_ERR_NONE;    

    RTNA_DBG_Str0("Start UVC 8589 CB\r\n");
#if 0 //Not support now.    
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP_CMD16, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP_CMD16, 16, m_XuCmdMmp16x02_x01); // SEC TV options

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_MMP_CMD16_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_CMD16_RESULT, 16, &glUVCDataLength, gbUVCDataBuf);

    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP_CMD16, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP_CMD16, 16, m_XuCmdMmp16x02_x02); // SEC TV options
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_MMP_CMD16, 2, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP_CMD16, 16, m_XuCmdMmp16x01); //set signal type
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); } 
 	
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_PROBE_CONTROL, 26, m_VsProbeCtl); //VS_PROBE_CONTROL
    sRet |= VideoRecordGetVSCmd(GET_CUR_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    sRet |= VideoRecordGetVSCmd(GET_MAX_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf); //VS_PROBE_CONTROL => UVC_DMA_SIZE => transaction
    sRet |= VideoRecordGetVSCmd(GET_MIN_CMD, VS_PROBE_CONTROL, 26, &glUVCDataLength, gbUVCDataBuf);
    
    sRet |= VideoRecordSetVSCmd(SET_CUR_CMD, VS_COMMIT_CONTROL, 26, m_VsCommitCtl); //VS_COMMIT_CONTROL
    
    RTNA_DBG_Str(0, "= GET_LEN_CMD =\r\n");
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_SET_ISP, 2, &glUVCDataLength, gbUVCDataBuf);
    //RTNA_DBG_Str(0, "= SET_CUR_CMD =\r\n");
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx08_8451);
    
    //RTNA_DBG_Str(0, "= GET_LEN_CMD =\r\n");
    sRet |= VideoRecordGetEU1Cmd(GET_LEN_CMD, EU1_GET_ISP_RESULT, 2, &glUVCDataLength, gbUVCDataBuf);
    //RTNA_DBG_Str(0, "= GET_CUR_CMD =\r\n");
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_ISP_RESULT, 8, &glUVCDataLength, gbUVCDataBuf);
    
    //VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx04);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); }     
#endif    
    return ;
}

void UVCRecordCbFileFull(void)
{
	printc("## UVC File full CB ##\r\n");
}

#if 0
//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoUSBPathSelect
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Select USB path by USB_PATH pin

 Select USB path by USB_PATH pin.
 Parameters: GPIO_LOW: Device mode, GPIO_HIGH: Host mode
 @param[in] USB_PATH pin
 @retval NONE
*/
void AHC_HostUVCVideoUSBPathSelect(AHC_BOOL ubUsbPath) // TBD
{
#if defined(USB_PATH_SELECT_GPIO)
	if (USB_PATH_SELECT_GPIO != MMP_GPIO_MAX)
	{
		MMPF_PIO_EnableOutputMode(USB_PATH_SELECT_GPIO, MMP_TRUE, MMP_FALSE);
		if (ubUsbPath)
			MMPF_PIO_SetData(USB_PATH_SELECT_GPIO, USB_PATH_SELECT_SET, MMP_TRUE);
		else
			MMPF_PIO_SetData(USB_PATH_SELECT_GPIO, !USB_PATH_SELECT_SET, MMP_TRUE);
	}
#endif
}
#endif

void AHC_HostUVCVideoReadErrorType(UINT32 *pErrorType)
{
    *pErrorType = gulUVCHostError;
}

void AHC_HostUVCVideoClearError(UINT32 ulErrorType)
{
    gulUVCHostError &= (~ulErrorType);    
}

void AHC_HostUVCVideoSetEp0TimeoutCB(void)
{
    gulUVCHostError |= UVC_HOST_ERROR_EP0_TIMEOUT;    
}

void AHC_HostUVCVideoSetTranTimeoutCB(void)
{
    gulUVCHostError |= UVC_HOST_ERROR_TRAN_TIMEOUT;
}

void AHC_HostUVCVideoSetFrmRxTimeoutCB(void)
{
    gulUVCHostError |= UVC_HOST_ERROR_FRM_RX_LOST;
}

void AHC_HostUVCVideoSetFrmRxTimeout2CB(void)
{
    gulUVCHostError |= UVC_HOST_ERROR_FRM_RX_LOST_TH_2;
}

void AHC_HostUVCVideoSetFrmSeqNoLostCB(void)
{
    gulUVCHostError |= UVC_HOST_ERROR_SEQ_NO_LOST;
}

void AHC_HostUVCVideoSetRecdFrmRxTimeoutCB(void)
{
    gulUVCHostError |= UVC_HOST_ERROR_RECD_FRM_RX_LOST_TH;
}

void AHC_HostUVCVideoSetFrmRxDropCB(void)
{
    gulUVCHostError |= UVC_HOST_ERROR_FRM_RX_LOST_TH_2; //Share with FRM RX lost 2.  Disconnect device.
}

void AHC_HostUVCVideoDevDisconnectedCB(void)
{
    gulUVCHostError |= UVC_HOST_ERROR_DISCONNECTED; 
}

AHC_BOOL AHC_HostUVCVideoSetWinActive(UINT8 ubWinID, AHC_BOOL bWinActive)
{
    MMP_ERR sRet = MMP_ERR_NONE;
    
    if (bWinActive) {
        sRet = MMPD_Display_SetWinActive(ubWinID, MMP_TRUE); 
    }
    else {
        sRet = MMPD_Display_SetWinActive(ubWinID, MMP_FALSE); 
    }
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}   

    return AHC_TRUE;
}

#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
AHC_BOOL AHC_HostUVCVideoInitDecMjpegToPreview( UINT8    ubWinID, 
                                                UINT16   usWinW,    UINT16   usWinH, 
                                                UINT16   usDispW,   UINT16   usDispH, 
                                                UINT16   usOffsetX, UINT16   usOffsetY,
                                                MMP_DISPLAY_COLORMODE sDispColor)
{
    MMP_ERR sRet = MMP_ERR_NONE;

    if (!MMP_IsSupportDecMjpegToPreview()) {
        return AHC_TRUE;
    }

#if (TVOUT_PREVIEW_EN)
    if(AHC_IsTVConnectEx()||AHC_IsHdmiConnect()){
         sRet = MMPS_3GPRECD_SetDecMjpegToPreviewDispAttr(ubWinID,
                                              MMP_FALSE,
                                              MMP_DISPLAY_ROTATE_NO_ROTATE,
                                              MMP_SCAL_FITMODE_OPTIMAL,
                                              usOffsetX, usOffsetY, usWinW, usWinH, usDispW, usDispH, sDispColor);
    }
    else
#endif
    {
#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
         sRet = MMPS_3GPRECD_SetDecMjpegToPreviewDispAttr(ubWinID,
                                                  MMP_FALSE,
                                                  MMP_DISPLAY_ROTATE_NO_ROTATE,
                                                  MMP_SCAL_FITMODE_OPTIMAL,
                                                  usOffsetX, usOffsetY, usWinW, usWinH, usDispW, usDispH, sDispColor);
#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90)
         sRet = MMPS_3GPRECD_SetDecMjpegToPreviewDispAttr(ubWinID,
                                                  MMP_TRUE,
                                                  MMP_DISPLAY_ROTATE_RIGHT_90,
                                                  MMP_SCAL_FITMODE_OPTIMAL,
                                                  usOffsetX, usOffsetY, usWinW, usWinH, usDispW, usDispH, sDispColor);   
#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
         sRet = MMPS_3GPRECD_SetDecMjpegToPreviewDispAttr(ubWinID,
                                                  MMP_TRUE,
                                                  MMP_DISPLAY_ROTATE_RIGHT_270,
                                                  MMP_SCAL_FITMODE_OPTIMAL,
                                                  usOffsetX, usOffsetY, usWinW, usWinH, usDispW, usDispH, sDispColor);   
#endif
    }

    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   

    sRet = MMPS_3GPRECD_SetDispColorFmtToJpgAttr(sDispColor);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   
    
    if ((gUsbh_UvcDevInfo.wPrevwMaxWidth == 0) || 
        (gUsbh_UvcDevInfo.wPrevwMaxHeight == 0) || 
        (gUsbh_UvcDevInfo.bParseDone == MMP_FALSE)) {
        RTNA_DBG_Str(0, "[ERR]: Incorrect seq:");
        RTNA_DBG_Byte(0, gUsbh_UvcDevInfo.bParseDone);
        RTNA_DBG_Short(0, gUsbh_UvcDevInfo.wPrevwMaxWidth);
        RTNA_DBG_Short(0, gUsbh_UvcDevInfo.wPrevwMaxHeight);
        RTNA_DBG_Str(0, " \r\n");
    }

    sRet = MMPS_3GPRECD_SetDecMjpegToPreviewSrcAttr(gUsbh_UvcDevInfo.wPrevwMaxWidth, gUsbh_UvcDevInfo.wPrevwMaxHeight); //CUR_DEV_64AB_W, CUR_DEV_64AB_H);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   

    return AHC_TRUE;
}
#endif

AHC_BOOL AHC_HostUVCVideoSetWinAttribute(UINT8 ubWinID, UINT16 usDisplayW, UINT16 usDisplayH, UINT16 usOffsetX, UINT16 usOffsetY, UINT8 sFitMode, AHC_BOOL bRotate)
{
    if (MMP_IsUSBCamIsoMode() && MMP_IsSupportDecMjpegToPreview()) 
    {
        MMP_SCAL_FIT_RANGE      fitrange;
        MMP_SCAL_GRAB_CTRL      dispgrabctl;
        MMP_USHORT              usW = 0;
        MMP_USHORT              usH = 0;
        MMP_DISPLAY_DISP_ATTR   dispAttr;
        MMP_DISPLAY_WIN_ATTR    winAttr;

        UINT16                  ubOldWinID = 0;
        UINT16                  usOldWinW = 0, usOldWinH = 0, usOldDisplayW = 0, usOldDisplayH = 0, usOldOffsetX = 0, usOldOffsetY = 0;
        MMP_USHORT              usDispColor;
        MMP_ERR                 sRet = MMP_ERR_NONE;
        AHC_BOOL                ahcRet = AHC_TRUE;
        printc("----------1232----------\r\n");
        ahcRet = AHC_HostUVCVideoSetWinActive(ubWinID, AHC_FALSE); //active after frame ready
        if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); /*return AHC_FALSE;*/}    
        
        sRet = MMPS_3GPRECD_GetDecMjpegToPreviewDispAttr(  &ubOldWinID,
                                                    &usOldOffsetX,  &usOldOffsetY,
                                                    &usOldWinW,     &usOldWinH,
                                                    &usOldDisplayW, &usOldDisplayH,
                                                    &usDispColor);
	//	 printc("----------1232----------\r\n");
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   

        sRet = MMPS_Display_GetWinAttributes(ubWinID, &winAttr);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   
	printc("usOldWinW=%d. usOldWinH=%d \r\n",usOldWinW,usOldWinH);
        usW = usOldWinW;
        usH = usOldWinH;

        dispAttr.usStartX          = 0;
        dispAttr.usStartY          = 0;

        if (AHC_IsTVConnectEx()||AHC_IsHdmiConnect())
        {
            dispAttr.usDisplayWidth    = usW; 
            dispAttr.usDisplayHeight   = usH; 
            dispAttr.usDisplayOffsetX  = (MMP_USHORT)usOffsetX;
            dispAttr.usDisplayOffsetY  = (MMP_USHORT)usOffsetY;
            dispAttr.rotatetype        = MMP_DISPLAY_ROTATE_NO_ROTATE;	
        }
        else
        {
        #if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
            dispAttr.usDisplayWidth    = usW; 
            dispAttr.usDisplayHeight   = usH; 
            dispAttr.usDisplayOffsetX  = (MMP_USHORT)usOffsetX;
            dispAttr.usDisplayOffsetY  = (MMP_USHORT)usOffsetY;
            dispAttr.rotatetype        = MMP_DISPLAY_ROTATE_NO_ROTATE;
        #elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
			dispAttr.usDisplayWidth    = FLOOR16(usH);
			dispAttr.usDisplayHeight   = (usW);
			dispAttr.usDisplayOffsetX  = (MMP_USHORT)usOffsetX;
			dispAttr.usDisplayOffsetY  = (MMP_USHORT)usOffsetY;
			dispAttr.rotatetype        = MMP_DISPLAY_ROTATE_NO_ROTATE;
        #endif
        }
        
        dispAttr.bMirror           = MMP_FALSE;
	printc("dispAttr.usDisplayWidth=%d////dispAttr.usDisplayHeight=%d///\r\n",dispAttr.usDisplayWidth,dispAttr.usDisplayHeight);
	printc("dispAttr.usDisplayOffsetX=%d////dispAttr.usDisplayOffsetY=%d///\r\n",dispAttr.usDisplayOffsetX,dispAttr.usDisplayOffsetY);
        sRet = MMPD_Display_SetWinToDisplay(ubWinID, &dispAttr);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   

        /* Window Scaling */
        fitrange.fitmode        = sFitMode;
        fitrange.scalerType     = MMP_SCAL_TYPE_WINSCALER;

        if (AHC_IsTVConnectEx() || AHC_IsHdmiConnect())
        {
            fitrange.ulInWidth      = usW; 
            fitrange.ulInHeight     = usH; 
            fitrange.ulOutWidth     = (MMP_USHORT)usDisplayW;
            fitrange.ulOutHeight    = (MMP_USHORT)usDisplayH;	
        }
        else
        {
        #if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
            fitrange.ulInWidth      = usW; 
            fitrange.ulInHeight     = usH; 
            fitrange.ulOutWidth     = (MMP_USHORT)usDisplayW;
            fitrange.ulOutHeight    = (MMP_USHORT)usDisplayH;
        #elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
			fitrange.ulInWidth      = FLOOR16(usH);
			fitrange.ulInHeight     =  (usW);
			fitrange.ulOutWidth     = (MMP_USHORT)usDisplayW;
			fitrange.ulOutHeight    = (MMP_USHORT)usDisplayH;
        #endif
        }
        
        fitrange.ulInGrabX      = 1; 
        fitrange.ulInGrabY     	= 1;
        fitrange.ulInGrabW      = fitrange.ulInWidth;
        fitrange.ulInGrabH     	= fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        sRet = MMPD_Scaler_GetGCDBestFitScale(&fitrange, &dispgrabctl);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   
        
        sRet = MMPD_Display_SetWinScaling(ubWinID, &fitrange, &dispgrabctl);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   
    }
    else //AIT 8451 or AIT8455
    {
        MMP_DISPLAY_DISP_ATTR   dispAttr;
        MMP_DISPLAY_WIN_ATTR    winAttr;
        MMP_SCAL_FIT_RANGE      fitrange;
        MMP_SCAL_GRAB_CTRL      dispgrabctl;
        MMP_USHORT              usW = 0;
        MMP_USHORT              usH = 0;
        UINT16                  ubOldWinID = 0;
        UINT16                  usOldWinW = 0, usOldWinH = 0, usOldDisplayW = 0, usOldDisplayH = 0, usOldOffsetX = 0, usOldOffsetY = 0;
        MMP_USHORT              usDispColor;
        MMP_ERR                 sRet = MMP_ERR_NONE;
        AHC_BOOL                ahcRet = AHC_TRUE;

        /* Config YUV */
        switch(gsUVCHostCfg.sYUVResol){
            case UVC_YUV_RESOL_90P:
                usW = 160;
                usH = 90;
                break;
            case UVC_YUV_RESOL_240P:
                usW = 320;
                usH = 240;
                break;
            case UVC_YUV_RESOL_360P:
                usW = 640;
                usH = 360;
                break; 
            case UVC_YUV_RESOL_480P:
                usW = 640;
                usH = 480;
                break;
            case UVC_YUV_RESOL_180P:
            default:
                usW = 320;
                usH = 180;
                break;
        }

        ahcRet = AHC_HostUVCVideoSetWinActive(ubWinID, AHC_FALSE); //active after frame ready
        if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); /*return AHC_FALSE;*/}    
        
        sRet = MMPS_3GPRECD_GetDecMjpegToPreviewDispAttr(  &ubOldWinID,
                                                    &usOldOffsetX,  &usOldOffsetY,
                                                    &usOldWinW,     &usOldWinH,
                                                    &usOldDisplayW, &usOldDisplayH,
                                                    &usDispColor);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}
        
        sRet = MMPS_Display_GetWinAttributes(ubWinID, &winAttr);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   
        
        if (CAM_CHECK_USB(USB_CAM_AIT) && MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_MJPEG_H264) {
            usW = usOldWinW;
            usH = usOldWinH;
        }
        
        /* For NV12 Format */
        if (AHC_IsTVConnectEx()||AHC_IsHdmiConnect())
        {
        	winAttr.usWidth             = usW;
			winAttr.usHeight            = usH;
			winAttr.usLineOffset        = winAttr.usWidth;
			winAttr.colordepth          = MMP_DISPLAY_WIN_COLORDEPTH_YUV420_INTERLEAVE;
        }
        else
        {
		#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
			winAttr.usWidth             = usW;
			winAttr.usHeight            = usH;
			winAttr.usLineOffset        = winAttr.usWidth;
			winAttr.colordepth          = MMP_DISPLAY_WIN_COLORDEPTH_YUV420_INTERLEAVE;
		#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
			winAttr.usWidth				= FLOOR16(usH);
			winAttr.usHeight			= (usW);
			winAttr.usLineOffset		= (winAttr.usWidth);
			winAttr.colordepth			= MMP_DISPLAY_WIN_COLORDEPTH_YUV420_INTERLEAVE;
		#endif
        }

        sRet = MMPD_Display_SetWinAttributes(ubWinID, &winAttr);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   

        dispAttr.usStartX          = 0;
        dispAttr.usStartY          = 0;

        if (AHC_IsTVConnectEx()||AHC_IsHdmiConnect())
        {
        	dispAttr.usDisplayWidth    = usW;
			dispAttr.usDisplayHeight   = usH;
			dispAttr.usDisplayOffsetX  = (MMP_USHORT)usOffsetX;
			dispAttr.usDisplayOffsetY  = (MMP_USHORT)usOffsetY;
			dispAttr.rotatetype        = MMP_DISPLAY_ROTATE_NO_ROTATE;
        }
        else
        {
		#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
			dispAttr.usDisplayWidth    = usW;
			dispAttr.usDisplayHeight   = usH;
			dispAttr.usDisplayOffsetX  = (MMP_USHORT)usOffsetX;
			dispAttr.usDisplayOffsetY  = (MMP_USHORT)usOffsetY;
			dispAttr.rotatetype        = MMP_DISPLAY_ROTATE_NO_ROTATE;
		#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
			dispAttr.usDisplayWidth    = FLOOR16(usH);
			dispAttr.usDisplayHeight   = (usW);
			dispAttr.usDisplayOffsetX  = (MMP_USHORT)usOffsetX;
			dispAttr.usDisplayOffsetY  = (MMP_USHORT)usOffsetY;
			dispAttr.rotatetype        = MMP_DISPLAY_ROTATE_NO_ROTATE;
		#endif
        }

        dispAttr.bMirror           = MMP_FALSE;
        
        sRet = MMPS_UVCRECD_SetPrevwWinID((MMP_UBYTE)ubWinID);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   
        
        sRet = MMPD_Display_SetWinToDisplay(ubWinID, &dispAttr);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   
     
        /* Window Scaling */
        fitrange.fitmode 		= MMP_SCAL_FITMODE_OPTIMAL;
        fitrange.scalerType     = MMP_SCAL_TYPE_WINSCALER;

        if (AHC_IsTVConnectEx()||AHC_IsHdmiConnect())
        {
        	fitrange.ulInWidth      = usW;
			fitrange.ulInHeight     = usH;
			fitrange.ulOutWidth     = (MMP_USHORT)usDisplayW;
			fitrange.ulOutHeight    = (MMP_USHORT)usDisplayH;
        }
        else
        {
		#if (VERTICAL_LCD == VERTICAL_LCD_DEGREE_0)
			fitrange.ulInWidth      = usW;
			fitrange.ulInHeight     = usH;
			fitrange.ulOutWidth     = (MMP_USHORT)usDisplayW;
			fitrange.ulOutHeight    = (MMP_USHORT)usDisplayH;
		#elif (VERTICAL_LCD == VERTICAL_LCD_DEGREE_90) || (VERTICAL_LCD == VERTICAL_LCD_DEGREE_270)
			fitrange.ulInWidth      = FLOOR16(usH);
			fitrange.ulInHeight     = (usW);
			fitrange.ulOutWidth     = (MMP_USHORT)usDisplayW;
			fitrange.ulOutHeight    = (MMP_USHORT)usDisplayH;
		#endif
        }
        fitrange.ulInGrabX      = 1; 
        fitrange.ulInGrabY     	= 1;
        fitrange.ulInGrabW      = fitrange.ulInWidth;
        fitrange.ulInGrabH     	= fitrange.ulInHeight;
        fitrange.ubChoseLit     = 0;
        sRet = MMPD_Scaler_GetGCDBestFitScale(&fitrange, &dispgrabctl);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}   

        sRet = MMPD_Display_SetWinScaling(ubWinID, &fitrange, &dispgrabctl);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}
    }
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoSetCmitStrmTyp
//  Description : AP set commit stream type and decide preview stream type
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCVideoSetCmitStrmTyp(UINT8 ubCmitStrmTyp)
{
    AHC_BOOL    ret = MMP_TRUE;
    
    if (ubCmitStrmTyp & BM_USB_STRM_MJPEG) {
        gsUVCHostCfg.ubCmitUVCStrmTyp = ubCmitStrmTyp;
        MMPS_UVCRECD_SetPrevwStrmTyp(ST_MJPEG);
        ret = AHC_TRUE;
    } 
    else if (ubCmitStrmTyp & BM_USB_STRM_NV12) {
        gsUVCHostCfg.ubCmitUVCStrmTyp = ubCmitStrmTyp;
        MMPS_UVCRECD_SetPrevwStrmTyp(ST_NV12);
        ret = AHC_TRUE;
    } 
    else {
        printc(FG_RED("[ERR] %s,%d, Not support stream type %d for preview.\r\n"), __func__, __LINE__, ubCmitStrmTyp);
        ret = AHC_FALSE;
    }
    return ret;
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoIsUVCDecMjpeg2Prevw
//  Description : AP common I/F to decide if UVC decode Mjpeg to preview
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCVideoIsUVCDecMjpeg2Prevw(void)
{
    MMP_UBYTE   ubDecMjpeg2Prevw = MMP_FALSE;

    /* AP make decision if request driver decode Mjpeg to preview */
    if (gsUVCHostCfg.ubCmitUVCStrmTyp & BM_USB_STRM_MJPEG)
    {
        ubDecMjpeg2Prevw = MMP_TRUE;
    }

    return ubDecMjpeg2Prevw;
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoSetMjpegBitrate
//  Description : Set UVC Mjpeg stream bitrate, noticed XU only
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCVideoSetMjpegBitrate(UINT32 ulkbps)
{
    AHC_BOOL    ret = MMP_TRUE;

    if (MMP_FALSE == AHC_HostUVCVideoIsUVCDecMjpeg2Prevw()) {
        printc(FG_RED("[ERR] Not commit Mjpeg stream! \n"));
        ret = AHC_FALSE;
    } 
    else if (0 == ulkbps) {
        printc(FG_RED("[ERR] Commit Mjpeg stream wrong bitrate %dkbps! \n"), ulkbps);
        ret = AHC_FALSE;
    } 
    else {
        /* Multicast select MJPEG stream ID */
        VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx23_yuv);

        /* Set bitrate */
        printc(FG_RED(">>[ Cmit %d kbps \n"), ulkbps);
        m_XuCmdIspx08xFF_8437[2] = (ulkbps & 0xFF);
        m_XuCmdIspx08xFF_8437[3] = ((ulkbps >> 8) & 0xFF);
        m_XuCmdIspx08xFF_8437[4] = ((ulkbps >> 16) & 0xFF);
        m_XuCmdIspx08xFF_8437[5] = ((ulkbps >> 24) & 0xFF);
        VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx08xFF_8437); /* bitrate */

        ret = AHC_TRUE;
    }
    return ret;
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoSetMjpegFrmSz
//  Description : Set UVC Mjpeg stream frame size, noticed XU only
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCVideoSetMjpegFrmSz(UINT32 ulKBytes)
{
    MMP_ULONG	ulFrmSz;
    AHC_BOOL    ret = MMP_TRUE;

    if (MMP_FALSE == AHC_HostUVCVideoIsUVCDecMjpeg2Prevw()) {
        printc(FG_RED("[ERR] Not commit Mjpeg stream! \n"));
        ret = AHC_FALSE;
    }
    else if (0 == ulKBytes) {
        printc(FG_RED("[ERR] Commit Mjpeg stream wrong frame size %dKB! \n"), ulKBytes);
        ret = AHC_FALSE;
    }
    else {
        /* Multicast select MJPEG stream ID */
        VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx23_yuv);

        /* Set frame size */
        ulFrmSz = ulKBytes << 10;
        printc(FG_RED(">>[ Cmit frame size %d Byte \n"), ulFrmSz);
        m_XuCmdIspx08x80_8437[2] = (ulFrmSz & 0xFF);
        m_XuCmdIspx08x80_8437[3] = ((ulFrmSz >> 8) & 0xFF);
        m_XuCmdIspx08x80_8437[4] = ((ulFrmSz >> 16) & 0xFF);
        m_XuCmdIspx08x80_8437[5] = ((ulFrmSz >> 24) & 0xFF);
        VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx08x80_8437); /* max frame size */

        ret = AHC_TRUE;
    }
    return ret;
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoSetting
//  Description : AP set UVC record & preview video setting
//------------------------------------------------------------------------------
void AHC_HostUVCVideoSetting(UVC_H264_RES_IDX sH264Resol, UVC_FRAMERATE_INDEX sFrameRate, UVC_H264_QUALITY_INDEX sH264Quality, UINT16 usPFrameCnt, UVC_YUV_RESOL_INDEX sYUVResol)
{
    MMPS_3GPRECD_PRESET_CFG    *pVideoConfig = MMPS_3GPRECD_GetConfig();
    MMP_UBYTE                   ubUVCYUVFrmRate = 30;    
    MMP_UBYTE                   ubUVCH264Resol = VIDRECD_RESOL_1920x1088;
    MMP_UBYTE                   ubUVCH264Quality  = VIDRECD_QUALITY_HIGH;
    MMP_UBYTE                   ubUVCH264FrmRate  = VIDRECD_FRAMERATE_30FPS;
    MMP_USHORT                  usbps10k = 0;
    MMP_USHORT                  usW = 0;
    MMP_USHORT                  usH = 0;
    MMP_BOOL                    bSts = MMP_HIF_ERR_PARAMETER;
    MMP_ERR                     sRet = MMP_ERR_NONE;
    AHC_BOOL                    ahcRet = AHC_TRUE;
    MMP_UBYTE                   ubUVCTblNum = 0;    
    
    if (MMP_IsUSBCamIsoMode()) 
        memcpy((MMP_UBYTE*)glUvcNaluTbl, (MMP_UBYTE*)glUvcNaluTblISO, sizeof(glUvcNaluTbl));
    else
        memcpy((MMP_UBYTE*)glUvcNaluTbl, (MMP_UBYTE*)glUvcNaluTblBulk, sizeof(glUvcNaluTbl));
    
    sRet = MMPS_USB_GetUVCStreamSts(&bSts);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return;}    
    if(bSts == MMP_TRUE){ RTNA_DBG_Str(0, "UVC Stream is ON, stop record/preview/stream first! [NG]\r\n"); AHC_PRINT_RET_ERROR(gbAhcDbgBrk,bSts); return;}    
    
    gsUVCHostCfg.sH264Resol = sH264Resol;
    gsUVCHostCfg.sFrameRate = sFrameRate;
    gsUVCHostCfg.sH264Quality = sH264Quality;
    gsUVCHostCfg.usPFrameCnt = usPFrameCnt;
    gsUVCHostCfg.sYUVResol = sYUVResol;
    
    switch(sFrameRate){
        case UVC_FRAMERATE_10FPS:
            ubUVCH264FrmRate = VIDRECD_FRAMERATE_10FPS;
            break;
        case UVC_FRAMERATE_15FPS:
            ubUVCH264FrmRate = VIDRECD_FRAMERATE_15FPS;
            break;
        case UVC_FRAMERATE_20FPS:
            ubUVCH264FrmRate = VIDRECD_FRAMERATE_20FPS;
            break;
        case UVC_FRAMERATE_25FPS:
            ubUVCH264FrmRate = VIDRECD_FRAMERATE_25FPS;
            break;
        case UVC_FRAMERATE_27d5FPS:
        	ubUVCH264FrmRate = VIDRECD_FRAMERATE_27d5FPS;
        	break;
        case UVC_FRAMERATE_30FPS:
        default:
            ubUVCH264FrmRate = VIDRECD_FRAMERATE_30FPS;
            break;
    }

    ubUVCYUVFrmRate = pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrResol / pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrement;
    
    /* config YUV */
    switch(sYUVResol){
        case UVC_YUV_RESOL_90P:
            m_XuCmdMmpx0B[1] = 0x12;
            usW = 160;
            usH = 90;
            break;
        case UVC_YUV_RESOL_240P:
            m_XuCmdMmpx0B[1] = 0x02;
            usW = 320;
            usH = 240;
            break;
        case UVC_YUV_RESOL_360P:
            m_XuCmdMmpx0B[1] = 0x0D;
            usW = 640;
            usH = 360;
            break;
        case UVC_YUV_RESOL_480P:
            m_XuCmdMmpx0B[1] = 0x03;
            usW = 640;
            usH = 480;
            break;
        case UVC_YUV_RESOL_180P:
        default:
            m_XuCmdMmpx0B[1] = 0x13;
            usW = 320;
            usH = 180;
            break;
    }

#if 1 //For AIT MJ+H264 format
    MMPS_UVCRECD_SetPrevwResol(usW, usH);

    if(gsUVCHostCfg.ubCmitUVCStrmTyp & BM_USB_STRM_MJPEG)
    {
        /* Fill ST_MJPEG (W, H, bps, FPS) instead of YUV if commit */
        m_XuCmdMmpx23_yuv[1] = USB_STRM_ID_MJPEG;
        m_XuCmdMmpx25_yuv[1] = USB_STRM_TYP_MJPEG;

        switch(gsUVCHostCfg.sMjpegResol){
            case UVC_MJPEG_RESOL_90P:
                m_XuCmdMmpx0B[1] = 0x12;
                usW = 160;
                usH = 90;
                break;
            case UVC_MJPEG_RESOL_180P:
                m_XuCmdMmpx0B[1] = 0x13;
                usW = 320;
                usH = 180;
                break;
            case UVC_MJPEG_RESOL_360P:
                m_XuCmdMmpx0B[1] = 0x0D;
                usW = 640;
                usH = 360;
                break;
            case UVC_MJPEG_RESOL_720P:
            default:
                m_XuCmdMmpx0B[1] = 0x0A;
                usW = 1280;
                usH = 720;
                break;
        }

        if (gUsbh_UvcMjpegWifiCfg.bEnWiFi) {
            MMP_ULONG fpsx10 = 30;
            MMP_ULONG bitRate = 5000000;

            /* TBD: CarDV no such flow, start Mjpeg WiFi before start local preview */
            AIHC_VIDEO_GetMovieCfgEx(1, AHC_VIDEO_BITRATE, &bitRate);
            AIHC_VIDEO_GetMovieCfgEx(1, AHC_FRAME_RATEx10, &fpsx10);
            ubUVCYUVFrmRate = gsUVCHostCfg.usMjpegFps = fpsx10/10;
            usbps10k = bitRate/10000;
            /* TBD: 8451 can not assign arbitrary resolution */
            m_XuCmdMmpx0B[1] = 0x0D; /* 360p */
            usW = gUsbh_UvcMjpegWifiCfg.dwTargetWidth;
            usH = gUsbh_UvcMjpegWifiCfg.dwTargetHeight;
        }
        else {
            gsUVCHostCfg.usMjpegFps = INIT_DEV_MJPEG_FPS;
            usbps10k = INIT_DEV_MJPEG_BITRATE/10000;
            gsUVCHostCfg.ulMjpegkbps = INIT_DEV_MJPEG_BITRATE/1000;
        }

        m_XuCmdMmpx0C_8451[1] = 0x8A;
    }
    
    if (gsUVCHostCfg.ubCmitUVCStrmTyp & BM_USB_STRM_NV12)
    {
        m_XuCmdMmpx23_yuv[1] = USB_STRM_ID_YUV;
        m_XuCmdMmpx25_yuv[1] = USB_STRM_TYP_NV12;
        usbps10k = ((usW*usH*3)>>1)/10000;

        m_XuCmdMmpx0C_8451[1] = 0x87;
    }
#endif

    m_XuCmdMmpx0B[2] = m_XuCmdMmpx25_yuv[2] = usW & 0xFF;
    m_XuCmdMmpx0B[3] = m_XuCmdMmpx25_yuv[3] = (usW >> 8) & 0xFF;
    m_XuCmdMmpx0B[4] = m_XuCmdMmpx25_yuv[4] = usH & 0xFF;
    m_XuCmdMmpx0B[5] = m_XuCmdMmpx25_yuv[5] = (usH >> 8) & 0xFF;
    m_XuCmdMmpx25_yuv[6] = ubUVCYUVFrmRate & 0x1F;

	if (gsUVCHostCfg.ubCmitUVCStrmTyp & BM_USB_STRM_MJPEG)
	{
    	m_XuCmdMmpx25_yuv[6] |= ((usbps10k<<5)&0xE0); //For AIT MJ+H264 format
    	m_XuCmdMmpx25_yuv[7] = (usbps10k>>3)&0xFF;
    }

    sRet = MMPS_UVCRECD_SetPrevwResol(usW, usH);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}    
    
    sRet = MMPS_UVCRECD_SetPrevwFrameRate(ubUVCYUVFrmRate);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}

    /* Config H264 */
    switch(sH264Resol){
        case UVC_H264_RES_360P:
            if (MMP_IsUSBCamIsoMode()) {
                ubUVCH264Resol = VIDRECD_RESOL_640x480; // Force as 4:3.
                usW = 640;
                usH = 480;
            }
            else {
                ubUVCH264Resol = VIDRECD_RESOL_640x368;
                m_VsCommitCtl_8451[3] = 0x07;
                usW = 640;
                usH = 360;
            }
            break;
        case UVC_H264_RES_720P:
            ubUVCH264Resol = VIDRECD_RESOL_1280x720;
            m_VsCommitCtl_8451[3] = 0x01;
            usW = 1280;
            usH = 720;
            break;
        case UVC_H264_RES_1440P:
        	ubUVCH264Resol = VIDRECD_RESOL_2560x1440;
            m_VsCommitCtl_8451[3] = 0x01;
            usW = 2560;
            usH = 1440;
            break;
        case UVC_H264_RES_1080P:
        default:
            ubUVCH264Resol = VIDRECD_RESOL_1920x1088;
            m_VsCommitCtl_8451[3] = 0x01;
            usW = 1920;
            usH = 1080;
            break;
    }

    switch(sH264Quality){
        case UVC_H264_QUALITY_LOW:
            ubUVCH264Quality = VIDRECD_QUALITY_LOW;
            break;
        case UVC_H264_QUALITY_MID:
            ubUVCH264Quality = VIDRECD_QUALITY_MID;
            break;
        case UVC_H264_QUALITY_HIGH:
        default:
            ubUVCH264Quality = VIDRECD_QUALITY_HIGH;
            break;
    }
    
    /* update start UVC CB */
    m_XuCmdMmpx25_h264[2] = usW & 0xFF;
    m_XuCmdMmpx25_h264[3] = (usW >> 8) & 0xFF;
    m_XuCmdMmpx25_h264[4] = usH & 0xFF;
    m_XuCmdMmpx25_h264[5] = (usH >> 8) & 0xFF;
    #ifdef AHC_VIDEO_REAR_CAM_BITRATE
    usbps10k = AHC_VIDEO_REAR_CAM_BITRATE/10000;
    #else
    usbps10k = (pVideoConfig->ulFps30BitrateMap[ubUVCH264Resol][ubUVCH264Quality]/10000);
    #endif
    m_XuCmdMmpx25_h264[6] = ((pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrResol /
                                    pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrement) & 0x1F)| ((usbps10k << 5) & 0xE0);
    m_XuCmdMmpx25_h264[7] = (usbps10k >> 3) & 0xFF;


	m_XuCmdIspx02[1] = ((pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrResol /
			pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrement) & 0xFF);

	m_XuCmdIspx14[1] = ((pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrResol * 10 /
		pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrement) & 0xFF);
	m_XuCmdIspx14[2] = (((pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrResol * 10 /
			pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrement) >> 8) & 0xFF);


    m_XuCmdIspx08_8451[2] = (10*usbps10k)     & 0xFF;
    m_XuCmdIspx08_8451[3] = (10*usbps10k>>8)  & 0xFF;
    m_XuCmdIspx08_8451[4] = (10*usbps10k>>16) & 0xFF;
    m_XuCmdIspx08_8451[5] = (10*usbps10k>>24) & 0xFF;

    m_XuCmdMmpx09_h264[1] = usPFrameCnt     & 0xFF;
    m_XuCmdMmpx09_h264[2] = (usPFrameCnt>>8)& 0xFF;
    m_XuCmdMmpx09_h264[3] = 0;
    m_XuCmdMmpx09_h264[4] = 0;
    
    /* config H264 */
    sRet = MMPS_UVCRECD_SetUVCRecdResol(ubUVCH264Resol);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}    
    
    #ifdef AHC_VIDEO_REAR_CAM_BITRATE
    sRet = MMPS_UVCRECD_SetRecdBitrate(AHC_VIDEO_REAR_CAM_BITRATE);
    #else
    sRet = MMPS_UVCRECD_SetRecdBitrate(pVideoConfig->ulFps30BitrateMap[ubUVCH264Resol][ubUVCH264Quality]);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}    
	#endif

    sRet = MMPS_UVCRECD_SetRecdFrameRate(pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrement, pVideoConfig->framerate[ubUVCH264FrmRate].usVopTimeIncrResol);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}    

    sRet = MMPS_UVCRECD_SetRecdPFrameCount(usPFrameCnt);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}    
    
    /* update start UVC CB/TBL */
    //MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_START_UVC_CB, (MMP_UBYTE *)glUvcNaluTbl[0].ubDevStrInfo,(void *)StartUVCCb8437);
    //MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_START_UVC_CB, (MMP_UBYTE *)glUvcNaluTbl[1].ubDevStrInfo,(void *)StartUVCCb8451);
    //MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_START_UVC_CB, (MMP_UBYTE *)glUvcNaluTbl[2].ubDevStrInfo,(void *)StartUVCCb8453);
    //MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_START_UVC_CB, (MMP_UBYTE *)glUvcNaluTbl[3].ubDevStrInfo,(void *)StartUVCCb8589);

	for(ubUVCTblNum = 0; ubUVCTblNum < (sizeof(glUvcNaluTbl)/sizeof(UVC_NALU_CFG)); ubUVCTblNum++)
	{
		sRet = MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_NALU_TBL, (MMP_UBYTE *)glUvcNaluTbl[ubUVCTblNum].ubDevStrInfo,(void *)&(glUvcNaluTbl[ubUVCTblNum].mNalu[sH264Resol]));
		if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}	  
	}

    #if (MOTION_DETECTION_EN)
    if (CAM_CHECK_USB(USB_CAM_AIT)) {
        ahcRet = AHC_HostUVCVideoRegisterMDCB((void *) VRMotionDetectCB);
        if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); /*return;*/}    
    }
    #endif

    #if (SUPPORT_DEC_MJPEG_TO_ENC_H264)
    if (MMP_IsSupportDecMjpegToEncH264()) {
    
        if ((gUsbh_UvcDevInfo.wPrevwMaxWidth == 0) ||
            (gUsbh_UvcDevInfo.wPrevwMaxHeight == 0) ||
            (gUsbh_UvcDevInfo.bParseDone == MMP_FALSE)) {
            RTNA_DBG_Str(0, "[ERR]: Incorrect seq:");
            RTNA_DBG_Byte(0, gUsbh_UvcDevInfo.bParseDone);
            RTNA_DBG_Byte(0, gUsbh_UvcDevInfo.wPrevwMaxWidth);
            RTNA_DBG_Byte(0, gUsbh_UvcDevInfo.wPrevwMaxHeight);
            RTNA_DBG_Str(0, " \r\n");
        }
        
        sRet = MMPS_3GPRECD_SetDecMjpegToEncodeAttr(MMP_SCAL_FITMODE_OPTIMAL,
                                                    gUsbh_UvcDevInfo.wPrevwMaxWidth,
                                                    gUsbh_UvcDevInfo.wPrevwMaxHeight,
                                                    MMPS_3GPRECD_GetConfig()->usEncWidth[ubUVCH264Resol],
                                                    MMPS_3GPRECD_GetConfig()->usEncHeight[ubUVCH264Resol]);    
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}        
    }
    #endif
}

#if (DUALENC_SUPPORT)
//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoDualEncodeSetting
//  Description :
//------------------------------------------------------------------------------
void AHC_HostUVCVideoDualEncodeSetting(void) 
{
    MMP_USHORT              usResoIdx = VIDRECD_RESOL_1280x720;
    VIDENC_PROFILE          sProfile = H264ENC_BASELINE_PROFILE;
    VIDENC_CURBUF_MODE      sVideoCurBufMode = VIDENC_CURBUF_FRAME;   
    MMPS_3GPRECD_FRAMERATE  sSensorInFps    = {1001, 30000};
    MMPS_3GPRECD_FRAMERATE  sContainerFps   = {1001, 30000};
    MMP_USHORT              usQuality       = VIDRECD_QUALITY_HIGH;
    MMP_USHORT              usPFrameCount = 14, usBFrameCount = 0;
    UVC_DEVICE_NAME_INDEX   sUVCDevcie;
    MMP_ERR                 sRet = MMP_ERR_NONE;
    AHC_BOOL                ahcRet = AHC_TRUE;
    MMP_ULONG               uFileStream = MMPS_3GPRECD_FILESTREAM_NORMAL; //MMPS_3GPRECD_FILESTREAM_DUAL

    if (!(AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_TRUE) & DUAL_REC_ENCODE_H264)) {
        return;
    }

    ahcRet = AHC_HostUVCVideoGetChipName(&sUVCDevcie);
    if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return;}                 

    if ((gUsbh_UvcDevInfo.wPrevwMaxWidth == 640) && (gUsbh_UvcDevInfo.wPrevwMaxHeight == 480)) {
        usResoIdx = VIDRECD_RESOL_640x480; //TBD.  Need to decide 4:3 or 16:9.
    }
    else if ((gUsbh_UvcDevInfo.wPrevwMaxWidth == 720) && (gUsbh_UvcDevInfo.wPrevwMaxHeight == 480)) {
        usResoIdx = VIDRECD_RESOL_720x480; 
    }
    else if ((gUsbh_UvcDevInfo.wPrevwMaxWidth == 1280) && (gUsbh_UvcDevInfo.wPrevwMaxHeight == 720)) {
        usResoIdx = VIDRECD_RESOL_1280x720;                            
    }
    else if ((gUsbh_UvcDevInfo.wPrevwMaxWidth == 1920) && (gUsbh_UvcDevInfo.wPrevwMaxHeight == 1080)) {
        usResoIdx = VIDRECD_RESOL_1920x1088;
    }
    else {
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0); 
        return;
    }    

	if (!CAM_CHECK_PRM(PRM_CAM_NONE))
	    uFileStream = MMPS_3GPRECD_FILESTREAM_DUAL;
	    
    /* Set 2nd Video Record Attribute */
    sRet = MMPS_3GPRECD_SetEncResIdx(uFileStream, usResoIdx);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}        

    sRet = MMPS_3GPRECD_SetProfile(uFileStream, sProfile);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}        

    sRet = MMPS_3GPRECD_SetCurBufMode(uFileStream, sVideoCurBufMode);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}        

    sSensorInFps.usVopTimeIncrement = 1001;
#if (SUPPORT_VR_FHD_60FPS)
    sSensorInFps.usVopTimeIncrResol = 60000;
#else
    sSensorInFps.usVopTimeIncrResol = 30000;
#endif
    sContainerFps.usVopTimeIncrement = sSensorInFps.usVopTimeIncrement;
    sContainerFps.usVopTimeIncrResol = sSensorInFps.usVopTimeIncrResol;
    
    if (MMP_IsUSBCamIsoMode()) {
        // Different device may have different frame rate.
        if (sUVCDevcie == UVC_DEVICE_NAME_PID_0210 || 
            sUVCDevcie == UVC_DEVICE_NAME_PID_64AB ||
            sUVCDevcie == UVC_DEVICE_NAME_PID_9230 ||
            sUVCDevcie == UVC_DEVICE_NAME_PID_2B95 ||
            sUVCDevcie == UVC_DEVICE_NAME_PID_6300 ||
            sUVCDevcie == UVC_DEVICE_NAME_PID_8301) {
            sSensorInFps.usVopTimeIncrement = 2400;
            sSensorInFps.usVopTimeIncrResol = 60000;
            sContainerFps.usVopTimeIncrement = 2400;
            sContainerFps.usVopTimeIncrResol = 60000;
        }
        else {
            sSensorInFps.usVopTimeIncrement = 2400;
            sSensorInFps.usVopTimeIncrResol = 60000;
            sContainerFps.usVopTimeIncrement = 2400;
            sContainerFps.usVopTimeIncrResol = 60000;
        } 
    }

#if 0//(FPS_CTL_EN)
    if (m_bTlpsEnable) {
        MMP_ULONG play_fps;

        sContainerFps.usVopTimeIncrement = 1001;
        sContainerFps.usVopTimeIncrResol = 30000;
        MMPS_3GPRECD_SetFrameRatePara(MMPS_3GPRECD_FILESTREAM_DUAL, &sSensorInFps, &m_TlpsEncFps, &sContainerFps);
        
        play_fps = (sContainerFps.usVopTimeIncrement +
                    sContainerFps.usVopTimeIncrResol - 1) /
                    sContainerFps.usVopTimeIncrement;
                    
        if (play_fps == 60)
            MMPS_3GPRECD_SetBitrate(MMPS_3GPRECD_FILESTREAM_DUAL, MMPS_3GPRECD_GetConfig()->ulFps60BitrateMap[usResoIdx][usQuality]);
        else
            MMPS_3GPRECD_SetBitrate(MMPS_3GPRECD_FILESTREAM_DUAL, MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[usResoIdx][usQuality]);
    }
    else
#endif
#if 0
    if (m_bSlowMotionEn) {
        sContainerFps.usVopTimeIncrement = 1001;
        sContainerFps.usVopTimeIncrResol = 15000;
        MMPS_3GPRECD_SetFrameRatePara(MMPS_3GPRECD_FILESTREAM_DUAL, &sSensorInFps, &sSensorInFps, &sContainerFps);
        MMPS_3GPRECD_SetBitrate(MMPS_3GPRECD_FILESTREAM_DUAL, MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[usResoIdx][usQuality] >> 1);
    }
    else
#endif
    {
        sRet = MMPS_3GPRECD_SetFrameRatePara(uFileStream, &sSensorInFps, &sSensorInFps, &sContainerFps);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}        
        
        #if (SUPPORT_VR_FHD_60FPS)
        sRet = MMPS_3GPRECD_SetBitrate(uFileStream, MMPS_3GPRECD_GetConfig()->ulFps60BitrateMap[usResoIdx][usQuality]);
        #else
        sRet = MMPS_3GPRECD_SetBitrate(uFileStream, MMPS_3GPRECD_GetConfig()->ulFps30BitrateMap[usResoIdx][usQuality]);
        #endif        
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}        
    }

    sRet = MMPS_3GPRECD_SetPFrameCount(uFileStream, usPFrameCount);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}        
    
    sRet = MMPS_3GPRECD_SetBFrameCount(uFileStream, usBFrameCount);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}
}
#endif

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoEnable
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCVideoEnable(AHC_BOOL bEnable)
{
    MMP_ERR sRet = MMP_ERR_NONE;
    
    if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) & DUAL_REC_ENCODE_H264) {
        #if (DUALENC_SUPPORT)
        gbUVCDeviceEnabled = bEnable;
        #endif
    }
    else if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) == DUAL_REC_STORE_FILE) {
        #if (UVC_VIDRECD_SUPPORT)
        if (AHC_TRUE == bEnable) {
            sRet = MMPS_UVCRECD_SetUVCRecdSupport(MMP_TRUE);   
        }
        else {
            sRet = MMPS_UVCRECD_SetUVCRecdSupport(MMP_FALSE);   
        }
        #endif
    }
      
    if (sRet != MMP_ERR_NONE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return;*/}        

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoIsEnabled
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCVideoIsEnabled(void)
{
    if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) & DUAL_REC_ENCODE_H264) {
        #if (DUALENC_SUPPORT)
        return gbUVCDeviceEnabled;
        #endif
    }
    else if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) == DUAL_REC_STORE_FILE) {
        #if (UVC_VIDRECD_SUPPORT)
        MMP_ERR     sRet = MMP_ERR_NONE;
        MMP_BOOL    bUVCVideoEnable = AHC_FALSE;

        sRet = MMPS_UVCRECD_GetUVCRecdSupport(&bUVCVideoEnable);
        if (bUVCVideoEnable == MMP_FALSE) { return AHC_FALSE; }
        #endif
        return AHC_TRUE;
    }

    return AHC_FALSE;
}

AHC_BOOL AHC_HostUVCVideoEnumDevice(AHC_BOOL busb_device_in)
{
    static AHC_BOOL busb_bdevice_enumed = 0;
    //static AHC_BOOL bUVCDeviceCFGAdded = 0;
    MMP_ERR     sRet = MMP_ERR_NONE;
    AHC_BOOL    ahcRet = AHC_TRUE;    
    MMP_UBYTE   ubH264ResolMapCFG;

    if (MMP_IsUSBCamIsoMode()) 
        memcpy((MMP_UBYTE*)glUvcNaluTbl, (MMP_UBYTE*)glUvcNaluTblISO, sizeof(glUvcNaluTbl));
    else
        memcpy((MMP_UBYTE*)glUvcNaluTbl, (MMP_UBYTE*)glUvcNaluTblBulk, sizeof(glUvcNaluTbl));

    if (busb_device_in) {
        if (busb_bdevice_enumed == 0) {
		
            MMPF_USBH_ClrDevCFG();
            //Total count is clear by MMPF_USBH_ClrDevCFG. Set it again.
		MMPS_UVCRECD_SetDevTotalCount(MMPS_3GPRECD_UVC_CFG_MAX_NUM);
		
            /* Add dev string/CB/TBL, only once at beginning */
            { //if (bUVCDeviceCFGAdded == 0) {
                
                if (MMP_IsUSBCamIsoMode()) {
                    sRet = MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[0].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb8437,(void *)AHC_HostUVCVideoStartUVCCb8437,(void *)&(glUvcNaluTbl[0].mNalu[UVC_H264_RES_1080P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[1].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_6300,(void *)AHC_HostUVCVideoStartUVCCb_PID_6300,(void *)&(glUvcNaluTbl[1].mNalu[UVC_H264_RES_720P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[2].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_0210,(void *)AHC_HostUVCVideoStartUVCCb_PID_0210,(void *)&(glUvcNaluTbl[2].mNalu[UVC_H264_RES_720P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[3].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_64AB,(void *)AHC_HostUVCVideoStartUVCCb_PID_64AB,(void *)&(glUvcNaluTbl[3].mNalu[UVC_H264_RES_720P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[4].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_9230,(void *)AHC_HostUVCVideoStartUVCCb_PID_9230,(void *)&(glUvcNaluTbl[4].mNalu[UVC_H264_RES_720P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[5].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_2B95,(void *)AHC_HostUVCVideoStartUVCCb_PID_2B95,(void *)&(glUvcNaluTbl[5].mNalu[UVC_H264_RES_720P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[6].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_8301,(void *)AHC_HostUVCVideoStartUVCCb_PID_8301,(void *)&(glUvcNaluTbl[6].mNalu[UVC_H264_RES_360P]));

                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[0].ubDevStrInfo,glUvcGuidEUTblISO[0][0],glUvcGuidEUTblISO[0][1],glUvcGuidEUTblISO[0][2],glUvcGuidEUTblISO[0][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[1].ubDevStrInfo,glUvcGuidEUTblISO[1][0],glUvcGuidEUTblISO[1][1],glUvcGuidEUTblISO[1][2],glUvcGuidEUTblISO[1][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[2].ubDevStrInfo,glUvcGuidEUTblISO[2][0],glUvcGuidEUTblISO[2][1],glUvcGuidEUTblISO[2][2],glUvcGuidEUTblISO[2][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[3].ubDevStrInfo,glUvcGuidEUTblISO[3][0],glUvcGuidEUTblISO[3][1],glUvcGuidEUTblISO[3][2],glUvcGuidEUTblISO[3][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[4].ubDevStrInfo,glUvcGuidEUTblISO[4][0],glUvcGuidEUTblISO[4][1],glUvcGuidEUTblISO[4][2],glUvcGuidEUTblISO[4][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[5].ubDevStrInfo,glUvcGuidEUTblISO[5][0],glUvcGuidEUTblISO[5][1],glUvcGuidEUTblISO[5][2],glUvcGuidEUTblISO[5][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[6].ubDevStrInfo,glUvcGuidEUTblISO[6][0],glUvcGuidEUTblISO[6][1],glUvcGuidEUTblISO[6][2],glUvcGuidEUTblISO[6][3]);
                }
                else {
                    sRet = MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[0].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb8437,(void *)AHC_HostUVCVideoStartUVCCb8437,(void *)&(glUvcNaluTbl[0].mNalu[UVC_H264_RES_1080P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[1].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb8437,(void *)AHC_HostUVCVideoStartUVCCb8451,(void *)&(glUvcNaluTbl[1].mNalu[UVC_H264_RES_1080P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[2].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_0210,(void *)AHC_HostUVCVideoStartUVCCb_PID_0210,(void *)&(glUvcNaluTbl[2].mNalu[UVC_H264_RES_1080P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[3].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_64AB,(void *)AHC_HostUVCVideoStartUVCCb_PID_64AB,(void *)&(glUvcNaluTbl[3].mNalu[UVC_H264_RES_720P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[4].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_9230,(void *)AHC_HostUVCVideoStartUVCCb_PID_9230,(void *)&(glUvcNaluTbl[4].mNalu[UVC_H264_RES_720P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[5].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_2B95,(void *)AHC_HostUVCVideoStartUVCCb_PID_2B95,(void *)&(glUvcNaluTbl[5].mNalu[UVC_H264_RES_720P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[6].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb_PID_8301,(void *)AHC_HostUVCVideoStartUVCCb_PID_8301,(void *)&(glUvcNaluTbl[6].mNalu[UVC_H264_RES_360P]));
                    sRet |= MMPS_UVCRECD_AddDevCFG((MMP_UBYTE *)glUvcNaluTbl[7].ubDevStrInfo,(void *)AHC_HostUVCVideoOpenUVCCb8328,(void *)AHC_HostUVCVideoStartUVCCb8328,(void *)&(glUvcNaluTbl[7].mNalu[UVC_H264_RES_1440P]));

                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[0].ubDevStrInfo,glUvcGuidEUTblBulk[0][0],glUvcGuidEUTblBulk[0][1],glUvcGuidEUTblBulk[0][2],glUvcGuidEUTblBulk[0][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[1].ubDevStrInfo,glUvcGuidEUTblBulk[1][0],glUvcGuidEUTblBulk[1][1],glUvcGuidEUTblBulk[1][2],glUvcGuidEUTblBulk[1][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[2].ubDevStrInfo,glUvcGuidEUTblBulk[2][0],glUvcGuidEUTblBulk[2][1],glUvcGuidEUTblBulk[2][2],glUvcGuidEUTblBulk[2][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[3].ubDevStrInfo,glUvcGuidEUTblBulk[3][0],glUvcGuidEUTblBulk[3][1],glUvcGuidEUTblBulk[3][2],glUvcGuidEUTblBulk[3][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[4].ubDevStrInfo,glUvcGuidEUTblBulk[4][0],glUvcGuidEUTblBulk[4][1],glUvcGuidEUTblBulk[4][2],glUvcGuidEUTblBulk[4][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[5].ubDevStrInfo,glUvcGuidEUTblBulk[5][0],glUvcGuidEUTblBulk[5][1],glUvcGuidEUTblBulk[5][2],glUvcGuidEUTblBulk[5][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[6].ubDevStrInfo,glUvcGuidEUTblBulk[6][0],glUvcGuidEUTblBulk[6][1],glUvcGuidEUTblBulk[6][2],glUvcGuidEUTblBulk[6][3]);
                    sRet |= MMPS_UVCRECD_RegDevAddiCFG(MMP_USBH_REG_TYP_GUID_EU,(MMP_UBYTE *)glUvcNaluTbl[7].ubDevStrInfo,glUvcGuidEUTblBulk[7][0],glUvcGuidEUTblBulk[7][1],glUvcGuidEUTblBulk[7][2],glUvcGuidEUTblBulk[7][3]);
                }

                if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}                    
                //bUVCDeviceCFGAdded = 1;
            }
            
            sRet = MMPS_USB_EnumUVC();
            if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
            
            busb_bdevice_enumed = 1;

            ahcRet = AHC_HostUVCVideoGetChipInfo((UINT32 *)&gUsbh_UvcDevInfo.ulVIDPID);
            if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return AHC_FALSE;}    
            
            #if (DUALENC_SUPPORT)
            if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) & DUAL_REC_ENCODE_H264) {
            
                if (!gUsbh_UvcDevInfo.bParseDone) { 
                    AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); 
                    return AHC_FALSE;
                }

                if ((gUsbh_UvcDevInfo.wPrevwMaxWidth == 640) && (gUsbh_UvcDevInfo.wPrevwMaxHeight == 480)) {
                    ubH264ResolMapCFG = UVC_H264_RES_360P; //TBD.  Need to decide 4:3 or 16:9.
                }
                else if ((gUsbh_UvcDevInfo.wPrevwMaxWidth == 720) && (gUsbh_UvcDevInfo.wPrevwMaxHeight == 480)) {
                    ubH264ResolMapCFG = UVC_H264_RES_480P;
                }
                else if ((gUsbh_UvcDevInfo.wPrevwMaxWidth == 1280) && (gUsbh_UvcDevInfo.wPrevwMaxHeight == 720)) {
                    ubH264ResolMapCFG = UVC_H264_RES_720P;                            
                }
                else if ((gUsbh_UvcDevInfo.wPrevwMaxWidth == 1920) && (gUsbh_UvcDevInfo.wPrevwMaxHeight == 1080)) {
                    ubH264ResolMapCFG = UVC_H264_RES_1080P;                            
                }
                else {
                    AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); 
                    return AHC_FALSE;
                }

                sRet  = MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_NALU_TBL, (MMP_UBYTE *)glUvcNaluTbl[0].ubDevStrInfo, (void *)&(glUvcNaluTbl[0].mNalu[ubH264ResolMapCFG]));
                sRet |= MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_NALU_TBL, (MMP_UBYTE *)glUvcNaluTbl[1].ubDevStrInfo, (void *)&(glUvcNaluTbl[1].mNalu[ubH264ResolMapCFG]));
                sRet |= MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_NALU_TBL, (MMP_UBYTE *)glUvcNaluTbl[2].ubDevStrInfo, (void *)&(glUvcNaluTbl[2].mNalu[ubH264ResolMapCFG]));
                sRet |= MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_NALU_TBL, (MMP_UBYTE *)glUvcNaluTbl[3].ubDevStrInfo, (void *)&(glUvcNaluTbl[3].mNalu[ubH264ResolMapCFG]));
                sRet |= MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_NALU_TBL, (MMP_UBYTE *)glUvcNaluTbl[4].ubDevStrInfo, (void *)&(glUvcNaluTbl[4].mNalu[ubH264ResolMapCFG]));
                sRet |= MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_NALU_TBL, (MMP_UBYTE *)glUvcNaluTbl[5].ubDevStrInfo, (void *)&(glUvcNaluTbl[5].mNalu[ubH264ResolMapCFG]));
                sRet |= MMPS_UVCRECD_UpdDevCFG(MMP_USBH_UPD_NALU_TBL, (MMP_UBYTE *)glUvcNaluTbl[6].ubDevStrInfo, (void *)&(glUvcNaluTbl[6].mNalu[ubH264ResolMapCFG]));
                if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}                    

                RTNA_DBG_Str(0, "Search W,H:");  RTNA_DBG_Short(0, gUsbh_UvcDevInfo.wPrevwMaxWidth);  RTNA_DBG_Short(0, gUsbh_UvcDevInfo.wPrevwMaxHeight);
            }
            #endif

            if (MMP_IsUSBCamIsoMode()) {
                if (gUsbh_UvcDevInfo.ulVIDPID == 0x83011B3F){ // For Sunplus rear cam.
                    MMPF_USBH_CheckJpegEOIEveryPacketEnable(MMP_TRUE);
                }
                else {
                    MMPF_USBH_CheckJpegEOIEveryPacketEnable(MMP_FALSE);
                }
            }
        }
    }
    else {
        busb_bdevice_enumed = 0;
    }
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoPreviewStart
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCVideoPreviewStart(void)
{
    MMP_DISPLAY_WIN_ATTR    winAttr;
    MMP_ERR                 sRet = MMP_ERR_NONE;
    AHC_BOOL                ahcRet = AHC_TRUE;    

    sRet = MMPS_UVCRECD_StartPreview();
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}

    #ifdef CUS_CFG_UVC_OSD_TEXT
    ahcRet = AHC_HostUVCSetOSD(CUS_CFG_UVC_OSD_TEXT, 30, 30, UVC_XU_OSD_TEXT_ENABLE);
    if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return AHC_FALSE;}   
    #endif

    /* Rear cam YUV address MUST be ready and set, or check mistake sequence! */
    sRet = MMPS_Display_GetWinAttributes(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), &winAttr);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
    
    if ((winAttr.ulBaseAddr == 0) || (winAttr.ulBaseUAddr == 0) || (winAttr.ulBaseVAddr == 0)) {
        printc(FG_RED("[ERR]: Mistake seq. rear cam addr (%x,%x,%x)!\r\n"),
               winAttr.ulBaseAddr, winAttr.ulBaseUAddr, winAttr.ulBaseVAddr);
        return AHC_TRUE;
    }

    if ((!CAM_CHECK_USB(USB_CAM_SONIX_MJPEG)) && 
        (!CAM_CHECK_USB(USB_CAM_SONIX_MJPEG2H264))) {
        //AHC_OS_SleepMs(200);
        ahcRet = AHC_HostUVCVideoSetWinActive(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), AHC_TRUE);
        if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return AHC_FALSE;}
    }
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoPreviewStop
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCVideoPreviewStop(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;

    sRet = MMPS_UVCRECD_StopPreview();
    if (sRet != MMP_ERR_NONE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    
#if (!SWITCH_MODE_FREEZE_WIN)
    sRet = MMPS_Display_SetWinActive(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), MMP_FALSE);
    if (sRet != MMP_ERR_NONE) { AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
#endif

    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoRecordStart
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCVideoRecordStart(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;
    
    if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) & DUAL_REC_ENCODE_H264) {
        #if (DUALENC_SUPPORT)
        AHC_HostUVCVideoDualEncodeSetting();

        sRet = MMPS_3GPRECD_StartDualH264();
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
        #endif
    }
    else if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) == DUAL_REC_STORE_FILE) {
        #if (UVC_VIDRECD_SUPPORT)
        sRet = MMPS_UVCRECD_EnableRecd();
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
        #endif
    }
    
    return AHC_TRUE;
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCVideoRecordStop
//  Description : 
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCVideoRecordStop(void)
{
    if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) & DUAL_REC_ENCODE_H264) {
        #if (DUALENC_SUPPORT)
        // NOP, Stop flow is inside MMPS_3GPRECD_StopRecord()
        #endif
    }
    else if (AHC_VIDEO_CheckDualRecEnabled(CAM_TYPE_USB, AHC_FALSE) == DUAL_REC_STORE_FILE) {
        #if (UVC_VIDRECD_SUPPORT)
        MMP_ERR sRet = MMP_ERR_NONE;
        
        sRet = MMPS_UVCRECD_StopRecd();
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
        #endif
    }

    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCResetFBMemory(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;
    
    sRet = MMPF_USBH_ResetFBMemory();
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    
    return AHC_TRUE;
}

#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
AHC_BOOL AHC_HostUVCResetMjpegToPreviewJpegBuf(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;
    
    if (!MMP_IsSupportDecMjpegToPreview()) {
        return AHC_TRUE;
    }
    
    sRet = MMPF_USBH_ResetDecMjpegToPreviewJpegBuf();
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    
    return AHC_TRUE;
}
#endif

AHC_BOOL AHC_HostUVCVideoGetChipName(UVC_DEVICE_NAME_INDEX *pUVCDeviceName)
{
    if (MMP_IsUSBCamIsoMode()) 
        memcpy((MMP_UBYTE*)glUvcNaluTbl, (MMP_UBYTE*)glUvcNaluTblISO, sizeof(glUvcNaluTbl));
    else
        memcpy((MMP_UBYTE*)glUvcNaluTbl, (MMP_UBYTE*)glUvcNaluTblBulk, sizeof(glUvcNaluTbl));

    if (gUsbh_UvcDevInfo.ulVIDPID == (((UINT32)glUvcNaluTbl[0].ubDevStrInfo[0] << 0) + ((UINT32)glUvcNaluTbl[0].ubDevStrInfo[1] << 8) + 
                                      ((UINT32)glUvcNaluTbl[0].ubDevStrInfo[2] << 16) + ((UINT32)glUvcNaluTbl[0].ubDevStrInfo[3] << 24))) {
        *pUVCDeviceName = UVC_DEVICE_NAME_8437;
    }
    else if (gUsbh_UvcDevInfo.ulVIDPID == (((UINT32)glUvcNaluTbl[1].ubDevStrInfo[0] << 0) + ((UINT32)glUvcNaluTbl[1].ubDevStrInfo[1] << 8) + 
                                           ((UINT32)glUvcNaluTbl[1].ubDevStrInfo[2] << 16) + ((UINT32)glUvcNaluTbl[1].ubDevStrInfo[3] << 24))) {
        if (MMP_IsUSBCamIsoMode()) {
            *pUVCDeviceName = UVC_DEVICE_NAME_PID_6300;
        }
        else {
            *pUVCDeviceName = UVC_DEVICE_NAME_8451;
        }
    }
    else if (gUsbh_UvcDevInfo.ulVIDPID == (((UINT32)glUvcNaluTbl[2].ubDevStrInfo[0] << 0) + ((UINT32)glUvcNaluTbl[2].ubDevStrInfo[1] << 8) + 
                                           ((UINT32)glUvcNaluTbl[2].ubDevStrInfo[2] << 16) + ((UINT32)glUvcNaluTbl[2].ubDevStrInfo[3] << 24))) {
        *pUVCDeviceName = UVC_DEVICE_NAME_PID_0210;
    }
    else if (gUsbh_UvcDevInfo.ulVIDPID == (((UINT32)glUvcNaluTbl[3].ubDevStrInfo[0] << 0) + ((UINT32)glUvcNaluTbl[3].ubDevStrInfo[1] << 8) + 
                                           ((UINT32)glUvcNaluTbl[3].ubDevStrInfo[2] << 16) + ((UINT32)glUvcNaluTbl[3].ubDevStrInfo[3] << 24))) {
        *pUVCDeviceName = UVC_DEVICE_NAME_PID_64AB;
    }      
    else if (gUsbh_UvcDevInfo.ulVIDPID == (((UINT32)glUvcNaluTbl[4].ubDevStrInfo[0] << 0) + ((UINT32)glUvcNaluTbl[4].ubDevStrInfo[1] << 8) + 
                                           ((UINT32)glUvcNaluTbl[4].ubDevStrInfo[2] << 16) + ((UINT32)glUvcNaluTbl[4].ubDevStrInfo[3] << 24))) {
        *pUVCDeviceName = UVC_DEVICE_NAME_PID_9230;
    }
	else if (gUsbh_UvcDevInfo.ulVIDPID == (((UINT32)glUvcNaluTbl[5].ubDevStrInfo[0] << 0) + ((UINT32)glUvcNaluTbl[5].ubDevStrInfo[1] << 8) + 
                                           ((UINT32)glUvcNaluTbl[5].ubDevStrInfo[2] << 16) + ((UINT32)glUvcNaluTbl[5].ubDevStrInfo[3] << 24))) {
        *pUVCDeviceName = UVC_DEVICE_NAME_PID_2B95;
    }
	else if (gUsbh_UvcDevInfo.ulVIDPID == (((UINT32)glUvcNaluTbl[6].ubDevStrInfo[0] << 0) + ((UINT32)glUvcNaluTbl[6].ubDevStrInfo[1] << 8) + 
                                           ((UINT32)glUvcNaluTbl[6].ubDevStrInfo[2] << 16) + ((UINT32)glUvcNaluTbl[6].ubDevStrInfo[3] << 24))) {
        *pUVCDeviceName = UVC_DEVICE_NAME_PID_8301;
    }
    else if(gUsbh_UvcDevInfo.ulVIDPID == (((UINT32)glUvcNaluTbl[7].ubDevStrInfo[0] << 0) + ((UINT32)glUvcNaluTbl[7].ubDevStrInfo[1] << 8) + 
                                           ((UINT32)glUvcNaluTbl[7].ubDevStrInfo[2] << 16) + ((UINT32)glUvcNaluTbl[7].ubDevStrInfo[3] << 24))){
		*pUVCDeviceName = UVC_DEVICE_NAME_PID_8328;
	}
    else {
        *pUVCDeviceName = UVC_DEVICE_NAME_MAX_SUPPORT_NUM;
        AHC_PRINT_RET_ERROR(gbAhcDbgBrk,0);
        return AHC_FALSE;
    }
    
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCVideoGetChipInfo(UINT32 *pUVCDeviceVIDPID)
{
    MMP_ULONG   ulDataLength = 0;
    MMP_UBYTE   ubDataBuf[EU1_GET_CHIP_INFO_LEN] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;
    
    memset(ubDataBuf, 0x0, 16);
    
    sRet = VideoRecordGetStdDevCmd(GET_DESCRIPTOR, (DEVICE_DESCR<<8), 0, 0x12, &glUVCDataLength, gbUVCDataBuf);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}                 
    
    ubDataBuf[0] = gbUVCDataBuf[DEVICE_DESCR_VID_LO_OFST];
    ubDataBuf[1] = gbUVCDataBuf[DEVICE_DESCR_VID_HI_OFST];
    ubDataBuf[2] = gbUVCDataBuf[DEVICE_DESCR_PID_LO_OFST];
    ubDataBuf[3] = gbUVCDataBuf[DEVICE_DESCR_PID_HI_OFST];
    ubDataBuf[4] = 0;
    ulDataLength = 4;

    *pUVCDeviceVIDPID = ((UINT32)ubDataBuf[0] << 0) + ((UINT32)ubDataBuf[1] << 8) + ((UINT32)ubDataBuf[2] << 16) + ((UINT32)ubDataBuf[3] << 24);
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCVideoRegisterMDCB(void *pmd_func)
{
#if (MOTION_DETECTION_EN)
    MMP_ERR sRet = MMP_ERR_NONE;

    sRet = MMPS_USB_RegUVCMDCallBack(pmd_func);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}        
#endif
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCVideoEnableMD(AHC_BOOL bEnableMD)
{
#if (MOTION_DETECTION_EN)
    MMP_ERR sRet = MMP_ERR_NONE;
    
    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }
    
    if (bEnableMD) {
        sRet = MMPS_USB_EnableUVCMD();
    }
    else {
        sRet = MMPS_USB_DisableUVCMD();
    }
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
#endif
    
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCSetMDCuttingWindows(MMP_UBYTE ubWidth, MMP_UBYTE ubHeight)
{
    MMP_BOOL    bUVCStates = MMP_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;
    
    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    m_XuCmdMmpx29[1] = 0x01;
    m_XuCmdMmpx29[2] = ubWidth;
    m_XuCmdMmpx29[3] = ubHeight;
  
    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx29);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}            
    
    return AHC_TRUE;    
}

//------------------------------------------------------------------------------
//  Function    : AHC_HostUVCSetMDSensitivity
//  Description :
//------------------------------------------------------------------------------
AHC_BOOL AHC_HostUVCSetMDSensitivity(MMP_UBYTE ubSensitivity)
{
    MMP_USHORT  usMdBlkNum = 0;
    MMP_BOOL    bUVCStates = MMP_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;
    AHC_BOOL    ahcRet = AHC_TRUE;
    
    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }

    if(ubSensitivity > UVC_DEV_MD_MAX_SENSITIVITY){AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE; }

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if ((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    #if (MOTION_DETECTION_EN)
    gubUVCMDSensitivity = ubSensitivity;
    #endif

    ahcRet = AHC_HostUVCSetMDCuttingWindows(UVC_DEV_MD_DIV_W, UVC_DEV_MD_DIV_H);
    if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return AHC_FALSE;}    
    
    for (usMdBlkNum=0; usMdBlkNum<(UVC_DEV_MD_DIV_W*UVC_DEV_MD_DIV_H); usMdBlkNum++){
        m_XuCmdMmpx2A[2] = usMdBlkNum&0xFF;
        m_XuCmdMmpx2A[3] = (usMdBlkNum>>8)&0xFF;
        m_XuCmdMmpx2A[4] = ubSensitivity;    //device FW dependent 0~40
        sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx2A);
    }
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}                

    return AHC_TRUE;    
}

AHC_BOOL AHC_HostUVCForceIFrame(void)
{
    MMP_BOOL    bStatus = MMP_TRUE;
    MMP_BOOL    bUVCStates = MMP_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;

    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if ((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    sRet = MMPF_USBH_GetUVCPrevwSts(&bStatus);
    if ((sRet != MMP_ERR_NONE) || (bStatus != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx23_h264); //Select Multicast Stream ID
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx04);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    printc("%s,%d\r\n",__func__,__LINE__);

    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCSetOSD(const char *pstring, UINT16 usoffset_x, UINT16 usoffset_y, UVC_XU_OSD_INDEX usdisplay_mode)
{
    UINT32      ulloop = 0;
    UBYTE       cosd_string[UVC_MAX_OSD_STRING_LEN] = {0};
    MMP_BOOL    bUVCStates = MMP_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;
    
    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if ((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    if (usdisplay_mode >= UVC_XU_OSD_MAX_SUPPORT_NUM) {
        printc(FG_RED("%s,%d error!\r\n"), __func__, __LINE__);
        return AHC_FALSE;
    }

    //Max string length is 32.
    for(ulloop=0; ulloop<UVC_MAX_OSD_STRING_LEN; ++ulloop){
        if(*(pstring + ulloop) == '\0'){
            break;
        }
    }
    memset((void *)cosd_string, 0x0, UVC_MAX_OSD_STRING_LEN);
    memcpy((void *)cosd_string, (void *)pstring, ulloop);
    
    if(ulloop == UVC_MAX_OSD_STRING_LEN){AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE; }
    
    m_XuCmdMmpx21[0x01] = usdisplay_mode;
    m_XuCmdMmpx21[0x02] = (UINT8)(usoffset_x & 0xFF);
    m_XuCmdMmpx21[0x03] = (UINT8)((usoffset_x >> 8) & 0xFF);
    m_XuCmdMmpx21[0x04] = (UINT8)(usoffset_y & 0xFF);
    m_XuCmdMmpx21[0x05] = (UINT8)((usoffset_y >> 8) & 0xFF);
    
    //VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, STREAMVIEW_MP8_23);            
    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx23_h264);
    sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx21);            

    if(usdisplay_mode == UVC_XU_OSD_TEXT_ENABLE){
        sRet |= VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_DATA_32, UVC_MAX_OSD_STRING_LEN, cosd_string); //TEXT
    }

    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;    
}

#if 0 //For AIT MJ+H264 format
AHC_BOOL AHC_HostUVCSetOSDColor(UVC_XU_OSD_COLOR ubColor)
{  
    UBYTE cosd_string[UVC_MAX_OSD_STRING_LEN];

    MMP_BOOL bUVCStates;

    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }

    MMPS_USB_GetDevConnSts(&bUVCStates);
    
    if (bUVCStates == MMP_FALSE) {
        printc(FG_RED("%s,%d error!\r\n"), __func__, __LINE__);
        return AHC_FALSE;
    }
    
    if (ubColor >= UVC_XU_OSD_COLOR_NUM) {
        printc(FG_RED("%s,%d error!\r\n"), __func__, __LINE__);
        return AHC_FALSE;
    }
    
    m_XuCmdMmpx21[0x01] = 02;
    m_XuCmdMmpx21[0x06] = ubColor;
              
    VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx21);            

    return AHC_TRUE;
}
#endif

AHC_BOOL AHC_HostUVCImageFlip(UVC_IMAGE_FLIP emFlipOption)
{
    MMP_BOOL    bUVCStates = MMP_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;
    
    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    switch( emFlipOption ){
        case UVC_IMAGE_FLIP_UPSIDE_DOWN:
            m_XuCmdIspx11[0x2] = 0x1;
            break;
        case UVC_IMAGE_FLIP_MIRROR:
            m_XuCmdIspx11[0x2] = 0x2;
            break;
        case UVC_IMAGE_FLIP_BLACK_IMAGE:
            m_XuCmdIspx11[0x2] = 0x4;
            break;
        default:
            m_XuCmdIspx11[0x2] = 0x0;
            break;
    }

    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx11);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
        
    return AHC_TRUE;    
}
AHC_BOOL AHC_HostUVC_NV12_Mirror(AHC_BOOL mirror_en)
{
    MMP_BOOL bUVCStates;

    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }

    MMPS_USB_GetDevConnSts(&bUVCStates);
    
    if (bUVCStates == MMP_FALSE) {
        printc(FG_RED("%s,%d error!\r\n"), __func__, __LINE__);
        return AHC_FALSE;
    }

    if (mirror_en == MMP_TRUE) {
        m_XuCmdIspx29[0x2] = 0x1;
    }
    else {
        m_XuCmdIspx29[0x2] = 0x0;
    }

    if (VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx29) != MMP_ERR_NONE)
    {
        printc(FG_RED("AHC_HostUVC_NV12_Mirror Error (%d)\r\n"), mirror_en);
        return AHC_FALSE;
    }

    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCGetFwVersion( UINT16* puwMajor, UINT16* puwMinor, UINT16* puwBuild )
{
    UINT32      uiDataLength = 0;
    UINT8       byFwVersionBuf[8] = {0};
    MMP_BOOL    bUVCStates = MMP_TRUE;
    MMP_ERR     sRet = MMP_ERR_NONE;
    
    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx0B);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_ISP_RESULT, 8, &uiDataLength, byFwVersionBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}

    //printc("AHC_HostUVCGetFwVersion: %d (%d)\r\n", byFwVersionBuf[0], uiDataLength);

    if( byFwVersionBuf[0] != 0x00 ){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE; }

    *puwMajor = (byFwVersionBuf[2]<<8) | byFwVersionBuf[3];
    *puwMinor = (byFwVersionBuf[4]<<8) | byFwVersionBuf[5];
    *puwBuild = (byFwVersionBuf[6]<<8) | byFwVersionBuf[7];
    
    //printc("0x%04x.%04x.%04x\r\n", *puwMajor, *puwMinor, *puwBuild);
    
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCGet_AHD_SigStatus( MMP_UBYTE * status)
{
    UINT32 uiDataLength = 0;
    UINT8  ubSigStatus[8] = {0};

    MMP_BOOL bUVCStates;
#if (SUPPORT_SONIX_UVC_ISO_MODE==1)
    return AHC_TRUE;
#endif

    MMPS_USB_GetDevConnSts(&bUVCStates);
    if(bUVCStates == MMP_FALSE){
        printc(FG_RED("%s,%d error!\r\n"), __func__, __LINE__);
        return AHC_FALSE;
    }

    VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx38);
    VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_ISP_RESULT, 8, &uiDataLength, ubSigStatus);

//    printc("AHC_HostUVCGet_AHD_SigStatus: %x %x (%d)\r\n", ubSigStatus[0], ubSigStatus[1],  uiDataLength);

   *status = ubSigStatus[1];


    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCResetTime(void)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_ERR sRet = MMP_ERR_NONE;
    AHC_BOOL ahcRet = AHC_TRUE;
    
    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    ahcRet = AHC_HostUVCGetRTCTime(&m_XuCmdMmpx22[0]);
    if(ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return AHC_FALSE;}    
    
    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx22); //RTC
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
  
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCSetFlicker(UVC_FLICKER_INDEX sFlickerIndex)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    UINT32 uiDataLength = 0;
    UINT8  bFlickerResultBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;
    
    if (MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }

    if(sFlickerIndex >= UVC_FLICKER_MAX){AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE; }

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
       
    m_XuCmdIspx09[1] = sFlickerIndex;
    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_ISP, 8, m_XuCmdIspx09);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_ISP_RESULT, 8, &uiDataLength, bFlickerResultBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}        
    if( bFlickerResultBuf[0] != 0x00){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE; }
    
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCSonixRegisterRead(UINT16  usAddr, UINT8 *pbData)
{
    UINT32 uiDataLength = 0;
    UINT8  bDataSend[8] = {0};
    UINT8  bDataRet[8] = {0};
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_ERR sRet = MMP_ERR_NONE;
    
    if (!MMP_IsUSBCamIsoMode()) {
        return AHC_TRUE;
    }
    
    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if ((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    // Switch command
    bDataSend[0] = usAddr & 0xFF;               // Tag
    bDataSend[1] = (usAddr & 0xFF00)>>8;
    bDataSend[3] = 0xff;                        // Dummy

    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, XU_SONIX_ASIC_CTRL, 4, bDataSend);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, XU_SONIX_ASIC_CTRL, 4, &uiDataLength, bDataRet);
    if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}        

    *pbData = bDataRet[2];

    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCSonixReverseGearDetection(void) //reverse gear detection.
{
    UINT16  usAddr = 0;
    UINT8 bData = 0;
    
    MMP_BOOL bUVCStates = MMP_TRUE;
    AHC_BOOL ahcRet = AHC_TRUE;
    MMP_ERR sRet = MMP_ERR_NONE;

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if ((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    if (!MMP_IsUSBCamIsoMode()){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk, 0); return AHC_FALSE;}

    usAddr = 0x1005;
    ahcRet = AHC_HostUVCSonixRegisterRead(usAddr, &bData);
    if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,ahcRet); return AHC_FALSE;}        
    if (bData & 0x01){ printc("Reverse gear\r\n"); return AHC_TRUE; }
    
    // printc("Non Reverse gear\r\n");
    return AHC_FALSE;
}

AHC_BOOL AHC_HostUVC_SetPUBackLightCompensation(UINT16 ulBackLightCompensationLevel)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;

    sRet= MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    ubDataBuf[0] = (MMP_UBYTE)(ulBackLightCompensationLevel & 0xFF);
    ubDataBuf[1] = (MMP_UBYTE)((ulBackLightCompensationLevel >> 8) & 0xFF);

    sRet= VideoRecordSetPUCmd(SET_CUR_CMD, PU_BACKLIGHT_COMPENSATION_CONTROL, 2, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;
}


AHC_BOOL AHC_HostUVC_SetPUBrightness(UINT16 ulBrightnessLevel)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    ubDataBuf[0] = (MMP_UBYTE)(ulBrightnessLevel & 0xFF);
    ubDataBuf[1] = (MMP_UBYTE)((ulBrightnessLevel >> 8) & 0xFF);

    sRet = VideoRecordSetPUCmd(SET_CUR_CMD, PU_BRIGHTNESS_CONTROL, 2, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVC_SetPUContrast(UINT16 ulContrastLevel)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    ubDataBuf[0] = (MMP_UBYTE)(ulContrastLevel & 0xFF);
    ubDataBuf[1] = (MMP_UBYTE)((ulContrastLevel >> 8) & 0xFF);

    sRet = VideoRecordSetPUCmd(SET_CUR_CMD, PU_CONTRAST_CONTROL, 2, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;    
}

AHC_BOOL AHC_HostUVC_SetPUGain(UINT16 ulGainLevel)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    ubDataBuf[0] = (MMP_UBYTE)(ulGainLevel & 0xFF);
    ubDataBuf[1] = (MMP_UBYTE)((ulGainLevel >> 8) & 0xFF);

    sRet = VideoRecordSetPUCmd(SET_CUR_CMD, PU_GAIN_CONTROL, 2, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    return AHC_TRUE;        
}

//0: Disabled ,1: 50 Hz ,2: 60 Hz 
AHC_BOOL AHC_HostUVC_SetPUPowerLineFrequency(UINT8 ulPowerLineFrequency)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;
    
    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    ubDataBuf[0] = (MMP_UBYTE)(ulPowerLineFrequency & 0xFF);

    sRet = VideoRecordSetPUCmd(SET_CUR_CMD, PU_POWER_LINE_FREQUENCY_CONTROL, 1, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;        
}

AHC_BOOL AHC_HostUVC_SetPUHue(UINT16 ulHueLevel)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;
    
    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    ubDataBuf[0] = (MMP_UBYTE)(ulHueLevel & 0xFF);
    ubDataBuf[1] = (MMP_UBYTE)((ulHueLevel >> 8) & 0xFF);

    sRet = VideoRecordSetPUCmd(SET_CUR_CMD, PU_HUE_CONTROL, 2, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;        
}

AHC_BOOL AHC_HostUVC_SetPUSaturation(UINT16 ulSaturationLevel)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;
    
    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    ubDataBuf[0] = (MMP_UBYTE)(ulSaturationLevel & 0xFF);
    ubDataBuf[1] = (MMP_UBYTE)((ulSaturationLevel >> 8) & 0xFF);

    sRet = VideoRecordSetPUCmd(SET_CUR_CMD, PU_SATURATION_CONTROL, 2, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;        
}

AHC_BOOL AHC_HostUVC_SetPUSharpness(UINT16 ulSharpnessLevel)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;
    
    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    ubDataBuf[0] = (MMP_UBYTE)(ulSharpnessLevel & 0xFF);
    ubDataBuf[1] = (MMP_UBYTE)((ulSharpnessLevel >> 8) & 0xFF);

    sRet = VideoRecordSetPUCmd(SET_CUR_CMD, PU_SHARPNESS_CONTROL, 2, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;        
}

AHC_BOOL AHC_HostUVC_SetPUGamma(UINT16 ulGammaLevel)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;
    
    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    ubDataBuf[0] = (MMP_UBYTE)(ulGammaLevel & 0xFF);
    ubDataBuf[1] = (MMP_UBYTE)((ulGammaLevel >> 8) & 0xFF);

    sRet = VideoRecordSetPUCmd(SET_CUR_CMD, PU_GAMMA_CONTROL, 2, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;        
}

AHC_BOOL AHC_HostUVC_SetPUWhiteBalanceTemperature(UINT16 ulWhiteBalanceTemperature)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;
    
    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    ubDataBuf[0] = (MMP_UBYTE)(ulWhiteBalanceTemperature & 0xFF);
    ubDataBuf[1] = (MMP_UBYTE)((ulWhiteBalanceTemperature >> 8) & 0xFF);

    sRet = VideoRecordSetPUCmd(SET_CUR_CMD, PU_WHITE_BALANCE_TEMPERATURE_CONTROL, 2, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;        
}

AHC_BOOL AHC_HostUVC_SetPUWhiteBalanceTemperatureAuto(UINT8 ulWhiteBalanceTemperatureAuto)
{
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_UBYTE ubDataBuf[8] = {0};
    MMP_ERR sRet = MMP_ERR_NONE;
    
    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    ubDataBuf[0] = (MMP_UBYTE)(ulWhiteBalanceTemperatureAuto & 0xFF);

    sRet = VideoRecordSetPUCmd(SET_CUR_CMD, PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL, 1, ubDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    return AHC_TRUE;        
}

AHC_BOOL AHC_HostUVC_GetReversingGearStatus(void)
{
    MMP_UBYTE byDataLen = 0;
    MMP_UBYTE pbyDataBuf[8] = {0};
    MMP_BOOL bUVCStates = MMP_TRUE;
    MMP_ERR sRet = MMP_ERR_NONE;
    
    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
        
    if(MMP_IsUSBCamIsoMode()){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
    
    sRet = VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx2B);
    sRet |= VideoRecordGetEU1Cmd(GET_CUR_CMD, EU1_GET_MMP_RESULT, 8, &byDataLen, pbyDataBuf);
    if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    if(pbyDataBuf[0] == 0x01){
        return AHC_TRUE;
    }
    
    return AHC_FALSE;
}

AHC_BOOL AHC_HostUVC_SetLedFlicker(UINT8 byFlag)
{
    MMP_BOOL bUVCStates;
    
    MMPS_USB_GetDevConnSts(&bUVCStates);
    if(bUVCStates == MMP_FALSE){
        printc(FG_RED("%s,%d error!\r\n"), __func__, __LINE__);
        return AHC_FALSE;
    }

    if( byFlag == 0 )
    {
        m_XuCmdMmpx0E[2] = 0x0;
    }
    else
    {
        m_XuCmdMmpx0E[2] = 0x1;
    }
    
    VideoRecordSetEU1Cmd(SET_CUR_CMD, EU1_SET_MMP, 8, m_XuCmdMmpx0E); //set LED flicker
    return AHC_TRUE;
}

AHC_BOOL AIHC_UVCPreviewStart(MMP_USHORT usVideoPreviewMode, AHC_DISPLAY_OUTPUTPANEL OutputDevice, UINT32 ulFlickerMode)
{
#if (UVC_HOST_VIDEO_ENABLE)

    MMPS_3GPRECD_PRESET_CFG	*pVideoConfig = MMPS_3GPRECD_GetConfig();
    USB_DETECT_PHASE        USBCurrentStates = 0;
    UINT32                  ulUSBTimeout = 0;
    MMP_ERR                 sRet = MMP_ERR_NONE;
    UINT8                   ubUVCPframeCount = 30;
    UINT8                   ubUVCPreviewRes  = UVC_YUV_RESOL_180P;
    printc("-------------fffffffffffff------------------\r\n");
    if (!MMP_IsUSBCamExist()) {
        return AHC_TRUE;
    }
    
    #ifdef CUSTOMER_CONFIG_UVC_PFRAME
    ubUVCPframeCount = CUSTOMER_CONFIG_UVC_PFRAME;
    #endif
    
    #ifdef CUSTOMER_CONFIG_UVC_PREVIEW_RES
    ubUVCPreviewRes = CUSTOMER_CONFIG_UVC_PREVIEW_RES;
    #endif

    // Wait until rear cam enumerates done.
    ulUSBTimeout = 80;
    
    do {
        AHC_OS_SleepMs(10);
        AHC_USBGetStates(&USBCurrentStates,AHC_USB_GET_PHASE_CURRENT);
		
  //	USBCurrentStates=6;
    } while(((USBCurrentStates == USB_DETECT_PHASE_OTG_SESSION) ||
            (USBCurrentStates == USB_DETECT_PHASE_CHECK_CONN)) && (--ulUSBTimeout > 0));

    if (0 == ulUSBTimeout) {printc("\r\n[No UVC device connected: %x]\r\n", USBCurrentStates); }

    AHC_USBDetectSetPriority(AHC_FALSE); //Set USB detection as low priority work after first preview.
        printc("-------------USBCurrentStates=%d------------------\r\n",USBCurrentStates);
    if (USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) {
        
        UINT16      usdisplay_width, usdisplay_height;
        UVC_DEVICE_NAME_INDEX sUVCDevcie;
        MMP_BOOL    bUserConfig, bRotate;
        MMP_UBYTE   ubRotateDir, sFitMode;
        MMP_USHORT  usBufWidth, usBufHeight, usStartX, usStartY, usWinWidth, usWinHeight;
        AHC_BOOL    ahcRet = AHC_TRUE;                                      	 
        
        sRet = MMPS_UVCRECD_GetCustomedPrevwAttr(&bUserConfig, &bRotate, &ubRotateDir, &sFitMode,
                                                 &usBufWidth, &usBufHeight,&usStartX, &usStartY, &usWinWidth, &usWinHeight);
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}                 
        
        AHC_HostUVCVideoEnable(AHC_TRUE);
        
        ahcRet = AHC_HostUVCVideoGetChipName(&sUVCDevcie);
        if (ahcRet != AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); /*return AHC_FALSE;*/}                 

        if (usVideoPreviewMode == VIDRECD_NORMAL_PREVIEW_MODE) {
            usdisplay_width = pVideoConfig->previewdata[0].usVidPreviewBufW[VIDRECD_FULL_PREVIEW_MODE];
            usdisplay_height = pVideoConfig->previewdata[0].usVidPreviewBufH[VIDRECD_FULL_PREVIEW_MODE];
        }
        else {
            usdisplay_width = pVideoConfig->previewdata[0].usVidPreviewBufW[usVideoPreviewMode];
            usdisplay_height = pVideoConfig->previewdata[0].usVidPreviewBufH[usVideoPreviewMode];
        }
        
        if (MMP_IsUSBCamIsoMode()) {
            
            AHC_HostUVCVideoSetCmitStrmTyp(BM_USB_STRM_MJPEG);
            
            if (sUVCDevcie == UVC_DEVICE_NAME_PID_0210 ||
                sUVCDevcie == UVC_DEVICE_NAME_PID_64AB ||
                sUVCDevcie == UVC_DEVICE_NAME_PID_9230 ||
                sUVCDevcie == UVC_DEVICE_NAME_PID_2B95) {
                AHC_HostUVCVideoSetting(UVC_H264_RES_720P, UVC_FRAMERATE_25FPS, UVC_H264_QUALITY_HIGH, 25, UVC_YUV_RESOL_180P);
            }
            else if (sUVCDevcie == UVC_DEVICE_NAME_PID_6300 ||
                     sUVCDevcie == UVC_DEVICE_NAME_PID_8301) {
                AHC_HostUVCVideoSetting(UVC_H264_RES_360P, UVC_FRAMERATE_25FPS, UVC_H264_QUALITY_HIGH, 25, UVC_YUV_RESOL_180P);
            }
            else {
                AHC_HostUVCVideoEnable(AHC_FALSE);
                printc(FG_RED("%s,%d error! Not suport device.\r\n"), __func__, __LINE__);
            }
        }
        else {
            
            if (MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_MJPEG_H264) {
                AHC_HostUVCVideoSetCmitStrmTyp(BM_USB_STRM_MJPEG | BM_USB_STRM_H264);
            }
            else if (MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_NV12_H264) {
                AHC_HostUVCVideoSetCmitStrmTyp(BM_USB_STRM_NV12 | BM_USB_STRM_H264);
            }

            if (sUVCDevcie == UVC_DEVICE_NAME_8437 || sUVCDevcie == UVC_DEVICE_NAME_PID_8328) {
                if (AHC_HostUVCVideoIsUVCDecMjpeg2Prevw()) {
                    if (MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_MJPEG_H264) {
                        /* Commit 720p for Mjpeg */
                        gsUVCHostCfg.sMjpegResol = UVC_MJPEG_RESOL_720P;
                    }
                }
                else {
                    AHC_HostUVC_NV12_Mirror(MMP_TRUE);
                }
                
                if (MMP_GetAitCamStreamType() == AIT_REAR_CAM_STRM_NV12_H264){
                    AHC_HostUVCVideoSetting(UVC_H264_RES_1080P, UVC_FRAMERATE_30FPS, UVC_H264_QUALITY_HIGH, ubUVCPframeCount, UVC_YUV_RESOL_480P);
                }
                else{
                	if(sUVCDevcie == UVC_DEVICE_NAME_8437)
                		AHC_HostUVCVideoSetting(UVC_H264_RES_1080P, UVC_FRAMERATE_30FPS, UVC_H264_QUALITY_LOW, ubUVCPframeCount, ubUVCPreviewRes);
                	else if(sUVCDevcie == UVC_DEVICE_NAME_PID_8328)
                    	AHC_HostUVCVideoSetting(UVC_H264_RES_1440P, UVC_FRAMERATE_30FPS, UVC_H264_QUALITY_MID, ubUVCPframeCount, ubUVCPreviewRes);
                }
            }
            else if (sUVCDevcie == UVC_DEVICE_NAME_8451) {
                AHC_HostUVCVideoSetting(UVC_H264_RES_720P, UVC_FRAMERATE_30FPS, UVC_H264_QUALITY_HIGH, ubUVCPframeCount, ubUVCPreviewRes);
            }
            else {
                AHC_HostUVCVideoEnable(AHC_FALSE);
                printc(FG_RED("%s,%d error! Not suport device.\r\n"), __func__, __LINE__);
            }
        }
    
        if (AHC_TRUE == AHC_HostUVCVideoIsEnabled()) {
        
            UINT8    ubWinID;
            UINT16   usWinW, usWinH;
            UINT16   usDispW, usDispH;
            UINT16   usOffsetX, usOffsetY;
            MMP_DISPLAY_COLORMODE sDisplayColorMode;
            
            if (COMMON_FLICKER_50HZ == ulFlickerMode)
                AHC_HostUVCSetFlicker(UVC_FLICKER_50HZ);
            else
                AHC_HostUVCSetFlicker(UVC_FLICKER_60HZ);

            if (bUserConfig) {
            
                if (MMP_IsUSBCamIsoMode()) {  
            
                    if (sUVCDevcie == UVC_DEVICE_NAME_PID_0210 ||
                        sUVCDevcie == UVC_DEVICE_NAME_PID_64AB ||
                        sUVCDevcie == UVC_DEVICE_NAME_PID_9230 ||
                        sUVCDevcie == UVC_DEVICE_NAME_PID_2B95) {
                        ubWinID     = GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor);
                        usWinW      = gUsbh_UvcDevInfo.wPrevwMaxWidth >> 1;
                        usWinH      = gUsbh_UvcDevInfo.wPrevwMaxHeight >> 1;
                        usDispW     = usWinWidth;
                        usDispH     = usWinHeight;
                        usOffsetX   = usStartX;
                        usOffsetY   = usStartY;
                        sDisplayColorMode = MMP_DISPLAY_COLOR_YUV420;
                    }
                    else if (sUVCDevcie == UVC_DEVICE_NAME_PID_6300 ||
                             sUVCDevcie == UVC_DEVICE_NAME_PID_8301) { //Max 640x480
                        ubWinID     = GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor);
                        usWinW      = gUsbh_UvcDevInfo.wPrevwMaxWidth;
                        usWinH      = gUsbh_UvcDevInfo.wPrevwMaxHeight;
                        usDispW     = usWinWidth;
                        usDispH     = usWinHeight;
                        usOffsetX   = usStartX;
                        usOffsetY   = usStartY;
                        sDisplayColorMode = MMP_DISPLAY_COLOR_YUV420;
                    }
                    else {
                        printc(FG_RED("%s,%d error! Not suport device.\r\n"), __func__, __LINE__);
                    }

                    #if (CCIR656_OUTPUT_ENABLE) 
                    usDispW = usdisplay_width; //full screen
                    usDispH = usdisplay_height;
                    usOffsetX = 0;
                    usOffsetY = 0;
                    
                    usWinW = (usWinW > usDispW) ? usWinW : usDispW;
                    usWinH = (usWinH > usDispH) ? usWinH : usDispH;
                    sDisplayColorMode = pVideoConfig->previewdata[1].DispColorFmt[OutputDevice]; //MMP_DISPLAY_COLOR_YUV422
                    #endif
                    
                    #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
                    if (MMP_IsSupportDecMjpegToPreview()) {
                        AHC_HostUVCVideoInitDecMjpegToPreview(ubWinID, usWinW, usWinH, usDispW, usDispH, usOffsetX, usOffsetY, sDisplayColorMode);                                                                           
                    }
                    #endif
                }
                else {
            
                    if (sUVCDevcie == UVC_DEVICE_NAME_8437 || sUVCDevcie == UVC_DEVICE_NAME_PID_8328){
                        if (AHC_HostUVCVideoIsUVCDecMjpeg2Prevw()) {
                            #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
                            if (MMP_IsSupportDecMjpegToPreview()) {
                                AHC_HostUVCVideoInitDecMjpegToPreview(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor),
                                                                      CUR_DEV_8437_MJPEG_W>>1, ALIGN16(CUR_DEV_8437_MJPEG_H>>1), usWinWidth, usWinHeight, 
                                                                      usStartX, usStartY, MMP_DISPLAY_COLOR_YUV420_INTERLEAVE);
                            }
                            #endif
                        }
                    }
                    
                    AHC_HostUVCVideoSetWinAttribute(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), usWinWidth, usWinHeight, usStartX, usStartY, MMP_SCAL_FITMODE_OPTIMAL, bRotate);                    
                }
            }
            else {
            
                if (MMP_IsUSBCamIsoMode()) {
            
                    if (sUVCDevcie == UVC_DEVICE_NAME_PID_0210 ||
                        sUVCDevcie == UVC_DEVICE_NAME_PID_64AB ||
                        sUVCDevcie == UVC_DEVICE_NAME_PID_9230 ||
                        sUVCDevcie == UVC_DEVICE_NAME_PID_2B95) {
                        
                        ubWinID     = GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor);
                        usWinW      = gUsbh_UvcDevInfo.wPrevwMaxWidth >> 1;
                        usWinH      = gUsbh_UvcDevInfo.wPrevwMaxHeight >> 1;
                        usDispW     = usdisplay_width >> 1;
                        usDispH     = usdisplay_height >> 1;
                        usOffsetX   = usdisplay_width >> 1;
                        usOffsetY   = usdisplay_height >> 1;
                        sDisplayColorMode = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
                    }
                    else if (sUVCDevcie == UVC_DEVICE_NAME_PID_6300 ||
                             sUVCDevcie == UVC_DEVICE_NAME_PID_8301){ //Max 640x480
                        
                        ubWinID     = GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor);
                        usWinW      = gUsbh_UvcDevInfo.wPrevwMaxWidth;
                        usWinH      = gUsbh_UvcDevInfo.wPrevwMaxHeight;
                        usDispW     = usdisplay_width >> 1;
                        usDispH     = usdisplay_height >> 1;
                        usOffsetX   = usdisplay_width >> 1;
                        usOffsetY   = usdisplay_height >> 1;
                        sDisplayColorMode = MMP_DISPLAY_COLOR_YUV420_INTERLEAVE;
                    }
                    else {
                        printc(FG_RED("%s,%d error! Not suport device.\r\n"), __func__, __LINE__);
                    }
                    
                    #if (CCIR656_OUTPUT_ENABLE) 
                    usDispW = usdisplay_width; //full screen
                    usDispH = usdisplay_height;
                    usOffsetX = 0;
                    usOffsetY = 0;
                    
                    usWinW = (usWinW > usDispW) ? usWinW : usDispW;
                    usWinH = (usWinH > usDispH) ? usWinH : usDispH;
                    sDisplayColorMode = pVideoConfig->previewdata[1].DispColorFmt[OutputDevice]; //MMP_DISPLAY_COLOR_YUV422
                    #endif
                    
                    #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
                    if (MMP_IsSupportDecMjpegToPreview()) {
                        AHC_HostUVCVideoInitDecMjpegToPreview(ubWinID, usWinW, usWinH, usDispW, usDispH, usOffsetX, usOffsetY, sDisplayColorMode);                                                    
                    }
                    #endif
                }
                else {
                    
                    if (sUVCDevcie == UVC_DEVICE_NAME_8437 || sUVCDevcie == UVC_DEVICE_NAME_PID_8328) {
                        if (AHC_HostUVCVideoIsUVCDecMjpeg2Prevw()) {
                            #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
                            if (MMP_IsSupportDecMjpegToPreview()) {
                                AHC_HostUVCVideoInitDecMjpegToPreview(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor),
                                                                      CUR_DEV_8437_MJPEG_W>>1, CUR_DEV_8437_MJPEG_H>>1, usWinWidth, usWinHeight, 
                                                                      usStartX, usStartY, MMP_DISPLAY_COLOR_YUV420_INTERLEAVE);
                            }
                            #endif
                        }
                    }

                    AHC_HostUVCVideoSetWinAttribute(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), usdisplay_width >> 1, usdisplay_height >> 1, usdisplay_width >> 1, usdisplay_height >> 1, MMP_SCAL_FITMODE_OPTIMAL, bRotate);
                }
            }
        }

        sRet = MMPS_3GPRECD_SetUVCFBMemory();
        if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}
        
        #if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
        if (MMP_IsSupportDecMjpegToPreview())
        {
            MMP_USHORT usW = 0, usH = 0;

            sRet = MMPS_3GPRECD_SetUVCRearMJPGMemory();
            sRet |= MMPS_3GPRECD_GetDecMjpegToPreviewSrcAttr(&usW, &usH);
            sRet |= MMPS_3GPRECD_InitDecMjpegToPreview(usW, usH);
            
            if (GET_VR_PREVIEW_ACTIVE(gsAhcUsbhSensor)) {
                sRet |= MMPS_3GPRECD_DecMjpegPreviewStart(gsAhcUsbhSensor);
                if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}                 
            }
        }
        #endif
    }

    AHC_USBGetStates(&USBCurrentStates,AHC_USB_GET_PHASE_CURRENT);
	printc("-----------2--USBCurrentStates=%d------------------\r\n",USBCurrentStates);
    if ((USBCurrentStates == USB_DETECT_PHASE_REAR_CAM) && (AHC_TRUE == AHC_HostUVCVideoIsEnabled())){
        if (uiGetParkingModeEnable() == AHC_TRUE && MenuSettingConfig()->uiMotionDtcSensitivity != MOTION_DTC_SENSITIVITY_OFF && (uiGetParkingCfg()->bParkingModeFuncEn == AHC_TRUE))
        {
        printc("CAM_CHECK_USB(USB_CAM_AIT)=%d\r\n",CAM_CHECK_USB(USB_CAM_AIT));
            if (CAM_CHECK_USB(USB_CAM_AIT))
                AHC_HostUVCVideoEnableMD(AHC_TRUE);
            else
                AHC_HostUVCVideoEnableMD(AHC_FALSE);
        }
        else
        {
        printc("==============1=================\r\n");
            AHC_HostUVCVideoEnableMD(AHC_FALSE);
        }

        AHC_HostUVCVideoPreviewStart();
        
        #if defined(WIFI_PORT) && (WIFI_PORT == 1)
        ncam_set_rear_cam_ready(AHC_TRUE);
        #endif
    }
#endif

    return AHC_TRUE;
}

AHC_BOOL AIHC_UVCPreviewStop(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;

#if (UVC_HOST_VIDEO_ENABLE)
    AHC_BOOL ahcRet = AHC_TRUE;
    
    ahcRet = AHC_HostUVCVideoPreviewStop();
    ahcRet = AHC_HostUVCVideoEnable(AHC_FALSE);
#endif

#if (SUPPORT_DEC_MJPEG_TO_PREVIEW)
    if (MMP_IsSupportDecMjpegToPreview()) {
        sRet = MMPS_3GPRECD_DecMjpegPreviewStop(gsAhcPrmSensor); // CHECK,not use USBH_SENSOR 
        if (sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;} 
    }
#endif

    // Move to AHC_VIDEO_SetRecordMode().
    //sRet = MMPS_Display_SetWinActive(GET_VR_PREVIEW_WINDOW(gsAhcUsbhSensor), MMP_FALSE);
    //if(sRet != MMP_ERR_NONE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}

    return AHC_TRUE;
}

AHC_BOOL AHC_UVCSetIdleStates(void)
{
    MMP_ERR sRet = MMP_ERR_NONE;
    AHC_BOOL ahcRet = AHC_TRUE;
    MMP_BOOL bUVCStates = MMP_TRUE;

    sRet = MMPS_USB_GetDevConnSts(&bUVCStates);
    if ((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    ahcRet = AHC_HostUVCVideoIsEnabled();
    //UVC preview is still working!
    if (ahcRet == AHC_TRUE){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    
    
    sRet = MMPF_USBH_StopCheckDevAliveTimer();
    if ((sRet != MMP_ERR_NONE) || (bUVCStates != MMP_TRUE)){ AHC_PRINT_RET_ERROR(gbAhcDbgBrk,sRet); return AHC_FALSE;}    

    return AHC_TRUE;
}

#else   // UVC_HOST_VIDEO_ENABLE

#if 0
void ____Dummy_Function____(){ruturn;} //dummy
#endif

AHC_BOOL AHC_HostUVCVideoIsEnabled(void)
{
    return AHC_FALSE;
}

AHC_BOOL AHC_HostUVCVideoPreviewStart(void)
{
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCVideoPreviewStop(void)
{
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCVideoSetWinAttribute(UINT8 ubWinID, UINT16 usDisplayW, UINT16 usDisplayH, UINT16 usOffsetX, UINT16 usOffsetY, AHC_BOOL bRotate)
{
    return AHC_TRUE;
}

void UVCRecordCbFileFull(void)
{
	printc("## UVC File full CB ##\r\n");
}

AHC_BOOL AIHC_UVCPreviewStart(MMP_USHORT usVideoPreviewMode, AHC_DISPLAY_OUTPUTPANEL OutputDevice, UINT32 ulFlickerMode)
{
    return AHC_TRUE;
}

AHC_BOOL AIHC_UVCPreviewStop(void)
{   
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCVideoRecordStart(void)
{
    return AHC_TRUE;
}

AHC_BOOL AHC_HostUVCVideoRecordStop(void)
{   
    return AHC_TRUE;
}

AHC_BOOL AHC_UVCSetIdleStates(void)
{
    return AHC_TRUE;
}
#endif  // UVC_HOST_VIDEO_ENABLE
