#ifndef PROJECTTREE_H
#define PROJECTTREE_H

#include <QList>
#include <QVariant>
#include <QVector>

class ProjectTree{
public:
    explicit ProjectTree(const QVector<QVariant> &data, ProjectTree *parent = 0);
    ~ProjectTree();

    ProjectTree *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    ProjectTree *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);

private:
    QList<ProjectTree*> childItems;
    QVector<QVariant> itemData;
    ProjectTree *parentItem;
};

#endif // PROJECTTREE_H
