

#pragma once
#ifndef FILETREEITEM_H
#define FILETREEITEM_H

#include <QList>
#include <QVariant>

class FileTreeItem
{
public:
    FileTreeItem(const QList<QVariant> &data, FileTreeItem *parent = 0);
    ~FileTreeItem();

    void appendChild(FileTreeItem *child);

    FileTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    FileTreeItem *parent();

private:
    QList<FileTreeItem*> childItems;
    QList<QVariant> itemData;
    FileTreeItem *parentItem;
};

#endif