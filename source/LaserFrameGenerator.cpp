#include <algorithm>
#include <cmath>
#include "LaserFrameGenerator.h"

static float easeInOut(float t)
{
    if (t < 0.5f)
        return 2.0f * t * t * t * t; // Simplified from (t*2)^4 / 2
    else
        t -= 1.0f;
        return -2.0f * t * t * t * t + 1.0f; // Simplified from 1 - (-2*t)^4 / 2
}

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

void LaserFrameGenerator::LineTo(float nextX, float nextY, bool LaserON, LaserColor color)
{
	// Implement LERP with smootherstep easing
    float dx = nextX - m_prevX;
    float dy = nextY - m_prevY;
    float lineLength = std::sqrt(dx * dx + dy * dy);
    size_t steps = std::max<size_t>(1, static_cast<size_t>(lineLength / m_averagePointSpacing));
    for (size_t i = 0; i <= steps; i++)
    {
        float t = float(i) / float(steps);
		float smoothed_t = smootherstep(t);
        float dxt = dx * t;
        float dyt = dy * t;
        float ix = m_prevX + dxt;
        float iy = m_prevY + dyt;
        LaserPoint p {};
        p.x = (int16_t)std::clamp(ix, -m_MaxValue, m_MaxValue);
        p.y = (int16_t)std::clamp(iy, -m_MaxValue, m_MaxValue);
        LaserColor::RGB8 colors = color.getRGB(t);
        p.r = colors.r;
        p.g = colors.g;
        p.b = colors.b;
        p.flags = LaserON;
        m_Frame.push_back(p);
    }
    m_prevX = std::clamp(nextX, -m_MaxValue, m_MaxValue);
    m_prevY = std::clamp(nextY, -m_MaxValue, m_MaxValue);
}

void LaserFrameGenerator::ArcTo(float centerX, float centerY, float nextX, float nextY, bool LaserON, LaserColor color, bool ccw)
{

}
