#include "viewer_window_command_button.h"

#include "XPLMGraphics.h"
#include "XPWidgets.h"

#include "commandref.h"

#include <array>

#define GLEW_STATIC
#include "../lib/glew/glew.h"

#if APL
#include <OpenGL/gl.h>
#endif

int commandButtonCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t /*inParam1*/, intptr_t /*inParam2*/, bool isPress) {

    switch(inMessage) {
        case xpMsg_Draw:
            {
			    CommandRefRecord * crr = (CommandRefRecord *) XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr);
                int left, top, right, bottom;
                XPGetWidgetGeometry(inWidget, &left, &top, &right, &bottom);

                XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);
                glDisable(GL_TEXTURE_2D);

				std::array<float,3> color_active = {{1.f, 1.f, 0.f}};
				std::array<float,3> color_inactive = {{0.f, 1.f, 0.f}};
                const std::array<float,3> & color = crr->isActivated() ? color_active : color_inactive;

                glColor4f(color[0], color[1], color[2], 1.f);

                glBegin(GL_LINE_STRIP);
                glVertex3i(left+2, top, 0);
                glVertex3i(right - 2, top, 0);
                glVertex3i(right, top - 2, 0);
                glVertex3i(right, bottom+2, 0);
                glVertex3i(right - 2, bottom, 0);
                glVertex3i(left+2, bottom, 0);
                glVertex3i(left, bottom+2, 0);
                glVertex3i(left, top - 2, 0);
                glVertex3i(left+2, top, 0);
                glEnd();
                glEnable(GL_TEXTURE_2D);

                if(isPress) {
                    XPLMDrawString(const_cast<float *>(color.data()), left + 5, bottom + 3, const_cast<char *>(crr->isActivated() ? "Holding" : "Press"), nullptr, xplmFont_Basic);
                } else {
                    XPLMDrawString(const_cast<float *>(color.data()), left + 5, bottom + 3, const_cast<char *>(crr->isActivated() ? "End" : "Begin"), nullptr, xplmFont_Basic);
                }
            }
            return 1;

        case xpMsg_MouseDown:
            {
			    CommandRefRecord * crr = (CommandRefRecord *) XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr);
                if(nullptr != crr) {

                    if(isPress) {
                        crr->commandBegin();
                    } else {
                        if(crr->isActivated()) {
                            crr->commandEnd();
                        } else {
                            crr->commandBegin();
                        }
                    }
                }
            }
            return 1;

        case xpMsg_MouseUp:
            {
			    CommandRefRecord * crr = (CommandRefRecord *) XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr);
                if(nullptr != crr) {
                    if(isPress) {
                        crr->commandEnd();
                    }
                }
            }
            return 1;

        case xpMsg_MouseDrag:
            return 1;
    }

    return 0;
}


int commandPressButtonCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2) {
    return commandButtonCallback(inMessage, inWidget, inParam1, inParam2, true);
}

int commandHoldButtonCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2) {
    return commandButtonCallback(inMessage, inWidget, inParam1, inParam2, false);
}