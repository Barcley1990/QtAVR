#include "projecttree.h"


ProjectTree::ProjectTree(const QVector<QVariant> &data, ProjectTree *parent)
{
    parentItem = parent;
    itemData = data;
}
ProjectTree::~ProjectTree()
{
    qDeleteAll(childItems);
}
ProjectTree *ProjectTree::parent()
{
    return parentItem;
}
ProjectTree *ProjectTree::child(int number)
{
    return childItems.value(number);
}
int ProjectTree::childCount() const
{
    return childItems.count();
}
int ProjectTree::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ProjectTree*>(this));

    return 0;
}
QVariant ProjectTree::data(int column) const
{
    return itemData.value(column);
}
bool ProjectTree::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}
bool ProjectTree::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        ProjectTree *item = new ProjectTree(data, this);
        childItems.insert(position, item);
    }

    return true;
}
bool ProjectTree::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}
bool ProjectTree::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    foreach (ProjectTree *child, childItems)
        child->insertColumns(position, columns);

    return true;
}
