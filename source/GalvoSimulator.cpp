#include "GalvoSimulator.h"
#include <algorithm>
#include <cmath>
#include <iostream>

static float constexpr DEG_TO_RAD = 0.01745329251994f;

GalvoSimulator::GalvoSimulator()
{
    // galvo physics state
    AngleX = 0;
    AngleY = 0;
    AngularVelX = 0;
    AngularVelY = 0;

    // physical properties (tunable)
    damping = 20.0f;
    stiffness = 1000.0f;
    maxSpeed = 50.0f;
    maxAngle = 30.0f;
    scaleFactor = 1.0f;
    toleranceSq = 0.01f;
    frameIndex = 0;
    SetMaxAngle(20);
}

void GalvoSimulator::LoadFrame(LaserFrame&& lF)
{
    std::lock_guard<std::mutex> lock(mtx);
    //std::cout << "Loading frame, size=" << lF.size() << std::endl;
    lFrame = std::move(lF);
    rFrame.clear();
    rFrame.reserve(lFrame.size());
    frameIndex = 0;

}

void GalvoSimulator::SetMaxAngle(float newMaxAngle)
{
    std::lock_guard<std::mutex> lock(mtx);
	maxAngle = newMaxAngle;
    scaleFactor = 1.0f / tan(maxAngle * 0.01745329251994f);
}

void GalvoSimulator::Simulate(float dt)
{
    std::lock_guard<std::mutex> lock(mtx);
    while(Step(dt));
}

bool GalvoSimulator::Step(float dt)
{
    if (lFrame.empty())
        return false;

    // get target
    const LaserPoint& target = lFrame[frameIndex];
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


    color.addHue(0.25f);
    CalcScreenPositions();
    rFrame.push_back({ screenX, screenY, color.getR(), color.getG(), color.getB(), target.flags});

    // advance to next target point
    if ((dx * dx + dy * dy) < toleranceSq)
    {
        frameIndex = (frameIndex + 1);
        color.setHSV(frameIndex * 20.0f, 1.0f, 1.0f);

    }
    return frameIndex < lFrame.size();
}

float GalvoSimulator::ConvertAngle(const int16_t angle) const
{
    return (float(angle) / 32768) * maxAngle;
}

RenderFrame GalvoSimulator::getRenderFrame()
{
    std::lock_guard<std::mutex> lock(mtx);
    return rFrame;
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
