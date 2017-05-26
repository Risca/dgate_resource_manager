#ifndef MUSIC_H
#define MUSIC_H

#include "abstractaudiomodel.h"

namespace model {

class Music : public AbstractAudioModel
{
    Q_OBJECT

public:
    explicit Music(QObject *parent = 0);

private:
    void parse(QDataStream &stream, const QString &filePath) override;
};

} // namespace model

#endif // MUSIC_H
