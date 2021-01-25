#pragma once

#include <array>

#include "glew.h"

#include <filesystem.h>

bool png_texture_load(std::ostream & log, const lb::filesystem::path & fs_path, GLuint texture, int * width, int * height);


inline std::array<float, 3> color3fFromHex(uint8_t r, uint8_t g, uint8_t b) {
    return {float(r) / 255.f,float(g) / 255.f,float(b) / 255.f};
}

inline void color3fGL(std::array<float, 3> color_array) {
    glColor3f(color_array[0], color_array[1], color_array[2]);
}