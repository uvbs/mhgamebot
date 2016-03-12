#ifndef HELPERFUN_H
#define HELPERFUN_H

#include "define.h"

#include <Shlwapi.h>

void _mhprintf(const char* type, const char* msg_format, va_list va_args, LOG_TYPE logtype);

//安装自动启动
bool install_autorun();

//卸载自动启动
bool uninstall_autorun();


void mhsleep(int ms);

#endif // HELPERFUN_H

