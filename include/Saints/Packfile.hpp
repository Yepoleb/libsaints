#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtCore/QVector>

#include "PackfileEntry.hpp"



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
    PackfileEntry* getEntryByFilename(const QString& filename);
    const PackfileEntry* getEntryByFilename(const QString& filename) const;
    PackfileEntry& getEntry(int index);
    const PackfileEntry& getEntry(int index) const;
    QVector<PackfileEntry>& getEntries();
    const QVector<PackfileEntry>& getEntries() const;
    int getEntriesCount() const;

    int getVersion() const;
    void setVersion(int value);
    int getFlags() const;
    void setFlags(int value);
    qint64 getTimestamp() const;
    void setTimestamp(qint64 value);

private:
    void loadHeader6();
    void loadHeader10();
    void loadHeader17();

    qint64 getEntriesOffset();
    qint64 getEntryNamesOffset();
    qint64 getDataOffset();

    QByteArray decompressStream(QIODevice& stream);

    QIODevice* m_stream;

    quint32 m_descriptor;
    int m_version;
    quint32 m_header_checksum;
    qint64 m_file_size;

    int m_flags;
    qint64 m_dir_size;
    qint64 m_filename_size;

    qint64 m_data_size;
    qint64 m_compressed_data_size;

    qint64 m_timestamp;
    qint64 m_data_offset;

    QVector<PackfileEntry> m_entries;
    bool m_condensed_cached;
};

}
