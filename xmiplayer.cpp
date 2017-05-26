#include "xmiplayer.h"

#include "models/music.h"

#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QDebug>
#include <QFile>
#include <QTimer>

#include <algorithm>
#include <vector>
#include <wildmidi_lib.h>

const uint32_t XmiPlayer::g_Samples = 16384;

XmiPlayer::XmiPlayer(QObject *parent) : QObject(parent), m_AudioOutput(0), m_Rate(44100)
{
    long libraryver = WildMidi_GetVersion();
    printf("Using libWildMidi %ld.%ld.%ld\n\n",
                        (libraryver>>16) & 255,
                        (libraryver>> 8) & 255,
                        (libraryver    ) & 255);
    WildMidi_SetCvtOption(WM_CO_XMI_TYPE, 2);

    if (WildMidi_Init("/etc/wildmidi/wildmidi.cfg", m_Rate, 0) == -1) {
        qWarning() << "WildMidi_Init failed";
    }
    WildMidi_MasterVolume(100);
}

XmiPlayer::~XmiPlayer()
{
    if (m_AudioOutput) {
        m_AudioOutput->stop();
        delete m_AudioOutput;
        m_AudioOutput = 0;
    }
    if (WildMidi_Shutdown() == -1) {
        qWarning() << "failed to shut down libWildMidi:" << WildMidi_GetError();
        WildMidi_ClearError();
    }
}

void XmiPlayer::play(const QModelIndex &index)
{
    int rv;
    if (!index.isValid()) {
        return;
    }

    if (m_AudioOutput) {
        m_AudioOutput->stop();
        delete m_AudioOutput;
        m_AudioOutput = 0;
    }

    model::Music::TrackInfo trackInfo = index.data(Qt::UserRole).value<model::Music::TrackInfo>();
    if (trackInfo.filePath.isEmpty()) {
        return;
    }

    QFile file(trackInfo.filePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "failed to open" << trackInfo.filePath;
        return;
    }

    qDebug() << "file:" << trackInfo.filePath
             << ", offset:" << trackInfo.offset
             << ", size:" << trackInfo.size;

    file.seek(trackInfo.offset);
    QByteArray inBuf = file.read(trackInfo.size);
    if (inBuf.size() != trackInfo.size) {
        qWarning() << "failed to read" << trackInfo.size << "bytes";
        return;
    }

    uint8_t* midiOutBuf = 0;
    uint32_t midiOutSize = 0;
    rv = WildMidi_ConvertBufferToMidi(reinterpret_cast<uint8_t*>(inBuf.data()),
                                      static_cast<uint32_t>(inBuf.size()),
                                      &midiOutBuf, &midiOutSize);
    if (rv < 0) {
        qWarning() << "failed to convert buffer to midi";
    }
    else if (midiOutBuf == 0) {
        qWarning() << "NULL output buffer";
    }
    else if (midiOutSize == 0) {
        qWarning() << "no bytes in output?";
    }
    else {
        qDebug() << "got" << midiOutSize/1024 << "kB of midi data";
        QFile midiFile("/tmp/test.mid");
        if (midiFile.open(QFile::WriteOnly | QFile::Truncate)) {
            midiFile.write(reinterpret_cast<const char*>(midiOutBuf), midiOutSize);
        }
    }

    midi *m = WildMidi_OpenBuffer(midiOutBuf, midiOutSize);
    if (m == NULL) {
        qWarning() << "failed to open buffer:" << WildMidi_GetError();
        return;
    }

    struct _WM_Info *wm_info = WildMidi_GetInfo(m);

    quint32 apr_mins = wm_info->approx_total_samples / (m_Rate * 60);
    quint32 apr_secs = (wm_info->approx_total_samples % (m_Rate * 60)) / m_Rate;

    qDebug() << "[Approx" << apr_mins << "m" << apr_secs << "s total]";

    std::vector<int8_t> outBuf(g_Samples, 0);
    QFile wavFile("/tmp/test.wav");
    wavFile.open(QFile::WriteOnly | QFile::Truncate);
    m_AudioBuf.open(QIODevice::ReadWrite);
    rv = WildMidi_GetOutput(m, &outBuf[0], g_Samples);
    while (rv > 0) {
        m_AudioBuf.write(reinterpret_cast<char*>(&outBuf[0]), rv);
        wavFile.write(reinterpret_cast<const char*>(&outBuf[0]), rv);
        rv = WildMidi_GetOutput(m, &outBuf[0], g_Samples);
    }
    free(midiOutBuf);
    wavFile.close();
    WildMidi_Close(m);
    outBuf.clear();
    if (rv < 0) {
        qWarning() << "failed to get output samples";
        return;
    }

    m_AudioBuf.seek(0);
    qDebug() << "got" << m_AudioBuf.size()/1024 << "Kb of wave data";

    QAudioFormat format;
    // Set up the format
    format.setCodec("audio/pcm");
    format.setSampleRate(m_Rate);
    format.setChannelCount(2);
    format.setSampleSize(16);
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
    connect(m_AudioOutput, SIGNAL(notify()), this, SLOT(showProgress()));
    m_AudioOutput->setNotifyInterval(1000);
    m_AudioOutput->start(&m_AudioBuf);
}

void XmiPlayer::showProgress()
{
}

void XmiPlayer::handleStateChanged(QAudio::State newState)
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
