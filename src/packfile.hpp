#include <stdint.h>
#include <stddef.h>
#include <iostream>
#include <string>
#include <vector>

class PackfileEntry;



constexpr uint32_t PFF_COMPRESSED = (1 << 0);
constexpr uint32_t PFF_CONDENSED = (1 << 1);

constexpr uint32_t PFEP_COMPRESSED = (1 << 0);



class Packfile
{
public:
    Packfile();
    Packfile(std::istream& stream);

    void load(std::istream& stream);
    void loadFileData(PackfileEntry& entry);
    PackfileEntry& getEntryByFilename(const std::string& filename);
    PackfileEntry& getEntry(size_t index);
    std::vector<PackfileEntry>& getEntries();
    size_t getEntriesCount() const;

    uint32_t getDescriptor() const;
    void setDescriptor(uint32_t value);
    uint32_t getVersion() const;
    void setVersion(uint32_t value);
    uint32_t getHeaderChecksum() const;
    void setHeaderChecksum(uint32_t value);
    uint32_t getFileSize() const;
    void setFileSize(uint32_t value);
    uint32_t getFlags() const;
    void setFlags(uint32_t value);
    uint32_t getNumFiles() const;
    void setNumFiles(uint32_t value);
    uint32_t getDirSize() const;
    void setDirSize(uint32_t value);
    uint32_t getFilenameSize() const;
    void setFilenameSize(uint32_t value);
    uint32_t getDataSize() const;
    void setDataSize(uint32_t value);
    uint32_t getCompressedDataSize() const;
    void setCompressedDataSize(uint32_t value);

private:
    std::istream* m_stream;

    uint32_t m_descriptor;
    uint32_t m_version;
    uint32_t m_header_checksum;
    uint32_t m_file_size;

    uint32_t m_flags;
    uint32_t m_num_files;
    uint32_t m_dir_size;
    uint32_t m_filename_size;

    uint32_t m_data_size;
    uint32_t m_compressed_data_size;

    std::vector<PackfileEntry> m_entries;
    bool m_condensed_cached;
};

class PackfileEntry
{
    friend Packfile;

public:
    PackfileEntry();
    PackfileEntry(Packfile& parent, std::istream& stream);

    void load(std::istream& stream);
    std::vector<char>& getData();

    void setFilename(const std::string& value);
    std::string getFilename() const;
    void setFilenameOffset(uint64_t value);
    uint64_t getFilenameOffset() const;
    void setStart(uint32_t value);
    uint32_t getStart() const;
    void setSize(uint32_t value);
    uint32_t getSize() const;
    void setCompressedSize(uint32_t value);
    uint32_t getCompressedSize() const;
    void setFlags(uint16_t value);
    uint16_t getFlags() const;
    void setAlignment(uint16_t value);
    uint16_t getAlignment() const;

private:
    Packfile* m_packfile;

    uint64_t m_filename_offset;
    uint32_t m_start;
    uint32_t m_size;
    uint32_t m_compressed_size;
    uint16_t m_flags;
    uint16_t m_alignment;

    std::string m_filename;
    std::vector<char> m_data_cache;
    bool m_is_cached;
};
