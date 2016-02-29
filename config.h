#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include <Windows.h>
#include <set>
#include <unordered_set>
#include "define.h"

#include <QString>
#include <QStringList>
#include <assert.h>

//保存游戏配置
struct GameConfig
{
    GameConfig();
    ~GameConfig();

    static GameConfig* instance(){
        assert(_inst != nullptr);
        return _inst;
    }

    QStringList recent_scripts;  // 最近使用的脚本
    void load();
    void save();
    void add(const QString &newfile);

    QString get_last_script();


    static GameConfig* _inst;
    int startgame_counts;       //自动开启客户端数量
    bool auto_startgame;    //开始时没有窗口就自动启动新游戏
    bool auto_skipgc;       //自动跳过动画
    QString helperaddr;   //人工帮助地址
};







#endif // FUNCTION_H
