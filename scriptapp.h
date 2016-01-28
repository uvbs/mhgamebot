#ifndef SCRIPTAPP_H
#define SCRIPTAPP_H


#include <windows.h>
#include <vector>
#include <thread>


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
    int find_game_window(char* classname);
    int hide_chat_window();
};

#endif // SCRIPTAPP_H
