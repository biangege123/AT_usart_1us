#include "usmart_str.h"
#include "usmart.h"

#include <string.h>

/**
 * @brief       字符串比较
 * @param       str1: 字符串1
 *              str2: 字符串2
 * @retval      0, 相等; 1, 不相等
 */
uint8_t usmart_strcmp(char *str1, char *str2)
{
    while (1)
    {
        if (*str1 != *str2)
        {
            return 1;               /* 不相等 */
        }

        if (*str1 == '\0')
        {
            break;                  /* 结束 */
        }

        str1++;
        str2++;
    }

    return 0;
}

/**
 * @brief       字符串拷贝(包含结束符)
 * @param       str1: 目的字符串
 *              str2: 源字符串
 * @retval      无
 */
void usmart_strcopy(char *str1, char *str2)
{
    while (*str2 != '\0')
    {
        *str1 = *str2;
        str1++;
        str2++;
    }

    *str1 = '\0';
}

/**
 * @brief       得到字符串长度(不包括结束符)
 * @param       str: 字符串
 * @retval      字符串长度
 */
uint8_t usmart_strlen(const char *str)
{
    uint8_t len = 0;

    while (*str != '\0')
    {
        len++;
        str++;
    }

    return len;
}

/**
 * @brief       返回a^b的值
 * @param       a: 底数
 *              b: 指数
 * @retval      a^b
 */
uint32_t usmart_pow(uint8_t a, uint8_t b)
{
    uint32_t res = 1;

    while (b)
    {
        res *= a;
        b--;
    }

    return res;
}

/**
 * @brief       字符串转数字
 *   @note      支持10进制、16进制(必须0X开头), 支持正负号
 * @param       str: 源字符串
 *              res: 转换结果
 * @retval      0, 成功;
 *              1, 含有非法字符;
 *              2, 16进制数据长度不足(0X后无数据);
 *              3, 16进制数据未以0X开头;
 *              4, 无有效数字
 */
uint8_t usmart_str2num(char *str, uint32_t *res)
{
    uint32_t t;
    uint8_t bnum = 0;           /* 数字位数 */
    char *p;
    uint8_t hexdec = 10;        /* 默认10进制 */
    uint8_t sign = 0;           /* 负数标志 */

    p = str;
    *res = 0;

    /* 处理符号 */
    if (*p == '-')
    {
        sign = 1;
        p++;
    }
    else if (*p == '+')
    {
        p++;
    }

    /* 检查字符串是否全部为合法数字 */
    while (1)
    {
        if ((*p <= '9' && *p >= '0') || (*p <= 'F' && *p >= 'A') || (*p == 'X' && bnum == 1))
        {
            if (*p >= 'A')
            {
                hexdec = 16;    /* 存在A~F或者X, 按16进制处理 */
            }
            bnum++;
        }
        else if (*p == '\0')
        {
            break;
        }
        else
        {
            return 1;           /* 非法字符 */
        }
        p++;
    }

    p = str;
    if (sign)
    {
        p++;
    }

    if (hexdec == 16)
    {
        if (bnum < 3)
        {
            return 2;           /* 0X后面没有数据 */
        }
        if (*p == '0' && *(p + 1) == 'X')
        {
            p += 2;
            bnum -= 2;
        }
        else
        {
            return 3;           /* 16进制必须以0X开头 */
        }
    }
    else if (bnum == 0)
    {
        return 4;               /* 没有数字 */
    }

    while (1)
    {
        if (bnum)
        {
            bnum--;
        }

        if (*p <= '9' && *p >= '0')
        {
            t = *p - '0';
        }
        else
        {
            t = *p - 'A' + 10;
        }

        *res += t * usmart_pow(hexdec, bnum);
        p++;

        if (*p == '\0')
        {
            break;
        }
    }

    if (sign)
    {
        *res = (uint32_t)(0U - *res);    /* 负数取补码 */
    }

    return 0;
}

/**
 * @brief       得到指令名
 * @param       str: 源字符串
 *              cmdname: 指令名缓存
 *              nlen: 指令名长度
 *              maxlen: 缓存最大长度(包含结束符)
 * @retval      0, 成功; 其他, 失败
 */
uint8_t usmart_get_cmdname(char *str, char *cmdname, uint8_t *nlen, uint8_t maxlen)
{
    if (maxlen == 0)
    {
        return 1;
    }

    *nlen = 0;

    while (*str != ' ' && *str != '\0')
    {
        if (*nlen >= (uint8_t)(maxlen - 1U))
        {
            *cmdname = '\0';
            return 1;           /* 指令太长 */
        }

        *cmdname = *str;
        str++;
        cmdname++;
        (*nlen)++;
    }

    *cmdname = '\0';
    return 0;
}

/**
 * @brief       获取下一个字符(忽略中间的空格)
 * @param       str: 字符串指针
 * @retval      下一个字符
 */
static uint8_t usmart_search_nextc(char *str)
{
    str++;

    while (*str == ' ' && *str != '\0')
    {
        str++;
    }

    return (uint8_t)*str;
}

/**
 * @brief       从str中得到函数名
 * @param       str: 源字符串指针
 *              fname: 获取到的函数名
 *              pnum: 函数参数个数
 *              rval: 是否需要显示返回值(0,不需要;1,需要)
 * @retval      0, 成功; 其他, 错误代码
 */
uint8_t usmart_get_fname(char *str, char *fname, uint8_t *pnum, uint8_t *rval)
{
    uint8_t res;
    uint8_t fover = 0;          /* 括号深度 */
    char *strtemp;
    uint8_t offset = 0;
    uint8_t parmnum = 0;
    uint8_t temp = 1;
    char fpname[6];             /* void + X + '\0' */
    uint8_t fplcnt = 0;         /* 第一个参数的长度计数器 */
    uint8_t pcnt = 0;           /* 参数计数器 */
    uint8_t nchar;
    uint8_t fname_idx = 0;

    *rval = 0;

    /* 判断函数是否有返回值 */
    strtemp = str;
    while (*strtemp != '\0')
    {
        if (*strtemp != ' ' && (pcnt & 0x7F) < 5)
        {
            if (pcnt == 0)
            {
                pcnt |= 0x80;   /* 置位最高位,标记开始接收返回值类型 */
            }

            if (((pcnt & 0x7F) == 4) && (*strtemp != '*'))
            {
                break;          /* 最多记录5个字符, 最后一个字符必须是* */
            }

            fpname[pcnt & 0x7F] = *strtemp;
            pcnt++;
        }
        else if (pcnt == 0x85)
        {
            break;
        }
        strtemp++;
    }

    if (pcnt)
    {
        fpname[pcnt & 0x7F] = '\0';

        if (usmart_strcmp(fpname, "void") == 0)
        {
            *rval = 0;          /* 不需要返回值 */
        }
        else
        {
            *rval = 1;          /* 需要返回值 */
        }
        pcnt = 0;
    }

    res = 0;
    strtemp = str;

    while (*strtemp != '(' && *strtemp != '\0')     /* 找到函数名的真正起始位置 */
    {
        strtemp++;
        res++;

        if (*strtemp == ' ' || *strtemp == '*')
        {
            nchar = usmart_search_nextc(strtemp);

            if (nchar != '(' && nchar != '*')
            {
                offset = res;   /* 跳过空格和*号 */
            }
        }
    }

    strtemp = str;
    if (offset)
    {
        strtemp += offset + 1;  /* 跳到函数名开始的地方 */
    }

    res = 0;
    nchar = 0;                  /* 是否正在字符串里面的标志 */

    while (1)
    {
        if (*strtemp == '\0')
        {
            res = USMART_FUNCERR;   /* 函数错误 */
            break;
        }
        else if (*strtemp == '(' && nchar == 0)
        {
            fover++;                /* 括号深度增加一级 */
        }
        else if (*strtemp == ')' && nchar == 0)
        {
            if (fover)
            {
                fover--;
            }
            else
            {
                res = USMART_FUNCERR;   /* 错误结束, 没收到'(' */
            }

            if (fover == 0)
            {
                break;              /* 到末尾了, 退出 */
            }
        }
        else if (*strtemp == '"')
        {
            nchar = !nchar;
        }

        if (fover == 0)         /* 函数名还没接收完 */
        {
            if (*strtemp != ' ')    /* 空格不属于函数名 */
            {
                if (fname_idx >= (MAX_FNAME_LEN - 1U))
                {
                    res = USMART_FUNCERR;   /* 函数名太长 */
                    break;
                }

                fname[fname_idx] = *strtemp;
                fname_idx++;
            }
        }
        else                    /* 已经接收完函数名 */
        {
            if (*strtemp == ',')
            {
                temp = 1;       /* 使能增加一个参数 */
                pcnt++;
            }
            else if (*strtemp != ' ' && *strtemp != '(')
            {
                if (pcnt == 0 && fplcnt < 5)     /* 记录第一个参数的起始字符 */
                {
                    fpname[fplcnt] = *strtemp;
                    fplcnt++;
                }
                temp++;         /* 得到有效参数(非空格) */
            }

            if (fover == 1 && temp == 2)
            {
                temp++;         /* 防止重复增加 */
                parmnum++;      /* 参数增加一个 */
            }
        }

        strtemp++;
    }

    if (parmnum == 1)           /* 只有1个参数 */
    {
        fpname[fplcnt] = '\0';

        if (usmart_strcmp(fpname, "void") == 0)
        {
            parmnum = 0;        /* 参数为void, 表示没有参数 */
        }
    }

    *pnum = parmnum;            /* 记录参数个数 */
    fname[fname_idx] = '\0';    /* 加入结束符 */

    return res;
}


/**
 * @brief       得到一个参数的字符串
 *   @note      支持数字、字符串, 数字支持正负号
 * @param       str: 源字符串
 *              fparm: 参数缓存(至少PARM_LEN+1字节)
 *              ptype: 参数类型(0,数字;1,字符串;0XFF,错误)
 * @retval      源字符串偏移量
 */
uint8_t usmart_get_aparm(char *str, char *fparm, uint8_t *ptype)
{
    uint8_t i = 0;
    uint8_t enout = 0;
    uint8_t type = 0;           /* 默认是数字 */
    uint8_t string = 0;         /* 标记是否正在读字符串 */
    uint8_t out = 0;            /* 已写入fparm的字节数 */

    while (1)
    {
        if (*str == ',' && string == 0)
        {
            enout = 1;          /* 暂缓立即退出, 目的是寻找下一个参数的起始地址 */
        }

        if ((*str == ')' || *str == '\0') && string == 0)
        {
            break;              /* 立即退出 */
        }

        if (type == 0)          /* 默认是数字 */
        {
            if ((*str >= '0' && *str <= '9') || (*str >= 'a' && *str <= 'f') ||
                (*str >= 'A' && *str <= 'F') || *str == 'X' || *str == 'x' ||
                *str == '-' || *str == '+')
            {
                if (enout)
                {
                    break;      /* 找到了下一个参数, 直接退出 */
                }

                if (out < (PARM_LEN - 1U))
                {
                    if (*str >= 'a')
                    {
                        *fparm = *str - 0x20;    /* 小写转换为大写 */
                    }
                    else
                    {
                        *fparm = *str;
                    }

                    fparm++;
                    out++;
                }
            }
            else if (*str == '"')    /* 找到字符串的开始标志 */
            {
                if (enout)
                {
                    break;
                }

                type = 1;
                string = 1;     /* 标记正在读字符串 */
            }
            else if (*str != ' ' && *str != ',')
            {
                type = 0xFF;    /* 非法字符, 参数错误 */
                break;
            }
        }
        else                    /* 字符串类 */
        {
            if (*str == '"')
            {
                string = 0;
            }

            if (enout)
            {
                break;
            }

            if (string)         /* 字符串正在读 */
            {
                if (*str == '\\')   /* 遇到转义符 */
                {
                    if (*(str + 1) == '\0')
                    {
                        type = 0xFF;    /* 转义符后没有字符 */
                        break;
                    }

                    str++;          /* 偏移到转义符后面的字符 */
                    i++;
                }

                if (out < (PARM_LEN - 1U))
                {
                    *fparm = *str;
                    fparm++;
                    out++;
                }
            }
        }

        i++;                    /* 偏移量增加 */
        str++;
    }

    *fparm = '\0';              /* 加入结束符 */
    *ptype = type;              /* 返回参数类型 */
    return i;                   /* 返回参数长度 */
}


/**
 * @brief       得到指定参数的起始地址
 * @param       num: 第num个参数, 范围0~9
 * @retval      该参数的起始地址
 */
uint8_t usmart_get_parmpos(uint8_t num)
{
    uint8_t temp = 0;
    uint8_t i;

    for (i = 0; i < num; i++)
    {
        temp += usmart_dev.plentbl[i];
    }

    return temp;
}

/**
 * @brief       从str中得到函数参数
 * @param       str: 源字符串
 *              parn: 参数的多少(0表示无参数void类型)
 * @retval      0, 成功; 其他, 错误代码
 */
uint8_t usmart_get_fparam(char *str, uint8_t *parn)
{
    uint8_t i, type;
    uint32_t res;
    uint8_t n = 0;
    uint16_t len;
    char tstr[PARM_LEN + 1];    /* 最多存放PARM_LEN个字符的字符串 */

    for (i = 0; i < MAX_PARM; i++)
    {
        usmart_dev.plentbl[i] = 0;  /* 清空参数长度表 */
    }

    while (*str != '(')         /* 偏移到参数开始的地方 */
    {
        str++;

        if (*str == '\0')
        {
            return USMART_FUNCERR;
        }
    }

    str++;                      /* 偏移到"("之后的第一个字节 */

    while (1)
    {
        i = usmart_get_aparm(str, tstr, &type);     /* 得到第一个参数 */
        str += i;                                   /* 偏移 */

        switch (type)
        {
            case 0:             /* 数字 */
                if (tstr[0] != '\0')    /* 接收到的参数有效 */
                {
                    i = usmart_str2num(tstr, &res); /* 记录该参数 */

                    if (i)
                    {
                        return USMART_PARMERR;      /* 参数错误 */
                    }

                    if (n >= MAX_PARM)
                    {
                        return USMART_PARMOVER;     /* 参数太多 */
                    }

                    if ((uint16_t)usmart_get_parmpos(n) + 4U > PARM_LEN)
                    {
                        return USMART_PARMOVER;
                    }

                    memcpy(usmart_dev.parm + usmart_get_parmpos(n), &res, sizeof(res));
                    usmart_dev.parmtype &= (uint16_t)~(1U << n);    /* 标记数字 */
                    usmart_dev.plentbl[n] = 4;      /* 该参数的长度为4 */
                    n++;                            /* 参数增加 */
                }
                break;

            case 1:             /* 字符串 */
                len = (uint16_t)usmart_strlen(tstr) + 1U;  /* 包含结束符 */

                if (n >= MAX_PARM)
                {
                    return USMART_PARMOVER;     /* 参数太多 */
                }

                if ((uint16_t)usmart_get_parmpos(n) + len > PARM_LEN)
                {
                    return USMART_PARMOVER;     /* 参数缓冲区溢出 */
                }

                usmart_strcopy((char *)(usmart_dev.parm + usmart_get_parmpos(n)), tstr);
                usmart_dev.parmtype |= (uint16_t)(1U << n);     /* 标记字符串 */
                usmart_dev.plentbl[n] = (uint8_t)len;           /* 该参数的长度 */
                n++;                            /* 参数增加 */
                break;

            case 0xFF:          /* 错误 */
                return USMART_PARMERR;

            default:
                return USMART_PARMERR;
        }

        if (*str == ')' || *str == '\0')
        {
            break;              /* 查到结束标志了 */
        }
    }

    *parn = n;                  /* 记录参数的个数 */
    return USMART_OK;
}

