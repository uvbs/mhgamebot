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
    void rclick(const char *image);
    void click(int x, int y);
    void click(const char *image);
    void click_nofix(int x, int y);
    void click_nofix(const char *image);
    void click_move(int x, int y);

    //输入字符 WM_CHAR
    void input(char* msg);

    //从屏幕匹配
    bool is_match_pic_in_screen(const char *image);
    bool is_match_pic_in_screen(const char *image, POINT &point);
    bool is_match_pic_in_point(const char *image, POINT &point, POINT game_wnd_pt, bool screen_exisit = false);
    bool is_match_pic_in_rect(const char *image, POINT &point, RECT game_wnd_rect);

    double Match_picture(const char *img1, const char *img2, Point &maxLoc);

    POINT Get_cur_mouse();
    void Rand_move_mouse();
    void Until_stop_run();


    void input_event(const char *input);
private:
    std::vector<int> Get_mouse_vec(int x, int y, int x2, int y2);
    int make_mouse_value(int x, int y);

private:
    HWND wnd;
    HDC hdc;

    //鼠标内能移动的大小和窗口的比, 用来转换窗口座标到游戏内座标
    double ratio_x;
    double ratio_y;

    int rx;
    int ry;
    int cur_game_x;
    int cur_game_y;
};

#endif // MH_MOUSEKEYBOARD_H
