#pragma once

#include <algorithm>
#include <optional>

struct Point {
    int x, y;

    [[nodiscard]] Point translated(int x_offset, int y_offset) const {
        Point ret{x, y};
        ret.x += x_offset;
        ret.y += y_offset;
        return ret;
    }

    bool operator==(const Point & other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Point & other) const {
        return x != other.x || y != other.y;
    }
};

struct Size {
    int width, height;

    bool operator==(const Size & other) const {
        return width == other.width && height == other.height;
    }
    bool operator!=(const Size & other) const {
        return width != other.width || height != other.height;
    }
};

template <class RectType>
struct RectGeneric {
    RectType left = 0, top = 0, right = 0, bottom = 0;

    [[nodiscard]] bool overlaps(const RectGeneric & other) const {
        if(other.right < left || other.left > right || other.top < bottom || other.bottom > top) {
            return false;
        }

        return true;
    }

    [[nodiscard]] bool contains(const Point & pt) const {
        return left <= pt.x && pt.x <= right && bottom <= pt.y && pt.y <= top;
    }

    void translate(int x, int y) {
        left += x;
        right += x;
        top += y;
        bottom += y;
    }

    RectGeneric translated(int x, int y) const {
        RectGeneric ret = *this;
        ret.translate(x, y);
        return ret;
    }

    Size size() const {
        return Size{right - left, top - bottom};
    }

    bool operator!=(const RectGeneric & other) const {
        return left != other.left || right != other.right || top != other.top || bottom != other.bottom;
    }

    [[nodiscard]] std::optional<RectGeneric> intersection(const RectGeneric & other) const {
        if(other.right < left || other.left > right || other.top < bottom || other.bottom > top) {
            return std::nullopt;
        }

        return RectGeneric{std::max(left, other.left), std::min(top, other.top), std::min(right, other.right), std::max(bottom, other.bottom)};
    }
};

using Rect = RectGeneric<int>;
struct Rectf: public RectGeneric<float> {
    using RectGeneric::RectGeneric;
    Rectf(float left, float top, float right, float bottom): RectGeneric<float>{left, top, right, bottom} {}
    Rectf(Rect intrect): RectGeneric<float>(Rectf{float(intrect.left), float(intrect.top), float(intrect.right), float(intrect.bottom)}) {}
};

