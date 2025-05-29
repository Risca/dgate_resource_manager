#include "videosurface.h"

#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSize>

namespace {

QScrollArea *locateArea(QObject *p)
{
    if (!p)
        return nullptr;

    QScrollArea *area = nullptr;
    do {
        area = dynamic_cast<QScrollArea*>(p);
        if (area)
            break;
    } while ((p = p->parent()) != nullptr);
    return area;
}

} // anonymous namespace

VideoSurface::VideoSurface(QWidget *parent, Qt::WindowFlags f) :
    QWidget( parent, f ),
    m_ScaleFactor(1),
    m_ScrollArea(locateArea(parent))
{
    setMouseTracking(true);
}

QSize VideoSurface::sizeHint() const
{
    return m_ScaledSize;
}

void VideoSurface::wheelEvent(QWheelEvent *e)
{
    if (!m_ScrollArea)
        m_ScrollArea = locateArea(this->parent());

    int value;
    switch (e->modifiers()) {
        case Qt::ShiftModifier: {
            if (!m_ScrollArea)
                break;

            auto *bar = m_ScrollArea->horizontalScrollBar();
            if (!bar)
                break;

            value = bar->value();
            if (e->angleDelta().y() < 0)
                value += bar->singleStep();
            else
                value -= bar->singleStep();
            bar->setValue(value);

            e->accept();
            return;
        }

        case Qt::ControlModifier:
            value = m_ScaleFactor;
            if (e->angleDelta().y() < 0)
                value--;
            else
                value++;
            value = std::clamp(value, 1, 20);
            this->resize(value);
            break;
        default:
            break;
    }

    e->ignore();
}

void VideoSurface::present( const QImage& frame )
{
    m_Image = frame;
    m_ScaledSize = m_Image.size() * m_ScaleFactor;
    this->setMinimumSize(m_ScaledSize);
    this->update();
}

void VideoSurface::resize(int times)
{
    if (times != m_ScaleFactor) {
        m_ScaleFactor = times;
        present(m_Image);
        emit scaleFactorChanged(m_ScaleFactor);
    }
}

void VideoSurface::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const int w = m_Image.width();
    const int h = m_Image.height();
    const int stride = m_Image.bytesPerLine();
    const QVector<QRgb> palette = m_Image.colorTable();
    const uchar *img = m_Image.constBits();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = img[y*stride + x];
            if (m_ScaleFactor == 1) {
                painter.setPen(palette[idx]);
            }
            else {
                painter.setBrush(QBrush(palette[idx]));
            }
            painter.drawRect(x * m_ScaleFactor, y * m_ScaleFactor, m_ScaleFactor, m_ScaleFactor);
        }
    }
    if (m_MousePos.x() < m_ScaledSize.width() && m_MousePos.y() < m_ScaledSize.height()) {
        int x = (m_MousePos.x() / m_ScaleFactor) * m_ScaleFactor;
        int y = (m_MousePos.y() / m_ScaleFactor) * m_ScaleFactor;
        painter.setPen(Qt::red);
        painter.setBrush(QBrush());
        painter.drawRect(x, y, m_ScaleFactor, m_ScaleFactor);
    }
}

void VideoSurface::mouseMoveEvent(QMouseEvent *event)
{
    if (m_Image.isNull()) {
        return;
    }

    m_MousePos = event->pos();
    this->update();
    emit mouseMoved(m_MousePos.x() / m_ScaleFactor, m_MousePos.y() / m_ScaleFactor);
    event->accept();
}
