

#include <QStringList>

#include "FileTreeItem.h"

FileTreeItem::FileTreeItem(const QList<QVariant> &data, FileTreeItem *parent)
{
    parentItem = parent;
    itemData = data;
}

FileTreeItem::~FileTreeItem()
{
    qDeleteAll(childItems);
}

void FileTreeItem::appendChild(FileTreeItem *item)
{
    childItems.append(item);
}

FileTreeItem *FileTreeItem::child(int row)
{
    return childItems.value(row);
}

int FileTreeItem::childCount() const
{
    return childItems.count();
}

int FileTreeItem::columnCount() const
{
    return itemData.count();
}

QVariant FileTreeItem::data(int column) const
{
    return itemData.value(column);
}

FileTreeItem *FileTreeItem::parent()
{
    return parentItem;
}

int FileTreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<FileTreeItem*>(this));

    return 0;
}
