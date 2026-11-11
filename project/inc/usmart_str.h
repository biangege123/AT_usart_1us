#ifndef __USMART_STR_H
#define __USMART_STR_H

#include "usmart_port.h"


uint8_t usmart_get_parmpos(uint8_t num);
uint8_t usmart_strcmp(char *str1,char *str2);
void usmart_strcopy(char *str1,char *str2);
uint8_t usmart_strlen(const char *str);
uint32_t usmart_pow(uint8_t m,uint8_t n);
uint8_t usmart_str2num(char *str,uint32_t *res);
uint8_t usmart_get_cmdname(char *str,char *cmdname,uint8_t *nlen,uint8_t maxlen);
uint8_t usmart_get_fname(char *str,char *fname,uint8_t *pnum,uint8_t *rval);
uint8_t usmart_get_aparm(char *str,char *fparm,uint8_t *ptype);
uint8_t usmart_get_fparam(char *str,uint8_t *parn);







#endif