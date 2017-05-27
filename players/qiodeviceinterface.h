#ifndef QIODEVICEINTERFACE_H
#define QIODEVICEINTERFACE_H

#include <flic/flic.h>

class QIODevice;

namespace flic {

class QIODeviceInterface : public FileInterface
{
public:
    QIODeviceInterface(QIODevice &device);

    // Returns true if we can read/write bytes from/into the file
    bool ok() const;

    // Current position in the file
    size_t tell();

    // Jump to the given position in the file
    void seek(size_t absPos);

    // Returns the next byte in the file or 0 if ok() = false
    uint8_t read8();

    // Writes one byte in the file (or do nothing if ok() = false)
    void write8(uint8_t value);

private:
    QIODevice &m_Dev;
    bool m_Ok;
};

}

#endif // QIODEVICEINTERFACE_H
