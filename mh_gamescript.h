#ifndef MH_GAMESCRIPT_H
#define MH_GAMESCRIPT_H


#include <windows.h>
#include <lua.hpp>


#include "mh_define.h"
#include "mh_config.h"
#include "mh_mousekeyboard.h"

//��ʾһ�����ƴ��ڵĽű�
class GameScriper
{

public:
    GameScriper(HWND game_wnd, int id);
    ~GameScriper(){}

    void mhprintf(const char *msg, ...);

    void task1to10();

    //����ɶ
    void do_task();
    void do_money(){
        mhprintf("ʲô��û��..");
    }

    //��ȥ��Ϸ
    void Entry_game();
    PLAYER_STATUS Get_player_status();
    void Run();

    bool is_in_city(const char *city);

    void Regist_lua_fun();
    bool check_offline();
private:
    Mouse_keyboard mouse;
    GameConfig config;
    lua_State *lua_status;

    std::string player_name;   //��ҵȼ�
    std::string player_level;

    static GameScriper* Get_instance(lua_State* L)
    {
        return inst_map[L];
    }


    //
private:
    static std::map<lua_State*, GameScriper*> inst_map;
    HWND wnd;
//    HDC hdc;
    int script_id;
};


#endif // MH_GAMESCRIPT_H
