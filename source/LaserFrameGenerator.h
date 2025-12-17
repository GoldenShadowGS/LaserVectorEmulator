#pragma once
#include <vector>
#include <cstdint>
#include "LaserColor.h"

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

    struct LaserPoint
    {
        int16_t x;
        int16_t y;
        uint8_t r, g, b;
        uint8_t flags; // 1 = laser on, 0 = blank
    };
    using LaserFrame = std::vector<LaserPoint>;
private:
	Distortion m_Distortion;
    LaserFrame m_Frame;
    float m_MaxValue;
    float m_pointSpacing;
	float m_lastX;
	float m_lastY;
    float m_currentdistance;
    float m_totaldistance;
public:
    LaserFrameGenerator() : m_MaxValue(32767 * 0.8f), m_pointSpacing(m_MaxValue * 0.01f), m_lastX(0.0f), m_lastY(0.0f), m_currentdistance(0.0f), m_totaldistance(0.0f) {}
    ~LaserFrameGenerator() {}
    void NewFrame() 
    { 
        m_Frame.clear(); 
		m_lastX = 0.0f;
		m_lastY = 0.0f;
    }
	void AddSquare(float xc, float yc, float size, LaserColor color);
	//void GenerateGrid(int rows, int cols, float spacing, LaserColor color);
	const LaserFrame& GetLaserFrame() { return m_Frame; } 
	void SetPointSpacing(float spacing) { m_pointSpacing = spacing; }
	void SetK1(float k1) { m_Distortion.k1 = k1; }
	void SetK2(float k2) { m_Distortion.k2 = k2; }
	void SetK3(float k3) { m_Distortion.k3 = k3; }
    void BlankTo(float x, float y);
	void SolidLineTo(float x, float y, LaserColor c);
	void ArcTo(float xc, float yc, float x, float y, LaserColor c0, LaserColor c1, bool ccw);
};
