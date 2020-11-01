#include "image.h"

#include <XPLMGraphics.h>

#include "gl_utils.h"

void Widget11Image::draw(Rect) {

    float left = bounds_.left;
    float top = bounds_.top;
    float right = bounds_.right;
    float bottom = bounds_.bottom;
    XPLMSetGraphicsState(0, 1, 0, 0, 1, 0, 0);

    glColor4f(1.f, 1.f, 1.f, 1.f);
    XPLMBindTexture2d(texture_id, 0);

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(1.f, 1.f);
    glVertex3f(right, top, 0);
    glTexCoord2f(1.f, 0.f);
    glVertex3f(right, bottom, 0);
    glTexCoord2f(0.f, 1.f);
    glVertex3f(left, top, 0);
    glTexCoord2f(0.f, 0.f);
    glVertex3f(left, bottom, 0);
    glEnd();
}

bool Widget11Image::load(std::ostream & log, const lb::filesystem::path & path) {
    XPLMGenerateTextureNumbers(&texture_id, 1);

    if(false == png_texture_load(log, path, texture_id, &image_width, &image_height)) {
        log << "Couldn't load texture file " << path << "\n";
        return false;
    }

    return true;
}
