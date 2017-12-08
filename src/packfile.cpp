#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cassert>

#include "zlib.h"

#include "byteio.hpp"
#include "packfile.hpp"



constexpr size_t PACKFILE_HEADER_SIZE = 40;
constexpr size_t ZLIB_CHUNK_SIZE = 16384;



Packfile::Packfile() :
    m_stream(nullptr)
{

}

Packfile::Packfile(std::istream& stream) :
    m_stream(&stream)
{
    load(stream);
}

void Packfile::load(std::istream& stream)
{
    ByteReader reader(stream);

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

    for (uint32_t i = 0; i < m_num_files; i++) {
        PackfileEntry entry(*this, stream);
        m_entries.push_back(entry);
    }

    std::streamoff dir_offset = PACKFILE_HEADER_SIZE + m_dir_size;

    for (PackfileEntry& entry : m_entries) {
        reader.seek(dir_offset + entry.m_filename_offset, std::ios::beg);
        entry.m_filename = reader.readCString();
    }
}

std::vector<char> decompress_stream(std::istream& stream, size_t out_size)
{
    std::vector<char> in_chunk(ZLIB_CHUNK_SIZE);
    std::vector<char> data_decompressed(out_size);
    size_t out_free = out_size;

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
        stream.read(in_chunk.data(), ZLIB_CHUNK_SIZE);
        zstrm.avail_in = stream.gcount();
        if (stream.bad()) {
            inflateEnd(&zstrm);
            throw std::runtime_error("Error reading file");
        }
        if (zstrm.avail_in == 0) {
            break;
        }
        zstrm.next_in = reinterpret_cast<unsigned char*>(in_chunk.data());
        zstrm.avail_out = out_free;
        zstrm.next_out = reinterpret_cast<unsigned char*>(data_decompressed.data());
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
        out_free -= zstrm.avail_out;
        if (out_free > out_size) { // overflow
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

    std::streamoff data_offset = PACKFILE_HEADER_SIZE + m_dir_size + m_filename_size;

    if ((m_flags & PFF_COMPRESSED) && (m_flags & PFF_CONDENSED)) {
        m_stream->seekg(data_offset, std::ios::beg);
        std::vector<char> decompress_cache(
            decompress_stream(*m_stream, m_data_size)
        );
        for (PackfileEntry& cond_entry : m_entries) {
            if (cond_entry.m_is_cached) {
                continue;
            }
            cond_entry.m_data_cache = std::vector<char>(
                decompress_cache.begin() + cond_entry.m_start,
                decompress_cache.begin() + cond_entry.m_start + cond_entry.m_size
            );
            cond_entry.m_is_cached = true;
        }
    } else {

        std::streamoff entry_offset = data_offset + entry.m_start;
        m_stream->seekg(entry_offset, std::ios::beg);

        if (entry.m_flags & PFEP_COMPRESSED) {
            entry.m_data_cache = decompress_stream(*m_stream, entry.m_size);
        } else {
            entry.m_data_cache.resize(entry.m_size);
            m_stream->read(entry.m_data_cache.data(), entry.m_size);
        }

        entry.m_is_cached = true;
    }
}

PackfileEntry& Packfile::getEntryByFilename(const std::string& filename)
{
    for (PackfileEntry& entry : m_entries) {
        if (entry.m_filename == filename) {
            return entry;
        }
    }
    throw std::runtime_error("No file found");
}

PackfileEntry& Packfile::getEntry(size_t index) {return m_entries.at(index);}
std::vector<PackfileEntry>& Packfile::getEntries() {return m_entries;}
size_t Packfile::getEntriesCount() const {return m_entries.size();}

void Packfile::setDescriptor(uint32_t value) {m_descriptor = value;}
uint32_t Packfile::getDescriptor() const {return m_descriptor;}
void Packfile::setVersion(uint32_t value) {m_version = value;}
uint32_t Packfile::getVersion() const {return m_version;}
void Packfile::setHeaderChecksum(uint32_t value) {m_header_checksum = value;}
uint32_t Packfile::getHeaderChecksum() const {return m_header_checksum;}
void Packfile::setFileSize(uint32_t value) {m_file_size = value;}
uint32_t Packfile::getFileSize() const {return m_file_size;}
void Packfile::setFlags(uint32_t value) {m_flags = value;}
uint32_t Packfile::getFlags() const {return m_flags;}
void Packfile::setDirSize(uint32_t value) {m_dir_size = value;}
uint32_t Packfile::getDirSize() const {return m_dir_size;}
void Packfile::setFilenameSize(uint32_t value) {m_filename_size = value;}
uint32_t Packfile::getFilenameSize() const {return m_filename_size;}
void Packfile::setDataSize(uint32_t value) {m_data_size = value;}
uint32_t Packfile::getDataSize() const {return m_data_size;}
void Packfile::setCompressedDataSize(uint32_t value) {m_compressed_data_size = value;}
uint32_t Packfile::getCompressedDataSize() const {return m_compressed_data_size;}



PackfileEntry::PackfileEntry() :
    m_packfile(nullptr),
    m_is_cached(false)
{

}

PackfileEntry::PackfileEntry(Packfile& packfile, std::istream& stream) :
    m_packfile(&packfile),
    m_is_cached(false)
{
    load(stream);
}

void PackfileEntry::load(std::istream& stream)
{
    ByteReader reader(stream);

    m_filename_offset = reader.readU64();
    m_start = reader.readU32();
    m_size = reader.readU32();
    m_compressed_size = reader.readU32();
    m_flags = reader.readU16();
    m_alignment = reader.readU16();
}

std::vector<char>& PackfileEntry::getData()
{
    if (!m_is_cached) {
        m_packfile->loadFileData(*this);
    }

    return m_data_cache;
}




void PackfileEntry::setFilename(const std::string& value) {m_filename = value;}
std::string PackfileEntry::getFilename() const {return m_filename;}
void PackfileEntry::setFilenameOffset(uint64_t value) {m_filename_offset = value;}
uint64_t PackfileEntry::getFilenameOffset() const {return m_filename_offset;}
void PackfileEntry::setStart(uint32_t value) {m_start = value;}
uint32_t PackfileEntry::getStart() const {return m_start;}
void PackfileEntry::setSize(uint32_t value) {m_size = value;}
uint32_t PackfileEntry::getSize() const {return m_size;}
void PackfileEntry::setCompressedSize(uint32_t value) {m_compressed_size = value;}
uint32_t PackfileEntry::getCompressedSize() const {return m_compressed_size;}
void PackfileEntry::setFlags(uint16_t value) {m_flags = value;}
uint16_t PackfileEntry::getFlags() const {return m_flags;}
void PackfileEntry::setAlignment(uint16_t value) {m_alignment = value;}
uint16_t PackfileEntry::getAlignment() const {return m_alignment;}
