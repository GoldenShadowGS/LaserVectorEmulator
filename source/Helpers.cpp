#include "Helpers.h"

RGB HSVtoRGB(float H, float S, float V)
{
    float C = V * S;               // chroma
    float X = C * (1 - fabs(fmod(H / 60.0, 2) - 1));
    float m = V - C;

    float r, g, b;

    if (H < 60) { r = C; g = X; b = 0; }
    else if (H < 120) { r = X; g = C; b = 0; }
    else if (H < 180) { r = 0; g = C; b = X; }
    else if (H < 240) { r = 0; g = X; b = C; }
    else if (H < 300) { r = X; g = 0; b = C; }
    else { r = C; g = 0; b = X; }

    return {
        int((r + m) * 255.0f),
        int((g + m) * 255.0f),
        int((b + m) * 255.0f)
    };
}

ColorHSV::ColorHSV(float hue, float sat, float val)
    : h(hue), s(sat), v(val)
{
    wrapHue();
    updateRGB();
}

void ColorHSV::setHSV(float hue, float sat, float val)
{
    h = hue; s = sat; v = val;
    wrapHue();
    updateRGB();
}

void ColorHSV::addHue(float delta)
{
    h += delta;
    wrapHue();
    updateRGB();   // <── recompute immediately
}

uint8_t ColorHSV::getR() const { return r8; }
uint8_t ColorHSV::getG() const { return g8; }
uint8_t ColorHSV::getB() const { return b8; }

void ColorHSV::wrapHue()
{
    if (h < 0)
        h += 360.0f * std::ceil(-h / 360.0f);
    else if (h >= 360) 
        h = fmod(h, 360.0f);
}

void ColorHSV::updateRGB()
{
    float r, g, b;
    HSVtoRGB(r, g, b);

    r8 = uint8_t(std::clamp(int(r * 255 + 0.5f), 0, 255));
    g8 = uint8_t(std::clamp(int(g * 255 + 0.5f), 0, 255));
    b8 = uint8_t(std::clamp(int(b * 255 + 0.5f), 0, 255));
}

void ColorHSV::HSVtoRGB(float& outR, float& outG, float& outB) const
{
    if (s == 0.0f)
    {
        outR = outG = outB = v;
        return;
    }

    float hh = h / 60.0f;
    int sector = int(std::floor(hh));
    float f = hh - sector;

    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (sector)
    {
    case 0: outR = v; outG = t; outB = p; break;
    case 1: outR = q; outG = v; outB = p; break;
    case 2: outR = p; outG = v; outB = t; break;
    case 3: outR = p; outG = q; outB = v; break;
    case 4: outR = t; outG = p; outB = v; break;
    default: // 5
        outR = v; outG = p; outB = q; break;
    }
}
