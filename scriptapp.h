#ifndef SCRIPTAPP_H
#define SCRIPTAPP_H


#include <windows.h>
#include <vector>
#include <thread>


class ScriptApp
{
public:
    ScriptApp();

    //╪стьсно╥
    bool launcher_game(const char* username, const char* pw);

private:
    std::vector<HWND> Game_wnd_vec;
    std::vector<std::thread> Game_thread;
public:
    void Run();
    int find_game_window();
};

#endif // SCRIPTAPP_H
