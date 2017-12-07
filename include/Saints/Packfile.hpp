#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtCore/QVector>



namespace Saints {

class PackfileEntry;

class Packfile
{
public:
    enum Flags {
        Compressed = (1 << 0),
        Condensed = (1 << 1)
    };

    Packfile();
    Packfile(QIODevice& stream);

    void open(QIODevice& stream);
    void load();
    void loadFileData(PackfileEntry& entry);
    PackfileEntry& getEntryByFilename(const QString& filename);
    const PackfileEntry& getEntryByFilename(const QString& filename) const;
    PackfileEntry& getEntry(int index);
    const PackfileEntry& getEntry(int index) const;
    QVector<PackfileEntry>& getEntries();
    const QVector<PackfileEntry>& getEntries() const;
    int getEntriesCount() const;

    int getVersion() const;
    void setVersion(int value);
    int getFlags() const;
    void setFlags(int value);

private:
    void loadHeader6();
    void loadHeader10();

    qint64 getEntriesOffset();
    qint64 getEntryNamesOffset();
    qint64 getDataOffset();

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

}
