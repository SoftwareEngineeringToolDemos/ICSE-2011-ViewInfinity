

#include <QStringList>

#include "FeatureTreeItem.h"

FeatureTreeItem::FeatureTreeItem(const QList<QVariant> &data, FeatureTreeItem *parent)
{
    parentItem = parent;
    itemData = data;
}

FeatureTreeItem::~FeatureTreeItem()
{
    qDeleteAll(childItems);
}

void FeatureTreeItem::appendChild(FeatureTreeItem *item)
{
    childItems.append(item);
}

FeatureTreeItem *FeatureTreeItem::child(int row)
{
    return childItems.value(row);
}

int FeatureTreeItem::childCount() const
{
    return childItems.count();
}

int FeatureTreeItem::columnCount() const
{
    return itemData.count();
}

QVariant FeatureTreeItem::data(int column) const
{
    return itemData.value(column);
}

FeatureTreeItem *FeatureTreeItem::parent()
{
    return parentItem;
}

int FeatureTreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<FeatureTreeItem*>(this));

    return 0;
}
