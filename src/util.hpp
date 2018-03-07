#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QIODevice>

#define ARRAYSIZE(a) ((int)(sizeof(a) / sizeof(a[0])))



namespace Saints {

constexpr quint32 makeFourCC(const char* four_c)
{
    return (four_c[3] << 24) | (four_c[2] << 16) | (four_c[1] << 8) | (four_c[0] << 0);
}
constexpr qint64 FOURCC_SIZE = 4;

constexpr int bitmask(int bits)
{
    return (1 << bits) - 1;
}

constexpr qint64 alignAddress(qint64 address, qint64 alignment)
{
    return (address + alignment - 1) / alignment * alignment;
}

QByteArray decompressZLIB(QIODevice& stream);
QByteArray decompressLZ4(QIODevice& stream);

}
