#ifndef XMIPLAYER_H
#define XMIPLAYER_H

#include <QAudio>
#include <QBuffer>
#include <QObject>

#include <stdint.h>

class QAudioOutput;
class QModelIndex;

class XmiPlayer : public QObject
{
    Q_OBJECT
public:
    explicit XmiPlayer(QObject *parent = 0);
    ~XmiPlayer();

signals:

public slots:
    void play(const QModelIndex &index);
    void showProgress();

private:
    QAudioOutput *m_AudioOutput;
    unsigned int m_Rate;
    QBuffer m_AudioBuf;

    static const uint32_t g_Samples;

private slots:
    void handleStateChanged(QAudio::State newState);
};

#endif // XMIPLAYER_H
