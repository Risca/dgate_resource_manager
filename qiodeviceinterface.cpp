#include "qiodeviceinterface.h"

#include <QFile>

flic::QIODeviceInterface::QIODeviceInterface(QIODevice &device) : m_Dev(device), m_Ok(true)
{
}

bool flic::QIODeviceInterface::ok() const
{
    return m_Ok;
}

size_t flic::QIODeviceInterface::tell()
{
    return m_Dev.pos();
}

void flic::QIODeviceInterface::seek(size_t absPos)
{
    m_Dev.seek(absPos);
}

uint8_t flic::QIODeviceInterface::read8()
{
    uint8_t data;
    if (m_Dev.read((char*)&data, 1) < 1) {
        m_Ok = false;
        data = 0;
    }
    return data;
}

void flic::QIODeviceInterface::write8(uint8_t value)
{
    if (m_Dev.write((char*)&value, 1) < 1) {
        m_Ok = false;
    }
}
