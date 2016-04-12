#include "scriptmanager.h"
#include "config.h"
#include "gamescript.h"
#include "helperfun.h"


#include <shlwapi.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <QFileDialog>



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
            mhprintf(LOG_NORMAL, "一个窗口启动失败, 原因:%d", ::GetLastError());
        }
    }
    
    mhprintf(LOG_NORMAL,"等待窗口出现..");
    int counts = 0;
    do{
       auto wndvec = get_game_window(GAME_WND_CLASS);
       counts = wndvec.size();
    }
    while(counts != new_counts);
    
    return true;
}


void ScriptManager::mhprintf(LOG_TYPE logtype, const char *msg_format, ...)
{
	assert(output_callback != nullptr);
	
    va_list va_args;
    va_start(va_args, msg_format);
    //_mhprintf("脚本", msg_format, va_args, logtype);

    char buf[256];
    vsprintf(buf, msg_format, va_args);


	output_callback(logtype, buf);
    va_end(va_args);
}

void ScriptManager::set_output_callback(std::function<void(int type, const char*)> _callback)
{
    output_callback = _callback;
}

void ScriptManager::set_script(const std::string& filename)
{
    mhprintf(LOG_INFO, "选定脚本: %s", filename.c_str());
    script_filename = filename;
}

void ScriptManager::stop()
{
    for(auto pscript: game_scripts){
        pscript->end_task();
    } 

    //等待线程退出
    for(auto pthread: game_threads){
        pthread->join();
        delete pthread;
    }


    //清除所有脚本
    for(auto pscript: game_scripts){
        delete pscript;
    }

    
    game_scripts.clear();
    game_threads.clear();

    mhprintf(LOG_INFO, "已停止");
}


void ScriptManager::hide_chat_window()
{
    HWND wnd = FindWindowExA(NULL, NULL, MHCHATWNDCLASS, NULL);
    if(wnd != NULL){
        for(;;){
            ShowWindow(wnd, SW_HIDE);

            wnd = FindWindowExA(NULL, wnd, MHCHATWNDCLASS, NULL);
            if(wnd == NULL) break;
        }

    }
}

const std::vector<HWND>& ScriptManager::get_game_window(const std::string& classname)
{

    //清空窗口集合
    //TODO
    if(game_wnds.size() <= 0)
    {
        game_wnds.clear();

        HWND wnd = FindWindowExA(NULL, NULL, classname.c_str(), NULL);
        if(wnd != NULL){
            for(;;){
                game_wnds.push_back(wnd);

                wnd = FindWindowExA(NULL, wnd, classname.c_str(), NULL);
                if(wnd == NULL) break;
            }

        }
    }

    return game_wnds;
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
    get_game_window(GAME_WND_CLASS);
    for(size_t i = 0; i < game_wnds.size(); i++)
    {
        DWORD pid;
        ::GetWindowThreadProcessId(game_wnds[i], &pid);
        HANDLE process = get_process_handle(pid);
        ::TerminateProcess(process, 0);
    }

    game_wnds.clear();
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


std::vector<GameScript*>& ScriptManager::create_all_script()
{
    stop();

    get_game_window(GAME_WND_CLASS);

    //遍历这台电脑上所有游戏窗口
    for(size_t i = 0; i < game_wnds.size(); i++)
    {
        GameScript* script = new GameScript(game_wnds[i], i);
        game_scripts.push_back(script);
    }

    return game_scripts;
}

void ScriptManager::start()
{
    if(script_filename.empty()){
        mhprintf(LOG_WARNING, "先设置要加载的脚本");
        return;
    }

    mhprintf(LOG_NORMAL, "开始执行脚本");

    //读取账户
    //TODO
    //read_accounts();
    //mhprintf(LOG_NORMAL, "读取到游戏账号: %d个", game_accounts.size());


    hide_chat_window();
    mhprintf(LOG_INFO, "隐藏游戏聊天窗口");

    //mhprintf(LOG_NORMAL,"排序窗口");
    //list_window();

    auto it = game_accounts.begin();

//    if(game_accounts.size() < game_wnds.size()){
//        qDebug() << "game_accounts" << game_accounts.size();
//        throw std::runtime_error("accounts small than game window!");
//    }

    if(game_scripts.size() == 0){
        mhprintf(LOG_INFO, "没有找到游戏窗口");
        mhprintf(LOG_INFO, "脚本退出");
    }
    else{
        for(auto script: game_scripts){
            game_threads.push_back(new std::thread([=](){
                script->start(script_filename);
            }));
        }

        mhprintf(LOG_INFO, "运行中..");
    }

}

GameScript* ScriptManager::get_script(int id)
{
    return game_scripts[id];
}

void ScriptManager::read_accounts()
{
    game_accounts.clear();

    try{

        QString fileName = QFileDialog::getOpenFileName(
                               nullptr,
                               QString::fromLocal8Bit("选择帐号列表"),
                               "",
                               QString::fromLocal8Bit("帐号 (*.txt)"));


        QFile inputFile(fileName);
        if (inputFile.open(QIODevice::ReadOnly))
        {
            QTextStream in(&inputFile);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                std::string line_str(line.toStdString());
                std::string name = line_str.substr(0, line_str.find(','));
                std::string password = line_str.substr(line_str.find(',')+1, line_str.length());
                qDebug() << name.c_str();
                qDebug() << password.c_str();
                game_accounts[name] = password;
            }

            inputFile.close();
        }
        else{
            throw std::runtime_error("open file error!");
        }

    }
    catch(...){
        throw std::runtime_error("error load accounts");
    }
    
    
}


