#ifndef SCRIPTAPP_H
#define SCRIPTAPP_H



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
    std::vector<std::thread*> game_threads;
    std::map<std::string, std::string> game_accounts;
    std::string script_filename;

    //回调
    output_fun output_callback;


public:
    void mhprintf(LOG_TYPE logtype, const char *msg_format, ...);

    void set_script(const std::string& filename);


    std::vector<GameScript*>& create_all_script();
    //设置脚本管理器的输出回调
    void set_output_callback(output_fun _callback);


public:
    void stop();  //停止脚本
    void start();     //运行脚本

    GameScript* get_script(int id);
    void read_accounts();
    const std::vector<HWND>& get_game_window(const std::string& classname);
    std::string find_game_path();
    void hide_chat_window();
    HANDLE get_process_handle(int nID);
    void close_all_game();
    void list_window(); //排列游戏窗口

};





#endif // SCRIPTAPP_H
