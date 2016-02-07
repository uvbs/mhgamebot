#include "ScriptTableModel.h"



ScriptTableModel::ScriptTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{}

ScriptTableModel::~ScriptTableModel()
{}

ScriptItem *ScriptTableModel::getItem(const int &row) const
{
    return items.at(row);
}

int ScriptTableModel::rowCount(const QModelIndex &) const
{
    return items.count();
}

int ScriptTableModel::columnCount(const QModelIndex &) const
{
    return 6;
}

QVariant ScriptTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    
    return 1;
}

QVariant ScriptTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
        return QVariant();
    }
  
    
    switch (section) {
    case 0:
        return QVariant(QString::fromLocal8Bit("机器编号"));
    case 1:
        return QVariant(QString::fromLocal8Bit("窗口数量"));
    case 2:
        return QVariant(QString::fromLocal8Bit("状态"));
    case 3:
        return QVariant(QString::fromLocal8Bit("脚本类型"));
    case 4:
        return QVariant(QString::fromLocal8Bit("脚本"));
    case 5:
        return QVariant(QString::fromLocal8Bit("运行时间"));
    default:
        return QVariant();
    }
}

QModelIndex ScriptTableModel::index(int row, int column, const QModelIndex &) const
{
    if (row < 0 || row >= items.size()) {
        return QModelIndex();
    } else {
        ScriptItem* item = items.at(row);
        return createIndex(row, column, item);//column is ignored (all columns have the same effect)
    }
}

bool ScriptTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0 || count + row > items.count()) {
        return false;
    }

    return true;
}



