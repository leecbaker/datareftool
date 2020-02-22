#pragma once

#include <array>

#include "XPLMDataAccess.h"

// This class assists in converting modelview coordinates to window coordinates,
// specifically for generating screen-space coordinates for the scissor test.
//
// It's based on the example code at 
// https://developer.x-plane.com/code-sample/sdk-3-0-window-scissoring-sample/

class ModelviewToWindowCoordinateConverter {
    XPLMDataRef mv_dref = nullptr;
    XPLMDataRef vp_dref = nullptr;
    XPLMDataRef pr_dref = nullptr;
public:
    ModelviewToWindowCoordinateConverter();

    // Convert rectangle coordinates in X-Plane's order: left top right bottom
    std::array<float, 4> convert(std::array<float, 4> mv_coords) const;
};
