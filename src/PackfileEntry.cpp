#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QIODevice>

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

}
