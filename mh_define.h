#ifndef WNDRECT_H
#define WNDRECT_H

#define GAME_LAUNCHER  "my.exe"
#define GAME_NAME "mhmain.exe"
#define GAME_WND_CLASS "WSGAME"

#include <windows.h>
#include <string>
#include <stdexcept>

//����ע���Լ��ĺ���
#define REGLUAFUN(z, x, y) \
    lua_register(z, #x, y);


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


//��Ҽ���״̬
enum PLAYER_STATUS
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
    enum Script_type type;
    std::string str;
};

const SCRIPT_TYPE_DESC Script_type_desc[] =
{
    {LEVEL, "�ȼ�"},
    {MONEY, "���"},
    {JIAOMAI, "����"},
    {SMART, "����"}
};


const RECT rect_task = {470, 140, 640, 480};
const RECT rect_position = {45, 27, 106, 40};

//��Ϸ��ť
const POINT point_player = {548, 27};
const POINT point_pet = {434, 21};
const POINT point_map = {75, 54};
const POINT point_player_healher = {610, 10};

const POINT rect_attack = {313,441};   //����
const POINT rect_tools = {331,467};     //����




#endif // WNDRECT_H
