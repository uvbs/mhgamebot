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
            std::runtime_error("no instance ScriptApp");

        return _inst;
    }


public:
    void Run();
    int find_game_window();
};

#endif // SCRIPTAPP_H
