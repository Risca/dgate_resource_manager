#ifndef CELLBORDERDELEGATE_H
#define CELLBORDERDELEGATE_H

#include <QModelIndex>
#include <QPainter>
#include <QStyledItemDelegate>

class CellBorderDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CellBorderDelegate(QObject *parent = 0) : QStyledItemDelegate(parent)
    { }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const
    {
        QStyledItemDelegate::paint(painter, option, index);
        bool selected = index.data(Qt::UserRole).toBool();
        if (selected) {
            QPen pen = painter->pen();
            pen.setColor(Qt::red);
            pen.setWidth(3);
            painter->setPen(pen);
            painter->drawRect(option.rect);
        }
    }
};

#endif // CELLBORDERDELEGATE_H
