#include "viewer_window_command_button.h"

#include "XPLMGraphics.h"
#include "XPWidgets.h"

#include "commandref.h"

#include <array>

#if IBM || LIN
#define GLEW_STATIC
#include "../lib/glew/glew.h"
#endif
#if APL
#include <OpenGL/gl.h>
#endif

enum class CommandButtonType {
    PRESS,
    HOLD,
    ONCE,
};

int commandButtonCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t /*inParam1*/, intptr_t /*inParam2*/, CommandButtonType button_type) {

    switch(inMessage) {
        case xpMsg_Draw:
            {
			    CommandRefRecord * crr = reinterpret_cast<CommandRefRecord *>(XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr));
                int left, top, right, bottom;
                XPGetWidgetGeometry(inWidget, &left, &top, &right, &bottom);

                XPWidgetID list = XPGetParentWidget(inWidget);
                int list_left, list_top, list_right, list_bottom;
                XPGetWidgetGeometry(list, &list_left, &list_top, &list_right, &list_bottom);

                XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);
                glDisable(GL_TEXTURE_2D);

				std::array<float,3> color_active = {{1.f, 1.f, 0.f}};
				std::array<float,3> color_inactive = {{0.f, 1.f, 0.f}};
                const std::array<float,3> & color = crr->isActivated() ? color_active : color_inactive;

                glColor4f(color[0], color[1], color[2], 1.f);
                glScissor(list_left, list_bottom, list_right - list_left, list_top - list_bottom);
                glEnable(GL_SCISSOR_TEST);

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

                switch(button_type) {
                    case CommandButtonType::ONCE:
                        XPLMDrawString(const_cast<float *>(color.data()), left + 5, bottom + 2, const_cast<char *>("Once"), nullptr, xplmFont_Basic);
                        break;
                    case CommandButtonType::PRESS:
                        XPLMDrawString(const_cast<float *>(color.data()), left + 5, bottom + 2, const_cast<char *>(crr->isActivated() ? "Holding" : "Press"), nullptr, xplmFont_Basic);
                        break;
                    case CommandButtonType::HOLD:
                        XPLMDrawString(const_cast<float *>(color.data()), left + 5, bottom + 2, const_cast<char *>(crr->isActivated() ? "End" : "Begin"), nullptr, xplmFont_Basic);
                        break;
                }

                glDisable(GL_SCISSOR_TEST);
            }
            return 1;

        case xpMsg_MouseDown:
            {
			    CommandRefRecord * crr = reinterpret_cast<CommandRefRecord *>(XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr));
                if(nullptr != crr) {

                switch(button_type) {
                    case CommandButtonType::ONCE:
                        crr->commandOnce();
                        break;
                    case CommandButtonType::PRESS:
                        crr->commandBegin();
                        break;
                    case CommandButtonType::HOLD:
                        if(crr->isActivated()) {
                            crr->commandEnd();
                        } else {
                            crr->commandBegin();
                        }
                        break;
                    }
                }
            }
            return 1;

        case xpMsg_MouseUp:
            {
			    CommandRefRecord * crr = reinterpret_cast<CommandRefRecord *>(XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr));
                if(nullptr != crr) {

                    switch(button_type) {
                        case CommandButtonType::PRESS:
                            crr->commandEnd();
                            break;
                        case CommandButtonType::HOLD:
                            break; //do nothing
                        case CommandButtonType::ONCE:
                            break; //do nothing
                    }
                }
            }
            return 1;

        case xpMsg_MouseDrag:
            return 1;
    }

    return 0;
}

int commandOnceButtonCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2) {
    return commandButtonCallback(inMessage, inWidget, inParam1, inParam2, CommandButtonType::ONCE);
}

int commandPressButtonCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2) {
    return commandButtonCallback(inMessage, inWidget, inParam1, inParam2, CommandButtonType::PRESS);
}

int commandHoldButtonCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2) {
    return commandButtonCallback(inMessage, inWidget, inParam1, inParam2, CommandButtonType::HOLD);
}