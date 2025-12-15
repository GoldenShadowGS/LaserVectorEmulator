#include "FrameRenderer.h"
#include <stdexcept>
#pragma comment(lib, "d2d1.lib")
#undef max
#undef min

FrameRenderer::~FrameRenderer()
{
    if (pBrush) pBrush->Release();
    if (pRenderTarget) pRenderTarget->Release();
    if (pFactory) pFactory->Release();
}

void FrameRenderer::Initialize(HWND hwnd)
{
    D2D1_FACTORY_OPTIONS options {};
#if defined(_DEBUG)
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &pFactory);
    if (FAILED(hr)) throw std::runtime_error("Failed to create D2D factory");

    RECT rc;
    GetClientRect(hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
    m_width = size.width;
    m_height = size.height;

    D2D1_HWND_RENDER_TARGET_PROPERTIES rtProps =
        D2D1::HwndRenderTargetProperties(hwnd, size);

    hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), rtProps, &pRenderTarget);
    if (FAILED(hr)) throw std::runtime_error("Failed to create render target");

    hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
    if (FAILED(hr)) throw std::runtime_error("Failed to create brush");
}

void FrameRenderer::OnResize(int width, int height)
{
    m_width = width;
    m_height = height;

    if (pRenderTarget)
    {
        pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

void FrameRenderer::DrawFrame()
{
    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    if (renderFrame.size() > 1)
    {
        for (size_t i = 0; i < renderFrame.size() - 1; i++)
        {
            const auto& a = renderFrame[i];
            const auto& b = renderFrame[i + 1];

            //if (!a.flags) 
            //    continue;

            int p1x;
            int p1y;
            int p2x;
			int p2y;
            SimToScreen(a.x,a.y,p1x,p1y);
            SimToScreen(b.x, b.y, p2x, p2y);

            pBrush->SetColor(D2D1::ColorF(a.r / 255.0f, a.g / 255.0f, a.b / 255.0f));
            pRenderTarget->DrawLine(D2D1::Point2F((float)p1x, (float)p1y), D2D1::Point2F((float)p2x, (float)p2y), pBrush, 1.0f);
        }
    }

    pRenderTarget->EndDraw();
}

void FrameRenderer::SimToScreen(float x, float y, int& outX, int& outY)
{
    float winAspect = float(m_width) / float(m_height);

    float sx = 1.0f;
    float sy = 1.0f;

    if (winAspect > 1.0f)
    {
        // window wider than simulation square
        sx = 1.0f / winAspect;
        sy = 1.0f;
    }
    else
    {
        // window taller than simulation square
        sx = 1.0f;
        sy = winAspect;
    }

    float drawX = x * sx;
    float drawY = y * sy;

    outX = int((drawX + 1.0f) * 0.5f * m_width);
    outY = int((drawY + 1.0f) * 0.5f * m_height);
}


//void FrameRenderer::RenderSimulated(const std::vector<SimPoint>& samples)
//{
//    if (!pRenderTarget) return;
//
//    pRenderTarget->BeginDraw();
//    pRenderTarget->Clear(D2D1::ColorF(0, 0, 0));
//
//    D2D1_SIZE_F size = pRenderTarget->GetSize();
//    float cx = size.width * 0.5f;
//    float cy = size.height * 0.5f;
//    float scale = std::min(cx, cy) * 0.9f;
//
//    // Use one brush for all colors
//    if (!pBrush) pRenderTarget->CreateSolidColorBrush(
//        D2D1::ColorF(1, 1, 1), &pBrush);
//
//    bool penDown = false;
//    float px = 0, py = 0;
//
//    for (const SimPoint& s : samples)
//    {
//        float x = cx + s.x * scale;
//        float y = cy - s.y * scale;
//
//        if (!s.blank)
//        {
//            pBrush->SetColor(D2D1::ColorF(
//                s.r / 255.0f,
//                s.g / 255.0f,
//                s.b / 255.0f
//            ));
//
//            if (penDown)
//            {
//                pRenderTarget->DrawLine(
//                    D2D1::Point2F(px, py),
//                    D2D1::Point2F(x, y),
//                    pBrush, 1.5f);
//            }
//            penDown = true;
//        }
//        else
//        {
//            penDown = false;
//        }
//
//        px = x;
//        py = y;
//    }
//
//    pRenderTarget->EndDraw();
//}

