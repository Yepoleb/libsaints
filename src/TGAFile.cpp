#include <QtCore/QtGlobal>
#include <QtCore/QIODevice>
#include <QtCore/QByteArray>
#include <QtCore/QVector>
#include <QtCore/QBuffer>
#include <algorithm>

#include "Saints/TGAFile.hpp"
#include "Saints/Exceptions.hpp"
#include "Saints/Colors.hpp"
#include "ByteIO.hpp"
#include "util.hpp"


namespace Saints {

QByteArray read_rle(QIODevice& stream, qint64 size, qint64 bytes_per_pixel)
{
    ByteReader reader(stream);
    QByteArray data;

    while (data.size() < size) {
        quint8 sect_header = reader.readU8();
        int sect_repeat = sect_header & (1 << 7);
        int sect_length = ((sect_header & bitmask(7)) + 1) * bytes_per_pixel;

        if (sect_repeat) {
            QByteArray color_value = reader.read(bytes_per_pixel);
            for (int i = 0; i < sect_length; i++) {
                data.append(color_value);
            }
        } else {
            data.append(reader.read(sect_length));
        }
    }

    return data;
}


TGAFile::TGAFile()
{
    id_length = 0;
    colormap_type = 0;
    data_type = TGAImageType::RGB;
    colormap_offset = 0;
    colormap_length = 0;
    colormap_entry_size = 0;
    origin_x = 0;
    origin_y = 0;
    width = 0;
    height = 0;
    bits_per_pixel = 32;
    image_attributes = 0x08; // 8-bit alpha
}

TGAFile::TGAFile(QIODevice& stream) : TGAFile()
{
    read(stream);
}

void TGAFile::read(QIODevice& stream)
{
    ByteReader reader(stream);

    int id_length = reader.readS8();
    colormap_type = reader.readS8();
    data_type = static_cast<TGAImageType>(reader.readS8());
    colormap_offset = reader.readS16();
    colormap_length = reader.readS16();
    colormap_entry_size = reader.readS8();
    origin_x = reader.readS16();
    origin_y = reader.readS16();
    width = reader.readS16();
    height = reader.readS16();
    bits_per_pixel = reader.readS8();
    image_attributes = reader.readS8();
    image_id = reader.read(id_length);

    checkDataType(data_type);
    checkBPP(bits_per_pixel);

    int bytes_per_pixel = bits_per_pixel / 8;
    qint64 num_bytes = width * height * bytes_per_pixel;

    QByteArray image_data;
    if (data_type == TGAImageType::RGB_RLE) {
        image_data = read_rle(stream, num_bytes, bytes_per_pixel);
    } else {
        image_data = reader.read(num_bytes);
    }
    QBuffer image_buffer(&image_data);
    image_buffer.open(QIODevice::ReadOnly);
    ByteReader image_reader(image_buffer);

    pixels.clear();
    for (int i_pixel = 0; i_pixel < width * height; i_pixel++) {
        LDRColor pixel;
        if (bits_per_pixel == 24) {
            pixel.b = image_reader.readU8();
            pixel.g = image_reader.readU8();
            pixel.r = image_reader.readU8();
            pixel.a = 0xFF;
        } else {
            pixel.b = image_reader.readU8();
            pixel.g = image_reader.readU8();
            pixel.r = image_reader.readU8();
            if (image_attributes & PIXEL_ATTRIB_BYTES) {
                pixel.a = image_reader.readU8();
            } else {
                pixel.a = 0xFF;
            }
        }
        pixels.append(pixel);
    }

    if ((image_attributes & SCREEN_ORIGIN) == ORIGIN_BOTTOM) {
        swapRowOrder(pixels, width, height);
    }
}

void TGAFile::write(QIODevice& stream)
{
    ByteWriter writer(stream);

    checkDataType(data_type);
    checkBPP(bits_per_pixel);
    if (data_type == TGAImageType::RGB_RLE) {
        throw ParsingError("Run-length encoding is not supported when writing");
    }
    if (pixels.size() != (width * height)) {
        throw ParsingError("Number of pixels does not match image dimensions");
    }

    writer.writeS8(image_id.size());
    writer.writeS8(colormap_type);
    writer.writeS8(static_cast<int>(data_type));
    writer.writeS16(colormap_offset);
    writer.writeS16(colormap_length);
    writer.writeS8(colormap_entry_size);
    writer.writeS16(origin_x);
    writer.writeS16(origin_y);
    writer.writeS16(width);
    writer.writeS16(height);
    writer.writeS8(bits_per_pixel);
    writer.writeS8(image_attributes);
    writer.write(image_id);

    QVector<LDRColor> write_pixels(pixels);
    if ((image_attributes & SCREEN_ORIGIN) == ORIGIN_BOTTOM) {
        swapRowOrder(write_pixels, width, height);
    }

    for (int i_pixel = 0; i_pixel < width * height; i_pixel++) {
        const LDRColor pixel = write_pixels[i_pixel];
        if (bits_per_pixel == 24) {
            writer.writeU8(pixel.b);
            writer.writeU8(pixel.g);
            writer.writeU8(pixel.r);
        } else {
            writer.writeU8(pixel.b);
            writer.writeU8(pixel.g);
            writer.writeU8(pixel.r);
            writer.writeU8(pixel.a);
        }
    }
}

void TGAFile::checkDataType(TGAImageType data_type)
{
    switch (data_type)
    {
    case TGAImageType::NONE:
        return;
    case TGAImageType::RGB:
    case TGAImageType::RGB_RLE:
        break;
    case TGAImageType::GRAYSCALE:
    case TGAImageType::GRAYSCALE_RLE:
        throw ParsingError("Grayscale images are not supported");
    case TGAImageType::INDEXED:
    case TGAImageType::INDEXED_RLE:
        throw ParsingError("Indexed images are not supported");
    default:
        throw ParsingError("Unknown image type");
    }
}

void TGAFile::checkBPP(int bits_per_pixel)
{
    if ((bits_per_pixel != 24) && (bits_per_pixel != 32)) {
        throw ParsingError("Only 24 and 32 bit images are supported");
    }
}

void TGAFile::swapRowOrder(QVector<LDRColor>& pixels, int width, int height)
{
    for (int i_top_row = 0; i_top_row < height / 2; i_top_row++) {
        int i_bottom_row = height - 1 - i_top_row;
        int top_pos = i_top_row * width;
        int bottom_pos = i_bottom_row * width;
        std::swap_ranges(
            pixels.begin() + top_pos,
            pixels.begin() + top_pos + width,
            pixels.begin() + bottom_pos
        );
    }
}

}
