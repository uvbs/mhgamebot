#ifndef SCRIPTAPP_H
#define SCRIPTAPP_H


#include <windows.h>
#include <vector>
#include <thread>
#include <string>
#include <map>


#include "define.h"
#include "config.h"
#include "gamescript.h"



class ScriptManager
{
public:
    ScriptManager();

    //加载游戏
    bool launcher_game(int new_counts);

private:
    std::vector<GameScript*> game_scripts;
    std::vector<HWND> game_wnds;
    std::vector<std::thread> game_threads;
    std::map<std::string, std::string> game_accounts;
    std::function<void(int type, char*)> output_callback;
    std::string script_filename;
    std::vector<std::function<void(int type, char*)>> _game_callback_list;



public:
    void mhprintf(LOG_TYPE logtype, const char *msg_format, ...);
    void set_output_callback(std::function<void(int type, char*)> _callback);
    void set_script(std::string filename);
    void set_gamescript_callback_list(std::vector<std::function<void(int type, char*)>> list);
    void delete_all_script();


public:
    void stop();  //停止脚本
    void pause(); //暂停脚本
    void run();     //运行脚本
    void read_accounts();
    int find_game_window(const std::string& classname);
    std::string find_game_path();
    int hide_chat_window();
    HANDLE get_process_handle(int nID);
    void close_all_game();
    void list_window(); //排列游戏窗口
    void wait_thread_exit();
    void create_game();
    int get_game_counts();
};





#endif // SCRIPTAPP_H
