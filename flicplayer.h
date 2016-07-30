#ifndef FLICPLAYER_H
#define FLICPLAYER_H

#include <flic/flic.h>

#include <QFile>
#include <QObject>
#include <QVector>

#include <inttypes.h>

class QImage;
class QModelIndex;
class QString;

namespace flic {
class QIODeviceInterface;
}

class FlicPlayer : public QObject
{
    Q_OBJECT
public:
    FlicPlayer();
    ~FlicPlayer();

public slots:
    void play(const QString &filename);
    void play(const QModelIndex &index);

signals:
    void frameReady(const QImage &frame);
    void finished();

private:
    flic::Decoder *m_FlicDecoder;
    flic::Header m_Header;
    flic::Frame m_Frame;
    int m_FrameCount;
    QVector<uint8_t> m_Pixels;
    QFile m_File;
    flic::QIODeviceInterface *m_FileInterface;

private slots:
    void processNextFrame();
};

#endif // FLICPLAYER_H
