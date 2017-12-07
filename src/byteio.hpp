#pragma once

#include "QtCore/QtGlobal"
#include "QtCore/QByteArray"
#include "QtCore/QString"
#include "QtCore/QIODevice"



namespace Saints {

qint64 alignAddress(qint64 address, qint64 alignment);


class ByteReader
{
public:
    explicit ByteReader(QIODevice& stream);

    void seek(qint64 pos);
    qint64 tell() const;
    void read(char* data, qint64 size);
    QByteArray read(qint64 size);
    void align(qint64 alignment);
    void ignore(qint64 size);
    QString readString(qint64 size);
    QString readCString(char delim='\0');

    template<typename T>
    T read_generic();
    qint64 readS64();
    qint32 readS32();
    qint16 readS16();
    qint8 readS8();
    quint64 readU64();
    quint32 readU32();
    quint16 readU16();
    quint8 readU8();
    char readChar();
    unsigned char readUChar();
    bool readBool();
    float readFloat();
    double readDouble();

private:
    QIODevice& m_stream;
};


class ByteWriter
{
public:
    explicit ByteWriter(QIODevice& stream);

    void seek(qint64 pos);
    qint64 tell() const;
    void write(const char* data, qint64 size);
    void write(const QByteArray& data);
    void align(qint64 alignment);
    void ignore(qint64 size);
    void writeString(const QString& str);
    void writeCString(const QString& str);

    template<typename T>
    void write_generic(T value);
    void writeS64(qint64 value);
    void writeS32(qint32 value);
    void writeS16(qint16 value);
    void writeS8(qint8 value);
    void writeU64(quint64 value);
    void writeU32(quint32 value);
    void writeU16(quint16 value);
    void writeU8(quint8 value);
    void writeChar(char value);
    void writeUChar(unsigned char value);
    void writeBool(bool value);
    void writeFloat(float value);
    void writeDouble(double value);

private:
    QIODevice& m_stream;
};

} // namespace saints
