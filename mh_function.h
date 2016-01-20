#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include <Windows.h>
#include <thread>

#include <opencv2/opencv.hpp>
using namespace cv;


#include "mh_define.h"

//��Ϸ����
extern std::vector<HWND> Game_wnd_vec;
extern std::vector<std::thread> Game_thread;


void MH_printf(const char* msg, ...);
int find_game_window();





//������Ϸ����
class GameConfig
{
public:
    GameConfig(){
        //Ĭ������
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

    //���ýű�����
    void Set_script_type(Script_type type){
        this->type = type;
    }

private:
    Script_type type;
    std::string srv_name;  //������
};







#endif // FUNCTION_H
