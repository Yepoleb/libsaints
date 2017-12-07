#include <stdexcept>
#include <cassert>
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtCore/QVector>

#include "ByteIO.hpp"
#include "util.hpp"
#include "Saints/PackfileEntry.hpp"
#include "Saints/Packfile.hpp"



namespace Saints {

constexpr qint64 PACKFILE_HEADER_SIZE_10 = 40;
constexpr qint64 PACKFILE_HEADER_SIZE_6 = 380;



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
    ByteReader reader(*m_stream);

    m_descriptor = reader.readU32();
    m_version = reader.readU32();

    if (m_version == 6) {
        loadHeader6();
    } else if (m_version == 10) {
        loadHeader10();
    } else {
        throw std::runtime_error("Unsupported version");
    }
}

void Packfile::loadHeader6()
{
    ByteReader reader(*m_stream);

    reader.ignore(0x144); // Skip runtime variables
    m_flags = reader.readU32();
    reader.ignore(4); // Ignore sector value

    m_num_files = reader.readU32();
    m_file_size = reader.readU32();
    m_dir_size = reader.readU32();
    m_filename_size = reader.readU32();

    m_data_size = reader.readU32();
    m_compressed_data_size = reader.readU32();

    reader.align(2048);

    QVector<qint64> filename_offsets;
    for (int i = 0; i < m_num_files; i++) {
        PackfileEntry entry(*this);
        filename_offsets.push_back(reader.readU32());
        entry.load6(*m_stream);
        m_entries.push_back(entry);
    }

    qint64 entryname_offset = getEntryNamesOffset();
    for (int i = 0; i < m_num_files; i++) {
        reader.seek(entryname_offset + filename_offsets[i]);
        m_entries[i].m_filename = reader.readCString();
    }
}

void Packfile::loadHeader10()
{
    ByteReader reader(*m_stream);

    m_header_checksum = reader.readU32();
    m_file_size = reader.readU32();

    m_flags = reader.readU32();
    m_num_files = reader.readU32();
    m_dir_size = reader.readU32();
    m_filename_size = reader.readU32();

    m_data_size = reader.readU32();
    m_compressed_data_size = reader.readU32();

    QVector<qint64> filename_offsets;
    for (int i = 0; i < m_num_files; i++) {
        PackfileEntry entry(*this);
        filename_offsets.push_back(reader.readU64());
        entry.load10(*m_stream);
        m_entries.push_back(entry);
    }

    qint64 entryname_offset = getEntryNamesOffset();
    for (int i = 0; i < m_num_files; i++) {
        reader.seek(entryname_offset + filename_offsets[i]);
        m_entries[i].m_filename = reader.readCString();
    }
}

void Packfile::loadFileData(PackfileEntry& entry)
{
    if (entry.m_is_cached) {
        return;
    }

    if ((m_flags & Compressed) && (m_flags & Condensed)) {
        m_stream->seek(getDataOffset());
        QByteArray decompress_cache(
            decompress_stream(*m_stream, m_data_size)
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
            entry.m_data_cache = decompress_stream(*m_stream, entry.m_size);
        } else {
            entry.m_data_cache = m_stream->read(entry.m_size);
        }

        entry.m_is_cached = true;
    }
}

PackfileEntry& Packfile::getEntryByFilename(const QString& filename)
{
    for (PackfileEntry& entry : m_entries) {
        if (entry.m_filename == filename) {
            return entry;
        }
    }
    throw std::runtime_error("No file found");
}

const PackfileEntry& Packfile::getEntryByFilename(const QString& filename) const
{
    for (const PackfileEntry& entry : m_entries) {
        if (entry.m_filename == filename) {
            return entry;
        }
    }
    throw std::runtime_error("No file found");
}

qint64 Packfile::getEntriesOffset()
{
    if (m_version == 6) {
        return alignAddress(PACKFILE_HEADER_SIZE_6, 2048);
    } else if (m_version) {
        return PACKFILE_HEADER_SIZE_10;
    } else {
        throw std::runtime_error("Unknown Version");
    }
}

qint64 Packfile::getEntryNamesOffset()
{
    if (m_version == 6) {
        return alignAddress(getEntriesOffset() + m_dir_size, 2048);
    } else if (m_version == 10) {
        return getEntriesOffset() + m_dir_size;
    } else {
        throw std::runtime_error("Unknown Version");
    }
}

qint64 Packfile::getDataOffset()
{
    if (m_version == 6) {
        return alignAddress(getEntryNamesOffset() + m_filename_size, 2048);
    } else if (m_version == 10) {
        return getEntryNamesOffset() + m_filename_size;
    } else {
        throw std::runtime_error("Unknown Version");
    }
}

PackfileEntry& Packfile::getEntry(int index) {return m_entries[index];}
const PackfileEntry& Packfile::getEntry(int index) const {return m_entries[index];}
QVector<PackfileEntry>& Packfile::getEntries() {return m_entries;}
const QVector<PackfileEntry>& Packfile::getEntries() const {return m_entries;}
int Packfile::getEntriesCount() const {return m_entries.size();}
void Packfile::setVersion(int value) {m_version = value;}
int Packfile::getVersion() const {return m_version;}
void Packfile::setFlags(int value) {m_flags = value;}
int Packfile::getFlags() const {return m_flags;}

}
