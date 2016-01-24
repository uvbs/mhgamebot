#ifndef WNDRECT_H
#define WNDRECT_H

#define GAME_LAUNCHER  "my.exe"
#define GAME_NAME "mhmain.exe"
#define GAME_WND_CLASS L"WSGAME"

#include <windows.h>
#include <string>


//方便注册自己的函数
#define REGLUAFUN(z, x, y) \
    lua_register(z, #x, y); \
    MH_printf("注册LUA函数: %s", #x);




//玩家几种状态
enum Player_status
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
enum Script_type
{
    LEVEL, //升级
    MONEY,  //金币
    JIAOMAI, //频道叫卖
    SMART //智能   根据当前金币数量, 等级, 任务剩余数量选择要做的事情
};

struct SCRIPT_TYPE_DESC
{
    Script_type type;
    std::string str;
};

const SCRIPT_TYPE_DESC Script_type_desc[] =
{
    {Script_type::LEVEL, "等级"},
    {Script_type::MONEY, "金币"},
    {Script_type::JIAOMAI, "叫卖"},
    {Script_type::SMART, "智能"}
};


const POINT point_task = {479, 139};
const RECT rect_position = {45, 27, 106, 40};

//游戏按钮
const POINT point_player = {548, 27};
const POINT point_pet = {434, 21};
const POINT point_map = {75, 54};
const POINT point_player_healher = {610, 10};

const POINT rect_attack = {313,441};   //攻击
const POINT rect_tools = {331,467};     //道具
const POINT rect_tools_close = {554,90};
const POINT rect_tools_grid_1 = {310,156};
const POINT rect_tools_grid_2 = {373,156};
const POINT rect_tools_grid_3 = {419,156};
const POINT rect_give = {365,451};
const POINT rect_jiaoyi = {389,454};
const POINT rect_party = {410,454};
const POINT rect_pet = {422,442};
const POINT rect_task = {444,439};
const POINT rect_friend = {511,442};
const POINT rect_system = {};

const POINT rect_entrygame = {564, 156};
const POINT rect_entrygame_nextstep = {565, 412};
const POINT rect_entrygame_selectsrv = {336, 220};
const POINT rect_entrygame_selectsrv1 = {267, 350};
const POINT rect_entrygame_selectsrv2 = {336, 220};



#endif // WNDRECT_H
