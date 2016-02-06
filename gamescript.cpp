#include "gamescript.h"
#include <regex>
#include <thread>
#include <boost/lexical_cast.hpp>
#include <codecvt>



#define MHCHATWND "梦幻西游2 聊天窗口"


#define FIRSTTASK(x)\
    if(mouse.is_match_pic_in_screen("pic\\"#x, pt))

#define FINDTASK(x) \
    else if(mouse.is_match_pic_in_screen("pic\\"#x, pt))

#define ENDTASK \
    else { mhprintf("什么任务.."); mouse.Rand_move_mouse(); }



//静态初始化
std::map<lua_State*, GameScriper*> GameScriper::inst_map;

//构造函数
GameScriper::GameScriper(HWND game_wnd, int id):
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
        GameScriper::get_instance(L)->mhprintf(err_str);
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

void GameScriper::mhprintf(const char* msg, ...)
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


bool GameScriper::is_in_city(const char* city)
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
bool GameScriper::check_offline()
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

void GameScriper::test_lua(const char *err)
{
    mhprintf(err);
}

void GameScriper::call_lua_func(const char* name)
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

//读取任务数组
void GameScriper::readLuaArray(lua_State *L)
{
    lua_getglobal(L, "任务");
    int n = luaL_len(L, -1);
    for (int i = 1; i <= n; ++i)
    {
        lua_pushnumber(L, i);  //往栈里面压入i
        lua_gettable(L, -2);  //读取table[i]，table位于-2的位置。
        const char* name = lua_tostring(L, -1);
        lua_pop(L, 1);
        //backup
        std::string taskname(name);
        std::string backup_taskname(name);

        taskname.insert(0, "pic\\");
        taskname += ".png";

        if(is_match_pic_in_screen(taskname.c_str()))
        {
            mhprintf("开始任务->%s", taskname.c_str());
            call_lua_func(backup_taskname.c_str());
            break;
        }
    }
    lua_pop(L, 1);
}

void GameScriper::load_lua_file(const char* name)
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
void GameScriper::close_game_wnd_stuff()
{
    //关对话框...
    click("pic\\关闭.png");
    click("pic\\关闭1.png");
    click("pic\\关闭2.png");
    click("pic\\取消.png");

}

void GameScriper::do_task()
{

    load_lua_file("任务.lua");
    load_lua_file("战斗.lua");

    while(true)
    {

        try
        {

            if(check_offline()) break;
            close_game_wnd_stuff();

            PLAYER_STATUS status = get_player_status();
            if(status == PLAYER_STATUS::COMBAT)
            {
                //有菜单出现再进行操作
                if(is_match_pic_in_screen("pic\\战斗-菜单.png"))
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
                readLuaArray(lua_status);
            }
        }
        catch(exception_xy &e){
            mhprintf("%s, 重新遍历任务", e.what());
        }
        catch(exception_status &e){
            mhprintf("%s, 重新尝试一次", e.what());
        }
    }
}


//当前玩家的状态
//登录界面
//载入界面
void GameScriper::entry_game()
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
PLAYER_STATUS GameScriper::get_player_status()
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

void GameScriper::regist_lua_fun()
{

    REGLUAFUN(lua_status, "玩家宠物", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\");

        GameScriper::get_instance(L)->click(point_pet.x, point_pet.y);
        GameScriper::get_instance(L)->click(imgname.c_str());

        return 0;
    });

    REGLUAFUN(lua_status, "存在图片", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\");
        bool bexist = GameScriper::get_instance(L)->is_match_pic_in_screen(imgname.c_str());
        lua_pushboolean(L, bexist);
        return 1;
    });

    REGLUAFUN(lua_status, "使用辅助技能", [](lua_State* L)->int{
        const std::string name = lua_tostring(L, 1);
        if(name == "烹饪"){
            GameScriper::get_instance(L)->click(point_player.x, point_player.y);
            GameScriper::get_instance(L)->click("pic\\辅助技能.png");
            GameScriper::get_instance(L)->click("pic\\烹饪.png");
        }
        else{
            GameScriper::get_instance(L)->mhprintf("参数错误 找唱哥");
        }

        return 0;
    });


    REGLUAFUN(lua_status, "加血", [](lua_State* L)->int{
        GameScriper::get_instance(L)->rclick(point_player_healher.x, point_player_healher.y);
        return 0;
    });

    REGLUAFUN(lua_status, "右击", [](lua_State* L)->int{
        const char *name = lua_tostring(L, 1);
        GameScriper::get_instance(L)->rclick(name);
    });

    REGLUAFUN(lua_status, "点击座标", [](lua_State* L)->int{
        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);
        GameScriper::get_instance(L)->click(x, y);
    });

    REGLUAFUN(lua_status, "获取玩家状态", [](lua_State *L)->int{
        GameScriper::get_instance(L)->do_money();
        return 0;
    });

    REGLUAFUN(lua_status, "当前城市", [](lua_State *L)->int{
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\map\\");
        bool isin = GameScriper::get_instance(L)->is_in_city(imgname.c_str());
        lua_pushboolean(L, isin);
        return 1;
    });

    REGLUAFUN(lua_status, "对话", [](lua_State* L)->int{
        GameScriper::get_instance(L)->click(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    })

    REGLUAFUN(lua_status, "点击对话框内图片", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\");
        GameScriper::get_instance(L)->dialog_click(imgname.c_str());
        return 0;
    });

    REGLUAFUN(lua_status, "点击图片", [](lua_State* L)->int{
        std::string imgname = lua_tostring(L, 1);
        imgname += ".png";
        imgname.insert(0, "pic\\");
        GameScriper::get_instance(L)->click(imgname.c_str());
        GameScriper::get_instance(L)->until_stop_run();
        return 0;
    });

    REGLUAFUN(lua_status, "等待停止奔跑", [](lua_State* L)->int{
        GameScriper::get_instance(L)->until_stop_run();
        return 0;
    });

    REGLUAFUN(lua_status, "装备物品", [](lua_State* L)->int{
        try{
            std::string imgname = lua_tostring(L, 1);
            imgname += ".png";
            imgname.insert(0, "pic\\");

            if(!GameScriper::get_instance(L)->is_match_pic_in_screen("pic\\道具行囊.png"))
                GameScriper::get_instance(L)->click(rect_tools.x, rect_tools.y);

            GameScriper::get_instance(L)->rclick(imgname.c_str());
        }catch(std::runtime_error &e){
            GameScriper::get_instance(L)->mhprintf(e.what());
        }

       return 0;
    });

    REGLUAFUN(lua_status, "点击小地图", [](lua_State* L)->int{
        std::string name = lua_tostring(L, 1);
        name += ".png";
        name.insert(0, "pic\\");
        GameScriper::get_instance(L)->click(point_map.x, point_map.y);
        GameScriper::get_instance(L)->click(name.c_str());
        GameScriper::get_instance(L)->close_game_wnd_stuff();
        GameScriper::get_instance(L)->until_stop_run();
        return 0;
    });
}

void GameScriper::run()
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
    else{
        throw std::runtime_error("未知脚本类型");
    }

}


int GameScriper::make_mouse_value(int x, int y)
{
    int v = 0;
    v = x;
    y = y << 16;
    v = v + y;

    return v;
}

std::vector<uchar> GameScriper::get_screen_data()
{
    RECT rect = {};
    ::GetClientRect(wnd, &rect);

    //mhprintf("GetClientRect: %d, %d, %d, %d", client.left, client.top, client.right, client.bottom);

    return get_screen_data(rect);
}

void GameScriper::input_event(const char* input)
{
    for(size_t i = 0; i < strlen(input); i++)
    {

        keybd_event(input[i],0,0,0);     //按下a键
        keybd_event(input[i],0,KEYEVENTF_KEYUP,0);//松开a键

    }

}


//也用来取消攻击状态
void GameScriper::rclick(int x, int y)
{
    //TODO:
    click(x, y, false);
}



void GameScriper::rclick(const char* image)
{
    POINT point;
    if(is_match_pic_in_screen(image, point))
    {
        rclick(point.x, point.y);
    }
    else
    {
        char buf[50];
        sprintf(buf, "rclick error! %s!", image);
        //mhprintf(buf);
    }
}

void GameScriper::click(const char* image)
{
    POINT point;
    if(is_match_pic_in_screen(image, point))
    {
        click(point.x, point.y);
    }
}

void GameScriper::click_nofix(const char* image)
{

    POINT point;
    if(is_match_pic_in_screen(image, point))
    {
        click_nofix(point.x, point.y);
    }

}


bool GameScriper::is_match_pic_in_screen(const char *image)
{
    POINT pt;
    return is_match_pic_in_screen(image, pt);
}

double GameScriper::match_picture(const std::vector<uchar> &img1, const char* img2, cv::Point &maxLoc)
{

    std::string img2_str(img2);
    if(img2_str.find(".png") == std::string::npos){
        img2_str += ".png";
    }

    if(::PathFileExistsA(img2_str.c_str()) == FALSE)
    {
        char buf[100];
        sprintf(buf, "图片 %s 不存在", img2_str.c_str());
        throw std::runtime_error(buf);
    }


    if(wnd == nullptr)
        throw std::runtime_error("无效的句柄");


    cv::Mat matchscreen = cv::imdecode(img1, cv::IMREAD_COLOR);
    cv::Mat matchpic = cv::imread(img2_str.c_str(), cv::IMREAD_COLOR);
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

    return maxVal;
}

double GameScriper::match_picture(const std::vector<uchar>& img1, const std::vector<uchar>& img2, cv::Point &maxLoc)
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


//这种匹配部分主要用来避免一些误差
//
bool GameScriper::is_match_pic_in_rect(const char *image, POINT &point, const RECT &rect)
{
    //取得屏幕图片
    std::vector<uchar>&& screen_buf = get_screen_data(rect);


    cv::Point maxLoc;
    double maxVal = match_picture(screen_buf, image, maxLoc);

    if(maxVal > 0.70)
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
bool GameScriper::is_match_pic_in_screen(const char *image, POINT &point)
{
    //两个对比的图
    std::vector<uchar>&& screen_buf = get_screen_data();

    cv::Point maxLoc;
    double maxVal = match_picture(screen_buf, image, maxLoc);
    //mhprintf("匹配: %s %f", image, maxVal);
    if(maxVal > 0.70)
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
void GameScriper::dialog_click(const char* img)
{
    ::SetForegroundWindow(wnd);
    click(img);
}

void GameScriper::rand_move_mouse()
{
    //取得当前坐标到目的的差值
    int x = rand()%200+100;
    int y = rand()%200+100;

    int v = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, v);

    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(200);
}


//TODO: 需要一个超时
void GameScriper::until_stop_run()
{
    while(1)
    {
        //来一张
        //TODO:
        std::vector<uchar>&& pos1 = get_screen_data(rect_position);
        Sleep(1000);
        std::vector<uchar>&& pos2 = get_screen_data(rect_position);

        cv::Point maxLoc;
        double isMatch = match_picture(pos1, pos2, maxLoc);
        if(isMatch > 0.9)
        {
            break;
        }
    }

}

//x -> x2
std::vector<int> GameScriper::get_mouse_vec(int x, int y, int x2, int y2)
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

void GameScriper::click_nofix(int x, int y)
{
    int v = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, v);
    ::PostMessage(wnd, WM_LBUTTONDOWN, 1,v);
    ::PostMessage(wnd, WM_LBUTTONUP, 0,v);
    Sleep(100);
    rand_move_mouse();
}

//匹配屏幕获取当前鼠标位置
POINT GameScriper::get_cur_mouse()
{

letstart:
    std::vector<uchar>&& screen_buf = get_screen_data();
    cv::Point maxLoc;
    bool first_mouse = true;
    bool third_mouse = false;
    bool second_mouse = false;

    double val = match_picture(screen_buf, "pic\\chk\\mouse1.png", maxLoc);
    if(val < 0.9)
    {
        first_mouse = false;
        val = match_picture(screen_buf, "pic\\chk\\mouse2.png", maxLoc);
    }

    if(val < 0.9)
    {
        second_mouse = true;
        val = match_picture(screen_buf, "pic\\chk\\mouse3.png", maxLoc);
    }

    if(val < 0.9)
    {
        third_mouse = true;
        val = match_picture(screen_buf, "pic\\chk\\mouse4.png", maxLoc);
    }



    if(val < 0.9)
    {
        rand_move_mouse();
        goto letstart;
    }

    //减去截取的鼠标图本身的偏差
    if(first_mouse)
    {
        maxLoc.x -= 0;
        maxLoc.y -= 3;
    }
    else if(first_mouse == false)
    {
        maxLoc.x -= 17;
        maxLoc.y -= 17;
    }
    else if(second_mouse == true)
    {
        maxLoc.x -= 2;
        maxLoc.y -= 3;
    }
    else if(third_mouse == true)
    {
        maxLoc.x -= 30;
    }



    return {maxLoc.x, maxLoc.y};
}


void GameScriper::click_move(int x, int y, bool lbutton)
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
        Sleep(rand()%3 + 3);

        if(i == mouse.size() - 1)
        {
            if(lbutton)
                ::PostMessage(wnd, WM_LBUTTONDOWN, 1,mouse[i]);
            else
                ::PostMessage(wnd, WM_RBUTTONDOWN, 1, mouse[i]);

            Sleep(10);

            if(lbutton)
                ::PostMessage(wnd, WM_LBUTTONUP, 0,mouse[i]);
            else
                ::PostMessage(wnd, WM_RBUTTONUP, 0, mouse[i]);
        }
    }


    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(200);
}


//传进来的是窗口坐标
//转化成游戏内坐标, 之后用WM_MOUSEMOVE移动
void GameScriper::click(int x, int y, bool lbutton)
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
        Sleep(rand()%3 + 1);

        now_game.x = LOWORD(r[i]);
        now_game.y = HIWORD(r[i]);
    }

    Sleep(rand()%100+100);

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
    Sleep(rand()%500+500);
}

void GameScriper::input(const std::string & msg)
{

    for(size_t j = 0; j < msg.size(); j++){
        ::PostMessage(wnd, WM_CHAR, msg[j], 1);
        Sleep(10);
    }

}


void GameScriper::set_player_name(std::string name)
{
    player_name = name;
}



//返回屏幕内存数据
std::vector<uchar> GameScriper::get_screen_data(const RECT &rcDC)
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

        if(!hdcMemDC)
        {
            throw std::runtime_error("CreateCompatibleDC has failed");
        }

        // Get the client area for size calculation
        RECT rcClient;
        GetClientRect(wnd, &rcClient);

        //This is the best stretch mode
        SetStretchBltMode(hdcWindow,HALFTONE);

        //The source DC is the entire screen and the destination DC is the current window (HWND)
        if(!StretchBlt(hdcMemDC,
                       0,0,
                       rcClient.right, rcClient.bottom,
                       hdcWindow,
                       0,0,
                       GetSystemMetrics (SM_CXSCREEN),
                       GetSystemMetrics (SM_CYSCREEN),
                       SRCCOPY))
        {
            throw std::runtime_error("StretchBlt has failed");
        }

        // Create a compatible bitmap from the Window DC
        hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);

        if(!hbmScreen)
        {
            throw std::runtime_error("CreateCompatibleBitmap Failed");
        }

        // Select the compatible bitmap into the compatible memory DC.
        SelectObject(hdcMemDC,hbmScreen);

        // Bit block transfer into our compatible memory DC.
        if(!BitBlt(hdcMemDC,
                   0,0,
                   rcClient.right-rcClient.left, rcClient.bottom-rcClient.top,
                   hdcWindow,
                   0,0,
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
    }catch(...)
    {

    }

    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(wnd,hdcWindow);


    return imgbuf;
}
