#include "scriptapp.h"
#include "mh_function.h"
#include "mh_gamescript.h"



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
        Sleep(150);
        goto tryagain;
    }
}


int ScriptApp::find_game_window()
{

    //清空窗口集合
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
    mhprintf("脚本执行..");


    int counts = find_game_window();
    if(counts == 0)
    {
        mhprintf("没有找到游戏窗口.");
        mhprintf("尝试开启几个游戏");
        launcher_game("username", "password");
    }


    //遍历这台电脑上所有游戏窗口
    for(size_t i = 0; i < Game_wnd_vec.size(); i++)
    {

        char title[256];
        ::GetWindowTextA(Game_wnd_vec[i], title, 256);

        mhprintf("窗口标题: %s", title);

        //为每个窗口分配一个线程单独操作
        Game_thread.push_back(std::thread([=]()
        {
            try
            {
                GameScriper script(Game_wnd_vec[i], i);
                script.Run();
            }
            catch(std::runtime_error &e)
            {
                printf(e.what());
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
