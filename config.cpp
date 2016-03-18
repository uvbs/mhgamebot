#include "config.h"

#include <QSettings>
#include <string>
#include <set>
#include <exception>
#include <iostream>
#include <QTextCodec>
#include <QDebug>



GameConfig* GameConfig::_inst = nullptr;

GameConfig::GameConfig()
{
    _inst = this;
}

GameConfig::~GameConfig()
{

}

void GameConfig::load()
{
    QSettings settings("mh.ini", QSettings::IniFormat);
    settings.beginGroup("Option");
    auto_skipgc = settings.value("auto skip gc", true).toBool();
    auto_startgame = settings.value("auto start game", false).toBool();
    startgame_counts = settings.value("start game counts", 1).toInt();
    helperaddr = settings.value("helper addr", "127.0.0.1").toString();
    helperport = settings.value("helper port", "5678").toString();
    recent_scripts = settings.value("recent scripts").toStringList();
    auto_run = settings.value("auto run", true).toBool();
    auto_hide = settings.value("auto hide", true).toBool();
    settings.endGroup();
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
    settings.setValue("helper port", helperport);
    settings.setValue("auto run", auto_run);
    settings.setValue("auto hide", auto_hide);
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

    return recent_scripts.at(recent_scripts.size() -1);
}
