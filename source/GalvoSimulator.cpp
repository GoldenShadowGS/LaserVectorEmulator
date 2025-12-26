#include "GalvoSimulator.h"
#include <algorithm>
#include <cmath>
#include <windows.h>
#include <string>


static float constexpr DEG_TO_RAD = 0.01745329251994f;

GalvoSimulator::GalvoSimulator()
{
    // galvo physics state
    AngleX = 0;
    AngleY = 0;
    AngularVelX = 0;
    AngularVelY = 0;

    // physical properties (tunable)
     
    stiffness = 1400.0f;
    damping = 95.0f;
    maxSpeed = 200.0f;
    toleranceSq = 0.5f; // tighter

    //damping = 20.0f;
    //stiffness = 1000.0f;
    //maxSpeed = 150.0f;
    //toleranceSq = 0.1f;

    maxAngle = 35.0f;
    scaleFactor = 1.0f;
    frameIndex = 0;
    SetMaxAngle(maxAngle);
}

void GalvoSimulator::SetMaxAngle(float newMaxAngle)
{
	maxAngle = newMaxAngle;
    scaleFactor = 1.0f / tan(maxAngle * 0.01745329251994f);
}

void GalvoSimulator::Simulate(const LaserFrame& frame, float dt)
{
    simFrame.clear();
    frameIndex = 0;
    while(Step(frame, dt));
	//std::string debugMsg = "Debug message: Simulated frame with " + std::to_string(simFrame.size()) + " points.\n";
 //   OutputDebugStringA(debugMsg.c_str());
}

bool GalvoSimulator::Step(const LaserFrame& frame, float dt)
{
    if (frame.empty())
        return false;
    // get target
    const LaserPoint& target = frame[frameIndex];

    //Temp Test
    //float x = std::clamp(ConvertAngle(target.x), -maxAngle, maxAngle);
    //float y = std::clamp(ConvertAngle(target.y), -maxAngle, maxAngle);
    //AngleX = x;
    //AngleY = y;
    //CalcScreenPositions();
    //simFrame.push_back({ screenX, screenY, target.r, target.g, target.b, target.flags });
    //frameIndex++;
    //return frameIndex < frame.size();

    float txa = std::clamp(ConvertAngle(target.x), -maxAngle, maxAngle);
    float tya = std::clamp(ConvertAngle(target.y), -maxAngle, maxAngle);

    // spring-damper model
    float dx = txa - AngleX;
    float dy = tya - AngleY;

    float ax = stiffness * dx - damping * AngularVelX;
    float ay = stiffness * dy - damping * AngularVelY;

    AngularVelX += ax * dt;
    AngularVelY += ay * dt;

    // clamp max speed
    float speed = std::sqrt(AngularVelX * AngularVelX + AngularVelY * AngularVelY);
    if (speed > maxSpeed)
    {
        float s = maxSpeed / speed;
        AngularVelX *= s;
        AngularVelY *= s;
    }

    AngleX += AngularVelX * dt;
    AngleY += AngularVelY * dt;

    CalcScreenPositions();
    simFrame.push_back({ screenX, screenY, target.r, target.g, target.b, target.flags});

    // advance to next target point
    static int hold = 0;
    if (dx * dx + dy * dy < toleranceSq)
    {
        if (++hold > 2) 
        { 
            frameIndex++; 
            hold = 0;
        }
    }
    return frameIndex < frame.size();
}

float GalvoSimulator::ConvertAngle(const int16_t angle) const
{
    return (float(angle) / 32768) * maxAngle;
}

void GalvoSimulator::CalcScreenPositions()
{
    // combined angular distance from center
    float r = std::sqrt(AngleX * AngleX + AngleY * AngleY);  // in degrees
    float theta = std::atan2(AngleY, AngleX);                // direction of point

    // map to tangent plane (radial distortion)
    float screenR = std::tan(r * DEG_TO_RAD);  // DEG_TO_RAD = 0.01745329251994f
    screenX = screenR * std::cos(theta) * scaleFactor;
    screenY = screenR * std::sin(theta) * scaleFactor;
}
