#ifndef PICTURERENDER_H
#define PICTURERENDER_H

#include <QImage>
#include <QObject>

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
    void render(const QModelIndex &index);
    void overlay(const QModelIndex &index);
    void enableOverlay(bool enable);

private:
    QImage m_Image;
    QImage m_OverlayedImage;
    bool m_OverlayEnabled;
};

#endif // PICTURERENDER_H
