#include "scriptapp.h"
#include "mh_config.h"
#include "gamescript.h"

#include <shlwapi.h>


ScriptApp::ScriptApp()
{
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

        //需要一个互斥
        //TODO:
        printf("脚本: ");

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

    //清空窗口集合
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

HANDLE ScriptApp::get_process_handle(int nID)//通过进程ID获取进程句柄
{
    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, nID);
}


//关闭所有窗口
void ScriptApp::close_all_game()
{
    find_game_window(GAME_WND_CLASS);
    for(size_t i = 0; i < Game_wnd_vec.size(); i++)
    {
        HWND wnd = Game_wnd_vec[i];
        DWORD pid;
        ::GetWindowThreadProcessId(wnd, &pid);
        HANDLE process = get_process_handle(pid);
        ::TerminateProcess(process, 0);
    }
}

void ScriptApp::run()
{
    mhprintf("脚本执行..");

    find_game_window(GAME_WND_CLASS);
    if(Game_wnd_vec.size() == 0)
    {
        mhprintf("没有找到游戏窗口.");
        mhprintf("尝试开启几个游戏");
        launcher_game("username", "password");
        return;
    }

    hide_chat_window();
    mhprintf("检测到%d个游戏窗口", Game_wnd_vec.size());

    //遍历这台电脑上所有游戏窗口
    for(size_t i = 0; i < Game_wnd_vec.size(); i++)
    {

        char title[256];
        ::GetWindowTextA(Game_wnd_vec[i], title, 256);


        //为每个窗口分配一个线程单独操作
        Game_thread.push_back(std::thread([=]()
        {
            try
            {
                GameScriper script(Game_wnd_vec[i], i);
                script.run();
            }
            catch(const std::runtime_error &e)
            {
                mhprintf(e.what());
            }
            catch(...)
            {
                mhprintf("未知异常!");
            }
        }));
    }

    //等待线程全部退出
    for(size_t i = 0; i < Game_thread.size(); i++)
    {
        Game_thread[i].join();
    }

    mhprintf("脚本退出...");
}
