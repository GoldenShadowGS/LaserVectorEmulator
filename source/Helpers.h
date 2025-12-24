#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>

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

struct RGB { int r, g, b; };

RGB HSVtoRGB(float H, float S, float V);

class ColorHSV
{
public:
    float h;   // 0–360
    float s;   // 0–1
    float v;   // 0–1

    uint8_t r8, g8, b8;

    ColorHSV(float hue = 0.0f, float sat = 1.0f, float val = 1.0f);
    void setHSV(float hue, float sat, float val);
    void addHue(float delta);
    uint8_t getR() const;
    uint8_t getG() const;
    uint8_t getB() const;

private:
    void wrapHue();
    void updateRGB();
    void HSVtoRGB(float& outR, float& outG, float& outB) const;
};
