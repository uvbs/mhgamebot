﻿#include "gamescript.h"
#include <regex>
#include <thread>
#include <boost/lexical_cast.hpp>
#include <codecvt>
#include <opencv2/core/types_c.h>


#define MHCHATWND "梦幻西游2 聊天窗口"

#define  script_inst  GameScript::get_instance(L)

#define FIRSTTASK(x)\
    if(mouse.is_match_pic_in_screen("pic\\"#x, pt))

#define FINDTASK(x) \
    else if(mouse.is_match_pic_in_screen("pic\\"#x, pt))

#define ENDTASK \
    else { mhprintf("什么任务.."); mouse.rand_move_mouse(); }


void on_trackbar1(int pos, void* param)
{
    cv::Mat *ori_img = (cv::Mat*)param;
    cv::Mat bin_img;

    // 转为二值图
    cv::threshold(*ori_img, bin_img, pos, 255, CV_THRESH_BINARY);

    // 显示二值图
    cv::imshow("ori", bin_img);
}

void on_trackbar2(int pos, void* param)
{
    cv::Mat *ori_img = (cv::Mat*)param;
    cv::Mat bin_img;

    // 转为二值图
    cv::threshold(*ori_img, bin_img, pos, 255, CV_THRESH_BINARY);

    // 显示二值图
    cv::imshow("test", bin_img);
}



//静态初始化
std::map<lua_State*, GameScript*> GameScript::inst_map;
std::mutex GameScript::topwnd_mutex;

//构造函数
GameScript::GameScript(HWND game_wnd, int id):
    player_name(std::string("窗口")+boost::lexical_cast<std::string>(id)),
    player_level("0")
{
    script_id = id;
    wnd = game_wnd;
    lua_status = nullptr;
    if(lua_status == nullptr)
        lua_status = luaL_newstate();

    if(lua_status == nullptr)
        throw std::runtime_error("创建lua状态失败");

    //将这个lua状态和这个实例绑定
    inst_map[lua_status] = this;


    lua_atpanic(lua_status, [](lua_State* L)->int{
        const char* err_str = lua_tostring(L, 1);
        script_inst->mhprintf(err_str);
        lua_pop(L, 1);

        throw std::runtime_error("lua脚本异常");
    });



    hdc = ::GetDC(game_wnd);
    script_id = id;

    ratio_x = (double)590/(double)SCREEN_WIDTH;
    ratio_y = (double)450/(double)SCREEN_HEIGHT;

    rx = 0;
    ry = 0;
    cur_game_x = 0;
    cur_game_y = 0;
}

void GameScript::mhprintf(const char* msg, ...)
{
    static bool can_printf = true;
tryagain:
    if(can_printf == true)
    {

        can_printf = false;


        //需要一个互斥
        //TODO:
        printf("%s: ", player_name.c_str());

        va_list va;
        va_start(va, msg);
        vprintf(msg, va);
        va_end(va);

        printf("\n");

        can_printf = true;
    }
    else
    {
        Sleep(50);
        goto tryagain;
    }
}


bool GameScript::is_in_city(const char* city)
{


    POINT pt;
    if(is_match_pic_in_screen(city, pt))
    {
        mhprintf("当前城市 %s..", city);
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
        mhprintf("检测到网络断线...");
        mhprintf("尝试重启..");

        ::SendMessageA(wnd, WM_COMMAND, 1, 0);
        return true;
    }


    return false;
}

void GameScript::test_lua(const char *err)
{
    mhprintf(err);
}

void GameScript::call_lua_func(const std::string& name)
{
    //调用这个名字的函数
    std::string str_name(name);
    str_name += "()";
    if(0 != luaL_dostring(lua_status, str_name.c_str()))
    {
        const char * err = lua_tostring(lua_status, -1);
        mhprintf(err);
        lua_pop(lua_status, 1);
        throw std::runtime_error("lua脚本执行失败");
    }
}

void GameScript::match_task()
{
    for(auto name :lua_task_list)
    {
        //backup
        std::string taskname(name);
        check_pic_exists(taskname);

        if(is_match_pic_in_screen(taskname.c_str()))
        {
            mhprintf("开始任务->%s", taskname.c_str());
            call_lua_func(name);
            break;
        }
    }

}

void GameScript::read_global(bool read)
{
    lua_pushglobaltable(lua_status);
    /* table is in the stack at index 't' */
    lua_pushnil(lua_status);  /* first key */
    while (lua_next(lua_status, -2) != 0) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        const char* name = lua_tostring(lua_status, -2);

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
                lua_task_list.push_back(name);
                mhprintf(name);
            }
        }


        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(lua_status, 1);
    }

    mhprintf("检测到脚本任务数量: %d", lua_task_list.size());
}

void GameScript::load_lua_file(const char* name)
{
    //加载任务脚本
    if(LUA_OK != luaL_dofile(lua_status, name))
    {
        const char * err = lua_tostring(lua_status, -1);
        mhprintf(err);
        lua_pop(lua_status, 1);
        throw std::runtime_error("加载lua脚本失败");
    }
    
    mhprintf("加载脚本: %s ok..", name);

}

//关掉一些东西
void GameScript::close_game_wnd_stuff()
{
    //关对话框...
    while(is_match_pic_in_screen("pic\\关闭3.png", rect_game, 8)){
        click("pic\\关闭3.png", 8);
        click("pic\\关闭1.png");
        click("pic\\关闭2.png");
        click("pic\\取消.png");
    }
}

void GameScript::do_task()
{
    can_task = true;
    
    //之前遍历一次
    read_global(true);
    load_lua_file("任务.lua");
    read_global(false);

    load_lua_file("战斗.lua");

    while(can_task)
    {

        try
        {

            if(check_offline()) break;
            close_game_wnd_stuff();

            PLAYER_STATUS status = get_player_status();
            if(status == PLAYER_STATUS::COMBAT)
            {
                //有菜单出现再进行操作
                if(is_match_pic_in_screen("pic\\战斗-菜单2.png"))
                {
                    call_lua_func("战斗回调");
                    bool processed = lua_toboolean(lua_status, -1);
                    if(processed == false){
                        //无事可做, 自动战斗吧
                        click("pic\\自动战斗.png");
                    }
                }
            }
            else if(status == PLAYER_STATUS::NORMAL){
                match_task();
            }
        }
        catch(exception_xy &e){
            mhprintf("%s, 重新遍历任务", e.what());
        }
        catch(exception_status &e){
            mhprintf("%s, 重新尝试一次", e.what());
        }
    }
    
    can_task = true;
}


//当前玩家的状态
//登录界面
//载入界面
void GameScript::entry_game()
{
    //找图检测吧

    while(1)
    {
        //登录界面
        if(is_match_pic_in_screen("pic\\登录界面.png"))
        {
            click_nofix("pic\\进入游戏按钮.png");
        }
        else if(is_match_pic_in_screen("pic\\服务器信息.png"))
        {
            click_nofix("pic\\下一步按钮.png");
        }
        else if(is_match_pic_in_screen("pic\\选择服务器.png"))
        {
            click_nofix("pic\\下一步按钮.png");
        }
        else if(is_match_pic_in_screen("pic\\输入密码界面.png"))
        {
            input_password("wanghao11911988@163.com,aa118118");
            click_nofix("pic\\下一步按钮.png");
        }
        else if(is_match_pic_in_screen("pic\\游戏内.png"))
        {
            //获取玩家姓名
            char title[256];
            ::GetWindowTextA(wnd, title, 256);
            //mhprintf("%s", title);


            //用正则匹配出来
            std::regex regex(R"(.*-\s?(.+)\[\d+\].*)");
            std::cmatch cmatch;
            if(std::regex_match(title, cmatch, regex))
            {
                player_name = cmatch[1];
                set_player_name(cmatch[1]);

                //debug
                //mhprintf("%s", cmatch[1]);
            }

            //进入游戏才出来...
            break;
        }
        else if(is_match_pic_in_screen("pic\\选择角色.png"))
        {
            click_nofix("pic\\确定按钮.png");
            click_nofix("pic\\下一步按钮.png");
        }
        else
        {
            mhprintf("未知场景..");
        }

        Sleep(100);
    }

}


//获取玩家状态
PLAYER_STATUS GameScript::get_player_status()
{
    if(is_match_pic_in_screen("pic\\战斗中.png"))
    {
        mhprintf("战斗状态");
        return PLAYER_STATUS::COMBAT;
    }
    else if(is_match_pic_in_screen("pic\\游戏内.png"))
    {
        mhprintf("平常状态");
        return PLAYER_STATUS::NORMAL;
    }
    else if(is_match_pic_in_screen("pic\\体验状态.png"))
    {
        mhprintf("体验状态");
        return PLAYER_STATUS::NOTIME;
    }
    else if(is_match_pic_in_screen("pic\\跳过动画.png"))
    {
        mhprintf("动画状态");
        return PLAYER_STATUS::GC;
    }
    else
    {
        throw exception_status("未知的玩家状态");
    }

}


//结束任务
void GameScript::end_task()
{
    can_task = false;
}

void GameScript::top_wnd()
{
    ::SetForegroundWindow(wnd);
    Sleep(200);
}

//用来处理任务区域的匹配, 通过过滤掉除了红色以外的像素
void GameScript::process_pic(cv::Mat src, cv::Mat &result)
{
    //找任务的红字部分
    cv::Mat out_red;
    cv::Mat out_red1;
    cv::inRange(src, cv::Scalar(0, 255, 0, 0), cv::Scalar(0, 255, 255, 0), out_red);
    cv::inRange(src, cv::Scalar(0, 254, 0, 0), cv::Scalar(0, 254, 255, 0), out_red1);
    cv::bitwise_or(out_red, out_red1, result);
    cv::inRange(src, cv::Scalar(179, 187, 240, 0), cv::Scalar(179, 187, 240, 0), out_red1);
    cv::bitwise_or(result, out_red1, result);
}

void GameScript::process_pic_red(cv::Mat src)
{
    int iLowH = 0;
    int iHighH = 0;

    int iLowS = 90;
    int iHighS = 255;

    int iLowV = 90;
    int iHighV = 255;

    //Threshold the image
    inRange(src, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), src);
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

    REGLUAFUN(lua_status, "延迟", [](lua_State* L){
        int time = lua_tointeger(L, 1);
        Sleep(time * 1000);
        return 0;
    });

    REGLUAFUN(lua_status, "点击小地图坐标", [](lua_State* L){
        return 0;
    });

    REGLUAFUN(lua_status, "调试信息", [](lua_State* L)->int{
        std::string info = lua_tostring(L, 1);
        script_inst->mhprintf(info.c_str());
        return 0;
    });


    REGLUAFUN(lua_status, "关闭无关窗口", [](lua_State* L)->int{
        script_inst->close_game_wnd_stuff();
        return 0;
    });

    REGLUAFUN(lua_status, "移动鼠标到", [](lua_State* L)->int{
        int arg_counts = lua_gettop(L); //参数数量
        if(arg_counts > 2)
            throw std::runtime_error("参数数量错误");

        std::string name = lua_tostring(L, 1);
        script_inst->check_pic_exists(name);

        POINT pt;
        if(arg_counts == 1){
            if(false == script_inst->is_match_pic_in_screen(name.c_str(), pt)){
                throw std::runtime_error("函数使用错误, 确保能匹配到");
            }

            script_inst->click(pt.x, pt.y, 3);   //只移动
        }
        else{
            int thershold = lua_tointeger(L, 2);
            if(false == script_inst->is_match_pic_in_screen(name.c_str(), pt, rect_game, thershold)){
                throw std::runtime_error("函数使用错误, 确保能匹配到");
            }

            script_inst->click(pt.x, pt.y, 3);   //只移动
        }


        lua_pushinteger(L, pt.x);
        lua_pushinteger(L, pt.y);
        return 2;
    });

    REGLUAFUN(lua_status, "按键", [](lua_State* L)->int{
        script_inst->top_wnd();
        std::string keystr = lua_tostring(L, 1);
        script_inst->key_press(keystr);
        return 0;
    });

    REGLUAFUN(lua_status, "存在任务", [](lua_State* L)->int{

        std::string img = lua_tostring(L, 1);
        script_inst->check_pic_exists(img);

        script_inst->mhprintf("匹配任务 %s", img.c_str());

        std::vector<uchar>&& screen_buf = script_inst->get_screen_data(rect_task);
        cv::Mat matchscreen = cv::imdecode(screen_buf, cv::IMREAD_COLOR);
        cv::Mat matchpic = cv::imread(img.c_str(), cv::IMREAD_COLOR);
        cv::Mat result;


        //转换到hsv
        cv::Mat hsv_screen;
        cv::Mat hsv_pic;
        cv::cvtColor(matchscreen, hsv_screen, cv::COLOR_BGR2HSV);
        cv::cvtColor(matchpic, hsv_pic, cv::COLOR_BGR2HSV);

        cv::Mat result_screen;
        cv::Mat result_pic;

        //找任务的红字部分
        script_inst->process_pic(hsv_screen, result_screen);
        script_inst->process_pic(hsv_pic, result_pic);

        cv::Point matchLoc;
        double maxVal = script_inst->_match_picture(hsv_screen, hsv_pic, matchLoc);
        script_inst->mhprintf("匹配结果 %f", maxVal);

        if(maxVal >= 0.7)
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);

        return 1;
    });

    REGLUAFUN(lua_status, "走到旁边", [](lua_State* L)->int{
        std::string img = lua_tostring(L, 1);
        script_inst->check_pic_exists(img);
        POINT pt;
        if(script_inst->is_match_pic_in_screen(img.c_str(), pt)){
            if(pt.x < 20) //点右边
                pt.x += 50;
            else{
                pt.x -= 50; //左边
            }

            script_inst->click(pt.x, pt.y);
        }
        else{
            throw std::runtime_error("函数使用错误, 确保能匹配到图片");
        }

        script_inst->until_stop_run();

        return 0;
    });

    REGLUAFUN(lua_status, "移动直到遇到", [](lua_State* L)->int{
        int arg_counts = lua_gettop(L);
        if(arg_counts != 2 && arg_counts != 3)
            throw std::runtime_error("参数数量错误");

        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);
        std::string img = lua_tostring(L, 3);
        double threshold = 0.7;
        if(arg_counts == 4)
            threshold = lua_tointeger(L, 4)/10;

        while(true)
        {
            script_inst->click(x, y, threshold);
            script_inst->until_stop_run();
            script_inst->mhprintf("一次");

            //退出条件
            if(script_inst->is_match_pic_in_screen(img.c_str(), rect_game, threshold) == true){
                break;  //退
            }
        }

        return 0;
    });

    REGLUAFUN(lua_status, "结束脚本", [](lua_State* L)->int{
        std::string reason = lua_tostring(L, 1);
        script_inst->mhprintf(reason.c_str());
        script_inst->end_task();
        return 0;
    });
    
    REGLUAFUN(lua_status, "释放法术", [](lua_State* L)->int{
        
        //前置检测
        if(script_inst->get_player_status() != COMBAT)
            throw std::runtime_error("非战斗状态使用了释放法术, 检查脚本逻辑");
        
        std::string imgname = lua_tostring(L, 1);
        script_inst->check_pic_exists(imgname);
        
        script_inst->click("战斗-法术");
        script_inst->click("战斗-法术1");
        script_inst->click(imgname.c_str());
        return 0;
    });
    
    REGLUAFUN(lua_status, "玩家宠物", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        script_inst->check_pic_exists(imgname);

        script_inst->click(point_pet.x, point_pet.y);
        script_inst->click(imgname.c_str());

        return 0;
    });

    REGLUAFUN(lua_status, "存在图片", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        script_inst->check_pic_exists(imgname);
        bool bexist;
        
        if(lua_gettop(L) == 2){
            int threshold = lua_tointeger(L, 2);
            bexist = script_inst->is_match_pic_in_screen(imgname.c_str(), rect_game, threshold);
        }
        else{
            bexist = script_inst->is_match_pic_in_screen(imgname.c_str());
        }

        lua_pushboolean(L, bexist);
        return 1;
    });

    REGLUAFUN(lua_status, "使用辅助技能", [](lua_State* L)->int{
        const std::string name = lua_tostring(L, 1);
        if(name == "烹饪"){
            script_inst->click(point_player.x, point_player.y);
            script_inst->click("pic\\辅助技能.png");
            script_inst->click("pic\\烹饪.png");
        }
        else{
            script_inst->mhprintf("参数错误 找唱哥");
        }

        return 0;
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
        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);
        script_inst->rand_move_mouse();
        script_inst->click(x, y);
        script_inst->until_stop_run(1500);
        return 0;
    });

    REGLUAFUN(lua_status, "获取玩家状态", [](lua_State *L)->int{
        script_inst->do_money();
        return 0;
    });

    REGLUAFUN(lua_status, "当前城市", [](lua_State *L)->int{
        std::string imgname = lua_tostring(L, 1);

        bool isin = script_inst->is_in_city(imgname.c_str());
        lua_pushboolean(L, isin);
        return 1;
    });

    REGLUAFUN(lua_status, "对话", [](lua_State* L)->int{

        //隐藏玩家
        script_inst->key_press(VK_F9);
        script_inst->click(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

        //等待对话框弹出
        int counts = 0;
        while(counts < 10){

            if(script_inst->is_match_pic_in_screen("pic\\关闭3.png")){
                break;
            }

            script_inst->mhprintf("等待对话框弹出..");
            counts++;
            Sleep(200);
        }


        return 0;
    });

    REGLUAFUN(lua_status, "点击对话框内图片", [](lua_State* L)->int{
        std::lock_guard<std::mutex> locker(GameScript::topwnd_mutex);
        std::string imgname = lua_tostring(L, 1);
        imgname.insert(0, "对话框\\");

        //如果不存在, 循环2秒
        int times = 0;
        while(times < 5){
            if(script_inst->is_match_pic_in_screen(imgname) == false){
                Sleep(500);
            }
            else{
                script_inst->dialog_click(imgname.c_str());
            }

            times++;
        }



        return 0;
    });

    REGLUAFUN(lua_status, "点击任务", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        POINT pt;

        //先检测三个长度的下划线
        if(script_inst->find_color("可点击下划线1", pt)){
            script_inst->click(pt.x, pt.y);
        }
        else if(script_inst->find_color("可点击下划线", pt)){
            script_inst->click(pt.x, pt.y);
        }
        else{
            char buf[100];
            sprintf(buf, "图片 %s 没有匹配到", "可点击下划线2");
            throw std::runtime_error(buf);
        }

        return 0;
    });

    REGLUAFUN(lua_status, "隐藏玩家", [](lua_State* L)->int{
        script_inst->key_press(VK_F9);
        return 0;
    });

    REGLUAFUN(lua_status, "点击图片", [](lua_State* L)->int{
        int arg_counts = lua_gettop(L);

        std::string imgname = lua_tostring(L, 1);

        if(arg_counts == 1){
            script_inst->click(imgname.c_str());
        }
        else if(arg_counts == 2){
            int threshold = lua_tointeger(L, 2);
            script_inst->click(imgname.c_str(), threshold);
        }
        else if(arg_counts == 3){
            int x = lua_tointeger(L, 2);
            int y = lua_tointeger(L, 3);
            POINT pt;
            if(script_inst->is_match_pic_in_screen(imgname, pt, rect_game))
                script_inst->click(pt.x + x, pt.y + y);
            else{
                script_inst->mhprintf("这个图片不存在");
            }
        }
        else if(arg_counts == 4){
            int x = lua_tointeger(L, 2);
            int y = lua_tointeger(L, 3);
            int thershold = lua_tointeger(L, 4);
            POINT pt;
            if(script_inst->is_match_pic_in_screen(imgname, pt, rect_game, thershold))
                script_inst->click(pt.x + x, pt.y + y);
            else{
                script_inst->mhprintf("这个图片不存在");
            }
        }
        else{
            throw std::runtime_error("参数数量错误");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "等待停止奔跑", [](lua_State* L)->int{
        script_inst->until_stop_run();
        return 0;
    });

    REGLUAFUN(lua_status, "装备物品", [](lua_State* L)->int{
        //首先打开背包
        POINT zinv;
        if(script_inst->is_match_pic_in_screen("pic\\子女按钮", zinv) == false){
            script_inst->key_press("ALT+E");
        }

        RECT bag_rect;
        bag_rect.left = zinv.x + 15;
        bag_rect.top = zinv.y + 15;
        bag_rect.bottom = SCREEN_HEIGHT;
        bag_rect.right = SCREEN_WIDTH;

        std::string imgname = lua_tostring(L, 1);
        imgname.insert(0, "物品\\");
        POINT pt;
        if(script_inst->is_match_pic_in_screen(imgname, pt, bag_rect) == false)
            script_inst->mhprintf("装备的物品不存在, 这不是一个严重错误");
        else
            script_inst->rclick(pt.x, pt.y);

        return 0;
    });

    REGLUAFUN(lua_status, "点击小地图", [](lua_State* L)->int{
        int arg_counts = lua_gettop(L);
        if(arg_counts != 1 && arg_counts != 3)
            throw std::runtime_error("参数数量错误");

        std::string name = lua_tostring(L, 1);
        name.insert(0, "小地图\\");

        //打开小地图
        script_inst->key_press(VK_TAB);
        script_inst->until_stop_run(2000);

        //点击图片
        int offset_x = 0;
        int offset_y = 0;
        if(arg_counts == 3){
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
                throw std::runtime_error("确保能匹配到图片");
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

void GameScript::run()
{
    //运行脚本
    mhprintf("%d脚本执行", std::this_thread::get_id());


    regist_lua_fun();


    mhprintf("开始进入游戏");
    entry_game();
    mhprintf("进去游戏[完成]");


    mhprintf("执行脚本: %s", Script_type_desc[config->type].str.c_str());

    if(config->type == Script_type::SMART){

    }
    else if(config->type == Script_type::MONEY){
        do_money();
    }
    else if(config->type == Script_type::LEVEL){
        do_task();
    }
    else if(config->type == Script_type::DAILY){
        do_daily();  //日常
    }
    else{
        throw std::runtime_error("未知脚本类型");
    }

}

void GameScript::do_daily()
{
    can_task = true;
    load_lua_file("日常.lua");
    load_lua_file("日常-战斗.lua");

    //开一个单独的线程检测战斗状态
    std::thread check_combat([&](){
        this->mhprintf("检测战斗状态..");
        if(this->get_player_status() == COMBAT){
            this->mhprintf("战斗状态中..");


        }
    });

    check_combat.join();


    while(can_task)
    {
        try
        {
            PLAYER_STATUS status = get_player_status();
            if(status == NORMAL)
                call_lua_func("日常1");
            else if(status == COMBAT)
                mhprintf("战斗中...");


            mhprintf("一次脚本完成了");

        }
        catch(exception_xy &e){
            mhprintf("%s, 重新遍历任务", e.what());
        }
        catch(exception_status &e){
            mhprintf("%s, 重新尝试一次", e.what());
        }
    }
}

int GameScript::make_mouse_value(int x, int y)
{
    int v = 0;
    v = x;
    y = y << 16;
    v = v + y;

    return v;
}

std::vector<uchar> GameScript::get_screen_data()
{
    RECT rect = {};
    ::GetClientRect(wnd, &rect);

    //mhprintf("GetClientRect: %d, %d, %d, %d", client.left, client.top, client.right, client.bottom);

    return get_screen_data(rect);
}

void GameScript::input_password(const char* input)
{
    
    for(size_t i = 0; i < strlen(input); i++){
        
        if(input[i] == '@')
        {
            keybd_event(16, 0, 0 ,0);
            Sleep(100);
            keybd_event(0x32,0,0,0);     //按下a键
            Sleep(50);
            keybd_event(0x32,0,KEYEVENTF_KEYUP,0);//松开a键
            Sleep(50);
            keybd_event(16,0,KEYEVENTF_KEYUP,0);//松开a键
            Sleep(100);
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
            Sleep(400);
        }
        else{
            keybd_event(toupper(input[i]),0,0,0);     //按下a键
            keybd_event(toupper(input[i]),0,KEYEVENTF_KEYUP,0);//松开a键
            Sleep(rand()%200+100);
        }
        

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
        mhprintf("点击一个屏幕不存在的图片 %s", image);
    }
}

void GameScript::click(const char* image, int threshold)
{
    POINT point;
    if(is_match_pic_in_screen(image, point, rect_game, threshold)){
        mhprintf("点击图片 %s", image);
        click(point.x, point.y);
    }
    else{
        mhprintf("点击一个屏幕不存在的图片 %s", image);
    }
}

void GameScript::click_nofix(const char* image)
{

    POINT point;
    if(is_match_pic_in_screen(image, point)){
        click_nofix(point.x, point.y);
    }

}


bool GameScript::is_match_pic_in_screen(std::string image, RECT rect, int threshold)
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

double GameScript::_match_picture(const cv::Mat screen, const cv::Mat pic, cv::Point &matchLoc)
{
    double minVal;
    double maxVal;
    cv::Point minLoc;
    cv::Point maxLoc;
    cv::Mat result;

    // Create the result matrix
    int result_cols =  screen.cols - pic.cols + 1;
    int result_rows = screen.rows - pic.rows + 1;

    result.create( result_rows, result_cols, 16 );

    //mhprintf("%matchpic %d %d", matchpic.size().height, matchpic.size().width);
    //mhprintf("%matchscreen %d %d", matchscreen.size().height, matchscreen.size().width);

    //匹配方式
    int match_method = cv::TM_CCOEFF_NORMED;
    cv::matchTemplate(screen, pic, result, match_method);
    cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }
    else{
        matchLoc = maxLoc;
    }


    //    cv::rectangle( matchpic, matchLoc, Point( matchLoc.x + img_in.cols , matchLoc.y + img_in.rows ), Scalar::all(0), 2, 8, 0 );
    //    cv::rectangle( result, matchLoc, Point( matchLoc.x + img_in.cols , matchLoc.y + img_in.rows ), Scalar::all(0), 2, 8, 0 );

    //cv::imshow("image_window", screen);
    //cv::imshow("result_window", pic);

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


//任务区域
bool GameScript::find_color(std::string image, POINT &point)
{
    check_pic_exists(image);
    auto screen = get_screen_data(rect_task);
    cv::Mat matchscreen = cv::imdecode(screen, cv::IMREAD_COLOR);
    cv::Mat matchpic = cv::imread(image.c_str(), cv::IMREAD_COLOR);

    //转换到hsv
    cv::Mat hsv_screen;
    cv::Mat hsv_pic;
    cv::cvtColor(matchscreen, hsv_screen, cv::COLOR_BGR2HSV);
    cv::cvtColor(matchpic, hsv_pic, cv::COLOR_BGR2HSV);

    //红色以外过滤掉
    process_pic_red(hsv_screen);
    process_pic_red(hsv_pic);


    cv::Point matchLoc;
    _match_picture(hsv_screen, hsv_pic, matchLoc);

    point.x = matchLoc.x + rect_task.left + matchpic.cols/2;
    point.y = matchLoc.y + rect_task.top - 5;

//    cv::imshow("image_window", hsv_screen);
//    cv::imshow("result_window", hsv_pic);
//    cv::waitKey();

    return true;
}

//POINT 返回匹配到的图片位置
bool GameScript::is_match_pic_in_screen(std::string image, POINT &point, RECT rect, int threshold)
{
    //两个对比的图
    std::vector<uchar>&& screen_buf = get_screen_data(rect);
    check_pic_exists(image);

    cv::Point matchLoc;
    double maxVal = match_picture(screen_buf, image, matchLoc);
    mhprintf("匹配: %s %f", image.c_str(), maxVal);
    if(maxVal > (double)threshold/10)
    {
        cv::Mat img_in = cv::imread(image.c_str());
        point.x = matchLoc.x + img_in.cols/2 + rect.left;
        point.y = matchLoc.y + img_in.rows/2 + rect.top;

        //TODO: 随机一点
        return true;
    }
    else
    {
        return false;
    }
}

//对话框点击
void GameScript::dialog_click(const char* img)
{
    ::SetForegroundWindow(wnd);
    Sleep(200);

    bool yes = false;
    //检测存在
    for(int i = 0; i < 5; i++){
        POINT pt;

        //如果图片不存在, 点击对话框
        if(is_match_pic_in_screen(img, pt)){
            click(img);
            yes = true;
            break;
        }
        else{
            //点击对话框, 直到遇到匹配的图片
            if(is_match_pic_in_screen("pic\\关闭3.png", pt)){
                rand_move_mouse();
                click(pt.x - 100, pt.y + 100);
            }
        }
    }

    if(yes == false){
        char buf[200];
        sprintf(buf, "确保图片存在 %s", img);
        throw std::runtime_error(buf);
    }
}

void GameScript::rand_move_mouse()
{
    //取得当前坐标到目的的差值
    int x = rand()%200+300;
    int y = rand()%200+200;

    int v = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, v);

    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(500);
}


//TODO: 需要一个超时
void GameScript::until_stop_run(int counts)
{
    //来一张
    std::vector<uchar>&& pos1 = get_screen_data();
    
    while(1)
    {
        //key_press(VK_F9);
        Sleep(counts);
        std::vector<uchar>&& pos2 = get_screen_data();

        cv::Point maxLoc;
        double isMatch = match_picture(pos1, pos2, maxLoc);
        if(isMatch > 0.8)
        {
            break;
        }
        
        pos1 = pos2;
        mhprintf("等待界面静止");
    }

}

//x -> x2
std::vector<int> GameScript::get_mouse_vec(int x, int y, int x2, int y2)
{
    std::vector<int> mouse_vec;

    if(x2 >= SCREEN_WIDTH || y2 >= SCREEN_HEIGHT)
    {
        char buf[200];
        sprintf(buf, "目的坐标异常 x: %d, y: %d", x2, y2);
        throw exception_xy(buf);
    }


    if(x < x2)
    {
        for(int i = x+1; i < x2; i++)
        {
            int v = make_mouse_value(i, y);
            mouse_vec.push_back(v);
        }
    }
    else if(x > x2)
    {
        for(int i = x-1; i > x2; i--)
        {
            int v = make_mouse_value(i, y);
            mouse_vec.push_back(v);
        }
    }



    if(y < y2)
    {
        for(int i = y+1; i < y2; i++)
        {
            int v = make_mouse_value(x2, i);
            mouse_vec.push_back(v);
        }
    }
    else if(y > y2)
    {

        for(int i = y-1; i > y2; i--)
        {
            int v = make_mouse_value(x2, i);
            mouse_vec.push_back(v);
        }
    }

    return mouse_vec;
}

void GameScript::click_nofix(int x, int y)
{
    int v = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, v);
    ::PostMessage(wnd, WM_LBUTTONDOWN, 1,v);
    ::PostMessage(wnd, WM_LBUTTONUP, 0,v);
    Sleep(100);
    rand_move_mouse();
}

//匹配屏幕获取当前鼠标位置
POINT GameScript::get_cur_mouse()
{

letstart:
    Sleep(rand()%300 + 400);
    std::vector<uchar>&& screen_buf = get_screen_data();
    cv::Point maxLoc;


    double val = match_picture(screen_buf, "pic\\chk\\mouse2.png", maxLoc);
    maxLoc.x -= 17;
    maxLoc.y -= 17;
    
    if(val < 0.9)
    {
        val = match_picture(screen_buf, "pic\\chk\\mouse4.png", maxLoc);
        maxLoc.x -= 30;
    }


    if(val < 0.9)
    {
        rand_move_mouse();
        mhprintf("没有检测到鼠标, 重新检测");
        goto letstart;
    }

    return {maxLoc.x, maxLoc.y};
}


void GameScript::click_move(int x, int y, int lbutton)
{

    if(x > SCREEN_WIDTH || y > SCREEN_HEIGHT)
        throw std::runtime_error("click_move() 目的坐标异常");

    //转换成游戏内鼠标坐标
    int mouse_x = x * ratio_x;
    int mouse_y = y * ratio_y;

    //加上那个误差
    mouse_x -= rx;
    mouse_y -= ry;

    std::vector<int> mouse = get_mouse_vec(cur_game_x, cur_game_y, mouse_x, mouse_y);
    for(size_t i = 0; i < mouse.size(); i++)
    {
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, mouse[i]);
        Sleep(7);

        if(i == mouse.size() - 1)
        {
            Sleep(250);  //停稳了...
            if(lbutton == 1)
                ::PostMessage(wnd, WM_LBUTTONDOWN, 1,mouse[i]);
            else if(lbutton == 0)
                ::PostMessage(wnd, WM_RBUTTONDOWN, 1, mouse[i]);

            Sleep(10);

            if(lbutton == 1)
                ::PostMessage(wnd, WM_LBUTTONUP, 0,mouse[i]);
            else if(lbutton == 0)
                ::PostMessage(wnd, WM_RBUTTONUP, 0, mouse[i]);

            //等待界面响应
            Sleep(200);
        }
    }


    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(200);
}

void GameScript::click_nomove(int x, int y)
{
    int xy = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, xy);
    Sleep(1000);
    ::PostMessage(wnd, WM_LBUTTONDOWN, 1, xy);
    ::PostMessage(wnd, WM_LBUTTONUP, 0, xy);
    Sleep(100);
}

void GameScript::key_press(std::string key)
{
    top_wnd();
    if(key.find('+') == std::string::npos){
        key_press(toupper(key[0]));
    }
    else{

        std::string f = key.substr(0, key.find('+'));
        std::string a = key.substr(key.find('+') + 1, key.length());
        if(f == "SHIFT")
            mhprintf("no shift..");
        else if(f == "ALT"){
            keybd_event(VK_MENU, 0, 0, 0);
            Sleep(100);
            keybd_event(toupper(a[0]),0, 0, 0);
            keybd_event(toupper(a[0]),0, KEYEVENTF_KEYUP, 0);
            Sleep(100);
            keybd_event(VK_MENU,0,KEYEVENTF_KEYUP,0);
            Sleep(100);
        }
    }

}

void GameScript::key_press(int vk)
{
    top_wnd();
    keybd_event(vk, 0, 0, 0);
    keybd_event(vk, 0, KEYEVENTF_KEYUP, 0);
    Sleep(500);
}


//传进来的是窗口坐标
//转化成游戏内坐标, 之后用WM_MOUSEMOVE移动
void GameScript::click(int x, int y, int lbutton)
{


    POINT now;
    POINT now_game;

    //获得当前鼠标位置
    now = get_cur_mouse();
    now_game = now;
    std::vector<int> r = get_mouse_vec(now.x, now.y, x > 550 ? x-70:x, y > 450 ? y - 70: y);
    for(size_t i = 0; i < r.size(); i++)
    {
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, r[i]);
        Sleep(2);

        now_game.x = LOWORD(r[i]);
        now_game.y = HIWORD(r[i]);
    }

    //就一下
    now = get_cur_mouse();

    //逐渐移动
    //转换成游戏内座标
    int game_x = now.x * ratio_x;
    int game_y = now.y * ratio_y;

    //取得误差
    rx = game_x - now_game.x;
    ry = game_y - now_game.y;
    cur_game_x = now_game.x;
    cur_game_y = now_game.y;

    click_move(x, y, lbutton);
}

void GameScript::input(const std::string & msg)
{

    for(size_t j = 0; j < msg.size(); j++){
        ::PostMessage(wnd, WM_CHAR, msg[j], 1);
        Sleep(10);
    }

}


void GameScript::set_player_name(std::string name)
{
    player_name = name;
}



//返回屏幕内存数据
std::vector<uchar> GameScript::get_screen_data(const RECT &rcClient)
{

    HDC hdcWindow = nullptr;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;

    try
    {

        // Retrieve the handle to a display device context for the client
        // area of the window.
        hdcWindow = GetDC(wnd);

        // Create a compatible DC which is used in a BitBlt from the window DC
        hdcMemDC = CreateCompatibleDC(hdcWindow);

        if(!hdcMemDC){
            throw std::runtime_error("CreateCompatibleDC has failed");
        }

        // Create a compatible bitmap from the Window DC
        hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);

        if(!hbmScreen){
            throw std::runtime_error("CreateCompatibleBitmap Failed");
        }

        // Select the compatible bitmap into the compatible memory DC.
        SelectObject(hdcMemDC,hbmScreen);

        // Bit block transfer into our compatible memory DC.
        if(!BitBlt(hdcMemDC,
                   0, 0,
                   rcClient.right-rcClient.left, rcClient.bottom-rcClient.top,
                   hdcWindow,
                   rcClient.left, rcClient.top,
                   SRCCOPY))
        {
            throw std::runtime_error("BitBlt has failed");
        }

        // Get the BITMAP from the HBITMAP
        GetObject(hbmScreen,sizeof(BITMAP),&bmpScreen);

        BITMAPFILEHEADER   bmfHeader;
        BITMAPINFOHEADER   bi;

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

        DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

        // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
        // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
        // have greater overhead than HeapAlloc.
        HANDLE hDIB = GlobalAlloc(GHND,dwBmpSize);
        char *lpbitmap = (char *)GlobalLock(hDIB);

        // Gets the "bits" from the bitmap and copies them into a buffer
        // which is pointed to by lpbitmap.
        GetDIBits(hdcWindow, hbmScreen, 0,
                  (UINT)bmpScreen.bmHeight,
                  lpbitmap,
                  (BITMAPINFO *)&bi, DIB_RGB_COLORS);



        // Add the size of the headers to the size of the bitmap to get the total file size
        DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        //Offset to where the actual bitmap bits start.
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

        //Size of the file
        bmfHeader.bfSize = dwSizeofDIB;

        //bfType must always be BM for Bitmaps
        bmfHeader.bfType = 0x4D42; //BM


        imgbuf.resize(bmfHeader.bfSize);

        //已经写入大小
        int readed = 0;
        memcpy(&imgbuf[0], &bmfHeader, sizeof(BITMAPFILEHEADER));
        readed += sizeof(BITMAPFILEHEADER);

        memcpy(&imgbuf[readed], &bi, sizeof(BITMAPINFOHEADER));
        readed += sizeof(BITMAPINFOHEADER);

        memcpy(&imgbuf[readed], lpbitmap, dwBmpSize);


        //Unlock and Free the DIB from the heap
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);

        //Clean up
    }catch(...){

    }

    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(wnd,hdcWindow);


    return imgbuf;
}
