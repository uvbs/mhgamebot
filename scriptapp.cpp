#include "scriptapp.h"
#include "mh_function.h"
#include "mh_gamescript.h"

ScriptApp::ScriptApp()
{

}

bool ScriptApp::launcher_game(const char *username, const char *pw)
{

    return false;
}


int ScriptApp::find_game_window()
{

    //��մ��ڼ���
    Game_wnd_vec.clear();

    HWND wnd = FindWindowEx(NULL, NULL, GAME_WND_CLASS, NULL);
    if(wnd != NULL){
        for(;;){
            Game_wnd_vec.push_back(wnd);

            wnd = FindWindowEx(NULL, wnd, GAME_WND_CLASS, NULL);
            if(wnd == NULL) break;
        }

    }

    return Game_wnd_vec.size();
}




void ScriptApp::Run()
{
    MH_printf("�ű�ִ��..");


    int counts = find_game_window();
    if(counts == 0)
    {
        MH_printf("û���ҵ���Ϸ����.");
        MH_printf("���Կ���������Ϸ");
        launcher_game("username", "password");
    }


    //������̨������������Ϸ����
    for(size_t i = 0; i < Game_wnd_vec.size(); i++)
    {

        char title[256];
        ::GetWindowTextA(Game_wnd_vec[i], title, 256);

        MH_printf("���ڱ���: %s", title);

        //Ϊÿ�����ڷ���һ���̵߳�������
        Game_thread.push_back(std::thread([=]()
        {
            try
            {
                GameScriper script(Game_wnd_vec[i], i);
                script.Run();
            }
            catch(std::runtime_error &e)
            {
                MH_printf(e.what());
            }
            catch(...)
            {
                MH_printf("δ֪�쳣!");
            }
        }));
    }

    //�ȴ��߳�ȫ���˳�
    for(size_t i = 0; i < Game_thread.size(); i++)
    {
        Game_thread[i].join();
    }

    MH_printf("�ű��˳�...");
}
