#ifndef PICTURERENDER_H
#define PICTURERENDER_H

#include <QObject>

class QImage;
class QPixmap;
class QModelIndex;

class PictureRender : public QObject
{
    Q_OBJECT
public:
    explicit PictureRender(QObject *parent = 0);
    ~PictureRender();

signals:
    void frameReady(const QImage &frame);

public slots:
    void play(const QModelIndex &index);
};

#endif // PICTURERENDER_H
