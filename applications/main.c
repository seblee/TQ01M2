/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

/* defined the LED0 pin: PC13 */
#define LED0_PIN GET_PIN(C, 13)
/* app version */
#define APP_VERSION "1.0.0"
void IIC_Capture_entry(void *parameter);
int main(void)
{
    rt_kprintf("APP_VERSION:%s\n", APP_VERSION);
    /* user app entry */
    rt_thread_t IIC_Capture;
    IIC_Capture = rt_thread_create("IIC_Capture", IIC_Capture_entry, RT_NULL,
                                   2560, 10, 20);
    RT_ASSERT(IIC_Capture != RT_NULL);
    if (IIC_Capture != RT_NULL)
        rt_thread_startup(IIC_Capture);

    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);



    while (1)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }

    // return RT_EOK;
}

/**
 * Function    ota_app_vtor_reconfig
 * Description Set Vector Table base location to the start addr of app(RT_APP_PART_ADDR).
*/
static int ota_app_vtor_reconfig(void)
{
#define NVIC_VTOR_MASK 0x3FFFFF80
    /* Set the Vector Table base location by user application firmware definition */
    SCB->VTOR = 0x08010000 & NVIC_VTOR_MASK;

    return 0;
}
INIT_BOARD_EXPORT(ota_app_vtor_reconfig);
