#pragma once
#include <vector>
#include <mutex>
#include <atomic>
#include <cstdint>
#include "Helpers.h"

class GalvoSimulator
{
public:
    GalvoSimulator();

    // give it the next “ideal frame” from your animation
    void LoadFrame(LaserFrame&& lF);
    RenderFrame getRenderFrame();

    // 30 kHz update
    void Step(float dt);

    // physical properties (tunable)
    float damping;
    float stiffness;
    float maxSpeed;
    float maxAngle;
    float toleranceSq;

private:
    float ConvertAngle(const int16_t angle) const;
    float GetXPosition() const;
    float GetYPosition() const; 
    // galvo physics state
    float AngleX, AngleY;
    float AngularVelX, AngularVelY;



    // target point stream
    LaserFrame lFrame;
    RenderFrame rFrame;
    size_t frameIndex;

    mutable std::mutex mtx;
};
