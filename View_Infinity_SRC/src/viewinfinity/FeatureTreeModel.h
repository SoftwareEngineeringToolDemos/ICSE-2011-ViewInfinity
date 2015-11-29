

#pragma once

#ifndef FEATURETREEMODEL_H
#define FEATURETREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class FeatureTreeItem;

//! [0]
class FeatureTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    FeatureTreeModel(const QString &data, QObject *parent = 0);
    ~FeatureTreeModel();

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
    void setupModelData(const QStringList &lines, FeatureTreeItem *parent);

    FeatureTreeItem *rootItem;
};
//! [0]

#endif