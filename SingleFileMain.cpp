// LaserEmulator.cpp
// Minimal Laser Emulator (Win32 + Direct2D)
// Build: Visual Studio -> new Win32 Project, replace main cpp with this file.
// Link: d2d1.lib (add #pragma comment(lib, "d2d1.lib") present below)

#include <windows.h>
#include <d2d1.h>
#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <chrono>
#include <algorithm>
#undef max
#undef min
#pragma comment(lib, "d2d1.lib")

// --------------------------- Helper macros / releases -----------------------
template<typename T> void SafeRelease(T** p)
{
    if (p && *p) { (*p)->Release(); *p = nullptr; }
}

// --------------------------- Laser point format ----------------------------
struct LaserPoint
{
    int16_t x;   // -32768 .. +32767
    int16_t y;   // -32768 .. +32767
    uint8_t r, g, b; // 0..255
    uint8_t flags; // bit0 = 1 => laser on; 0 => blank
};

// --------------------------- Global Direct2D objects -----------------------
ID2D1Factory* g_factory = nullptr;
ID2D1HwndRenderTarget* g_rt = nullptr;
ID2D1SolidColorBrush* g_brush = nullptr;

// --------------------------- Parameters (tweak these) ----------------------
const float USABLE_RANGE = 0.80f;   // 80% of window extents used
const float PPS = 30000.0f;         // simulated points-per-second
const float TIME_PER_POINT = 1.0f / PPS;
const float BRIGHT_SCALE = 1.0f;    // arbitrary brightness scaling
const float SUBSEG_PIXEL = 3.0f;    // subdivide segments into steps of ~this px
const float MAX_DIM_ALPHA = 0.95f;
const float MIN_DIM_ALPHA = 0.03f;

// --------------------------- Coordinate mapping ----------------------------
static inline float clampf(float v, float a, float b)
{
    return (v < a) ? a : ((v > b) ? b : v);
}

// map int16 laser coords to screen pixels (y inverted for screen coords)
static void LaserMapToScreen(int16_t xi, int16_t yi, int width, int height, float& sx, float& sy)
{
    // normalize to -1..1
    float nx = (xi >= 0) ? (xi / 32767.0f) : (xi / 32768.0f);
    float ny = (yi >= 0) ? (yi / 32767.0f) : (yi / 32768.0f);
    // apply usable range margin
    float halfw = (width * 0.5f) * USABLE_RANGE;
    float halfh = (height * 0.5f) * USABLE_RANGE;
    sx = (width * 0.5f) + nx * halfw;
    sy = (height * 0.5f) - ny * halfh; // invert Y to screen coordinates
}

// --------------------------- 3D cube generator ------------------------------
// Returns LaserPoint vector using int16 XY coordinates. Adds small blanked jumps.
static std::vector<LaserPoint> GenerateRotatingCubeFrame(float t)
{
    // base cube vertices (size)
    const float half = 0.6f;
    std::array<std::array<float, 3>, 8> verts = { {
        {-half,-half,-half},{ half,-half,-half},{ half, half,-half},{-half, half,-half},
        {-half,-half, half},{ half,-half, half},{ half, half, half},{-half, half, half}
    } };

    // rotations (Y and X)
    float cy = std::cos(t); float sy = std::sin(t);
    float cx = std::cos(t * 0.6f); float sx = std::sin(t * 0.6f);

    auto rotate = [&] (const std::array<float, 3>& p) {
        // yaw around Y
        float x1 = p[0] * cy - p[2] * sy;
        float z1 = p[0] * sy + p[2] * cy;
        float y1 = p[1];
        // pitch around X
        float y2 = y1 * cx - z1 * sx;
        float z2 = y1 * sx + z1 * cx;
        return std::array<float, 3>{ x1, y2, z2 };
        };

    // project to 2D
    std::vector<std::pair<float, float>> proj;
    proj.reserve(8);
    const float focal = 1.2f;
    for (auto& v : verts)
    {
        auto r = rotate(v);
        float z = r[2] + 2.8f; // shift forward so z>0
        float px = (r[0] * focal) / z;
        float py = (r[1] * focal) / z;
        proj.push_back({ px, py });
    }

    // cube edges
    const int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    std::vector<LaserPoint> frame;
    frame.reserve(size_t(12) * 64);

    // For each edge, interpolate 12-32 points
    for (int ei = 0; ei < 12; ++ei)
    {
        const auto& a = proj[edges[ei][0]];
        const auto& b = proj[edges[ei][1]];
        // pick interpolation count proportional to projected length
        float dx = b.first - a.first;
        float dy = b.second - a.second;
        float len = std::sqrt(dx * dx + dy * dy);
        int samples = (int)clampf(16.0f + len * 80.0f, 12.0f, 80.0f);
        for (int s = 0; s <= samples; ++s)
        {
            float u = float(s) / float(samples);
            float nx = a.first * (1.0f - u) + b.first * u;
            float ny = a.second * (1.0f - u) + b.second * u;
            // map from approx normalized (-1..1) to int16 range
            float fx = clampf(nx, -1.0f, 1.0f) * 32767.0f;
            float fy = clampf(ny, -1.0f, 1.0f) * 32767.0f;
            LaserPoint p { (int16_t)fx, (int16_t)fy, 0, 255, 192, 1 };
            frame.push_back(p);
        }
        // blank jump between edges
        LaserPoint blank {};
        blank.x = 0; blank.y = 0; blank.r = blank.g = blank.b = 0; blank.flags = 0;
        frame.push_back(blank);
    }

    return frame;
}

// --------------------------- Drawing utilities -----------------------------
static void DrawAdditiveSegment(ID2D1RenderTarget* rt, float x0, float y0, float x1, float y1, float r, float g, float b, float alpha, float thickness)
{
    if (!rt) return;
    // create a temporary color brush (we reuse the global brush by SetColor to avoid many creations)
    D2D1_COLOR_F col = D2D1::ColorF(r, g, b, alpha);
    g_brush->SetColor(col);
    D2D1_POINT_2F p0 = D2D1::Point2F(x0, y0);
    D2D1_POINT_2F p1 = D2D1::Point2F(x1, y1);
    rt->DrawLine(p0, p1, g_brush, thickness);
}

// brightness model: longer distance -> lower alpha
static float ComputeSegmentAlpha(float pixelDist)
{
    if (pixelDist <= 1.0f) return MAX_DIM_ALPHA;
    // time spent on a pixel ~ time_per_point / pixelDist  (rough proxy)
    float raw = (TIME_PER_POINT * 1000.0f) / (pixelDist + 1.0f); // scale tweak
    float alpha = clampf(raw * BRIGHT_SCALE * 60.0f, MIN_DIM_ALPHA, MAX_DIM_ALPHA);
    return alpha;
}

// subdivide a segment into smaller steps so brightness & stroke coverage is uniform
static void DrawSegmentWithSubdivision(ID2D1RenderTarget* rt, float x0, float y0, float x1, float y1,
    uint8_t r8, uint8_t g8, uint8_t b8)
{
    float dx = x1 - x0;
    float dy = y1 - y0;
    float dist = std::hypot(dx, dy);
    if (dist < 1.0f)
    {
        // draw a short dot (small line)
        float alpha = ComputeSegmentAlpha(dist);
        DrawAdditiveSegment(rt, x0, y0, x1, y1, r8 / 255.0f, g8 / 255.0f, b8 / 255.0f, alpha, 2.0f);
        return;
    }

    int steps = std::max(1, (int)std::ceil(dist / SUBSEG_PIXEL));
    for (int i = 0; i < steps; ++i)
    {
        float tA = float(i) / float(steps);
        float tB = float(i + 1) / float(steps);
        float ax = x0 + dx * tA;
        float ay = y0 + dy * tA;
        float bx = x0 + dx * tB;
        float by = y0 + dy * tB;
        float segDist = std::hypot(bx - ax, by - ay);
        float alpha = ComputeSegmentAlpha(segDist);
        // thickness: slightly larger for short segments to look solid
        float thickness = clampf((3.0f * (1.0f - (segDist / 6.0f))) + 1.0f, 1.0f, 4.0f);
        DrawAdditiveSegment(rt, ax, ay, bx, by, r8 / 255.0f, g8 / 255.0f, b8 / 255.0f, alpha, thickness);
    }
}

// --------------------------- Graphics creation / resize --------------------
static HRESULT CreateGraphicsResources(HWND hwnd)
{
    if (g_rt) return S_OK;

    RECT rc;
    GetClientRect(hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    HRESULT hr = g_factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, size),
        &g_rt
    );
    if (SUCCEEDED(hr))
    {
        hr = g_rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &g_brush);
    }
    return hr;
}

static void DiscardGraphicsResources()
{
    SafeRelease(&g_rt);
    SafeRelease(&g_brush);
}

// --------------------------- Render frame ----------------------------------
static void RenderLaserFrame(HWND hwnd, const std::vector<LaserPoint>& frame)
{
    if (!g_rt) return;

    g_rt->BeginDraw();
    // clear black each frame (we use alpha blending to emulate persistence subtly)
    g_rt->Clear(D2D1::ColorF(0, 0, 0));

    int width = 0, height = 0;
    RECT rc; GetClientRect(hwnd, &rc);
    width = rc.right - rc.left; height = rc.bottom - rc.top;

    // iterate pairwise and draw segments (skip blanked points)
    for (size_t i = 0; i + 1 < frame.size(); ++i)
    {
        const LaserPoint& p0 = frame[i];
        const LaserPoint& p1 = frame[i + 1];

        float sx0, sy0, sx1, sy1;
        LaserMapToScreen(p0.x, p0.y, width, height, sx0, sy0);
        LaserMapToScreen(p1.x, p1.y, width, height, sx1, sy1);

        // distance on screen
        float dist = std::hypot(sx1 - sx0, sy1 - sy0);

        bool blank = (p0.flags == 0) || (p1.flags == 0);
        // large jumps are treated as blanked
        if (dist > std::max(width, height) * 0.6f) blank = true;

        if (!blank)
        {
            DrawSegmentWithSubdivision(g_rt, sx0, sy0, sx1, sy1, p0.r, p0.g, p0.b);
        }
        // else: just move pen (do nothing)
    }

    HRESULT hr = g_rt->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        DiscardGraphicsResources();
    }
}

// --------------------------- Win32 and main loop ----------------------------
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        if (!g_factory)
        {
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_factory);
        }
        return 0;

    case WM_SIZE:
        if (g_rt)
        {
            UINT width = LOWORD(lParam), height = HIWORD(lParam);
            g_rt->Resize(D2D1::SizeU(width, height));
        }
        return 0;

    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&g_factory);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    // Register class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LaserEmuWndClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);

    // Create window
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"Laser Emulator (Direct2D)",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // high-resolution timer
    using clock = std::chrono::high_resolution_clock;
    auto t0 = clock::now();
    float animT = 0.0f;

    // message + render loop
    MSG msg;
    bool running = true;
    while (running)
    {
        // pump messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!running) break;

        // ensure resources
        HRESULT hr = CreateGraphicsResources(hwnd);
        if (FAILED(hr))
        {
            Sleep(10);
            continue;
        }

        // advance animation time
        auto now = clock::now();
        std::chrono::duration<float> dt = now - t0;
        t0 = now;
        animT += dt.count() * 0.9f; // speed tweak

        // generate frame (replace with UDP receiver later)
        std::vector<LaserPoint> frame = GenerateRotatingCubeFrame(animT);

        // render frame
        RenderLaserFrame(hwnd, frame);

        // simple frame cap ~60Hz (cooperative)
        Sleep(1);
    }

    return 0;
}
