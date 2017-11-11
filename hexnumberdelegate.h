#ifndef HEXNUMBERDELEGATE_H
#define HEXNUMBERDELEGATE_H

#include <QStyledItemDelegate>

class HexNumberDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit HexNumberDelegate(QObject *parent = 0) : QStyledItemDelegate(parent)
    { }

    QString displayText(const QVariant &value, const QLocale &) const
    {
        bool ok;
        qlonglong v = value.toLongLong(&ok);
        if (!ok) {
            return value.toString();
        }

        return QString("0x") + QString::number(v, 16);
    }
};

#endif // HEXNUMBERDELEGATE_H
