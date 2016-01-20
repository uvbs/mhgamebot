#include "mh_gamescript.h"


#define FINDTASK(x) mouse.is_match_pic_in_point("pic\\"#x, pt, point_task)

GameScriper::GameScriper(HWND game_wnd)
    :mouse(game_wnd)
{
    wnd = game_wnd;
}


bool GameScriper::is_in_city(const char* city)
{
    MH_printf("判断城市中..");
    //char buf[30];
    //sprintf(buf, "pic\\map\\%s.png", city);
    POINT pt;
    if(mouse.is_match_pic_in_screen(city, pt))
    {
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

        MH_printf("一次循环");


        MH_printf("x:%d, y:%d", mouse.Get_cur_mouse().x, mouse.Get_cur_mouse().y);
        //先他妈关对话框...
        mouse.click("pic\\close.png");
        mouse.click("pic\\取消.png");


        if(Get_player_status() == Player_status::COMBAT)
        {
            //
            MH_printf("战斗状态");

            mouse.click("pic\\自动战斗.png");
            continue;

        }

        POINT pt;
        if(FINDTASK(task000001.png))
        {
            mouse.click(pt.x, pt.y);
        }
        else if(FINDTASK(task000002.png))
        {
            mouse.click(pt.x, pt.y);
        }
        else if(FINDTASK(task000003.png))
        {
            mouse.click(rect_tools.x, rect_tools.y);

            Sleep(1000);
            POINT close_pt;
            if(mouse.is_match_pic_in_screen("pic\\close.png", close_pt))
            {
                mouse.click(close_pt.x, close_pt.y);
            }
        }
        else if(FINDTASK(task000004.png))
        {
            mouse.click(pt.x, pt.y);
        }
        else if(FINDTASK(task000005.png))
        {
            //曲柳仗
            mouse.click(rect_tools.x, rect_tools.y);

            //Sleep(1000);
            mouse.rclick("pic\\曲柳仗.png");
        }
        else if(FINDTASK(task000006.png))
        {
            mouse.click(pt.x, pt.y);

            //阻塞, 直到停止奔跑
            mouse.Until_stop_run();

            if(is_in_city("pic\\map\\建邺城.png"))
            {
               mouse.click(640, 480);
            }
           else if(is_in_city("pic\\map\\东海湾.png"))
            {
                mouse.click(400, 0);
            }
            else if(is_in_city("pic\\map\\桃源村.png"))
            {
                mouse.click("pic\\动手吧-战斗.png");
            }


        }
        else if(!mouse.is_match_pic_in_screen("pic\\ingame.png", pt))
        {
            //选中了攻击?
            MH_printf("取消攻击");
            mouse.rclick();
        }
        else
        {
            MH_printf("什么任务...");
        }

        Sleep(100);
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
            mouse.click("pic\\entrygame.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\srvinfo.png"))
        {
            MH_printf("当前玩家状态: 服务器信息");
            mouse.click("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\selsrv.png"))
        {
            MH_printf("当前玩家状态: 选择服务器");
            mouse.click("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\inputpw.png"))
        {
            MH_printf("当前玩家状态: 输入密码");
        }
        else if(mouse.is_match_pic_in_screen("pic\\ingame.png"))
        {
            //进入游戏才出来...
            MH_printf("当前玩家状态: 游戏中");
            break;
        }
        else if(mouse.is_match_pic_in_screen("pic\\selplayer.png"))
        {
            MH_printf("当前玩家状态: 选择角色");
            mouse.click("pic\\ok.png");
            mouse.click("pic\\nextstep.png");
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
    if(mouse.is_match_pic_in_screen("pic\\战斗中.png"))
    {
        //进入游戏才出来...
        MH_printf("当前玩家状态: 战斗中");
        return Player_status::COMBAT;
    }
    else if(mouse.is_match_pic_in_screen("pic\\ingame.png"))
    {
        MH_printf("当前玩家状态: 平常");
        return Player_status::NORMAL;
    }

    throw std::runtime_error("什么玩家状态..");
}



void GameScriper::Run()
{
    //运行脚本
    MH_printf("线程ID:%d 脚本开始执行", std::this_thread::get_id());

    //类型
    Script_type type = config.Get_script_type();
    MH_printf("脚本类型: %s", Script_type_desc[type].str.c_str());

    MH_printf("----开始进入游戏流程----");
    Entry_game();
    MH_printf("----进去游戏[完成]----");


    MH_printf("--开始校准指针--");
    mouse.Right_point();
    MH_printf("--校准指针完成--");

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
