#include "mh_gamescript.h"
#include <regex>

#define FIRSTTASK(x)\
    if(mouse.is_match_pic_in_point("pic\\task\\"#x, pt, point_task, false))

#define FINDTASK(x) \
    else if(mouse.is_match_pic_in_point("pic\\task\\"#x, pt, point_task, true))

#define ENDTASK \
    else { mhprintf("什么任务.."); }


GameScriper* GameScriper::_inst = nullptr;
GameScriper::GameScriper(HWND game_wnd, int id)
    :mouse(game_wnd, id),
      player_name("unknow"),
      player_level("0")
{
    script_id = id;
    wnd = game_wnd;
    _inst = this;
    lua_status = nullptr;
    hdc = ::GetDC(wnd);
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
        Sleep(150);
        goto tryagain;
    }
}


bool GameScriper::is_in_city(const char* city, bool screen_exist)
{


    POINT pt;
    if(mouse.is_match_pic_in_screen(city, pt, screen_exist))
    {
        mhprintf("当前城市 %s..", city);
        return true;
    }

    return false;
}

void GameScriper::task1to10()
{
    POINT pt;
    FIRSTTASK(霞姑娘.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(立即注册.png)
    {

        mouse.input("sdfawerqasda");
    }
    FINDTASK(刘大婶.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
//    FINDTASK(穿上布衣.png)
//    {
//        mouse.click(rect_tools.x, rect_tools.y);
//        mouse.rclick("pic\\task\\布衣.png");
//    }
//    FINDTASK(穿上布裙.png)
//    {
//        mouse.click(rect_tools.x, rect_tools.y);
//        mouse.rclick("pic\\task\\布裙.png");
//    }
    FINDTASK(孙猎户.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(道具.png)
    {
        mouse.click(rect_tools.x, rect_tools.y);
        mouse.rclick("pic\\task\\折扇.png");
        mouse.rclick("pic\\task\\布衣.png");
        mouse.rclick("pic\\task\\布裙.png");
        mouse.rclick("pic\\task\\双短剑.png");
        mouse.rclick("pic\\task\\牛皮鞭.png");
        mouse.rclick("pic\\task\\曲柳仗.png"); //男角色
        mouse.rclick("pic\\task\\黄铜圈.png");  //女角色
    }
    FINDTASK(干掉野猪.png)
    {
        mouse.click(pt.x, pt.y);

        //阻塞, 直到停止奔跑
        mouse.Until_stop_run();

        if(is_in_city("pic\\map\\建邺城.png", false)){
           mouse.click(640, 480);
        }
        else if(is_in_city("pic\\map\\东海湾.png", true)){
            mouse.click(400, 0);
        }
        else if(is_in_city("pic\\map\\桃源村.png", true)){
            mouse.click(269,253);
            mouse.click("pic\\对话框-战斗.png");
        }

    }
    FINDTASK(孙厨娘.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(红色任务血条.png)
    {
        mouse.rclick(point_player_healher.x, point_player_healher.y);
    }

    FINDTASK(烹饪包子.png)
    {
        mouse.click(point_player.x, point_player.y);
        mouse.click("pic\\辅助技能.png");
        mouse.click("pic\\烹饪.png");
    }
    FINDTASK(郭大哥.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(赶走捣乱的狸.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);

        if(mouse.is_match_pic_in_screen("pic\\对话框-战斗.png", pt))
        {
            mouse.click(pt.x, pt.y);
        }
    }
    FINDTASK(召唤兽参战.png)
    {
        mouse.click(point_pet.x, point_pet.y);
        mouse.click("pic\\参战.png");
    }
    FINDTASK(玄大夫.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
        mouse.click("pic\\task\\玄大夫治病.png");
    }
    FINDTASK(雨画师什么事.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(地图找平儿.png)
    {
        mouse.click(point_map.x, point_map.y);
        mouse.click("pic\\task\\去找萍儿.png");
        mouse.Until_stop_run();
        mouse.click("pic\\关闭.png");
        mouse.click("pic\\关闭1.png");
        mouse.click("pic\\关闭2.png");
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(这就上船.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
        mouse.click("pic\\task\\这就上船啦.png");
    }
    FINDTASK(告别夏大叔.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        if(mouse.is_match_pic_in_screen("pic\\task\\夏大叔-新手主线.png", pt))
        {
            mouse.click(pt.x, pt.y);
        }
        else if(mouse.is_match_pic_in_screen("pic\\task\\注册和绑定.png", pt))
        {
            mouse.click(pt.x, pt.y);
        }
        else{
            mouse.Until_stop_run();
            mouse.click(640/2, 480/2);
        }
    }
    ENDTASK

}

void GameScriper::do_task()
{
    //匹配任务
    //TODO:
    while(1)
    {

        //::SetActiveWindow(wnd);
        ::SetForegroundWindow(wnd);

        PLAYER_STATUS status = Get_player_status();
        if(status == PLAYER_STATUS::COMBAT)
        {

            POINT pt;
            if(mouse.is_match_pic_in_screen("pic\\task\\捕捉-狸.png", pt))
            {
                mouse.click("pic\\捕捉.png");
                mouse.click(pt.x, pt.y - 40);
            }

            if(mouse.is_match_pic_in_screen("pic\\task\\点击捕捉.png", pt))
            {
                mouse.click("pic\\捕捉.png");
                mouse.click(pt.x, pt.y - 40);
            }

            //无事才自动战斗
            mouse.click("pic\\自动战斗.png");

        }
        else if(status == PLAYER_STATUS::NORMAL)
        {
            task1to10();

            //关对话框...
            mouse.click("pic\\关闭.png");
            mouse.click("pic\\关闭1.png");
            mouse.click("pic\\关闭2.png");
            mouse.click("pic\\取消.png");
        }



    }
}


//当前玩家的状态
//登录界面
//载入界面
void GameScriper::Entry_game()
{
    //找图检测吧

    while(1)
    {
        //登录界面
        if(mouse.is_match_pic_in_screen("pic\\login.png"))
        {
            mouse.click_nofix("pic\\entrygame.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\srvinfo.png", true))
        {
            mouse.click_nofix("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\selsrv.png", true))
        {
            mouse.click_nofix("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\inputpw.png", true))
        {

        }
        else if(mouse.is_match_pic_in_screen("pic\\游戏内.png", true))
        {
            //获取玩家姓名
            char title[256];
            ::GetWindowTextA(wnd, title, 256);
            mhprintf("%s", title);

            //用正则匹配出来
            std::regex regex("\-\\s*(.*)[");
            std::cmatch cmatch;
            if(std::regex_match(title, cmatch, regex))
            {
                player_name = cmatch.str(2);
                mouse.set_player_name(cmatch.str(2));
            }

            //进入游戏才出来...
            break;
        }
        else if(mouse.is_match_pic_in_screen("pic\\selplayer.png", true))
        {
            mouse.click_nofix("pic\\ok.png");
            mouse.click_nofix("pic\\nextstep.png");
        }
        else
        {
            mhprintf("未知场景..");
        }

        Sleep(500);
    }

}


//获取玩家状态
PLAYER_STATUS GameScriper::Get_player_status()
{
    if(mouse.is_match_pic_in_screen("pic\\战斗中.png"))
    {
        mhprintf("战斗状态");
        return PLAYER_STATUS::COMBAT;
    }
    else if(mouse.is_match_pic_in_screen("pic\\游戏内.png"))
    {
        mhprintf("平常状态");
        return PLAYER_STATUS::NORMAL;
    }
    else if(mouse.is_match_pic_in_screen("pic\\体验状态.png"))
    {
        mhprintf("体验状态");
        return PLAYER_STATUS::NOTIME;
    }
    else if(mouse.is_match_pic_in_screen("pic\\跳过动画.png"))
    {
        mhprintf("动画状态");
        return PLAYER_STATUS::GC;
    }
    else
    {
        std::runtime_error("未知的玩家状态");
    }

}

void GameScriper::Regist_lua_fun()
{
    //lua
    mhprintf("加载lua");
    if(lua_status == nullptr)
        lua_status = luaL_newstate();

    if(lua_status == nullptr) std::runtime_error("创建lua状态失败");



    REGLUAFUN(lua_status, "获取玩家状态", [](lua_State *L)->int{
        GameScriper::GetInstance()->do_money();
        return 0;
    });

    REGLUAFUN(lua_status, "当前城市", [](lua_State *L)->int{
        const char* city = lua_tostring(L, 1);
        bool isin = GameScriper::GetInstance()->is_in_city(city);
        lua_pushboolean(L, isin);
        return 1;
    });

    mouse.Regist_lua_fun(lua_status);


    mhprintf("加载lua完成");
}

void GameScriper::Run()
{
    //运行脚本
    mhprintf("脚本开始执行, id:%d", std::this_thread::get_id());


    Regist_lua_fun();


    mhprintf("开始进入游戏");
    Entry_game();
    mhprintf("进去游戏[完成]");


    Script_type type = config.Get_script_type();
    mhprintf("执行脚本: %s", Script_type_desc[type].str.c_str());

    if(type == Script_type::SMART){

    }
    else if(type == Script_type::MONEY){
        do_money();
    }
    else if(type == Script_type::LEVEL){
        do_task();
    }
    else{
        std::runtime_error("未知脚本类型");
    }

}
