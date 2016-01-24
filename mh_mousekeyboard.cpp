#include "mh_mousekeyboard.h"
#include "mh_function.h"

#include <boost/lexical_cast.hpp>
#include <string>
#include <shlwapi.h>
#include <stdlib.h>
#include <stdio.h>

Mouse_keyboard* Mouse_keyboard::_inst = nullptr;
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

void Mouse_keyboard::Regist_lua_fun(lua_State *lua_status)
{
    REGLUAFUN(lua_status, "点击图片", [](lua_State* L)->int{
        const char* image = lua_tostring(L, 1);
        Mouse_keyboard::GetInstance()->click(image);
        return 0;
    });

    REGLUAFUN(lua_status, "等待停止奔跑", [](lua_State* L)->int{
        Mouse_keyboard::GetInstance()->Until_stop_run();
    });

    REGLUAFUN(lua_status, "装备物品", [](lua_State* L)->int{
       const char* str = lua_tostring(L, 1);
       Mouse_keyboard::GetInstance()->click(rect_tools.x, rect_tools.y);
       Mouse_keyboard::GetInstance()->click(str);
       return 0;
    });

    REGLUAFUN(lua_status, "走向小地图", [](lua_State* L)->int{
        const char* name = lua_tostring(L, 1);
        Mouse_keyboard::GetInstance()->click(point_map.x, point_map.y);
        Mouse_keyboard::GetInstance()->click(name);
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
        Sleep(150);
        goto tryagain;
    }
}

Mouse_keyboard::Mouse_keyboard(HWND gamewnd, int id)
{
    wnd = gamewnd;
    hdc = ::GetDC(gamewnd);
    _inst = this;
    script_id = id;
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


bool Mouse_keyboard::is_match_pic_in_screen(const char *image, bool screen_exisit)
{
    POINT pt;
    return is_match_pic_in_screen(image, pt, screen_exisit);
}


double Mouse_keyboard::Match_picture(std::string img1, const char* img2, cv::Point &maxLoc)
{

    img1.insert(0, boost::lexical_cast<std::string>(script_id));

    //MH_printf("匹配图片: %s - %s", img1, img2);
    if(::PathFileExistsA(img1.c_str()) == FALSE)
    {
        char buf[30];
        sprintf(buf, "图片%s 不存在", img1);
        mhprintf(buf);
        std::runtime_error("图片不存在");
    }

    if(::PathFileExistsA(img2) == FALSE)
    {
        char buf[30];
        sprintf(buf, "图片%s 不存在", img2);
        mhprintf(buf);
        std::runtime_error("图片不存在");
    }


    if(wnd == nullptr)
        std::runtime_error("无效的句柄");


    Mat img_screen = imread(img1.c_str());
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

    cv::Point maxLoc;
    double maxVal = Match_picture("screen.bmp", image, maxLoc);

    if(maxVal > 0.65)
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

bool Mouse_keyboard::is_match_pic_in_screen(const char *image, POINT &point, bool screen_exisit)
{
    //两个对比的图

    if(screen_exisit == false)
    {
        WriteBmp("screen.bmp", hdc);
    }

    cv::Point maxLoc;
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
    int x = rand()%200+100;
    int y = rand()%200+100;

    int v = make_mouse_value(x, y);
    ::PostMessage(wnd, WM_MOUSEMOVE, 0, v);

    //需要延迟一下, 这个延迟是等待界面做出响应
    Sleep(500);
}


//TODO: 需要一个超时
void Mouse_keyboard::Until_stop_run()
{
    std::string img2 = "position2.bmp";
    img2.insert(0, boost::lexical_cast<std::string>(script_id));
    while(1)
    {
        //来一张
        WriteBmp("position1.bmp", hdc, rect_position);
        Sleep(3000);
        WriteBmp("position2.bmp", hdc, rect_position);

        cv::Point maxLoc;
        double isMatch = Match_picture("position1.bmp", img2.c_str(), maxLoc);
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
        std::runtime_error("目的坐标异常");
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
    Sleep(200);
    Rand_move_mouse();
}

//匹配屏幕获取当前鼠标位置
POINT Mouse_keyboard::Get_cur_mouse()
{

letstart:
    WriteBmp("screen.bmp", hdc);
    cv::Point maxLoc;
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


void Mouse_keyboard::click_move(int x, int y, bool lbutton)
{

    if(x > 640 || y > 480)
        std::runtime_error("click_move() 目的坐标异常");

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
        Sleep(10);

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
        Sleep(2);

        now_game.x = LOWORD(r[i]);
        now_game.y = HIWORD(r[i]);
    }

    Sleep(1000);

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


BOOL Mouse_keyboard::WriteBmp(std::string strFile,const std::vector<BYTE> &vtData,const SIZE &sizeImg)
{

    BITMAPINFOHEADER bmInfoHeader = {0};
    bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmInfoHeader.biWidth = sizeImg.cx;
    bmInfoHeader.biHeight = sizeImg.cy;
    bmInfoHeader.biPlanes = 1;
    bmInfoHeader.biBitCount = 24;

    //Bimap file header in order to write bmp file
    BITMAPFILEHEADER bmFileHeader = {0};
    bmFileHeader.bfType = 0x4d42;  //bmp
    bmFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmFileHeader.bfSize = bmFileHeader.bfOffBits + ((bmInfoHeader.biWidth * bmInfoHeader.biHeight) * 3); ///3=(24 / 8)


    strFile.insert(0, boost::lexical_cast<std::string>(script_id));
    HANDLE hFile = CreateFileA(strFile.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    DWORD dwWrite = 0;
    WriteFile(hFile,&bmFileHeader,sizeof(BITMAPFILEHEADER),&dwWrite,NULL);
    WriteFile(hFile,&bmInfoHeader, sizeof(BITMAPINFOHEADER),&dwWrite,NULL);
    WriteFile(hFile,&vtData[0], vtData.size(),&dwWrite,NULL);

    CloseHandle(hFile);

    return TRUE;
}

void Mouse_keyboard::set_player_name(std::__cxx11::string name)
{
    player_name = name;
}

BOOL Mouse_keyboard::WriteBmp(std::string strFile,HDC hdc)
{
    int iWidth = 640;
    int iHeight = 480;
    RECT rcDC = {0,0,iWidth,iHeight};

    return WriteBmp(strFile,hdc,rcDC);
}

BOOL Mouse_keyboard::WriteBmp(std::string strFile,HDC hdc,const RECT &rcDC)
{

    //根据线程id重新命名, 避免冲突
    //char buf[40];
    //sprintf(buf, "%d%s", ::GetCurrentThreadId(), strFile.c_str());

    BOOL bRes = FALSE;
    BITMAPINFO bmpInfo = {0};
    BYTE *pData = NULL;
    SIZE sizeImg = {0};
    HBITMAP hBmp = NULL;
    std::vector<BYTE> vtData;
    HGDIOBJ hOldObj = NULL;
    HDC hdcMem = NULL;

    //Initilaize the bitmap information
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = rcDC.right - rcDC.left;
    bmpInfo.bmiHeader.biHeight = rcDC.bottom - rcDC.top;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 24;

    //Create the compatible DC to get the data
    hdcMem = CreateCompatibleDC(hdc);
    if(hdcMem == NULL)
    {
        goto EXIT;
    }

    //Get the data from the memory DC
    hBmp = CreateDIBSection(hdcMem, &bmpInfo,DIB_RGB_COLORS,reinterpret_cast<VOID **>(&pData),NULL,0);
    if(hBmp == NULL)
    {
        goto EXIT;
    }
    hOldObj = SelectObject(hdcMem, hBmp);

    //Draw to the memory DC
    sizeImg.cx = bmpInfo.bmiHeader.biWidth;
    sizeImg.cy = bmpInfo.bmiHeader.biHeight;
    StretchBlt(hdcMem,
                0,
                0,
                sizeImg.cx,
                sizeImg.cy,
                hdc,
                rcDC.left,
                rcDC.top,
                rcDC.right - rcDC.left + 1,
                rcDC.bottom - rcDC.top + 1,
                SRCCOPY);


    vtData.resize(sizeImg.cx * sizeImg.cy * 3);
    memcpy(&vtData[0], pData, vtData.size());
    bRes = WriteBmp(strFile.c_str(), vtData, sizeImg);

    SelectObject(hdcMem, hOldObj);


EXIT:
    if(hBmp != NULL)
    {
        DeleteObject(hBmp);
    }

    if(hdcMem != NULL)
    {
        DeleteDC(hdcMem);
    }

    return bRes;
}


