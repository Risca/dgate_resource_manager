#ifndef TRACKINFO_H
#define TRACKINFO_H

#include <QMetaType>
#include <QtGlobal>
#include <QString>

namespace model {

struct TrackInfo {
    QString filePath;
    quint32 offset;
    qint32 size;
};

}

Q_DECLARE_METATYPE(model::TrackInfo)

#endif // TRACKINFO_H
