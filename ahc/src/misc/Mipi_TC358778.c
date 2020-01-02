
#include "Customer_config.h" // CarDV

#include "mmpf_pio.h"
#include "Mipi_TC358778.h"

#define PANLE_BACKLIGHT	(MMP_GPIO_MAX)
#define SDA_GPIO_PIN    (MMP_GPIO123)//PLCD_FLM----F1
#define SCL_GPIO_PIN    (MMP_GPIO122)//PLCD2_CS_N
//#define RST_GPIO_PIN    (MMP_GPIO19)//BGPIO9
//#define RST_GPIO_PIN    (MMP_GPIO63)//CGPIO31----TP24----B15
//#define RST_GPIO_PIN    (MMP_GPIO6)//AGPIO4---- long
#define TC358778LCD_CS	(MMP_GPIO124)//PLCD_GPIO ----C6
#define IIC_SLAVE_ADDR  (0x1c)

#define Delay_us(n)   MMPC_System_WaitUs(n*100/27)	//(lcm_util.udelay(n))
#define Delay_ms(n)   MMPC_System_WaitMs(n*100/27)	//(lcm_util.mdelay(n))

static void IIC_SET_SCL(MMP_UBYTE value)
{
    MMPF_PIO_SetData(SCL_GPIO_PIN, value, MMP_TRUE);
}

static void IIC_SET_SDA(MMP_UBYTE value)
{
    MMPF_PIO_EnableOutputMode(SDA_GPIO_PIN, MMP_TRUE, MMP_TRUE);
    MMPF_PIO_SetData(SDA_GPIO_PIN, value, MMP_TRUE);
}
static void sccb_start(void)
{
	IIC_SET_SCL(GPIO_HIGH);//gpio_write_io(pSccb->scl_port, DATA_HIGH);
    
	IIC_SET_SDA(GPIO_HIGH);//gpio_write_io(pSccb->sda_port, DATA_HIGH);
	Delay_us(5);
	IIC_SET_SDA(GPIO_LOW);//gpio_write_io(pSccb->sda_port, DATA_LOW);
	Delay_us(5);
	IIC_SET_SCL(GPIO_LOW);//gpio_write_io(pSccb->scl_port, DATA_LOW);
	Delay_us(5);
}

static void sccb_stop(void)
{
	IIC_SET_SCL(GPIO_LOW);//gpio_write_io(pSccb->scl_port, DATA_LOW);
	IIC_SET_SDA(GPIO_LOW);//gpio_write_io(pSccb->sda_port, DATA_LOW);
	Delay_us(5);
	IIC_SET_SCL(GPIO_HIGH);//gpio_write_io(pSccb->scl_port, DATA_HIGH);
	Delay_us(5);
	IIC_SET_SDA(GPIO_HIGH);//gpio_write_io(pSccb->sda_port, DATA_HIGH);
	Delay_us(5);
}

int sccb_w_phase(MMP_USHORT value, MMP_UBYTE ack)
{
	int i;
	int ret = 1;

	for(i=0;i<8;i++){
		IIC_SET_SCL(GPIO_LOW);//gpio_write_io(pSccb->scl_port, DATA_LOW);
		if(value & 0x80) {
			IIC_SET_SDA(GPIO_HIGH);//gpio_write_io(pSccb->sda_port, DATA_HIGH);
		} 
		else {
			IIC_SET_SDA(GPIO_LOW);//gpio_write_io(pSccb->sda_port, DATA_LOW);
		}

		Delay_us(5);
		IIC_SET_SCL(GPIO_HIGH);//gpio_write_io(pSccb->scl_port, DATA_HIGH);
		Delay_us(5);
		value <<= 1;
	}

	// The 9th bit transmission
	IIC_SET_SCL(GPIO_LOW);//gpio_write_io(pSccb->scl_port, DATA_LOW);
	//gpio_init_io(pSccb->sda_port, GPIO_FLOAT);	//SDA is Hi-Z mode
	MMPF_PIO_EnableOutputMode(SDA_GPIO_PIN, MMP_FALSE, MMP_TRUE);
	Delay_us(5);
	IIC_SET_SCL(GPIO_HIGH);//gpio_write_io(pSccb->scl_port, DATA_HIGH);

	// check ack = low
	if(ack) {
        MMP_UBYTE io_data;
		for(i=0; i<5; i++) {
            MMPF_PIO_GetData(SDA_GPIO_PIN, &io_data);
			if(io_data == 0) {
                //RTNA_DBG_Str0("ack 1\r\n");
				break;
			}
		}
	}

	if(i == 5) {
        RTNA_DBG_Str0("NOT ack 0\r\n");
		ret = 0;
	}

	Delay_us(5);
	IIC_SET_SCL(GPIO_LOW);//gpio_write_io(pSccb->scl_port, DATA_LOW);
	//gpio_init_io(pSccb->sda_port, GPIO_OUTPUT);	//SDA is Hi-Z mode
	MMPF_PIO_EnableOutputMode(SDA_GPIO_PIN, MMP_TRUE, MMP_TRUE);
	return ret;
}

MMP_USHORT sccb_r_phase(MMP_UBYTE phase)
{
	MMP_USHORT i;
	MMP_USHORT data = 0x00;
    MMP_UBYTE value;

	//gpio_init_io(pSccb->sda_port, GPIO_INPUT);	//SDA is Hi-Z mode
	MMPF_PIO_EnableOutputMode(SDA_GPIO_PIN, MMP_FALSE, MMP_TRUE);
	for(i=0;i<8;i++) {
		IIC_SET_SCL(GPIO_LOW);//gpio_write_io(pSccb->scl_port, DATA_LOW);
		Delay_us(5);
		IIC_SET_SCL(GPIO_HIGH);//gpio_write_io(pSccb->scl_port, DATA_HIGH);
		data <<= 1;
        MMPF_PIO_GetData(SDA_GPIO_PIN, &value);
		data |=(value);//( gpio_read_io(pSccb->sda_port));
		Delay_us(5);
	}

	// The 9th bit transmission
	IIC_SET_SCL(GPIO_LOW);//gpio_write_io(pSccb->scl_port, DATA_LOW);
	//gpio_init_io(pSccb->sda_port, GPIO_OUTPUT);	//SDA is output mode
	MMPF_PIO_EnableOutputMode(SDA_GPIO_PIN, MMP_TRUE, MMP_TRUE);
	if(phase) {
		IIC_SET_SDA(GPIO_LOW);//gpio_write_io(pSccb->sda_port, DATA_LOW);       //SDA0, the nineth bit is ACK must be 1
	} 
	else {
		IIC_SET_SDA(GPIO_HIGH);//gpio_write_io(pSccb->sda_port, DATA_HIGH);	//SDA0, the nineth bit is NAK must be 1
	}

	Delay_us(5);
	IIC_SET_SCL(GPIO_HIGH);//gpio_write_io(pSccb->scl_port, DATA_HIGH);
	Delay_us(5);
	IIC_SET_SCL(GPIO_LOW);//gpio_write_io(pSccb->scl_port, DATA_LOW);
	return data;
}

MMP_LONG IIC_TC358778_WrData(MMP_ULONG data)
{
	MMP_UBYTE temp,i;
	MMP_LONG ret;
	MMP_UBYTE data_bits = 32;
	MMP_UBYTE ack = 1;

	// 3-Phase write transmission
	// Transmission start
	sccb_start();

	// Phase 1, SLAVE_ID
	ret = sccb_w_phase(IIC_SLAVE_ADDR, 1);
	if(ret < 0) {
        RTNA_DBG_Str0("sccb_w_phase err\r\n!");
		goto __exit;
	}
    //RTNA_DBG_Long0(data);
	while(data_bits >= 8) {
    	data_bits -= 8;
    	if(data_bits) {
    		ack = 1; //ACK
    	} else {
    		ack = 0; //NACK
    	}

    	temp = (data >> data_bits)&0xff;
        //RTNA_DBG_Byte0(temp);
    	ret = sccb_w_phase(temp, ack);
    	if(ret < 0) {
            RTNA_DBG_Str0("sccb_w_phase err 1\r\n!");
    		goto __exit;
    	}
	}
__exit:
    //RTNA_DBG_Str0("\r\n");
	// Transmission stop
	sccb_stop();
	return ret;
}

MMP_LONG IIC_TC358778_RdData(MMP_USHORT sub_addr, MMP_USHORT* value)
{
	MMP_UBYTE temp;
	MMP_UBYTE id = IIC_SLAVE_ADDR;
	MMP_UBYTE addr_bits = 16;
	MMP_UBYTE data_bits = 16;
	MMP_UBYTE ack = 1;
	MMP_USHORT addr = sub_addr;
	INT32U read_data;
	MMP_LONG ret;

	// 2-Phase write transmission cycle
	// Transmission start
	sccb_start();

	// Phase 1, SLAVE_ID
	ret = sccb_w_phase(id, 1);
	if(ret < 0) {
        RTNA_DBG_Str0("r err 1\r\n!");
		goto __exit;
	}

	// Phase 2, Register Address
	while(addr_bits >= 8) {
		addr_bits -= 8;
		temp = addr >> addr_bits;
		ret = sccb_w_phase(temp, ack);
		if(ret < 0) {
            RTNA_DBG_Str0("r err 2\r\n!");
			goto __exit;
		}
	}

	// Transmission stop
	sccb_stop();

	// 2-Phase read transmission cycle
	// Transmission start
	sccb_start();

	// Phase 1 (read)
	ret = sccb_w_phase(id | 0x01, 1);
	if(ret < 0) {
        RTNA_DBG_Str0("r err 3\r\n!");
		goto __exit;
	}

	// Phase 2 (read)
	read_data = 0;
	while(data_bits >= 8) {
		data_bits -= 8;
		if(data_bits) {
			ack = 1; //ACK
		} else {
			ack = 0; //NACK
		}

		temp = sccb_r_phase(ack);
		read_data <<= 8;
		read_data |= temp;
	}
	*value = read_data;

__exit:
	// Transmission stop
	sccb_stop();
    #if 1
        RTNA_DBG_Short0(sub_addr);
        RTNA_DBG_Str0(" = 0");
        RTNA_DBG_Short0(read_data);
        RTNA_DBG_Str0("\r\n");
    #endif
	return ret;
}

///added by eric 
#if 1
void TC358778_Send_Long_Dsi_Packet(char *ParamList)
{
	unsigned short cnt = 0;
	unsigned short TmpCmd = 0;
	unsigned short paramcnt;
	unsigned short paramcnt_l = 0;

	char *pParam = ParamList;

	IIC_TC358778_WrData(0x00080001);
	IIC_TC358778_WrData(0x00500029);

    paramcnt = *pParam;

#if (PANEL_USE == PANEL_TG078UW019A0_EK79030_400x1280_MIPI)
	if(*(pParam + 1) == 0x53)
	{
		paramcnt = 19;
		paramcnt_l = 10;
		
		RTNA_DBG_Str(0, "got cnt 53 param");
		RTNA_DBG_Str(0, "\r\n");		
    }
	else if(*(pParam + 1) == 0x54)
	{
		paramcnt = 19;
		paramcnt_l = 10;	

		RTNA_DBG_Str(0, "got cnt 54 param");
		RTNA_DBG_Str(0, "\r\n");		
	}	
	else if(*(pParam + 1) == 0x55)
	{		
		paramcnt = 8;
		paramcnt_l = 5;	

		RTNA_DBG_Str(0, "got cnt 55 param");
		RTNA_DBG_Str(0, "\r\n");		
	}	
	else if(*(pParam + 1) == 0x56)
	{		
		paramcnt = 16;
		paramcnt_l = 9;	

		RTNA_DBG_Str(0, "got cnt 56 param");
		RTNA_DBG_Str(0, "\r\n");		
	}
	
#elif (PANEL_USE == PANEL_S8500L0_EK79202_1280x320_MIPI)
	if(*(pParam + 1) == 0x52)
	{
		paramcnt = 23;
		paramcnt_l = 12;	

		RTNA_DBG_Str(0, "got cnt 52 param");
		RTNA_DBG_Str(0, "\r\n");	
	}
    else if(*(pParam + 1) == 0x59)
	{
		paramcnt = 23;
		paramcnt_l = 12;
		
		RTNA_DBG_Str(0, "got cnt 59 param");
		RTNA_DBG_Str(0, "\r\n");
	}

	else if(*(pParam + 1) == 0x55)
	{
		paramcnt =	13;
		paramcnt_l = 8;	

		RTNA_DBG_Str(0,	"got cnt 55 param");
		RTNA_DBG_Str(0, "\r\n");
	}
	else if(*(pParam+1)==0x53)
	{
	    paramcnt = 14;
	    paramcnt_l = 8;
		
	    RTNA_DBG_Str(0,	"got cnt 53 param");
	    RTNA_DBG_Str(0, "\r\n");
    }
	else if(*(pParam + 1) == 0x54)
	{
        paramcnt = 14;
        paramcnt_l = 8;	
		 
        RTNA_DBG_Str(0,"got cnt 54 param");
        RTNA_DBG_Str(0, "\r\n");
	}
	
#elif (PANEL_USE == PANEL_V920TW01A_EK79202_1280x320_MIPI)
	if(*(pParam + 1) == 0x52)
	{
		paramcnt = 23;
		paramcnt_l = 12;	

		RTNA_DBG_Str(0, "got cnt 52 param");
		RTNA_DBG_Str(0, "\r\n");	
	}
    else if(*(pParam + 1) == 0x59)
	{
		paramcnt = 23;
		paramcnt_l = 12;
		
		RTNA_DBG_Str(0, "got cnt 59 param");
		RTNA_DBG_Str(0, "\r\n");
	}

	else if(*(pParam + 1) == 0x55)
	{
		paramcnt =	13;
		paramcnt_l = 8;	

		RTNA_DBG_Str(0,	"got cnt 55 param");
		RTNA_DBG_Str(0, "\r\n");
	}
	else if(*(pParam+1)==0x53)
	{
	    paramcnt = 14;
	    paramcnt_l = 8;
		
	    RTNA_DBG_Str(0,	"got cnt 53 param");
	    RTNA_DBG_Str(0, "\r\n");
    }
	else if(*(pParam + 1) == 0x54)
	{
        paramcnt = 14;
        paramcnt_l = 8;	
		 
        RTNA_DBG_Str(0,"got cnt 54 param");
        RTNA_DBG_Str(0, "\r\n");
	}
#endif
	
	IIC_TC358778_WrData(0x00220000 | paramcnt);
	IIC_TC358778_WrData(0x00e08000);
	
    pParam++;
    if(paramcnt > 0) 
	{
        for(cnt = 0; cnt < paramcnt_l ; cnt++) 
		{
			TmpCmd = 0;
			TmpCmd |= (*pParam);
			pParam++;
			TmpCmd |= (*pParam) << 8;
			pParam++;
			IIC_TC358778_WrData(0x00e80000 | TmpCmd);
     
	        RTNA_DBG_Str(0, "duncan lcd init");
       		RTNA_DBG_Short(0, TmpCmd);	
		 	RTNA_DBG_Str(0, "\r\n");	
        }	
    }
	
	IIC_TC358778_WrData(0x00e0e000);
	Delay_us(1000);
	IIC_TC358778_WrData(0x00e02000);
	IIC_TC358778_WrData(0x00e00000);
	Delay_us(10);

   RTNA_DBG_Str(0, "finish writing a long command");
   RTNA_DBG_Str(0, "\r\n");	
}
#endif

///added by eric 
#if 0
void TC358778_Send_Long_Dsi_Packet(char *ParamList)
{
    unsigned short cnt = 0;
    unsigned short TmpCmd = 0;
    unsigned short  paramcnt;
    unsigned short  paramcnt_l=0;
	
    char  *pParam=ParamList;
	
	IIC_TC358778_WrData(0x00080001);
	IIC_TC358778_WrData(0x00500039);

    paramcnt=*pParam;

	if(*(pParam+1)==0x53 ){

     paramcnt  =20;
	 paramcnt_l=10;	

	 RTNA_DBG_Str(0,"got cnt 53 param");
     RTNA_DBG_Str(0, "\r\n");
	
	}

    else  if(*(pParam+1)==0x54 ){

	    RTNA_DBG_Str(0,"got cnt 54 param");
	    RTNA_DBG_Str(0, "\r\n");
		
	    paramcnt =20;
	    paramcnt_l=10;	
	}

	else  if(*(pParam+1)==0x55 ){
		
	    RTNA_DBG_Str(0,"got cnt 55 param");
	    RTNA_DBG_Str(0, "\r\n");
		
	    paramcnt =9;
	    paramcnt_l=6;	
	}

	else  if(*(pParam+1)==0x56 ){

	    RTNA_DBG_Str(0,"got cnt 56 param");
	    RTNA_DBG_Str(0, "\r\n");
		
	    paramcnt =17;
	    paramcnt_l=10;	
    }
	else if(*(pParam+1)==0x57 ){

        RTNA_DBG_Str(0,"got cnt 57 param");
        RTNA_DBG_Str(0, "\r\n");
		
        paramcnt =5;
        paramcnt_l=4;	
	}

	
	IIC_TC358778_WrData(0x00220000 | paramcnt);
	
	IIC_TC358778_WrData(0x00e08000);
	
    pParam++;
    if(paramcnt > 0) {
        for(cnt=0; cnt< paramcnt_l ; cnt++) 
		{

		    TmpCmd = 0;
			TmpCmd |= (*pParam);
			pParam++;
			TmpCmd |= (*pParam) << 8;
			pParam++;
			IIC_TC358778_WrData(0x00e80000 | TmpCmd);
	        RTNA_DBG_Str(0,"duncan lcd init");
            RTNA_DBG_Short(0, TmpCmd);	
		    RTNA_DBG_Str(0, "\r\n");
			
        }	
    }
	IIC_TC358778_WrData(0x00e0e000);
	Delay_us(1000);
	IIC_TC358778_WrData(0x00e02000);
	IIC_TC358778_WrData(0x00e00000);
	Delay_us(10);

	RTNA_DBG_Str(0,"finish writing a long command");
	RTNA_DBG_Str(0, "\r\n");
}
#endif



void TC358778_Send_Short_Dsi_Packet(char addr, char data)
{
	IIC_TC358778_WrData(0x06021015);
	IIC_TC358778_WrData(0x06040000);
	IIC_TC358778_WrData(0x06100000 | (data<<8) | addr);
	IIC_TC358778_WrData(0x06000001);
    
}
void IIC_TC358778_InitGpio(void)
{
	//initial status 
	MMPF_PIO_SetData(SDA_GPIO_PIN, GPIO_HIGH, MMP_TRUE);
	MMPF_PIO_SetData(SCL_GPIO_PIN, GPIO_HIGH, MMP_TRUE);

	//setting output
	MMPF_PIO_EnableOutputMode(SDA_GPIO_PIN, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_EnableOutputMode(SCL_GPIO_PIN, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_EnableOutputMode(TC358778LCD_CS, MMP_TRUE, MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(PANLE_BACKLIGHT, MMP_TRUE, MMP_TRUE);

	//cs disable
	//MMPF_PIO_SetData(TC358778LCD_CS, GPIO_HIGH, MMP_TRUE);
	MMPF_PIO_SetData(TC358778LCD_CS, GPIO_LOW, MMP_TRUE);
	
	//ctrl reset pin
	MMPF_PIO_EnableOutputMode(RST_GPIO_PIN, MMP_TRUE, MMP_TRUE);
	MMPF_PIO_SetData(RST_GPIO_PIN, GPIO_HIGH, MMP_TRUE);
	MMPF_OS_Sleep_MS(500);
	MMPF_PIO_SetData(RST_GPIO_PIN, GPIO_LOW, MMP_TRUE);
	MMPF_OS_Sleep_MS(50);
	MMPF_PIO_SetData(RST_GPIO_PIN, GPIO_HIGH, MMP_TRUE);
	MMPF_OS_Sleep_MS(10);

	//initial status 
    //MMPF_PIO_SetData(SDA_GPIO_PIN, GPIO_HIGH, MMP_TRUE);
    //MMPF_PIO_SetData(SCL_GPIO_PIN, GPIO_HIGH, MMP_TRUE);
	//MMPF_PIO_SetData(TC358778LCD_CS, GPIO_LOW, MMP_TRUE);

	//Turn on bl
	//MMPF_PIO_SetData(PANLE_BACKLIGHT, GPIO_HIGH, MMP_TRUE);
}

void Dcs_Write_2P_XXX(MMP_BYTE cmd1, MMP_BYTE parm1)
{
	MMP_USHORT regdata;
	MMP_ULONG value;

	regdata=parm1;
	regdata=regdata<<8;
	regdata|=cmd1 ;

	value = 0x0610;
	value <<= 16;
	value |= regdata;

	IIC_TC358778_WrData(0x06021015); // 
	IIC_TC358778_WrData(0x06040000); // 
	IIC_TC358778_WrData(value); // 
	IIC_TC358778_WrData(0x06000001); //
	//delay1us(100);
	//delayms(delay);
}

