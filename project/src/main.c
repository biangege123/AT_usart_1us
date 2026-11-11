/* add user code begin Header */
/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  * Copyright (c) 2025, Artery Technology, All rights reserved.
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "at32f403a_407_wk_config.h"
#include "wk_exint.h"
#include "wk_i2c.h"
#include "wk_tmr.h"
#include "wk_usart.h"
#include "wk_dma.h"
#include "wk_system.h"
#include "i2c_app.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */


/*PE9,PC6-->PA0*/


#define DELAY  100
#define FAST   1
#define SLOW   4
#define PULSE_HIGH_US   1U            //脉冲步长
#define TMR1_TICKS_PER_US 200U        
#define MAX_PULSE_INDEX (65535U / (PULSE_HIGH_US * TMR1_TICKS_PER_US))







#include "button.h"
#include "OLED.h"
#include "led.h"
#include "delay.h"
#include "usmart.h"
#include "string.h"
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */
#define MAX_LEN     256
/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */
uint16_t i=1;
u8 uart_rx_buf[MAX_LEN] = {0};     /* USART1 RX DMA 缓冲区, 空闲中断后交给 USMART */
u8 uart_tx_buf[MAX_LEN] = {0};

static uint32_t tmr1_rising_edge_us;
static uint32_t measured_elapsed_us;
static uint8_t waiting_for_tmr8_rising_edge;
static uint8_t measurement_available;



/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */


//这个完成了TMR1脉冲启动的准备工作
static void at32_tmr1_pulse_start(void)
{
  tmr_counter_enable(TMR1,FALSE);     //先禁用TMR1计数器
  tmr_counter_value_set(TMR1,0);      //设置TMR1计数器值=0
  tmr_flag_clear(TMR1,TMR_OVF_FLAG);  //清除TMR1的标志位：溢出中断标记
  tmr_counter_enable(TMR1,TRUE);      //后启用TMR1计数器
}


//这个完成了TMR8脉冲启动的准备工作
static void at32_tmr8_pulse_start(void)
{
  tmr_counter_enable(TMR8,FALSE);     //先禁用TMR8计数器
  tmr_counter_value_set(TMR8,0);      //设置TMR8计数器值=0
  tmr_flag_clear(TMR8,TMR_OVF_FLAG);  //清除TMR8的标志位：溢出中断标记
  tmr_counter_enable(TMR8,TRUE);      //后启用TMR8计数器
}




/*usmart专门用于调试*/

uint8_t usmart_set_pulse_width_us(uint32_t width_us)
{
  if ((width_us == 0U) || (width_us > MAX_PULSE_INDEX))
  {
    return USMART_PARMERR;
  }

  i = (uint16_t)width_us;
  tmr_period_value_set(TMR1, i * PULSE_HIGH_US * TMR1_TICKS_PER_US);

  OLED_ShowNum(3, 5, i, 3);
  OLED_ShowNum(2, 5, i * PULSE_HIGH_US, 3);
  at32_tmr1_pulse_start();      //开始发射脉冲
  at32_led_toggle(LED2);

  return USMART_OK;
}

uint8_t usmart_set_pulse_width_us2(uint32_t width_us)
{
  if ((width_us == 0U) || (width_us > MAX_PULSE_INDEX))
  {
    return USMART_PARMERR;
  }

  i = (uint16_t)width_us;
  tmr_period_value_set(TMR8, i * PULSE_HIGH_US * TMR1_TICKS_PER_US);

  OLED_ShowNum(3, 5, i, 3);
  OLED_ShowNum(2, 5, i * PULSE_HIGH_US, 3);
  at32_tmr8_pulse_start();      //开始发射脉冲
  at32_led_toggle(LED2);

  return USMART_OK;
}


void send_data(u8 *buf,u16 len)
{
    memcpy(uart_tx_buf,buf,len);
    while (usart_flag_get(USART1,USART_TDBE_FLAG) == RESET);
    dma_channel_enable(DMA1_CHANNEL2,FALSE);
    dma_data_number_set(DMA1_CHANNEL2,len);
    dma_channel_enable(DMA1_CHANNEL2,TRUE);
}


static void tmr1_capture_process(uint32_t edge_time_us)
{
  tmr1_rising_edge_us = edge_time_us;
  waiting_for_tmr8_rising_edge = 1U;
  measurement_available = 0U;
}

static void tmr8_capture_process(uint32_t edge_time_us)
{
  if (waiting_for_tmr8_rising_edge != 0U)
  {
    measured_elapsed_us = edge_time_us - tmr1_rising_edge_us;
    measurement_available = 1U;
    waiting_for_tmr8_rising_edge = 0U;
  }
}


//不断轮询捕获TMR2的输入信号
static void tmr2_capture_poll(void)
{
  uint8_t tmr1_pending = (tmr_flag_get(TMR2, TMR_C1_FLAG) == SET);
  uint8_t tmr8_pending = (tmr_flag_get(TMR2, TMR_C2_FLAG) == SET);
  uint32_t tmr1_time = 0U;
  uint32_t tmr8_time = 0U;

  if (tmr1_pending != 0U)
  {
    tmr1_time = tmr_channel_value_get(TMR2, TMR_SELECT_CHANNEL_1);
    tmr_flag_clear(TMR2, TMR_C1_FLAG);
  }

  if (tmr8_pending != 0U)
  {
    tmr8_time = tmr_channel_value_get(TMR2, TMR_SELECT_CHANNEL_2);
    tmr_flag_clear(TMR2, TMR_C2_FLAG);
  }

  if ((tmr1_pending != 0U) && (tmr8_pending != 0U))
  {
    if ((uint32_t)(tmr8_time - tmr1_time) < 0x80000000U)
    {
      tmr1_capture_process(tmr1_time);
      tmr8_capture_process(tmr8_time);
    }
    else
    {
      tmr8_capture_process(tmr8_time);
      tmr1_capture_process(tmr1_time);
    }
  }
  else if (tmr1_pending != 0U)
  {
    tmr1_capture_process(tmr1_time);
  }
  else if (tmr8_pending != 0U)
  {
    tmr8_capture_process(tmr8_time);
  }
}


/* add user code end 0 */

/**
  * @brief main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* add user code begin 1 */

  // wk_dma_channel_config(DMA1_CHANNEL1, 
  //                       (uint32_t)&USART1->dt, 
  //                       (uint32_t)uart_rx_buf, 
  //                       sizeof(uart_rx_buf));




  button_type key_num,key_num2,key_num3,key_num4,key_num5;
  // button_type key_num2;
  /* add user code end 1 */

  /* system clock config. */
  wk_system_clock_config();

  /* config periph clock. */
  wk_periph_clock_config();

  /* nvic config. */
  wk_nvic_config();

  /* timebase config for
     void wk_delay_us(uint32_t delay);
     void wk_delay_ms(uint32_t delay); */
  wk_timebase_init();

  /* init usart1 function. */
  wk_usart1_init();

  /* init i2c1 function. */
  wk_i2c1_init();

  /* init i2c app function. */
  wk_i2c_app_init();

  /* init dma1 channel1 */
  wk_dma1_channel1_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR 
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA1_CHANNEL1, 
                        (uint32_t)&USART1->dt, 
                        (uint32_t)uart_rx_buf, 
                        sizeof(uart_rx_buf));
  dma_channel_enable(DMA1_CHANNEL1, TRUE);

  /* init dma1 channel2 */
  wk_dma1_channel2_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR 
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA1_CHANNEL2, 
                        (uint32_t)&USART1->dt, 
                        DMA1_CHANNEL2_MEMORY_BASE_ADDR, 
                        DMA1_CHANNEL2_BUFFER_SIZE);
  dma_channel_enable(DMA1_CHANNEL2, TRUE);

  /* init exint function. */
  wk_exint_config();

  /* init tmr1 function. */
  wk_tmr1_init();

  /* init tmr8 function. */
  wk_tmr8_init();

  /* init tmr2 function. */
  wk_tmr2_init();

  /* add user code begin 2 */
  OLED_Init();
  at32_board_init();
  at32_button_init();

  //脉冲发生器1设置
  tmr_counter_enable(TMR1, FALSE);      //先禁用TMR1计数器
  tmr_counter_value_set(TMR1, 0);       //设置TMR1计数器值=0
  tmr_period_value_set(TMR1, i* PULSE_HIGH_US * TMR1_TICKS_PER_US);
  //设置TMR1 设置定时器周期
  tmr_output_channel_mode_select(TMR1, TMR_SELECT_CHANNEL_1, TMR_OUTPUT_CONTROL_PWM_MODE_B);
  //设置TMR1 通道1 模式：PWM模式B
  tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_1, 1); //设置TMR1 定时器通道1 值为1
  tmr_one_cycle_mode_enable(TMR1, TRUE); //启动TMR1单周期模式
  tmr_flag_clear(TMR1, TMR_OVF_FLAG);   //清除TMR1溢出标志位




  //脉冲发生器2设置
  tmr_counter_enable(TMR8, FALSE);      //先禁用TMR8计数器
  tmr_counter_value_set(TMR8, 0);       //设置TMR8计数器值=0
  tmr_period_value_set(TMR8, i* PULSE_HIGH_US * TMR1_TICKS_PER_US);
  //设置TMR8 设置定时器周期
  tmr_output_channel_mode_select(TMR8, TMR_SELECT_CHANNEL_1, TMR_OUTPUT_CONTROL_PWM_MODE_B);
  //设置TMR8 通道1 模式：PWM模式B
  tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_1, 1); //设置TMR8 定时器通道1 值为1
  tmr_one_cycle_mode_enable(TMR8, TRUE); //启动TMR8单周期模式
  tmr_flag_clear(TMR8, TMR_OVF_FLAG);   //清除TMR8溢出标志位




  OLED_ShowString(1, 1, "Pulse Generator");


  OLED_ShowString(2, 1, "PW=");
  OLED_ShowNum(2, 5, i * PULSE_HIGH_US, 3);
  OLED_ShowString(2, 13, "us");

  OLED_ShowString(3, 1, "i=");
  OLED_ShowNum(3, 5, i, 3);

  // OLED_ShowString(4, 1, "step:");
  // OLED_ShowNum(4, 10, PULSE_HIGH_US, 3);


  usart_interrupt_enable(USART1,USART_IDLE_INT,TRUE);

  /* init usmart: TMR3 计数 10kHz, 用于函数运行时间统计 */
  usmart_init(USMART_TIMX_CLK_MHZ);
  /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */
    /* USMART 命令解析与执行: 必须周期性调用.
     * 串口数据由 USART1 RX DMA + 空闲中断送入 usmart_port_uart_rx_byte(),
     * 这里只负责取出已接收完整的一行并执行, 不能再去读 USART1->dt,
     * 否则会与 DMA 抢接收数据, 造成命令缺字符. */
    usmart_scan();
    tmr2_capture_poll();
    
    key_num = at32_button_user_press();
    key_num2 = at32_button_s1_press();
    key_num3 = at32_button_s2_press();
    key_num4 = at32_button_s3_press();
    key_num5 = at32_button_s4_press();
    if(key_num == USER_BUTTON)
    {
      if(i < MAX_PULSE_INDEX)
      {
        i++;
      }
      tmr_period_value_set(TMR1, i * PULSE_HIGH_US * TMR1_TICKS_PER_US);
      
      
      OLED_ShowString(3, 1, "i=");
      OLED_ShowNum(3, 5, i, 3);

      OLED_ShowString(2, 1, "PW=");
      OLED_ShowNum(2, 5, i * PULSE_HIGH_US, 3);
      OLED_ShowString(2, 13, "us");
      at32_tmr1_pulse_start();
    }

    if(key_num2 == S1_BUTTON)
    {
      if(i < MAX_PULSE_INDEX)
      {
        i++;
      }
      tmr_period_value_set(TMR8, i * PULSE_HIGH_US * TMR1_TICKS_PER_US);
      
      
      OLED_ShowString(3, 1, "i=");
      OLED_ShowNum(3, 5, i, 3);

      OLED_ShowString(2, 1, "PW=");
      OLED_ShowNum(2, 5, i * PULSE_HIGH_US, 3);
      OLED_ShowString(2, 13, "us");
      at32_tmr8_pulse_start();
    }

    if(key_num3 == S2_BUTTON)
    {
      if (measurement_available != 0U)
      {
        printf("TIM1->TIM8: %lu us\r\n",
              (unsigned long)measured_elapsed_us);
      }
      else
      {
        printf("No complete TIM1->TIM8 measurement\r\n");
      }
    }


    /* add user code end 3 */
  }
}

  /* add user code begin 4 */
void USART1_IRQHandler(void)
{
    u16 tmp_len;

    if (usart_flag_get(USART1,USART_IDLEF_FLAG)!=RESET)
    {
      uint32_t guard = 10000U;

      /* 等待 DMA 取走最后一个字节(RDBF 清零), 否则下面清除空闲标志时
       * 库函数会读 USART1->dt, 从而把 DMA 还没搬走的最后一个字节丢掉 */
      while ((usart_flag_get(USART1,USART_RDBF_FLAG)!=RESET) && (--guard != 0U))
      {
      }

      dma_channel_enable(DMA1_CHANNEL1,FALSE);
      tmp_len = MAX_LEN - dma_data_number_get(DMA1_CHANNEL1);
      if (tmp_len > MAX_LEN)
      {
        tmp_len = MAX_LEN;
      }
      dma_data_number_set(DMA1_CHANNEL1,MAX_LEN);

      /* 读 STS + DT 清除 IDLE 标志 */
      usart_flag_clear(USART1,USART_IDLEF_FLAG);

      if (tmp_len != 0U)
      {
        usmart_port_uart_rx_frame(uart_rx_buf,tmp_len);
      }

      dma_channel_enable(DMA1_CHANNEL1,TRUE);


    }
  

}




  /* add user code end 4 */
