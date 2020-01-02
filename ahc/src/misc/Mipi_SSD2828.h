/*******************************************************************************
 *
 *   UartShell.c
 *
 *   Interface for the UartShell class.
 *
 *   Copyright 2010 by Caesar Chang.
 *
 *   AUTHOR : Caesar Chang
 *
 *   VERSION: 1.0
 *
 *
*******************************************************************************/


#ifndef _MIPI_SSD2828_H_
#define _MIPI_SSD2828_H_

//void Init_SSD2828_SPI(void);
void Set_RST(unsigned long index);
void Set_2828_CS(unsigned long index);
void Set_SCL(unsigned long index);
void Set_SDI(unsigned long index);
void SPI_init_gpio(void);

void SPI_SET_Cmd(unsigned char cmd);
void SPI_SET_PAs(unsigned char value);
void SSD2828_Read_Status(void);
void SSD2828_Read_Burst(unsigned char cmd, unsigned short bytenum);
void SPI_2828_WrReg(unsigned char c,unsigned short value);
void SPI_2828_RdReg(unsigned char cmd, unsigned short *reg_val);
void SSD2828_Send_Read_Dsi_Packet(unsigned char reg, unsigned char bytnum);
void SSD2828_Send_Write_Dsi_Packet(char *ParamList);

#define		SSD2828_DIR	    0xB0
#define		SSD2828_VICR1	0xB1
#define		SSD2828_VICR2	0xB2
#define		SSD2828_VICR3	0xB3
#define		SSD2828_VICR4	0xB4
#define		SSD2828_VICR5	0xB5
#define		SSD2828_VICR6	0xB6
#define		SSD2828_CFGR	0xB7
#define		SSD2828_VCR	    0xB8
#define		SSD2828_PCR	    0xB9
#define		SSD2828_PLCR	0xBA
#define		SSD2828_CCR	    0xBB
#define		SSD2828_PSCR1	0xBC
#define		SSD2828_PSCR2	0xBD
#define		SSD2828_PSCR3	0xBE
#define		SSD2828_PDR	    0xBF
#define		SSD2828_OCR	    0xC0
#define		SSD2828_MRSR	0xC1
#define		SSD2828_RDCR	0xC2
#define		SSD2828_ARSR	0xC3
#define		SSD2828_LCR	    0xC4
#define		SSD2828_ICR	    0xC5
#define		SSD2828_ISR	    0xC6
#define		SSD2828_ESR	    0xC7
#define		SSD2828_DAR1	0xC9
#define		SSD2828_DAR2	0xCA
#define		SSD2828_DAR3	0xCB
#define		SSD2828_DAR4	0xCC
#define		SSD2828_DAR5	0xCD
#define		SSD2828_DAR6	0xCE
#define		SSD2828_HTTR1	0xCF
#define		SSD2828_HTTR2	0xD0
#define		SSD2828_LRTR1	0xD1
#define		SSD2828_LRTR2	0xD2
#define		SSD2828_TSR	    0xD3
#define		SSD2828_LRR	    0xD4
#define		SSD2828_PLLR	0xD5
#define		SSD2828_TR	    0xD6
#define		SSD2828_TECR	0xD7
#define		SSD2828_ACR1	0xD8
#define		SSD2828_ACR2	0xD9
#define		SSD2828_ACR3	0xDA
#define		SSD2828_ACR4	0xDB
#define		SSD2828_IOCR	0xDC
#define		SSD2828_VICR7	0xDD
#define		SSD2828_LCFR	0xDE
#define		SSD2828_DAR7	0xDF
#define		SSD2828_PUCR1	0xE0
#define		SSD2828_PUCR2	0xE1
#define		SSD2828_PUCR3	0xE2
#define		SSD2828_CBCR1	0xE9
#define		SSD2828_CBCR2	0xEA
#define		SSD2828_CBSR	0xEB
#define		SSD2828_ECR	    0xEC
#define		SSD2828_VSDR	0xED
#define		SSD2828_TMR	    0xEE
#define		SSD2828_GPIO1	0xEF
#define		SSD2828_GPIO2	0xF0
#define		SSD2828_DLYA01	0xF1
#define		SSD2828_DLYA23	0xF2
#define		SSD2828_DLYB01	0xF3
#define		SSD2828_DLYB23	0xF4
#define		SSD2828_DLYC01	0xF5
#define		SSD2828_DLYC23	0xF6
#define		SSD2828_ACR5	0xF7
#define		SSD2828_RR	0xFF

// for register 0xB6
#define     VICR6_VSYNC_ACT_HIGH                    (1<<15)
#define     VICR6_HSYNC_ACT_HIGH                    (1<<14)
#define     VICR6_DATA_LATCH_RISE_EDGE              (1<<13)
#define     VICR6_VIDEO_NON_BURST_SYNC_PULSE        ((0)<<2)
#define     VICR6_VIDEO_NON_BURST_SYNC_EVENT        ((1)<<2)
#define     VICR6_VIDEO_BURST_MODE                  ((2)<<2)
#define     VICR6_VIDEO_DATA_FORMAT_16BPP           (0x00)
#define     VICR6_VIDEO_DATA_FORMAT_18BPP           (0x01)
#define     VICR6_VIDEO_DATA_FORMAT_18BPP_L         (0x10)
#define     VICR6_VIDEO_DATA_FORMAT_24              (0x11)

// for register 0xB7
#define     CFGR_LONG_PKG_EN                        (1<<10)
#define     CFGR_EOT_PKG_EN                         (1<<9)
#define     CFGR_ECC_CRC_CHK_EN                     (1<<8)
#define     CFGR_READ_EN                            (1<<7)
#define     CFGR_DCS_PKG_EN                         (1<<6)
#define     CFGR_CLK_SRC_SEL                        (1<<5) // from pclk or tx_clk
#define     CFGR_HS_CLK_EN                          (1<<4)
#define     CFGR_VID_MODE_EN                        (1<<3)
#define     CFGR_SLEEP_MODE_EN                      (1<<2)
#define     CFGR_CLK_LANE_EN                        (1<<1) //clock enter LP mode if it not in reverse direction, clock lane will enter HS mode
#define     CFGR_HS_MODE                            (1<<0) // 0 - LP mode , 1 - HS mode

#define SSD2828_LANE_CFG_1_LANE 0
#define SSD2828_LANE_CFG_2_LANE 1
#define SSD2828_LANE_CFG_3_LANE 2
#define SSD2828_LANE_CFG_4_LANE 3

// for register 0xC5
#define     ICR_PLL_LOCK_STATUS_EN                   (1<<7) // 
#define     ICR_LP_RX_TIME_OUT_STATUS_EN             (1<<6) // 
#define     ICR_HS_TX_TIME_OUT_STATUS_EN             (1<<5) // 
#define     ICR_READ_PKG_STATUS_EN                   (1<<0) // 
// for register 0xC6
#define     ISR_CLK_LANE_STATUS                      (1<<11) // 1 = Clock lane is in LP-11, 0 = Clock lane is NOT in LP-11
#define     ISR_DATA_LANE_STATUS                     (1<<10) // 1 = Data lane is in LP-11, 0 = Data lane is NOT in LP-11
#define     ISR_PLL_LOCK_STATUS                      (1<<7) // 
#define     ISR_LP_RX_TIME_OUT_STATUS                (1<<6) // 
#define     ISR_HS_TX_TIME_OUT_STATUS                (1<<5) // 
#define     ISR_READ_PKG_STATUS                      (1<<0) // 

#define ISR_ATR    (1<<4)
#define ISR_ARR     (1<<3)
#define ISR_BTAR    (1<<2)
//#define Lcd_95_A00_540_960		0
//#define Lcd_765_E00_480_854		0
//#define lcd_HX8394_1280_720  0

#endif 
