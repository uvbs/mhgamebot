#ifndef WNDRECT_H
#define WNDRECT_H

#include <windows.h>
#include <string>
#include <stdexcept>


#define GAME_LAUNCHER  "my.exe"
#define GAME_NAME "mhmain.exe"
#define GAME_WND_CLASS "WSGAME"
#define MHCHATWNDCLASS    "XYWZ_CHAT"



//默认匹配程度
#define DEFAULT_THERSHOLD 7


//方便注册自己的函数
#define REGLUAFUN(z, x, y) \
    lua_register(z, x, y);
//    mhprintf(LOG_NORMAL,"注册函数 %s", x);


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
    ATTACK  //点击了选中鼠标状态
};

//脚本的类型, 设置优先级
//让脚本能智能的在没有任务时选择事情做
enum SCRIPT_TYPE
{
    LEVEL, //升级
    MONEY,  //金币
    JIAOMAI, //频道叫卖
    DAILY, //日常
    SMART //智能   根据当前金币数量, 等级, 任务剩余数量选择要做的事情
};

struct SCRIPT_TYPE_DESC
{
    enum SCRIPT_TYPE type;
    std::string str;
};

const SCRIPT_TYPE_DESC script_type_desc[] =
{
    {LEVEL, "等级"},
    {MONEY, "金币"},
    {JIAOMAI, "叫卖"},
    {DAILY, "日常"},
    {SMART, "智能"}
};

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
const RECT rect_task = {470, 140, SCREEN_WIDTH, SCREEN_HEIGHT - 100};
const RECT rect_position = {27, 27, 120, 41};
const RECT rect_game = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};


//游戏按钮
const POINT point_player = {548, 27};
const POINT point_pet = {434, 21};
const POINT point_map = {75, 54};
const POINT point_player_healher = {610, 10};

const POINT rect_attack = {313,441};   //攻击
const POINT rect_tools = {331,467};     //道具




#endif // WNDRECT_H
