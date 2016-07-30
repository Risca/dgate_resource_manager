#include "music.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>

namespace model {

Music::Music(QObject *parent) :
    QAbstractListModel(parent)
{

}

int Music::rowCount(const QModelIndex &) const
{
    return m_Tracks.count();
}

QVariant Music::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= rowCount()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return QString::number(row + 1);
        break;

    default:
        break;
    }
    return QVariant();
}

QVariant Music::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal && section == 0) {
            return "Track name";
        }
        if (orientation == Qt::Vertical && section < rowCount()) {
            return QString::number(section + 1);
        }
    }

    return QVariant();
}

void Music::processDirectory(const QString &dir)
{
    QFile file(dir + "/DGATE001.MUS");
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QDataStream fileStream(&file);
    parse(fileStream);
}

void Music::parse(QDataStream &stream)
{
    quint16 nTracks;

    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> nTracks;

    m_Tracks.clear();
    for (int t = 0; t < nTracks; ++t) {
        TrackInfo track;
        stream >> track.offset >> track.size;
        if (!track.size) {
            // last track is supposed to have zero size
            if (t != (nTracks - 1)) {
                qWarning() << "Zero size of track" << t;
            }
        }
        else {
            m_Tracks.append(track);
        }
    }

    emit dataChanged(createIndex(0, 0),
                     createIndex(rowCount() - 1, 0));
}

} // namespace model