#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QImage>
#include <QSize>
#include <QWidget>

class VideoSurface : public QWidget
{
    Q_OBJECT

public:
    explicit VideoSurface(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags() );

    QSize sizeHint() const;

signals:
    void mouseMoved(int x, int y);
    void scaleFactorChanged(int);

public slots:
    void present(const QImage &frame);
    void resize(int times);

protected:
    void paintEvent(QPaintEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

private:
    QImage m_Image;
    QSize m_ScaledSize;
    int m_ScaleFactor;
    QPoint m_MousePos;
};

#endif // VIDEOSURFACE_H
