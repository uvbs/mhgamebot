#include "scriptmanager.h"
#include "config.h"
#include "gamescript.h"
#include "helperfun.h"


#include <shlwapi.h>
#include <boost/filesystem.hpp>
#include <fstream>




ScriptManager::ScriptManager()
{
    script_filename = "";
}

bool ScriptManager::launcher_game(int new_counts)
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
    
    for(int i = 0; i < new_counts; i++){
        sprintf(buf, "%s", gamepath.c_str());
        BOOL bok = ::CreateProcessA(NULL, buf, nullptr, nullptr, FALSE, 0, nullptr, workdir, &ls, &li);
        if(bok == FALSE){
            mhprintf(LOG_NORMAL,"一个窗口启动失败, 原因:%d", ::GetLastError());
        }
    }
    
    mhprintf(LOG_NORMAL,"等待窗口出现..");
    int counts = 0;
    do{
       counts = get_game_window(GAME_WND_CLASS);
    }
    while(counts != new_counts);
    
    return true;
}


void ScriptManager::mhprintf(LOG_TYPE logtype, const char *msg_format, ...)
{
    va_list va_args;
    va_start(va_args, msg_format);
    //_mhprintf("脚本", msg_format, va_args, logtype);
    char buf[256];
    vsprintf(buf, msg_format, va_args);
    if(output_callback != nullptr)
        output_callback(logtype, buf);
    va_end(va_args);
}

void ScriptManager::set_output_callback(std::function<void(int type, char*)> _callback)
{
    output_callback = _callback;
}

void ScriptManager::set_script(std::string filename)
{
    mhprintf(LOG_DEBUG, "选定脚本: %s", filename.c_str());
    script_filename = filename;
}

GameScript*ScriptManager::get_script(int id)
{
    for(auto script: game_scripts){
        if(script->get_id() == id)
            return script;
    }

    assert(false);
    return nullptr;
}


void ScriptManager::stop()
{
    for(auto script: game_scripts){
        script->end_task();
    }

    try{
    //等待线程全部退出
    for(size_t i = 0; i < game_threads.size(); i++){
        game_threads[i].join();
    }
    }
    catch(std::system_error &e){
        mhprintf(LOG_WARNING, e.what());
    }

    for(auto script: game_scripts){
        delete script;
    }

    game_scripts.clear();
    game_threads.clear();
}

void ScriptManager::pause()
{
    for(auto script: game_scripts){
        script->end_task();
    }
}

int ScriptManager::hide_chat_window()
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

int ScriptManager::get_game_window(const std::string& classname)
{

    //清空窗口集合
    game_wnds.clear();

    HWND wnd = FindWindowExA(NULL, NULL, classname.c_str(), NULL);
    if(wnd != NULL){
        for(;;){
            game_wnds.push_back(wnd);

            wnd = FindWindowExA(NULL, wnd, classname.c_str(), NULL);
            if(wnd == NULL) break;
        }

    }

    return game_wnds.size();
}

std::string ScriptManager::find_game_path()
{
    //从c盘找 pg 目录下有没有梦幻西游文件夹
    char drivers[] = "CDEF";
    for(size_t i = 0; i < strlen(drivers); i++){
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

HANDLE ScriptManager::get_process_handle(int nID)//通过进程ID获取进程句柄
{
    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, nID);
}


//关闭所有窗口
void ScriptManager::close_all_game()
{
    for(size_t i = 0; i < game_wnds.size(); i++)
    {
        DWORD pid;
        ::GetWindowThreadProcessId(game_wnds[i], &pid);
        HANDLE process = get_process_handle(pid);
        ::TerminateProcess(process, 0);
    }
}

void ScriptManager::list_window()
{
    int x = 0;
    int y = 0;
    
    //获取屏幕宽度
    RECT desk_rect;
    ::GetWindowRect(GetDesktopWindow(), &desk_rect);
    
    for(auto wnd: game_wnds)
    {
        if(y + SCREEN_HEIGHT > desk_rect.bottom - desk_rect.top){
            mhprintf(LOG_NORMAL,"显示器宽度不够....");
            break;
        }
        
        if(x + 640 > desk_rect.right - desk_rect.left){
            y += SCREEN_HEIGHT;
            x = 0;
        }
        
        
        ::SetWindowPos(wnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
        
        x += SCREEN_WIDTH;
        x += 20;
    }
}


std::vector<GameScript*>& ScriptManager::start()
{
    if(script_filename.empty()){
        mhprintf(LOG_WARNING, "先设置要加载的脚本");
        return game_scripts;
    }


    mhprintf(LOG_NORMAL,"运行脚本");


    //读取账户
    read_accounts();
    mhprintf(LOG_NORMAL,"读取到游戏账号: %d个", game_accounts.size());

    get_game_window(GAME_WND_CLASS);
    mhprintf(LOG_WARNING,"检测到%d个游戏窗口", game_wnds.size());

    hide_chat_window();
    mhprintf(LOG_INFO, "隐藏游戏聊天窗口");

    //mhprintf(LOG_NORMAL,"排序窗口");
    //list_window();
    
    //遍历这台电脑上所有游戏窗口
    for(size_t i = 0; i < game_wnds.size(); i++)
    {
        GameScript* script = new GameScript(game_wnds[i], i);

        //为每个窗口分配一个线程单独操作
        game_threads.push_back(std::thread([=](){
            script->start(script_filename);
        }));

        game_scripts.push_back(script);
    }

    return game_scripts;
}

void ScriptManager::read_accounts()
{
    game_accounts.clear();

    try{
        std::fstream file("帐号.txt");
        
        char line[200];
        file.getline(line, 200);
        
        std::string line_str(line);
        std::string name = line_str.substr(0, line_str.find(','));
        std::string password = line_str.substr(line_str.find(',')+1, line_str.length());
        game_accounts[name] = password;    

    }
    catch(...){
        throw std::runtime_error("error load accounts");
    }
    
    
}


