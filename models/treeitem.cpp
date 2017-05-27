#include "treeitem.h"

#include <QtAlgorithms>

namespace model {

TreeItem::TreeItem(const QMap<int, QVariant> &data, TreeItem *parentItem) :
    m_ItemData(data), m_ParentItem(parentItem)
{

}

TreeItem::~TreeItem()
{
    qDeleteAll(m_ChildItems);
}

void TreeItem::appendChild(TreeItem *child)
{
    m_ChildItems.append(child);
}

TreeItem *TreeItem::child(int row)
{
    return m_ChildItems.value(row);
}

int TreeItem::childCount() const
{
    return m_ChildItems.count();
}

int TreeItem::columnCount() const
{
    return m_ItemData.count();
}

QVariant TreeItem::data(int column) const
{
    return m_ItemData.value(column);
}

int TreeItem::row() const
{
    if (m_ParentItem) {
        return m_ParentItem->m_ChildItems.indexOf(const_cast<TreeItem*>(this));
    }
    return 0;
}

TreeItem *TreeItem::parentItem()
{
    return m_ParentItem;
}

} // namespace model
