#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QMap>
#include <QVariant>

namespace model {

class TreeItem
{
public:
    typedef QMap<int, QVariant> ItemData;

    explicit TreeItem(const ItemData &data, TreeItem *parentItem = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parentItem();

private:
    QList<TreeItem*> m_ChildItems;
    ItemData m_ItemData;
    TreeItem *m_ParentItem;
};

} // namespace model

#endif // TREEITEM_H
