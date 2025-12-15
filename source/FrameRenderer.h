#pragma once
#include <d2d1.h>
#include <vector>
//#include "GalvoSimulator.h"




class FrameRenderer
{
public:
    struct RenderPoint
    {
        float x; // -1.0 , 1.0
        float y; // -1.0 , 1.0
        uint8_t r, g, b;
        uint8_t flags; // 1 = laser on, 0 = blank
    };

    using RenderFrame = std::vector<RenderPoint>;
    FrameRenderer() : m_width(10), m_height(10) { }
    ~FrameRenderer();
	void Initialize(HWND hwnd);
    void OnResize(int width, int height);
    void DrawFrame();
	RenderFrame& getRenderFrame() { return renderFrame; }
    int getScreenWidth() { return m_width; }
    int getScreenHeight() { return m_height; }

private:
    void SimToScreen(float x, float y, int& outX, int& outY);
    RenderFrame renderFrame;
    ID2D1Factory* pFactory = nullptr;
    ID2D1HwndRenderTarget* pRenderTarget = nullptr;
    ID2D1SolidColorBrush* pBrush = nullptr; // single reusable brush
    int m_width;
    int m_height;
};
