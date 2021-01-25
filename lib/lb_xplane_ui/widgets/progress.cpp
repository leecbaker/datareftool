#include "progress.h"

#include <cmath>

#include "../gl_utils.h"

#include "XPLMGraphics.h"

void Widget11Progress::draw(Rect) {
    int bar_bottom = bounds_.bottom;
    int bar_top = bounds_.top;
    int bar_left = bounds_.left;
    int bar_right = bounds_.right;

    int bar_overall_length = bar_right - bar_left;

    int bar_middle = bar_left + lrint(fraction_ * bar_overall_length);

    XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);

    color3fGL(color3fFromHex(0x16, 0x78, 0xb5)); //active
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(float(bar_middle), float(bar_top), 0);
    glVertex3f(float(bar_middle), float(bar_bottom), 0);
    glVertex3f(float(bar_left), float(bar_top), 0);
    glVertex3f(float(bar_left), float(bar_bottom), 0);
    glEnd();
    color3fGL(color3fFromHex(0x14, 0x1c, 0x26)); //inactive
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(float(bar_right), float(bar_top), 0);
    glVertex3f(float(bar_right), float(bar_bottom), 0);
    glVertex3f(float(bar_middle), float(bar_top), 0);
    glVertex3f(float(bar_middle), float(bar_bottom), 0);
    glEnd();
}
