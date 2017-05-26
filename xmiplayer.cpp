#include "xmiplayer.h"

#include "models/music.h"

#include <QDebug>

#include <vector>
#include <wildmidi_lib.h>

XmiPlayer::XmiPlayer(QObject *parent) : WavePlayer(44100, 16, parent)
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
    if (WildMidi_Shutdown() == -1) {
        qWarning() << "failed to shut down libWildMidi:" << WildMidi_GetError();
        WildMidi_ClearError();
    }
}

void XmiPlayer::play(const QModelIndex &index)
{
    QByteArray inBuf;
    if (!readFile(inBuf, index)) {
        return;
    }

    int rv;
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

    std::vector<int8_t> outBuf(g_BufSize, 0);
    m_AudioBuf.open(QIODevice::ReadWrite);
    rv = WildMidi_GetOutput(m, &outBuf[0], g_BufSize);
    while (rv > 0) {
        m_AudioBuf.write(reinterpret_cast<char*>(&outBuf[0]), rv);
        rv = WildMidi_GetOutput(m, &outBuf[0], g_BufSize);
    }
    free(midiOutBuf);
    WildMidi_Close(m);
    outBuf.clear();
    if (rv < 0) {
        qWarning() << "failed to get output samples";
        return;
    }

    m_AudioBuf.seek(0);
    qDebug() << "got" << m_AudioBuf.size()/1024 << "Kb of wave data";
    playWave();
}
