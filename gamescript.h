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
#include <mutex>
#include <condition_variable>



#include "../control/protocol.h"

#include "define.h"
#include "config.h"

using help_fun = std::function<bool(DAMA_PARAM*, const char*, int)>;
using output_fun = std::function<void(int type, char*)>;


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
    bool send_pic_to_helper(DAMA_PARAM *param, const char* data, int len);
    void recv_help_answer(int x, int y);

    //进去游戏
    void entry_game();
    PLAYER_STATUS get_player_status();
    void start(std::string script_name);

    bool is_in_city(std::string city);

    void regist_lua_fun();
    bool check_offline();

    void set_player_name(std::string name);
    void call_lua_func(std::string func);
    bool do_script(std::string filename);
    void end_task();


    static GameScript* get_instance(lua_State* L){
        return inst_map[L];
    }



    //右键点击, 攻击状态取消攻击
    void rclick(int x, int y);
    void rclick(const char *image);
    void click(int x, int y, int lbutton = 1);
    void click(const char *image, int threshold = 7);
    void click_nofix(int x, int y);
    void click_nofix(const char *image);
    void click_nomove(int x, int y);

    //发送按键
    void key_press(int vk);
    void key_press(std::string key);

    //npc对话中的点击( 会设置焦点 )
    void dialog_click(const char *img);


    //输入字符 WM_CHAR
    void input(const std::string & msg);

    //从屏幕匹配
    bool is_match_pic_in_screen(std::string image, RECT rect = rect_game, int threshold = DEFAULT_THERSHOLD);
    bool is_match_pic_in_screen(std::string image, POINT &point, RECT rect = rect_game, int threshold = DEFAULT_THERSHOLD);   //参数2: 返回匹配到的POINT结构

    //匹配
    double match_picture(const std::vector<uchar>& img1, std::string img2, cv::Point &matchLoc);
    double _match_picture(const cv::Mat& screen, const cv::Mat& pic, cv::Point &matchLoc);
    double match_picture(const std::vector<uchar>& img1, const std::vector<uchar>& img2, cv::Point &matchLoc);
    double match_picture(const std::vector<uchar>& img1, const cv::Mat& pic, cv::Point &matchLoc);


    POINT get_cur_game_mouse();
    POINT get_cur_mouse();
    void rand_move_mouse();
    void until_stop_run(int counts = 1000);
    void top_wnd();

    void input_password(const char *input);

    HWND get_game_wnd(){
        return wnd;
    }


    const std::vector<uchar>& get_screen_data(const RECT& rect = rect_game);



    void read_global(bool read);
    void slow_click(int x1, int y1, int lbutton);

private:
    void get_mouse_vec(int x, int y, int x2, int y2, std::vector<int>& r);


    //获得焦点的互斥
    static std::mutex topwnd_mutex;
    std::string _script_name;


private:
    BYTE *screen_buf;
    static std::map<lua_State*, GameScript*> inst_map;
    HWND wnd;
    int script_id;
    lua_State *lua_status;

    std::string player_name;   //玩家等级
    std::string player_level;
    std::list<std::string> lua_task_list;

    bool task_running = false;

    HDC hdc;
    std::vector<uchar> _screen_data;

    //鼠标内能移动的大小和窗口的比, 用来转换窗口座标到游戏内座标
    double ratio_x;
    double ratio_y;

    int rx;
    int ry;
    int cur_game_x;
    int cur_game_y;

    void close_game_wnd_stuff();

    //从脚本读取的可用任务列表中匹配任务, 匹配到返回true, 没有返回false
    bool match_task();
    bool _match_task(std::string imgname);

    void process_pic_task(cv::Mat &src, cv::Mat& result);
    void process_pic_task_redline(cv::Mat& src, cv::Mat& result);
    void process_pic_mouse(cv::Mat& src, cv::Mat& result);
    void process_pic_mouse1(cv::Mat& src, cv::Mat& result);


    void check_pic_exists(std::string &imgfile);
    bool find_red_line(std::string image, POINT &point, RECT rect);
    const std::vector<uchar> &screen_data();

    output_fun output_callback;
    help_fun help_callback;
    void mhsleep(int ms, bool chk_status = true);

    void set_helper_event(){
        helper_event = false;
        //让等待的线程继续执行
    }
    bool get_helper_event(){
        return helper_event;
    }

    static cv::Mat mouse1;
    static cv::Mat mouse2;
    static cv::Mat mouse3;
    static cv::Mat mouse4;
    static std::once_flag just_once_read;

    //事件
private:
     bool helper_event;
};


#endif // MH_GAMESCRIPT_H
