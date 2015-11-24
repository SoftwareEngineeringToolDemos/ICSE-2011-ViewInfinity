

#pragma once
#ifndef FEATURETREEITEM_H
#define FEATURETREEITEM_H

#include <QList>
#include <QVariant>

class FeatureTreeItem
{
public:
    FeatureTreeItem(const QList<QVariant> &data, FeatureTreeItem *parent = 0);
    ~FeatureTreeItem();

    void appendChild(FeatureTreeItem *child);

    FeatureTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    FeatureTreeItem *parent();

private:
    QList<FeatureTreeItem*> childItems;
    QList<QVariant> itemData;
    FeatureTreeItem *parentItem;
};

#endif