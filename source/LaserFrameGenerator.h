#pragma once
#include <vector>
#include <cstdint>
#include "LaserColor.h"
#include "Point2D.h"

struct LaserPoint
{
    int16_t x;
    int16_t y;
    uint8_t r, g, b;
    uint8_t flags; // 1 = laser on, 0 = blank
};
using LaserFrame = std::vector<LaserPoint>;

class LaserFrameGenerator
{
public:
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
    enum class Arc
    {
        COUNTERCLOCKWISE,
        CLOCKWISE
    };
    LaserFrameGenerator(float maxextent, float maxAngle);
    ~LaserFrameGenerator() {}
    void NewFrame() { m_Frame.clear(); }
    const LaserFrame& GetLaserFrame() { return m_Frame; }
    void SetAveragePointSpacing(float spacing) { m_averagePointSpacing = spacing; }
    void LineTo(Point2D next, LaserState laserstate, PointSharpness pointsharpness, LaserColor color);
    void ArcTo(Point2D center, Point2D next, LaserState laserstate, PointSharpness pointsharpness, LaserColor color, Arc direction);
    void DrawShape(const std::vector<Point2D>& points, LaserColor color);
private:
	void DistortionCorrection(Point2D& p);
    float ConvertAngle(const float angle) const;
    Point2D LerpTo(Point2D next, float t) const;
    void ClampPoint2D(Point2D& p);
    LaserFrame m_Frame;
    Point2D m_prev;
    float m_MaxAngle;
    float m_MaxValue;
    float m_averagePointSpacing;
};
