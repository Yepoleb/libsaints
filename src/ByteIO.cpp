#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>

#include "ByteIO.hpp"



namespace Saints {

qint64 alignAddress(qint64 address, qint64 alignment)
{
    return (address + alignment - 1) / alignment * alignment;
}


ByteReader::ByteReader(QIODevice& stream) :
    m_stream(stream)
{

}

void ByteReader::seek(qint64 pos)
{
    m_stream.seek(pos);
}

qint64 ByteReader::tell() const
{
    return m_stream.pos();
}

void ByteReader::read(char* data, qint64 size)
{
    m_stream.read(data, size);
}

QByteArray ByteReader::read(qint64 size)
{
    return m_stream.read(size);
}

// Additional methods

void ByteReader::align(qint64 alignment)
{
    qint64 current_pos = tell();
    qint64 align_pos = alignAddress(current_pos, alignment);
    if (align_pos != current_pos) {
        seek(align_pos);
    }
}

void ByteReader::ignore(qint64 size)
{
    seek(tell() + size);
}

QString ByteReader::readString(qint64 size)
{
    return QString::fromUtf8(read(size));
}

QString ByteReader::readCString(char delim)
{
    QByteArray buffer;
    char c;
    do {
        m_stream.getChar(&c);
        buffer.append(c);
    } while (c != delim);
    return QString::fromUtf8(buffer);
}

template<typename T>
T ByteReader::read_generic()
{
    T value;
    m_stream.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

qint64 ByteReader::readS64()
{
    return ByteReader::read_generic<qint64>();
}

qint32 ByteReader::readS32()
{
    return ByteReader::read_generic<qint32>();
}

qint16 ByteReader::readS16()
{
    return ByteReader::read_generic<qint16>();
}

qint8 ByteReader::readS8()
{
    return ByteReader::read_generic<qint8>();
}

quint64 ByteReader::readU64()
{
    return ByteReader::read_generic<quint64>();
}

quint32 ByteReader::readU32()
{
    return ByteReader::read_generic<quint32>();
}

quint16 ByteReader::readU16()
{
    return ByteReader::read_generic<quint16>();
}

quint8 ByteReader::readU8()
{
    return ByteReader::read_generic<quint8>();
}

char ByteReader::readChar()
{
    return ByteReader::read_generic<char>();
}

unsigned char ByteReader::readUChar()
{
    return ByteReader::read_generic<unsigned char>();
}

bool ByteReader::readBool()
{
    return ByteReader::read_generic<bool>();
}

float ByteReader::readFloat()
{
    return ByteReader::read_generic<float>();
}

double ByteReader::readDouble()
{
    return ByteReader::read_generic<double>();
}



ByteWriter::ByteWriter(QIODevice& stream) :
    m_stream(stream)
{

}

void ByteWriter::seek(qint64 pos)
{
    m_stream.seek(pos);
}

qint64 ByteWriter::tell() const
{
    return m_stream.pos();
}

void ByteWriter::write(const char* data, qint64 size)
{
    m_stream.write(data, size);
}

void ByteWriter::write(const QByteArray& data)
{
    m_stream.write(data);
}

// Additional methods

void ByteWriter::align(qint64 alignment)
{
    qint64 current_pos = tell();
    qint64 to_align = current_pos % alignment;
    for (qint64 i = 0; i < to_align; i++) {
        m_stream.putChar('\0');
    }
}

void ByteWriter::ignore(qint64 size)
{
    seek(tell() + size);
}

void ByteWriter::writeString(const QString& str)
{
    QByteArray buffer(str.toUtf8());
    buffer.truncate(buffer.size() - 1); // Remove \0 from end
    m_stream.write(buffer);
}

void ByteWriter::writeCString(const QString& str)
{
    m_stream.write(str.toUtf8());
}

template<typename T>
void ByteWriter::write_generic(T value)
{
    m_stream.write(reinterpret_cast<char*>(&value), sizeof(T));
}

void ByteWriter::writeS64(qint64 value)
{
    return ByteWriter::write_generic<qint64>(value);
}

void ByteWriter::writeS32(qint32 value)
{
    return ByteWriter::write_generic<qint32>(value);
}

void ByteWriter::writeS16(qint16 value)
{
    return ByteWriter::write_generic<qint16>(value);
}

void ByteWriter::writeS8(qint8 value)
{
    return ByteWriter::write_generic<qint8>(value);
}

void ByteWriter::writeU64(quint64 value)
{
    return ByteWriter::write_generic<quint64>(value);
}

void ByteWriter::writeU32(quint32 value)
{
    return ByteWriter::write_generic<quint32>(value);
}

void ByteWriter::writeU16(quint16 value)
{
    return ByteWriter::write_generic<quint16>(value);
}

void ByteWriter::writeU8(quint8 value)
{
    return ByteWriter::write_generic<quint8>(value);
}

void ByteWriter::writeChar(char value)
{
    return ByteWriter::write_generic<char>(value);
}

void ByteWriter::writeUChar(unsigned char value)
{
    return ByteWriter::write_generic<unsigned char>(value);
}

void ByteWriter::writeBool(bool value)
{
    return ByteWriter::write_generic<bool>(value);
}

void ByteWriter::writeFloat(float value)
{
    return ByteWriter::write_generic<float>(value);
}

void ByteWriter::writeDouble(double value)
{
    return ByteWriter::write_generic<double>(value);
}

}
