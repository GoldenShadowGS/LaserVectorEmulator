#include "FrameRenderer.h"
#include "GalvoSimulator.h"
#include <stdexcept>
#pragma comment(lib, "d2d1.lib")
#undef max
#undef min

FrameRenderer::FrameRenderer(HWND hwnd)
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

FrameRenderer::~FrameRenderer()
{
    if (pBrush) pBrush->Release();
    if (pRenderTarget) pRenderTarget->Release();
    if (pFactory) pFactory->Release();
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

void FrameRenderer::DrawFrame(const SimFrame& frame)
{
    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));


    if (!frame.empty())
    {
		const size_t stride = 1;
        for (size_t i = 0; i < frame.size() - 1; i += stride)
        {
            const auto& p1 = frame[i];
            const auto& p2 = frame[i + 1];

            if (!p1.flags)
                continue;

            D2D1_POINT_2F r1 = SimToScreen(p1.x,p1.y);
            D2D1_POINT_2F r2 = SimToScreen(p2.x, p2.y);
			FLOAT size = 2.0f;

            pBrush->SetColor(D2D1::ColorF(p1.r / 255.0f, p1.g / 255.0f, p1.b / 255.0f));
            pRenderTarget->DrawLine(r1, r2, pBrush, 1.0f);
            //pRenderTarget->FillEllipse(D2D1::Ellipse(r1, size, size), pBrush);
        }
    }

    pRenderTarget->EndDraw();
}

D2D1_POINT_2F FrameRenderer::SimToScreen(float x, float y) const
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

    //outX = (drawX + 1.0f) * 0.5f * m_width;
    //outY = int((drawY + 1.0f) * 0.5f * m_height);
    return D2D1::Point2F((drawX + 1.0f) * 0.5f * m_width, (drawY + 1.0f) * 0.5f * m_height);
}

