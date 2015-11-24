

#pragma once

#ifndef FILETREEMODEL_H
#define FILETREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class FileTreeItem;

//! [0]
class FileTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    FileTreeModel(const QString &data, QObject *parent = 0);
    ~FileTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    void setupModelData(const QStringList &lines, FileTreeItem *parent);

    FileTreeItem *rootItem;
};
//! [0]

#endif