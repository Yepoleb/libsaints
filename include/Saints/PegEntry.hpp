#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <QtCore/QHash>

#include "Colors.hpp"


namespace Saints {

class DDSFile;
class PegFile;
class TGAFile;
struct LDRColor;

constexpr int BM_F_ALPHA = 0x1; // bitmap has alpha
constexpr int BM_F_NONPOW2 = 0x2; // bitmap is not power of 2
constexpr int BM_F_ALPHA_TEST = 0x4;
constexpr int BM_F_CUBE_MAP = 0x8; // bitmap is a cube map, react appropriately on load.
constexpr int BM_F_INTERLEAVED_MIPS = 0x10; // bitmap contains interleaved mips (they exist inside of the NEXT bitmap)
constexpr int BM_F_INTERLEAVED_DATA = 0x20; // bitmap contains interleaved mips from the previous bitmap
constexpr int BM_F_DEBUG_DATA_COPIED = 0x40; // used by the peg assembler only.
constexpr int BM_F_DYNAMIC = 0x80; // bitmap was loaded dynamically (not from a peg) (runtime only)
constexpr int BM_F_ANIM_SHEET = 0x100; // bitmap animation frames are stored in one bitmap spaced sequentially left to right
constexpr int BM_F_LINEAR_COLOR_SPACE = 0x200; // bitmap is NOT stored in SRGB space, it is linear (not gamma corrected)
constexpr int BM_F_HIGH_MIP = 0x400; // bitmap is a separately streamed high mip
constexpr int BM_F_HIGH_MIP_ELIGIBLE = 0x800; // bitmap is eligible for linking up with a high mip (runtime only flag)
constexpr int BM_F_LINKED_TO_HIGH_MIP = 0x1000; // bitmap is currently linked to a high mip (runtime only flag)
constexpr int BM_F_PERM_REGISTERED = 0x2000; // bitmap is permanently registered. used on the PC so d3d becomes the permanent owner of the texture memory.

enum class TextureFormat
{
    NONE = 0,
    PC_BC1 = 400,
    PC_BC2,
    PC_BC3,
    PC_565,
    PC_1555,
    PC_4444,
    PC_888,
    PC_8888,
    PC_16_DUDV, // V8U8
    PC_16_DOT3_COMPRESSED, // CxV8U8
    PC_A8,
    PC_BC6HU,
    PC_BC6HS,
    PC_BC7,
    PC_BC4,
    PC_BC5,
    PC_16161616,
    PC_32323232
};

QString getFormatName(TextureFormat fmt);
TextureFormat getFormatId(QString name);
QStringList getEntryFlagNames(int flags);

constexpr qint64 PEGENTRY_BINSIZE = 72;
class PegEntry
{
    friend PegFile;

public:
    PegEntry();
    PegEntry(PegFile& parent);
    void read13(QIODevice& stream);
    void read19(QIODevice& stream);
    void write13(QIODevice& stream, qint64 data_offset) const;
    void write19(QIODevice& stream, qint64 data_offset) const;
    void fromDDS(const DDSFile& ddsfile);
    void fromTGA(const TGAFile& tgafile, TextureFormat fmt);
    DDSFile toDDS() const;
    TGAFile toTGA() const;

    qint64 offset; // File position of texture data
    int width; // Width of texture
    int height; // Height of texture
    TextureFormat bm_fmt; // Texture format (see TextureFormat enum).
    int pal_fmt; // Not used by engine. Always 0.
    int anim_tiles_width; // For animated textures using an anim sheet BM_F_ANIM_SHEET
    int anim_tiles_height;
    int num_frames; // Not used by engine. Always 1.
    int depth;
    int flags; // Various flags for texture. (See BM_F_* constants).
    int pal_size; // Not used by engine. Always 0.
    int fps; // Not used by engine. Always 1.
    int mip_levels; // Number of mipmaps in texture + 1 for the base image.
    qint64 data_size; // Size of the texture data.
    HDRColor avg_color;
    int num_mips_split;
    quint32 data_max_size;

    PegFile* m_parent;
    QString filename;
    QByteArray data;
};

constexpr uint qHash(const TextureFormat& key, uint seed)
{
    return ::qHash(static_cast<int>(key), seed);
}

}
