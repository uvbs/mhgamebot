﻿#include "gamescript.h"
#include <regex>
#include <thread>
#include <chrono>
#include <boost/lexical_cast.hpp>
#include "helperfun.h"
#include <QDebug>

#define  script_inst  GameScript::get_instance(L)





//静态初始化
std::map<lua_State*, GameScript*> GameScript::inst_map;
//获得焦点的互斥
std::mutex topwnd_mutex;
cv::Mat GameScript::mouse1;
cv::Mat GameScript::mouse2;
cv::Mat GameScript::mouse3;
cv::Mat GameScript::mouse4;

std::once_flag GameScript::just_once_read;

//构造函数
GameScript::GameScript(HWND game_wnd, int id):
    player_name(std::string("窗口")+boost::lexical_cast<std::string>(id)),
    output_callback(nullptr)
{
    script_id = id;
    wnd = game_wnd;
    lua_status = nullptr;
    if(lua_status == nullptr){
        lua_status = luaL_newstate();
    }

    if(lua_status == nullptr)
        throw std::runtime_error("创建lua状态失败");

    //默认的标准库
    luaL_openlibs(lua_status);

    //将这个lua状态和这个实例绑定
    inst_map[lua_status] = this;
    screen_buf = new BYTE[2048000];

    hdc = ::GetDC(game_wnd);
    script_id = id;

    ratio_x = (double)590/(double)SCREEN_WIDTH;
    ratio_y = (double)450/(double)SCREEN_HEIGHT;

    rx = 0;
    ry = 0;
    cur_game_x = 0;
    cur_game_y = 0;
}

GameScript::~GameScript()
{
    delete []screen_buf;
    lua_close(lua_status);
    ReleaseDC(wnd,hdc);
}

void GameScript::mhprintf(LOG_TYPE logtype,const char* msg_format, ...)
{
    assert(output_callback != nullptr);

    va_list va_args;
    va_start(va_args, msg_format);
    //_mhprintf(player_name.c_str(), msg, va_args, logtype);

    char buf[400];
    vsprintf(buf, msg_format, va_args);
    std::string msgbuf(buf);
    msgbuf.insert(0, player_name + ": ");

    //转换编码
    output_callback(logtype, QString::fromLocal8Bit(msgbuf.c_str()).toStdString().c_str());

    va_end(va_args);
}

void GameScript::set_output_callback(output_fun _callback)
{
    output_callback = _callback;
}

bool GameScript::send_pic_to_helper(DAMA_PARAM *param, const char *data, int len)
{
    bool result = help_callback(param, data, len);
    if(result == false) //可能帮助回调处理失败了
    {
        //应该报告一个严重的错误, 让外挂停止当前任务
        //TODO

    }

    return result;
}

void GameScript::recv_help_answer(int x, int y)
{
    click(x, y);
}

void GameScript::set_sendhelp_callback(help_fun callback)
{
    help_callback = callback;
}


bool GameScript::is_in_city(std::string city)
{
    city.insert(0, "map\\");

    POINT pt;
    if(is_match_pic_in_screen(city, pt))
    {
        mhprintf(LOG_NORMAL, "当前城市 %s..", city.c_str());
        return true;
    }

    return false;
}


//检测离线, 网络断线
bool GameScript::check_offline()
{

    HWND wnd = FindWindowExA(NULL, NULL, nullptr, "网络错误");
    if(wnd)
    {
        mhprintf(LOG_NORMAL, "检测到网络断线...");
        mhprintf(LOG_NORMAL, "尝试重启..");

        ::SendMessageA(wnd, WM_COMMAND, 1, 0);
        return true;
    }


    return false;
}


void GameScript::call_lua_func(std::string func)
{
    if(func.find("()") == std::string::npos)
        func += "()";

    if(0 != luaL_dostring(lua_status, func.c_str())){

        const char * err = lua_tostring(lua_status, -1);
        lua_pop(lua_status, 1);
        throw std::runtime_error(err);
    }
}


void GameScript::read_global(bool read)
{
    lua_pushglobaltable(lua_status);
    /* table is in the stack at index 't' */
    lua_pushnil(lua_status);  /* first key */
    while (lua_next(lua_status, -2) != 0) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        std::string name = lua_tostring(lua_status, -2);

        if(read){
            lua_task_list.push_back(name);
        }
        else{

            //筛选多余的
            //删掉已经存在的, 只压入不存在的就可以了
            auto it = std::find(lua_task_list.begin(), lua_task_list.end(), name);
            if(it != lua_task_list.end()){
                lua_task_list.erase(it);
            }
            else{
                if(name.find("通用_") == std::string::npos &&
                        name.find("脚本") == std::string::npos){
                    lua_task_list.push_back(name);
                }
            }
        }


        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(lua_status, 1);
    }

}

void GameScript::do_script(std::string filename)
{
    mhprintf(LOG_NORMAL, "加载脚本: %s", filename.c_str());

    //加载任务脚本
    if(0 != luaL_dofile(lua_status, filename.c_str()))
    {
        const char * err = lua_tostring(lua_status, -1);
        lua_pop(lua_status, 1);
        throw std::runtime_error(err);
    }
}


bool GameScript::match_task()
{
    bool finded = false;
    for(auto name :lua_task_list)
    {
        //backup
        std::string taskname(name);
        check_pic_exists(taskname);

        if(_match_task(taskname.c_str())){
            call_lua_func(name.c_str());
            finded = true;
            break;
        }
    }

    return finded;
}

bool GameScript::_match_task(std::string imgname)
{

    bool match = false;
    cv::Mat matchscreen = cv::imdecode(
                get_screen_data(rect_task),
                cv::IMREAD_COLOR);
    cv::Mat matchpic = cv::imread(imgname.c_str(), cv::IMREAD_COLOR);


    cv::Mat result_screen;
    cv::Mat result_pic;
    //处理成所有白字
    process_pic_task(matchscreen, result_screen);
    process_pic_task(matchpic, result_pic);

    cv::Point matchLoc;
    double maxVal = _match_picture(result_screen, result_pic, matchLoc);


    //TODO
    if(maxVal >= 0.90){
        mhprintf(LOG_INFO,"任务: %s, \t\t匹配结果 %f", imgname.c_str(), maxVal);
        match = true;
    }
    else{
        match = false;
    }

    return match;
}


//获取玩家状态
PLAYER_STATUS GameScript::get_player_status()
{
    if(is_match_pic_in_screen("战斗中.png", rect_game, 0.8)){
        return PLAYER_STATUS::COMBAT;
    }
    else if(is_match_pic_in_screen("游戏内.png", rect_game, 0.8)){
        return PLAYER_STATUS::NORMAL;
    }
    else if(is_match_pic_in_screen("体验状态.png", rect_game, 0.8)){
        mhprintf(LOG_INFO, "体验状态");
        return PLAYER_STATUS::NOTIME;
    }
    else if(is_match_pic_in_screen("跳过动画.png", rect_game, 0.8)){
        mhprintf(LOG_INFO, "跳过动画");
        return PLAYER_STATUS::GC;
    }
    else{
        mhprintf(LOG_INFO, "位置状态, 假装成正常状态");
        return PLAYER_STATUS::NORMAL;
    }

}


//结束任务
void GameScript::end_task()
{
    task_running = false;
}

void GameScript::top_wnd()
{
    ::SetForegroundWindow(wnd);
    mhsleep(WAIT_NORMAL);
}

//包括红色和白色都处理成白色, 其余颜色处理成黑色
void GameScript::process_pic_task(cv::Mat &src, cv::Mat& result)
{
    cv::Mat hsv;
    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
    inRange(hsv, cv::Scalar(0, 0, 180), cv::Scalar(179, 255, 255), result);
}

//除红色之外的其他颜色都处理成黑色
void GameScript::process_pic_task_redline(cv::Mat &src, cv::Mat& result)
{
    //转换到hsv
    cv::Mat hsv;
    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
    inRange(hsv, cv::Scalar(0, 90, 90), cv::Scalar(5, 255, 255), result);
}

void GameScript::process_pic_mouse(cv::Mat& src, cv::Mat& result)
{
    cv::Mat hsv;
    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
    inRange(hsv, cv::Scalar(0, 180, 150), cv::Scalar(20, 255, 255), result);
}

void GameScript::process_pic_mouse1(cv::Mat& src, cv::Mat& result)
{
    cv::Mat hsv;
    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
    inRange(hsv, cv::Scalar(5, 0, 0), cv::Scalar(156, 255, 255), result);
}

//检测图片存在
void GameScript::check_pic_exists(std::string & imgfile)
{
    if(imgfile.find("pic\\") == std::string::npos){
        imgfile.insert(0, "pic\\");
    }

    if(imgfile.find(".png") == std::string::npos){
        imgfile += ".png";
    }

    if(::PathFileExistsA(imgfile.c_str()) == FALSE){
        char buf[200];
        sprintf(buf, "文件 %s 不存在", imgfile.c_str());
        throw std::runtime_error(buf);
    }
}



void GameScript::regist_lua_fun()
{
    REGLUADATA(lua_status, COMBAT, "战斗状态");
    REGLUADATA(lua_status, NORMAL, "正常状态");
    REGLUADATA(lua_status, GC, "动画状态");
    REGLUADATA(lua_status, UNKNOW, "未知状态");



    REGLUAFUN(lua_status, "获得焦点", [](lua_State* L)->int{
        script_inst->top_wnd();
        return 0;
    });

    
    REGLUAFUN(lua_status, "点击对话框内图片", [](lua_State* L)->int{
        int arg_counts = lua_gettop(L);
        std::string img;
        int offset_x = 0;
        int offset_y = 0;

        if(arg_counts == 1){
            img = lua_tostring(L, 1);
        }
        else if(arg_counts == 3){
            img = lua_tostring(L, 1);
            offset_x = lua_tointeger(L, 2);
            offset_y = lua_tointeger(L, 3);
        }
        else{
            luaL_error(L, "点击对话框内图片 参数数量错误");
        }

        std::lock_guard<std::mutex> locker(topwnd_mutex);
        script_inst->top_wnd();

        img.insert(0, "对话框/");
        int times = WAIT_TIMES;
        int click_times = 0;
        POINT pt;

        script_inst->mhprintf(LOG_INFO, "点击对话框内图片 pic:%s", img.c_str());

        std::vector<uchar> a1;
        RECT click_area;

        while(times){

            if(script_inst->is_match_pic_in_screen(img, pt)){
                int x = pt.x + offset_x;
                int y = pt.y + offset_y;



                //获取这个点击区域的截图
                click_area.left = x - 30;
                click_area.right = x + 30;
                click_area.top = y - 30;
                click_area.bottom = y + 30;
                a1 = script_inst->get_screen_data(click_area);

                script_inst->click(x, y);
                break;
            }
            else{
                if(script_inst->is_match_pic_in_screen("关闭", pt))
                {
                    script_inst->click(pt.x - 100 - click_times * 5,
                    					pt.y + 100 + click_times * 5);
                    click_times++;

                    if(click_times == 5){
                        luaL_error(L, "点了5次依然没有匹配到");
                    }
                }
            }

            script_inst->mhsleep(WAIT_NORMAL);
            times--;
        }

        if(times == 0){
            std::string info("对话框内没有匹配到图片 ");
            info += img;
            luaL_error(L, info.c_str());
        }

        //等待区域变化返回
        //按十秒算
        times = WAIT_TIMES;
        while(times){

            auto a2 = script_inst->get_screen_data(click_area);
            cv::Point pt;
            double result = script_inst->match_picture(a1, a2, pt);
            script_inst->mhprintf(LOG_INFO, "等待点击区域变动: %f", result);

            //变动了
            if(result < (DEFAULT_THERSHOLD/10))
                break;

            script_inst->mhsleep(WAIT_NORMAL);
            times--;
        }


        return 0;
    });


    REGLUAFUN(lua_status, "发送人工请求", [](lua_State* L)->int{

        //检测是否需要人工
        POINT pt;
        if(script_inst->is_match_pic_in_screen("面对着你的角色", pt)){
            RECT rect;
            rect.left = pt.x-50;
            rect.top = pt.y;
            rect.right = rect.left + 300;
            rect.bottom = rect.top + 200;

            //获取这块区域数据
            const std::vector<uchar>& pic = script_inst->get_screen_data(rect);
            cv::Mat mat_pic = cv::imdecode(pic, cv::IMREAD_COLOR);

            DAMA_PARAM param;
            param.width = mat_pic.cols;
            param.height = mat_pic.rows;
            param.x = pt.x - 50;
            param.y = pt.y;
            param.tip_len = 0;

            //回调到上层
            if(script_inst->send_pic_to_helper(&param, (const char*)&pic[0], pic.size()))
            {
                //发送成功
                //等待事件, 这个事件让应用层去触发
                //阻塞这个操作
                while(script_inst->get_helper_event()){
                    script_inst->mhprintf(LOG_INFO, "等待人工操作结果..");
                    script_inst->mhsleep(WAIT_NORMAL);
                };
            }


        }
        else{
            luaL_error(L, "没有找到需要人工的标志");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "匹配任务", [](lua_State* L)->int{
        lua_pushboolean(L, script_inst->match_task());
        return 1;
    });

    REGLUAFUN(lua_status, "获取玩家状态", [](lua_State* L)->int{
        lua_pushinteger(L, script_inst->get_player_status());
        return 1;
    });

    REGLUAFUN(lua_status, "检测离线", [](lua_State* L)->int{
        lua_pushboolean(L, script_inst->check_offline());
        return 1;
    });


    REGLUAFUN(lua_status, "延迟", [](lua_State* L)->int{
        int time = lua_tointeger(L, 1);
        script_inst->mhsleep(time * 1000);
        return 0;
    });

    REGLUAFUN(lua_status, "调试信息", [](lua_State* L)->int{
        int arg_counts = lua_gettop(L);
        std::string info;
        LOG_TYPE logtype = LOG_NORMAL;

        if(arg_counts == 1){
            info = lua_tostring(L, 1);
        }
        else if(arg_counts == 2){
            info = lua_tostring(L, 1);
            logtype = (LOG_TYPE)lua_tointeger(L, 2);
        }

        script_inst->mhprintf(logtype, info.c_str());
        return 0;
    });


    //TODO: 这个函数更改过, 可能不再使用之前的脚本
    REGLUAFUN(lua_status, "点击", [](lua_State* L)->int{
        assert(lua_gettop(L) == 2);
        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);

        //实验测试这样移动会完成给予的点击操作
        script_inst->slow_click(script_inst->get_cur_game_mouse().x + x,
                                script_inst->get_cur_game_mouse().y + y,
                                1);

        return 0;
    });


    REGLUAFUN(lua_status, "移动鼠标到", [](lua_State* L)->int{

        int arg_counts = lua_gettop(L);
        POINT pt;

        if(arg_counts == 2){
            //判断lua函数
            if(LUA_TNUMBER == lua_type(L, 1)){
                pt.x = lua_tointeger(L, 1);
                pt.y = lua_tointeger(L, 2);
            }
            else
            {

                std::string name = lua_tostring(L, 1);
                double thershold = lua_tonumber(L, 2);

                script_inst->mhprintf(LOG_INFO, "移动鼠标到 %s, thersold:%f", name.c_str(), thershold);
                if(false == script_inst->is_match_pic_in_screen(name.c_str(), pt, rect_game, thershold)){
                    std::string err("图片没有匹配到");
                    err += name;
                    luaL_error(L, err.c_str());
                }
            }
        }
        else if(arg_counts == 1){
            std::string name = lua_tostring(L, 1);
            script_inst->mhprintf(LOG_INFO, "移动鼠标到 %s", name.c_str());
            if(false == script_inst->is_match_pic_in_screen(name.c_str(), pt)){
                std::string err("图片没有匹配到");
                err += name;
                luaL_error(L, err.c_str());
            }
        }
        else if(arg_counts == 3 || arg_counts == 4){
            std::string name = lua_tostring(L, 1);
            int offset_x = lua_tointeger(L, 2);
            int offset_y = lua_tointeger(L, 3);
            double thershold = DEFAULT_THERSHOLD;

            if(arg_counts == 4)
            {
                thershold = lua_tonumber(L, 4);
            }

            script_inst->mhprintf(LOG_INFO,
                                  "移动鼠标到 %s, offset_x:%d, offset_y:%d, thershold:%f",
                                  name.c_str(),
                                  offset_x,
                                  offset_y,
                                  thershold);
            if(false == script_inst->is_match_pic_in_screen(name.c_str(), pt, rect_game, thershold)){
                std::string err("图片没有匹配到");
                err += name;
                luaL_error(L, err.c_str());
            }

            pt.x += offset_x;
            pt.y += offset_y;
        }
        else
        {
            luaL_error(L, "移动鼠标到 参数错误");
        }


        script_inst->click(pt.x, pt.y, 3);

        lua_pushinteger(L, script_inst->get_cur_game_mouse().x);
        lua_pushinteger(L, script_inst->get_cur_game_mouse().y);
        return 2;
    });

    REGLUAFUN(lua_status, "按键", [](lua_State* L)->int{
        std::string keystr = lua_tostring(L, 1);
        script_inst->mhprintf(LOG_INFO, "按键 %s", keystr.c_str());
        script_inst->key_press(keystr);
        return 0;
    });

    REGLUAFUN(lua_status, "存在任务", [](lua_State* L)->int{

        std::string img = lua_tostring(L, 1);
        script_inst->check_pic_exists(img);

        bool match = script_inst->_match_task(img);
        lua_pushboolean(L, match);
        return 1;
    });

    REGLUAFUN(lua_status, "获得图片位置", [](lua_State* L)->int{
        std::string pic = lua_tostring(L, 1);
        double thershold = DEFAULT_THERSHOLD;
        if(lua_gettop(L) == 2)
            thershold = lua_tonumber(L, 2);

        POINT pt;
        if(script_inst->is_match_pic_in_screen(pic, pt, rect_game, thershold))
        {
            lua_pushinteger(L, pt.x);
            lua_pushinteger(L, pt.y);
        }
        else{
            std::string err = pic;
            err += " 不存在";
            luaL_error(L, err.c_str());
        }

        return 2;
    });

    REGLUAFUN(lua_status, "结束脚本", [](lua_State* L)->int{
        std::string reason = lua_tostring(L, 1);
        script_inst->mhprintf(LOG_NORMAL,reason.c_str());
        script_inst->end_task();
        return 0;
    });



    REGLUAFUN(lua_status, "存在图片", [](lua_State* L)->int{
        int arg_counts = lua_gettop(L);

        std::string imgname;
        RECT rect = rect_game;
        double threshold = DEFAULT_THERSHOLD;

        if(arg_counts == 1){
            imgname = lua_tostring(L, 1);
        }
        else if(arg_counts == 2){
            imgname = lua_tostring(L, 1);
            threshold = lua_tonumber(L, 2);
        }
        else if(arg_counts == 3){
            imgname = lua_tostring(L, 1);
            threshold = lua_tonumber(L, 2);
            int m = lua_tointeger(L, 3);
            if(m == 1){
                rect.right = (rect.right - rect.left) / 2 + rect.left;
            }
        }
        else{
            luaL_error(L, "参数错误");
        }

        bool bexist = script_inst->is_match_pic_in_screen(imgname, rect, threshold);
        lua_pushboolean(L, bexist);
        return 1;
    });


    REGLUAFUN(lua_status, "加血", [](lua_State* L)->int{
        script_inst->rclick(point_player_healher.x, point_player_healher.y);
        return 0;
    });

    REGLUAFUN(lua_status, "右击", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        script_inst->rclick(imgname.c_str());
        return 0;
    });

    REGLUAFUN(lua_status, "点击坐标", [](lua_State* L)->int{
        int counts = lua_gettop(L);
        if(counts != 2)
            luaL_error(L, "点击坐标 参数数量错误");

        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);
        script_inst->click(x, y);
        return 0;
    });


    REGLUAFUN(lua_status, "存在下划线", [](lua_State* L)->int{
        //参数数量
        int arg_count = lua_gettop(L);
        RECT default_rect = rect_task;
        POINT pt;
        bool exists = false;

        //参数 行数量
        if(arg_count == 1)
        {
            int line = lua_tointeger(L, 1);
            default_rect.top += line * 21;      //一行14个像素
        }

        //先检测三个长度的下划线
        if(script_inst->find_red_line("可点击下划线1", pt, default_rect)){
            exists = true;
        }
        else if(script_inst->find_red_line("可点击下划线", pt, default_rect)){
            exists = true;
        }
        else{
            exists = false;
        }

        lua_pushboolean(L, exists);
        return 1;
    });

    REGLUAFUN(lua_status, "点击下划线", [](lua_State* L)->int{

        //参数数量
        int arg_count = lua_gettop(L);
        RECT default_rect = rect_task;
        POINT pt;
        //参数 行数量
        if(arg_count == 1)
        {
            int line = lua_tointeger(L, 1);
            default_rect.top += line * 21;      //一行14个像素
        }

        //先检测三个长度的下划线
        if(script_inst->find_red_line("可点击下划线1", pt, default_rect)){
            script_inst->click(pt.x, pt.y);
        }
        else if(script_inst->find_red_line("可点击下划线", pt, default_rect)){
            script_inst->click(pt.x, pt.y);
        }
        else{
            luaL_error(L, "可点击下划线没有匹配到");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "隐藏玩家", [](lua_State* L)->int{
        script_inst->key_press(VK_F9);
        return 1;
    });

    REGLUAFUN(lua_status, "变动点击", [](lua_State* L)->int{
        int arg_counts = lua_gettop(L);


        std::string pic;
        double thershold = DEFAULT_THERSHOLD;
        auto a1 = script_inst->get_screen_data();

        if(arg_counts == 0){
            script_inst->mhprintf(LOG_INFO, "变动点击 xy:当前位置");
            script_inst->slow_click(script_inst->get_cur_game_mouse().x,
                               script_inst->get_cur_game_mouse().y);
        }
        else if(arg_counts == 1){
             pic = lua_tostring(L, 1);
             script_inst->mhprintf(LOG_INFO, "变动点击 pic:%s", pic.c_str());
             script_inst->click(pic.c_str(), thershold);
        }
        else if(arg_counts == 2){
             if(LUA_TNUMBER == lua_type(L, 1)){
                 int x = lua_tointeger(L, 1);
                 int y = lua_tointeger(L, 2);
                 script_inst->mhprintf(LOG_INFO, "变动点击 x:%d y:%d", x, y);
                 script_inst->click(x, y);
             }
             else{
                 pic = lua_tostring(L, 1);
                 thershold = lua_tonumber(L, 2);
                 script_inst->mhprintf(LOG_INFO, "变动点击 pic:%s thershold:%d", pic.c_str(), thershold);
                 script_inst->click(pic.c_str(), thershold);
             }

        }
        else if(arg_counts == 3 || arg_counts == 4){
            pic = lua_tostring(L, 1);
            int x = lua_tointeger(L, 2);
            int y = lua_tointeger(L, 3);

            if(arg_counts == 4)
                  thershold = lua_tonumber(L, 4);

            POINT pt;
            if(script_inst->is_match_pic_in_screen(pic, pt, rect_game, thershold)){
                script_inst->mhprintf(LOG_INFO, "变动点击 pic:%s x:%d y:%d thershold:%f", pic.c_str(), x, y, thershold);
                script_inst->click(pt.x + x, pt.y + y);
            }
            else{
                std::string err("这个图片不存在");
                err += pic.c_str();
                luaL_error(L, err.c_str());
            }
        }
        else{
            luaL_error(L, "参数数量错误");
        }

        //按十秒算
        int times = WAIT_TIMES;
        while(times){

            auto a2 = script_inst->get_screen_data();
            cv::Point pt;
            double result = script_inst->match_picture(a1, a2, pt);
            qDebug() << QString::fromLocal8Bit("变动点击:") << result;

            //变动了
            if(result < DEFAULT_THERSHOLD)
                break;

            script_inst->mhsleep(WAIT_NORMAL);
            times--;
        }

        if(times == 0) luaL_error(L, "变动点击尝试后依然失败");
        return 0;
    });

    REGLUAFUN(lua_status, "点击图片", [](lua_State* L)->int{
        int arg_counts = lua_gettop(L);

        std::string imgname = lua_tostring(L, 1);

        if(arg_counts == 1){
            script_inst->click(imgname.c_str());
        }
        else if(arg_counts == 2){
            double thershold = lua_tonumber(L, 2);
            script_inst->click(imgname.c_str(), thershold);
        }
        else if(arg_counts == 3){
            int x = lua_tointeger(L, 2);
            int y = lua_tointeger(L, 3);
            POINT pt;
            if(script_inst->is_match_pic_in_screen(imgname, pt, rect_game))
                script_inst->click(pt.x + x, pt.y + y);
            else{
                script_inst->mhprintf(LOG_WARNING,"这个图片不存在 %s", imgname.c_str());
            }
        }
        else if(arg_counts == 4){
            int x = lua_tointeger(L, 2);
            int y = lua_tointeger(L, 3);
            double thershold = lua_tonumber(L, 4);

            POINT pt;
            if(script_inst->is_match_pic_in_screen(imgname, pt, rect_game, thershold))
                script_inst->click(pt.x + x, pt.y + y);
            else{
                script_inst->mhprintf(LOG_WARNING,"这个图片不存在 %s", imgname.c_str());
            }
        }
        else{
            luaL_error(L, "参数数量错误");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "等待停止奔跑", [](lua_State* L)->int{
        script_inst->until_stop_run();
        return 0;
    });

    REGLUAFUN(lua_status, "装备物品", [](lua_State* L)->int{
        bool waiting = false;
        if(lua_gettop(L) == 2){
            waiting = true;
        }

        //首先打开背包
        POINT flag_rect;

        for(int i = 0; i < 3; i++)
        {
            if(script_inst->is_match_pic_in_screen("现金按钮", flag_rect) == false)
            {
                script_inst->key_press("ALT+E");

                int times = WAIT_TIMES;
                while(times){ //等待出现
                    if(script_inst->is_match_pic_in_screen("现金按钮", flag_rect)) break;
                    script_inst->mhprintf(LOG_NORMAL, "等待装备界面出现..");
                    script_inst->mhsleep(WAIT_NORMAL);

                    times--;
                }

                if(times == 0) luaL_error(L, "等待装备界面出现失败, 重新遍历任务");
                break;
            }
            else{
                break;
            }
        }

        RECT bag_rect;
        bag_rect.left = flag_rect.x - 30;
        bag_rect.top = 0;
        bag_rect.bottom = SCREEN_HEIGHT;
        bag_rect.right = SCREEN_WIDTH;

        std::string imgname = lua_tostring(L, 1);
        imgname.insert(0, "物品\\");
        POINT pt;
        if(script_inst->is_match_pic_in_screen(imgname, pt, bag_rect) == false)
            script_inst->mhprintf(LOG_WARNING,"装备的物品不存在 %s", imgname.c_str());
        else
        {
            std::vector<uchar> a1;
            if(waiting){
                a1 = script_inst->get_screen_data();
            }

            script_inst->rclick(pt.x, pt.y);
            script_inst->mhprintf(LOG_INFO, "装备物品 %s", imgname.c_str());

            if(waiting)
            {
                int times = WAIT_TIMES;
                while(times){
                    auto a2 = script_inst->get_screen_data();
                    cv::Point pt;
                    double result = script_inst->match_picture(a1, a2, pt);
                    qDebug() << QString::fromLocal8Bit("变动点击:") << result;

                    //变动了
                    if(result < DEFAULT_THERSHOLD)
                        break;

                    times--;
                }
            }

        }

        return 0;
    });

    REGLUAFUN(lua_status, "点击小地图", [](lua_State* L)->int{
        //非递归互斥, 注意死锁问题
        int arg_counts = lua_gettop(L);
        assert(arg_counts == 1 || arg_counts == 3);

        std::string name = lua_tostring(L, 1);
        name.insert(0, "小地图\\");

        //打开小地图
        script_inst->key_press(VK_TAB);

        int times = WAIT_TIMES;
        while(times--){ //等待出现
            if(script_inst->is_match_pic_in_screen("世界按钮")) break;
            script_inst->mhprintf(LOG_INFO, "等待小地图出现..");
            script_inst->mhsleep(WAIT_NORMAL);


            if(times == 1) luaL_error(L, "等待小地图出现失败, 重新遍历任务");
        }

        //点击图片
        int offset_x = 0;
        int offset_y = 0;
        if(arg_counts == 3){//TODO
            offset_x = lua_tointeger(L, 2);
            offset_y = lua_tointeger(L, 3);

            POINT pt;
            if(script_inst->is_match_pic_in_screen(name.c_str(), pt))
            {
                pt.x += offset_x;
                pt.y += offset_y;
                script_inst->click(pt.x, pt.y);
            }
            else
            {
                std::string err("没有匹配到 ");
                err += name;
                luaL_error(L, err.c_str());
            }
        }
        else{
            script_inst->click(name.c_str());
        }

        //关闭小地图
        script_inst->key_press(VK_TAB);
        script_inst->until_stop_run();
        return 0;
    });
}


//单独脚本线程入口
void GameScript::start(std::string script_name)
{

    try
    {
        task_running = true;
        mhprintf(LOG_NORMAL, "线程ID:%d, 脚本执行", std::this_thread::get_id());


        //初始化几个鼠标
        std::call_once(just_once_read, [this](int id){
            mouse1 = cv::imread("pic\\chk\\mouse1.png");
            mouse2 = cv::imread("pic\\chk\\mouse2.png");
            cv::Mat mouse3_temp = cv::imread("pic\\chk\\mouse3.png");
            mouse4 = cv::imread("pic\\chk\\mouse4.png");

            //单独特殊处理
            process_pic_mouse(mouse3_temp, mouse3);
            mhprintf(LOG_INFO, "读取指针图像完成");
        }, 0);



        regist_lua_fun();
        do_script("lualib/gamefun.lua");


        //mhprintf(LOG_NORMAL,"进入游戏..");
        //entry_game();
        //mhprintf(LOG_NORMAL,"进去游戏..完成");

        //title
        ::SetWindowTextA(wnd, player_name.c_str());

        read_global(true);
        do_script(script_name);
        read_global(false);

        mhprintf(LOG_INFO, "脚本能处理任务数量%d个", lua_task_list.size());

        //这是个阻塞调用, 异常返回, 或者正常结束返回
        while(task_running){
            try{
                call_lua_func("脚本入口");
            }
            catch(exception_status &e){
                mhprintf(LOG_WARNING, "%s", e.what());
            }
            catch(exception_xy &e){
                mhprintf(LOG_WARNING, "%s", e.what());
            }
            catch(std::runtime_error &e){
                mhprintf(LOG_ERROR, "%s", e.what());
            }
            catch(std::logic_error &e){
                mhprintf(LOG_ERROR, "%s", e.what());
            }
        }
    }
    catch(std::runtime_error& e)
    {
        mhprintf(LOG_INFO, e.what());
    }
    catch(...)
    {
        mhprintf(LOG_ERROR, "未知的异常");
    }

    mhprintf(LOG_INFO, "脚本停止");
}


void GameScript::input_password(const char* input)
{

    for(size_t i = 0; i < strlen(input); i++){

        if(input[i] == '@')
        {
            keybd_event(16, 0, 0 ,0);
            mhsleep(100);

            keybd_event(0x32,0,0,0);     //按下a键
            mhsleep(10);
            keybd_event(0x32,0,KEYEVENTF_KEYUP,0);//松开a键


            mhsleep(50);
            keybd_event(16,0,KEYEVENTF_KEYUP,0);//松开a键
            mhsleep(100);
        }
        else if(input[i] == '.')
        {
            keybd_event(VK_OEM_PERIOD,0,0,0);     //按下a键
            keybd_event(VK_OEM_PERIOD,0,KEYEVENTF_KEYUP,0);//松开a键
        }
        else if(input[i] == ',')
        {
            keybd_event(VK_TAB,0,0,0);     //按下a键
            keybd_event(VK_TAB,0,KEYEVENTF_KEYUP,0);//松开a键
            mhsleep(400);
        }
        else{
            keybd_event(toupper(input[i]),0,0,0);     //按下a键
            keybd_event(toupper(input[i]),0,KEYEVENTF_KEYUP,0);//松开a键
            mhsleep(rand()%200+100);
        }


        mhsleep(500);
    }

}


//也用来取消攻击状态
void GameScript::rclick(int x, int y)
{
    //TODO:
    click(x, y, false);
}



void GameScript::rclick(const char* image)
{
    POINT point;
    if(is_match_pic_in_screen(image, point)){
        rclick(point.x, point.y);
    }
    else{
        mhprintf(LOG_WARNING,"点击一个屏幕不存在的图片 %s", image);
    }
}

void GameScript::click(const char* image, double threshold)
{
    POINT point;
    if(is_match_pic_in_screen(image, point, rect_game, threshold)){
        click(point.x, point.y);
    }
    else{
        std::string err("点击一个屏幕不存在的图片 ");
        err += image;
        throw std::runtime_error(err.c_str());
    }
}

void GameScript::click_nofix(const char* image)
{

    POINT point;
    if(is_match_pic_in_screen(image, point)){
        click_nofix(point.x, point.y);
    }

}


bool GameScript::is_match_pic_in_screen(std::string image, const RECT& rect, double threshold)
{
    POINT pt;
    return is_match_pic_in_screen(image, pt, rect, threshold);
}


double GameScript::match_picture(const std::vector<uchar> &img1, std::string img2, cv::Point &matchLoc)
{
    check_pic_exists(img2);
    cv::Mat matchscreen = cv::imdecode(img1, cv::IMREAD_COLOR);
    cv::Mat matchpic = cv::imread(img2.c_str(), cv::IMREAD_COLOR);
    return _match_picture(matchscreen, matchpic, matchLoc);
}

double GameScript::_match_picture(const cv::Mat& screen, const cv::Mat& pic, cv::Point &matchLoc)
{
    double minVal;
    double maxVal;
    cv::Point minLoc;
    cv::Point maxLoc;
    cv::Mat result;

    if(screen.empty() || pic.empty())
        throw std::runtime_error("空的源图Mat, 检查代码!");

    //匹配方式
    int match_method = cv::TM_CCOEFF_NORMED;

    if(pic.rows > screen.rows || pic.cols > screen.cols){
        maxVal = 0.0;
    }
    else{

        cv::matchTemplate(screen, pic, result, match_method);
        cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
        if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
            matchLoc = minLoc;
        }
        else{
            matchLoc = maxLoc;
        }
    }

    //      cv::imshow("image_window", screen);
    //      cv::imshow("result_window", pic);

    //    int nThreshold = 0;
    //    cv::namedWindow("ori", CV_WINDOW_NORMAL);
    //    cv::createTrackbar("11", "ori", &nThreshold, 254, on_trackbar1, &matchscreen);

    //    cv::namedWindow("test", CV_WINDOW_NORMAL);
    //    cv::createTrackbar("11", "test", &nThreshold, 254, on_trackbar2, &matchpic);

    //    cv::waitKey(0);

    return maxVal;
}

double GameScript::match_picture(const std::vector<uchar>& img1, const std::vector<uchar>& img2, cv::Point &matchLoc)
{
    cv::Mat matscreen = cv::imdecode(img1, cv::IMREAD_COLOR);
    cv::Mat matchpic = cv::imdecode(img2, cv::IMREAD_COLOR);
    return _match_picture(matscreen, matchpic, matchLoc);
}

double GameScript::match_picture(const std::vector<uchar>& img1, const cv::Mat& pic, cv::Point& matchLoc)
{
    cv::Mat matscreen = cv::imdecode(img1, cv::IMREAD_COLOR);
    return _match_picture(matscreen, pic, matchLoc);
}

POINT GameScript::get_cur_game_mouse()
{
    return {cur_game_x, cur_game_y};
}



//任务区域
bool GameScript::find_red_line(std::string image, POINT &point, RECT rect)
{
    check_pic_exists(image);

    cv::Mat matchscreen = cv::imdecode(get_screen_data(rect), cv::IMREAD_COLOR);
    cv::Mat matchpic = cv::imread(image.c_str(), cv::IMREAD_COLOR);

//    cv::imshow("image_window", matchscreen);
//    cv::imshow("result_window", matchpic);
//    cv::waitKey();

    cv::Mat result_screen;
    cv::Mat result_pic;

    //红色以外过滤掉
    process_pic_task_redline(matchscreen, result_screen);
    process_pic_task_redline(matchpic, result_pic);


    cv::Point matchLoc;
    double result = _match_picture(result_screen, result_pic, matchLoc);
    bool ret = (result >= 0.9);
    if(ret){
        point.x = matchLoc.x + rect.left + matchpic.cols/2;
        point.y = matchLoc.y + rect.top - 5;
    }

    return ret;
}

//POINT 返回匹配到的图片位置
bool GameScript::is_match_pic_in_screen(std::string image, POINT &point, const RECT& rect, double threshold)
{
    if(threshold <= 0.3 || threshold >= 1.0){
        throw std::runtime_error("匹配程度设置错误, 大于3小于10为妙");
    }

    //两个对比的图
    check_pic_exists(image);

    cv::Point matchLoc;
    double maxVal = match_picture(get_screen_data(rect), image, matchLoc);
    //mhprintf(LOG_NORMAL,"匹配: %s %f", image.c_str(), maxVal);
    if(maxVal > threshold)
    {
        cv::Mat img_in = cv::imread(image.c_str());
        point.x = matchLoc.x + img_in.cols/2 + rect.left;
        point.y = matchLoc.y + img_in.rows/2 + rect.top;

        return true;
    }
    else
    {
        return false;
    }
}

void GameScript::rand_move_mouse()
{
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, make_mouse_value(100, 100));
    mhsleep(WAIT_NORMAL);
}



void GameScript::until_stop_run(int counts)
{
    //来一张
    auto pos1 = get_screen_data();

    while(1)
    {
        mhsleep(counts);

        auto pos2 = get_screen_data();

        cv::Point maxLoc;
        double isMatch = match_picture(pos1, pos2, maxLoc);
        if(isMatch > 0.8) break;

        pos1 = pos2;
        qDebug() << QString::fromLocal8Bit("等待界面静止");
    }

}

//x -> x2
void GameScript::get_mouse_vec(int x, int y, int x2, int y2, std::vector<int>& r)
{

    if(x2 >= SCREEN_WIDTH || y2 >= SCREEN_HEIGHT)
    {
        char buf[200];
        sprintf(buf, "目的坐标异常 x: %d, y: %d", x2, y2);
        throw exception_xy(buf);
    }

    do{
        if(x < x2)
        {
            x++;
            if(y < y2) y++;
            else if(y > y2) y--;

            int v = make_mouse_value(x, y);
            r.push_back(v);

        }
        else if(x > x2)
        {
            x--;
            if(y < y2) y++;
            else if(y > y2) y--;

            int v = make_mouse_value(x, y);
            r.push_back(v);
        }
        else{
            if(y < y2) y++;
            else if(y > y2) y--;

            int v = make_mouse_value(x, y);
            r.push_back(v);
        }
    }
    while(x != x2 || y != y2);
}

void GameScript::click_nofix(int x, int y)
{
    int v = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, v);
    ::PostMessage(wnd, WM_LBUTTONDOWN, 1,v);
    ::PostMessage(wnd, WM_LBUTTONUP, 0,v);
    mhsleep(WAIT_NORMAL);
    rand_move_mouse();
}

//匹配屏幕获取当前鼠标位置
POINT GameScript::get_cur_mouse()
{

    int try_times = 0;

letstart:
    mhsleep(WAIT_POSTMSG);
    cv::Point maxLoc;
    double val = match_picture(get_screen_data(), mouse2, maxLoc);
    maxLoc.x -= 17;
    maxLoc.y -= 17;


    if(val < 0.90)
    {
        val = match_picture(screen_data(), mouse1, maxLoc);
        maxLoc.x -= 5;
        maxLoc.y -= 5;
    }

    if(val < 0.90)
    {
        val = match_picture(screen_data(), mouse4, maxLoc);
        maxLoc.x -= 30;
    }


    //单独处理alt+g的图标
    if(val < 0.90)
    {

        cv::Mat ori = cv::imdecode(screen_data(), cv::IMREAD_COLOR);
        cv::Mat processed_screen;
        process_pic_mouse(ori, processed_screen);
        val = _match_picture(processed_screen, mouse3, maxLoc);
    }


    if(val < 0.90)
    {
        if(try_times == 10){
            end_task();
            throw std::runtime_error("超过指定的尝试次数");
        }

        rand_move_mouse();
        mhprintf(LOG_INFO,"没有检测到鼠标, 重新检测 %f", val);
        try_times++;
        goto letstart;
    }

    return {maxLoc.x, maxLoc.y};
}

void GameScript::slow_click(int x1, int y1, int lbutton)
{
    std::vector<int> mouse;
    get_mouse_vec(cur_game_x, cur_game_y, x1, y1, mouse);
    for(size_t i = 0; i < mouse.size(); i++){
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, mouse[i]);
        mhsleep(8, false);
    }


    mhsleep(WAIT_NORMAL);  //停稳了...
    int last = *mouse.end();
    if(lbutton == 1)
        ::PostMessage(wnd, WM_LBUTTONDOWN, 1, last);
    else if(lbutton == 0)
        ::PostMessage(wnd, WM_RBUTTONDOWN, 1, last);

    //这个操作不要用mhsleep了, 不然造成只按下没能松开的问题
    mhsleep(50 + rand()%50, false);
    if(lbutton == 1)
        ::PostMessage(wnd, WM_LBUTTONUP, 0, last);
    else if(lbutton == 0)
        ::PostMessage(wnd, WM_RBUTTONUP, 0, last);

    cur_game_x = x1;
    cur_game_y = y1;

    //停稳
    mhsleep(WAIT_NORMAL);
}


void GameScript::click_nomove(int x, int y)
{
    int xy = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, xy);
    mhsleep(WAIT_NORMAL);
    ::PostMessage(wnd, WM_LBUTTONDOWN, 1, xy);
    ::PostMessage(wnd, WM_LBUTTONUP, 0, xy);
    mhsleep(WAIT_NORMAL);
}

void GameScript::key_press(std::string key)
{
    std::lock_guard<std::mutex> locker(topwnd_mutex);
    top_wnd();
    if(key == "TAB"){
        key_press(VK_TAB);
    }
    else if(key == "F8"){
        key_press(VK_F8);
    }
    else if(key == "F9"){
        key_press(VK_F9);
    }
    else if(key.find('+') == std::string::npos){
        key_press(toupper(key[0]));
    }
    else{
        std::string f = key.substr(0, key.find('+'));
        std::string a = key.substr(key.find('+') + 1, key.length());
        if(f == "SHIFT")
            mhprintf(LOG_NORMAL,"no shift..");
        else if(f == "ALT"){
            keybd_event(VK_MENU, 0, 0, 0);
            mhsleep(100);
            keybd_event(toupper(a[0]),0, 0, 0);
            keybd_event(toupper(a[0]),0, KEYEVENTF_KEYUP, 0);
            mhsleep(100);
            keybd_event(VK_MENU,0,KEYEVENTF_KEYUP,0);
            mhsleep(100);
        }
    }

    mhsleep(WAIT_NORMAL);
}

void GameScript::key_press(int vk)
{
    top_wnd();

    ::PostMessageA(wnd, WM_KEYDOWN, vk, 0);
    ::PostMessageA(wnd, WM_KEYUP, vk, 0);

    mhsleep(WAIT_NORMAL);
}


//传进来的是窗口坐标
//转化成游戏内坐标, 之后用WM_MOUSEMOVE移动
void GameScript::click(int x, int y, int lbutton)
{
    POINT now;
    std::vector<int> r;

    //y - 70 还不够, 会被游戏偏移鼠标到窗口外, 大致的移动过去
    int tar_x = (x > 550 ? x - 50:x);
    int tar_y = (y > 400 ? y - 50:y);
    get_mouse_vec(cur_game_x, cur_game_y, tar_x, tar_y, r);

    for(size_t i = 0; i < r.size(); i++)
    {
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, r[i]);
        mhsleep(2, false);
    }

    cur_game_x = tar_x;
    cur_game_y = tar_y;

    //就一下
    now = get_cur_mouse();

    //逐渐移动
    //转换成游戏内座标
    int game_x = static_cast<int>((double)now.x * ratio_x);
    int game_y = static_cast<int>((double)now.y * ratio_y);

    //取得误差
    rx = game_x - cur_game_x;
    ry = game_y - cur_game_y;

    //转换成游戏内鼠标坐标
    int mouse_x = static_cast<int>((double)x * ratio_x);
    int mouse_y = static_cast<int>((double)y * ratio_y);

    //加上那个误差
    mouse_x -= rx;
    mouse_y -= ry;

    slow_click(mouse_x, mouse_y, lbutton);
}

void GameScript::input(const std::string & msg)
{

    for(size_t j = 0; j < msg.size(); j++){
        ::PostMessage(wnd, WM_CHAR, msg[j], 1);
        mhsleep(10);
    }

}


void GameScript::set_player_name(std::string name)
{
    player_name = name;
}

const std::vector<uchar>& GameScript::screen_data()
{
    return _screen_data;
}

//返回屏幕内存数据
const std::vector<uchar>& GameScript::get_screen_data(const RECT& rect)
{

    HDC mem_dc;
    HBITMAP hbm_screen;
    BITMAP bmpScreen;

    try
    {
        mem_dc = ::CreateCompatibleDC(hdc);
        hbm_screen = ::CreateCompatibleBitmap(
                    hdc,
                    rect.right - rect.left,
                    rect.bottom - rect.top
                    );
        SelectObject(mem_dc, hbm_screen);
        BitBlt(mem_dc, 0, 0,
               rect.right - rect.left,
               rect.bottom - rect.top,
               hdc, rect.left, rect.top,
               SRCCOPY);

        // Retrieve the handle to a display device context for the client
        // area of the window.
        HBITMAP hbmScreen = (HBITMAP)GetCurrentObject(mem_dc, OBJ_BITMAP);

        // Get the BITMAP from the HBITMAP
        GetObject(hbmScreen,sizeof(BITMAP),&bmpScreen);

        BITMAPFILEHEADER   bmfHeader;
        memset(&bmfHeader, 0, sizeof(BITMAPFILEHEADER));
        BITMAPINFOHEADER   bi;
        memset(&bi, 0, sizeof(BITMAPINFOHEADER));

        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = bmpScreen.bmWidth;
        bi.biHeight = bmpScreen.bmHeight;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        DWORD dwBmpSize = GetBitmapBytes(bmpScreen.bmWidth, bmpScreen.bmHeight, 32);

        // Gets the "bits" from the bitmap and copies them into a buffer
        // which is pointed to by lpbitmap.
        GetDIBits(hdc, hbmScreen, 0,
                  (UINT)bmpScreen.bmHeight,
                  screen_buf,
                  (BITMAPINFO *)&bi, DIB_RGB_COLORS);



        // Add the size of the headers to the size of the bitmap to get the total file size
        DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        //Offset to where the actual bitmap bits start.
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
		bmfHeader.bfReserved1 = bmfHeader.bfReserved2 = 0;
        bmfHeader.bfSize = dwSizeofDIB;
        bmfHeader.bfType = 0x4D42; //BM


        _screen_data.resize(dwSizeofDIB);

        //已经写入大小
        int readed = 0;
        memcpy(&_screen_data[0], &bmfHeader, sizeof(BITMAPFILEHEADER));
        readed += sizeof(BITMAPFILEHEADER);

        memcpy(&_screen_data[readed], &bi, sizeof(BITMAPINFOHEADER));
        readed += sizeof(BITMAPINFOHEADER);

        assert(dwBmpSize <= 2048000);
        memcpy(&_screen_data[readed], screen_buf, dwBmpSize);

    }
    catch(...){
        mhprintf(LOG_WARNING, "get_screen_data exception");
    }

    DeleteObject(mem_dc);
    DeleteObject(hbm_screen);

    return _screen_data;
}


void GameScript::mhsleep(int ms, bool chk_status)
{
    if(chk_status){
        //检测游戏状态改变
        PLAYER_STATUS now_player_status = get_player_status();
        if(now_player_status != last_player_status)
        {
            //状态发生改变
            //报异常, 重新执行任务逻辑
            last_player_status = now_player_status;
            throw std::runtime_error("在等待操作时游戏内状态发生变化");
        }

        if(task_running == false){
            throw std::runtime_error("在等待操作时检测到停止标志");
        }
    }

    Sleep(ms);
}
