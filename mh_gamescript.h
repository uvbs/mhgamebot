#ifndef MH_GAMESCRIPT_H
#define MH_GAMESCRIPT_H


#include <windows.h>

#include "mh_define.h"
#include "mh_function.h"
#include "mh_mousekeyboard.h"

//表示一个控制窗口的脚本
class GameScriper
{

public:
    GameScriper(HWND game_wnd);
    ~GameScriper(){}



    void do_task();
    void do_money(){}

    //进去游戏
    void Entry_game();
    Player_status Get_player_status();
    void Run();

    bool is_in_city(const char *city);



private:
    Mouse_keyboard mouse;
    GameConfig config;
    //
private:
    HWND wnd;
};


#endif // MH_GAMESCRIPT_H
