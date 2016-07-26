#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QWidget>

class QPolygon;

class VideoSurface : public QWidget
{
    Q_OBJECT

public:
    explicit VideoSurface(QWidget *parent = 0, Qt::WindowFlags f = 0 );

public slots:
    void present(const QImage &frame);

protected:
    void paintEvent(QPaintEvent * event);

private:
    QImage m_Image;
};

#endif // VIDEOSURFACE_H
