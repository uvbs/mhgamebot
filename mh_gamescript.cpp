#include "mh_gamescript.h"

#define FIRSTTASK(x)\
    if(mouse.is_match_pic_in_point("pic\\"#x, pt, point_task, false))
#define FINDTASK(x) \
    else if(mouse.is_match_pic_in_point("pic\\"#x, pt, point_task, true))
#define ENDTASK \
    else { MH_printf("什么任务.."); }

GameScriper::GameScriper(HWND game_wnd)
    :mouse(game_wnd)
{
    wnd = game_wnd;
}


bool GameScriper::is_in_city(const char* city)
{


    POINT pt;
    if(mouse.is_match_pic_in_screen(city, pt))
    {
        MH_printf("当前城市 %s..", city);
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
        //先他妈关对话框...
        mouse.click("pic\\关闭.png");
        mouse.click("pic\\取消.png");

        POINT pt;
        if(mouse.is_match_pic_in_screen("pic\\捕捉.png", pt))
        {
            mouse.click(pt.x, pt.y);
            mouse.click("pic\\捕捉-鼠标.png");
        }


        if(!mouse.is_match_pic_in_screen("pic\\ingame.png", pt))
        {
            //选中了攻击?
            MH_printf("取消攻击");
            mouse.rclick();
        }

        FIRSTTASK(task000001.png)
        {
            mouse.click(pt.x, pt.y);
        }
        FINDTASK(task000002.png)
        {
            mouse.click(pt.x, pt.y);
        }
        FINDTASK(task000003.png)
        {
            mouse.click(rect_tools.x, rect_tools.y);

            Sleep(1000);
            POINT close_pt;
            if(mouse.is_match_pic_in_screen("pic\\close.png", close_pt))
            {
                mouse.click(close_pt.x, close_pt.y);
            }
        }
        FINDTASK(task000004.png)
        {
            mouse.click(pt.x, pt.y);
        }
        FINDTASK(task000005.png)
        {
            //曲柳仗
            mouse.click(rect_tools.x, rect_tools.y);

            //Sleep(1000);
            mouse.rclick("pic\\曲柳仗.png");
        }
        FINDTASK(task000006.png)
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
                mouse.click(269,253);
                mouse.click("pic\\动手吧-战斗.png");
            }


        }
        FINDTASK(孙厨娘.png)
        {
            mouse.click(pt.x, pt.y);
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
            mouse.click("pic\\玄大夫治病.png");
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
            POINT cur = mouse.Get_cur_mouse();
            mouse.click(cur.x, cur.y);
        }
        ENDTASK


        if(Get_player_status() == Player_status::COMBAT)
        {
            MH_printf("战斗状态");
            POINT pt;
            if(mouse.is_match_pic_in_screen("pic\\捕捉.png", pt))
            {
                mouse.click(pt.x, pt.y);
            }

            mouse.click("pic\\自动战斗.png");
            continue;
        }

        //mouse.Until_stop_run();
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
        else if(mouse.is_match_pic_in_screen("pic\\ingame.png"))
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
