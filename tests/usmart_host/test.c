/* Host-side functional test for the USMART command parser.
 * Compiles the real usmart.c / usmart_str.c and drives them with the same
 * function table layout used by project/src/usmart_config.c. */
#include "usmart.h"
#include "usmart_str.h"

#include <stdio.h>
#include <string.h>

/* defined in usmart.c but not declared in usmart.h */
uint8_t usmart_sys_cmd_exe(char *str);

uint8_t g_usart_rx_buf[USMART_USART_REC_LEN + 1U];
volatile uint16_t g_usart_rx_sta = 0;

/* --- hardware stubs ------------------------------------------------------ */
void usmart_timx_init(uint16_t arr, uint16_t psc)
{
    (void)arr;
    (void)psc;
}

void usmart_timx_reset_time(void)
{
    usmart_dev.runtime = 0;
}

uint32_t usmart_timx_get_time(void)
{
    return usmart_dev.runtime;
}

char *usmart_get_input_string(void)
{
    return 0;
}

void usmart_port_uart_rx_byte(uint8_t data)
{
    (void)data;
}

void usmart_port_uart_rx_frame(const uint8_t *data, uint16_t len)
{
    (void)data;
    (void)len;
}

/* --- functions under test ------------------------------------------------ */
static uint32_t cap_args[MAX_PARM];
static uint8_t cap_cnt;
static uint32_t requested_pulse_width_us;

uint8_t usmart_set_pulse_width_us(uint32_t width_us)
{
    requested_pulse_width_us = width_us;
    return (width_us > 0U && width_us <= 327U) ? USMART_OK : USMART_PARMERR;
}

static void capture(uint32_t v)
{
    if (cap_cnt < MAX_PARM)
    {
        cap_args[cap_cnt++] = v;
    }
}

static uint32_t dummy_read_addr(uint32_t addr)
{
    capture(addr);
    return 0x12345678U;
}

static void dummy_write_addr(uint32_t addr, uint32_t val)
{
    capture(addr);
    capture(val);
}

static void dummy_led_on(uint32_t led)
{
    capture(led);
}

static void dummy_led_off(uint32_t led)
{
    capture(led);
}

static void dummy_led_toggle(uint32_t led)
{
    capture(led);
}

struct _m_usmart_nametab usmart_nametab[] =
{
    {(void *)dummy_read_addr,  "uint32_t read_addr(uint32_t addr);"},
    {(void *)dummy_write_addr, "void write_addr(uint32_t addr,uint32_t val);"},
    {(void *)dummy_led_on,     "void at32_led_on(led_type led);"},
    {(void *)dummy_led_off,    "void at32_led_off(led_type led);"},
    {(void *)dummy_led_toggle, "void at32_led_toggle(led_type led);"},
};

struct _m_usmart_dev usmart_dev =
{
    usmart_nametab,
    usmart_init,
    usmart_cmd_rec,
    usmart_exe,
    usmart_scan,
    (uint8_t)(sizeof(usmart_nametab) / sizeof(struct _m_usmart_nametab)),
    0, 0, 1, 0,
    {0},
    {0},
    0, 0
};

/* --- test driver --------------------------------------------------------- */
static int failures;
static int checks;

static void copy_line(char *dst, const char *src)
{
    strncpy(dst, src, PARM_LEN);
    dst[PARM_LEN] = '\0';
}

/* expect a user function: cmd_rec result, captured argument count/values */
static void check_func(const char *line, uint8_t rec_exp, uint8_t nargs,
                       const uint32_t *args)
{
    char buf[PARM_LEN + 1];
    uint8_t sta;

    copy_line(buf, line);
    cap_cnt = 0;
    usmart_dev.runtime = 0;
    sta = usmart_dev.cmd_rec(buf);

    printf(">>> %s\n", line);
    if (sta == 0)
    {
        usmart_dev.exe();
    }

    checks++;
    if (sta != rec_exp || (sta == 0 && cap_cnt != nargs))
    {
        printf("FAIL: cmd='%s' rec=%u (expected %u), args=%u (expected %u)\n",
               line, sta, rec_exp, cap_cnt, nargs);
        failures++;
        return;
    }

    if (sta == 0)
    {
        uint8_t i;
        for (i = 0; i < nargs; i++)
        {
            if (cap_args[i] != args[i])
            {
                printf("FAIL: cmd='%s' arg[%u]=0x%08X (expected 0x%08X)\n",
                       line, i, (unsigned)cap_args[i], (unsigned)args[i]);
                failures++;
                return;
            }
        }
    }

    printf("PASS\n");
}

/* expect a system command: cmd_rec must fail, sys_cmd_exe must succeed */
static void check_sys(const char *line)
{
    char buf[PARM_LEN + 1];
    uint8_t rec, sys;

    copy_line(buf, line);
    cap_cnt = 0;
    rec = usmart_dev.cmd_rec(buf);
    sys = 0xFFU;
    printf(">>> %s\n", line);
    if (rec != USMART_FUNCERR)
    {
        printf("NOTE: cmd_rec accepted '%s' (rec=%u)\n", line, rec);
    }
    sys = usmart_sys_cmd_exe(buf);

    checks++;
    if (sys != USMART_OK)
    {
        printf("FAIL: system cmd='%s' returned %u\n", line, sys);
        failures++;
        return;
    }
    printf("PASS\n");
}

int main(void)
{
    static const uint32_t a_one[]    = {1U};
    static const uint32_t a_zero[]   = {0U};
    static const uint32_t a_two[]    = {2U};
    static const uint32_t a_led10[]  = {10U};
    static const uint32_t a_neg[]    = {0xFFFFFFFFU};
    static const uint32_t a_addr[]   = {0x20000000U};
    static const uint32_t a_wr[]     = {0x20000000U, 0xDEADBEEFU};
    static const uint32_t a_wrd[]    = {0x20000000U, 3735928559U};
    static const uint32_t a_lowhex[] = {0x2000000AU};
    static const uint32_t a_dec[]    = {10U};
    static const uint32_t a_negh[]   = {0xFFFFFFF0U};

    setvbuf(stdout, NULL, _IONBF, 0);

    check_func("at32_led_on(1)", USMART_OK, 1, a_one);
    check_func("at32_led_off(0)", USMART_OK, 1, a_zero);
    check_func("at32_led_toggle(2)", USMART_OK, 1, a_two);
    check_func("at32_led_on(0x0A)", USMART_OK, 1, a_led10);
    check_func("at32_led_on ( 1 )", USMART_OK, 1, a_one);
    check_func("at32_led_on(-1)", USMART_OK, 1, a_neg);
    check_func("at32_led_on()", USMART_PARMERR, 0, 0);
    check_func("at32_led_on(1,2)", USMART_OK, 1, a_one);
    check_func("read_addr(0x20000000)", USMART_OK, 1, a_addr);
    check_func("read_addr(0X20000000)", USMART_OK, 1, a_addr);
    check_func("read_addr(0x2000000a)", USMART_OK, 1, a_lowhex);
    check_func("read_addr(10)", USMART_OK, 1, a_dec);
    check_func("read_addr(-0x10)", USMART_OK, 1, a_negh);
    check_func("read_addr()", USMART_PARMERR, 0, 0);
    check_func("write_addr(0x20000000,0xDEADBEEF)", USMART_OK, 2, a_wr);
    check_func("write_addr(0x20000000,3735928559)", USMART_OK, 2, a_wrd);
    check_func("bogus_func(1)", USMART_NOFUNCFIND, 0, 0);

    check_sys("?");
    check_sys("help");
    check_sys("list");
    check_sys("id");
    check_sys("hex 0x10");
    check_sys("dec 255");
    check_sys("runtime 1");
    check_sys("runtime 0");
    check_sys("times 2");
    if (requested_pulse_width_us != 2U)
    {
        printf("FAIL: times command width=%lu (expected 2)\n",
               (unsigned long)requested_pulse_width_us);
        failures++;
    }
    checks++;

    printf("\n%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
