#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtCore/QVector>



namespace Saints {

class PackfileEntry;

constexpr int PFF_COMPRESSED = (1 << 0);
constexpr int PFF_CONDENSED = (1 << 1);

constexpr int PFEP_COMPRESSED = (1 << 0);



class Packfile
{
public:
    Packfile();
    Packfile(QIODevice& stream);

    void open(QIODevice& stream);
    void load();
    void loadFileData(PackfileEntry& entry);
    PackfileEntry& getEntryByFilename(const QString& filename);
    PackfileEntry& getEntry(int index);
    QVector<PackfileEntry>& getEntries();
    int getEntriesCount() const;

    quint32 getDescriptor() const;
    void setDescriptor(quint32 value);
    int getVersion() const;
    void setVersion(int value);
    quint32 getHeaderChecksum() const;
    void setHeaderChecksum(quint32 value);
    qint64 getFileSize() const;
    void setFileSize(qint64 value);
    int getFlags() const;
    void setFlags(int value);
    int getNumFiles() const;
    void setNumFiles(int value);
    qint64 getDirSize() const;
    void setDirSize(qint64 value);
    qint64 getFilenameSize() const;
    void setFilenameSize(qint64 value);
    qint64 getDataSize() const;
    void setDataSize(qint64 value);
    qint64 getCompressedDataSize() const;
    void setCompressedDataSize(qint64 value);

private:
    QIODevice* m_stream;

    quint32 m_descriptor;
    int m_version;
    quint32 m_header_checksum;
    qint64 m_file_size;

    int m_flags;
    int m_num_files;
    qint64 m_dir_size;
    qint64 m_filename_size;

    qint64 m_data_size;
    qint64 m_compressed_data_size;

    QVector<PackfileEntry> m_entries;
    bool m_condensed_cached;
};

class PackfileEntry
{
    friend Packfile;

public:
    PackfileEntry();
    PackfileEntry(Packfile* parent, QIODevice& stream);

    void load(QIODevice& stream);
    QByteArray& getData();

    void setFilename(const QString& value);
    QString getFilename() const;
    void setFilenameOffset(qint64 value);
    qint64 getFilenameOffset() const;
    void setStart(qint64 value);
    qint64 getStart() const;
    void setSize(qint64 value);
    qint64 getSize() const;
    void setCompressedSize(qint64 value);
    qint64 getCompressedSize() const;
    void setFlags(int value);
    int getFlags() const;
    void setAlignment(int value);
    int getAlignment() const;

private:
    Packfile* m_packfile;

    qint64 m_filename_offset;
    qint64 m_start;
    qint64 m_size;
    qint64 m_compressed_size;
    int m_flags;
    int m_alignment;

    QString m_filename;
    QByteArray m_data_cache;
    bool m_is_cached;
};

} // namespace saints
