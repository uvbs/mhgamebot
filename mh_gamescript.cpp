#include "mh_gamescript.h"
#include <regex>
#include <thread>
#include <boost/lexical_cast.hpp>

#define MHCHATWND "�λ�����2 ���촰��"


#define FIRSTTASK(x)\
    if(mouse.is_match_pic_in_screen("pic\\task\\"#x, pt))

#define FINDTASK(x) \
    else if(mouse.is_match_pic_in_screen("pic\\task\\"#x, pt))

#define ENDTASK \
    else { mhprintf("ʲô����.."); mouse.Rand_move_mouse(); }

std::map<lua_State*, GameScriper*> GameScriper::inst_map;
GameScriper::GameScriper(HWND game_wnd, int id):
    mouse(game_wnd, id),
    player_name(std::string("����")+boost::lexical_cast<std::string>(id)),
    player_level("0")
{
    script_id = id;
    wnd = game_wnd;
    lua_status = nullptr;
    if(lua_status == nullptr)
        lua_status = luaL_newstate();

    if(lua_status == nullptr)
        throw std::runtime_error("����lua״̬ʧ��");

    //�����lua״̬�����ʵ����
    inst_map[lua_status] = this;
}

void GameScriper::mhprintf(const char* msg, ...)
{
    static bool can_printf = true;
tryagain:
    if(can_printf == true)
    {

        can_printf = false;

        //��Ҫһ������
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
        mhprintf("��ǰ���� %s..", city);
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
    FINDTASK(���Ի�.png)
    {
        mouse.click(pt.x, pt.y);
        mouse.Until_stop_run();
        mouse.click(640/2, 480/2);
    }
    FINDTASK(����.png)
    {
        mouse.click(rect_tools.x, rect_tools.y);
        mouse.rclick("pic\\task\\��ͭ�̽�.png");
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

        if(is_in_city("pic\\map\\������.png")){
           mouse.click(640, 480);
        }
        else if(is_in_city("pic\\map\\������.png")){
            mouse.click(400, 0);
        }
        else if(is_in_city("pic\\map\\��Դ��.png")){
            mouse.click(269,253);
            mouse.Dialog_click("pic\\�Ի���-ս��.png");
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
        mouse.Dialog_click("pic\\�Ի���-ս��.png");
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
        mouse.Dialog_click("pic\\task\\������β�.png");
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
        mouse.Dialog_click("pic\\task\\����ϴ���.png");
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


//�������, �������
bool GameScriper::check_offline()
{

    HWND wnd = FindWindowExA(NULL, NULL, nullptr, "�������");
    if(wnd)
    {
        mhprintf("��⵽�������...");
        mhprintf("��������..");

        ::SendMessageA(wnd, WM_COMMAND, 1, 0);
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

        try
        {
            if(check_offline())
                break;

            PLAYER_STATUS status = Get_player_status();
            if(status == PLAYER_STATUS::COMBAT)
            {
                //�в˵������ٽ��в���
                if(mouse.is_match_pic_in_screen("pic\\task\\ս��-����.png"))
                {
                    POINT pt;
                    if(mouse.is_match_pic_in_screen("pic\\task\\��׽-��.png", pt))
                    {
                        mouse.click("pic\\��׽.png");
                        mouse.click(pt.x, pt.y - 50);
                        continue;
                    }

                    //���²��Զ�ս��
                    mouse.click("pic\\�Զ�ս��.png");
                }

            }
            else if(status == PLAYER_STATUS::NORMAL)
            {
                //�ضԻ���...
                mouse.click("pic\\�ر�.png");
                mouse.click("pic\\�ر�1.png");
                mouse.click("pic\\�ر�2.png");
                mouse.click("pic\\ȡ��.png");

                task1to10();
            }
        }
        catch(exception_xy &e)
        {
            mhprintf("%s, ���±�������", e.what());
        }
        catch(exception_status &e)
        {
            mhprintf("%s, ���³���һ��", e.what());
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
        else if(mouse.is_match_pic_in_screen("pic\\��Ϸ��.png"))
        {
            //��ȡ�������
            char title[256];
            ::GetWindowTextA(wnd, title, 256);
            //mhprintf("%s", title);


            //������ƥ�����
            std::regex regex(R"(.*-\s?(.+)\[\d+\].*)");
            std::cmatch cmatch;
            if(std::regex_match(title, cmatch, regex))
            {
                player_name = cmatch[1];
                mouse.set_player_name(cmatch[1]);

                //debug
                //mhprintf("%s", cmatch[1]);
            }

            //������Ϸ�ų���...
            break;
        }
        else if(mouse.is_match_pic_in_screen("pic\\selplayer.png"))
        {
            mouse.click_nofix("pic\\ok.png");
            mouse.click_nofix("pic\\nextstep.png");
        }
        else
        {
            mhprintf("δ֪����..");
        }

        Sleep(200);
    }

}


//��ȡ���״̬
PLAYER_STATUS GameScriper::Get_player_status()
{
    if(mouse.is_match_pic_in_screen("pic\\ս����.png"))
    {
        mhprintf("ս��״̬");
        return PLAYER_STATUS::COMBAT;
    }
    else if(mouse.is_match_pic_in_screen("pic\\��Ϸ��.png"))
    {
        mhprintf("ƽ��״̬");
        return PLAYER_STATUS::NORMAL;
    }
    else if(mouse.is_match_pic_in_screen("pic\\����״̬.png"))
    {
        mhprintf("����״̬");
        return PLAYER_STATUS::NOTIME;
    }
    else if(mouse.is_match_pic_in_screen("pic\\��������.png"))
    {
        mhprintf("����״̬");
        return PLAYER_STATUS::GC;
    }
    else
    {
        throw exception_status("δ֪�����״̬");
    }

}

void GameScriper::Regist_lua_fun()
{

    REGLUAFUN(lua_status, "��ȡ���״̬", [](lua_State *L)->int{
        GameScriper::Get_instance(L)->do_money();
        return 0;
    });

    REGLUAFUN(lua_status, "��ǰ����", [](lua_State *L)->int{
        const char* city = lua_tostring(L, 1);
        bool isin = GameScriper::Get_instance(L)->is_in_city(city);
        lua_pushboolean(L, isin);
        return 1;
    });

    mouse.Regist_lua_fun(lua_status);
}

void GameScriper::Run()
{
    //���нű�
    mhprintf("%d�ű�ִ��", std::this_thread::get_id());


    Regist_lua_fun();


    mhprintf("��ʼ������Ϸ");
    Entry_game();
    mhprintf("��ȥ��Ϸ[���]");


    Script_type type = config.Get_script_type();
    mhprintf("ִ�нű�: %s", Script_type_desc[type].str.c_str());

    if(type == Script_type::SMART){

    }
    else if(type == Script_type::MONEY){
        do_money();
    }
    else if(type == Script_type::LEVEL){
        do_task();
    }
    else{
        throw std::runtime_error("δ֪�ű�����");
    }

}
