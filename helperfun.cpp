#include "helperfun.h"
#include <mutex>
#include <assert.h>


std::mutex print_mutex;
void _mhprintf(const char *type, const char *msg_format, va_list va_args, LOG_TYPE logtype)
{
    assert(type != nullptr && buf != nullptr);

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
