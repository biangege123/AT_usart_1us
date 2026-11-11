#include "usmart.h"
#include "usmart_str.h"
#include "led.h"

/* 用户函数表: 可按相同格式添加自己的函数
 * 格式: {(void *)函数名, "返回值类型 函数名(参数类型 参数名...);"},
 * 注意: 字段顺序必须与 struct _m_usmart_nametab 一致(func 在前, name 在后);
 *       函数名必须与实际函数一致, 参数个数不能超过 MAX_PARM(10)。
 */
struct _m_usmart_nametab usmart_nametab[] =
{
#if USMART_USE_WRFUNS == 1
    {(void *)read_addr,  "uint32_t read_addr(uint32_t addr);"},
    {(void *)write_addr, "void write_addr(uint32_t addr,uint32_t val);"},
#endif
    {(void *)at32_led_on,     "void at32_led_on(led_type led);"},
    {(void *)at32_led_off,    "void at32_led_off(led_type led);"},
    {(void *)at32_led_toggle, "void at32_led_toggle(led_type led);"},
    {(void *)usmart_set_pulse_width_us,  "uint8_t usmart_set_pulse_width_us(uint32_t width_us);"},
    {(void *)usmart_set_pulse_width_us2, "uint8_t usmart_set_pulse_width_us2(uint32_t width_us);"},
};

/* USMART控制管理器 */
struct _m_usmart_dev usmart_dev =
{
    usmart_nametab,
    usmart_init,
    usmart_cmd_rec,
    usmart_exe,
    usmart_scan,
    (uint8_t)(sizeof(usmart_nametab) / sizeof(struct _m_usmart_nametab)),
    0,                          /* pnum */
    0,                          /* id */
    1,                          /* sptype: 默认16进制显示 */
    0,                          /* parmtype */
    {0},                        /* plentbl */
    {0},                        /* parm */
    0,                          /* runtimeflag */
    0                           /* runtime */
};
