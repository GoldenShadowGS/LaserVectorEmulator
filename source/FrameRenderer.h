#pragma once
#include <d2d1.h>
#include "Helpers.h"
#include "GalvoSimulator.h"

class FrameRenderer
{
public:
    FrameRenderer(HWND hwnd);
    ~FrameRenderer();
    void OnResize(int width, int height);
    void RenderFrame(const LaserFrame& currentframe, const LaserFrame& previousframe);

private:
    void SimToScreen(float x, float y, int& outX, int& outY);
    ID2D1Factory* pFactory = nullptr;
    ID2D1HwndRenderTarget* pRenderTarget = nullptr;
    ID2D1SolidColorBrush* pBrush = nullptr; // single reusable brush
    int m_width;
    int m_height;
};
