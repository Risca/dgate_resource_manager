#include "picturerender.h"

#include "models/image.h"

#include <QColor>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QModelIndex>
#include <QPixmap>
#include <QString>
#include <QVariant>
#include <QVector>

#define PALETTE_SIZE (3 * 256)

namespace {

bool GetOffsets(const QModelIndex &parent, int row, qint32 &fileOffset, qint32 &imageOffset)
{
    bool ok;
    fileOffset = parent.child(row, model::Image::COLUMN_OFFSET).data().toLongLong(&ok);
    if (!ok) {
        qWarning() << "no file offset";
        return false;
    }

    imageOffset = parent.child(row, model::Image::COLUMN_IMG_OFF).data().toLongLong(&ok);
    if (!ok) {
        qWarning() << "no image offset";
        return false;
    }

    return true;
}

bool GetSizes(const QModelIndex &parent, int row, qint16 &width, qint16 &height)
{
    width = parent.child(row, model::Image::COLUMN_WIDTH).data().toLongLong();
    if (!width) {
        qWarning() << "no width";
        return false;
    }

    height = parent.child(row, model::Image::COLUMN_HEIGHT).data().toLongLong();
    if (!height) {
        qWarning() << "no height";
        return false;
    }

    return true;
}

void ReadPaletteFromFile(const QModelIndex &parent, int row, QFile &file, QVector<QRgb> &colors)
{
    qint32 fileOff, imgOff;
    if (!GetOffsets(parent, row, fileOff, imgOff)) {
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

} // anonymous namespace

PictureRender::PictureRender(QObject *parent) : QObject(parent)
{

}

PictureRender::~PictureRender()
{

}

void PictureRender::play(const QModelIndex &index)
{
    int row = index.row();
    QModelIndex parent = index.parent();
    if (!parent.isValid()) {
        qDebug() << "top level item, using first pic";
        parent = index.sibling(row, 0);
        row = 0;
    }

    if (!parent.isValid()) {
        qWarning() << "no valid indexes";
        return;
    }

    QString filename = parent.data(Qt::UserRole).toString();
    qDebug() << "showing pic" << row + 1 << "in" << filename;

    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open" << filename;
        return;
    }

    // figure out what palette to use
    QVector<QRgb> colors;
    if (parent.child(row, model::Image::COLUMN_PALETTE).data().toBool()) {
        // Image contain its own palette
        ReadPaletteFromFile(parent, row, file, colors);
    }
    else {
        // Search backwards trying to find a palette from an other pic
        int paletteRow = -1;
        for (int r = row; r >= 0 && paletteRow < 0; --r) {
            if (parent.child(r, model::Image::COLUMN_PALETTE).data().toBool()) {
                paletteRow = r;
            }
        }
        if (paletteRow != -1) {
            // read palette from image
            qDebug() << "using palette from image" << paletteRow;
            ReadPaletteFromFile(parent, paletteRow, file, colors);
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

    qint32 fileOff, imgOff;
    if (!GetOffsets(parent, row, fileOff, imgOff)) {
        return;
    }

    qint16 width, height;
    if (!GetSizes(parent, row, width, height)) {
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

    QImage image = QImage(data, width, height, width, QImage::Format_Indexed8);
    image.setColorTable(colors);
    emit frameReady(image);
}
