#ifndef _SOFT_I2C_H_
#define _SOFT_I2C_H_


#include <AHC_General.h>
#include <fm_i2c.h>



MMP_ULONG iic_read_bytes(MMP_UBYTE *data, MMP_UBYTE byte_len);

MMP_ULONG iic_write_bytes(MMP_UBYTE *data, MMP_UBYTE byte_len);

void IIC_write_data(MMP_UBYTE reg,MMP_UBYTE data, MMP_UBYTE byte_len);

void IIC_write_data_e2prom(MMP_UBYTE reg,MMP_UBYTE* data, MMP_UBYTE byte_len);
MMP_ULONG iic_read_bytes_e2prom(MMP_UBYTE reg,MMP_UBYTE *data, MMP_UBYTE byte_len);



#endif



























