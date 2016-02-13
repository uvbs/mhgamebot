#include "scriptapp.h"
#include "config.h"
#include "gamescript.h"

#include <shlwapi.h>
#include <boost/filesystem.hpp>


void print_err_msg(DWORD msg)
{
    char buf[128];
    LPSTR lpMsgBuf;
    FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                msg,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                lpMsgBuf,
                0, NULL );
    sprintf(buf,
             "出错信息 (出错码=%d): %s",
             msg, lpMsgBuf);
    
    printf(buf);
    LocalFree(lpMsgBuf);
}


ScriptApp::ScriptApp()
{
}

bool ScriptApp::launcher_game()
{
    char buf[500];
    std::string gamepath = find_game_path();
    if(gamepath.empty()){
        throw std::runtime_error("没能找到游戏路径");
    }
    
    
    STARTUPINFOA ls;
    memset(&ls, 0, sizeof(STARTUPINFO));
    ls.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION li;
    memset(&li, 0, sizeof(PROCESS_INFORMATION));
    
    
    //workdir
    char workdir[MAX_PATH];
    strcpy(workdir, gamepath.c_str());
    ::PathRemoveFileSpecA(workdir);
    gamepath += " __start_by_mh_launcher__ 919026 0";
    
    for(int i = 0; i < 5; i++){
        sprintf(buf, "%s", gamepath.c_str());
        BOOL bok = ::CreateProcessA(NULL, buf, nullptr, nullptr, FALSE, 0, nullptr, workdir, &ls, &li);
        if(bok == FALSE){
            mhprintf("一个窗口启动失败, 原因:%d", ::GetLastError());
        }
    }
    
    mhprintf("等待所有窗口出现...");
    int counts = 0;
    do{
       counts = find_game_window(GAME_WND_CLASS);
    }
    while(counts != 5);
    
    mhprintf("ok..搞起");
    
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

std::string ScriptApp::find_game_path()
{
    //从c盘找 pg 目录下有没有梦幻西游文件夹
    char drivers[] = "CDEF";
    for(int i = 0; i < strlen(drivers); i++){
        char path[256];
        sprintf(path, R"(%c:\Program Files (x86)\梦幻西游)", drivers[i]);
        if(::PathIsDirectoryA(path))
        {
            std::string finded(path);
            finded += "\\mhmain.exe";
            return finded;
        }
    }

    return "";
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

void ScriptApp::list_window()
{
    int x = 0;
    int y = 0;
    
    //获取屏幕宽度
    RECT desk_rect;
    ::GetWindowRect(GetDesktopWindow(), &desk_rect);
    
    for(auto wnd: Game_wnd_vec)
    {
        if(y + SCREEN_HEIGHT > desk_rect.bottom - desk_rect.top){
            mhprintf("显示器宽度不够....");
            break;
        }
        
        if(x + 640 > desk_rect.right - desk_rect.left){
            y += SCREEN_HEIGHT;
            x = 0;
        }
        
        
        ::SetWindowPos(wnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
        
        x += SCREEN_WIDTH;
    }
}

void ScriptApp::run()
{
    mhprintf("脚本执行..");

    find_game_window(GAME_WND_CLASS);
    if(Game_wnd_vec.size() == 0)
    {
        mhprintf("没有找到游戏窗口.");
        mhprintf("那就开启几个...");
        launcher_game();
    }

    hide_chat_window();
    mhprintf("检测到%d个游戏窗口", Game_wnd_vec.size());

    mhprintf("排序窗口");
    list_window();
    
    
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
                GameScript script(Game_wnd_vec[i], i);
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
