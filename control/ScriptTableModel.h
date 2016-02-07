#ifndef SCRIPTTABLEMODEL_H
#define SCRIPTTABLEMODEL_H

#include <QAbstractTableModel>
#include "scriptitem.h"

class ScriptTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ScriptTableModel(QObject *parent = nullptr);
    ~ScriptTableModel();

    ScriptItem *getItem(const int &row) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
 

signals:
    void message(const QString &);
    void rowStatusChanged(int row, bool running);

private:
    QList<ScriptItem*> items;
};

#endif // SCRIPTTABLEMODEL_H
