//==============================================================================
//
//  File        : mmp_reg_nand.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMP_REG_NAND_H_
#define _MMP_REG_NAND_H_

#include "mmp_register.h"

//--------------------------------------------
// NAND structure (0x80005E00)
//--------------------------------------------
typedef struct _AITS_NAND {
	AIT_REG_B	NAD_CAD;					// 0x00
	AIT_REG_B   NAD_CTL;		 			// 0x01
		/*---------------------------------------------*/
		#define NAD_RB			        0x80	
		#define NAD_CE2			        0x40
		#define NAD_CE1			        0x20
		#define NAD_WPN			        0x10
		#define NAD_ALE			        0x08
		#define NAD_CLE			        0x04
		#define NAD_READ	            0x02
		#define NAD_WRITE		        0x00
		#define NAD_EN			        0x01
		/*---------------------------------------------*/
	AIT_REG_B	NAD_TIMING;			        // 0x02
		/*---------------------------------------------*/
		#define NAD_RCV_CYC(_a)	        (_a << 6) & 0xC0	
		#define NAD_CMD_CYC(_a)	        (_a << 3) & 0x38	
		/*---------------------------------------------*/
	AIT_REG_B   					_x03;
	AIT_REG_B	NAD_ECC0;					// 0x04
	AIT_REG_B	NAD_ECC1;					// 0x05
	AIT_REG_B	NAD_ECC2;					// 0x06
	AIT_REG_B						_x07;
	AIT_REG_B	NAD_ECC3;                   // 0x08
	AIT_REG_B	NAD_ECC4;					// 0x09
	AIT_REG_B	NAD_ECC5;					// 0x0A
	AIT_REG_B						_x0B;
	AIT_REG_D	NAD_DMA_ADDR;				// 0x0C
	
	AIT_REG_W	NAD_DMA_LEN;				// 0x10
	AIT_REG_B	NAD_DMA_CTL;				// 0x12
		/*---------------------------------------------*/
		#define NAD_GPIO_EN			    0x04
		#define	NAD_FB_EN			    0x02
		#define NAD_DMA_EN			    0x01
		/*---------------------------------------------*/
	AIT_REG_B	                    _x13;
	AIT_REG_B	NAD_HOST_INT_EN;			// 0x14
	AIT_REG_B	NAD_CPU_INT_EN;				// 0x15
	AIT_REG_B	NAD_HOST_INT_SR;			// 0x16
	AIT_REG_B	NAD_CPU_INT_SR;				// 0x17
		/*-------------------------------------------*/
		#define NAD_DMA_DONE		    0x80
		#define NAD_DMA_TO		        0x40
		#define NAD_AUTO_ADDR_DONE	    0x20
		#define NAD_RB_POS			    0x02
		#define NAD_RB_NEG		        0x01 
		/*-------------------------------------------*/
	AIT_REG_B						_x18[0x18];

	AIT_REG_B	NAD_CMD_CTL;				// 0x30
		/*-------------------------------------------*/
		#define NAD_CMD1_ADDR_CMD2		0x12
		#define NAD_CMD1_ADDR_DMA_CMD2	0x0A
		#define NAD_CMD1_ADDR_CMD2_DMA	0x06
		#define NAD_CMD1_ADDR_DMA		0x01
		/*-------------------------------------------*/
	AIT_REG_B	NAD_CMD_TIMING;				// 0x31
	AIT_REG_B	NAD_CMD_1;					// 0x32
	AIT_REG_B	NAD_CMD_2;					// 0x33
	AIT_REG_B	NAD_ADDR_CTL;				// 0x34
		/*-------------------------------------------*/
		#define NAD_BYPSS_ADDR			0x10
		#define NAD_ADDR_CYCLE_MASK     0x03
		/*-------------------------------------------*/
	AIT_REG_B	NAD_ADDR_TIMING;			// 0x35
		/*-------------------------------------------*/
		#define NAD_ALS_MASK            0x30
		#define NAD_ALH_MASK            0x03
		/*-------------------------------------------*/
	AIT_REG_B	NAD_ADDR[6];				// 0x36
	AIT_REG_W	NAD_TR;						// 0x3C
    AIT_REG_B                       _x3E;
	AIT_REG_B	NAD_EXC_CTL;				// 0x3F
		/*------------------------------------------*/
		#define NAD_EXC_ST         	    0x01
		/*------------------------------------------*/
		
	AIT_REG_B	NAD_RS[10];					// 0x40
	AIT_REG_B	NAD_RDN_BYTE;				// 0x4A
	AIT_REG_B	NAD_RDN_CTL;				// 0x4B
		/*-------------------------------------------*/
		#define NAD_RSECC_EN			0x04
		#define NAD_RS_EN				0x01
		/*-------------------------------------------*/
	AIT_REG_B	NAD_CMD_ST_HIGH;			// 0x4C
	AIT_REG_B	NAD_CMD_ST_LOW;				// 0x4D
	AIT_REG_B						_x4E[2];
	
	AIT_REG_D	NAD_ABORT_TIMING;			// 0x50
	AIT_REG_D	NAD_ECC_DMA_ADDR;			// 0x54
	AIT_REG_B	NAD_RS_ADDR_SEG;			// 0x58 
	AIT_REG_B	NAD_PAGE_SEG;				// 0x59
	AIT_REG_B	NAD_ECC_NUM;				// 0x5A
	AIT_REG_B	NAD_ECC_STATE;				// 0x5B
		/*--------------------------------------------*/
		#define NAD_ECC_NO_ERR			0x00
		#define NAD_ECC_TOO_MANY_ERR	0x01
		#define NAD_ECC_CANCORRECT  	0x03
		/*--------------------------------------------*/
	AIT_REG_B	NAD_ECC_SERR;				// 0x5C
	AIT_REG_B	NAD_ECC_CORR_CTL;		    // 0x5D
	AIT_REG_B						_x5E[2];
	
	AIT_REG_W   NAND_ECC_ERR_ADDR1;         // 0x60
	AIT_REG_W   NAND_ECC_ERR_ADDR2;         // 0x62
	AIT_REG_W   NAND_ECC_ERR_ADDR3;         // 0x64
	AIT_REG_W   NAND_ECC_ERR_ADDR4;         // 0x66
	AIT_REG_W   NAND_ECC_ERR_VAL1;          // 0x68
	AIT_REG_W   NAND_ECC_ERR_VAL2;          // 0x6A
	AIT_REG_W   NAND_ECC_ERR_VAL3;          // 0x6C
	AIT_REG_W   NAND_ECC_ERR_VAL4;          // 0x6E
} AITS_NAND, *AITPS_NAND;

#endif //_MMP_REG_NAND_H_
