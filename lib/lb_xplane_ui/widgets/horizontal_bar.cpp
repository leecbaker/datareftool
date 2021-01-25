#include "horizontal_bar.h"

#include <XPLMGraphics.h>

#include "../gl_utils.h"

void Widget11HorizontalBar::draw(Rect) {
    XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);
    color3fGL(color3fFromHex(0x60, 0x64, 0x6b));

    glBegin(GL_LINES);
    glLineWidth(1.0);
    glVertex3f(bounds_.left, bounds_.bottom, 0.f);
    glVertex3f(bounds_.right, bounds_.bottom, 0.f);
    glEnd();
}
