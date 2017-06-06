#ifndef PICTURERENDER_H
#define PICTURERENDER_H

#include <QImage>
#include <QObject>
#include <QPoint>

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
    void setSubPalette1(const QModelIndex &index);
    void setSubPalette2(const QModelIndex &index);

private:
    QImage m_Image;
    struct {
        bool enabled = 0;
        QPoint coord;
        QImage image;
    } m_Overlay;

    void performOverlay();
};

#endif // PICTURERENDER_H
