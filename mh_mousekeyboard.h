#ifndef MH_MOUSEKEYBOARD_H
#define MH_MOUSEKEYBOARD_H

#include <windows.h>
#include "mh_function.h"


//���, ���̲���
class Mouse_keyboard
{
public:
    Mouse_keyboard(HWND gamewnd);
    ~Mouse_keyboard(){}

    //�Ҽ����, ����״̬ȡ������
    void rclick(int x, int y);
    void rclick();
    void click(int x, int y);
    void click(const char *image);

    //�����ַ� WM_CHAR
    void input(char* msg);

    //����Ļƥ��
    bool is_match_pic_in_screen(const char *image);
    bool is_match_pic_in_screen(const char *image, POINT &point);
    bool is_match_pic_in_point(const char *image, POINT &point, POINT game_wnd_pt);
    void Right_point();
    double Match_picture(const char *img1, const char *img2, Point &maxLoc);
    void Rang_move_mouse();

    void Until_stop_run();
private:
    HWND wnd;
    HDC hdc;
    //����һ����������ʱ(�����ͼƥ������Ľ��), ��εõ���Ϸ�ڵ�������?
    //Right_point�������У׼���, ȡ��һ����������̶�ֵʱ
    //��Ϸ�ڵ����ƫ��


    //�����Ϸ����Ϸ�����ʹ���������в���
    //���細������ 2,2��δ֪, ��Ϸ�����ָ�� WM_MOUSEMOVE 4,4���ص�
    //�󲿷���������, 2,2�п�������Ϸ�ڵ������� 32,51.
    //����ÿ�ε�����Ƶ�����֮��ʱ�������䶯
    int rx;
    int ry;

    int cur_game_x;
    int cur_game_y;

    //��Ϸ������ƶ��Ŀ�͸ߺʹ���Ҳ���в���.
    double ratio_x;
    double ratio_y;

private:
    std::vector<int> Get_mouse_vec(int x, int y, int x2, int y2);
    int make_mouse_value(int x, int y);
public:
    void test1();
    void rclick(const char *image);
    bool is_match_pic_in_rect(const char *image, POINT &point, RECT game_wnd_rect);
    void click_nofix(int x, int y);
    POINT Get_cur_mouse();
};

#endif // MH_MOUSEKEYBOARD_H
