#include "helperfun.h"
#include <mutex>
#include <assert.h>


std::mutex print_mutex;
void _mhprintf(const char *type, const char *msg_format, va_list va_args, LOG_TYPE logtype)
{
    assert(type != nullptr && msg_format != nullptr);

    //取得互斥
    std::lock_guard<std::mutex> locker(print_mutex);

    if(logtype == LOG_TYPE::LOG_WARNING)
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),STD_COLOR::YELLOW );
    else if(logtype == LOG_TYPE::LOG_DEBUG)
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),STD_COLOR::GREEN );
    else if(logtype == LOG_TYPE::LOG_NORMAL)
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),STD_COLOR::GRAY);
    else if(logtype == LOG_TYPE::LOG_ERROR)
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),STD_COLOR::RED);
    else if(logtype == LOG_TYPE::LOG_INFO)
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),STD_COLOR::PINK);

    printf("%s: ", type);
    vprintf(msg_format, va_args);
    printf("\n");



    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),STD_COLOR::GRAY);
}


//安装自动启动
bool install_autorun()
{
    char buf[MAX_PATH];
    GetModuleFileNameA(NULL, buf, MAX_PATH);

    //写入注册表，以便开机自动运行
    HKEY hKey;
    bool result = false;

    //打开注册表
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey ) == ERROR_SUCCESS)
    {
        //新增一个值,名称随意命名,值为要开机运行的文件的完整路径
        RegSetValueExA(hKey, "mhbot", 0, REG_SZ, (const BYTE*)buf, MAX_PATH);
        //关闭注册表:
        RegCloseKey(hKey);

        result = true;
    }

    return result;
}

bool uninstall_autorun()
{

    char buf[MAX_PATH];
    GetModuleFileNameA(NULL, buf, MAX_PATH);

    bool result = false;
    HKEY hKey;

    //打开注册表:路径如下
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        //删除键值
        RegDeleteValueA(hKey, "mhbot");
        //关闭注册表
        RegCloseKey(hKey);

        result = true;
    }

    return result;
}

