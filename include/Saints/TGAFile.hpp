#include <QtCore/QtGlobal>
#include <QtCore/QIODevice>
#include <QtCore/QByteArray>
#include <QtCore/QVector>

#include "Colors.hpp"


namespace Saints {

enum class TGAImageType
{
    NONE = 0,
    INDEXED = 1,
    RGB = 2,
    GRAYSCALE = 3,
    INDEXED_RLE = 9,
    RGB_RLE = 10,
    GRAYSCALE_RLE = 11
};

class TGAFile
{
public:
    enum Attrib
    {
        PIXEL_ATTRIB_BYTES = 0xF,
        RESERVED = (1 << 4),
        SCREEN_ORIGIN = (1 << 5),
        DATA_INTERLEAVED = 0x3 << 6
    };

    const int ORIGIN_BOTTOM = (0 << 5);
    const int ORIGIN_TOP = (1 << 5);

    TGAFile();
    TGAFile(QIODevice& stream);
    void read(QIODevice& stream);
    void write(QIODevice& stream);
    static void checkDataType(TGAImageType data_type);
    static void checkBPP(int bits_per_pixel);
    static void swapRowOrder(QVector<LDRColor>& pixels, int width, int height);

    int id_length;
    int colormap_type;
    TGAImageType data_type;
    int colormap_offset;
    int colormap_length;
    int colormap_entry_size;
    int origin_x;
    int origin_y;
    int width;
    int height;
    int bits_per_pixel;
    int image_attributes;

    QByteArray image_id;
    QVector<LDRColor> pixels; // Pixels with top left origin
};

QByteArray read_rle(QIODevice& stream, qint64 size, qint64 bytes_per_pixel);

}
