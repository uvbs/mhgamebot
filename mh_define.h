#ifndef WNDRECT_H
#define WNDRECT_H

#define GAME_LAUNCHER  "my.exe"
#define GAME_NAME "mhmain.exe"
#define GAME_WND_CLASS L"WSGAME"

#include <windows.h>
#include <string>


//����ע���Լ��ĺ���
#define REGLUAFUN(z, x, y) \
    lua_register(z, #x, y); \
    MH_printf("ע��LUA����: %s", #x);




//��Ҽ���״̬
enum Player_status
{
    NORMAL,
    COMBAT,
    PAOSHANG, //����
    BIAO,  //��
    NOTIME, //����״̬
    GC,     //����״̬
    ATTACK  //�����ѡ�����״̬
};

//�ű�������, �������ȼ�
//�ýű������ܵ���û������ʱѡ��������
enum Script_type
{
    LEVEL, //����
    MONEY,  //���
    JIAOMAI, //Ƶ������
    SMART //����   ���ݵ�ǰ�������, �ȼ�, ����ʣ������ѡ��Ҫ��������
};

struct SCRIPT_TYPE_DESC
{
    Script_type type;
    std::string str;
};

const SCRIPT_TYPE_DESC Script_type_desc[] =
{
    {Script_type::LEVEL, "�ȼ�"},
    {Script_type::MONEY, "���"},
    {Script_type::JIAOMAI, "����"},
    {Script_type::SMART, "����"}
};


const POINT point_task = {479, 139};
const RECT rect_position = {45, 27, 106, 40};

//��Ϸ��ť
const POINT point_player = {548, 27};
const POINT point_pet = {434, 21};
const POINT point_map = {75, 54};
const POINT point_player_healher = {610, 10};

const POINT rect_attack = {313,441};   //����
const POINT rect_tools = {331,467};     //����
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
