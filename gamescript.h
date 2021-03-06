﻿#ifndef MH_GAMESCRIPT_H
#define MH_GAMESCRIPT_H


#include <windows.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <list>
#include <boost/lexical_cast.hpp>
#include <string>
#include <shlwapi.h>
#include <map>
#include <opencv2/opencv.hpp>
#include <mutex>
#include <condition_variable>



#include "../control/protocol.h"

#include "define.h"
#include "config.h"

using help_fun = std::function<bool(const char*, int)>;
using output_fun = std::function<void(int type, const char*)>;


//表示一个控制窗口的脚本
class GameScript
{

public:
    GameScript(HWND game_wnd, int id);
    ~GameScript();

    int get_id(){
        return script_id;
    }

    void mhprintf(LOG_TYPE logtype, const char *msg_format, ...);

    //设置输出信息的回调
    void set_output_callback(output_fun _callback);

    //设置发送到人工服务器的回调
    void set_sendhelp_callback(help_fun callback);

    //发送图片到人工服务器
    bool send_pic_to_helper(const char* data, int len){
        return help_callback(data, len);
    }


    PLAYER_STATUS get_player_status();
    void start(std::string script_name);

    bool is_in_city(std::string city);

    void regist_lua_fun();
    bool check_offline();

    void set_player_name(std::string name);
    void set_account(std::string acc){
        mhprintf(LOG_INFO, "set account %s", acc.c_str());
        player_account = acc;
    }

    void set_password(std::string pw){
        mhprintf(LOG_INFO, "set password %s", pw.c_str());
        player_password = pw;
    }

    void call_lua_func(std::string func);
    void do_script(std::string filename);
    void end_task();


    static GameScript* get_instance(lua_State* L){
        return inst_map[L];
    }



    //右键点击, 攻击状态取消攻击
    void rclick(const char *image);
    void _click(int x, int y, int lbutton = MOUSE_LEFT_CLICK);
    void click(int x, int y, int lbutton = MOUSE_LEFT_CLICK);
    void only_move(int x, int y);
    void click(const std::string& image, double threshold = DEFAULT_THERSHOLD, bool check_exists = true);
    void click(const std::string &image, int offset_x, int offset_y, double thershold = DEFAULT_THERSHOLD, bool check_exists = true);

    void click_nofix(int x, int y);
    void click_nofix(const char *image);
    void click_nomove(int x, int y, bool bclick = true);

    //发送按键
    void key_press(std::string key);


    //输入字符 WM_CHAR
    void input(const std::string & msg);

    //从屏幕匹配
    bool is_match_pic_in_screen(std::string image, const RECT& rect = rect_game, double threshold = DEFAULT_THERSHOLD);
    bool is_match_pic_in_screen(std::string image, POINT &point, const RECT& rect = rect_game, double threshold = DEFAULT_THERSHOLD);   //参数2: 返回匹配到的POINT结构

    //匹配
    double match_picture(const std::vector<uchar>& img1, std::string img2, cv::Point &matchLoc);
    double _match_picture(const cv::Mat& screen, const cv::Mat& pic, cv::Point &matchLoc);
    double match_picture(const std::vector<uchar>& img1, const std::vector<uchar>& img2, cv::Point &matchLoc);
    double match_picture(const std::vector<uchar>& img1, const cv::Mat& pic, cv::Point &matchLoc);


    //坐标相关, 用来抓鬼
    std::string get_task_xy();
    std::string get_map_xy();

    //

    POINT get_cur_game_mouse();
    POINT get_cur_mouse();
    void move_mouse_vec(int x, int y, int tar_x, int tar_y);
    void rand_move_mouse();
    void until_stop_run(int counts = 1000);
    void top_wnd();

    void input_password(const char *input);

    HWND get_game_wnd(){
        return wnd;
    }

    void cache_folder_png();
    const std::vector<uchar>& get_screen_data(const RECT& rect = rect_game);



    void read_global(bool read);
    void slow_click(int x1, int y1, int lbutton = 1);

    void set_help_ok(){
        recv_help = true;
    }

    void pass_static_check();
private:
    void get_mouse_vec(int x, int y, int x2, int y2, std::vector<int>& r);

    void entry_game();
    bool recv_help;  //请求帮助被处理的标志
    std::string _script_name;
    PLAYER_STATUS last_player_status;

private:
    BYTE *screen_buf;
    std::vector<uchar> _screen_data;

    static std::map<lua_State*, GameScript*> inst_map;
    HWND wnd;
    HANDLE game_process;
    int script_id;
    lua_State *lua_status;

    std::string player_account; //玩家账户
    std::string player_password; //玩家密码
    std::string player_name;   //玩家等级
    std::string player_level;
    std::list<std::string> lua_task_list;

    bool task_running = false;

    HDC hdc;

    //鼠标内能移动的大小和窗口的比, 用来转换窗口座标到游戏内座标
    double ratio_x;
    double ratio_y;

    int rx;
    int ry;
    int cur_game_x;
    int cur_game_y;


    //从脚本读取的可用任务列表中匹配任务, 匹配到返回true, 没有返回false
    bool match_task();
    double _match_task(std::string imgname, cv::Point& matchLoc);

    void process_pic_task(const cv::Mat &src, cv::Mat& result);
    void process_pic_task_redline(const cv::Mat& src, cv::Mat& result);
    void process_pic_mouse(cv::Mat& src, cv::Mat& result);
    void process_pic_mouse1(cv::Mat& src, cv::Mat& result);
    void process_pic_door(cv::Mat& src);        //传送门

    void check_pic_exists(std::string& imgfile);
    bool find_red_line(std::string image, POINT &point, RECT rect);
    const std::vector<uchar>& screen_data();


    output_fun output_callback;
    help_fun help_callback;
    void mhsleep(int ms, bool chk_status = true);

    static cv::Mat mouse1;
    static cv::Mat mouse2;
    static cv::Mat mouse3;
    static cv::Mat mouse4;


    //这个映射来优化速度, 避免大量的读图片造成的IO
    //方式就是把所有图片缓存到内存中
    //TODO:
    static std::map<std::string, cv::Mat> _pic_data;

    //所有客户端匹配抓鬼任务, 匹配到的共享到这里
    //TODO:
    static std::string zg_map;  //抓鬼的地图
    static std::string zg_xy;   //共享的坐标

    void wait_appear(std::string name, RECT rect = rect_game, double threshold = DEFAULT_THERSHOLD);
    std::string _get_xy_string(std::string dir, const RECT& rect);
};


#endif // MH_GAMESCRIPT_H
