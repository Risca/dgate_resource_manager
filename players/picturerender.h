#ifndef PICTURERENDER_H
#define PICTURERENDER_H

#include <QImage>
#include <QModelIndexList>
#include <QObject>
#include <QPoint>

class QModelIndex;

class PictureRender : public QObject
{
    Q_OBJECT
public:
    explicit PictureRender(QObject *parent = 0);
    ~PictureRender();

signals:
    void frameReady(const QImage &frame);
    void colorIndexSelected(int);

public slots:
    void render(const QModelIndex &index);
    void enableOverlay(bool enable);
    void selectPixel(int x, int y);

public:
    void overlay(const QModelIndexList &images);

private:
    QImage m_Image;
    struct {
        bool enabled = false;
        QImage surface;
        QModelIndexList images;
    } m_Overlay;

    void performOverlay();
    void overlayOneImage(const QModelIndex& index);
};

#endif // PICTURERENDER_H
