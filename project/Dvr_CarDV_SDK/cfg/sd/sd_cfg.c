
#include "mmpf_sd.h"
#include "mmp_gpio_inc.h"
#include "sd_cfg.h"

MMP_GPIO_PIN sd_pwr_gpio[MMPF_SD_DEV_NUM] = 
{
	MMP_GPIO39,// MMP_GPIO39 lyj 20181026
	MMP_GPIO_MAX,
	MMP_GPIO_MAX
};

MMP_BOOL sd_pwr_active_level[MMPF_SD_DEV_NUM] = 
{
	MMP_FALSE,
	MMP_FALSE,
	MMP_FALSE
};


MMP_GPIO_PIN MMP_SD_GetPwrPin(MMPF_SD_ID sd_id)
{
	if (sd_id >= MMPF_SD_DEV_NUM)
		return MMP_GPIO_MAX;
	else
		return sd_pwr_gpio[sd_id];
}

MMP_BOOL MMP_SD_GetPwrActiveLvl(MMPF_SD_ID sd_id)
{
	if (sd_id >= MMPF_SD_DEV_NUM)
		return MMP_FALSE;
	else
		return sd_pwr_active_level[sd_id];
}











