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
    explicit Music(QObject* parent = 0);

    struct TrackInfo {
        QString filePath;
        quint32 offset;
        quint16 size;
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
    void processDirectory(const QString &dir);

private:
    QList<TrackInfo> m_Tracks;

    void parse(QDataStream &stream, const QString &filePath);
};

} // namespace model

Q_DECLARE_METATYPE(model::Music::TrackInfo)

#endif // MUSIC_H
