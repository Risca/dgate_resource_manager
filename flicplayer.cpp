#include "flicplayer.h"

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QRgb>
#include <QTimer>
#include <QVector>

FlicPlayer::FlicPlayer() : m_FlicDecoder(0), m_FileInterface(0)
{
}

FlicPlayer::~FlicPlayer()
{
    delete m_FlicDecoder;
    m_FlicDecoder = 0;
    delete m_FileInterface;
    m_FileInterface = 0;
}

void FlicPlayer::play(const QString &filename)
{
    if (m_File.isOpen()) {
        m_File.close();
    }
    m_File.setFileName(filename);

    if (!m_File.open(QFile::ReadOnly)) {
        qWarning() << "failed to open" << filename;
    }
    if (m_FileInterface) {
        delete m_FileInterface;
        m_FileInterface = 0;
    }
    m_FileInterface = new flic::QFileInterface(m_File);

    if (m_FlicDecoder) {
        delete m_FlicDecoder;
        m_FlicDecoder = 0;
    }
    m_FlicDecoder = new flic::Decoder(m_FileInterface);

    if (m_FlicDecoder) {
        if (!m_FlicDecoder->readHeader(m_Header)) {
            qWarning() << "failed to read header";
            delete m_FlicDecoder;
            m_FlicDecoder = 0;
        }
        else {
            m_Pixels.resize(3 * m_Header.height * m_Header.width);
            m_Frame.pixels = &m_Pixels[0];
            m_Frame.rowstride = m_Header.width;
            m_FrameCount = 0;
            processNextFrame();
        }
    }
    else {
        qWarning() << "Failed to open" << filename;
    }
}

void FlicPlayer::processNextFrame()
{
    qDebug() << "processNextFrame()";
    if (m_FlicDecoder) {
        if (!m_FlicDecoder->readFrame(m_Frame)) {
            qWarning() << "failed to read frame";
            return;
        }

        QImage image((const uchar *)m_Frame.pixels, m_Header.width, m_Header.height, QImage::Format_Indexed8);
        QVector<QRgb> palette(m_Frame.colormap.size());
        for (int c = 0; c < palette.size(); ++c) {
            palette[c] = qRgb(m_Frame.colormap[c].r,
                              m_Frame.colormap[c].g,
                              m_Frame.colormap[c].b);
        }
        image.setColorTable(palette);
        emit frameReady(image);

        m_FrameCount++;
        if (m_FrameCount >= m_Header.frames) {
            qDebug() << "Processed the last frame";
            delete m_FlicDecoder;
            m_FlicDecoder = 0;
            delete m_FileInterface;
            m_FileInterface = 0;
            m_File.close();
            m_Pixels.clear();
        }
        else {
            QTimer::singleShot(m_Header.speed, this, SLOT(processNextFrame()));
        }
    }
}
