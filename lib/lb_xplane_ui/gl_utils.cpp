#include "gl_utils.h"
#include <png.h>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "logging.h"

#include "XPLMGraphics.h"

void user_error_fn(png_structp /*png_ptr*/, png_const_charp error_msg) {
    xplog << "libPNG error: " << error_msg << "\n";
}

void user_warning_fn(png_structp /*png_ptr*/, png_const_charp warning_msg) {
    xplog << "libPNG warn: " << warning_msg << "\n";
}

bool png_texture_load(std::ostream & log, const lb::filesystem::path & file_name, GLuint texture, int * width, int * height) {
    assert(0 != texture);

    FILE * fp = fopen(file_name.string().c_str(), "rb");
    if(fp == 0) {
		log << "Error: fopen() failed to open file " << file_name << " because: " << std::strerror(errno) << "\n";
        return false;
    }

    // read the header
    png_byte header[8];
    fread(header, 1, 8, fp);

    if(png_sig_cmp(header, 0, 8)) {
        log << "error: " << file_name << " is not a PNG.\n";
        fclose(fp);
        return false;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, user_error_fn, user_warning_fn);
    if(!png_ptr) {
        log << "error: png_create_read_struct returned 0.\n";
        fclose(fp);
        return false;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        log << "error: png_create_info_struct returned 0.\n";
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        fclose(fp);
        return false;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if(!end_info) {
        log << "error: png_create_info_struct returned 0.\n";
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
        return false;
    }

    // the code in this if statement gets called if libpng encounters an error
    if(setjmp(png_jmpbuf(png_ptr))) {
        log << "error from libpng\n";
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return false;
    }

    // init png reading
    png_init_io(png_ptr, fp);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type, NULL, NULL, NULL);

    if(width) {
        *width = temp_width;
    }
    if(height) {
        *height = temp_height;
    }

    // printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);

    if(bit_depth != 8) {
        log << file_name << ": Unsupported bit depth " << bit_depth << ".  Must be 8.\n";
        return 0;
    }

    GLint format;
    switch(color_type) {
        case PNG_COLOR_TYPE_RGB: format = GL_RGB; break;
        case PNG_COLOR_TYPE_RGB_ALPHA: format = GL_RGBA; break;
        default: 
            log << file_name << ": Unknown libpng color type " << color_type << ".\n"; 
            return 0;
    }

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes - 1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    std::vector<png_byte> image_data;
    image_data.resize(rowbytes * temp_height + 15);

    // row_pointers is for pointing to image_data for reading the png with libpng
    std::vector<png_byte *> row_pointers;
    row_pointers.resize(temp_height);

    // set the individual row_pointers to point at the correct offsets of
    // image_data
    for(unsigned int i = 0; i < temp_height; i++) {
        row_pointers[temp_height - 1 - i] = image_data.data() + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers.data());

    XPLMBindTexture2d(texture, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, format, temp_width, temp_height, 0, format, GL_UNSIGNED_BYTE, image_data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NONE);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    return true;
}
