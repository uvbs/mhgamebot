#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include <Windows.h>
#include <thread>

#include <opencv2/opencv.hpp>
using namespace cv;

#include "mh_define.h"


void MH_printf(const char* msg, ...);


//保存游戏配置
class GameConfig
{
public:
    GameConfig(){
        //默认练级
        type = Script_type::LEVEL;
    }
    ~GameConfig(){}

    bool save_to_file()
    {
        return false;
    }

    bool load_from_file(){
        return false;
    }


    Script_type Get_script_type() {
        return this->type;
    }

    //设置脚本类型
    void Set_script_type(Script_type type){
        this->type = type;
    }

private:
    Script_type type;
    std::string srv_name;  //区名称
};







#endif // FUNCTION_H
