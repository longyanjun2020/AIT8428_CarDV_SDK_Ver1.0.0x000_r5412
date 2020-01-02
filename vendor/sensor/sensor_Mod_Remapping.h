//==============================================================================
//
//  File        : sensor_Mod_Remapping.h
//  Description : Firmware Sensor Control File
//  Author      : Andy Liu
//  Revision    : 1.0
//
//==============================================================================

#ifndef	_SENSOR_MOD_REMAPPING_
#define	_SENSOR_MOD_REMAPPING_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "includes_fw.h"
#include "customer_config.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#ifndef SENSOR_DRIVER_MODE_NOT_SUUPORT
#define SENSOR_DRIVER_MODE_NOT_SUUPORT  (0xFFFF)
#endif

#if (BIND_SENSOR_AR0237)
#include "sensor_ar0237.h"
#endif // BIND_SENSOR_AR0237

#if (BIND_SENSOR_AR0330)
#include "sensor_ar0330.h"
#endif // BIND_SENSOR_AR0330

#if (BIND_SENSOR_AR0330_OTPM)
#include "sensor_ar0330_OTPM.h"
#endif // BIND_SENSOR_AR0330_OTPM

#if (BIND_SENSOR_AR0331)
#include "sensor_ar0331.h"
#endif // (BIND_SENSOR_AR0331)

#if (BIND_SENSOR_AR0835)
#include "sensor_ar0835.h"
#endif // (BIND_SENSOR_AR0835)

#if (BIND_SENSOR_AR1820)
#include "sensor_ar1820.h"
#endif // (BIND_SENSOR_AR1820)

#if (BIND_SENSOR_GC2023)
#include "sensor_gc2023.h"
#endif // (BIND_SENSOR_GC2023)

#if (BIND_SENSOR_IMX175)
#include "sensor_imx175.h"
#endif // (BIND_SENSOR_IMX175)

#if (BIND_SENSOR_IMX214)
#include "sensor_imx214.h"
#endif // (BIND_SENSOR_IMX214)

#if (BIND_SENSOR_IMX291)
#include "sensor_imx291.h"
#endif // (BIND_SENSOR_IMX291)

#if (BIND_SENSOR_IMX322)
#include "sensor_imx322.h"
#endif // (BIND_SENSOR_IMX322)

#if (BIND_SENSOR_IMX326)
#include "sensor_imx326.h"
#endif // (BIND_SENSOR_IMX326)

#if (BIND_SENSOR_OV2643)
#include "sensor_ov2643.h"
#endif // (BIND_SENSOR_OV2643)

#if (BIND_SENSOR_AP1302)
#include "sensor_ap1302.h"
#endif // (BIND_SENSOR_AP1302)

#if (BIND_SENSOR_OV2710)
#include "sensor_ov2710.h"
#endif // (BIND_SENSOR_OV2710)

#if (BIND_SENSOR_OV2710_MIPI)
#include "sensor_ov2710_mipi.h"
#endif // (BIND_SENSOR_OV2710_MIPI)

#if (BIND_SENSOR_OV2718)
#include "sensor_ov2718.h"
#endif // (BIND_SENSOR_OV2718)

#if (BIND_SENSOR_OV2718_2A)
#include "sensor_ov2718_2A.h"
#endif // (BIND_SENSOR_OV2718_2A)

#if (BIND_SENSOR_OV2735)
#include "sensor_OV2735.h"
#endif // (BIND_SENSOR_OV2735)

#if (BIND_SENSOR_H42_MIPI)
#include "sensor_H42_mipi.h"
#endif // (BIND_SENSOR_H42_MIPI)

#if (BIND_SENSOR_OV4689)
#include "sensor_ov4689.h"
#endif // (BIND_SENSOR_OV4689)

#if (BIND_SENSOR_OV4689_2LINE)
#include "sensor_ov4689_2lane.h"
#endif // (BIND_SENSOR_OV4689_2LINE)

#if (BIND_SENSOR_OV5653)
#include "sensor_ov5653.h"
#endif // (BIND_SENSOR_OV5653)

#if (BIND_SENSOR_OV9712)
#include "sensor_ov9712.h"
#endif // (BIND_SENSOR_OV9712)

#if (BIND_SENSOR_OV9732_MIPI)
#include "sensor_ov9732_mipi.h"
#endif // (BIND_SENSOR_OV9732_MIPI)

#if (BIND_SENSOR_OV10822)
#include "sensor_ov10822.h"
#endif // (BIND_SENSOR_OV10822)

#if (BIND_SENSOR_DM5150)
#include "TvDecoder_dm5150.h"
#endif // (BIND_SENSOR_DM5150)

#if (BIND_SENSOR_BIT1603)
#include "TvDecoder_BIT1603.h"
#endif // (BIND_SENSOR_BIT1603)

#if (BIND_SENSOR_TW9992)
#include "TvDecoder_TW9992.h"
#endif // (BIND_SENSOR_TW9992)

#if (BIND_SENSOR_NVP6124B)
#include "TvDecoder_NVP6124B.h"
#endif // (BIND_SENSOR_NVP6124B)

#if (BIND_SENSOR_CJC5150)
#include "TvDecoder_CJC5150.h"
#endif // (BIND_SENSOR_CJC5150)

#if (BIND_SENSOR_GM7150)
#include "TvDecoder_GM7150.h"
#endif // (BIND_SENSOR_GM7150)

#if (BIND_SENSOR_RN6750)
#include "TvDecoder_RN6750.h"
#endif // (BIND_SENSOR_RN6750)

#if (BIND_SENSOR_TP2825)
#include "TvDecoder_TP2825.h"
#endif // (BIND_SENSOR_TP2825)

#if (BIND_SENSOR_PR2000)
#include "TvDecoder_PR2000.h"
#endif // (BIND_SENSOR_PR2000)

#if (BIND_SENSOR_CP8210)
#include "sensor_cp8210.h"
#endif // (BIND_SENSOR_CP8210)

#if (BIND_SENSOR_PS1210)
#include "sensor_ps1210.h"
#endif // (BIND_SENSOR_PS1210)

#if (BIND_SENSOR_JXF02)
#include "sensor_jxf02.h"
#endif // (BIND_SENSOR_JXF02)

#if (BIND_SENSOR_JXF22)
#include "sensor_jxf22.h"
#endif // (BIND_SENSOR_JXF22)

#if (BIND_SENSOR_PS5226)
#include "sensor_ps5226.h"
#endif // (BIND_SENSOR_PS5226)

#if (BIND_SENSOR_AR0832E)
#include "sensor_ar0832e.h"
#endif // (BIND_SENSOR_AR0832E)

#if (BIND_SENSOR_HM1375)
#include "sensor_hm1375.h"
#endif // (BIND_SENSOR_HM1375)

#if (BIND_SENSOR_SC2143)
#include "sensor_sc2143.h"
#endif // (BIND_SENSOR_SC2143)

#if (BIND_SENSOR_BRV0200)
#include "sensor_brv0200.h"
#endif // (BIND_SENSOR_BRV0200)

#if (BIND_SENSOR_BRV0500)
#include "sensor_brv0500.h"
#endif // (BIND_SENSOR_BRV0500)

/////////////////////////////////////////////////////////////////////////////////////////////////
// Keep this at last line
#ifndef SENSOR_DRIVER_MODE_PCCAM_DEFAULT_RESOLUTION
#define SENSOR_DRIVER_MODE_PCCAM_DEFAULT_RESOLUTION     (SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION)
#endif

#ifndef SENSOR_DRIVER_MODE_PAL_25P_RESOLUTION
#define SENSOR_DRIVER_MODE_PAL_25P_RESOLUTION           (SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION)
#endif

#ifndef SENSOR_DRIVER_MODE_NTSC_30P_RESOLUTION
#define SENSOR_DRIVER_MODE_NTSC_30P_RESOLUTION          (SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION)
#endif

#ifndef SENSOR_DRIVER_MODE_AHDHD_30P_RESOLUTION
#define SENSOR_DRIVER_MODE_AHDHD_30P_RESOLUTION          (SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION)
#endif

#ifndef SENSOR_DRIVER_MODE_AHDHD_25P_RESOLUTION
#define SENSOR_DRIVER_MODE_AHDHD_25P_RESOLUTION          (SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION)
#endif

#ifndef SENSOR_DRIVER_MODE_AHDFHD_30P_RESOLUTION
#define SENSOR_DRIVER_MODE_AHDFHD_30P_RESOLUTION          (SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION)
#endif

#ifndef SENSOR_DRIVER_MODE_AHDFHD_25P_RESOLUTION
#define SENSOR_DRIVER_MODE_AHDFHD_25P_RESOLUTION          (SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION)
#endif


#endif	// _SENSOR_MOD_REMAPPING_
