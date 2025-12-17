#pragma once

#include <vector>
#include <cstdint>
#include <d2d1.h>
#include <wrl.h>

struct SimPoint
{
    float x;   // -1 .. 1
    float y;   // -1 .. 1
    uint8_t r, g, b;
    uint8_t flags; // bit0 = laser on
};

using SimFrame = std::vector<SimPoint>;

struct LaserRendererSettings
{
    int width = 800;
    int height = 800;

    float beamRadius = 0.25f; // pixels
    float exposure = 1.0f;
    float gamma = 2.2f;
};

class LaserRenderer
{
public:
    void Initialize(ID2D1RenderTarget* renderTarget);
    void Resize(int width, int height);
    void Clear();

    // Feed GalvoSim output
    void Accumulate(const SimFrame& frame);

    // Draw accumulated energy
    void Present();

private:
    struct EnergyPixel
    {
        float r = 0, g = 0, b = 0;
    };

    void createBitmap();
    void uploadBitmap();

    void depositSegment(const SimPoint& a, const SimPoint& b);
    void depositPoint(float x, float y, float r, float g, float b, float dwell);

private:
    LaserRendererSettings m_settings;

    Microsoft::WRL::ComPtr<ID2D1RenderTarget> m_rt;
    Microsoft::WRL::ComPtr<ID2D1Bitmap>       m_bitmap;

    std::vector<EnergyPixel> m_energy;
};
