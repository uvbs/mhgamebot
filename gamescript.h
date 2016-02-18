#ifndef MH_GAMESCRIPT_H
#define MH_GAMESCRIPT_H


#include <windows.h>
#include <lua.hpp>
#include <list>
#include <boost/lexical_cast.hpp>
#include <string>
#include <shlwapi.h>
#include <map>
#include <opencv2/opencv.hpp>

#include "define.h"
#include "config.h"


//表示一个控制窗口的脚本
class GameScript
{

public:
    GameScript(HWND game_wnd, int id);
    ~GameScript(){
        lua_close(lua_status);
    }

    void mhprintf(const char *msg, ...);

    //做点啥
    void do_daily();
    void do_task();
    void do_money(){
        mhprintf("什么都没有..");
    }

    //进去游戏
    void entry_game();
    PLAYER_STATUS get_player_status();
    void run();

    bool is_in_city(const char *city);

    void regist_lua_fun();
    bool check_offline();
    void test_lua(const char* err);


public:
    GameConfig* get_config(){
        return &config;
    }

private:
    GameConfig config;
    lua_State *lua_status;

    std::string player_name;   //玩家等级
    std::string player_level;
    std::list<std::string> lua_func_list;

    bool can_task = true;
    
public:
    static GameScript* get_instance(lua_State* L){
        return inst_map[L];
    }


    void readLuaArray(lua_State *L);
private:
    static std::map<lua_State*, GameScript*> inst_map;
    HWND wnd;
//    HDC hdc;
    int script_id;


    //右键点击, 攻击状态取消攻击
    void rclick(int x, int y);
    void rclick(const char *image);
    void click(int x, int y, int lbutton = 1);
    void click(const char *image, int threshold = 7);
    void click_nofix(int x, int y);
    void click_nofix(const char *image);
    void click_move(int x, int y, int lbutton);
    void click_nomove(int x, int y);

    //发送按键
    void key_press(int vk);

    //npc对话中的点击( 会设置焦点 )
    void dialog_click(const char *img);


    //输入字符 WM_CHAR
    void input(const std::string & msg);

    //从屏幕匹配
    bool is_match_pic_in_screen(const char *image, int threshold = 7);
    bool is_match_pic_in_screen(const char *image, POINT &point, int threshold = 7);   //参数2: 返回匹配到的POINT结构
    
    bool is_match_pic_in_rect(const char *image, const RECT &rect, int threshold = 7);
    bool is_match_pic_in_rect(const char *image, POINT &point, const RECT &rect, int threshold = 7);
    
    //从文件匹配
    double match_picture(const std::vector<uchar>& img1, const char* img2, cv::Point &maxLoc);

    //从内存匹配
    double match_picture(const std::vector<uchar>& img1, const std::vector<uchar>& img2, cv::Point &maxLoc);


    POINT get_cur_mouse();
    void rand_move_mouse();
    void until_stop_run(int counts = 1000);
    void top_wnd();

    void input_password(const char *input);

    //注册lua
    void regist_lua_fun(lua_State* lua_status);

public:
    HWND get_game_wnd(){
        return wnd;
    }

private:
    std::vector<int> get_mouse_vec(int x, int y, int x2, int y2);
    int make_mouse_value(int x, int y);

public:
    std::vector<uchar> get_screen_data();
    std::vector<uchar> get_screen_data(const RECT &rcClient);


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

    void close_game_wnd_stuff();
    void match_task();
public:
    void set_player_name(std::string name);
    void call_lua_func(const std::string& name);
    void load_lua_file(const char *name);
    void end_task();
};


#endif // MH_GAMESCRIPT_H
