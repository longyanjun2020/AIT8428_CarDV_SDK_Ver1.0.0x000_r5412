
//==============================================================================
//
//  File        : dsc_Charger.c
//  Description : 
//  Author      : Mark Chang
//  Revision    : 1.0
//
//==============================================================================
#include "dsc_Charger.h"
#include "ControlIC_DS9525.h"
#include "AHC_parameter.h"
#include "AHC_General.h"
#include "AHC_USB.h"
#include "mmp_reg_gbl.h"
#include "mmpf_usbvend.h"
#include "mmps_usb.h"
#include "mmps_pio.h"

//extern void SPI_Write(MMP_UBYTE addr, MMP_USHORT data);

AHC_BOOL IsAdapter_Connect(void)
{
	AHC_BOOL 	ubAdapterConnect = AHC_TRUE;

    ubAdapterConnect = AHC_IsUsbConnect();
    if(ubAdapterConnect == AHC_TRUE){
        USB_DETECT_PHASE sUSBNextStates;
       AHC_USBGetStates(&sUSBNextStates, AHC_USB_GET_PHASE_NEXT);
       if((USB_DETECT_PHASE_VBUS_ACTIVE == sUSBNextStates) && (!MMPF_USB_IsAdapter())){
            //Connect to PC not adaptor.
            ubAdapterConnect = AHC_FALSE;
       }
    }
    else if (!AHC_IsDcCableConnect()) {
        ubAdapterConnect = AHC_FALSE;
    }

	return ubAdapterConnect;
}
