#pragma once
#include <cmath>

struct Point2D
{
    float x { 0.0f };
    float y { 0.0f };

    Point2D() noexcept = default;
    Point2D(float x_, float y_) noexcept : x(x_), y(y_) {}

    [[nodiscard]] float Length() const noexcept
    {
        return std::sqrt(x * x + y * y); // sqrt is NOT constexpr in C++17/C++20
    }

    [[nodiscard]] Point2D Normalized() const noexcept
    {
        float len = Length();
        if (len == 0.0f) return { 0.0f, 0.0f };
        return { x / len, y / len };
    }

    [[nodiscard]] Point2D Perpendicular() const noexcept
    {
        return { -y, x };
    }

    [[nodiscard]] Point2D Rotate(float angleRad) const noexcept
    {
        float cosA = std::cos(angleRad); // cos/sin not constexpr in standard C++20
        float sinA = std::sin(angleRad);
        return { x * cosA - y * sinA, x * sinA + y * cosA };
    }

    Point2D& operator*=(float v) noexcept
    {
        x *= v;
        y *= v;
        return *this;
    }

    Point2D& operator+=(const Point2D& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Point2D& operator-=(const Point2D& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
};


inline Point2D operator+(Point2D lhs, const Point2D& rhs) noexcept
{
    lhs += rhs;
    return lhs;
}

inline Point2D operator-(Point2D lhs, const Point2D& rhs) noexcept
{
    lhs -= rhs;
    return lhs;
}

inline Point2D operator*(Point2D lhs, float v) noexcept
{
    lhs *= v;
    return lhs;
}

inline Point2D operator*(float v, Point2D rhs) noexcept
{
    rhs *= v;
    return rhs;
}