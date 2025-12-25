#pragma once
#include <vector>
#include <cstdint>
#include "LaserColor.h"

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

private:
    Distortion m_Distortion;
    LaserFrame m_Frame;
    float m_MaxValue;
    float m_averagePointSpacing;
    float m_prevX;
    float m_prevY;
public:
    LaserFrameGenerator(float maxextent) : m_MaxValue(32767 * maxextent), m_averagePointSpacing(m_MaxValue * 0.02f), m_prevX(0.0f), m_prevY(0.0f) {}
    ~LaserFrameGenerator() {}
    void NewFrame() { m_Frame.clear(); }
    const LaserFrame& GetLaserFrame() { return m_Frame; }
	const float GetMaxValue() const { return m_MaxValue; }
    void SetAveragePointSpacing(float spacing) { m_averagePointSpacing = spacing; }
    void SetK1(float k1) { m_Distortion.k1 = k1; }
    void SetK2(float k2) { m_Distortion.k2 = k2; }
    void SetK3(float k3) { m_Distortion.k3 = k3; }
    void LineTo(float nextX, float nextY, bool LaserON, LaserColor color);
    void ArcTo(float centerX, float centerY, float nextX, float nextY, bool LaserON, LaserColor color, bool ccw);
};
