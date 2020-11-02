#pragma once

#include "geometry.h"

#include <array>
#include <string>

void drawRect(Rectf bounds);
void drawRoundRect(Rectf bounds);
void drawString(std::array<float, 3> color, float size, Point location, const std::string & text);
float measureString(float size, const std::string & text);
