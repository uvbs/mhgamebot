#include "gamescript.h"
#include <regex>
#include <thread>
#include <chrono>
#include <boost/lexical_cast.hpp>
#include "helperfun.h"
#include <QDebug>
#include <QImage>
#include <memory>
#include <opencv2/opencv.hpp>
#include <QDirIterator>

#define  script_inst  GameScript::get_instance(L)



//静态初始化
std::map<lua_State*, GameScript*> GameScript::inst_map;
std::map<std::string, cv::Mat> GameScript::_pic_data;

//获得焦点的互斥
std::mutex topwnd_mutex;
std::mutex login_mutex;

cv::Mat GameScript::mouse1;
cv::Mat GameScript::mouse2;
cv::Mat GameScript::mouse3;
cv::Mat GameScript::mouse4;


std::once_flag once_cache;

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
    ReleaseDC(wnd, hdc);
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
    if(output_callback){
        output_callback(logtype, QString::fromLocal8Bit(msgbuf.c_str()).toStdString().c_str());
    }

    va_end(va_args);
}

void GameScript::set_output_callback(output_fun _callback)
{
    output_callback = _callback;
}




void GameScript::set_sendhelp_callback(help_fun callback)
{
    help_callback = callback;
}


bool GameScript::is_in_city(std::string city)
{
    city.insert(0, "map/");

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
    /* push functions and arguments */
    lua_getglobal(lua_status, func.c_str());  /* function to be called */

    /* do the call (2 arguments, 1 result) */
    if (lua_pcall(lua_status, 0, 0, 0) != 0)
    {
        mhprintf(LOG_WARNING, lua_tostring(lua_status, -1));
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
    mhprintf(LOG_INFO, "加载脚本: %s", filename.c_str());

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

        cv::Point matchLoc;
        if(_match_task(taskname, matchLoc) > 0.95){
            mhprintf(LOG_INFO, "开始任务:%s", name.c_str());
            call_lua_func(name);
            finded = true;
            break;
        }
    }

    return finded;
}

double GameScript::_match_task(std::string imgname, cv::Point& matchLoc)
{
    check_pic_exists(imgname);

    cv::Mat matchscreen = cv::imdecode(
                get_screen_data(),
                cv::IMREAD_COLOR);
    const cv::Mat& matchpic = _pic_data[imgname];

    cv::Mat result_screen;
    cv::Mat result_pic;

    //处理成所有白字
    process_pic_task(matchscreen, result_screen);
    process_pic_task(matchpic, result_pic);


    double maxVal = _match_picture(result_screen, result_pic, matchLoc);
    qDebug() << QString::fromLocal8Bit("任务:") << QString::fromLocal8Bit(imgname.c_str()) << maxVal;

    return maxVal;
}


//获取玩家状态
PLAYER_STATUS GameScript::get_player_status()
{
    if(is_match_pic_in_screen("战斗中", rect_game, 0.95)){
        return PLAYER_STATUS::COMBAT;
    }
    else if(is_match_pic_in_screen("战斗状态1", rect_game, 0.95))
    {
        return PLAYER_STATUS::COMBAT;
    }
    else if(is_match_pic_in_screen("游戏内", rect_game, 0.95)){
        return PLAYER_STATUS::NORMAL;
    }
    else if(is_match_pic_in_screen("游戏状态1", rect_game, 0.95)){
        return PLAYER_STATUS::NORMAL;
    }
//    else if(is_match_pic_in_screen("体验状态.png", rect_game, 0.95)){
//        mhprintf(LOG_INFO, "体验状态");
//        return PLAYER_STATUS::NOTIME;
//    }
    else if(is_match_pic_in_screen("跳过动画", rect_game, 0.95)){
        mhprintf(LOG_INFO, "跳过动画");
        return PLAYER_STATUS::GC;
    }
    else if(is_match_pic_in_screen("小地图按钮", rect_game, 0.95) == false){
        mhprintf(LOG_INFO, "攻击状态");
        return PLAYER_STATUS::ATTACK;
    }
    else{
        throw std::runtime_error("未知状态");
    }

    //attack_start = std::chrono::steady_clock::now();
}


//结束任务
void GameScript::end_task()
{
    task_running = false;
}

void GameScript::top_wnd()
{

    if(!::IsWindow(wnd)) return;

	BYTE keyState[256] = {0};
	//to unlock SetForegroundWindow we need to imitate Alt pressing
	if(::GetKeyboardState((LPBYTE)&keyState))
	{
		if(!(keyState[VK_MENU] & 0x80))
		{
			::keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		}
	}

	::SetForegroundWindow(wnd);

	if(::GetKeyboardState((LPBYTE)&keyState))
	{
		if(!(keyState[VK_MENU] & 0x80))
		{
			::keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		}
	}
}

//包括红色和白色都处理成白色, 其余颜色处理成黑色
//
void GameScript::process_pic_task(const cv::Mat &src, cv::Mat& result)
{
    cv::Mat hsv;
    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);

    //170是调整了坐标数字匹配的.
    //原本的179就导致9这个数字不太清晰
    //fix 179能处理红字..
    inRange(hsv, cv::Scalar(0, 0, 180), cv::Scalar(180, 255, 255), result);
}

//除红色之外的其他颜色都处理成黑色
void GameScript::process_pic_task_redline(const cv::Mat &src, cv::Mat& result)
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

void GameScript::process_pic_door(cv::Mat& src)
{
    cv::Mat hsv;
    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
    inRange(hsv, cv::Scalar(94, 165, 136), cv::Scalar(114, 255, 255), src);
}

//检测图片存在
void GameScript::check_pic_exists(std::string & imgfile)
{
    if(imgfile.find("pic/") == std::string::npos){
        imgfile.insert(0, "pic/");
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

    lua_pushnumber(lua_status, DEFAULT_THERSHOLD);
    lua_setglobal(lua_status, "默认程度");



    REGLUAFUN(lua_status, "过验证", [](lua_State* L)->int{
        try{
            script_inst->pass_static_check();
        }
        catch(...){
            luaL_error(L, "过验证 异常");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "当前队长", [](lua_State* L)->int{
        try{
            lua_pushboolean(L, script_inst->get_id() == 0 ? true: false);
        }
        catch(...){
            luaL_error(L, "当前队长异常");
        }

        return 1;
    });

    REGLUAFUN(lua_status, "获得地图坐标", [](lua_State* L)->int{
        try{
            lua_pushstring(L, script_inst->get_map_xy().c_str());
        }
        catch(std::exception& e){
            luaL_error(L, "获得地图坐标 异常, 原因 %s", e.what());
        }
        catch(...){
            luaL_error(L, "获得地图坐标 异常");
        }

        return 1;
    });

    REGLUAFUN(lua_status, "调试信息2", [](lua_State* L)->int{

        try
        {
            int arg_counts = lua_gettop(L);
            if(arg_counts != 1)
                luaL_error(L, "调试信息2 参数错误");

            std::string info = lua_tostring(L, 1);
            qDebug() << QString::fromLocal8Bit(info.c_str());
        }
        catch(...)
        {
            luaL_error(L, "调试信息2 未知异常");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "获取坐标", [](lua_State* L)->int{

        try
        {
            int arg_counts = lua_gettop(L);
            if(arg_counts != 0)
                luaL_error(L, "获取坐标 参数错误");

            std::string xy = script_inst->get_task_xy();
            script_inst->mhprintf(LOG_INFO, "%s", xy.c_str());
        }
        catch(...)
        {
            luaL_error(L, "获取坐标 未知异常");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "加血", [](lua_State* L)->int{
        bool success = false;

        //保证加血成功
        do{
            try{
                int arg_counts = lua_gettop(L);
                if(arg_counts != 0)
                    luaL_error(L, "加血 参数错误");

                script_inst->click(point_player_healher.x, point_player_healher.y, 0);
                script_inst->click(point_pet_healher.x, point_pet_healher.y, 0);
                success = true;
            }
            catch(...){
                script_inst->only_move(600, 400);
            }
        }while(success == false && task_running);


        return 0;
    });

    REGLUAFUN(lua_status, "点击传送门", [](lua_State* L)->int{

        try
        {
            int arg_counts = lua_gettop(L);
            if(arg_counts != 0)
                luaL_error(L, "点击传送门 参数错误");

            cv::Mat _screen = cv::imdecode(script_inst->get_screen_data(), cv::IMREAD_COLOR);
            cv::Mat _pic = _pic_data["pic/传送门.png"];

            script_inst->process_pic_door(_screen);
            script_inst->process_pic_door(_pic);

            //        cv::imshow("1", _screen);
            //        cv::imshow("2", _pic);
            //        cv::waitKey(0);

            cv::Point matchLoc;
            double result = script_inst->_match_picture(_screen, _pic, matchLoc);
            if(result > 0.3)
            {
                script_inst->click(matchLoc.x + _pic.cols/2, matchLoc.y);
            }
            else
            {
                luaL_error(L, "传送门没有匹配到");
            }
        }
        catch(...)
        {
            luaL_error(L, "点击传送门 异常");
        }

        return 0;
    });


    REGLUAFUN(lua_status, "等待对话框出现", [](lua_State* L)->int{
        try
        {

            int arg_counts = lua_gettop(L);
            if(arg_counts != 0)
                luaL_error(L, "等待对话框出现 参数错误");


            //等待对话框出现
            script_inst->wait_appear("关闭", rect_dlg_flag);
        }
        catch(...)
        {
            luaL_error(L, "等待对话框出现 异常");
        }

        return 0;
    })


    REGLUAFUN(lua_status, "点击对话框", [](lua_State* L)->int{
        try
        {
            int arg_counts = lua_gettop(L);
            std::string img;
            int offset_x = 0;
            int offset_y = 0;
            POINT pt;
            std::vector<uchar> a1;
            RECT click_area;
            int times = 5;


            if(arg_counts == 1){
                img = lua_tostring(L, 1);
            }
            else if(arg_counts == 3){
                img = lua_tostring(L, 1);
                offset_x = lua_tointeger(L, 2);
                offset_y = lua_tointeger(L, 3);
            }
            else{
                luaL_error(L, "点击对话框 参数数量错误");
            }

            img.insert(0, "对话框/");


            script_inst->mhprintf(LOG_INFO, "点击对话框 pic:%s", img.c_str());


            //等待对话框出现
            script_inst->wait_appear("关闭", rect_dlg_flag);

            while(times)
            {
                if(script_inst->is_match_pic_in_screen(img, pt)){
                    int x = pt.x + offset_x;
                    int y = pt.y + offset_y;

                    //获取这个点击区域的截图
                    click_area.left = x - 30;
                    click_area.right = x + 30;
                    click_area.top = y - 30;
                    click_area.bottom = y + 30;
                    a1 = script_inst->get_screen_data(click_area);

                    std::lock_guard<std::mutex> locker(topwnd_mutex);
                    script_inst->top_wnd();

                    script_inst->click(x, y);
                    break;
                }
                else
                {
                    //点击空白区域
                    if(script_inst->is_match_pic_in_screen("关闭", pt))
                    {
                        std::lock_guard<std::mutex> locker(topwnd_mutex);
                        script_inst->top_wnd();

                        script_inst->click(pt.x - 100 - times * 5, pt.y + 100 + times * 5);
                    }
                }

                script_inst->mhsleep(WAIT_NORMAL);
                times--;
            }

            if(times == 0)
            {
                std::string info("这个对话框内不存在");
                info += img;
                luaL_error(L, info.c_str());
            }

        }
        catch(...)
        {
            luaL_error(L, "点击对话框 异常");
        }

        return 0;
    });

    //发送屏幕数据到远程计算机上
    //远程计算机返回一个坐标点, 客户端点击
    REGLUAFUN(lua_status, "发送人工请求", [](lua_State* L)->int{

        try
        {
            //获取这块区域数据
            //这里还是转换好, 主要时减少网络流量
            const std::vector<uchar>& pic = script_inst->get_screen_data();

            QImage image;
            if(false == image.loadFromData(&pic[0], pic.size()))
            {
                throw std::runtime_error("image loadfromdata fail!");
            }

            qDebug() << image;
            QImage image2 = image.convertToFormat(QImage::Format_RGB16);
            qDebug() << image2;


            DAMA_PARAM param;
            param.script_id = script_inst->get_id();
            param.width = 640;
            param.height = 480;
            param.x = 0;
            param.y = 0;

            int size = sizeof(DAMA_PARAM) + image2.byteCount();
            char* buf = new char[size];
            memcpy(buf, &param, sizeof(DAMA_PARAM));
            memcpy(buf+sizeof(DAMA_PARAM), image2.constBits(), image2.byteCount());

            script_inst->send_pic_to_helper(buf, size);

            script_inst->recv_help = false;
            while(script_inst->recv_help == false && script_inst->task_running == true)
            {
                script_inst->mhprintf(LOG_INFO, "等待人工操作");
                ::Sleep(3000);
            }

            script_inst->recv_help = false;

            delete []buf;
        }
        catch(...)
        {
            script_inst->end_task();
            luaL_error(L, "发送人工请求 异常");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "匹配任务", [](lua_State* L)->int{

        bool have_task = false;
        try
        {
            have_task = script_inst->match_task();
            lua_pushboolean(L, have_task);
        }
        catch(std::exception& e){
            luaL_error(L, "匹配任务 异常, 原因 %s", e.what());
        }
        catch(...)
        {
            luaL_error(L, "匹配任务 异常");
        }

        return 1;
    });

    REGLUAFUN(lua_status, "获取玩家状态", [](lua_State* L)->int{
        try
        {
            lua_pushinteger(L, script_inst->get_player_status());
        }
        catch(...)
        {
            luaL_error(L, "获取玩家状态 异常");
        }

        return 1;
    });

    REGLUAFUN(lua_status, "检测离线", [](lua_State* L)->int{
        try
        {
            lua_pushboolean(L, script_inst->check_offline());
        }
        catch(...)
        {
            luaL_error(L, "检测离线 异常");
        }

        return 1;
    });


    REGLUAFUN(lua_status, "延迟", [](lua_State* L)->int{
        try
        {
            int time = lua_tointeger(L, 1);
            script_inst->mhsleep(time * 1000, false);
        }
        catch(...)
        {
            luaL_error(L, "延迟 异常");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "调试信息", [](lua_State* L)->int{
        try
        {
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

        }
        catch(...)
        {
            luaL_error(L, "调试信息 异常");
        }

        return 0;
    });



    REGLUAFUN(lua_status, "点击", [](lua_State* L)->int{
        try
        {
            assert(lua_gettop(L) == 2);
            int x = lua_tointeger(L, 1);
            int y = lua_tointeger(L, 2);

            //实验测试这样移动会完成给予的点击操作
            script_inst->slow_click(script_inst->get_cur_game_mouse().x + x,
                                    script_inst->get_cur_game_mouse().y + y,
                                    1);

        }
        catch(...)
        {
            luaL_error(L, "点击 异常");
        }

        return 0;
    });


    REGLUAFUN(lua_status, "移动鼠标到", [](lua_State* L)->int{

        int arg_counts = lua_gettop(L);
        POINT pt;

        try
        {
            if(arg_counts == 2){
                pt.x = lua_tointeger(L, 1);
                pt.y = lua_tointeger(L, 2);
                script_inst->click(pt.x, pt.y, 3);
            }
            else
            {
                luaL_error(L, "移动鼠标到 参数错误");
            }
        }
        catch(std::runtime_error& e){
            luaL_error(L, "移动鼠标到 异常, 原因 %s", e.what());
        }
        catch(...)
        {
            luaL_error(L, "移动鼠标到 异常");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "按键", [](lua_State* L)->int{

        try{
            std::string keystr = lua_tostring(L, 1);
            //script_inst->mhprintf(LOG_INFO, "按键 %s", keystr.c_str());
            script_inst->key_press(keystr);
        }
        catch(...)
        {
            luaL_error(L, "按键 异常");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "存在任务", [](lua_State* L)->int{

        try
        {
            std::string img = lua_tostring(L, 1);
            script_inst->check_pic_exists(img);

            cv::Point matchLoc;
            bool match = (script_inst->_match_task(img, matchLoc) >= 0.95) ? true: false;
            if(match){
                script_inst->mhprintf(LOG_INFO, "存在任务 %s", img.c_str());
            }
            lua_pushboolean(L, match);
        }
        catch(std::exception& e){
            luaL_error(L, "存在任务 异常, 原因 %s", e.what());
        }
        catch(...){
            luaL_error(L, "存在任务 异常");
        }

        return 1;
    });

    REGLUAFUN(lua_status, "获得图片位置", [](lua_State* L)->int{
        try
        {
            int arg_counts = lua_gettop(L);
            std::string pic;
            double thershold = DEFAULT_THERSHOLD;

            if(arg_counts == 2)
            {
                pic = lua_tostring(L, 1);
                thershold = lua_tonumber(L, 2);
            }
            else if(arg_counts == 1)
            {
                pic = lua_tostring(L, 1);
            }
            else
            {
                luaL_error(L, "获得图片位置 参数数量错误");
            }

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
        }
        catch(std::exception& e){
            luaL_error(L, "获得图片位置 异常, 原因 %s", e.what());
        }
        catch(...)
        {
            luaL_error(L, "获得图片位置 异常");
        }

        return 2;
    });

    REGLUAFUN(lua_status, "结束脚本", [](lua_State* L)->int{
        try{
            int arg_counts = lua_gettop(L);
            if(arg_counts != 0){
                luaL_error(L, "结束脚本 参数数量错误");
            }

            std::string reason = lua_tostring(L, 1);
            script_inst->mhprintf(LOG_NORMAL,reason.c_str());
            script_inst->end_task();
        }
        catch(...){
            script_inst->end_task();
        }

        return 0;
    });



    REGLUAFUN(lua_status, "存在图片", [](lua_State* L)->int{
        std::string imgname;
        RECT rect = rect_game;
        try
        {
            int arg_counts = lua_gettop(L);
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
        }
        catch(std::exception& e){
            luaL_error(L, "存在图片 %s 异常, 原因 :%s", imgname.c_str(), e.what());
        }
        catch(...)
        {
            luaL_error(L, "存在图片 异常");
        }

        return 1;
    });



    REGLUAFUN(lua_status, "右击", [](lua_State* L)->int{
        try{
            int arg_counts = lua_gettop(L);
            if(arg_counts == 1){
                std::string imgname = lua_tostring(L, 1);
                script_inst->rclick(imgname.c_str());
            }
            else if(arg_counts == 2){
                int x = lua_tointeger(L, 1);
                int y = lua_tointeger(L, 2);
                script_inst->click(x, y, MOUSE_RIGHT_CLICK);
            }
            else{
                luaL_error(L, "右击 参数错误");
            }
        }
        catch(...){
            luaL_error(L, "右击 异常");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "点击坐标", [](lua_State* L)->int{

        try{
            int counts = lua_gettop(L);

            if(counts != 2)
                luaL_error(L, "点击坐标 参数数量错误");

            int x = lua_tointeger(L, 1);
            int y = lua_tointeger(L, 2);
            script_inst->click(x, y);
        }
        catch(...){
            luaL_error(L, "点击坐标 异常");
        }

        return 0;
    });


    REGLUAFUN(lua_status, "点击下划线", [](lua_State* L)->int{

        try
        {
            //参数数量
            int arg_count = lua_gettop(L);
            RECT default_rect = rect_task;
            POINT pt;

            //避免匹配到字体顶层连横
            default_rect.top = default_rect.top + 10;


            //参数 行数量
            if(arg_count == 1)
            {
                int line = lua_tointeger(L, 1);
                default_rect.top += line * 14;      //一行14个像素
            }

            //先检测三个长度的下划线
            if(script_inst->find_red_line("可点击下划线2", pt, default_rect)){
                script_inst->click(pt.x, pt.y);
            }
            else if(script_inst->find_red_line("可点击下划线", pt, default_rect)){
                script_inst->click(pt.x, pt.y);
            }
            else{
                //NOTE: 让流程继续, 解决鼠标指针正好挡住关闭, 又没法匹配到任务
                //导致脚本中执行点击下划线时又重新来过的问题
                //luaL_error(L, "可点击下划线没有匹配到");
            }

            script_inst->only_move(600, 400);
            script_inst->until_stop_run();
        }
        catch(...)
        {
            luaL_error(L, "点击下划线 异常");
        }

        return 0;
    });


    REGLUAFUN(lua_status, "点击图片", [](lua_State* L)->int{

        try
        {
            int arg_counts = lua_gettop(L);

            std::string imgname = lua_tostring(L, 1);

            if(arg_counts == 1){
                script_inst->click(imgname);
            }
            else if(arg_counts == 2){
                double thershold = lua_tonumber(L, 2);
                script_inst->click(imgname, thershold);
            }
            else if(arg_counts == 3){
                int x = lua_tointeger(L, 2);
                int y = lua_tointeger(L, 3);
                script_inst->click(imgname, x, y);
            }
            else if(arg_counts == 4){
                int x = lua_tointeger(L, 2);
                int y = lua_tointeger(L, 3);
                double thershold = lua_tonumber(L, 4);
                script_inst->click(imgname, x, y, thershold);
            }
            else{
                luaL_error(L, "参数数量错误");
            }
        }
        catch(std::exception& e){
            luaL_error(L, "点击图片 异常, 原因: %s", e.what());
        }
        catch(...)
        {
            luaL_error(L, "点击图片 异常");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "等待停止奔跑", [](lua_State* L)->int{
        try{
            script_inst->until_stop_run();
        }
        catch(...){
            luaL_error(L, "等待停止奔跑 异常");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "装备物品", [](lua_State* L)->int{

        try
        {
            bool waiting = false;
            POINT flag_rect;
            POINT pt;
            std::vector<uchar> a1;

            if(lua_gettop(L) == 2){
                waiting = true;
            }

            script_inst->only_move(600, 400);
            
            //确保打开背包
            while(script_inst->is_match_pic_in_screen("现金按钮", flag_rect) == false)
            {
                script_inst->key_press("ALT+E");
                script_inst->wait_appear("现金按钮");
            }

            RECT bag_rect;
            bag_rect.left = flag_rect.x - 30;
            bag_rect.top = 0;
            bag_rect.bottom = SCREEN_HEIGHT;
            bag_rect.right = SCREEN_WIDTH;

            //TODO: 做一个点击格子的功能
            std::string imgname = lua_tostring(L, 1);
            imgname.insert(0, "物品/");

            if(script_inst->is_match_pic_in_screen(imgname, pt, bag_rect) == false){
                script_inst->mhprintf(LOG_WARNING,"装备的物品不存在 %s", imgname.c_str());
            }
            else
            {

                if(waiting){
                    a1 = script_inst->get_screen_data();
                }

                script_inst->click(pt.x, pt.y, 0);
                script_inst->mhprintf(LOG_INFO, "装备物品 %s", imgname.c_str());

                if(waiting)
                {
                    int times = WAIT_TIMES;
                    while(times){
                        auto a2 = script_inst->get_screen_data();
                        cv::Point pt;
                        double result = script_inst->match_picture(a1, a2, pt);
                        qDebug() << QString::fromLocal8Bit("变动点击:") << imgname.c_str() << result;

                        //变动了
                        if(result < DEFAULT_THERSHOLD)
                            break;

                        script_inst->mhsleep(WAIT_NORMAL);
                        times--;
                    }
                }

            }

        }
        catch(...)
        {
            luaL_error(L, "装备物品 异常");
        }

        return 0;
    });


    REGLUAFUN(lua_status, "点击小地图", [](lua_State* L)->int{

        try
        {
            //非递归互斥, 注意死锁问题
            int arg_counts = lua_gettop(L);

            //打开小地图
            if(script_inst->is_match_pic_in_screen("世界按钮") == false){
                script_inst->key_press("TAB");
            }


            script_inst->wait_appear("世界按钮");


            //点击图片
            if(arg_counts == 3){//TODO
                std::string name = lua_tostring(L, 1);
                name.insert(0, "小地图/");
                int offset_x = lua_tointeger(L, 2);
                int offset_y = lua_tointeger(L, 3);
                script_inst->click(name, offset_x, offset_y, DEFAULT_THERSHOLD, false);
            }
            else if(arg_counts == 1){
                std::string name = lua_tostring(L, 1);
                name.insert(0, "小地图/");
                script_inst->click(name.c_str(), DEFAULT_THERSHOLD, false);
            }
            else if(arg_counts == 2){

                int map_x = lua_tointeger(L, 1);
                int map_y = lua_tointeger(L, 2);

                script_inst->only_move(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
                POINT pt = script_inst->get_cur_game_mouse();
                int x;
                int y;
                int cx = abs(x - map_x);
                int cy = abs(y - map_y);

                do{
                    std::string xy = script_inst->get_map_xy();
                    script_inst->mhprintf(LOG_INFO, xy.c_str());
                    int dotindex = xy.find(',');

                    if(dotindex == 0){
                        xy.erase(dotindex);
                    }
                    std::string sx = xy.substr(0, dotindex);
                    std::string sy = xy.substr(dotindex+1, xy.length());
                    x = std::stoi(sx);
                    y = std::stoi(sy);
                    qDebug() << "x: " << x << "y: " << y;


                    cx = abs(x - map_x);
                    cy = abs(y - map_y);

                    int click_y;
                    int click_x;
                    if(x < map_x){
                        click_x = cx > 2 ? pt.x + cx/2 + cx/4:pt.x + 1;
                    }
                    else if(x > map_x){
                        click_x = cx > 2 ? pt.x - cx/2 - cx/4:pt.x - 1;
                    }


                    if(y < map_y){
                        click_y = cy > 2 ? pt.y - cy/2 - cy/4: pt.y - 1;
                    }
                    else if(y > map_y){
                        click_y = cy > 2 ? pt.y + cy/2 + cy/4: pt.y + 1;
                    }
                    script_inst->move_mouse_vec(pt.x, pt.y, click_x, click_y);
                    pt.y = click_y;
                    pt.x = click_x;

                }
                while(cx >= 2 || cy >= 2);

                script_inst->_click(pt.x, pt.y);
            }
            else{
                luaL_error(L, "点击小地图 参数错误");
            }


            //关闭小地图
            if(script_inst->is_match_pic_in_screen("世界按钮")){
                script_inst->key_press("TAB");
            }

            script_inst->until_stop_run();

        }
        catch(std::exception& e)
        {
            luaL_error(L, "点击小地图 异常, 原因 %s", e.what());
        }
        catch(...)
        {
            luaL_error(L, "点击小地图 异常");
        }

        return 0;
    });
}


//单独脚本线程入口
void GameScript::start(std::string script_name)
{

    try
    {
        task_running = true;
        mhprintf(LOG_NORMAL, "线程ID:%08x, 脚本执行", std::this_thread::get_id());

        std::call_once(once_cache, [this](int id){
            mhprintf(LOG_INFO, "read img cache...");
            cache_folder_png();
            mhprintf(LOG_INFO, "read img cache...ok");
        }, 0);


        last_player_status = get_player_status();
        mhprintf(LOG_INFO, "get current status %d", (int)last_player_status);

//        mhprintf(LOG_NORMAL, "设置窗口GWL_EXSTYLE");
//        LONG now_exstyle =  GetWindowLong(wnd, GWL_EXSTYLE);
//        if(now_exstyle != WS_EX_LAYERED)
//        {
//            //设置层叠窗口
//            SetWindowLong(wnd, GWL_EXSTYLE, now_exstyle);
//        }
//        else
//        {
//            mhprintf(LOG_INFO, "当前游戏窗口已经是GWL_EXSTYLE");
//        }

        regist_lua_fun();
        do_script("lualib/gamefun.lua");


        mhprintf(LOG_NORMAL,"进入游戏..");
        //entry_game();
        mhprintf(LOG_NORMAL,"进入游戏..ok");

        //title
        std::string title = player_name + " " + player_account;
        ::SetWindowTextA(wnd, title.c_str());

        read_global(true);
        do_script(script_name);
        read_global(false);


        //这是个阻塞调用, 异常返回, 或者正常结束返回
        while(task_running){
            try{
                call_lua_func("脚本入口");
            }
            catch(exception_status &e){
                mhprintf(LOG_ERROR, "%s", e.what());
                mhsleep(WAIT_NORMAL, false);
            }
            catch(exception_xy &e){
                mhprintf(LOG_ERROR, "%s", e.what());
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
            mhsleep(100, false);

            keybd_event(0x32,0,0,0);     //按下a键
            mhsleep(10, false);
            keybd_event(0x32,0,KEYEVENTF_KEYUP,0);//松开a键


            mhsleep(50, false);
            keybd_event(16,0,KEYEVENTF_KEYUP,0);//松开a键
            mhsleep(100, false);
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
            mhsleep(400, false);
        }
        else{
            keybd_event(toupper(input[i]),0,0,0);     //按下a键
            keybd_event(toupper(input[i]),0,KEYEVENTF_KEYUP,0);//松开a键
            mhsleep(rand()%200+100, false);
        }


        mhsleep(WAIT_NORMAL, false);
    }

}

//缓存所有png文件
void GameScript::cache_folder_png()
{
    //判断路径是否存在
    QDir dir("./pic");
    if(!dir.exists()){
        throw std::runtime_error("cache png error!");
    }

    //获取所选文件类型过滤器
    QStringList filters;
    filters << QString("*.png");

    //定义迭代器并设置过滤器
    QDirIterator dir_iterator("./pic",
                              filters,
                              QDir::Files | QDir::NoSymLinks,
                              QDirIterator::Subdirectories);
    QStringList string_list;
    while(dir_iterator.hasNext())
    {
        dir_iterator.next();
        QFileInfo file_info = dir_iterator.fileInfo();
        QString file_path = file_info.absoluteFilePath();
        int picindex = file_path.indexOf("pic/");
        QString n = file_path.mid(picindex, file_path.length() - picindex);
        qDebug() << "dir cache " << n ;

        //format convert
        std::string picname = n.toLocal8Bit().toStdString();

        //一些单独处理下
        if(n == "pic/chk/mouse4.png"){
            cv::Mat mouse3_temp = cv::imread(picname);
            process_pic_mouse(mouse3_temp, mouse3_temp);
            _pic_data[picname] = mouse3_temp;
        }
        else{
            //做个和路径的映射
            cv::Mat mat = cv::imread(picname);
            _pic_data[picname] = mat;
            if(mat.empty()){
                qDebug() << file_path;
                throw std::runtime_error("empty mat!");
            }
        }
    }
}


void GameScript::rclick(const char* image)
{
    POINT point;
    if(is_match_pic_in_screen(image, point)){
        click(point.x, point.y, 0);
    }
    else{
        mhprintf(LOG_WARNING,"右击一个屏幕不存在的图片 %s", image);
    }
}

void GameScript::_click(int x, int y, int lbutton)
{
    int tar_mouse = make_mouse_value(x, y);
    if(lbutton == MOUSE_LEFT_CLICK){
        ::PostMessage(wnd, WM_LBUTTONDOWN, 1, tar_mouse);
    }
    else if(lbutton == MOUSE_RIGHT_CLICK){
        ::PostMessage(wnd, WM_RBUTTONDOWN, 1, tar_mouse);
    }

    if(lbutton == MOUSE_LEFT_CLICK){
        ::PostMessage(wnd, WM_LBUTTONUP, 0, tar_mouse);
    }
    else if(lbutton == MOUSE_RIGHT_CLICK){
        ::PostMessage(wnd, WM_RBUTTONUP, 0, tar_mouse);
    }

    mhsleep(WAIT_NORMAL, false);
}

void GameScript::click(const std::string& image, double threshold, bool check_exists)
{
    POINT point;
    mhprintf(LOG_INFO, "点击图片 [%s]", image.c_str());

    if(check_exists){
        wait_appear(image, rect_game, threshold);
    }

    if(is_match_pic_in_screen(image, point, rect_game, threshold)){
        click(point.x, point.y);
    }
    else{
        std::string err("点击一个屏幕不存在的图片 ");
        err += image;
        throw std::runtime_error(err.c_str());
    }
}

void GameScript::click(const std::string& image, int offset_x, int offset_y, double thershold, bool check_exists)
{
    POINT pt;
    mhprintf(LOG_INFO, "点击图片 [%s] offset_x:%d offset_y:%d", image.c_str(), offset_x, offset_y);

    if(check_exists){
        wait_appear(image, rect_game, thershold);
    }

    if(is_match_pic_in_screen(image, pt, rect_game, thershold)){
        click(pt.x + offset_x, pt.y + offset_y);
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
    cv::Mat matchscreen = cv::imdecode(img1, cv::IMREAD_COLOR);
    return _match_picture(matchscreen, _pic_data[img2], matchLoc);
}

double GameScript::_match_picture(const cv::Mat& matchscreen, const cv::Mat& matchpic, cv::Point &matchLoc)
{
    double minVal;
    double maxVal = 0;
    cv::Point minLoc;
    cv::Point maxLoc;
    cv::Mat result;

    //匹配方式
    int match_method = cv::TM_CCOEFF_NORMED;

    if(matchpic.rows > matchscreen.rows || matchpic.cols > matchscreen.cols){
        maxVal = 0.0;
    }
    else
    {
        cv::matchTemplate(matchscreen, matchpic, result, match_method);
        cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );

        if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
            matchLoc = minLoc;
        }
        else{
            matchLoc = maxLoc;
        }
    }


    //    cv::imshow("image_window", screen);
    //    cv::imshow("result_window", pic);

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

typedef struct _FINDXYR
{
    int id;
    POINT pt;
}FINDXYR;

//几个抓鬼的支持函数
std::string GameScript::get_task_xy()
{
    return _get_xy_string("zdata", rect_task);
}

std::string GameScript::_get_xy_string(std::string dir, const RECT& rect)
{
    std::vector<FINDXYR> pointvec;
    std::string xy;
    std::string pic;

    cv::Mat matchscreen = cv::imdecode(
                get_screen_data(rect),
                cv::IMREAD_COLOR);


    //
    for(int i = 0; i < 11; i++)
    {
        if(i == 10)
        {
            pic = "dot";
        }
        else
        {
            pic = std::to_string(i);
        }

        pic.insert(0, "/");
        pic.insert(0, dir);
        check_pic_exists(pic);

        const cv::Mat& matchpic = _pic_data[pic];


        cv::Mat result_screen;
        cv::Mat result_pic;

        //处理成所有白字
        process_pic_task(matchscreen, result_screen);
        process_pic_task(matchpic, result_pic);

        double minVal;
        double maxVal;
        cv::Point matchLoc;
        cv::Point minLoc;
        cv::Point maxLoc;
        cv::Mat result;


        //匹配方式
        int match_method = cv::TM_CCOEFF_NORMED;

        if(matchpic.rows > matchscreen.rows || matchpic.cols > matchscreen.cols){
            maxVal = 0.0;
        }
        else{
            cv::matchTemplate(result_screen, result_pic, result, match_method);

            //匹配多个结果
            while(true)
            {
                cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
                if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
                    matchLoc = minLoc;
                }
                else{
                    matchLoc = maxLoc;
                }


                if(maxVal > 0.90)
                {
                    FINDXYR fr;
                    fr.pt.x = matchLoc.x + rect.left;
                    fr.pt.y = matchLoc.y + rect.top;
                    fr.id = i;
                    pointvec.push_back(fr);

                    //mhprintf(LOG_INFO, "x:%d y:%d %d %f", fr.pt.x, fr.pt.y, fr.id, maxVal);
                    cv::floodFill(result, matchLoc, cv::Scalar(0), 0, cv::Scalar(.1), cv::Scalar(1.));

                }
                else
                {
                    //没有匹配到
                    break;
                }
            }
        }
    }

    qDebug() << "find over, find size: " << pointvec.size();
    if(pointvec.size() == 0){
        throw std::runtime_error("can't find xy string in task aera!");
    }

    std::sort(pointvec.begin(), pointvec.end(), [](FINDXYR f1, FINDXYR f2){
        return f1.pt.x < f2.pt.x;
    });

    bool exists_dot = false;
    //找个数字为基准计算行上行下
    for(int i = 0; i < pointvec.size(); i++)
    {
        if(pointvec[i].id < 10){
         xy.push_back(std::to_string(pointvec[i].id).at(0));
        }
        else{
            //避免识别到开始就是dot的问题
            if(xy.size() != 0){
                if(exists_dot == false){
                    xy.push_back(',');
                    exists_dot = true;
                }
            }
        }
    }

    if(xy.size() < 3){
        throw std::runtime_error("finded xy string is error(size < 3)!");
    }

    return xy;
}

std::string GameScript::get_map_xy()
{
    //地图区域内找坐标
    return _get_xy_string("mdata", rect_game);
}


POINT GameScript::get_cur_game_mouse()
{
    return {cur_game_x, cur_game_y};
}



//任务区域
bool GameScript::find_red_line(std::string image, POINT &point, RECT rect)
{
    check_pic_exists(image);

    const cv::Mat& matchscreen = cv::imdecode(get_screen_data(rect), cv::IMREAD_COLOR);
    const cv::Mat& matchpic = _pic_data[image];

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
    //mhprintf(LOG_INFO, "find_red_line result:%f", result);
    bool ret = (result >= 0.95);
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

    qDebug() << QString::fromLocal8Bit("匹配: ")
             << QString::fromLocal8Bit(image.c_str())
             << maxVal
             << rect.left
             << rect.right
             << rect.top
             << rect.bottom;

    if(maxVal > threshold)
    {
        const cv::Mat& img_in = _pic_data[image];
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
    cur_game_x = 200;
    cur_game_y = 200;
    only_move(100, 100);
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
        cur_game_x = 200;
        cur_game_y = 200;
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

void GameScript::entry_game()
{
    //登录焦点的互斥
    std::lock_guard<std::mutex> locker(login_mutex);

    while(task_running)
    {
        ::SetForegroundWindow(wnd);

        if(::IsIconic(wnd)){
            mhprintf(LOG_INFO, "窗口最小化状态..");
            ::ShowWindow(wnd, SW_RESTORE);
        }

        //登录界面
        if(is_match_pic_in_screen("登录界面"))
        {
            click_nofix("进入游戏按钮");
        }
        else if(is_match_pic_in_screen("服务器信息"))
        {
            click_nofix("下一步按钮");
        }
        else if(is_match_pic_in_screen("选择服务器"))
        {
            click_nofix("下一步按钮");
        }
        else if(is_match_pic_in_screen("输入密码界面"))
        {
            while(is_match_pic_in_screen("为了你的帐号安全标识")){
                mhsleep(500, false);
            }

            input_password("pengchang@live.cn,123456");
            click_nofix("下一步按钮");
        }
        else if(is_match_pic_in_screen("游戏内.png"))
        {
            break;
        }
        else if(is_match_pic_in_screen("选择角色"))
        {
            click_nofix("确定按钮");
            click_nofix("下一步");
        }
        else
        {
            mhprintf(LOG_INFO, "未知场景..");
        }

        mhsleep(WAIT_NORMAL, false);
    }
}

void GameScript::click_nofix(int x, int y)
{
    int v = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, v);
    ::PostMessage(wnd, WM_LBUTTONDOWN, 1,v);
    ::PostMessage(wnd, WM_LBUTTONUP, 0,v);
    mhsleep(WAIT_NORMAL, false);
    only_move(100, 100);
}

//匹配屏幕获取当前鼠标位置
POINT GameScript::get_cur_mouse()
{

    mhsleep(WAIT_POSTMSG);
    cv::Point maxLoc;
    double val = match_picture(get_screen_data(), _pic_data["pic/chk/mouse2.png"], maxLoc);
    maxLoc.x -= 17;
    maxLoc.y -= 17;


    if(val < 0.90)
    {
        val = match_picture(screen_data(), _pic_data["pic/chk/mouse1.png"], maxLoc);
        maxLoc.x -= 5;
        maxLoc.y -= 5;
    }

    if(val < 0.90)
    {
        val = match_picture(screen_data(), _pic_data["pic/chk/mouse4.png"], maxLoc);
        maxLoc.x -= 30;
    }


    //单独处理alt+g的图标
    if(val < 0.90)
    {
        cv::Mat ori = cv::imdecode(screen_data(), cv::IMREAD_COLOR);
        cv::Mat processed_screen;
        process_pic_mouse(ori, processed_screen);
        val = _match_picture(processed_screen, _pic_data["pic/chk/mouse3.png"], maxLoc);
    }


    if(val < 0.90)
    {
        rand_move_mouse();
        throw std::runtime_error("没有检测到鼠标, 重新检测");
    }

    return {maxLoc.x, maxLoc.y};
}

void GameScript::move_mouse_vec(int x, int y, int tar_x, int tar_y)
{
    std::vector<int> mouse;
    get_mouse_vec(x, y, tar_x, tar_y, mouse);
    for(size_t i = 0; i < mouse.size(); i++){
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, mouse[i]);
        mhsleep(11, false);
    }

    mhsleep(WAIT_NORMAL, false);
}

void GameScript::slow_click(int x1, int y1, int lbutton)
{
    std::vector<int> mouse;
    get_mouse_vec(cur_game_x, cur_game_y, x1, y1, mouse);
    for(size_t i = 0; i < mouse.size(); i++){
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, mouse[i]);
        mhsleep(9, false);
    }

    //mhsleep(WAIT_NORMAL);  //停稳了...
    int last = *mouse.end();
    if(lbutton == MOUSE_LEFT_CLICK)
        ::PostMessage(wnd, WM_LBUTTONDOWN, 1, last);
    else if(lbutton == MOUSE_RIGHT_CLICK)
        ::PostMessage(wnd, WM_RBUTTONDOWN, 1, last);

    if(lbutton == MOUSE_LEFT_CLICK)
        ::PostMessage(wnd, WM_LBUTTONUP, 0, last);
    else if(lbutton == MOUSE_RIGHT_CLICK)
        ::PostMessage(wnd, WM_RBUTTONUP, 0, last);

    cur_game_x = x1;
    cur_game_y = y1;

    //这个如果没有会导致点击无效
    mhsleep(WAIT_NORMAL, false);
}


void GameScript::click_nomove(int x, int y, bool bclick)
{
    int xy = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, xy);
    cur_game_x = x;
    cur_game_y = y;

    mhsleep(WAIT_NORMAL, false);
    if(bclick){
        ::PostMessage(wnd, WM_LBUTTONDOWN, 1, xy);
        ::PostMessage(wnd, WM_LBUTTONUP, 0, xy);
        mhsleep(WAIT_NORMAL, false);
    }
}

void GameScript::key_press(std::string key)
{
    std::lock_guard<std::mutex> locker(topwnd_mutex);
    mhprintf(LOG_INFO, "按键 %s", key.c_str());
    top_wnd();
    if(key == "TAB"){
        PostMessage(wnd, WM_KEYDOWN, VK_TAB, 0x000f0001);
        PostMessage(wnd, WM_KEYUP, VK_TAB, 0xc00f0001);
    }
    else if(key == "F8"){
        PostMessage(wnd, WM_KEYDOWN, VK_F8, 0x00420001);
        PostMessage(wnd, WM_KEYUP, VK_F8, 0xc0420001);
    }
    else if(key == "F9"){
        PostMessage(wnd, WM_KEYDOWN, VK_F9, 0x00430001);
        PostMessage(wnd, WM_KEYUP, VK_F9, 0xc0430001);
    }
    else if(key == "ALT+H"){
        PostMessage(wnd, WM_SYSKEYDOWN, VK_MENU, 0x20380001);
        PostMessage(wnd, WM_SYSKEYDOWN, 0x48, 0x20230001);
        PostMessage(wnd, WM_KEYUP, VK_MENU, 0xc0380001);
        PostMessage(wnd, WM_KEYUP, 0x48, 0xc0230001);
    }
    else if(key == "ALT+E"){
        PostMessage(wnd, WM_SYSKEYDOWN, VK_MENU, 0x20380001);
        PostMessage(wnd, WM_SYSKEYDOWN, 0x45, 0x20120001);
        PostMessage(wnd, WM_KEYUP, VK_MENU, 0xc0380001);
        PostMessage(wnd, WM_SYSKEYUP, 0x45, 0xe0120001);
    }
    else if(key == "ALT+G"){
        PostMessage(wnd, WM_SYSKEYDOWN, VK_MENU, 0x60380001);
        PostMessage(wnd, WM_SYSKEYDOWN, 0x47, 0x20220001);
        PostMessage(wnd, WM_SYSKEYUP, 0x47, 0xe0220001);
        PostMessage(wnd, WM_KEYUP, VK_MENU, 0xc0380001);
    }
    else if(key == "ALT+A"){
        PostMessage(wnd, WM_SYSKEYDOWN, VK_MENU, 0x20380001);
        PostMessage(wnd, WM_SYSKEYDOWN, 0x41, 0x201e0001);
        PostMessage(wnd, WM_KEYUP, VK_MENU, 0xc0380001);
        PostMessage(wnd, WM_KEYUP, 0x41, 0xc01e0001);
    }
    else if(key == "ALT+W")
    {
        PostMessage(wnd, WM_SYSKEYDOWN, VK_MENU, 0x20380001);
        PostMessage(wnd, WM_SYSKEYDOWN, 0x57, 0x20110001);
        PostMessage(wnd, WM_KEYUP, VK_MENU, 0xc0380001);
        PostMessage(wnd, WM_KEYUP, 0x57, 0xc0110001);
    }

    mhsleep(WAIT_NORMAL, false);
}


//传进来的是窗口坐标
//转化成游戏内坐标, 之后用WM_MOUSEMOVE移动
void GameScript::click(int x, int y, int lbutton)
{
    POINT now;

    only_move(x, y);

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

void GameScript::only_move(int x, int y)
{
    std::vector<int> r;

    //y - 70 还不够, 会被游戏偏移鼠标到窗口外, 大致的移动过去
    int tar_x = (x > 550 ? x - 90:x);
    tar_x = (x < 40 ? x + 40: tar_x);
    int tar_y = (y > 400 ? y - 50:y);
    tar_y = (y < 40 ? y + 40: tar_y);
    get_mouse_vec(cur_game_x, cur_game_y, tar_x, tar_y, r);

    for(size_t i = 0; i < r.size(); i++)
    {
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, r[i]);
        mhsleep(1, false);
    }

    cur_game_x = tar_x;
    cur_game_y = tar_y;
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

        _screen_data.clear();

        for(int i = 0; i < sizeof(BITMAPFILEHEADER); i++){
            uchar* buf = (uchar*)&bmfHeader;
            _screen_data.push_back(buf[i]);
        }

        for(int i = 0; i < sizeof(BITMAPINFOHEADER); i++){
            uchar* buf = (uchar*)&bi;
            _screen_data.push_back(buf[i]);
        }

        for(int i = 0; i < dwBmpSize; i++){
            _screen_data.push_back(screen_buf[i]);
        }

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
            throw exception_status("在等待操作时游戏内状态发生变化");
        }

        if(task_running == false){
            throw std::runtime_error("在等待操作时检测到停止标志");
        }
    }

    Sleep(ms);
}

//等待出现
void GameScript::wait_appear(std::string name, RECT rect, double threshold)
{
    int times = WAIT_TIMES;


    while(times)
    {
        if(is_match_pic_in_screen(name, rect, threshold))
            break;

        qDebug() << QString::fromLocal8Bit("等待出现[")
                 << QString::fromLocal8Bit(name.c_str()) << QString::fromLocal8Bit("]..");

        mhsleep(WAIT_NORMAL);
        times--;
    }

    if(times == 0)
    {
        std::string info;
        info += "等待[";
        info += name;
        info += "]出现失败";
        throw std::runtime_error(info);
    }
}

//过静态验证
void GameScript::pass_static_check()
{
    for(int i = 0; i <= 3; i++)
    {
        int fx; //方向
        int qd;
        int yzx;
        int yzy;
        int yzwndx;
        int yzwndy;
        std::string checkaddr("11820AE4]+74]+0]+74]+");
        checkaddr += int2hex(i*4);
        checkaddr += "]+28]";

        bool find_check = false;
        if(readmem<int>(game_process, checkaddr) == 0)
        {
            for(int iii = 1; iii <= 2; iii++){

                std::string fxaddr("11820ae4]+74]+0]+74]+");
                fxaddr += int2hex(i*4);
                fxaddr += "]+74+";
                fxaddr += int2hex(iii*4);
                fxaddr += "]+74]+28]+4]+10]+c]+30";

                fx = readmem<int>(game_process,fxaddr);
                if(fx == 1 || fx == 3){
                    qd = iii * 4;
                    find_check = true;
                    break;
                }
            }

            if(find_check == false)
            {
                mhprintf(LOG_INFO, "no find check addr");
                break;
            }

            for(int iii = 0; iii <= 3; iii++){
                std::string checkaddr1("11820ae4]+74]+0]+74]+");
                checkaddr1 += int2hex(i * 4);
                checkaddr1 += "]+74]+";
                checkaddr1 += int2hex(qd + iii * 4);
                checkaddr1 += "]+74]+28]+4]+10]+C]+30";
                if(readmem<int>(game_process, checkaddr1) == 1){
                    std::string yzxaddr("11820ae4]+74]+0]+74]+");
                    yzxaddr += int2hex(i*4);
                    yzxaddr += "]+74+";
                    yzxaddr += int2hex(qd + iii*4);
                    yzxaddr += "]+8";
                    std::string yzyaddr("11820ae4]+74]+0]+74]+");
                    yzyaddr += int2hex(i*4);
                    yzyaddr += "]+74+";
                    yzyaddr += int2hex(qd + iii*4);
                    yzyaddr += "]+c";

                    yzx = readmem<int>(game_process, yzxaddr);
                    yzy = readmem<int>(game_process, yzyaddr);

                    std::string yzwndxaddr("11820ae4]+74]+0]+74]+");
                    yzwndxaddr += int2hex(i*4);
                    yzwndxaddr += "]+8";

                    std::string yzwndyaddr("11820ae4]+74]+0]+74]+");
                    yzwndyaddr += int2hex(i*4);
                    yzwndyaddr += "]+c";

                    yzwndx = readmem<int>(game_process, yzwndxaddr);
                    yzwndy = readmem<int>(game_process, yzwndyaddr);

                    qDebug() << "right x = " << yzwndx + yzx;
                    qDebug() << "right y = " << yzwndy + yzy;
                }
            }
        }
        else{
            mhprintf(LOG_INFO, "no check");
        }
    }
}
