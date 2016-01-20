#ifndef MH_MOUSEKEYBOARD_H
#define MH_MOUSEKEYBOARD_H

#include <windows.h>
#include "mh_function.h"


//鼠标, 键盘操作
class Mouse_keyboard
{
public:
    Mouse_keyboard(HWND gamewnd);
    ~Mouse_keyboard(){}

    //右键点击, 攻击状态取消攻击
    void rclick(int x, int y);
    void rclick();
    void click(int x, int y);
    void click(const char *image);

    //输入字符 WM_CHAR
    void input(char* msg);

    //从屏幕匹配
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
    //当有一个窗口坐标时(比如截图匹配出来的结果), 如何得到游戏内的坐标呢?
    //Right_point这个函数校准鼠标, 取得一个窗口坐标固定值时
    //游戏内的鼠标偏移


    //这个游戏的游戏内鼠标和窗口鼠标是有差距的
    //比如窗口坐标 2,2的未知, 游戏内鼠标指向 WM_MOUSEMOVE 4,4才重叠
    //大部分情况更糟糕, 2,2有可能在游戏内的坐标是 32,51.
    //而且每次当鼠标移到窗口之外时这个误差会变动
    int rx;
    int ry;

    int cur_game_x;
    int cur_game_y;

    //游戏内鼠标移动的宽和高和窗口也是有差距的.
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
