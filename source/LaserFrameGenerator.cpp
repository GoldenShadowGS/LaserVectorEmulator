#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include "LaserFrameGenerator.h"

static float constexpr DEG_TO_RAD = 0.01745329251994f;
constexpr float PI = 3.14159265358979323846f;
constexpr float PI2 = 2.0f * PI;

static float GalvoEaseOut(float t)
{
    return (t ==  0.0f) ? 0.0f : 1.0f - float(std::pow(2, 10 * t - 10));
}

LaserFrameGenerator::LaserFrameGenerator(float maxextent, float maxAngle) : m_MaxAngle(maxAngle), m_MaxValue(32767 * maxextent), m_averagePointSpacing(0.025f), m_prev(Point2D()) {}

Point2D LaserFrameGenerator::LerpTo(Point2D next, float t) const
{
    Point2D d = next - m_prev;
    return m_prev + (d * t);
}

void LaserFrameGenerator::ClampPoint2D(Point2D& p)
{
    p.x = std::clamp(p.x, -m_MaxValue, m_MaxValue);
    p.y = std::clamp(p.y, -m_MaxValue, m_MaxValue);
}

float LaserFrameGenerator::ConvertAngle(const float angle) const
{
    return (float(angle) / 32768) * m_MaxAngle;
}

void LaserFrameGenerator::DistortionCorrection(Point2D& p)
{
    // Input: x,y in [-1,1]
    float r = sqrtf(p.x * p.x + p.y * p.y);
    float th = atan2f(p.y, p.x);

    // normalized radius -> angle space
    float angleDeg = r * m_MaxAngle;

    // *** distortion correction ***
    float correctedAngleDeg = atanf(angleDeg * DEG_TO_RAD) / DEG_TO_RAD;

    // angle space -> back to normalized
    float correctedR = correctedAngleDeg / m_MaxAngle;

    // build corrected coordinate
    p.x = correctedR * cosf(th);
    p.y = correctedR * sinf(th);
}

void LaserFrameGenerator::LineTo(Point2D next, LaserState laserstate, PointSharpness pointsharpness, LaserColor color)
{
    //next *= (m_MaxValue);
    Point2D d = m_prev - next;
    const float length = d.Length();
	const float segmentLength = length / m_averagePointSpacing;
    int steps = std::max<int>(1, static_cast<int>(segmentLength));
	int basesteps = (pointsharpness == PointSharpness::SHARP) ? steps - 1 : steps;
    for (int i = 0; i <= basesteps; i++)
    {
        float t = float(i) / float(steps);
        Point2D ipoint = LerpTo(next, t);
        DistortionCorrection(ipoint);
		ipoint *= m_MaxValue;
        ClampPoint2D(ipoint);
        LaserPoint p {};
        p.x = (int16_t)ipoint.x;
        p.y = (int16_t)ipoint.y;
        // Color interpolation
        LaserColor::RGB8 colors = color.getRGB(t);
        p.r = colors.r;
        p.g = colors.g;
        p.b = colors.b;
        p.flags = (laserstate == LaserState::ON) ? true : false;
        m_Frame.push_back(p);
    }
    // Dwell, add a few extra points to ensure laser lingers
    if (pointsharpness == PointSharpness::SHARP)
    {
	    float rampdownsteps = float(steps - 1);
	    float baseT = rampdownsteps / float(steps);
		const int brakingPoints =  6;
        for (int i = 0; i <= brakingPoints; i++)
        {
			float t = float(i) / float(brakingPoints);
			float easedT = baseT + (1.0f - baseT) * GalvoEaseOut(t);
            Point2D ipoint = LerpTo(next, easedT);
            DistortionCorrection(ipoint);
            ipoint *= m_MaxValue;
            ClampPoint2D(ipoint);
            LaserPoint p {};
            p.x = (int16_t)ipoint.x;
            p.y = (int16_t)ipoint.y;
            LaserColor::RGB8 colors = color.getRGB(easedT);
            p.r = colors.r;
            p.g = colors.g;
            p.b = colors.b;
            p.flags = (laserstate == LaserState::ON) ? true : false;
            m_Frame.push_back(p);
        }
        const int dwellPoints = 4;
        for (int i = 0; i < dwellPoints; i++)
        {
            Point2D ipoint = next;
            DistortionCorrection(ipoint);
            ipoint *= m_MaxValue;
            ClampPoint2D(ipoint);
            LaserPoint p {};
            p.x = (int16_t)ipoint.x;
            p.y = (int16_t)ipoint.y;
            LaserColor::RGB8 colors = color.getRGB(1.0);
            p.r = colors.r;
            p.g = colors.g;
            p.b = 255;
            p.flags = (laserstate == LaserState::ON) ? true : false;
            m_Frame.push_back(p);
        }
    }
    m_prev = next;
}

void LaserFrameGenerator::ArcTo(Point2D center, Point2D next, LaserState laserstate, PointSharpness pointsharpness, LaserColor color, Arc direction)
{
	Point2D radiusVecPrev = m_prev - center;
    Point2D radiusVecNext = next - center;
	float radius = radiusVecPrev.Length();
	float startAngle = std::atan2(radiusVecPrev.y, radiusVecPrev.x);
    float endAngle = std::atan2(radiusVecNext.y, radiusVecNext.x);
	float sweepangle = endAngle - startAngle;
    if (direction == Arc::COUNTERCLOCKWISE)
    {
        if (sweepangle >= 0.0f)
            sweepangle -= PI2;
    }
    else
    {
        if (sweepangle <= 0.0f)
            sweepangle += PI2;
	}
	float arclength = std::abs(sweepangle * radius);
	const float segmentLength = arclength / m_averagePointSpacing;
	int steps = std::max<int>(1, static_cast<int>(segmentLength));
    int basesteps = (pointsharpness == PointSharpness::SHARP) ? steps - 1 : steps;
    for (int i = 0; i <= basesteps; i++)
    {
        float t = float(i) / float(steps);
		Point2D ipoint = radiusVecPrev.Rotate(sweepangle * t) + center;
        DistortionCorrection(ipoint);
        ipoint *= m_MaxValue;
        ClampPoint2D(ipoint);
        LaserPoint p {};
        p.x = (int16_t)ipoint.x;
        p.y = (int16_t)ipoint.y;
        // Color interpolation
        LaserColor::RGB8 colors = color.getRGB(t);
        p.r = colors.r;
        p.g = colors.g;
        p.b = colors.b;
        p.flags = (laserstate == LaserState::ON) ? true : false;
        m_Frame.push_back(p);
    }
    if (pointsharpness == PointSharpness::SHARP)
    {
        float rampdownsteps = float(steps - 1);
        float baseT = rampdownsteps / float(steps);
        const int brakingPoints = 6;
        for (int i = 0; i <= brakingPoints; i++)
        {
            float t = float(i) / float(brakingPoints);
            float easedT = baseT + (1.0f - baseT) * GalvoEaseOut(t);
            Point2D ipoint = radiusVecPrev.Rotate(sweepangle * easedT) + center;
            //Point2D ipoint = LerpTo(next, easedT);
            DistortionCorrection(ipoint);
            ipoint *= m_MaxValue;
            ClampPoint2D(ipoint);
            LaserPoint p {};
            p.x = (int16_t)ipoint.x;
            p.y = (int16_t)ipoint.y;
            LaserColor::RGB8 colors = color.getRGB(easedT);
            p.r = colors.r;
            p.g = colors.g;
            p.b = colors.b;
            p.flags = (laserstate == LaserState::ON) ? true : false;
            m_Frame.push_back(p);
        }
        const int dwellPoints = 4;
        Point2D ipoint = radiusVecPrev.Rotate(sweepangle) + center;
        DistortionCorrection(ipoint);
        ipoint *= m_MaxValue;
        ClampPoint2D(ipoint);
        for (int i = 0; i < dwellPoints; i++)
        {
            LaserPoint p {};
            p.x = (int16_t)ipoint.x;
            p.y = (int16_t)ipoint.y;
            LaserColor::RGB8 colors = color.getRGB(1.0);
            p.r = colors.r;
            p.g = colors.g;
            p.b = 255;
            p.flags = (laserstate == LaserState::ON) ? true : false;
            m_Frame.push_back(p);
        }
    }
    m_prev = radiusVecPrev.Rotate(sweepangle) + center;
}

void LaserFrameGenerator::DrawShape(const std::vector<Point2D>& points, LaserColor color)
{
    for (int i = 0; i < points.size(); i++)
    {
        float t = float(i) / float(points.size());
        Point2D ipoint = points.at(i);
        DistortionCorrection(ipoint);
        ipoint *= m_MaxValue;
        ClampPoint2D(ipoint);
        LaserPoint p {};
        p.x = (int16_t)ipoint.x;
        p.y = (int16_t)ipoint.y;
        // Color interpolation
        LaserColor::RGB8 colors = color.getRGB(t);
        p.r = colors.r;
        p.g = colors.g;
        p.b = colors.b;
        p.flags = true;
        m_Frame.push_back(p);
    }
    m_prev = points.at(points.size()-1);
}