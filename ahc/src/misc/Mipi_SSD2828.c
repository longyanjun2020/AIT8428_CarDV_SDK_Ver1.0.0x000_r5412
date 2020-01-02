/*********************************************************************
;* Project Name : s3c2443x
;*
;* Copyright 2006 by Samsung Electronics, Inc.
;* All rights reserved.
;*
;* Project Description :
;* This software is only for verifying functions of the s3c2443x
;* Anybody can use this code without our permission.
;**********************************************************************/
#include "Customer_config.h" // CarDV
#include "mmpf_pio.h"
#include "Mipi_SSD2828.h"

#if 1//(ENABLE_MIPI_PANEL)

#define Lcd_LP079X01_768_1024	0
#define lcd_HX8394_1280_720		1
#define Lcd_95_A00_540_960		2
#define Lcd_765_E00_480_854		3

#define SSD2828_MIPI_PANEL_USED		(lcd_HX8394_1280_720)

#if (SSD2828_MIPI_PANEL_USED == Lcd_95_A00_540_960)
#define LCD_XSIZE_TFT  (540)
#define LCD_YSIZE_TFT  (960)	

#define LCD_VBPD		(12)   // 13
#define LCD_VFPD		(7)	   // 12
#define LCD_VSPW		(1)	   // 1
#define LCD_HBPD		(13)   // 13
#define LCD_HFPD		(11)   // 11
#define LCD_HSPW		(4)	   // 4

#elif (SSD2828_MIPI_PANEL_USED == Lcd_LP079X01_768_1024)

/*#define LCD_VBPD		(14)
#define LCD_VFPD		(15)
#define LCD_VSPW		(16)
#define LCD_HBPD		(38)
#define LCD_HFPD		(38)
#define LCD_HSPW		(40)*/
#elif (SSD2828_MIPI_PANEL_USED == Lcd_765_E00_480_854)
#define LCD_XSIZE_TFT  (480)
#define LCD_YSIZE_TFT  (854)	
#define LCD_VBPD		(14)
#define LCD_VFPD		(6)
#define LCD_VSPW		(2)
#define LCD_HBPD		(8)
#define LCD_HFPD		(8)
#define LCD_HSPW		(8)

#elif (SSD2828_MIPI_PANEL_USED == lcd_HX8394_1280_720)
#define LCD_YSIZE_TFT  (720)
#define LCD_XSIZE_TFT  (1280)	
#define LCD_HBPD		(5)
#define LCD_HFPD		(25)
#define LCD_HSPW		(5)
#define LCD_VBPD		(10)
#define LCD_VFPD		(30)
#define LCD_VSPW		(20)

#else
"error";
#endif

#define LSCE_GPIO_PIN   (MMP_GPIO124)
#define LSCK_GPIO_PIN   (MMP_GPIO122)
#define LSDA_GPIO_PIN   (MMP_GPIO123)
#if 0
#define LSDR_GPIO_PIN   (MMP_GPIO28)//(MMP_GPIO60)
#else
#define LSDR_GPIO_PIN   (MMP_GPIO24)// Reset pin
#endif
#define LSDI_GPIO_PIN   (MMP_GPIO57)


#define Delay_us(n)   MMPC_System_WaitUs(n*100/27)	//(lcm_util.udelay(n))
#define Delay_ms(n)   MMPC_System_WaitMs(n*100/27)	//(lcm_util.mdelay(n))
                                           
void Set_RST(unsigned long index)
{
	if(index) MMPF_PIO_SetData(LSDR_GPIO_PIN, GPIO_HIGH, MMP_TRUE);//rGPEDAT |= 0x8000;
	else      MMPF_PIO_SetData(LSDR_GPIO_PIN, GPIO_LOW, MMP_TRUE);//rGPEDAT &= ~0x8000;	
}

void Set_2828_CS(unsigned long index)
{
	if(index) MMPF_PIO_SetData(LSCE_GPIO_PIN, GPIO_HIGH, MMP_TRUE);//rGPEDAT |= 0x8000;
	else       MMPF_PIO_SetData(LSCE_GPIO_PIN, GPIO_LOW, MMP_TRUE);//rGPEDAT &= ~0x8000;	
}

void Set_SCL(unsigned long index)
{
	if(index) MMPF_PIO_SetData(LSCK_GPIO_PIN, GPIO_HIGH, MMP_TRUE);//rGPEDAT |= 0x2000;
	else       MMPF_PIO_SetData(LSCK_GPIO_PIN, GPIO_LOW, MMP_TRUE);//rGPEDAT &= ~0x2000;	
}

void Set_SDI(unsigned long index)
{
	if(index) MMPF_PIO_SetData(LSDA_GPIO_PIN, GPIO_HIGH, MMP_TRUE);//rGPEDAT |= 0x1000;
	else      MMPF_PIO_SetData(LSDA_GPIO_PIN, GPIO_LOW, MMP_TRUE);//rGPEDAT &= ~0x1000;	
}


void SPI_init_gpio(void)
{	
	//Delay_ms(100);
	MMPF_PIO_EnableOutputMode(LSCE_GPIO_PIN, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_EnableOutputMode(LSCK_GPIO_PIN, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_EnableOutputMode(LSDA_GPIO_PIN, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_EnableOutputMode(LSDR_GPIO_PIN, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_EnableOutputMode(LSDI_GPIO_PIN, MMP_FALSE, MMP_TRUE);
	//Delay_ms(10);
	Set_2828_CS(1);
	Set_RST(1);
	//Set_CSX(1);
	Set_SCL(0);	
	Set_SDI(0);
	
}
#if 0 // for IDE function name list by section
void ___SPI_8BIT_3WIRE_____(){}
#endif
void SPI_SET_Cmd(unsigned char cmd)
{
	unsigned long kk;
	
	Set_SDI(0);			//Set DC=0, for writing to Command register
	Set_SCL(0);
	Set_SCL(1);	
	Set_SCL(0);
	for(kk=0;kk<8;kk++)
	{
		if((cmd&0x80)==0x80) 
            Set_SDI(1);
		else         
            Set_SDI(0);
		Set_SCL(1);
		Set_SCL(0);
		cmd = cmd<<1;	
	}
}

void SPI_SET_PAs(unsigned char value)
{
	unsigned long kk;

	Set_SDI(1);			//Set DC=1, for writing to Data register
	Set_SCL(0);
	Set_SCL(1);
	
	Set_SCL(0);
	for(kk=0;kk<8;kk++)
	{
		if((value&0x80)==0x80) Set_SDI(1);
		else         Set_SDI(0);
		Set_SCL(1);
		Set_SCL(0);
		value = value<<1;	
	}	
}

void SPI_2828_WrCmd(unsigned char cmd)
{
	unsigned long kk;
	
	Set_2828_CS(0);
	
	Set_SDI(0);			//Set DC=0, for writing to Command register
	Set_SCL(0);
	Set_SCL(1);

	Set_SCL(0);
	for(kk=0;kk<8;kk++)
	{
		if((cmd&0x80)==0x80) Set_SDI(1);
		else         Set_SDI(0);
		Set_SCL(1);
		Set_SCL(0);
		cmd = cmd<<1;	
	}
	
	Set_2828_CS(1);	
}

//void SPI_2828_WrCmd(unsigned char)
void SPI_2828_WrReg(unsigned char c,unsigned short value)
{
	Set_2828_CS(0);
	SPI_SET_Cmd(c);
	SPI_SET_PAs(value&0xff);
	SPI_SET_PAs((value>>8)&0xff);	
	Set_2828_CS(1);	

    #if 0
    {
        unsigned short regval=0;
        if( (c != 0xBF || c != 0xFF)) {
        SPI_2828_RdReg(c, &regval); 
        RTNA_DBG_Str0("Reg: ");
        RTNA_DBG_Byte0(c);
        RTNA_DBG_Str0(" ");
        RTNA_DBG_Short0(regval);
        RTNA_DBG_Str0("\r\n");
        }
    }
    #endif
}

void SPI_2828_RdReg(unsigned char cmd, unsigned short *reg_val)
{
    unsigned long kk;
    unsigned char retval;
    //cmd= 0xB1;
    //SPI_2828_WrReg(0xD4, 0xFA);
	Set_2828_CS(0);
    //SPI_2828_WrReg(0xD4, 0xFA);
    Set_SDI(0);			//Set DC=0, for writing to Command register
	Set_SCL(0);
    Delay_us(1);
	Set_SCL(1);	
    Delay_us(1);
	Set_SCL(0);
    Delay_us(1);
	for(kk=0;kk<8;kk++)
	{
		if((cmd&0x80)==0x80) Set_SDI(1);
		else         Set_SDI(0);
		Set_SCL(1);
        Delay_us(1);
		Set_SCL(0);
        Delay_us(1);
		cmd = cmd<<1;	
	}
    cmd=0xFA;           // stored value in 0xD4 
    Set_SDI(0);			//Set DC=0, for writing to Command register
    Delay_us(1);
	Set_SCL(0);
    Delay_us(1);
	Set_SCL(1);	
    Delay_us(1);
	Set_SCL(0);
    Delay_us(1);
	for(kk=0;kk<8;kk++)
	{
		if((cmd&0x80)==0x80) 
            Set_SDI(1);
		else         
                Set_SDI(0);
		Set_SCL(1);
        Delay_us(1);
		Set_SCL(0);
        Delay_us(1);
		cmd = cmd<<1;	
	}
    *reg_val = 0;
    for(kk=0;kk<8;kk++)
	{
        
		Set_SCL(1);
        Delay_us(1);
        MMPF_PIO_GetData(LSDI_GPIO_PIN, &retval);
        *reg_val |= retval<<(7-kk);
		Set_SCL(0);
        Delay_us(1);	
	}

    for(kk=0;kk<8;kk++)
	{        
		Set_SCL(1);
        Delay_us(1);
        MMPF_PIO_GetData(LSDI_GPIO_PIN, &retval);
        *reg_val |= retval<<(15-kk);
		Set_SCL(0);
        Delay_us(1);
		cmd = cmd<<1;	
	}
   Set_2828_CS(1); 
}


void SPI_2828_Read_Burst(unsigned char cmd, unsigned short bytenum)
{
    unsigned long kk;
    unsigned long i=10;
    unsigned char retval;
    unsigned short ret_val;

	Set_2828_CS(0);
    //send 0xFF with DC bit =0
    Set_SDI(0);			//Set DC=0, for writing to Command register
	Set_SCL(0);
    Delay_us(1);
	Set_SCL(1);	
    Delay_us(1);
	Set_SCL(0);
    Delay_us(1);
	for(kk=0;kk<8;kk++)
	{
		if((cmd&0x80)==0x80) Set_SDI(1);
		else         Set_SDI(0);
		Set_SCL(1);
        Delay_us(1);
		Set_SCL(0);
        Delay_us(1);
		cmd = cmd<<1;	
	}
    
    //repeat send 0xFA before read data from SDO
    for(i=0; i< bytenum>>1 ; i++) {
        cmd=0xFA;
        Set_SDI(0);			//Set DC=0, for writing to Command register
        Delay_us(1);
	    Set_SCL(0);
        Delay_us(1);
	    Set_SCL(1);	
        Delay_us(1);
	    Set_SCL(0);
        Delay_us(1);
	    for(kk=0;kk<8;kk++)
	    {
		    if((cmd&0x80)==0x80) 
                Set_SDI(1);
		    else         
                Set_SDI(0);
		    Set_SCL(1);
            Delay_us(1);
		    Set_SCL(0);
            Delay_us(1);
		    cmd = cmd<<1;	
	    }
        ret_val = 0;
        for(kk=0;kk<8;kk++)
	    {
		    Set_SCL(1);
            Delay_us(1);
            MMPF_PIO_GetData(LSDI_GPIO_PIN, &retval);
            ret_val |= retval<<(7-kk);
		    Set_SCL(0);
            Delay_us(1);
	    }
        for(kk=0;kk<8;kk++)
	    {
		    Set_SCL(1);
            Delay_us(1);
            MMPF_PIO_GetData(LSDI_GPIO_PIN, &retval);
            ret_val |= retval<<(15-kk);
		    Set_SCL(0);
            Delay_us(1);
	    }
        #if 0
        RTNA_DBG_Str0("param");
        RTNA_DBG_Byte0(i*2);
        RTNA_DBG_Str0(":");
        RTNA_DBG_Short0(ret_val & 0xff);
        RTNA_DBG_Str0("\r\n");
        RTNA_DBG_Str0("param");
        RTNA_DBG_Byte0(i*2+1);
        RTNA_DBG_Str0(":");
        RTNA_DBG_Short0((ret_val>>8) & 0xff);
        RTNA_DBG_Str0("\r\n"); 
        #endif   
    }
    if(bytenum & 0x01) {
        cmd=0xFA;
        Set_SDI(0);			//Set DC=0, for writing to Command register
        Delay_us(1);
	    Set_SCL(0);
        Delay_us(1);
	    Set_SCL(1);	
        Delay_us(1);
	    Set_SCL(0);
        Delay_us(1);
	    for(kk=0;kk<8;kk++)
	    {
		    if((cmd&0x80)==0x80) 
                Set_SDI(1);
		    else         
                Set_SDI(0);
		    Set_SCL(1);
            Delay_us(1);
		    Set_SCL(0);
            Delay_us(1);
		    cmd = cmd<<1;	
	    }
        ret_val = 0;
        for(kk=0;kk<8;kk++)
	    {
		    Set_SCL(1);
            Delay_us(1);
            MMPF_PIO_GetData(LSDI_GPIO_PIN, &retval);
            ret_val |= retval<<(7-kk);
		    Set_SCL(0);
            Delay_us(1);
		    //cmd = cmd<<1;	
	    }
        for(kk=0;kk<8;kk++)
	    {
		    Set_SCL(1);
            Delay_us(1);
            MMPF_PIO_GetData(LSDI_GPIO_PIN, &retval);
            ret_val |= retval<<(15-kk);
		    Set_SCL(0);
            Delay_us(1);
		//cmd = cmd<<1;	
	    }
        #if 0
        RTNA_DBG_Str0("param");
        RTNA_DBG_Byte0(bytenum);
        RTNA_DBG_Str0(":");
        RTNA_DBG_Short0(ret_val & 0xff);
        RTNA_DBG_Str0("\r\n"); 
        #endif         
    }
	Set_2828_CS(1);	
  
}

void SSD2828_Send_Read_Dsi_Packet(unsigned char reg, unsigned char bytnum)
{
    //SPI_2828_WrReg(0xb7,0x0782);

    unsigned short cfg = 0;
    cfg = CFGR_LONG_PKG_EN | CFGR_EOT_PKG_EN | CFGR_ECC_CRC_CHK_EN | CFGR_DCS_PKG_EN;
    cfg |= CFGR_READ_EN;
    cfg |= CFGR_CLK_LANE_EN;
    cfg &= ~CFGR_HS_MODE;
    
    SPI_2828_WrReg(0xb7,cfg); //0x07C2

    #if 0
    RTNA_DBG_Str0("reg: ");
    RTNA_DBG_Short0(reg);
    RTNA_DBG_Str0(" \r\n");
    #endif
    SPI_2828_WrReg(SSD2828_OCR,0x0001);
    SPI_2828_WrReg(SSD2828_CCR,0x0008);
    SPI_2828_WrReg(SSD2828_MRSR,bytnum);
    //SPI_2828_WrReg(0xc0,0x0100);
    /* Set the payload size (set 1 because only the DCS command itself) */
    SPI_2828_WrReg(SSD2828_PSCR1,1); //send DCS short packet, data type:0x06
    SPI_2828_WrReg(SSD2828_PDR,reg);

    #if 0
    {
    unsigned short ret_val;
    SPI_2828_RdReg(SSD2828_ISR, &ret_val);
    RTNA_DBG_Str0("ISR CMD:");
    RTNA_DBG_Byte0(SSD2828_ISR);
    RTNA_DBG_Str0(" ");
    
    SSD2828_ACK_ResStaus(ret_val);
    RTNA_DBG_Str0("ret: ");
    RTNA_DBG_Short0(ret_val);
    RTNA_DBG_Str0(" \r\n");

    SPI_2828_RdReg(SSD2828_RDCR, &ret_val);
    RTNA_DBG_Str0("return byte num: ");
    RTNA_DBG_Short0(ret_val);
    RTNA_DBG_Str0(" \r\n");
    }
    #endif
    SPI_2828_Read_Burst(SSD2828_RR,bytnum);

}
/*Read register from driver IC */
void SSD2828_Send_Write_Dsi_Packet(char *ParamList)
{
    unsigned short cfg = 0;
    unsigned short cnt = 0;
    //unsigned short ret_val;
    unsigned char  reg;
    unsigned short restore;
    unsigned char  paramcnt;
    char  *pParam=ParamList;

    reg=*pParam;
    pParam++;
    SPI_2828_RdReg(0xb7, &restore); //0x07C2

    SSD2828_Send_Read_Dsi_Packet(reg, ParamList[1]);
    cfg = CFGR_LONG_PKG_EN | CFGR_EOT_PKG_EN | CFGR_ECC_CRC_CHK_EN | CFGR_DCS_PKG_EN;
    cfg &= ~CFGR_READ_EN;
    cfg |= CFGR_CLK_LANE_EN;
    cfg &= ~CFGR_HS_MODE;
    SPI_2828_WrReg(0xb7, cfg); //0x07C2
	SPI_2828_WrReg(0xb8, 0x0000);
    SPI_2828_WrReg(0xc0, 0x0100);

    SPI_2828_WrReg(SSD2828_LCR,1); 
    SPI_2828_WrReg(SSD2828_OCR,0x0001); //cancel the transimission
    SPI_2828_WrReg(SSD2828_CCR,0x0008); //set clock divider of LP mode clock rate
    //SPI_2828_WrReg(SSD2828_MRSR,4);
    //SPI_2828_WrReg(0xc0,0x0100);
    paramcnt=*pParam;
    SPI_2828_WrReg(SSD2828_PSCR1,(paramcnt+1)); //params cnt + 1 

    RTNA_DBG_Byte0(reg);
    Set_2828_CS(0);
    SPI_SET_Cmd(SSD2828_PDR);
    Set_2828_CS(1);
    Set_2828_CS(0);
    SPI_SET_PAs(reg);
    Set_2828_CS(1);
    pParam++;
    if(paramcnt > 0) {
        for(cnt=0; cnt< paramcnt ; cnt++) {
            Set_2828_CS(0);
            SPI_SET_PAs(*pParam);
            Set_2828_CS(1);
            //RTNA_DBG_Byte0(*pParam);
            pParam++;
        }
    }
    //SPI_2828_WrReg(0xbF, 0x00A3);
    SSD2828_Send_Read_Dsi_Packet(reg, ParamList[1]);
    SPI_2828_WrReg(0xb7, restore); 
    #if 0
    SPI_2828_RdReg(SSD2828_ISR, &ret_val);
    RTNA_DBG_Str0("ISR CMD:");
    RTNA_DBG_Byte0(SSD2828_ISR);
    RTNA_DBG_Str0(" ");
    
    SSD2828_ACK_ResStaus(ret_val);
    RTNA_DBG_Str0("ACK ret: ");
    RTNA_DBG_Short0(ret_val);
    RTNA_DBG_Str0(" \r\n");
    #endif
}


#define TX_CLK 24  //8~30MHz need check by external crystal
int SSD2828_Config_PLL(unsigned long divider, unsigned long multiplier)
{
    unsigned char config_pll=0;
    unsigned long reference_freq;
    if(divider > 31)
        return -1;
    if(multiplier > 255)
        return -1;

    config_pll = (divider & 0xff) << 8;
    config_pll |= multiplier;
    reference_freq = TX_CLK*multiplier/divider;
    if(reference_freq < 125)
        config_pll |= 0x00<<14;
    else if(reference_freq < 250)
        config_pll |= 0x01<<14;
    else if(reference_freq < 500)
        config_pll |= 0x10<<14;
    else if(reference_freq < 1000)
        config_pll |= 0x11<<14;
	SPI_2828_WrReg(SSD2828_PLCR, config_pll);	
    return 1;
}


void SSD2828_ACK_ResStaus(unsigned short dsi_ack_response) 
{

    RTNA_DBG_Str0("DSI ACK: ");
    RTNA_DBG_Short0(dsi_ack_response);
    RTNA_DBG_Str0(" \r\n");
    if(dsi_ack_response & ISR_BTAR) {
        RTNA_DBG_Str0("MIPI Slave pass lane authority back\r\n");
    }else {
        RTNA_DBG_Str0("MIPI Slave NOT pass lane authority back\r\n");
    }
    if(dsi_ack_response & ISR_ARR) {
        RTNA_DBG_Str0("ACK response has been received\r\n");
    }else {
        RTNA_DBG_Str0("ACK response has not been received\r\n");
    }

    if(dsi_ack_response & ISR_ATR) {
        RTNA_DBG_Str0("ACK Trigger response has been received\r\n");
    }else {
        RTNA_DBG_Str0("ACK Trigger response has NOT been received\r\n");
    }
    if(dsi_ack_response & 0x01) {
        RTNA_DBG_Str0("Read data ready\r\n");
    }else {
        RTNA_DBG_Str0("Read data NOT ready\r\n");
    }
}

/*  Read clock lane and data lane status
    Read PLL lock status
*/
void SSD2828_Read_Status()
{
    unsigned short status;
    SPI_2828_RdReg(SSD2828_ISR, &status);		

    RTNA_DBG_Str0("Clock LANE in LP-11 state: ");
    if(status & ISR_CLK_LANE_STATUS) {
        RTNA_DBG_Str0("Yes");  
    }
    else {
        RTNA_DBG_Str0("No"); 
    }
    RTNA_DBG_Str0("\r\n");

    RTNA_DBG_Str0("Data LANE in LP-11 state: ");
    if(status & ISR_DATA_LANE_STATUS) {
        RTNA_DBG_Str0("Yes"); 
    }
    else {
        RTNA_DBG_Str0("No"); 
    }
    RTNA_DBG_Str0("\r\n");

    RTNA_DBG_Str0("PLL already lock : ");
    if((status & ISR_PLL_LOCK_STATUS)) {
        RTNA_DBG_Str0("Yes");
    }
    else {
        RTNA_DBG_Str0("No"); 
    }
    RTNA_DBG_Str0("\r\n");
   return;
}

#endif
