/* linux/arch/arm/mach-s5pv210/mach-fv1.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 *
 *
 * Adapted by Angus Gratton <gus@projectgus.com> from mach-smdkv210.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <asm/hardware/vic.h>
#include <linux/wl12xx.h>

#include <mach/map.h>
#include <mach/regs-clock.h>

#include <plat/regs-serial.h>
#include <plat/regs-srom.h>
#include <plat/gpio-cfg.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adc.h>
#include <plat/ts.h>
#include <plat/iic.h>
#include <plat/pm.h>
#include <plat/fb.h>
#include <plat/s5p-time.h>
#include <plat/regs-fb-v4.h>
#include <plat/clock.h>
#include <plat/ehci.h>
#include <plat/sdhci.h>

#include "common.h"

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define SMDKV210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define SMDKV210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define SMDKV210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg smdkv210_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
};

static struct s5p_ehci_platdata smdkv210_ehci_pdata;

static struct wl12xx_platform_data s5p_fv1_wlan_data __initdata = {
	.irq = 0, // placeholder until we register a gpio, see below
	.board_ref_clock = WL12XX_REFCLOCK_38,
	.board_tcxo_clock = WL12XX_TCXOCLOCK_38_4,
};

static void __init fv1_wl12xx_configure(void) {
        /* wl12xx GPIO IRQ external pin setup */
	int res;

	/* enable (todo: move to plat data power on/off fn) */
	gpio_request(S5PV210_GPH2(5), "GPH2");
	gpio_direction_output(S5PV210_GPH2(5), 1);
	gpio_set_value(S5PV210_GPH2(5), 1);
	gpio_free(S5PV210_GPH2(5));

        /* this second gpio may not be needed for wifi */
	gpio_request(S5PV210_GPH2(4), "GPH2");
	gpio_direction_output(S5PV210_GPH2(4), 1);
	gpio_set_value(S5PV210_GPH2(4), 1);
	gpio_free(S5PV210_GPH2(4));

	s5p_fv1_wlan_data.irq = s5p_register_gpio_interrupt(S5PV210_GPH2(6));
	if(s5p_fv1_wlan_data.irq) {
		s3c_gpio_cfgpin(S5PV210_GPH2(6), S3C_GPIO_SFN(0xf)); // set as ext int
		s3c_gpio_setpull(S5PV210_GPH2(6), S3C_GPIO_PULL_NONE);

		res = wl12xx_set_platform_data(&s5p_fv1_wlan_data);
		if (res != 0)
			pr_err("error setting wl12xx data %d\n", res);
	}
}


static struct platform_device *smdkv210_devices[] __initdata = {
	&s3c_device_adc,
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,
	&s3c_device_hsmmc2,
	&s3c_device_hsmmc3,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&s5p_device_i2c_hdmiphy,
	&s3c_device_rtc,
	&s3c_device_ts,
	&s3c_device_wdt,
	&s5pv210_device_ac97,
	&s5pv210_device_iis0,
	&s5p_device_ehci,
	&s5pv210_device_spdif,
	&samsung_asoc_dma,
	&samsung_asoc_idma,
	&s5p_device_hdmi,
	&s5p_device_mixer,
};

/* Wireless LAN WL1271 */
static struct s3c_sdhci_platdata fv1_hsmmc1_data __initdata = {
	.max_width		= 4,
	.cd_type		= S3C_SDHCI_CD_PERMANENT,
	/* ext_cd_{init,cleanup} callbacks will be added later */
};


static struct i2c_board_info smdkv210_i2c_devs0[] __initdata = {
	{ I2C_BOARD_INFO("24c08", 0x50), },     /* Samsung S524AD0XD1 */
};

static struct i2c_board_info smdkv210_i2c_devs1[] __initdata = {
	/* To Be Updated */
};

static struct i2c_board_info smdkv210_i2c_devs2[] __initdata = {
	/* To Be Updated */
};

static void __init smdkv210_map_io(void)
{
	s5pv210_init_io(NULL, 0);
	s3c24xx_init_clocks(24000000);
	s3c24xx_init_uarts(smdkv210_uartcfgs, ARRAY_SIZE(smdkv210_uartcfgs));
	s5p_set_timer_source(S5P_PWM2, S5P_PWM4);
}

static void __init smdkv210_machine_init(void)
{
	s3c_pm_init();

	s3c24xx_ts_set_platdata(NULL);

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	s3c_i2c2_set_platdata(NULL);
	s5p_i2c_hdmiphy_set_platdata(NULL);
	i2c_register_board_info(0, smdkv210_i2c_devs0,
			ARRAY_SIZE(smdkv210_i2c_devs0));
	i2c_register_board_info(1, smdkv210_i2c_devs1,
			ARRAY_SIZE(smdkv210_i2c_devs1));
	i2c_register_board_info(2, smdkv210_i2c_devs2,
			ARRAY_SIZE(smdkv210_i2c_devs2));

	s5p_ehci_set_platdata(&smdkv210_ehci_pdata);

	s3c_sdhci1_set_platdata(&fv1_hsmmc1_data);

	fv1_wl12xx_configure();

	platform_add_devices(smdkv210_devices, ARRAY_SIZE(smdkv210_devices));
}

MACHINE_START(SMDKV210, "Flexiview FV1")
	/* Maintainer: Angus Gratton <gus@projectgus.com> */
	.atag_offset	= 0x100,
	.init_irq	= s5pv210_init_irq,
	.handle_irq	= vic_handle_irq,
	.map_io		= smdkv210_map_io,
	.init_machine	= smdkv210_machine_init,
	.timer		= &s5p_timer,
	.restart	= s5pv210_restart,
MACHINE_END
