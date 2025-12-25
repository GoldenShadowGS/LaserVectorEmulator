#pragma once
#include <d2d1.h>
#include <vector>

struct SimPoint;
using SimFrame = std::vector<SimPoint>;

class FrameRenderer
{
public:
    FrameRenderer(HWND hwnd);
    ~FrameRenderer();
    void OnResize(int width, int height);
    void DrawFrame(const SimFrame& frame);
    int getScreenWidth() const { return m_width; }
    int getScreenHeight() const { return m_height; }

private:
    D2D1_POINT_2F SimToScreen(float x, float y) const;
    ID2D1Factory* pFactory = nullptr;
    ID2D1HwndRenderTarget* pRenderTarget = nullptr;
    ID2D1SolidColorBrush* pBrush = nullptr; // single reusable brush
    int m_width;
    int m_height;
};
