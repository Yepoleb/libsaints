#include <QtCore/QtGlobal>
#include <QtCore/QIODevice>
#include <QtCore/QString>

#include "Saints/DDSFile.hpp"
#include "Saints/Exceptions.hpp"
#include "Saints/PegEntry.hpp"
#include "ByteIO.hpp"
#include "util.hpp"


namespace Saints {

constexpr quint32 FOURCC_DDS = makeFourCC("DDS ");

// Documentation of DDS format:
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982.aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb943984.aspx

constexpr DDSPixelformat PIXELFORMATS[] = {
    // DXT1
    {DDPF_FOURCC, makeFourCC("DXT1"), 0, 0, 0, 0, 0},
    // DXT3
    {DDPF_FOURCC, makeFourCC("DXT3"), 0, 0, 0, 0, 0},
    // DXT5
    {DDPF_FOURCC, makeFourCC("DXT5"), 0, 0, 0, 0, 0},
    // R5G6B5
    {DDPF_RGB, 0, 16, 0xf800, 0x07e0, 0x001f, 0x0000},
    // R5G5B5
    {DDPF_RGBA, 0, 16, 0x7c00, 0x03e0, 0x001f, 0x8000},
    // A4R4G4B4
    {DDPF_RGBA, 0, 16, 0x0f00, 0x00f0, 0x000f, 0xf000},
    // 8G8B8
    {DDPF_RGB, 0, 24, 0xff0000, 0x00ff00, 0x0000ff, 0x000000},
    // A8R8G8B8
    {DDPF_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000},
    // V8U8
    {DDPF_BUMPDUDV, 0, 16, 0x00ff, 0xff00, 0x0000, 0x0000},
    // CxV8U8
    {DDPF_FOURCC, 117, 16, 0x00ff, 0xff00, 0x0000, 0x0000},
    // A8
    {DDPF_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff}
};
constexpr int PIXELFORMATS_SIZE = ARRAYSIZE(PIXELFORMATS);

DDSPixelformat getPixelformat(TextureFormat fmt)
{
    // Enums start at 400
    int index = static_cast<int>(fmt) - 400;
    if (!(0 <= index && index < static_cast<int>(PIXELFORMATS_SIZE))) {
        throw FieldError("format", QString::number(static_cast<int>(fmt)));
    }

    return PIXELFORMATS[index];
}

TextureFormat detectPixelformat(const DDSPixelformat& ddspf)
{
    for (int i = 0; i < PIXELFORMATS_SIZE; i++) {
        const DDSPixelformat& ddspf_ref = PIXELFORMATS[i];

        if (ddspf.flags != ddspf_ref.flags) continue;
        if (ddspf.flags & DDPF_FOURCC) {
            if (ddspf.four_cc != ddspf_ref.four_cc) continue;
        }
        if (ddspf.flags & DDPF_RGB) {
            if (ddspf.rgb_bit_count != ddspf_ref.rgb_bit_count) continue;
            if (ddspf.r_bitmask != ddspf_ref.r_bitmask) continue;
            if (ddspf.g_bitmask != ddspf_ref.g_bitmask) continue;
            if (ddspf.b_bitmask != ddspf_ref.b_bitmask) continue;
        }
        if (ddspf.flags & DDPF_ALPHAPIXELS) {
            if (ddspf.a_bitmask != ddspf_ref.a_bitmask) continue;
        }

        return static_cast<TextureFormat>(i + 400);
    }

    return TextureFormat::NONE;
}



void DDSPixelformat::read(QIODevice& stream)
{
    ByteReader reader(stream);

    quint32 size = reader.readU32();
    if (size != DDS_PIXELFORMAT_SIZE) {
        throw FieldError("size", QString::number(size));
    }
    flags = reader.readU32();
    four_cc = reader.readU32();
    rgb_bit_count = reader.readU32();
    r_bitmask = reader.readU32();
    g_bitmask = reader.readU32();
    b_bitmask = reader.readU32();
    a_bitmask = reader.readU32();
}

void DDSPixelformat::write(QIODevice& stream) const
{
    ByteWriter writer(stream);

    writer.writeU32(DDS_PIXELFORMAT_SIZE);
    writer.writeU32(flags);
    writer.writeU32(four_cc);
    writer.writeU32(rgb_bit_count);
    writer.writeU32(r_bitmask);
    writer.writeU32(g_bitmask);
    writer.writeU32(b_bitmask);
    writer.writeU32(a_bitmask);
}



DDSFile::DDSFile()
{
    flags = DDSD_REQUIRED;
    height = 0;
    width = 0;
    pitch_or_linear_size = 0;
    depth = 0;
    mipmap_count = 1;
    reserved1[11] = {};
    ddspf = {};
    caps = DDSCAPS_TEXTURE;
    caps2 = 0;
    caps3 = 0;
    caps4 = 0;
    reserved2 = 0;
}

void DDSFile::open(QIODevice& stream)
{
    ByteReader reader(stream);

    quint32 descriptor = reader.readU32();
    if (descriptor != FOURCC_DDS) {
        throw FieldError("descriptor", QString::number(descriptor, 16));
    }
    quint32 size = reader.readU32();
    if (size != DDS_HEADER_SIZE) {
        throw FieldError("size", QString::number(size));
    }
    flags = reader.readU32();
    height = reader.readU32();
    width = reader.readU32();
    pitch_or_linear_size = reader.readU32();
    depth = reader.readU32();
    mipmap_count = reader.readU32();
    for (int i = 0; i < ARRAYSIZE(reserved1); i++) {
        reserved1[i] = reader.readU32();
    }
    ddspf.read(stream);
    caps = reader.readU32();
    caps2 = reader.readU32();
    caps3 = reader.readU32();
    caps4 = reader.readU32();
    reserved2 = reader.readU32();

    data = stream.readAll();
}

void DDSFile::write(QIODevice& stream) const
{
    ByteWriter writer(stream);

    writer.writeU32(FOURCC_DDS);
    writer.writeU32(DDS_HEADER_SIZE);
    writer.writeU32(flags);
    writer.writeU32(height);
    writer.writeU32(width);
    writer.writeU32(pitch_or_linear_size);
    writer.writeU32(depth);
    writer.writeU32(mipmap_count);
    for (int i = 0; i < ARRAYSIZE(reserved1); i++) {
        writer.writeU32(reserved1[i]);
    }
    ddspf.write(stream);
    writer.writeU32(caps);
    writer.writeU32(caps2);
    writer.writeU32(caps3);
    writer.writeU32(caps4);
    writer.writeU32(reserved2);

    writer.write(data);
}

}
