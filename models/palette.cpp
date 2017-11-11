#include "palette.h"

#include <QColor>
#include <QImage>
#include <QSize>

namespace model {

model::Palette::Palette(QObject *parent) :
    QAbstractTableModel(parent),
    m_colorTable(256, 0xFFFFFFFF)
{
}

int Palette::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 16;
}

int Palette::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 16;
}

QVariant Palette::data(const QModelIndex &index, int role) const
{
    QVariant value;

    int x = index.column();
    int y = index.row();
    if (index.isValid() && x < 16 && y < 16) {
        switch (role) {
        case Qt::BackgroundRole:
            value = QColor(m_colorTable[x + y*16]);
            break;

        case Qt::UserRole:
            if (m_selectedIndex.isValid() && index == m_selectedIndex) {
                value = true;
            }
            else {
                value = false;
            }
            break;

        default:
            break;
        }
    }

    return value;
}

void Palette::setPalette(const QImage &frame)
{
    QVector<QRgb> colors = frame.colorTable();
    if (!colors.isEmpty()) {
        m_colorTable = frame.colorTable();
        m_selectedIndex = QModelIndex();
        emit dataChanged(index(0, 0), index(15, 15));
    }
}

void Palette::selectPaletteColor(int colorIndex)
{
    QModelIndex oldIndex = m_selectedIndex;
    m_selectedIndex = index(colorIndex / 16, colorIndex % 16);
    emit dataChanged(oldIndex, oldIndex);
    emit dataChanged(m_selectedIndex, m_selectedIndex);
}

} // namespace model
