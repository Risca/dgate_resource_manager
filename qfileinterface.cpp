#include "qfileinterface.h"

#include <QFile>

flic::QFileInterface::QFileInterface(QFile &file) : m_File(file), m_Ok(true)
{
}

bool flic::QFileInterface::ok() const
{
    return m_Ok;
}

size_t flic::QFileInterface::tell()
{
    return m_File.pos();
}

void flic::QFileInterface::seek(size_t absPos)
{
    m_File.seek(absPos);
}

uint8_t flic::QFileInterface::read8()
{
    uint8_t data;
    if (m_File.read((char*)&data, 1) < 1) {
        m_Ok = false;
        data = 0;
    }
    return data;
}

void flic::QFileInterface::write8(uint8_t value)
{
    if (m_File.write((char*)&value, 1) < 1) {
        m_Ok = false;
    }
}
