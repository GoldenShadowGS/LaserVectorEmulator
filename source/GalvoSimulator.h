#pragma once
#include <vector>
#include <cstdint>
#include "LaserFrameGenerator.h"

struct SimPoint
{
    float x; // -1.0 , 1.0
    float y; // -1.0 , 1.0
    uint8_t r, g, b;
    uint8_t flags; // 1 = laser on, 0 = blank
};

using SimFrame = std::vector<SimPoint>;

class GalvoSimulator
{
public:
    GalvoSimulator(float maxAngle);
    void Simulate(const LaserFrame& frame, float dt);
	SimFrame& GetSimFrame() { return simFrame; }
private:
    void SetMaxAngle(float newMaxAngle);
    bool Step(const LaserFrame& frame, float dt);
    float ConvertAngle(const int16_t angle) const;
    void CalcScreenPositions();
    // physical properties (tunable)
    SimFrame simFrame;
    float damping;
    float stiffness;
    float maxSpeed;
    float toleranceSq;
    float m_maxAngle;
    // galvo physics state
    float scaleFactor;
    float AngleX, AngleY;
    float AngularVelX, AngularVelY;
    float screenX;
    float screenY;
    size_t frameIndex;
};
