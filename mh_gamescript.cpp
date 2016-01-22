#include "mh_gamescript.h"

#define FIRSTTASK(x)\
    if(mouse.is_match_pic_in_point("pic\\"#x, pt, point_task, false))
#define FINDTASK(x) \
    else if(mouse.is_match_pic_in_point("pic\\"#x, pt, point_task, true))
#define ENDTASK \
    else { MH_printf("ʲô����.."); }

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
        MH_printf("��ǰ���� %s..", city);
        return true;
    }

    return false;
}

void GameScriper::do_task()
{
    //ƥ������
    //TODO:
    while(1)
    {
        //������ضԻ���...
        mouse.click("pic\\�ر�.png");
        mouse.click("pic\\ȡ��.png");

        POINT pt;
        if(mouse.is_match_pic_in_screen("pic\\��׽.png", pt))
        {
            mouse.click(pt.x, pt.y);
            mouse.click("pic\\��׽-���.png");
        }


        if(!mouse.is_match_pic_in_screen("pic\\ingame.png", pt))
        {
            //ѡ���˹���?
            MH_printf("ȡ������");
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
            //������
            mouse.click(rect_tools.x, rect_tools.y);

            //Sleep(1000);
            mouse.rclick("pic\\������.png");
        }
        FINDTASK(task000006.png)
        {
            mouse.click(pt.x, pt.y);

            //����, ֱ��ֹͣ����
            mouse.Until_stop_run();

            if(is_in_city("pic\\map\\������.png"))
            {
               mouse.click(640, 480);
            }
            else if(is_in_city("pic\\map\\������.png"))
            {
                mouse.click(400, 0);
            }
            else if(is_in_city("pic\\map\\��Դ��.png"))
            {
                mouse.click(269,253);
                mouse.click("pic\\���ְ�-ս��.png");
            }


        }
        FINDTASK(�����.png)
        {
            mouse.click(pt.x, pt.y);
        }
        FINDTASK(��⿰���.png)
        {
            mouse.click(point_player.x, point_player.y);
            mouse.click("pic\\��������.png");
            mouse.click("pic\\���.png");
        }
        FINDTASK(�����.png)
        {
            mouse.click(pt.x, pt.y);
            mouse.Until_stop_run();
            mouse.click(640/2, 480/2);
        }
        FINDTASK(���ߵ��ҵ���.png)
        {
            mouse.click(pt.x, pt.y);
            mouse.Until_stop_run();
            mouse.click(640/2, 480/2);

            if(mouse.is_match_pic_in_screen("pic\\�Ի���-ս��.png", pt))
            {
                mouse.click(pt.x, pt.y);
            }
        }
        FINDTASK(�ٻ��޲�ս.png)
        {
            mouse.click(point_pet.x, point_pet.y);
            mouse.click("pic\\��ս.png");
        }
        FINDTASK(�����.png)
        {
            mouse.click(pt.x, pt.y);
            mouse.Until_stop_run();
            mouse.click(640/2, 480/2);
            mouse.click("pic\\������β�.png");
        }
        FINDTASK(�껭ʦʲô��.png)
        {
            mouse.click(pt.x, pt.y);
            mouse.Until_stop_run();
            mouse.click(640/2, 480/2);
        }
        FINDTASK(��ͼ��ƽ��.png)
        {
            mouse.click(point_map.x, point_map.y);
            POINT cur = mouse.Get_cur_mouse();
            mouse.click(cur.x, cur.y);
        }
        ENDTASK


        if(Get_player_status() == Player_status::COMBAT)
        {
            MH_printf("ս��״̬");
            POINT pt;
            if(mouse.is_match_pic_in_screen("pic\\��׽.png", pt))
            {
                mouse.click(pt.x, pt.y);
            }

            mouse.click("pic\\�Զ�ս��.png");
            continue;
        }

        //mouse.Until_stop_run();
        Sleep(100);
    }
}


//��ǰ��ҵ�״̬
//��¼����
//�������
void GameScriper::Entry_game()
{
    //��ͼ����

    while(1)
    {
        //��¼����
        if(mouse.is_match_pic_in_screen("pic\\login.png"))
        {
            MH_printf("��ǰ���״̬: ��¼����");
            mouse.click_nofix("pic\\entrygame.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\srvinfo.png"))
        {
            MH_printf("��ǰ���״̬: ��������Ϣ");
            mouse.click_nofix("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\selsrv.png"))
        {
            MH_printf("��ǰ���״̬: ѡ�������");
            mouse.click_nofix("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\inputpw.png"))
        {
            MH_printf("��ǰ���״̬: ��������");

        }
        else if(mouse.is_match_pic_in_screen("pic\\ingame.png"))
        {
            //������Ϸ�ų���...
            MH_printf("��ǰ���״̬: ��Ϸ��");
            break;
        }
        else if(mouse.is_match_pic_in_screen("pic\\selplayer.png"))
        {
            MH_printf("��ǰ���״̬: ѡ���ɫ");
            mouse.click_nofix("pic\\ok.png");
            mouse.click_nofix("pic\\nextstep.png");
        }
        else
        {
            //��ͼ?
            //TODO:
            MH_printf("δ֪����..");
        }

        Sleep(500);
    }

}


//��ȡ���״̬
Player_status GameScriper::Get_player_status()
{
    if(mouse.is_match_pic_in_screen("pic\\ս����.png"))
    {
        //������Ϸ�ų���...
        MH_printf("��ǰ���״̬: ս����");
        return Player_status::COMBAT;
    }
    else if(mouse.is_match_pic_in_screen("pic\\ingame.png"))
    {
        MH_printf("��ǰ���״̬: ƽ��");
        return Player_status::NORMAL;
    }

    throw std::runtime_error("ʲô���״̬..");
}



void GameScriper::Run()
{
    //���нű�
    MH_printf("�߳�ID:%d �ű���ʼִ��", std::this_thread::get_id());


    MH_printf("��ʼ������Ϸ");
    Entry_game();
    MH_printf("��ȥ��Ϸ[���]");


    Script_type type = config.Get_script_type();
    MH_printf("ִ�нű�: %s", Script_type_desc[type].str.c_str());

    if(type == Script_type::SMART){

    }
    else if(type == Script_type::MONEY){
        do_money();
    }
    else if(type == Script_type::LEVEL){
        do_task();
    }
    else{
        std::runtime_error("δ֪�ű�����");
    }

}
