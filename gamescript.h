#ifndef MH_GAMESCRIPT_H
#define MH_GAMESCRIPT_H


#include <windows.h>
#include <lua.hpp>
#include <list>
#include <boost/lexical_cast.hpp>
#include <windows.h>
#include <string>
#include <shlwapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <opencv2/opencv.hpp>

#include "define.h"
#include "mh_config.h"


//表示一个控制窗口的脚本
class GameScriper
{

public:
    GameScriper(HWND game_wnd, int id);
    ~GameScriper(){
        lua_close(lua_status);
    }

    void mhprintf(const char *msg, ...);

    //做点啥
    void do_task();
    void do_money(){
        mhprintf("什么都没有..");
    }

    //进去游戏
    void Entry_game();
    PLAYER_STATUS Get_player_status();
    void Run();

    bool is_in_city(const char *city);

    void Regist_lua_fun();
    bool check_offline();
    void test_lua(const char* err);

public:
    GameConfig* Get_config()
    {
        return &config;
    }

private:
    GameConfig config;
    lua_State *lua_status;

    std::string player_name;   //玩家等级
    std::string player_level;
    std::list<std::string> lua_func_list;


public:
    static GameScriper* Get_instance(lua_State* L)
    {
        return inst_map[L];
    }


    //
    void add_lua_func(const char *funcname);
    void readLuaArray(lua_State *L);
private:
    static std::map<lua_State*, GameScriper*> inst_map;
    HWND wnd;
//    HDC hdc;
    int script_id;


    //右键点击, 攻击状态取消攻击
    void rclick(int x, int y);
    void rclick(const char *image);
    void click(int x, int y, bool lbutton = true);
    void click(const char *image);
    void click_nofix(int x, int y);
    void click_nofix(const char *image);
    void click_move(int x, int y, bool lbutton);


    //npc对话中的点击( 会设置焦点 )
    void Dialog_click(const char *img);


    //输入字符 WM_CHAR
    void input(const std::string & msg);

    //从屏幕匹配
    bool is_match_pic_in_screen(const char *image);
    bool is_match_pic_in_screen(const char *image, POINT &point);
    bool is_match_pic_in_rect(const char *image, POINT &point, const RECT &rect);

    //从文件匹配
    double Match_picture(const std::vector<uchar>& img1, const char* img2, cv::Point &maxLoc);

    //从内存匹配
    double Match_picture(const std::vector<uchar>& img1, const std::vector<uchar>& img2, cv::Point &maxLoc);


    POINT Get_cur_mouse();
    void Rand_move_mouse();
    void Until_stop_run();


    void input_event(const char *input);

    //注册lua
    void Regist_lua_fun(lua_State* lua_status);

public:
    HWND get_game_wnd(){
        return wnd;
    }

private:
    std::vector<int> Get_mouse_vec(int x, int y, int x2, int y2);
    int make_mouse_value(int x, int y);

public:
    std::vector<uchar> Get_screen_data();
    std::vector<uchar> Get_screen_data(const RECT &rcDC);
    bool Write_bmp_to_file(std::string file, const RECT &rect);


private:
    HDC hdc;
    std::vector<uchar> imgbuf;

    //鼠标内能移动的大小和窗口的比, 用来转换窗口座标到游戏内座标
    double ratio_x;
    double ratio_y;

    int rx;
    int ry;
    int cur_game_x;
    int cur_game_y;

public:
    void set_player_name(std::string name);
};


#endif // MH_GAMESCRIPT_H
