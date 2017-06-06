#include "picturerender.h"

#include "models/image.h"

#include <QColor>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QModelIndex>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QVariant>
#include <QVector>

#define PALETTE_SIZE (3 * 256)

namespace {

bool openFile(const QModelIndex &index, QFile &file)
{
    QString filename = index.parent().data(Qt::UserRole).toString();
    qDebug() << "using pic" << index.row() + 1 << "in" << filename;

    file.setFileName(filename);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "failed to open" << filename;
        return false;
    }

    return true;
}

bool GetOffsets(const QModelIndex &index, qint32 &fileOffset, qint32 &imageOffset)
{
    bool ok;
    fileOffset = index.sibling(index.row(), model::Image::COLUMN_OFFSET).data().toLongLong(&ok);
    if (!ok) {
        qWarning() << "no file offset";
        return false;
    }

    imageOffset = index.sibling(index.row(), model::Image::COLUMN_IMG_OFF).data().toLongLong(&ok);
    if (!ok) {
        qWarning() << "no image offset";
        return false;
    }

    return true;
}

bool GetSizes(const QModelIndex &index, qint16 &width, qint16 &height)
{
    width = index.sibling(index.row(), model::Image::COLUMN_WIDTH).data().toLongLong();
    if (!width) {
        qWarning() << "no width";
        return false;
    }

    height = index.sibling(index.row(), model::Image::COLUMN_HEIGHT).data().toLongLong();
    if (!height) {
        qWarning() << "no height";
        return false;
    }

    return true;
}

void ReadPaletteFromFile(const QModelIndex &index, QFile &file, QVector<QRgb> &colors)
{
    qint32 fileOff, imgOff;
    if (!GetOffsets(index, fileOff, imgOff)) {
        return;
    }

    if (!file.seek(fileOff + imgOff - PALETTE_SIZE)) {
        qWarning() << "failed to seek to" << fileOff + imgOff - PALETTE_SIZE;
        return;
    }

    QByteArray paletteData = file.read(PALETTE_SIZE);
    const char* d = paletteData.constBegin();
    for (int i=0; i<256; ++i) {
        // red, green, blue
        uchar r=*d++;
        uchar g=*d++;
        uchar b=*d++;
        // Convert 6bit to 8bit
        r = (r<<2) | (r>>4);
        g = (g<<2) | (g>>4);
        b = (b<<2) | (b>>4);
        QRgb color = 0xff000000 | (r<<16) | (g<<8) | b;
        colors.push_back(color);
    }
}

void readImage(const QModelIndex &index, QFile &file, QImage &image, const QVector<QRgb> &colors)
{
    qint32 fileOff, imgOff;
    if (!GetOffsets(index, fileOff, imgOff)) {
        return;
    }

    qint16 width, height;
    if (!GetSizes(index, width, height)) {
        return;
    }

    if (!file.seek(fileOff + imgOff)) {
        qWarning() << "failed to seek to image @" << fileOff + imgOff;
        return;
    }

    quint32 size = width * height;
    uchar *data = new uchar[size];
    if (file.read(reinterpret_cast<char*>(data), size) != size) {
        qWarning() << "failed to read" << size << "bytes";
        delete [] data;
        return;
    }

    QImage img(data, width, height, width, QImage::Format_Indexed8);
    img.setColorTable(colors);
    image = img.copy();
    delete [] data;
}

} // anonymous namespace

PictureRender::PictureRender(QObject *parent) : QObject(parent), m_OverlayEnabled(false)
{

}

PictureRender::~PictureRender()
{

}

void PictureRender::render(const QModelIndex &index)
{
    if (!index.parent().isValid()) {
        qWarning() << "no valid parent";
        return;
    }

    QFile file;
    if (!openFile(index, file)) {
        return;
    }

    // figure out what palette to use
    QVector<QRgb> colors;
    if (index.sibling(index.row(), model::Image::COLUMN_PALETTE).data().toBool()) {
        // Image contain its own palette
        ReadPaletteFromFile(index, file, colors);
    }
    else {
        // Search backwards trying to find a palette from an other pic
        int paletteRow = -1;
        for (int r = index.row(); r >= 0 && paletteRow < 0; --r) {
            if (index.sibling(r, model::Image::COLUMN_PALETTE).data().toBool()) {
                paletteRow = r;
            }
        }
        if (paletteRow != -1) {
            // read palette from image
            qDebug() << "using palette from image" << paletteRow;
            ReadPaletteFromFile(index.sibling(paletteRow, 0), file, colors);
        }
        else {
            // use a default palette
            for (int i=0; i<256; ++i) {
                QRgb color = 0xff000000 | (i<<16) | (i<<8) | i;
                colors.push_back(color);
            }
        }
    }
    if (colors.empty()) {
        qWarning() << "failed to determine palette";
        return;
    }

    readImage(index, file, m_Image, colors);
    emit frameReady(m_Image);
}

void PictureRender::overlay(const QModelIndex &index)
{
    if (!m_OverlayEnabled) {
        return;
    }

    if (!index.parent().isValid()) {
        qWarning() << "no valid parent";
        return;
    }

    if (m_Image.isNull()) {
        qWarning() << "no image to overlay";
        return;
    }

    int flags = index.sibling(index.row(), model::Image::COLUMN_FLAGS).data().toInt();
    if ((flags & model::Image::FlagCoordinates) == 0) {
        qWarning() << "no coordinates in image header";
        return;
    }

    qint32 fileOff, imgOff;
    if (!GetOffsets(index, fileOff, imgOff)) {
        return;
    }

    QFile file;
    if (!openFile(index, file)) {
        return;
    }
    if (!file.seek(fileOff)) {
        return;
    }

    char buf[4];
    file.read(buf, 4);
    quint16 x = *reinterpret_cast<quint16*>(&buf[0]);
    quint16 y = *reinterpret_cast<quint16*>(&buf[2]);

    QImage overlay;
    readImage(index, file, overlay, m_Image.colorTable());
    QList<QPoint> alphaPixels;
    const uchar *bits = overlay.bits();
    for (int line = 0; line < overlay.height(); ++line) {
        for (int row = 0; row < overlay.width(); ++row) {
            const uchar pixel = bits[line * overlay.width() + row];
            if (pixel == 0) {
                alphaPixels.push_back(QPoint(row, line));
            }
        }
    }
    overlay = overlay.convertToFormat(QImage::Format_ARGB32);
    foreach (const QPoint& p, alphaPixels) {
        overlay.setPixel(p, qRgba(0, 0, 0, 0));
    }

    QImage surface = m_Image.convertToFormat(QImage::Format_ARGB32);
    QPainter p(&surface);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawImage(x, y, overlay);
    p.end();
    emit frameReady(surface);
}

void PictureRender::enableOverlay(bool enable)
{
    m_OverlayEnabled = enable;
    if (!enable) {
        emit frameReady(m_Image);
    }
}
