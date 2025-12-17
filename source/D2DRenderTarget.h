#pragma once
#include <d2d1.h>
#include <vector>

class D2DRenderTarget
{
public:
    ~D2DRenderTarget();
	void Initialize(HWND hwnd);
    void OnResize(int width, int height);
    void DrawFrame();
    int getScreenWidth() const { return m_width; }
    int getScreenHeight() const { return m_height; }
	ID2D1HwndRenderTarget* GetD2DRenderTarget() { return pRenderTarget; }

private:
    ID2D1Factory* pFactory = nullptr;
    ID2D1HwndRenderTarget* pRenderTarget = nullptr;
    ID2D1SolidColorBrush* pBrush = nullptr; // single reusable brush
    int m_width = 0;
    int m_height = 0;
};
