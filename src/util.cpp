#include <assert.h>
#include <stddef.h>
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QIODevice>

#include "zlib.h"
#include "lz4frame.h"

#include "util.hpp"
#include "Saints/Exceptions.hpp"

constexpr qint64 CHUNK_SIZE = 16384;



namespace Saints {

QByteArray decompressZLIB(QIODevice& stream)
{
    QByteArray out_data;

    QByteArray in_buffer(CHUNK_SIZE, 0);
    QByteArray out_buffer(CHUNK_SIZE, 0);

    int ret;
    z_stream zstrm;
    zstrm.zalloc = Z_NULL;
    zstrm.zfree = Z_NULL;
    zstrm.opaque = Z_NULL;
    zstrm.avail_in = 0;
    zstrm.next_in = Z_NULL;
    ret = inflateInit(&zstrm);

    if (ret != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib");
    }

    do {
        qint64 bytes_read = stream.read(in_buffer.data(), CHUNK_SIZE);
        if (bytes_read == -1) {
            inflateEnd(&zstrm);
            throw IOError("Error reading input file");
        }
        if (bytes_read == 0) {
            break;
        }
        zstrm.avail_in = bytes_read;
        zstrm.next_in = reinterpret_cast<unsigned char*>(in_buffer.data());

        do {
            zstrm.avail_out = CHUNK_SIZE;
            zstrm.next_out = reinterpret_cast<unsigned char*>(out_buffer.data());
            ret = inflate(&zstrm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
                inflateEnd(&zstrm);
                throw ParsingError("Invalid compression data");
            case Z_MEM_ERROR:
                inflateEnd(&zstrm);
                throw std::bad_alloc();
            }
            qint64 out_len = CHUNK_SIZE - zstrm.avail_out;
            out_data.append(out_buffer.data(), out_len);
        } while (zstrm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&zstrm);
    return out_data;
}

static int getLZ4BlockSize(const LZ4F_frameInfo_t* info) {
    switch (info->blockSizeID) {
        case LZ4F_default:
        case LZ4F_max64KB:  return 1 << 16;
        case LZ4F_max256KB: return 1 << 18;
        case LZ4F_max1MB:   return 1 << 20;
        case LZ4F_max4MB:   return 1 << 22;
        default:
            throw std::runtime_error("Invalid block size");
    }
}

QByteArray decompressLZ4(QIODevice& stream)
{
    QByteArray out_data;

    QByteArray in_buffer(CHUNK_SIZE, 0);
    char* in_ptr_init = in_buffer.data();
    QByteArray out_buffer;
    char* out_ptr;
    int out_capacity = 0;

    LZ4F_dctx* dctx = nullptr;
    int ret = 1;

    int dctx_status = LZ4F_createDecompressionContext(&dctx, 100);
    if (LZ4F_isError(dctx_status)) {
        throw std::runtime_error("Failed to initialize lz4");
    }

    while (ret != 0) {
        char* in_ptr = in_ptr_init;
        size_t in_len = stream.read(in_ptr, CHUNK_SIZE);
        if (in_len < 1) {
            LZ4F_freeDecompressionContext(dctx);
            throw ParsingError("Error reading input data");
        }

        if (out_buffer.isNull()) {
            LZ4F_frameInfo_t info;
            size_t in_consumed = in_len;
            ret = LZ4F_getFrameInfo(dctx, &info, in_ptr, &in_consumed);
            if (LZ4F_isError(ret)) {
                LZ4F_freeDecompressionContext(dctx);
                throw ParsingError("Failed to read frame info");
            }
            out_capacity = getLZ4BlockSize(&info);
            out_buffer.resize(out_capacity);
            out_ptr = out_buffer.data();
            in_ptr += in_consumed;
            in_len -= in_consumed;
        }

        while (in_len > 0 && ret != 0) {
            size_t out_len = out_capacity;
            size_t in_consumed = in_len;
            ret = LZ4F_decompress(dctx, out_ptr, &out_len, in_ptr, &in_consumed, nullptr);
            if (LZ4F_isError(ret)) {
                LZ4F_freeDecompressionContext(dctx);
                throw ParsingError("Invalid compression data");
            }

            out_data.append(out_ptr, out_len);
            in_ptr += in_consumed;
            in_len -= in_consumed;
        }
    }

    LZ4F_freeDecompressionContext(dctx);
    return out_data;
}

}
