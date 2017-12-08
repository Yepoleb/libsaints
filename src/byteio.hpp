#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>

class ByteReader
{
public:
    explicit ByteReader(std::istream& stream);

    void seek(std::streampos pos);
    void seek(std::streamoff off, std::ios::seekdir way);
    std::streampos tell() const;
    void read(char* s, std::streamsize n);
    void align(std::streamoff alignment);
    std::vector<char> readBytes(std::streamsize n);
    std::string readString(std::streamsize n);
    std::string readCString(char delim='\0');

    template<typename T>
    T read_generic();
    int64_t readS64();
    int32_t readS32();
    int16_t readS16();
    int8_t readS8();
    uint64_t readU64();
    uint32_t readU32();
    uint16_t readU16();
    uint8_t readU8();
    char readChar();
    unsigned char readUChar();
    bool readBool();
    float readFloat();
    double readDouble();

private:
    std::istream& m_stream;
};


class ByteWriter
{
public:
    explicit ByteWriter(std::ostream& stream);

    void seek(std::streampos pos);
    void seek(std::streamoff off, std::ios::seekdir way);
    std::streampos tell() const;
    void write(const char* s, std::streamsize n);
    void align(std::streamoff alignment);
    void writeBytes(std::vector<char> data);
    void writeString(std::string str);
    void writeCString(std::string str);

    template<typename T>
    void write_generic(T value);
    void writeS64(int64_t value);
    void writeS32(int32_t value);
    void writeS16(int16_t value);
    void writeS8(int8_t value);
    void writeU64(uint64_t value);
    void writeU32(uint32_t value);
    void writeU16(uint16_t value);
    void writeU8(uint8_t value);
    void writeChar(char value);
    void writeUChar(unsigned char value);
    void writeBool(bool value);
    void writeFloat(float value);
    void writeDouble(double value);

private:
    std::ostream& m_stream;
};
