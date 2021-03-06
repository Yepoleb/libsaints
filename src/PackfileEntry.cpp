#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtCore/QFileInfo>

#include "ByteIO.hpp"
#include "Saints/Packfile.hpp"
#include "Saints/PackfileEntry.hpp"



namespace Saints {

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

void PackfileEntry::load17(QIODevice& stream)
{
    ByteReader reader(stream);

    m_start = reader.readU64();
    m_size = reader.readU64();
    m_compressed_size = reader.readU64();
    m_flags = reader.readU16();
    m_alignment = reader.readU32();
    reader.ignore(2);
}

QByteArray& PackfileEntry::getData()
{
    if (!m_is_cached) {
        m_packfile->loadFileData(*this);
    }

    return m_data_cache;
}

QString PackfileEntry::getFilepath() const
{
    if (m_filepath.isEmpty()) {
        return m_filename;
    } else {
        return m_filepath + '\\' + m_filename;
    }
}

void PackfileEntry::setFilepath(const QString& value)
{
    if (value.contains('\\')) {
        int last_sep = value.lastIndexOf('\\');
        m_filename = value.mid(last_sep + 1);
        m_filepath = value.left(last_sep - 1);
    } else {
        m_filename = value;
    }
}



QString PackfileEntry::getFilename() const {return m_filename;}
void PackfileEntry::setFilename(const QString& value) {m_filename = value;}
QString PackfileEntry::getDirectory() const {return m_filepath;}
void PackfileEntry::setDirectory(const QString& value) {m_filepath = value;}
qint64 PackfileEntry::getStart() const {return m_start;}
void PackfileEntry::setStart(qint64 value) {m_start = value;}
qint64 PackfileEntry::getSize() const {return m_size;}
void PackfileEntry::setSize(qint64 value) {m_size = value;}
qint64 PackfileEntry::getCompressedSize() const {return m_compressed_size;}
void PackfileEntry::setCompressedSize(qint64 value) {m_compressed_size = value;}
int PackfileEntry::getFlags() const {return m_flags;}
void PackfileEntry::setFlags(int value) {m_flags = value;}
int PackfileEntry::getAlignment() const {return m_alignment;}
void PackfileEntry::setAlignment(int value) {m_alignment = value;}
}
