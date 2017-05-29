#include "videosurface.h"

#include <QPainter>
#include <QPaintEvent>

VideoSurface::VideoSurface(QWidget *parent, Qt::WindowFlags f) :
    QWidget( parent, f )
{
}

void VideoSurface::present( const QImage& frame )
{
    m_Image = frame;
    this->update();
}

void VideoSurface::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(event->rect(), m_Image, event->rect());
}
