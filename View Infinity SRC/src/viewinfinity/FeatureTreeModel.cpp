

#include <QtGui>

#include "FeatureTreeItem.h"
#include "FeatureTreeModel.h"

//! [0]
FeatureTreeModel::FeatureTreeModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    //rootData << "Title" << "Summary";
	rootData << "Project feature tree";
    rootItem = new FeatureTreeItem(rootData);
    setupModelData(data.split(QString("\n")), rootItem);
}
//! [0]

//! [1]
FeatureTreeModel::~FeatureTreeModel()
{
    delete rootItem;
}
//! [1]

//! [2]
int FeatureTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<FeatureTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}
//! [2]

//! [3]
QVariant FeatureTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    FeatureTreeItem *item = static_cast<FeatureTreeItem*>(index.internalPointer());

    return item->data(index.column());
}
//! [3]

//! [4]
Qt::ItemFlags FeatureTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//! [4]

//! [5]
QVariant FeatureTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}
//! [5]

//! [6]
QModelIndex FeatureTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    FeatureTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FeatureTreeItem*>(parent.internalPointer());

    FeatureTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}
//! [6]

//! [7]
QModelIndex FeatureTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    FeatureTreeItem *childItem = static_cast<FeatureTreeItem*>(index.internalPointer());
    FeatureTreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}
//! [7]

//! [8]
int FeatureTreeModel::rowCount(const QModelIndex &parent) const
{
    FeatureTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FeatureTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}
//! [8]

void FeatureTreeModel::setupModelData(const QStringList &lines, FeatureTreeItem *parent)
{
    QList<FeatureTreeItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].mid(position, 1) != " ")
                break;
            position++;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QList<QVariant> columnData;
            for (int column = 0; column < columnStrings.count(); ++column)
                columnData << columnStrings[column];

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            parents.last()->appendChild(new FeatureTreeItem(columnData, parents.last()));
        }

        number++;
    }
}
