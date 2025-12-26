#include <algorithm>
#include <cmath>
#include "LaserFrameGenerator.h"

static float smootherstep(float t)
{
    // Clamp to avoid weirdness if t is slightly out of range
    if (t < 0.0f)
        t = 0.0f;
    if (t > 1.0f)
        t = 1.0f;

    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    // 6t^5 - 15t^4 + 10t^3
}

static float easeGalvo(float t)
{
    // Clamp to valid range (guard for float edge cases)
    t = std::clamp(t, 0.0f, 1.0f);

    // Galvo-tuned parameters
    constexpr float k = 10.0f;   // stiffness / inertia factor (6–14 typical)
    constexpr float b = 0.5f;    // midpoint balancing

    // Logistic sigmoid (smooth acceleration & braking)
    float x = (t - b) * k;
    float y = 1.0f / (1.0f + expf(-x));

    // Normalize to 0–1
    return (y - 1.0f / (1.0f + expf(k * b))) /
        ((1.0f / (1.0f + expf(-k * (1.0f - b)))) - (1.0f / (1.0f + expf(k * b))));
}

float easeGalvoStrong(float t)
{
    return powf(easeGalvo(t), 0.85f); // lower exponent = heavier braking
}

//static float easeInOut(float t)
//{
//    if (t < 0.5f)
//    {
//        return 2.0f * t * t * t * t; // Simplified from (t*2)^4 / 2
//    }
//    else
//    {
//        t -= 1.0f;
//        return -2.0f * t * t * t * t + 1.0f; // Simplified from 1 - (-2*t)^4 / 2
//    }
//}

float easeInOut(float t)
{
    return (t < 0.5f) ?
        4.0f * t * t * t :                      // cubic instead of quartic
        1.0f - powf(-2.0f * t + 2.0f, 3) * 0.5f;
}

Point2D LaserFrameGenerator::Lerp(Point2D next, float t) const
{
    Point2D d = next - m_prev;
    return m_prev + (d * t);
}

void LaserFrameGenerator::ClampPoint2D(Point2D& p)
{
    p.x = std::clamp(p.x, -m_MaxValue, m_MaxValue);
    p.y = std::clamp(p.y, -m_MaxValue, m_MaxValue);
}

void LaserFrameGenerator::LineTo(Point2D next, LaserState laserstate, PointSharpness pointsharpness, LaserColor color)
{
    next *= (m_MaxValue);
    ClampPoint2D(next);
    Point2D d = m_prev - next;
    float length = d.Length();
    size_t steps = std::max<size_t>(1, static_cast<size_t>(length / m_averagePointSpacing));
    for (size_t i = 0; i <= steps; i++)
    {
        float t = float(i) / float(steps);
        float easedT = (pointsharpness == PointSharpness::SHARP) ? easeGalvoStrong(t) : t;
        Point2D ipoint = Lerp(next, easedT);
        LaserPoint p {};
        p.x = (int16_t)ipoint.x;
        p.y = (int16_t)ipoint.y;
		// Color interpolation
        LaserColor::RGB8 colors = color.getRGB(easedT);
        p.r = colors.r;
        p.g = colors.g;
        p.b = colors.b;
		p.flags = (laserstate == LaserState::ON) ? true : false;
        m_Frame.push_back(p);
    }
	// Dwell, add a few extra points to ensure laser lingers
    if (pointsharpness == PointSharpness::SHARP)
    {
        for (size_t i = 0; i < 8; i++)
        {
            LaserPoint p {};
            p.x = (int16_t)next.x;
            p.y = (int16_t)next.y;
            LaserColor::RGB8 colors = color.getRGB(1.0f);
            p.r = colors.r;
            p.g = colors.g;
            p.b = colors.b;
            p.flags = (laserstate == LaserState::ON) ? true : false;
            m_Frame.push_back(p);
        }
    }
    m_prev = next;
}

void LaserFrameGenerator::ArcTo(Point2D center, Point2D next, bool LaserON, LaserColor color, bool ccw)
{

}
