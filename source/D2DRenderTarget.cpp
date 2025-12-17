#include "D2DRenderTarget.h"
#include <stdexcept>
#pragma comment(lib, "d2d1.lib")
#undef max
#undef min

D2DRenderTarget::~D2DRenderTarget()
{
    if (pBrush) pBrush->Release();
    if (pRenderTarget) pRenderTarget->Release();
    if (pFactory) pFactory->Release();
}

void D2DRenderTarget::Initialize(HWND hwnd)
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

void D2DRenderTarget::OnResize(int width, int height)
{
    m_width = width;
    m_height = height;

    if (pRenderTarget)
    {
        pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

void D2DRenderTarget::DrawFrame()
{
    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

}
