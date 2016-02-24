#include "gamescript.h"
#include <regex>
#include <thread>
#include <boost/lexical_cast.hpp>


#define MHCHATWND "梦幻西游2 聊天窗口"

#define  script_inst  GameScript::get_instance(L)

#define FIRSTTASK(x)\
    if(mouse.is_match_pic_in_screen("pic\\"#x, pt))

#define FINDTASK(x) \
    else if(mouse.is_match_pic_in_screen("pic\\"#x, pt))

#define ENDTASK \
    else { mhprintf(LOG_NORMAL,"什么任务.."); mouse.rand_move_mouse(); }


int GetBytesPerPixel(int depth)
{
    return (depth==32 ? 4 : 3);
}
int GetBytesPerRow(int width, int depth)
{
    int bytesPerPixel = GetBytesPerPixel(depth);
    int bytesPerRow = ((width * bytesPerPixel + 3) & ~3);
    return bytesPerRow;
}

int GetBitmapBytes(int width, int height, int depth)
{
    return height * GetBytesPerRow(width, depth);
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
    lua_close(lua_status);
    ReleaseDC(wnd,hdc);
}

void GameScript::mhprintf(LOG_TYPE logtype,const char* msg, ...)
{
    va_list va_args;
    va_start(va_args, msg);
    _mhprintf(player_name.c_str(), msg, va_args, logtype);
    va_end(va_args);
}


bool GameScript::is_in_city(std::string city)
{
    city.insert(0, "map\\");

    POINT pt;
    if(is_match_pic_in_screen(city, pt))
    {
        mhprintf(LOG_NORMAL,"当前城市 %s..", city.c_str());
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
        mhprintf(LOG_NORMAL,"检测到网络断线...");
        mhprintf(LOG_NORMAL,"尝试重启..");

        ::SendMessageA(wnd, WM_COMMAND, 1, 0);
        return true;
    }


    return false;
}


void GameScript::call_lua_func(std::string func)
{
    if(func.find("()") == std::string::npos)
        func += "()";

    luaL_dostring(lua_status, func.c_str());
}

void GameScript::match_task()
{
    bool finded = false;
    for(auto name :lua_task_list)
    {
        //backup
        std::string taskname(name);
        check_pic_exists(taskname);

        if(is_match_pic_in_screen(taskname.c_str()))
        {
            mhprintf(LOG_NORMAL,"开始任务->%s", taskname.c_str());
            call_lua_func(name.c_str());
            finded = true;
            break;
        }
    }

    if(finded == false){
        call_lua_func(lua_task_generic_fun.c_str());
        mhprintf(LOG_DEBUG, "调用通用任务处理");
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
                if(name.find("通用_") == std::string::npos){
                    lua_task_list.push_back(name);
                }
                else{
                    lua_task_generic_fun = name;
                }
            }
        }


        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(lua_status, 1);
    }

    mhprintf(LOG_DEBUG,"检测到脚本任务数量: %d", lua_task_list.size());
}

void GameScript::load_lua_file(const char* name)
{
    //加载任务脚本
    if(0 != luaL_dofile(lua_status, name))
    {
        const char * err = lua_tostring(lua_status, -1);
        mhprintf(LOG_NORMAL,err);
        lua_pop(lua_status, 1);
        throw std::runtime_error("加载lua脚本失败");
    }

    mhprintf(LOG_NORMAL,"加载脚本: %s 完成", name);
}

//关掉一些东西
void GameScript::close_game_wnd_stuff()
{
    //关对话框...
    if(is_match_pic_in_screen("pic\\关闭.png", rect_game, 8))
        click("pic\\关闭.png");

    if(is_match_pic_in_screen("pic\\关闭2.png", rect_game, 8))
        click("pic\\关闭2.png");

    if(is_match_pic_in_screen("pic\\关闭1.png", rect_game, 8))
        click("pic\\关闭1.png");

    if(is_match_pic_in_screen("pic\\取消.png", rect_game, 8))
        click("pic\\取消.png");
}

void GameScript::do_task()
{
    can_task = true;

    //之前遍历一次
    read_global(true);
    load_lua_file("任务.lua");
    read_global(false);

    load_lua_file("任务-战斗.lua");

    while(can_task)
    {

        try
        {

            if(check_offline()) break;
            close_game_wnd_stuff();

            PLAYER_STATUS status = get_player_status();
            if(status == PLAYER_STATUS::COMBAT)
            {
                //自动战斗中
                if(is_match_pic_in_screen("pic\\自动战斗中"))
                {

                }
                else
                {
                    //有菜单出现再进行操作
                    if(is_match_pic_in_screen("pic\\战斗-菜单2"))
                    {
                        call_lua_func("战斗回调");
                        bool processed = lua_toboolean(lua_status, -1);
                        if(processed == false){
                            //无事可做, 自动战斗吧
                            click("pic\\自动战斗.png");
                        }
                    }
                }


            }
            else if(status == PLAYER_STATUS::NORMAL){
                match_task();
            }
            else if(status == PLAYER_STATUS::GC){
                click("跳过动画");
            }
        }
        catch(exception_xy &e){
            mhprintf(LOG_DEBUG,"%s, 重新遍历任务", e.what());
        }
        catch(exception_status &e){
            mhprintf(LOG_DEBUG,"%s, 重新尝试一次", e.what());
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
            //mhprintf(LOG_NORMAL,"%s", title);


            //用正则匹配出来
            std::regex regex(R"(.*-\s?(.+)\[\d+\].*)");
            std::cmatch cmatch;
            if(std::regex_match(title, cmatch, regex))
            {
                player_name = cmatch[1];
                set_player_name(cmatch[1]);

                //debug
                //mhprintf(LOG_NORMAL,"%s", cmatch[1]);
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
            mhprintf(LOG_WARNING,"未知场景..");
        }

        Sleep(500);
    }

}


//获取玩家状态
PLAYER_STATUS GameScript::get_player_status()
{
    if(is_match_pic_in_screen("pic\\战斗中.png"))
    {
        mhprintf(LOG_DEBUG,"战斗状态");
        return PLAYER_STATUS::COMBAT;
    }
    else if(is_match_pic_in_screen("pic\\游戏内.png"))
    {
        mhprintf(LOG_DEBUG,"平常状态");
        return PLAYER_STATUS::NORMAL;
    }
    else if(is_match_pic_in_screen("pic\\体验状态.png"))
    {
        mhprintf(LOG_DEBUG,"体验状态");
        return PLAYER_STATUS::NOTIME;
    }
    else if(is_match_pic_in_screen("pic\\跳过动画.png"))
    {
        mhprintf(LOG_DEBUG,"动画状态");
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
    Sleep(100);
}

//过滤掉红色, 白色以外的颜色 (用来坐任务匹配)
void GameScript::process_pic(cv::Mat &src, cv::Mat &result)
{
    cv::inRange(src, cv::Scalar(0, 0, 180, 0), cv::Scalar(179, 255, 255, 0), result);
}

void GameScript::process_pic_red(cv::Mat &src)
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

    REGLUAFUN(lua_status, "调试信息", [](lua_State* L)->int{
        std::string info = lua_tostring(L, 1);
        script_inst->mhprintf(LOG_NORMAL,info.c_str());
        return 0;
    });


    REGLUAFUN(lua_status, "点击", [](lua_State* L)->int{
        assert(lua_gettop(L) == 2);
        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);

        //实验测试这样移动会完成给予的点击操作
        script_inst->slow_click(x, y, x, y+3, 1);
        //script_inst->slow_click(x, y+20, x, y, 1);

        //移开, 避免挡住
        script_inst->click_move(100, 100, 1);
        return 0;
    });

    REGLUAFUN(lua_status, "关闭无关窗口", [](lua_State* L)->int{
        script_inst->close_game_wnd_stuff();
        return 0;
    });

    REGLUAFUN(lua_status, "移动鼠标到", [](lua_State* L)->int{
        assert(lua_gettop(L) <= 2);

        std::string name = lua_tostring(L, 1);
        script_inst->check_pic_exists(name);

        POINT pt;
        int thershold = DEFAULT_THERSHOLD;
        if(lua_gettop(L) == 2)
            thershold = lua_tointeger(L, 2);

        if(false == script_inst->is_match_pic_in_screen(name.c_str(), pt, rect_game, thershold)){
            throw std::runtime_error("函数使用错误, 确保能匹配到");
        }

        script_inst->click(pt.x, pt.y, 3);   //只移动

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

        ;
        cv::Mat matchscreen = cv::imdecode(
                    script_inst->get_screen_data(rect_task),
                    cv::IMREAD_COLOR);
        cv::Mat matchpic = cv::imread(img.c_str(), cv::IMREAD_COLOR);

        //转换到hsv
        cv::Mat hsv_screen;
        cv::Mat hsv_pic;
        cv::cvtColor(matchscreen, hsv_screen, cv::COLOR_BGR2HSV);
        cv::cvtColor(matchpic, hsv_pic, cv::COLOR_BGR2HSV);

        //找任务的红字部分

        cv::Mat screen_result;
        cv::Mat pic_result;

        script_inst->process_pic(hsv_screen, screen_result);
        script_inst->process_pic(hsv_pic, pic_result);

        cv::Point matchLoc;
        double maxVal = script_inst->_match_picture(screen_result, pic_result, matchLoc);

        script_inst->mhprintf(LOG_NORMAL,"任务: %s, 匹配结果 %f", img.c_str(), maxVal);

        if(maxVal >= ((double)DEFAULT_THERSHOLD / (double)10))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);

        return 1;
    });

    REGLUAFUN(lua_status, "获得图片位置", [](lua_State* L)->int{
        std::string pic = lua_tostring(L, 1);
        POINT pt;
        if(script_inst->is_match_pic_in_screen(pic, pt))
        {
            lua_pushinteger(L, pt.x);
            lua_pushinteger(L, pt.y);
        }
        else{
            std::string err = pic;
            err += " 不存在";
            throw std::runtime_error(err);
        }

        return 2;
    });

    REGLUAFUN(lua_status, "结束脚本", [](lua_State* L)->int{
        std::string reason = lua_tostring(L, 1);
        script_inst->mhprintf(LOG_NORMAL,reason.c_str());
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
        }//TODO
        else{
            script_inst->mhprintf(LOG_NORMAL,"参数错误 找唱哥");
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
        int counts = lua_gettop(L);
        if(counts != 2)
            throw std::runtime_error("点击坐标 参数数量错误");

        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);
        script_inst->rand_move_mouse();
        script_inst->click(x, y);
        script_inst->until_stop_run(1500);
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
        POINT pt;

        //先检测三个长度的下划线
        if(script_inst->find_color("可点击下划线1", pt)){
            script_inst->click(pt.x, pt.y);
        }
        else if(script_inst->find_color("可点击下划线", pt)){
            script_inst->click(pt.x, pt.y);
        }
        else{
            throw std::runtime_error("可点击下划线没有匹配到");
        }

        return 0;
    });

    REGLUAFUN(lua_status, "隐藏玩家", [](lua_State* L)->int{
        script_inst->key_press(VK_F9);
        return 0;
    });

    REGLUAFUN(lua_status, "变动点击", [](lua_State* L)->int{
        std::string pic = lua_tostring(L, 1);

        auto a1 = script_inst->get_screen_data();
        script_inst->click(pic.c_str());
        while(true){

            Sleep(500);
            auto a2 = script_inst->get_screen_data();
            cv::Point pt;
            double result = script_inst->match_picture(a1, a2, pt);
            if(result < 0.7){
                //变动了
                break;
            }

            a1 = a2;
        }

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
                script_inst->mhprintf(LOG_WARNING,"这个图片不存在 %s", imgname.c_str());
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
                script_inst->mhprintf(LOG_WARNING,"这个图片不存在 %s", imgname.c_str());
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

        for(int i = 0; i < 3; i++){
            if(script_inst->is_match_pic_in_screen("pic\\子女按钮", zinv) == false){
                script_inst->key_press("ALT+E");
            }
            else{
                break;
            }
        }


        Sleep(500);
        RECT bag_rect;
        bag_rect.left = zinv.x + 15;
        bag_rect.top = zinv.y + 15;
        bag_rect.bottom = SCREEN_HEIGHT;
        bag_rect.right = SCREEN_WIDTH;

        std::string imgname = lua_tostring(L, 1);
        imgname.insert(0, "物品\\");
        POINT pt;
        if(script_inst->is_match_pic_in_screen(imgname, pt, bag_rect) == false)
            script_inst->mhprintf(LOG_DEBUG,"装备的物品不存在, 这不是一个严重错误");
        else
            script_inst->rclick(pt.x, pt.y);

        return 0;
    });

    REGLUAFUN(lua_status, "点击小地图", [](lua_State* L)->int{
        script_inst->mhprintf(LOG_DEBUG, "点击小地图");

        //非递归互斥, 注意死锁问题
        int arg_counts = lua_gettop(L);
        assert(arg_counts == 1 || arg_counts == 3);

        std::string name = lua_tostring(L, 1);
        name.insert(0, "小地图\\");

        //打开小地图
        script_inst->key_press(VK_TAB);
        while(true){ //等待出现
            if(script_inst->is_match_pic_in_screen("世界按钮")) break;
            Sleep(500);
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
    mhprintf(LOG_NORMAL,"线程ID:%d, 脚本执行", std::this_thread::get_id());

    regist_lua_fun();
    load_lua_file("lualib\\gamefun.lua");


    mhprintf(LOG_NORMAL,"开始进入游戏");
    entry_game();
    mhprintf(LOG_NORMAL,"进去游戏[完成]");


    mhprintf(LOG_NORMAL,"执行脚本: %d", config->type);

    if(config->type == SCRIPT_TYPE::SMART){

    }
    else if(config->type == SCRIPT_TYPE::MONEY){
        do_money();
    }
    else if(config->type == SCRIPT_TYPE::LEVEL){
        do_task();
    }
    else if(config->type == SCRIPT_TYPE::DAILY){
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


    while(can_task)
    {
        try
        {
            PLAYER_STATUS status = get_player_status();
            if(status == NORMAL)
                call_lua_func("日常1");
            else if(status == COMBAT){

            }
        }
        catch(exception_xy &e){
            mhprintf(LOG_DEBUG,"%s, 重新遍历任务", e.what());
        }
        catch(exception_status &e){
            mhprintf(LOG_DEBUG,"%s, 重新尝试一次", e.what());
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

void GameScript::input_password(const char* input)
{

    for(size_t i = 0; i < strlen(input); i++){

        if(input[i] == '@')
        {
            keybd_event(16, 0, 0 ,0);
            Sleep(100);

            keybd_event(0x32,0,0,0);     //按下a键
            Sleep(10);
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


        Sleep(500);
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

void GameScript::click(const char* image, int threshold)
{
    POINT point;
    if(is_match_pic_in_screen(image, point, rect_game, threshold)){
        mhprintf(LOG_NORMAL,"点击图片 %s", image);
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
    cv::matchTemplate(screen, pic, result, match_method);
    cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }
    else{
        matchLoc = maxLoc;
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


//任务区域
bool GameScript::find_color(std::string image, POINT &point)
{
    check_pic_exists(image);

    cv::Mat matchscreen = cv::imdecode(get_screen_data(rect_task), cv::IMREAD_COLOR);
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
    if(threshold <= 3 || threshold >=10)
        throw std::runtime_error("匹配程度设置错误, 大于3小于10为妙");

    //两个对比的图
    check_pic_exists(image);

    cv::Point matchLoc;
    double maxVal = match_picture(get_screen_data(rect), image, matchLoc);
    //mhprintf(LOG_NORMAL,"匹配: %s %f", image.c_str(), maxVal);
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
    top_wnd();

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
            if(is_match_pic_in_screen("pic\\关闭.png", pt)){
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
    int x = rand()%40+640/2;
    int y = rand()%40+480/2;

    int v = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, v);

    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(500);
}


//TODO: 需要一个超时
void GameScript::until_stop_run(int counts)
{
    //来一张
    get_screen_data();
    auto pos1 = _screen_data;

    while(1)
    {
        //key_press(VK_F9);
        Sleep(counts);
        get_screen_data();
        auto pos2 = _screen_data;

        cv::Point maxLoc;
        double isMatch = match_picture(pos1, pos2, maxLoc);
        if(isMatch > 0.8)
        {
            break;
        }

        pos1 = pos2;
        mhprintf(LOG_NORMAL,"等待界面静止");

        //检测战斗状态
        if(get_player_status() == COMBAT){
            break;
        }
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


    if(x < x2)
    {
        for(int i = x+1; i < x2; i++)
        {
            int v = make_mouse_value(i, y);
            r.push_back(v);
        }
    }
    else if(x > x2)
    {
        for(int i = x-1; i > x2; i--)
        {
            int v = make_mouse_value(i, y);
            r.push_back(v);
        }
    }



    if(y < y2)
    {
        for(int i = y+1; i < y2; i++)
        {
            int v = make_mouse_value(x2, i);
            r.push_back(v);
        }
    }
    else if(y > y2)
    {

        for(int i = y-1; i > y2; i--)
        {
            int v = make_mouse_value(x2, i);
            r.push_back(v);
        }
    }
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

    cv::Point maxLoc;
    double val = match_picture(get_screen_data(), "pic\\chk\\mouse2.png", maxLoc);
    maxLoc.x -= 17;
    maxLoc.y -= 17;

    if(val < 0.85)
    {
        val = match_picture(screen_data(), "pic\\chk\\mouse4.png", maxLoc);
        maxLoc.x -= 30;
    }


    if(val < 0.85)
    {
        rand_move_mouse();
        mhprintf(LOG_DEBUG,"没有检测到鼠标, 重新检测");
        goto letstart;
    }

    return {maxLoc.x, maxLoc.y};
}

void GameScript::slow_click(int x, int y, int x1, int y1, int lbutton)
{
    std::vector<int> mouse;
    get_mouse_vec(x, y, x1, y1, mouse);
    for(size_t i = 0; i < mouse.size(); i++)
    {
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, mouse[i]);
        Sleep(8);

        if(i == mouse.size() - 1)
        {
            Sleep(250);  //停稳了...
            if(lbutton == 1)
                ::PostMessage(wnd, WM_LBUTTONDOWN, 1,mouse[i]);
            else if(lbutton == 0)
                ::PostMessage(wnd, WM_RBUTTONDOWN, 1, mouse[i]);

            if(lbutton == 1)
                ::PostMessage(wnd, WM_LBUTTONUP, 0,mouse[i]);
            else if(lbutton == 0)
                ::PostMessage(wnd, WM_RBUTTONUP, 0, mouse[i]);

            //等待界面响应
            Sleep(200);
        }
    }
}

void GameScript::click_move(int x, int y, int lbutton)
{

    if(x > SCREEN_WIDTH || y > SCREEN_HEIGHT)
        throw std::runtime_error("click_move() 目的坐标异常");

    //转换成游戏内鼠标坐标
    int mouse_x = static_cast<int>((double)x * ratio_x);
    int mouse_y = static_cast<int>((double)y * ratio_y);

    //加上那个误差
    mouse_x -= rx;
    mouse_y -= ry;

    slow_click(cur_game_x, cur_game_y, mouse_x, mouse_y, lbutton);
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
    if(key == "TAB"){
        key_press(VK_TAB);
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
    std::lock_guard<std::mutex> locker(GameScript::topwnd_mutex);
    top_wnd();
    keybd_event(vk, 0, 0, 0);
    keybd_event(vk, 0, KEYEVENTF_KEYUP, 0);
    Sleep(200);
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
    std::vector<int> r;
    get_mouse_vec(now.x, now.y, x > 550 ? x-70:x, y > 450 ? y - 70: y, r);
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
    int game_x = static_cast<int>((double)now.x * ratio_x);
    int game_y = static_cast<int>((double)now.y * ratio_y);

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

const std::vector<uchar>& GameScript::screen_data()
{
    return _screen_data;
}

//返回屏幕内存数据
const std::vector<uchar>& GameScript::get_screen_data(const RECT& rect)
{

    BITMAP bmpScreen;

    try
    {
        HDC mem_dc = ::CreateCompatibleDC(hdc);
        HBITMAP hbm_screen = ::CreateCompatibleBitmap(
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
    catch(std::exception &e){
        mhprintf(LOG_WARNING, e.what());

    }
    catch(...){
        mhprintf(LOG_WARNING, "get_screen_data exception");
    }

    return _screen_data;
}
