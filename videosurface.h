#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QWidget>

class QPolygon;

class VideoSurface : public QWidget
{
    Q_OBJECT

public:
    explicit VideoSurface(QWidget *parent = 0, Qt::WindowFlags f = 0 );

    QSize sizeHint() const;

signals:
    void mouseMoved(int x, int y);

public slots:
    void present(const QImage &frame);
    void resize(int times);

protected:
    void paintEvent(QPaintEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

private:
    QImage m_Image;
    int m_ScaleFactor;
};

#endif // VIDEOSURFACE_H
