#include "mh_gamescript.h"


#define FIRSTTASK(x)\
    if(mouse.is_match_pic_in_point("pic\\task\\"#x, pt, point_task, false))

#define FINDTASK(x) \
    else if(mouse.is_match_pic_in_point("pic\\task\\"#x, pt, point_task, true))

#define ENDTASK \
    else { MH_printf("什么任务.."); }


GameScriper* GameScriper::_inst = nullptr;
GameScriper::GameScriper(HWND game_wnd, int id)
    :mouse(game_wnd, id)
{
    script_id = id;
    wnd = game_wnd;
    _inst = this;
    lua_status = nullptr;
    hdc = ::GetDC(wnd);
}


bool GameScriper::is_in_city(const char* city, bool screen_exist)
{


    POINT pt;
    if(mouse.is_match_pic_in_screen(city, pt, screen_exist))
    {
        MH_printf("当前城市 %s..", city);
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
    FINDTASK(穿上布衣.png)
    {
        mouse.click(rect_tools.x, rect_tools.y);
        mouse.rclick("pic\\task\\布衣.png");
    }
    FINDTASK(穿上布裙.png)
    {
        mouse.click(rect_tools.x, rect_tools.y);
        mouse.rclick("pic\\task\\布裙.png");
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

        Player_status status = Get_player_status();
        if(status == Player_status::COMBAT)
        {

            POINT pt;
            if(mouse.is_match_pic_in_screen("pic\\task\\捕捉-狸.png", pt))
            {
                mouse.click("pic\\捕捉.png");
                mouse.click(pt.x, pt.y - 40);
            }

            //无事才自动战斗
            mouse.click("pic\\自动战斗.png");

        }
        else if(status == Player_status::NORMAL)
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
            MH_printf("当前玩家状态: 登录界面");
            mouse.click_nofix("pic\\entrygame.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\srvinfo.png"))
        {
            MH_printf("当前玩家状态: 服务器信息");
            mouse.click_nofix("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\selsrv.png"))
        {
            MH_printf("当前玩家状态: 选择服务器");
            mouse.click_nofix("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\inputpw.png"))
        {
            MH_printf("当前玩家状态: 输入密码");

        }
        else if(mouse.is_match_pic_in_screen("pic\\游戏内.png"))
        {
            //进入游戏才出来...
            MH_printf("当前玩家状态: 游戏中");
            break;
        }
        else if(mouse.is_match_pic_in_screen("pic\\selplayer.png"))
        {
            MH_printf("当前玩家状态: 选择角色");
            mouse.click_nofix("pic\\ok.png");
            mouse.click_nofix("pic\\nextstep.png");
        }
        else
        {
            //过图?
            //TODO:
            MH_printf("未知场景..");
        }

        Sleep(500);
    }

}


//获取玩家状态
Player_status GameScriper::Get_player_status()
{
    POINT pt;
    if(mouse.is_match_pic_in_screen("pic\\战斗中.png"), false)
    {
        MH_printf("战斗状态");
        return Player_status::COMBAT;
    }
    else if(mouse.is_match_pic_in_screen("pic\\游戏内.png"), false)
    {
        MH_printf("平常状态");
        return Player_status::NORMAL;
    }
    else if(mouse.is_match_pic_in_screen("pic\\体验状态.png", pt, false))
    {
        MH_printf("体验状态");
        return Player_status::NOTIME;
    }
    else if(mouse.is_match_pic_in_screen("pic\\跳过动画.png", pt, false))
    {
        MH_printf("动画状态");
        return Player_status::GC;
    }
    else
    {
        std::runtime_error("未知的玩家状态");
    }

}

void GameScriper::Regist_lua_fun()
{
    //lua
    MH_printf("加载lua");
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


    MH_printf("加载lua完成");
}

void GameScriper::Run()
{
    //运行脚本
    MH_printf("脚本开始执行, id:%d", std::this_thread::get_id());


    Regist_lua_fun();


    MH_printf("开始进入游戏");
    Entry_game();
    MH_printf("进去游戏[完成]");


    Script_type type = config.Get_script_type();
    MH_printf("执行脚本: %s", Script_type_desc[type].str.c_str());

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
