#ifndef SCRIPTAPP_H
#define SCRIPTAPP_H


#include <windows.h>
#include <vector>
#include <thread>


#include "define.h"

class ScriptApp
{
public:
    ScriptApp();

    //加载游戏
    bool launcher_game(const char* username, const char* pw);

private:
    std::vector<HWND> Game_chat_vec;
    std::vector<HWND> Game_wnd_vec;
    std::vector<std::thread> Game_thread;

public:
    void mhprintf(const char *msg, ...);

public:
    void Run();
    int find_game_window(const std::string& classname);
    int hide_chat_window();
    HANDLE GetProcessHandle(int nID);
    void Close_all_game();
};

#endif // SCRIPTAPP_H
