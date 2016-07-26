#include "videosurface.h"

#include <QPainter>
#include <QPaintEvent>

VideoSurface::VideoSurface(QWidget *parent, Qt::WindowFlags f) :
    QWidget( parent, f )
{
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void VideoSurface::present( const QImage& frame )
{
    m_Image = frame;
    this->setMinimumSize( m_Image.size() );
    this->resize( m_Image.size() );
    this->update();
}

void VideoSurface::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(event->rect(), m_Image, event->rect());
}
