/**
 ****************************************************************************
 * @Warning Without permission from the author,Not for commercial use
 * @File    IIC_Capture.c
 * @Author  
 * @date    
 * @version V1.0
 *************************************************
 * @brief   
 ****************************************************************************
 * @attention 
 * Powered By Xiaowine
 * <h2><center>&copy;  Copyright(C) 2015-2019</center></h2>
 * All rights reserved
 * 
**/
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "IIC_Capture.h"

int status_SDA = 0;
int status_SCL = 0;

typedef enum
{
    IIC_IDEL,
    IIC_START,
    IIC_ADDR,
    IIC_DATA,
    IIC_ACK,
    IIC_STOP,
} IIC_STATUS_t;
IIC_STATUS_t iic_status = IIC_IDEL;

struct msg
{
    rt_uint8_t state; /* iic bus status */
    rt_uint8_t data;  /* iic buff data */
};

/* defined the SCL pin:   */
#define SCL_PIN GET_PIN(D, 11)
/* defined the SCA pin:   */
#define SDA_PIN GET_PIN(D, 12)

rt_mq_t status_mq = RT_NULL;
rt_uint8_t bitCount = 0;
rt_uint8_t data = 0;

void send_mq(rt_uint8_t state, rt_uint8_t data)
{
    struct msg message;
    rt_err_t rc;
    message.state = iic_status;
    message.data = data;
    rc = rt_mq_send(status_mq, &message, sizeof(struct msg)); /* 发送消息到消息队列中 */
    if (rc != RT_EOK)
    {
        rt_kprintf("rt_mq_send ERR\n");
    }
}

void SDA_LEVEL(void *arg)
{
    status_SDA = rt_pin_read(SDA_PIN);
    if ((status_SCL == 1) && (status_SDA == 0))
    {
        iic_status = IIC_START;
        send_mq(iic_status, 0);
    }
    if ((status_SCL == 1) && (status_SDA == 1))
    {
        if (iic_status == IIC_ACK)
        {
            iic_status = IIC_STOP;
            send_mq(iic_status, 0);
        }
    }
}

void SCL_LEVEL(void *arg)
{
    status_SCL = rt_pin_read(SCL_PIN);
    if (iic_status == IIC_START)
    {
        iic_status = IIC_ADDR;
        bitCount = 0;
    }
    if (iic_status == IIC_ACK)
    {
        iic_status = IIC_DATA;
        bitCount = 0;
    }

    if ((status_SCL == 1) && ((iic_status == IIC_ADDR) || (iic_status == IIC_DATA)))
    {
        if (bitCount < 8)
        {
            data <<= 1;
            data |= status_SDA;
            bitCount++;
        }
        else if (bitCount == 8)
        {
            iic_status = IIC_ACK;
            bitCount++;
        }

        if (bitCount == 8)
        {
            send_mq(iic_status, data);
            data = 0;
        }
        if (bitCount == 9)
        {
            send_mq(iic_status, status_SDA);
        }
    }
}

void IIC_Capture_entry(void *parameter)
{
    rt_kprintf("IIC_Capture_entry \n");
    status_mq = rt_mq_create("status_mq", sizeof(struct msg), 100, RT_IPC_FLAG_FIFO);
    if (!status_mq)
    {
        rt_kprintf("status_mq create failed\n");
    }

    /* set SCL pin mode to INPUT */
    rt_pin_mode(SCL_PIN, PIN_MODE_INPUT);

    /* 绑定中断，上升沿模式，回调函数名为beep_on */
    rt_pin_attach_irq(SCL_PIN, PIN_IRQ_MODE_FALLING, SCL_LEVEL, RT_NULL);
    /* 使能中断 */
    rt_pin_irq_enable(SCL_PIN, PIN_IRQ_ENABLE);

    /* set SDA pin mode to INPUT */
    rt_pin_mode(SDA_PIN, PIN_MODE_INPUT);
    /* 绑定中断，上升沿模式，回调函数名为beep_on */
    rt_pin_attach_irq(SDA_PIN, PIN_IRQ_MODE_FALLING, SDA_LEVEL, RT_NULL);
    /* 使能中断 */
    rt_pin_irq_enable(SDA_PIN, PIN_IRQ_ENABLE);

    status_SDA = rt_pin_read(SDA_PIN);
    status_SCL = rt_pin_read(SCL_PIN);

    while (1)
    {
        struct msg message;
        rt_err_t rc;
        rc = rt_mq_recv(status_mq, (void *)&message, sizeof(struct msg), RT_WAITING_FOREVER);
        if (rc == RT_EOK)
        {
            if (message.state == IIC_START)
            {
                rt_kprintf("\nIIC_START ");
            }
            if (message.state == IIC_ADDR)
            {
                rt_kprintf("ADDR:%02x ", message.data);
            }
            if (message.state == IIC_DATA)
            {
                rt_kprintf("DATA:%02x ", message.data);
            }
            if (message.state == IIC_ACK)
            {
                rt_kprintf("%s ", (message.data == 1) ? "NACK" : "ACK");
            }
        }
        else
        {
            rt_kprintf("mq_recv:%s\n", (rc == (-RT_ETIMEOUT)) ? "-RT_ETIMEOUT" : "-RT_ERROR");
        }
    }
}
