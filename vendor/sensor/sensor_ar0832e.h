//==============================================================================
//
//  File        : sensor_ar0832e.h
//  Description : Firmware Sensor Control File
//  Author      : 
//  Revision    : 1.0
//
//==============================================================================

#ifndef SENSOR_AR0832E_H_
#define SENSOR_AR0832E_H_

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

#ifndef SENSOR_IF
#define SENSOR_IF                   SENSOR_IF_MIPI_2_LANE
#endif

#ifndef HD_30P_FOV_ENLARGE
#define HD_30P_FOV_ENLARGE          (1) // maybe defined in Config_SDK_xxx.h
#endif

#ifndef FHD_30P_FOV_ENLARGE
#define FHD_30P_FOV_ENLARGE         HD_30P_FOV_ENLARGE // maybe defined in Config_SDK_xxx.h
#endif

#ifndef SHD_30P_FOV_ENLARGE
#define SHD_30P_FOV_ENLARGE         HD_30P_FOV_ENLARGE // maybe defined in Config_SDK_xxx.h
#endif

/* The default slave addresses used by the AR0832E for the MIPI configured sensor are
 * 0x6C (write address) and 0x6D (read address) in accordance with the MIPI specification.
 * Alternate slave addresses of 0x6E (write address) and 0x6F (read address) can be
 * selected by enabling and asserting the SADDR signal through the GPI pad.
 */
#ifndef SENSOR_I2C_ADDR_AR0832E
#define SENSOR_I2C_ADDR_AR0832E     (0x6C >> 1) // default: 0x36, alternate: 0x37
#endif

#define RES_IDX_2304x1296_30P       (0) // mode 0,  2304*1296 30P   // Video (16:9), FOV = 80%?     // TBD, FOV should be comfirmed with sensor vendor later
#define RES_IDX_1280x720_30P        (1) // mode 1,  1280*720  30P   // Video (16:9), FOV = 100%?    // TBD, FOV should be comfirmed with sensor vendor later
#define RES_IDX_2592x1944_18P       (2) // mode 2,  2592*1944 18P   // Camera (4:3), FOV = ?        // TBD, frame rate should be adjusted to 30fps later

#ifndef SENSOR_TEST_PATTERN_EN
#define SENSOR_TEST_PATTERN_EN      (0)
#endif

//==============================================================================
//
//                              MACRO DEFINE (Resolution For UI)
//
//==============================================================================

#ifndef SENSOR_DRIVER_MODE_NOT_SUUPORT
#define SENSOR_DRIVER_MODE_NOT_SUUPORT              (0xFFFF)
#endif

// Index 0
#define SENSOR_DRIVER_MODE_VGA_30P_RESOLUTION       (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 640*360 30P
#define SENSOR_DRIVER_MODE_VGA_50P_RESOLUTION       (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 640*360 50P
#define SENSOR_DRIVER_MODE_VGA_60P_RESOLUTION       (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 640*360 60P
#define SENSOR_DRIVER_MODE_VGA_100P_RESOLUTION      (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 640*360 100P
#define SENSOR_DRIVER_MODE_VGA_120P_RESOLUTION      (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 640*360 120P

// Index 5
#define SENSOR_DRIVER_MODE_HD_24P_RESOLUTION        (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 1280*720 24P
#if (HD_30P_FOV_ENLARGE)
#define SENSOR_DRIVER_MODE_HD_30P_RESOLUTION        (RES_IDX_1280x720_30P)              // 1280*720 30P
#else
#define SENSOR_DRIVER_MODE_HD_30P_RESOLUTION        (RES_IDX_2304x1296_30P)             // 1280*720 30P
#endif
#define SENSOR_DRIVER_MODE_HD_50P_RESOLUTION        (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 1280*720 50P
#define SENSOR_DRIVER_MODE_HD_60P_RESOLUTION        (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 1280*720 60P
#define SENSOR_DRIVER_MODE_HD_100P_RESOLUTION       (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 1280*720 100P

// Index 10
#define SENSOR_DRIVER_MODE_HD_120P_RESOLUTION       (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 1280*720 120P
#if (FHD_30P_FOV_ENLARGE)
#define SENSOR_DRIVER_MODE_1600x900_30P_RESOLUTION  (RES_IDX_2304x1296_30P)             // 1600*900 30P
#else
#define SENSOR_DRIVER_MODE_1600x900_30P_RESOLUTION  (RES_IDX_2304x1296_30P)             // 1600*900 30P
#endif
#define SENSOR_DRIVER_MODE_FULL_HD_15P_RESOLUTION   (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 1920*1080 15P
#define SENSOR_DRIVER_MODE_FULL_HD_24P_RESOLUTION   (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 1920*1080 24P
#define SENSOR_DRIVER_MODE_FULL_HD_25P_RESOLUTION   (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 1920*1080 25P

// Index 15
#if (FHD_30P_FOV_ENLARGE)
#define SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION   (RES_IDX_1280x720_30P)              // 1920*1080 30P
#else
#define SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION   (RES_IDX_2304x1296_30P)             // 1920*1080 30P
#endif
#define SENSOR_DRIVER_MODE_FULL_HD_50P_RESOLUTION   (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 1920*1080 50P
#define SENSOR_DRIVER_MODE_FULL_HD_60P_RESOLUTION   (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 1920*1080 60P
#if (SHD_30P_FOV_ENLARGE)
#define SENSOR_DRIVER_MODE_SUPER_HD_30P_RESOLUTION  (RES_IDX_1280x720_30P)              // 2304*1296 30P
#else
#define SENSOR_DRIVER_MODE_SUPER_HD_30P_RESOLUTION  (RES_IDX_2304x1296_30P)             // 2304*1296 30P
#endif
#define SENSOR_DRIVER_MODE_SUPER_HD_25P_RESOLUTION  (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 2304*1296 25P

// Index 20
#define SENSOR_DRIVER_MODE_SUPER_HD_24P_RESOLUTION  (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 2304*1296 24P
#define SENSOR_DRIVER_MODE_1440_30P_RESOLUTION  	(SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 2560*1440 30P
#define SENSOR_DRIVER_MODE_2D7K_15P_RESOLUTION      (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 2704*1524 15P
#define SENSOR_DRIVER_MODE_2D7K_30P_RESOLUTION      (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 2704*1524 30P
#define SENSOR_DRIVER_MODE_4K2K_15P_RESOLUTION      (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 3840*2160 15P

// Index 25
#define SENSOR_DRIVER_MODE_4K2K_30P_RESOLUTION      (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 3840*2160 30P
#define SENSOR_DRIVER_MODE_4TO3_VGA_30P_RESOLUTION  (RES_IDX_2592x1944_18P)             // 640*480   30P // TBD, frame rate should be adjusted to 30fps later
#define SENSOR_DRIVER_MODE_4TO3_1D2M_30P_RESOLUTION (RES_IDX_2592x1944_18P)             // 1280*960  30P // TBD, frame rate should be adjusted to 30fps later
#define SENSOR_DRIVER_MODE_4TO3_1D5M_30P_RESOLUTION (RES_IDX_2592x1944_18P)             // 1440*1080 30P // TBD, frame rate should be adjusted to 30fps later
#define SENSOR_DRIVER_MODE_4TO3_3M_15P_RESOLUTION   (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 2048*1536 15P

// Index 30
#define SENSOR_DRIVER_MODE_4TO3_3M_30P_RESOLUTION   (RES_IDX_2592x1944_18P)             // 2048*1536 30P // TBD, frame rate should be adjusted to 30fps later
#define SENSOR_DRIVER_MODE_4TO3_5M_15P_RESOLUTION   (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 2560*1920 15P
#define SENSOR_DRIVER_MODE_4TO3_5M_30P_RESOLUTION   (RES_IDX_2592x1944_18P)             // 2560*1920 30P // TBD, frame rate should be adjusted to 30fps later
#define SENSOR_DRIVER_MODE_4TO3_8M_15P_RESOLUTION   (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 3264*2448 15P
#define SENSOR_DRIVER_MODE_4TO3_8M_30P_RESOLUTION   (RES_IDX_2592x1944_18P)             // 3264*2448 30P // TBD, frame rate should be adjusted to 30fps later

// Index 35
#define SENSOR_DRIVER_MODE_4TO3_10M_15P_RESOLUTION  (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 3648*2736 15P
#define SENSOR_DRIVER_MODE_4TO3_10M_30P_RESOLUTION  (RES_IDX_2592x1944_18P)             // 3648*2736 30P // TBD, frame rate should be adjusted to 30fps later
#define SENSOR_DRIVER_MODE_4TO3_12M_15P_RESOLUTION  (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 4032*3024 15P
#define SENSOR_DRIVER_MODE_4TO3_12M_30P_RESOLUTION  (RES_IDX_2592x1944_18P)             // 4032*3024 30P // TBD, frame rate should be adjusted to 30fps later
#define SENSOR_DRIVER_MODE_4TO3_14M_15P_RESOLUTION  (SENSOR_DRIVER_MODE_NOT_SUUPORT)    // 4352*3264 15P

// Index 40
#define SENSOR_DRIVER_MODE_4TO3_14M_30P_RESOLUTION  (RES_IDX_2592x1944_18P)             // 4352*3264 30P // TBD, frame rate should be adjusted to 30fps later
#define SENSOR_DRIVER_MODE_4K2K_24P_RESOLUTION      (SENSOR_DRIVER_MODE_NOT_SUUPORT)

// For Camera Preview
#define SENSOR_DRIVER_MODE_BEST_CAMERA_CAPTURE_16TO9_RESOLUTION (RES_IDX_2304x1296_30P)
#define SENSOR_DRIVER_MODE_BEST_CAMERA_CAPTURE_4TO3_RESOLUTION  (RES_IDX_2592x1944_18P) // TBD, frame rate should be adjusted to 30fps later

#if (LCD_MODEL_RATIO_X == 16) && (LCD_MODEL_RATIO_Y == 9)
#define SENSOR_DRIVER_MODE_BEST_CAMERA_PREVIEW_RESOLUTION       (SENSOR_DRIVER_MODE_BEST_CAMERA_CAPTURE_16TO9_RESOLUTION)
#else
#define SENSOR_DRIVER_MODE_BEST_CAMERA_PREVIEW_RESOLUTION       (SENSOR_DRIVER_MODE_BEST_CAMERA_CAPTURE_4TO3_RESOLUTION)
#endif

#if (HDR_FOV_ENLARGE)
#define SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION_HDR           (SENSOR_DRIVER_MODE_NOT_SUUPORT)
#else
#define SENSOR_DRIVER_MODE_FULL_HD_30P_RESOLUTION_HDR           (SENSOR_DRIVER_MODE_NOT_SUUPORT)
#endif

#endif //_SENSOR_AR0832E_H_
