#include "music.h"

#include <QDataStream>
#include <QDebug>

namespace model {

Music::Music(QObject *parent) : AbstractAudioModel("DGATE001.MUS", parent)
{
}

void Music::parse(QDataStream &stream, const QString &filePath)
{
    quint16 nTracks;

    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> nTracks;

    m_Tracks.clear();
    for (int t = 0; t < nTracks; ++t) {
        quint32 offset;
        quint16 size;
        stream >> offset >> size;
        if (!size) {
            // last track is supposed to have zero size
            if (t != (nTracks - 1)) {
                qWarning() << "Zero size of track" << t;
            }
        }
        else {
            TrackInfo track = { filePath, offset, size };
            m_Tracks.append(track);
        }
    }
}

} // namespace model
