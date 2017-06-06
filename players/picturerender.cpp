#include "picturerender.h"

#include "models/image.h"

#include <algorithm>
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

bool OpenFile(const QModelIndex &index, QFile &file)
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

bool ImageHasPalette(const QModelIndex &index)
{
    return index.sibling(index.row(), model::Image::COLUMN_PALETTE).data().toBool();
}

void ReadPalette(const QModelIndex &index, QVector<QRgb> &colors)
{
    QFile file;
    if (!OpenFile(index, file)) {
        return;
    }

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

void ReadImageFromFile(const QModelIndex &index, QFile &file, QImage &image, const QVector<QRgb> &colors)
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

void ReadImage(const QModelIndex &index, QImage &image, const QVector<QRgb> &colors)
{
    QFile file;
    if (!OpenFile(index, file)) {
        return;
    }

    return ReadImageFromFile(index, file, image, colors);
}

} // anonymous namespace

PictureRender::PictureRender(QObject *parent) :
    QObject(parent)
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

    // figure out what palette to use
    QVector<QRgb> colors;
    if (ImageHasPalette(index)) {
        // Image contain its own palette
        ReadPalette(index, colors);
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
            ReadPalette(index.sibling(paletteRow, 0), colors);
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

    ReadImage(index, m_Image, colors);
    emit frameReady(m_Image);
}

void PictureRender::overlay(const QModelIndex &index)
{
    if (!m_Overlay.enabled || !index.parent().isValid() || m_Image.isNull()) {
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
    if (!OpenFile(index, file)) {
        return;
    }
    if (!file.seek(fileOff)) {
        return;
    }

    char buf[4];
    file.read(buf, 4);
    quint16 x = *reinterpret_cast<quint16*>(&buf[0]);
    quint16 y = *reinterpret_cast<quint16*>(&buf[2]);
    m_Overlay.coord = QPoint(x, y);
    ReadImageFromFile(index, file, m_Overlay.image, m_Image.colorTable());

    performOverlay();
}

void PictureRender::enableOverlay(bool enable)
{
    m_Overlay.enabled = enable;
    if (enable) {
        if (!m_Overlay.image.isNull()) {
            performOverlay();
        }
    }
    else {
        emit frameReady(m_Image);
    }
}

void PictureRender::setSubPalette1(const QModelIndex &index)
{
    if (!m_Overlay.enabled || !index.parent().isValid() || m_Image.isNull() || !ImageHasPalette(index)) {
        return;
    }

    QVector<QRgb> subPalette;
    ReadPalette(index, subPalette);

    QVector<QRgb> palette = m_Overlay.image.colorTable();
    std::copy(subPalette.begin() + 16, subPalette.begin() + (16+16), palette.begin() + 16);
    m_Overlay.image.setColorTable(palette);
    performOverlay();
}

void PictureRender::setSubPalette2(const QModelIndex &index)
{
    if (!m_Overlay.enabled || !index.parent().isValid() || m_Image.isNull() || !ImageHasPalette(index)) {
        return;
    }

    QVector<QRgb> subPalette;
    ReadPalette(index, subPalette);

    QVector<QRgb> palette = m_Overlay.image.colorTable();
    std::copy(subPalette.begin() + 32, subPalette.begin() + (32+48), palette.begin() + 32);
    m_Overlay.image.setColorTable(palette);
    performOverlay();
}

void PictureRender::performOverlay()
{
    QImage image = m_Overlay.image;
    QList<QPoint> alphaPixels;
    const uchar *bits = image.bits();
    for (int line = 0; line < image.height(); ++line) {
        for (int row = 0; row < image.width(); ++row) {
            const uchar pixel = bits[line * image.width() + row];
            if (pixel == 0) {
                alphaPixels.push_back(QPoint(row, line));
            }
        }
    }
    image = image.convertToFormat(QImage::Format_ARGB32);
    foreach (const QPoint& p, alphaPixels) {
        image.setPixel(p, qRgba(0, 0, 0, 0));
    }

    QImage surface = m_Image.convertToFormat(QImage::Format_ARGB32);
    QPainter p(&surface);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawImage(m_Overlay.coord, image);
    p.end();
    emit frameReady(surface);
}
