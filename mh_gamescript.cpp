#include "mh_gamescript.h"
#include <regex>
#include <thread>
#include <boost/lexical_cast.hpp>

#define MHCHATWND "梦幻西游2 聊天窗口"


#define FIRSTTASK(x)\
    if(mouse.is_match_pic_in_screen("pic\\task\\"#x, pt))

#define FINDTASK(x) \
    else if(mouse.is_match_pic_in_screen("pic\\task\\"#x, pt))

#define ENDTASK \
    else { mhprintf("什么任务.."); mouse.Rand_move_mouse(); }

std::map<lua_State*, GameScriper*> GameScriper::inst_map;
GameScriper::GameScriper(HWND game_wnd, int id):
    mouse(game_wnd, id),
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


bool GameScriper::is_in_city(const char* city)
{


    POINT pt;
    if(mouse.is_match_pic_in_screen(city, pt))
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
    FINDTASK(孙猎户.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(道具.png)
    {
        mouse.click(rect_tools.x, rect_tools.y);
        mouse.rclick("pic\\task\\青铜短剑.png");
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

        if(is_in_city("pic\\map\\建邺城.png")){
           mouse.click(640, 480);
        }
        else if(is_in_city("pic\\map\\东海湾.png")){
            mouse.click(400, 0);
        }
        else if(is_in_city("pic\\map\\桃源村.png")){
            mouse.click(269,253);
            mouse.Dialog_click("pic\\对话框-战斗.png");
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
        mouse.Dialog_click("pic\\对话框-战斗.png");
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
        mouse.Dialog_click("pic\\task\\玄大夫治病.png");
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
        mouse.Dialog_click("pic\\task\\这就上船啦.png");
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

void GameScriper::do_task()
{
    //匹配任务
    //TODO:
    while(1)
    {

        try
        {
            if(check_offline())
                break;

            PLAYER_STATUS status = Get_player_status();
            if(status == PLAYER_STATUS::COMBAT)
            {
                //有菜单出现再进行操作
                if(mouse.is_match_pic_in_screen("pic\\task\\战斗-防御.png"))
                {
                    POINT pt;
                    if(mouse.is_match_pic_in_screen("pic\\task\\捕捉-狸.png", pt))
                    {
                        mouse.click("pic\\捕捉.png");
                        mouse.click(pt.x, pt.y - 50);
                        continue;
                    }

                    //无事才自动战斗
                    mouse.click("pic\\自动战斗.png");
                }

            }
            else if(status == PLAYER_STATUS::NORMAL)
            {
                //关对话框...
                mouse.click("pic\\关闭.png");
                mouse.click("pic\\关闭1.png");
                mouse.click("pic\\关闭2.png");
                mouse.click("pic\\取消.png");

                task1to10();
            }
        }
        catch(exception_xy &e)
        {
            mhprintf("%s, 重新遍历任务", e.what());
        }
        catch(exception_status &e)
        {
            mhprintf("%s, 重新尝试一次", e.what());
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
        else if(mouse.is_match_pic_in_screen("pic\\srvinfo.png"))
        {
            mouse.click_nofix("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\selsrv.png"))
        {
            mouse.click_nofix("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\inputpw.png"))
        {

        }
        else if(mouse.is_match_pic_in_screen("pic\\游戏内.png"))
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
                mouse.set_player_name(cmatch[1]);

                //debug
                //mhprintf("%s", cmatch[1]);
            }

            //进入游戏才出来...
            break;
        }
        else if(mouse.is_match_pic_in_screen("pic\\selplayer.png"))
        {
            mouse.click_nofix("pic\\ok.png");
            mouse.click_nofix("pic\\nextstep.png");
        }
        else
        {
            mhprintf("未知场景..");
        }

        Sleep(200);
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
        throw exception_status("未知的玩家状态");
    }

}

void GameScriper::Regist_lua_fun()
{

    REGLUAFUN(lua_status, "获取玩家状态", [](lua_State *L)->int{
        GameScriper::Get_instance(L)->do_money();
        return 0;
    });

    REGLUAFUN(lua_status, "当前城市", [](lua_State *L)->int{
        const char* city = lua_tostring(L, 1);
        bool isin = GameScriper::Get_instance(L)->is_in_city(city);
        lua_pushboolean(L, isin);
        return 1;
    });

    mouse.Regist_lua_fun(lua_status);
}

void GameScriper::Run()
{
    //运行脚本
    mhprintf("%d脚本执行", std::this_thread::get_id());


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
        throw std::runtime_error("未知脚本类型");
    }

}
