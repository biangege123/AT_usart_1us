/* Host-side test shim: replaces the AT32 hardware port layer so that
 * usmart.c / usmart_str.c can be compiled and executed on the PC.
 * This file is only used by tests/usmart_host and is not part of the firmware.
 * It is force-included (-include) before project/inc/usmart.h, so its include
 * guard suppresses the real AT32 usmart_port.h. */
#ifndef __USMART_PORT_H
#define __USMART_PORT_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_FNAME_LEN       30
#define MAX_PARM            10
#define PARM_LEN            200

#define USMART_ENTIMX_SCAN  1
#define USMART_USE_HELP     1
#define USMART_USE_WRFUNS   1

#define USMART_PRINTF       printf

#define USMART_TIMX_SCAN_PERIOD  100U
#define USMART_TIMX_CLK_MHZ      200U

#define USMART_USART_REC_LEN     200U

extern uint8_t g_usart_rx_buf[USMART_USART_REC_LEN + 1U];
extern volatile uint16_t g_usart_rx_sta;

char *usmart_get_input_string(void);
void usmart_port_uart_rx_byte(uint8_t data);
void usmart_port_uart_rx_frame(const uint8_t *data, uint16_t len);

void usmart_timx_reset_time(void);
uint32_t usmart_timx_get_time(void);
void usmart_timx_init(uint16_t arr, uint16_t psc);

#endif
