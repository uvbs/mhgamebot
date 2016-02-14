#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include <Windows.h>


#include "define.h"



//保存游戏配置
struct GameConfig
{

    GameConfig():
        lua_txt_name("task.lua"),
        type(LEVEL),
        start_counts(1)
    {}

    ~GameConfig(){}

    bool save_to_file()
    {
        return false;
    }

    bool load_from_file(){
        return false;
    }


    Script_type type;
    std::string srv_name;  //区名称
    std::string lua_txt_name; //脚本名称
    int start_counts;       //自动开启客户端数量
};







#endif // FUNCTION_H
