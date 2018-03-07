#include <cassert>
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtCore/QVector>
#include <QtCore/QBuffer>

#include "Saints/Packfile.hpp"
#include "Saints/PackfileEntry.hpp"
#include "Saints/Exceptions.hpp"
#include "ByteIO.hpp"
#include "util.hpp"



namespace Saints {

constexpr quint32 PACKFILE_DESCRIPTOR = 0x51890ACE;

constexpr qint64 PACKFILE_HEADER_SIZE_6 = 380;
constexpr qint64 PACKFILE_HEADER_SIZE_10 = 40;
constexpr qint64 PACKFILE_HEADER_SIZE_17 = 120;



Packfile::Packfile() :
    m_stream(nullptr)
{

}

Packfile::Packfile(QIODevice& stream) :
    m_stream(&stream)
{
    load();
}

void Packfile::open(QIODevice& stream)
{
    m_stream = &stream;
    load();
}

void Packfile::load()
{
    assert(m_stream);
    ByteReader reader(*m_stream);

    quint32 descriptor = reader.readU32();

    if (descriptor != PACKFILE_DESCRIPTOR) {
        throw FieldError("descriptor", QString::number(descriptor, 16));
    }

    m_version = reader.readU32();

    switch (m_version) {
        case 6: loadHeader6(); break;
        case 10: loadHeader10(); break;
        case 17: loadHeader17(); break;
        default: throw ParsingError("Unsupported version");
    }
}

void Packfile::loadHeader6()
{
    assert(m_stream);
    ByteReader reader(*m_stream);

    reader.ignore(0x144); // Skip runtime variables
    m_flags = reader.readU32();
    reader.ignore(4); // Ignore sector value

    int num_files = reader.readU32();
    m_file_size = reader.readU32();
    m_dir_size = reader.readU32();
    m_filename_size = reader.readU32();

    m_data_size = reader.readU32();
    m_compressed_data_size = reader.readU32();

    m_data_offset = 0;
    m_timestamp = 0;

    reader.seek(getEntriesOffset());

    QVector<qint64> filename_offsets;
    for (int i = 0; i < num_files; i++) {
        PackfileEntry entry(*this);
        filename_offsets.push_back(reader.readU32());
        entry.load6(*m_stream);
        m_entries.push_back(entry);
    }

    qint64 entryname_offset = getEntryNamesOffset();
    for (int i = 0; i < num_files; i++) {
        reader.seek(entryname_offset + filename_offsets[i]);
        m_entries[i].m_filename = reader.readCString();
    }
}

void Packfile::loadHeader10()
{
    assert(m_stream);
    ByteReader reader(*m_stream);

    m_header_checksum = reader.readU32();
    m_file_size = reader.readU32();

    m_flags = reader.readU32();
    int num_files = reader.readU32();
    m_dir_size = reader.readU32();
    m_filename_size = reader.readU32();

    m_data_size = reader.readU32();
    m_compressed_data_size = reader.readU32();

    m_data_offset = 0;
    m_timestamp = 0;

    QVector<qint64> filename_offsets;
    for (int i = 0; i < num_files; i++) {
        PackfileEntry entry(*this);
        filename_offsets.push_back(reader.readU64());
        entry.load10(*m_stream);
        m_entries.push_back(entry);
    }

    qint64 entryname_offset = getEntryNamesOffset();
    for (int i = 0; i < num_files; i++) {
        reader.seek(entryname_offset + filename_offsets[i]);
        m_entries[i].m_filename = reader.readCString();
    }
}

void Packfile::loadHeader17()
{
    assert(m_stream);
    ByteReader reader(*m_stream);

    m_header_checksum = reader.readU32();

    m_flags = reader.readU32();
    int num_files = reader.readU32();
    int num_paths = reader.readU32();
    m_dir_size = reader.readU32();
    m_filename_size = reader.readU32();
    m_file_size = reader.readU64();

    m_data_size = reader.readU64();
    m_compressed_data_size = reader.readU64();

    m_timestamp = reader.readU64();
    m_data_offset = reader.readU64();

    reader.seek(getEntriesOffset());

    QVector<qint64> filename_offsets;
    QVector<qint64> filepath_offsets;
    for (int i = 0; i < num_files; i++) {
        PackfileEntry entry(*this);
        filename_offsets.push_back(reader.readU64());
        filepath_offsets.push_back(reader.readU64());
        entry.load17(*m_stream);
        m_entries.push_back(entry);
    }

    reader.seek(getEntryNamesOffset());

    // Reads filenames into a buffer for faster seeking
    // Currently not functional
//    QByteArray filename_data(reader.read(m_filename_size));
//    QBuffer filename_buffer(&filename_data);
//    filename_buffer.open(QIODevice::ReadOnly);
//    ByteReader filename_reader(filename_buffer);
    qint64 names_offset = getEntryNamesOffset();
    for (int i = 0; i < num_files; i++) {
        reader.seek(names_offset + filename_offsets[i]);
        m_entries[i].m_filename = reader.readCString();
        reader.seek(names_offset + filepath_offsets[i]);
        m_entries[i].m_filepath = reader.readCString();
    }
}

void Packfile::loadFileData(PackfileEntry& entry)
{
    assert(m_stream);
    if (entry.m_is_cached) {
        return;
    }

    if ((m_flags & Compressed) && (m_flags & Condensed)) {
        m_stream->seek(getDataOffset());
        QByteArray decompress_cache(
            decompressStream(*m_stream)
        );
        for (PackfileEntry& cond_entry : m_entries) {
            if (cond_entry.m_is_cached) {
                continue;
            }
            cond_entry.m_data_cache = decompress_cache.mid(
                cond_entry.m_start, cond_entry.m_size);
            cond_entry.m_is_cached = true;
        }
    } else {

        qint64 entry_offset = getDataOffset() + entry.m_start;
        m_stream->seek(entry_offset);

        if (entry.m_flags & Compressed) {
            entry.m_data_cache = decompressStream(*m_stream);
        } else {
            entry.m_data_cache = m_stream->read(entry.m_size);
        }

        entry.m_is_cached = true;
    }
}

PackfileEntry* Packfile::getEntryByFilename(const QString& filename)
{
    for (PackfileEntry& entry : m_entries) {
        if (entry.m_filename == filename) {
            return &entry;
        }
    }
    return nullptr;
}

const PackfileEntry* Packfile::getEntryByFilename(const QString& filename) const
{
    for (const PackfileEntry& entry : m_entries) {
        if (entry.m_filename == filename) {
            return &entry;
        }
    }
    return nullptr;
}

qint64 Packfile::getEntriesOffset()
{
    switch (m_version) {
        case 6: return alignAddress(PACKFILE_HEADER_SIZE_6, 2048);
        case 10: return PACKFILE_HEADER_SIZE_10;
        case 17: return PACKFILE_HEADER_SIZE_17;
        default: throw ParsingError("Unsupported version");
    }
}

qint64 Packfile::getEntryNamesOffset()
{
    switch (m_version) {
        case 6: return alignAddress(getEntriesOffset() + m_dir_size, 2048);
        case 10: return getEntriesOffset() + m_dir_size;
        case 17: return getEntriesOffset() + m_dir_size;
        default: throw ParsingError("Unsupported version");
    }
}

qint64 Packfile::getDataOffset()
{
    switch (m_version) {
        case 6: return alignAddress(getEntryNamesOffset() + m_filename_size, 2048);
        case 10: return getEntryNamesOffset() + m_filename_size;
        case 17: return m_data_offset;
        default: throw ParsingError("Unsupported version");
    }
}

QByteArray Packfile::decompressStream(QIODevice& stream)
{
    switch (m_version) {
        case 6:
        case 10: return decompressZLIB(stream);
        case 17: return decompressLZ4(stream);
        default: throw ParsingError("Unsupported version");
    }
}

PackfileEntry& Packfile::getEntry(int index) {return m_entries[index];}
const PackfileEntry& Packfile::getEntry(int index) const {return m_entries[index];}
QVector<PackfileEntry>& Packfile::getEntries() {return m_entries;}
const QVector<PackfileEntry>& Packfile::getEntries() const {return m_entries;}
int Packfile::getEntriesCount() const {return m_entries.size();}
int Packfile::getVersion() const {return m_version;}
void Packfile::setVersion(int value) {m_version = value;}
int Packfile::getFlags() const {return m_flags;}
void Packfile::setFlags(int value) {m_flags = value;}
qint64 Packfile::getTimestamp() const {return m_timestamp;}
void Packfile::setTimestamp(qint64 value) {m_timestamp = value;}

}
