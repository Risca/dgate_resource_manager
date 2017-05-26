#include "waveplayer.h"

#include "models/trackinfo.h"

#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QDebug>
#include <QFile>
#include <QModelIndex>

const uint32_t WavePlayer::g_BufSize = 16384;

WavePlayer::WavePlayer(QObject *parent) :
    QObject(parent),
    m_Rate(10900),
    m_SampleSize(8),
    m_AudioOutput(0)
{

}

WavePlayer::WavePlayer(const unsigned int sampleRate, int sampleSize, QObject *parent) :
    QObject(parent),
    m_Rate(sampleRate),
    m_SampleSize(sampleSize),
    m_AudioOutput(0)
{

}

WavePlayer::~WavePlayer()
{
    if (m_AudioOutput) {
        m_AudioOutput->stop();
        delete m_AudioOutput;
        m_AudioOutput = 0;
    }
}

void WavePlayer::play(const QModelIndex &index)
{
    QByteArray buf;
    if (!readFile(buf, index)) {
        return;
    }

    m_AudioBuf.close();
    m_AudioBuf.setData(buf);
    m_AudioBuf.open(QIODevice::ReadWrite);

    playWave();
}

void WavePlayer::stop()
{
    if (m_AudioOutput) {
        m_AudioOutput->stop();
        delete m_AudioOutput;
        m_AudioOutput = 0;
    }
}

bool WavePlayer::readFile(QByteArray &buf, const QModelIndex &index)
{
    if (!index.isValid()) {
        return false;
    }

    stop();

    model::TrackInfo trackInfo = index.data(Qt::UserRole).value<model::TrackInfo>();
    if (trackInfo.filePath.isEmpty()) {
        return false;
    }

    QFile file(trackInfo.filePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "failed to open" << trackInfo.filePath;
        return false;
    }

    qDebug() << "file:" << trackInfo.filePath
             << ", offset:" << trackInfo.offset
             << ", size:" << trackInfo.size;

    file.seek(trackInfo.offset);
    buf = file.read(trackInfo.size);
    if (buf.size() != trackInfo.size) {
        qWarning() << "failed to read" << trackInfo.size << "bytes";
        return false;
    }

    return true;
}

void WavePlayer::playWave()
{
    QAudioFormat format;
    // Set up the format
    format.setCodec("audio/pcm");
    format.setSampleRate(m_Rate);
    format.setChannelCount(2);
    format.setSampleSize(m_SampleSize);
    format.setByteOrder(QAudioFormat::Endian(QSysInfo::ByteOrder));
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo dev = QAudioDeviceInfo::defaultOutputDevice();
    if (!dev.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by default output, checking others";
        bool found = false;
        foreach (dev, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
            if (dev.isFormatSupported(format)) {
                qDebug() << "Found working device:" << dev.deviceName();
                break;
            }
            else {
                qDebug() << dev.deviceName() << "does not support raw audio";
            }
        }
        if (!found) {
            qWarning() << "Found no output devices able to play raw audio, aborting";
            return;
        }
    }

    m_AudioOutput = new QAudioOutput(dev, format, this);
    connect(m_AudioOutput, SIGNAL(stateChanged(QAudio::State)),
            this, SLOT(handleStateChanged(QAudio::State)));
    m_AudioOutput->setNotifyInterval(1000);
    m_AudioOutput->start(&m_AudioBuf);
}

void WavePlayer::handleStateChanged(QAudio::State newState)
{
    switch (newState) {
    case QAudio::IdleState:
        m_AudioOutput->stop();
        m_AudioBuf.close();
        m_AudioOutput->deleteLater();
        m_AudioOutput = 0;
        break;

    case QAudio::StoppedState:
        if (m_AudioOutput->error() != QAudio::NoError) {
            qWarning() << "Some error occurred:" << m_AudioOutput->error();
        }

    default:
        break;
    }
}
