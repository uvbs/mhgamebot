#include "mh_mousekeyboard.h"
#include "mh_function.h"
#include "mh_writebmp.h"



int Mouse_keyboard::make_mouse_value(int x, int y)
{
    int v = 0;
    v = x;
    y = y << 16;
    v = v + y;
    return v;
}


Mouse_keyboard::Mouse_keyboard(HWND gamewnd)
{
    wnd = gamewnd;
    ratio_x = (double)590/(double)640;
    ratio_y = (double)450/(double)480;

    rx = 0;
    ry = 0;
    cur_game_x = 0;
    cur_game_y = 0;
    hdc = ::GetDC(gamewnd);

}

void Mouse_keyboard::test1()
{
//    for(int i = 0; i < 480; i++)
//    {
//        click(0, i);
//    }

    //选中屏幕左上角?
    click(0, 0);
    Sleep(5000);

    //右上角
    click(0, 620);
    Sleep(5000);
}

void Mouse_keyboard::rclick(int x, int y)
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
        ::SendMessage(wnd, WM_MOUSEMOVE, 0, mouse[i]);
        //Sleep(7);

        if(i == mouse.size() - 1)
        {
            ::PostMessage(wnd, WM_RBUTTONDOWN, 1,mouse[i]);
            ::PostMessage(wnd, WM_RBUTTONUP, 0,mouse[i]);

            cur_game_x = LOWORD(mouse[i]);
            cur_game_y = HIWORD(mouse[i]);
        }
    }


    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(150);
}

void Mouse_keyboard::rclick()
{
    rclick(cur_game_x, cur_game_y);
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
        MH_printf("点击:%s", image);
        click(point.x, point.y);
    }

}



//const POINT right_point_1 = {606,4};
const POINT chk_point_leftbottom = {40, 456};

//检验指针
//本来是右上角, 这个血值也会导致不能正常校验的..

void Mouse_keyboard::Right_point()
{
    RECT rect;
    ::GetWindowRect(wnd, &rect);

    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;

    bool finded =false;

    //鼠标指针移动到左下角匹配


    //y
    for(int i = 445; i < 480; i++)
    {
        //x
        for(int j = 30; j < 70; j++)
        {
            //int x = width-j;
            int v = make_mouse_value(j, i);
            ::SendMessage(wnd, WM_MOUSEMOVE, 0, v);

            //截图
            WriteBmp("right.bmp", hdc);
            Point maxLoc;
            double maxVal = Match_picture("right.bmp", "rightpoint.png", maxLoc);
            if(maxVal > 0.9)
            {

                //应该的x  -  现实的x
                rx = chk_point_leftbottom.x * ratio_x - j;
                ry = chk_point_leftbottom.y * ratio_y - i;

                cur_game_x = j;
                cur_game_y = i;

                finded = true;
                goto fined;

            }
        }
    }




fined:

    if(finded)
    {
        MH_printf("妈的匹配到了");
    }
    else
    {
        std::runtime_error("指针校准初始化失败了你妹啊");
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

bool Mouse_keyboard::is_match_pic_in_point(const char *image, POINT &point, POINT game_wnd_pt)
{

    RECT rect;
    ::GetClientRect(wnd, &rect);
    rect.left = game_wnd_pt.x;
    rect.top = game_wnd_pt.y;

    //取得屏幕图片
    WriteBmp("screen.bmp", hdc, rect);
    Point maxLoc;
    double maxVal = Match_picture("screen.bmp", image, maxLoc);

    if(maxVal > 0.6)
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

void Mouse_keyboard::Rang_move_mouse()
{
    //取得当前坐标到目的的差值
    int x = rand()%100;
    int y = rand()%100;

    int v = make_mouse_value(x, y);
    ::SendMessage(wnd, WM_MOUSEMOVE, 0, v);

    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(200);
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
}

//匹配屏幕获取当前鼠标位置
POINT Mouse_keyboard::Get_cur_mouse()
{
    WriteBmp("screen.bmp", hdc);
    Point maxLoc;
    bool first_mouse = true;
    double val = Match_picture("screen.bmp", "pic\\chk\\mouse1.png", maxLoc);
    if(val < 0.9)
    {
        first_mouse = false;
        val = Match_picture("screen.bmp", "pic\\chk\\mouse2.png", maxLoc);
    }

    if(val < 0.9)
    {
        click(100, 100);
    }

    //减去截取的鼠标图本身的偏差
    if(first_mouse)
    {
        maxLoc.x -= 0;
        maxLoc.y -= 3;
    }
    else
    {
        maxLoc.x -= 17;
        maxLoc.y -= 17;
    }



    return {maxLoc.x, maxLoc.y};
}


//传进来的是窗口坐标
//转化成游戏内坐标, 之后用WM_MOUSEMOVE移动
void Mouse_keyboard::click(int x, int y)
{

    if(cur_game_x == 0 && cur_game_y == 0 && rx == 0 && ry == 0)
    {
        click_nofix(x, y);
        Rang_move_mouse();
        return;
    }


    //转换成游戏内鼠标坐标
    int mouse_x = x * ratio_x;
    int mouse_y = y * ratio_y;

    //加上那个误差
    mouse_x -= rx;
    mouse_y -= ry;

    std::vector<int> mouse = Get_mouse_vec(cur_game_x, cur_game_y, mouse_x, mouse_y);

    for(size_t i = 0; i < mouse.size(); i++)
    {
        ::SendMessage(wnd, WM_MOUSEMOVE, 0, mouse[i]);
        //Sleep(7);

        if(i == mouse.size() - 1)
        {
            ::PostMessage(wnd, WM_LBUTTONDOWN, 1,mouse[i]);
            ::PostMessage(wnd, WM_LBUTTONUP, 0,mouse[i]);

            cur_game_x = LOWORD(mouse[i]);
            cur_game_y = HIWORD(mouse[i]);
        }
    }


    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(150);
}

void Mouse_keyboard::input(char* msg)
{

    for(size_t j = 0; j < strlen(msg); j++){
        ::PostMessage(wnd, WM_CHAR, msg[j], 1);
    }

}
