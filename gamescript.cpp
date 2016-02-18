#include "gamescript.h"
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
    for(auto name :lua_func_list)
    {
        //backup
        std::string taskname(name);

        taskname.insert(0, "pic\\");
        taskname += ".png";
    
        if(is_match_pic_in_screen(taskname.c_str()))
        {
            mhprintf("开始任务->%s", taskname.c_str());
            call_lua_func(name);
            break;
        }  
    }

}

//读取任务数组
void GameScript::readLuaArray(lua_State *L)
{
    lua_getglobal(L, "任务");
    int n = luaL_len(L, -1);
    for (int i = 1; i <= n; ++i)
    {
        lua_pushnumber(L, i);  //往栈里面压入i
        lua_gettable(L, -2);  //读取table[i]，table位于-2的位置。
        const char* name = lua_tostring(L, -1);
        lua_pop(L, 1);
        
        
        lua_func_list.push_back(name);
   
    }
    lua_pop(L, 1);
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
    

}

//关掉一些东西
void GameScript::close_game_wnd_stuff()
{
    //关对话框...
    click("pic\\关闭3.png", 8);
    click("pic\\关闭1.png");
    click("pic\\关闭2.png");
    click("pic\\取消.png");

}

void GameScript::do_task()
{
    can_task = true;
    
    load_lua_file("任务.lua");
    load_lua_file("战斗.lua");

    readLuaArray(lua_status);

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
        if(is_match_pic_in_screen("pic\\login.png"))
        {
            click_nofix("pic\\entrygame.png");
        }
        else if(is_match_pic_in_screen("pic\\srvinfo.png"))
        {
            click_nofix("pic\\nextstep.png");
        }
        else if(is_match_pic_in_screen("pic\\selsrv.png"))
        {
            click_nofix("pic\\nextstep.png");
        }
        else if(is_match_pic_in_screen("pic\\inputpw.png"))
        {
            input_password("wanghao11911988@163.com,aa118118");
            click_nofix("pic\\nextstep.png");
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
        else if(is_match_pic_in_screen("pic\\selplayer.png"))
        {
            click_nofix("pic\\ok.png");
            click_nofix("pic\\nextstep.png");
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

void GameScript::regist_lua_fun()
{

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
        name.insert(0, "pic\\");
        name += ".png";

        POINT pt;
        if(arg_counts == 1){
            if(false == script_inst->is_match_pic_in_screen(name.c_str(), pt)){
                throw std::runtime_error("函数使用错误, 确保能匹配到");
            }

            script_inst->click(pt.x, pt.y, 3);   //只移动
        }
        else{
            int thershold = lua_tointeger(L, 2);
            if(false == script_inst->is_match_pic_in_screen(name.c_str(), pt, thershold)){
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
        if(keystr.find('+') == std::string::npos){
            script_inst->key_press(toupper(keystr[0]));
        }
        else{
            std::string f = keystr.substr(0, keystr.find('+'));
            std::string a = keystr.substr(keystr.find('+') + 1, keystr.length());
            if(f == "SHIFT") script_inst->mhprintf("no shift..");
            else if(f == "ALT"){
                keybd_event(VK_MENU, 0, 0, 0);
                Sleep(100);
                keybd_event(toupper(keystr[0]),0, 0, 0);
                keybd_event(toupper(keystr[0]),0, KEYEVENTF_KEYUP, 0);
                Sleep(100);
                keybd_event(VK_MENU,0,KEYEVENTF_KEYUP,0);
                Sleep(100);
            }
        }

        return 0;
    });

    REGLUAFUN(lua_status, "存在任务", [](lua_State* L)->int{

        std::string img = lua_tostring(L, 1);
        img.insert(0, "pic\\");
        img += ".png";

        if(::PathFileExistsA(img.c_str()) == FALSE){
            char buf[200];
            sprintf(buf, "文件 %s 不存在", img.c_str());
            throw std::runtime_error(buf);
        }

        script_inst->mhprintf("匹配图片 %s", img.c_str());

        std::vector<uchar>&& screen_buf = script_inst->get_screen_data(rect_task);
        cv::Mat matchscreen = cv::imdecode(screen_buf, cv::IMREAD_COLOR);
        cv::Mat matchpic = cv::imread(img.c_str(), cv::IMREAD_COLOR);
        cv::Mat result;

        cv::Mat bin_img_screen;
        cv::Mat bin_img_pic;

        double thershold = 0.7;   //太高个别匹配不到

        //转换到hsv
        cv::Mat hsv_screen;
        cv::Mat hsv_pic;
        cv::cvtColor(matchscreen, hsv_screen, cv::COLOR_BGR2HSV);
        cv::cvtColor(matchpic, hsv_pic, cv::COLOR_BGR2HSV);

        //找任务的红字部分
        cv::Mat out_red;
        cv::Mat out_red1;
        cv::Mat result_screen;
        cv::Mat result_pic;
        cv::inRange(hsv_screen, cv::Scalar(0, 255, 0, 0), cv::Scalar(0, 255, 255, 0), out_red);
        cv::inRange(hsv_screen, cv::Scalar(0, 254, 0, 0), cv::Scalar(0, 254, 255, 0), out_red1);
        cv::bitwise_or(out_red, out_red1, result_screen);
        cv::inRange(hsv_screen, cv::Scalar(179, 187, 240, 0), cv::Scalar(179, 187, 240, 0), out_red1);
        cv::bitwise_or(result_screen, out_red1, result_screen);


        //找匹配图的红字
        cv::inRange(hsv_pic, cv::Scalar(0, 255, 0, 0), cv::Scalar(0, 255, 255, 0), out_red);
        cv::inRange(hsv_pic, cv::Scalar(0, 254, 0, 0), cv::Scalar(0, 254, 255, 0), out_red1);

        cv::bitwise_or(out_red, out_red1, result_pic);
        cv::inRange(hsv_pic, cv::Scalar(179, 187, 240, 0), cv::Scalar(179, 187, 240, 0), out_red1);
        cv::bitwise_or(result_pic, out_red1, result_pic);


//        int nThreshold = 0;
//        cv::namedWindow("ori", CV_WINDOW_NORMAL);
//        cv::createTrackbar("", "ori", &nThreshold, 254, on_trackbar1, &result_screen);

//        cv::namedWindow("test", CV_WINDOW_NORMAL);
//        cv::createTrackbar("", "test", &nThreshold, 254, on_trackbar2, &result_pic);

//        cv::imshow("test", result_screen);
//        cv::waitKey(0);
//        cv::imshow("test", result_pic);
//        cv::waitKey(0);


        int match_method = cv::TM_CCOEFF_NORMED;
        matchTemplate(result_screen, result_pic, result, match_method);

        // 通过函数 minMaxLoc 定位最匹配的位置
        double minVal;
        double maxVal;
        cv::Point minLoc;
        cv::Point maxLoc;
        cv::Point matchLoc;

        cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
        if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
            matchLoc = minLoc;
        }
        else{
            matchLoc = maxLoc;
        }

        script_inst->mhprintf("匹配结果 %f", maxVal);


        if(maxVal > thershold)
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);

        return 1;
    });

    REGLUAFUN(lua_status, "走到旁边", [](lua_State* L)->int{
        std::string img = lua_tostring(L, 1);
        img.insert(0, "pic\\");
        img += ".png";

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

        std::string pos = lua_tostring(L, 1);
        std::string img = lua_tostring(L, 2);
        double threshold = 0.7;
        if(arg_counts == 3)
            threshold = lua_tointeger(L, 3)/10;

        img.insert(0, "pic\\");
        img += ".png";

        while(1)
        {


            if(pos == "左上"){
                script_inst->click(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 50);
            }
            else if(pos == "左下"){
                script_inst->click(100, SCREEN_HEIGHT - 100);
            }
            else if(pos == "右下"){
                script_inst->click(SCREEN_WIDTH - 100, 100);
            }
            else if(pos == "右下"){
                script_inst->click(SCREEN_WIDTH - 100, SCREEN_HEIGHT - 100);
            }
            else if(pos == "上"){
                script_inst->click(SCREEN_WIDTH / 2, 100);
            }
            else if(pos == "下"){
                script_inst->click(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 100);
            }
            else if(pos == "左"){
                script_inst->click(100, SCREEN_HEIGHT / 2);
            }
            else{
                throw std::runtime_error("你这是什么方位...");
            }

            script_inst->until_stop_run();
            Sleep(200);
            script_inst->mhprintf("一次");

            //退出条件
            if(script_inst->is_match_pic_in_screen(img.c_str(), threshold) == true){
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
        imgname += ".png";
        imgname.insert(0, "pic\\");
        
        script_inst->click("战斗-法术");
        script_inst->click("战斗-法术1");
        script_inst->click(imgname.c_str());
        return 0;
    });
    
    REGLUAFUN(lua_status, "玩家宠物", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\");

        script_inst->click(point_pet.x, point_pet.y);
        script_inst->click(imgname.c_str());

        return 0;
    });

    REGLUAFUN(lua_status, "存在图片", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\");
        bool bexist;
        
        if(lua_gettop(L) == 2){
            int threshold = lua_tointeger(L, 2);
            bexist = script_inst->is_match_pic_in_screen(imgname.c_str(), threshold);
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
        imgname += ".png";
        imgname.insert(0, "pic\\");
        script_inst->rclick(imgname.c_str());
        return 0;
    });

    REGLUAFUN(lua_status, "点击", [](lua_State* L)->int{
        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);

        script_inst->click_move(x, y, 1);
        return 0;
    });

    REGLUAFUN(lua_status, "点击坐标", [](lua_State* L)->int{
        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);
        script_inst->rand_move_mouse();
        script_inst->click(x, y);
        script_inst->until_stop_run();
        return 0;
    });

    REGLUAFUN(lua_status, "获取玩家状态", [](lua_State *L)->int{
        script_inst->do_money();
        return 0;
    });

    REGLUAFUN(lua_status, "当前城市", [](lua_State *L)->int{
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\map\\");
        bool isin = script_inst->is_in_city(imgname.c_str());
        lua_pushboolean(L, isin);
        return 1;
    });

    REGLUAFUN(lua_status, "对话", [](lua_State* L)->int{
        script_inst->click(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
        return 0;
    })

    REGLUAFUN(lua_status, "点击对话框内图片", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\");
        script_inst->dialog_click(imgname.c_str());
        return 0;
    });

    REGLUAFUN(lua_status, "点击任务", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\");
        POINT pt;
        if(script_inst->is_match_pic_in_rect(imgname.c_str(), pt, rect_task)){
            script_inst->click(pt.x, pt.y);
        }
        else{
            char buf[100];
            sprintf(buf, "图片 %s 没有匹配到", imgname.c_str());    
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
        imgname += ".png";
        imgname.insert(0, "pic\\");


        if(arg_counts == 1){
            script_inst->click(imgname.c_str());
        }
        else if(arg_counts == 2){
            int threshold = lua_tointeger(L, 2);
            script_inst->click(imgname.c_str(), threshold);
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
        
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\");
        
        if(!script_inst->is_match_pic_in_screen("pic\\道具行囊.png", 5))
            script_inst->click(rect_tools.x, rect_tools.y);
        
        script_inst->rclick(imgname.c_str());
        return 0;
    });

    REGLUAFUN(lua_status, "点击小地图", [](lua_State* L)->int{
        int arg_counts = lua_gettop(L);
        if(arg_counts != 1 && arg_counts != 3)
            throw std::runtime_error("参数数量错误");

        std::string name = lua_tostring(L, 1);
        name += ".png";
        name.insert(0, "pic\\");

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


    mhprintf("执行脚本: %s", Script_type_desc[config.type].str.c_str());

    if(config.type == Script_type::SMART){

    }
    else if(config.type == Script_type::MONEY){
        do_money();
    }
    else if(config.type == Script_type::LEVEL){
        do_task();
    }
    else if(config.type == Script_type::DAILY){
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
            keybd_event(0x32,0,KEYEVENTF_KEYUP,0);//松开a键
            Sleep(100);
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
    if(is_match_pic_in_screen(image, point, threshold)){
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


bool GameScript::is_match_pic_in_screen(const char *image, int threshold)
{
    POINT pt;
    return is_match_pic_in_screen(image, pt, threshold);
}


double GameScript::match_picture(const std::vector<uchar> &img1, const char* img2, cv::Point &maxLoc)
{

    std::string img2_str(img2);

    //bmp
    if(img2_str.find(".png") == std::string::npos){
        img2_str += ".png";
    }


    if(::PathFileExistsA(img2_str.c_str()) == FALSE)
    {
        char buf[100];
        sprintf(buf, "图片 %s 不存在", img2_str.c_str());
        throw std::runtime_error(buf);
    }


    if(wnd == nullptr || ::IsWindow(wnd) == FALSE)
        throw std::runtime_error("无效的句柄");

    cv::Mat matchscreen = cv::imdecode(img1, cv::IMREAD_GRAYSCALE);
    cv::Mat matchpic = cv::imread(img2_str.c_str(), cv::IMREAD_GRAYSCALE);
    cv::Mat result;

    //mhprintf("%matchpic %d %d", matchpic.size().height, matchpic.size().width);
    //mhprintf("%matchscreen %d %d", matchscreen.size().height, matchscreen.size().width);

    //匹配方式
    int match_method = cv::TM_CCOEFF_NORMED;
    cv::matchTemplate(matchscreen, matchpic, result, match_method);
    //normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// 通过函数 minMaxLoc 定位最匹配的位置
    double minVal;
    double maxVal;
    cv::Point minLoc;
    cv::Point matchLoc;

    cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );

    /// 对于方法 SQDIFF 和 SQDIFF_NORMED, 越小的数值代表更高的匹配结果. 而对于其他方法, 数值越大匹配越好
    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }
    else{
        matchLoc = maxLoc;
    }


//    cv::rectangle( matchpic, matchLoc, Point( matchLoc.x + img_in.cols , matchLoc.y + img_in.rows ), Scalar::all(0), 2, 8, 0 );
//    cv::rectangle( result, matchLoc, Point( matchLoc.x + img_in.cols , matchLoc.y + img_in.rows ), Scalar::all(0), 2, 8, 0 );

//    cv::imshow("image_window", mat_pic);
//    cv::imshow( result_window, result );

//    int nThreshold = 0;
//    cv::namedWindow("ori", CV_WINDOW_NORMAL);
//    cv::createTrackbar("11", "ori", &nThreshold, 254, on_trackbar1, &matchscreen);

//    cv::namedWindow("test", CV_WINDOW_NORMAL);
//    cv::createTrackbar("11", "test", &nThreshold, 254, on_trackbar2, &matchpic);

//    cv::waitKey(0);

    return maxVal;
}

double GameScript::match_picture(const std::vector<uchar>& img1, const std::vector<uchar>& img2, cv::Point &maxLoc)
{

    if(wnd == nullptr)
        throw std::runtime_error("无效的句柄");


    cv::Mat result;

    //匹配方式
    int match_method = cv::TM_CCOEFF_NORMED;
    cv::Mat matscreen = cv::imdecode(img1, cv::IMREAD_COLOR);
    cv::Mat matchpic = cv::imdecode(img2, cv::IMREAD_COLOR);

    matchTemplate(matscreen, matchpic, result, match_method);
    //normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// 通过函数 minMaxLoc 定位最匹配的位置
    double minVal;
    double maxVal;
    cv::Point minLoc;
    cv::Point matchLoc;

    cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );

    /// 对于方法 SQDIFF 和 SQDIFF_NORMED, 越小的数值代表更高的匹配结果. 而对于其他方法, 数值越大匹配越好
    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }
    else{
        matchLoc = maxLoc;
    }

    return maxVal;
}


bool GameScript::is_match_pic_in_rect(const char *image, const RECT &rect, int threshold)
{
    POINT pt;
    return is_match_pic_in_rect(image, pt, rect, threshold);
}

//这种匹配部分主要用来避免一些误差
//
bool GameScript::is_match_pic_in_rect(const char *image, POINT &point, const RECT &rect, int threshold)
{
    //取得屏幕图片
    std::vector<uchar>&& screen_buf = get_screen_data(rect);


    cv::Point maxLoc;
    double maxVal = match_picture(screen_buf, image, maxLoc);

    if(maxVal > (double)threshold/10)
    {
        cv::Mat img_in = cv::imread(image);
        point.x = maxLoc.x + img_in.cols/2 + rect.left;
        point.y = maxLoc.y + img_in.rows/2 + rect.top;
        return true;
    }
    else
    {
        return false;
    }
}

//POINT 返回匹配到的图片位置
bool GameScript::is_match_pic_in_screen(const char *image, POINT &point, int threshold)
{
    //两个对比的图
    std::vector<uchar>&& screen_buf = get_screen_data();

    cv::Point maxLoc;
    double maxVal = match_picture(screen_buf, image, maxLoc);
    //mhprintf("匹配: %s %f", image, maxVal);
    if(maxVal > (double)threshold/10)
    {
        cv::Mat img_in = cv::imread(image);
        point.x = maxLoc.x + img_in.cols/2;
        point.y = maxLoc.y + img_in.rows/2;

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
    Sleep(500);
    click(img);
}

void GameScript::rand_move_mouse()
{
    //取得当前坐标到目的的差值
    int x = rand()%200+100;
    int y = rand()%200+100;

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

    if(x2 > SCREEN_WIDTH || y2 > SCREEN_HEIGHT)
    {
        mhprintf("x: %d, y: %d", x2, y2);
        throw exception_xy("目的坐标异常");
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
        Sleep(1);

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
