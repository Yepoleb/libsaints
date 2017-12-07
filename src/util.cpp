#include <assert.h>
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QIODevice>

#include "zlib.h"

#include "util.hpp"

constexpr qint64 ZLIB_CHUNK_SIZE = 16384;



namespace Saints {

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

}
