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
    void Simulate(float dt);
    void SetMaxAngle(float newMaxAngle);

    // 30 kHz update
    bool Step(float dt);

    // physical properties (tunable)
    float damping;
    float stiffness;
    float maxSpeed;
    float toleranceSq;
    float maxAngle;

private:
    float ConvertAngle(const int16_t angle) const;
    void CalcScreenPositions();
    // galvo physics state
    float scaleFactor;
    float AngleX, AngleY;
    float AngularVelX, AngularVelY;
    float screenX;
    float screenY;

	ColorHSV color; // point color cycling

    // target point stream
    LaserFrame lFrame;
    RenderFrame rFrame;
    size_t frameIndex;

    mutable std::mutex mtx;
};
