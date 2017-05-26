#ifndef VOICE_H
#define VOICE_H

#include "abstractaudiomodel.h"

namespace model {

class Voice : public AbstractAudioModel
{
    Q_OBJECT

public:
    explicit Voice(QObject *parent = 0);

private:
    void parse(QDataStream &stream, const QString &filePath) override;
};

} // namespace model

#endif // VOICE_H
