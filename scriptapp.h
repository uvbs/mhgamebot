#ifndef SCRIPTAPP_H
#define SCRIPTAPP_H


#include <windows.h>
#include <vector>
#include <thread>
#include <string>
#include <map>


#include "define.h"
#include "config.h"

enum STD_COLOR {
    DARKBLUE = 1,
    DARKGREEN,
    DARKTEAL,
    DARKRED,
    DARKPINK,
    DARKYELLOW,
    GRAY,
    DARKGRAY,
    BLUE,
    GREEN,
    TEAL,
    RED,
    PINK,
    YELLOW,
    WHITE
};

enum LOG_TYPE{
    LOG_WARNING,  //警告
    LOG_DEBUG,  //调试
    LOG_NORMAL, //正常
    LOG_ERROR,  //错误
    LOG_INFO     //信息
};


class ScriptApp
{
public:
    ScriptApp();

    //加载游戏
    bool launcher_game();

private:
    std::vector<HWND> game_wnds;
    std::vector<std::thread> game_threads;
    std::map<std::string, std::string> game_accounts;
    GameConfig config;

    void read_config();
public:
    void mhprintf(LOG_TYPE logtype, const char *msg, ...);

public:
    void run();
    void read_accounts();
    int find_game_window(const std::string& classname);
    std::string find_game_path();
    int hide_chat_window();
    HANDLE get_process_handle(int nID);
    void close_all_game();
    void list_window(); //排列游戏窗口
};

void _mhprintf(const char* type, const char* buf, va_list va_args, LOG_TYPE logtype);




#endif // SCRIPTAPP_H
