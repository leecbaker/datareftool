#include "text.h"

#include <XPLMGraphics.h>

// TODO: line wrap
void Widget11Text::setText(const std::string & text) {
    lines.clear();
    std::string::const_iterator text_begin = text.cbegin();
    while(text_begin != text.cend()) {
        //find next position
        std::string::const_iterator newline_pos = std::find(text_begin, text.cend(), '\n');
        if(newline_pos != text.cend()) {
            lines.emplace_back(text_begin, newline_pos);
            text_begin = newline_pos + 1;
        } else {
            lines.emplace_back(text_begin, text.cend());
            break;
        }
    }

    widest_line_width = 0;
    for(const std::string & line: lines) {
        widest_line_width = std::max<int>(widest_line_width, XPLMMeasureString(xplmFont_Proportional, line.c_str(), line.size()));
    }
}

void Widget11Text::draw(Rect) {
    int line_y = line_height * (lines.size() - 1) + bounds_.bottom + y_margin + 2;
    for(const std::string & line : lines) {
        XPLMDrawString(color_.data(), bounds_.left, line_y, const_cast<char *>(line.c_str()), nullptr, xplmFont_Proportional);
        line_y -= line_height;
    }
}
