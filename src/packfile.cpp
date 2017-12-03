#include <stdexcept>
#include <cassert>
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtCore/QVector>

#include "zlib.h"

#include "byteio.hpp"
#include "packfile.hpp"



namespace Saints {

constexpr qint64 PACKFILE_HEADER_SIZE = 40;
constexpr qint64 ZLIB_CHUNK_SIZE = 16384;



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
    m_header_checksum = reader.readU32();
    m_file_size = reader.readU32();

    m_flags = reader.readU32();
    m_num_files = reader.readU32();
    m_dir_size = reader.readU32();
    m_filename_size = reader.readU32();

    m_data_size = reader.readU32();
    m_compressed_data_size = reader.readU32();

    for (int i = 0; i < m_num_files; i++) {
        PackfileEntry entry(this, *m_stream);
        m_entries.push_back(entry);
    }

    qint64 dir_offset = PACKFILE_HEADER_SIZE + m_dir_size;

    for (PackfileEntry& entry : m_entries) {
        reader.seek(dir_offset + entry.m_filename_offset);
        entry.m_filename = reader.readCString();
    }
}

QByteArray decompress_stream(QIODevice& stream, qint64 out_size)
{
    QByteArray in_chunk(ZLIB_CHUNK_SIZE, 0);
    QByteArray data_decompressed(out_size, 0);
    qint64 out_pos = 0;

    int ret;
    z_stream zstrm;
    zstrm.zalloc = Z_NULL;
    zstrm.zfree = Z_NULL;
    zstrm.opaque = Z_NULL;
    zstrm.avail_in = 0;
    zstrm.next_in = Z_NULL;
    ret = inflateInit(&zstrm);

    if (ret != Z_OK) {
        throw std::runtime_error("Failed to initialize");
    }

    do {
        qint64 bytes_read = stream.read(in_chunk.data(), ZLIB_CHUNK_SIZE);
        if (bytes_read == -1) {
            inflateEnd(&zstrm);
            throw std::runtime_error("Error reading file");
        }
        if (bytes_read == 0) {
            break;
        }
        zstrm.avail_in = bytes_read;
        zstrm.next_in = reinterpret_cast<unsigned char*>(in_chunk.data());
        zstrm.avail_out = out_size - out_pos;
        zstrm.next_out = reinterpret_cast<unsigned char*>(
            data_decompressed.data() + out_pos);
        ret = inflate(&zstrm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);
        switch (ret) {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
            inflateEnd(&zstrm);
            throw std::runtime_error("Invalid compression data");
        case Z_MEM_ERROR:
            inflateEnd(&zstrm);
            throw std::bad_alloc();
        }
        out_pos -= zstrm.avail_out;
        if (out_pos > out_size) {
            inflateEnd(&zstrm);
            throw std::runtime_error("Invalid data size");
        }
    } while (ret != Z_STREAM_END);

    inflateEnd(&zstrm);
    return data_decompressed;
}

void Packfile::loadFileData(PackfileEntry& entry)
{
    if (entry.m_is_cached) {
        return;
    }

    qint64 data_offset = PACKFILE_HEADER_SIZE + m_dir_size + m_filename_size;

    if ((m_flags & PFF_COMPRESSED) && (m_flags & PFF_CONDENSED)) {
        m_stream->seek(data_offset);
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

        qint64 entry_offset = data_offset + entry.m_start;
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

PackfileEntry& Packfile::getEntry(int index) {return m_entries[index];}
QVector<PackfileEntry>& Packfile::getEntries() {return m_entries;}
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

PackfileEntry::PackfileEntry(Packfile* packfile, QIODevice& stream) :
    m_packfile(packfile),
    m_is_cached(false)
{
    load(stream);
}

void PackfileEntry::load(QIODevice& stream)
{
    ByteReader reader(stream);

    m_filename_offset = reader.readU64();
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
void PackfileEntry::setFilenameOffset(qint64 value) {m_filename_offset = value;}
qint64 PackfileEntry::getFilenameOffset() const {return m_filename_offset;}
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
