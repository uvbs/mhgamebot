#ifndef MH_GAMESCRIPT_H
#define MH_GAMESCRIPT_H


#include <windows.h>
#include <lua.hpp>


#include "mh_define.h"
#include "mh_function.h"
#include "mh_mousekeyboard.h"

//��ʾһ�����ƴ��ڵĽű�
class GameScriper
{

public:
    GameScriper(HWND game_wnd, int id);
    ~GameScriper(){}


    void task1to10();

    //����ɶ
    void do_task();
    void do_money(){
        MH_printf("ʲô��û��..");
    }

    //��ȥ��Ϸ
    void Entry_game();
    Player_status Get_player_status();
    void Run();

    bool is_in_city(const char *city, bool screen_exist = false);

    static GameScriper* GetInstance(){
        if(_inst == nullptr)
            std::runtime_error("û��ʼ���� GameScript");
        return _inst;
    }

    static GameScriper* _inst;

    void Regist_lua_fun();
private:
    Mouse_keyboard mouse;
    GameConfig config;
    lua_State *lua_status;
    //
private:
    HWND wnd;
    HDC hdc;
    int script_id;
};


#endif // MH_GAMESCRIPT_H
