#pragma once
#include "LaserFrameGenerator.h"
#include "LaserRenderer.h"


class GalvoSimulator
{
    // galvo physics state
    const LaserFrameGenerator::LaserFrame& lFrame;
    SimFrame& sFrame;
    float scaleFactor;
    float AngleX, AngleY;
    float AngularVelX, AngularVelY;
    float screenX;
    float screenY;
    size_t frameIndex;

public:
    float damping;
    float stiffness;
    float maxSpeed;
    float toleranceSq;
    float maxAngle;
    GalvoSimulator(const LaserFrameGenerator::LaserFrame& lf, SimFrame& sf) :
        lFrame(lf), 
        sFrame(sf),
        scaleFactor(1.0f),
        AngleX(0.0f),
        AngleY(0.0f),
        AngularVelX(0.0f),
        AngularVelY(0.0f),
        screenX(0.0f),
        screenY(0.0f),
        frameIndex(0),
        damping(20.0f),
        stiffness(1000.0f),
        maxSpeed(50.0f),
        toleranceSq(0.01f),
        maxAngle(30.0f)
    {}

    // give it the next “ideal frame” from your animation
	//const SimFrame& getSimFrame() { return sFrame; }
    void Simulate(float dt);
    void SetMaxAngle(float newMaxAngle) 
    {
        maxAngle = newMaxAngle;  
        scaleFactor = 1.0f / tan(maxAngle * 0.01745329251994f);
    }

    // 30 kHz update
    bool Step(float dt);

    // physical properties (tunable)


private:
    float ConvertToAngle(const int16_t angle) const
    {
        return (float(angle) / 32768) * maxAngle;
    }
    void CalcScreenPositions()
    {
        // combined angular distance from center
        float r = std::sqrt(AngleX * AngleX + AngleY * AngleY);  // in degrees
        float theta = std::atan2(AngleY, AngleX);                // direction of point

        // map to tangent plane (radial distortion)
        float DEG_TO_RAD = 0.01745329251994f;
        float screenR = std::tan(r * DEG_TO_RAD);  // DEG_TO_RAD = 0.01745329251994f
        screenX = screenR * std::cos(theta) * scaleFactor;
        screenY = screenR * std::sin(theta) * scaleFactor;
    }

};
