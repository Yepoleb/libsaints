#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QIODevice>
#include <QtCore/QString>
#include <QtCore/QByteArray>


namespace Saints {

struct DDSPixelformat;
enum class TextureFormat;


constexpr qint64 DDS_HEADER_SIZE = 124;
constexpr qint64 DDS_PIXELFORMAT_SIZE = 32;

// Pixelformat flags
// Source: Ddraw.h

constexpr quint32 DDPF_ALPHAPIXELS = 0x1;
constexpr quint32 DDPF_ALPHA = 0x2;
constexpr quint32 DDPF_FOURCC = 0x4;
constexpr quint32 DDPF_PALETTEINDEXED4 = 0x8;
constexpr quint32 DDPF_PALETTEINDEXEDTO8 = 0x10;
constexpr quint32 DDPF_PALETTEINDEXED8 = 0x20;
constexpr quint32 DDPF_RGB = 0x40;
constexpr quint32 DDPF_RGBA = 0x41;
constexpr quint32 DDPF_COMPRESSED = 0x80;
constexpr quint32 DDPF_RGBTOYUV = 0x100;
constexpr quint32 DDPF_YUV = 0x200;
constexpr quint32 DDPF_ZBUFFER = 0x400;
constexpr quint32 DDPF_PALETTEINDEXED1 = 0x800;
constexpr quint32 DDPF_PALETTEINDEXED2 = 0x1000;
constexpr quint32 DDPF_ZPIXELS = 0x2000;
constexpr quint32 DDPF_STENCILBUFFER = 0x4000;
constexpr quint32 DDPF_ALPHAPREMULT = 0x8000;
constexpr quint32 DDPF_LUMINANCE = 0x20000;
constexpr quint32 DDPF_BUMPLUMINANCE = 0x40000;
constexpr quint32 DDPF_BUMPDUDV = 0x80000;

// Header flags

constexpr quint32 DDSD_CAPS = 0x1;
constexpr quint32 DDSD_HEIGHT = 0x2;
constexpr quint32 DDSD_WIDTH = 0x4;
constexpr quint32 DDSD_PITCH = 0x8;
constexpr quint32 DDSD_BACKBUFFERCOUNT = 0x20;
constexpr quint32 DDSD_ZBUFFERBITDEPTH = 0x40;
constexpr quint32 DDSD_ALPHABITDEPTH = 0x80;
constexpr quint32 DDSD_LPSURFACE = 0x800;
constexpr quint32 DDSD_PIXELFORMAT = 0x1000;
constexpr quint32 DDSD_CKDESTOVERLAY = 0x2000;
constexpr quint32 DDSD_CKDESTBLT = 0x4000;
constexpr quint32 DDSD_CKSRCOVERLAY = 0x8000;
constexpr quint32 DDSD_CKSRCBLT = 0x10000;
constexpr quint32 DDSD_MIPMAPCOUNT = 0x20000;
constexpr quint32 DDSD_REFRESHRATE = 0x40000;
constexpr quint32 DDSD_LINEARSIZE = 0x80000;
constexpr quint32 DDSD_TEXTURESTAGE = 0x100000;
constexpr quint32 DDSD_FVF = 0x200000;
constexpr quint32 DDSD_SRCVBHANDLE = 0x400000;
constexpr quint32 DDSD_DEPTH = 0x800000;
constexpr quint32 DDSD_REQUIRED = 0x1007;
constexpr quint32 DDSD_ALL = 0xfff9ee;

// Caps flags

constexpr quint32 DDSCAPS_RESERVED1 = 0x1;
constexpr quint32 DDSCAPS_ALPHA = 0x2;
constexpr quint32 DDSCAPS_BACKBUFFER = 0x4;
constexpr quint32 DDSCAPS_COMPLEX = 0x8;
constexpr quint32 DDSCAPS_FLIP = 0x10;
constexpr quint32 DDSCAPS_FRONTBUFFER = 0x20;
constexpr quint32 DDSCAPS_OFFSCREENPLAIN = 0x40;
constexpr quint32 DDSCAPS_OVERLAY = 0x80;
constexpr quint32 DDSCAPS_PALETTE = 0x100;
constexpr quint32 DDSCAPS_PRIMARYSURFACE = 0x200;
constexpr quint32 DDSCAPS_RESERVED3 = 0x400;
constexpr quint32 DDSCAPS_SYSTEMMEMORY = 0x800;
constexpr quint32 DDSCAPS_TEXTURE = 0x1000;
constexpr quint32 DDSCAPS_3DDEVICE = 0x2000;
constexpr quint32 DDSCAPS_VIDEOMEMORY = 0x4000;
constexpr quint32 DDSCAPS_VISIBLE = 0x8000;
constexpr quint32 DDSCAPS_WRITEONLY = 0x10000;
constexpr quint32 DDSCAPS_ZBUFFER = 0x20000;
constexpr quint32 DDSCAPS_OWNDC = 0x40000;
constexpr quint32 DDSCAPS_LIVEVIDEO = 0x80000;
constexpr quint32 DDSCAPS_HWCODEC = 0x100000;
constexpr quint32 DDSCAPS_MODEX = 0x200000;
constexpr quint32 DDSCAPS_MIPMAP = 0x400000;
constexpr quint32 DDSCAPS_RESERVED2 = 0x800000;
constexpr quint32 DDSCAPS_ALLOCONLOAD = 0x4000000;
constexpr quint32 DDSCAPS_VIDEOPORT = 0x8000000;
constexpr quint32 DDSCAPS_LOCALVIDMEM = 0x10000000;
constexpr quint32 DDSCAPS_NONLOCALVIDMEM = 0x20000000;
constexpr quint32 DDSCAPS_STANDARDVGAMODE = 0x40000000;
constexpr quint32 DDSCAPS_OPTIMIZED = 0x80000000;

DDSPixelformat getPixelformat(TextureFormat fmt);
TextureFormat detectPixelformat(const DDSPixelformat& ddspf);

struct DDSPixelformat
{
    void read(QIODevice& stream);
    void write(QIODevice& stream) const;

    quint32 flags;
    quint32 four_cc; // Four character code for format identification
    quint32 rgb_bit_count; // Bits per pixel
    quint32 r_bitmask;
    quint32 g_bitmask;
    quint32 b_bitmask;
    quint32 a_bitmask;
};

class DDSFile
{
public:
    DDSFile();
    void open(QIODevice& stream);
    void write(QIODevice& stream) const;

    quint32 flags;
    quint32 height;
    quint32 width;
    quint32 pitch_or_linear_size;
    quint32 depth;
    quint32 mipmap_count;
    quint32 reserved1[11];
    DDSPixelformat ddspf;
    quint32 caps;
    quint32 caps2;
    quint32 caps3;
    quint32 caps4;
    quint32 reserved2;
    QByteArray data;
};

}
