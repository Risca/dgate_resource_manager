#include "voice.h"

#include <QDataStream>
#include <QDebug>

namespace model {

Voice::Voice(QObject *parent) : AbstractAudioModel("DGATE001.VOC", parent)
{
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
}

} // namespace model
