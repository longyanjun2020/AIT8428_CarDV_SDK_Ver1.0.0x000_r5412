#include "mmp_gpio_inc.h"

#include "mmpf_pio.h"
#include "mmp_err.h"
#include "mmps_pio.h"
#include "AHC_OS.h"

#include "lib_retina.h"




static  void SPI_DELAY(void)
{	
	RTNA_WAIT_US(10);	
}


static void SPI_CLK_OUT(void)
{
	 MMPF_PIO_EnableOutputMode(MMP_GPIO16, 1, MMP_TRUE);// clk

}

static void SPI_CLK_set(MMP_UBYTE Value)
{
	
	MMPF_PIO_SetData(MMP_GPIO16, Value, MMP_TRUE);//clk

}





static void spi_SDA_IN(void)
{
	AHC_GPIO_SetOutputMode(MMP_GPIO19,MMP_FALSE);

}

static void spi_SDA_OUT(void)
{

	 AHC_GPIO_SetOutputMode(MMP_GPIO18,MMP_TRUE);
}

static void spi_SDA_L(void)
{

	MMPS_PIO_SetData(MMP_GPIO18, 0);
}

static void spi_SDA_H(void)
{
	MMPS_PIO_SetData(MMP_GPIO18, 1);
}


unsigned char SPI_MISO_EX(void)
{
	MMP_UBYTE returnValue;
	MMPF_PIO_GetData(MMP_GPIO19, &returnValue);
	return returnValue;
}
void SPI_CS(MMP_UBYTE Value)
{
	 MMPF_PIO_EnableOutputMode(MMP_GPIO17, 1, MMP_TRUE);// cs
	MMPF_PIO_SetData(MMP_GPIO17, Value, MMP_TRUE);//cs
	//return returnValue;
}





//CPOL: Low
//CPHA: 1Edge
MMP_UBYTE _SPI_WriterByte(MMP_UBYTE dat)
{	
	MMP_UBYTE i;
	MMP_UBYTE rx_data = 0;
	SPI_CS(0);
	spi_SDA_OUT();
	SPI_CLK_OUT();
	
	for(i = 0;i < 8;i++)
	{

		SPI_CLK_set(1); // read

		SPI_DELAY();
		rx_data <<= 1;
		if(SPI_MISO_EX())
		{
			rx_data |= 1;
		}

		SPI_DELAY();
		
		SPI_CLK_set(0); // write

		SPI_DELAY();
		
		if(dat&0x80)
		{
			spi_SDA_H();
		}
		else	spi_SDA_L();
		dat <<= 1;
	}

	SPI_CS(1);
	
	return rx_data;
}


MMP_UBYTE SPI_READ_BYTE(void)
{
	MMP_UBYTE i;
	MMP_UBYTE rx_data = 0;
	SPI_CS(0);

	SPI_CLK_OUT();
	for(i = 0;i < 8;i++)
	{
		SPI_CLK_set(1); // read	

		SPI_DELAY();

		rx_data <<= 1;
		if(SPI_MISO_EX())
		{
			rx_data |= 1;
		}

		SPI_DELAY();

		SPI_CLK_set(0);
		
		SPI_DELAY();

	}
	return rx_data;

	SPI_CS(1);

}


















































