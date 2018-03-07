#include <cassert>
#include <algorithm>
#include <QtCore/QtGlobal>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <QtCore/QIODevice>

#include "crosstex/BC.hpp"
#include "crosstex/Colors.hpp"

#include "Saints/PegEntry.hpp"
#include "Saints/PegFile.hpp"
#include "Saints/DDSFile.hpp"
#include "Saints/Exceptions.hpp"
#include "Saints/TGAFile.hpp"
#include "Saints/Colors.hpp"
#include "ByteIO.hpp"



namespace Saints {

static LDRColor HDRToTGAPixel(Tex::HDRColorA color);
static Tex::HDRColorA TGAToHDRPixel(LDRColor pixel);
static QVector<LDRColor> decompressBC(const QByteArray& data, int width, int height, TextureFormat format);
static QByteArray compressBC(const QVector<LDRColor>& pixels, int width, int height, TextureFormat format);

struct format_pair_t
{
    TextureFormat format;
    QString name;
};

static const QVector<format_pair_t> FORMAT_NAMES {
    {TextureFormat::PC_BC1, QStringLiteral("BC1")},
    {TextureFormat::PC_BC2, QStringLiteral("BC2")},
    {TextureFormat::PC_BC3, QStringLiteral("BC3")},
    {TextureFormat::PC_565, QStringLiteral("RGB565")},
    {TextureFormat::PC_1555, QStringLiteral("RGBA5551")},
    {TextureFormat::PC_4444, QStringLiteral("RGBA4444")},
    {TextureFormat::PC_888, QStringLiteral("RGB888")},
    {TextureFormat::PC_8888, QStringLiteral("RGB8888")},
    {TextureFormat::PC_16_DUDV, QStringLiteral("V8U8")},
    {TextureFormat::PC_16_DOT3_COMPRESSED, QStringLiteral("CxV8U8")},
    {TextureFormat::PC_A8, QStringLiteral("A8")},
    {TextureFormat::PC_BC6HU, QStringLiteral("BC6HU")},
    {TextureFormat::PC_BC6HS, QStringLiteral("BC6HS")},
    {TextureFormat::PC_BC7, QStringLiteral("BC7")},
    {TextureFormat::PC_BC4, QStringLiteral("BC4")},
    {TextureFormat::PC_BC5, QStringLiteral("BC5")},
    {TextureFormat::PC_16161616, QStringLiteral("RGBA16161616")},
    {TextureFormat::PC_32323232, QStringLiteral("RGBA32323232")}
};

static const QList<QString> ENTRY_FLAG_NAMES {
    QStringLiteral("ALPHA"),
    QStringLiteral("NONPOW2"),
    QStringLiteral("ALPHA_TEST"),
    QStringLiteral("CUBE_MAP"),
    QStringLiteral("INTERLEAVED_MIPS"),
    QStringLiteral("INTERLEAVED_DATA"),
    QStringLiteral("DEBUG_DATA_COPIED"),
    QStringLiteral("DYNAMIC"),
    QStringLiteral("ANIM_SHEET"),
    QStringLiteral("LINEAR_COLOR_SPACE"),
    QStringLiteral("HIGH_MIP"),
    QStringLiteral("HIGH_MIP_ELIGIBLE"),
    QStringLiteral("LINKED_TO_HIGH_MIP"),
    QStringLiteral("PERM_REGISTERED")
};

QString getFormatName(TextureFormat fmt)
{
    for (const format_pair_t& format_pair : FORMAT_NAMES) {
        if (format_pair.format == fmt) {
            return format_pair.name;
        }
    }
    return QString::number(static_cast<int>(fmt));
}

TextureFormat getFormatId(QString name)
{
    for (const format_pair_t& format_pair : FORMAT_NAMES) {
        if (format_pair.name.compare(name, Qt::CaseInsensitive) == 0) {
            return format_pair.format;
        }
    }
    return TextureFormat::NONE;
}

QStringList getEntryFlagNames(int flags)
{
    QStringList names;
    for (int i = 0; i < ENTRY_FLAG_NAMES.size(); i++) {
        int mask = 1 << i;
        if (flags & mask) {
            names << ENTRY_FLAG_NAMES[i];
        }
    }
    return names;
}

static int calcCompressedSize(int width, int height, int blocksize)
{
    // Round up to multiple of 4 pixels
    int width_blocks = std::max(1, (width + 3) / 4);
    int height_blocks = std::max(1, (height + 3) / 4);

    return width_blocks * height_blocks * blocksize;
}

PegEntry::PegEntry()
{
    offset = 0;
    width = 0;
    height = 0;
    bm_fmt = TextureFormat::NONE;
    pal_fmt = 0;
    anim_tiles_width = 1;
    anim_tiles_height = 1;
    num_frames = 1;
    depth = 1; // TODO
    flags = 0;
    pal_size = 0;
    fps = 1;
    mip_levels = 1;
    data_size = 0;
    avg_color = {0.f, 0.f, 0.f, 0.f};
    num_mips_split = 0;
    data_max_size = 0;

    m_parent = nullptr;
}

PegEntry::PegEntry(PegFile& parent) :
    PegEntry()
{
    m_parent = &parent;
}

void PegEntry::read13(QIODevice& stream)
{
    ByteReader reader(stream);

    offset = reader.readS64();
    width = reader.readU16();
    height = reader.readU16();
    bm_fmt = static_cast<TextureFormat>(reader.readU16());
    pal_fmt = reader.readU16();
    anim_tiles_width = reader.readU16();
    anim_tiles_height = reader.readU16();
    num_frames = reader.readU16();
    flags = reader.readU16();
    reader.ignore(8); // Runtime variable
    pal_size = reader.readU16();
    fps = reader.readU8();
    mip_levels = reader.readU8();
    data_size = reader.readU32();
    reader.ignore(32); // Runtime variables and padding
}

void PegEntry::read19(QIODevice& stream)
{
    ByteReader reader(stream);

    offset = reader.readS64();
    width = reader.readU16();
    height = reader.readU16();
    bm_fmt = static_cast<TextureFormat>(reader.readU16());
    pal_fmt = reader.readU16();
    anim_tiles_width = reader.readU16();
    anim_tiles_height = reader.readU16();
    depth = reader.readU16();
    flags = reader.readU16();
    avg_color.r = reader.readFloat();
    avg_color.g = reader.readFloat();
    avg_color.b = reader.readFloat();
    avg_color.a = reader.readFloat();
    reader.ignore(8); // Runtime variable (filename)
    pal_size = reader.readU16();
    fps = reader.readU8();
    mip_levels = reader.readU8();
    data_size = reader.readU32();
    reader.ignore(32); // Runtime variables
    num_mips_split = reader.readU32();
    data_max_size = reader.readU32();
    reader.ignore(8); // Padding
}


void PegEntry::write13(QIODevice& stream, qint64 data_offset) const
{
    ByteWriter writer(stream);

    writer.writeS64(data_offset);
    writer.writeU16(width);
    writer.writeU16(height);
    writer.writeU16(static_cast<uint16_t>(bm_fmt));
    writer.writeU16(pal_fmt);
    writer.writeU16(anim_tiles_width);
    writer.writeU16(anim_tiles_height);
    writer.writeU16(num_frames);
    writer.writeU16(flags);
    writer.pad(8);
    writer.writeU16(pal_size);
    writer.writeU8(fps);
    writer.writeU8(mip_levels);
    writer.writeU32(data.size());
    writer.pad(32);
}

void PegEntry::write19(QIODevice& stream, qint64 data_offset) const
{
    ByteWriter writer(stream);

    writer.writeS64(data_offset);
    writer.writeU16(width);
    writer.writeU16(height);
    writer.writeU16(static_cast<uint16_t>(bm_fmt));
    writer.writeU16(pal_fmt);
    writer.writeU16(anim_tiles_width);
    writer.writeU16(anim_tiles_height);
    writer.writeU16(depth);
    writer.writeU16(flags);
    writer.writeFloat(avg_color.r);
    writer.writeFloat(avg_color.g);
    writer.writeFloat(avg_color.b);
    writer.writeFloat(avg_color.a);
    writer.pad(8);
    writer.writeU16(pal_size);
    writer.writeU8(fps);
    writer.writeU8(mip_levels);
    writer.writeU32(data.size());
    writer.pad(32);
    writer.writeU32(num_mips_split);
    writer.writeU32(data_max_size);
    writer.pad(8);
}

void PegEntry::fromDDS(const DDSFile& ddsfile)
{
    width = ddsfile.width;
    height = ddsfile.height;
    bm_fmt = detectPixelformat(ddsfile.ddspf);
    if (ddsfile.mipmap_count > 1) {
        mip_levels = ddsfile.mipmap_count;
    } else {
        mip_levels = 1;
    }

    data = ddsfile.data;
}

void PegEntry::fromTGA(const TGAFile& tgafile, TextureFormat fmt)
{
    width = tgafile.width;
    height = tgafile.height;
    bm_fmt = fmt;
    data = compressBC(tgafile.pixels, width, height, bm_fmt);

    avg_color = {0.f, 0.f, 0.f, 0.f};
    bool has_alpha = false;
    for (LDRColor pixel : tgafile.pixels) {
        if (pixel.a < 0xFF) {
            has_alpha = true;
        }
        avg_color.r += pixel.r;
        avg_color.g += pixel.g;
        avg_color.b += pixel.b;
        avg_color.a += pixel.a;
    }
    // Divide by number of pixels and 255
    float avg_factor = 1.f / (width * height * 255.f);
    // Cap at 1.0
    avg_color.r = std::min(1.f, avg_color.r * avg_factor);
    avg_color.g = std::min(1.f, avg_color.g * avg_factor);
    avg_color.b = std::min(1.f, avg_color.b * avg_factor);
    avg_color.a = std::min(1.f, avg_color.a * avg_factor);

    if (has_alpha) {
        flags |= BM_F_ALPHA;
    } else {
        flags &= ~BM_F_ALPHA;
        avg_color.a = 1.f;
    }
}

DDSFile PegEntry::toDDS() const
{
    DDSFile ddsfile;
    ddsfile.height = height;
    ddsfile.width = width;

    if (mip_levels > 1) {
        ddsfile.flags |= DDSD_MIPMAPCOUNT;
        ddsfile.mipmap_count = mip_levels;
        ddsfile.caps |= DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
    }

    ddsfile.ddspf = getPixelformat(bm_fmt);

    // Calculate pitch
    switch (bm_fmt) {
    case TextureFormat::PC_BC1:
        ddsfile.flags |= DDSD_LINEARSIZE;
        ddsfile.pitch_or_linear_size = calcCompressedSize(width, height, 8);
        break;
    case TextureFormat::PC_BC2:
    case TextureFormat::PC_BC3:
        ddsfile.flags |= DDSD_LINEARSIZE;
        ddsfile.pitch_or_linear_size = calcCompressedSize(width, height, 16);
        break;
    default:
        if (ddsfile.ddspf.rgb_bit_count > 0) {
            ddsfile.flags |= DDSD_PITCH;
            ddsfile.pitch_or_linear_size =
                (width * ddsfile.ddspf.rgb_bit_count + 7) / 8;
        } else {
            throw FieldError("format", QString::number(static_cast<int>(bm_fmt)));
        }
    }

    ddsfile.data = data;

    return ddsfile;
}

TGAFile PegEntry::toTGA() const
{
    TGAFile tga;
    tga.width = width;
    tga.height = height;
    tga.pixels = decompressBC(data, width, height, bm_fmt);
    tga.data_type = TGAImageType::RGB;
    tga.bits_per_pixel = 32;
    tga.image_attributes = 0x08;
    return tga;
}

LDRColor HDRToTGAPixel(Tex::HDRColorA color)
{
    color.Clamp(0.0f, 1.0f);

    LDRColor pixel;
    pixel.r = color.r * 255.f;
    pixel.g = color.g * 255.f;
    pixel.b = color.b * 255.f;
    pixel.a = color.a * 255.f;
    return pixel;
}

Tex::HDRColorA TGAToHDRPixel(LDRColor pixel)
{
    Tex::HDRColorA color;
    color.r = pixel.r / 255.f;
    color.g = pixel.g / 255.f;
    color.b = pixel.b / 255.f;
    color.a = pixel.a / 255.f;
    return color;
}

QVector<LDRColor> decompressBC(const QByteArray& data, int width, int height, TextureFormat format)
{
    int width_blocks = width / 4;
    int height_blocks = height / 4;

    Tex::BC_DECODE decompress_func;
    int block_size;
    switch(format)
    {
    case TextureFormat::PC_BC1:
        decompress_func = Tex::DecodeBC1;
        block_size = 8;
        break;
    case TextureFormat::PC_BC2:
        decompress_func = Tex::DecodeBC2;
        block_size = 16;
        break;
    case TextureFormat::PC_BC3:
        decompress_func = Tex::DecodeBC3;
        block_size = 16;
        break;
    case TextureFormat::PC_BC4:
        decompress_func = Tex::DecodeBC4U;
        block_size = 8;
        break;
    case TextureFormat::PC_BC5:
        decompress_func = Tex::DecodeBC5U;
        block_size = 16;
        break;
    case TextureFormat::PC_BC6HU:
        decompress_func = Tex::DecodeBC6HU;
        block_size = 16;
        break;
    case TextureFormat::PC_BC6HS:
        decompress_func = Tex::DecodeBC6HS;
        block_size = 16;
        break;
    case TextureFormat::PC_BC7:
        decompress_func = Tex::DecodeBC7;
        block_size = 16;
        break;
    default:
        throw ParsingError("Unknown texture format");
    }

    QVector<LDRColor> pixels(width * height);
    for (int block_x = 0; block_x < width_blocks; block_x++) {
        for (int block_y = 0; block_y < height_blocks; block_y++) {
            int data_pos = (block_y * width_blocks + block_x) * block_size;
            const char* data_block_p = data.constData() + data_pos;
            Tex::HDRColorA block_texels[16];
            decompress_func(
                block_texels,
                reinterpret_cast<const uint8_t*>(data_block_p)
            );
            for (int texel_x = 0; texel_x < 4; texel_x++) {
                for (int texel_y = 0; texel_y < 4; texel_y++) {
                    int absolute_x = block_x * 4 + texel_x;
                    int absolute_y = block_y * 4 + texel_y;
                    int texel_pos = absolute_y * width + absolute_x;
                    int block_pos = texel_y * 4 + texel_x;
                    pixels[texel_pos] = HDRToTGAPixel(block_texels[block_pos]);
                }
            }
        }
    }

    return pixels;
}

QByteArray compressBC(const QVector<LDRColor>& pixels, int width, int height, TextureFormat format)
{
    int width_blocks = width / 4;
    int height_blocks = height / 4;

    Tex::BC_ENCODE compress_func;
    int block_size;
    switch(format)
    {
    case TextureFormat::PC_BC1:
        compress_func = Tex::EncodeBC1;
        block_size = 8;
        break;
    case TextureFormat::PC_BC2:
        compress_func = Tex::EncodeBC2;
        block_size = 16;
        break;
    case TextureFormat::PC_BC3:
        compress_func = Tex::EncodeBC3;
        block_size = 16;
        break;
    case TextureFormat::PC_BC4:
        compress_func = Tex::EncodeBC4U;
        block_size = 8;
        break;
    case TextureFormat::PC_BC5:
        compress_func = Tex::EncodeBC5U;
        block_size = 16;
        break;
    case TextureFormat::PC_BC6HU:
        compress_func = Tex::EncodeBC6HU;
        block_size = 16;
        break;
    case TextureFormat::PC_BC6HS:
        compress_func = Tex::EncodeBC6HS;
        block_size = 16;
        break;
    case TextureFormat::PC_BC7:
        compress_func = Tex::EncodeBC7;
        block_size = 16;
        break;
    default:
        throw ParsingError("Unknown texture format");
    }

    QByteArray data(width_blocks * height_blocks * block_size, 0x00);
    for (int block_x = 0; block_x < width_blocks; block_x++) {
        for (int block_y = 0; block_y < height_blocks; block_y++) {
            int data_pos = (block_y * width_blocks + block_x) * block_size;
            char* data_block_p = data.data() + data_pos;
            Tex::HDRColorA block_texels[16];
            for (int texel_x = 0; texel_x < 4; texel_x++) {
                for (int texel_y = 0; texel_y < 4; texel_y++) {
                    int absolute_x = block_x * 4 + texel_x;
                    int absolute_y = block_y * 4 + texel_y;
                    int texel_pos = absolute_y * width + absolute_x;
                    int block_pos = texel_y * 4 + texel_x;
                    block_texels[block_pos] = TGAToHDRPixel(pixels[texel_pos]);
                }
            }
            compress_func(
                reinterpret_cast<uint8_t*>(data_block_p),
                block_texels,
                Tex::BC_FLAGS_NONE
            );
        }
    }

    return data;
}

}
