#ifndef SCRIPTAPP_H
#define SCRIPTAPP_H


#include <windows.h>
#include <vector>
#include <thread>
#include <list>

#include "../mh_define.h"

class ScriptApp
{
public:
    ScriptApp();

    //º”‘ÿ”Œœ∑
    bool launcher_game(const char* username, const char* pw);

private:
    std::vector<HWND> Game_chat_vec;
    std::vector<HWND> Game_wnd_vec;
    std::vector<std::thread> Game_thread;

public:
    std::list<GAME_WND_INFO> Get_game_wnd_list()
    {
        std::list<GAME_WND_INFO> infolist;
        find_game_window(GAME_WND_CLASS);
        for(auto i = Game_wnd_vec.begin(); i != Game_wnd_vec.end(); i++)
        {
            GAME_WND_INFO wndinfo;
            DWORD pid;
            ::GetWindowThreadProcessId(*i, &pid);
            wndinfo.wnd = *i;
            wndinfo.pid =pid;
            infolist.push_back(wndinfo);
        }

        return infolist;
    }

public:
    void mhprintf(const char *msg, ...);

private:
    //µ•¿˝
    static ScriptApp* _inst;

public:
    static ScriptApp* GetInstance(){
        if(_inst == nullptr)
            throw std::runtime_error("no instance ScriptApp");

        return _inst;
    }


public:
    void Run();
    int find_game_window(const std::string& classname);
    int hide_chat_window();
    HANDLE GetProcessHandle(int nID);
    void Close_all_game();
};

#endif // SCRIPTAPP_H
