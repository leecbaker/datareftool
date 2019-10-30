#include "modelview_to_window.h"

ModelviewToWindowCoordinateConverter::ModelviewToWindowCoordinateConverter() {
    mv_dref = XPLMFindDataRef("sim/graphics/view/modelview_matrix");
    vp_dref = XPLMFindDataRef("sim/graphics/view/viewport");
    pr_dref = XPLMFindDataRef("sim/graphics/view/projection_matrix");
}

static std::array<float, 4> multMatrixVec4f(const std::array<float, 16> & m, const std::array<float, 4> & v)
{
    std::array<float, 4> dst;
    dst[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
    dst[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
    dst[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
    dst[3] = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];
    return dst;
}

static std::array<float, 2> modelview_to_window_coords(const std::array<float, 2> coordinates_modelview, const std::array<float, 16> & mv, const std::array<float, 16> & pr, const std::array<int, 4> & viewport) {
    const std::array<float, 4> in_mv{coordinates_modelview[0], coordinates_modelview[1], 0, 1};
    std::array<float, 4> eye = multMatrixVec4f(mv, in_mv);
    std::array<float, 4> ndc = multMatrixVec4f(pr, eye);
    ndc[3] = 1.0f / ndc[3];
    ndc[0] *= ndc[3];
    ndc[1] *= ndc[3];

    std::array<float, 2> out_w;
    out_w[0] = (ndc[0] * 0.5f + 0.5f) * viewport[2] + viewport[0];
    out_w[1] = (ndc[1] * 0.5f + 0.5f) * viewport[3] + viewport[1];
    return out_w;
}

std::array<float, 4> ModelviewToWindowCoordinateConverter::convert(std::array<float, 4> coordinates) {
    std::array<float, 16> mv, pr;
    std::array<int, 4> vp;
    XPLMGetDatavf(mv_dref, mv.data(), 0, 16);
    XPLMGetDatavf(pr_dref, pr.data(), 0, 16);
    XPLMGetDatavi(vp_dref, vp.data(), 0, 4);

    std::array<float, 2> left_top_modelview{coordinates[0], coordinates[1]};
    std::array<float, 2> right_bottom_modelview{coordinates[2], coordinates[3]};

    std::array<float, 2> left_top_window = modelview_to_window_coords(left_top_modelview, mv, pr, vp);
    std::array<float, 2> right_bottom_window = modelview_to_window_coords(right_bottom_modelview,  mv, pr, vp);

    return {left_top_window[0], left_top_window[1], right_bottom_window[0], right_bottom_window[1]};
}