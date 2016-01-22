#include "mh_mousekeyboard.h"
#include "mh_function.h"
#include "mh_writebmp.h"



int Mouse_keyboard::make_mouse_value(int x, int y)
{
    int v = 0;
    v = x;
    y = y << 16;
    v = v + y;

    ratio_x = (double)590/(double)640;
    ratio_y = (double)450/(double)480;

    rx = 0;
    ry = 0;
    cur_game_x = 0;
    cur_game_y = 0;

    return v;
}

void Mouse_keyboard::input_event(const char* input)
{
    for(size_t i = 0; i < strlen(input); i++)
    {

        keybd_event(input[i],0,0,0);     //按下a键
        keybd_event(input[i],0,KEYEVENTF_KEYUP,0);//松开a键

    }

}

Mouse_keyboard::Mouse_keyboard(HWND gamewnd)
{
    wnd = gamewnd;
    hdc = ::GetDC(gamewnd);
}


void Mouse_keyboard::rclick(int x, int y)
{
    //TODO:
}

void Mouse_keyboard::rclick()
{
    //TODO:
}

void Mouse_keyboard::rclick(const char* image)
{
    POINT point;
    if(is_match_pic_in_screen(image, point))
    {
        rclick(point.x, point.y);
    }
    else
    {
        MH_printf("rclick error! no pic match!");
    }
}

void Mouse_keyboard::click(const char* image)
{
    POINT point;
    if(is_match_pic_in_screen(image, point))
    {
        click(point.x, point.y);
    }
}

void Mouse_keyboard::click_nofix(const char* image)
{

    POINT point;
    if(is_match_pic_in_screen(image, point))
    {
        click_nofix(point.x, point.y);
    }

}


bool Mouse_keyboard::is_match_pic_in_screen(const char *image)
{
    POINT pt;
    return is_match_pic_in_screen(image, pt);
}


double Mouse_keyboard::Match_picture(const char* img1, const char* img2, Point &maxLoc)
{

    //MH_printf("匹配图片: %s - %s", img1, img2);

    if(img1 == nullptr || img2 == nullptr)
        std::runtime_error("参数错误");

    if(wnd == nullptr)
        std::runtime_error("无效的句柄");

    //char img1_fix[40];
    //sprintf(img1_fix, "%d%s", ::GetCurrentThreadId(), img1);
    Mat img_screen = imread(img1);
    Mat img_in = imread(img2);
    Mat result;
    if(img_in.empty() || img_screen.empty()){
        std::runtime_error("原图片是空图");
    }

    //匹配方式
    int match_method = TM_CCOEFF_NORMED;
    matchTemplate(img_screen, img_in, result, TM_CCOEFF_NORMED);
    //normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// 通过函数 minMaxLoc 定位最匹配的位置
    double minVal;
    double maxVal;
    Point minLoc;
    Point matchLoc;

    cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

    /// 对于方法 SQDIFF 和 SQDIFF_NORMED, 越小的数值代表更高的匹配结果. 而对于其他方法, 数值越大匹配越好
    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }
    else{
        matchLoc = maxLoc;
    }

//    rectangle( img_screen, matchLoc, Point( matchLoc.x + img_in.cols , matchLoc.y + img_in.rows ), Scalar::all(0), 2, 8, 0 );
//    rectangle( result, matchLoc, Point( matchLoc.x + img_in.cols , matchLoc.y + img_in.rows ), Scalar::all(0), 2, 8, 0 );

//    imshow( image_window, img_screen );
//    imshow( result_window, result );

//    MH_printf("match x:%d, y:%d", matchLoc.x, matchLoc.y);
//    MH_printf("img_in x:%d, y:%d", img_in.cols, img_in.rows);
//    MH_printf("minVal, maxVal x:%f, y:%f", minVal, maxVal);
//    MH_printf("minLoc x:%d, y:%d", minLoc.x, minLoc.y);
//    MH_printf("maxLoc x:%d, y:%d", maxLoc.x, maxLoc.y);
//    cv::waitKey(0);

    return maxVal;
}

bool Mouse_keyboard::is_match_pic_in_rect(const char *image, POINT &point, RECT game_wnd_rect)
{

    //取得屏幕图片
    WriteBmp("screen.bmp", hdc, game_wnd_rect);
    Point maxLoc;
    double maxVal = Match_picture("screen.bmp", image, maxLoc);

    if(maxVal > 0.6)
    {
        Mat img_in = imread(image);
        point.x = maxLoc.x + img_in.cols/2 + game_wnd_rect.left;
        point.y = maxLoc.y + img_in.rows/2 + game_wnd_rect.top;
        return true;
    }
    else
    {
        return false;
    }
}



bool Mouse_keyboard::is_match_pic_in_point(const char *image, POINT &point, POINT game_wnd_pt, bool screen_exisit)
{

    if(screen_exisit == false)
    {
        RECT rect;
        ::GetClientRect(wnd, &rect);
        rect.left = game_wnd_pt.x;
        rect.top = game_wnd_pt.y;

        //取得屏幕图片
        WriteBmp("screen.bmp", hdc, rect);
    }

    Point maxLoc;
    double maxVal = Match_picture("screen.bmp", image, maxLoc);

    if(maxVal > 0.7)
    {
        Mat img_in = imread(image);
        point.x = maxLoc.x + img_in.cols/2 + game_wnd_pt.x;
        point.y = maxLoc.y + img_in.rows/2 + game_wnd_pt.y;
        return true;
    }
    else
    {
        return false;
    }
}

bool Mouse_keyboard::is_match_pic_in_screen(const char *image, POINT &point)
{

    //取得两个对比的图
    Sleep(100);
    WriteBmp("screen.bmp", hdc);
    Point maxLoc;
    double maxVal = Match_picture("screen.bmp", image, maxLoc);

    if(maxVal > 0.7)
    {
        Mat img_in = imread(image);
        point.x = maxLoc.x + img_in.cols/2;
        point.y = maxLoc.y + img_in.rows/2;

        //TODO: 随机一点
        return true;
    }
    else
    {
        return false;
    }
}

void Mouse_keyboard::Rand_move_mouse()
{
    //取得当前坐标到目的的差值
    int x = rand()%300+100;
    int y = rand()%300+100;

    int v = make_mouse_value(x, y);
    ::SendMessage(wnd, WM_MOUSEMOVE, 0, v);

    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(100);
}


//TODO: 需要一个超时
void Mouse_keyboard::Until_stop_run()
{
    while(1)
    {
        //来一张
        WriteBmp("position1.bmp", hdc, rect_position);
        Sleep(2000);
        WriteBmp("position2.bmp", hdc, rect_position);
        Point maxLoc;
        double isMatch = Match_picture("position1.bmp", "position2.bmp", maxLoc);
        if(isMatch > 0.9)
        {
            break;
        }
    }

}

//x -> x2
std::vector<int> Mouse_keyboard::Get_mouse_vec(int x, int y, int x2, int y2)
{
    std::vector<int> mouse_vec;


    if(x < x2)
    {
        for(int i = x+1; i < x2; i++)
        {
            int v = make_mouse_value(i, y);
            mouse_vec.push_back(v);
        }
    }
    else if(x > x2)
    {
        for(int i = x-1; i > x2; i--)
        {
           int v = make_mouse_value(i, y);
            mouse_vec.push_back(v);
        }
    }



    if(y < y2)
    {
        for(int i = y+1; i < y2; i++)
        {
           int v = make_mouse_value(x2, i);
            mouse_vec.push_back(v);
        }
    }
    else if(y > y2)
    {

        for(int i = y-1; i > y2; i--)
        {
            int v = make_mouse_value(x2, i);
            mouse_vec.push_back(v);
        }
    }

    return mouse_vec;
}

void Mouse_keyboard::click_nofix(int x, int y)
{
    int v = make_mouse_value(x, y);
    ::SendMessage(wnd, WM_MOUSEMOVE, 0, v);
    ::PostMessage(wnd, WM_LBUTTONDOWN, 1,v);
    ::PostMessage(wnd, WM_LBUTTONUP, 0,v);
    Sleep(200);
    Rand_move_mouse();
}

//匹配屏幕获取当前鼠标位置
POINT Mouse_keyboard::Get_cur_mouse()
{

letstart:
    Sleep(100);
    WriteBmp("screen.bmp", hdc);
    Point maxLoc;
    bool first_mouse = true;
    bool third_mouse = false;
    double val = Match_picture("screen.bmp", "pic\\chk\\mouse1.png", maxLoc);
    if(val < 0.9)
    {
        first_mouse = false;
        val = Match_picture("screen.bmp", "pic\\chk\\mouse2.png", maxLoc);
    }

    if(val < 0.9)
    {
        third_mouse = true;
        val = Match_picture("screen.bmp", "pic\\chk\\mouse3.png", maxLoc);
    }

    if(val < 0.9)
    {
        Rand_move_mouse();
        goto letstart;
    }

    //减去截取的鼠标图本身的偏差
    if(first_mouse)
    {
        maxLoc.x -= 0;
        maxLoc.y -= 3;
    }
    else if(first_mouse == false)
    {
        maxLoc.x -= 17;
        maxLoc.y -= 17;
    }
    else if(third_mouse == true)
    {
        maxLoc.x -= 2;
        maxLoc.y -= 3;
    }



    return {maxLoc.x, maxLoc.y};
}


void Mouse_keyboard::click_move(int x, int y)
{




    //转换成游戏内鼠标坐标
    int mouse_x = x * ratio_x;
    int mouse_y = y * ratio_y;

    //加上那个误差
    mouse_x -= rx;
    mouse_y -= ry;

    std::vector<int> mouse = Get_mouse_vec(cur_game_x, cur_game_y, mouse_x, mouse_y);

    for(size_t i = 0; i < mouse.size(); i++)
    {
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, mouse[i]);
        Sleep(7);

        if(i == mouse.size() - 1)
        {
            ::PostMessage(wnd, WM_LBUTTONDOWN, 1,mouse[i]);
            Sleep(50);
            ::PostMessage(wnd, WM_LBUTTONUP, 0,mouse[i]);
        }
    }


    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(150);
}


//传进来的是窗口坐标
//转化成游戏内坐标, 之后用WM_MOUSEMOVE移动
void Mouse_keyboard::click(int x, int y)
{


    POINT now;
    POINT now_game;

    //获得当前鼠标位置
    now = Get_cur_mouse();
    now_game = now;
    std::vector<int> r = Get_mouse_vec(now.x, now.y, x > 600 ? x-50:x, y);
    for(size_t i = 0; i < r.size(); i++)
    {
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, r[i]);
        Sleep(2);

        now_game.x = LOWORD(r[i]);
        now_game.y = HIWORD(r[i]);
    }

    Sleep(100);

    //就一下
    now = Get_cur_mouse();

    //逐渐移动
    //转换成游戏内座标
    int game_x = now.x * ratio_x;
    int game_y = now.y * ratio_y;

    //取得误差
    rx = game_x - now_game.x;
    ry = game_y - now_game.y;
    cur_game_x = now_game.x;
    cur_game_y = now_game.y;

    click_move(x, y);

    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(150);
}

void Mouse_keyboard::input(char* msg)
{

    for(size_t j = 0; j < strlen(msg); j++){
        ::PostMessage(wnd, WM_CHAR, msg[j], 1);
    }

}
