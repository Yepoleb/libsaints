#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtCore/QVector>

#include "PegEntry.hpp"



namespace Saints {

struct PegEntry;

const qint64 PEGHEADER_BINSIZE = 24;
class PegFile
{
    friend PegEntry;

public:
    PegFile();
    PegFile(QIODevice& header_stream);
    PegFile(QIODevice& header_stream, QIODevice& data_stream);
    void open(QIODevice& header_stream);
    void open(QIODevice& header_stream, QIODevice& data_stream);
    void readHeader(QIODevice& stream);
    void writeHeader(QIODevice& stream) const;
    void readData(QIODevice& stream);
    void writeData(QIODevice& stream) const;
    int getEntryIndex(const QString& name) const;

    qint16 version; // 13 for SRTT and SRIV
    qint16 platform; // 0 = PC
    quint32 header_size; // Size of the header file.
    quint32 data_size; // Size of the data file.
    quint16 flags; // Always 0
    quint16 alignment; // Always 16 for the PC
    QVector<PegEntry> entries;

private:
    int calcHeaderSize() const;
    qint64 calcDataSize() const;
    void loadEntryData(PegEntry& entry);
};

}
