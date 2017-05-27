#ifndef WAVEPLAYER_H
#define WAVEPLAYER_H

#include <QAudio>
#include <QBuffer>
#include <QByteArray>
#include <QObject>

#include <inttypes.h>

class QAudioOutput;
class QModelIndex;

class WavePlayer : public QObject
{
    Q_OBJECT
public:
    explicit WavePlayer(QObject *parent = 0);
    virtual ~WavePlayer();

signals:

public slots:
    virtual void play(const QModelIndex &index);
    void stop();

protected:
    explicit WavePlayer(const unsigned int sampleRate, int sampleSize, QObject *parent = 0);

    QBuffer m_AudioBuf;
    const unsigned int m_Rate;
    static const uint32_t g_BufSize;

    bool readFile(QByteArray &buf, const QModelIndex &index);
    void playWave();

private:
    const int m_SampleSize;
    QAudioOutput *m_AudioOutput;

private slots:
    void handleStateChanged(QAudio::State newState);
};

#endif // WAVEPLAYER_H
