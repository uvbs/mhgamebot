#include "mh_gamescript.h"


#define FIRSTTASK(x)\
    if(mouse.is_match_pic_in_point("pic\\task\\"#x, pt, point_task, false))

#define FINDTASK(x) \
    else if(mouse.is_match_pic_in_point("pic\\task\\"#x, pt, point_task, true))

#define ENDTASK \
    else { MH_printf("ʲô����.."); }


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
        MH_printf("��ǰ���� %s..", city);
        return true;
    }

    return false;
}

void GameScriper::task1to10()
{
    POINT pt;
    FIRSTTASK(ϼ����.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(����ע��.png)
    {

        mouse.input("sdfawerqasda");
    }
    FINDTASK(������.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(���ϲ���.png)
    {
        mouse.click(rect_tools.x, rect_tools.y);
        mouse.rclick("pic\\task\\����.png");
    }
    FINDTASK(���ϲ�ȹ.png)
    {
        mouse.click(rect_tools.x, rect_tools.y);
        mouse.rclick("pic\\task\\��ȹ.png");
    }
    FINDTASK(���Ի�.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(����.png)
    {
        mouse.click(rect_tools.x, rect_tools.y);
        mouse.rclick("pic\\task\\����.png");
        mouse.rclick("pic\\task\\����.png");
        mouse.rclick("pic\\task\\��ȹ.png");
        mouse.rclick("pic\\task\\˫�̽�.png");
        mouse.rclick("pic\\task\\ţƤ��.png");
        mouse.rclick("pic\\task\\������.png"); //�н�ɫ
        mouse.rclick("pic\\task\\��ͭȦ.png");  //Ů��ɫ
    }
    FINDTASK(�ɵ�Ұ��.png)
    {
        mouse.click(pt.x, pt.y);

        //����, ֱ��ֹͣ����
        mouse.Until_stop_run();

        if(is_in_city("pic\\map\\������.png", false)){
           mouse.click(640, 480);
        }
        else if(is_in_city("pic\\map\\������.png", true)){
            mouse.click(400, 0);
        }
        else if(is_in_city("pic\\map\\��Դ��.png", true)){
            mouse.click(269,253);
            mouse.click("pic\\�Ի���-ս��.png");
        }

    }
    FINDTASK(�����.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(��ɫ����Ѫ��.png)
    {
        mouse.rclick(point_player_healher.x, point_player_healher.y);
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
        mouse.click("pic\\task\\������β�.png");
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
        mouse.click("pic\\task\\ȥ��Ƽ��.png");
        mouse.Until_stop_run();
        mouse.click("pic\\�ر�.png");
        mouse.click("pic\\�ر�1.png");
        mouse.click("pic\\�ر�2.png");
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(����ϴ�.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
        mouse.click("pic\\task\\����ϴ���.png");
    }
    FINDTASK(����Ĵ���.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        if(mouse.is_match_pic_in_screen("pic\\task\\�Ĵ���-��������.png", pt))
        {
            mouse.click(pt.x, pt.y);
        }
        else if(mouse.is_match_pic_in_screen("pic\\task\\ע��Ͱ�.png", pt))
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
    //ƥ������
    //TODO:
    while(1)
    {

        //::SetActiveWindow(wnd);
        ::SetForegroundWindow(wnd);

        Player_status status = Get_player_status();
        if(status == Player_status::COMBAT)
        {

            POINT pt;
            if(mouse.is_match_pic_in_screen("pic\\task\\��׽-��.png", pt))
            {
                mouse.click("pic\\��׽.png");
                mouse.click(pt.x, pt.y - 40);
            }

            //���²��Զ�ս��
            mouse.click("pic\\�Զ�ս��.png");

        }
        else if(status == Player_status::NORMAL)
        {
            task1to10();

            //�ضԻ���...
            mouse.click("pic\\�ر�.png");
            mouse.click("pic\\�ر�1.png");
            mouse.click("pic\\�ر�2.png");
            mouse.click("pic\\ȡ��.png");
        }



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
        else if(mouse.is_match_pic_in_screen("pic\\��Ϸ��.png"))
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
    POINT pt;
    if(mouse.is_match_pic_in_screen("pic\\ս����.png"), false)
    {
        MH_printf("ս��״̬");
        return Player_status::COMBAT;
    }
    else if(mouse.is_match_pic_in_screen("pic\\��Ϸ��.png"), false)
    {
        MH_printf("ƽ��״̬");
        return Player_status::NORMAL;
    }
    else if(mouse.is_match_pic_in_screen("pic\\����״̬.png", pt, false))
    {
        MH_printf("����״̬");
        return Player_status::NOTIME;
    }
    else if(mouse.is_match_pic_in_screen("pic\\��������.png", pt, false))
    {
        MH_printf("����״̬");
        return Player_status::GC;
    }
    else
    {
        std::runtime_error("δ֪�����״̬");
    }

}

void GameScriper::Regist_lua_fun()
{
    //lua
    MH_printf("����lua");
    if(lua_status == nullptr)
        lua_status = luaL_newstate();

    if(lua_status == nullptr) std::runtime_error("����lua״̬ʧ��");



    REGLUAFUN(lua_status, "��ȡ���״̬", [](lua_State *L)->int{
        GameScriper::GetInstance()->do_money();
        return 0;
    });

    REGLUAFUN(lua_status, "��ǰ����", [](lua_State *L)->int{
        const char* city = lua_tostring(L, 1);
        bool isin = GameScriper::GetInstance()->is_in_city(city);
        lua_pushboolean(L, isin);
        return 1;
    });

    mouse.Regist_lua_fun(lua_status);


    MH_printf("����lua���");
}

void GameScriper::Run()
{
    //���нű�
    MH_printf("�ű���ʼִ��, id:%d", std::this_thread::get_id());


    Regist_lua_fun();


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
