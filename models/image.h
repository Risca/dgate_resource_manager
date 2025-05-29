#ifndef IMAGE_H
#define IMAGE_H

#include <QAbstractItemModel>
#include <QSet>
#include <QString>

namespace model {

class TreeItem;

class Image : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit Image(QObject *parent = 0);
    ~Image();

    enum {
        COLUMN_OFFSET,
        COLUMN_IMG_OFF,
        COLUMN_FLAGS,
        COLUMN_WIDTH,
        COLUMN_HEIGHT,
        COLUMN_PALETTE,

        COLUMN_END
    };

    enum Flags {
        FlagCoordinates = (1 << 0),
        FlagPalette     = (1 << 12),
        FlagTransparent = (1 << 14),
    };

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void processDirectory(const QString &dir);

private:
    TreeItem *m_RootItem;
    QString m_Dir;
    static const QSet<QString> g_KnownImageFiles;

    void parse(QDataStream &stream, TreeItem *parent);
};

} // namespace model

#endif // IMAGE_H
