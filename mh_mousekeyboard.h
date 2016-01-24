#ifndef MH_MOUSEKEYBOARD_H
#define MH_MOUSEKEYBOARD_H

#include <windows.h>
#include <lua.hpp>
#include <gdiplus.h>
using namespace Gdiplus;

#include "mh_function.h"


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

    //�����ַ� WM_CHAR
    void input(char* msg);

    //����Ļƥ��
    bool is_match_pic_in_screen(const char *image, bool screen_exisit = false);
    bool is_match_pic_in_screen(const char *image, POINT &point, bool screen_exisit = false);
    bool is_match_pic_in_point(const char *image, POINT &point, POINT game_wnd_pt, bool screen_exisit = false);
    bool is_match_pic_in_rect(const char *image, POINT &point, RECT game_wnd_rect);

    double Match_picture(std::string img1, const char *img2, cv::Point &maxLoc);

    POINT Get_cur_mouse();
    void Rand_move_mouse();
    void Until_stop_run();


    void input_event(const char *input);

    //ע��lua
    void Regist_lua_fun(lua_State* lua_status);


private:
    std::vector<int> Get_mouse_vec(int x, int y, int x2, int y2);
    int make_mouse_value(int x, int y);

public:
    static Mouse_keyboard* GetInstance(){
        if(_inst == nullptr)
            std::runtime_error("û��ʼ���� Mouse_Keyboard");
        return _inst;
    }

    WINBOOL WriteBmp(std::string strFile, HDC hdc);
    WINBOOL WriteBmp(std::string strFile, HDC hdc, const RECT &rcDC);
    WINBOOL WriteBmp(std::string strFile, const std::vector<BYTE> &vtData, const SIZE &sizeImg);
private:
    static Mouse_keyboard* _inst;

private:
    HWND wnd;
    int script_id;
    HDC hdc;

    //��������ƶ��Ĵ�С�ʹ��ڵı�, ����ת���������굽��Ϸ������
    double ratio_x;
    double ratio_y;

    int rx;
    int ry;
    int cur_game_x;
    int cur_game_y;
};

#endif // MH_MOUSEKEYBOARD_H
