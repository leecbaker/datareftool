#pragma once

#include <array>

#include "geometry.h"

void drawRect(Rectf bounds);
void drawRoundRect(Rectf bounds);
void drawString(std::array<float, 3> color, float size, Point location, const std::string & text);
float measureString(float size, const std::string & text);
