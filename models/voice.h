#ifndef VOICE_H
#define VOICE_H

#include "trackinfo.h"

#include <QAbstractListModel>
#include <QList>
#include <QObject>

class QDataStream;

namespace model {

class Voice : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit Voice(QObject *parent = 0);

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

#endif // VOICE_H
