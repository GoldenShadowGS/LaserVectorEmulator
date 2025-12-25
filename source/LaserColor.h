#pragma once
#include <cstdint>
#include <algorithm>
#include <cmath>

class LaserColor
{
public:
    struct RGB8
    {
        uint8_t r, g, b;
    };

    // Solid color
    LaserColor(float h, float s, float v) noexcept : m_h0(h), m_h1(h), m_s0(s), m_s1(s), m_v0(v), m_v1(v) {}

    // Gradient
    LaserColor(float h0, float h1, float s0, float s1, float v0, float v1) noexcept : m_h0(h0), m_h1(h1), m_s0(s0), m_s1(s1), m_v0(v0), m_v1(v1) {}

    // Solid color (t=0)
    RGB8 getRGB() const noexcept
    {
        return HSVtoRGB(m_h0, m_s0, m_v0);
    }

    // Gradient color (t=0..1)
    RGB8 getRGB(float t) const noexcept
    {
        float clampedT = std::clamp(t, 0.0f, 1.0f);

        float hue = lerpHue(clampedT);
        float sat = std::lerp(m_s0, m_s1, clampedT);
        float val = std::lerp(m_v0, m_v1, clampedT);
        // perceptual brightness preserving
        //float val = std::sqrt(std::lerp(m_v0 * m_v0, m_v1 * m_v1, clampedT));

        return HSVtoRGB(hue, sat, val);
    }

private:
    float m_h0, m_h1;   // Hue 0–360
    float m_s0, m_s1;   // Saturation 0–1
    float m_v0, m_v1;   // Value 0–1

    // Linear interpolation with hue wrap
    float lerpHue(float t) const noexcept
    {
        float delta = std::fmod(m_h1 - m_h0 + 540.0f, 360.0f) - 180.0f;
        return std::fmod(m_h0 + delta * t + 360.0f, 360.0f);
    }

    // HSV ? RGB8 conversion
    static RGB8 HSVtoRGB(float h, float s, float v) noexcept
    {
        RGB8 color { 0,0,0 };

        if (s <= 0.0f)
        {
            uint8_t gray = static_cast<uint8_t>(std::clamp(int(v * 255.0f + 0.5f), 0, 255));
            color.r = color.g = color.b = gray;
            return color;
        }

        h = std::fmod(h, 360.0f);
        float hh = h / 60.0f;
        //int sector = static_cast<int>(std::floor(hh)) % 6;
        int sector = (static_cast<int>(std::floor(hh)) % 6 + 6) % 6;
        float f = hh - sector;

        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t2 = v * (1.0f - s * (1.0f - f));

        float r = 0, g = 0, b = 0;
        switch (sector)
        {
        case 0: r = v; g = t2; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t2; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t2; g = p; b = v; break;
        case 5: default: r = v; g = p; b = q; break;
        }

        color.r = static_cast<uint8_t>(std::clamp(int(r * 255.0f + 0.5f), 0, 255));
        color.g = static_cast<uint8_t>(std::clamp(int(g * 255.0f + 0.5f), 0, 255));
        color.b = static_cast<uint8_t>(std::clamp(int(b * 255.0f + 0.5f), 0, 255));

        return color;
    }
};
