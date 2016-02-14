#ifndef SCRIPTAPP_H
#define SCRIPTAPP_H


#include <windows.h>
#include <vector>
#include <thread>
#include <string>
#include <map>

#include "define.h"
#include "config.h"


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
    
public:
    void mhprintf(const char *msg, ...);

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


void print_err_msg(DWORD msg);

#endif // SCRIPTAPP_H
