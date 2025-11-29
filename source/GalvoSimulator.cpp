#include "GalvoSimulator.h"
#include <algorithm>
#include <cmath>
#include <iostream>

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
    toleranceSq = 0.1f;

    frameIndex = 0;
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

void GalvoSimulator::Step(float dt)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (lFrame.empty())
        return;

    // get target
    const LaserPoint& target = lFrame[frameIndex];
    float txa = ConvertAngle(target.x);
    float tya = ConvertAngle(target.y);
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

    if (AngleX > maxAngle)
        AngleX = maxAngle;
    if (AngleX < -maxAngle)
        AngleX = -maxAngle;
    if (AngleY > maxAngle)
        AngleY = maxAngle;
    if (AngleY < -maxAngle)
        AngleY = -maxAngle;

    rFrame.push_back({ GetXPosition(), GetYPosition(), target.r, target.g, target.b, target.flags });

    // advance to next target point
    if ((dx * dx + dy * dy) < toleranceSq)
    {
        frameIndex = (frameIndex + 1) % lFrame.size();
    }
}

float GalvoSimulator::ConvertAngle(const int16_t angle) const
{
    return (float(angle) / 32768) * maxAngle;
}

float GalvoSimulator::GetXPosition() const
{
    return sin(AngleX * 0.01745329251994f);
}
float GalvoSimulator::GetYPosition() const
{
    return sin(AngleY * 0.01745329251994f);
}

RenderFrame GalvoSimulator::getRenderFrame()
{
    std::lock_guard<std::mutex> lock(mtx);
    return rFrame;
}