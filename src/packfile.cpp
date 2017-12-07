#include <stdexcept>
#include <cassert>
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtCore/QVector>

#include "byteio.hpp"
#include "packfile.hpp"
#include "util.hpp"



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
    m_sector = reader.readU32();

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

    if ((m_flags & PFF_COMPRESSED) && (m_flags & PFF_CONDENSED)) {
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

        if (entry.m_flags & PFEP_COMPRESSED) {
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

void Packfile::setDescriptor(quint32 value) {m_descriptor = value;}
quint32 Packfile::getDescriptor() const {return m_descriptor;}
void Packfile::setVersion(int value) {m_version = value;}
int Packfile::getVersion() const {return m_version;}
void Packfile::setHeaderChecksum(quint32 value) {m_header_checksum = value;}
quint32 Packfile::getHeaderChecksum() const {return m_header_checksum;}
void Packfile::setFileSize(qint64 value) {m_file_size = value;}
qint64 Packfile::getFileSize() const {return m_file_size;}
void Packfile::setFlags(int value) {m_flags = value;}
int Packfile::getFlags() const {return m_flags;}
void Packfile::setDirSize(qint64 value) {m_dir_size = value;}
qint64 Packfile::getDirSize() const {return m_dir_size;}
void Packfile::setFilenameSize(qint64 value) {m_filename_size = value;}
qint64 Packfile::getFilenameSize() const {return m_filename_size;}
void Packfile::setDataSize(qint64 value) {m_data_size = value;}
qint64 Packfile::getDataSize() const {return m_data_size;}
void Packfile::setCompressedDataSize(qint64 value) {m_compressed_data_size = value;}
qint64 Packfile::getCompressedDataSize() const {return m_compressed_data_size;}



PackfileEntry::PackfileEntry() :
    m_packfile(nullptr),
    m_is_cached(false)
{

}

PackfileEntry::PackfileEntry(Packfile& packfile) :
    m_packfile(&packfile),
    m_is_cached(false)
{

}

void PackfileEntry::load6(QIODevice& stream)
{
    ByteReader reader(stream);

    m_start = reader.readU32();
    m_size = reader.readU32();
    m_compressed_size = reader.readU32();
    m_flags = 0;
    m_alignment = 0;
    reader.ignore(4); // parent pointer, we have our own
}

void PackfileEntry::load10(QIODevice& stream)
{
    ByteReader reader(stream);

    m_start = reader.readU32();
    m_size = reader.readU32();
    m_compressed_size = reader.readU32();
    m_flags = reader.readU16();
    m_alignment = reader.readU16();
}

QByteArray& PackfileEntry::getData()
{
    if (!m_is_cached) {
        m_packfile->loadFileData(*this);
    }

    return m_data_cache;
}



void PackfileEntry::setFilename(const QString& value) {m_filename = value;}
QString PackfileEntry::getFilename() const {return m_filename;}
void PackfileEntry::setStart(qint64 value) {m_start = value;}
qint64 PackfileEntry::getStart() const {return m_start;}
void PackfileEntry::setSize(qint64 value) {m_size = value;}
qint64 PackfileEntry::getSize() const {return m_size;}
void PackfileEntry::setCompressedSize(qint64 value) {m_compressed_size = value;}
qint64 PackfileEntry::getCompressedSize() const {return m_compressed_size;}
void PackfileEntry::setFlags(int value) {m_flags = value;}
int PackfileEntry::getFlags() const {return m_flags;}
void PackfileEntry::setAlignment(int value) {m_alignment = value;}
int PackfileEntry::getAlignment() const {return m_alignment;}

} // namespace saints
