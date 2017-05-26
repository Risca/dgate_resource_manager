#include "voice.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>

namespace model {

Voice::Voice(QObject *parent) : QAbstractListModel(parent)
{
}

int Voice::rowCount(const QModelIndex &) const
{
    return m_Tracks.count();
}

QVariant Voice::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= rowCount()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return QString::number(row + 1);
        break;

    case Qt::UserRole:
        return QVariant::fromValue<TrackInfo>(m_Tracks[row]);

    default:
        break;
    }
    return QVariant();
}

QVariant Voice::headerData(int section, Qt::Orientation orientation, int role) const
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

void Voice::processDirectory(const QString &dir)
{
    QFile file(dir + "/DGATE001.VOC");
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QDataStream fileStream(&file);
    parse(fileStream, file.fileName());
}

void Voice::parse(QDataStream &stream, const QString &filePath)
{
    quint16 nTracks;

    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> nTracks;

    m_Tracks.clear();
    quint32 offset;
    stream >> offset;
    for (int t = 0; t < nTracks; ++t) {
        quint32 nextOffset;
        stream >> nextOffset;

        TrackInfo track;
        track.offset = offset;
        track.size = nextOffset - offset;
        if (track.size > 0) {
            track.filePath = filePath;
            m_Tracks.append(track);
        }
        else if (track.size == 0) {
            // group break;
        }
        else if (track.size < 0) {
            // last track is supposed to have negative size
            if (t != (nTracks - 1)) {
                qWarning() << "track" << t << "has negative size";
            }
        }
        offset = nextOffset;
    }

    emit dataChanged(createIndex(0, 0),
                     createIndex(rowCount() - 1, 0));
}

} // namespace model
