#ifndef MUSIC_H
#define MUSIC_H

#include <QAbstractListModel>
#include <QList>
#include <QObject>

class QDataStream;

namespace model {

class Music : public QAbstractListModel
{
    Q_OBJECT;

public:
    Music(QObject* parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
    void processDirectory(const QString &dir);

private:
    struct TrackInfo {
        quint32 offset;
        quint16 size;
    };

    QList<TrackInfo> m_Tracks;

    void parse(QDataStream &stream);
};

} // namespace model

#endif // MUSIC_H
