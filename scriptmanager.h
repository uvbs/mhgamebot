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
    std::vector<std::thread> game_threads;
    std::map<std::string, std::string> game_accounts;
    std::string script_filename;

    //回调
    std::vector<output_fun> _game_output_callback_list;
    help_fun _game_helper_callback;
    output_fun output_callback;

public:
    void mhprintf(LOG_TYPE logtype, const char *msg_format, ...);

    void set_script(std::string filename);
    std::string& get_script_name(){
        return script_filename;
    }

    void delete_all_script();

    //设置脚本管理器的输出回调
    void set_output_callback(output_fun _callback);
    void set_gamescript_callback_list(std::vector<output_fun> list);

    //帮助的回调
    void set_helper_callback(help_fun callback);



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
