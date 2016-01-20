#include "mh_gamescript.h"


#define FINDTASK(x) mouse.is_match_pic_in_point("pic\\"#x, pt, point_task)

GameScriper::GameScriper(HWND game_wnd)
    :mouse(game_wnd)
{
    wnd = game_wnd;
}


bool GameScriper::is_in_city(const char* city)
{
    MH_printf("�жϳ�����..");
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
    //ƥ������
    //TODO:
    while(1)
    {

        MH_printf("һ��ѭ��");


        MH_printf("x:%d, y:%d", mouse.Get_cur_mouse().x, mouse.Get_cur_mouse().y);
        //������ضԻ���...
        mouse.click("pic\\close.png");
        mouse.click("pic\\ȡ��.png");


        if(Get_player_status() == Player_status::COMBAT)
        {
            //
            MH_printf("ս��״̬");

            mouse.click("pic\\�Զ�ս��.png");
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
            //������
            mouse.click(rect_tools.x, rect_tools.y);

            //Sleep(1000);
            mouse.rclick("pic\\������.png");
        }
        else if(FINDTASK(task000006.png))
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
                mouse.click("pic\\���ְ�-ս��.png");
            }


        }
        else if(!mouse.is_match_pic_in_screen("pic\\ingame.png", pt))
        {
            //ѡ���˹���?
            MH_printf("ȡ������");
            mouse.rclick();
        }
        else
        {
            MH_printf("ʲô����...");
        }

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
            mouse.click("pic\\entrygame.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\srvinfo.png"))
        {
            MH_printf("��ǰ���״̬: ��������Ϣ");
            mouse.click("pic\\nextstep.png");
        }
        else if(mouse.is_match_pic_in_screen("pic\\selsrv.png"))
        {
            MH_printf("��ǰ���״̬: ѡ�������");
            mouse.click("pic\\nextstep.png");
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
            mouse.click("pic\\ok.png");
            mouse.click("pic\\nextstep.png");
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

    //����
    Script_type type = config.Get_script_type();
    MH_printf("�ű�����: %s", Script_type_desc[type].str.c_str());

    MH_printf("----��ʼ������Ϸ����----");
    Entry_game();
    MH_printf("----��ȥ��Ϸ[���]----");


    MH_printf("--��ʼУ׼ָ��--");
    mouse.Right_point();
    MH_printf("--У׼ָ�����--");

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
