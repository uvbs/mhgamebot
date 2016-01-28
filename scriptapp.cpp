#include "scriptapp.h"
#include "mh_config.h"
#include "mh_gamescript.h"

#define MHCHATWNDCLASS    "XYWZ_CHAT"

ScriptApp* ScriptApp::_inst = nullptr;
ScriptApp::ScriptApp()
{
    _inst = this;
}

bool ScriptApp::launcher_game(const char *username, const char *pw)
{

    return false;
}

void ScriptApp::mhprintf(const char *msg, ...)
{
    static bool can_printf = true;
tryagain:
    if(can_printf == true)
    {

        can_printf = false;

        //��Ҫһ������
        //TODO:
        printf("�ű�: ");

        va_list va;
        va_start(va, msg);
        vprintf(msg, va);
        va_end(va);

        printf("\n");

        can_printf = true;
    }
    else
    {
        Sleep(50);
        goto tryagain;
    }
}

int ScriptApp::hide_chat_window()
{
    HWND wnd = FindWindowExA(NULL, NULL, MHCHATWNDCLASS, NULL);
    if(wnd != NULL){
        for(;;){
            ShowWindow(wnd, SW_HIDE);

            wnd = FindWindowExA(NULL, wnd, MHCHATWNDCLASS, NULL);
            if(wnd == NULL) break;
        }

    }

    return 0;
}

int ScriptApp::find_game_window(const std::string& classname)
{

    //��մ��ڼ���
    Game_wnd_vec.clear();

    HWND wnd = FindWindowExA(NULL, NULL, classname.c_str(), NULL);
    if(wnd != NULL){
        for(;;){
            Game_wnd_vec.push_back(wnd);

            wnd = FindWindowExA(NULL, wnd, classname.c_str(), NULL);
            if(wnd == NULL) break;
        }

    }

    return Game_wnd_vec.size();
}

HANDLE ScriptApp::GetProcessHandle(int nID)//ͨ������ID��ȡ���̾��
{
    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, nID);
}


//�ر����д���
void ScriptApp::Close_all_game()
{
    find_game_window(GAME_WND_CLASS);
    for(int i = 0; i < Game_wnd_vec.size(); i++)
    {
        HWND wnd = Game_wnd_vec[i];
        DWORD pid;
        ::GetWindowThreadProcessId(wnd, &pid);
        HANDLE process = GetProcessHandle(pid);
        ::TerminateProcess(process, 0);
    }
}

void ScriptApp::Run()
{
    mhprintf("�ű�ִ��..");

    find_game_window(GAME_WND_CLASS);
    if(Game_wnd_vec.size() == 0)
    {
        mhprintf("û���ҵ���Ϸ����.");
        mhprintf("���Կ���������Ϸ");
        launcher_game("username", "password");
        return;
    }

    hide_chat_window();
    mhprintf("�ܹ�%d����Ϸ����", Game_wnd_vec.size());

    //������̨������������Ϸ����
    for(size_t i = 0; i < Game_wnd_vec.size(); i++)
    {

        char title[256];
        ::GetWindowTextA(Game_wnd_vec[i], title, 256);


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
                mhprintf(e.what());
            }
            catch(...)
            {
                mhprintf("δ֪�쳣!");
            }
        }));
    }

    //�ȴ��߳�ȫ���˳�
    for(size_t i = 0; i < Game_thread.size(); i++)
    {
        Game_thread[i].join();
    }

    mhprintf("�ű��˳�...");
}
