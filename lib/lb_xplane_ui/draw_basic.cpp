#include "draw_basic.h"

#include "gl_utils.h"

#include <XPLMGraphics.h>

#define TEXT_DEFAULT_HEIGHT 12

void drawRect(Rectf bounds) {
    float left = bounds.left;
    float top = bounds.top;
    float right = bounds.right;
    float bottom = bounds.bottom;
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(float(right), float(top), 0);
    glVertex3f(float(right), float(bottom), 0);
    glVertex3f(float(left), float(bottom), 0);
    glVertex3f(float(left), float(top), 0);
    glEnd();
};

void drawRoundRect(Rectf bounds) {
    float left = bounds.left;
    float top = bounds.top;
    float right = bounds.right;
    float bottom = bounds.bottom;
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(float(right + left) / 2.f, float(top + bottom) / 2.f, 0);
    glVertex3f(float(right - 4.f), float(top), 0);
    glVertex3f(float(right - 1.f), float(top - 1.f), 0);
    glVertex3f(float(right), float(top - 4.f), 0);
    glVertex3f(float(right), float(bottom + 4.f), 0);
    glVertex3f(float(right - 1.f), float(bottom + 1.f), 0);
    glVertex3f(float(right - 4.f), float(bottom), 0);
    glVertex3f(float(left + 4.f), float(bottom), 0);
    glVertex3f(float(left + 1.f), float(bottom + 1.f), 0);
    glVertex3f(float(left), float(bottom + 4.f), 0);
    glVertex3f(float(left), float(top - 4.f), 0);
    glVertex3f(float(left + 1.f), float(top - 1.f), 0);
    glVertex3f(float(left + 4.f), float(top), 0);
    glVertex3f(float(right - 4.f), float(top), 0);
    glEnd();
};

void drawString(std::array<float, 3> color, float size, Point location, const std::string & text) {
    glPushMatrix();
    glTranslatef(location.x, location.y, 0);
    float scale_factor = size / 8.f;
    glScalef(scale_factor, scale_factor, 1);

    XPLMDrawString(color.data(), 0, 0, const_cast<char *>(text.c_str()), nullptr, xplmFont_Proportional);

    glPopMatrix();
}

float measureString(float size, const std::string & text) {
    return XPLMMeasureString(xplmFont_Proportional, text.c_str(), text.size()) * size / 8.f;
}
