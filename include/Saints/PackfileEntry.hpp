#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>



namespace Saints {

class Packfile;

class PackfileEntry
{
    friend Packfile;

public:
    enum Flags {
        Compressed = (1 << 0)
    };

    PackfileEntry();
    PackfileEntry(Packfile& parent);

    void load6(QIODevice& stream);
    void load10(QIODevice& stream);
    void load17(QIODevice& stream);
    QByteArray& getData();
    void setFilepath(const QString& value);
    QString getFilepath() const;
    qint64 getSize() const;

    void setFilename(const QString& value);
    QString getFilename() const;
    void setDirectory(const QString& value);
    QString getDirectory() const;
    void setFlags(int value);
    int getFlags() const;
    void setAlignment(int value);
    int getAlignment() const;

private:
    Packfile* m_packfile;

    qint64 m_start;
    qint64 m_size;
    qint64 m_compressed_size;
    int m_flags;
    int m_alignment;

    QString m_filename;
    QString m_filepath;
    QByteArray m_data_cache;
    bool m_is_cached;
};

}
