#include <cassert>
#include <QtCore/QtGlobal>
#include <QtCore/QString>
#include <QtCore/QIODevice>

#include "Saints/PegFile.hpp"
#include "Saints/PegEntry.hpp"
#include "Saints/Exceptions.hpp"
#include "ByteIO.hpp"
#include "util.hpp"



namespace Saints {

constexpr quint32 PEG_SIGNATURE = makeFourCC("GEKV");


PegFile::PegFile()
{
    version = 13;
    platform = 0;
    header_size = 0;
    data_size = 0;
    flags = 0;
    alignment = 16;
}

PegFile::PegFile(QIODevice& header_stream) :
    PegFile()
{
    open(header_stream);
}

PegFile::PegFile(QIODevice& header_stream, QIODevice& data_stream) :
    PegFile()
{
    open(header_stream, data_stream);
}

void PegFile::open(QIODevice& header_stream)
{
    readHeader(header_stream);
}

void PegFile::open(QIODevice& header_stream, QIODevice& data_stream)
{
    readHeader(header_stream);
    readData(data_stream);
}

void PegFile::readHeader(QIODevice& stream)
{
    ByteReader reader(stream);

    quint32 signature = reader.readU32();
    version = reader.readS16();
    platform = reader.readS16();
    header_size = reader.readU32();
    data_size = reader.readU32();
    int num_bitmaps = reader.readU16();
    flags = reader.readU16();
    int total_entries = reader.readU16();
    alignment = reader.readU16();

    if (signature != PEG_SIGNATURE) {
        throw FieldError("signature", QString::number(signature, 16));
    }

    if (version != 13 && version != 19) {
        throw FieldError("version", QString::number(version));
    }

    if (num_bitmaps != total_entries) {
        throw ParsingError(
            QString("num_bitmaps (%1) does not match total_entries (%2)")
                .arg(num_bitmaps).arg(total_entries));
    }

    if (version == 19) {
        reader.align(16);
    }

    for (int entry_i = 0; entry_i < total_entries; entry_i++) {
        PegEntry entry(*this);
        if (version == 13) entry.read13(stream);
        if (version == 19) entry.read19(stream);
        entries.push_back(entry);
    }

    for (PegEntry& entry : entries) {
        entry.filename = reader.readCString();
    }
}

void PegFile::writeHeader(QIODevice& stream) const
{
    ByteWriter writer(stream);

    writer.writeU32(PEG_SIGNATURE);
    writer.writeS16(version);
    writer.writeS16(platform);
    writer.writeU32(calcHeaderSize());
    writer.writeU32(calcDataSize());
    writer.writeU16(entries.size());
    writer.writeU16(flags);
    writer.writeU16(entries.size());
    writer.writeU16(alignment);

    if (version == 19) {
        writer.align(16);
    }

    qint64 data_offset = 0;
    for (const PegEntry& entry : entries) {
        data_offset = alignAddress(data_offset, alignment);
        switch (version) {
            case 13: entry.write13(stream, data_offset); break;
            case 19: entry.write19(stream, data_offset); break;
            default: throw ParsingError("Unsupported version");
        }
        data_offset += entry.data.size();
    }

    for (const PegEntry& entry : entries) {
        if (entry.filename.isEmpty()) {
            throw FieldError("filename", "empty");
        }
        writer.writeCString(entry.filename);
    }
}

void PegFile::readData(QIODevice& stream)
{
    ByteReader reader(stream);

    for (PegEntry& entry : entries) {
        reader.seek(entry.offset);
        entry.data = reader.read(entry.data_size);
    }
}

void PegFile::writeData(QIODevice& stream) const
{
    ByteWriter writer(stream);

    for (const PegEntry& entry : entries) {
        writer.align(alignment);
        writer.write(entry.data);
    }
}

int PegFile::calcHeaderSize() const
{
    int total_size = PEGHEADER_BINSIZE;
    total_size += PEGENTRY_BINSIZE * entries.size();
    for (const PegEntry& entry : entries) {
        total_size += entry.filename.size() + 1; // String terminator
    }
    return total_size;
}

qint64 PegFile::calcDataSize() const
{
    qint64 data_size = 0;
    for (const PegEntry& entry : entries) {
        data_size = alignAddress(data_size, alignment);
        data_size += entry.data.size();
    }
    return data_size;
}

int PegFile::getEntryIndex(const QString& name) const
{
    for (int entry_i = 0; entry_i < entries.size(); entry_i++) {
        if (entries[entry_i].filename == name) {
            return entry_i;
        }
    }
    return -1;
}

}
