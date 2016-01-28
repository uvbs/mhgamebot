#include "mh_mousekeyboard.h"
#include "mh_config.h"
#include "mh_gamescript.h"


#include <boost/lexical_cast.hpp>
#include <windows.h>
#include <string>
#include <shlwapi.h>
#include <stdlib.h>
#include <stdio.h>




int Mouse_keyboard::make_mouse_value(int x, int y)
{
    int v = 0;
    v = x;
    y = y << 16;
    v = v + y;

    return v;
}

std::vector<uchar> Mouse_keyboard::Get_screen_data()
{
    RECT rect = {};
    ::GetClientRect(wnd, &rect);

    //mhprintf("GetClientRect: %d, %d, %d, %d", client.left, client.top, client.right, client.bottom);

    return Get_screen_data(rect);
}

void Mouse_keyboard::input_event(const char* input)
{
    for(size_t i = 0; i < strlen(input); i++)
    {

        keybd_event(input[i],0,0,0);     //按下a键
        keybd_event(input[i],0,KEYEVENTF_KEYUP,0);//松开a键

    }

}

void Mouse_keyboard::Regist_lua_fun(lua_State *lua_status)
{
    REGLUAFUN(lua_status, "点击图片", [](lua_State* L)->int{
        const char* image = lua_tostring(L, 1);
       GameScriper::Get_instance(L)->Get_mouse()->click(image);
        return 0;
    });

    REGLUAFUN(lua_status, "等待停止奔跑", [](lua_State* L)->int{
        GameScriper::Get_instance(L)->Get_mouse()->Until_stop_run();
        return 0;
    });

    REGLUAFUN(lua_status, "装备物品", [](lua_State* L)->int{
       const char* str = lua_tostring(L, 1);
       GameScriper::Get_instance(L)->Get_mouse()->click(rect_tools.x, rect_tools.y);
       GameScriper::Get_instance(L)->Get_mouse()->click(str);
       return 0;
    });

    REGLUAFUN(lua_status, "走向小地图", [](lua_State* L)->int{
        const char* name = lua_tostring(L, 1);
        GameScriper::Get_instance(L)->Get_mouse()->click(point_map.x, point_map.y);
        GameScriper::Get_instance(L)->Get_mouse()->click(name);
        return 0;
    });

    REGLUAFUN(lua_status, "点击对话框中的", [](lua_State* L)->int{
        const char* img = lua_tostring(L, 1);
        //前置窗口
        ::BringWindowToTop(GameScriper::Get_instance(L)->Get_mouse()->get_game_wnd());
        GameScriper::Get_instance(L)->Get_mouse()->click(img);
        return 0;
    });

}

void Mouse_keyboard::mhprintf(const char *msg, ...)
{
    static bool can_printf = true;
tryagain:
    if(can_printf == true)
    {

        can_printf = false;

        //需要一个互斥
        //TODO:
        printf("%s: ", player_name.c_str());

        va_list va;
        va_start(va, msg);
        vprintf(msg, va);
        va_end(va);

        printf("\n");

        can_printf = true;
    }
    else
    {
        Sleep(50);
        goto tryagain;
    }
}

Mouse_keyboard::Mouse_keyboard(HWND gamewnd, int id):
    player_name(std::string("窗口")+boost::lexical_cast<std::string>(id))
{
    wnd = gamewnd;
    hdc = ::GetDC(gamewnd);
    script_id = id;

    ratio_x = (double)590/(double)640;
    ratio_y = (double)450/(double)480;

    rx = 0;
    ry = 0;
    cur_game_x = 0;
    cur_game_y = 0;
}


//也用来取消攻击状态
void Mouse_keyboard::rclick(int x, int y)
{
    //TODO:
    click(x, y, false);
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
        char buf[50];
        sprintf(buf, "rclick error! %s!", image);
        //mhprintf(buf);
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

double Mouse_keyboard::Match_picture(const std::vector<uchar> &img1, const char* img2, cv::Point &maxLoc)
{

    if(::PathFileExistsA(img2) == FALSE)
    {
        char buf[50];
        sprintf(buf, "图片%s 不存在", img2);
        mhprintf(buf);
        throw std::runtime_error("图片不存在");
    }


    if(wnd == nullptr)
        throw std::runtime_error("无效的句柄");


    Mat matchscreen = cv::imdecode(img1, IMREAD_COLOR);
    Mat matchpic = cv::imread(img2, IMREAD_COLOR);
    Mat result;

    //mhprintf("%matchpic %d %d", matchpic.size().height, matchpic.size().width);
    //mhprintf("%matchscreen %d %d", matchscreen.size().height, matchscreen.size().width);

    //匹配方式
    int match_method = TM_CCOEFF_NORMED;
    cv::matchTemplate(matchscreen, matchpic, result, match_method);
    //normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// 通过函数 minMaxLoc 定位最匹配的位置
    double minVal;
    double maxVal;
    cv::Point minLoc;
    cv::Point matchLoc;

    cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

    /// 对于方法 SQDIFF 和 SQDIFF_NORMED, 越小的数值代表更高的匹配结果. 而对于其他方法, 数值越大匹配越好
    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }
    else{
        matchLoc = maxLoc;
    }

    return maxVal;
}

double Mouse_keyboard::Match_picture(const std::vector<uchar>& img1, const std::vector<uchar>& img2, cv::Point &maxLoc)
{

    if(wnd == nullptr)
        throw std::runtime_error("无效的句柄");


    Mat result;

    //匹配方式
    int match_method = TM_CCOEFF_NORMED;
    cv::Mat matscreen = cv::imdecode(img1, IMREAD_COLOR);
    cv::Mat matchpic = cv::imdecode(img2, IMREAD_COLOR);

    matchTemplate(matscreen, matchpic, result, TM_CCOEFF_NORMED);
    //normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// 通过函数 minMaxLoc 定位最匹配的位置
    double minVal;
    double maxVal;
    cv::Point minLoc;
    cv::Point matchLoc;

    cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

    /// 对于方法 SQDIFF 和 SQDIFF_NORMED, 越小的数值代表更高的匹配结果. 而对于其他方法, 数值越大匹配越好
    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }
    else{
        matchLoc = maxLoc;
    }

    return maxVal;
}


//这种匹配部分主要用来避免一些误差
//
bool Mouse_keyboard::is_match_pic_in_rect(const char *image, POINT &point, const RECT &rect)
{
    //取得屏幕图片
    std::vector<uchar>&& screen_buf = Get_screen_data(rect);


    cv::Point maxLoc;
    double maxVal = Match_picture(screen_buf, image, maxLoc);

    if(maxVal > 0.65)
    {
        Mat img_in = cv::imread(image);
        point.x = maxLoc.x + img_in.cols/2 + rect.left;
        point.y = maxLoc.y + img_in.rows/2 + rect.top;
        return true;
    }
    else
    {
        return false;
    }
}

//POINT 返回匹配到的图片位置
bool Mouse_keyboard::is_match_pic_in_screen(const char *image, POINT &point)
{
    //两个对比的图
    std::vector<uchar>&& screen_buf = Get_screen_data();

    cv::Point maxLoc;
    double maxVal = Match_picture(screen_buf, image, maxLoc);
    //mhprintf("匹配: %s %f", image, maxVal);
    if(maxVal > 0.65)
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

//对话框点击
void Mouse_keyboard::Dialog_click(const char* img)
{
    //::SetActiveWindow(wnd);
    ::SetForegroundWindow(wnd);
    ::BringWindowToTop(wnd);
    click(img);
    return;
}

void Mouse_keyboard::Rand_move_mouse()
{
    //取得当前坐标到目的的差值
    int x = rand()%200+100;
    int y = rand()%200+100;

    int v = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, v);

    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(1000);
}


//TODO: 需要一个超时
void Mouse_keyboard::Until_stop_run()
{
    while(1)
    {
        //来一张
        //TODO:
        std::vector<uchar>&& pos1 = Get_screen_data(rect_position);
        Sleep(2000);
        std::vector<uchar>&& pos2 = Get_screen_data(rect_position);

        cv::Point maxLoc;
        double isMatch = Match_picture(pos1, pos2, maxLoc);
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

    if(x2 > 640 || y2 > 480)
    {
        mhprintf("x: %d, y: %d", x2, y2);
        throw exception_xy("目的坐标异常");
    }


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
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, v);
    ::PostMessage(wnd, WM_LBUTTONDOWN, 1,v);
    ::PostMessage(wnd, WM_LBUTTONUP, 0,v);
    Sleep(1000);
    Rand_move_mouse();
}

//匹配屏幕获取当前鼠标位置
POINT Mouse_keyboard::Get_cur_mouse()
{

letstart:
    std::vector<uchar>&& screen_buf = Get_screen_data();
    cv::Point maxLoc;
    bool first_mouse = true;
    bool third_mouse = false;
    bool second_mouse = false;

    double val = Match_picture(screen_buf, "pic\\chk\\mouse1.png", maxLoc);
    if(val < 0.9)
    {
        first_mouse = false;
        val = Match_picture(screen_buf, "pic\\chk\\mouse2.png", maxLoc);
    }

    if(val < 0.9)
    {
        second_mouse = true;
        val = Match_picture(screen_buf, "pic\\chk\\mouse3.png", maxLoc);
    }

    if(val < 0.9)
    {
        third_mouse = true;
        val = Match_picture(screen_buf, "pic\\chk\\mouse4.png", maxLoc);
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
    else if(second_mouse == true)
    {
        maxLoc.x -= 2;
        maxLoc.y -= 3;
    }
    else if(third_mouse == true)
    {
        maxLoc.x -= 30;
    }



    return {maxLoc.x, maxLoc.y};
}


void Mouse_keyboard::click_move(int x, int y, bool lbutton)
{

    if(x > 640 || y > 480)
        throw std::runtime_error("click_move() 目的坐标异常");

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
        Sleep(rand()%7 + 1);

        if(i == mouse.size() - 1)
        {
            if(lbutton)
                ::PostMessage(wnd, WM_LBUTTONDOWN, 1,mouse[i]);
            else
                ::PostMessage(wnd, WM_RBUTTONDOWN, 1, mouse[i]);

            Sleep(10);

            if(lbutton)
                ::PostMessage(wnd, WM_LBUTTONUP, 0,mouse[i]);
            else
                ::PostMessage(wnd, WM_RBUTTONUP, 0, mouse[i]);
        }
    }


    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(1000);
}


//传进来的是窗口坐标
//转化成游戏内坐标, 之后用WM_MOUSEMOVE移动
void Mouse_keyboard::click(int x, int y, bool lbutton)
{


    POINT now;
    POINT now_game;

    //获得当前鼠标位置
    now = Get_cur_mouse();
    now_game = now;
    std::vector<int> r = Get_mouse_vec(now.x, now.y, x > 550 ? x-70:x, y > 450 ? y - 70: y);
    for(size_t i = 0; i < r.size(); i++)
    {
        ::PostMessage(wnd, WM_MOUSEMOVE, 0, r[i]);
        Sleep(rand()%3 + 1);

        now_game.x = LOWORD(r[i]);
        now_game.y = HIWORD(r[i]);
    }

    Sleep(rand()%1000+500);

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

    click_move(x, y, lbutton);
}

void Mouse_keyboard::input(char* msg)
{

    for(size_t j = 0; j < strlen(msg); j++){
        ::PostMessage(wnd, WM_CHAR, msg[j], 1);
        Sleep(10);
    }

}


void Mouse_keyboard::set_player_name(std::__cxx11::string name)
{
    player_name = name;
}



//返回屏幕内存数据
std::vector<uchar> Mouse_keyboard::Get_screen_data(const RECT &rcDC)
{

    HDC hdcWindow;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;

    try
    {

        // Retrieve the handle to a display device context for the client
        // area of the window.
        hdcWindow = GetDC(wnd);

        // Create a compatible DC which is used in a BitBlt from the window DC
        hdcMemDC = CreateCompatibleDC(hdcWindow);

        if(!hdcMemDC)
        {
            throw std::runtime_error("CreateCompatibleDC has failed");
        }

        // Get the client area for size calculation
        RECT rcClient;
        GetClientRect(wnd, &rcClient);

        //This is the best stretch mode
        SetStretchBltMode(hdcWindow,HALFTONE);

        //The source DC is the entire screen and the destination DC is the current window (HWND)
        if(!StretchBlt(hdcMemDC,
                       0,0,
                       rcClient.right, rcClient.bottom,
                       hdcWindow,
                       0,0,
                       GetSystemMetrics (SM_CXSCREEN),
                       GetSystemMetrics (SM_CYSCREEN),
                       SRCCOPY))
        {
            throw std::runtime_error("StretchBlt has failed");
        }

        // Create a compatible bitmap from the Window DC
        hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);

        if(!hbmScreen)
        {
            throw std::runtime_error("CreateCompatibleBitmap Failed");
        }

        // Select the compatible bitmap into the compatible memory DC.
        SelectObject(hdcMemDC,hbmScreen);

        // Bit block transfer into our compatible memory DC.
        if(!BitBlt(hdcMemDC,
                   0,0,
                   rcClient.right-rcClient.left, rcClient.bottom-rcClient.top,
                   hdcWindow,
                   0,0,
                   SRCCOPY))
        {
            throw std::runtime_error("BitBlt has failed");
        }

        // Get the BITMAP from the HBITMAP
        GetObject(hbmScreen,sizeof(BITMAP),&bmpScreen);

        BITMAPFILEHEADER   bmfHeader;
        BITMAPINFOHEADER   bi;

        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = bmpScreen.bmWidth;
        bi.biHeight = bmpScreen.bmHeight;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

        // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
        // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
        // have greater overhead than HeapAlloc.
        HANDLE hDIB = GlobalAlloc(GHND,dwBmpSize);
        char *lpbitmap = (char *)GlobalLock(hDIB);

        // Gets the "bits" from the bitmap and copies them into a buffer
        // which is pointed to by lpbitmap.
        GetDIBits(hdcWindow, hbmScreen, 0,
                  (UINT)bmpScreen.bmHeight,
                  lpbitmap,
                  (BITMAPINFO *)&bi, DIB_RGB_COLORS);



        // Add the size of the headers to the size of the bitmap to get the total file size
        DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        //Offset to where the actual bitmap bits start.
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

        //Size of the file
        bmfHeader.bfSize = dwSizeofDIB;

        //bfType must always be BM for Bitmaps
        bmfHeader.bfType = 0x4D42; //BM


        imgbuf.resize(bmfHeader.bfSize);

        //已经写入大小
        int readed = 0;
        memcpy(&imgbuf[0], &bmfHeader, sizeof(BITMAPFILEHEADER));
        readed += sizeof(BITMAPFILEHEADER);

        memcpy(&imgbuf[readed], &bi, sizeof(BITMAPINFOHEADER));
        readed += sizeof(BITMAPINFOHEADER);

        memcpy(&imgbuf[readed], lpbitmap, dwBmpSize);


        //Unlock and Free the DIB from the heap
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);

        //Clean up
    }catch(...)
    {

    }

    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(wnd,hdcWindow);


    return imgbuf;
}


//写入图片到一个文件
bool Mouse_keyboard::Write_bmp_to_file(std::__cxx11::string file, const RECT &rect)
{
    throw std::runtime_error("Write_bmp_to_file() no imp");
}


