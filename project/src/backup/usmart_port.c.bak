#include "usmart.h"
#include "usmart_port.h"

#include <string.h>

/* USMART 命令行接收缓冲区 */
uint8_t g_usart_rx_buf[USMART_USART_REC_LEN + 1U];
volatile uint16_t g_usart_rx_sta = 0;

/* 当前命令行长度 */
static uint16_t usmart_rx_len = 0;

/* 待解析命令副本: 解析期间串口可继续接收下一条命令, 避免缓冲区被覆盖 */
static char usmart_cmd_buf[USMART_USART_REC_LEN + 1U];

/**
 * @brief       获取输入数据流(字符串)
 *   @note      USMART通过解析该函数返回的字符串以获取函数名及参数等信息
 * @param       无
 * @retval
 *   @arg       0,  没有接收到数据
 *   @arg       其他,数据流首地址(不能是0)
 */
char *usmart_get_input_string(void)
{
    uint16_t len;
    char *pbuf = 0;

    if ((g_usart_rx_sta & 0x8000U) != 0U)     /* 串口接收完成？ */
    {
        len = g_usart_rx_sta & 0x3FFFU;       /* 得到此次接收到的数据长度 */
        if (len > USMART_USART_REC_LEN)
        {
            len = USMART_USART_REC_LEN;
        }
        g_usart_rx_buf[len] = '\0';           /* 在末尾加入结束符 */
        memcpy(usmart_cmd_buf, g_usart_rx_buf, (size_t)len + 1U);
        pbuf = usmart_cmd_buf;
        g_usart_rx_sta = 0;                   /* 开启下一次接收 */
    }

    return pbuf;
}

/**
 * @brief       向USMART输入一个字节
 *   @note      在串口接收中断中调用, 支持回车/换行结束命令, 退格删除
 * @param       data: 接收到的字节
 * @retval      无
 */
void usmart_port_uart_rx_byte(uint8_t data)
{
    if ((g_usart_rx_sta & 0x8000U) != 0U)     /* 上一条命令还没被处理 */
    {
        return;
    }

    if ((data == '\r') || (data == '\n'))
    {
        if (usmart_rx_len != 0U)              /* 空行不产生命令 */
        {
            g_usart_rx_buf[usmart_rx_len] = '\0';
            g_usart_rx_sta = 0x8000U | usmart_rx_len;
            usmart_rx_len = 0;
        }
    }
    else if ((data == '\b') || (data == 0x7FU))
    {
        if (usmart_rx_len != 0U)
        {
            usmart_rx_len--;
        }
    }
    else if (usmart_rx_len < USMART_USART_REC_LEN)
    {
        g_usart_rx_buf[usmart_rx_len++] = data;
    }
    else
    {
        usmart_rx_len = 0;                    /* 超长命令丢弃 */
    }
}

/**
 * @brief       向USMART输入一帧数据
 *   @note      可在串口DMA空闲中断中使用, 例如:
 *              usmart_port_uart_rx_frame(uart_rx_buf, recv_len);
 * @param       data: 数据首地址
 * @param       len: 数据长度
 * @retval      无
 */
void usmart_port_uart_rx_frame(const uint8_t *data, uint16_t len)
{
    if (data == 0)
    {
        return;
    }

    while (len-- != 0U)
    {
        usmart_port_uart_rx_byte(*data++);
    }
}

/* 如果使能了定时器扫描, 则需要定义如下函数 */
#if USMART_ENTIMX_SCAN == 1

/**
 * @brief       复位runtime
 *   @note      清除定时器计数寄存器、溢出标志, 并设置自动重装载值为最大,
 *              以最大限度延长计时时间
 * @param       无
 * @retval      无
 */
void usmart_timx_reset_time(void)
{
    tmr_flag_clear(USMART_TIMX, TMR_OVF_FLAG);          /* 清除定时器溢出标志 */
    tmr_period_value_set(USMART_TIMX, 0xFFFFU);         /* 自动重装载值设为最大值 */
    tmr_counter_value_set(USMART_TIMX, 0U);             /* 计数器清零 */
    usmart_dev.runtime = 0;
}

/**
 * @brief       获得runtime时间
 *   @note      定时器计数频率为10kHz, 返回值单位为0.1ms,
 *              最大计时时间约为定时器CNT值的2倍(13.1s左右)
 * @param       无
 * @retval      执行时间,单位:0.1ms
 */
uint32_t usmart_timx_get_time(void)
{
    if (tmr_flag_get(USMART_TIMX, TMR_OVF_FLAG) != RESET)   /* 运行期间产生了定时器溢出 */
    {
        usmart_dev.runtime += 0xFFFFU;
    }
    usmart_dev.runtime += tmr_counter_value_get(USMART_TIMX);
    return usmart_dev.runtime;                              /* 返回计数值 */
}

/**
 * @brief       定时器初始化函数
 * @param       arr:自动重装载值
 *              psc:定时器分频系数
 * @retval      无
 */
void usmart_timx_init(uint16_t arr, uint16_t psc)
{
    USMART_TIMX_CLK_ENABLE();                           /* 使能定时器外设时钟 */

    /* 定时器基础参数配置: 向上计数, 时钟不分频, 关闭自动重装载预装载 */
    tmr_cnt_dir_set(USMART_TIMX, TMR_COUNT_UP);
    tmr_clock_source_div_set(USMART_TIMX, TMR_CLOCK_DIV1);
    tmr_period_buffer_enable(USMART_TIMX, FALSE);
    tmr_base_init(USMART_TIMX, (uint32_t)arr, (uint32_t)psc);

    tmr_counter_value_set(USMART_TIMX, 0U);
    tmr_flag_clear(USMART_TIMX, TMR_OVF_FLAG);          /* 清除初始化产生的标志 */

    /* 使能定时器溢出中断 */
    tmr_interrupt_enable(USMART_TIMX, TMR_OVF_INT, TRUE);

    /* NVIC中断配置 */
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(USMART_TIMX_IRQn, 3, 0);

    /* 启动定时器 */
    tmr_counter_enable(USMART_TIMX, TRUE);
}

/**
 * @brief       USMART定时器中断服务函数
 * @param       无
 * @retval      无
 */
void USMART_TIMX_IRQHandler(void)
{
    if (tmr_interrupt_flag_get(USMART_TIMX, TMR_OVF_FLAG) != RESET)
    {
        tmr_counter_value_set(USMART_TIMX, 0U);         /* 计数器清零 */
        tmr_period_value_set(USMART_TIMX, USMART_TIMX_SCAN_PERIOD); /* 恢复扫描周期 */
    }
    tmr_flag_clear(USMART_TIMX, TMR_OVF_FLAG);          /* 清除溢出中断标志 */
}

#endif
