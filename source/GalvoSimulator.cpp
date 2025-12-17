#include "GalvoSimulator.h"

void GalvoSimulator::Simulate(float dt)
{
    sFrame.clear();
    sFrame.reserve(lFrame.size());
    frameIndex = 0;
    while(Step(dt));
}

bool GalvoSimulator::Step(float dt)
{
    if (lFrame.empty())
        return false;

    // get target
    const LaserFrameGenerator::LaserPoint& target = lFrame[frameIndex];
    float txa = std::clamp(ConvertToAngle(target.x), -maxAngle, maxAngle);
    float tya = std::clamp(ConvertToAngle(target.y), -maxAngle, maxAngle);

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
    sFrame.push_back({ screenX, screenY, target.r, target.g, target.b, target.flags});

    // advance to next target point
    if ((dx * dx + dy * dy) < toleranceSq)
        frameIndex = (frameIndex + 1);
    return frameIndex < lFrame.size();
}
