#ifndef __USMART_PORT_H
#define __USMART_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f403a_407_wk_config.h"
#include <stdio.h>

/* USMART 内部缓冲区大小 */
#define MAX_FNAME_LEN       30
#define MAX_PARM            10
#define PARM_LEN            200

/* 1: 使用定时器支持函数运行时间统计 */
#define USMART_ENTIMX_SCAN  1

/* 系统功能开关 */
#define USMART_USE_HELP     1
#define USMART_USE_WRFUNS   1

/* 打印接口 */
#define USMART_PRINTF       printf

/*----------------------------------------------------------------------------
 * USMART 定时器移植层
 * 使用 TMR3 作为扫描/运行时间统计定时器, 计数频率固定为 10kHz。
 * 若改用其它定时器, 只需修改下面的宏以及 usmart_timx_init 中的 AT32 库调用。
 *---------------------------------------------------------------------------*/
#define USMART_TIMX                  TMR3
#define USMART_TIMX_IRQn             TMR3_GLOBAL_IRQn
#define USMART_TIMX_IRQHandler       TMR3_GLOBAL_IRQHandler
#define USMART_TIMX_CLK_ENABLE()     crm_periph_clock_enable(CRM_TMR3_PERIPH_CLOCK, TRUE)

/* TMR3 计数时钟频率(MHz), 用于 usmart_init() 计算分频:
 * AT32F403A 在 APB1 分频不为 1 时, 定时器时钟 = APB1CLK * 2 = 200MHz */
#define USMART_TIMX_CLK_MHZ          200U

/* 命令扫描周期, 单位 0.1ms (计数频率 10kHz), 100 -> 10ms */
#define USMART_TIMX_SCAN_PERIOD      100U

/*----------------------------------------------------------------------------
 * USMART 串口输入移植层
 * 应用层在串口接收中断/DMA空闲中断中调用 usmart_port_uart_rx_byte() 或
 * usmart_port_uart_rx_frame(), 把收到的数据送入 USMART 命令行缓冲区。
 *---------------------------------------------------------------------------*/
#define USMART_USART                 USART1
#define USMART_USART_REC_LEN         200U

extern uint8_t g_usart_rx_buf[USMART_USART_REC_LEN + 1U];
extern volatile uint16_t g_usart_rx_sta;

/* 获取一条已经接收完成的命令, 没有命令时返回 0 */
char *usmart_get_input_string(void);

/* 串口输入接口: 单字节 / 整帧 */
void usmart_port_uart_rx_byte(uint8_t data);
void usmart_port_uart_rx_frame(const uint8_t *data, uint16_t len);

/* USMART 定时器接口 */
void usmart_timx_reset_time(void);
uint32_t usmart_timx_get_time(void);
void usmart_timx_init(uint16_t arr, uint16_t psc);

#ifdef __cplusplus
}
#endif

#endif
