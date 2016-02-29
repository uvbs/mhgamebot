#include "config.h"

#include <QSettings>
#include <string>
#include <set>
#include <exception>
#include <iostream>
#include <QTextCodec>


GameConfig* GameConfig::_inst = nullptr;

GameConfig::GameConfig()
{
    startgame_counts = 1;
    auto_skipgc = true;
    auto_startgame = true;

    _inst = this;
}

GameConfig::~GameConfig()
{

}

void GameConfig::load()
{
    QSettings settings("mh.ini", QSettings::IniFormat);
    auto_skipgc = settings.value("Option/auto skip gc").toBool();
    auto_startgame = settings.value("Option/auto start game").toBool();
    startgame_counts = settings.value("Option/start game counts").toInt();
    helperaddr = settings.value("Option/helper addr").toString();
    recent_scripts = settings.value("Option/recent scripts").toStringList();
}


void GameConfig::save()
{
    QSettings settings("mh.ini", QSettings::IniFormat);
    settings.beginGroup("Option");
    settings.setValue("auto skip gc", auto_skipgc);
    settings.setValue("auto start game", auto_startgame);
    settings.setValue("start game counts", startgame_counts);
    settings.setValue("helper addr", helperaddr);
    settings.setValue("recent scripts", recent_scripts);
    settings.endGroup();
}

void GameConfig::add(const QString &newfile)
{
    if(recent_scripts.size() > 5){
        recent_scripts.removeFirst();
    }

    recent_scripts.push_back(newfile);
}

QString GameConfig::get_last_script()
{
    int size = recent_scripts.size();
    if(size == 0)
        return "";

    return recent_scripts.at(size - 1);
}
