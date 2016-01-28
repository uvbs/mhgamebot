#ifndef MH_MOUSEKEYBOARD_H
#define MH_MOUSEKEYBOARD_H

#include <windows.h>
#include <lua.hpp>
#include <gdiplus.h>
using namespace Gdiplus;

#include <opencv2/opencv.hpp>
using namespace cv;


#include "mh_config.h"


//���, ���̲���
class Mouse_keyboard
{
public:
    Mouse_keyboard(HWND gamewnd, int id);
    ~Mouse_keyboard(){}

    //�Ҽ����, ����״̬ȡ������
    void rclick(int x, int y);
    void rclick(const char *image);
    void click(int x, int y, bool lbutton = true);
    void click(const char *image);
    void click_nofix(int x, int y);
    void click_nofix(const char *image);
    void click_move(int x, int y, bool lbutton);


    //npc�Ի��еĵ��( �����ý��� )
    void Dialog_click(const char *img);


    //�����ַ� WM_CHAR
    void input(char* msg);

    //����Ļƥ��
    bool is_match_pic_in_screen(const char *image);
    bool is_match_pic_in_screen(const char *image, POINT &point);
    bool is_match_pic_in_rect(const char *image, POINT &point, const RECT &rect);

    //���ļ�ƥ��
    double Match_picture(const std::vector<uchar>& img1, const char* img2, cv::Point &maxLoc);

    //���ڴ�ƥ��
    double Match_picture(const std::vector<uchar>& img1, const std::vector<uchar>& img2, cv::Point &maxLoc);


    POINT Get_cur_mouse();
    void Rand_move_mouse();
    void Until_stop_run();


    void input_event(const char *input);

    //ע��lua
    void Regist_lua_fun(lua_State* lua_status);

public:
    void mhprintf(const char *msg, ...);

    HWND get_game_wnd(){
        return wnd;
    }

private:
    std::vector<int> Get_mouse_vec(int x, int y, int x2, int y2);
    int make_mouse_value(int x, int y);

public:
    static Mouse_keyboard* GetInstance(){
        if(_inst == nullptr)
            throw std::runtime_error("û��ʼ���� Mouse_Keyboard");
        return _inst;
    }

    std::vector<uchar> Get_screen_data();
    std::vector<uchar> Get_screen_data(const RECT &rcDC);
    bool Write_bmp_to_file(std::string file, const RECT &rect);

private:
    static Mouse_keyboard* _inst;

private:
    HWND wnd;
    int script_id;
    HDC hdc;
    std::vector<uchar> imgbuf;

    //��������ƶ��Ĵ�С�ʹ��ڵı�, ����ת���������굽��Ϸ������
    double ratio_x;
    double ratio_y;

    int rx;
    int ry;
    int cur_game_x;
    int cur_game_y;

    std::string player_name;
    static std::map<lua_State*, Mouse_keyboard*> inst_map;
public:
    void set_player_name(std::string name);

};

#endif // MH_MOUSEKEYBOARD_H
