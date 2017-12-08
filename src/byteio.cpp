#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream> // std::stringbuf

#include "byteio.hpp"

ByteReader::ByteReader(std::istream& stream) :
    m_stream(stream)
{

}

void ByteReader::seek(std::streampos pos)
{
    m_stream.seekg(pos);
}

void ByteReader::seek(std::streamoff off, std::ios::seekdir way)
{
    m_stream.seekg(off, way);
}

std::streampos ByteReader::tell() const
{
    return m_stream.tellg();
}

void ByteReader::read(char* s, std::streamsize n)
{
    m_stream.read(s, n);
}

// Additional methods

void ByteReader::align(std::streamoff alignment)
{
    std::streampos current_pos = tell();
    std::streamoff to_align = current_pos % alignment;
    if (to_align > 0) {
        seek(to_align, std::ios::cur);
    }
}

std::vector<char> ByteReader::readBytes(std::streamsize n)
{
    std::vector<char> data(n);
    m_stream.read(data.data(), n);
    return data;
}

std::string ByteReader::readString(std::streamsize n)
{
    //std::string str(n, '\0');
    std::vector<char> buffer(n);
    m_stream.read(buffer.data(), n);
    std::string str(buffer.data(), n);
    return str;
}

std::string ByteReader::readCString(char delim)
{
    std::stringbuf buffer;
    m_stream.get(buffer, delim);
    m_stream.ignore(1);
    return buffer.str();
}

template<typename T>
T ByteReader::read_generic()
{
    T value;
    m_stream.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

int64_t ByteReader::readS64()
{
    return ByteReader::read_generic<int64_t>();
}

int32_t ByteReader::readS32()
{
    return ByteReader::read_generic<int32_t>();
}

int16_t ByteReader::readS16()
{
    return ByteReader::read_generic<int16_t>();
}

int8_t ByteReader::readS8()
{
    return ByteReader::read_generic<int8_t>();
}

uint64_t ByteReader::readU64()
{
    return ByteReader::read_generic<uint64_t>();
}

uint32_t ByteReader::readU32()
{
    return ByteReader::read_generic<uint32_t>();
}

uint16_t ByteReader::readU16()
{
    return ByteReader::read_generic<uint16_t>();
}

uint8_t ByteReader::readU8()
{
    return ByteReader::read_generic<uint8_t>();
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



ByteWriter::ByteWriter(std::ostream& stream) :
    m_stream(stream)
{

}

void ByteWriter::seek(std::streampos pos)
{
    m_stream.seekp(pos);
}

void ByteWriter::seek(std::streamoff off, std::ios::seekdir way)
{
    m_stream.seekp(off, way);
}

std::streampos ByteWriter::tell() const
{
    return m_stream.tellp();
}

void ByteWriter::write(const char* s, std::streamsize n)
{
    m_stream.write(s, n);
}

// Additional methods

void ByteWriter::align(std::streamoff alignment)
{
    std::streampos current_pos = tell();
    std::streamoff to_align = current_pos % alignment;
    for (std::streamoff i = 0; i < to_align; i++) {
        m_stream.put('\0');
    }
}

void ByteWriter::writeBytes(std::vector<char> data)
{
    m_stream.write(data.data(), data.size());
}

void ByteWriter::writeString(std::string str)
{
    m_stream.write(str.data(), str.size());
}

void ByteWriter::writeCString(std::string str)
{
    m_stream.write(str.c_str(), str.size() + 1);
}

template<typename T>
void ByteWriter::write_generic(T value)
{
    m_stream.write(reinterpret_cast<char*>(&value), sizeof(T));
}

void ByteWriter::writeS64(int64_t value)
{
    return ByteWriter::write_generic<int64_t>(value);
}

void ByteWriter::writeS32(int32_t value)
{
    return ByteWriter::write_generic<int32_t>(value);
}

void ByteWriter::writeS16(int16_t value)
{
    return ByteWriter::write_generic<int16_t>(value);
}

void ByteWriter::writeS8(int8_t value)
{
    return ByteWriter::write_generic<int8_t>(value);
}

void ByteWriter::writeU64(uint64_t value)
{
    return ByteWriter::write_generic<uint64_t>(value);
}

void ByteWriter::writeU32(uint32_t value)
{
    return ByteWriter::write_generic<uint32_t>(value);
}

void ByteWriter::writeU16(uint16_t value)
{
    return ByteWriter::write_generic<uint16_t>(value);
}

void ByteWriter::writeU8(uint8_t value)
{
    return ByteWriter::write_generic<uint8_t>(value);
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
