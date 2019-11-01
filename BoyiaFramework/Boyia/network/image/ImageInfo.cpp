#include "ImageInfo.h"
#include "jpeglib.h"
#include "png.h"
#include <stdio.h>

namespace yanbo {

typedef struct
{
    LByte* data;
    LInt size;
    LInt offset;
} ImageSource;

static void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    ImageSource* source = (ImageSource*)png_get_io_ptr(png_ptr);

    if (source->offset + length <= source->size) {
        LMemcpy(data, source->data + source->offset, length);
        source->offset += length;
    } else {
        png_error(png_ptr, "pngReaderCallback failed");
    }
}

ImageInfo::ImageInfo()
    : width(0)
    , height(0)
    , pixels(NULL)
{
}

ImageInfo::ImageInfo(const ImageInfo& info)
    : width(info.width)
    , height(info.height)
    , pixels(info.pixels)
{
}

LVoid ImageInfo::decodeImage(const String& data)
{
    LInt type = getType(GET_STR(data));
    switch (type) {
    case kImageJpeg:
        readJPEG(data.GetBuffer(), data.GetLength());
        break;
    case kImagePng:
        readPNG(data.GetBuffer(), data.GetLength());
        break;
    default:
        break;
    }
}

// jpeg FFD8FFE000104A464946
// png 89 50 4e 47 0d 0a 1a 0a 00 00
// gif 47494638396126026f01
LInt ImageInfo::getType(const char* data)
{
    if (data[0] == 0xFF
        && data[1] == 0xD8
        && data[2] == 0xFF) {
        return kImageJpeg;
    }

    if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4e && data[3] == 0x47) {
        return kImagePng;
    }

    return kImageNone;
}

LVoid ImageInfo::readJPEG(const LByte* data, size_t size)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    //初始化信息
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, size);
    (void)jpeg_read_header(&cinfo, TRUE);

    height = cinfo.image_height;
    width = cinfo.image_width;

    pixels = new LByte[cinfo.image_width * cinfo.image_height * cinfo.num_components];
    jpeg_start_decompress(&cinfo);

    JSAMPROW row_pointer[1];
    while (cinfo.output_scanline < cinfo.output_height) {
        /* row_pointer[0] = &m_bgr[(cinfo.output_height-cinfo.output_scanline-1)
	     * cinfo.image_width*cinfo.num_components];*/
        row_pointer[0] = &pixels[cinfo.output_scanline * cinfo.image_width * cinfo.num_components];
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    //sendMessage(width, height, pixels);
}

LVoid ImageInfo::readPNG(const LByte* data, size_t size)
{
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png_ptr) {
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, 0, 0);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        return;
    }

    png_set_sig_bytes(png_ptr, 0);

    ImageSource imgsource;
    imgsource.data = (LByte*)data;
    imgsource.size = size;
    imgsource.offset = 0;
    png_set_read_fn(png_ptr, &imgsource, pngReadCallback);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_ALPHA, 0);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    LInt color_type = png_get_color_type(png_ptr, info_ptr);
    LInt bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    //png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

    //LInt rowBytes = info_ptr->rowbytes;
    //LInt palette = info_ptr->palette;
    //LInt rowBytes = png_get_rowbytes(png_ptr, info_ptr);
    //LInt palette = png_get_palette_max(png_ptr, info_ptr);

    // Convert stuff to appropriate formats!
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_packing(png_ptr);
        png_set_palette_to_rgb(png_ptr); //Expand data to 24-bit RGB or 32-bit RGBA if alpha available.
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        //png_set_gray_1_2_4_to_8(png_ptr);
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);

    //Expand paletted or RGB images with transparency to full alpha channels so the data will be available as RGBA quartets.
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    // png_read_update_info(png_ptr, info_ptr);
    pixels = new LByte[width * height * 4]; //each pixel(RGBA) has 4 bytes
    // png_bytep* row_pointers = (png_bytep*)png_malloc(sizeof(png_bytep) * height);
    // for (int y = 0; y < height; y++) {
    //     row_pointers[y] = (png_bytep)png_malloc(width << 2); //each pixel(RGBA) has 4 bytes
    // }
    // png_read_image(png_ptr, row_pointers);

    png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

    // unlike store the pixel data from top-left corner, store them from bottom-left corner for OGLES Texture drawing...
    int pos = (width * height * 4) - (4 * width);

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < (4 * width); col += 4) {
            pixels[pos++] = row_pointers[row][col]; // red
            pixels[pos++] = row_pointers[row][col + 1]; // green
            pixels[pos++] = row_pointers[row][col + 2]; // blue
            pixels[pos++] = row_pointers[row][col + 3]; // alpha
        }
        pos = (pos - (width * 4) * 2); //move the pointer back two rows
    }

    // LInt pos = 0;
    // if (PNG_COLOR_TYPE_GRAY == color_type) {
    //     m_pixels = new LByte[width * height];
    //     LMemset(m_pixels, 0, width * height);
    //     for (int i = 0; i < height; i++) {
    //         for (int j = 0; j < width; j += 1) {
    //             m_pixels[pos++] = row_pointers[i][j];
    //         }
    //     }
    // } else if (PNG_COLOR_TYPE_RGB == color_type) {
    //     m_pixels = new LByte[width * height * 3];
    //     LMemset(m_pixels, 0, width * height * 3);

    //     //int pos = (width * height * 4) - (4 * width);
    //     for (int i = 0; i < height; i++) {
    //         for (int j = 0; j < 3 * width; j += 3) {
    //             m_pixels[pos++] = row_pointers[i][j + 2]; //BLUE
    //             m_pixels[pos++] = row_pointers[i][j + 1]; //GREEN
    //             m_pixels[pos++] = row_pointers[i][j]; //RED
    //         }
    //     }

    //     //pos=(pos - (width * 4)*2); //move the pointer back two rows
    // } else if (PNG_COLOR_TYPE_RGBA == color_type) {
    //     m_pixels = new LByte[width * height * 4];
    //     LMemset(m_pixels, 0, width * height * 4);
    //     for (int i = 0; i < height; i++) {
    //         for (int j = 0; j < 4 * width; j += 4) {
    //             m_pixels[pos++] = row_pointers[i][j]; //RED
    //             m_pixels[pos++] = row_pointers[i][j + 1]; //GREEN
    //             m_pixels[pos++] = row_pointers[i][j + 2]; //BLUE
    //             m_pixels[pos++] = row_pointers[i][j + 3]; //ALPHA
    //         }
    //     }
    // }

    // free memory
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
}
}