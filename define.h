#ifndef WNDRECT_H
#define WNDRECT_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <stdexcept>


#define GAME_LAUNCHER  "my.exe"
#define GAME_NAME "mhmain.exe"
#define GAME_WND_CLASS "WSGAME"
#define MHCHATWNDCLASS    "XYWZ_CHAT"
#define WAIT_TIMES  50		//等待次数, 对待新服卡的问题
#define WAIT_NORMAL 300			//平常等待的时辰
#define WAIT_POSTMSG 100

//默认匹配程度
#define DEFAULT_THERSHOLD 0.7


//方便注册自己的函数
#define REGLUAFUN(z, x, y) \
    lua_register(z, x, y);
//    mhprintf(LOG_NORMAL,"注册函数 %s", x);

#define REGLUADATA(z, x, y) \
    lua_pushinteger(z, x); \
    lua_setglobal(z, y);


struct GAME_WND_INFO
{
    HWND wnd;
    DWORD tid;
    DWORD pid;
};


class exception_status: public std::runtime_error
{
public:
    exception_status(std::string str):
        std::runtime_error(str){}
};

class exception_xy: public std::runtime_error
{
public:
    exception_xy(std::string str):
        std::runtime_error(str){}
};


//玩家几种状态
enum PLAYER_STATUS
{
    NORMAL,
    COMBAT,
    PAOSHANG, //跑商
    BIAO,  //镖
    NOTIME, //体验状态
    GC,     //动画状态
    ATTACK,  //点击了选中鼠标状态
    UNKNOW
};

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480


const RECT rect_task = {470, 155, SCREEN_WIDTH, SCREEN_HEIGHT/2+50};
const RECT rect_position = {27, 27, 120, 41};
const RECT rect_game = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
const RECT rect_left_wnd = {0, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT};
const RECT rect_health = {570, 0 , SCREEN_WIDTH, 14};  //玩家血条和宠物血条的区域

//游戏按钮
const POINT point_player = {548, 27};
const POINT point_pet = {434, 21};
const POINT point_map = {75, 54};
const POINT point_player_healher = {610, 10};

const POINT rect_attack = {313,441};   //攻击
const POINT rect_tools = {331,467};     //道具



enum STD_COLOR {
    DARKBLUE = 1,
    DARKGREEN,
    DARKTEAL,
    DARKRED,
    DARKPINK,
    DARKYELLOW,
    GRAY,
    DARKGRAY,
    BLUE,
    GREEN,
    TEAL,
    RED,
    PINK,
    YELLOW,
    WHITE
};

enum LOG_TYPE{
    LOG_WARNING,  //警告
    LOG_NORMAL, //正常
    LOG_ERROR,  //错误
    LOG_INFO     //信息
};


#endif // WNDRECT_H
