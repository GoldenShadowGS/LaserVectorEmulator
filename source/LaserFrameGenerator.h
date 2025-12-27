#pragma once
#include <vector>
#include <cstdint>
#include <cmath>
#include "LaserColor.h"

struct LaserPoint
{
    int16_t x;
    int16_t y;
    uint8_t r, g, b;
    uint8_t flags; // 1 = laser on, 0 = blank
};
using LaserFrame = std::vector<LaserPoint>;

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




class LaserFrameGenerator
{
public:
    struct Distortion
    {
        float k1 = 0.0f;
        float k2 = 0.0f;
        float k3 = 0.0f;

        void apply(float& x, float& y) const
        {
            float r2 = x * x + y * y;
            float factor = 1.0f + k1 * r2 + k2 * r2 * r2 + k3 * r2 * r2 * r2;
            x *= factor;
            y *= factor;
        }
    };
	enum class PointSharpness
    {
        SHARP,
        SMOOTH
	};
    enum class LaserState
    {
        OFF,
        ON
	};
    LaserFrameGenerator(float maxextent) : m_MaxValue(32767 * maxextent), m_averagePointSpacing(m_MaxValue * 0.025f), m_prev(Point2D()) {}
    ~LaserFrameGenerator() {}
    void NewFrame() { m_Frame.clear(); }
    const LaserFrame& GetLaserFrame() { return m_Frame; }
    void SetAveragePointSpacing(float spacing) { m_averagePointSpacing = spacing; }
    void SetK1(float k1) { m_Distortion.k1 = k1; }
    void SetK2(float k2) { m_Distortion.k2 = k2; }
    void SetK3(float k3) { m_Distortion.k3 = k3; }
    void LineTo(Point2D next, LaserState laserstate, PointSharpness pointsharpness, LaserColor color);
    void ArcTo(Point2D center, Point2D next, bool LaserON, LaserColor color, bool ccw);
private:
    Point2D LerpTo(Point2D next, float t) const;
    void ClampPoint2D(Point2D& p);
    Distortion m_Distortion;
    LaserFrame m_Frame;
    Point2D m_prev;
    float m_MaxValue;
    float m_averagePointSpacing;
};
