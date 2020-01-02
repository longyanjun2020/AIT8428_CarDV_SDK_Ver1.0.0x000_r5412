#ifndef __MIPI_TC358778_H__
#define __MIPI_TC358778_H__

//int IIC_TC358778_WrData(MMP_USHORT sub_addr, MMP_USHORT value);
MMP_LONG IIC_TC358778_WrData(MMP_ULONG data);
int IIC_TC358778_RdData(MMP_USHORT sub_addr, MMP_USHORT* value);
void IIC_TC358778_InitGpio(void);

void dcs_write_2P_XXX(MMP_BYTE cmd1, MMP_BYTE parm1);
void dcs_write_2P(MMP_UBYTE cmd1, MMP_UBYTE cmd2);
void IIC_TC358778_DCS_Long_Write(char *ParamList);
void IIC_TC358778_DCS_Long_Write_for_less_8byte(char *ParamList);

#endif
