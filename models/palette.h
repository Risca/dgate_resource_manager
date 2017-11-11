#ifndef PALETTE_H
#define PALETTE_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVector>
#include <QRgb>

class QImage;

namespace model {

class Palette : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit Palette(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

public slots:
    void setPalette(const QImage &frame);
    void selectPaletteColor(int colorIndex);

private:
    QVector<QRgb> m_colorTable;
    QModelIndex m_selectedIndex;
};

} // namespace model

#endif // PALETTE_H
