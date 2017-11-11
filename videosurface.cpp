#include "videosurface.h"

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

VideoSurface::VideoSurface(QWidget *parent, Qt::WindowFlags f) :
    QWidget( parent, f ),
    m_ScaleFactor(1)
{
    setMouseTracking(true);
}

QSize VideoSurface::sizeHint() const
{
    return m_Image.size();
}

void VideoSurface::present( const QImage& frame )
{
    m_Image = frame.scaled(frame.size() * m_ScaleFactor);
    this->setMinimumSize(m_Image.size());
    this->update();
}

void VideoSurface::resize(int times)
{
    QSize size = m_Image.size();
    size *= times;
    size /= m_ScaleFactor;
    m_Image = m_Image.scaled(size);
    m_ScaleFactor = times;
    this->setMinimumSize(size);
    this->update();
}

void VideoSurface::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(event->rect(), m_Image, event->rect());
}

void VideoSurface::mouseMoveEvent(QMouseEvent *event)
{
    if (m_Image.isNull()) {
        return;
    }

    int x = event->pos().x();
    int y = event->pos().y();
    if (x <= m_Image.width() && y <= m_Image.height()) {
        x /= m_ScaleFactor;
        y /= m_ScaleFactor;
        emit mouseMoved(x, y);
    }
}
