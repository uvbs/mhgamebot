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

        keybd_event(input[i],0,0,0);     //����a��
        keybd_event(input[i],0,KEYEVENTF_KEYUP,0);//�ɿ�a��

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

    //MH_printf("ƥ��ͼƬ: %s - %s", img1, img2);

    if(img1 == nullptr || img2 == nullptr)
        std::runtime_error("��������");

    if(wnd == nullptr)
        std::runtime_error("��Ч�ľ��");

    //char img1_fix[40];
    //sprintf(img1_fix, "%d%s", ::GetCurrentThreadId(), img1);
    Mat img_screen = imread(img1);
    Mat img_in = imread(img2);
    Mat result;
    if(img_in.empty() || img_screen.empty()){
        std::runtime_error("ԭͼƬ�ǿ�ͼ");
    }

    //ƥ�䷽ʽ
    int match_method = TM_CCOEFF_NORMED;
    matchTemplate(img_screen, img_in, result, TM_CCOEFF_NORMED);
    //normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// ͨ������ minMaxLoc ��λ��ƥ���λ��
    double minVal;
    double maxVal;
    Point minLoc;
    Point matchLoc;

    cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

    /// ���ڷ��� SQDIFF �� SQDIFF_NORMED, ԽС����ֵ������ߵ�ƥ����. ��������������, ��ֵԽ��ƥ��Խ��
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

    //ȡ����ĻͼƬ
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

        //ȡ����ĻͼƬ
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

    //ȡ�������Աȵ�ͼ
    Sleep(100);
    WriteBmp("screen.bmp", hdc);
    Point maxLoc;
    double maxVal = Match_picture("screen.bmp", image, maxLoc);

    if(maxVal > 0.7)
    {
        Mat img_in = imread(image);
        point.x = maxLoc.x + img_in.cols/2;
        point.y = maxLoc.y + img_in.rows/2;

        //TODO: ���һ��
        return true;
    }
    else
    {
        return false;
    }
}

void Mouse_keyboard::Rand_move_mouse()
{
    //ȡ�õ�ǰ���굽Ŀ�ĵĲ�ֵ
    int x = rand()%300+100;
    int y = rand()%300+100;

    int v = make_mouse_value(x, y);
    ::SendMessage(wnd, WM_MOUSEMOVE, 0, v);

    //��Ҫ�ӳ�һ��, ����ӳ��ǵȴ�����������Ӧ
    Sleep(100);
}


//TODO: ��Ҫһ����ʱ
void Mouse_keyboard::Until_stop_run()
{
    while(1)
    {
        //��һ��
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

//ƥ����Ļ��ȡ��ǰ���λ��
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

    //��ȥ��ȡ�����ͼ�����ƫ��
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




    //ת������Ϸ���������
    int mouse_x = x * ratio_x;
    int mouse_y = y * ratio_y;

    //�����Ǹ����
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


    //��Ҫ�ӳ�һ��, ����ӳ��ǵȴ�����������Ӧ
    Sleep(150);
}


//���������Ǵ�������
//ת������Ϸ������, ֮����WM_MOUSEMOVE�ƶ�
void Mouse_keyboard::click(int x, int y)
{


    POINT now;
    POINT now_game;

    //��õ�ǰ���λ��
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

    //��һ��
    now = Get_cur_mouse();

    //���ƶ�
    //ת������Ϸ������
    int game_x = now.x * ratio_x;
    int game_y = now.y * ratio_y;

    //ȡ�����
    rx = game_x - now_game.x;
    ry = game_y - now_game.y;
    cur_game_x = now_game.x;
    cur_game_y = now_game.y;

    click_move(x, y);

    //��Ҫ�ӳ�һ��, ����ӳ��ǵȴ�����������Ӧ
    Sleep(150);
}

void Mouse_keyboard::input(char* msg)
{

    for(size_t j = 0; j < strlen(msg); j++){
        ::PostMessage(wnd, WM_CHAR, msg[j], 1);
    }

}
