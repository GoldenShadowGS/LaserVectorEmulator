#pragma once
#include <vector>
#include <cstdint>

struct LaserPoint
{
    int16_t x;
    int16_t y;
    uint8_t r, g, b;
    uint8_t flags; // 1 = laser on, 0 = blank
};

using LaserFrame = std::vector<LaserPoint>;

struct RenderPoint
{
    float x; // -1.0 , 1.0
    float y; // -1.0 , 1.0
    uint8_t r, g, b;
    uint8_t flags; // 1 = laser on, 0 = blank
};

using RenderFrame = std::vector<RenderPoint>;