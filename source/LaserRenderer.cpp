#include "LaserRenderer.h"
#include <algorithm>
#include <cmath>
#pragma comment(lib, "d2d1.lib")
#undef max
#undef min

using Microsoft::WRL::ComPtr;

static constexpr float EPS = 1e-6f;

void LaserRenderer::Initialize(ID2D1RenderTarget* renderTarget)
{
    m_rt = renderTarget;
}

void LaserRenderer::Resize(int w, int h)
{
    m_settings.width = w;
    m_settings.height = h;

    m_energy.assign(static_cast<size_t>(w) * static_cast<size_t>(h), EnergyPixel {});
    createBitmap();
}

void LaserRenderer::Clear()
{
    std::fill(m_energy.begin(), m_energy.end(), EnergyPixel {});
}

void LaserRenderer::Accumulate(const SimFrame& frame)
{
    if (frame.size() < 2)
        return;

    for (size_t i = 1; i < frame.size(); ++i)
    {
        const auto& a = frame[i - 1];
        const auto& b = frame[i];

        if (!(a.flags & 1) && !(b.flags & 1))
            continue;

        depositSegment(a, b);
    }
}

void LaserRenderer::Present()
{
    uploadBitmap();

    auto size = m_rt->GetSize();
    m_rt->DrawBitmap(
        m_bitmap.Get(),
        D2D1::RectF(0, 0, size.width, size.height),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
}

void LaserRenderer::createBitmap()
{
    D2D1_BITMAP_PROPERTIES props = {};
    props.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;
    props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
    props.dpiX = props.dpiY = 96.0f;

    m_rt->CreateBitmap(
        D2D1::SizeU(m_settings.width, m_settings.height),
        nullptr,
        m_settings.width * 4,
        &props,
        &m_bitmap);
}

void LaserRenderer::uploadBitmap()
{
    const int w = m_settings.width;
    const int h = m_settings.height;

    std::vector<uint8_t> rgba(w * h * 4);

    auto tonemap = [&] (float e) {
        float v = 1.0f - std::exp(-e * m_settings.exposure);
        return std::pow(v, 1.0f / m_settings.gamma);
        };

    for (int i = 0; i < w * h; ++i)
    {
        rgba[i * 4 + 0] = uint8_t(std::clamp(tonemap(m_energy[i].r), 0.f, 1.f) * 255);
        rgba[i * 4 + 1] = uint8_t(std::clamp(tonemap(m_energy[i].g), 0.f, 1.f) * 255);
        rgba[i * 4 + 2] = uint8_t(std::clamp(tonemap(m_energy[i].b), 0.f, 1.f) * 255);
        rgba[i * 4 + 3] = 255;
    }

    m_bitmap->CopyFromMemory(
        nullptr,
        rgba.data(),
        w * 4);
}

void LaserRenderer::depositSegment(const SimPoint& a, const SimPoint& b)
{
    float x0 = (a.x * 0.5f + 0.5f) * m_settings.width;
    float y0 = (a.y * 0.5f + 0.5f) * m_settings.height;
    float x1 = (b.x * 0.5f + 0.5f) * m_settings.width;
    float y1 = (b.y * 0.5f + 0.5f) * m_settings.height;

    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < EPS) return;

    constexpr int MAX_SUBSTEPS = 1;

    int steps = std::clamp(int(len), 1, MAX_SUBSTEPS);
    //int steps = std::max(1, int(len));
    float dwell = 1.0f / float(steps); // implicit dwell

    for (int i = 0; i <= steps; ++i)
    {
        float t = float(i) / steps;

        depositPoint(
            std::lerp(x0, x1, t),
            std::lerp(y0, y1, t),
            std::lerp(static_cast<float>(a.r), static_cast<float>(b.r), t) / 255.0f,
            std::lerp(static_cast<float>(a.g), static_cast<float>(b.g), t) / 255.0f,
            std::lerp(static_cast<float>(a.b), static_cast<float>(b.b), t) / 255.0f,
            dwell);
    }
}

void LaserRenderer::depositPoint(
    float x, float y,
    float r, float g, float b,
    float dwell)
{
    int cx = int(std::round(x));
    int cy = int(std::round(y));

    float sigma = m_settings.beamRadius;
    int radius = int(std::ceil(2.0f * sigma));

    for (int dy = -radius; dy <= radius; ++dy)
    {
        int py = cy + dy;
        if (py < 0 || py >= m_settings.height) continue;

        for (int dx = -radius; dx <= radius; ++dx)
        {
            int px = cx + dx;
            if (px < 0 || px >= m_settings.width) continue;

            float fx = (px + 0.5f) - x;
            float fy = (py + 0.5f) - y;
            float w = std::exp(-(fx * fx + fy * fy) / (2.0f * sigma * sigma));

            auto& e = m_energy[px + py * m_settings.width];
            e.r += r * w * dwell;
            e.g += g * w * dwell;
            e.b += b * w * dwell;
        }
    }
}
